/*
 * mem_i2c.h
 *
 *  Created on: Oct 31, 2011
 *      Author: IronMan
 */

#ifndef MEM_I2C_H_
#define MEM_I2C_H_

#define MC24LC64_ADDR 0xa0		/* I2C address of the 24LC64 EEPROM */
#define MC24LC512_ADDR 0xa0		/* I2C address of the 24LC512 EEPROM */

/*
 * These are states returned by the I2CEngine:
 *
 * IDLE     - is never returned but only used internally
 * PENDING  - is never returned but only used internally in the I2C functions
 * ACK      - The transaction finished and the slave returned ACK (on all bytes)
 * NACK     - The transaction is aborted since the slave returned a NACK
 * SLA_NACK - The transaction is aborted since the slave returned a NACK on the SLA
 *            this can be intentional (e.g. an 24LC08 EEPROM states it is busy)
 *            or the slave is not available/accessible at all.
 * ARB_LOSS - Arbitration loss during any part of the transaction.
 *            This could only happen in a multi master system or could also
 *            identify a hardware problem in the system.
 */
#define I2CSTATE_IDLE     0x000
#define I2CSTATE_PENDING  0x001
#define I2CSTATE_ACK      0x101
#define I2CSTATE_NACK     0x102
#define I2CSTATE_SLA_NACK 0x103
#define I2CSTATE_ARB_LOSS 0x104

#define FAST_MODE_PLUS	0

#define Master_Buffer_BUFSIZE	35
#define Slave_Buffer_BUFSIZE	32
#define MAX_TIMEOUT		0x00FFFFFF

#define I2CMASTER		0x01
#define I2CSLAVE		0x02

#define PCF8594_ADDR	0xA0
#define READ_WRITE		0x01

#define RD_BIT			0x01

#define I2CONSET_I2EN		0x00000040  /* I2C Control Set Register */
#define I2CONSET_AA			0x00000004
#define I2CONSET_SI			0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004  /* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040

#define I2DAT_I2C			0x00000000  /* I2C Data Reg */
#define I2ADR_I2C			0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH			58  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL			57  /* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000020  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000020  /* Fast Plus I2C SCL Duty Cycle Low Reg */


extern volatile uint8_t I2CMasterBuffer[Master_Buffer_BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[Slave_Buffer_BUFSIZE];
extern volatile uint32_t I2CReadLength, I2CWriteLength;

uint32_t I2CInit( uint32_t I2cMode );
uint32_t I2CEngine( void );

void I2C1_IRQHandler(void);
uint32_t I2CInit( uint32_t I2cMode );
uint32_t I2CEngine( void );
uint32_t I2CStart( void );
uint32_t I2CStop( void );

void EepromWriteByte(uint16_t address, uint8_t data, int *error);
void EepromWriteWord(uint16_t address, uint16_t data, int *error);
void EepromWriteDoubleWord(uint16_t address, uint32_t data, int *error);
void EepromWriteBuffer(uint16_t address, uint8_t *buffer, uint16_t len, int *error);
uint8_t EepromReadByte(uint16_t address, int *error);
uint16_t EepromReadWord(uint16_t address, int *error);
uint32_t EepromReadDoubleWord(uint16_t address, int *error);
void EepromReadBuffer(uint16_t address, uint8_t *buffer, int len, int *error);


#endif /* MEM_I2C_H_ */
