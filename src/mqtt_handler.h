#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

extern WiFiClientSecure espClient;
extern PubSubClient client;
extern String mqtt_base;

void setup_wifi();
void initMQTT();
void connectToMQTTBroker();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif