#ifndef BSE_H
#define BSE_H

#include <Arduino.h>

void BSE_monitor();
void receiveID49();
void BSE_Pin_Setup();
int brake_val();
void brake_lanp(byte brake);

#endif