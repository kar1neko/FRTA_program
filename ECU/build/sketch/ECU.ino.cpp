#line 1 "/home/kanek/project/frta/program/ECU/ECU.ino"
#include "Arduino.h"
#include "ECU.h"
#include "BSE.h"
#include "APPS.h"
#include <mcp_can_dfs.h>
#include <mcp_can.h>

MCP_CAN CAN0(SPI_CS_PIN);

bool CanUseMCP2515 = false;
#line 11 "/home/kanek/project/frta/program/ECU/ECU.ino"
void CAN_Setup();
#line 35 "/home/kanek/project/frta/program/ECU/ECU.ino"
void setup();
#line 44 "/home/kanek/project/frta/program/ECU/ECU.ino"
void loop();
#line 11 "/home/kanek/project/frta/program/ECU/ECU.ino"
void CAN_Setup() {
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        CanUseMCP2515 = true;
        Serial.println("[ECU]MCP2515 initialized successfully");
    } else {
        CanUseMCP2515 = false;
        byte err = CAN0.checkError();
        Serial.print("[error][ECU]MCP2515 initialized failed");Serial.println(err);
    }
    CAN0.setMode(MCP_NORMAL);

    // フィルタ無効化
    CAN0.init_Mask(0, 0, 0x0000);
    CAN0.init_Mask(1, 0, 0x0000);
    CAN0.init_Filt(0, 0, 0x0000);
    CAN0.init_Filt(1, 0, 0x0000);
    CAN0.init_Filt(2, 0, 0x0000);
    CAN0.init_Filt(3, 0, 0x0000);
    CAN0.init_Filt(4, 0, 0x0000);
    CAN0.init_Filt(5, 0, 0x0000);

    pinMode(CAN0_INT, INPUT);
}

void setup() {
    Serial.begin(115200);

    BSE_Pin_Setup();
    APPS_Pin_Setup();
    CAN_Setup();
    
}

void loop() {
    static unsigned long lastRun = 0;
    if(millis() - lastRun >= 10) {
        lastRun = millis();
        receiveID49(); // BSE
    
        BSE_monitor();
        APPS_monitor();
    }
}
