#line 1 "C:\\Users\\kanek\\kanek_project\\frta\\program\\ECU\\BSE.h"
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
extern byte brake;
extern int average_max_val;

extern const unsigned long now;

void BSE_monitor();
void receiveID100();
void BSE_Pin_Setup();
int brake_val(int& average_max_val);
void brake_lanp(byte brake);
// bool IsHardBrake(byte brake, int average_max_val);

#endif