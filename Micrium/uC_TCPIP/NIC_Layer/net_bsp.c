/*
*********************************************************************************************************
*                                              uC/TCP-IP
*                                      The Embedded TCP/IP Suite
*
*                          (c) Copyright 2003-2005; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*
*                                             NXP LPC2378
*                                                on the
*                                    KEIL MCB2300 Evaluation Board
*
* Filename      : net_bsp.c
* Version       : V1.89
* Programmer(s) : Eric Shufro
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

// Standard Libraries
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

// CMSIS Drivers
#include "lpc17xx_gpio.h"
#include "lpc17xx_libcfg.h"

// Libraries
#include  <cpu.h>
#include  <cpu_core.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>

#include  <app_cfg.h>
#include  <bsp.h>
#include  <os.h>
#include  <os_csp.h>


#include  <net.h>
#include  <net_nic.h>
#include  <net_phy.h>
#include  <net_phy_def.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                  NETWORK INTERFACE CARD FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            NetNIC_LinkUp()
*
* Description : Message from NIC that the ethernet link is up.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : WARNING: Called in interruption context most of the time.
*********************************************************************************************************
*/

void  NetNIC_LinkUp (void)
{
    CPU_INT32U  link_speed;
    CPU_INT32U  link_duplex;


    link_speed              =   NetNIC_PhyLinkSpeed();                  /* Read the PHY's current link speed                    */
    link_duplex             =   NetNIC_PhyLinkDuplex();                 /* Read the PHY's current link duplex mode              */

    if (link_speed == NET_PHY_SPD_0 || link_duplex == NET_PHY_DUPLEX_UNKNOWN) {

        NetNIC_LinkChange(NET_PHY_SPD_0, NET_PHY_DUPLEX_UNKNOWN);       /* Inform the EMAC about the current PHY settings       */

    } else {

        NetNIC_LinkChange(link_speed,    link_duplex);                  /* Inform the EMAC about the current PHY settings       */
    }
}


/*
*********************************************************************************************************
*                                           NetNIC_LinkDown()
*
* Description : Message from NIC that the ethernet link is down.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : none.
*
* Note(s)     : WARNING: Called in interruption context most of the time.
*********************************************************************************************************
*/

void  NetNIC_LinkDown (void)
{
    CPU_INT32U  link_speed;
    CPU_INT32U  link_duplex;


    link_speed              =   NetNIC_PhyLinkSpeed();                  /* Read the PHY's current link speed                    */
    link_duplex             =   NetNIC_PhyLinkDuplex();                 /* Read the PHY's current link duplex mode              */

    if (link_speed == NET_PHY_SPD_0 || link_duplex == NET_PHY_DUPLEX_UNKNOWN) {

        NetNIC_LinkChange(NET_PHY_SPD_0, NET_PHY_DUPLEX_UNKNOWN);       /* Inform the EMAC about the current PHY settings       */

    } else {

        NetNIC_LinkChange(link_speed,    link_duplex);                  /* Inform the EMAC about the current PHY settings       */
    }
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                  Board Support Functions
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  RESET & INIT PHY HARDWARE
*
* Description : This function is called by EMAC_Init() to hardware reset the PHY
*             : and configure the IO pins necessary to communicate between the MAC and PHY
*
* Arguments   : none
*
* Return(s)   : none.
*
* Notes(s)    : (1) Unlike the AT91SAM7X-EK board, the PHY pin function include both peripheral A and
*                   peripheral B functions.
*               (2) The PHY PWRDWN pin is not under software control; instead, it is tied to ground.
*********************************************************************************************************
*/
#define  PINSEL2                      (*((volatile CPU_INT32U *)0x4002C008))
#define  PINSEL3                      (*((volatile CPU_INT32U *)0x4002C00C))

void  NetBSP_Phy_HW_Init (void)
{
    PINSEL2             =   0x50150105;	                                /* Selects P1[0,1,4,8,9,10,14,15]                           */
    PINSEL3             =   0x00000005;	                                /* Selects P1[17:16]                                        */
}

/*
*********************************************************************************************************
*                                        NetBSP_GetHostClkFreq()
*
* Description : This function returns the LPC2378 Host Clock (SYSCLK also named HCLK) frequency which is
*               used by the EMAC driver to select the MII Management interface clock divider.
*
* Arguments   : none
*
* Notes       : none
*********************************************************************************************************
*/

CPU_INT32U  NetBSP_GetHostClkFreq(void)
{
    CPU_INT32U  hclk_freq;

    SystemCoreClockUpdate();
    hclk_freq  =  SystemCoreClock;                                    /* Get the cClkFrq which feeds the EMAC module              */
    return (hclk_freq);
}

/*
*********************************************************************************************************
*                                        NetBSP_DlyMs()
*
* Description : This function is used by the NIC driver to create delays of an arbitrary abount
*               of milliseconds during initialization.
*
* Arguments   : none
*********************************************************************************************************
*/

void  NetBSP_DlyMs (CPU_INT32U ms)
{
    OS_ERR error;
    
    OSTimeDlyHMSM(0, 0, 0, ms, OS_OPT_TIME_HMSM_STRICT, &error);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                      NETWORK MODULE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          NetUtil_TS_Get()
*
* Description : Get current Internet Timestamp.
*
*               (1) "The Timestamp is a right-justified, 32-bit timestamp in milliseconds since midnight
*                    UT [Universal Time]" (RFC #791, Section 3.1 'Options : Internet Timestamp').
*
*               (2) The developer is responsible for providing a real-time clock with correct time-zone
*                   configuration to implement the Internet Timestamp.
*
*
* Argument(s) : none.
*
* Return(s)   : Internet Timestamp.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

NET_TS  NetUtil_TS_Get (void)
{
   NET_TS  ts;


   /* $$$$ Insert code to return Internet Timestamp (see Notes #1 & #2).   */

   ts = NET_TS_NONE;

   return (ts);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                            TRANSMISSION CONTROL PROTOCOL LAYER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NetTCP_InitTxSeqNbr()
*
* Description : Initialize the TCP Transmit Initial Sequence Counter, 'NetTCP_TxSeqNbrCtr'.
*
*               (1) Possible initialization methods include :
*
*                   (a) Time-based initialization is one preferred method since it more appropriately
*                       provides a pseudo-random initial sequence number.
*                   (b) Hardware-generated random number initialization is NOT a preferred method since it
*                       tends to produce a discrete set of pseudo-random initial sequence numbers--often
*                       the same initial sequence number.
*                   (c) Hard-coded initial sequence number is NOT a preferred method since it is NOT random.
*
*                   See also 'net_tcp.h  MACRO'S  Note #2'.
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetTCP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  NET_TCP_MODULE_PRESENT
void  NetTCP_InitTxSeqNbr (void)
{
   NetTCP_TxSeqNbrCtr = 5000;
}
#endif


/*
*********************************************************************************************************
*                                     NetTCP_TxRTT_GetTS()
*
* Description : Get a TCP Round-Trip Time (RTT) timestamp.
*
*               (1) (a) (1) Although RFC #2988, Section 4 states that "there is no requirement for the
*                           clock granularity G used for computing RTT measurements ... experience has
*                           shown that finer clock granularities (<= 100 msec) perform somewhat better
*                           than more coarse granularities".
*
*                       (2) (A) RFC #2988, Section 2.4 states that "whenever RTO is computed, if it is
*                               less than 1 second then the RTO SHOULD be rounded up to 1 second".
*
*                           (B) RFC #1122, Section 4.2.3.1 states that "the recommended ... RTO ... upper
*                               bound should be 2*MSL" where RFC #793, Section 3.3 'Sequence Numbers :
*                               Knowing When to Keep Quiet' states that "the Maximum Segment Lifetime
*                               (MSL) is ... to be 2 minutes".
*
*                               Therefore, the required upper bound is :
*
*                                   2 * MSL = 2 * 2 minutes = 4 minutes = 240 seconds
*
*                   (b) Therefore, the developer is responsible for providing a timestamp clock with
*                       adequate resolution to satisfy the clock granularity (see Note #1a1) & adequate
*                       range to satisfy the minimum/maximum RTO values (see Note #1a2).
*
*
* Argument(s) : none.
*
* Return(s)   : TCP RTT timestamp, in milliseconds.
*
* Caller(s)   : NetTCP_RxPktValidate(),
*               NetTCP_TxPktPrepareHdr().
*
* Note(s)     : (2) (a) To avoid timestamp calculation overflow, a maximum clock tick threshold value
*                       MUST be configured to truncate all clock tick values.
*
*                   (b) Also, since the clock tick integer will periodically overflow; the maximum
*                       threshold MUST be a multiple of the clock tick to avoid a discontinuity in
*                       the timestamp calculation when the clock tick integer overflows.
*********************************************************************************************************
*/

#ifdef  NET_TCP_MODULE_PRESENT
NET_TCP_TX_RTT_TS_MS  NetTCP_TxRTT_GetTS (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR                cpu_sr;
#endif
    NET_TCP_TX_RTT_TS_MS  rtt_ts_ms;
    CPU_INT32U            clk_tick;
    CPU_INT32U            clk_tick_th;


    CPU_CRITICAL_ENTER();
    clk_tick = (CPU_INT32U)OSTickCtr;
    CPU_CRITICAL_EXIT();

    if (OS_CFG_TICK_RATE_HZ > 0) {
        clk_tick_th  = (NET_TCP_TX_RTT_TS_MAX                       /* MUST cfg clk tick th ...                         */
                     /  DEF_TIME_NBR_mS_PER_SEC);                   /* ... as multiple of max ts (see Note #2b).        */
        clk_tick    %=  clk_tick_th;                                /* Truncate clk ticks to avoid ovf (see Note #2a).  */

        rtt_ts_ms    = (NET_TCP_TX_RTT_TS_MS)((clk_tick * DEF_TIME_NBR_mS_PER_SEC) / OS_CFG_TICK_RATE_HZ);

    } else {
        rtt_ts_ms    = (NET_TCP_TX_RTT_TS_MS)clk_tick;
    }


    return (rtt_ts_ms);
}
#endif

/*
*********************************************************************************************************
*                                     NetTCP_TxConnRTT_GetTS_ms()
*
* Description : Get a TCP Round-Trip Time (RTT) timestamp.
*
*               (1) (a) (1) Although RFC #2988, Section 4 states that "there is no requirement for the
*                           clock granularity G used for computing RTT measurements ... experience has
*                           shown that finer clock granularities (<= 100 msec) perform somewhat better
*                           than more coarse granularities".
*
*                       (2) (A) RFC #2988, Section 2.4 states that "whenever RTO is computed, if it is
*                               less than 1 second then the RTO SHOULD be rounded up to 1 second".
*
*                           (B) RFC #1122, Section 4.2.3.1 states that "the recommended ... RTO ... upper
*                               bound should be 2*MSL" where RFC #793, Section 3.3 'Sequence Numbers :
*                               Knowing When to Keep Quiet' states that "the Maximum Segment Lifetime
*                               (MSL) is ... to be 2 minutes".
*
*                               Therefore, the required upper bound is :
*
*                                   2 * MSL = 2 * 2 minutes = 4 minutes = 240 seconds
*
*                   (b) Therefore, the developer is responsible for providing a timestamp clock with
*                       adequate resolution to satisfy the clock granularity (see Note #1a1) & adequate
*                       range to satisfy the minimum/maximum RTO values (see Note #1a2).
*
*
* Argument(s) : none.
*
* Return(s)   : TCP RTT timestamp, in milliseconds.
*
* Caller(s)   : NetTCP_RxPktValidate(),
*               NetTCP_TxPktPrepareHdr().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  NET_TCP_MODULE_PRESENT
NET_TCP_TX_RTT_TS_MS  NetTCP_TxConnRTT_GetTS_ms (void)
{
    NET_TCP_TX_RTT_TS_MS  rtt_ts_ms;


    /* $$$$ Insert code to return TCP RTT timestamp (see Note #1).  */

    rtt_ts_ms = NET_TCP_TX_RTT_TS_NONE;

    return (rtt_ts_ms);
}
#endif

