#include "gui.h"

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
