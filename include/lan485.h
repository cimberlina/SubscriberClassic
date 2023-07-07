/*
 * lan485.h
 *
 *  Created on: Mar 14, 2013
 *      Author: claudio
 */

#ifndef LAN485_H_
#define LAN485_H_

#define	ANS_PKT_OK		0
#define	ANS_CHK_NG		1

#define MAXQTYPTM	25



#define ASALPBT_FIRSTZONE	0x005
#define ASALPBT_LASTZONE	0x008

#define	LOCKGATE_RTUADDR	247
#define	SKIMMING1_RTUADDR	244
#define	SKIMMING2_RTUADDR	245
#define	SKIMMING3_RTUADDR	246

typedef struct	{
	unsigned char 	rtuaddr;				//direccion en la red 485
	unsigned int 	cuenta;
	unsigned char	particion;
	//unsigned char	curr_command;
	unsigned char	flags;
	unsigned char	com_error_counter;
	uint16_t 	    event_alarm;
	unsigned char	disparo;
	unsigned char	state485;
	unsigned int	timeout485;
	time_t			normtimer;
	unsigned char	normstate;
	unsigned char	version;

	unsigned char   RFALRMDLY_flag;
	time_t          rfalrmdly_timer;
	unsigned char   rfalrmdly_state;


} PTM_device;

typedef struct 
{
	uint16_t	particion;
	uint16_t	zona;
	uint16_t	cuenta;
} PBT_device;

extern int lan485errorpkt;
extern int howmuchdev;
extern long accumulated_errors, totalpakets;

extern PTM_device ptm_dcb[MAXQTYPTM];
extern PBT_device	pbt_dcb[MAXPBTPTM];

extern unsigned int dlyedptm_zone[MAXQTYPTM];
extern unsigned int dlyedevo_part;
extern uint8_t DlyBor_time;
extern int rfdly_time;

extern uint8_t cid_ptm_index;

extern uint8_t         PTMSIGNAL_flag;
#define PTMSIG_PANIC    0x01


extern time_t          ptmsignal_timer;
extern uint8_t         ptmsignal_state;
#define PTMSIGNAL_IDLE          0x10
#define PTMSIGNAL_PANIC_TRG     0x20

#define	PTNORM_IDLE	0x10
#define	PTNORM_WAIT	0x20


#define P485_IDLE	0x10
#define	P485_NG		0x20
#define	P485_WAIT	0x30

//defines de flags en pt dcb
#define	COMM_TROUBLE	0x01
#define	AUTONORMAL		0x02
#define PTMCMD_ARM      0x04
#define RELESIGNAL	    0x40

//estados de la fsm de retardo de alarma de los ptm
#define PTRFDLY_ST_IDLE     0x10
#define PTRFDLY_ST_WAITTRIG 0x20
#define PTRFDLY_ST_WAITE393 0x30

//extern unsigned char   RFALRMDLY_flag;
#define RFALRMDLY_ASAL_FLAG     0x01
#define RFALRMDLY_INCE_FLAG     0x02
#define RFALRMDLY_TESO_FLAG     0x04
#define RFALRMDLY_DISARM_FLAG   0x08
#define RFALRMDLY_BORNERA_FLAG  0x10
#define RFALRMDLY_EVOZ24_FLAG   0x20
#define PTMTERMICOTRIGG         0x40
#define PTMSIG_OLDPANIC         0x80

extern uint8_t RFDLYBOR_flag;
#define RFDLYBOR_TESO_FLAG      0x01
#define RFDLYBOR_TDONEI_FLAG    0x02
#define RFDLYBOR_TDONE_FLAG     0x04
#define RFDLYBOR_E393HAB_FLAG   0x08
#define RFDLYBOR_EVOPART_FLAG   0x10
#define RFDLYBOR_TESOGAP_FLAG   0x20
#define RFDLYBOR_LLOP_FLAG      0x40
#define RFDLYBOR_TDONEP_FLAG    0x80

extern uint32_t fsmhlock_timer;
extern uint8_t fsmhsbclock_state;
#define FSMHBL_IDLE     0x10
#define FSMHBL_LOCK1    0x20
#define FSMHBL_LOCK2    0x30
#define FSMHBL_LOCK12   0x40
#define FSMHBL_LOCKALR  0x50
#define FSMHBL_LOCKALR1  0x51
#define FSMHBL_LOCKALR2  0x52
#define FSMHBL_LOCKLONG 0x60
#define FSMHBL_LOCKLONG1 0x61
#define FSMHBL_LOCKLONG2 0x62

extern uint8_t fsmnppwd;
extern uint32_t fsmnpp_timer;

extern OS_TCB		LAN485_Task_TCB;
extern CPU_STK		LAN485_Task_Stk[LAN485_Task_STK_SIZE];

extern PTM_device ptm_dcb[MAXQTYPTM];
extern uint8_t PTM_dev_status[MAXQTYPTM];
extern uint8_t PDX_dev_status[3];

extern uint8_t  PT_estado_particion[5];
extern uint8_t  VolumetricRedundance[5];

extern unsigned char SysFlag2;
#define	NORM_ASAL		0x01
#define	NORM_INCE		0x02
#define	NORM_TESO		0x04

#define	REMOTEASALT_TRIGGER		888

#define FIRSTPBTZONE	1
#define	LASTPBTZONE		8

extern uint8_t relestate[10], relepulse[10];

unsigned int IntToBCD( unsigned int value );
unsigned int BCDToInt( unsigned int value);
unsigned int UCharToBCD( unsigned char value );
void init_lan_cfgfile( void );
void LAN485_Task(void  *p_arg);
void LAN485_Send( unsigned char sendbuffer[], int bufflen );
void device_poll( unsigned char index );
void SendPtmACK( unsigned char index );
void PTm_process_answer( int nread, unsigned char * rxbuffer, unsigned char index );
void ParsePtmCID_Event( unsigned char event_buffer[] );
void ProcessEvents( unsigned char event_buffer[], unsigned char index );
void ProcessRestoreMainBoardEvents( void );
void GenerateCIDEventPTm( unsigned char index, unsigned char eventtype, unsigned int eventcode, unsigned int eventzone );
void ProcessRestoreByTimeout(void);
void SendProblem485(uint8_t ptm_index, uint8_t erevent);
void guardar_PDX_status(void);
void recuperar_PDX_status(void);
void set_array_bit( uint8_t bit, uint8_t bitarray[]);
void reset_array_bit( uint8_t bit, uint8_t bitarray[]);
int get_array_bit( uint8_t bit, uint8_t bitarray[]);
void openptm_process( void );
void ptm_group_replica(void);
int ModuleInTable( int addr );
void Modules485_Scan( void );
int PBTinTable( int particion, int zone );

void fsm_newptmpwd( void );
void fsm_hsbclock( void );
void fsm_e401_volumetrica_p5(void);
void fsm_e401_volumetrica_p6(void);
void fsm_e401_volumetrica_p7(void);
void fsm_e401_volumetrica_p8(void);
void fsm_e401_volumetrica_p9(void);
uint16_t GetPTMZoneNumber( int particion, int zone12 );

void fsm_rfdlyptm( void );
void fsm_rfdlybornera_teso( void );

void fsm_ptmsignalling( void );

void SendProblem386(uint8_t ptm_index, uint8_t erevent);


//eventos de alarmas que necesitan normalizacion
#define	EVEALRM_PANIC		0x0001
#define	EVEALRM_BURG_IMD	0x0002
#define	EVEALRM_BURG_TMP	0x0004
#define	EVEALRM_TAMPER		0x0008
#define	EVEALRM_TECHMAN		0x0010
#define	EVEALRM_TECHMAN9	0x0020
#define	EVEALRM_ENTRYEXIT	0x0040
#define	EVEALRM_BURG_TERMIC	0x0080
#define	EVEALRM_PANIC128	0x0100
#define PDX_PANICNORMAL     0x0200


extern uint16_t wdevo_event;

extern uint16_t PDX_dev_alarm[18];
extern time_t  PDX_dev_normtimer[18];
extern uint8_t PDX_dev_normstate[18];

extern uint8_t diag485[8];
extern uint8_t habi485[8];
extern uint8_t devfound[32];
extern uint8_t hsbc_lock_partition;

extern uint8_t PartDec_group[10];

#endif /* LAN485_H_ */
