//
// Created by Claudio Imberlina on 13/11/2018.
//

#ifndef IRIDIUMSBD_IRIDIUMSBD_H
#define IRIDIUMSBD_IRIDIUMSBD_H

typedef unsigned short uint16_t;
typedef unsigned char u_int8_t;
typedef unsigned char uint8_t;
//typedef unsigned long size_t;

typedef enum { DEFAULT_POWER_PROFILE = 0, USB_POWER_PROFILE = 1 } POWERPROFILE;

#define TRUE    1
#define FALSE   0

#define ISBD_LIBRARY_REVISION           2
#define ISBD_DEFAULT_AT_TIMEOUT         30
#define ISBD_MSSTM_RETRY_INTERVAL       10
#define ISBD_DEFAULT_SBDIX_INTERVAL     10
#define ISBD_USB_SBDIX_INTERVAL         30
#define ISBD_DEFAULT_SENDRECEIVE_TIME   300
#define ISBD_STARTUP_MAX_TIME           240
#define ISBD_MAX_MESSAGE_LENGTH         340
#define ISBD_MSSTM_WORKAROUND_FW_VER    13001

#define ISBD_SUCCESS             0
#define ISBD_ALREADY_AWAKE       1
#define ISBD_SERIAL_FAILURE      2
#define ISBD_PROTOCOL_ERROR      3
#define ISBD_CANCELLED           4
#define ISBD_NO_MODEM_DETECTED   5
#define ISBD_SBDIX_FATAL_ERROR   6
#define ISBD_SENDRECEIVE_TIMEOUT 7
#define ISBD_RX_OVERFLOW         8
#define ISBD_REENTRANT           9
#define ISBD_IS_ASLEEP           10
#define ISBD_NO_SLEEP_PIN        11
#define ISBD_NO_NETWORK          12
#define ISBD_MSG_TOO_LONG        13

extern int port;

// Timings
extern int sbdixInterval;
extern int atTimeout;
extern int sendReceiveTimeout;

// State variables
extern int remainingMessages;
extern int asleep;
extern int reentrant;
extern int sleepPin;
extern int ringPin;
extern int msstmWorkaroundRequested;
extern int ringAlertsEnabled;
extern int ringAsserted;
extern unsigned long lastPowerOnTime;

extern const char *head, *tail;
extern int nextChar;

extern const char SBDRING[];



extern uint32_t    IRIDIUM_flag;
#define     IRI_SENDALRM_FLAG   (1 << 0)
#define     IRI_SENDOK_FLAG     (1 << 1)
#define     IRI_SENDNG_FLAG     (1 << 2)
#define     IRI_SENDING_FLAG    (1 << 3)
#define     IRI_SENDHB_FLAG     (1 << 4)
#define     IRI_IPDWNHAB_FLAG   (1 << 5)
#define     IRI_RFDWNHAB_FLAG   (1 << 6)
#define     IRI_ALWAYSHAB_FLAG  (1 << 7)
#define     IRI_BOTHDWNHAB_FLAG (1 << 8)
#define     IRI_IRIDIUMOK_FLAG  (1 << 9)
#define     IRI_IRIHBTDONE_FLAG (1 << 10)
#define     IRI_USEIRIDIUM_FLAG (1 << 11)
#define     IRI_SNDCONDOK_FLAG  (1 << 12)
#define     IRI_IPNG_FLAG       (1 << 13)
#define     IRI_MEASDONE_FLAG   (1 << 14)
#define     IRI_GPRSKICK1_FLAG  (1 << 15)
#define     IRI_GPRSKICK2_FLAG  (1 << 16)
#define     IRI_GPRSNG_FLAG     (1 << 17)
#define     IRI_GPRSDWN_FLAG    (1 << 18)
#define     IRI_MDMALIVE_FLAG   (1 << 19)
#define     IRI_PENDING_FLAG    (1 << 20)

extern uint8_t     FSMIRI_state;
#define     FSMIRI_IDLE             0x10
#define     FSMIRI_PRESENDING       0x15
#define     FSMIRI_SENDING          0x20
#define     FSMIRI_CHKSIGQTY        0x30
#define     FSMIRI_CHKSIGQTYWAIT    0x35
#define     FSMIRI_CHKIRIOK         0x50
#define     FSMIRI_CHKIRIOK2        0x55

extern uint8_t     IRI_mode;

extern uint32_t    HBIRI_timer, hbiri_time, iphbiri_timer;

long milisec (void);
//void send(unsigned char *databuff, int lendatabuff);

int cancelled(void);
int ISBDCallback(void);
void SBDRINGSeen(void);

int waitForATResponse(char *response, int responseSize, const char *prompt, const char *terminator);
int filteredavailable(void);
int filteredread(void);
void filterSBDRING(void);
int internalSendReceiveSBD(const char *txTxtMessage, const u_int8_t *txData, int txDataSize, u_int8_t *rxBuffer, int *prxBufferSize);
int doSBDIX(uint16_t *moCode, uint16_t *moMSN, uint16_t *mtCode, uint16_t *mtMSN, uint16_t *mtLen, uint16_t *mtRemaining);
int doSBDRB(uint8_t *rxBuffer, int *prxBufferSize);
int noBlockWait(int seconds);
int internalMSSTMWorkaround(int *okToProceed);
int internalBegin( void );
void power(int on);
int getFirmwareVersion(char *version, int bufferSize);
int IridiumSBD_begin();
int sendSBDBinary(const uint8_t *txData, int txDataSize);
int sendReceiveSBDBinary(const uint8_t *txData, int txDataSize, uint8_t *rxBuffer, int *rxBufferSize);
int sendSBDText(const char *message);
int getSignalQuality(int *quality);
int internalGetSignalQuality(int *quality);
int internalSleep( void );
int IridiumSBD_sleep(void);
int isAsleep( void );
int getWaitingMessageCount( void );
int getSystemTime(struct tm *tmptr);
void setPowerProfile(POWERPROFILE profile);
void adjustATTimeout(int seconds);
void adjustSendReceiveTimeout(int seconds);
void useMSSTMWorkaround(int useWorkAround);
void enableRingAlerts(int enable);
int hasRingAsserted(void);

void SendIRI_ALRM_packet( void );
void fsm_alrm_iridium( void );

#endif //IRIDIUMSBD_IRIDIUMSBD_H
