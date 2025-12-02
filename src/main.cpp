/*
    File: main.cpp
    Date: 2025-11-30
    Author: Christian Tan, Josh Horsley, Josh Rice
    Description:
        This file is responsible for handling the setup and loop of the application
        as well as power button and button A checks.
*/

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
    M5.Display.setRotation(1);
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
        unlockTopic();
    }


    if (isSending && (millis() - lastPublish >= publishInterval)) {
        lastPublish = millis();
        publishSensorData();
    }
}


void read_buttons(){
    M5.update();

    //If A button pressed, publish recording.
    if(M5.BtnA.wasPressed()){
        Serial.println("-- A BUTTON PRESS --");
        mic.PublishRecording();
    }
    //If pwr button pressed, toggle sending data.
    if(M5.BtnPWR.wasPressed()){
        Serial.println("-- PWR BUTTON PRESS --");
        isSending = !isSending;
        updateBackground(isSending);
    }
}