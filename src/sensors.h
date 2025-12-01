#ifndef SENSORS_H
#define SENSORS_H


#include "microphone.h"
#include <M5UnitENV.h>


void setup_sensors();
void publishSensorData();
bool handlePowerButton(bool currentSending);

#endif