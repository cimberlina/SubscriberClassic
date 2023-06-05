/*
*********************************************************************************************************
*											EEPROM Module
*										Serial EEPROM Interface
*							(c) Copyright 2011, INDAV Ingenieria de Avanzada
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*									Implementacion del modulo
* Filename		:	mem_i2c.c
* Version		:	V1.00
* Programmer(s)	:	CMI
*********************************************************************************************************
*/

/*
*********************************************************************************************************
* 											INCLUDE FILES
*********************************************************************************************************
*/
#include "includes.h"

/*
*********************************************************************************************************
* 											GLOBAL VARIABLES
*********************************************************************************************************
*/

volatile uint32_t I2CMasterState = I2CSTATE_IDLE;
volatile uint32_t I2CSlaveState = I2CSTATE_IDLE;

volatile uint8_t I2CMasterBuffer[Master_Buffer_BUFSIZE];
volatile uint8_t I2CSlaveBuffer[Slave_Buffer_BUFSIZE];
volatile uint32_t I2CReadLength;
volatile uint32_t I2CWriteLength;

volatile uint32_t RdIndex = 0;
volatile uint32_t WrIndex = 0;

/*
*********************************************************************************************************
*********************************************************************************************************
* 											LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*****************************************************************************
** Function name:		I2C_IRQHandler
**
** Descriptions:		I2C interrupt handler, deal with master mode only.
**
** parameters:			None
** Returned value:		None
**
*****************************************************************************/
void I2C1_IRQHandler(void)
{
	uint8_t StatValue;

	/* this handler deals with master read and master write only */
	StatValue = LPC_I2C1->I2STAT;
	switch ( StatValue )
	{
	case 0x08:
		/*
		 * A START condition has been transmitted.
		 * We now send the slave address and initialize
		 * the write buffer
		 * (we always start with a write after START+SLA)
		 */
		WrIndex = 0;
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		//LPC_I2C1->I2CONSET = I2CONSET_AA;
		LPC_I2C1->I2CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
		I2CMasterState = I2CSTATE_PENDING;
		break;

	case 0x10:
		/*
		 * A repeated START condition has been transmitted.
		 * Now a second, read, transaction follows so we
		 * initialize the read buffer.
		 */
		RdIndex = 0;
		/* Send SLA with R bit set, */
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		//LPC_I2C1->I2CONSET = I2CONSET_AA;
		LPC_I2C1->I2CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
	break;

	case 0x18:
		/*
		 * SLA+W has been transmitted; ACK has been received.
		 * We now start writing bytes.
		 */
		LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		//LPC_I2C1->I2CONSET = I2CONSET_AA;
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		break;

	case 0x20:
		/*
		 * SLA+W has been transmitted; NOT ACK has been received.
		 * Send a stop condition to terminate the transaction
		 * and signal I2CEngine the transaction is aborted.
		 */
		LPC_I2C1->I2CONSET = I2CONSET_STO;
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x28:
		/*
		 * Data in I2DAT has been transmitted; ACK has been received.
		 * Continue sending more bytes as long as there are bytes to send
		 * and after this check if a read transaction should follow.
		 */
		if ( WrIndex < I2CWriteLength )
		{
			/* Keep writing as long as bytes avail */
			LPC_I2C1->I2DAT = I2CMasterBuffer[WrIndex++];
		}
		else
		{
			if ( I2CReadLength != 0 )
			{
				/* Send a Repeated START to initialize a read transaction */
				/* (handled in state 0x10)                                */
				LPC_I2C1->I2CONSET = I2CONSET_STA;	/* Set Repeated-start flag */
			}
			else
			{
				I2CMasterState = I2CSTATE_ACK;
				LPC_I2C1->I2CONSET = I2CONSET_STO;      /* Set Stop flag */
			}
		}
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		break;

	case 0x30:
		/*
		 * Data byte in I2DAT has been transmitted; NOT ACK has been received
		 * Send a STOP condition to terminate the transaction and inform the
		 * I2CEngine that the transaction failed.
		 */
		LPC_I2C1->I2CONSET = I2CONSET_STO;
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		I2CMasterState = I2CSTATE_NACK;
		break;

	case 0x38:
		/*
		 * Arbitration loss in SLA+R/W or Data bytes.
		 * This is a fatal condition, the transaction did not complete due
		 * to external reasons (e.g. hardware system failure).
		 * Inform the I2CEngine of this and cancel the transaction
		 * (this is automatically done by the I2C hardware)
		 */
		I2CMasterState = I2CSTATE_ARB_LOSS;
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		break;

	case 0x40:
		/*
		 * SLA+R has been transmitted; ACK has been received.
		 * Initialize a read.
		 * Since a NOT ACK is sent after reading the last byte,
		 * we need to prepare a NOT ACK in case we only read 1 byte.
		 */
		if ( I2CReadLength == 1 )
		{
			/* last (and only) byte: send a NACK after data is received */
			LPC_I2C1->I2CONCLR = I2CONCLR_AAC;
		}
		else
		{
			/* more bytes to follow: send an ACK after data is received */
			LPC_I2C1->I2CONSET = I2CONSET_AA;
		}
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		break;

	case 0x48:
		/*
		 * SLA+R has been transmitted; NOT ACK has been received.
		 * Send a stop condition to terminate the transaction
		 * and signal I2CEngine the transaction is aborted.
		 */
		LPC_I2C1->I2CONSET = I2CONSET_STO;
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x50:
		/*
		 * Data byte has been received; ACK has been returned.
		 * Read the byte and check for more bytes to read.
		 * Send a NOT ACK after the last byte is received
		 */
		I2CSlaveBuffer[RdIndex++] = LPC_I2C1->I2DAT;
		if ( RdIndex < (I2CReadLength-1) )
		{
			/* lmore bytes to follow: send an ACK after data is received */
			LPC_I2C1->I2CONSET = I2CONSET_AA;
		}
		else
		{
			/* last byte: send a NACK after data is received */
			LPC_I2C1->I2CONCLR = I2CONCLR_AAC;
		}
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
		break;

	case 0x58:
		/*
		 * Data byte has been received; NOT ACK has been returned.
		 * This is the last byte to read.
		 * Generate a STOP condition and flag the I2CEngine that the
		 * transaction is finished.
		 */
		I2CSlaveBuffer[RdIndex++] = LPC_I2C1->I2DAT;
		I2CMasterState = I2CSTATE_ACK;
		LPC_I2C1->I2CONSET = I2CONSET_STO;	/* Set Stop flag */
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;	/* Clear SI flag */
		break;


	default:
		LPC_I2C1->I2CONCLR = I2CONCLR_SIC;
	break;
  }
  return;
}


/*****************************************************************************
** Function name:	I2CInit
**
** Descriptions:	Initialize I2C controller
**
** parameters:		I2c mode is either MASTER or SLAVE
** Returned value:	true or false, return false if the I2C
**					interrupt handler was not installed correctly
**
*****************************************************************************/
uint32_t I2CInit( uint32_t I2cMode )
{
	LPC_PINCON->PINSEL1 |= (0x3 << 6);
	LPC_PINCON->PINSEL1 |= (0x3 << 8);
	LPC_PINCON->PINMODE1 &= ~(0x1 << 6);
	LPC_PINCON->PINMODE1 |= (0x1 << 7);
	LPC_PINCON->PINMODE1 &= ~(0x1 << 8);
	LPC_PINCON->PINMODE1 |= (0x1 << 9);
	LPC_PINCON->PINMODE_OD0 |= (0x3 << 19);

	LPC_SC->PCLKSEL1 |= (0x3 << 6);  // cclk/8

	/*--- Clear flags ---*/
	LPC_I2C1->I2CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;

	/*--- Reset registers ---*/
#if FAST_MODE_PLUS
	LPC_I2C1->I2SCLL   = I2SCLL_HS_SCLL;
	LPC_I2C1->I2SCLH   = I2SCLH_HS_SCLH;
#else
	LPC_I2C1->I2SCLL   = 60;
	LPC_I2C1->I2SCLH   = 60;
#endif

	if ( I2cMode == I2CSLAVE )
	{
		LPC_I2C1->I2ADR0 = PCF8594_ADDR;
	}

	/* Enable the I2C Interrupt */
	NVIC_EnableIRQ(I2C1_IRQn);

	LPC_I2C1->I2CONSET = I2CONSET_I2EN;
	return( 1 );
}



/*****************************************************************************
** Function name:	I2CEngine
**
** Descriptions:	The routine to complete a I2C transaction
**					from start to stop. All the intervening
**					steps are handled in the interrupt handler.
**					Before this routine is called, the read
**					length, write length and I2C master buffer
**					need to be filled.
**
** parameters:		None
** Returned value:	Any of the I2CSTATE_... values. See i2c.h
**
*****************************************************************************/
uint32_t I2CEngine( void )
{
  I2CMasterState = I2CSTATE_IDLE;
  RdIndex = 0;
  WrIndex = 0;
  if ( I2CStart() == 0 )
  {
	I2CStop();
	return ( 0 );
  }

  /* wait until the state is a terminal state */
  while (I2CMasterState < 0x100);

  return ( I2CMasterState );
}



/*****************************************************************************
** Function name:	I2CStart
**
** Descriptions:	Create I2C start condition, a timeout
**					value is set if the I2C never gets started,
**					and timed out. It's a fatal error.
**
** parameters:		None
** Returned value:	true or false, return false if timed out
**
*****************************************************************************/
uint32_t I2CStart( void )
{
	uint32_t timeout = 0;

	/*--- Issue a start condition ---*/
	LPC_I2C1->I2CONSET = I2CONSET_STA;	/* Set Start flag */

	while((I2CMasterState != I2CSTATE_PENDING) && (timeout < MAX_TIMEOUT))
	{
		timeout++;
	}

	return (timeout < MAX_TIMEOUT);
}



/*****************************************************************************
** Function name:	I2CStop
**
** Descriptions:	Set the I2C stop condition
**
** parameters:		None
** Returned value:	true or never return
**
*****************************************************************************/
uint32_t I2CStop( void )
{
	uint32_t timeout = 0;

	LPC_I2C1->I2CONSET = I2CONSET_STO;      /* Set Stop flag */
	LPC_I2C1->I2CONCLR = I2CONCLR_SIC;  /* Clear SI flag */

	/*--- Wait for STOP detected ---*/
	while((LPC_I2C1->I2CONSET & I2CONSET_STO) && (timeout < MAX_TIMEOUT))
	{
		timeout++;
	}
	return (timeout >= MAX_TIMEOUT);
}


/*****************************************************************************
** Function name:	EepromWriteByte
**
** Descriptions:	Escribe un byte en la serial EEPROM
**
** parameters:		address, direccion donde escriber en la memoria
** 					data, dato a escribir en la memoria
** Returned value:	TRUE, si la escritura fue correcta
**					FALSE, si hubo error en la escritura
**
*****************************************************************************/
void EepromWriteByte(uint16_t address, uint8_t data, int *error)
{
	uint8_t addr_msb, addr_lsb;

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	I2CWriteLength = 4;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = data;

	uint32_t fin_state = I2CEngine();

	*error = 0;
	return;
}

void EepromWriteWord(uint16_t address, uint16_t data, int *error)
{
	uint8_t addr_msb, addr_lsb;

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	I2CWriteLength = 5;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = (uint8_t)((data >> 8) & 0x00FF);
	I2CMasterBuffer[4] = (uint8_t)(data & 0x00FF);

	uint32_t fin_state = I2CEngine();

	*error = 0;
	return;
}

void EepromWriteDoubleWord(uint16_t address, uint32_t data, int *error)
{
	uint8_t addr_msb, addr_lsb;

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	I2CWriteLength = 7;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = (uint8_t)((data >> 24) & 0x000000FF);
	I2CMasterBuffer[4] = (uint8_t)((data >> 16) & 0x000000FF);
	I2CMasterBuffer[5] = (uint8_t)((data >> 8) &  0x000000FF);
	I2CMasterBuffer[6] = (uint8_t)(data & 0x00FF);

	uint32_t fin_state = I2CEngine();

	*error = 0;
	return;
}


/*****************************************************************************
** Function name:	EepromWriteBuffer
**
** Descriptions:	Escribe una secuencia de longitud len, de bytes,
** 					en la serial EEPROM
**
** parameters:		address, direccion donde escriber en la memoria
** 					buffer, puntero al buffer con los datos a escribir
** 					len, cantidad de bytes a escribir
** Returned value:	TRUE, si la escritura fue correcta
**					FALSE, si hubo error en la escritura
**
*****************************************************************************/
void EepromWriteBuffer(uint16_t address, uint8_t *buffer, uint16_t len, int *error)
{
	uint8_t addr_msb, addr_lsb;
	uint16_t i;

	if(len > (Master_Buffer_BUFSIZE-3) )	{
		*error = 1;
		return;
	}
	for(i = 3; i < (len + 3); i++)
		I2CMasterBuffer[i] = buffer[i-3];

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	I2CWriteLength = len + 3;
	I2CReadLength = 0;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */

	uint32_t fin_state = I2CEngine();

	*error = 0;
	return;
}

uint8_t EepromReadByte(uint16_t address, int *error)
{
	uint8_t addr_msb, addr_lsb;
	uint16_t i;

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	for ( i = 0; i < Slave_Buffer_BUFSIZE; i++ )	{
		I2CSlaveBuffer[i] = 0x00;
	}

	I2CWriteLength = 3;
	I2CReadLength = 1;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = MC24LC512_ADDR | RD_BIT;

	i=0;
	while (I2CEngine() == I2CSTATE_SLA_NACK)
		i++;

	*error = 0;
	return I2CSlaveBuffer[0];
}

uint16_t EepromReadWord(uint16_t address, int *error)
{
	uint8_t addr_msb, addr_lsb;
	uint16_t i, retval;

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	for ( i = 0; i < Slave_Buffer_BUFSIZE; i++ )	{
		I2CSlaveBuffer[i] = 0x00;
	}

	I2CWriteLength = 3;
	I2CReadLength = 2;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = MC24LC512_ADDR | RD_BIT;

	i=0;
	while (I2CEngine() == I2CSTATE_SLA_NACK)
		i++;

	*error = 0;
	retval = (I2CSlaveBuffer[0] << 8) + (I2CSlaveBuffer[1]);
	return retval;
}


//uint32_t EepromReadDoubleWord(uint16_t address, int *error)
//{
//	uint8_t addr_msb, addr_lsb;
//	uint16_t i;
//	uint32_t retval;
//
//	addr_lsb = (uint8_t)(address & 0x00FF);
//	addr_msb = (uint8_t)((address >> 8) & 0x00FF);
//
//	for ( i = 0; i < Slave_Buffer_BUFSIZE; i++ )	{
//		I2CSlaveBuffer[i] = 0x00;
//	}
//
//	I2CWriteLength = 3;
//	I2CReadLength = 4;
//	I2CMasterBuffer[0] = MC24LC512_ADDR;
//	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
//	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
//	I2CMasterBuffer[3] = MC24LC512_ADDR | RD_BIT;
//
//	i=0;
//	while (I2CEngine() == I2CSTATE_SLA_NACK)
//		i++;
//
//	*error = 0;
//
//	retval =  ((I2CSlaveBuffer[0] << 24) & 0xFF000000);
//	retval += ((I2CSlaveBuffer[1] << 16) & 0x00FF0000);
//	retval += ((I2CSlaveBuffer[2] << 8)  & 0x0000FF00);
//	retval += ( I2CSlaveBuffer[0] 	     & 0x000000FF);
//
//	return retval;
//}

void EepromReadBuffer(uint16_t address, uint8_t *buffer, int len, int *error)
{
	uint8_t addr_msb, addr_lsb;
	uint16_t i;

	if(len > (Slave_Buffer_BUFSIZE) )	{
		*error = 1;
		return;
	}

	addr_lsb = (uint8_t)(address & 0x00FF);
	addr_msb = (uint8_t)((address >> 8) & 0x00FF);

	for ( i = 0; i < Slave_Buffer_BUFSIZE; i++ )	{
		I2CSlaveBuffer[i] = 0x00;
	}

	I2CWriteLength = 3;
	I2CReadLength = len;
	I2CMasterBuffer[0] = MC24LC512_ADDR;
	I2CMasterBuffer[1] = addr_msb;		/* address MSB */
	I2CMasterBuffer[2] = addr_lsb;		/* address LSB */
	I2CMasterBuffer[3] = MC24LC512_ADDR | RD_BIT;

	i=0;
	while (I2CEngine() == I2CSTATE_SLA_NACK)
		i++;

	*error = 0;

	for(i = 0; i < len; i++)
		buffer[i] = I2CSlaveBuffer[i];

	return;
}



