#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <SPIFFS.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include "home.h"
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include "savedData.h"
#include "settings.h"
#include <ESPmDNS.h>
#include <Update.h>

//buffers for temperature and humidity readings
std::vector<float> temperatureBuffer;
std::vector<float> humidityBuffer;

#define POST_METHOD 0
#define MQTT_METHOD 1

//  configFile at the global scope
const char *configFile = "/config.json";

const char *ap_ssid = "SH_ESP";
const char *ap_password = "simon&hafiz";
const IPAddress apIP(192, 168, 1, 1);
const IPAddress apGateway(192, 168, 1, 1);
const IPAddress apSubnet(255, 255, 255, 0);
WebServer server(80);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define LED_PIN 2 // LED pin
#define POWER_PIN 18
#define LIGHT_SENSOR_PIN 34 // LDR PIN
// Define DHT sensor
#define DHTPIN 14      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);


unsigned long lastSaveMillis = 0;
const unsigned long saveInterval = 60000; // Save records every 1 minute
unsigned long previousTempMillis = 0;
unsigned long previousHumidityMillis = 0;
unsigned long previousLDRMillis = 0;
unsigned long previousHeartbeatMillis = 0;
const long intervalTemperature = 6000; // Interval for temperature reading (6 seconds)
const long intervalHumidity = 3000;    // Interval for humidity reading (3 seconds)
const long intervalLDR = 6000;         // Interval for LDR reading (6 seconds)
const long intervalHeartbeat = 100;   // Interval for heartbeat signal (2 seconds)

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// default configuration settings
String uniqueID = "smartnode_sh";
float triggerTemperature = 25.0;
bool manualOverride = false;
int selectedMethod = POST_METHOD;
const int METHOD_ADDRESS = 0;
// Sensor variables
float temperature = 0.0;
float humidity = 0.0;
int ldrReading = 0;


#include "utils.h"
#include "extra_credit.h"
#include "ota.h"


void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT); // Initialize LED pin
    pinMode(POWER_PIN, OUTPUT);
    lcd.init();
    lcd.backlight();

    if (SPIFFS.begin(true))
    {
        // SPIFFS.format();
        Serial.println("Flash memory mounted successfully");
    }
    else
    {
        Serial.println("Failed to mount filesystem");
    }

    preferences.begin("wifi_config", false);
    // Check if WiFi is configured
    bool isWiFiConfigured = getWiFiConfigStatus();

    if (isWiFiConfigured)
    {
        // If configured, to connect to WiFi
        connectToWiFi();
        
    }
    else
    {
        setupAPMode();
    }

    preferences.end();


   // Synchronize time with NTP server
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    // Wait for time to be set
    while (!time(nullptr)) {
        delay(1000);
        Serial.println("Waiting for time to be set...");
    }
    // load saved configurations
    loadSavedConfigurations();
    // Start the server
    server.on("/", handleRoot);
    server.on("/temperature", HTTP_GET, handleTemperature);
    server.on("/humidity", HTTP_GET, handleHumidity);
    server.on("/fan", HTTP_POST, startFanManual);
    server.on("/clear", HTTP_POST, clearMemory);
    server.on("/ldrData", HTTP_GET, handleLDRData);
    server.on("/savedData", HTTP_GET, handleSavedData);
    server.on("/data", HTTP_GET, handleDataPage);
    server.on("/configurations", HTTP_POST, handleConfigurations);
    server.on("/settings", HTTP_GET, handleSettingsPage);
    server.on("/setupwifi", HTTP_GET, handleWifiConfigPage);
    server.on("/configure", HTTP_POST, handleWifiConfig);



 server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
    server.begin();
}

void loop()
{

    server.handleClient();
    // Handle OTA
    ArduinoOTA.handle();
    unsigned long currentMillis = millis();

    // Handle heartbeat LED blinking
    if (currentMillis - previousHeartbeatMillis >= intervalHeartbeat)
    {
        previousHeartbeatMillis = currentMillis;
        toggleHeartbeat();
    }

  
    // Read temperature every 6 seconds
    if (currentMillis - previousTempMillis >= intervalTemperature)
    {
        previousTempMillis = currentMillis;
        readTemperature();
        // Check if the temperature exceeds the set value
        if (!manualOverride){
          if (temperature > triggerTemperature)
          {
              startFan();
          }
          else
          {
              stopFan();
          } 
        }
    }

    // Read humidity every 3 seconds
    if (currentMillis - previousHumidityMillis >= intervalHumidity)
    {
        previousHumidityMillis = currentMillis;
        readHumidity();
    }

    // Read LDR every 6 seconds
    if (currentMillis - previousLDRMillis >= intervalLDR)
    {
        previousLDRMillis = currentMillis;
        readLDR();
    }

    // Save records to SPIFFS every minute
    if (currentMillis - lastSaveMillis >= saveInterval)
    {
        lastSaveMillis = currentMillis;
        saveRecords();
    }


}

void toggleHeartbeat()
{
    static bool ledState = false;
    digitalWrite(LED_PIN, ledState ? LOW : HIGH); // Toggle LED state
    ledState = !ledState;
}


void startFan()
{
    //  the fan is controlled via a relay
    digitalWrite(POWER_PIN, HIGH); // Turn on the fan
    Serial.println("Fan started");
}

void stopFan()
{
    // the fan is controlled via a relay connected
    digitalWrite(POWER_PIN, LOW); // Turn off the fan
    Serial.println("Fan stopped");
}



void handleRoot()
{
    String homePage = String(HOME_PAGE);
    homePage.replace("{uniqueID}", uniqueID);
    homePage.replace("{isAPModeActive}", nodeDetails.ipAddress ? "true" : "false");
    homePage.replace("{isStatModeActive}", nodeDetails.uniqueMacId ? "true" : "false");
    homePage.replace("{isManualOverrideOn}", manualOverride ? "true" : "false");
    server.send(200, "text/html", homePage );
}

void handleSettingsPage() {
    // Construct the HTML content with initial settings
    String settingsPage = String(SETTINGS_PAGE);
    settingsPage.replace("{uniqueID}", uniqueID);
    settingsPage.replace("{triggerTemperature}", String(triggerTemperature));
    settingsPage.replace("{postingMethod}", selectedMethod == POST_METHOD ? "post" : "mqtt");
    settingsPage.replace("{manualOverrideChecked}", manualOverride ? "true" : "false");

    // Send the HTML response with initial settings
    server.send(200, "text/html", settingsPage);
}


void handleDataPage()
{
    server.send_P(200, "text/html", SAVED_DATA_PAGE);
}

void handleTemperature()
{
    server.send(200, "text/plain", String(temperature));
}

void handleHumidity()
{
    server.send(200, "text/plain", String(humidity));
}


void clearMemory() {
  // Clear SPIFFS file system
  if (SPIFFS.begin()) {
    SPIFFS.format();
    SPIFFS.end();
    Serial.println("SPIFFS file system cleared");
  } else {
    Serial.println("SPIFFS mount failed");
  }
  preferences.begin("wifi_config", false);
  preferences.clear();
  preferences.end();
  Serial.println("Preferences storage cleared");
  ESP.restart();
  server.send(200, "text/plain", "Memory cleared successfully!");
}



void startFanManual() {
    if (!manualOverride) {
        server.send(400, "text/plain", "Manual override is not enabled");
        return;
    }

    if (server.hasArg("action")) {
        String action = server.arg("action");

        if (strcmp(action.c_str(), "start") == 0) {
            startFan();
            server.send(200, "text/plain", "Fan started");
        } else if (strcmp(action.c_str(), "stop") == 0) {
            stopFan();
            server.send(200, "text/plain", "Fan stopped");
        } else {
            server.send(400, "text/plain", "Invalid request");
        }
    } else {
        server.send(400, "text/plain", "No action specified");
    }
}


void handleSavedData()
{
    // Read saved sensor data from SPIFFS
    String savedData = readSavedData();
    Serial.println(savedData);
    server.send(200, "application/json", savedData);
}


void handleLDRData()
{
    Serial.println("Starting LDR data fetch...");

    HTTPClient http;

    // Make GET request to the PHP server
    http.begin("http://172.16.11.84/iot/final/ldr.php?uniqueMacId=" + String(nodeDetails.uniqueMacId));
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        if (httpResponseCode == 200)
        {
            // Read the response from the server
            String response = http.getString();
            Serial.println("LDR data fetched successfully"); // Debug success
            server.send(200, "application/json", response);
        }
        else
        {
            Serial.print("Error fetching LDR data. HTTP response code: ");
            Serial.println(httpResponseCode);
            server.send(500, "text/plain", "Error fetching LDR data");
        }
    }
    else
    {
        Serial.print("Error in the HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
        server.send(500, "text/plain", "Error fetching LDR data");
    }

    http.end();
}

#define MAX_JSON_DOC_SIZE 256


// Function to save records to file with timestamps
void saveRecords() {
    // Get current time
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    // Set time zone to GMT
    setenv("TZ", "GMT0", 1);
    tzset();

    // Open files for writing temperature and humidity records
    File tempFile = SPIFFS.open("/temperature.txt", "w"); // Use "a" to append to the file
    File humFile = SPIFFS.open("/humidity.txt", "w");     // Use "a" to append to the file
    if (!tempFile || !humFile) {
        Serial.println("Failed to open files for writing");
        return;
    }

    // Iterate over the temperature buffer and save readings to the temperature file
    for (float temp : temperatureBuffer) {
        char timestamp[20]; // Buffer for timestamp
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo); // Format timestamp
        tempFile.printf("%s,%f\n", timestamp, temp); // Write timestamp and temperature to file
    }

    // Iterate over the humidity buffer and save readings to the humidity file
    for (float hum : humidityBuffer) {
        char timestamp[20]; // Buffer for timestamp
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo); // Format timestamp
        humFile.printf("%s,%f\n", timestamp, hum); // Write timestamp and humidity to file
    }

    // Close files
    tempFile.close();
    humFile.close();

    // Clear the buffers after saving records
    temperatureBuffer.clear();
    humidityBuffer.clear();

    Serial.println("Records saved successfully");
}

// Function to read saved data from file with timestamps
String readSavedData() {
    // Open temperature file for reading
    File tempFile = SPIFFS.open("/temperature.txt", "r");
    if (!tempFile) {
        Serial.println("Failed to open temperature file for reading");
        return "[]";
    }

    // Create a JSON array to store temperature records
    DynamicJsonDocument tempDoc(MAX_JSON_DOC_SIZE);
    JsonArray tempRecords = tempDoc.to<JsonArray>();

    // Read temperature records from the file
    while (tempFile.available()) {
        String tempLine = tempFile.readStringUntil('\n');
        tempLine.trim();
        tempRecords.add(tempLine); // Add timestamped temperature data to array
    }

    tempFile.close();

    // Open humidity file for reading
    File humFile = SPIFFS.open("/humidity.txt", "r");
    if (!humFile) {
        Serial.println("Failed to open humidity file for reading");
        return "[]";
    }

    // Create a JSON array to store humidity records
    DynamicJsonDocument humDoc(MAX_JSON_DOC_SIZE);
    JsonArray humRecords = humDoc.to<JsonArray>();

    // Read humidity records from the file
    while (humFile.available()) {
        String humLine = humFile.readStringUntil('\n');
        humLine.trim();
        humRecords.add(humLine); // Add timestamped humidity data to array
    }

    humFile.close();

    // Create an array to hold both temperature and humidity arrays
    DynamicJsonDocument resultDoc(MAX_JSON_DOC_SIZE);
    JsonArray resultArray = resultDoc.to<JsonArray>();

    // Add temperature and humidity arrays to the result array
    resultArray.add(tempRecords);
    resultArray.add(humRecords);

    // Serialize the result array to a string
    String jsonData;
    serializeJson(resultArray, jsonData);

    return jsonData;
}


void readTemperature()
{
    // Read temperature from DHT sensor
    float newTemp = dht.readTemperature();
    // Check if reading is valid
    if (isnan(newTemp))
    {
        Serial.println("Error reading temp from DHT sensor");
    }

    else if (!isnan(newTemp))
    {
        temperature = newTemp;
        temperatureBuffer.push_back(newTemp);

            // Post sensor data using the current selected method
        if (selectedMethod == POST_METHOD)
        {
            postSensorDataHTTP(1, newTemp);
        }
        else if (selectedMethod == MQTT_METHOD)
        {
          postSensorDataMQTT(1, newTemp, "temp");
        }
    }

    // Print temperature value to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Update LCD display
      lcd.setCursor(0, 1);
      lcd.print("T:" + String(temperature) + ",H:" + String(humidity) + ",L:" + String(ldrReading));
    // updateScrollText();

}

void readHumidity()
{
    // Read humidity from DHT sensor
    float newHumidity = dht.readHumidity();

    // Check if reading is valid
    if (isnan(newHumidity))
    {
        Serial.println("Error reading humidity from DHT sensor");
    }
    else if (!isnan(newHumidity))
    {
        // Update humidity value
        humidity = newHumidity;
        humidityBuffer.push_back(newHumidity);

        if (selectedMethod == POST_METHOD)
        {
            postSensorDataHTTP(2, newHumidity);
        }
        else if (selectedMethod == MQTT_METHOD)
        {
          postSensorDataMQTT(2, newHumidity, "hum");
        }
    }

    // Print humidity value to Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Update LCD display
     
      lcd.setCursor(0, 1);
      lcd.print("T:" + String(temperature) + ",H:" + String(humidity) + ",L:" + String(ldrReading));
}

void readLDR()
{
    // Read light intensity from LDR sensor
    int newLDRReading = analogRead(LIGHT_SENSOR_PIN);

    // Check if reading is valid
    if (newLDRReading < 0)
    {
        Serial.println("Error reading LDR sensor");
    }
    else if (newLDRReading > 0)
    {
        // Update LDR reading value
        ldrReading = newLDRReading;
        if (selectedMethod == POST_METHOD)
        {
            postSensorDataHTTP(3, newLDRReading);
        }
        else if (selectedMethod == MQTT_METHOD)
        {
          postSensorDataMQTT(3, newLDRReading, "ldr");
        }
        
    }

    // Print LDR reading to Serial Monitor
    Serial.print("LDR Reading: ");
    Serial.println(ldrReading);

    // Update LCD display
      lcd.setCursor(0, 1);
      lcd.print("T:" + String(temperature) + ",L:" + String(ldrReading));
    // updateScrollText();
    
}

void postSensorDataHTTP(int sensorId, float reading)
{
    HTTPClient http;

    // Construct the JSON payload
    String jsonData = "{\"sensorId\":" + String(sensorId) + ",\"reading\":" + String(reading) + ",\"nodeID\":" + String(nodeDetails.nodeId) + ",\"protocol\":\"HTTP\"" +  "}";

    // Make POST request to the HTTP server
    http.begin("http://172.16.11.84/iot/final/readings.php");
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0)
    {
        if (httpResponseCode == 200)
        {
            Serial.println("Sensor data posted successfully via HTTP");
        }
        else
        {
            Serial.print("Error posting sensor data via HTTP. HTTP response code: ");
            Serial.println(httpResponseCode);
            // Print response body
            String responseBody = http.getString();
            Serial.println("Response body: " + responseBody);
        }
    }
    else
    {
        Serial.print("Error in the HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
}

void postSensorDataMQTT(int sensorId, float reading, char* topic)
{
    if (!mqttClient.connected())
    {
        connectToMQTTBroker();
    }
    // Construct  JSON payload
    String jsonData = "{\"sensorId\":" + String(sensorId) + ",\"reading\":" + String(reading) + ",\"nodeID\":" + String(nodeDetails.nodeId) + ",\"protocol\":\"MQTT\"" +  "}";
    // String jsonData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"nodeID\":\"2\"" + ",\"protocol\":\"MQTT\"" + ",\"lightIntensity\":" + String(ldrReading) + "}";
    // Publish the JSON payload to the MQTT broker
    mqttClient.publish(("sensor_data/" + String(topic)).c_str(), jsonData.c_str(), 1);
    Serial.println("Sensor data published via MQTT");
}

void saveConfigurations()
{
    // Open the configuration file for writing
    File configFile = SPIFFS.open(::configFile, "w"); // Use ::configFile to access the global variable
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return;
    }
    // Create a JSON object to store configurations
    DynamicJsonDocument doc(256);
    JsonObject config = doc.to<JsonObject>();
    config["uniqueID"] = uniqueID;
    config["postingMethod"] = selectedMethod == MQTT_METHOD ? "mqtt" : "http";
    config["manualOverride"] = manualOverride;
    config["triggerTemperature"] = triggerTemperature;

    // Serialize the JSON object to a string
    serializeJson(doc, configFile);
    configFile.close();
    loadSavedConfigurations();
    Serial.println("Configurations saved successfully to SPIFFS");
}

void loadSavedConfigurations()
{
    Serial.println("Loading saved configurations from SPIFFS...");
    // Open the configuration file for reading
    File file = SPIFFS.open(configFile, "r");
    if (!file)
    {
        Serial.println("Config file not found. Using default values.");
        return;
    }
    // Read the entire file content into a string
    String configData = file.readString();
    file.close();
    // Deserialize the JSON string into a JsonObject
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, configData);
    if (error)
    {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.f_str());
        return;
    }

    JsonObject config = doc.as<JsonObject>();
    // Read configuration values from the JSON object
    if (config.containsKey("uniqueID"))
    {
        uniqueID = config["uniqueID"].as<String>(); // Convert to String
    }
    if (config.containsKey("postingMethod"))
    {
        String method = config["postingMethod"].as<String>(); // Convert to String
        if (method == "mqtt")
        {
            selectedMethod = MQTT_METHOD;
        }
        else
        {
            selectedMethod = POST_METHOD;
        }
    }
    if (config.containsKey("manualOverride"))
    {
        manualOverride = config["manualOverride"].as<bool>(); // Convert to bool
    }
    if (config.containsKey("triggerTemperature"))
    {
        triggerTemperature = config["triggerTemperature"].as<float>(); // Convert to float
    }

    Serial.println("Configurations loaded successfully from SPIFFS");
}

void handleConfigurations()
{
    Serial.println();
    Serial.println("Wait a second while we update your configuration settings");
    server.sendHeader("Content-Type", "application/x-www-form-urlencoded");
    server.send(200, "text/plain", "Data received successfully");
    if (server.hasArg("uniqueID"))
    {
        uniqueID = server.arg("uniqueID");
        Serial.print("New uniqueID: ");
        Serial.println(uniqueID);
    }
    if (server.hasArg("postingMethod"))
    {
        String method = server.arg("postingMethod");
        if (method == "mqtt")
        {
            selectedMethod = MQTT_METHOD;
        }
        else
        {
            selectedMethod = POST_METHOD;
        }
    }
    if (server.hasArg("valueField"))
    {
        manualOverride = server.arg("valueField") == "on";
    }
    if (server.hasArg("triggerTemp"))
    {
        triggerTemperature = server.arg("triggerTemp").toFloat();
    }

    // Save configurations to SPIFFS after receiving updates
    saveConfigurations();
    Serial.println("Configuration settings successfully updated");
}
