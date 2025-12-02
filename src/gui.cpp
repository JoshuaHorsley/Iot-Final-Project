#include "gui.h"
#include <M5Unified.h>

static String currentTopic = "";
static String currentMessage = "";
static bool lockedToTopic = false;

void updateBackground(bool sending) {
    M5.Display.fillScreen(sending ? TFT_GREEN : TFT_BLUE);
    M5.Display.setTextColor(sending ? TFT_BLACK : TFT_WHITE);
    M5.Display.setTextSize(2);

    M5.Display.setCursor(10, 20);
    M5.Display.println("SENDING:");

    M5.Display.setCursor(10, 45);
    M5.Display.println(sending ? "ON" : "OFF");
}

void handleIncomingMqttMessage(const String& topic, const String& message) {
    Serial.print("GUI handling topic: ");
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
    M5.Display.setTextSize(2);

    M5.Display.setCursor(10, 10);
    M5.Display.print("Topic:");
    M5.Display.setCursor(10, 35);
    M5.Display.println(topic);

    M5.Display.setCursor(10, 55);
    M5.Display.print("Msg:");
    M5.Display.setCursor(10, 75);
    M5.Display.println(message);
}