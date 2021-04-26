/*
 * serial.h
 *
 *  Created on: Oct 17, 2011
 *      Author: IronMan
 */

#ifndef SERIAL_H_
#define SERIAL_H_


#define		COMM_RX_BUF_SIZE	64		//128
#define		COMM_TX_BUF_SIZE	64		//256
#define		COMM_RX_BUF_SIZE2	64

#define  COMM0                  0
#define  COMM1                  1
#define  COMM2                  2
#define  COMM3                  3
                                                 /* ERROR CODES                                        */
#define  COMM_NO_ERR            0                /* Function call was successful                       */
#define  COMM_BAD_CH            1                /* Invalid communications port channel                */
#define  COMM_RX_EMPTY          2                /* Rx buffer is empty, no character available         */
#define  COMM_TX_FULL           3                /* Tx buffer is full, could not deposit character     */
#define  COMM_TX_EMPTY          4                /* If the Tx buffer is empty.                         */
#define  COMM_RX_TIMEOUT        5                /* If a timeout occurred while waiting for a character*/
#define  COMM_TX_TIMEOUT        6                /* If a timeout occurred while waiting to send a char.*/

#define	DEBUG_PORT		COMM0
#define	LAN1_PORT		COMM2
#define	LAN2_PORT		COMM3


void InitUART_Debug(void);
void InitUART_LAN1(void);
void InitUART_LAN2(void);
void UART0_IntTransmit(void);
void UART0_IntReceive(void);
void UART2_IntTransmit(void);
void UART2_IntReceive(void);
void UART3_IntTransmit(void);
void UART3_IntReceive(void);
CPU_INT16U ComGetBuff(CPU_INT08U CommNum, CPU_INT16U Timeout,CPU_INT08U *RxBuff,int Len);
CPU_INT08U ComGetChar(int CommNum);
void  CommSendString(CPU_INT08U CommNum,char *Str);
void ComPutChar(CPU_INT08U CommNum,char c);
CPU_BOOLEAN  CommIsFull (CPU_INT08U ch);
CPU_BOOLEAN  CommIsEmpty (CPU_INT08U ch);
void  CommInit (CPU_INT08U ch, CPU_INT08U *err);
CPU_INT08U  CommGetTxChar (CPU_INT08U ch, CPU_INT08U *err);
CPU_INT08U  CommGetChar (CPU_INT08U ch, OS_TICK to, CPU_INT08U *err);
CPU_INT08U  CommPutChar (CPU_INT08U ch, CPU_INT08U c, CPU_INT16U to);
void InitUART3(void);

int COMMXwrFree( CPU_INT08U ch);
int COMMXwrUsed( CPU_INT08U ch);
int COMMXrdUsed( CPU_INT08U ch);
void  ComSendBuff(CPU_INT08U CommNum,CPU_INT08U *TxBuff,int Len);


#endif /* SERIAL_H_ */
