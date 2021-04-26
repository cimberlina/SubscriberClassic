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
* Filename      : net_bsp.h
* Version       : V1.89
* Programmer(s) : Eric Shufro
*********************************************************************************************************
*/

#ifndef _NET_BSP_H
#define _NET_BSP_H

/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef     NET_BSP_MODULE
#define    NET_BSP_EXT
#else
#define    NET_BSP_EXT  extern
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                               DEFINES
*
* Note(s) : 1) The LPC2378 does not support automatic loading of MAC address from external
*              EEPROM. Therefore, the macro EMAC_CFG_MAC_ADDR_SEL must be set to
*              EMAC_CFG_MAC_ADDR_SEL_CFG, and the user should take care of reading an
*              external EEPROM before calling Net_Init(). The data retrieved from the
*              EEPROM may optionally be copied into NetIF_MAC_Addr[]. Otherwise, a hard
*              coded MAC address may be set within NetIF_MAC_Addr[].
*
*           2) The LPC2378 only supports RMII. Future chips, e.g LPC24xx should support MII. Be
*              sure to set EMAC_CFG_RMII to 1 when using the LPC2378.
*
*           3) Consult hardware schematics and user manuals to determine the address of your PHY.
*              The Keil MCB2300 EVB has a pullup on PHY pin AN0, while AN[3:1] are pulled down.
*              Therefore, the address of the Nationap DP83848 PHY on the Keil MCB2300 is 0x01.
*
*           4) EMAC_NUM_RX_DESC will allocate the given number of DESCRIPTORS and BUFFERS. Buffer size
*              can be selected by means of adjusting the macro EMAC_RX_BUF_SIZE.
*
*               ** EMAC_RX_BUF_SIZE MUST BE SELECTED SUCH THAT IT IS DIVISIBLE BY 8
*                  TO ENSURE PACKET BOUNDRY ALIGNMENT!
*
*           5) EMAC_NUM_TX_DESC allocates the given number of DESCRIPTORS AND BUFFERS, however,
*              all buffers are of size 1536 bytes.
*********************************************************************************************************
*/

#define  EMAC_RAM_BASE_ADDR                   (0x20080000)              /* Base address of dedicated AHB2 Ethernet RAM              */

                                                                        /* Configure MAC address set method :                       */
#define  EMAC_CFG_MAC_ADDR_SEL         EMAC_CFG_MAC_ADDR_SEL_CFG        /* EMAC_CFG_MAC_ADDR_SEL_CFG    from application            */

#define  NEW_SILICON                                    0               /* For LPC2378 silicon revisions ending in AY set to 1      */
                                                                        /* Previous revisions, ending in -X or -Y set to 0          */
                                                                        /* When 0, RMII GPIO work around is enabled for old silicon */

#define  EMAC_CFG_RMII                                  1               /* Enable or disable RMII mode, 1 = enabled 0 = disabled    */

#define  EMAC_CFG_PHY_ADDR                              1               /* PHY Address                                              */
                                                                        /* AD0 has a pull-up and AD[4:1] have pull-downs            */
                                                                        /* therefore the address is 0x01                            */

#define  EMAC_RX_BUF_SIZE                            256               /* 1536 Configure the size of the Rx buffers                     */

#if NEW_SILICON > 0
#define  EMAC_NUM_RX_DESC                               8               /* Configure  8 Rx descriptors / buffers placed in AHB2 RAM */
#else
#define  EMAC_NUM_RX_DESC                               4               /* Configure  4 Rx descriptors to avoid old silicon errata  */
#endif

#define  EMAC_NUM_TX_DESC                               2               /* Configure  2 Tx descriptors / buffers placed in AHB2 RAM */

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


/*
*********************************************************************************************************
*                                              MACRO'S
*
* Note(s) : (1) When NET_NIC_CFG_RD_WR_SEL configured to NET_NIC_RD_WR_SEL_MACRO, implement NIC read/write
*               functionality with macro's.
*
*               (a) The developer MUST implement the NIC read/write macro functions with the correct number
*                   & type of arguments necessary to implement the NIC's read/write functionality.
*
*               (b) The developer SHOULD appropriately name the NIC read/write macro functions by appending
*                   the number of bits that the macro functions read/write to the end of the macro function
*                   name :
*
*                           NetNIC_Rd_xx()
*                           NetNIC_Wr_xx()
*
*                   For example, 16-bit read/write macro functions should be named :
*
*                           NetNIC_Rd_16()
*                           NetNIC_Wr_16()
*
*
*               See also 'net_bsp.c  NetNIC_Rd_xx()  Note #1'
*                      & 'net_bsp.c  NetNIC_Wr_xx()  Note #1'.
*********************************************************************************************************
*/

#if (NET_NIC_CFG_RD_WR_SEL != NET_NIC_RD_WR_SEL_FNCT)

#define  NetNIC_Rd_xx(...)

#define  NetNIC_Wr_xx(..., val)

#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        NetBSP_Phy_HW_Init      (void);
void        NetBSP_DlyMs            (CPU_INT32U ms);
CPU_INT32U  NetBSP_GetHostClkFreq   (void);

/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/




#endif
