#ifndef HOKKAIDO_SFE_MC_LIBRARY
#define HOKKAIDO_SFE_MC_LIBRARY

#include <Arduino.h>
#include <inttypes.h>

//O=0,I=1,V=5,X=10,L=50,C=100,D=500,M=1000

//CAN ID
#define VCM_ORDER 0x100
#define MC_MOTOR  0x101
#define MC_METER  0x501
#define VCM_REVER 0x102

//My data ID
//id 100 data
#define E_PT_Run 0x80	//  e-PT_Run_Request
#define T_Trq   0x81	//  Target_Torque
#define T_Lim_P 0x82	//  Torque_Limit_Plus
#define T_Lim_M 0x83	//  Torque_Limit_Minus
//id 101 data
#define MC_HV  0x84		//  MC_HV
#define MC_Rev 0x85		//  Motor_Revolution
#define MC_Trq 0x86		//  MC_Calculated_Torque
#define MC_Seq 0x87		//  SequenceMode
//id 501 data
#define MC_XII_V 0x88		//  MC_12V
#define MC_Diag_O 0x89		//  DiagCode0
#define MC_Diag_I 0x8A		//  DiagCode1
#define MC_Diag_II  0x8B	//  DiagCode2
#define MC_Diag_III 0x8C	//  DiagCode3
#define Mtr_Mrgn 0x8D		//  MC_Moter_Margin
#define Inv_Mrgn 0x8E		//  MC_Inverter_Margin
//id 102 data
#define E_PT_Run_EB 0x90	//  e-RT_Run_Request_echo_back
#define T_Trq_EB  0x91		//  Target_Torque_echo_back
#define T_Limp_EB 0x92		//  Torque_Limit_Plus_echo_back
#define T_Limm_EB 0x93		//  Torque_Limit_Minus_echo_back

//Set up mode
#define IGNITION 0x01
#define ACCEL    0x11
#define ACCEL_I  0x12
#define ACCEL_II 0x13
#define ACCEL_FAIL 0x1F
#define BRAKE      0x20
#define BRAKE_FAIL 0x2F
#define PRI_DIS_FIRST  0x30
#define PRI_DIS_SECOND 0x31
#define DRIVE    0x40
#define NEUTRAL 0x41
#define REVERSE  0x42

#define PRESET_SET 0xF0	
#define PRESET_I   0xF1
#define PRESET_II  0xF2
#define PRESET_III 0xF3
#define PRESET_IV  0xF4

class SFNICAN {
	private:

		//Data Input Output Check
		void Tx_Assign();	//100 ORDER
		void Rx_Assign(word rxin_id, byte rxin_buf[8]);	//101 MOTOR , 105 METER , 102 REVER

		//Control
		void main_control(void);
		void Sequence(void);
		void stepup(byte sepup_now);
		byte Prun_Make(byte set_prun);
		byte Run_Request();
		byte CRC_Make(byte crc_buf[8]);	//[8]

		bool pidStart_flag = false;
		bool CanRegenerate = false;
		
	public:

		//Setup Read Write
		void pinSetup(byte set_pin, byte set_mode);
		void dataRead(word id_id, byte buf_buf[8]);	//[8]
		void torqueWrite(word get_trq);
		void limiterWrite(byte set_upper, byte set_lower);
		byte dataPut(word set_id, byte set_buf_n);
		word dataPut(byte set_data);

		bool getPidStartFlag() {return pidStart_flag;}
		bool getCanRegenerate() {return CanRegenerate;}
};

#endif