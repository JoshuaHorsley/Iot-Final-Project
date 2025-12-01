//Everything is in this include chain.
#include "sensors.h"

void read_buttons();
void setup();
void loop();


unsigned long lastPublish = 0;
const unsigned long publishInterval = 1000;
Microphone mic = Microphone();

static bool isSending = false;

void setup() {
    Serial.begin(115200);
    Serial.println("** Starting Setup **");

    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    
    if(!mic.Init()){
        return;
    }

    setup_sensors();
    setup_wifi();
    initMQTT();
    connectToMQTTBroker();

    if(!client.setBufferSize(RECORDING_MQTT_BUFFER_SIZE)){
        Serial.println("ERR:Main: MQTT Client buffer allocation failed.");
        return;
    }
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
        
    mic.RecordOneBlock();
    read_buttons();
    if(isShaking()){
        play_beep();
    }

    if (isSending && (millis() - lastPublish >= publishInterval)) {
        lastPublish = millis();
        publishSensorData();
    }
}


void read_buttons(){
    M5.update();

    if(M5.BtnA.wasPressed()){
        Serial.println("boop.");
        mic.PublishRecording();
    }
}