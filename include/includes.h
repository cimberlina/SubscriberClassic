/*
 * includes.h
 *
 *  Created on: Oct 19, 2011
 *      Author: IronMan
 */

//#define ABOSOLITARIO 1

#ifndef INCLUDES_H_
#define INCLUDES_H_

#ifndef __INT8_T_TYPE__
#define __INT8_T_TYPE__ signed char
#endif

#ifndef __UINT8_T_TYPE__
#define __UINT8_T_TYPE__ unsigned char
#endif

#ifndef __INT16_T_TYPE__
#define __INT16_T_TYPE__ signed short int
#endif

#ifndef __UINT16_T_TYPE__
#define __UINT16_T_TYPE__ unsigned short int
#endif

#ifndef __INT32_T_TYPE__
#define __INT32_T_TYPE__ signed int
#endif

#ifndef __UINT32_T_TYPE__
#define __UINT32_T_TYPE__ unsigned int
#endif

#ifndef __INT64_T_TYPE__
#define __INT64_T_TYPE__ signed long long int
#endif

#ifndef __UINT64_T_TYPE__
#define __UINT64_T_TYPE__ unsigned long long int
#endif

//#define NOEVENTS    1

//#define USAR_LICENSIA	1
//#define	USAR_IRIDIUM6	1
//#define	USAR_IRIDIUM1H	1
//#define	USAR_IRIDIUM	1

#define RESETENABLE     1



#define PGM1_ON() 	GPIO_SetValue(0, 1<<27)
#define PGM1_OFF() 	GPIO_ClearValue(0, 1<<27 )

#define PERIFPWR_OFF()  GPIO_ClearValue(2, (1 << 8));
#define PERIFPWR_ON()  GPIO_SetValue(2, (1 << 8));

#define BELL1_ON()  GPIO_ClearValue(1, (1 << 25));
#define BELL1_OFF()  GPIO_SetValue(1, (1 << 25));
#define BELL2_ON()  GPIO_ClearValue(1, (1 << 31));
#define BELL2_OFF()  GPIO_SetValue(1, (1 << 31));

#define	LAN485_PTM			1
#define	R3K_SERIAL			1
//#define AUTORST_PRUEBA		1

#define TABLAS_ABO_ON		1
//#define	DEBUG_CID_SERIAL	1

#include "lpc_types.h"
#include  <app_cfg.h>

void  App_InitTCPIP (void);

#define	ETHLNK_CONNECTED	0x10
#define	ETHLNK_DISCONNECTED	0x20
extern uint8_t ethlink_state;

#define	ADC_SCAN_VERSION_0
//#define	ADC_SCAN_VERSION_1

extern uint16_t	NIC_timer;

extern unsigned char SystemFlag;
#define WDTRESET_FLAG	0x04
#define	PORRESET_FLAG	0x01
#define	RESETRESET_FLAG	0x02
#define	BODRESET_FLAG	0x08
#define	PGM1_OFF_FLAG	0x10
#define	R3KSERSPACE		0x20
#define	R3KSERACK		0x40
#define PGM1_ON_FLAG	0x80

extern uint8_t SIRENA_Flag;
#define SIR1_ON_FLAG	0x01
#define SIR2_ON_FLAG	0x02
#define SIR1_OFF_FLAG	0x04
#define SIR2_OFF_FLAG	0x08
#define	DOOR1_OC_FLAG	0x10
#define	DOOR2_OC_FLAG	0x20
#define	STRIKE1_FLAG	0x40
#define	STRIKE2_FLAG	0x80

extern uint8_t STRIKE_Flag;
#define	STRIKE1_ON_FLAG		0x01
#define	STRIKE2_ON_FLAG		0x02
#define	STRIKE1_OFF_FLAG	0x04
#define	STRIKE2_OFF_FLAG	0x08
#define	STRKLLAVE_STATE		0x10

extern uint8_t	fsmstk1_state, fsmstk2_state;
#define	FSMSTK_OFF		0x10
#define	FSMSTK_WAIT1	0x20
#define	FSMSTK_ON		0x30
#define	FSMSTK_WAIT2	0x40


extern unsigned char SystemFlag1;
#define BISEL_FLAG		0x01



extern unsigned char TasFlags;
#define	TASVANDAL_FLAG	0x01
#define	TAS220ON_FLAG	0x02
#define	TASCMDON_FLAG	0x04
#define	TASCMDOFF_FLAG	0x08
#define	APER2STATE_Flag	0x40
#define	TASMODE_FLAG	0x80



extern unsigned char SystemFlag2;
#define MONBUFFER_BUSY	0x01
#define	APE2_sbit		0x02
#define	APE1WDOG_FLAG	0x04
#define	APE2WDOG_FLAG	0x08
#define	LOGAP2E_FLAG		0x10
#define	LOGAP1E_FLAG		0x20
#define	LOGAP2R_FLAG		0x40
#define	LOGAP1R_FLAG		0x80

extern unsigned char SystemFlag3;
#define NAPER_flag		0x01
#define	NAPER_RFPOLL	0x02
#define	NAPER_F220V		0x04
#define ADC_EMERGENCY	0x10
#define	RS485RELE_DONE	0x20
#define	WDOG_EVO_ENABLE	0x40
#define	RADAR_ENABLE	0x80
#define	MEMAP_PREVE		0x08

extern unsigned char SystemFlag4;
#define	LOWBATT_flag	0x01
#define E943F220_DONE	0x02
#define	RS485F220_DONE	0x04
#define	NPMED_FLAG		0x08
#define	LICACT_DONE		0x10
#define	ARSTOK_FLAG		0x20
#define	DOORPAT_FLAG	0x40
#define	NOTUSESIGNATURE	0x80

extern unsigned char SystemFlag5;
#define SERIALNUM_OK	0x01
#define VERSION_SENT	0x02
#define	OPENPTM			0x04
#define SCAN485_DONE	0x08
#define	INHIBIT_LLAVE	0x10
#define	INHCPOLL_FLAG	0x20
#define	LOGAP3E_FLAG	0x40
#define	LOGAP3R_FLAG	0x80

extern unsigned char SystemFlag6;
#define	LIC_SETRTC_flag	0x01
#define LOGAP0R_FLAG	0x02
#define	USE_RADAR_FLAG	0x04
#define USE_LICENSE		0x08
#define USE_ARADAR		0x10
#define	TRGNPPWD_FLAG	0x20
#define	RHB_FLAG		0x40
#define	ENARHB_FLAG		0x80

extern unsigned char SystemFlag7;
#define	CASTDVR_ACK		0x01
#define	RHBVOLU_FLAG	0x02
#define	RHBBUZZON_FLAG	0x04
#define	RHBNETOK_FLAG	0x08
#define	IP150_ALIVE		0x10
#define	IP150_KICK		0x20
#define IP150_CHECK     0x40
#define INPATT_CHECK    0x80

extern unsigned char SystemFlag8;
#define	LOCK1_OPEN		0x01
#define	LOCK2_OPEN		0x02
#define ASKIM1_ALR1     0x04
#define ASKIM1_ALR2     0x08
#define ASKIM2_ALR1     0x10
#define ASKIM2_ALR2     0x20
#define ASKIM3_ALR1     0x40
#define ASKIM3_ALR2     0x80

extern uint32_t SystemFlag9;
//no usar este flag

extern uint8_t SystemFlag10;
#define	CASTROTUSND_FLAG	0x01
#define CAF_MODE            0x02
#define F220INDICATION1P    0x04
#define IBUTTONREAD         0x08
#define UDPLICOK_FLAG       0x10
#define UDPUSELIC_FLAG      0x20
#define VALIDSCAN_FLAG      0x40

extern uint32_t SystemFlag11;
#define DONTSENDEVENTS  (1 << 0)
#define APERASAL_FLAG   (1 << 1)
#define FIRSTCMD_FLAG   (1 << 2)
#define CONSOLASAL_FLAG (1 << 3)
#define OPTOAPER2_FLAG  (1 << 4)
#define INCE2MODE_FLAG  (1 << 5)
#define PWR1PULSE_FLAG  (1 << 6)
#define PWR2PULSE_FLAG  (1 << 7)
#define MACROMODE_FLAG  (1 << 8)
#define EV110P0_FLAG    (1 << 9)
#define EV110P3_FLAG    (1 << 10)
#define EV120P0_FLAG    (1 << 11)
#define EV120P2_FLAG    (1 << 12)
#define EV130P0_FLAG    (1 << 13)
#define EV130P4_FLAG    (1 << 14)
#define EV401P5_FLAG    (1 << 15)
#define EV401P6_FLAG    (1 << 16)
#define EV401P7_FLAG    (1 << 17)
#define EV401P8_FLAG    (1 << 18)
#define EV401P9_FLAG    (1 << 19)

#define GEV110P0_FLAG    (1 << 20)
#define GEV110P3_FLAG    (1 << 21)
#define GEV120P0_FLAG    (1 << 22)
#define GEV120P2_FLAG    (1 << 23)
#define GEV130P0_FLAG    (1 << 24)
#define GEV130P4_FLAG    (1 << 25)
#define GEV401P5_FLAG    (1 << 26)
#define GEV401P6_FLAG    (1 << 27)
#define GEV401P7_FLAG    (1 << 28)
#define GEV401P8_FLAG    (1 << 29)
#define GEV401P9_FLAG    (1 << 30)



extern uint32_t SystemFlag12;
#define NETRECOVERY_FLAG    (1 << 0)
#define ACKNG_FLAG          (1 << 1)
#define FORCESND_FLAG       (1 << 2)
#define E606SND_FLAG        (1 << 3)
#define R606SND_FLAG        (1 << 4)
#define CONSOLTESO_FLAG     (1 << 5)
#define APERTESO_FLAG       (1 << 6)
#define CLEAR485_FLAG       (1 << 7)
#define E130TECNICO_FLAG    (1 << 8)
#define E120TECNICO_FLAG    (1 << 9)

extern uint8_t FSM_FLAG_1;
#define WDEVO0_ALRM_FLAG    (1 << 0)
#define WDEVO1_ALRM_FLAG    (1 << 1)
#define WDEVO2_ALRM_FLAG    (1 << 2)
#define APER2_ALRM_FLAG     (1 << 3)
#define APER13_ALRM_FLAG    (1 << 4)
#define ROT943_FLAG         (1 << 5)
#define APERNG_ALRM_FLAG    (1 << 6)

#define PART5_REDU_FLAGS		0
#define PART6_REDU_FLAGS		4
#define PART7_REDU_FLAGS		8
#define PART8_REDU_FLAGS		12
#define PART9_REDU_FLAGS		16

#define	P5_E401_71_RCVD	(1 << 0)
#define	P5_R401_71_RCVD	(1 << 1)
#define	P5_E401_05_RCVD	(1 << 2)
#define	P5_R401_05_RCVD	(1 << 3)
#define	P6_E401_71_RCVD	(1 << 4)
#define	P6_R401_71_RCVD	(1 << 5)
#define	P6_E401_05_RCVD	(1 << 6)
#define	P6_R401_05_RCVD	(1 << 7)
#define	P7_E401_71_RCVD	(1 << 8)
#define	P7_R401_71_RCVD	(1 << 9)
#define	P7_E401_05_RCVD	(1 << 10)
#define	P7_R401_05_RCVD	(1 << 11)
#define	P8_E401_71_RCVD	(1 << 12)
#define	P8_R401_71_RCVD	(1 << 13)
#define	P8_E401_05_RCVD	(1 << 14)
#define	P8_R401_05_RCVD	(1 << 15)
#define	P9_E401_71_RCVD	(1 << 16)
#define	P9_R401_71_RCVD	(1 << 17)
#define	P9_E401_05_RCVD	(1 << 18)
#define	P9_R401_05_RCVD	(1 << 19)


extern unsigned char RADAR_flags;
#define	LLAVEON_FLAG	0x01
#define AP_RADAR_FLAG	0x02
#define	RADAR_START		0x04
#define	RADAR_IRQ_RISE	0x08
#define	CONSOLE_ENTER	0x10
#define	LIC_ENTER		0x20
#define	CONSOLE_OUT		0x40
#define	CONSOLE_CMDIN	0x80

extern unsigned char RADAR2_flags;
#define RADAR_DETECTED_FLAG		0x01

extern	uint8_t	EVOWD_Flag;

extern int ibuttonid, lic_ibuttonid;




// Standard Libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define IDZONE0		0
#define IDZONE1		31
#define	IDZONE3		81
#define	IDZONE4		111
#define	IDZONE5		171
#define	IDZONE6		201

//definicion de codigos de eventos CID customizados para el abonad
#define SPV_PGM_NUMABO	670
#define	SPV_PGM_MODE	671
#define	SPV_PGM_ZONE	672
#define	SPV_PGM_EFORMAT	673
#define	SPV_AAPE_USER	674
#define	SPV_AAPR_USER	675

//niveles de seguridad del los usuarios de cortex
#ifndef  USAR_LICENSIA
#define	MCMI_LEVEL	0 		//180
#define	PROD_LEVEL	0 		//90
#define	MONI_LEVEL	0 		//75
#define OPER_LEVEL	0 		//45
#define	FREE_LEVEL	0
#else
#define	MCMI_LEVEL	180
#define	PROD_LEVEL	90
#define	MONI_LEVEL	75
#define OPER_LEVEL	45
#define	FREE_LEVEL	0
#endif



// CMSIS Drivers
#include "LPC17xx.h"			/* LPC17xx Peripheral Registers */
#include "lpc17xx_gpio.h"
#include "lpc17xx_libcfg.h"
#include "lpc17xx_emac.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_wdt.h"
#include "lpc17xx_rtc.h"
#include "lpc17xx_timer.h"

// Libraries
#include  <cpu.h>
#include  <cpu_core.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


#include  <bsp.h>
#include  <os.h>
#include  <os_csp.h>

#include  <net.h>
//#include  <net_dev_cfg.h>
//#include  <net_bsp.h>
#include  <net_phy.h>

#include "unixtime.h"
#include "serial.h"
#include "console.h"
//#include "telnet_console.h"
#include "mem_i2c.h"
#include "utils.h"
#include "DataFlash.h"
#include "modem.h"
#include "zonescan.h"
#include "fsm_deteccion.h"
#include "eeprom_map.h"
#include "ibuttons.h"
#include "autoprotect.h"


#include "log_event.h"
#include "protocols.h"
//#include "connection.h"
#include "rabbit.h"
#include "lan485.h"
#include "udpserver.h"
#include "TI_aes.h"
#include "radar.h"

#include "dhcp.h"
#include "IridiumSBD.h"
#include "fsm_deteccion.h"


void RTC_IRQHandler(void);
extern time_t SEC_TIMER;
extern time_t MSEC_TIMER;

extern time_t wdip150_timer;

extern uint32_t timer_rhb;

void fsm_R3kSendCid(void);
void byteToledBlink( uint8_t);
void ByteToledBlink( uint8_t);

void fsm_TAS( void );
void fsm_wdog_evo( uint8_t this, uint8_t partition );
void fsm_wdog_ip150(void);

NET_IP_ADDR GetLocalIpAdd( void );
NET_IP_ADDR GetNetmask( void );
NET_IP_ADDR GetGateway( void );
int GetMacaddress( uint8_t *boardmac);

void fsm_sir1( void );
void fsm_sir2( void );
void fsm_strike1( void);
void fsm_strike2( void);
void check_fidu_dflash( void );
void fsm_console_enter(void);
void fsm_gprs_detection(void);
void fsm_login(uint8_t incharrx);

void account_time(int basehour, int baseminute, int * ahour, int * amin);

void fsm_event_preve(void);
extern uint8_t epreve_state;
#define	PRV_NORMAL		0x10
#define	PRV_TACHO		0x20
#define	PRV_PREVE		0x30

#define	R3KSERIALBUFFLEN	25
#define LogT_BUFFLEN        40

extern EventRecord R3KeventRecord[R3KSERIALBUFFLEN];
extern int R3KeventRec_writeptr;
extern int R3KeventRec_readptr;
extern int R3KeventRec_count;

extern EventRecord LogT_eventRecord[LogT_BUFFLEN];
extern int LogT_eventRec_writeptr;
extern int LogT_eventRec_readptr;
extern int LogT_eventRec_count;

extern EventRecord EV110P0_temp, EV110P3_temp, EV120P0_temp,EV120P2_temp, EV130P0_temp, EV130P4_temp;
extern EventRecord EV401P5_temp, EV401P6_temp, EV401P7_temp, EV401P8_temp, EV401P9_temp;

extern uint16_t r3kaccount;

extern uint16_t wdtimer, RHB_Time, RHBCAST_Time;

extern OS_SEM LogEventRdyPtr;

typedef struct {
	uint8_t ibutton_data[8];
	int usernumber;
	int zone;
	int CommandLevel;
} IbuttonData;

#define	FSMWDEVO_ENTRY	0x05
#define	FSMWDEVO_IDLE	0x10
#define	FSMWDEVO_ALARM	0x20

extern uint8_t  fsmwdip150_state;
#define	FSMWDIP150_ENTRY	0x05
#define	FSMWDIP150_IDLE	    0x10
#define	FSMWDIP150_ALARM	0x20

extern uint8_t fsm_gprsdet_state;
#define	FSM_GPRSD_INIT			0x01
#define	FSM_GPRSD_IDLE			0x10
#define	FSM_GPRSD_NG			0x20

extern const IbuttonData IbuttonTable[];
extern const unsigned char AES_key[];

extern uint8_t e2promBuffer[128], dflashBuffer[128];

extern uint8_t SerialNumData[8];


extern	NET_IP_ADDR  App_IP_Addr;
extern  NET_IP_ADDR  App_IP_Mask;
extern  NET_IP_ADDR  App_IP_DfltGateway;
extern  NET_IP_ADDR  App_IP_DNS_Srvr;
extern  NET_IP_ADDR  App_IP_NTP_Srvr;
#endif /* INCLUDES_H_ */
