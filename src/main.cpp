//Everything is in this include chain.
#include "sensors.h"


unsigned long lastPublish = 0;
const unsigned long publishInterval = 1000;

void setup() {
    Serial.begin(115200);
    Serial.println("** Starting Setup **");
    
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    
    setup_sensors();
    setup_wifi();
    initMQTT();
    connectToMQTTBroker();
    
    updateBackground();
    Serial.println("** Ready! **");
}

void loop() {
    M5.update();
    
    if (!client.connected()) {
        connectToMQTTBroker();
    }
    client.loop();
    
    checkPowerButton();
    
    if (isSending && (millis() - lastPublish >= publishInterval)) {
        lastPublish = millis();
        publishSensorData();
    }
}