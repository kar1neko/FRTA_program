#include "APPS.h"
#include "Arduino.h"
#include <SPI.h>
// まじもうえぐいよ...なんでプログラムワンオペなんだよ
uint8_t 

constexpr uint8_t ACCEL_SENSOR_1 = 5;
constexpr uint8_t ACCEL_SENSOR_2 = 6;

void Accel_val(uint8_t ACCEL_PIN) { 
    long sum = 0;
    int average_val = 0;
    int gain = 200;
    float scalling = 255.0 / 1023.0;
    for(int i=0;i<10;++i) {
        sum += analogRead(ACCEL_PIN);
    }

    average_val /= gain;
    average_val *= scalling;

}

// caseで分けるべきかな？