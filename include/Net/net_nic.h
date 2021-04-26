/*
*********************************************************************************************************
*                                              uC/TCP-IP
*                                      The Embedded TCP/IP Suite
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TCP-IP is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/TCP-IP in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to help
*               you experience uC/TCP-IP.  The fact that the source code is provided does
*               NOT mean that you can use it without paying a licensing fee.
*
*               Network Interface Card (NIC) port files provided, as is, for FREE and do
*               NOT require any additional licensing or licensing fee.
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
*                                        NETWORK INTERFACE CARD
*
*                                          LPC23XX/LPC24XX EMAC
*
* Filename      : net_nic.c
* Version       : V1.89
* Programmer(s) : EHS
*********************************************************************************************************
* Note(s)       : (1) Supports EMAC section of NXP's LPC23XX and LPC24XX microcontrollers
*                     as described in
*
*                         NXP Corporation (NXP; http://www.nxp.com).
*                         (a) "LPC2364/6/8/78 User Manual" (UM10211)    Revision 01 ( 6 October  2006)
*                         (b) "LPC2468 User Manual" (UM10237)           Revision 01 (18 December 2006)
*
*                 (2) REQUIREs Ethernet Network Interface Layer located in the following network directory :
*
*                         \<Network Protocol Suite>\IF\Ether\
*
*                              where
*                                      <Network Protocol Suite>    directory path for network protocol suite.
*
*                 (2) Since the EMAC module is integrated into either an LPC23XX or LPC24XX microcontroller,
*                     the endianness of the registers is the same as the CPU, which is little endian by
*                     default.
*********************************************************************************************************
*/

#ifndef _NET_NIC_H
#define _NET_NIC_H

/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   NET_NIC_MODULE
#define  NET_NIC_EXT
#else
#define  NET_NIC_EXT  extern
#endif

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  PHY_RDWR_RETRIES                  8                            /* Attempt to read or write the PHY 8 times             */

/*
*********************************************************************************************************
*                               NIC DRIVER LAYER / ETHERNET ERROR CODES
*
* Note(s) : (1) ALL NIC-independent    error codes #define'd in      'net_err.h';
*               ALL LPC2378-specific error codes #define'd in this 'net_nic.h'.
*
*           (2) Network error code '10,000' series reserved for NIC drivers.
*********************************************************************************************************
*/

#define  EMAC_ERR_RX_BUSY                         10700
#define  EMAC_ERR_TX_BUSY                         10701

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

NET_NIC_EXT  CPU_BOOLEAN  NetNIC_ConnStatus;                            /* NIC's connection status : DEF_ON/DEF_OFF.            */


#if (NET_CTR_CFG_STAT_EN == DEF_ENABLED)                                /* ------------------- NET DRV STATS ------------------ */
NET_NIC_EXT  NET_CTR      NetNIC_StatRxPktCtr;
NET_NIC_EXT  NET_CTR      NetNIC_StatTxPktCtr;
#endif

#if (NET_CTR_CFG_ERR_EN  == DEF_ENABLED)                                /* ------------------- NET DRV ERRS ------------------- */
NET_NIC_EXT  NET_CTR      NetNIC_ErrRxPktDiscardedCtr;
NET_NIC_EXT  NET_CTR      NetNIC_ErrTxPktDiscardedCtr;
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         NetNIC_Init             (NET_ERR      *perr);

void         NetNIC_IntEn            (NET_ERR      *perr);


                                                                        /* ------------------- STATUS FNCTS ------------------- */
CPU_BOOLEAN  NetNIC_ConnStatusGet    (void);

void         NetNIC_LinkChange      (CPU_INT32U link_speed,
                                     CPU_INT32U link_duplex);

void         NetNIC_ISR_Handler      (void);                            /* Decode & handle rx/tx ISRs.                          */

                                                                        /* --------------------- RX FNCTS --------------------- */
CPU_INT16U   NetNIC_RxPktGetSize     (void);                            /* Get NIC rx pkt size.                                 */

void         NetNIC_RxPkt            (void         *ppkt,               /* Rx pkt from NIC.                                     */
                                      CPU_INT16U    size,
                                      NET_ERR      *perr);

void         NetNIC_RxPktDiscard     (CPU_INT16U    size,               /* Discard rx pkt in NIC.                               */
                                      NET_ERR      *perr);


                                                                        /* --------------------- TX FNCTS --------------------- */
void         NetNIC_TxPkt            (void         *ppkt,
                                      CPU_INT16U    size,
                                      NET_ERR      *perr);

                                                                        /* ------------------- PHYTER FNCTS ------------------ */
void         NetNIC_PhyInit         (NET_ERR     *perr);

CPU_INT16U   NetNIC_PhyRegRd        (CPU_INT08U   phy,
                                     CPU_INT08U   reg,
                                     NET_ERR    *perr);

void         NetNIC_PhyRegWr        (CPU_INT08U   phy,
                                     CPU_INT08U   reg,
                                     CPU_INT16U   val,
                                     NET_ERR    *perr);

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                      DEFINED IN OS'S  net_os.c
*********************************************************************************************************
*/

void         NetOS_NIC_Init          (NET_ERR      *perr);              /* Create Drv objs & start Drv tasks.                   */


void         NetOS_NIC_TxRdyWait     (NET_ERR      *perr);              /* Wait on drv tx rdy signal from NIC.                  */

void         NetOS_NIC_TxRdySignal   (void);                            /* Post    drv tx rdy signal from NIC.                  */

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                    DEFINED IN PRODUCT'S  net_isr.c
*********************************************************************************************************
*/

#if (NET_NIC_CFG_INT_CTRL_EN == DEF_ENABLED)
void         NetNIC_IntInit          (void);                            /* Init int ctrl'r.                                     */
#endif

/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  NET_NIC_CFG_INT_CTRL_EN
#error  "NET_NIC_CFG_INT_CTRL_EN           not #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_ENABLED ]           "
#elif  ((NET_NIC_CFG_INT_CTRL_EN != DEF_ENABLED ))
#error  "NET_NIC_CFG_INT_CTRL_EN     illegally #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_ENABLED ]           "
#endif

#ifndef  NET_NIC_CFG_RD_WR_SEL
#error  "NET_NIC_CFG_RD_WR_SEL             not #define'd in 'net_cfg.h'"
#error  "                            [MUST be  NET_NIC_RD_WR_SEL_FNCT ]"
#error  "                            [     ||  NET_NIC_RD_WR_SEL_MACRO]"
#elif  ((NET_NIC_CFG_RD_WR_SEL != NET_NIC_RD_WR_SEL_FNCT ) && \
        (NET_NIC_CFG_RD_WR_SEL != NET_NIC_RD_WR_SEL_MACRO))
#error  "NET_NIC_CFG_RD_WR_SEL       illegally #define'd in 'net_cfg.h'"
#error  "                            [MUST be  NET_NIC_RD_WR_SEL_FNCT ]"
#error  "                            [     ||  NET_NIC_RD_WR_SEL_MACRO]"
#endif

#ifndef  EMAC_CFG_MAC_ADDR_SEL
#error  "EMAC_CFG_MAC_ADDR_SEL        not #define'd in 'net_bsp.h'      "
#error  "                            [MUST be  EMAC_CFG_MAC_ADDR_SEL_CFG]"
#elif   (EMAC_CFG_MAC_ADDR_SEL != EMAC_CFG_MAC_ADDR_SEL_CFG)
#error  "EMAC_CFG_MAC_ADDR_SEL  illegally #define'd in 'net_bsp.h'      "
#error  "                            [MUST be  EMAC_CFG_MAC_ADDR_SEL_CFG]"
#endif

#ifndef  EMAC_CFG_RMII
#error  "EMAC_CFG_RMII                not #define'd in 'net_bsp.h'      "
#error  "                            [MUST be  0 or 1]"
#elif  ((EMAC_CFG_RMII != 0   ) && \
        (EMAC_CFG_RMII != 1))
#error  "EMAC_CFG_RMII  illegally #define'd in 'net_bsp.h'      "
#error  "                            [MUST be  0 or 1]"
#endif

#ifndef  EMAC_NUM_RX_DESC
#error  "EMAC_NUM_RX_DESC must be defined in net_bsp.h"
#endif

#ifndef  EMAC_NUM_TX_DESC
#error  "EMAC_NUM_TX_DESC must be defined in net_bsp.h"
#endif

#if    ((EMAC_NUM_RX_DESC *  EMAC_RX_BUF_SIZE)  + \
        (EMAC_NUM_RX_DESC *   16)               + \
        (EMAC_NUM_TX_DESC *   12)               + \
        (EMAC_NUM_TX_DESC * 1536) > (16384))
#error "Not enough EMAC AHB2 RAM available to fit the specified"
#error "number of Rx / Tx descriptors, status arrays and Rx Buffers."
#error "Adjust net_bsp.h macros for EMAC_NUM_RX_DESC, EMAC_NUM_TX_DESC"
#error "and EMAC_NUM_RX_BUF accordingly."
#endif

#if      EMAC_NUM_TX_DESC < 2
#error   EMAC_NUM_TX_DESC must be configured >= 2
#endif

#if      EMAC_NUM_RX_DESC < 2
#error   EMAC_NUM_RX_DESC must be configured >= 2
#endif

#ifndef  NET_NIC_CFG_TX_PKT_PREPARE_EN
#error  "NET_NIC_CFG_TX_PKT_PREPARE_EN             not #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_DISABLED]"
#elif   (NET_NIC_CFG_TX_PKT_PREPARE_EN != DEF_DISABLED)
#error  "NET_NIC_CFG_TX_PKT_PREPARE_EN       illegally #define'd in 'net_cfg.h'"
#error  "                            [MUST be  DEF_DISABLED]"
#endif

extern CPU_INT16U phymode, phymodewarm;


#endif
