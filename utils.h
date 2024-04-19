// broker connection strings
const char *mqttBroker = "172.16.11.84";
const int mqttPort = 1883;
const char *mqttTopic = "sensor_data";

void setupAPMode()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("Access Point mode enabled");
    lcd.setCursor(0, 0);
    lcd.print("AP:");
    lcd.print(WiFi.softAPIP());
}

void connectToMQTTBroker()
{
    mqttClient.setServer(mqttBroker, mqttPort);
    if (mqttClient.connect("ESP32Client"))
    {
        Serial.println("Connected to MQTT broker");
    }
    else
    {
        Serial.print("Failed to connect to MQTT broker, rc=");
        Serial.println(mqttClient.state());
    }
}


// Define a struct to hold node details
struct NodeDetails {
    int nodeId;
    int locationId;
    String nodeName;
    String ipAddress;
    String uniqueMacId;
};

// Declare a global variable to hold node details
NodeDetails nodeDetails;

void fetchNodeDetails()
{
 HTTPClient http;
    // Construct the URL with the ESP32's unique ID
    String url = "http://172.16.11.84/iot/final/espnode.php?uniqueMacId=" + String(ESP.getEfuseMac(), HEX);

    Serial.println("Fetching node details from: " + url);

    // Make GET request to fetch node details
    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        if (httpResponseCode == 200)
        {
            // Read the response from the server
            String response = http.getString();
            Serial.println("Node details fetched successfully"); // Debug success

            // Parse the JSON response to extract node details
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, response);
            if (!error)
            {
                // Extract node details
                nodeDetails.nodeId = doc["NodeID"];
                nodeDetails.locationId = doc["LocationID"];
                nodeDetails.nodeName = doc["NodeName"].as<String>();
                nodeDetails.ipAddress = doc["IPAddress"].as<String>();
                nodeDetails.uniqueMacId = doc["UniqueMacId"].as<String>();
            }
            else
            {
                Serial.println("Error parsing JSON response");
            }
        }
        else
        {
            Serial.print("Error fetching node details. HTTP response code: ");
            Serial.println(httpResponseCode);
        }
    }
    else
    {
        Serial.print("Error in the HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
}

