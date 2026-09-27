```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>

// =====================================================
// Project Information
// =====================================================

#define FIRMWARE_VERSION "0.2.0"

const char* DEVICE_NAME = "ESP32-IoT";


// =====================================================
// Hardware
// =====================================================

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const int LED_PIN = LED_BUILTIN;

bool ledState = false;


// =====================================================
// Web Server
// =====================================================

WebServer server(80);


// =====================================================
// Preferences
// Stores Wi-Fi credentials in ESP32 flash
// =====================================================

Preferences preferences;


// =====================================================
// OTA Authentication
// =====================================================

const char* OTA_USER = "admin";
const char* OTA_PASS = "esp32";


// =====================================================
// Wi-Fi Configuration
// =====================================================

String wifiSSID;
String wifiPassword;

bool accessPointMode = false;


// =====================================================
// Forward Declarations
// =====================================================

void handleRoot();
void handleLedOn();
void handleLedOff();

void handleWiFiSetup();
void handleWiFiSave();
void handleWiFiReset();

void handleUpdatePage();
void handleFirmwareUpdate();
void handleFirmwareUpload();

void startAccessPoint();
bool connectToWiFi();

String makePortalPage();
String makeWiFiSetupPage();


// =====================================================
// Load Wi-Fi credentials
// =====================================================

void loadWiFiCredentials()
{
    preferences.begin("wifi", true);

    wifiSSID =
        preferences.getString("ssid", "");

    wifiPassword =
        preferences.getString("password", "");

    preferences.end();


    Serial.println();
    Serial.println("Wi-Fi credentials:");

    if (wifiSSID.length() == 0)
    {
        Serial.println("No Wi-Fi credentials stored.");
    }
    else
    {
        Serial.print("SSID: ");
        Serial.println(wifiSSID);
    }
}


// =====================================================
// Save Wi-Fi credentials
// =====================================================

void saveWiFiCredentials(
    const String& ssid,
    const String& password
)
{
    preferences.begin("wifi", false);

    preferences.putString(
        "ssid",
        ssid
    );

    preferences.putString(
        "password",
        password
    );

    preferences.end();

    wifiSSID = ssid;
    wifiPassword = password;
}


// =====================================================
// Clear Wi-Fi credentials
// =====================================================

void clearWiFiCredentials()
{
    preferences.begin("wifi", false);

    preferences.clear();

    preferences.end();

    wifiSSID = "";
    wifiPassword = "";

    Serial.println(
        "Wi-Fi credentials cleared."
    );
}


// =====================================================
// Start ESP32 Access Point
// =====================================================

void startAccessPoint()
{
    accessPointMode = true;

    WiFi.mode(WIFI_AP);

    String apName =
        String(DEVICE_NAME) +
        "-" +
        String((uint32_t)ESP.getEfuseMac(), HEX);

    bool result =
        WiFi.softAP(
            apName.c_str(),
            "esp32setup"
        );

    if (result)
    {
        Serial.println();
        Serial.println(
            "================================"
        );

        Serial.println(
            "ESP32 Access Point Started"
        );

        Serial.print(
            "AP Name: "
        );

        Serial.println(
            apName
        );

        Serial.println(
            "AP Password: esp32setup"
        );

        Serial.print(
            "Setup URL: http://"
        );

        Serial.println(
            WiFi.softAPIP()
        );

        Serial.println(
            "================================"
        );
    }
    else
    {
        Serial.println(
            "Failed to start Access Point."
        );
    }
}


// =====================================================
// Connect to configured Wi-Fi
// =====================================================

bool connectToWiFi()
{
    if (wifiSSID.length() == 0)
    {
        return false;
    }


    accessPointMode = false;

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        wifiSSID.c_str(),
        wifiPassword.c_str()
    );


    Serial.println();
    Serial.println(
        "Connecting to Wi-Fi..."
    );

    Serial.print(
        "SSID: "
    );

    Serial.println(
        wifiSSID
    );


    const unsigned long timeout =
        15000;

    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < timeout
    )
    {
        delay(500);

        Serial.print(".");
    }


    Serial.println();


    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(
            "Wi-Fi connected."
        );

        Serial.print(
            "IP Address: "
        );

        Serial.println(
            WiFi.localIP()
        );

        Serial.print(
            "MAC Address: "
        );

        Serial.println(
            WiFi.macAddress()
        );

        return true;
    }


    Serial.println(
        "Wi-Fi connection failed."
    );

    return false;
}


// =====================================================
// Main Portal HTML
// =====================================================

String makePortalPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>ESP32 IoT Portal</title>


<style>

body
{
    font-family: Arial, sans-serif;

    background: #f2f2f2;

    margin: 0;

    padding: 20px;

    color: #222;
}


.container
{
    max-width: 800px;

    margin: auto;
}


.card
{
    background: white;

    padding: 20px;

    margin-bottom: 20px;

    border-radius: 12px;

    box-shadow:
        0 2px 8px
        rgba(0,0,0,0.12);
}


h1
{
    margin-top: 0;
}


h2
{
    margin-top: 0;
}


.status
{
    font-size: 22px;

    font-weight: bold;

    margin: 15px 0;
}


.on
{
    color: green;
}


.off
{
    color: red;
}


.mode
{
    font-weight: bold;
}


button
{
    border: none;

    border-radius: 8px;

    padding: 14px 28px;

    margin: 5px;

    font-size: 18px;

    cursor: pointer;
}


.onButton
{
    background: #28a745;

    color: white;
}


.offButton
{
    background: #dc3545;

    color: white;
}


.otaButton
{
    background: #007bff;

    color: white;
}


.setupButton
{
    background: #6f42c1;

    color: white;
}


.resetButton
{
    background: #555;

    color: white;
}


table
{
    width: 100%;

    border-collapse: collapse;
}


td
{
    padding: 9px;

    border-bottom:
        1px solid #ddd;
}


td:first-child
{
    font-weight: bold;

    width: 45%;
}


a
{
    text-decoration: none;
}

</style>

</head>


<body>


<div class="container">


<div class="card">

<h1>
ESP32 IoT Portal
</h1>


<p>

Network Mode:

<strong>
NETWORK_MODE
</strong>

</p>


<div class="status">

LED Status:

<span class="STATUS_CLASS">
STATUS_TEXT
</span>

</div>


<a href="/led/on">

<button class="onButton">
ON
</button>

</a>


<a href="/led/off">

<button class="offButton">
OFF
</button>

</a>

</div>


<div class="card">

<h2>
ESP32 Information
</h2>


<table>


<tr>

<td>
Device Name
</td>

<td>
DEVICE_NAME
</td>

</tr>


<tr>

<td>
Firmware
</td>

<td>
FIRMWARE_VERSION
</td>

</tr>


<tr>

<td>
Chip Model
</td>

<td>
CHIP_MODEL
</td>

</tr>


<tr>

<td>
Chip Revision
</td>

<td>
CHIP_REVISION
</td>

</tr>


<tr>

<td>
CPU Frequency
</td>

<td>
CPU_FREQ MHz
</td>

</tr>


<tr>

<td>
Flash Size
</td>

<td>
FLASH_SIZE MB
</td>

</tr>


<tr>

<td>
Free Heap
</td>

<td>
FREE_HEAP bytes
</td>

</tr>


<tr>

<td>
MAC Address
</td>

<td>
MAC_ADDRESS
</td>

</tr>


<tr>

<td>
IP Address
</td>

<td>
IP_ADDRESS
</td>

</tr>


<tr>

<td>
Uptime
</td>

<td>
UPTIME seconds
</td>

</tr>


</table>

</div>


<div class="card">

<h2>
Firmware Update
</h2>


<p>

Current Firmware:

<strong>
FIRMWARE_VERSION
</strong>

</p>


<a href="/update">

<button class="otaButton">

OTA Firmware Update

</button>

</a>

</div>


<div class="card">

<h2>
Wi-Fi
</h2>


<a href="/wifi">

<button class="setupButton">

Wi-Fi Setup

</button>

</a>


<a href="/wifi/reset">

<button class="resetButton">

Reset Wi-Fi

</button>

</a>

</div>


</div>


</body>

</html>
)rawliteral";


    String networkMode;

    if (accessPointMode)
    {
        networkMode =
            "ESP32 Access Point";
    }
    else
    {
        networkMode =
            "Wi-Fi Station";
    }


    String ipAddress;

    if (accessPointMode)
    {
        ipAddress =
            WiFi.softAPIP().toString();
    }
    else
    {
        ipAddress =
            WiFi.localIP().toString();
    }


    html.replace(
        "NETWORK_MODE",
        networkMode
    );


    html.replace(
        "STATUS_CLASS",
        ledState ? "on" : "off"
    );


    html.replace(
        "STATUS_TEXT",
        ledState ? "ON" : "OFF"
    );


    html.replace(
        "DEVICE_NAME",
        DEVICE_NAME
    );


    html.replace(
        "FIRMWARE_VERSION",
        FIRMWARE_VERSION
    );


    html.replace(
        "CHIP_MODEL",
        ESP.getChipModel()
    );


    html.replace(
        "CHIP_REVISION",
        String(ESP.getChipRevision())
    );


    html.replace(
        "CPU_FREQ",
        String(getCpuFrequencyMhz())
    );


    html.replace(
        "FLASH_SIZE",
        String(
            ESP.getFlashChipSize()
            /
            (1024 * 1024)
        )
    );


    html.replace(
        "FREE_HEAP",
        String(ESP.getFreeHeap())
    );


    html.replace(
        "MAC_ADDRESS",
        WiFi.macAddress()
    );


    html.replace(
        "IP_ADDRESS",
        ipAddress
    );


    html.replace(
        "UPTIME",
        String(
            millis() / 1000
        )
    );


    return html;
}


// =====================================================
// Wi-Fi Setup Page
// =====================================================

String makeWiFiSetupPage()
{
    String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>ESP32 Wi-Fi Setup</title>


<style>

body
{
    font-family: Arial;

    background: #f2f2f2;

    padding: 20px;
}


.card
{
    max-width: 600px;

    margin: auto;

    background: white;

    padding: 25px;

    border-radius: 12px;
}


input
{
    width: 100%;

    box-sizing: border-box;

    padding: 12px;

    margin:
        8px 0 18px 0;

    font-size: 16px;
}


button
{
    padding: 12px 25px;

    font-size: 17px;

    cursor: pointer;
}


.save
{
    background: #28a745;

    color: white;

    border: none;

    border-radius: 7px;
}


</style>

</head>


<body>


<div class="card">


<h1>
Wi-Fi Setup
</h1>


<form
method="POST"
action="/wifi/save">


<label>
Wi-Fi SSID
</label>


<input
type="text"
name="ssid"
required>


<label>
Wi-Fi Password
</label>


<input
type="password"
name="password">


<button
class="save"
type="submit">

Save and Connect

</button>


</form>


<p>

After saving,
ESP32 will restart its
Wi-Fi connection.

</p>


<p>

<a href="/">
Back to Portal
</a>

</p>


</div>


</body>

</html>

)rawliteral";


    return html;
}


// =====================================================
// Root Portal
// =====================================================

void handleRoot()
{
    server.send(
        200,
        "text/html; charset=UTF-8",
        makePortalPage()
    );
}


// =====================================================
// LED ON
// =====================================================

void handleLedOn()
{
    ledState = true;

    digitalWrite(
        LED_PIN,
        HIGH
    );


    server.sendHeader(
        "Location",
        "/"
    );


    server.send(
        303
    );
}


// =====================================================
// LED OFF
// =====================================================

void handleLedOff()
{
    ledState = false;

    digitalWrite(
        LED_PIN,
        LOW
    );


    server.sendHeader(
        "Location",
        "/"
    );


    server.send(
        303
    );
}


// =====================================================
// Wi-Fi Setup
// =====================================================

void handleWiFiSetup()
{
    server.send(
        200,
        "text/html; charset=UTF-8",
        makeWiFiSetupPage()
    );
}


// =====================================================
// Save Wi-Fi Settings
// =====================================================

void handleWiFiSave()
{
    if (
        !server.hasArg("ssid")
    )
    {
        server.send(
            400,
            "text/plain",
            "SSID is required."
        );

        return;
    }


    String newSSID =
        server.arg("ssid");


    String newPassword =
        server.arg("password");


    saveWiFiCredentials(
        newSSID,
        newPassword
    );


    String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<title>Wi-Fi Configuration</title>

</head>


<body>

<h1>
Wi-Fi Configuration Saved
</h1>


<p>
ESP32 is restarting Wi-Fi...
</p>


<p>
Please wait a few seconds.
</p>


</body>

</html>

)rawliteral";


    server.send(
        200,
        "text/html; charset=UTF-8",
        html
    );


    delay(1500);


    ESP.restart();
}


// =====================================================
// Reset Wi-Fi
// =====================================================

void handleWiFiReset()
{
    clearWiFiCredentials();


    String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<title>Wi-Fi Reset</title>

</head>


<body>

<h1>
Wi-Fi Settings Reset
</h1>


<p>
ESP32 will restart.
</p>


<p>
After restart,
connect to the ESP32-IoT
Access Point.
</p>


</body>

</html>

)rawliteral";


    server.send(
        200,
        "text/html; charset=UTF-8",
        html
    );


    delay(1500);


    ESP.restart();
}


// =====================================================
// OTA Page
// =====================================================

void handleUpdatePage()
{
    if (
        !server.authenticate(
            OTA_USER,
            OTA_PASS
        )
    )
    {
        return server.requestAuthentication();
    }


    String html = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>ESP32 OTA Update</title>


<style>

body
{
    font-family: Arial;

    background: #f2f2f2;

    padding: 20px;
}


.card
{
    max-width: 600px;

    margin: auto;

    background: white;

    padding: 25px;

    border-radius: 12px;
}


input
{
    width: 100%;

    margin: 15px 0;
}


button
{
    padding: 12px 25px;

    font-size: 17px;
}

</style>

</head>


<body>


<div class="card">


<h1>
ESP32 OTA Update
</h1>


<p>

Current firmware:

<strong>
)rawliteral";


    html += FIRMWARE_VERSION;


    html += R"rawliteral(
</strong>

</p>


<p>
Select the new firmware
<code>.bin</code> file.
</p>


<form
method="POST"
action="/update"
enctype="multipart/form-data">


<input
type="file"
name="firmware"
accept=".bin"
required>


<button type="submit">

Upload Firmware

</button>


</form>


<p>

<a href="/">
Back to Portal
</a>

</p>


</div>


</body>

</html>

)rawliteral";


    server.send(
        200,
        "text/html; charset=UTF-8",
        html
    );
}


// =====================================================
// OTA Upload Handler
// =====================================================

void handleFirmwareUpload()
{
    if (
        !server.authenticate(
            OTA_USER,
            OTA_PASS
        )
    )
    {
        return;
    }


    HTTPUpload& upload =
        server.upload();


    if (
        upload.status ==
        UPLOAD_FILE_START
    )
    {
        Serial.println();

        Serial.println(
            "OTA Update Started"
        );

        Serial.print(
            "File: "
        );

        Serial.println(
            upload.filename
        );


        if (
            !Update.begin(
                UPDATE_SIZE_UNKNOWN
            )
        )
        {
            Update.printError(
                Serial
            );
        }
    }


    else if (
        upload.status ==
        UPLOAD_FILE_WRITE
    )
    {
        if (
            Update.write(
                upload.buf,
                upload.currentSize
            )
            != upload.currentSize
        )
        {
            Update.printError(
                Serial
            );
        }
    }


    else if (
        upload.status ==
        UPLOAD_FILE_END
    )
    {
        if (
            Update.end(true)
        )
        {
            Serial.printf(
                "OTA Update Success: %u bytes\n",
                upload.totalSize
            );
        }
        else
        {
            Update.printError(
                Serial
            );
        }
    }


    else if (
        upload.status ==
        UPLOAD_FILE_ABORTED
    )
    {
        Update.abort();

        Serial.println(
            "OTA Update Aborted."
        );
    }
}


// =====================================================
// OTA Update Result
// =====================================================

void handleFirmwareUpdate()
{
    if (
        !server.authenticate(
            OTA_USER,
            OTA_PASS
        )
    )
    {
        return server.requestAuthentication();
    }


    if (
        Update.hasError()
    )
    {
        server.send(
            500,
            "text/plain",
            "Firmware update failed."
        );

        return;
    }


    server.send(
        200,
        "text/plain",
        "Firmware update successful. ESP32 will restart."
    );


    delay(1000);


    ESP.restart();
}


// =====================================================
// Register HTTP Routes
// =====================================================

void registerRoutes()
{
    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );


    server.on(
        "/led/on",
        HTTP_GET,
        handleLedOn
    );


    server.on(
        "/led/off",
        HTTP_GET,
        handleLedOff
    );


    server.on(
        "/wifi",
        HTTP_GET,
        handleWiFiSetup
    );


    server.on(
        "/wifi/save",
        HTTP_POST,
        handleWiFiSave
    );


    server.on(
        "/wifi/reset",
        HTTP_GET,
        handleWiFiReset
    );


    server.on(
        "/update",
        HTTP_GET,
        handleUpdatePage
    );


    server.on(
        "/update",
        HTTP_POST,
        handleFirmwareUpdate,
        handleFirmwareUpload
    );
}


// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);


    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "ESP32 IoT Test Firmware"
    );

    Serial.print(
        "Firmware Version: "
    );

    Serial.println(
        FIRMWARE_VERSION
    );

    Serial.println(
        "================================"
    );


    // -------------------------------------------------
    // LED
    // -------------------------------------------------

    pinMode(
        LED_PIN,
        OUTPUT
    );


    digitalWrite(
        LED_PIN,
        LOW
    );


    // -------------------------------------------------
    // Load Wi-Fi credentials
    // -------------------------------------------------

    loadWiFiCredentials();


    // -------------------------------------------------
    // Try connecting to saved Wi-Fi
    // -------------------------------------------------

    bool connected =
        connectToWiFi();


    // -------------------------------------------------
    // If connection failed, start AP
    // -------------------------------------------------

    if (!connected)
    {
        startAccessPoint();
    }


    // -------------------------------------------------
    // HTTP Routes
    // -------------------------------------------------

    registerRoutes();


    // -------------------------------------------------
    // Start Web Server
    // -------------------------------------------------

    server.begin();


    Serial.println();

    Serial.println(
        "HTTP server started."
    );


    if (accessPointMode)
    {
        Serial.print(
            "Portal: http://"
        );

        Serial.println(
            WiFi.softAPIP()
        );
    }
    else
    {
        Serial.print(
            "Portal: http://"
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "OTA: http://"
        );

        Serial.print(
            WiFi.localIP()
        );

        Serial.println(
            "/update"
        );
    }
}


// =====================================================
// Loop
// =====================================================

void loop()
{
    server.handleClient();
}
```
