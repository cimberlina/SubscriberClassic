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
*                                        NETWORK PHYSICAL LAYER
*
*                                           National DP83848
*
* Filename      : net_phy.c
* Version       : V1.89
* Programmer(s) : EHS
*********************************************************************************************************
* Note(s)       : (1) Supports National Semiconductor DP83848 10/100 PHY
*
*                 (2) The MII interface port is assumed to be part of the host EMAC; consequently,
*                     reads from and writes to the PHY are made through the EMAC.  The functions
*                     NetNIC_PhyRegRd() and NetNIC_PhyRegWr(), which are used to access the PHY, should
*                     be provided in the EMAC driver.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <net.h>
#include  <net_phy.h>
#include  <net_phy_def.h>

/*
*********************************************************************************************************
*********************************************************************************************************
*                                            GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         NetNIC_PhyInit()
*
* Description : Initialize phyter (ethernet link controller)
*
* Argument(s) : none.
*
* Return(s)   : 1 for OK, 0 for error
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : Assumes the MDI port as already been enabled for the PHY.
*********************************************************************************************************
*/
CPU_INT32U	phy_in_use;

void  NetNIC_PhyInit (NET_ERR *perr)
{
    CPU_INT16U   reg_val;
    CPU_INT16U   i;
    //CPU_INT16U   oui_msb;
    //CPU_INT16U   oui_lsb;
    CPU_INT32U	phyid1, phyid2;

    phy_in_use = 0;

    NetNIC_PhyRegWr(EMAC_CFG_PHY_ADDR, MII_BMCR, BMCR_RESET, perr);     /* Reset the PHY                                            */
    i               = DP83848_INIT_RESET_RETRIES;
    reg_val         = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_BMCR, perr) & BMCR_RESET;

    while (reg_val == BMCR_RESET && i > 0) {
        NetBSP_DlyMs(200);                                              /* Delay while reset completes                              */
        reg_val     =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_BMCR, perr) & BMCR_RESET; /* Read the control register                */
        i--;
    }

    if (i == 0) {                                                       /* If reset has not completed and no retries remain         */
       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return a reset timeout error                             */
        return;
    }

//    oui_msb         = (DP83848_OUI >>   6) & 0xFFFF;
//    oui_lsb         = (DP83848_OUI & 0x3F) <<    10;
//
//    reg_val         =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID1, perr);   /* Read the PHY ID, ensure the PHY has reset        */
//    if (reg_val != oui_msb) {
//       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return an error  if the PHY is not properly reset        */
//        return;
//    }
//
//    reg_val         =  NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID2, perr);   /* Read the PHY ID, ensure the PHY has reset        */
//    if ((reg_val & 0xFC00) != oui_lsb) {
//       *perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return an error  if the PHY is not properly reset        */
//        return;
//    }

    //Modificacion para manejar los Phyters DP83848, LAN8720 y el KSZ8041NL
    phyid1 = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID1, perr);
    phyid2 = NetNIC_PhyRegRd(EMAC_CFG_PHY_ADDR, MII_PHYSID2, perr);

    if (((phyid1 << 16) | (phyid2 & 0xFFF0)) == DP83848C_ID) {
    	phy_in_use = DP83848C_ID;
    }
    else if (((phyid1 << 16) | (phyid2 & 0xFFF0)) == LAN8720_ID) {
    	phy_in_use = LAN8720_ID;
    }
    else if (((phyid1 << 16) | (phyid2 & 0xFFF0)) == KSZ8041NL_ID) {
        phy_in_use = KSZ8041NL_ID;
    }
    if (phy_in_use == 0) {
    	*perr        =  NET_PHY_ERR_RESET_TIMEOUT;                       /* Return an error  if the PHY is not properly reset        */
    	return;
    }
    //---------------------------------------------------------------------
    NetNIC_PhyAutoNeg();                                                /* Attempt Auto-Negotiation                                 */

    NetNIC_ConnStatus = NetNIC_PhyLinkState();                          /* Set NetNIC_ConnStatus according to link state            */

    if (NetNIC_ConnStatus == DEF_ON) {
        NetNIC_LinkUp();
    } else {
        NetNIC_LinkDown();
    }
}


/*
*********************************************************************************************************
*                                        NetNIC_PhyAutoNeg()
*
* Description : Do link auto-negotiation
*
* Argument(s) : none.
*
* Return(s)   : 1 = no error, 0 = error
*
* Caller(s)   : NetNIC_PhyInit.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  NetNIC_PhyAutoNeg (void)
{
    CPU_INT16U   i, value;

    i = DP83848_INIT_AUTO_NEG_RETRIES;
    // Set PHY to autonegotiation link speed
	WriteToPHY (PHY_REG_BMCR, PHY_AUTO_NEG);
	// loop until autonegotiation completes
	value = ReadFromPHY (PHY_REG_BMSR);

	while( !(value & 0x0020) && (i > 0))	{
		NetBSP_DlyMs(1500);
		value = ReadFromPHY (PHY_REG_BMSR);
		i--;
		if (value & 0x0020) {
			// Autonegotiation has completed
			break;
		}
	}

 }

/*
*********************************************************************************************************
*                                    NetNIC_PhyAutoNegState()
*
* Description : Returns state of auto-negotiation
*
* Argument(s) : none.
*
* Return(s)   : State of auto-negociation (DEF_OFF = not completed, DEF_ON = completed).
*
* Caller(s)   : NetNIC_PhyInit().
*
* Note(s)     : If any error is encountered while reading the PHY, this function
*               will return Auto Negotiation State = DEF_OFF (incomplete).
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_PhyAutoNegState (void)
{
    CPU_INT16U   i, value;

    i = DP83848_INIT_AUTO_NEG_RETRIES;
    // Set PHY to autonegotiation link speed
	WriteToPHY (PHY_REG_BMCR, PHY_AUTO_NEG);
	// loop until autonegotiation completes
	value = ReadFromPHY (PHY_REG_BMSR);

	while( !(value & 0x0020) && (i > 0))	{
		NetBSP_DlyMs(1500);
		value = ReadFromPHY (PHY_REG_BMSR);
		i--;
		if (value & 0x0020) {
			// Autonegotiation has completed
			return (DEF_ON);
		} else {
			return (DEF_OFF);
		}
	}
	return (DEF_OFF);
}

/*
*********************************************************************************************************
*                                     NetNIC_PhyLinkState()
*
* Description : Returns state of ethernet link
*
* Argument(s) : none.
*
* Return(s)   : State of ethernet link (DEF_OFF = link down, DEF_ON = link up).
*
* Note(s)     : If any error is encountered while reading the PHY, this function
*               will return link state = DEF_OFF.
*********************************************************************************************************
*/

CPU_BOOLEAN  NetNIC_PhyLinkState (void)
{
    CPU_INT16U  value, i;
    CPU_INT32U	phy_linkstatus_reg, phy_linkstatus_mask;

#if (PHY_IN_USE == LAN8720_ID)
    phy_linkstatus_reg = PHY_REG_BMSR;
    phy_linkstatus_mask = 0x0004;
#endif

#if (PHY_IN_USE == DP83848C_ID)
    phy_linkstatus_reg = PHY_REG_STS;		// Default to DP83848C
    phy_linkstatus_mask = 0x0001;
#endif


	for (i = 0; i < 0x10000; i++) {
		value = ReadFromPHY (phy_linkstatus_reg);
		if (value & phy_linkstatus_mask) {
		  // The link is on
			return (DEF_ON);
		} else	{
			return (DEF_OFF);
		}
	}
	return (DEF_OFF);
}

/*
*********************************************************************************************************
*                                     NetPHY_GetLinkSpeed()
*
* Description : Returns the speed of the current Ethernet link
*
* Argument(s) : none.
*
* Return(s)   : 0 = No Link, 10 = 10mbps, 100 = 100mbps
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  NetNIC_PhyLinkSpeed (void)
{
    CPU_INT16U  value, i;
    CPU_INT32U	phy_linkstatus_reg, phy_linkstatus_mask;

#if (PHY_IN_USE == LAN8720_ID)
    phy_linkstatus_reg = PHY_REG_BMSR;
    phy_linkstatus_mask = 0x0004;
#endif

#if (PHY_IN_USE == DP83848C_ID)
    phy_linkstatus_reg = PHY_REG_STS;		// Default to DP83848C
    phy_linkstatus_mask = 0x0001;
#endif


    if (NetNIC_PhyLinkState() == DEF_OFF) {

        return (NET_PHY_SPD_0);

    } else {

    	//-------------------------------------------------------------------------------



    	for (i = 0; i < 0x10000; i++) {
    		value = ReadFromPHY (phy_linkstatus_reg);
    		if (value & phy_linkstatus_mask) {
    		  // The link is on
    			break;
    		}
    	}
    	if (value & 0x0002) {
    		// 10MBit mode
    		return (NET_PHY_SPD_10);
    	} else {
    	// 100MBit mode
    		return (NET_PHY_SPD_100);
    	}
    	//-------------------------------------------------------------------------------

    }
}

/*
*********************************************************************************************************
*                                     NetPHY_GetDuplex()
*
* Description : Returns the duplex mode of the current Ethernet link
*
* Argument(s) : none.
*
* Return(s)   : 0 = Unknown (Auto-Neg in progress), 1 = Half Duplex, 2 = Full Duplex
*
* Caller(s)   : EMAC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  NetNIC_PhyLinkDuplex (void)
{
	   CPU_INT16U  value, i;
	    CPU_INT32U	phy_linkstatus_reg, phy_linkstatus_mask;

	#if (PHY_IN_USE == LAN8720_ID)
	    phy_linkstatus_reg = PHY_REG_BMSR;
	    phy_linkstatus_mask = 0x0004;
	#endif

	#if (PHY_IN_USE == DP83848C_ID)
	    phy_linkstatus_reg = PHY_REG_STS;		// Default to DP83848C
	    phy_linkstatus_mask = 0x0001;
	#endif


	    if (NetNIC_PhyLinkState() == DEF_OFF) {
	        return (NET_PHY_SPD_0);
	    } else {
	    	for (i = 0; i < 0x10000; i++) {
	    		value = ReadFromPHY (phy_linkstatus_reg);
	    		if (value & phy_linkstatus_mask) {
	    		  // The link is on
	    			break;
	    		}
	    	}
            if ((value & PHYSTS_DUPLEX_STATUS) != 0) {
                return (NET_PHY_DUPLEX_FULL);
            } else {
                return (NET_PHY_DUPLEX_HALF);
            }
	    }
}
