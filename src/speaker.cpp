/*
    File: speaker.cpp
    Date: 2025-12-1
    Author: Josh Rice
    Description:
        This file is responsible for playing audio recordings received to the speaker
        and playing the beeping noise when the device is being shaken.
*/

#include "speaker.h"


void play_recording(char* buffer, size_t bufferSize){
    if(!M5.Speaker.isEnabled()){
        Serial.println("ERR:play_recording(): Speaker is not enabled.");
        return;
    }

    while(M5.Mic.isRecording()){
        delay(1);
    }

    M5.Mic.end();
    M5.Speaker.begin();

    const size_t adjustedBufferSize = bufferSize / 2;

    M5.Speaker.playRaw((int16_t*)buffer, adjustedBufferSize, 11025, false, 1, 0);

    do{
        delay(1);
        M5.update();
    }while(M5.Speaker.isPlaying());

    M5.Speaker.end();
    M5.Mic.begin();
}


void play_beep(){
if(!M5.Speaker.isEnabled()){
        Serial.println("ERR:play_beep(): Speaker is not enabled.");
        return;
    }

    while(M5.Mic.isRecording()){
        delay(1);
    }

    M5.Mic.end();
    M5.Speaker.begin();

    M5.Speaker.tone(8000, 250);


    do{
        delay(1);
        M5.update();
    }while(M5.Speaker.isPlaying());

    M5.Speaker.end();
    M5.Mic.begin();
}