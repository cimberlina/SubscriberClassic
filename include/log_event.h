/*
 * log_event.h
 *
 *  Created on: Apr 24, 2012
 *      Author: ironman
 */

#ifndef LOG_EVENT_H_
#define LOG_EVENT_H_

typedef struct	{			//16 bytes por registro
	uint8_t		ack_tag;
	uint16_t	index;
	time_t		timestamp;
	uint16_t	account;
	uint8_t		cid_qualifier;
	uint16_t	cid_eventcode;
	uint8_t		cid_partition;
	uint16_t	cid_zoneuser;
	uint8_t		checksum;
} EventRecord;

typedef struct	{
	uint16_t cidevent;
	char *cidstring;

} CID_String_struct;

//memoria para log de eventos
#define DF_MAXEVENTS	128*33		// 128 paginas, cada una con 33 eventos de 16 bytes cada uno.
#define	DF_EVELEN		16
#define	DF_EVENT0		528*2

//Hay paginas libres entre 130 y 199
#define DF_PBTREGLEN	16
#define MAXPBTPTM		8
#define DF_PBTTBLBEG	528*132				// la tabla de los pulsadores remotos esta en la pag 132
#define DF_PBTTBL0		DF_PBTTBLBEG + 0	// 128, son 8 registros de 16 bytes cada uno
#define DF_PBTFIDU		DF_PBTTBL0 + MAXPBTPTM*DF_PBTREGLEN	// 4 bytes fiduciarios, 0x55, 0x5A, 0xA5, 0xAA

//memoria auditoria de tecnicos
#define	DF_MAXTEVES		20*33		// 20 paginas para eventos de auditoria, 660 eventos en total
#define	DF_EVENT_TEC	528*200		//en la pagina 200

#define	DF_CFGPAGE		0
#define	DF_ALARMHISTORY_OFFSET	0		//8
#define	DF_EVENTWRPTR_OFFSET	8		//4
#define	DF_EVENTINDEX_OFFSET	12		//4
#define	DF_FSMAPINDEX_OFFSET	16		//8
#define	DF_RELESTATES			24		//10

#define	LIC_TIMESTAMP			34		//4
#define	LIC_NUMABO				38		//1
#define	LIC_INTERVAL			39		//1
#define	LIC_RNDNUM				40		//1
#define	LIC_SERIALNUM			41		//8
#define	LIC_CHKSUM				49		//1
#define	MEM_ALARM				50		//1

#define	DOOR1_STATE				51		//1
#define	DOOR2_STATE				52		//1
#define	SLLAVE_STATE			53		//1
#define	LOGIN_ENABLED			54		//1

#define	DF_Z0CALIB_OFFSET		60		//2
#define	DF_Z1CALIB_OFFSET		62		//2
#define	DF_Z2CALIB_OFFSET		64		//2
#define	DF_Z3CALIB_OFFSET		66		//2
#define	DF_Z4CALIB_OFFSET		68		//2
#define	DF_Z5CALIB_OFFSET		70		//2
#define	DF_Z6CALIB_OFFSET		72		//2
#define	DF_Z7CALIB_OFFSET		74		//2
#define	DF_PDXSTATUS_OFFSET		76		//4
#define	DF_PARTITION_OFFSET		80		//5
#define	DF_VOLREDUNT_OFFSET		85		//5
#define	DF_PTM485NG_OFFSET		90		//34

#define	DF_MIPADD_OFFSET		124		//16
#define	DF_MGWADD_OFFSET		140		//16
#define	DF_MNMASK_OFFSET		156		//16
#define	DF_MACADD_OFFSET		172		//16
#define	DF_SVR1ADD_OFFSET		188		//16
#define	DF_SVR2ADD_OFFSET		204		//16


#define	DF_MIPCHK_OFFSET		220		//1
#define	DF_MGWCHK_OFFSET		221		//1
#define	DF_MNMCKH_OFFSET		222		//1
#define	DF_MACCHK_OFFSET		223		//1
#define	DF_SR1CHK_OFFSET		224		//1
#define	DF_SR2CHK_OFFSET		225		//1
#define	DF_FIDUMK_OFFSET		226		//1
#define	DF_FIDUDT_OFFSET		227		//1

#define	DF_PORT1_OFFSET			228		//2
#define	DF_PORT2_OFFSET			230		//2
#define	DF_SERIALNUM_OFFSET		232		//9
#define	DF_AUDWRPTR_OFFSET		241		//4

#define	DF_GROUPVOLU_OFFSET		255		//10

#define DF_LICTSVENC_OFFSET		265		//4
#define DF_BLOCKCODE_OFFSET		269		//1
#define DF_RADARP_OFFSET		270		//1

#define DF_PTMPWD_OFFSET		280		//2
#define DF_LICFLAG_OFFSET		282		//2
#define DF_ARADAR_OFFSET		284		//2
#define DF_ENARHB_OFFSET		286		//2
#define	DF_IP150CHK_OFFSET		288		//2
#define	DF_INPATTERN_OFFSET		290		//2
#define DF_ENAIRI_OFFSET		292		//2
#define DF_IRIMODE_OFFSET		294		//2
#define DF_CASTROTU_OFFSET		296		//2
#define DF_AUTORST_OFFSET		298		//2
#define DF_NSIGNAL_OFFSET		300		//3   0:ASALTO, 1:INCENDIO, 2:TESORO
#define DF_NMAX_OFFSET			303		//1
#define DF_NMAXASAL_OFFSET		304		//1
#define DF_NMAXTESO_OFFSET		305		//1
#define DF_NMAXINCE_OFFSET		306		//1
#define DF_HBRSTRTRY_OFFSET		307		//1
#define DF_BORDLY_OFFSET        308     //2
#define DF_PTMDLY_OFFSET        310     //2
#define DF_E393HAB_OFFSET       312     //2
#define DF_PPONWDOG_OFFSET      314     //2
#define DF_RFFILTER1_OFFSET		316		//2
#define DF_RFFILTER2_OFFSET		318		//2

#define	ACKRCVDMON0				0x01
#define	ACKRCVDMON1				0x02

extern EventRecord currentEvent, LcurrentEvent;
extern uint16_t eventIndex;

void logCidEvent(uint16_t account, uint8_t qualifier, uint16_t eventcode, uint8_t partition, uint16_t zoneuser);

void WriteEventToTxBuffer(int co_id, EventRecord *event);
void preReadEvent(int co_id, EventRecord *event);
void ReadOutEvent( int co_id, EventRecord *event);

void WriteEventToR3KBuffer(EventRecord *event);
void R3KpreReadEvent(EventRecord *event);
void R3KReadOutEvent( EventRecord *event);
int find_event_by_index( uint16_t fnd_index, uint32_t *eveaddr);

void FormatEventFlash( void );
int HowManyEvents( void );
void Set_evwrptr( uint16_t index );
void Get_evwrptr(void);
int WriteEventToFlash(EventRecord *event);
int verify_event_wr(uint8_t buffer[DF_EVELEN], uint32_t dfindex);
int RawWriteEventToFlash(uint8_t buffer[DF_EVELEN], uint32_t dfindex);
void ReloadUnAckEvents( int monid);
int ReloadUnAckEvents25( int monid);

int WriteEventToAuditFlash(EventRecord *event);
void Get_audwrptr(void);
void Set_audwrptr( uint16_t index );
int ReadEventFromAuditFlash( uint16_t evindex, EventRecord *event );

int ReadEventFromFlash( uint16_t evindex, EventRecord *event );
int BufPrintCidEvent( char *strbuffer, EventRecord *event, int lenbuff );
void RawCID_LogEvent( uint8_t *cidbuff);
int EventCodeToInt(uint16_t eventcode);

uint16_t ZoneCodeToDigits( uint16_t data);
uint8_t PartitionToDigits( uint8_t data);
uint16_t EventCodeToDigits( uint16_t data);
uint16_t AccountToDigits( uint16_t data);

int con_EvDF_format(ConsoleState* state);
int con_DumpEvMemory(ConsoleState* state);
int con_DumpEvIndex(ConsoleState* state);
int con_DumpEventByTime(ConsoleState* state);
int con_audit_dump(ConsoleState* state);

int con_setrtc(ConsoleState* state);
int con_dumprtc(ConsoleState* state);
int con_set1rtc(ConsoleState* state);

time_t ConvTimestamp(char *contimestamp);


#endif /* LOG_EVENT_H_ */
