/*
 * zonescan.h
 *
 *  Created on: Dec 5, 2011
 *      Author: IronMan
 */

#ifndef ZONESCAN_H_
#define ZONESCAN_H_

extern OS_TCB		ZoneScanTask_TCB;
extern CPU_STK		ZoneScanTask_Stk[];

extern uint8_t Status_Zonas[16];
extern uint32_t zvaluemed1[16];

extern uint32_t np_med1, bat_med1;

void MUX4051_Address(uint8_t zonenumber);
void InitZoneScan( void);

void  ZoneScanTask (void  *p_arg);


//status de las zonas
#define	ALRM_INDEF		'X'		//0x10
#define	ALRM_ROTU		'R'		//0x11
#define	ALRM_EVENTO		'E'		//0x12
#define	ALRM_NORMAL		'N'		//0x13

//thresholds para la deteccion analogica de zonas supervizadas
//#define	V_ROTURA_HIGH	4095
//#define	V_ROTURA_LOW	3601
//
//#define	V_NORMAL_HIGH	3600		//3150
//#define	V_NORMAL_MED	2818
//#define	V_NORMAL_LOW	2458		//2400
//
//#define	V_SABOTAGE_HIGH	2457
//#define	V_SABOTAGE_LOW	1966
//
//#define	V_ALARMA_HIGH	1965		//1586
//#define	V_ALARMA_LOW	819			//1250

#define	V_ROTURA_HIGH	4095
#define	V_ROTURA_LOW	3151

#define	V_NORMAL_HIGH	3150		//3150
#define	V_NORMAL_MED	2818
#define	V_NORMAL_LOW	2200		//2400

#define	V_SABOTAGE_HIGH	2199
#define	V_SABOTAGE_LOW	1901

#define	V_ALARMA_HIGH	1900		//1586
#define	V_ALARMA_LOW	819			//1250
#endif /* ZONESCAN_H_ */
