#ifndef GUI_H
#define GUI_H

#include "config.h"
#include "config.h"
#include <M5Unified.h>

void updateBackground(bool sending);
void handleIncomingMqttMessage(const String& topic, const String& message);

#endif