#include <Arduino.h>
#line 1 "/home/kanek/project/frta/program/CAN_BSE/CAN_BSE.ino"
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
// #include "mcp_can.cpp"

#define SPI_CS_PIN 10
#define CAN0_INT 2

MCP_CAN CAN0(SPI_CS_PIN);

char txBuf[8] = {0};

void Pin_Setup();   
void CAN_Setup();

#line 54 "/home/kanek/project/frta/program/CAN_BSE/CAN_BSE.ino"
void send_data(const unsigned long now, byte txBuf[8]);
#line 70 "/home/kanek/project/frta/program/CAN_BSE/CAN_BSE.ino"
void setup();
#line 77 "/home/kanek/project/frta/program/CAN_BSE/CAN_BSE.ino"
void loop();
#line 16 "/home/kanek/project/frta/program/CAN_BSE/CAN_BSE.ino"
void Pin_Setup() {
    pinMode(CAN0_INT, INPUT_PULLUP);
}

bool CanUseMCP2515 = false;

void CAN_Setup() {
    pinMode(SPI_CS_PIN, OUTPUT);
    digitalWrite(SPI_CS_PIN, HIGH);
    
    // MCP2515 status check
    if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        CanUseMCP2515 = true;
        Serial.println("[BSE]MCP2515 Initialized Successfully :)");
    } else {
        CanUseMCP2515 = false;
        byte err = CAN0.checkError();
        Serial.print("[error][BSE]MCP2515 Initialized failed: ");Serial.println(err);
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

// byte ID100[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

unsigned long ID100_Timecnt = 0;
void send_data(const unsigned long now, byte txBuf[8]) {
    if (now - ID100_Timecnt > 50) { // 50ms更新
        static uint8_t cnt=0;
        txBuf[0] = cnt++;
        if(cnt == 10) {
            cnt=0;
            Serial.println(" cnt reset");
        }
        ID100_Timecnt = now;
        CAN0.sendMsgBuf(0x49, 0, 8, txBuf);
        Serial.print(" cnt: ");Serial.print(cnt);
    }
    
    
}

void setup() {
    Serial.begin(115200);
    Pin_Setup();
    CAN_Setup();
    
}

void loop() {
    //! use millies() this line.
    unsigned long now = millis();
    send_data(now, txBuf);
    if(CanUseMCP2515 == true) {
        
        // Serial.println("txData sent successfully");
    } else {
        Serial.println("txData sent failed");
    }


}


