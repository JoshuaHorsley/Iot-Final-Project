#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT credentials
const char* mqtt_username = "YOUR_USERNAME";
const char* mqtt_password = "YOUR_PASSWORD";
const char* mqtt_broker = "p07da41d.ala.us-east-1.emqxsl.com";
const int mqtt_port = 8883;

// Device ID (same as mqtt_username)
const char* deviceId = "YOUR_USERNAME";

#endif