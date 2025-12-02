/*
    File: gui.cpp
    Date: 2025-11-30
    Author: Josh Horsley & Christian Tan
    Description:
        This file contains the functions responsible for displaying the GUI
        on the m5 stick. This file is also responsible for locking and unlocking
        the display.
*/

#include "gui.h"

static String currentTopic = "";
static String currentMessage = "";
static bool lockedToTopic = false;

void updateBackground(bool sending) {
    M5.Display.fillScreen(sending ? TFT_GREEN : TFT_BLUE);
    M5.Display.setTextColor(sending ? TFT_BLACK : TFT_WHITE);

    M5.Display.setCursor(10, 20);
    M5.Display.println("SENDING:");

    M5.Display.setCursor(10, 45);
    M5.Display.println(sending ? "ON" : "OFF");
}

void handleIncomingMqttMessage(const String& topic, const String& message) {
    Serial.print("GUI topic: ");
    Serial.println(topic);

    if (!lockedToTopic) {
        currentTopic = topic;
        currentMessage = message;
        lockedToTopic = true;

        drawTopicMessage(currentTopic, currentMessage);
        return;
    }

    if (topic == currentTopic) {
        currentMessage = message;
        drawTopicMessage(currentTopic, currentMessage);
        return;
    }
}

void unlockTopic() {
    Serial.println("Topic unlocked");
    lockedToTopic = false;
}

void drawTopicMessage(const String& topic, const String& message) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);

    int x = 5;
    int y = 10;

    M5.Display.setCursor(x, y);
    M5.Display.print("Topic: ");

    y += 20;
    M5.Display.setCursor(x,y);
    M5.Display.println(topic);

    y += 30;
    M5.Display.setCursor(x, y);
    M5.Display.print("Msg: ");
    y += 20;
    M5.Display.setCursor(x, y);
    M5.Display.println(message);
}