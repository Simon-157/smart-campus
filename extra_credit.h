#include "HardwareSerial.h"
#include <Preferences.h>

Preferences preferences;

void saveWiFiConfigStatus(bool configured) {
    preferences.begin("wifi_config", false);
    preferences.putBool("configured", configured);
    preferences.end();
    Serial.println("WiFi configuration status saved");
}

bool getWiFiConfigStatus() {
    preferences.begin("wifi_config", true); // Open preferences with namespace "wifi_config", read-only
    bool configured = preferences.getBool("configured", false); // Default to false if not found
    preferences.end();
    return configured;
}

void saveFirstConnectionStatus(bool isFirst) {
    preferences.begin("first_connect", true);
    preferences.putBool("isFirst", isFirst);
    preferences.end();
    Serial.println("First connection status saved");
}

bool isFirstConnection() {
    preferences.begin("first_connect", true);
    bool isFirst = preferences.getBool("isFirst", true);
    preferences.end();
    return isFirst;
}


void handleWifiConfigPage()
{
   String page = "<!DOCTYPE html><html><head><title>Configure WiFi</title></head><body style='font-family: Arial, sans-serif; background-color: #f2f2f2; display:flex; justify-content:center; align-items:center; height: 100vh;'>";
        page += "<div style='text-align: center;'>";
        page += "<h2 style='color: #333;'>Configure WiFi</h2>";
        page += "<form method='post' action='/configure' style='background-color: #fff; border-radius: 5px; padding: 20px; width: 300px; margin: 0 auto;'>";
        page += "<label for='ssid' style='display: block; margin-bottom: 5px;'>SSID:</label><br>";
        page += "<input type='text' id='ssid' name='ssid' style='width: 100%; padding: 10px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;'><br>";
        page += "<label for='password' style='display: block; margin-bottom: 5px;'>Password:</label><br>";
        page += "<input type='password' id='password' name='password' style='width: 100%; padding: 10px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;'><br>";
        page += "<input type='submit' value='Submit' style='background-color: #4CAF50; color: white; border: none; cursor: pointer; width: 100%; padding: 10px; margin-bottom: 10px; border-radius: 2px; box-sizing: border-box;'>";
        page += "</form></div></body></html>";


    server.send(200, "text/html", page);
}

void saveWiFiConfig(String ssid, String password)
{
    File ssidFile = SPIFFS.open("/ssid.txt", "w");
    File passwordFile = SPIFFS.open("/password.txt", "w");
    if (ssidFile && passwordFile) {
        ssidFile.println(ssid);
        passwordFile.println(password);
        ssidFile.close();
        passwordFile.close();
        Serial.println("WiFi configuration saved");

    } else {
        Serial.println("Failed to save WiFi configuration");
    }
}

void handleWifiConfig()
{
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Validate input
    if (ssid.length() > 0 && password.length() > 0) {
        // Save configuration to SPIFFS
        saveWiFiConfig(ssid, password);
        saveWiFiConfigStatus(true);
        server.send(200, "text/plain", "Configuration saved. Rebooting...");
        delay(2000);
        ESP.restart(); // Reboot to connect to configured WiFi
    } else {
        server.send(400, "text/plain", "Invalid SSID or password");
    }
}



void postSmartNodeCreationData(String nodeName, String locationID, String ipAddress, String uniqueMacId) {
    // Define your PHP endpoint URL
    String endpoint = "http://172.16.11.84/iot/final/node.php";
    
    // Create JSON payload
    String payload = "{\"nodeName\":\"" + nodeName + "\", \"locationID\":\"" + locationID + "\", \"ipAddress\":\"" + ipAddress + "\", \"uniqueMacId\":\"" + uniqueMacId + "\"}";
    // Send POST request with payload
    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);
    
    // Check HTTP response code
    if (httpResponseCode == HTTP_CODE_OK) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        // Check if the response contains a success message
        if (response.indexOf("Smart node inserted successfully") != -1) {
            // Smart node creation was successful, set first connection status to false
            saveFirstConnectionStatus(false);
        } else {
            Serial.println("Failed to insert smart node.");
        }
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}



String generateRandomName(int length) {
    // Define the characters from which to generate the random name
    const String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    String randomName = "";
    for (int i = 0; i < length; ++i) {
        // Generate a random index within the range of characters
        int randomIndex = random(characters.length());
        // Append the randomly selected character to the name
        randomName += characters.charAt(randomIndex);
    }

    return randomName;
}

void connectToWiFi()
{
    File ssidFile = SPIFFS.open("/ssid.txt", "r");
    File passwordFile = SPIFFS.open("/password.txt", "r");
    if (!ssidFile || !passwordFile) {
        Serial.println("WiFi configuration not found");
        return;
    }

    String ssid = ssidFile.readStringUntil('\n');
    String password = passwordFile.readStringUntil('\n');
    ssid.trim(); // Remove leading and trailing whitespace characters
    password.trim(); // Remove leading and trailing whitespace characters
    Serial.println(ssid);
    Serial.println(password);
    ssidFile.close();
    passwordFile.close();

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str()); 

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(1000);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("WiFi connected IP: ");
        Serial.print(WiFi.localIP());
        bool isFirstConn = isFirstConnection();
        if (isFirstConn) {
            // Fetch necessary details
            String nodeName = generateRandomName(10);  
            String locationID = "1";  
            String ipAddress = WiFi.localIP().toString();
            String uniqueMacId = String(ESP.getEfuseMac(), HEX);
            
            // Post node creation data
            postSmartNodeCreationData(nodeName, locationID, ipAddress, uniqueMacId);

            
        }
        Serial.println("");
        lcd.setCursor(0, 0);
        lcd.print("IP:");
        lcd.print(WiFi.localIP());
        fetchNodeDetails();
    } else {
        Serial.println("");
        Serial.println("Failed to connect to WiFi; will try to set it up in AP mode");
        setupAPMode();
    }
}

