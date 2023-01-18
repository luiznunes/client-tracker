void saveConfigFile()
// Save Config in JSON format
{
  Serial.println(F("Saving configuration..."));
  
  // Create a JSON document
  StaticJsonDocument<2048> json;
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  // json["mqtt_user"] = mqtt_user;
  // json["mqtt_pass"] = mqtt_pass;

  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }

  // Serialize JSON data to write to file
  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
  Serial.println("Saved.");
  // Close file
  configFile.close();
}

bool loadConfigFile()
// Load existing configuration file
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();

  // Read configuration from FS json
  Serial.println("Mounting File System...");

  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // The file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("Opened configuration file");
        StaticJsonDocument<2048> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("Parsing JSON");

          // strcpy(mqtt_server, json["mqtt_server"]);
          // strcpy(mqtt_port, json["mqtt_port"]);
          // strcpy(mqtt_user, json["mqtt_user"]);
          // strcpy(mqtt_pass, json["mqtt_pass"]);

          return true;
        }
        else
        {
          // Error loading JSON data
          Serial.println("Failed to load json config");
        }
      }
    }
  }
  else
  {
    // Error mounting file system
    Serial.println("Failed to mount FS");
  }

  return false;
}


void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
  Serial.println("Entered Configuration Mode");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void setup_wifi()
{
  // Change to true when testing to force configuration every time we run
  bool forceConfig = false;

  bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup)
  {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }

  // Explicitly set WiFi mode
  WiFi.mode(WIFI_STA);

  // Setup Serial monitor
  Serial.begin(115200);
  delay(10);

  // Reset settings (only for development)
  wm.resetSettings();

  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  // Custom elements

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  // WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
  // WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);
  
  // Add all defined parameters
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  // wm.addParameter(&custom_mqtt_user);
  // wm.addParameter(&custom_mqtt_pass);

  if (forceConfig)
    // Run if we need a configuration
  {
    if (!wm.startConfigPortal("NEWTEST_AP", "password"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("NEWTEST_AP", "password"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  // If we get here, we are connected to the WiFi

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  const uint16_t mqtt_port_x = 1883; 
  client.setServer(mqtt_server, mqtt_port_x);
  // Lets deal with the user config values

  // Copy the string value
  // strcpy(mqtt_server, custom_mqtt_server.getValue());
  // strcpy(mqtt_port, custom_mqtt_port.getValue());
  // strcpy(mqtt_user, custom_mqtt_user.getValue());
  // strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  Serial.print("MQTT Server: ");
  Serial.println(mqtt_server);
  Serial.print("MQTT Port: ");
  Serial.println(mqtt_port);

  //Convert the number value
  // testNumber = atoi(custom_text_box_num.getValue());
  // Serial.print("testNumber: ");
  // Serial.println(testNumber);


  // Save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveConfigFile();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect(mqtt_id)) {
    // if (client.connect(mqtt_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}