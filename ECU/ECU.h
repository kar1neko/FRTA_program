#ifndef ECU_H
#define ECU_H

#include <SPI.h>
#include <mcp_can.h>

#define SPI_CS_PIN 10
#define CAN0_INT 2

extern MCP_CAN CAN0;
extern byte ID100[8];
extern bool CanUseMCP2515;

void CAN_Setup();
void receiveID100();

#endif