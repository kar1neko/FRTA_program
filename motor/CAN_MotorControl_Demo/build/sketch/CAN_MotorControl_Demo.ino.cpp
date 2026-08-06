#line 1 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
/// altimate kasu
//TODO RTDの信号がLOW状態の時アクセルの出力信号を0に固定
//TODO IGNITIONはREADY_OKになったのであればすぐにHIGHにする
/*************************************************
  Motor_Control Program
学生フォーミュラで使う日産モータのインバータとのやり取りのサンプルプログラムです。
CANモジュールとライブラリが必要です。
本プログラムのライブラリのバージョンはv0.0.1です。
ライブラリは以下からダウンロードして使ってください。
CAN library : MCP_CAN Library for Arduino
MCP_CAN_lib-master : https://github.com/coryjfowler/MCP_CAN_lib
場合によっては、CAN_Setup()、Upload_CAN(byte buf[8])内を変更して使用してください。

  <port>
  AnalogPin A5-->Input Accel Voltage
  //DigitalPin 3-->REVERSE Switch
  DigitalPin 4-->READY Switch
  //DigitalPin 5-->LED
  Digitalpin 10,11,12,13,5V,GND-->CANcontroller,SPI
  Digitalpin 2-->CANcontroller,INTPIN

*************************************************/
// ライブラリヘッダのインクルード
#include <SPI.h>
#include <mcp_can.h>
#include "Arduino.h"
#include "NI_CU.h"
#include "mcp_can_dfs.h"
#include "ECU.h" 


//入力ピン
#define ACCEL_POINT 5 // APPSから状態受け取り
#define IGNITION_SW 4
#define SPI_CS_PIN 10
#define CAN0_INT 2
//フラグスイッチ
#define OFF 0x00
#define ON OFF+0x01
#define SHOW OFF
//Sequence
#define IG_ON 0x01
#define READY_OK 0x02
#define STOP_ON 0x03
#define FAIL_ON 0x0F
//readyオン
#define READY_ON 0x1F

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

MCP_CAN CAN0(SPI_CS_PIN);
SFNICAN NICU;

//データライン
byte ID100[8] = {};
//CAN input data
long unsigned int id;
byte len;
byte buf[8];

//セットアップ関数
void Pin_Setup(void);
void CAN_Setup(void);
void Sequence_Setup(void);
//各データ入力関数
void CAN_ID101_Read(void);
void CAN_APPS_Read();
int Accel_Read(void);
//データ出力関数
void Upload_CAN(byte buf[8]);

byte accel = 0;

class PIDcontroller {
  private:
    double kp_t, ki_t, kd_t;
    double prev_err=0.0, integral=0.0;
    double out_min, out_max;

  public:
    PIDcontroller(double kp, double ki, double kd, double out_min = 0.0, double out_max = 8.0)
      :kp_t(kp), ki_t(ki), kd_t(kd), prev_err(0.0), integral(0.0), out_min(out_min), out_max(out_max) {}

      double update(double target_rpm, double cur_rpm, long long start_time) {
        long long dt = start_time;
        double err = target_rpm - cur_rpm;
        integral += err;
        double diff = (err - prev_err) / dt;
        prev_err = err;
        
        double temp_result = kp_t * err + ki_t * integral + kd_t * diff;
        double result = constrain(temp_result, out_min, out_max);
        Serial.print("constrated result: ");Serial.println(result);

        return result;
      }
};

// PID制御関数
byte PID_Control(PIDcontroller& pidcon);

//* PIDゲイン設定
PIDcontroller val(0.15, 0.01, 0.02);

#line 105 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
void setup();
#line 125 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
void loop();
#line 214 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
void CAN_ID101_Read();
#line 270 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
void CAN_APPS_Read(byte &accel);
#line 105 "/home/kanek/project/frta/program/motor/CAN_MotorControl_Demo/CAN_MotorControl_Demo.ino"
void setup() {
  Serial.begin(115200);
  unsigned long start_time = millis();
  while (millis() - start_time < 3000) {}
  pinMode(3, OUTPUT); //* Precharge Relay切り替え
  pinMode(7, OUTPUT);
  Pin_Setup();
  Serial.println("[Success] Pin_Setup Successfully.");
  CAN_Setup();
  Serial.println("[Success] CAN_Setup Successfully.");
  Serial.println("<setup OK!>");
  Serial.println("<START!>");
}

unsigned long setup_StartTime=0;
int setup_initialized=0;

//* 臨時
extern byte mcprun;

void loop() {
  //input data
  word mcrev = (word)NICU.dataPut(MC_Rev);
  word mctrq = (word)NICU.dataPut(MC_Trq);
  Sequence_Setup();
  CAN_ID101_Read();
  // byte tx[8] = {0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB, 0xCC, 0xDD};
  // CAN0.sendMsgBuf(0x101, 0, 8, tx);
  
  Serial.print(" mcrev: ");Serial.print(mcrev);
  Serial.print(" mctrq: ");Serial.print(mctrq);
  // Serial.print(" rege_flag: ");
  // if (!NICU.getCanRegenerate()) Serial.println("false");
  // if (NICU.getCanRegenerate()) Serial.println("true");
  Serial.print(" inv, mtr margin: ");Serial.print(NICU.dataPut(Inv_Mrgn));Serial.print(", ");Serial.println(NICU.dataPut(Mtr_Mrgn));
  // Serial.print("mcprun");Serial.println(mcprun);

  // Serial.println(average_value);
  // byte wri=0;
  // if(NICU.getPidStartFlag() == true) {
  //   wri = PID_Control(val);
  // }

  //* APPS処理
  
  CAN_APPS_Read();
  int APPS_state = digitalRead(ACCEL_POINT);
  if(APPS_state == HIGH) {
    NICU.torqueWrite(accel); // アクセル入力値
  } else if (APPS_state == LOW) {
    accel = 0;
    NICU.torqueWrite(accel);
  }
  NICU.limiterWrite(0x09, 0x10); // 出力制限値
  
  for (int i = 0; i < 8; i++) ID100[i]= NICU.dataPut(VCM_ORDER,i);
  Upload_CAN(ID100);
}

void Sequence_Setup(void) {
  // 初回起動時のシーケンス初期化
if (!setup_initialized == 1) {
  setup_StartTime = millis();
}
if (millis() - setup_StartTime >= 1000) {
  // byte dummy_buf[8] = {0, 0, 0, 0, 0, 0, 0};
  setup_initialized = 1;
  Serial.println("Setup Sequence initialized");
  }
  return;
}

void Pin_Setup(void){
  pinMode(IGNITION_SW, INPUT_PULLUP);
  pinMode(CAN0_INT, INPUT_PULLUP); // INTをプルアップ
}

void CAN_Setup(void){ 
  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) //TODO CAN側のバススピードとMCP基板の周波数確認
    Serial.println("MCP2515 Initialized Successfully!");

  else {
    byte err = CAN0.checkError();
    Serial.print("[Error]MCP2515 Initialized failed ");Serial.println(err);
  }
    
  // Set operation mode to normal so the MCP2515 sends acks to received data.
  // Change to normal mode to allow messages to be transmitted
  CAN0.setMode(MCP_NORMAL);

  // フィルタを無効化
  CAN0.init_Mask(0, 0, 0x0000);
  CAN0.init_Mask(1, 0, 0x0000);
  CAN0.init_Filt(0, 0, 0x0000);
  CAN0.init_Filt(1, 0, 0x0000);
  CAN0.init_Filt(2, 0, 0x0000);
  CAN0.init_Filt(3, 0, 0x0000);
  CAN0.init_Filt(4, 0, 0x0000);
  CAN0.init_Filt(5, 0, 0x0000);

  // Configuring pin for /INT input
  pinMode(CAN0_INT, INPUT);
  Serial.println("MCP2515 Library Receive Example...");
}

// 101の読み取り
void CAN_ID101_Read(){
  // INT依存のポーリング
  // Serial.println("fuck!!!!!!!!");
  // Serial.print("CAN0_INT: ");Serial.println(digitalRead(CAN0_INT));
  // Serial.println();
  if (!digitalRead(CAN0_INT)) {
    CAN0.readMsgBuf(&id, &len, buf);
    // Serial.print("RX! id: ");Serial.print(id);
    // Serial.print(" len: ");Serial.println(len);
    if(id == 0x101) {
      NICU.dataRead(MC_MOTOR, buf);
      // Serial.println("0x101 Received");
    }
  }

  for (int i=0;i<len;++i) {
    // if(buf[i]<0x10) Serial.print('0');
    // Serial.print(buf[i], HEX);
    // Serial.print(" ");
  }
}

// ここはAPPSで算出するため、コメントアウト
// int Accel_Read(void){
//   int actual_value[10]={};
//   int average_value=0;
//   byte accel_limit=0x10;  // これ何？
//   for(int i=0;i<10;i++){
//     *(actual_value+i)=analogRead(ACCEL_POINT);
//     average_value+=*(actual_value+i);
//   }
//   // Serial.print("before");Serial.println(average_value);
//   average_value/=200;
//   average_value*=0.249266862170088;//(255/1023)
//   // Serial.print("after");Serial.println(average_value);
  
//   return average_value;
// }

void Upload_CAN(byte buf[8]){
  // 送信データ:  ID = 0x100,0,データ長=8,データ=Tx_ID100[8]
  CAN0.sendMsgBuf(0x100, 0, 8, buf);
}

byte PID_Control(PIDcontroller& pidcon) {
  unsigned long long start_time = millis();
  double target_rpm = 800;
  double cur_rpm = (int)NICU.dataPut(MC_Rev);
  double output = pidcon.update(target_rpm, cur_rpm, start_time);

  output = constrain(output, 0.0, 255.0); // scalling
  word output_word = (word)output;
  return output_word;
}

// APPSから受け取ったアクセルの値の反映
void CAN_APPS_Read(byte &accel) {
  if(CAN0.checkReceive() == CAN_MSGAVAIL) {
    CAN0.readMsgBuf(&id, &len, buf);
    if(id == 0x51) {
      accel = buf[1];
    }
  }
}
