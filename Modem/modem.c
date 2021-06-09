/*
 * modem.c
 *
 *  Created on: Nov 27, 2011
 *      Author: IronMan
 */

#include "includes.h"
#include "cpu.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

// Chip Select para DataFlash0
#define MDM_CS_OFF() 	GPIO_SetValue(0, 1<<6)
#define MDM_CS_ON()  	GPIO_ClearValue(0, 1<<6 )
#define INTERBYTEDELAY	0x01

/******************************************************************************
 * Local variables
 *****************************************************************************/

static void SSPSend(uint8_t *buf, uint32_t Length)
{
    SSP_DATA_SETUP_Type xferConfig;

	xferConfig.tx_data = buf;
	xferConfig.rx_data = NULL;
	xferConfig.length  = Length;

    SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING);
}

static void SSPReceive( uint8_t *buf, uint32_t Length )
{
    SSP_DATA_SETUP_Type xferConfig;
    uint32_t i;

	xferConfig.tx_data = NULL;
	xferConfig.rx_data = buf;
	xferConfig.length  = Length;

    SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING);
	for(i = 0; i < INTERBYTEDELAY;)
		i++;

}

void reset_cbus(void)
{
	uint8_t cmd = 0x01;
	
	//CPU_SR_ALLOC();
	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();

	SSPSend( (uint8_t *)&cmd, 1 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();
}

uint8_t rd8_cbus( uint8_t address)
{
	uint8_t rvalue;
	
	//CPU_SR_ALLOC();
	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();

	SSPSend( (uint8_t *)&address, 1 );
	SSPReceive( &rvalue, 1 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();

	return rvalue;
}

uint16_t rd16_cbus( uint8_t address)
{
	uint16_t rvalue;
	uint8_t tmpdata[2];
	
	//CPU_SR_ALLOC();
	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();
	SSPSend( (uint8_t *)&address, 1 );
	SSPReceive( tmpdata, 2 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();

	rvalue = tmpdata[0]*0x100 + tmpdata[1];
	return rvalue;
}

void wr8_cbus( uint8_t address, uint8_t data)
{
	if((address == CMXTXDATA_ADDR) && (DebugFlag & DBGRF_ON_flag) )	{
		CommPutChar(DEBUG_COMM,'[',0);
		printByte(data);
		CommPutChar(DEBUG_COMM,']',0);
		CommPutChar(DEBUG_COMM,' ',0);
	}
  	//CPU_SR_ALLOC();
  	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();
	SSPSend( (uint8_t *)&address, 1 );
	SSPSend( (uint8_t *)&data, 1 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();
}

void wr16_cbus( uint8_t address, uint8_t data1, uint8_t data2)
{
  	//CPU_SR_ALLOC();
	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();
	SSPSend( (uint8_t *)&address, 1 );
	SSPSend( (uint8_t *)&data1, 1 );
	SSPSend( (uint8_t *)&data2, 1 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();
}

uint16_t Read869Status(void)
{
	uint16_t rvalue;
	uint8_t tmpdata[2], address;

	//CPU_SR_ALLOC();
	address = CMXSTAT_ADDR;
	//CPU_CRITICAL_ENTER();
	MDM_CS_ON();
	SSPSend( (uint8_t *)&address, 1 );
	SSPReceive( tmpdata, 2 );
	MDM_CS_OFF();
	//CPU_CRITICAL_EXIT();

	rvalue = tmpdata[0]*0x100 + tmpdata[1];
	return rvalue;
}

void mdm_powerup(void)
{
	OS_ERR os_err;

	wr16_cbus( CMXGENCTRL_ADDR, 0x11, 0x80);
	OSTimeDlyHMSM(0, 0, 0, 20,
	              OS_OPT_TIME_HMSM_STRICT,
	              &os_err);
	wr16_cbus( CMXGENCTRL_ADDR, 0x11, 0x0D);
	Read869Status();
}


uint16_t InitCMX869(void)
{
	OS_ERR os_err;
	uint16_t control;

	reset_cbus();

	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXGENCTRL_ADDR, 0x01, 0xC1 );		//0x80
	OSTimeDlyHMSM(0, 0, 0, 40,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXGENCTRL_ADDR, 0x01, 0x41 );
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXTXMODE_ADDR, 0x7E, 0x0F ); // 7e Configuración del Registro Tx.
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);;
	wr16_cbus( CMXRXMODE_ADDR, 0x6E, 0x37 );// 6e Configuración del Registro RX.
	OSTimeDlyHMSM(0, 0, 0, 40,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	control=rd16_cbus( CMXSTAT_ADDR );
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);
	control=rd16_cbus( CMXSTAT_ADDR );
    OSTimeDlyHMSM(0, 0, 0, 500,
                  OS_OPT_TIME_HMSM_STRICT,
                  &os_err);
	return control;
}


uint16_t InitCMX869_PAP(void)
{
	OS_ERR os_err;
	uint16_t control;

	reset_cbus();

	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXGENCTRL_ADDR, 0x01, 0xC1 );		//0x80
	OSTimeDlyHMSM(0, 0, 0, 40,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXGENCTRL_ADDR, 0x01, 0x41 );
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	wr16_cbus( CMXTXMODE_ADDR, 0x6E, 0x0F ); // 7e Configuración del Registro Tx.
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);;
	wr16_cbus( CMXRXMODE_ADDR, 0x7E, 0x37 );// 6e Configuración del Registro RX.
	OSTimeDlyHMSM(0, 0, 0, 40,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);

	control=rd16_cbus( CMXSTAT_ADDR );
	OSTimeDlyHMSM(0, 0, 0, 3,
		          OS_OPT_TIME_HMSM_STRICT,
		          &os_err);
	control=rd16_cbus( CMXSTAT_ADDR );
    OSTimeDlyHMSM(0, 0, 0, 500,
                  OS_OPT_TIME_HMSM_STRICT,
                  &os_err);
	return control;
}
