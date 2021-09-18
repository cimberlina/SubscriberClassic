/*
 * autoprotect.c
 *
 *  Created on: Jan 5, 2012
 *      Author: IronMan
 */


#include "includes.h"

//para controlar el led infrarojo
#define IR_APER_ON() 	GPIO_SetValue(1, 1<<21)
#define IR_APER_OFF() 	GPIO_ClearValue(1, 1<<21)
#define IR_APER_READ()	(GPIO_ReadValue(1) & (1<<22))

OS_TCB		AutoprotectTask_TCB;
CPU_STK		AutoprotectTask_Stk[AutoprotectTask_STK_SIZE];

uint8_t	SysFlag_AP_Apertura;
uint8_t SysFlag_AP_Reset;
uint8_t SysFlag_AP_zvolt;

uint8_t SysFlag_AP_GenAlarm;

uint16_t	tout_AP_apertura;
uint16_t	tout_AP_reset;

uint8_t		VRST_count;
uint8_t		ptm_pwd;

uint16_t	tapsttimer;

uint8_t AP_apertura_state;		//variable de estado de la maquina de autoproteccion por apertura
//#define	AP_APER_IDLE		0x10
//#define	AP_APER_WAIT_IBUTT	0x15
//#define	AP_APER_OP_NORMAL	0x20
//#define	AP_APER_OP_NORMAL2	0x22
//#define	AP_APER_WAIT_PREVE	0x25
//#define	AP_APER_OP_PREVE	0x30
//#define	AP_APER_WAIT_15MIN	0x40
//#define AP_APER_WAIT_1MIN	0x41

uint8_t AP_reset_state;			//variable de estado de la maquina de autoproteccion por reset
#define	AP_RST_IDLE			0x10
#define	AP_RST_WAIT_IBUTT	0x15
#define	AP_RST_WTOUT		0x20

uint8_t TasAperState;
#define	TAPST_IDLE			0x10
#define	TAPST_DBNC			0x20
#define	TAPST_ON			0x30
#define	TAPST_DBNC2			0x40

uint8_t	AP_zvolt_state;
//#define	AP_ZVOLT_ENTRY		0x05
//#define	AP_ZVOLT_IDLE		0x10
//#define	AP_ZVOLT_MEAS1		0x15
//#define	AP_ZVOLT_MEAS2		0x20
//#define	AP_ZVOLT_MEAS3		0x25
#define	WAIT_IBUTTON_LED_CADENCE	7*0x100 + 7
#define	WAIT_PREVE_LED_CADENCE		3*0x100 + 3
#define	WAIT_INPREVE_LED_CADENCE	7*0x100 + 20


#define WAIT_IBUTTON_BUZ_CADENCE	2*0x100 + 20
#define WAIT_PREVE_BUZ_CADENCE		4*0x100 + 10
#define NORMAL_BUZ_CADENCE			2*0x100 + 50
#define WAIT_1MIN_CADENCE			3*0x100 + 50

void  AutoprotectTask(void  *p_arg)
{
	//uint8_t	i;
	OS_ERR	os_err;
	//int error;
	uint8_t tempstate;
	struct tm mytime1;
	time_t timestamp1;

	(void)p_arg;

	AP_apertura_state = AP_APER_IDLE;
	AP_reset_state = AP_RST_IDLE;
	AP_zvolt_state = AP_ZVOLT_ENTRY;
	TasAperState = TAPST_IDLE;

	OSTimeDlyHMSM(0, 0, TIME_STARTUP_AUTOP, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);


	POWER_TX_ON();
    //InitMonitoreoStruct();
	if( SystemFlag & PORRESET_FLAG )	{
		logCidEvent(account, 1, 902, 0, 0);
		SystemFlag &= ~PORRESET_FLAG;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
		reset_mem_alrm(ASAL_bit);
		reset_mem_alrm(TESO_bit);
		reset_mem_alrm(INCE_bit);
		reset_mem_alrm(ROTU_bit);
	}
	if( SystemFlag & RESETRESET_FLAG )	{
		logCidEvent(account, 1, 903, 0, 0);
		SystemFlag &= ~RESETRESET_FLAG;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	}
	if( SystemFlag & BODRESET_FLAG )	{
		logCidEvent(account, 1, 904, 0, 0);
		SystemFlag &= ~BODRESET_FLAG;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
	}

	if( SystemFlag & WDTRESET_FLAG )	{
		SystemFlag &= ~WDTRESET_FLAG;
		SysFlag_AP_Reset &= ~AP_RST_SYSRESET;
		tempstate = fsmAperReadHistory();
		BaseAlarm_alarm_1 = BaseAlarmPkt_alarm;
		BaseAlarm_estados_dispositivos_1 = BaseAlarmPkt_estado_dispositivos;
		BaseAlarm_memoria_dispositivos_1 = BaseAlarmPkt_memoria_dispositivos;

		//logCidEvent(account, 1, 901, 0, 0);
		switch(tempstate)	{
		case AP_APER_IDLE :
			AP_apertura_state = AP_APER_IDLE;
			break;
		case AP_APER_WAIT_IBUTT :
			AP_apertura_state = AP_APER_WAIT_IBUTT;
			tout_AP_apertura = 60;
			SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
			AP_Aper_led_dcb.led_cad = WAIT_IBUTTON_LED_CADENCE;
			AP_Aper_led_dcb.led_state = LED_IDLE;
			AP_Aper_led_dcb.led_blink = BLINK_FOREVER;
			break;
		case AP_APER_OP_NORMAL :
			AP_apertura_state = AP_APER_OP_NORMAL;
			SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
			SysFlag_AP_Apertura |= AP_APR_VALID;
			POWER_TX_ON();
			SysFlag4 &= ~LOGICALPWRTXOFF;
			SysFlag_AP_Apertura &= ~AP_APR_INPREVE;
			break;
		case AP_APER_WAIT_PREVE :
			AP_apertura_state = AP_APER_WAIT_PREVE;
			tout_AP_apertura = 60*6;
			led_dcb[LLOP_led].led_cad = 255*0x100 + 0;
			AP_Aper_led_dcb.led_cad = WAIT_PREVE_LED_CADENCE;
			AP_Aper_led_dcb.led_state = LED_IDLE;
			AP_Aper_led_dcb.led_blink = BLINK_FOREVER;
			break;
		case AP_APER_OP_PREVE :
			//SystemFlag3 |= MEMAP_PREVE;
			AP_apertura_state = AP_APER_OP_PREVE;
			//POWER_TX_OFF();
			//epreve_state = PRV_PREVE;
			//SysFlag1 |= PREVE_CENTRAL_TX;
			tout_AP_apertura = 60*20;
			//SysFlag4 |= LOGICALPWRTXOFF;
			SysFlag_AP_Apertura |= AP_APR_INPREVE;
			AP_Aper_led_dcb.led_cad = WAIT_INPREVE_LED_CADENCE;
			AP_Aper_led_dcb.led_state = LED_IDLE;
			AP_Aper_led_dcb.led_blink = BLINK_FOREVER;
			SysFlag_AP_Apertura &= ~AP_APR_VALID;
			break;
		case AP_APER_WAIT_15MIN :
			AP_apertura_state = AP_APER_WAIT_15MIN;
			tout_AP_apertura = 60*15;
			break;
		case AP_APER_WAIT_1MIN :
			AP_apertura_state = AP_APER_WAIT_1MIN;
			tout_AP_apertura = 60*1;
			break;
		}
	}


	while(DEF_ON)	{
		WDT_Feed();
		OSTimeDlyHMSM(0, 0, 0, 50,
				  	  OS_OPT_TIME_HMSM_STRICT,
				  	  &os_err);
		if( !(SysFlag0 & STARTUP_flag))	{
		    if(!(DebugFlag & MSGSYSUP_flag))   {
		        timestamp1 = SEC_TIMER;
		        gmtime((const time_t *) &(timestamp1), &mytime1);

		        CommSendString(DEBUG_COMM, asctime(&mytime1));
		        CommSendString(DEBUG_COMM, "\n\r***  System-UP  ***\n\r");
		        DebugFlag |= MSGSYSUP_flag;
		    }
			//----------------------------------------------
			// proceso la linea de apertura2
			GPIO_SetDir(0, (1 << 29), 0);		//la mando como entrada

			switch(TasAperState)	{
			case TAPST_IDLE :
				SystemFlag2 &= ~APE2_sbit;
				TasFlags |= TAS220ON_FLAG;
				if (!(GPIO_ReadValue(0) & (1<<29)))	{
					TasAperState = TAPST_DBNC;
					tapsttimer = 1000;
				}
				break;
			case TAPST_DBNC	:
				if((GPIO_ReadValue(0) & (1<<29)))	{
					TasAperState = TAPST_IDLE;
					SystemFlag2 &= ~APE2_sbit;
					TasFlags |= TAS220ON_FLAG;
				} else
				if(!tapsttimer)	{
					TasAperState = TAPST_ON;
					SystemFlag2 |= APE2_sbit;
					TasFlags &= ~TAS220ON_FLAG;
				}
				break;
			case TAPST_ON :
				SystemFlag2 |= APE2_sbit;
				TasFlags &= ~TAS220ON_FLAG;
				if((GPIO_ReadValue(0) & (1<<29)))	{
					TasAperState = TAPST_DBNC2;
					tapsttimer = 1000;
				}
				break;
			case TAPST_DBNC2	:
				if (!(GPIO_ReadValue(0) & (1<<29)))	{
					TasAperState = TAPST_ON;
					SystemFlag2 |= APE2_sbit;
					TasFlags &= ~TAS220ON_FLAG;
				} else
				if(!tapsttimer)	{
					SystemFlag2 &= ~APE2_sbit;
					TasFlags |= TAS220ON_FLAG;
					TasAperState = TAPST_IDLE;
				}
				break;
			default :
				TasAperState = TAPST_IDLE;
			}

//
//			if (!(GPIO_ReadValue(0) & (1<<29)))	{
//				SystemFlag2 |= APE2_sbit;
//				TasFlags &= ~TAS220ON_FLAG;
//			} else	{
//				SystemFlag2 &= ~APE2_sbit;
//				TasFlags |= TAS220ON_FLAG;
//			}
			GPIO_SetDir(0, (1 << 29), 1);		//la vuelvo a dejar como salida
			//----------------------------------------------
			if(SysFlag4 & SERIALSTART_flag)	{
				fsm_event_preve();
			}
/*			else	{
				SysFlag1 &= ~PREVE_CENTRAL_TX;
				SysFlag1 &= ~PREVE_CENTRAL_RX;
			}*/
			fsm_AP_apertura();
			fsm_AP_reset();
		}
	}

}



void fsm_AP_apertura(void)
{

	static uint32_t arst_timer;
	int error;

	SysFlag_AP_Apertura &= ~AP_APR_SYSRESET;

	switch(AP_apertura_state)	{
		case AP_APER_IDLE :
			SysFlag4 &= ~LOGICALPWRTXOFF;
			SystemFlag4 |= ARSTOK_FLAG;
			SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
			lic_ibuttonid = 0;
			if(SysFlag_AP_Apertura & AP_APR_APRLINE)	{
				tout_AP_apertura = 60;
				AP_apertura_state = AP_APER_WAIT_IBUTT;
				fsmAperWriteHistory();
				SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
				AP_Aper_led_dcb.led_cad = WAIT_IBUTTON_LED_CADENCE;
				AP_Aper_led_dcb.led_state = LED_IDLE;
				AP_Aper_led_dcb.led_blink = BLINK_FOREVER;

			}
			break;
		case AP_APER_WAIT_IBUTT :
			SysFlag4 &= ~LOGICALPWRTXOFF;
			SystemFlag4 |= ARSTOK_FLAG;
			SysFlag1 |= AP_APERLED_CTRL;
			if(SysFlag_AP_Apertura & AP_APR_IBUTTON_OK )	{
				SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
				SysFlag_AP_Apertura |= AP_APR_VALID;
				AP_apertura_state = AP_APER_OP_NORMAL;
				lic_ibuttonid = ibuttonid;
				logCidEvent(account, 1, SPV_AAPE_USER, 0, IbuttonTable[ibuttonid].usernumber + 500);
				SystemFlag4 &= ~ARSTOK_FLAG;
				arst_timer = SEC_TIMER + 60*60*2;
				fsmAperWriteHistory();
				//vuelvo a habilitar el jumper de DHCP
				EepromWriteByte(DHCPJUMPER_E2P_ADDR, 0x01, &error);
				recharge5min_alarm();
			} else
			if(!tout_AP_apertura){
				tout_AP_apertura = 60*6;
				SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
				SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
				AP_apertura_state = AP_APER_WAIT_PREVE;
				fsmAperWriteHistory();
				AP_Aper_led_dcb.led_cad = WAIT_PREVE_LED_CADENCE;
				AP_Aper_led_dcb.led_state = LED_IDLE;
				AP_Aper_led_dcb.led_blink = BLINK_FOREVER;

			}
			break;
		case AP_APER_OP_NORMAL :
			SysFlag4 &= ~LOGICALPWRTXOFF;
			//SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
			SysFlag1 &= ~AP_APERLED_CTRL;
			if(arst_timer == SEC_TIMER)	{
				SystemFlag4 |= ARSTOK_FLAG;
			}
			if(!(SysFlag_AP_Apertura & AP_APR_APRLINE))	{
				tout_AP_apertura = 60*30;
				AP_apertura_state = AP_APER_WAIT_15MIN;

				Buzzer_dcb.led_cad = WAIT_1MIN_CADENCE;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = BLINK_FOREVER;
				fsmAperWriteHistory();
				//CommSendString(DEBUG_COMM, "AP_APER_OP_NORMAL --> AP_APER_IDLE\n\r");
			}
			break;
//		case AP_APER_OP_NORMAL2 :
//			SysFlag4 &= ~LOGICALPWRTXOFF;
//			SysFlag1 |= AP_APERLED_CTRL;
//			if(SysFlag_AP_Apertura & AP_APR_IBUTTON_OK )	{
//				SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
//				SysFlag_AP_Apertura |= AP_APR_VALID;
//				AP_apertura_state = AP_APER_OP_NORMAL;
//				Buzzer_dcb.led_cad = NORMAL_BUZ_CADENCE;
//				Buzzer_dcb.led_state = LED_IDLE;
//				Buzzer_dcb.led_blink = BLINK_FOREVER;
//				fsmAperWriteHistory();
//				//CommSendString(DEBUG_COMM, "AP_APER_WAIT_IBUTT --> AP_APER_OP_NORMAL\n\r");
//			} else
//			if(!(SysFlag_AP_Apertura & AP_APR_APRLINE))	{
//				AP_apertura_state = AP_APER_IDLE;
//				Buzzer_dcb.led_cad = 0;
//				Buzzer_dcb.led_state = LED_IDLE;
//				Buzzer_dcb.led_blink = BLINK_FOREVER;
//				fsmAperWriteHistory();
//			}
//			break;
		case AP_APER_WAIT_PREVE :
			SysFlag1 |= AP_APERLED_CTRL;
			if(SysFlag_AP_Apertura & AP_APR_IBUTTON_OK )	{
				SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
				SysFlag_AP_Apertura |= AP_APR_VALID;
				AP_apertura_state = AP_APER_OP_NORMAL;
				lic_ibuttonid = ibuttonid;
				logCidEvent(account, 1, SPV_AAPE_USER, 0, IbuttonTable[ibuttonid].usernumber + 500);
                recharge5min_alarm();

				fsmAperWriteHistory();
				//CommSendString(DEBUG_COMM, "AP_APER_WAIT_PREVE --> AP_APER_OP_NORMAL\n\r");
			} else
			if(!tout_AP_apertura)	{
				tout_AP_apertura = 60*20;
				POWER_TX_OFF();
				SysFlag4 |= LOGICALPWRTXOFF;
				SysFlag_AP_Apertura |= AP_APR_INPREVE;
				AP_apertura_state = AP_APER_OP_PREVE;
				fsmAperWriteHistory();
				AP_Aper_led_dcb.led_cad = WAIT_INPREVE_LED_CADENCE;
				AP_Aper_led_dcb.led_state = LED_IDLE;
				AP_Aper_led_dcb.led_blink = BLINK_FOREVER;

				SysFlag_AP_Apertura &= ~AP_APR_VALID;
				//CommSendString(DEBUG_COMM, "AP_APER_WAIT_PREVE --> AP_APER_OP_PREVE\n\r");
			}
			break;
		case AP_APER_OP_PREVE :
			//SysFlag4 |= LOGICALPWRTXOFF;
			SysFlag1 |= AP_APERLED_CTRL;
			//POWER_TX_OFF();
			if(SysFlag_AP_Apertura & AP_APR_IBUTTON_OK )	{
				SysFlag_AP_Apertura &= ~AP_APR_IBUTTON_OK;
				SysFlag_AP_Apertura |= AP_APR_VALID;
				AP_apertura_state = AP_APER_OP_NORMAL;
				lic_ibuttonid = ibuttonid;
				logCidEvent(account, 1, SPV_AAPE_USER, 0, IbuttonTable[ibuttonid].usernumber + 500);
                recharge5min_alarm();

				fsmAperWriteHistory();
				//CommSendString(DEBUG_COMM, "AP_APER_OP_PREVE --> AP_APER_OP_NORMAL\n\r");
				POWER_TX_ON();
				SysFlag4 &= ~LOGICALPWRTXOFF;
				SysFlag_AP_Apertura &= ~AP_APR_INPREVE;
			} else
			if(!tout_AP_apertura)	{
				POWER_TX_ON();
				SysFlag4 &= ~LOGICALPWRTXOFF;
			}
			break;
		case AP_APER_WAIT_15MIN:
			if(Buzzer_dcb.led_blink == 0)	{
				if(tout_AP_apertura > 10*60)	{
					Buzzer_dcb.led_cad = WAIT_1MIN_CADENCE;
					Buzzer_dcb.led_state = LED_IDLE;
					Buzzer_dcb.led_blink = BLINK_FOREVER;
				} else	{
					Buzzer_dcb.led_cad = 2*0x100 + 5;
					Buzzer_dcb.led_state = LED_IDLE;
					Buzzer_dcb.led_blink = BLINK_FOREVER;
				}
			}
			if(!tout_AP_apertura)	{
				SysFlag_AP_Apertura &= ~AP_APR_VALID;
				AP_apertura_state = AP_APER_IDLE;
				SystemFlag4 |= ARSTOK_FLAG;
				Buzzer_dcb.led_cad = 0;
				Buzzer_dcb.led_state = LED_IDLE;
				fsmAperWriteHistory();
				SystemFlag6 |= TRGNPPWD_FLAG;
				//new_ptm_pwd();
			} else if((tout_AP_apertura == 10*60) && (Buzzer_dcb.led_cad == WAIT_1MIN_CADENCE))	{
				Buzzer_dcb.led_cad = 2*0x100 + 5;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = BLINK_FOREVER;
			}
			else if(SysFlag_AP_Apertura & AP_APR_APRLINE)	{
				tout_AP_apertura = 60;
				AP_apertura_state = AP_APER_WAIT_IBUTT;
				fsmAperWriteHistory();
				AP_Aper_led_dcb.led_cad = WAIT_IBUTTON_LED_CADENCE;
				AP_Aper_led_dcb.led_state = LED_IDLE;
				AP_Aper_led_dcb.led_blink = BLINK_FOREVER;
				Buzzer_dcb.led_cad = 0;
				Buzzer_dcb.led_state = LED_IDLE;
				//CommSendString(DEBUG_COMM, "AP_APER_IDLE --> AP_APER_WAIT_IBUTT\n\r");
			} else if((!(BaseAlarmPkt_alarm & bitpat[APER_bit])) && (BaseAlarmPkt_alarm & bitpat[NORMAL_bit]) )	{
				AP_apertura_state = AP_APER_WAIT_1MIN;
				fsmAperWriteHistory();
				tout_AP_apertura = 60*1;
				Buzzer_dcb.led_cad = WAIT_1MIN_CADENCE;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = BLINK_FOREVER;
			}
			break;
		case AP_APER_WAIT_1MIN:
			if(Buzzer_dcb.led_blink == 0)	{
				Buzzer_dcb.led_cad = WAIT_1MIN_CADENCE;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = BLINK_FOREVER;
			}
			if(!tout_AP_apertura)	{
				SysFlag_AP_Apertura &= ~AP_APR_VALID;
				AP_apertura_state = AP_APER_IDLE;
				SystemFlag4 |= ARSTOK_FLAG;
				Buzzer_dcb.led_cad = 0;
				Buzzer_dcb.led_state = LED_IDLE;
				fsmAperWriteHistory();
				SystemFlag6 |= TRGNPPWD_FLAG;
				//new_ptm_pwd();
			} else
			if(SysFlag_AP_Apertura & AP_APR_APRLINE)	{
				tout_AP_apertura = 60;
				Buzzer_dcb.led_cad = 0;
				Buzzer_dcb.led_state = LED_IDLE;
				AP_apertura_state = AP_APER_WAIT_IBUTT;
				fsmAperWriteHistory();
				AP_Aper_led_dcb.led_cad = WAIT_IBUTTON_LED_CADENCE;
				AP_Aper_led_dcb.led_state = LED_IDLE;
				AP_Aper_led_dcb.led_blink = BLINK_FOREVER;

				//CommSendString(DEBUG_COMM, "AP_APER_IDLE --> AP_APER_WAIT_IBUTT\n\r");
			}
			break;
		default:
			AP_apertura_state = AP_APER_IDLE;
			Buzzer_dcb.led_cad = 0;
			Buzzer_dcb.led_state = LED_IDLE;
			SystemFlag4 |= ARSTOK_FLAG;
			fsmAperWriteHistory();
			//CommSendString(DEBUG_COMM, "default --> AP_APER_IDLE\n\r");
			break;
	}
}



void fsm_AP_reset(void)
{
	switch(AP_reset_state)	{
		case AP_RST_IDLE:
			if(SysFlag_AP_Reset & AP_RST_SYSRESET)	{
				SysFlag_AP_Reset &= ~AP_RST_SYSRESET;
				SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
				AP_reset_state = AP_RST_WAIT_IBUTT;
				tout_AP_reset = 60*60*3;
				VRST_count = 2;
				fsmAperWriteHistory();
			}
			break;
		case AP_RST_WAIT_IBUTT:
			if(SysFlag_AP_Reset & AP_RST_IBUTTON_OK)	{
				SysFlag_AP_Reset &= ~AP_RST_IBUTTON_OK;
				AP_reset_state = AP_RST_WTOUT;
				lic_ibuttonid = ibuttonid;
				logCidEvent(account, 1, SPV_AAPR_USER, 0, IbuttonTable[ibuttonid].usernumber + 500);
                recharge5min_alarm();
				tout_AP_reset = 60*3;
			} else
			if(!tout_AP_reset)	{
				SysFlag_AP_Reset &= ~AP_RST_IBUTTON_OK;
				AP_reset_state = AP_RST_WTOUT;
			}

			break;
		case AP_RST_WTOUT:
			if((!tout_AP_reset) && (VRST_count == 0))	{
				rotu_autr_counter = 0;
				rotu_state = AUTR_NORMAL;
				BaseAlarmPkt_alarm &= ~bitpat[ROTU_bit];
				AP_reset_state = AP_RST_IDLE;
				fsmAperWriteHistory();
			}
			break;
		default:
			AP_reset_state = AP_RST_IDLE;
			break;
	}
}

void fsm_AP_zvolt(void)
{
	static time_t timer;
	static uint8_t zcount;
	//static uint32_t zvoltmed0[16], zvoltmed1[16], promedio[16];
	static uint32_t zvoltmed0;
	

	switch(AP_zvolt_state)	{
	case AP_ZVOLT_ENTRY:
		timer = MSEC_TIMER + 50000;
		AP_zvolt_state = AP_ZVOLT_IDLE;
		zcount = 0;
		break;
	case AP_ZVOLT_IDLE:
		if(timer < MSEC_TIMER)	{
			timer = MSEC_TIMER + 25;
			PERPWR_OFF();
			if( zcount > 2 )
				zcount = 0;
                SysFlag_AP_zvolt |= AP_ZVOLT_MEAS_flag;
			AP_zvolt_state = AP_ZVOLT_MEAS1;
		}
		break;
	case AP_ZVOLT_MEAS1:
		if(timer < MSEC_TIMER)	{
			SysFlag_AP_zvolt |= AP_ZVOLT_MEAS_flag;
            MUX4051_Address(zcount);
            AP_zvolt_state = AP_ZVOLT_MEAS2;
            timer = MSEC_TIMER + 10;
		}
		break;
	case AP_ZVOLT_MEAS2:
		if(timer < MSEC_TIMER)	{
 			zvoltmed0 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0)) & 0x00000FFF;

			if((zvoltmed0 >= V_NORMAL_LOW) && (zvoltmed0 <= V_NORMAL_HIGH))	{
				SysFlag_AP_GenAlarm |= bitpat[TESO_bit];
				SysFlag_AP_GenAlarm |= bitpat[ROTU_bit];
			}
			PERPWR_ON();
			timer = MSEC_TIMER + 50;
			AP_zvolt_state = AP_ZVOLT_MEAS3;
		}
		break;
	case AP_ZVOLT_MEAS3:
		if(timer < MSEC_TIMER)	{
			timer = MSEC_TIMER + 30000;
			SysFlag_AP_zvolt &= ~AP_ZVOLT_MEAS_flag;
			AP_zvolt_state = AP_ZVOLT_IDLE;
			zcount++;
		}
		break;
	default:
		AP_zvolt_state = AP_ZVOLT_ENTRY;
		break;
	}
}

void fsmAperWriteHistory(void)
{
	uint32_t len;
	uint8_t mybuffer[8], temp[8], i;

	mybuffer[0] = AP_apertura_state;
	mybuffer[1] = BaseAlarmPkt_alarm;
	mybuffer[2] = AP_reset_state;
	mybuffer[3] = 0x00;
	mybuffer[4] = 0x00;
	mybuffer[5] = 0x00;
	mybuffer[6] = 0xAA;
	mybuffer[7] = 0x55;

	len = flash0_write(1, mybuffer, DF_FSMAPINDEX_OFFSET, 8);

	len = flash0_read(temp, DF_FSMAPINDEX_OFFSET, 8);
	for( i = 0; i < 8; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_write(1, mybuffer, DF_FSMAPINDEX_OFFSET, 8);
			break;
		}
	}
	AlarmWriteHistory();
}

uint8_t fsmAperReadHistory(void)
{
	uint32_t len;
	uint8_t mybuffer[8], temp[8],i;

	len = flash0_read(mybuffer, DF_FSMAPINDEX_OFFSET, 8);

	len = flash0_read(temp, DF_FSMAPINDEX_OFFSET, 8);

	for( i = 0; i < 8; i++ )	{
		if( mybuffer[i] != temp[i] )	{
			len = flash0_read(mybuffer, DF_ALARMHISTORY_OFFSET, 8);
			break;
		}
	}

	return mybuffer[0];

}

void set_mem_alrm(uint8_t alarm)
{
	uint8_t temp[8];
	uint32_t error;

	error = flash0_read(temp, MEM_ALARM, 1);
	temp[0] &= 0x0F;
	temp[0] |= 0x50;
	switch(alarm)	{
	case ASAL_bit:
		temp[0] |= 0x01;
		break;
	case TESO_bit:
		temp[0] |= 0x04;
		break;
	case INCE_bit:
		temp[0] |= 0x02;
		break;
	case ROTU_bit:
		temp[0] |= 0x08;
		break;
	}
	error = flash0_write(1, temp, MEM_ALARM, 1);
}


void reset_mem_alrm(uint8_t alarm)
{
	uint8_t temp[8];
	uint32_t error;

	error = flash0_read(temp, MEM_ALARM, 1);
	temp[0] &= 0x0F;
	temp[0] |= 0x50;
	switch(alarm)	{
	case ASAL_bit:
		temp[0] &= ~0x01;
		break;
	case TESO_bit:
		temp[0] &= ~0x04;
		break;
	case INCE_bit:
		temp[0] &= ~0x02;
		break;
	case ROTU_bit:
		temp[0] &= ~0x08;
		break;
	}
	error = flash0_write(1, temp, MEM_ALARM, 1);
}

void warm_reset_mem_alrm(uint8_t alarm)
{
    uint8_t temp[8];
    uint32_t error;

    error = flash0_read(temp, MEM_ALARM, 1);
    temp[0] &= 0x0F;
    temp[0] |= 0x50;
    switch(alarm)	{
        case ASAL_bit:
            temp[0] &= ~0x01;
            if( !(BaseAlarmPkt_alarm & bitpat[ASAL_bit]) ) {
                led_dcb[ASAL_led].led_cad = 0;
                led_dcb[ASAL_led].led_blink = 0;
                led_dcb[ASAL_led].led_state = LED_IDLE;
            }
            break;
        case TESO_bit:
            temp[0] &= ~0x04;
            if( !(BaseAlarmPkt_alarm & bitpat[TESO_bit]) ) {
                led_dcb[TESO_led].led_cad = 0;
                led_dcb[TESO_led].led_blink = 0;
                led_dcb[TESO_led].led_state = LED_IDLE;
            }
            break;
        case INCE_bit:
            temp[0] &= ~0x02;
            if( !(BaseAlarmPkt_alarm & bitpat[INCE_bit]) ) {
                led_dcb[INCE_led].led_cad = 0;
                led_dcb[INCE_led].led_blink = 0;
                led_dcb[INCE_led].led_state = LED_IDLE;
            }
            break;
        case ROTU_bit:
            temp[0] &= ~0x08;
            if( !(BaseAlarmPkt_alarm & bitpat[ROTU_bit]) ) {
                led_dcb[ROTU_led].led_cad = 0;
                led_dcb[ROTU_led].led_blink = 0;
                led_dcb[ROTU_led].led_state = LED_IDLE;
            }
            break;
    }
    error = flash0_write(1, temp, MEM_ALARM, 1);
}

void new_ptm_pwd( void)
{
	uint32_t len;
	uint8_t temp[2];

	ptm_pwd = (uint8_t)((Math_Rand() % 256) + (SEC_TIMER % 256));
	if(ptm_pwd < 0x80)
		ptm_pwd |= 0x80;
	temp[0] = ptm_pwd;
	temp[1] = 0x5A;

	len = flash0_write(1, temp, DF_PTMPWD_OFFSET, 2);
}