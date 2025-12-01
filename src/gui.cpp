#include "gui.h"
#include <M5Unified.h>

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
    // needs to show topic + message
}