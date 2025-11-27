#include "gui.h"
#include "mqtt_handler.h"
#include <M5Unified.h>

bool isSending = false;

void updateBackground() {
    M5.Display.fillScreen(isSending ? TFT_GREEN : TFT_BLUE);
    M5.Display.setTextColor(isSending ? TFT_BLACK : TFT_WHITE);
    M5.Display.setTextSize(2);
    
    M5.Display.setCursor(10, 20);
    M5.Display.println("SENDING:");
    
    M5.Display.setCursor(10, 45);
    M5.Display.println(isSending ? "ON" : "OFF");
}

void checkPowerButton() {
    if (M5.BtnB.wasClicked()) {
        isSending = !isSending;
        updateBackground();
        
        Serial.print("Data sending: ");
        Serial.println(isSending ? "ON" : "OFF");
        
        client.publish((mqtt_base + "status").c_str(), 
                       isSending ? "sending_on" : "sending_off");
    }
}