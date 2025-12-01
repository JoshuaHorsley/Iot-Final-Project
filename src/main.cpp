//Everything is in this include chain.
#include "sensors.h"


unsigned long lastPublish = 0;
const unsigned long publishInterval = 1000;
static bool isSending = false;

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

    isSending = false; // device starts with sending off
    updateBackground(isSending);

    Serial.println("** Ready! **");
}

void loop() {
    M5.update();

    if (!client.connected()) {
        connectToMQTTBroker();
    }
    client.loop();

    bool prevSending = isSending;
    isSending = handlePowerButton(isSending);

    if (isSending != prevSending) {
        updateBackground(isSending);
    }

    if (isSending && (millis() - lastPublish >= publishInterval)) {
        lastPublish = millis();
        publishSensorData();
    }
}