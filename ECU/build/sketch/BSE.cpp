#line 1 "C:\\Users\\kanek\\kanek_project\\frta\\program\\ECU\\BSE.cpp"
#include <SPI.h>
// #include <cstdint>
#include "Arduino.h"
#include "C:\Users\kanek\kanek_project\frta\program\mcp_can.h"
#include "C:\Users\kanek\kanek_project\frta\program\mcp_can_dfs.h"
#include "ECU.h"
#include "BSE.h"

unsigned long rxID;
unsigned char rxlen = 0;
unsigned char txlen = 0;
unsigned char rxBuf[8];
unsigned char txBuf[8];
uint8_t ExpecteCnt = 0;
uint8_t receivedCnt = 0;
// long errTimePoint = 0;
const long errTimeLimit = 100; // 100ms
const unsigned long now = millis();
bool IsReceivedData = false;
long lastReceiveTime = 0;

constexpr uint8_t BRAKE_POINT = 5;

enum BSE_Status {
    BSE_SYNC, // 同期
    BSE_NORMAL, // 正常状態
    BSE_ERR // エラー状態
};

BSE_Status bseState = BSE_SYNC; // 起動直後は初期値をBSE_SYNCにする

void BSE_monitor() {

    unsigned long now = millis();
    // Serial.print("received: ");Serial.print(receivedCnt);
    // Serial.print(" expect: ");Serial.println(ExpecteCnt);
    switch (bseState) {
        case BSE_SYNC: // 初回同期
        if(IsReceivedData == true) { // IsreceivedData == trueになるまでbseState=BSE_SYNCで保持
            ExpecteCnt = (receivedCnt +1) % 10;
            bseState = BSE_NORMAL; // BSE_NORMALに移行
            Serial.println("BSE cnt is successfully");
            lastReceiveTime = now;
        }

            break;
        
        case BSE_NORMAL:
            Serial.println("BSE_NORMAL");
            if(IsReceivedData == true) {
                IsReceivedData = false;
                if (receivedCnt == ExpecteCnt) {
                    ExpecteCnt = (ExpecteCnt + 1) % 10; // 0 ~ 9の範囲でインクリメント
                    // errTimePoint = 0;
                    lastReceiveTime = now;
                }
            }
            if (now - lastReceiveTime >= errTimeLimit) {
                        bseState = BSE_ERR; // BSE_ERRに移行
                        Serial.println("BSE error occurred. transit ERROR MODE");
            } else if(IsReceivedData == true) { // 100ms以内に信号復帰の場合、BSE_SYNCに移行
                IsReceivedData = false;
                bseState = BSE_SYNC;
            }
            byte brake = 0;
            brake = brake_val(); //TODO BSEの値をどう送るか CANか、何かしらarduinoから直接送ってしまうのか
            Serial.print("brake_val: ");Serial.println(brake);

            break;
        case BSE_ERR:
            Serial.println("BSE_ERR");
            txBuf[1] = 0xFF;
            CAN0.sendMsgBuf(0x100, 0, 8, txBuf); // ID100にtxBufを送信
    }
}

void receiveID100() { // ID100のメッセージを受け取る
    if(CAN0.checkReceive() == CAN_MSGAVAIL) {
        CAN0.readMsgBuf(&rxID, &rxlen, rxBuf);

        if(rxID == 0x100) {
            receivedCnt = rxBuf[0]; // 受信したデータの0byte目をrecivedcntに代入
            IsReceivedData = true; // BSEからID100受信
        }
    }
}

//TODO ハードブレーキの定義を決める
int brake_val() {
    int actual_val[10] = {};
    int average_val = 0;
    float scalling = 255.0 / 1023.0;
    for (int i=0;i<10;++i) {
        *(actual_val + i) = analogRead(BRAKE_POINT);
        average_val += *(actual_val + i);
    }

    average_val /= 200;
    average_val *= scalling;

    return average_val;
}