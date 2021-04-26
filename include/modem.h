/*
 * modem.h
 *
 *  Created on: Nov 27, 2011
 *      Author: IronMan
 */

#ifndef MODEM_H_
#define MODEM_H_

//////////////////////////////////////////////////////////
/* 	CMX868 Register Address Definitions					*/
//////////////////////////////////////////////////////////


#define	CMXGENRESET			0x01
#define	CMXGENCTRL_ADDR		0xe0
#define	CMXTXMODE_ADDR		0xe1
#define	CMXRXMODE_ADDR		0xe2
#define	CMXTXDATA_ADDR		0xe3
#define	CMXTXDATAV14_ADDR	0xe4
#define	CMXRXDATA_ADDR		0xe5
#define	CMXSTAT_ADDR		0xe6
#define	CMXPROG_ADDR		0xe8
#define	CMXTESTADDR_ADDR	0xe9
#define	CMXTESTWR_ADDR		0xea
#define	CMXTESTRD_ADDR		0xeb

// bits en el status H para tx del CMX868
#define	S1DET			0x0200
#define	RXENGY			0x0400
#define	TXUF			0x0800
#define	CMX_TXRDY			0x1000
#define	PROGFLAG		0x2000
#define	RINGDET			0x4000
#define	IRQFLAG			0x8000
// bits en el status L para rx del CMX868
#define	CMX_RXRDY			0x0040
#define	RXOF			0x0020
#define	RXFERR			0x0010


uint16_t InitCMX869(void);
uint16_t InitCMX869_PAP(void);
void mdm_powerup(void);
uint16_t Read869Status(void);
void wr16_cbus( uint8_t address, uint8_t data1, uint8_t data2);
void wr8_cbus( uint8_t address, uint8_t data);
uint16_t rd16_cbus( uint8_t address);
uint8_t rd8_cbus( uint8_t address);
void reset_cbus(void);


#endif /* MODEM_H_ */
