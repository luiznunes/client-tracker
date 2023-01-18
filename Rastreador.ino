/*
  WiFiManager with saved textboxes Demo
  wfm-text-save-demo.ino
  Saves data in JSON file on ESP32
  Uses SPIFFS

  DroneBot Workshop 2022
  https://dronebotworkshop.com

  Functions based upon sketch by Brian Lough
  https://github.com/witnessmenow/ESP32-WiFi-Manager-Examples
*/

#define ESP_DRD_USE_SPIFFS true

// Include Libraries

// WiFi Library
#include <WiFi.h>
// File System Library
#include <FS.h>
// SPI Flash Syetem Library
#include <SPIFFS.h>
// WiFiManager Library
#include <WiFiManager.h>
// Arduino JSON library
#include <ArduinoJson.h>

#include <PubSubClient.h>

// JSON configuration file
#define JSON_CONFIG_FILE "/test_config.json"

// Flag for saving data
bool shouldSaveConfig = true;

#define mqtt_server       "192.168.3.3"
#define mqtt_port         "1883"
#define mqtt_user         "tracker"
#define mqtt_pass         "tracker"
#define mqtt_topic        "/tracker"
#define mqtt_id           "ESP32Client"
#include <TinyGPSPlus.h>
#include "HardwareSerial.h";

TinyGPSPlus gps;
WiFiManager wm;
WiFiClient espClient;

HardwareSerial SerialGPS(1);
PubSubClient client(espClient);

void setup(){
  setup_wifi();
  Serial.begin(115200); //Serial port of USB
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  while (SerialGPS.available() >0) {
      gps.encode(SerialGPS.read());
        String lat = String(gps.location.lat(), 6);
        String lng = String(gps.location.lng(), 6);

        String mqtt_json = String("{\"command\":\"set_geo\",\"disp_id\":\""mqtt_id"\",\"lat\":\"" + lat + "\",\"lng\":\"" + lng + "\"}");
        Serial.println(mqtt_json.c_str());
        
        if (!client.connected()) {
          reconnect();
        }
        client.loop();

        delay(30000);
        client.publish(mqtt_topic, mqtt_json.c_str(), true);
  }  
}