

/*
 * fsm_deteccion.c
 *
 *  Created on: Dec 15, 2011
 *      Author: IronMan
 */

#include "includes.h"


uint8_t fsm_rot485_state, fsm_rotrele485_state, fsm_rotEVO_state;
uint8_t	Rot485_flag;
time_t rot485timer, rotrele485timer, rotEVOtimer;
uint16_t lowbatt_timer;

uint8_t fsm_npd_state;
#define	NPD_IDLE	0x10
#define	NPD_PRELOW	0x20
#define	NPD_LOW		0x30
#define	NPD_PRENORM	0x40
uint16_t npd_timer;

uint16_t zonenumber;

const uint8_t numabo_z1[200] = {
		62, 46, 40, 33, 118, 50, 183, 137, 112, 169, 26, 159, 177, 38, 8, 194, 197, 35, 188, 126,
		5, 104, 199, 4, 101, 119, 160, 180, 127, 63, 185, 15, 89, 47, 187, 128, 172, 184, 174, 92,
		54, 93, 163, 138, 88, 136, 71, 30, 186, 124, 158, 18, 58, 53, 150, 189, 154, 193, 145, 57,
		168, 36, 110, 52, 105, 131, 135, 69, 142, 123, 1, 86, 161, 21, 16, 43, 115, 98, 167, 153,
		9, 37, 44, 6, 97, 39, 120, 82, 2, 166, 73, 12, 90, 49, 149, 55, 81, 28, 130, 25, 29, 156,
		170, 78, 106, 61, 178, 85, 121, 60, 91, 152, 179, 87, 77, 51, 0, 192, 59, 17, 34, 70, 157,
		139, 27, 100, 132, 48, 22, 143, 80, 133, 66, 94, 146, 111, 198, 125, 45, 95, 117, 31, 10,
		96, 107, 195, 129, 79, 42, 182, 151, 20, 141, 74, 3, 83, 109, 32, 147, 75, 7, 19, 165, 103,
		196, 13, 99, 84, 190, 134, 102, 162, 176, 14, 76, 173, 116, 164, 65, 114, 23, 175, 140, 67,
		68, 191, 64, 144, 155, 113, 11, 181, 108, 148, 171, 56, 24, 72, 41, 122
};
const uint8_t numabo_z2[200] = {
		136, 152, 196, 29, 44, 144, 131, 165, 140, 46, 143, 51, 190, 84, 80, 198, 82, 54, 19, 169,
		13, 32, 28,	176, 186, 52, 16, 101, 112, 117, 17, 103, 194, 14, 64, 6, 99, 92, 100, 27, 147,
		73, 23, 183, 108, 47, 130, 53, 105, 66, 109, 145, 158, 173, 199, 111, 182, 174, 97, 83, 9,
		96, 37, 121, 172, 185, 35, 124, 114, 139, 88, 129, 142, 197, 8, 40, 98, 132, 75, 110, 57,
		128, 56, 107, 187, 155, 77, 58, 118, 171, 67, 68, 59, 123, 94, 184, 134, 149, 4, 71, 55, 113,
		179, 65, 115, 137, 72, 5, 119, 166, 95, 160, 45, 48, 122, 10, 177, 175, 178, 146, 90, 150,
		85, 62, 3, 69, 63, 36, 61, 79, 42, 163, 195, 89, 192, 135, 43, 138, 76, 41, 74, 38, 104, 18,
		20, 86, 49, 125, 164, 26, 162, 30, 151, 31, 102, 34, 11, 81, 70, 191, 168, 24, 161, 141, 0,
		180, 120, 116, 154, 91, 126, 7, 153, 156, 127, 181, 33, 148, 133, 159, 193, 188, 22, 157, 87,
		167, 170, 2, 12, 93, 21, 50, 189, 60, 78, 25, 15, 1, 39, 106
};
const uint8_t numabo_z3[200] = {
		6, 109, 119, 16, 115, 77, 113, 125, 123, 51, 43, 165, 189, 150, 54, 146, 39, 18, 12, 82, 98,
		197, 11, 9, 133, 138, 99, 93, 92, 155, 45, 152, 66, 140, 153, 42, 175, 121, 63, 17, 180, 179,
		4, 132, 130, 141, 61, 129, 148, 5, 0, 52, 187, 106, 128, 131, 97, 183, 176, 186, 196, 177,
		164, 29, 169, 181, 151, 62, 31, 178, 166, 190, 168, 34, 76, 105, 89, 160, 21, 94, 30, 49, 135,
		117, 103, 198, 32, 91, 134, 124, 36, 116, 101, 154, 3, 80, 8, 81, 145, 110, 143, 192, 47, 142,
		20, 112, 86, 27, 95, 149, 73, 83, 157, 35, 191, 40, 104, 55, 136, 78, 37, 171, 111, 139, 188,
		58, 57, 161, 88, 84, 15, 184, 159, 158, 127, 156, 108, 173, 137, 118, 24, 72, 23, 59, 100, 120,
		64, 170, 26, 48, 33, 182, 162, 1, 174, 22, 56, 13, 19, 194, 167, 199, 75, 144, 7, 46, 96, 2,
		114, 102, 10, 25, 38, 126, 67, 60, 28, 147, 122, 65, 163, 193, 195, 85, 70, 69, 41, 185, 14,
		68, 90, 107, 71, 79, 87, 74, 53, 44, 50, 172
};
const uint8_t numabo_z4[200] = {
		47, 128, 60, 14, 44, 138, 195, 118, 10, 158, 56, 167, 92, 31, 22, 26, 182, 4, 193, 77, 170, 103,
		139, 143, 140, 173, 136, 96, 125, 188, 46, 9, 40, 99, 1, 38, 137, 63, 57, 130, 73, 21, 16, 124,
		89, 65, 83, 135, 79, 3, 127, 97, 11, 100, 54, 146, 76, 187, 148, 168, 129, 88, 71, 119, 149, 50,
		131, 180, 18, 179, 123, 154, 39, 27, 37, 70, 114, 62, 120, 104, 59, 8, 110, 147, 132, 2, 95, 159,
		185, 64, 61, 175, 33, 194, 198, 164, 93, 191, 48, 178, 87, 41, 72, 157, 116, 15, 196, 151, 84, 42,
		25, 142, 53, 17, 155, 101, 67, 69, 6, 12, 108, 35, 82, 145, 171, 32, 184, 43, 98, 94, 78, 36, 24,
		174, 49, 165, 153, 106, 91, 150, 190, 162, 163, 81, 5, 160, 111, 66, 122, 113, 68, 134, 189, 80,
		199, 90, 181, 161, 144, 45, 13, 183, 172, 51, 121, 107, 102, 55, 19, 105, 20, 29, 74, 166, 86, 177,
		28, 176, 30, 141, 75, 109, 133, 85, 192, 156, 112, 34, 52, 23, 115, 186, 117, 126, 58, 152, 169, 0,
		197, 7
};
const uint8_t numabo_z5[200] = {
		48, 173, 140, 7, 127, 79, 43, 96, 129, 10, 104, 82, 118, 107, 117, 53, 4, 182, 77, 150, 78, 125, 16,
		38, 90, 167, 138, 176, 194, 6, 111, 28, 60, 163, 188, 95, 8, 162, 29, 126, 196, 27, 183, 106, 86, 155,
		144, 177, 56, 87, 83, 71, 179, 166, 17, 186, 149, 112, 141, 74, 151, 153, 3, 20, 108, 157, 14, 42, 101,
		76, 148, 123, 197, 66, 21, 62, 131, 98, 191, 55, 136, 22, 37, 64, 168, 31, 91, 69, 172, 30, 0, 39, 185,
		192, 9, 52, 122, 5, 59, 165, 164, 47, 25, 24, 94, 169, 161, 88, 143, 181, 84, 135, 159, 49, 51, 190, 72,
		152, 41, 116, 26, 73, 46, 67, 158, 23, 170, 65, 174, 187, 44, 45, 156, 142, 80, 13, 134, 109, 97, 32,
		154, 33, 113, 184, 19, 89, 171, 121, 139, 132, 145, 68, 92, 35, 102, 133, 130, 85, 54, 2, 195, 100, 50,
		18, 11, 103, 137, 36, 34, 57, 12, 99, 61, 115, 110, 124, 189, 70, 58, 40, 105, 160, 198, 63, 114, 15, 199,
		175, 1, 193, 180, 178, 128, 120, 75, 81, 146, 93, 119, 147
};

const uint8_t numabo_z6[200] = {
		90, 18, 158, 140, 107, 121, 152, 78, 33, 166, 138, 2, 63, 75, 62, 186, 98, 48, 31, 76, 3, 69, 196, 23,
		178, 39, 159, 21, 139, 146, 188, 79, 137, 184, 10, 182, 37, 101, 54, 176, 50, 0, 124, 52, 108, 143, 34,
		71, 189, 11, 185, 58, 191, 119, 194, 13, 70, 81, 73, 198, 112, 162, 141, 153, 30, 105, 104, 100, 35,
		99, 8, 110, 49, 77, 111, 95, 148, 66, 181, 149, 115, 92, 60, 163, 80, 51, 102, 27, 164, 168, 144, 109,
		118, 169, 4, 170, 5, 87, 106, 22, 117, 151, 65, 68, 130, 197, 86, 45, 53, 59, 180, 26, 122, 199, 84,
		127, 41, 57, 43, 20, 67, 82, 74, 38, 25, 167, 46, 89, 174, 96, 72, 24, 150, 114, 125, 155, 132, 133,
		93, 83, 6, 44, 147, 195, 32, 134, 129, 1, 135, 190, 120, 28, 142, 19, 56, 179, 172, 94, 12, 61, 116,
		14, 161, 29, 177, 103, 88, 154, 7, 97, 171, 131, 183, 40, 193, 17, 192, 91, 85, 173, 16, 15, 160, 55,
		145, 64, 42, 156, 165, 187, 113, 36, 9, 123, 128, 157, 126, 47, 175, 136
};

uint16_t account;

OS_TCB		AlarmDetectTask_TCB;
CPU_STK		AlarmDetectTask_Stk[AlarmDetectTask_STK_SIZE];

const unsigned char bitpat[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
//estados de las maquinas de deteccion
uint8_t daper_state;
uint8_t daper2_state;
uint8_t daper_stateAP;
uint8_t dasa_state;
uint8_t df220_state;
uint8_t dinc_state;
uint8_t drst_state;
uint8_t dteso_state;
uint8_t drotu_state[16];

unsigned char csign_state;
#define CSIGN_IDLE		0x10
#define	CSIGN_RFP1		0x20
#define	CSIGN_RFP2		0x30
#define	CSIGN_RFP3		0x40
#define	CSIGN_RFP4		0x50

uint8_t fmode_state;
#define	FMODE_IDLE		0x10
#define	FMODE_WAIT201	0x20
#define	FMODE_DELAY		0x30

uint8_t fsmEvOnPreve_state;
#define	FSMEVP_IDLE		0x10
#define	FSMEVP_WAIT_T1	0x12
#define	FSMEVP_WAIT_T2	0x14
#define	FSMEVP_WAIT_T3	0x16
#define	FSMEVP_WAIT_T4	0x18

uint8_t fevimdllave_state;
#define	FEVIMD_IDLE		0x10
#define	FEVIMD_LLAVEON	0x15

uint32_t fevptimer;
uint32_t ftxontimer;
uint16_t timerInitCMX;
uint16_t timerIrqCMX;
uint8_t FmodeDelay;

uint8_t fsm_autorstd;
#define FSM_ARSTD_IDLE		0x10
#define FSM_ARSTD_WAIT		0x20
#define FSM_ARSTD_PREVETX	0x30

uint8_t fsm_icmx_state;
#define FSM_ICMX_IDLE		0x10
#define FSM_ICMX_INIT		0x20
#define	FSM_ICMX_NOIRQ		0x40

uint16_t dbnc_ince_timer;
uint16_t dbnc_asal_timer;
uint16_t dbnc_teso_timer;
uint16_t dbnc_rotu_timer;

uint8_t	BaseAlarm_alarm_1, diff_alarm, event_alarm, restore_alarm;
uint8_t BaseAlarm_estados_dispositivos_1, diff_estados, event_estados, restore_estados;
uint8_t BaseAlarm_memoria_dispositivos_1, diff_memoria, event_memoria, restore_memoria;

uint8_t BaseAlarmPkt_numabo;
uint8_t BaseAlarmPkt_alarm;
uint8_t BaseAlarmPkt_estado_dispositivos;
uint8_t BaseAlarmPkt_memoria_dispositivos;
uint8_t TypeAboAns;

//estados de la maquina de autoreset
uint8_t asal_state;
uint8_t teso_state;
uint8_t ince_state;
uint8_t rotu_state;

uint16_t rfw_timer, signature_timer;
uint8_t rfw_fail_counter;
uint8_t rfwatch_state;
#define	FSM_RFW_IDLE	0x10
#define	FSM_RFW_WAITOK	0x11
#define	FSM_RFW_TXOFF	0x12
#define	FSM_RFW_PANIC	0x13

uint8_t ppon_state;


time_t ppon_wdog_timer;


//contadores de autoreset
uint8_t asal_autr_counter;	//para asalto
uint8_t teso_autr_counter;	//para tesoro
uint8_t ince_autr_counter;	//para incendio
uint8_t rotu_autr_counter;	//para rotura
uint8_t autoreset_data;

time_t asal_autr_timer;
time_t ince_autr_timer;
time_t teso_autr_timer;
time_t rotu_autr_timer;


uint8_t BaseZone;
uint8_t Aper_Poll_counter;

uint8_t OptoInputs;			//imagen validada de las entradas optoacopladas
uint8_t SysInputs;			//idem de las alarmas del sistema

uint8_t SysFlag0;			//flag general del sistema

uint8_t SysFlag1;			//flags para el modulo de ADC

uint8_t	SysFlag3;			//flag para el modulo de aviso inmediato

uint8_t	SysFlag4;

uint8_t	DebugFlag;

uint8_t oldptalarm;
#define	OPTZIPT6	0x01
#define	OPTZTPT6	0x02
#define	OPTZIPT7	0x04
#define	OPTZTPT7	0x08
#define	OPTZIPT8	0x10
#define	OPTZTPT8	0x20
#define	OPTZIPT9	0x40
#define	OPTZTPT9	0x80

uint8_t	isrclear;
uint8_t modem_error;

//datos de la maquina de transminsion por RF
uint8_t state_transmit_tx;

uint16_t timerdbncaper;
uint16_t timerdbncaperAP;
uint16_t timerdbncf220v;
uint16_t tachof220v;

uint16_t fsmtx_timer1;
uint8_t acumu_buffer_tx, acumu_buffer_sended;
uint8_t buffer_tx[8];

uint8_t pollsig_state;

uint16_t	preve_timer;

#define	EYSE_EDSC_bit	0x08	//Particion 5
#define	EYSE_EPT1_bit	0x10	//Particion 6
#define	EYSE_EPT2_bit	0x20	//Particion 7
#define	EYSE_EPT3_bit	0x40	//Particion 8
#define	EYSE_EPT4_bit	0x80	//Particion 9

#define	EYSE_IPT1_bit	0x01	//zona 61
#define	EYSE_TPT1_bit	0x02	//zona 62
#define	EYSE_IPT2_bit	0x04	//zona 71
#define	EYSE_TPT2_bit	0x08	//zona 72
#define	EYSE_IPT3_bit	0x10	//zona 81
#define	EYSE_TPT3_bit	0x20	//zona 82
#define	EYSE_IPT4_bit	0x40	//zona 91
#define	EYSE_TPT4_bit	0x80	//zona 92

#define	EYSE_PART_DSC	5
#define	EYSE_PART_PT1	6
#define	EYSE_PART_PT2	7
#define	EYSE_PART_PT3	8
#define	EYSE_PART_PT4	9

#define	EYSE_ZI_PT1		61
#define	EYSE_ZT_PT1		62
#define	EYSE_ZI_PT2		71
#define	EYSE_ZT_PT2		72
#define	EYSE_ZI_PT3		81
#define	EYSE_ZT_PT3		82
#define	EYSE_ZI_PT4		91
#define	EYSE_ZT_PT4		92


uint8_t	ptxormask;

//del abonado pap
uint16_t paptslot;

void DeteccionEventos(void)
{
	OS_ERR os_err;

	if( SystemFlag & WDTRESET_FLAG )	{
		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
		BaseAlarm_memoria_dispositivos_1 = BaseAlarmPkt_memoria_dispositivos;
	}

	if( SystemFlag & PORRESET_FLAG )	{
		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	}
	if( SystemFlag & RESETRESET_FLAG )	{
		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	}
	if( SystemFlag & BODRESET_FLAG )	{
		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	}

	diff_alarm = BaseAlarm_alarm_1 ^ BaseAlarmPkt_alarm;
	diff_estados = BaseAlarm_estados_dispositivos_1 ^ BaseAlarmPkt_estado_dispositivos;
	diff_memoria = BaseAlarm_memoria_dispositivos_1 ^ BaseAlarmPkt_memoria_dispositivos;

	event_alarm = diff_alarm & BaseAlarmPkt_alarm;
	event_estados = diff_estados & BaseAlarmPkt_estado_dispositivos;
	//event_memoria = diff_memoria & BaseAlarmPkt_memoria_dispositivos;
	event_memoria = BaseAlarmPkt_memoria_dispositivos;

	restore_alarm = diff_alarm & (~BaseAlarmPkt_alarm);
	restore_estados = diff_estados & (~BaseAlarmPkt_estado_dispositivos);
	restore_memoria = diff_memoria & (~BaseAlarmPkt_memoria_dispositivos);
	//----------------------------------------------------------------------------------
	//manejamos la se�alizacion de las distintas aperturas
	if( SystemFlag2 & LOGAP1E_FLAG )	{
		SystemFlag2 &= ~LOGAP1E_FLAG;
		logCidEvent(account, 1, 137, 0, 1);
	}
	if( SystemFlag2 & LOGAP1R_FLAG )	{
		SystemFlag2 &= ~LOGAP1R_FLAG;
		logCidEvent(account, 3, 137, 0, 1);
	}
	if( SystemFlag2 & LOGAP2E_FLAG )	{
		SystemFlag2 &= ~LOGAP2E_FLAG;
		logCidEvent(account, 1, 137, 0, 2);
	}
	if( SystemFlag2 & LOGAP2R_FLAG )	{
		SystemFlag2 &= ~LOGAP2R_FLAG;
		logCidEvent(account, 3, 137, 0, 2);
	}
	if( SystemFlag5 & LOGAP3E_FLAG )	{
		SystemFlag5 &= ~LOGAP3E_FLAG;
		logCidEvent(account, 1, 137, 0, 3);
	}
	if( SystemFlag5 & LOGAP3R_FLAG )	{
		SystemFlag5 &= ~LOGAP3R_FLAG;
		logCidEvent(account, 3, 137, 0, 3);
	}
	if( SystemFlag6 & LOGAP0R_FLAG )	{
		SystemFlag6 &= ~LOGAP0R_FLAG;
		logCidEvent(account, 3, 137, 0, 0);
	}

	//----------------------------------------------------------------------------------
	//procesamos las normalizaciones
	if(restore_alarm)	{
		if(restore_alarm & bitpat[ASAL_bit])	{
#ifdef USAR_IRIDIUM
			IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 120, 0, 0);
			logCidEvent(account, 3, 120, 2, 0);
		}
		if(restore_alarm & bitpat[TESO_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 130, 0, 0);
			logCidEvent(account, 3, 130, 4, 0);
			if(oldptalarm & OPTZIPT6)	{
				oldptalarm &= ~OPTZIPT6;
				logCidEvent(account, 3, 130, EYSE_PART_PT1,EYSE_ZI_PT1);
			}
			if(oldptalarm & OPTZTPT6)	{
				oldptalarm &= ~OPTZTPT6;
				logCidEvent(account, 3, 130, EYSE_PART_PT1,EYSE_ZT_PT1);
			}
			if(oldptalarm & OPTZIPT7)	{
				oldptalarm &= ~OPTZIPT7;
				logCidEvent(account, 3, 130, EYSE_PART_PT2,EYSE_ZI_PT2);
			}
			if(oldptalarm & OPTZTPT7)	{
				oldptalarm &= ~OPTZTPT7;
				logCidEvent(account, 3, 130, EYSE_PART_PT2,EYSE_ZT_PT2);
			}
			if(oldptalarm & OPTZIPT8)	{
				oldptalarm &= ~OPTZIPT8;
				logCidEvent(account, 3, 130, EYSE_PART_PT3,EYSE_ZI_PT3);
			}
			if(oldptalarm & OPTZTPT8)	{
				oldptalarm &= ~OPTZTPT8;
				logCidEvent(account, 3, 130, EYSE_PART_PT3,EYSE_ZT_PT3);
			}
			if(oldptalarm & OPTZIPT9)	{
				oldptalarm &= ~OPTZIPT9;
				logCidEvent(account, 3, 130, EYSE_PART_PT4,EYSE_ZI_PT4);
			}
			if(oldptalarm & OPTZTPT9)	{
				oldptalarm &= ~OPTZTPT9;
				logCidEvent(account, 3, 130, EYSE_PART_PT4,EYSE_ZT_PT4);
			}
		}
		if(restore_alarm & bitpat[INCE_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 110, 0, 0);
			logCidEvent(account, 3, 110, 3, 0);
		}
		if(restore_alarm & bitpat[ROTU_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 380, 0, 0);
		}
		if(restore_alarm & bitpat[F220_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 301, 0, 0);
		}
//		if(restore_alarm & bitpat[APER_bit])	{
//			logCidEvent(account, 3, 137, 0, 1);
//		}
		if(restore_alarm & bitpat[LLOP_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 3, 393, 0, 0);
		}
	}
	if(restore_estados)	{
		AlarmWriteHistory();
		if(restore_estados & EYSE_EDSC_bit)	{
			if(PT_estado_particion[0] == 0x00)	{
				if(ptxormask & EYSE_EDSC_bit) {
                    logCidEvent(account, 3, 401, EYSE_PART_DSC, 0);
                    SystemFlag9 |= P5_R401_05_RCVD;
					recharge25min_alarm(ASAL_bit, 3);
					last_voluclose = SEC_TIMER;
                }
				else    {
				    logCidEvent(account, 1, 401, EYSE_PART_DSC, 0);
                    SystemFlag9 |= P5_E401_05_RCVD;
					recharge25min_alarm(ASAL_bit, 1);
				}
			}
		}
		if(restore_estados & EYSE_EPT1_bit)	{
			if(PT_estado_particion[1] == 0x00)	{
				if(ptxormask & EYSE_EPT1_bit) {
                    logCidEvent(account, 3, 401, EYSE_PART_PT1, 0);
                    SystemFlag9 |= P6_R401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 1, 401, EYSE_PART_PT1, 0);
                    SystemFlag9 |= P6_E401_05_RCVD;
				}
			}
		}
		if(restore_estados & EYSE_EPT2_bit)	{
			if(PT_estado_particion[2] == 0x00)	{
				if(ptxormask & EYSE_EPT2_bit) {
                    logCidEvent(account, 3, 401, EYSE_PART_PT2, 0);
                    SystemFlag9 |= P7_R401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 1, 401, EYSE_PART_PT2, 0);
                    SystemFlag9 |= P7_E401_05_RCVD;
				}
			}
		}
		if(restore_estados & EYSE_EPT3_bit)	{
			if(PT_estado_particion[3] == 0x00)	{
				if(ptxormask & EYSE_EPT3_bit) {
                    logCidEvent(account, 3, 401, EYSE_PART_PT3, 0);
                    SystemFlag9 |= P8_R401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 1, 401, EYSE_PART_PT3, 0);
                    SystemFlag9 |= P8_E401_05_RCVD;
				}
			}
		}
		if(restore_estados & EYSE_EPT4_bit)	{
			if(PT_estado_particion[4] == 0x00)	{
				if(ptxormask & EYSE_EPT4_bit) {
                    logCidEvent(account, 3, 401, EYSE_PART_PT4, 0);
                    SystemFlag9 |= P9_R401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 1, 401, EYSE_PART_PT4, 0);
                    SystemFlag9 |= P9_E401_05_RCVD;
				}
			}
		}
	}

	//procesamos las activaciones o eventos
	if(event_alarm)	{
		if(event_alarm & bitpat[ASAL_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			//logCidEvent(account, 1, 120, 2, 0);
			logCidEvent(account, 1, 120, 0, 0);
			recharge_alarm(ASAL_bit);
		}
		if(event_alarm & bitpat[TESO_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			if(event_memoria & EYSE_IPT1_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT1,EYSE_ZI_PT1);
				oldptalarm |= OPTZIPT6;
			} else
			if(event_memoria & EYSE_TPT1_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT1,EYSE_ZT_PT1);
				oldptalarm |= OPTZTPT6;
			} else
			if(event_memoria & EYSE_IPT2_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT2,EYSE_ZI_PT2);
				oldptalarm |= OPTZIPT7;
			} else
			if(event_memoria & EYSE_TPT2_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT2,EYSE_ZT_PT2);
				oldptalarm |= OPTZTPT7;
			} else
			if(event_memoria & EYSE_IPT3_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT3,EYSE_ZI_PT3);
				oldptalarm |= OPTZIPT8;
			} else
			if(event_memoria & EYSE_TPT3_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT3,EYSE_ZT_PT3);
				oldptalarm |= OPTZTPT8;
			} else
			if(event_memoria & EYSE_IPT4_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT4,EYSE_ZI_PT4);
				oldptalarm |= OPTZIPT9;
			} else
			if(event_memoria & EYSE_TPT4_bit)	{
				logCidEvent(account, 1, 130, EYSE_PART_PT4,EYSE_ZT_PT4);
				oldptalarm |= OPTZTPT9;
			}
			//logCidEvent(account, 1, 130, 4, 0);
			logCidEvent(account, 1, 130, 0, 0);
			recharge_alarm(TESO_bit);
		}
		if(event_alarm & bitpat[INCE_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			//logCidEvent(account, 1, 110, 3, 0);
			logCidEvent(account, 1, 110, 0, 0);
			recharge_alarm(INCE_bit);
		}
		if(event_alarm & bitpat[ROTU_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 1, 380, 0, 0);
		}
		if(event_alarm & bitpat[F220_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 1, 301, 0, 0);
		}
//		if(event_alarm & bitpat[APER_bit])	{
//			logCidEvent(account, 1, 137, 0, 1);
//		}
		if(event_alarm & bitpat[LLOP_bit])	{
#ifdef USAR_IRIDIUM
            IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			logCidEvent(account, 1, 393, 0, 0);
		}
	}
	if(event_estados)	{
		AlarmWriteHistory();
		if(event_estados & EYSE_EDSC_bit)	{
			if(PT_estado_particion[0] == 0x00)	{
				if(ptxormask & EYSE_EDSC_bit) {
                    logCidEvent(account, 1, 401, EYSE_PART_DSC, 0);
                    SystemFlag9 |= P5_E401_05_RCVD;
                    recharge25min_alarm(ASAL_bit, 1);
                }
				else    {
				    logCidEvent(account, 3, 401, EYSE_PART_DSC, 0);
                    SystemFlag9 |= P5_R401_05_RCVD;
                    recharge25min_alarm(ASAL_bit, 3);
                    last_voluclose = SEC_TIMER;
				}
			}
		}
		if(event_estados & EYSE_EPT1_bit)	{
			if(PT_estado_particion[1] == 0x00)	{
				if(ptxormask & EYSE_EPT1_bit) {
                    logCidEvent(account, 1, 401, EYSE_PART_PT1, 0);
                    SystemFlag9 |= P6_E401_05_RCVD;
                }
				else {
                    logCidEvent(account, 3, 401, EYSE_PART_PT1, 0);
                    SystemFlag9 |= P6_R401_05_RCVD;
                }
			}
		}
		if(event_estados & EYSE_EPT2_bit)	{
			if(PT_estado_particion[2] == 0x00)	{
				if(ptxormask & EYSE_EPT2_bit) {
                    logCidEvent(account, 1, 401, EYSE_PART_PT2, 0);
                    SystemFlag9 |= P7_E401_05_RCVD;
                }
				else {
                    logCidEvent(account, 3, 401, EYSE_PART_PT2, 0);
                    SystemFlag9 |= P7_R401_05_RCVD;
                }
			}
		}
		if(event_estados & EYSE_EPT3_bit)	{
			if(PT_estado_particion[3] == 0x00)	{
				if(ptxormask & EYSE_EPT3_bit) {
                    logCidEvent(account, 1, 401, EYSE_PART_PT3, 0);
                    SystemFlag9 |= P8_E401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 3, 401, EYSE_PART_PT3, 0);
                    SystemFlag9 |= P8_R401_05_RCVD;
				}
			}
		}
		if(event_estados & EYSE_EPT4_bit)	{
			if(PT_estado_particion[4] == 0x00)	{
				if(ptxormask & EYSE_EPT4_bit) {
                    logCidEvent(account, 1, 401, EYSE_PART_PT4, 0);
                    SystemFlag9 |= P9_E401_05_RCVD;
                }
				else    {
				    logCidEvent(account, 3, 401, EYSE_PART_PT4, 0);
                    SystemFlag9 |= P9_R401_05_RCVD;
				}
			}
		}
	}

	//----------------------------------------------------------------------------------
	BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
	BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	BaseAlarm_memoria_dispositivos_1 = BaseAlarmPkt_memoria_dispositivos;

	//----------------------------------------------------------------------------------
	if((epreve_state == PRV_PREVE) && (BaseAlarmPkt_alarm == 0x80))	{
		eyseAlarma = 0xAA;
	} else	{
		eyseAlarma = BaseAlarmPkt_alarm;
	}
	eyseEstados = BaseAlarmPkt_estado_dispositivos;
	eyseDisparos = BaseAlarmPkt_memoria_dispositivos;
}

//***************************************************************************************************
//* Maquinas de estado para la deteccion de alarma

void fsm_deteccion_incendio( void )
{

	int16_t vreal;

	if(SysFlag_AP_GenAlarm & bitpat[INCE_bit])	{
		SysFlag_AP_GenAlarm &= ~bitpat[INCE_bit];
		if( !(BaseAlarmPkt_alarm & bitpat[INCE_bit]) )	{
			BaseAlarmPkt_alarm |= bitpat[INCE_bit];
			AlarmWriteHistory();
			SysFlag3 |= SEND_flag;
			SysFlag3 |= SENDM_flag;
		}
		BaseAlarmPkt_alarm |= bitpat[INCE_bit];
		dinc_state = INCENDIO_ALRM;
	}

	switch( dinc_state )	{
		case INCENDIO_IDLE :
			if( Status_Zonas[ZONA_INCENDIO] == ALRM_EVENTO )	{
				dinc_state = INCENDIO_WAIT;
				dbnc_ince_timer = 500;
			} else
			if( Status_Zonas[ZONA_INCENDIO] == ALRM_INDEF )	{
				dinc_state = INCENDIO_INDEF;
				dbnc_ince_timer = 500;
			}
			break;
		case INCENDIO_WAIT:
			if(!dbnc_ince_timer)	{
				dinc_state = INCENDIO_ALRM;
				if( !(BaseAlarmPkt_alarm & bitpat[INCE_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[INCE_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
				BaseAlarmPkt_alarm |= bitpat[INCE_bit];
				//*** OJO
				logCidEvent(account, 1, 110, 3, 0);
				//*******
				recharge_alarm(INCE_bit);
			} else if(Status_Zonas[ZONA_INCENDIO] != ALRM_EVENTO)	{
				dinc_state = INCENDIO_IDLE;
			}
			break;
		case INCENDIO_ALRM :
			if( Status_Zonas[ZONA_INCENDIO] != ALRM_EVENTO )	{
				dinc_state = INCENDIO_IDLE;
			} else if( Status_Zonas[ZONA_INCENDIO] == ALRM_EVENTO )	{
				if( !(BaseAlarmPkt_alarm & bitpat[INCE_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[INCE_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
			}
			break;
		case INCENDIO_INDEF:
			if(!dbnc_ince_timer)	{
				dinc_state = INCENDIO_INDEF1;
				if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
				BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
				recharge_alarm(ROTU_bit);
				vreal = get_vreal(1);
				logCidEvent(account, 1, 381, 3, vreal);
				recharge_alarm(ROTU_bit);
			} else
			if( Status_Zonas[ZONA_INCENDIO] != ALRM_INDEF )	{
				dinc_state = INCENDIO_IDLE;
			}
			break;
		case ASALTO_INDEF1:
			if( Status_Zonas[ZONA_INCENDIO] != ALRM_INDEF )	{
				dasa_state = INCENDIO_IDLE;
			}
			break;
		default :
			dinc_state = INCENDIO_IDLE;
			break;
	}
}

void fsm_deteccion_asalto( void )
{

	int16_t vreal;

	if(SysFlag_AP_GenAlarm & bitpat[ASAL_bit])	{
		SysFlag_AP_GenAlarm &= ~bitpat[ASAL_bit];
		if( !(BaseAlarmPkt_alarm & bitpat[ASAL_bit]) )	{
			BaseAlarmPkt_alarm |= bitpat[ASAL_bit];
			AlarmWriteHistory();
			SysFlag3 |= SEND_flag;
			SysFlag3 |= SENDM_flag;
			recharge_alarm(ASAL_bit);
		}
		BaseAlarmPkt_alarm |= bitpat[ASAL_bit];
		dasa_state = ASALTO_ALRM;
	}

	switch( dasa_state )	{
		case ASALTO_IDLE :
			if( Status_Zonas[ZONA_ASALTO] == ALRM_EVENTO )	{
				dasa_state = ASALTO_WAIT;
				dbnc_asal_timer = 500;
			} else
			if( Status_Zonas[ZONA_ASALTO] == ALRM_INDEF )	{
				dasa_state = ASALTO_INDEF;
				dbnc_asal_timer = 500;
			}
			break;
		case ASALTO_WAIT:
			if(!dbnc_asal_timer)	{
				dasa_state = ASALTO_ALRM;
				if( !(BaseAlarmPkt_alarm & bitpat[ASAL_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[ASAL_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
				BaseAlarmPkt_alarm |= bitpat[ASAL_bit];
				//****** OJO
				logCidEvent(account, 1, 120, 2, 0);
				//**********
				recharge_alarm(ASAL_bit);

			} else if(Status_Zonas[ZONA_ASALTO] != ALRM_EVENTO)	{
				dasa_state = ASALTO_IDLE;
			}
			break;
		case ASALTO_ALRM :
			if( Status_Zonas[ZONA_ASALTO] != ALRM_EVENTO )	{
				dasa_state = ASALTO_IDLE;
			} else if( Status_Zonas[ZONA_ASALTO] == ALRM_EVENTO )	{
				if( !(BaseAlarmPkt_alarm & bitpat[ASAL_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[ASAL_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
			}
			break;
		case ASALTO_INDEF:
			if(!dbnc_asal_timer)	{
				dasa_state = ASALTO_INDEF1;
				if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
				BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
				vreal = get_vreal(0);
				logCidEvent(account, 1, 381, 2, vreal);
				recharge_alarm(ROTU_bit);
			} else
			if( Status_Zonas[ZONA_ASALTO] != ALRM_INDEF )	{
				dasa_state = ASALTO_IDLE;
			}
			break;
		case ASALTO_INDEF1:
			if( Status_Zonas[ZONA_ASALTO] != ALRM_INDEF )	{
				dasa_state = ASALTO_IDLE;
			}
			break;
		default :
			dasa_state = ASALTO_IDLE;
			break;
	}

}

void fsm_deteccion_tesoro( void )
{
	int16_t vreal;

	if(SysFlag_AP_GenAlarm & bitpat[TESO_bit])	{
		SysFlag_AP_GenAlarm &= ~bitpat[TESO_bit];
		if( !(BaseAlarmPkt_alarm & bitpat[TESO_bit]) )	{
			BaseAlarmPkt_alarm |= bitpat[TESO_bit];
			BaseAlarmPkt_memoria_dispositivos = OptoInputs;
			AlarmWriteHistory();
			SysFlag3 |= SEND_flag;
			SysFlag3 |= SENDM_flag;
		}
		BaseAlarmPkt_alarm |= bitpat[TESO_bit];
		dteso_state = TESORO_ALRM;
	}

	switch( dteso_state )	{
		case TESORO_IDLE :
			if( Status_Zonas[ZONA_TESORO] == ALRM_EVENTO )	{
				dteso_state = TESORO_WAIT;
				dbnc_teso_timer = 500;
			} else
			if( Status_Zonas[ZONA_TESORO] == ALRM_INDEF )	{
				dteso_state = TESORO_INDEF;
				dbnc_teso_timer = 500;
			}
			break;
		case TESORO_WAIT:
			if(!dbnc_teso_timer)	{
                RFDLYBOR_flag |= RFDLYBOR_TESO_FLAG;
				dteso_state = TESORO_ALRM;

//				if( !(BaseAlarmPkt_alarm & bitpat[TESO_bit]) )	{
//					BaseAlarmPkt_alarm |= bitpat[TESO_bit];
//					BaseAlarmPkt_memoria_dispositivos = OptoInputs;
//					AlarmWriteHistory();
//					SysFlag3 |= SEND_flag;
//					SysFlag3 |= SENDM_flag;
//				}
//				BaseAlarmPkt_alarm |= bitpat[TESO_bit];
//				//*** OJO
//				logCidEvent(account, 1, 130, 4, 0);
//				//******
//				recharge_alarm(TESO_bit);
				
			} else if(Status_Zonas[ZONA_TESORO] != ALRM_EVENTO)	{
				dteso_state = TESORO_IDLE;
			}
			break;
		case TESORO_ALRM :
			if( Status_Zonas[ZONA_TESORO] != ALRM_EVENTO )	{
				dteso_state = TESORO_IDLE;
			} else if( Status_Zonas[ZONA_TESORO] == ALRM_EVENTO )	{
			    if(!(RFDLYBOR_flag & RFDLYBOR_TESO_FLAG))   {
                    RFDLYBOR_flag |= RFDLYBOR_TESO_FLAG;
                    dteso_state = TESORO_ALRM;
			    }
/*				if( !(BaseAlarmPkt_alarm & bitpat[TESO_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[TESO_bit];
					BaseAlarmPkt_memoria_dispositivos = OptoInputs;
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}*/
			}
			break;
		case TESORO_INDEF:
			if(!dbnc_teso_timer)	{
				dteso_state = TESORO_INDEF1;
				if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
					BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
					AlarmWriteHistory();
					SysFlag3 |= SEND_flag;
					SysFlag3 |= SENDM_flag;
				}
				BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
				recharge_alarm(TESO_bit);
				vreal = get_vreal(2);
				logCidEvent(account, 1, 381, 4, vreal);
			} else
			if( Status_Zonas[ZONA_TESORO] != ALRM_INDEF )	{
				dteso_state = TESORO_IDLE;
			}
			break;
		case TESORO_INDEF1:
			if( Status_Zonas[ZONA_TESORO] != ALRM_INDEF )	{
				dteso_state = TESORO_IDLE;
			}
			break;
		default :
			dteso_state = TESORO_IDLE;
			break;
	}
}


uint8_t apdisp_state[5];
uint16_t apdisp_timer[5];

void fsm_deteccion_dispositivos( void )
{
	int i;

	for( i = 3; i < 8; i++ )	{
		if(PT_estado_particion[i-3] != 0x00)
			continue;
		switch(apdisp_state[i - 3])	{
		case TESORO_IDLE :
			if( Status_Zonas[i] == ALRM_EVENTO )	{
				apdisp_state[i - 3] = TESORO_WAIT;
				apdisp_timer[i - 3] = 1000;
			}
			break;
		case TESORO_WAIT:
			if(!apdisp_timer[i - 3])	{
				apdisp_state[i - 3] = TESORO_ALRM;
				switch(i){
					case 3:
						BaseAlarmPkt_estado_dispositivos |= 0x08;
						break;
					case 4:
						BaseAlarmPkt_estado_dispositivos |= 0x10;
						break;
					case 5:
						BaseAlarmPkt_estado_dispositivos |= 0x20;
						break;
					case 6:
						BaseAlarmPkt_estado_dispositivos |= 0x40;
						break;
					case 7:
						BaseAlarmPkt_estado_dispositivos |= 0x80;
						break;
				}
			} else if(Status_Zonas[i] != ALRM_EVENTO)	{
				apdisp_state[i - 3] = TESORO_IDLE;
			}
			break;
		case TESORO_ALRM :
			if( Status_Zonas[i] == ALRM_NORMAL )	{
				apdisp_timer[i - 3] = 1000;
				apdisp_state[i - 3] = TESORO_WAIT2;
			}
			break;
		case TESORO_WAIT2:
			if( !apdisp_timer[i - 3] )		{
				apdisp_state[i - 3] = TESORO_IDLE;
				switch(i){
				case 3:
					BaseAlarmPkt_estado_dispositivos &= ~0x08;
					break;
				case 4:
					BaseAlarmPkt_estado_dispositivos &= ~0x10;
					break;
				case 5:
					BaseAlarmPkt_estado_dispositivos &= ~0x20;
					break;
				case 6:
					BaseAlarmPkt_estado_dispositivos &= ~0x40;
					break;
				case 7:
					BaseAlarmPkt_estado_dispositivos &= ~0x80;
					break;
				}
			} else
			if( Status_Zonas[i] == ALRM_EVENTO )	{
				apdisp_state[i - 3] = TESORO_ALRM;
			}
			break;
		}
	}

}


void fsm_deteccion_rotura( void )
{
	unsigned char i;
	int16_t vreal;
	OS_ERR os_err;

	if(SysFlag_AP_GenAlarm & bitpat[ROTU_bit])	{
		SysFlag_AP_GenAlarm &= ~bitpat[ROTU_bit];
		if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
			BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
			AlarmWriteHistory();
			SysFlag3 |= SEND_flag;
			SysFlag3 |= SENDM_flag;
		}
		BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
	}
	for( i = 0; i < 8; i++ )	{
		switch( drotu_state[i] )	{
			case ROTU_IDLE :
				if( Status_Zonas[i] == ALRM_ROTU )	{
					drotu_state[i] = ROTU_WAIT;
					dbnc_rotu_timer = 1000;
				}
				break;
			case ROTU_WAIT:
				if(!dbnc_rotu_timer)	{
					drotu_state[i] = ROTU_ALRM;
					if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
						BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
						AlarmWriteHistory();
						SysFlag3 |= SEND_flag;
						SysFlag3 |= SENDM_flag;
					}
					BaseAlarmPkt_alarm |= bitpat[ROTU_bit];

					OSTimeDlyHMSM(0, 0, 0, 700, OS_OPT_TIME_HMSM_STRICT, &os_err);
					vreal = get_vreal(i);
					logCidEvent(account, 1, 380, i+2, vreal);

				} else if(Status_Zonas[i] != ALRM_ROTU)	{
					drotu_state[i] = ROTU_IDLE;
				}
				break;
			case ROTU_ALRM:
				if( Status_Zonas[i] != ALRM_ROTU )	{
					drotu_state[i] = ROTU_IDLE;

					OSTimeDlyHMSM(0, 0, 0, 700, OS_OPT_TIME_HMSM_STRICT, &os_err);
					vreal = get_vreal(i);
					logCidEvent(account, 3, 380, i+2, vreal);

				} else if( Status_Zonas[i] == ALRM_ROTU )	{
					if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) )	{
						BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
						AlarmWriteHistory();
						SysFlag3 |= SEND_flag;
						SysFlag3 |= SENDM_flag;

						OSTimeDlyHMSM(0, 0, 0, 700, OS_OPT_TIME_HMSM_STRICT, &os_err);
						vreal = get_vreal(i);
						logCidEvent(account, 1, 380, i+2, vreal);
					}
				}
				break;
			default :
				drotu_state[i] = ROTU_IDLE;
				break;
		}
	}
}

//void fsm_deteccion_dispositivos( void )
//{
//	unsigned char i;
//
//	for( i = 3; i < 8; i++ )	{
//		if( (Status_Zonas[i] == ALRM_EVENTO) || (Status_Zonas[i] == ALRM_INDEF))	{
//			switch(i){
//				case 3:
//					BaseAlarmPkt_estado_dispositivos |= 0x08;
//					break;
//				case 4:
//					BaseAlarmPkt_estado_dispositivos |= 0x10;
//					break;
//				case 5:
//					BaseAlarmPkt_estado_dispositivos |= 0x20;
//					break;
//				case 6:
//					BaseAlarmPkt_estado_dispositivos |= 0x40;
//					break;
//				case 7:
//					BaseAlarmPkt_estado_dispositivos |= 0x80;
//					break;
//			}
//		}
//		else if( Status_Zonas[i] == ALRM_NORMAL )
//			switch(i){
//				case 3:
//					BaseAlarmPkt_estado_dispositivos &= ~0x08;
//					break;
//				case 4:
//					BaseAlarmPkt_estado_dispositivos &= ~0x10;
//					break;
//				case 5:
//					BaseAlarmPkt_estado_dispositivos &= ~0x20;
//					break;
//				case 6:
//					BaseAlarmPkt_estado_dispositivos &= ~0x40;
//					break;
//				case 7:
//					BaseAlarmPkt_estado_dispositivos &= ~0x80;
//					break;
//			}
//	}
//}

void fsm_deteccion_aperturaAP( void )
{
	switch( daper_stateAP )	{
			case APER_IDLE :
				if( (SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG) )	{
					daper_stateAP = APER_WAIT;
					timerdbncaperAP = 500;
				}
				break;
			case APER_WAIT:
				if( (!(SysInputs & APER_sbit)) && (!(RADAR_flags & AP_RADAR_FLAG)) )	{
					daper_stateAP = APER_IDLE;
				} else
				if( !timerdbncaperAP)	{
					daper_stateAP = APER_ALRM;
					SysFlag_AP_Apertura |= AP_APR_APRLINE;		//notifico a la maquina de autoprotect
				}
				break;
			case APER_ALRM :
				if( (!(SysInputs & APER_sbit)) && (!(RADAR_flags & AP_RADAR_FLAG)) )	{
					daper_stateAP = APER_WAIT2;
					timerdbncaperAP = 500;
				}
				break;
			case APER_WAIT2:
				if( (SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG) )	{
					daper_stateAP = APER_ALRM;
				} else
				if(!timerdbncaperAP)	{
					SysFlag_AP_Apertura &= ~AP_APR_APRLINE;		//notifico a la maquina de autoprotect
					daper_stateAP = APER_IDLE;
				}
				break;
			default :
				daper_stateAP = APER_IDLE;
				break;
		}
}

void fsm_deteccion_apertura( void )
{

	switch( daper_state )	{
		case APER_IDLE :
			if( (SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG)  )	{
				daper_state = APER_WAIT;
				timerdbncaper = 500;
			}
			else
			if(SystemFlag2 & APE1WDOG_FLAG)	{
				SystemFlag2 &= ~APE1WDOG_FLAG;
			}
			break;
		case APER_WAIT:
			if( (!(SysInputs & APER_sbit)) && (!(RADAR_flags & AP_RADAR_FLAG)) )	{
				daper_state = APER_IDLE;
			} else
			if( !timerdbncaper)	{
				SystemFlag3 |= NAPER_flag;
				daper_state = APER_ALRM;
				BaseAlarmPkt_alarm |= bitpat[APER_bit];
				if(SystemFlag2 & APE1WDOG_FLAG)	{
					SystemFlag2 &= ~APE1WDOG_FLAG;
					Aper_Poll_counter = 0;
				} else	{
					if(RADAR_flags & AP_RADAR_FLAG)
						SystemFlag5 |= LOGAP3E_FLAG;
					if(SysInputs & APER_sbit)
						SystemFlag2 |= LOGAP1E_FLAG;
					Aper_Poll_counter = APER_POLL_COUNT;
				}
				AlarmWriteHistory();
				//Aper_Poll_counter = APER_POLL_COUNT;
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
				SysFlag3 |= SEND_flag;
				SysFlag3 |= SENDM_flag;
#ifdef USAR_IRIDIUM
                IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			}
			break;
		case APER_ALRM :
			if(led_dcb[APER_led].led_blink == 0)	{
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				//led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
			}
			if( (!(SysInputs & APER_sbit)) && (!(RADAR_flags & AP_RADAR_FLAG)) )	{
				daper_state = APER_WAIT2;
				timerdbncaper = 500;
			}
			break;
		case APER_WAIT2:
			if(led_dcb[APER_led].led_blink == 0)	{
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				//led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
			}
			if( (SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG) )	{
				daper_state = APER_ALRM;
			} else
			if(!timerdbncaper)	{
				if( (!(SysInputs & APER_sbit)) && (!(RADAR_flags & AP_RADAR_FLAG)) && (Aper_Poll_counter == 0) )	{
					//SystemFlag2 |= LOGAP1R_FLAG;
					//SystemFlag5 |= LOGAP3R_FLAG;
					SystemFlag6 |= LOGAP0R_FLAG;
					daper_state = APER_IDLE;
					if(!(((SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG)) || (SystemFlag2 & APE2_sbit)))	{
						BaseAlarmPkt_alarm &= ~bitpat[APER_bit];
						AlarmWriteHistory();
						led_dcb[APER_led].led_state = LED_IDLE;
						led_dcb[APER_led].led_cad = 0;
						led_dcb[APER_led].led_flags &= ~0x01;
						daper_state = APER_IDLE;
						SysFlag3 |= SENDM_flag;
#ifdef USAR_IRIDIUM
                        IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
					}
				}
			}
			break;
		default :
			daper_state = APER_IDLE;
			break;
	}
	//tambien manejo el pulsador de llamada de operador
	//if( !(SysInputs & LLOP_sbit) && !(BaseAlarmPkt_alarm & bitpat[LLOP_bit]) )	{
	//	BaseAlarmPkt_alarm |= bitpat[LLOP_bit];
	//	led_dcb[LLOP_led].led_cad = 255*0x100 + 0;
	//}


}

//si es version 5.00 o superior se controla doble apertura con radar
uint8_t AperMagRad_state;
#define AMR_IDLE	10
#define	AMR_RADWAIT	20
#define	AMR_MAGWAIT	30
#define	AMR_APEROK	40
#define	AMR_APERNG	50
#define	AMR_RADWAIC	60
#define	AMR_MAGWAIC	70
#define	AMR_CLOSENG	80

uint32_t amr_timer;

void fsm_AperMagRadar( void )
{
	switch( AperMagRad_state )	{
		case AMR_IDLE:
			if(SysInputs & APER_sbit)	{
				amr_timer = 30000;
				AperMagRad_state = AMR_RADWAIT;
			}
			if(RADAR_flags & AP_RADAR_FLAG)	{
				amr_timer = 30000;
				AperMagRad_state = AMR_MAGWAIT;
			}
			break;
		case AMR_MAGWAIT:
			if(SysInputs & APER_sbit)	{
				AperMagRad_state = AMR_APEROK;
			} else
			if(!amr_timer)	{
				AperMagRad_state = AMR_APERNG;
				logCidEvent(account, 1, 913, 0, 1);
			}
			break;
		case AMR_RADWAIT:
			if(RADAR_flags & AP_RADAR_FLAG)	{
				AperMagRad_state = AMR_APEROK;
			} else
			if(!amr_timer)	{
				AperMagRad_state = AMR_APERNG;
				logCidEvent(account, 1, 913, 0, 2);
			}
			break;
		case AMR_APERNG:
			if((!(SysInputs & APER_sbit))&&(!(RADAR_flags & AP_RADAR_FLAG)))	{
				AperMagRad_state = AMR_IDLE;
			}
			break;
		case AMR_APEROK:
			if(!(SysInputs & APER_sbit))	{
				amr_timer = 30000;
				AperMagRad_state = AMR_RADWAIC;
			}
			if(!(RADAR_flags & AP_RADAR_FLAG))	{
				amr_timer = 30000;
				AperMagRad_state = AMR_MAGWAIC;
			}
			break;
		case AMR_RADWAIC:
			if(!(RADAR_flags & AP_RADAR_FLAG))	{
				AperMagRad_state = AMR_IDLE;
			} else
			if(!amr_timer)	{
				AperMagRad_state = AMR_CLOSENG;
				logCidEvent(account, 1, 914, 0, 2);
			}
			break;
		case AMR_MAGWAIC:
			if(!(SysInputs & APER_sbit))	{
				AperMagRad_state = AMR_IDLE;
			} else
			if(!amr_timer)	{
				AperMagRad_state = AMR_CLOSENG;
				logCidEvent(account, 1, 914, 0, 1);
			}
			break;
		case AMR_CLOSENG:
			if((!(SysInputs & APER_sbit))&&(!(RADAR_flags & AP_RADAR_FLAG)))	{
				AperMagRad_state = AMR_IDLE;
			}
			break;
		default:
			AperMagRad_state = AMR_IDLE;
			break;
	}
}

void fsm_deteccion_apertura2( void )
{

	switch( daper2_state )	{
		case APER_IDLE :
			if( SystemFlag2 & APE2_sbit  )	{
				daper2_state = APER_WAIT;
				timerdbncaper = 500;
			}
			else
			if(SystemFlag2 & APE2WDOG_FLAG)	{
				SystemFlag2 &= ~APE2WDOG_FLAG;
			}
			break;
		case APER_WAIT:
			if( !(SystemFlag2 & APE2_sbit) )	{
				daper2_state = APER_IDLE;
			} else
			if( !timerdbncaper)	{
				daper2_state = APER_ALRM;
				BaseAlarmPkt_alarm |= bitpat[APER_bit];
				if(SystemFlag2 & APE2WDOG_FLAG)	{
					SystemFlag2 &= ~APE2WDOG_FLAG;
					Aper_Poll_counter = 0;
				} else	{
					SystemFlag2 |= LOGAP2E_FLAG;
					Aper_Poll_counter = APER_POLL_COUNT;
				}
				AlarmWriteHistory();
				//Aper_Poll_counter = APER_POLL_COUNT;
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
				SysFlag3 |= SEND_flag;
				SysFlag3 |= SENDM_flag;
#ifdef USAR_IRIDIUM
                IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
			}
			break;
		case APER_ALRM :
			if(led_dcb[APER_led].led_blink == 0)	{
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				//led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
			}
			if( !(SystemFlag2 & APE2_sbit) )	{
				daper2_state = APER_WAIT2;
				timerdbncaper = 500;
			}
			break;
		case APER_WAIT2:
			if(led_dcb[APER_led].led_blink == 0)	{
				led_dcb[APER_led].led_cad = 255*0x100 + 0;
				//led_dcb[APER_led].led_state = LED_IDLE;
				led_dcb[APER_led].led_flags |= 0x01;
			}
			if( SystemFlag2 & APE2_sbit )	{
				daper2_state = APER_ALRM;
			} else
			if(!timerdbncaper)	{
				if( !(SystemFlag2 & APE2_sbit) && (Aper_Poll_counter == 0) )	{
					SystemFlag2 |= LOGAP2R_FLAG;
					daper2_state = APER_IDLE;
					if(!(((SysInputs & APER_sbit) || (RADAR_flags & AP_RADAR_FLAG)) || (SystemFlag2 & APE2_sbit)))	{
						BaseAlarmPkt_alarm &= ~bitpat[APER_bit];
						AlarmWriteHistory();
						led_dcb[APER_led].led_state = LED_IDLE;
						led_dcb[APER_led].led_cad = 0;
						led_dcb[APER_led].led_flags &= ~0x01;
						daper2_state = APER_IDLE;
						SysFlag3 |= SENDM_flag;
#ifdef USAR_IRIDIUM
                        IRIDIUM_flag |= IRI_SENDALRM_FLAG;
#endif
					}
				}
			}
			break;
		default :
			daper2_state = APER_IDLE;
			break;
	}
	//tambien manejo el pulsador de llamada de operador
	//if( !(SysInputs & LLOP_sbit) && !(BaseAlarmPkt_alarm & bitpat[LLOP_bit]) )	{
	//	BaseAlarmPkt_alarm |= bitpat[LLOP_bit];
	//	led_dcb[LLOP_led].led_cad = 255*0x100 + 0;
	//}


}


void fsm_deteccion_f220( void )
{
    if(BaseAlarmPkt_alarm & bitpat[F220_bit])   {
        led_dcb[F220_led].led_cad = 255*0x100 + 0;
    }
	switch( df220_state )	{
		case F220_IDLE :
			if( SysInputs & F220_sbit )	{
				df220_state = F220_WAIT;
                if(SysFlag1 & SF220_flag)   {
                    timerdbncf220v = 0;
                    SysFlag1 &= ~SF220_flag;
                } else
				if(AP_apertura_state == AP_APER_OP_NORMAL)	{
					timerdbncf220v = 2;
				} else	{
					timerdbncf220v = 15 + (tachof220v * 60);
				}
			} else
			if(BaseAlarmPkt_alarm & bitpat[F220_bit])	{
				BaseAlarmPkt_alarm &= ~bitpat[F220_bit];
				AlarmWriteHistory();
				led_dcb[F220_led].led_state = LED_IDLE;
				led_dcb[F220_led].led_cad = 0;
				df220_state = F220_IDLE;
				SysFlag3 |= SENDM_flag;
			}
			break;
		case F220_WAIT:
			if( !(SysInputs & F220_sbit) )	{
				df220_state = F220_IDLE;
			} else
			if( !timerdbncf220v)	{
				BaseAlarmPkt_alarm |= bitpat[F220_bit];
				AlarmWriteHistory();
				led_dcb[F220_led].led_cad = 255*0x100 + 0;
				SysFlag3 |= SENDM_flag;
				df220_state = F220_ALRM;
			}
			break;
		case F220_ALRM :
			if( !(SysInputs & F220_sbit) )	{
				BaseAlarmPkt_alarm &= ~bitpat[F220_bit];
				AlarmWriteHistory();
				led_dcb[F220_led].led_state = LED_IDLE;
				led_dcb[F220_led].led_cad = 0;
				df220_state = F220_IDLE;
				SysFlag3 |= SENDM_flag;
			}
			break;
		default :
			df220_state = F220_IDLE;
			break;
	}

}


void fsm_ppon_wdog( void )
{
	OS_ERR os_err;

	if(!(SysFlag4 & USEPPONWDOG_flag))
	    return;

    switch(ppon_state)  {
        case FSM_PPON_IDLE:
            if( SysFlag1 & INPPON_flag )    {
                ppon_state = FSM_PPON_ON;
                ppon_wdog_timer = SEC_TIMER;
            } else
            if( SEC_TIMER > (ppon_wdog_timer + 10*60)) {
                ppon_state = FSM_PPON_WAIT;
                ppon_wdog_timer = SEC_TIMER;
                POWER_TX_OFF();
                logCidEvent(account, 1, 353, 0, 1);
            }
            break;
        case FSM_PPON_ON:
            if( !(SysFlag1 & INPPON_flag) ) {
                ppon_state = FSM_PPON_IDLE;
                ppon_wdog_timer = SEC_TIMER;
            } else
            if( SEC_TIMER > (ppon_wdog_timer + 1*60)) {
                ppon_state = FSM_PPON_ALRM2;
                ppon_wdog_timer = SEC_TIMER;
                logCidEvent(account, 1, 353, 0, 2);
            }
            break;
        case FSM_PPON_OFF:
            if( SysFlag1 & INPPON_flag )    {
                ppon_state = FSM_PPON_ON;
                ppon_wdog_timer = SEC_TIMER;
            } else
            if( SEC_TIMER > (ppon_wdog_timer + 10*60)) {
				ppon_state = FSM_PPON_WAIT;
				ppon_wdog_timer = SEC_TIMER;
				POWER_TX_OFF();
				logCidEvent(account, 1, 353, 0, 1);
            }
            break;
		case FSM_PPON_WAIT:
			if( SEC_TIMER > (ppon_wdog_timer + 15))	{
				ppon_state = FSM_PPON_WAITON;
				ppon_wdog_timer = SEC_TIMER;
				POWER_TX_ON();
			}
			break;
    	case FSM_PPON_WAITON: 
			if( SEC_TIMER > (ppon_wdog_timer + 15))	{
				ppon_state = FSM_PPON_ALRM;
				ppon_wdog_timer = SEC_TIMER;
				POWER_TX_ON();
			}
    		break;
        case FSM_PPON_ALRM:
            if( SysFlag1 & INPPON_flag ) {
                ppon_state = FSM_PPON_ON;
                ppon_wdog_timer = SEC_TIMER;
                logCidEvent(account, 3, 353, 0, 1);
            } else
            //if((SEC_TIMER >  (ppon_wdog_timer + 80*60)) && (!(IRIDIUM_flag & IRI_PENDING_FLAG)))	{
			if((SEC_TIMER >  (ppon_wdog_timer + 80*60)) )	{
                logCidEvent(account, 3, 353, 0, 1);
                LLAVE_TX_OFF();
                POWER_TX_OFF();
                OSTimeDlyHMSM(0, 3, 0, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
                ppon_state = FSM_PPON_IDLE;
                ppon_wdog_timer = SEC_TIMER;
                POWER_TX_ON();
                //while (1);        //se los saco por el caso vicuna maquena

			}
            break;
		case FSM_PPON_ALRM2:
			if( !(SysFlag1 & INPPON_flag) ) {
				ppon_state = FSM_PPON_IDLE;
				ppon_wdog_timer = SEC_TIMER;
				logCidEvent(account, 3, 353, 0, 2);
			}
			break;
        case FSM_PPON_TSTPP:
            if(SEC_TIMER >  (ppon_wdog_timer + 5))  {
                if( !(SysFlag1 & INPPON_flag) ) {
                    logCidEvent(account, 3, 353, 0, 1);
                    LLAVE_TX_OFF();
                    POWER_TX_OFF();
                    OSTimeDlyHMSM(0, 1, 0, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);

                    while (1);
                } else  {
                    ppon_state = FSM_PPON_ALRM;
                    ppon_wdog_timer = SEC_TIMER;
                    POWER_TX_ON();
                    LLAVE_TX_OFF();
                }
            }
            break;
        default:
            ppon_state = FSM_PPON_IDLE;
            ppon_wdog_timer = SEC_TIMER;
            break;
    }
}

void fsm_rfwatchdog( void )
{
	switch( rfwatch_state )	{
		case FSM_RFW_IDLE :
			if( SysFlag1 & INPPON_flag )	{
				rfw_timer = 180;
				rfwatch_state = FSM_RFW_WAITOK;
			}
			break;
		case FSM_RFW_WAITOK :
			if( !rfw_timer )	{
				rfw_timer = 10;
				POWER_TX_OFF();
				LLAVE_TX_OFF();
				rfwatch_state = FSM_RFW_TXOFF;
			}
			if( !(SysFlag1 & INPPON_flag) )	{
				rfwatch_state = FSM_RFW_IDLE;
			}
			break;
		case FSM_RFW_TXOFF :
			if( !rfw_timer )	{
				POWER_TX_ON();
				rfw_fail_counter++;
				rfwatch_state = FSM_RFW_IDLE;
			}
			else if( rfw_fail_counter >= 3 )	{
				rfwatch_state = FSM_RFW_PANIC;   //tirar evento
				POWER_TX_OFF();
				rfw_timer = 60*60;
				logCidEvent(account, 1, 922, 0, 0);
			}
			break;
		case FSM_RFW_PANIC :
			if( !rfw_timer )	{
				POWER_TX_ON();
				rfw_fail_counter = 0;
				rfwatch_state = FSM_RFW_IDLE;
			}
			break;
		default :
			POWER_TX_ON();
			rfw_fail_counter = 0;
			rfwatch_state = FSM_RFW_IDLE;
			break;
	}
}

void RF_Poll( void )
{
//	static uint8_t count;

	led_dcb[APER_led].led_blink = 1;
	led_dcb[APER_led].led_cad = 2*0x100+2;
	led_dcb[APER_led].led_state = LED_IDLE;

	Buzzer_dcb.led_cad = 2*0x100 + 5;
	Buzzer_dcb.led_state = LED_IDLE;
	Buzzer_dcb.led_blink = 1;

	//actualizamos el contador de polling durante el evento de Apertura
	if( Aper_Poll_counter )	{
		Aper_Poll_counter--;
	}
	if( VRST_count )	{
		VRST_count--;
	}
	if ( ( autoreset_data >= 0xE0 ) &&  ( autoreset_data <= 0xEF ) ) {
		//actualizo los contadores de autoreset, si corresponde
		if( asal_autr_counter && (autoreset_data & bitpat[ASAL_bit]) )
			asal_autr_counter--;
		if( teso_autr_counter && (autoreset_data & bitpat[TESO_bit]) )
			teso_autr_counter--;
		if( ince_autr_counter && (autoreset_data & bitpat[INCE_bit]) )
			ince_autr_counter--;
		if( rotu_autr_counter && (autoreset_data & bitpat[ROTU_bit]) )
			rotu_autr_counter--;
	}


	if( SysFlag0 & AUTORSTDATA_flag )	{
		autoreset_data = 0;
		SysFlag0 &= ~AUTORSTDATA_flag;
	}

	//reseteo el timer de preve de central
	SysFlag1 &= ~PREVE_CENTRAL_RX;
	preve_timer = TIEMPO_PREVE;			// 5 minutos de tiempo para avisar que no tiene encuesta de rf
}

uint16_t autorst_timer_min, asal_autorst_timer_min, teso_autorst_timer_min, ince_autorst_timer_min, rotu_autorst_timer_min;
uint16_t autorst_timer;
uint16_t paparst_timer;

void autoreset_logo3d( void )
{
	//si el flag de reset de alarmas esta seteado, reseteo las alarmas policiales

	if( SysFlag0 & RSTALRM_flag )	{
		if( asal_autr_counter < AUTORESET_POLL_COUNT )	{
			asal_state = AUTR_NORMAL;
			if( BaseAlarmPkt_alarm & bitpat[ASAL_bit] )	{
				BaseAlarmPkt_alarm &= ~bitpat[ASAL_bit];
				AlarmWriteHistory();
			}
			BaseAlarmPkt_alarm &= ~bitpat[ASAL_bit];
			asal_autr_counter = 0;
		}

		if( teso_autr_counter < AUTORESET_POLL_COUNT )	{
			teso_state = AUTR_NORMAL;
			if( BaseAlarmPkt_alarm & bitpat[TESO_bit] )	{
				BaseAlarmPkt_alarm &= ~bitpat[TESO_bit];
				AlarmWriteHistory();
			}
			BaseAlarmPkt_alarm &= ~bitpat[TESO_bit];
			teso_autr_counter = 0;
			BaseAlarmPkt_memoria_dispositivos = 0x00;
		}

		if( ince_autr_counter < AUTORESET_POLL_COUNT )	{
			ince_state = AUTR_NORMAL;
			if( BaseAlarmPkt_alarm & bitpat[INCE_bit] )	{
				BaseAlarmPkt_alarm &= ~bitpat[INCE_bit];
				AlarmWriteHistory();
			}
			BaseAlarmPkt_alarm &= ~bitpat[INCE_bit];
			ince_autr_counter = 0;
		}

		if( rotu_autr_counter < AUTORESET_POLL_COUNT )	{
			rotu_state = AUTR_NORMAL;
			if( BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
				BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
				AlarmWriteHistory();
			}
			BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
			rotu_autr_counter = 0;
		}

		//---------------------------------------------------
		BaseAlarmPkt_alarm &= ~bitpat[F220_bit];
		led_dcb[F220_led].led_state = LED_IDLE;
		led_dcb[F220_led].led_cad = 0;
		SysFlag0 &= ~RSTALRM_flag;	//consumo el flag
	}


	//--- para el asalto
	switch( asal_state )	{
		case AUTR_NORMAL :
			led_dcb[ASAL_led].led_state = LED_IDLE;
			led_dcb[ASAL_led].led_cad = 0;
			if( BaseAlarmPkt_alarm & bitpat[ASAL_bit] )	{
				recharge_alarm(ASAL_bit);
			}
			break;
		case AUTR_ALRMED :
			if( (asal_autr_counter == 0) && (SEC_TIMER > (asal_autr_timer + asal_autorst_timer_min)) )	{
				asal_autr_counter = 0;
				asal_state = AUTR_MEMALR;
				SysFlag2 |= NORM_ASAL;
				logCidEvent(account, 1, 885, 0, n_asal);
				n_asal = 0;
				write_nsignals();
				led_dcb[ASAL_led].led_cad = AUTORESET_BLINK_CAD;
				led_dcb[ASAL_led].led_blink = BLINK_FOREVER;
				led_dcb[ASAL_led].led_state = LED_IDLE;
				if( BaseAlarmPkt_alarm & bitpat[ASAL_bit] )	{
					BaseAlarmPkt_alarm &= ~bitpat[ASAL_bit];
					AlarmWriteHistory();
				}
            	BaseAlarmPkt_alarm &= ~bitpat[ASAL_bit];
            	SysFlag3 |= SENDM_flag;
            	set_mem_alrm(ASAL_bit);
			}
			break;
		case AUTR_MEMALR :
			if( BaseAlarmPkt_alarm & bitpat[ASAL_bit] )	{
				recharge_alarm(ASAL_bit);
			}
			break;
		default :
			asal_state = AUTR_NORMAL;
			break;
	}
	//--- para el tesoro
	switch( teso_state )	{
		case AUTR_NORMAL :
			led_dcb[TESO_led].led_state = LED_IDLE;
			led_dcb[TESO_led].led_cad = 0;
			if( BaseAlarmPkt_alarm & bitpat[TESO_bit] )	{
				recharge_alarm(TESO_bit);
			}
			break;
		case AUTR_ALRMED :
			if( (teso_autr_counter == 0) && (SEC_TIMER > (teso_autr_timer + teso_autorst_timer_min)) )	{
				teso_autr_counter = 0;
				teso_state = AUTR_MEMALR;
				SysFlag2 |= NORM_TESO;
				logCidEvent(account, 1, 887, 0, n_teso);
				n_teso = 0;
				write_nsignals();
				led_dcb[TESO_led].led_cad = AUTORESET_BLINK_CAD;
				led_dcb[TESO_led].led_blink = BLINK_FOREVER;
				led_dcb[TESO_led].led_state = LED_IDLE;
				if( BaseAlarmPkt_alarm & bitpat[TESO_bit] )	{
					BaseAlarmPkt_alarm &= ~bitpat[TESO_bit];
					AlarmWriteHistory();
				}
				BaseAlarmPkt_alarm &= ~bitpat[TESO_bit];
				SysFlag3 |= SENDM_flag;
				set_mem_alrm(TESO_bit);
			}
			break;
		case AUTR_MEMALR :
			if( BaseAlarmPkt_alarm & bitpat[TESO_bit] )	{
				recharge_alarm(TESO_bit);
			}
			break;
		default :
			teso_state = AUTR_NORMAL;
			break;
	}
	//--- para el incendio
	switch( ince_state )	{
		case AUTR_NORMAL :
			led_dcb[INCE_led].led_state = LED_IDLE;
			led_dcb[INCE_led].led_cad = 0;
			if( BaseAlarmPkt_alarm & bitpat[INCE_bit] )	{
				recharge_alarm(INCE_bit);
			}
			break;
		case AUTR_ALRMED :
			if( (ince_autr_counter == 0) && (SEC_TIMER > (ince_autr_timer + ince_autorst_timer_min)) )	{
				ince_autr_counter = 0;
				ince_state = AUTR_MEMALR;
				SysFlag2 |= NORM_INCE;
				logCidEvent(account, 1, 886, 0, n_ince);
				n_ince = 0;
				write_nsignals();
				led_dcb[INCE_led].led_cad = AUTORESET_BLINK_CAD;
				led_dcb[INCE_led].led_blink = BLINK_FOREVER;
				led_dcb[INCE_led].led_state = LED_IDLE;
				if( BaseAlarmPkt_alarm & bitpat[INCE_bit] )	{
					BaseAlarmPkt_alarm &= ~bitpat[INCE_bit];
					AlarmWriteHistory();
				}
            	BaseAlarmPkt_alarm &= ~bitpat[INCE_bit];
            	SysFlag3 |= SENDM_flag;
            	set_mem_alrm(INCE_bit);
			}
			break;
		case AUTR_MEMALR :
			if( BaseAlarmPkt_alarm & bitpat[INCE_bit] )	{
				recharge_alarm(INCE_bit);
			}
			break;
		default :
			ince_state = AUTR_NORMAL;
			break;
	}
	//--- para la rotura
	switch( rotu_state )	{
		case AUTR_NORMAL :
			led_dcb[ROTU_led].led_state = LED_IDLE;
			led_dcb[ROTU_led].led_cad = 0;
			if( BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
				BaseAlarmPkt_alarm &= ~0x80;
				rotu_state = AUTR_ALRMED;
				rotu_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						rotu_autr_counter = 3;
						rotu_autorst_timer_min = 2*60;
					} else	{
						rotu_autr_counter = 0;
						rotu_autorst_timer_min = 2*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						rotu_autr_counter = AUTORESET_POLL_COUNT;
						rotu_autorst_timer_min = autorst_timer*60;
					} else	{
						rotu_autr_counter = 0;
						rotu_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[ROTU_led].led_cad = 255*0x100 + 0;
			}
			break;
		case AUTR_ALRMED :
            if( (rotu_autr_counter == 0) && (SEC_TIMER > (rotu_autr_timer + rotu_autorst_timer_min)) && (!(Rot485_flag & ROT485_FLAG)) )	{
                rotu_autr_counter = 0;
                rotu_state = AUTR_MEMALR;
                led_dcb[ROTU_led].led_cad = AUTORESET_BLINK_CAD;
                led_dcb[ROTU_led].led_blink = BLINK_FOREVER;
                led_dcb[ROTU_led].led_state = LED_IDLE;
                if( BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
                    BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
                    AlarmWriteHistory();
                }
                BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
                SysFlag3 |= SENDM_flag;
                set_mem_alrm(ROTU_bit);
            }
			break;
		case AUTR_MEMALR :
			if( BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
				BaseAlarmPkt_alarm &= ~0x80;
				rotu_state = AUTR_ALRMED;
				rotu_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						rotu_autr_counter = 3;
						rotu_autorst_timer_min = 2*60;
					} else	{
						rotu_autr_counter = 0;
						rotu_autorst_timer_min = 2*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						rotu_autr_counter = AUTORESET_POLL_COUNT;
						rotu_autorst_timer_min = autorst_timer*60;
					} else	{
						rotu_autr_counter = 0;
						rotu_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[ROTU_led].led_cad = 255*0x100 + 0;
			}
			break;
		default :
			rotu_state = AUTR_NORMAL;
			break;
	}

}

void  AlarmDetectTask(void  *p_arg)
{
	uint8_t	i, j,tzone, temp[16], temp2[8];
	OS_ERR	os_err;
	int error;
	

	(void)p_arg;


		//inicializacion de los estados de las maquinas de deteccion
		daper_state = APER_IDLE;
		daper2_state = APER_IDLE;
		daper_stateAP = APER_IDLE;
		dasa_state = ASALTO_IDLE;
		df220_state = F220_IDLE;
		dinc_state = INCENDIO_IDLE;
		drst_state = RSTALM_IDLE;
		dteso_state = TESORO_IDLE;

		for( i = 0; i < 5; i++)
			apdisp_state[i] = TESORO_IDLE;

		for( i = 0; i < 16; i++)
			drotu_state[i] = ROTU_IDLE;

		//inicializacion de los estados de la maquina de autoreset
		asal_state = AUTR_NORMAL;
		teso_state = AUTR_NORMAL;
		ince_state = AUTR_NORMAL;
		rotu_state = AUTR_NORMAL;


		fsm_autorstd = FSM_ARSTD_IDLE;
		fsm_icmx_state = FSM_ICMX_IDLE;
		//fsm_rot485_state = FSM_ROT485_IDLE;
		csign_state = CSIGN_IDLE;

		fmode_state = FMODE_IDLE;
		fsmEvOnPreve_state = FSMEVP_IDLE;
		fevimdllave_state = FEVIMD_IDLE;

		rfw_timer = 0;
		rfw_fail_counter = 0;
		rfwatch_state = FSM_RFW_IDLE;

		ppon_state = FSM_PPON_IDLE;
		ppon_wdog_timer = SEC_TIMER;

		state_transmit_tx = FSMTX_IDLE;
		pollsig_state = FSM_PS_NOISE;

		fsm_npd_state = NPD_IDLE;

		SysFlag0 |= STARTUP_flag;
		SysFlag1 = 0x00;
		SysFlag2 = 0x00;
		DebugFlag = 0x00;

		preve_timer = TIEMPO_PREVE;

		AlarmReadHistory();
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		WDT_Feed();

		error = flash0_read(temp, DF_PARTITION_OFFSET, 5);
		for(i = 0; i < 5; i++)	{
			if(temp[i] <= 0x99)	{
				PT_estado_particion[i] = temp[i];
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		WDT_Feed();

		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
		BaseAlarm_memoria_dispositivos_1 = BaseAlarmPkt_memoria_dispositivos;

		//cargo el estado de las maquinas segun ultima memoria de alarmas
		if(BaseAlarmPkt_alarm & bitpat[INCE_bit] )
			dinc_state = INCENDIO_ALRM;
		if(BaseAlarmPkt_alarm & bitpat[ASAL_bit] )
			dasa_state = ASALTO_ALRM;
		if(BaseAlarmPkt_alarm & bitpat[TESO_bit] )
			dteso_state = TESORO_ALRM;
		if(BaseAlarmPkt_alarm & bitpat[ROTU_bit] )	{
			for( i = 0; i < 8; i++)
					drotu_state[i] = ROTU_IDLE;
		}

		BaseZone = 8;

		//aca leer ultima alarma desde dataflash
		//--------------------------------------

		for( i = 0; i < 8; i++ )	{
				led_dcb[i].led_cad = 3*0x100+3;
		}

	account = EepromReadWord(CIDACCOUNT1_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();

	r3kaccount = EepromReadWord(R3KACCOUNT_E2P_ADDR, &error);
	Get_evwrptr();

#ifdef TABLAS_ABO_ON
	// Aca impactar con la tabla de conversion de nros de aboando
	i = (uint8_t)EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
	tzone = (uint8_t)EepromReadByte(ZONE_E2P_ADDR, &error);
	zonenumber = tzone;
	if( (i >= 1) && (i <= 200) && (tzone >= 1) && (tzone <= 6) )	{
		switch(tzone)	{
		case 1:
			BaseAlarmPkt_numabo = numabo_z1[i-1];
			break;
		case 2:
			BaseAlarmPkt_numabo = numabo_z2[i-1];
			break;
		case 3:
			BaseAlarmPkt_numabo = numabo_z3[i-1];
			break;
		case 4:
			BaseAlarmPkt_numabo = numabo_z4[i-1];
			break;
		case 5:
			BaseAlarmPkt_numabo = numabo_z5[i-1];
			break;
		case 6:
			BaseAlarmPkt_numabo = numabo_z6[i-1];
			break;
		}
	} else	{
		BaseAlarmPkt_numabo = 200;
	}
	
#else
	BaseAlarmPkt_numabo = (uint8_t)EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
#endif

	ptxormask = (uint8_t)EepromReadByte(PTXORMASK_E2P_ADDR, &error);
	ptxormask &= 0xF8;

	TypeAboAns = (uint8_t)EepromReadByte(RF_ANSWER_TYPE, &error);
	i = (uint8_t)EepromReadByte(FMODEON_ADDR, &error);
	FmodeDelay = (uint8_t)EepromReadByte(FMODELAY_ADDR, &error);
	if((i == 0x5A) || (i == 0x01))
		SysFlag3 |= FMODE_flag;
	else
		SysFlag3 &= ~FMODE_flag;

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	if(((TypeAboAns == 1) || (TypeAboAns == 2)) && (SysFlag3 & FMODE_flag) )	{
		EepromWriteByte(FMODEON_ADDR, (uint8_t)0x00, &error);
		SysFlag3 &= ~FMODE_flag;
	}

	OSTimeDlyHMSM(0, 0, TIME_STARTUP_DETECT, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);

		SysFlag0 &= ~STARTUP_flag;
		//inicializacion de dcb de leds basicos
		led_dcb[NORMAL_led].led_mask = NORMAL_led_mask;
		led_dcb[APER_led].led_mask = APER_led_mask;
		led_dcb[LLOP_led].led_mask = LLOP_led_mask;
		led_dcb[F220_led].led_mask = F220_led_mask;
		led_dcb[ROTU_led].led_mask = ROTU_led_mask;
		led_dcb[TESO_led].led_mask = TESO_led_mask;
		led_dcb[INCE_led].led_mask = INCE_led_mask;
		led_dcb[ASAL_led].led_mask = ASAL_led_mask;
		for( i = 4; i < 8; i++ )	{
			led_dcb[i].led_state = LED_IDLE;
			led_dcb[i].led_cad = 0x0000;
			led_dcb[i].led_tim_off = 0x0000;
			led_dcb[i].led_tim_on = 0x0000;
			led_dcb[i].led_blink = 0;
		}
//		bledShadow = 0x00;

		paptimerslot = SEC_TIMER;
		fsm_pap_state = FSMPAP_IDLE;
		if((TypeAboAns == 5) || (TypeAboAns == 6))	{
			error = InitCMX869_PAP();
		}

		oldptalarm = 0x00;


	//chequeo al inicio, si hay diferencias en el serial de la flash y de la eprom
	flash0_read(temp, DF_SERIALNUM_OFFSET, 8);
	OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
    EepromReadBuffer(SERIALNUM_E2P_ADDR, temp2, 8, &error);
    OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
    for(i = 0; i < 8; i++)	{
    	if(temp[i] != temp2[i])	{
    		logCidEvent(account, 1, 912, 0, 0);
    		break;
    	}
    }

    fsm_rotrele485_state = FSM_ROT485_IDLE;

    flash0_read(temp, DF_RFFILTER1_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SysFlag4 |= RFFILTER1;
    } else  {
        SysFlag4 &= ~RFFILTER1;
    }
    flash0_read(temp, DF_RFFILTER2_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SysFlag4 |= RFFILTER2;
    } else  {
        SysFlag4 &= ~RFFILTER2;
    }

	while(DEF_ON)	{
		WDT_Feed();
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);		//previo timeinterval = 5
		UDP_cmd_server();
		fsm_init_cmx_preve();
        refresh_alrm_led();

		//fsm_txoff();
		//fsm_txoff2();

		if(modem_error)	{
			modem_error = 0;
			if((TypeAboAns == 5) || (TypeAboAns == 6))	{
				error = InitCMX869_PAP();
			} else	{
				error = InitCMX869();
			}
		}

		//---------------------------------------------------------------------------------
		// Si soy PAP ejecuto su fsm
		if(paptslot > 0)	{
			fsm_pap();
		}

		//reporto cualquier problem de comunicacion en la 485 con rotura a policia
		Rot485_flag &= ~ROT485_FLAG;
        Rot485_flag &= ~ROT485RELE85_FLAG;
		for( i = 0; i < MAXQTYPTM; i++ )	{
			if(ptm_dcb[i].rtuaddr == 0x00)
				continue;
			if(ptm_dcb[i].flags & COMM_TROUBLE )	{
				if(!((ptm_dcb[i].rtuaddr >= 230) && (ptm_dcb[i].rtuaddr < 236)))	{
					Rot485_flag |= ROT485_FLAG;
				} else  {
                    Rot485_flag |= ROT485RELE85_FLAG;
				}
				//break;
			}
		}
        fsm_roturaRele485();
		fsm_rotura485();
		fsm_roturaEVO();
		rf_cortex_signature();
		//deteccion de bateria baja
		if(bat_med1 <= 3240)	{
			SystemFlag4 |= LOWBATT_flag;
		} else
		if(bat_med1 >= 3370)	{
			SystemFlag4 &= ~LOWBATT_flag;
		}
		fsm_deteccion_lowbattery();
		if(SystemFlag4 & NPMED_FLAG)
			fsm_deteccion_NP_umbral();

//		if(!(SysFlag_AP_zvolt & AP_ZVOLT_MEAS_flag))	{
			DeteccionEventos();
			fsm_deteccion_incendio();
			fsm_deteccion_asalto();
			fsm_deteccion_tesoro();
			fsm_deteccion_rotura();
			
			if( SysFlag0 & TIMROT_flag )	{
				SysFlag0 &= ~TIMROT_flag;
				//fsm_deteccion_rotura();
			}
			fsm_deteccion_apertura();
			if(TasFlags & TASMODE_FLAG)	{
				fsm_TAS();
			} else	{
				fsm_deteccion_apertura2();
			}
			fsm_deteccion_aperturaAP();
			fsm_deteccion_f220();
			if( BaseZone > 3 )
				fsm_deteccion_dispositivos();


			autoreset_logo3d();
			
			if(paptslot == 0)	{
				fsm_rfwatchdog();
				//fsm_ppon_wdog();
			}

			if(SystemFlag6 & USE_RADAR_FLAG)
				fsm_AperMagRadar();

			BaseAlarmPkt_memoria_dispositivos = OptoInputs;
			//si corresponde prendo el led de normal
			if( !(BaseAlarmPkt_alarm & 0x7F) )	{
				BaseAlarmPkt_alarm |= 0x80;
				if( !led_dcb[NORMAL_led].led_blink )	{
					led_dcb[NORMAL_led].led_cad = 255*0x100 + 0;
				}
			} else	{
				BaseAlarmPkt_alarm &= ~0x80;
				if( !led_dcb[NORMAL_led].led_blink )	{
					led_dcb[NORMAL_led].led_cad = 0;
					led_dcb[NORMAL_led].led_state = LED_IDLE;
				}
			}
//		}
//		if( SysFlag3 & FMODE_flag)
//			fsm_fastmode();
	}
}

void fsm_roturaRele485( void ) {
    struct tm currtime;

    switch (fsm_rotrele485_state)   {
        case FSM_ROT485_IDLE:
            if(Rot485_flag & ROT485RELE85_FLAG) {
                fsm_rotrele485_state = FSM_ROT485_WAIT;
                rotrele485timer = SEC_TIMER;
            }
            break;
        case FSM_ROT485_WAIT:
            if( !(Rot485_flag & ROT485RELE85_FLAG) )	{
                fsm_rotrele485_state = FSM_ROT485_IDLE;
            } else
            if(SEC_TIMER > rotrele485timer + 90)	{
                fsm_rotrele485_state = FSM_ROT485_ROT;
                logCidEvent(account, 1, 385, 58, 0);
            }
            break;
        case FSM_ROT485_ROT:
            if( !(Rot485_flag & ROT485RELE85_FLAG) )	{
                fsm_rotrele485_state = FSM_ROT485_IDLE;
                logCidEvent(account, 1, 385, 58, 0);
            } else	{
                currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
                currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
                currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
                if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 0) && (!(SystemFlag3 & RS485RELE_DONE)))	{
                    SystemFlag3 |= RS485RELE_DONE;
                    logCidEvent(account, 3, 385, 58, 0);
                }
                if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 10) )	{
                    SystemFlag3 &= ~RS485RELE_DONE;
                }
            }
            break;
        default:
            fsm_rotrele485_state = FSM_ROT485_IDLE;
            SystemFlag3 &= ~RS485RELE_DONE;
            break;
    }
}


void fsm_rotura485( void )
{
	struct tm currtime;

	int version_number = VERSION_NUMBER;
	switch(fsm_rot485_state)	{
	case FSM_ROT485_IDLE:
		if( (Rot485_flag & ROT485_FLAG) && (version_number != 111) )	{
			fsm_rot485_state = FSM_ROT485_WAIT;
			rot485timer = SEC_TIMER;
		}
		break;
	case FSM_ROT485_WAIT:
		if( !(Rot485_flag & ROT485_FLAG) )	{
			fsm_rot485_state = FSM_ROT485_IDLE;
		} else
		if(SEC_TIMER > rot485timer + 90)	{
			fsm_rot485_state = FSM_ROT485_ROT;
			SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
			SystemFlag3 |= NAPER_flag;
			SystemFlag3 |= NAPER_F220V;
			PTM485NG_HistoryWrite();
		}
		break;
    case FSM_ROT485_ROT:
		if( !(Rot485_flag & ROT485_FLAG) )	{
			fsm_rot485_state = FSM_ROT485_IDLE;
			PTM485NG_HistoryWrite();
		} else	{
			SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
			BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
			currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
			currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
			currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
			if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 0) && (!(SystemFlag4 & RS485F220_DONE)))	{
				SystemFlag4 |= RS485F220_DONE;
				SystemFlag3 |= NAPER_flag;
			}
			if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 10) )	{
				SystemFlag4 &= ~RS485F220_DONE;
			}
		}
//		if( (!(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) ) && (!(SysFlag_AP_GenAlarm & bitpat[ROTU_bit])))	{
//			fsm_rot485_state = FSM_ROT485_WAIT;
//			rot485timer = SEC_TIMER;
//		}
		break;
	default:
		fsm_rot485_state = FSM_ROT485_IDLE;
		break;
	}
}

void fsm_roturaEVO( void )
{
	struct tm currtime;

	switch(fsm_rotEVO_state)	{
	case FSM_ROT485_IDLE:
		if( Rot485_flag & ROTEVO_FLAG )	{
			fsm_rotEVO_state = FSM_ROT485_WAIT;
			rotEVOtimer = SEC_TIMER;
		}
		break;
	case FSM_ROT485_WAIT:
		if( !(Rot485_flag & ROTEVO_FLAG) )	{
			fsm_rotEVO_state = FSM_ROT485_IDLE;
		} else
		if(SEC_TIMER > rotEVOtimer + 180)	{
			fsm_rotEVO_state = FSM_ROT485_ROT;
			SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
			SystemFlag3 |= NAPER_flag;
			SystemFlag3 |= NAPER_F220V;
		}
		break;
	case FSM_ROT485_ROT:
		if( !(Rot485_flag & ROTEVO_FLAG) )	{
			fsm_rotEVO_state = FSM_ROT485_IDLE;
		} else	{
			SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
			BaseAlarmPkt_alarm |= bitpat[ROTU_bit];
			if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 0) && (!(SystemFlag4 & RS485F220_DONE)))	{
				SystemFlag4 |= RS485F220_DONE;
				SystemFlag3 |= NAPER_flag;
			}
			if((currtime.tm_hour == 22) && (currtime.tm_min == 15) && (currtime.tm_sec == 10) )	{
				SystemFlag4 &= ~RS485F220_DONE;
			}
		}
//		if( (!(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) ) && (!(SysFlag_AP_GenAlarm & bitpat[ROTU_bit])))	{
//			fsm_rotEVO_state = FSM_ROT485_WAIT;
//			rotEVOtimer = SEC_TIMER;
//		}
		break;
	default:
		fsm_rotEVO_state = FSM_ROT485_IDLE;
		break;
	}
}

int rxabonum_prev(uint8_t rxchar, int delta)
{
    int rnumabo, abomin;

    rnumabo = BaseAlarmPkt_numabo;

    if(!(SysFlag4 & RFFILTER1))    {
        return TRUE;
    }

    if((zonenumber != 0) && (zonenumber != 1))  {
        return TRUE;
    }

    abomin = rnumabo - delta;

    if(abomin >= 0) {
        if((rxchar >= abomin) && (rxchar <= (rnumabo-1)))    {
            return TRUE;
        } else  {
            return FALSE;
        }
    } else  {
        if((rxchar >= 0) && (rxchar <= (rnumabo-1)))    {
            return TRUE;
        } else {
            abomin = 150 - (delta - rnumabo);
            if((rxchar >= abomin) && (rxchar < 150))    {
                return TRUE;
            } else  {
                return FALSE;
            }
        }
    }
}

int IsWrightTimePoll( void )
{
    static time_t polltime0, polltime1, polltime2, polltime3;
    uint32_t deltat1, deltat2, deltat3;

    if(!(SysFlag4 & RFFILTER2))    {
        return TRUE;
    }

    if((zonenumber != 0) && (zonenumber != 1))  {
        return TRUE;
    }

    polltime3 = polltime2;
    polltime2 = polltime1;
    polltime1 = polltime0;
    polltime0 = MSEC_TIMER;

    deltat3 = polltime3 - polltime2;
    deltat2 = polltime2 - polltime1;
    deltat1 = polltime1 - polltime0;

    if(deltat1 > 50*1000)   {
        return TRUE;
    } else  {
        if((deltat3 < 2000) && (deltat2 < 2000) && (deltat1 < 2000))    {
            return TRUE;
        } else  {
            return FALSE;
        }
    }
}
//*****************************************************************************
//* Submodulo para el manejo del modem y el enlace de rf
//*
void MDM_IrqHandler( void )
{
	uint16_t status, Control;
	uint8_t rxchar, i;
	static uint8_t rxchar_m0, rxchar_m1, rxchar_m2;
	static uint8_t count;
    static time_t prevetimeout, t1, t2;
    time_t delta_t;
    static uint8_t startup = 0;

	
	EXTI_ClearEXTIFlag(1);
	
	if(SysFlag0 & STARTUP_flag)	{
		return;
	}

//	if(!(SystemFlag5 & SERIALNUM_OK))	{
//		return;
//	}


	timerIrqCMX = 30;
	status = rd16_cbus(CMXSTAT_ADDR);

	rxchar = rd8_cbus(CMXRXDATA_ADDR);
	
	
	if((status & 0x0010) && (status & 0x0020))	{
		for(i = 0; i < 5; i++ )	{
			if((status & 0x0010) && (status & 0x0020))	{
				status = rd16_cbus(CMXSTAT_ADDR);
				rxchar = rd8_cbus(CMXRXDATA_ADDR);
			} else
				break;
		}
		if( i == 5 )
			modem_error = 1;
		isrclear = 0;
		return;
	}

	rxchar_m2 = rxchar_m1;
	rxchar_m1 = rxchar_m0;
	rxchar_m0 = rxchar;


	if( (TypeAboAns >= 1) && (TypeAboAns <= 4) && (SystemFlag7 & INPATT_CHECK))	{
		PollingSignature(rxchar);
	} else {
        SysFlag0 |= INPATTERN_flag;
    }


	if(DebugFlag & DBGRF_ON_flag)	{
		//CommPutChar(DEBUG_COMM,'{',0);
		printByte(rxchar);
		//CommPutChar(DEBUG_COMM,'}',0);
		CommPutChar(DEBUG_COMM,' ',0);
	}

	if(startup == 0)    {
	    startup = 1;
	    t1 = MSEC_TIMER;
	    t2 = MSEC_TIMER;
	} else
    if( (rxchar == BaseAlarmPkt_numabo) && (rxchar_m1 >= 0xE0) && (rxchar_m1 <= 0xFF) && \
        (rxchar_m2 >= 0x00) && (rxchar_m2 <= 149) )     {
        t1 = t2;
        t2 = MSEC_TIMER;
        delta_t = t2 - t1;
    }
//--------------------------------------------------------------------------
// aca procesamos el codigo de autoreset para deteccion de preves de tx
    if( (rxchar == (BaseAlarmPkt_numabo + 1)) && (SysFlag4 & ABONUMBER_flag) )	{
        SysFlag1 |= ABOMASUNO_flag;
        SysFlag4 &= ~ABONUMBER_flag;
    } else if( SysFlag1 & ABOMASUNO_flag ) {
        SysFlag1 &= ~ABOMASUNO_flag;
        if((rxchar >= 0xE0) && (rxchar <= 0xFE))	{
            SysFlag3 |= VALIDRXCHAR_flag;
            if(DebugFlag & DBGABORF_flag)	{
                CommPutChar(DEBUG_COMM,'-',0);
                printByte(rxchar);
                CommPutChar(DEBUG_COMM,'-',0);
                CommPutChar(DEBUG_COMM,'\n',0);
                CommPutChar(DEBUG_COMM,'\r',0);
            }
            //- - - - - - - - - - - - - - - - - - - - -
            //aca llego un cogigo de autoreset valido
            {
                led_dcb[NORMAL_led].led_blink = 2;
                led_dcb[NORMAL_led].led_cad = 1*0x100+1;
                led_dcb[NORMAL_led].led_state = LED_IDLE;

                if( OptoInputs == 0x00)	{
                    Buzzer_dcb.led_cad = 5*0x100 + 5;
                    Buzzer_dcb.led_state = LED_IDLE;
                    Buzzer_dcb.led_blink = 1;
                }

                //--------------------------------------------------------------------
                // Manejo los asuntos de autoreset y autoreset data
                autoreset_data = rxchar;
                SysFlag0 |= AUTORSTDATA_flag;
                // Manejo el autoreset data
                if ( ( autoreset_data >= 0xE0 ) &&  ( autoreset_data <= 0xEF ) ) {
                    //actualizo los contadores de autoreset, si corresponde
                    if( asal_autr_counter && (autoreset_data & bitpat[ASAL_bit]) )
                        asal_autr_counter--;
                    if( teso_autr_counter && (autoreset_data & bitpat[TESO_bit]) )
                        teso_autr_counter--;
                    if( ince_autr_counter && (autoreset_data & bitpat[INCE_bit]) )
                        ince_autr_counter--;
                    if( rotu_autr_counter && (autoreset_data & bitpat[ROTU_bit]) )
                        rotu_autr_counter--;
                }
                //-------------------------------------------------------------------
                // Ponemos la maquinita para evitar falsas preves de TX
                switch(fsm_autorstd)	{
                    case FSM_ARSTD_IDLE :
                        SysFlag1 &= ~PREVE_CENTRAL_TX;
                        if(autoreset_data == 0xFE)	{
                            prevetimeout = SEC_TIMER + 15*60;
                            count = 1;
                            fsm_autorstd = FSM_ARSTD_WAIT;
                        }
                        break;
                    case FSM_ARSTD_WAIT :
                        SysFlag1 &= ~PREVE_CENTRAL_TX;
                        //if(autoreset_data != 0xFE)	{
                        if(SEC_TIMER > prevetimeout)    {
                            count = 0;
                            fsm_autorstd = FSM_ARSTD_IDLE;
                        } else if(autoreset_data == 0xFE)	{
                            count++;
                            if(count >= 5)	{
                                SysFlag1 |= PREVE_CENTRAL_TX;
                                fsm_autorstd = FSM_ARSTD_PREVETX;
                                count = 0;
                            }
                        }
                        break;
                    case FSM_ARSTD_PREVETX :
                        SysFlag1 |= PREVE_CENTRAL_TX;
                        if((autoreset_data >= 0xE0) && (autoreset_data < 0xF0))	{
                            if( count == 0 )
                                prevetimeout = SEC_TIMER + 15*60;
                            count++;
                            if(count >= 2)  {
                                fsm_autorstd = FSM_ARSTD_IDLE;
                                SysFlag1 &= ~PREVE_CENTRAL_TX;
                            }
                            if(SEC_TIMER > prevetimeout)
                                count = 0;

                        }
                        break;
                    default:
                        fsm_autorstd = FSM_ARSTD_IDLE;
                        SysFlag1 &= ~PREVE_CENTRAL_TX;
                        break;
                }
                autoreset_data = 0;
                //-------------------------------------------------------------------

            }
            //- - - - - - - - - - - - - - - - - - - - -
        }

    } else

//--------------------------------------------------------------------------

	if( (!(SysFlag0 & FSMTX_flag)) && (SysFlag0 & INPATTERN_flag))	{
		if((rxchar > 0) && (rxchar < 203))	{
			SysFlag3 |= VALIDRXCHAR_flag;
		}
		if( (rxchar == BaseAlarmPkt_numabo) && (delta_t > 80) && (rxchar_m1 >= 0xE0) && (rxchar_m1 <= 0xFF) && \
		    (rxchar_m2 >= 0x00) && (rxchar_m2 <= 149)   && \
		    rxabonum_prev(rxchar_m2, 20) && IsWrightTimePoll())	{

            SysFlag4 |= ABONUMBER_flag;
			if(DebugFlag & DBGABORF_flag)	{
				CommPutChar(DEBUG_COMM,'{',0);
				printByte(rxchar);
				CommPutChar(DEBUG_COMM,'}',0);
				CommPutChar(DEBUG_COMM,' ',0);
			}
			//---------------------------------------------------------------------
			//lo que haya que hacer cuando llega la encuesta a este abonado
			SysFlag0 |= FSMTX_flag;
			SysFlag3 &= ~SEND_flag;
			//SysFlag0 |= RF_POLL_flag;
			SysFlag3 |= FRFPOLL_flag;
			SysFlag1 &= ~PREVE_CENTRAL_RX;
			preve_timer = TIEMPO_PREVE;

			
			//aviso el polling destellando una vez el led de apertura
			led_dcb[APER_led].led_blink = 1;
			led_dcb[APER_led].led_cad = 1*0x100+1;
			led_dcb[APER_led].led_state = LED_IDLE;

			if( OptoInputs == 0x00)	{
				Buzzer_dcb.led_cad = 1*0x100 + 1;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = 1;
			}

			
			//actualizamos el contador de polling durante el evento de Apertura
			if( Aper_Poll_counter )	{
				Aper_Poll_counter--;
			}
			if( VRST_count )	{
				VRST_count--;
			}
			//---------------------------------------------------------------------
		} else if( rxchar == NABO2_STOP_TXON )	{
            SysFlag4 &= ~ABONUMBER_flag;
			LLAVE_TX_OFF();
			SysFlag3 &= ~SEND_flag;
		} else if( rxchar == NABO1_STOP_TXON )	{
            SysFlag4 &= ~ABONUMBER_flag;
			if( SysFlag3 & SEND_flag )	{
				Control = rd16_cbus( CMXSTAT_ADDR );
				if (Control & 0x1000)	{
					wr8_cbus( CMXTXDATA_ADDR, BaseAlarmPkt_numabo);
				}
			}
			//SysFlag3 |= RX201_flag;
		} else if((SysFlag4 & ABONUMBER_flag) && (rxchar < 0xE0))  {
            SysFlag4 &= ~ABONUMBER_flag;
		}
	}

	for( i = 0; i < 10; i++ )	{
		status = rd16_cbus(CMXSTAT_ADDR);
		if( status & 0x8000 )
			rxchar = rd8_cbus(CMXRXDATA_ADDR);
		else
			break;
	}
	
	isrclear = 0;
}

void PollingSignature( uint8_t rxchar )
{

	if(SystemFlag4 & NOTUSESIGNATURE)	{
		SysFlag0 |= INPATTERN_flag;
		return;
	} else	{

		switch( pollsig_state )	{
			case FSM_PS_NOISE :
				SysFlag0 &= ~INPATTERN_flag;
				if( rxchar <= 149 )	{
					pollsig_state = FSM_PS_ABOSYNC;
				}
				break;
			case FSM_PS_ABOSYNC :
				if( (rxchar >= 0xE0) && (rxchar <= 0xFF) )	{
					SysFlag0 |= INPATTERN_flag;
					pollsig_state = FSM_PS_DARST;
				}
				break;
			case FSM_PS_DARST :
				if( rxchar > 149 )	{
					SysFlag0 &= ~INPATTERN_flag;
					pollsig_state = FSM_PS_NOISE;
				} else if( rxchar <= 149 )	{
					SysFlag0 |= INPATTERN_flag;
					pollsig_state = FSM_PS_NABO;
				}
				break;
			case FSM_PS_NABO :
				if( rxchar >= 0xE0 )	{
					SysFlag0 |= INPATTERN_flag;
					pollsig_state = FSM_PS_DARST;
				} else if( rxchar < 0xE0 )	{
					SysFlag0 &= ~INPATTERN_flag;
					pollsig_state = FSM_PS_NOISE;
				}
				break;
			default :
				pollsig_state = FSM_PS_NOISE;
				SysFlag0 &= ~INPATTERN_flag;
				break;
		}
	}
}


//**************************************************************************************
//* implementa la fsm de transmision por rf cuando encuestan al abonado


void fsm_transmit_cmx( void )
{
	uint16_t Control;

//	if( SysFlag4 & LOGICALPWRTXOFF )
//		return;

	switch (state_transmit_tx)
	{
		case FSMTX_IDLE:
			if ( SysFlag0 & FSMTX_flag) {
                POWER_TX_ON();
				LLAVE_TX_ON();
                preve_timer = TIEMPO_PREVE;
                SysFlag1 &= ~PREVE_CENTRAL_RX;

				fsmtx_timer1 = 33;							//60
				acumu_buffer_sended=0;
				state_transmit_tx = FSMTX_WAIT1;

				load_buffer_tx();
			}
			break;
		case FSMTX_WAIT1:
			if(!fsmtx_timer1 )	{
				state_transmit_tx = FSMTX_TXING;
				fsmtx_timer1 = 43;							//43; se esta reciviendo el 2do byte de la encuesta, info autoreset data
			}
			break;
		case FSMTX_TXING:
			Control = rd16_cbus( CMXSTAT_ADDR );
			if ((Control & 0x1000) && (!fsmtx_timer1))	{	//control de estado de la flag TX
				if(acumu_buffer_sended == acumu_buffer_tx )	{
					SysFlag0 &= ~FSMTX_flag;
				}
				if(acumu_buffer_sended < (acumu_buffer_tx + 1) ) {
					wr8_cbus( CMXTXDATA_ADDR, buffer_tx[acumu_buffer_sended]);
					if(DebugFlag & DBGABORF_flag)	{
						CommPutChar(DEBUG_COMM,'[',0);
						printByte(buffer_tx[acumu_buffer_sended]);
						CommPutChar(DEBUG_COMM,']',0);
					}
					acumu_buffer_sended++;
					fsmtx_timer1 = 43;						//40 mseg para transmision de un char a 300baudios
				}	else {
					state_transmit_tx = FSMTX_WAIT2;
					acumu_buffer_sended = 0;
					fsmtx_timer1 = 10;						//10
				}
			}
			break;
		case FSMTX_WAIT2:
			if( !fsmtx_timer1 ) {
				state_transmit_tx = FSMTX_IDLE;
				LLAVE_TX_OFF();
				SystemFlag3 |= NAPER_RFPOLL;
			}	else if( SysFlag0 & FSMTX_flag ) {
				state_transmit_tx = FSMTX_TXING;
				fsmtx_timer1 = 43;							//40
			}
			break;
		default :
			state_transmit_tx = FSMTX_IDLE;
			break;
	}
}

void load_buffer_tx(void)
{
	uint8_t check_sum;
	int error;

//	if( BaseAlarmPkt_numabo >= 20 )
//		TypeAboAns = 2;				  //2
//	else TypeAboAns = 1;
	if( TypeAboAns > 6)	{
		TypeAboAns = (uint8_t)EepromReadByte(RF_ANSWER_TYPE, &error);
	}
	acumu_buffer_tx = 0;
	switch (TypeAboAns)
	{
		case 1:
			if (BaseAlarmPkt_alarm != 0x80)	{
				buffer_tx[acumu_buffer_tx] = ~BaseAlarmPkt_numabo;	// lleno el buffer de envio con n� de abonado
				check_sum = ~BaseAlarmPkt_numabo;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; 	// agrego al buffer de envio con las alarmas
				check_sum += BaseAlarmPkt_alarm;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = check_sum;
			}	else	{
				buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_numabo;	// lleno el buffer de envio con n� de abonado
			}
			break;
		case 2:
			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_numabo;// lleno el buffer de envio con el n� de abonado
			check_sum = BaseAlarmPkt_numabo;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = 0xff;// lleno el buffer de envio con 0xff
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; // agrego al buffer de envio con las alarmas
			check_sum += BaseAlarmPkt_alarm;
			acumu_buffer_tx++;
			//---
//			buffer_tx[acumu_buffer_tx]=(BaseAlarmPkt_estado_dispositivos & 0xF8);
//			check_sum += BaseAlarmPkt_estado_dispositivos;
//			acumu_buffer_tx++;
			//---
			buffer_tx[acumu_buffer_tx]=(BaseAlarmPkt_estado_dispositivos & 0xF8);
			buffer_tx[acumu_buffer_tx] |= 0x06;
			check_sum += buffer_tx[acumu_buffer_tx];
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx]= BaseAlarmPkt_memoria_dispositivos;
			check_sum +=  BaseAlarmPkt_memoria_dispositivos;
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx]=check_sum;
			break;
		case 3:
			if (BaseAlarmPkt_alarm != 0x80)	{
				buffer_tx[acumu_buffer_tx] = 0xE0;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = ~BaseAlarmPkt_numabo;	// lleno el buffer de envio con n� de abonado
				check_sum = ~BaseAlarmPkt_numabo;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; 	// agrego al buffer de envio con las alarmas
				check_sum += BaseAlarmPkt_alarm;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = check_sum;
			}	else	{
				buffer_tx[acumu_buffer_tx] = 0xE0;
				acumu_buffer_tx++;
				buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_numabo;	// lleno el buffer de envio con n� de abonado
			}
			break;
		case 4:
			buffer_tx[acumu_buffer_tx] = 0xE0;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_numabo;// lleno el buffer de envio con el n� de abonado
			check_sum = BaseAlarmPkt_numabo;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = 0xff;// lleno el buffer de envio con 0xff
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; // agrego al buffer de envio con las alarmas
			check_sum += BaseAlarmPkt_alarm;
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx]=(BaseAlarmPkt_estado_dispositivos & 0xF8);
			buffer_tx[acumu_buffer_tx] |= 0x06;
			check_sum += buffer_tx[acumu_buffer_tx];
			acumu_buffer_tx++;

			buffer_tx[acumu_buffer_tx]= BaseAlarmPkt_memoria_dispositivos;
			check_sum +=  BaseAlarmPkt_memoria_dispositivos;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx]=check_sum;
			break;
		case 5:
			check_sum = 0x00;
			buffer_tx[acumu_buffer_tx] = 0xAA;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; // agrego al buffer de envio con las alarmas
			check_sum += BaseAlarmPkt_alarm;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = 0x00;
			check_sum += 0x00;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx]=check_sum;
			break;
		case 6:
			check_sum = 0x00;
			buffer_tx[acumu_buffer_tx] = 0xAC;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = BaseAlarmPkt_alarm; // agrego al buffer de envio con las alarmas
			check_sum += BaseAlarmPkt_alarm;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx] = 0x00;
			check_sum += 0x00;
			acumu_buffer_tx++;
			buffer_tx[acumu_buffer_tx]=check_sum;
			break;
		default:
			break;
	}
}

void AlarmWriteHistory(void)
{
	uint32_t len;
	uint8_t mybuffer[8], temp[8], i;

	mybuffer[0] = BaseAlarmPkt_numabo;
	mybuffer[1] = BaseAlarmPkt_alarm;
	mybuffer[2] = BaseAlarmPkt_estado_dispositivos;
	mybuffer[3] = BaseAlarmPkt_memoria_dispositivos;
	mybuffer[4] = epreve_state;
	mybuffer[5] = SysFlag1;
	mybuffer[6] = 0xAA;
	mybuffer[7] = 0x55;

	len = flash0_write(1, mybuffer, DF_ALARMHISTORY_OFFSET, 8);

	len = flash0_read(temp, DF_ALARMHISTORY_OFFSET, 8);
	for( i = 0; i < 8; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_write(1, mybuffer, DF_ALARMHISTORY_OFFSET, 8);
			break;
		}
	}
}

void PTM485NG_HistoryWrite(void)
{
	uint8_t mybuffer[MAXQTYPTM + 3], temp[MAXQTYPTM + 3], i;
	int ptindex;
	uint32_t len;

	for( ptindex = 0; ptindex < MAXQTYPTM; ptindex++)	{
		mybuffer[ptindex] = ptm_dcb[ptindex].state485;
	}
	mybuffer[MAXQTYPTM] = fsm_rot485_state;
	mybuffer[MAXQTYPTM + 1] = 0xAA;
	mybuffer[MAXQTYPTM + 2] = 0x55;

	len = flash0_write(1, mybuffer, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);

	len = flash0_read(temp, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);
	for( i = 0; i < MAXQTYPTM + 3; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_write(1, mybuffer, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);
			break;
		}
	}
}

void PTM485NG_HistoryRead(void)
{
	uint8_t mybuffer[MAXQTYPTM + 3], temp[MAXQTYPTM + 3], i;
	int ptindex;
	uint32_t len;

	len = flash0_read(mybuffer, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);

	len = flash0_read(temp, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);

	for( i = 0; i < MAXQTYPTM + 3; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_read(mybuffer, DF_PTM485NG_OFFSET, MAXQTYPTM + 3);
			break;
		}
	}

	if((mybuffer[MAXQTYPTM + 1] == 0xAA) && (mybuffer[MAXQTYPTM + 2] == 0x55))	{
		for( ptindex = 0; ptindex < MAXQTYPTM; ptindex++)	{
			if(mybuffer[ptindex] == P485_NG)	{
				ptm_dcb[ptindex].state485 = P485_NG;
				ptm_dcb[ptindex].com_error_counter = 254;
				ptm_dcb[ptindex].flags |= COMM_TROUBLE;
			} else	{
				ptm_dcb[ptindex].state485 = P485_IDLE;
				ptm_dcb[ptindex].flags &= ~COMM_TROUBLE;
				ptm_dcb[ptindex].com_error_counter = 0;
			}
		}
		if( mybuffer[MAXQTYPTM] == FSM_ROT485_ROT )	{
			fsm_rot485_state = FSM_ROT485_ROT;
			Rot485_flag |= ROT485_FLAG;
		}
	}
}

void AlarmReadHistory(void)
{
	uint32_t len, error;
	uint8_t mybuffer[10], temp[8],i;


	//manejo primero la memoria de alarma
	error = flash0_read(temp, MEM_ALARM, 1);
	if(((temp[0] & 0xF0) == 0x50) && (!(SystemFlag & PORRESET_FLAG )))	{
		if(temp[0] & 0x01)	{		//memoria de alarma de asalto
			asal_state = AUTR_MEMALR;
			led_dcb[ASAL_led].led_cad = AUTORESET_BLINK_CAD;
			led_dcb[ASAL_led].led_blink = BLINK_FOREVER;
			led_dcb[ASAL_led].led_state = LED_IDLE;
		}
		if(temp[0] & 0x02)	{		//memoria de alarma de incendio
			ince_state = AUTR_MEMALR;
			led_dcb[INCE_led].led_cad = AUTORESET_BLINK_CAD;
			led_dcb[INCE_led].led_blink = BLINK_FOREVER;
			led_dcb[INCE_led].led_state = LED_IDLE;
		}
		if(temp[0] & 0x04)	{		//memoria de alarma de tesoro
			teso_state = AUTR_MEMALR;
			led_dcb[TESO_led].led_cad = AUTORESET_BLINK_CAD;
			led_dcb[TESO_led].led_blink = BLINK_FOREVER;
			led_dcb[TESO_led].led_state = LED_IDLE;
		}
		if(temp[0] & 0x08)	{		//memoria de alarma de tesoro
			rotu_state = AUTR_MEMALR;
			led_dcb[ROTU_led].led_cad = AUTORESET_BLINK_CAD;
			led_dcb[ROTU_led].led_blink = BLINK_FOREVER;
			led_dcb[ROTU_led].led_state = LED_IDLE;
		}
	}


	len = flash0_read(mybuffer, DF_ALARMHISTORY_OFFSET, 8);

	len = flash0_read(temp, DF_ALARMHISTORY_OFFSET, 8);

	for( i = 0; i < 8; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_read(mybuffer, DF_ALARMHISTORY_OFFSET, 8);
			break;
		}
	}

	if((mybuffer[6] == 0xAA) && (mybuffer[7] == 0x55))	{
		BaseAlarmPkt_alarm = mybuffer[1] & 0x57; 				//	& 0x07;
		BaseAlarmPkt_estado_dispositivos = mybuffer[2];
		BaseAlarmPkt_estado_dispositivos &= 0xF8;
		BaseAlarmPkt_memoria_dispositivos = mybuffer[3];
		if( BaseAlarmPkt_alarm & bitpat[F220_bit] )	{
			df220_state = F220_ALRM;
            led_dcb[F220_led].led_cad = 255*0x100 + 0;
            led_dcb[F220_led].led_state = LED_IDLE;
		}
		if(BaseAlarmPkt_alarm & bitpat[APER_bit]){
			daper_state = APER_ALRM;
		}
	} else {
		BaseAlarmPkt_alarm = 0x80;
		BaseAlarmPkt_estado_dispositivos = 0x00;
		BaseAlarmPkt_memoria_dispositivos = 0x00;
	}

	switch(mybuffer[4])	{
	case PRV_NORMAL:
		epreve_state = PRV_NORMAL;
		break;
	case PRV_PREVE:
		epreve_state = PRV_PREVE;
		break;
	}

	//memoria de preve
	if(mybuffer[5] & PREVE_CENTRAL_TX)	{
		SysFlag1 |= PREVE_CENTRAL_TX;
	}
	if(mybuffer[5] & PREVE_CENTRAL_RX)	{
		SysFlag1 |= PREVE_CENTRAL_RX;
	}

	len = flash0_read( mybuffer, DF_RELESTATES, 10);
	for( i = 0; i < 10; i++ )
		relestate[i] = mybuffer[i];

	if(relestate[0] & bitpat[0])	{
		SIRENA_Flag |= STRIKE1_FLAG;
		if(relestate[0] & bitpat[4])	{
			fsmstk1_state = FSMSTK_ON;
		}
	}
	if(relestate[0] & bitpat[1])	{
		SIRENA_Flag |= STRIKE2_FLAG;
		if(relestate[0] & bitpat[5])	{
			fsmstk2_state = FSMSTK_ON;
		}
	}

	len = flash0_read(mybuffer, DOOR1_STATE, 1);
	if(mybuffer[0] == 0x5A)	{
		SIRENA_Flag |= DOOR1_OC_FLAG;
	} else	{
		SIRENA_Flag &= ~DOOR1_OC_FLAG;
	}

	len = flash0_read(mybuffer, DOOR2_STATE, 1);
	if(mybuffer[0] == 0x5A)	{
		SIRENA_Flag |= DOOR2_OC_FLAG;
	} else	{
		SIRENA_Flag &= ~DOOR2_OC_FLAG;
	}

	len = flash0_read(mybuffer, SLLAVE_STATE, 1);
	if(mybuffer[0] == 0x5A)	{
		STRIKE_Flag |= STRKLLAVE_STATE;
	} else	{
		STRIKE_Flag &= ~STRKLLAVE_STATE;
	}
}

//void fsm_fastmode(void)
//{
//	uint16_t Control;
//	uint32_t temp;
//
//	switch(fmode_state)	{
//		case FMODE_IDLE:
//			if( SysFlag3 & SEND_flag )	{
//				SysFlag3 &= ~SEND_flag;
//				LLAVE_TX_ON();
//				fmode_state = FMODE_WAIT201;
//				fmodetimer = 180000;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: SEND_flag ===> IDLE -> WAIT201\n\r");
////				}
////				//-------------------------------
//			}
//			break;
//		case FMODE_WAIT201:
//			if(SysFlag3 & RX201_flag)	{
//				SysFlag3 &= ~RX201_flag;
//				temp = fmodetimer - FmodeDelay;
//				fmodetimer = FmodeDelay;
//				fmode_state = FMODE_DELAY;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: RX201_flag ===> WAIT201 -> DELAY\n\r");
////				}
////				//-------------------------------
//
//			} else
//			if( SysFlag3 & FRFPOLL_flag){
//				SysFlag3 &= ~FRFPOLL_flag;
//				SysFlag3 &= ~FSTOP_flag;
//				fmode_state = FMODE_IDLE;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: FRFPOLL_flag ===> WAIT201 -> IDLE\n\r");
////				}
////				//-------------------------------
//
//			} else
//			if( SysFlag3 & FSTOP_flag )	{
//				SysFlag3 &= ~FSTOP_flag;
//				LLAVE_TX_OFF();
//				fmode_state = FMODE_IDLE;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: FSTOP_flag===> WAIT201 -> IDLE\n\r");
////				}
////				//-------------------------------
//
//			} else
//			if ( !fmodetimer)	{
//				SysFlag3 &= ~FSTOP_flag;
//				SysFlag3 &= ~FRFPOLL_flag;
//				LLAVE_TX_OFF();
//				fmode_state = FMODE_IDLE;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: TimeOut ===> WAIT201 -> IDLE\n\r");
////				}
////				//-------------------------------
//
//			}
//			break;
//		case FMODE_DELAY:
//			if(!fmodetimer)	{
//				Control = rd16_cbus( CMXSTAT_ADDR );
//				if (Control & 0x1000)	{
//					wr8_cbus( CMXTXDATA_ADDR, BaseAlarmPkt_numabo);
//				}
//				fmodetimer = temp;
//				fmode_state = FMODE_WAIT201;
////				//-------------------------------
////				if(DebugFlag & DBGRF_ON_flag)	{
////					CommSendString(DEBUG_COMM, "Cond: TimeOut ===> DELAY -> WAIT201\n\r");
////				}
////				//-------------------------------
//
//			}
//			break;
//		default:
//			fmode_state = FMODE_IDLE;
//			break;
//	}
//}



void fsm_init_cmx_preve( void )
{
	int error;

	switch(fsm_icmx_state)	{
	case FSM_ICMX_IDLE :
		if(((SysFlag1 & PREVE_CENTRAL_RX) || (SysFlag1 & PREVE_CENTRAL_TX)))	{			//si la etapa de RX esta en preve reinicio el modem cada 5 seg, por si el es el culpable
			fsm_icmx_state = FSM_ICMX_INIT;
			timerInitCMX = 30000;
		} else if(!timerIrqCMX)	{					//si no me llega una interrupcion antes de cada 30 segundos, tambien reinicio el modem
			timerInitCMX = 30000;
			fsm_icmx_state = FSM_ICMX_NOIRQ;
		}
		break;
	case FSM_ICMX_INIT :
		if( (!timerInitCMX) && ((SysFlag1 & PREVE_CENTRAL_RX) || (SysFlag1 & PREVE_CENTRAL_TX)) ){
			timerInitCMX = 30000;
			if((TypeAboAns == 5) || (TypeAboAns == 6))	{
				error = InitCMX869_PAP();
			} else	{
				error = InitCMX869();
			}
		} else if((SysFlag3 & VALIDRXCHAR_flag) || (!(SysFlag1 & PREVE_CENTRAL_RX) && !(SysFlag1 & PREVE_CENTRAL_TX)))	{
			SysFlag3 &= ~VALIDRXCHAR_flag;
			fsm_icmx_state = FSM_ICMX_IDLE;
		}
		break;
	case FSM_ICMX_NOIRQ :
		if(timerIrqCMX)	{
			fsm_icmx_state = FSM_ICMX_IDLE;
		} else
		if(!timerInitCMX)	{
			timerInitCMX = 30000;
			if((TypeAboAns == 5) || (TypeAboAns == 6))	{
				error = InitCMX869_PAP();
			} else	{
				error = InitCMX869();
			}
		}
		break;
	default :
		fsm_icmx_state = FSM_ICMX_IDLE;
		break;
	}
}


void fsm_event_on_preve( void )
{
	uint16_t Control;

	switch(fsmEvOnPreve_state)	{
	case FSMEVP_IDLE :
		if( (SysFlag3 & SEND_flag) && (SysFlag3 & FMODE_flag) )	{
			fsmEvOnPreve_state = FSMEVP_WAIT_T1;
			fevptimer = 500;
			SysFlag3 &= ~SEND_flag;
            POWER_TX_ON();
			LLAVE_TX_ON();

		}
		break;
	case FSMEVP_WAIT_T1 :
		if(!fevptimer)	{
			Control = rd16_cbus( CMXSTAT_ADDR );
			if (Control & 0x1000)	{
				wr8_cbus( CMXTXDATA_ADDR, BaseAlarmPkt_numabo);
			}
			fevptimer = 90;
			fsmEvOnPreve_state = FSMEVP_WAIT_T2;
		}
		break;
	case FSMEVP_WAIT_T2 :
		if(!fevptimer)	{
			Control = rd16_cbus( CMXSTAT_ADDR );
			if (Control & 0x1000)	{
				wr8_cbus( CMXTXDATA_ADDR, BaseAlarmPkt_numabo);
			}
			fevptimer = 160;
			fsmEvOnPreve_state = FSMEVP_WAIT_T3;
		}
		break;
	case FSMEVP_WAIT_T3 :
		if(!fevptimer)	{
			SysFlag0 |= FSMTX_flag;
			fevptimer = 320;
			fsmEvOnPreve_state = FSMEVP_WAIT_T4;
		}
		break;
	case FSMEVP_WAIT_T4 :
		if(!fevptimer)	{
			SysFlag0 |= FSMTX_flag;
			fsmEvOnPreve_state = FSMEVP_IDLE;
		}
		break;
	default:
		fsmEvOnPreve_state = FSMEVP_IDLE;
		break;
	}
}



void fsm_evimd_llavetx( void )
{
	switch(fevimdllave_state)	{
	case FEVIMD_IDLE :
		if( (SysFlag3 & SEND_flag) && (SysFlag3 & FMODE_flag) )	{
			ftxontimer = 50000;
			fevimdllave_state = FEVIMD_LLAVEON;
			SystemFlag4 |= NOTUSESIGNATURE;
            POWER_TX_ON();
			LLAVE_TX_ON();
            preve_timer = TIEMPO_PREVE;
            SysFlag1 &= ~PREVE_CENTRAL_RX;

		}
		break;
	case FEVIMD_LLAVEON :
		if(!ftxontimer)	{
			fevimdllave_state = FEVIMD_IDLE;
			LLAVE_TX_OFF();
			SystemFlag4 &= ~NOTUSESIGNATURE;
			SysFlag3 &= ~SEND_flag;
		} else
		if(!(SysFlag3 & SEND_flag))	{
			fevimdllave_state = FEVIMD_IDLE;
			SystemFlag4 &= ~NOTUSESIGNATURE;
		}
		break;
	default :
		fevimdllave_state = FEVIMD_IDLE;
		break;
	}
}

time_t paptimerslot;
uint8_t fsm_pap_state;

void fsm_pap(void)
{

	switch(fsm_pap_state)	{
	case FSMPAP_IDLE:
		if( SEC_TIMER >= paptimerslot + paptslot)	{
			paptimerslot = SEC_TIMER;
			fsm_pap_state = FSMPAP_TXING;
			//---------------------------------------------------------------------
			//lo que haya que hacer cuando llega la encuesta a este abonado
			SysFlag0 |= FSMTX_flag;
			//SysFlag0 |= RF_POLL_flag;
			SysFlag3 &= ~SEND_flag;
			SysFlag1 &= ~PREVE_CENTRAL_RX;
			preve_timer = TIEMPO_PREVE;

			//aviso el polling destellando dos veces el led verde
			led_dcb[APER_led].led_blink = 1;
			led_dcb[APER_led].led_cad = 1*0x100+1;
			led_dcb[APER_led].led_state = LED_IDLE;

			Buzzer_dcb.led_cad = 1*0x100 + 1;
			Buzzer_dcb.led_state = LED_IDLE;
			Buzzer_dcb.led_blink = 1;

			if( Aper_Poll_counter )	{
				Aper_Poll_counter--;
			}

			//---------------------------------------------------------
		}
		break;
	case FSMPAP_TXING:
		if(!(SysFlag0 & FSMTX_flag))	{
			fsm_pap_state = FSMPAP_IDLE;
		}
		break;
	default:
		fsm_pap_state = FSMPAP_IDLE;
		break;
	}

}



void rf_cortex_signature( void )
{
	switch(csign_state)	{
	case CSIGN_IDLE :
//        if(SysFlag1 & SF220_flag)   {
//            SystemFlag3 &= ~NAPER_flag;
//            SystemFlag3 &= ~NAPER_RFPOLL;
//            signature_timer = 90;
//            SysFlag1 &= ~SF220_flag;
//            csign_state = CSIGN_RFP2;
//            SystemFlag3 |= NAPER_F220V;
//        } else
		if(SystemFlag3 & NAPER_flag)	{
			SystemFlag3 &= ~NAPER_flag;
			SystemFlag3 &= ~NAPER_RFPOLL;
            signature_timer = 90;
			csign_state = CSIGN_RFP1;

		}
		break;
	case CSIGN_RFP1 :
		if((SystemFlag3 & NAPER_RFPOLL) && (!(signature_timer)))	{
			SystemFlag3 &= ~NAPER_RFPOLL;
			csign_state = CSIGN_RFP2;
			SystemFlag3 |= NAPER_F220V;
			signature_timer = 90;
		}
		break;
	case CSIGN_RFP2 :
		if((SystemFlag3 & NAPER_RFPOLL) && (!(signature_timer)))	{
			SystemFlag3 &= ~NAPER_RFPOLL;
			csign_state = CSIGN_IDLE;
			SystemFlag3 &= ~NAPER_F220V;
			if(VERSION_NUMBER >= 300)	{
			    if(SystemFlag10 & F220INDICATION1P) {
                    csign_state = CSIGN_IDLE;
			    } else {
                    csign_state = CSIGN_RFP3;
                    signature_timer = 90;
                }
			}
		}
		break;
	case CSIGN_RFP3 :
		if((SystemFlag3 & NAPER_RFPOLL) && (!(signature_timer)))	{
			SystemFlag3 &= ~NAPER_RFPOLL;
			csign_state = CSIGN_RFP4;
			SystemFlag3 |= NAPER_F220V;
			signature_timer = 90;
		}
		break;
	case CSIGN_RFP4 :
		if((SystemFlag3 & NAPER_RFPOLL) && (!(signature_timer)))	{
			SystemFlag3 &= ~NAPER_RFPOLL;
			csign_state = CSIGN_IDLE;
			SystemFlag3 &= ~NAPER_F220V;
		}
                break;
	default :
		csign_state = CSIGN_IDLE;
		break;
	}
}

uint8_t fsm_lbat_state;
#define	LBATT_IDLE		0x10
#define	LBATT_LOWBATT1	0x20
#define	LBATT_LOWBATT2	0x30
#define	LBATT_NORMAL1	0x40
#define	LBATT_NOBATT	0x50


void fsm_deteccion_lowbattery( void )
{
	switch(fsm_lbat_state)	{
	case LBATT_IDLE:
		if((SystemFlag4 & LOWBATT_flag) && (BaseAlarmPkt_alarm & bitpat[F220_bit]))	{
			lowbatt_timer = 45;
			fsm_lbat_state = LBATT_LOWBATT1;
		}
		break;
	case LBATT_LOWBATT1:
		if(!lowbatt_timer)	{
			if(bat_med1 < 1700)	{
				fsm_lbat_state = LBATT_NOBATT;
			} else	{
				fsm_lbat_state = LBATT_LOWBATT2;
				logCidEvent(account, 1, 302, 0, 0);
			}
		} else
		if(!(SystemFlag4 & LOWBATT_flag))	{
			fsm_lbat_state = LBATT_IDLE;
		}
		break;
	case LBATT_LOWBATT2:
		if(!(SystemFlag4 & LOWBATT_flag))	{
			fsm_lbat_state = LBATT_NORMAL1;
			lowbatt_timer = 10;
		}
		break;
	case LBATT_NORMAL1:
		if(!lowbatt_timer)	{
			fsm_lbat_state = LBATT_IDLE;
			logCidEvent(account, 3, 302, 0, 0);
		} else
		if(SystemFlag4 & LOWBATT_flag)	{
			fsm_lbat_state = LBATT_LOWBATT2;
		}
		break;
	case LBATT_NOBATT:
		if(!(BaseAlarmPkt_alarm & bitpat[F220_bit]))	{
			fsm_lbat_state = LBATT_IDLE;
		}
		break;
	default:
		fsm_lbat_state = LBATT_IDLE;
		break;
	}
}




void fsm_deteccion_NP_umbral( void )
{
	switch(fsm_npd_state)	{
	case NPD_IDLE:
		if(np_med1 < 1578)	{
			fsm_npd_state = NPD_PRELOW;
			npd_timer = 10;
		}
		break;
	case NPD_PRELOW:
		if(!npd_timer)	{
			fsm_npd_state = NPD_LOW;
			logCidEvent(account, 1, 921, 0, 0);
			log_nivel_portadora();
		} else
		if(np_med1 >= 1578)	{
			fsm_npd_state = NPD_IDLE;
		}
		break;
	case NPD_LOW:
		if(np_med1 >= 1578)	{
			fsm_npd_state = NPD_PRENORM;
			npd_timer = 10;
		}
		break;
	case NPD_PRENORM:
		if(!npd_timer)	{
			fsm_npd_state = NPD_IDLE;
			logCidEvent(account, 3, 921, 0, 0);
			log_nivel_portadora();
		} else
		if(np_med1 < 1578)	{
			fsm_npd_state = NPD_LOW;
		}
		break;
	default:
		fsm_npd_state = NPD_IDLE;
		break;
	}
}

void LLAVE_TX_ON( void)
{
	if(!(SystemFlag5 & INHIBIT_LLAVE))	{
		GPIO_SetValue(3, 1<<26);
	}
	RADAR_flags |= LLAVEON_FLAG;
}

void LLAVE_TX_OFF( void )
{
	GPIO_ClearValue(3, 1<<26 );
	RADAR_flags &= ~LLAVEON_FLAG;
}


uint32_t timer_llaveon, timer_contpoll, timer_llaveoff, timer_inhcpoll;
uint8_t fsm_llon_state;

void fsm_llaveon_wdog( void )
{
	static int llcount;

	switch( fsm_llon_state )	{
		case FSMLLON_IDLE:
			SystemFlag5 &= ~INHIBIT_LLAVE;
			if(RADAR_flags & LLAVEON_FLAG)	{
				timer_llaveon = 0;
				llcount = 0;
				timer_contpoll = 0;
				fsm_llon_state = FSMLLON_TXON;
				//CommSendString(DEBUG_COMM, "IDLE -> TXON\r\n");
			}
			break;
		case FSMLLON_TXON:
			if(!(RADAR_flags & LLAVEON_FLAG))	{
				fsm_llon_state = FSMLLON_TXOFF;
				llcount++;
				timer_llaveoff = 0;
				//CommSendString(DEBUG_COMM, "TXON -> TXOFF\r\n");
			} else
			if(timer_llaveon > 60000)	{	//aca detecto que la llave estuvo encendida mas de un minuto
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_TXDISP;
				timer_llaveoff = 0;
				POWER_TX_OFF();
                logCidEvent(account, 1, 816, 0, 0);
				//CommSendString(DEBUG_COMM, "TXON -> TXDISP\r\n");
				//oooooooooooooo
			} else
			if(timer_contpoll > 60000)	{	//controlo que el tiempo de polling continuo no supere un minuto
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_CPOLL_ON;
				timer_inhcpoll = 0;
				//CommSendString(DEBUG_COMM, "TXON -> CPOLL_ON 1\r\n");
			}
			if((SystemFlag5 & INHCPOLL_FLAG) && (llcount >= 10 ))	{
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_CPOLL_ON;
				timer_inhcpoll = 0;
				//CommSendString(DEBUG_COMM, "TXON -> CPOLL_ON 2\r\n");
			} else
			if((SystemFlag5 & INHCPOLL_FLAG) && (timer_inhcpoll > 5*60*1000 ))	{
				SystemFlag5 &= ~INHCPOLL_FLAG;
				fsm_llon_state = FSMLLON_IDLE;
				//CommSendString(DEBUG_COMM, "TXON -> IDLE\r\n");
			}
			break;
		case FSMLLON_TXOFF:
			if(timer_llaveoff > 5000)	{	//si la llave estuvo apagada mas de 5 segundos, vulvo a normal
				fsm_llon_state = FSMLLON_IDLE;
				SystemFlag5 &= ~INHIBIT_LLAVE;
				//CommSendString(DEBUG_COMM, "TXOFF -> IDLE\r\n");
			} else
			if(RADAR_flags & LLAVEON_FLAG)	{
				timer_llaveon = 0;
				fsm_llon_state = FSMLLON_TXON;
				//CommSendString(DEBUG_COMM, "TXOFF -> TXON\r\n");
			} else
			if(timer_contpoll > 65000)	{	//controlo que el tiempo de polling continuo no supere dos minuto
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_CPOLL_OFF;
				timer_inhcpoll = 0;
				//CommSendString(DEBUG_COMM, "TXOFF -> CPOLL_OFF 1\r\n");
			} else
			if((SystemFlag5 & INHCPOLL_FLAG) && (llcount >= 10 ))	{
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_CPOLL_OFF;
				timer_inhcpoll = 0;
				//CommSendString(DEBUG_COMM, "TXOFF -> CPOLL_OFF 2\r\n");
			} else
			if((SystemFlag5 & INHCPOLL_FLAG) && (timer_inhcpoll > 5*60*1000 ))	{
				SystemFlag5 &= ~INHCPOLL_FLAG;
				fsm_llon_state = FSMLLON_IDLE;
				//CommSendString(DEBUG_COMM, "TXOFF -> IDLE\r\n");
			}
			break;
		case FSMLLON_CPOLL_ON:
			SystemFlag5 |= INHIBIT_LLAVE;
			//LLAVE_TX_OFF();
			if(!(RADAR_flags & LLAVEON_FLAG))	{
				fsm_llon_state = FSMLLON_CPOLL_OFF;
				llcount++;
				timer_llaveoff = 0;
				//CommSendString(DEBUG_COMM, "CPOLL_ON -> CPOLL_OFF\r\n");
			} else
			if(timer_llaveon > 60000)	{	//aca detecto que la llave estuvo encendida mas de un minuto
				SystemFlag5 |= INHIBIT_LLAVE;
				LLAVE_TX_OFF();
				fsm_llon_state = FSMLLON_TXDISP;
				timer_llaveoff = 0;
				POWER_TX_OFF();
                logCidEvent(account, 1, 816, 0, 0);
				//CommSendString(DEBUG_COMM, "CPOLL_ON -> IDLE\r\n");
				//oooooooooooooo
			}
			break;
		case FSMLLON_CPOLL_OFF:
			SystemFlag5 |= INHIBIT_LLAVE;
			//LLAVE_TX_OFF();
			if(RADAR_flags & LLAVEON_FLAG)	{
				timer_llaveon = 0;
				fsm_llon_state = FSMLLON_CPOLL_ON;
				//CommSendString(DEBUG_COMM, "CPOLL_OFF -> CPOLL_ON\r\n");
			} else
			if(timer_llaveoff > 5000)	{	//si la llave estuvo apagada mas de 5 segundos, vulvo a normal
				fsm_llon_state = FSMLLON_IDLE;
				SystemFlag5 &= ~INHIBIT_LLAVE;
				SystemFlag5 |= INHCPOLL_FLAG;
				//CommSendString(DEBUG_COMM, "CPOLL_OFF -> IDLE\r\n");
			}
			break;
		case FSMLLON_TXDISP:
			SystemFlag5 |= INHIBIT_LLAVE;
			//LLAVE_TX_OFF();
			if(timer_llaveoff > 60000)	{
				POWER_TX_ON();
				SystemFlag5 &= ~INHIBIT_LLAVE;
				fsm_llon_state = FSMLLON_IDLE;
				//CommSendString(DEBUG_COMM, "TXDISP -> IDLE\r\n");
			}
			break;
		default:
			fsm_llon_state = FSMLLON_IDLE;
			break;
	}
}


uint8_t prLlaveState;
uint8_t PruebasFlags;


uint32_t timer_prueba, timpr_llaveon, timpr_llaveoff, timpr_gap, prretries;
uint32_t Timer_prueba, Timpr_llaveon, Timpr_llaveoff, Timpr_gap, Prretries;

void PruebaLlaveWdog( void )
{
	switch(prLlaveState)	{
		case PRLLST_IDLE:
			if( PruebasFlags & StartCPOLL_flag)	{
				timer_prueba = 0;
				timpr_llaveon = 0;
				prretries = 0;
				PruebasFlags &= ~StartCPOLL_flag;
				LlaveOnEvent();
				prLlaveState = PRLLST_ON;
			}
			break;
		case PRLLST_ON:
			if( timpr_llaveon > Timpr_llaveon)	{
				LLAVE_TX_OFF();
				timpr_llaveoff = 0;
				prLlaveState = PRLLST_OFF;
			} else
			if(timer_prueba > Timer_prueba)	{
				timpr_gap = 0;
				prLlaveState = PRLLST_WGAP;
				LLAVE_TX_OFF();

			}
			break;
		case PRLLST_OFF:
			if( timpr_llaveoff > Timpr_llaveoff)	{
				LlaveOnEvent();
				prLlaveState = PRLLST_ON;
				timpr_llaveon = 0;
			} else
			if(timer_prueba > Timer_prueba)	{
				timpr_gap = 0;
				prLlaveState = PRLLST_WGAP;
				LLAVE_TX_OFF();

			}
			break;
		case PRLLST_WGAP:
			if(timpr_gap > Timpr_gap)	{
				if(prretries == Prretries - 1)	{
					LLAVE_TX_OFF();
					prLlaveState = PRLLST_IDLE;
				} else	{
					timpr_llaveon = 0;
					timer_prueba = 0;
					LlaveOnEvent();
					prLlaveState = PRLLST_ON;
					prretries++;
				}
			}
			break;
		default:
			prLlaveState = PRLLST_IDLE;
			break;
	}
}

void LlaveOnEvent( void )
{


		led_dcb[APER_led].led_blink = 1;
		led_dcb[APER_led].led_cad = 2*0x100+2;
		led_dcb[APER_led].led_state = LED_IDLE;

	if(!(SystemFlag5 & INHIBIT_LLAVE))	{
		Buzzer_dcb.led_cad = 2*0x100 + 5;
		Buzzer_dcb.led_state = LED_IDLE;
		Buzzer_dcb.led_blink = 1;
	}
    POWER_TX_ON();
		LLAVE_TX_ON();
    SysFlag1 &= ~PREVE_CENTRAL_RX;
    preve_timer = TIEMPO_PREVE;



}

uint8_t FTXOFF_state;
int ftxoff_timer;

void fsm_txoff(void)
{
	switch(FTXOFF_state)	{
		case FTXOFF_IDLE:
			if(PruebasFlags & TXOFFCMD_flag)	{
				PruebasFlags &= ~TXOFFCMD_flag;
				POWER_TX_OFF();
				LLAVE_TX_OFF();
				FTXOFF_state = FTXOFF_OFF;
				ftxoff_timer = 0;
			}
			break;
		case FTXOFF_OFF:
			if( ftxoff_timer > 15*60)	{
				POWER_TX_ON();
				FTXOFF_state = FTXOFF_WAIT;
				ftxoff_timer = 0;
			}
			break;
		case FTXOFF_WAIT:
			if( ftxoff_timer > 24*60*60)	{
				FTXOFF_state = FTXOFF_IDLE;
				ftxoff_timer = 0;
			}
			break;
		default:
			FTXOFF_state = FTXOFF_IDLE;
			ftxoff_timer = 0;
			break;
	}
}

uint8_t FTXOFF2_state;
int ftxoff2_timer;
int ftxoff2_retries;

void fsm_txoff2(void)
{
	switch(FTXOFF2_state)	{
		case FTXOFF2_IDLE:
			if(PruebasFlags & TXOFF2CMD_flag)	{
				PruebasFlags &= ~TXOFF2CMD_flag;
				POWER_TX_OFF();
				LLAVE_TX_OFF();
				FTXOFF2_state = FTXOFF2_OFF;
				ftxoff2_timer = 0;
				ftxoff2_retries = 0;
			}
			break;
		case FTXOFF2_OFF:
			if( ftxoff2_timer > 6*60)	{
				POWER_TX_ON();
				FTXOFF2_state = FTXOFF2_ON;
				ftxoff2_timer = 0;
			}
			break;
		case FTXOFF2_ON:
			if( ftxoff2_timer > 10*60)	{
				POWER_TX_OFF();
				LLAVE_TX_OFF();
				FTXOFF2_state = FTXOFF2_OFF;
				ftxoff2_timer = 0;
				ftxoff2_retries++;
			}
			if(ftxoff2_retries >= 3)	{
				FTXOFF2_state = FTXOFF2_WAIT;
				POWER_TX_ON();
				ftxoff_timer = 0;
			}
			break;
		case FTXOFF2_WAIT:
			if( ftxoff2_timer > 24*60*60)	{
				FTXOFF2_state = FTXOFF2_IDLE;
				ftxoff2_timer = 0;
			}
			break;
		default:
			FTXOFF2_state = FTXOFF2_IDLE;
			ftxoff2_timer = 0;
			break;
	}
}

int	n_asal, n_teso, n_ince, nmax;
int nmax_asal, nmax_teso, nmax_ince;
time_t last_ASAL, last_voluclose;

void recharge_alarm(uint8_t alarm)
{
	int asal_volu_dt;

	asal_volu_dt = 0;
	
	switch(alarm)	{
		case TESO_bit:
			if( n_teso <= nmax_teso )	{
				BaseAlarmPkt_alarm &= ~0x80;
				teso_state = AUTR_ALRMED;
				teso_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = 2*60;
					} else	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = 1*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						teso_autr_counter = AUTORESET_POLL_COUNT;
						teso_autorst_timer_min = autorst_timer*60;
					} else	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[TESO_led].led_cad = 255*0x100 + 0;
			}
			if(n_teso < 200)	{
				n_teso++;
				write_nsignals();
			}
			break;
		case ASAL_bit:
			last_ASAL = SEC_TIMER;
			if((last_voluclose <= SEC_TIMER) && (last_voluclose != 0))	{
				asal_volu_dt = SEC_TIMER - last_voluclose;
				if(asal_volu_dt <= 300)	{
					n_asal = nmax_asal + 1;
					asal_state = AUTR_ALRMED;
					asal_autr_timer = SEC_TIMER;
					if(SysFlag_AP_Apertura & AP_APR_VALID)	{
						if(paptslot == 0)	{
							asal_autr_counter = 0;
							asal_autorst_timer_min = 2*60;
						} else	{
							asal_autr_counter = 0;
							asal_autorst_timer_min = 1*60;
						}
					}
					else	{
						if(paptslot == 0)	{
							asal_autr_counter = AUTORESET_POLL_COUNT;
							asal_autorst_timer_min = 25*60;
						} else	{
							asal_autr_counter = 0;
							asal_autorst_timer_min = paparst_timer*60;
						}
					}
					led_dcb[ASAL_led].led_cad = 255*0x100 + 0;
					return;
				}
			}
			if( n_asal <= nmax_asal )	{
				BaseAlarmPkt_alarm &= ~0x80;
				asal_state = AUTR_ALRMED;
				asal_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = 2*60;
					} else	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = 1*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						asal_autr_counter = AUTORESET_POLL_COUNT;
						asal_autorst_timer_min = autorst_timer*60;
					} else	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[ASAL_led].led_cad = 255*0x100 + 0;
			}
			if(n_asal < 200)	{
				n_asal++;
				write_nsignals();
			}
			break;
		case INCE_bit:
			if( n_ince <= nmax_ince )	{
				BaseAlarmPkt_alarm &= ~0x80;
				ince_state = AUTR_ALRMED;
				ince_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = 2*60;
					} else	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = 1*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						ince_autr_counter = AUTORESET_POLL_COUNT;
						ince_autorst_timer_min = autorst_timer*60;
					} else	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[INCE_led].led_cad = 255*0x100 + 0;
			}
			if(n_ince < 200)	{
				n_ince++;
				write_nsignals();
			}
			break;
	}
}

void read_nsignals(void)
{
	uint8_t buffer[4];
	int error;

	flash0_read(buffer, DF_NSIGNAL_OFFSET, 3);
	if(buffer[0] < 200)
		n_asal = buffer[0];
	else n_asal = 0;

	if(buffer[1] < 200)
		n_ince = buffer[1];
	else n_ince = 0;

	if(buffer[2] < 200)
		n_teso = buffer[2];
	else n_teso = 0;
}

void write_nsignals(void)
{
	uint8_t buffer[4];
	int error;

	buffer[0] = (uint8_t)n_asal;
	buffer[1] = (uint8_t)n_ince;
	buffer[2] = (uint8_t)n_teso;
	
	error = flash0_write(1, buffer, DF_NSIGNAL_OFFSET, 3);
}


void recharge25min_alarm(uint8_t alarm, uint8_t etype)
{
	int asal_volu_dt;

	asal_volu_dt = 0;

	switch(alarm)	{
		case TESO_bit:
			if( BaseAlarmPkt_alarm & bitpat[TESO_bit])	{
				n_teso = nmax_teso + 1;
				BaseAlarmPkt_alarm &= ~0x80;
				teso_state = AUTR_ALRMED;
				teso_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = 2*60;
					} else	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = 1*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						teso_autr_counter = AUTORESET_POLL_COUNT;
						teso_autorst_timer_min = 25*60;
					} else	{
						teso_autr_counter = 0;
						teso_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[TESO_led].led_cad = 255*0x100 + 0;
			}
			break;
		case ASAL_bit:
			// si estoy entrando
			if(etype == 1) {
                last_voluclose = 0;
                if (last_ASAL <= SEC_TIMER) {
                    asal_volu_dt = SEC_TIMER - last_ASAL;
                    if (asal_volu_dt >= 300) {
                        return;
                    }
                }
            }
			
			if( BaseAlarmPkt_alarm & bitpat[ASAL_bit])	{
				n_asal = nmax_asal + 1;
				asal_state = AUTR_ALRMED;
				asal_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = 1*60;
					} else	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = 2*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						asal_autr_counter = AUTORESET_POLL_COUNT;
						asal_autorst_timer_min = 25*60;
					} else	{
						asal_autr_counter = 0;
						asal_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[ASAL_led].led_cad = 255*0x100 + 0;
			}
			break;
		case INCE_bit:
			if(BaseAlarmPkt_alarm & bitpat[INCE_bit])	{
				n_ince = nmax_ince + 1;
				BaseAlarmPkt_alarm &= ~0x80;
				ince_state = AUTR_ALRMED;
				ince_autr_timer = SEC_TIMER;
				if(SysFlag_AP_Apertura & AP_APR_VALID)	{
					if(paptslot == 0)	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = 2*60;
					} else	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = 1*60;
					}
				}
				else	{
					if(paptslot == 0)	{
						ince_autr_counter = AUTORESET_POLL_COUNT;
						ince_autorst_timer_min = 25*60;
					} else	{
						ince_autr_counter = 0;
						ince_autorst_timer_min = paparst_timer*60;
					}
				}
				led_dcb[INCE_led].led_cad = 255*0x100 + 0;
			}
			break;
	    default:
	        break;
	}
}

void recharge5min_alarm(void)
{


        if( BaseAlarmPkt_alarm & bitpat[TESO_bit])	{
            n_teso = nmax_teso + 1;
            BaseAlarmPkt_alarm &= ~0x80;
            teso_state = AUTR_ALRMED;
            teso_autr_timer = SEC_TIMER;
            teso_autr_counter = 0;
            teso_autorst_timer_min = 3*60;
            led_dcb[TESO_led].led_cad = 255*0x100 + 0;
        }


        if( BaseAlarmPkt_alarm & bitpat[ASAL_bit])	{
            n_asal = nmax_asal + 1;
            asal_state = AUTR_ALRMED;
            asal_autr_timer = SEC_TIMER;
            asal_autr_counter = 0;
            asal_autorst_timer_min = 3*60;
            led_dcb[ASAL_led].led_cad = 255*0x100 + 0;
        }


        if(BaseAlarmPkt_alarm & bitpat[INCE_bit])	{
            n_ince = nmax_ince + 1;
            BaseAlarmPkt_alarm &= ~0x80;
            ince_state = AUTR_ALRMED;
            ince_autr_timer = SEC_TIMER;
            ince_autr_counter = 0;
            ince_autorst_timer_min = 3*60;
            led_dcb[INCE_led].led_cad = 255*0x100 + 0;
        }

}

void refresh_alrm_led( void )
{
    if(led_dcb[TESO_led].led_cad != AUTORESET_BLINK_CAD)    {
        if(BaseAlarmPkt_alarm & bitpat[TESO_bit])   {
            led_dcb[TESO_led].led_cad = 255*0x100 + 0;
            led_dcb[TESO_led].led_state = LED_IDLE;
            led_dcb[TESO_led].led_blink = 0;
        } else  {
            led_dcb[TESO_led].led_cad = 255*0 + 0;
            led_dcb[TESO_led].led_state = LED_IDLE;
            led_dcb[TESO_led].led_blink = 0;
        }
    }

    if(led_dcb[ASAL_led].led_cad != AUTORESET_BLINK_CAD)    {
        if(BaseAlarmPkt_alarm & bitpat[ASAL_bit])   {
            led_dcb[ASAL_led].led_cad = 255*0x100 + 0;
            led_dcb[ASAL_led].led_state = LED_IDLE;
            led_dcb[ASAL_led].led_blink = 0;
        } else  {
            led_dcb[ASAL_led].led_cad = 255*0 + 0;
            led_dcb[ASAL_led].led_state = LED_IDLE;
            led_dcb[ASAL_led].led_blink = 0;
        }
    }

    if(led_dcb[INCE_led].led_cad != AUTORESET_BLINK_CAD)    {
        if(BaseAlarmPkt_alarm & bitpat[INCE_bit])   {
            led_dcb[INCE_led].led_cad = 255*0x100 + 0;
            led_dcb[INCE_led].led_state = LED_IDLE;
            led_dcb[INCE_led].led_blink = 0;
        } else  {
            led_dcb[INCE_led].led_cad = 255*0 + 0;
            led_dcb[INCE_led].led_state = LED_IDLE;
            led_dcb[INCE_led].led_blink = 0;
        }
    }

    if(led_dcb[ROTU_led].led_cad != AUTORESET_BLINK_CAD)    {
        if(BaseAlarmPkt_alarm & bitpat[ROTU_bit])   {
            led_dcb[ROTU_led].led_cad = 255*0x100 + 0;
            led_dcb[ROTU_led].led_state = LED_IDLE;
            led_dcb[ROTU_led].led_blink = 0;
        } else  {
            led_dcb[ROTU_led].led_cad = 255*0 + 0;
            led_dcb[ROTU_led].led_state = LED_IDLE;
            led_dcb[ROTU_led].led_blink = 0;
        }
    }
}