#ifndef BSE_H
#define BSE_H

#include <Arduino.h>

enum BSE_Status {
    BSE_SYNC,
    BSE_NORMAL,
    BSE_ERR
};

void BSE_monitor();
void receiveID49();
void BSE_Pin_Setup();
int brake_val();
void brake_lanp(byte brake);
void analogVol_FaultDetection();
bool isBSEErr();

#endif