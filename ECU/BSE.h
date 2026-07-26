#ifndef BSE_H
#define BSE_H

#include "ECU.h"
// #include <cstdint>

extern unsigned long rxID;
extern unsigned char rxlen;
extern unsigned char txlen;
extern unsigned char rxBuf[8];
extern unsigned char txBuf[8];
extern uint8_t ExpecteCnt;
extern uint8_t receivedCnt;
// extern long errTimePoint;
extern const long errTimeLimit; // 100ms
extern bool IsReceivedData;
extern long lastReceiveTime;

extern const unsigned long now;

void BSE_monitor();
void receiveID100();
int brake_val();

#endif