/*
 * serial.c
 *
 *  Created on: Oct 12, 2011
 *      Author: IronMan
 */
#include "includes.h"

#define	NUL		0x00





/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/

typedef struct {
    CPU_INT16U     	RingBufRxCtr;                /* Number of characters in the Rx ring buffer              */
    OS_SEM			RingBufRxSem;                /* Pointer to Rx semaphore                                 */
    CPU_INT08U     	*RingBufRxInPtr;              /* Pointer to where next character will be inserted        */
    CPU_INT08U     	*RingBufRxOutPtr;             /* Pointer from where next character will be extracted     */
    CPU_INT08U      RingBufRx[COMM_RX_BUF_SIZE]; /* Ring buffer character storage (Rx)            */

    CPU_INT16U     	RingBufTxCtr;                /* Number of characters in the Tx ring buffer              */
    OS_SEM			RingBufTxSem;                /* Pointer to Tx semaphore                                 */
    CPU_INT08U     	*RingBufTxInPtr;              /* Pointer to where next character will be inserted        */
    CPU_INT08U     	*RingBufTxOutPtr;             /* Pointer from where next character will be extracted     */
    CPU_INT08U      RingBufTx[COMM_TX_BUF_SIZE]; /* Ring buffer character storage (Tx)                      */
    CPU_INT08U      RingBufTXBusy;                //2003-11-1?????

} COMM_RING_BUF;
typedef struct {
    CPU_INT16U     	RingBufRxCtr;                /* Number of characters in the Rx ring buffer              */
    OS_SEM			RingBufRxSem;                /* Pointer to Rx semaphore                                 */
    CPU_INT08U     	*RingBufRxInPtr;              /* Pointer to where next character will be inserted        */
    CPU_INT08U     	*RingBufRxOutPtr;             /* Pointer from where next character will be extracted     */
    CPU_INT08U      RingBufRx[COMM_RX_BUF_SIZE2]; /* Ring buffer character storage (Rx)            */

    CPU_INT16U     	RingBufTxCtr;                /* Number of characters in the Tx ring buffer              */
    OS_SEM			RingBufTxSem;                /* Pointer to Tx semaphore                                 */
    CPU_INT08U     	*RingBufTxInPtr;              /* Pointer to where next character will be inserted        */
    CPU_INT08U     	*RingBufTxOutPtr;             /* Pointer from where next character will be extracted     */
    CPU_INT08U      RingBufTx[COMM_TX_BUF_SIZE]; /* Ring buffer character storage (Tx)                      */
    CPU_INT08U      RingBufTXBusy;                //2003-11-1?????

} COMM_RING_BUF2;

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

COMM_RING_BUF  UART0_Buf;

COMM_RING_BUF  UART1_Buf;
COMM_RING_BUF  UART2_Buf;
COMM_RING_BUF2  UART3_Buf;

/*
*********************************************************************************************************
*                                    REMOVE CHARACTER FROM RING BUFFER
*
*
* Description : This function is called by your application to obtain a character from the communications
*               channel.  The function will wait for a character to be received on the serial channel or
*               until the function times out.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
*               'to'    is the amount of time (in clock ticks) that the calling function is willing to
*                       wait for a character to arrive.  If you specify a timeout of 0, the function will
*                       wait forever for a character to arrive.
*               'err'   is a pointer to where an error code will be placed:
*                           *err is set to COMM_NO_ERR     if a character has been received
*                           *err is set to COMM_RX_TIMEOUT if a timeout occurred
*                           *err is set to COMM_BAD_CH     if you specify an invalid channel number
* Returns     : The character in the buffer (or NUL if a timeout occurred)
*********************************************************************************************************
*/

CPU_INT08U  CommGetChar (CPU_INT08U ch, OS_TICK to, CPU_INT08U *err)
{

	CPU_INT08U		c;
    OS_ERR			os_err;
    COMM_RING_BUF 	*pbuf;
	COMM_RING_BUF2 	*pbuf2;
	int commrxbuffsize;

    CPU_SR_ALLOC();

    switch (ch) {                                          /* Obtain pointer to communications channel */
        case COMM0:
             pbuf = &UART0_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
             break;
        case COMM1:
             pbuf = &UART1_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
             break;
        case COMM2:
             pbuf = &UART2_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
             break;
        case COMM3:
             pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE2;
             break;

        default:
             *err = COMM_BAD_CH;
             return (NUL);
    }

    OSSemPend(&(pbuf->RingBufRxSem),
              to,
              OS_OPT_PEND_BLOCKING,
              0,
              &os_err);

    if (os_err == OS_ERR_TIMEOUT) {                             /* See if characters received within timeout*/
        *err = COMM_RX_TIMEOUT;                            /* No, return error code                    */
        return (NUL);
    } else {
        CPU_CRITICAL_ENTER();
        pbuf->RingBufRxCtr--;                              /* Yes, decrement character count           */
        c = *pbuf->RingBufRxOutPtr++;                      /* Get character from buffer                */
        if (pbuf->RingBufRxOutPtr == &pbuf->RingBufRx[commrxbuffsize]) {     /* Wrap OUT pointer     */
            pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        }
        CPU_CRITICAL_EXIT();
        *err = COMM_NO_ERR;
        return (c);
    }
}

int COMMXrdUsed( CPU_INT08U ch)
{
	COMM_RING_BUF 	*pbuf;

	switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 break;

		default:
			 
			 return (NUL);
	}
	return pbuf->RingBufRxCtr;
}

int COMMXwrUsed( CPU_INT08U ch)
{
	COMM_RING_BUF 	*pbuf;

	switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 break;

		default:
			 
			 return (NUL);
	}
	return pbuf->RingBufTxCtr;
}

int COMMXwrFree( CPU_INT08U ch)
{
	COMM_RING_BUF 	*pbuf;

	switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 break;

		default:
			 
			 return (NUL);
	}
	return (COMM_TX_BUF_SIZE - pbuf->RingBufTxCtr);
}




/*
*********************************************************************************************************
*                                  GET TX CHARACTER FROM RING BUFFER
*
*
* Description : This function is called by the Tx ISR to extract the next character from the Tx buffer.
*               The function returns FALSE if the buffer is empty after the character is extracted from
*               the buffer.  This is done to signal the Tx ISR to disable interrupts because this is the
*               last character to send.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
*               'err'   is a pointer to where an error code will be deposited:
*                           *err is set to COMM_NO_ERR         if at least one character was available
*                                                              from the buffer.
*                           *err is set to COMM_TX_EMPTY       if the Tx buffer is empty.
*                           *err is set to COMM_BAD_CH         if you have specified an incorrect channel
* Returns     : The next character in the Tx buffer or NUL if the buffer is empty.
*********************************************************************************************************
*/

CPU_INT08U  CommGetTxChar (CPU_INT08U ch, CPU_INT08U *err)
{
	CPU_INT08U      c;
    COMM_RING_BUF	*pbuf;
    OS_ERR			os_err;

    switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 break;

		default:
			 *err = COMM_BAD_CH;
			 return (NUL);
	}

    if (pbuf->RingBufTxCtr > 0) {                          /* See if buffer is empty                   */
        pbuf->RingBufTxCtr--;                              /* No, decrement character count            */
        c = *pbuf->RingBufTxOutPtr++;                      /* Get character from buffer                */
        if (pbuf->RingBufTxOutPtr == &pbuf->RingBufTx[COMM_TX_BUF_SIZE]) {     /* Wrap OUT pointer     */
            pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        }
        OSSemPost(&(pbuf->RingBufTxSem),					/* Indicate that character will be sent     */
				  OS_OPT_POST_ALL,
				  &os_err);
        *err = COMM_NO_ERR;
        return (c);                                        /* Characters are still available           */
    } else {
        *err = COMM_TX_EMPTY;
        return (NUL);                                      /* Buffer is empty                          */
    }
}

/*
*********************************************************************************************************
*                                  INITIALIZE COMMUNICATIONS MODULE
*
*
* Description : This function is called by your application to initialize the communications module.  You
*               must call this function before calling any other functions.
* Arguments   : none
*********************************************************************************************************
*/

void  CommInit (CPU_INT08U ch, CPU_INT08U *err)
{
    COMM_RING_BUF *pbuf;
    OS_ERR	os_err;
    uint16_t i;
	int commrxbuffsize;

    switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE2;
			 break;

		default:
			 *err = COMM_BAD_CH;
			 break;
	}

    for(i = 0; i < COMM_TX_BUF_SIZE; i++)
    	pbuf->RingBufTx[i] = 0x00;
    for(i = 0; i < commrxbuffsize; i++)
        	pbuf->RingBufRx[i] = 0x00;

    pbuf->RingBufRxCtr    = 0;
    pbuf->RingBufRxInPtr  = &pbuf->RingBufRx[0];
    pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
    //pbuf->RingBufRxSem    = OSSemCreate(0);
    OSSemCreate(&(pbuf->RingBufRxSem),
    			"RxBuff Sem",
    			0,
    			&os_err);

    pbuf->RingBufTxCtr    = 0;
    pbuf->RingBufTxInPtr  = &pbuf->RingBufTx[0];
    pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
    //pbuf->RingBufTxSem    = OSSemCreate(COMM_TX_BUF_SIZE);
    OSSemCreate(&(pbuf->RingBufTxSem),
        			"TxBuff Sem",
        			COMM_TX_BUF_SIZE,
        			&os_err);
    pbuf->RingBufTXBusy   = 0;
}

/*
*********************************************************************************************************
*                                   SEE IF RX CHARACTER BUFFER IS EMPTY
*
*
* Description : This function is called by your application to see if any character is available from the
*               communications channel.  If at least one character is available, the function returns
*               FALSE otherwise, the function returns TRUE.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
* Returns     : TRUE    if the buffer IS empty.
*               FALSE   if the buffer IS NOT empty or you have specified an incorrect channel.
*********************************************************************************************************
*/

CPU_BOOLEAN  CommIsEmpty (CPU_INT08U ch)
{
	CPU_BOOLEAN		empty;
    COMM_RING_BUF	*pbuf;

    CPU_SR_ALLOC();

    switch (ch) {                                          /* Obtain pointer to communications channel */
	case COMM0:
		 pbuf = &UART0_Buf;
		 break;
	case COMM1:
		 pbuf = &UART1_Buf;
		 break;
	case COMM2:
		 pbuf = &UART2_Buf;
		 break;
	case COMM3:
		 pbuf = (COMM_RING_BUF *)&UART3_Buf;
		 break;

        default:
             return (TRUE);
    }
    CPU_CRITICAL_ENTER();
    if (pbuf->RingBufRxCtr > 0) {                          /* See if buffer is empty                   */
        empty = FALSE;                                     /* Buffer is NOT empty                      */
    } else {
        empty = TRUE;                                      /* Buffer is empty                          */
    }
    CPU_CRITICAL_EXIT();
    return (empty);
}


/*
*********************************************************************************************************
*                                   SEE IF TX CHARACTER BUFFER IS FULL
*
*
* Description : This function is called by your application to see if any more characters can be placed
*               in the Tx buffer.  In other words, this function check to see if the Tx buffer is full.
*               If the buffer is full, the function returns TRUE otherwise, the function returns FALSE.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
* Returns     : TRUE    if the buffer IS full.
*               FALSE   if the buffer IS NOT full or you have specified an incorrect channel.
*********************************************************************************************************
*/

//CPU_BOOLEAN  CommIsFull (CPU_INT08U ch)
//{
//	CPU_BOOLEAN		full;
//    COMM_RING_BUF	*pbuf;
//
//    CPU_SR_ALLOC();
//
//    switch (ch) {                                          /* Obtain pointer to communications channel */
//	case COMM0:
//		 pbuf = &UART0_Buf;
//		 break;
//	case COMM1:
//		 pbuf = &UART1_Buf;
//		 break;
//	case COMM2:
//		 pbuf = &UART2_Buf;
//		 break;
//	case COMM3:
//		 pbuf = (COMM_RING_BUF *)&UART3_Buf;
//		 break;
//
//        default:
//             return (TRUE);
//    }
//    CPU_CRITICAL_ENTER();
//    if (pbuf->RingBufTxCtr < COMM_TX_BUF_SIZE) {           /* See if buffer is full                    */
//        full = FALSE;                                      /* Buffer is NOT full                       */
//    } else {
//        full = TRUE;                                       /* Buffer is full                           */
//    }
//    CPU_CRITICAL_EXIT();
//    return (full);
//}

/*
*********************************************************************************************************
*                                            OUTPUT CHARACTER
*
*
* Description : This function is called by your application to send a character on the communications
*               channel.  The function will wait for the buffer to empty out if the buffer is full.
*               The function returns to your application if the buffer doesn't empty within the specified
*               timeout.  A timeout value of 0 means that the calling function will wait forever for the
*               buffer to empty out.  The character to send is first inserted into the Tx buffer and will
*               be sent by the Tx ISR.  If this is the first character placed into the buffer, the Tx ISR
*               will be enabled.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
*               'c'     is the character to send.
*               'to'    is the timeout (in clock ticks) to wait in case the buffer is full.  If you
*                       specify a timeout of 0, the function will wait forever for the buffer to empty.
* Returns     : COMM_NO_ERR      if the character was placed in the Tx buffer
*               COMM_TX_TIMEOUT  if the buffer didn't empty within the specified timeout period
*               COMM_BAD_CH      if you specify an invalid channel number
*********************************************************************************************************
*/

CPU_INT08U  CommPutChar (CPU_INT08U ch, CPU_INT08U c, CPU_INT16U to)
{
	OS_ERR			os_err;
    COMM_RING_BUF	*pbuf;
    CPU_INT08U err;

    CPU_SR_ALLOC();

    switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, ENABLE);
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 UART_IntConfig((LPC_UART_TypeDef *)LPC_UART1, UART_INTCFG_THRE, ENABLE);
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 UART_IntConfig((LPC_UART_TypeDef *)LPC_UART2, UART_INTCFG_THRE, ENABLE);
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_THRE, ENABLE);
			 break;

        default:
             return (COMM_BAD_CH);
    }

    OSSemPend(&(pbuf->RingBufTxSem),						/* Wait for space in Tx buffer              */
              to,
              OS_OPT_PEND_BLOCKING,
              0,
              &os_err);

     if (os_err == OS_ERR_TIMEOUT) {
        return (COMM_TX_TIMEOUT);                          /* Timed out, return error code             */
    }

     CPU_CRITICAL_ENTER();
    pbuf->RingBufTxCtr++;                                  /* No, increment character count            */
    *(pbuf->RingBufTxInPtr++) = c;                           /* Put character into buffer                */
    if (pbuf->RingBufTxInPtr == &pbuf->RingBufTx[COMM_TX_BUF_SIZE]) {     /* Wrap IN pointer           */
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
    }
      // CommTxIntEn(ch);                                   /* Yes, Enable Tx interrupts                */
  //------------------------------------------
    if(pbuf->RingBufTXBusy==0)                            //2003-11-1?????
        {
          pbuf->RingBufTXBusy=1;
          c = CommGetTxChar(ch,&err);

          switch (ch) {                                          /* Obtain pointer to communications channel */
			case COMM0:
				UART_SendByte((LPC_UART_TypeDef *)LPC_UART0, c);
				break;
			case COMM1:
				UART_SendByte((LPC_UART_TypeDef *)LPC_UART1, c);
				break;
			case COMM2:
				UART_SendByte((LPC_UART_TypeDef *)LPC_UART2, c);
				break;
			case COMM3:
				UART_SendByte((LPC_UART_TypeDef *)LPC_UART3, c);
				break;
          }

        }
  //-------------------------------------------

    CPU_CRITICAL_EXIT();
    return (COMM_NO_ERR);
}

void  CommSendIdle(CPU_INT08U ch)
{
     COMM_RING_BUF *pbuf;

     switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 break;

        default:
             return ;
    }
    pbuf->RingBufTXBusy = 0;

}

/*
*********************************************************************************************************
*                                    INSERT CHARACTER INTO RING BUFFER
*
*
* Description : This function is called by the Rx ISR to insert a character into the receive ring buffer.
* Arguments   : 'ch'    is the COMM port channel number and can either be:
*                           COMM1
*                           COMM2
*               'c'     is the character to insert into the ring buffer.  If the buffer is full, the
*                       character will not be inserted, it will be lost.
*********************************************************************************************************
*/

void  CommPutRxChar (CPU_INT08U ch, CPU_INT08U c)
{
    COMM_RING_BUF *pbuf;
    OS_ERR os_err;
	int commrxbuffsize;

    switch (ch) {                                          /* Obtain pointer to communications channel */
		case COMM0:
			 pbuf = &UART0_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM1:
			 pbuf = &UART1_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM2:
			 pbuf = &UART2_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE;
			 break;
		case COMM3:
			 pbuf = (COMM_RING_BUF *)&UART3_Buf;
			 commrxbuffsize = COMM_RX_BUF_SIZE2;
			 break;

        default:
             return;
    }

    if (pbuf->RingBufRxCtr < commrxbuffsize) {           /* See if buffer is full                    */
        pbuf->RingBufRxCtr++;                              /* No, increment character count            */
        *pbuf->RingBufRxInPtr++ = c;                       /* Put character into buffer                */
        if (pbuf->RingBufRxInPtr == &pbuf->RingBufRx[commrxbuffsize]) { /* Wrap IN pointer           */
            pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        }
        OSSemPost(&(pbuf->RingBufRxSem),
                  OS_OPT_POST_ALL,
                  &os_err);                     /* Indicate that character was received     */
    }
}

void  ComSendBuff(CPU_INT08U CommNum,CPU_INT08U *TxBuff,int Len)
{
   while(Len--)
     CommPutChar(CommNum,*TxBuff++,0);
}

void ComPutChar(CPU_INT08U CommNum,char c)
{
  CommPutChar(CommNum, c,0);
}



void  CommSendString(CPU_INT08U CommNum,char *Str)
{
  while(*Str)
  {
    CommPutChar(CommNum,*Str++,0);
  }
}


CPU_INT08U ComGetChar(int CommNum)
{
	CPU_INT08U err;
  return(CommGetChar(CommNum,0,&err));
}


CPU_INT16U ComGetBuff(CPU_INT08U CommNum, CPU_INT16U Timeout,CPU_INT08U *RxBuff,int Len)
{
	CPU_INT16U   i=0;
	CPU_INT08U  c;
	CPU_INT08U  err;
   while(Len--)
   {
     c = CommGetChar(CommNum,Timeout,&err);
     if( err==COMM_NO_ERR )
      {
       *RxBuff++=c;
       i++;
      }
      else
      {
       break;
      }
   }
   return(i);
}

/*********************************************************************//**
 * @brief		UART0 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void UART0_IRQHandler(void)
{
	uint32_t intsrc, tmp;
	uint8_t myreg8;


	while( 1 )	{
		/* Determine the interrupt source */
		intsrc = UART_GetIntId(LPC_UART0);
		tmp = intsrc & 0x07;

		switch( tmp )	{
			case 0x01 :
				return;
			case 0x04 :			//Rx Interrupt : RDA, Receive Data Avalaible
				UART0_IntReceive();
				break;
			case 0x02 :			//TX Interrupt, THRE
				UART0_IntTransmit();
				break;
			case 0x06 :
				myreg8 = UART_GetLineStatus(LPC_UART0);
				break;    //return
		}

	}

}

/*********************************************************************//**
 * @brief		UART2 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void UART2_IRQHandler(void)
{
	uint32_t intsrc, tmp;
	uint8_t myreg8;



	while( 1 )	{
		/* Determine the interrupt source */
		intsrc = UART_GetIntId(LPC_UART2);
		tmp = intsrc & 0x07;
		switch( tmp )	{
			case 0x01 :
				return;
			case 0x04 :			//Rx Interrupt : RDA, Receive Data Avalaible
				UART2_IntReceive();
				break;
			case 0x02 :			//TX Interrupt, THRE
				UART2_IntTransmit();
				break;
			case 0x06 :
				myreg8 = UART_GetLineStatus(LPC_UART2);
				break;
			default:
				break;
		}

	}

}


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		UART3 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void UART3_IRQHandler(void)
{
	uint32_t intsrc, tmp;
	uint8_t myreg8;

	while( 1 )	{
		/* Determine the interrupt source */
		intsrc = UART_GetIntId(LPC_UART3);
		tmp = intsrc & 0x07;
		switch( tmp )	{
			case 0x01 :
				return;
			case 0x04 :			//Rx Interrupt : RDA, Receive Data Avalaible
				UART3_IntReceive();
				break;
			case 0x02 :			//TX Interrupt, THRE
				UART3_IntTransmit();
				break;
			case 0x06 :
				myreg8 = UART_GetLineStatus(LPC_UART3);
				break;
		}

	}

}

/********************************************************************//**
 * @brief 		UART0 receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART0_IntReceive(void)
{
	uint8_t tmpc;
	uint32_t rLen;

		// Call UART read function in UART driver
		rLen = UART_Receive((LPC_UART_TypeDef *)LPC_UART0, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if(!(SysFlag4 & SERIALSTART_flag))
			return;
		if (rLen){
			CommPutRxChar(COMM0,tmpc);
		}
}

/********************************************************************//**
 * @brief 		UART2 receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART2_IntReceive(void)
{
	uint8_t tmpc;
	uint32_t rLen;

		// Call UART read function in UART driver
		rLen = UART_Receive((LPC_UART_TypeDef *)LPC_UART2, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen){
			CommPutRxChar(COMM2,tmpc);
		}
}


/********************************************************************//**
 * @brief 		UART3 receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART3_IntReceive(void)
{
	uint8_t tmpc;
	uint32_t rLen;

		// Call UART read function in UART driver
		rLen = UART_Receive((LPC_UART_TypeDef *)LPC_UART3, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen){
			CommPutRxChar(COMM3,tmpc);
		}
}

/********************************************************************//**
 * @brief 		UART0 transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART0_IntTransmit(void)
{
	unsigned char c, err;

    err = COMM_NO_ERR;
   	c = CommGetTxChar(COMM0,&err);
   	if( err == COMM_NO_ERR )	{
   		UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&c, 1, NONE_BLOCKING);
   	} else	{
   		CommSendIdle(COMM0);
   	}
 }

/********************************************************************//**
 * @brief 		UART2 transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART2_IntTransmit(void)
{
	unsigned char c, err;

    err = COMM_NO_ERR;
   	c = CommGetTxChar(COMM2,&err);
   	if( err == COMM_NO_ERR )	{
   		UART_Send((LPC_UART_TypeDef *)LPC_UART2, (uint8_t *)&c, 1, NONE_BLOCKING);
   	} else	{
   		CommSendIdle(COMM2);
   	}
 }


/********************************************************************//**
 * @brief 		UART3 transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART3_IntTransmit(void)
{
	unsigned char c, err;

    err = COMM_NO_ERR;
   	c = CommGetTxChar(COMM3,&err);
   	if( err == COMM_NO_ERR )	{
   		UART_Send((LPC_UART_TypeDef *)LPC_UART3, (uint8_t *)&c, 1, NONE_BLOCKING);
   	} else	{
   		CommSendIdle(COMM3);
   	}
 }

void InitUART3(void)
{
	// Pin configuration
	PINSEL_CFG_Type PinCfg;
	// UART Configuration structure variable
	UART_CFG_Type uartCfg;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	CPU_INT08U	err;

	CommInit(COMM3, &err);

	/* Initialize UART3 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate = 9600;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(LPC_UART3, &uartCfg);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART3, &UARTFIFOConfigStruct);

	UART_TxCmd(LPC_UART3, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_RBR, ENABLE);
	/* Enable UART Tx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_THRE, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART3_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(UART3_IRQn);
}

void InitUART_Debug(void)
{
	// Pin configuration
	PINSEL_CFG_Type PinCfg;
	// UART Configuration structure variable
	UART_CFG_Type uartCfg;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	CPU_INT08U	err;

	CommInit(COMM0, &err);

	/* Initialize UART0 pin connect */
	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate = 9600;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_EVEN;
	uartCfg.Stopbits = UART_STOPBIT_2;


	UART_Init(LPC_UART0, &uartCfg);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);

	UART_TxCmd(LPC_UART0, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RBR, ENABLE);
	/* Enable UART Tx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_THRE, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART0, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first timeexcu
	 */

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART0_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(UART0_IRQn);
}

void InitUART_LAN1(void)
{
	// Pin configuration
	PINSEL_CFG_Type PinCfg;
	// UART Configuration structure variable
	UART_CFG_Type uartCfg;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	CPU_INT08U	err;

	CommInit(COMM2, &err);

	/* Initialize UART2 pin connect */
	PinCfg.Funcnum = 1;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate = 9600;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(LPC_UART2, &uartCfg);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);

	UART_TxCmd(LPC_UART2, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART2, UART_INTCFG_RBR, ENABLE);
	/* Enable UART Tx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART2, UART_INTCFG_THRE, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART2, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART2_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(UART2_IRQn);
}

void InitUART_LAN2(void)
{
	// Pin configuration
	PINSEL_CFG_Type PinCfg;
	// UART Configuration structure variable
	UART_CFG_Type uartCfg;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	CPU_INT08U	err;

	CommInit(COMM3, &err);

	/* Initialize UART3 pin connect */
	PinCfg.Funcnum = 3;
	PinCfg.Pinnum = 28;
	PinCfg.Portnum = 4;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 29;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate = 19200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(LPC_UART3, &uartCfg);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART3, &UARTFIFOConfigStruct);

	UART_TxCmd(LPC_UART3, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_RBR, ENABLE);
	/* Enable UART Tx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_THRE, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART3, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART3_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(UART3_IRQn);
}
