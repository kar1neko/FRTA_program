#ifndef ECU_H
#define ECU_H

#include <SPI.h>
#include <mcp_can.h>

#define SPI_CS_PIN 10
#define CAN0_INT 2

void CAN_Setup();
void receiveID49();

#endif