/*
 * dhcp.h
 *
 *  Created on: Feb 9, 2015
 *      Author: claudio
 */


/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    dhcp.h

  Summary:


  Description:
    DHCP Defs for Microchip TCP/IP Stack

 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) <2014> released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef __DHCP_H_
#define __DHCP_H_

//----------------------------------------------------------------------------

typedef union
{
    uint16_t Val;
    uint8_t v[2];
    __packed struct
    {
        uint8_t LB;
        uint8_t HB;
    } byte;
    __packed struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
    } bits;
} TCPIP_UINT16_VAL, TCPIP_UINT16_BITS;

typedef union
{
    uint32_t Val;
    __packed uint16_t w[2];
    uint8_t  v[4];
    __packed struct
    {
        uint16_t LW;
        uint16_t HW;
    } word;
    __packed struct
    {
        uint8_t LB;
        uint8_t HB;
        uint8_t UB;
        uint8_t MB;
    } byte;
    __packed struct
    {
        TCPIP_UINT16_VAL low;
        TCPIP_UINT16_VAL high;
    }wordUnion;
    __packed struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t b16:1;
         uint8_t b17:1;
         uint8_t b18:1;
         uint8_t b19:1;
         uint8_t b20:1;
         uint8_t b21:1;
         uint8_t b22:1;
         uint8_t b23:1;
         uint8_t b24:1;
         uint8_t b25:1;
         uint8_t b26:1;
         uint8_t b27:1;
         uint8_t b28:1;
         uint8_t b29:1;
         uint8_t b30:1;
         uint8_t b31:1;
    } bits;
} TCPIP_UINT32_VAL;

// Structure to contain a MAC address
typedef __packed struct
{
    uint8_t v[6];
} MAC_ADDR;

// Definition to represent an IP address
#define IP_ADDR     TCPIP_UINT32_VAL
//----------------------------------------------------------------------------




// UDP client port for DHCP Client transactions
#define DHCP_CLIENT_PORT                (68u)
// UDP listening port for DHCP Server messages
#define DHCP_SERVER_PORT                (67u)

#define BOOT_REQUEST                    (1u)    // BOOT_REQUEST DHCP type
#define BOOT_REPLY                      (2u)    // BOOT_REPLY DHCP type
#define BOOT_HW_TYPE                    (1u)    // BOOT_HW_TYPE DHCP type
#define BOOT_LEN_OF_HW_TYPE             (6u)    // BOOT_LEN_OF_HW_TYPE DHCP type

#define DHCP_MESSAGE_TYPE               (53u)   // DHCP Message Type constant
#define DHCP_MESSAGE_TYPE_LEN           (1u)    // Length of DHCP Message Type

#define DHCP_UNKNOWN_MESSAGE            (0u)    // Code for unknown DHCP message

#define DHCP_DISCOVER_MESSAGE           (1u)    // DCHP Discover Message
#define DHCP_OFFER_MESSAGE              (2u)    // DHCP Offer Message
#define DHCP_REQUEST_MESSAGE            (3u)    // DHCP Request message
#define DHCP_DECLINE_MESSAGE            (4u)    // DHCP Decline Message
#define DHCP_ACK_MESSAGE                (5u)    // DHCP ACK Message
#define DHCP_NAK_MESSAGE                (6u)    // DHCP NAK Message
#define DHCP_RELEASE_MESSAGE            (7u)    // DCHP Release message

#define DHCP_SERVER_IDENTIFIER          (54u)   // DHCP Server Identifier
#define DHCP_SERVER_IDENTIFIER_LEN      (4u)    // DHCP Server Identifier length

#define DHCP_OPTION_ACK_MESSAGE         (53u)   // DHCP_OPTION_ACK_MESSAGE Type
#define DHCP_PARAM_REQUEST_LIST         (55u)   // DHCP_PARAM_REQUEST_LIST Type
#define DHCP_PARAM_REQUEST_LIST_LEN     (4u)    // DHCP_PARAM_REQUEST_LIST_LEN Type
#define DHCP_PARAM_REQUEST_IP_ADDRESS       (50u)   // DHCP_PARAM_REQUEST_IP_ADDRESS Type
#define DHCP_PARAM_REQUEST_IP_ADDRESS_LEN   (4u)    // DHCP_PARAM_REQUEST_IP_ADDRESS_LEN Type
#define DHCP_PARAM_REQUEST_CLIENT_ID       (61u)    // DHCP_PARAM_REQUEST_IP_ADDRESS Type

#define DHCP_SUBNET_MASK                (1u)    // DHCP_SUBNET_MASK Type
#define DHCP_ROUTER                     (3u)    // DHCP_ROUTER Type
#define DHCP_DNS                        (6u)    // DHCP_DNS Type
#define DHCP_HOST_NAME                  (12u)   // DHCP_HOST_NAME Type
#define DHCP_IP_LEASE_TIME              (51u)   // DHCP_IP_LEASE_TIME Type
#define DHCP_END_OPTION                 (255u)  // DHCP_END_OPTION Type

// DHCP or BOOTP Header structure
//#pragma data_alignment = 2
typedef __packed struct //__attribute__(aligned(2))
{
    uint8_t     MessageType;    // Message type for this message
    uint8_t     HardwareType;   // Hardware type for this message
    uint8_t     HardwareLen;    // Length of hardware type
    uint8_t     Hops;           // Number of hops
    uint32_t        TransactionID;  // DHCP Transaction ID
    uint16_t        SecondsElapsed; // Number of elapsed seconds
    uint16_t        BootpFlags;     // BOOTP Flags
    IP_ADDR     ClientIP;       // Client IP
    IP_ADDR     YourIP;         // Your IP
    IP_ADDR     NextServerIP;   // Next Server IP
    IP_ADDR     RelayAgentIP;   // Relay Agent IP
    MAC_ADDR    ClientMAC;      // Client MAC Address
} BOOTP_HEADER;

// DHCP State Machine
typedef enum
{
    SM_DHCP_DISABLED = 0,       // DHCP is not currently enabled
    SM_DHCP_GET_SOCKET,         // DHCP is trying to obtain a socket
    SM_DHCP_SEND_DISCOVERY,     // DHCP is sending a DHCP Discover message
    SM_DHCP_GET_OFFER,          // DHCP is waiting for a DHCP Offer
    SM_DHCP_SEND_REQUEST,       // DHCP is sending a DHCP Send Reequest message
    SM_DHCP_GET_REQUEST_ACK,    // DCHP is waiting for a Request ACK message
    SM_DHCP_BOUND,              // DHCP is bound
    SM_DHCP_SEND_RENEW,         // DHCP is sending a DHCP renew message (first try)
    SM_DHCP_GET_RENEW_ACK,      // DHCP is waiting for a renew ACK
    SM_DHCP_SEND_RENEW2,        // DHCP is sending a DHCP renew message (second try)
    SM_DHCP_GET_RENEW_ACK2,     // DHCP is waiting for a renew ACK
    SM_DHCP_SEND_RENEW3,        // DHCP is sending a DHCP renew message (third try)
    SM_DHCP_GET_RENEW_ACK3      // DHCP is waiting for a renew ACK
} SM_DHCP;


void DHCPInit(uint8_t vInterface);
void DHCPTask(void);
void DHCPServerTask(void);
void DHCPDisable(uint8_t vInterface);
void DHCPEnable(uint8_t vInterface);

void DHCPServer_Disable(void);
void DHCPServer_Enable(void);
uint8_t DHCPIsEnabled(uint8_t vInterface);
uint8_t DHCPIsBound(uint8_t vInterface);
uint8_t DHCPStateChanged(uint8_t vInterface);
uint8_t DHCPIsServerDetected(uint8_t vInterface);
void DHCPTempIPAddr(void);                      // SOFTAP_ZEROCONF_SUPPORT

extern uint8_t bDHCPServerEnabled;

#endif


