/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                          (c) Copyright 2004-2009; Micrium, Inc.; Weston, FL               
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/OS-II is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/OS-II in a commercial    
*               product you need to contact Micrium to properly license its use in your     
*               product.  We provide ALL the source code for your convenience and to        
*               help you experience uC/OS-II.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                       APPLICATION CONFIGURATION
*
*                                      KEIL MCB1700 DEVELOPMENT KIT
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_PROBE_COM_EN                   DEF_DISABLED
#define  APP_CFG_SERIAL_EN                      DEF_DISABLED


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_OS_CFG_TASK_START_PRIO                     1u

//#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_PRIO               7u
#define  NET_OS_CFG_TMR_TASK_PRIO                       2u		//2
#define  NET_OS_CFG_IF_RX_TASK_PRIO						2u		//2

#define  APP_OS_CFG_TASK_1_PRIO                         4u
#define	TelnetServerTask_PRIO							15u
#define	ZoneScanTask_PRIO								10u
#define	FsmLedsTask_PRIO								11u
#define	AlarmDetectTask_PRIO							12u
#define	AutoprotectTask_PRIO							13u
#define	RabbitTask_PRIO									14u		//14
#define	LAN485_Task_PRIO								5u


#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                    288		//*

#define  APP_OS_CFG_TASK_1_STK_SIZE                     464		//*
#define	TelnetServerTask_STK_SIZE						1024	//*
#define	ZoneScanTask_STK_SIZE							136		//*
#define	FsmLedsTask_STK_SIZE							104		//*
#define	AlarmDetectTask_STK_SIZE						296		//*
#define	AutoprotectTask_STK_SIZE						104		//*
#define	RabbitTask_STK_SIZE								2000	//*2000
#define	LAN485_Task_STK_SIZE							700	//* 700


#define  NET_OS_CFG_TMR_TASK_STK_SIZE                   200u	//*
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_STK_SIZE         128u	//*
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE                 804		//*


/*
*********************************************************************************************************
*                                          TASK STACK SIZES LIMIT
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_START_STK_SIZE_LIMIT       (APP_CFG_TASK_START_STK_SIZE * (100u - APP_CFG_TASK_START_STK_SIZE_PCT_FULL))   / 100u

#define  APP_OS_CFG_TASK_1_STK_SIZE_PCT_FULL             90u
#define  APP_OS_CFG_TASK_1_STK_SIZE_LIMIT			(APP_OS_CFG_TASK_1_STK_SIZE * (100u - APP_OS_CFG_TASK_1_STK_SIZE_PCT_FULL)) / 100u
/*
*********************************************************************************************************
*                                          uC/LIB CONFIGURATION
*********************************************************************************************************
*/

#define  LIB_MEM_CFG_ARG_CHK_EXT_EN             DEF_ENABLED
#define  LIB_MEM_CFG_OPTIMIZE_ASM_EN            DEF_DISABLED
#define  LIB_MEM_CFG_ALLOC_EN                   DEF_DISABLED
//#define  LIB_MEM_CFG_HEAP_SIZE                    1u * 512u
//#define  LIB_MEM_CFG_HEAP_BASE_ADDR              0x20082000u
//#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

/*
*********************************************************************************************************
*                                    NO DHCP / STATIC CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_IP_ADDR_STR_THIS_HOST         "192.168.1.200"
#define  APP_CFG_IP_ADDR_STR_REMOTE_HOST       "192.168.1.203"
#define  APP_CFG_IP_ADDR_STR_NET_MASK          "255.255.255.0"
#define  APP_CFG_IP_ADDR_STR_DFLT_GATEWAY      "192.168.1.1"
#define  APP_CFG_IP_ADDR_STR_DNS_SRVR          "192.168.1.1"
#define  APP_CFG_IP_ADDR_STR_NTP_SRVR          "132.246.168.164"
#define  APP_CFG_CLK_UTC_OFFSET                 -18000
#define  APP_CFG_DFLT_HOST_NAME                "CSB637"


/*
*********************************************************************************************************
*                                               SERIAL
*
* Note(s) : (1) Configure SERIAL_CFG_MAX_NBR_IF to the number of interfaces (i.e., UARTs) that will be
*               present.
*
*           (2) Configure SERIAL_CFG_RD_BUF_EN to enable/disable read buffer functionality.  The serial
*               core stores received data in the read buffer until the user requests it, providing a
*               reliable guarantee against receive overrun.
*
*           (3) Configure SERIAL_CFG_WR_BUF_EN to enable/disable write buffer functionality.  The serial
*               core stores line driver transmit data in the write buffer while the serial interface is
*               transmitting application data.
*
*           (4) Configure SERIAL_CFG_ARG_CHK_EXT_EN to enable/disable extended argument checking
*               functionality.
*
*           (5) Configure SERIAL_CFG_TX_DESC_NBR to allow multiple transmit operations (i.e., Serial_Wr,
*               Serial_WrAsync) to be queued.
*********************************************************************************************************
*/


#define  SERIAL_CFG_MAX_NBR_IF                             2    /* See Note #1.                                         */

#define  SERIAL_CFG_RD_BUF_EN                    DEF_DISABLED   /* See Note #2.                                         */

#define  SERIAL_CFG_WR_BUF_EN                    DEF_DISABLED   /* See Note #3.                                         */

#define  SERIAL_CFG_ARG_CHK_EXT_EN               DEF_DISABLED   /* See Note #4.                                         */

#define  SERIAL_CFG_TX_DESC_NBR                            1    /* See Note #5.                                         */


/*
*********************************************************************************************************
*                                    uC/SERIAL APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_SERIAL_CFG_TRACE_EN                DEF_DISABLED
#define  APP_SERIAL_CFG_TRACE_PORT_NAME         "UART1"


/*
*********************************************************************************************************
*                                       uC/PROBE APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_PROBE_SERIAL_PORT_NAME         "UART0"


/*
*********************************************************************************************************
*                                       TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                   0
#define  TRACE_LEVEL_INFO                                  1
#define  TRACE_LEVEL_DBG                                   2

#include <cpu.h>
extern  void  App_SerPrintf (CPU_CHAR  *p_fmt, ...);

#define  APP_CFG_TRACE_LEVEL                    TRACE_LEVEL_OFF
#define  APP_CFG_TRACE                          App_SerPrintf

#define  APP_TRACE_INFO(x)                     ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                      ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)

/*
*********************************************************************************************************
*                                       Opciones de configuracion
*********************************************************************************************************
*/

#define	DEBUG_COMM		COMM0




#endif
