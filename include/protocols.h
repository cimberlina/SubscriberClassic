/*
 * protocols.h
 *
 *  Created on: Apr 25, 2012
 *      Author: ironman
 */

#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_

typedef enum
        {
            AP_EYSE1,
            AP_NTSEC4,
            AP_NTSEC5,
            AP_NTSEC6,
            AP_NTSEC7,
            AP_INDAV1,
            AP_NTSEC6E,
} AlarmProtocols;

#define	PKTLEN_PROTOCOL_EYSE1		16
#define	PKTLEN_PROTOCOL_NTSEC4		23
#define	PKTLEN_PROTOCOL_NTSEC5
#define	PKTLEN_PROTOCOL_NTSEC6

extern uint8_t numabo;
extern uint8_t numcen;
//extern char nomsuc[50];
extern char myipaddrstr[16];

extern uint8_t eyseAlarma;
extern uint8_t eyseEstados;
extern uint8_t eyseDisparos;

int ProtocolEncoder(int coid, int account, AlarmProtocols aprotocol, EventRecord *event, uint8_t *outputstr);
int ProtEncoder_EYSE1(EventRecord *event, uint8_t *outputstr);
int ProtEncoder_NTSEC4(int account, EventRecord *event, uint8_t *outputstr);
int ProtEncoder_NTSEC5(int account, EventRecord *event, uint8_t *outputstr, uint8_t sec);
int ProtEncoder_NTSEC6(int account, EventRecord *event, uint8_t *outputstr, uint8_t sec);
int ProtEncoder_NTSEC7(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec);
int ProtEncoder_NTSEC6E(int paccount, EventRecord *event, uint8_t *outputstr, uint8_t sec);
int ProtEncoder_INDAV1(int account, EventRecord *event, uint8_t *outputstr);
int ProtEncoder_R3KCID(EventRecord *event, uint8_t *outputstr);
int ProtEncoder_R3KCID_T(EventRecord *event, uint8_t *outputstr);
void ConvertEventToEYSEbytes( EventRecord *event, uint8_t *EyseAlarma, uint8_t *EyseEstados, uint8_t *EyseDisparos);

int heartbeat_NTSEC4(int account, uint8_t *outputstr);
int heartbeat_NTSEC5(int account, uint8_t *outputstr, uint8_t sec);
int heartbeat_NTSEC6(int account, uint8_t *outputstr, uint8_t sec);
int heartbeat_NTSEC7(int account, uint8_t *outputstr, uint8_t sec);
int heartbeat_EYSE1( uint8_t *outputstr);
uint8_t hex2int( uint8_t mydata);
void partition_bit_set( uint8_t *databuffer, uint8_t this_partition, uint8_t this_state);
int sendEyseId(uint8_t *outputstr);

//*******************************************************************
//
//				 ---------------------------------------
//	1er byte	|			  	NUMABO					|	Numero de abonado
//				 ---------------------------------------
//	2do byte	|NORM|F220|LLOP|APER|ROTU|TESO|INCE|ASAL|	Alarma
//				 ---------------------------------------
//	3er byte	|EPT4|EPT3|EPT2|EPT1|EDSC|    |    |    |	Estado de dispositivos
//				 ---------------------------------------
//	4to byte	|TPT4|IPT4|TPT3|IPT3|TPT2|IPT2|TPT1|IPT1|	Memoria de disparos
//				 ---------------------------------------
//
//*******************************************************************

//defines de los bits de alarma
#define	EYSE_NORM_bit	0x80
#define	EYSE_F220_bit	0x40
#define	EYSE_LLOP_bit	0x20
#define	EYSE_APER_bit	0x10
#define	EYSE_ROTU_bit	0x08
#define	EYSE_TESO_bit	0x04
#define	EYSE_INCE_bit	0x02
#define	EYSE_ASAL_bit	0x01

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

#define	CEYSE_PART_DSC	0x5
#define	CEYSE_PART_PT1	0x6
#define	CEYSE_PART_PT2	0x7
#define	CEYSE_PART_PT3	0x8
#define	CEYSE_PART_PT4	0x9

#define	EYSE_ZI_PT1		61
#define	EYSE_ZT_PT1		62
#define	EYSE_ZI_PT2		71
#define	EYSE_ZT_PT2		72
#define	EYSE_ZI_PT3		81
#define	EYSE_ZT_PT3		82
#define	EYSE_ZI_PT4		91
#define	EYSE_ZT_PT4		92

#define	CEYSE_ZI_PT1		0x61
#define	CEYSE_ZT_PT1		0x62
#define	CEYSE_ZI_PT2		0x71
#define	CEYSE_ZT_PT2		0x72
#define	CEYSE_ZI_PT3		0x81
#define	CEYSE_ZT_PT3		0x82
#define	CEYSE_ZI_PT4		0x91
#define	CEYSE_ZT_PT4		0x92

#endif /* PROTOCOLS_H_ */
