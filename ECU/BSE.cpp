#include <SPI.h>
#include "Arduino.h"
#include <mcp_can.h>
#include <mcp_can_dfs.h>
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
int average_max_val = 0;

constexpr uint8_t BRAKE_POINT = 5; // ブレーキペダル入力ピン
constexpr uint8_t BRAKE_LANP_POINT = 7; // ブレーキランプ出力ピン

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
            brake = brake_val(average_max_val); //TODO BSEの値をどう送るか CANか、何かしらarduinoから直接送ってしまうのか
            txBuf[1] = brake; // txBuf[1]にbrakeの値を格納
            CAN0.sendMsgBuf(0x100, 0, 8, txBuf); // ID100でtxBufを送信
            brake_lanp(brake);
            // if (IsHardBrake(brake, average_max_val) == true) {

            // }
            Serial.print("brake_val: ");Serial.println(brake);

            break;
        case BSE_ERR:
            Serial.println("BSE_ERR");
            txBuf[7] = 0xFF; // txBuf[7]にFFを格納
            CAN0.sendMsgBuf(0x100, 0, 8, txBuf); // ID100でtxBufを送信
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

int brake_val(int& average_max_val) {
    long sum = 0;
    int average_val = 0;
    int gain = 200;
    float scalling = 255.0 / 1023.0;
    for (int i=0;i<10;++i) {
        sum += analogRead(BRAKE_POINT);
    }

    average_val /= gain;
    average_val *= scalling; // スケーリング

    return average_val;
}

void BSE_Pin_Setup() {
    pinMode(BRAKE_POINT, INPUT);
    pinMode(BRAKE_LANP_POINT, OUTPUT);
}

//! 動作未確認
// ブレーキランプ点灯判断
void brake_lanp(byte brake) {
    if (brake >= 1) { // ブレーキペダルから受け取った値が１以上の時、
        digitalWrite(BRAKE_LANP_POINT, HIGH);
    } else {
        digitalWrite(BRAKE_LANP_POINT, LOW);
    }
    
}