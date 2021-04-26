/*
 * DHCP_client.c
 *
 *  Created on: Feb 9, 2015
 *      Author: claudio
 */

#include "includes.h"


// Defines how long to wait before a DHCP request times out
#define DHCP_BASE_TIMEOUT                (2ul)

// Unique variables per interface
typedef struct
{
	NET_SOCK_ID         hDHCPSocket;    // Handle to DHCP client socket
    SM_DHCP             smState;        // DHCP client state machine variable
    union
    {
        struct
        {
            unsigned char bIsBound : 1;             // Whether or not DHCP is currently bound
            unsigned char bEvent : 1;               // Indicates to an external module that the DHCP client has been reset, has obtained new parameters via the DHCP client, or has refreshed a lease on existing ones
            unsigned char bOfferReceived : 1;       // Whether or not an offer has been received
            unsigned char bDHCPServerDetected : 1;  // Indicates if a DCHP server has been detected
            unsigned char bUseUnicastMode : 1;      // Indicates if the
        } bits;
        uint8_t val;
    } flags;
    uint32_t                dwTimer;        // Tick timer value used for triggering future events after a certain wait period.
    uint32_t                dwBaseTime;        // Base timer for timeouts in seconds
    uint32_t                dwLeaseTime;    // DHCP lease time remaining, in seconds
    uint32_t                dwServerID;     // DHCP Server ID cache
    IP_ADDR             tempIPAddress;  // Temporary IP address to use when no DHCP lease
    IP_ADDR             tempGateway;    // Temporary gateway to use when no DHCP lease
    IP_ADDR             tempMask;       // Temporary mask to use when no DHCP lease
    #if defined(STACK_USE_DNS)
    IP_ADDR             tempDNS;        // Temporary primary DNS server
    IP_ADDR             tempDNS2;       // Temporary secondary DNS server
    #endif
    // Indicates which DHCP values are currently valid
    union
    {
        struct
        {
            char IPAddress:1;   // Leased IP address is valid
            char Gateway:1;     // Gateway address is valid
            char Mask:1;        // Subnet mask is valid
            char DNS:1;         // Primary DNS is valid
            char DNS2:1;        // Secondary DNS is valid
            char HostName:1;    // Host name is valid (not implemented)
        } bits;
        uint8_t val;
    } validValues;
} DHCP_CLIENT_VARS;

uint8_t DHCPClientInitializedOnce = FALSE;

static uint8_t _DHCPReceive(void);
static void _DHCPSend(uint8_t messageType, uint8_t bRenewing);

#define DHCPBUFFERMAX	384

uint8_t dhcp_tx_buffer[DHCPBUFFERMAX];
uint8_t dhcp_rx_buffer[DHCPBUFFERMAX];
uint16_t dhcp_txbuf_ptr, dhcp_rxbuf_ptr;

static DHCP_CLIENT_VARS DHCPClient;

uint16_t dhcp_tx_put( uint8_t val )
{
	if( dhcp_txbuf_ptr >= DHCPBUFFERMAX )
		return FALSE;
	dhcp_tx_buffer[dhcp_txbuf_ptr] = val;
	dhcp_txbuf_ptr++;

	return TRUE;
}

uint16_t dhcp_tx_putarray(uint8_t *cData, uint16_t wDataLen )
{
	uint16_t tmpLen, charptr;

	if( (dhcp_txbuf_ptr + wDataLen) >= DHCPBUFFERMAX )
		return FALSE;
	tmpLen = wDataLen;
	charptr = 0;

	while(tmpLen)	{
		dhcp_tx_buffer[dhcp_txbuf_ptr] = cData[charptr++];
		dhcp_txbuf_ptr++;
		tmpLen--;
	}
	return wDataLen;
}


/*****************************************************************************
  Function:
    static void _DHCPSend(uint8_t messageType, bool bRenewing)

  Description:
    Sends a DHCP message.

  Precondition:
    UDP is ready to write a DHCP packet.

  Parameters:
    messageType - One of the DHCP_TYPE constants
    bRenewing - Whether or not this is a renewal request

  Returns:
    None
***************************************************************************/
static void _DHCPSend(uint8_t messageType, uint8_t bRenewing)
{
    uint8_t i;
    IP_ADDR MyIP;

    dhcp_txbuf_ptr = 0;

    dhcp_tx_put(BOOT_REQUEST);                       // op
    dhcp_tx_put(BOOT_HW_TYPE);                       // htype
    dhcp_tx_put(BOOT_LEN_OF_HW_TYPE);                // hlen
    dhcp_tx_put(0);                                  // hops
    dhcp_tx_put(0x12);                               // xid[0]
    dhcp_tx_put(0x23);                               // xid[1]
    dhcp_tx_put(0x34);                               // xid[2]
    dhcp_tx_put(0x56);                               // xid[3]
    dhcp_tx_put(0);                                  // secs[0]
    dhcp_tx_put(0);                                  // secs[1]
    dhcp_tx_put(DHCPClient.flags.bits.bUseUnicastMode ? 0x00: 0x80);// flags[0] with Broadcast flag clear/set to correspond to bUseUnicastMode
    dhcp_tx_put(0);                                  // flags[1]

    // If this is DHCP REQUEST message, use previously allocated IP address.
    if((messageType == DHCP_REQUEST_MESSAGE) && bRenewing)
    {
    	dhcp_tx_putarray((uint8_t*)&DHCPClient.tempIPAddress, sizeof(DHCPClient.tempIPAddress));
    }
    else
    {
    	dhcp_tx_put(0x00);
    	dhcp_tx_put(0x00);
    	dhcp_tx_put(0x00);
    	dhcp_tx_put(0x00);
    }

    // Set yiaddr, siaddr, giaddr as zeros,
    for ( i = 0; i < 12u; i++ )
    	dhcp_tx_put(0x00);

    // Load chaddr - Client hardware address.
    dhcp_tx_putarray(NetIF_MAC_Addr, 6);

    // Set chaddr[6..15], sname and file as zeros.
    for ( i = 0; i < 202u; i++ )
    	dhcp_tx_put(0);

    // Load magic cookie as per RFC 1533.
    dhcp_tx_put(99);
    dhcp_tx_put(130);
    dhcp_tx_put(83);
    dhcp_tx_put(99);

    // Load message type.
    dhcp_tx_put(DHCP_MESSAGE_TYPE);
    dhcp_tx_put(DHCP_MESSAGE_TYPE_LEN);
    dhcp_tx_put(messageType);

    if(messageType == DHCP_DISCOVER_MESSAGE)
    {
        // Reset offered flag so we know to act upon the next valid offer
        DHCPClient.flags.bits.bOfferReceived = FALSE;
    }


    if((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing)
    {
        // DHCP REQUEST message must include server identifier the first time
        // to identify the server we are talking to.
        // _DHCPReceive() would populate "serverID" when it
        // receives DHCP OFFER message. We will simply use that
        // when we are replying to server.
        // If this is a renwal request, we must not include server id.
    	dhcp_tx_put(DHCP_SERVER_IDENTIFIER);
    	dhcp_tx_put(DHCP_SERVER_IDENTIFIER_LEN);
    	dhcp_tx_put(((uint8_t*)(&DHCPClient.dwServerID))[3]);
    	dhcp_tx_put(((uint8_t*)(&DHCPClient.dwServerID))[2]);
    	dhcp_tx_put(((uint8_t*)(&DHCPClient.dwServerID))[1]);
    	dhcp_tx_put(((uint8_t*)(&DHCPClient.dwServerID))[0]);
    }

    // Load our interested parameters
    // This is hardcoded list.  If any new parameters are desired,
    // new lines must be added here.
    dhcp_tx_put(DHCP_PARAM_REQUEST_LIST);
    dhcp_tx_put(DHCP_PARAM_REQUEST_LIST_LEN);
    dhcp_tx_put(DHCP_SUBNET_MASK);
    dhcp_tx_put(DHCP_ROUTER);
    dhcp_tx_put(DHCP_DNS);
    dhcp_tx_put(DHCP_HOST_NAME);

    // Add requested IP address to DHCP Request Message
    if( ((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing) ||
        ((messageType == DHCP_DISCOVER_MESSAGE) && DHCPClient.tempIPAddress.Val))
    {
    	dhcp_tx_put(DHCP_PARAM_REQUEST_IP_ADDRESS);
    	dhcp_tx_put(DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
    	dhcp_tx_putarray((uint8_t*)&DHCPClient.tempIPAddress, DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
    }

    // Add any new paramter request here.

    // End of Options.
    dhcp_tx_put(DHCP_END_OPTION);

    // Add zero padding to ensure compatibility with old BOOTP relays that discard small packets (<300 UDP octets)
    while(dhcp_txbuf_ptr < 300)
    	dhcp_tx_put(0);

    // Make sure we advertise a 0.0.0.0 IP address so all DHCP servers will respond.  If we have a static IP outside the DHCP server's scope, it may simply ignore discover messages.
    MyIP.Val = App_IP_Addr;
    if(!bRenewing)	{
    	App_IP_Addr = 0x00000000;
        NetIP_CfgAddrThisHost(App_IP_Addr, App_IP_Mask);
    }

    //ss UDPFlush();

    App_IP_Addr = MyIP.Val;
    NetIP_CfgAddrThisHost(App_IP_Addr, App_IP_Mask);


}


