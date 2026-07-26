/*
NIMC.cpp
NISSAN inverter motor control
NISSAN Inverter 291A0-3NFA CAN control header (StudentFormula custom version)
// */

#include "NI_CU.h"

// flag
#define OFF 0x00
#define ON OFF + 0x01
// flag stepup
#define TORQUE_OK 0x11
#define LIMITER_OK 0x22
#define READ_OK 0x44
#define SETUP_OK 0x77

// sequence
#define IG_START 0x01
#define READY_OK 0x02
#define STOP_MODE 0x03
#define FAIL_MODE 0x0F

// request ready
#define READY_ON 0x1F

// MAX & MIN limit
#define TORQUE_MAX 0x03F8
#define TORQUE_MIN 0xFC08
#define TORQUE_LIMIT_UPPER_MAX 0x7F
#define TORQUE_LIMIT_UPPER_MIN 0x00
#define TORQUE_LIMIT_LOWER_MAX 0x7F
#define TORQUE_LIMIT_LOWER_MIN 0x00
// spec
#define INV_VOL_MAX 400
#define INV_VOL_MIN 240
#define ROTATE_MAX 10500


// Tx 100
byte vcnprun = 0; 
byte eptrun = 0;
word ttrq = 0;
byte tlimp = 0;
byte tlimm = 0;
byte vcmCcrc = 0;
// Rx 101
byte mcprun = 0;
byte buf_mcprun = 0;
word mchv = 0;
word mcrev = 0;
word mctrq = 0;
byte mcseq = 0;
byte mcCIcrc = 0;
// 501
byte mcXIIv = 0;
byte mcdiag0 = 0;
byte mcdiag1 = 0;
byte mcdiag2 = 0;
byte mcdiag3 = 0;
byte mtrmrgn = 0;
byte invmrgn = 0;
byte mcDIcrc = 0;
// 102
byte vcnpruneb = 0;
byte eptruneb = 0;
word ttrqeb = 0;
byte tlimpeb = 0;
byte tlimmeb = 0;
byte vcmCcrceb = 0;

byte Tx_ID100[8] = {};
byte Buf_ID100[8] = {};
byte Rx_ID101[8] = {};
byte Rx_ID501[8] = {};
byte Rx_ID102[8] = {};

byte Stepup_f = 0;

// set I/O pin
byte igp = 0;
byte acp = 0;
byte acIp = 0;
byte acIIp = 0;
byte acfp = 0;
byte brp = 0;
byte brfp = 0;
byte pdfp = 0;
byte pdsp = 0;
byte drep = 0;
byte nulp = 0;
byte revp = 0;
byte pstp = 0;

// Control
byte SFNICAN::CRC_Make(byte crc_buf[8])
{												   //[8]
	bool CRC_MSB[9] = {1, 0, 0, 0, 1, 1, 1, 0, 1}; // CRC
	bool bit[64] = {};
	byte k = 0;
	byte hex_x = 0;
	byte CRC_exe = 0;

	// BIT_Convert
	for (byte j = 0; j < 7; j++)
	{
		k = j + 1;
		k <<= 3;
		hex_x = *(crc_buf + j);
		for (byte jj = 0; jj < 8; jj++)
		{
			k--;
			*(bit + k) = hex_x >> jj & 1;
		}
	}
	// CRC
	k = 0;
	for (byte j = 0; j < 56; j++)
	{
		if (*(bit + j) == 1)
		{
			for (byte jj = 0; jj < 9; jj++)
			{
				k = j + jj;
				*(bit + k) = bit[j + jj] ^ *(CRC_MSB + jj);
			}
		}
		else
			*(bit + j) = 0;
	}
	// HEX_Convert
	k = 55;
	CRC_exe |= *(bit + k);
	for (byte j = 0; j < 8; j++)
	{
		k++;
		CRC_exe <<= 1;
		CRC_exe |= *(bit + k);
	}
	return CRC_exe;
}

byte SFNICAN::Prun_Make(byte set_prun)
{
	set_prun++;
	if (set_prun == 8)
		set_prun = 0;
	return set_prun;
}

byte SFNICAN::Run_Request()
{
	byte req = 0;

	req = READY_ON;

	return req;
}

unsigned long HV_time;
unsigned long diff = 0;

// const int
void SFNICAN::Sequence()
{
	switch (mcseq)
	{
	case FAIL_MODE:
		ttrq = OFF;
		tlimp = OFF;
		tlimm = OFF;
		eptrun = OFF;
		Serial.println("FAIL_MODE");
		break;
	case IG_START: // キースイッチがひねられた状態

		if (mchv >= 0x178) // 0x178(16) = 376(10) [V]
		{ 
			eptrun = Run_Request();
			digitalWrite(7, HIGH); // Precharge終了後のリレー
			// Serial.println(mchv);
		}
		else
		{
      		diff = millis();
			eptrun = OFF;
			digitalWrite(3, HIGH);
			digitalWrite(7, LOW);
		}
		ttrq = OFF;
		tlimp = OFF;
		tlimm = OFF;
		Serial.print("IG_Start!:) Voltage Value -> ");
		Serial.println(mchv);
		// Serial.println("IG_START");
		break;
	case READY_OK: //! インバータ充電済み 動作可能
    	HV_time = millis();
		eptrun = Run_Request();
		Serial.println("READY_OK"); // インバータ充電済み　動作可能
		if (HV_time - diff > 5000)
		{
			digitalWrite(3, LOW); // Precharge
			pidStart_flag = true;
			
		} else {
      		Serial.print("HV_time -> ");Serial.println(HV_time - diff);
    }
		// ttrq  --> torqueWrite( USE )
		// tlimp --> limiterWrite( USE, OFF )
		// tlimm --> limiterWrite( OFF, USE )
		break;
	case STOP_MODE:
		eptrun = OFF;
		if (mctrq > 0x00FF)
			ttrq /= 2;
		else
			ttrq = OFF;
		tlimp = ttrq;
		tlimm = OFF;
		Serial.println("STOP_MODE");
		break;
	}
}

void SFNICAN::main_control()
{
	// Control System
	vcnprun = Prun_Make(vcnprun);
	Sequence();
}

// Data Input Output Check
void SFNICAN::Tx_Assign()
{ // 100 ORDER
	word unite = 0;
	word upper = 0;
	word lower = 0;
	// Bufin & Line reset
	for (byte i = 0; i < 8; i++)
	{
		*(Buf_ID100 + i) = *(Tx_ID100 + i);
		*(Tx_ID100 + i) = 0;
	}
	// Assign
	unite = vcnprun;
	unite <<= 5;
	unite |= eptrun;
	// トルク指令 tx = 送信
	lower = ttrq & 0x00FF; // トルク下位8bit
	upper = ttrq >> 8;	   // トルク上位8bit

	*(Tx_ID100 + 0) = unite;
	*(Tx_ID100 + 1) = upper;
	*(Tx_ID100 + 2) = lower;
	*(Tx_ID100 + 3) = tlimp;
	*(Tx_ID100 + 4) = tlimm;
	*(Tx_ID100 + 5) = OFF;
	*(Tx_ID100 + 6) = OFF;
	*(Tx_ID100 + 7) = CRC_Make(Tx_ID100);
}
//! Rx = 受信
void SFNICAN::Rx_Assign(word rxin_id, byte rxin_buf[8])
{					// 101 MOTOR , 105 METER , 102 REVER
	word upper = 0; // 上位ビット格納用
	word lower = 0; // 下位ビット格納用
	word unite = 0; // 2バイト結合用

	// public:
	// 	word& getMcrev() {
	// 		return mcrev;
	// 	}

	switch (rxin_id, rxin_id)
	{ // 受信IDで分岐
	case MC_MOTOR:
		// --- 受信値リセット ---
		mcprun = 0;	 // モータ運転状態リセット
		mchv = 0;	 // 高電圧値リセット
		mcrev = 0;	 // 回転数リセット
		mctrq = 0;	 // トルク値リセット
		mcseq = 0;	 // シーケンスモードリセット
		mcCIcrc = 0; // CRCリセット
		// --- データ分解・格納 ---
		//* 順次bit start positionからbit length分移動させる
		lower = *(rxin_buf + 0) & 0x01; // バイト0の最下位1ビット抽出（HV下位ビット）
		upper = *(rxin_buf + 0) >> 5;	// バイト0の上位3ビット抽出（運転状態）
		mcprun = upper;					// 運転状態格納
		buf_mcprun = upper;				// 運転状態コピー

		unite = lower; // HV値下位ビットをセット
		unite <<= 8;   // 8ビット左シフト（上位ビット用）

		unite |= *(rxin_buf + 1); // バイト1を下位に結合（HV値完成）
		mchv = unite;			  // 高電圧値格納 //TODO 16進数表記されているので変換する(10進数変換させる)

		unite = 0;				  // 初期化
		unite = *(rxin_buf + 2);  // バイト2取得（回転数上位）
		unite <<= 8;			  // 8ビット左シフト
		unite |= *(rxin_buf + 3); // バイト3を下位に結合（回転数完成）
		mcrev = unite;			  // 回転数格納

		unite = 0;				  // 初期化
		unite = *(rxin_buf + 4);  // バイト4取得（トルク上位）
		unite <<= 8;			  // 8ビット左シフト
		unite |= *(rxin_buf + 5); // バイト5を下位に結合（トルク完成）
		mctrq = unite;			  // トルク値格納

		mcseq = *(rxin_buf + 6);   // バイト6：シーケンスモード格納
		mcCIcrc = *(rxin_buf + 7); // バイト7：CRC格納
		// --- 受信データ保存 ---
		for (byte i = 0; i < 8; i++)
		{									   // 8バイト分ループ
			*(Rx_ID101 + i) = 0;			   // 一旦クリア
			*(Rx_ID101 + i) = *(rxin_buf + i); // 受信データを保存
		}
		// Serial.print("CRC_motor: ");Serial.println(vcmCcrceb);
		// Serial.print("mcrev: ");Serial.print(mcrev);Serial.print(" mctrq: ");Serial.println(mctrq);

		break;

	case MC_METER:
		// Reset
		mcXIIv = 0;
		mcdiag0 = 0;
		mcdiag1 = 0;
		mcdiag2 = 0;
		mcdiag3 = 0;
		mtrmrgn = 0;
		invmrgn = 0;
		mcDIcrc = 0;
		// Assign
		mcXIIv = *(rxin_buf + 0);
		mcdiag0 = *(rxin_buf + 1);
		mcdiag1 = *(rxin_buf + 2);
		mcdiag2 = *(rxin_buf + 3);
		mcdiag3 = *(rxin_buf + 4);
		mtrmrgn = *(rxin_buf + 5);
		invmrgn = *(rxin_buf + 6);
		mcDIcrc = *(rxin_buf + 7);
		// Line assign
		for (byte i = 0; i < 8; i++)
		{
			*(Rx_ID501 + i) = 0;
			*(Rx_ID501 + i) = *(rxin_buf + i);
		}
		
		
		break;
		
		//* 故障コード出力
		Serial.print("mcdiag0: ");Serial.println(mcdiag0);
		Serial.print("mcdiag1: ");Serial.println(mcdiag1);
		Serial.print("mcdiag2: ");Serial.println(mcdiag2);
		Serial.print("mcdiag3: ");Serial.println(mcdiag3);
		Serial.print("CRC_meter: ");Serial.println(vcmCcrceb);
	case VCM_REVER:
		// Reset
		vcnpruneb = 0;
		eptruneb = 0;
		ttrqeb = 0;
		tlimpeb = 0;
		tlimmeb = 0;
		vcmCcrceb = 0;
		// Assign
		lower = *(rxin_buf + 0) & 0x001F;
		upper = *(rxin_buf + 0) >> 5;
		vcnpruneb = upper;
		eptruneb = lower;

		unite = *(rxin_buf + 1);  // バイト1取得（トルクエコーバック上位）
		unite <<= 0x00FF;		  // 0x00FFビット左シフト（※通常8ビットシフトで十分）
		unite |= *(rxin_buf + 2); // バイト2を下位に結合（トルクエコーバック完成）
		ttrqeb = unite;			  // トルクエコーバック格納

		tlimpeb = *(rxin_buf + 3);	 // バイト3：トルクリミットプラスエコーバック
		tlimmeb = *(rxin_buf + 4);	 // バイト4：トルクリミットマイナスエコーバック
		vcmCcrceb = *(rxin_buf + 7); // バイト7：CRCエコーバック
		// --- 受信データ保存 ---
		for (byte i = 0; i < 8; i++)
		{									   // 8バイト分ループ
			*(Rx_ID102 + i) = 0;			   // 一旦クリア
			*(Rx_ID102 + i) = *(rxin_buf + i); // 受信データを保存
		}
		Serial.print("CRC_rever: ");Serial.println(vcmCcrceb);

		break;
	}
}

void SFNICAN::stepup(byte sepup_now)
{
	Stepup_f |= sepup_now;
	if (Stepup_f == SETUP_OK)
	{
		main_control();
		Tx_Assign();
		Stepup_f = OFF;
	}
}

// Setup Read Write

void SFNICAN::dataRead(word id_id, byte buf_buf[8])
{ //[8]
	Rx_Assign(id_id, buf_buf);
	if (id_id == MC_MOTOR)
		stepup(READ_OK);
}

void SFNICAN::torqueWrite(word get_trq)
{
	int16_t trq_transed = (int16_t)get_trq;
	int16_t reverse_val = -4;
	if ((int16_t)TORQUE_MAX < trq_transed) {
		ttrq = TORQUE_MAX;
		Serial.println("max");
	}
	else if (0x7FFFF < trq_transed && (int16_t)TORQUE_MIN < trq_transed) {
		ttrq = (int16_t)TORQUE_MIN;
		Serial.println("min/");
	} 
	else if(mchv < 380 || mcrev > 3000) {
		if (mcrev < 3200) {
			ttrq = 0;
			// Serial.println("low mchv");
		} else {
			ttrq = reverse_val;
		} 
	}
	else {
		if (trq_transed > 0) {
			ttrq = trq_transed;
			CanRegenerate = true;
			// Serial.println("Tx >");
		} else {
			if (abs((int16_t)mcrev) < 100) {
				CanRegenerate = false;
				ttrq = 0;
				Serial.println("stopped");
			} else if(CanRegenerate == true) {
				ttrq = reverse_val;
				Serial.println("reve");
			} else {
				ttrq = 0;
				Serial.print("?");
			}
		}
	}

	stepup(TORQUE_OK);
}

void SFNICAN::limiterWrite(byte set_upper, byte set_lower)
{
	if (TORQUE_LIMIT_UPPER_MAX < set_upper)
		tlimp = TORQUE_LIMIT_UPPER_MAX;
	else
		tlimp = set_upper;
	if (TORQUE_LIMIT_LOWER_MAX < set_lower)
		tlimm = TORQUE_LIMIT_LOWER_MAX;
	else
		tlimm = set_lower;

	stepup(LIMITER_OK);
}

void SFNICAN::pinSetup(byte set_pin, byte set_mode)
{
	switch (set_mode)
	{
		// set I/O pin
	case IGNITION:
		igp = set_pin;
		break;
	case ACCEL:
		acp = set_pin;
		break;
	case ACCEL_I:
		acIp = set_pin;
		break;
	case ACCEL_II:
		acIIp = set_pin;
		break;
	case ACCEL_FAIL:
		acfp = set_pin;
		break;
	case BRAKE:
		brp = set_pin;
		break;
	case BRAKE_FAIL:
		brfp = set_pin;
		break;
	case PRI_DIS_FIRST:
		pdfp = set_pin;
		break;
	case PRI_DIS_SECOND:
		pdsp = set_pin;
		break;
	case DRIVE:
		drep = set_pin;
		break;
	case NEUTRAL:
		nulp = set_pin;
		break;
	case REVERSE:
		revp = set_pin;
		break;
		// set preset pin
	case PRESET_SET:
		pstp = set_pin;
		break;
	}
}

byte SFNICAN::dataPut(word set_id, byte set_buf_n)
{
	switch (set_id)
	{
	case VCM_ORDER:
		return *(Tx_ID100 + set_buf_n);
	case MC_MOTOR:
		return *(Rx_ID101 + set_buf_n);
	case MC_METER:
		return *(Rx_ID501 + set_buf_n);
	case VCM_REVER:
		return *(Rx_ID102 + set_buf_n);
	default:
		return 0;
	}
}

word SFNICAN::dataPut(byte set_data)
{

	switch (set_data)
	{
		// id 100 data
	case E_PT_Run:
		return eptrun;
	case T_Trq:
		return ttrq;
	case T_Lim_P:
		return tlimp;
	case T_Lim_M:
		return tlimm;
		// id 101 data
	case MC_HV:
		return mchv;
	case MC_Rev:
		return mcrev;
	case MC_Trq:
		return mctrq;
	case MC_Seq:
		return mcseq;
		// id 501 data
	case MC_XII_V:
		return mcXIIv;
	case MC_Diag_O:
		return mcdiag0;
	case MC_Diag_I:
		return mcdiag1;
	case MC_Diag_II:
		return mcdiag2;
	case MC_Diag_III:
		return mcdiag3;
	case Mtr_Mrgn:
		return mtrmrgn;
	case Inv_Mrgn:
		return invmrgn;
		// id 102 data
	case E_PT_Run_EB:
		return eptruneb;
	case T_Trq_EB:
		return ttrqeb;
	case T_Limp_EB:
		return tlimpeb;
	case T_Limm_EB:
		return tlimmeb;
	default:
		return 0;
	}
}