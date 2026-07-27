#line 1 "C:\\Users\\kanek\\kanek_project\\frta\\program\\ECU\\ECU.h"
#ifndef ECU_H
#define ECU_H

#include <SPI.h>
#include "C:\Users\kanek\kanek_project\frta\program\mcp_can\mcp_can.h"
// #include "mcp_can.h" // これ使えねえ

#define SPI_CS_PIN 10
#define CAN0_INT 2

extern MCP_CAN CAN0;
extern byte ID100[8];
extern bool CanUseMCP2515;

void CAN_Setup();
void receiveID100();

#endif