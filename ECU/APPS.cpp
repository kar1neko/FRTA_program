#include "APPS.h"
#include "Arduino.h"
#include "ECU.h"
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

uint8_t average_val_1 = 0;
uint8_t average_val_2 = 0;
uint8_t average_val = 0;
int raw_data = 0;
int raw_data_1 = 0;
int raw_data_2 = 0;
extern MCP_CAN CAN0;
static unsigned char txBuf[8] = {0};
const int basics = 1000 * 0.1; // 妥当性基準値 //? ここは実機ないのでわかりません(2026/07/29)
int diff = 0;
uint8_t accel = 0;

constexpr uint8_t ACCEL_SENSOR_1 = A5;
constexpr uint8_t ACCEL_SENSOR_2 = A6;
constexpr uint8_t implasbility = 7;

void Accel_val(uint8_t ACCEL_PIN, uint8_t &average_val, int &raw_data) { // 各アクセルペダルの入力値
    long sum = 0;
    int gain = 200;
    float scalling = 255.0 / 1023.0;
    for(int i=0;i<10;++i) {
        sum += analogRead(ACCEL_PIN);
    }
    raw_data = sum / 10;

    sum /= gain;
    sum *= scalling;
    average_val = sum;

}

void APPS_Pin_Setup() {
    pinMode(ACCEL_SENSOR_1, INPUT);
    pinMode(ACCEL_SENSOR_2, INPUT);
    pinMode(implasbility, OUTPUT);
}

enum APPS_Status {
    APPS_NORMAL,
    APPS_ERR
};

APPS_Status appsState = APPS_NORMAL;

void APPS_monitor() {
    switch (appsState) {
        case APPS_NORMAL:
            Accel_val(ACCEL_SENSOR_1, average_val_1, raw_data_1);
            Accel_val(ACCEL_SENSOR_2, average_val_2, raw_data_2);
            diff = abs(raw_data_1 - raw_data_2); // 現在のアクセルペダルの差
            if (diff >= basics) {
                digitalWrite(implasbility, LOW); // 異常発生時implasbility=LOW
                appsState = APPS_ERR;
                txBuf[0] = 0xFF; // APPS state
                // raw_data_1について
                uint16_t problem_raw = (uint16_t)raw_data_1;
                txBuf[2] = (problem_raw >> 8) & 0xFF; // 上位8ビットを取得
                txBuf[3] = problem_raw & 0xFF; // 下位８ビット取得
                problem_raw = (uint16_t)raw_data_2;
                txBuf[4] = (problem_raw >> 8) & 0xFF; // 上位８ビット取得
                txBuf[5] = problem_raw & 0xFF; // 下位８ビット取得
                CAN0.sendMsgBuf(0x51, 0, 8, txBuf);
                Serial.println("error occurred. transit APPS_ERR");
                
                return;
            }
            digitalWrite(implasbility, HIGH); // 正常時implasblity=HIGH
            accel = (average_val_1 + average_val_2) / 2; // accel値算出
            txBuf[1] = accel; // アクセル値格納
            txBuf[0] = 0x01; // ステータス格納(for debug)
            CAN0.sendMsgBuf(0x51, 0, 8, txBuf); // txBuf出力
            Serial.println("APPS Normal");

            break;

        case APPS_ERR:
            Serial.println("APPS_ERR");
            digitalWrite(implasbility, LOW); // 再定義
            txBuf[0] = 0xFF; // ステータス更新
            CAN0.sendMsgBuf(0x51, 0, 8, txBuf);
    }
}