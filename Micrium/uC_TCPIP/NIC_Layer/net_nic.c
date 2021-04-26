/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/




#define    NET_NIC_MODULE
#include  <net.h>
#include  <net_phy.h>
#include  <net_phy_def.h>
#include "eeprom_map.h"

/*
*********************************************************************************************************
*                                     NXP LPC1769 REGISTERS
*********************************************************************************************************
*/
#define	ETH_AUTO_NEG	0x01
#define	ETH_FULLD_100M	0x02
#define	ETH_FULLD_10M	0x04
#define	ETH_HALFD_100M	0x03
#define	ETH_HALFD_10M	0x05
#define	ETH_FD10M_DEF	0x06
#define	ETH_PHY_END		ETH_FD10M_DEF + 1

#define  PCONP                      (*((volatile CPU_INT32U *)0x400FC0C4))

CPU_INT16U phymode, phymodewarm;
/*
*********************************************************************************************************
*                                   NXP LPC1769x EMAC REGISTERS
*
* Note(s) : (1) See NXP LPC1769 documentation for register summary.
*
*           (2) Since the EMAC module is integrated into either a LPC1769 microcontroller,
*               the endianness of the registers is the same as the CPU, which is little endian by default.
*********************************************************************************************************
*/

/* EMAC Memory Buffer configuration for 16K Ethernet RAM. */
#define NUM_RX_FRAG         4           /* Num.of RX Fragments 4*1536= 6.0kB */
#define NUM_TX_FRAG         2           /* Num.of TX Fragments 3*1536= 4.6kB */
#define ETH_FRAG_SIZE       1536        /* Packet Fragment size 1536 Bytes   */

#define ETH_MAX_FLEN        1536        /* Max. Ethernet Frame Size          */

/* EMAC variables located in AHB SRAM bank 1*/
// Below is base address for first silicon
//#define RX_DESC_BASE        0x20004000
// Below is base address for production silicon
#define RX_DESC_BASE        0x2007C000			//0x2007c000
#define RX_STAT_BASE        (RX_DESC_BASE + NUM_RX_FRAG*8)
#define TX_DESC_BASE        (RX_STAT_BASE + NUM_RX_FRAG*8)
#define TX_STAT_BASE        (TX_DESC_BASE + NUM_TX_FRAG*8)
#define RX_BUF_BASE         (TX_STAT_BASE + NUM_TX_FRAG*4)
#define TX_BUF_BASE         (RX_BUF_BASE  + NUM_RX_FRAG*ETH_FRAG_SIZE)

/* Back-to-Back Inter-Packet-Gap Register */
#define IPGT_FULL_DUP       0x00000015  /* Recommended value for Full Duplex */
#define IPGT_HALF_DUP       0x00000012  /* Recommended value for Half Duplex */

/* Non Back-to-Back Inter-Packet-Gap Register */
#define IPGR_DEF            0x00000012  /* Recommended value                 */

/* Collision Window/Retry Register */
#define CLRT_DEF            0x0000370F  /* Default value                     */

/* PHY Support Register */
//#define SUPP_SPEED          0x00000100  /* Reduced MII Logic Current Speed   */


                                                                            /* ------------- MAC REGISTERS ---------------- */
#define  MAC1                       (*((volatile CPU_INT32U *)0x50000000))  /* MAC Configuration Register 1                 */
#define  MAC2                       (*((volatile CPU_INT32U *)0x50000004))  /* MAC Configuration Register 2                 */
#define  IPGT                       (*((volatile CPU_INT32U *)0x50000008))  /* Back-to-Back Inter-Packet-Gap Register       */
#define  IPGR                       (*((volatile CPU_INT32U *)0x5000000C))  /* Non Back-to-Back Inter-Packet-Gap Register   */
#define  CLRT                       (*((volatile CPU_INT32U *)0x50000010))  /* Collision Window/Retyr Register              */
#define  MAXF                       (*((volatile CPU_INT32U *)0x50000014))  /* Maximum Frame Register                       */
#define  SUPP                       (*((volatile CPU_INT32U *)0x50000018))  /* PHY Support Register                         */
#define  TEST                       (*((volatile CPU_INT32U *)0x5000001C))  /* Test Register                                */
#define  MCFG                       (*((volatile CPU_INT32U *)0x50000020))  /* MII Mgmt Configuration Register              */
#define  MCMD                       (*((volatile CPU_INT32U *)0x50000024))  /* MII Mgmt Command       Register              */
#define  MADR                       (*((volatile CPU_INT32U *)0x50000028))  /* MII Mgmt Address       Register              */
#define  MWTD                       (*((volatile CPU_INT32U *)0x5000002C))  /* MII Mgmt Write Data    Register              */
#define  MRDD                       (*((volatile CPU_INT32U *)0x50000030))  /* MII Mgmt Read  Data    Register              */
#define  MIND                       (*((volatile CPU_INT32U *)0x50000034))  /* MII Mgmt Indicators    Register              */
#define  SA0                        (*((volatile CPU_INT32U *)0x50000040))  /* Station Address 0 Register                   */
#define  SA1                        (*((volatile CPU_INT32U *)0x50000044))  /* Station Address 1 Register                   */
#define  SA2                        (*((volatile CPU_INT32U *)0x50000048))  /* Station Address 2 Register                   */

                                                                            /* ----------- CONTROL REGISTERS -------------- */
#define  COMMAND                    (*((volatile CPU_INT32U *)0x50000100))  /* Command Register                             */
#define  STATUS                     (*((volatile CPU_INT32U *)0x50000104))  /* Status Register                              */
#define  RXDESCRIPTOR               (*((volatile CPU_INT32U *)0x50000108))  /* Receive  Descriptor Base Address Register    */
#define  RXSTATUS                   (*((volatile CPU_INT32U *)0x5000010C))  /* Receive  Status     Base Address Register    */
#define  RXDESCRIPTORNUMBER         (*((volatile CPU_INT32U *)0x50000110))  /* Receive  Number of Descriptors   Register    */
#define  RXPRODUCEINDEX             (*((volatile CPU_INT32U *)0x50000114))  /* Receive  Produce Index           Register    */
#define  RXCONSUMEINDEX             (*((volatile CPU_INT32U *)0x50000118))  /* Receive  Consume Index           Register    */
#define  TXDESCRIPTOR               (*((volatile CPU_INT32U *)0x5000011C))  /* Transmit Descriptor Base Address Register    */
#define  TXSTATUS                   (*((volatile CPU_INT32U *)0x50000120))  /* Transmit Status     Base Address Register    */
#define  TXDESCRIPTORNUMBER         (*((volatile CPU_INT32U *)0x50000124))  /* Transmit Number of Descriptors   Register    */
#define  TXPRODUCEINDEX             (*((volatile CPU_INT32U *)0x50000128))  /* Transmit Produce Index           Register    */
#define  TXCONSUMEINDEX             (*((volatile CPU_INT32U *)0x5000012C))  /* Transmit Consume Index           Register    */
#define  TSV0                       (*((volatile CPU_INT32U *)0x50000158))  /* Transmit Status Vector 0 Register            */
#define  TSV1                       (*((volatile CPU_INT32U *)0x5000015C))  /* Transmit Status Vector 1 Register            */
#define  RSV                        (*((volatile CPU_INT32U *)0x50000160))  /* Receive  Status Vector   Register            */
#define  FLOWCONTROLCOUNTER         (*((volatile CPU_INT32U *)0x50000170))  /* Flow Control Counter Register                */
#define  FLOWCONTROLSTATUS          (*((volatile CPU_INT32U *)0x50000174))  /* Flow Control Status  Register                */

                                                                            /* ---------- RX FILTER REGISTERS ------------- */
#define  RXFILTERCTRL               (*((volatile CPU_INT32U *)0x50000200))  /* Receive Filter Control Register              */
#define  RXFILTERWOLSTATUS          (*((volatile CPU_INT32U *)0x50000204))  /* Receive Filter WoL Status Register           */
#define  RXFILTERWOLCLEAR           (*((volatile CPU_INT32U *)0x50000208))  /* Receive Filter WoL Clear  Register           */
#define  HASHFILTERL                (*((volatile CPU_INT32U *)0x50000210))  /* Hash Filter Table LSBs Register              */
#define  HASHFILTERH                (*((volatile CPU_INT32U *)0x50000214))  /* Hash Filter Table MSBs Register              */

                                                                            /* -------- MODULE CONTROL REGISTERS ---------- */
#define  INTSTATUS                  (*((volatile CPU_INT32U *)0x50000FE0))  /* Interrupt status  register                   */
#define  INTENABLE                  (*((volatile CPU_INT32U *)0x50000FE4))  /* Interrupt enable  register                   */
#define  INTCLEAR                   (*((volatile CPU_INT32U *)0x50000FE8))  /* Interrupt clear   register                   */
#define  INTSET                     (*((volatile CPU_INT32U *)0x50000FEC))  /* Interrupt set     register                   */
#define  POWERDOWN                  (*((volatile CPU_INT32U *)0x50000FF4))  /* Power-down        register                   */

/*
*********************************************************************************************************
*                                NXP LPC1769 EMAC REGISTER BITS
*********************************************************************************************************
*/
                                                                            /* ----------------- MAC1 bits ---------------- */
#define  MAC1_RX_ENABLE                             DEF_BIT_00
#define  MAC1_PASS_ALL_FRAMES                       DEF_BIT_01
#define  MAC1_RESET_TX                              DEF_BIT_08
#define  MAC1_RESET_MCS_TX                          DEF_BIT_09
#define  MAC1_RESET_RX                              DEF_BIT_10
#define  MAC1_RESET_MCS_RX                          DEF_BIT_11
#define  MAC1_RESET_SIM                             DEF_BIT_14
#define  MAC1_RESET_SOFT                            DEF_BIT_15

                                                                            /* ----------------- MAC2 bits ---------------- */
#define  MAC2_FULL_DUPLEX                           DEF_BIT_00
#define  MAC2_CRC_EN                                DEF_BIT_04
#define  MAC2_PAD_EN                                DEF_BIT_05

                                                                            /* ----------------- IPGT bits ---------------- */
#define  IPG_HALF_DUP                                 0x0012
#define  IPG_FULL_DUP                                 0x0015

                                                                            /* ----------------- SUPP bits ---------------- */
#define  SUPP_SPEED                                 DEF_BIT_08

                                                                            /* ----------------- MCFG bits ---------------- */
#define  MCFG_CLKSEL_DIV4                           0x00000000
#define  MCFG_CLKSEL_DIV6                           0x00000008
#define  MCFG_CLKSEL_DIV8                           0x0000000C
#define  MCFG_CLKSEL_DIV10                          0x00000010
#define  MCFG_CLKSEL_DIV14                          0x00000014
#define  MCFG_CLKSEL_DIV20                          0x00000018
#define  MCFG_CLKSEL_DIV28                          0x0000001C
#define  MCFG_CLKSEL_DIV36                          0x00000020
#define  MCFG_CLKSEL_DIV64                          0x0000003C
#define  MCFG_RESET_MII_MGMT                        DEF_BIT_15

                                                                            /* ----------------- MCMD bits ---------------- */
#define  MCMD_WRITE                                 DEF_BIT_NONE
#define  MCMD_READ                                  0x00000001				//DEF_BIT_00


                                                                            /* --------------- COMMAND bits --------------- */
#define  COMMAND_RX_EN                              DEF_BIT_00
#define  COMMAND_TX_EN                              DEF_BIT_01
#define  COMMAND_RESET_REG                          DEF_BIT_03
#define  COMMAND_RESET_TX                           DEF_BIT_04
#define  COMMAND_RESET_RX                           DEF_BIT_05
#define  COMMAND_PASS_RUNT_FRAMES                   DEF_BIT_06
#define  COMMAND_PASS_ALL_FRAMES                    DEF_BIT_07
#define  COMMAND_RMII                               DEF_BIT_09
#define  COMMAND_FULL_DUPLEX                        DEF_BIT_10
#define  COMMAND_PASS_RX_FILT						0x00000080

                                                                            /* -------------- INTENABLE bits -------------- */
                                                                            /* -------------- INTSTATUS bits -------------- */
                                                                            /* -------------- INTCLEAR  bits -------------- */
                                                                            /* -------------- INTSET    bits -------------- */
#define  INT_RX_OVERRUN                             DEF_BIT_00
#define  INT_RX_ERROR                               DEF_BIT_01
#define  INT_RX_FINISHED                            DEF_BIT_02
#define  INT_RX_DONE                                DEF_BIT_03
#define  INT_TX_UNDERRUN                            DEF_BIT_04
#define  INT_TX_ERROR                               DEF_BIT_05
#define  INT_TX_FINISHED                            DEF_BIT_06
#define  INT_TX_DONE                                DEF_BIT_07
#define  INT_SOFT                                   DEF_BIT_12
#define  INT_WAKEUP                                 DEF_BIT_13

                                                                            /* ------------- RXFILTERCTRL bits ------------ */
#define  ACCEPT_BROADCAST                           DEF_BIT_01
#define  ACCEPT_PERFECT                             DEF_BIT_05

#define MII_WR_TOUT         0x00050000  /* MII Write timeout count           */
#define MII_RD_TOUT         0x00050000  /* MII Read timeout count            */

/* MII Management Address Register */
#define MADR_REG_ADR        0x0000001F  /* MII Register Address Mask         */
#define MADR_PHY_ADR        0x00001F00  /* PHY Address Mask                  */

/* MII Management Indicators Register */
#define MIND_BUSY           0x00000001  /* MII is Busy                       */
#define MIND_SCAN           0x00000002  /* MII Scanning in Progress          */
#define MIND_NOT_VAL        0x00000004  /* MII Read Data not valid           */
#define MIND_MII_LINK_FAIL  0x00000008  /* MII Link Failed                   */


/*
*********************************************************************************************************
*                      DESCRIPTOR CONTROL AND STATUS BIT DEFINITIONS
*********************************************************************************************************
*/

#define  EMAC_TX_DESC_INT                       0x80000000              /* EMAC Descriptor Tx and Rx Control bits                   */
#define  EMAC_TX_DESC_LAST                      0x40000000
#define  EMAC_TX_DESC_CRC                       0x20000000
#define  EMAC_TX_DESC_PAD                       0x10000000
#define  EMAC_TX_DESC_HUGE                      0x08000000
#define  EMAC_TX_DESC_OVERRIDE                  0x04000000

#define  EMAC_RX_DESC_INT                       0x80000000

#define  TX_DESC_STATUS_ERR                     0x80000000              /* EMAC Tx Status bits                                      */
#define  TX_DESC_STATUS_NODESC                  0x40000000
#define  TX_DESC_STATUS_UNDERRUN                0x20000000
#define  TX_DESC_STATUS_LCOL                    0x10000000
#define  TX_DESC_STATUS_ECOL                    0x08000000
#define  TX_DESC_STATUS_EDEFER                  0x04000000
#define  TX_DESC_STATUS_DEFER                   0x02000000
#define  TX_DESC_STATUS_COLCNT                  0x01E00000	

#define  RX_DESC_STATUS_ERR	                    0x80000000              /* EMAC Rx Status bits                                      */
#define  RX_DESC_STATUS_LAST                    0x40000000
#define  RX_DESC_STATUS_NODESC                  0x20000000
#define  RX_DESC_STATUS_OVERRUN	                0x10000000
#define  RX_DESC_STATUS_ALGNERR	                0x08000000
#define  RX_DESC_STATUS_RNGERR                  0x04000000
#define  RX_DESC_STATUS_LENERR                  0x02000000
#define  RX_DESC_STATUS_SYMERR                  0x01000000
#define  RX_DESC_STATUS_CRCERR                  0x00800000
#define  RX_DESC_STATUS_BCAST                   0x00400000
#define  RX_DESC_STATUS_MCAST                   0x00200000
#define  RX_DESC_STATUS_FAILFLT                 0x00100000
#define  RX_DESC_STATUS_VLAN                    0x00080000
#define  RX_DESC_STATUS_CTLFRAM	                0x00040000
#define  RX_DESC_STATUS_SIZE_MASK               0x000007FF
                                                                        /* Determine addresses of descriptor lists                  */
#define  EMAC_RX_DESC_BASE_ADDR                (EMAC_RAM_BASE_ADDR)
#define  EMAC_RX_STATUS_BASE_ADDR              (EMAC_RX_DESC_BASE_ADDR   + (EMAC_NUM_RX_DESC * sizeof(EMAC_DESCRIPTOR)))
#define  EMAC_TX_DESC_BASE_ADDR                (EMAC_RX_STATUS_BASE_ADDR + (EMAC_NUM_RX_DESC * sizeof(RX_STATUS)))
#define  EMAC_TX_STATUS_BASE_ADDR              (EMAC_TX_DESC_BASE_ADDR   + (EMAC_NUM_TX_DESC * sizeof(EMAC_DESCRIPTOR)))
#define  EMAC_RX_BUFF_BASE_ADDR                (EMAC_TX_STATUS_BASE_ADDR + (EMAC_NUM_TX_DESC * sizeof(TX_STATUS)))
#define  EMAC_TX_BUFF_BASE_ADDR                (EMAC_RX_BUFF_BASE_ADDR   + (EMAC_NUM_RX_DESC * EMAC_RX_BUF_SIZE))

/* Receive Filter Control Register */
#define RFC_UCAST_EN        0x00000001  /* Accept Unicast Frames Enable      */
#define RFC_BCAST_EN        0x00000002  /* Accept Broadcast Frames Enable    */
#define RFC_MCAST_EN        0x00000004  /* Accept Multicast Frames Enable    */
#define RFC_UCAST_HASH_EN   0x00000008  /* Accept Unicast Hash Filter Frames */
#define RFC_MCAST_HASH_EN   0x00000010  /* Accept Multicast Hash Filter Fram.*/
#define RFC_PERFECT_EN      0x00000020  /* Accept Perfect Match Enable       */
#define RFC_MAGP_WOL_EN     0x00001000  /* Magic Packet Filter WoL Enable    */
#define RFC_PFILT_WOL_EN    0x00002000  /* Perfect Filter WoL Enable         */

/* Receive Filter WoL Status/Clear Registers */
#define WOL_UCAST           0x00000001  /* Unicast Frame caused WoL          */
#define WOL_BCAST           0x00000002  /* Broadcast Frame caused WoL        */
#define WOL_MCAST           0x00000004  /* Multicast Frame caused WoL        */
#define WOL_UCAST_HASH      0x00000008  /* Unicast Hash Filter Frame WoL     */
#define WOL_MCAST_HASH      0x00000010  /* Multicast Hash Filter Frame WoL   */
#define WOL_PERFECT         0x00000020  /* Perfect Filter WoL                */
#define WOL_RX_FILTER       0x00000080  /* RX Filter caused WoL              */
#define WOL_MAG_PACKET      0x00000100  /* Magic Packet Filter caused WoL    */

/*
*********************************************************************************************************
*                                          DATA TYPES
*********************************************************************************************************
*/

typedef  struct  emac_descriptor {                                      /* EMAC Descriptor                                          */
    CPU_INT32U  PacketAddr;	                                            /* DMA Buffer Address                                       */
    CPU_INT32U  Control;                                                /* DMA Control bits                                         */
}   EMAC_DESCRIPTOR;

typedef  struct  rx_status {                                            /* Rx Status data type                                      */
    CPU_INT32U  StatusInfo;                                             /* Status information                                       */
    CPU_INT32U  StatusHashCRC;                                          /* Status Hash CRC                                          */
}   RX_STATUS;

typedef  struct  tx_status {                                            /* Rx Status data type                                      */
    CPU_INT32U  StatusInfo;                                             /* Status information                                       */
}   TX_STATUS;

CPU_INT32U	NIC_timer;
/*
*********************************************************************************************************
*                                      GLOBAL VARIABLES
*********************************************************************************************************
*/

//static  CPU_INT08U        MII_Dividers [7][2] =  {{4,  MCFG_CLKSEL_DIV4},
//                                                  {6,  MCFG_CLKSEL_DIV6},
//                                                  {8,  MCFG_CLKSEL_DIV8},
//                                                  {10, MCFG_CLKSEL_DIV10},
//                                                  {14, MCFG_CLKSEL_DIV14},
//                                                  {20, MCFG_CLKSEL_DIV20},
//                                                  {28, MCFG_CLKSEL_DIV28}};

static  EMAC_DESCRIPTOR  *RxDesc;
static  EMAC_DESCRIPTOR  *TxDesc;
static  RX_STATUS        *RxStatus;
static  TX_STATUS        *TxStatus;
static  CPU_INT08U       *RxBufBaseAddr;
static  CPU_INT08U       *TxBufBaseAddr;

static  CPU_INT32U        NIC_RxNRdyCtr;

/*
*********************************************************************************************************
*                                 LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void        NetNIC_TxPktDiscard      (NET_ERR      *perr);


                                                                    /* ----------- LPC17xx EMAC FNCTS ------------- */

static  void        EMAC_Init                (NET_ERR      *perr);


                                                                    /* ---------- LPC17xx EMAC RX FNCTS ----------- */

static  void        EMAC_RxEn                (void);
static  void        EMAC_RxDis               (void);


static  void        EMAC_RxIntEn             (void);

static  void        EMAC_RxPkt               (void         *ppkt,
                                              CPU_INT16U    size,
                                              NET_ERR      *perr);

static  void        EMAC_RxPktDiscard        (CPU_INT16U    size);

                                                                    /* ---------- LPC17xx EMAC TX FNCTS ----------- */

static  void        EMAC_TxEn                (void);
static  void        EMAC_TxDis               (void);


static  void        EMAC_TxPkt               (void         *ppkt,
                                              CPU_INT16U    size,
                                              NET_ERR      *perr);

static  CPU_INT16U  NIC_RxGetNRdy            (void);

/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            NetNIC_Init()
*
* Description : (1) Initialize Network Interface Card :
*
*                   (a) Perform NIC Layer OS initialization
*                   (b) Initialize NIC status
*                   (c) Initialize NIC statistics & error counters
*                   (d) Initialize EMAC
*
*
* Argument(s) : perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                    Network interface card successfully initialized.
*
*                                                                   -------- RETURNED BY NetOS_NIC_Init() : --------
*                               NET_OS_ERR_INIT_NIC_TX_RDY          NIC transmit ready signal NOT successfully
*                                                                       initialized.
*                               NET_OS_ERR_INIT_NIC_TX_RDY_NAME     NIC transmit ready name   NOT successfully
*                                                                       configured.
* Return(s)   : none.
*
* Caller(s)   : Net_Init().
*
* Note(s)     : (1) This function calls EMAC_Init() which initializes the LPC1769 hardware.
*********************************************************************************************************
*/

void  NetNIC_Init (NET_ERR  *perr)
{                                                                       /* --------------- PERFORM NIC/OS INIT -------------------- */
    NetOS_NIC_Init(perr);                                               /* Create NIC objs.                                         */
    if (*perr != NET_OS_ERR_NONE) {
         return;
    }
                                                                        /* ----------------- INIT NIC STATUS ---------------------- */
    NetNIC_ConnStatus           =  DEF_OFF;
                                                                        /* ------------- INIT NIC STAT & ERR CTRS ----------------- */
#if (NET_CTR_CFG_STAT_EN        == DEF_ENABLED)
    NetNIC_StatRxPktCtr         =  0;
    NetNIC_StatTxPktCtr         =  0;
#endif

#if (NET_CTR_CFG_ERR_EN         == DEF_ENABLED)
    NetNIC_ErrRxPktDiscardedCtr =  0;
    NetNIC_ErrTxPktDiscardedCtr =  0;
#endif
                                                                        /* ----------------- INIT LPC1769 EMAC -------------------- */
    EMAC_Init(perr);
}

/*
*********************************************************************************************************
*                                            NetNIC_IntEn()
*
* Description : Enable NIC interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Net_Init().
*********************************************************************************************************
*/

void  NetNIC_IntEn (NET_ERR  *perr)
{
    EMAC_RxIntEn();                                                     /* Enable Rx interrupts                                     */

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                        NetNIC_ConnStatusGet()
*
* Description : Get NIC's network connection status.
*
*               (1) Obtaining the NIC's network connection status is encapsulated in this function for the
*                   possibility that obtaining a NIC's connection status requires a non-trivial procedure.
*
*
* Argument(s) : none.
*
* Return(s)   : NIC network connection status :
*
*                   DEF_OFF     Network connection DOWN.
*                   DEF_ON      Network connection UP.
*
* Caller(s)   : NetIF_Pkt_Tx().
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_ConnStatusGet (void)
{
    return (NetNIC_ConnStatus);
}

/*
*********************************************************************************************************
*                                        EMAC Link Settings Update
*
* Description : This function is called by NetNIC_Init and the PHY ISR in order to update the
*             : speed and duplex settings for the EMAC.
*
* Arguments   : link_speed      Indicates link speed.  This can be one of
*                                   NET_PHY_SPD_0
*                                   NET_PHY_SPD_10
*                                   NET_PHY_SPD_100
*                                   NET_PHY_SPD_1000
*
*               link_duplex     Indicates link duplex.  This can be one of
*                                   NET_PHY_DUPLEX_UNKNOWN
*                                   NET_PHY_DUPLEX_HALF
*                                   NET_PHY_DUPLEX_FULL
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  NetNIC_LinkChange (CPU_INT32U link_speed, CPU_INT32U link_duplex)
{
    switch (link_speed) {
        case NET_PHY_SPD_0:                                             /* Assume 10Mbps operation until linked                     */
        case NET_PHY_SPD_10:
             SUPP      &=  ~SUPP_SPEED;                                 /* Configure the RMII logic (if used) for 10MBps operation  */
             break;

        case NET_PHY_SPD_100:
             SUPP      |=   SUPP_SPEED;                                 /* Configure the RMII logic (if uses) for 100MBps operation */
             break;
    }

    switch (link_duplex) {
        case NET_PHY_DUPLEX_UNKNOWN:                                    /* Assume half duplex until link duplex is known            */
        case NET_PHY_DUPLEX_HALF:
             MAC2      &=  ~MAC2_FULL_DUPLEX;                           /* Configure the EMAC to run in HALF duplex mode            */
             COMMAND   &=  ~COMMAND_FULL_DUPLEX;                        /* Configure the MII logic for a Half Duplex PHY Link       */
             IPGT       =   IPG_HALF_DUP;	                            /* Set inter packet gap to the recommended Half Duplex      */
             break;

        case NET_PHY_DUPLEX_FULL:
             MAC2      |=   MAC2_FULL_DUPLEX;                           /* Configure the EMAC to run in FULL duplex mode            */
             COMMAND   |=   COMMAND_FULL_DUPLEX;                        /* Configure the MII logic for a Full Duplex PHY Link       */
             IPGT       =   IPG_FULL_DUP;	                            /* Set inter packet gap to the recommended Full Duplex      */
             break;
    }
}


/*
*********************************************************************************************************
*                                         NetNIC_ISR_Handler()
*
* Description : (1) Decode ISR & call appropriate ISR handler :
*
*                   (a) Receive Buffer Not Available ISR     NetNIC_RxPktDiscard().
*                   (b) Receive  ISR                         NetNIC_RxISR_Handler().
*                   (c) Transmit ISR                         NetNIC_TxISR_Handler().
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR
*
* Note(s)     : 1) VIC interrupts do not need to be explicity acknowledged if the BSP exception
*                  handler writes a 0x00 to the VICAddress register after calling the ISR handler.
*                  User ISR handlers (such as this one) need only clear the local interrupt source.
*********************************************************************************************************
*/

void  NetNIC_ISR_Handler (void)
{
    CPU_INT32U  intstatus;
    CPU_INT16U  n_new;
    NET_ERR     err;


    intstatus               =  INTSTATUS;                               /* Read the interrupt status register                       */

    if ((intstatus & INT_RX_DONE) > 0) {                                /* If a receiver event has occured                          */
        n_new               =  NIC_RxGetNRdy() - NIC_RxNRdyCtr;         /* Determine how many NEW franes have been received         */
        while (n_new > 0) {
            NetOS_IF_RxTaskSignal(&err);                                /* Signal Net IF Rx Task that a new frame has arrived       */
            switch (err) {                                              /* Each frame is processed sequentially from the Rx task    */
                case NET_IF_ERR_NONE:
                    if (NIC_RxNRdyCtr < EMAC_NUM_RX_DESC) {
                         NIC_RxNRdyCtr++;
                     }
                     break;

                case NET_IF_ERR_RX_Q_FULL:
                case NET_IF_ERR_RX_Q_POST_FAIL:
                default:
                     NetNIC_RxPktDiscard(0, &err);                      /* If an error occured while signaling the task, discard    */
                     break;                                             /* the received frame                                       */
            }

            n_new--;
        }
        INTCLEAR            = (INT_RX_DONE);                            /* Clear the interrupt flags                                */
    }

    if ((intstatus & (INT_RX_OVERRUN)) > 0) {                           /* If a fator Overrun error has occured                     */
        INTCLEAR            = (INT_RX_OVERRUN);                         /* Clear the overrun interrupt flag                         */
        COMMAND            |=  COMMAND_RESET_RX;                        /* Soft reset the Rx datapath, this disables the receiver   */
        COMMAND            |=  COMMAND_RX_EN;                           /* Re-enable the reciever                                   */
        MAC1               |=  MAC1_RX_ENABLE;                          /* Re-enable the reciever                                   */
    }
}

/*
*********************************************************************************************************
*                                         NetNIC_RxPktGetSize()
*
* Description : Get network packet size from NIC.
*
* Argument(s) : none.
*
* Return(s)   : Size, in octets, of next frame to be read from the EMAC buffers
*
* Caller(s)   : NetIF_RxTaskHandler()
*
* Notes       : None.
*********************************************************************************************************
*/

CPU_INT16U  NetNIC_RxPktGetSize (void)
{
    CPU_INT16S   size;
    CPU_INT32U   rxstatus;


    rxstatus        =  RxStatus[RXCONSUMEINDEX].StatusInfo;             /* Accquire the status word for this desciptor              */

    rxstatus       &= (RX_DESC_STATUS_NODESC    |                       /* Obtain the descriptor error bits                         */
                       RX_DESC_STATUS_OVERRUN   |
                       RX_DESC_STATUS_ALGNERR   |
                       RX_DESC_STATUS_LENERR    |
                       RX_DESC_STATUS_SYMERR    |
                       RX_DESC_STATUS_CRCERR    |
                       RX_DESC_STATUS_FAILFLT);

    if (rxstatus > 0) {                                                 /* If any receive errors (except range error) occured       */
        size        =  0;                                               /* then return 0 so upper layer call NetNIC_RxPktDiscard()  */
    } else {
        size        =  RxStatus[RXCONSUMEINDEX].StatusInfo & 0x7FF;     /* Obtain the fragment size from the status struct pointer  */
        size       -=  3;                                               /* +1 since the size is 0 based, -4 to ignore FCS           */
    }

    if (size < 0) {                                                     /* Ensure that the subtraction didnt cause an underflow     */
        size = 0;
    }

    return (size);                                                      /* Return the size of the current frame                     */
}

/*
*********************************************************************************************************
*                                            NetNIC_RxPkt()
*
* Description : Read network packet from NIC into buffer.
*
* Argument(s) : ppkt        Pointer to memory buffer to receive NIC packet.
*
*               size        Number of packet frame octets to read into buffer.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully read.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*                               NET_NIC_ERR_NULL_PTR        Argument 'ppkt' passed a NULL pointer.
*                               NET_NIC_ERR_INVALID_SIZE    Invalid size.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_RxTaskHandler().
*
* Note(s)     : 1) NetNIC_RxPkt() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_RxPkt (void        *ppkt,
                    CPU_INT16U   size,
                    NET_ERR     *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit rx (see Note #2).         */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

#if (NET_ERR_CFG_ARG_CHK_DBG_EN == DEF_ENABLED)                         /* ------------------- VALIDATE PTR ------------------  */
    if (ppkt == (void *)0) {
       *perr = NET_NIC_ERR_NULL_PTR;
        return;
    }

                                                                        /* ------------------- VALIDATE SIZE -----------------  */
    if (size < NET_IF_FRAME_MIN_SIZE) {
       *perr = NET_NIC_ERR_INVALID_SIZE;
        return;
    }
#endif

    EMAC_RxPkt(ppkt, size, perr);                                       /* Read the received Frame from the EMAC buffers        */
    if (*perr != NET_NIC_ERR_NONE) {
        return;
    }

    NET_CTR_STAT_INC(NetNIC_StatRxPktCtr);

    *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                         NetNIC_RxPktDiscard()
*
* Description : Discard network packet from NIC to free NIC packet frames for new receive packets.
*
* Argument(s) : size        Number of packet frame octets.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE            Packet successfully discarded.
*                               NET_ERR_INIT_INCOMPLETE     Network initialization NOT complete.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_RxTaskHandler().
*
* Note(s)     : (1) NetNIC_RxPktDiscard() blocked until network initialization completes; perform NO action.
*
*               (2) #### 'perr' may NOT be necessary (remove before product release if unnecessary).
*********************************************************************************************************
*/

void  NetNIC_RxPktDiscard (CPU_INT16U   size,
                           NET_ERR     *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif

    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit discard (see Note #1).    */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }

    EMAC_RxPktDiscard(size);

    CPU_CRITICAL_ENTER();                                               /* This routine alters shared data. Disable interrupts!     */
    if (NIC_RxNRdyCtr > 0) {                                            /* One less packet to process                               */
        NIC_RxNRdyCtr--;
    }
    CPU_CRITICAL_EXIT();                                                /* Re-Enable interrupts                                     */

    NET_CTR_ERR_INC(NetNIC_ErrRxPktDiscardedCtr);

   *perr = NET_NIC_ERR_NONE;
}

/*
*********************************************************************************************************
*                                             NetNIC_TxPkt()
*
* Description : Transmit data packets from network driver layer to network interface card.
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*               ----        Argument validated in NetIF_Pkt_Tx().
*
*               size        Number of packet frame octets to write to frame.
*               ----        Argument checked   in NetIF_Tx().
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
*                               NET_ERR_INIT_INCOMPLETE         Network initialization NOT complete.
*
*                                                               - RETURNED BY NetNIC_TxPktDiscard() : -
*                               NET_ERR_TX                      Transmit error; packet discarded.
*
* Return(s)   : none.
*
* Caller(s)   : NetIF_Pkt_Tx().
*
* Note(s)     : (1) NetNIC_TxPkt() blocked until network initialization completes; perform NO action.
*********************************************************************************************************
*/

void  NetNIC_TxPkt (void        *ppkt,
                    CPU_INT16U   size,
                    NET_ERR     *perr)
{
#if ((NET_CTR_CFG_STAT_EN     == DEF_ENABLED)                    && \
     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL))
    CPU_SR  cpu_sr;
#endif


    if (Net_InitDone != DEF_YES) {                                      /* If init NOT complete, exit tx (see Note #1).             */
       *perr = NET_ERR_INIT_INCOMPLETE;
        return;
    }


    EMAC_TxPkt(ppkt, size, perr);                                       /* Hardware transmit routi                                  */

    if (*perr != NET_NIC_ERR_NONE) {
        NetNIC_TxPktDiscard(perr);
        return;
    }

    NET_CTR_STAT_INC(NetNIC_StatTxPktCtr);
}

/*
*********************************************************************************************************
*                                        NetNIC_PhyRegRd()
*
* Description : (1) Read PHY register
*
* Argument(s) : phy         PHY address, normally 0.
*               reg         PHY register.
*               perr        Pointer to variable that will hold the return error code from this function
*
* Return(s)   : MRDD        PHY register data.
*
* Caller(s)   : EMAC_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16U  NetNIC_PhyRegRd (CPU_INT08U  phy,
                             CPU_INT08U  reg,
                             NET_ERR    *perr)
{
    CPU_INT08U  retries;
    volatile  CPU_INT32U  i;


    retries     =   0;                                                  /* Initialize retries to 0                                  */

    MCMD        =   0;                                                  /* Clear the Read COMMAND                                   */
    MADR        =  (phy << 8) | reg;	                                /* [12:8] == PHY addr, [4:0]=0x00(BMCR) register addr       */
    MCMD        =   MCMD_READ;                                          /* Issue a Read COMMAND                                     */

    while ((MIND != 0) && (retries < PHY_RDWR_RETRIES)) {               /* Read the Management Indicator register, MII busy if > 0  */
    
        for (i = 0; i < 1000; i++) {                                    /* Delay while the read is in progress                      */
            ;
        }
        retries++;
    }

    if (retries >= PHY_RDWR_RETRIES) {                                  /* If there are no retries remaining                        */
       *perr     = NET_PHY_ERR_REGRD_TIMEOUT;                           /* Return an error code if the PHY Read timed out           */
    } else {
       *perr     = NET_PHY_ERR_NONE;
    }

    return (MRDD);                                                      /* Return the content of the Management Read Data register  */
}

/*
*********************************************************************************************************
*                                        NetNIC_PhyRegWr()
*
* Description : (1) Write PHY register
*
* Argument(s) : phy             PHY address, normally 0.
*               reg             PHY register.
*               val             Data to write to PHY register.
*               perr            Pointer to variable that will hold the return error code from this function
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetNIC_PhyRegWr (CPU_INT08U  phy,
                       CPU_INT08U  reg,
                       CPU_INT16U  val,
                       NET_ERR    *perr)
{
              CPU_INT08U  retries;
    volatile  CPU_INT32U  i;


    retries     =   0;                                                  /* Initialize retries to 0                                  */

    MCMD        =   MCMD_WRITE;                                         /* Issue a Write COMMAND                                    */
    MADR        =  (phy << 8) | reg;                                    /* [12:8] == PHY addr, [4:0]=0x00(BMCR) register addr       */
    MWTD        =   val;                                                /* Write the data to the Management Write Data register     */

    while ((MIND != 0) && (retries < PHY_RDWR_RETRIES)) {               /* Read the Management Indicator register, MII busy if > 0  */
    
        for (i = 0; i < 1000; i++) {                                    /* Delay while the read is in progress                      */
            ;
        }
        retries++;
    }

    if (retries >= PHY_RDWR_RETRIES) {                                  /* If there are no retries remaining                        */
       *perr     = NET_PHY_ERR_REGWR_TIMEOUT;                           /* Return an error code if the PHY Read timed out           */
    } else {
       *perr     = NET_PHY_ERR_NONE;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         NetNIC_TxPktDiscard()
*
* Description : On any Transmit errors, set error.
*
* Argument(s) : pbuf        Pointer to network buffer.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_ERR_TX                  Transmit error; packet discarded.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_TxPkt().
*********************************************************************************************************
*/

static  void  NetNIC_TxPktDiscard (NET_ERR  *perr)
{
#if (NET_CTR_CFG_STAT_EN     == DEF_ENABLED)
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
#endif


    NET_CTR_ERR_INC(NetNIC_ErrTxPktDiscardedCtr);

   *perr = NET_ERR_TX;
}

/*
*********************************************************************************************************
*                                           EMAC_Init()
*
* Description : (1) Initialize the LPC1769 integrated EMAC
*
*                   (a) Initialize Registers
*                   (b) Initialize MAC  Address
*                   (c) Initialize Auto Negotiation
*                   (d) Initialize Interrupts
*                   (e) Enable     Receiver/Transmitter
*
* Argument(s) : perr    a return error code indicating the result of initialization
*                       NET_NIC_ERR_NONE      -  success
*                       NET_NIC_ERR_NIC_OFF   -  link is down, or init failure occured
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_Init().
*
* Note(s)     : (1) (a) Assumes MAC address to set has previously been initialized by
*                       (1) Application code          for EMAC_MAC_ADDR_SEL_CFG
*
*               (2) Interrupts MUST be enabled ONLY after ALL network initialization is complete (see also
*                   'net.c  Net_Init()  Note #4d').
*
*              (3) P1.6, ENET-TX_CLK, has to be set for EMAC to address a BUG in early revisions of the LPC1769.
*                  This pin must be set even if RMII is not used.  The bug has been fixed on newer revisions of
*                  the chip and can be used for GPIO in some cicumstances. The Keil MCB2300 that this driver
*                  has been developed with contains the errata. RMII mode can be switched on and off from
*                  net_bsp.h by adjusting the EMAC_CFG_RMII macro accordingly.
*
*             (4)  The external PHY address is defined in net_phy.h as PHY_ADDR and should be set to
*                  the address of your external PHY on the MII bus. This value is shifted over to the
*                  left by 8 bits and written to the MAC_MADR register during PHY communications.
*                  Consult your PHY documentation for additional information on how to determine the
*                  address of your PHY.
*
*             (5)  The EMAC does not support the use of serial EEPROMS for automatically loading
*                  the MAC address after reset. Therefore, the macro EMAC_CFG_MAC_ADDR_SEL in
*                  net_bsp.h MUST be configured to EMAC_CFG_MAC_ADDR_SEL_CFG. This will allow
*                  the driver to load the Station Address registers with the data found in
*                  NetIF_MAC_Addr[], which is set by the user in app.c before calling Net_Init().
*                  If necessary, the user may take steps to read a serial EEPROM and then
*                  fill the content of NetIF_MAC_Addr[] accordingly.
*
*             (6)  Interrupts are enabled by uC/TCP-IP, Net_Init() calling NetNIC_IntEn().
*
*             (7) Rx descriptors must be 32 bit aligned, Rx status descriptors must be 64 bit aligned.
*
*             (8) The EMAC has 16KB of dedicated RAM located on the AHB2 bus at address 0x7FE00000,
*                 see net_bsp.h.
*********************************************************************************************************
*/

/*
// Ethernet power/clock control bit in PCONP register
#define PCENET 0x40000000
// Ethernet configuration for PINSEL2, as per user guide section 5.3
#define ENET_PINSEL2_CONFIG 0x50150105
// Ethernet configuration for PINSEL3, as per user guide section 5.4
#define ENET_PINSEL3_CONFIG 0x00000005
// Only bottom byte of PINSEL3 relevant to Ethernet
#define ENET_PINSEL3_MASK 0x0000000F

*/
// CodeRed - function added to write to external ethernet PHY chip
void WriteToPHY (int reg, int writeval)
{
  unsigned int loop;
  // Set up address to access in MII Mgmt Address Register
  MADR = DP83848C_DEF_ADR | reg;
  // Write value into MII Mgmt Write Data Register
  MWTD = writeval;
  // Loop whilst write to PHY completes
  for (loop = 0; loop < MII_WR_TOUT; loop++) {
    if ((MIND & MIND_BUSY) == 0) { break; }
  }
}

// CodeRed - function added to read from external ethernet PHY chip
unsigned short ReadFromPHY (unsigned char reg)
{
  unsigned int loop;
  // Set up address to access in MII Mgmt Address Register
  MADR = DP83848C_DEF_ADR | reg;
  // Trigger a PHY read via MII Mgmt Command Register
  MCMD = MCMD_READ;
  // Loop whilst read from PHY completes
  for (loop = 0; loop < MII_RD_TOUT; loop++) {
    if ((MIND & MIND_BUSY) == 0)  { break; }
  }
  MCMD = 0; // Cancel read
  // Returned value is in MII Mgmt Read Data Register
  return (MRDD);
}


static  void  EMAC_Init (NET_ERR *perr)
{
    CPU_INT32U  i, value, phyid1, phyid2, phy_in_use;
    CPU_INT32U	phy_linkstatus_reg, phy_linkstatus_mask;
    //CPU_INT16U phymode;
    int error;

    OS_ERR os_err;
    CPU_SR_ALLOC();



    NIC_RxNRdyCtr       =   0;                                          /* Initialize global Rx packet count to 0                   */

    NetNIC_LinkUp();                                                    /* Set NetNIC_ConnStatus to TRUE by default (for uC/TCP-IP) */

                                                                        /* ------- Reset and configure the EMAC registers --------- */

   *perr                =   NET_NIC_ERR_NONE;                           /* Initialize the returned error code to NO Error           */

    // Set Ethernet power/clock control bit
	PCONP              |=  (1 << 30);                                   /* Enable the EMAC clock                                    */

	//Enable Ethernet pins through PINSEL registers
    PINSEL2             =   0x50150105;	                                /* Selects P1[0,1,4,8,9,10,14,15]                           */
    PINSEL3             =   0x00000005;	                                /* Selects P1[17:16]                                        */

    

	// Set up MAC Configuration Register 1
	MAC1                =   MAC1_RESET_TX       |                       /* [15],[14],[11:8] -> soft resets all MAC internal modules */
                            MAC1_RESET_MCS_TX   |
                            MAC1_RESET_RX       |
                            MAC1_RESET_MCS_RX   |
                            MAC1_RESET_SIM      |
                            MAC1_RESET_SOFT;

	// Set up MAC Command Register
    COMMAND             =   COMMAND_RESET_REG   |                       /* Reset all datapaths and host registers                   */
                            COMMAND_RESET_TX    |
                            COMMAND_RESET_RX	|
							COMMAND_PASS_RUNT_FRAMES;

	// Short delay
    NetBSP_DlyMs(2);                                                    /* Delay for a few milliseconds after reset                 */

	// Set up MAC Configuration Register 1 to pass all receive frames
    MAC1                =   MAC1_PASS_ALL_FRAMES;
	// Set up MAC Configuration Register 2 to append CRC and pad out frames
	MAC2				=	MAC2_CRC_EN | MAC2_PAD_EN;

	// Set Ethernet Maximum Frame Register
	MAXF				=	ETH_MAX_FLEN;
	// Set Collision Window / Retry Register
	CLRT				=	CLRT_DEF;
	// Set Non Back-to-Back Inter-Packet-Gap Register
	IPGR				=	IPGR_DEF;
	
	/* Enable Reduced MII interface. */
	MCFG				=	MCFG_CLKSEL_DIV64 | MCFG_RESET_MII_MGMT;
	for (i = 100; i; i--);
	MCFG				=	MCFG_CLKSEL_DIV64;
	// Set MAC Command Register to enable Reduced MII interface
	// and prevent runt frames being filtered out
	COMMAND				=	COMMAND_RMII | COMMAND_PASS_RUNT_FRAMES | COMMAND_PASS_RX_FILT;
	
	// Put DP83848C PHY into reset mode
	WriteToPHY (PHY_REG_BMCR, 0x8000);
	// Loop until hardware reset completes
	for (i = 0; i < 0x100000; i++) {
		value = ReadFromPHY (PHY_REG_BMCR);
		if (!(value & 0x8000)) {
			// Reset has completed
			break;
		}
	}
       
        // Just check this actually is a DP83848C PHY
        phyid1 = ReadFromPHY (PHY_REG_IDR1);
        phyid2 = ReadFromPHY (PHY_REG_IDR2);
	if (((phyid1 << 16) | (phyid2 & 0xFFF0)) == DP83848C_ID) {
		phy_in_use =  DP83848C_ID;
	}
	else if (((phyid1 << 16) | (phyid2 & 0xFFF0)) == LAN8720_ID) {
		phy_in_use = LAN8720_ID;
	}



	OSTimeDlyHMSM(0, 0, 0, 1000, OS_OPT_TIME_HMSM_STRICT, &os_err);
	if (phy_in_use != 0) {
		//***************************************************************************************
		//*		Seteo de velocidad en capa fisica de Ethernet


		//-------------------------------------------------------------------------------------------
		phymode = EepromReadByte(ETHPHYMODE_E2P_ADDR, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		switch(phymode)	{
		case ETH_AUTO_NEG:
			// Set PHY to autonegotiation link speed
			WriteToPHY (PHY_REG_BMCR, PHY_AUTO_NEG);
			/* Wait to complete Auto_Negotiation */
			NIC_timer = 10;
			while(NIC_timer)	{
				value = ReadFromPHY (PHY_REG_BMSR);
				WDT_Feed();
				if (value & 0x0020)	{
					if( value & 0x4000 )	{
						WriteToPHY (PHY_REG_BMCR, PHY_FULLD_100M);
						OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
						phymodewarm = ETH_FULLD_100M;
					} else
					if( value & 0x2000 )	{
						WriteToPHY (PHY_REG_BMCR, PHY_HALFD_100M);
						OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
						phymodewarm = ETH_HALFD_100M;
					} else
					if( value & 0x1000 )	{
						WriteToPHY (PHY_REG_BMCR, PHY_FULLD_10M);
						OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
						phymodewarm = ETH_FULLD_10M;
					} else
					if( value & 0x0800 )	{
						WriteToPHY (PHY_REG_BMCR, PHY_HALFD_10M);
						OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
						phymodewarm = ETH_HALFD_10M;
					}
					break;
				}
				OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			}
			break;
		case ETH_FULLD_100M:
			WriteToPHY (PHY_REG_BMCR, PHY_FULLD_100M);
			OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			phymodewarm = ETH_FULLD_100M;
			break;
		case ETH_HALFD_100M:
			WriteToPHY (PHY_REG_BMCR, PHY_HALFD_100M);
			OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			phymodewarm = ETH_HALFD_100M;
			break;
		case ETH_FULLD_10M:
			WriteToPHY (PHY_REG_BMCR, PHY_FULLD_10M);
			OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			phymodewarm = ETH_FULLD_10M;
			break;
		case ETH_HALFD_10M:
			WriteToPHY (PHY_REG_BMCR, PHY_HALFD_10M);
			OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			phymodewarm = ETH_HALFD_10M;
			break;
		case ETH_FD10M_DEF:
			//****
			for( phymode = ETH_AUTO_NEG; phymode < ETH_PHY_END; phymode++ )	{
				switch(phymode)	{
				case ETH_AUTO_NEG:
					// Set PHY to autonegotiation link speed
					WriteToPHY (PHY_REG_BMCR, PHY_AUTO_NEG);
					/* Wait to complete Auto_Negotiation */
					NIC_timer = 10;
					while(NIC_timer)	{
						value = ReadFromPHY (PHY_REG_BMSR);
						WDT_Feed();
						if (value & 0x0020)	{
							if( value & 0x4000 )	{
								WriteToPHY (PHY_REG_BMCR, PHY_FULLD_100M);
								OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
								phymodewarm = ETH_FULLD_100M;
								phymode = ETH_PHY_END;
								break;
							} else
							if( value & 0x2000 )	{
								WriteToPHY (PHY_REG_BMCR, PHY_HALFD_100M);
								OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
								phymodewarm = ETH_HALFD_100M;
								phymode = ETH_PHY_END;
								break;
							} else
							if( value & 0x0800 )	{
								WriteToPHY (PHY_REG_BMCR, PHY_FULLD_10M);
								OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
								phymodewarm = ETH_FULLD_10M;
								phymode = ETH_PHY_END;
								break;
							} else
							if( value & 0x0080 )	{
								WriteToPHY (PHY_REG_BMCR, PHY_HALFD_10M);
								OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
								phymodewarm = ETH_HALFD_10M;
								phymode = ETH_PHY_END;
								break;
							}

						}
						OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					}
					break;
				case ETH_FULLD_100M:
					WriteToPHY (PHY_REG_BMCR, PHY_FULLD_100M);
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					phymodewarm = ETH_FULLD_100M;
					break;
				case ETH_HALFD_100M:
					WriteToPHY (PHY_REG_BMCR, PHY_HALFD_100M);
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					phymodewarm = ETH_HALFD_100M;
					break;
				case ETH_FULLD_10M:
					WriteToPHY (PHY_REG_BMCR, PHY_FULLD_10M);
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					phymodewarm = ETH_FULLD_10M;
					break;
				case ETH_HALFD_10M:
					WriteToPHY (PHY_REG_BMCR, PHY_HALFD_10M);
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					phymodewarm = ETH_HALFD_10M;
					break;
				case ETH_FD10M_DEF:
					WriteToPHY (PHY_REG_BMCR, PHY_FULLD_100M);
					OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
					phymodewarm = ETH_FULLD_100M;
					phymode = ETH_PHY_END;
					break;
				}
				OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
				// Now check the link status
				for (i = 0; i < 0x10000; i++) {
					value = ReadFromPHY (PHY_REG_STS);
				    if (value & 0x0001) {
				      // The link is on
				    	phymode = ETH_PHY_END;
				    	break;
				    }
				}
			}
			//****
			break;
		}
		//-------------------------------------------------------------------------------------------

		//***************************************************************************************
	}

	CPU_CRITICAL_ENTER();
	//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	phy_linkstatus_reg = PHY_REG_STS;		// Default to DP83848C
	phy_linkstatus_mask = 0x0001;

	if (phy_in_use == LAN8720_ID) {
		phy_linkstatus_reg = PHY_REG_BMSR;
		phy_linkstatus_mask = 0x0004;
	}

	// Now check the link status
	for (i = 0; i < 0x10000; i++) {
		value = ReadFromPHY (phy_linkstatus_reg);
	    if (value & phy_linkstatus_mask) {
	      // The link is on
	    	NetNIC_ConnStatus = DEF_ON;
	    	NetNIC_LinkUp();
	    	//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	    	break;
	    } else	{
	    	NetNIC_ConnStatus = DEF_OFF;
	    	NetNIC_LinkDown();
	    	break;
	    }
	}
	//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	// Now configure for full/half duplex mode
	if (value & 0x0004) {
	// We are in full duplex is enabled mode
	  MAC2    |= MAC2_FULL_DUPLEX;
	  COMMAND |= COMMAND_FULL_DUPLEX;
	  IPGT     = IPGT_FULL_DUP;
	}
	else {
	// Otherwise we are in half duplex mode
	  IPGT = IPGT_HALF_DUP;
	}
	//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	// Now configure 100MBit or 10MBit mode
	if (value & 0x0002) {
	// 10MBit mode
	  SUPP = 0;
	}
	else {
	// 100MBit mode
	  SUPP = SUPP_SPEED;
	}
	//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	//----------------------------------------------------------------------------------------------------------------------------------------------

    SA0                 =  (NetIF_MAC_Addr[5] << 8) |                   /* Write the MAC Address, octect 2 and 1 to the EMAC        */
                           (NetIF_MAC_Addr[4]);

    SA1                 =  (NetIF_MAC_Addr[3] << 8) |                   /* Write the MAC Address, octect 4 and 3 to the EMAC        */
                           (NetIF_MAC_Addr[2]);

    SA2                 =  (NetIF_MAC_Addr[1] << 8) |                   /* Write the MAC Address, octect 6 and 5 to the EMAC        */
                           (NetIF_MAC_Addr[0]);


    NetIF_MAC_AddrValid =   DEF_YES;                                    /* Inform uC/TCP-IP, that we have a valid MAC address set   */


                                                                        /* ----- Descriptor List Global Pointer Initialization ---- */
    RxDesc              =  (EMAC_DESCRIPTOR *)(EMAC_RX_DESC_BASE_ADDR);
    TxDesc              =  (EMAC_DESCRIPTOR *)(EMAC_TX_DESC_BASE_ADDR);
    RxStatus            =  (RX_STATUS  *)(EMAC_RX_STATUS_BASE_ADDR);
    TxStatus            =  (TX_STATUS  *)(EMAC_TX_STATUS_BASE_ADDR);
    RxBufBaseAddr       =  (CPU_INT08U *)(EMAC_RX_BUFF_BASE_ADDR);
    TxBufBaseAddr       =  (CPU_INT08U *)(EMAC_TX_BUFF_BASE_ADDR);

                                                                        /* ---------------- Rx Descriptor Initialization ----------- */

                                                                        /* Rx Descriptor and Status array initialization            */
                                                                        /* Set the addr of the DMA Rx buf                           */
                                                                        /* Interrupt when Rx complete, set buf size (-1 encoded)    */
                                                                        /* Initialize the Status Info to 0                          */
                                                                        /* Initialize the Status Hash CRC to 0                      */
    for (i = 0; i < EMAC_NUM_RX_DESC; i++) {
        RxDesc[i].PacketAddr        =  (CPU_INT32U)(RxBufBaseAddr + (i * EMAC_RX_BUF_SIZE));
        RxDesc[i].Control           =   EMAC_RX_DESC_INT | (EMAC_RX_BUF_SIZE - 1);
        RxStatus[i].StatusInfo      =   0;
        RxStatus[i].StatusHashCRC   =   0;
    }

    RXDESCRIPTOR        =  (CPU_INT32U)(RxDesc);                        /* Write the Rx Descriptor base address register            */
    RXSTATUS            =  (CPU_INT32U)(RxStatus);                      /* Write the Rx Status base address register                */
    RXDESCRIPTORNUMBER  =   EMAC_NUM_RX_DESC - 1;                       /* Write the Rx Descriptor Number register with NumDesc - 1 */
    RXCONSUMEINDEX      =   0;                                          /* Configure the Rx Consume Index register to 0             */

                                                                        /* ------- Tx Descriptor and Status Initialization -------- */

                                                                        /* Rx Descriptor and Status array initialization            */
                                                                        /* Set the addr of the DMA Rx buf, assume 1536B frames */
                                                                        /* Configure the control bits in the EMAC_TxPkt() function  */
                                                                        /* Initialize the Status Info to 0                          */
    for (i = 0; i < EMAC_NUM_TX_DESC; i++) {
        TxDesc[i].PacketAddr        =  (CPU_INT32U)(TxBufBaseAddr + (i * 1536));
        TxDesc[i].Control           =   0;
        TxStatus[i].StatusInfo      =   0;
    }

    TXDESCRIPTOR        =  (CPU_INT32U)(TxDesc);                        /* Write the Tx Descriptor base address register            */
    TXSTATUS            =  (CPU_INT32U)(TxStatus);                      /* Write the Tx Status base address register                */
    TXDESCRIPTORNUMBER  =   EMAC_NUM_TX_DESC - 1;                       /* Write the Tx Descriptor Number register with NumDesc - 1 */
    TXPRODUCEINDEX      =   0;                                          /* Configure the Tx Produce Index register to 0             */

    RXFILTERCTRL		= RFC_BCAST_EN | RFC_PERFECT_EN;
                                                                        /* ------- Configure Interrupts on the VIC Level ---------- */

    INTENABLE  |= (INT_RX_DONE | INT_RX_OVERRUN);                       /* Enable RxDone interrupts and Rx Overrun interrupts       */

    INTCLEAR            =  (INT_RX_OVERRUN   |                          /* Clear all EMAC interrupt sources                         */
                            INT_RX_ERROR     |
                            INT_RX_FINISHED  |
                            INT_RX_DONE      |
                            INT_TX_UNDERRUN  |
                            INT_TX_ERROR     |
                            INT_TX_FINISHED  |
                            INT_TX_DONE      |
                            INT_SOFT         |
                            INT_WAKEUP);


                                                                        /* -------- Enable the transmitter and receiver ----------- */

    // Finally enable receive and transmit mode in ethernet core
    COMMAND  |= (COMMAND_RX_EN | COMMAND_TX_EN);
    MAC1     |= MAC1_RX_ENABLE;
    *perr    =   NET_NIC_ERR_NONE;                           /* Initialize the returned error code to NO Error           */
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*                                         NIC_RxGetNRdy()
*
* Description : Determines how many packets we are ready to be processed.
*
* Parameters  : None.
*
* Returns     : Number of NIC buffers that are ready to be processed by the stack.
*********************************************************************************************************
*/

static  CPU_INT16U  NIC_RxGetNRdy (void)
{
    CPU_INT16U     n_rdy;
    CPU_INT16U     rxconsumeix;
    CPU_INT16U     rxproduceix;


    rxconsumeix =   RXCONSUMEINDEX;
    rxproduceix =   RXPRODUCEINDEX;

    if (rxproduceix < rxconsumeix) {                                    /* If the produce index has wrapped around                  */
        n_rdy   =   EMAC_NUM_RX_DESC - rxconsumeix + rxproduceix;
    } else {                                                            /* If consumeix is < produceix, then no wrap around occured */
        n_rdy   =   rxproduceix - rxconsumeix;
    }

    return (n_rdy);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                              LOCAL FUNCTIONS: EMAC RX FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          EMAC_RxEn()
*
* Description : Enable LPC1769X EMAC Receiver.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*********************************************************************************************************
*/

static  void  EMAC_RxEn (void)
{
    COMMAND    |=  COMMAND_RX_EN;                                       /* Enable the receiver                                      */
    MAC1       |=  COMMAND_RX_EN;                                       /* Enable the receiver                                      */
}

/*
*********************************************************************************************************
*                                          EMAC_RxDis()
*
* Description : Disable LPC1769 EMAC Receiver.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*********************************************************************************************************
*/

static  void  EMAC_RxDis (void)
{
    COMMAND    &= ~COMMAND_RX_EN;                                       /* Disable the receiver                                     */
    MAC1       &= ~COMMAND_RX_EN;                                       /* Disable the receiver (same bit position as COMMAND reg)  */
}

/*
*********************************************************************************************************
*                                       EMAC_RxIntEn()
*
* Description : Enable EMAC Receiver Interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_IntEn(), NetNIC_RxPktGetSize(), NetNIC_RxPkt().
*
* Notes       : 1) The RxDone interrupt occurs when a Rx descriptor has been produced by the EMAC.
*********************************************************************************************************
*/

static  void  EMAC_RxIntEn (void)
{
    INTENABLE  |= (INT_RX_DONE | INT_RX_OVERRUN);                       /* Enable RxDone interrupts and Rx Overrun interrupts       */
}

/*
*********************************************************************************************************
*                                        EMAC_RxPkt()
*
* Description : (1) Read network packet from NIC into buffer :
*                   (a) Read received frame DMA memory space.
*                   (b) Release descriptors once data has been read
*
* Argument(s) : ppkt        Pointer to memory buffer to receive NIC packet.
*               ----        Argument checked in NetNIC_RxPkt().
*
*               size        Number of packet frame octets to read into buffer.
*               ----        Argument checked in NetNIC_RxPkt().
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
*                               NET_ERR_RX                      Error Reading Buffers (EOF not found)
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_RxPkt().
*
* Notes       : none.
*********************************************************************************************************
*/

static  void  EMAC_RxPkt (void        *ppkt,
                          CPU_INT16U   size,
                          NET_ERR     *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR       cpu_sr = 0;
#endif
    void         *prxbuf;


   *perr                =  NET_NIC_ERR_NONE;                            /* Assume no errors have occured                            */
    prxbuf              = (void *)RxDesc[RXCONSUMEINDEX].PacketAddr;    /* Get a pointer to the Rx fragment / frame buffer          */

    RxStatus[RXCONSUMEINDEX].StatusInfo     = 0;                        /* Clear status for debugging purposes                      */

    Mem_Copy(ppkt, prxbuf, size);                                       /* Copy the fragment / frame into the stacks buffer         */

    RXCONSUMEINDEX      = (RXCONSUMEINDEX + 1) % EMAC_NUM_RX_DESC;      /* Discard the fragment / frame                             */

    CPU_CRITICAL_ENTER();                                               /* This routine alters shared data. Disable interrupts!     */
    if (NIC_RxNRdyCtr > 0) {                                            /* One less packet to process                               */
        NIC_RxNRdyCtr--;
    }
    CPU_CRITICAL_EXIT();                                                /* Re-Enable interrupts                                     */
}

/*
*********************************************************************************************************
*                                    EMAC_RxPktDiscard()
*
* Description : Discard network packet from NIC to free NIC packet frames for new receive packets.
*
* Argument(s) : Size: how much data to discard. However, we will discard an entire frame
*               so this information is not necessary.
*
* Return(s)   : none.
*
* Caller(s)   : NetNIC_RxPktDiscard().
*********************************************************************************************************
*/

static  void  EMAC_RxPktDiscard (CPU_INT16U  size)
{
   (void)size;                                                          /* Size is not required since frames are discarded by index */
    RXCONSUMEINDEX      = (RXCONSUMEINDEX + 1) % EMAC_NUM_RX_DESC;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                              LOCAL FUNCTIONS: MACB TX FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          EMAC_TxEn()
*
* Description : Enable LPC1769X EMAC Transmitter.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*********************************************************************************************************
*/

static  void  EMAC_TxEn (void)
{
    COMMAND    |=  COMMAND_TX_EN;                                       /* Enable the transmitter                                   */
}

/*
*********************************************************************************************************
*                                          EMAC_TxDis()
*
* Description : Disable LPC1769 EMAC Transmitter.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : EMAC_Init().
*********************************************************************************************************
*/

static  void  EMAC_TxDis (void)
{
    COMMAND    &= ~COMMAND_TX_EN;                                       /* Disable the transmitter                                  */
}

/*
*********************************************************************************************************
*                                        EMAC_TxPkt()
*
* Description : (1) Instruct EMAC to send network packet :
*                   (a) Check if transmitter ready.
*                   (b) Clear all transmitter errors.
*                   (c) Inform transmitter about buffer address and size.
*                       This starts actual transmission of the packet.
*
* Argument(s) : ppkt        Pointer to memory buffer to transmit NIC packet.
*               ----        Argument checked in NetNIC_TxPkt().
*
*               size        Number of packet frame octets to write to frame.
*
*               perr        Pointer to variable that will hold the return error code from this function :
*
*                               NET_NIC_ERR_NONE                Packet successfully transmitted.
*                               EMAC_ERR_TX_BUSY                Transmitter not ready.
* Return(s)   : none.
*
* Caller(s)   : NetNIC_TxPkt().
*
* Notes       : 1) The TCP-IP stack is signaled immediately after issuing the Tx command
*                  because it is assumed that the hardware transmits frames much faster
*                  than the CPU can load the next Tx descriptor.
*********************************************************************************************************
*/

static  void  EMAC_TxPkt (void        *ppkt,
                          CPU_INT16U   size,
                          NET_ERR     *perr)
{
    Mem_Copy((void *)(TxDesc[TXPRODUCEINDEX].PacketAddr), ppkt, size);  /* Copy the packet into the EMAC AHB2 RAM                   */

    TxDesc[TXPRODUCEINDEX].Control =  (EMAC_TX_DESC_OVERRIDE   |        /* Override the defaults from the MAC internal registers    */
                                       EMAC_TX_DESC_PAD        |        /* Add padding for frames < 64 bytes                        */
                                       EMAC_TX_DESC_LAST       |        /* No additional descriptors to follow, this is the last    */
                                       EMAC_TX_DESC_CRC)       |        /* Append the CRC automatically                             */
                                      (size - 1);                       /* Write the size of the frame, starting from 0             */

    TXPRODUCEINDEX      =   (TXPRODUCEINDEX + 1) % EMAC_NUM_TX_DESC;    /* Increment the produce Ix register, initiate Tx of frame  */

   *perr                =    NET_NIC_ERR_NONE;                          /* No error condition                                       */

    NetOS_NIC_TxRdySignal();                                            /* Signal the stack that it may transmit another frame      */
}

