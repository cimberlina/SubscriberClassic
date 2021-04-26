/*
 * autoprotect.h
 *
 *  Created on: Jan 5, 2012
 *      Author: IronMan
 */

#ifndef AUTOPROTECT_H_
#define AUTOPROTECT_H_


#define PERPWR_ON() 	GPIO_SetValue(2, 1<<8)
#define PERPWR_OFF() 	GPIO_ClearValue(2, 1<<8 )

extern OS_TCB		AutoprotectTask_TCB;
extern CPU_STK		AutoprotectTask_Stk[AutoprotectTask_STK_SIZE];

void  AutoprotectTask(void  *p_arg);
void fsm_AP_apertura(void);
void fsm_AP_reset(void);
void fsm_AP_zvolt(void);
void fsmAperWriteHistory(void);
uint8_t fsmAperReadHistory(void);
void set_mem_alrm(uint8_t alarm);
void reset_mem_alrm(uint8_t alarm);
void warm_reset_mem_alrm(uint8_t alarm);
void new_ptm_pwd( void);

extern uint16_t	tout_AP_apertura;
extern uint16_t	tout_AP_reset;

extern uint8_t	SysFlag_AP_Apertura;
#define	AP_APR_SYSRESET		0x01
#define	AP_APR_IBUTTON_OK	0x02
#define	AP_APR_VALID		0x04
#define	AP_APR_APRLINE		0x10
#define	AP_APR_INPREVE		0x20


extern uint8_t SysFlag_AP_Reset;
#define	AP_RST_SYSRESET		0x01
#define	AP_RST_IBUTTON_OK	0x02
#define	AP_RST_VALID		0x04

extern uint8_t SysFlag_AP_zvolt;
#define	AP_ZVOLT_MEAS_flag		0x01

extern uint8_t SysFlag_AP_GenAlarm;

extern uint8_t	AP_zvolt_state;
#define	AP_ZVOLT_ENTRY		0x05
#define	AP_ZVOLT_IDLE		0x10
#define	AP_ZVOLT_MEAS1		0x15
#define	AP_ZVOLT_MEAS2		0x20
#define	AP_ZVOLT_MEAS3		0x25

extern uint16_t	tapsttimer;

extern uint8_t		VRST_count;

extern uint8_t		ptm_pwd;

extern uint8_t AP_apertura_state;		//variable de estado de la maquina de autoproteccion por apertura
#define	AP_APER_IDLE		0x10
#define	AP_APER_WAIT_IBUTT	0x15
#define	AP_APER_OP_NORMAL	0x20
#define	AP_APER_OP_NORMAL2	0x22
#define	AP_APER_WAIT_PREVE	0x25
#define	AP_APER_OP_PREVE	0x30
#define	AP_APER_WAIT_15MIN	0x40
#define AP_APER_WAIT_1MIN	0x41

#endif /* AUTOPROTECT_H_ */
