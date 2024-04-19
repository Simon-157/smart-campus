
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
