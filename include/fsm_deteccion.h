/*
 * fsm_deteccion.h
 *
 *  Created on: Dec 15, 2011
 *      Author: IronMan
 */

//#define	AUTORST_PRUEBA	1

#ifndef FSM_DETECCION_H_
#define FSM_DETECCION_H_

#define NABO1_STOP_TXON		201
#define NABO2_STOP_TXON		202

#define	TIME_STARTUP_SCAN		10
#define	TIME_STARTUP_DETECT		TIME_STARTUP_SCAN + 2
#define	TIME_STARTUP_AUTOP		TIME_STARTUP_DETECT + 10
#define	TIME_STARTUP_R3K		TIME_STARTUP_AUTOP + 2

void LLAVE_TX_OFF( void );
void LLAVE_TX_ON( void );

//#define LLAVE_TX_ON() 	GPIO_SetValue(3, 1<<26)
//#define LLAVE_TX_OFF() 	GPIO_ClearValue(3, 1<<26 )
#define POWER_TX_ON() 	GPIO_SetValue(3, 1<<25);
#define POWER_TX_OFF() 	GPIO_ClearValue(3, 1<<25 );

extern uint8_t numerozona;
extern OS_TCB		AlarmDetectTask_TCB;
extern CPU_STK		AlarmDetectTask_Stk[AlarmDetectTask_STK_SIZE];

extern const unsigned char bitpat[8];

extern const uint8_t numabo_z1[200];
extern const uint8_t numabo_z2[200];
extern const uint8_t numabo_z3[200];
extern const uint8_t numabo_z4[200];
extern const uint8_t numabo_z5[200];
extern const uint8_t numabo_z6[200];

extern uint16_t account;
extern uint8_t	BaseAlarm_alarm_1, diff_alarm, event_alarm, restore_alarm;
extern uint8_t BaseAlarm_estados_dispositivos_1, diff_estados, event_estados, restore_estados;
extern uint8_t BaseAlarm_memoria_dispositivos_1, diff_memoria, event_memoria, restore_memoria;

extern uint8_t BaseAlarmPkt_numabo;
extern uint8_t BaseAlarmPkt_alarm;
extern uint8_t BaseAlarmPkt_estado_dispositivos;
extern uint8_t BaseAlarmPkt_memoria_dispositivos;
extern uint8_t TypeAboAns;
extern uint8_t FmodeDelay;

extern uint8_t asal_state;
extern uint8_t teso_state;
extern uint8_t ince_state;
extern uint8_t rotu_state;
extern uint8_t apdisp_state[5];

extern uint16_t dbnc_ince_timer, dbnc_ince2_timer, dbnc_fince2_timer;
extern uint16_t dbnc_asal_timer;
extern uint16_t dbnc_teso_timer;
extern uint16_t dbnc_rotu_timer;
extern uint16_t apdisp_timer[5];
extern uint16_t signature_timer;
extern uint16_t lowbatt_timer;

extern uint32_t fevptimer;
extern uint32_t ftxontimer;
extern uint16_t timerInitCMX;
extern uint16_t timerIrqCMX;

extern uint8_t BaseZone;
extern uint8_t Aper_Poll_counter;

extern uint8_t OptoInputs;			//imagen validada de las entradas optoacopladas
extern uint8_t SysInputs;			//idem de las alarmas del sistema

extern uint8_t SysFlag0;			//flag general del sistema
extern uint8_t SysFlag1;
extern uint8_t	SysFlag3;			//flag para el modulo de aviso inmediato
extern uint8_t	SysFlag4;
extern uint8_t	DebugFlag;

extern uint16_t fsmtx_timer1;
extern uint16_t timerdbncaper;
extern uint16_t timerdbncaperAP;
extern uint16_t timerdbncf220v;
extern uint16_t tachof220v;

extern uint8_t	isrclear;
extern uint8_t 	modem_error;

extern uint16_t preve_timer;
extern uint16_t rfw_timer;

extern uint8_t asal_autr_counter;	//para asalto
extern uint8_t teso_autr_counter;	//para tesoro
extern uint8_t ince_autr_counter;	//para incendio
extern uint8_t rotu_autr_counter;	//para rotura
extern uint8_t autoreset_data;

extern time_t asal_autr_timer;
extern time_t ince_autr_timer;
extern time_t teso_autr_timer;
extern time_t rotu_autr_timer;

extern	uint16_t npd_timer;

extern	uint16_t autorst_timer_min, asal_autorst_timer_min, teso_autorst_timer_min, ince_autorst_timer_min, rotu_autorst_timer_min;
extern	uint16_t autorst_timer;
extern	uint16_t paparst_timer;

extern int dualA_delay, DeltaT;

extern uint32_t amr_timer;
extern int norm_asal_timer, norm_teso_timer;

extern uint8_t ppon_state;
#define FSM_PPON_IDLE   0x10
#define FSM_PPON_ON     0x20
#define FSM_PPON_OFF    0x30
#define FSM_PPON_ALRM   0x40
#define FSM_PPON_WAIT   0x50
#define FSM_PPON_WAITON 0x60
#define FSM_PPON_ALRM2  0x70
#define FSM_PPON_TSTPP  0x80
#define FSM_PPON_TSTPP1 0x81
#define FSM_PPON_TSTPP2 0x82

extern time_t ppon_wdog_timer;

// estados de las maquinas de deteccion
#define	APER_IDLE		0x10
#define	APER_WAIT		0x15
#define	APER_ALRM		0x20
#define	APER_WAIT2		0x25
#define ASALTO_IDLE		0x10
#define ASALTO_WAIT		0x15
#define	ASALTO_ALRM		0x20
#define	ASALTO_INDEF	0x25
#define	ASALTO_INDEF1	0x26
#define	F220_IDLE		0x10
#define	F220_WAIT		0x15
#define	F220_ALRM		0x20
#define INCENDIO_IDLE	0x10
#define INCENDIO_WAIT	0x15
#define	INCENDIO_ALRM	0x20
#define	INCENDIO_INDEF	0x25
#define	INCENDIO_INDEF1	0x26
#define RSTALM_IDLE		0x10
#define RSTALM_ALRM		0x20
#define TESORO_IDLE		0x10
#define TESORO_WAIT		0x15
#define TESORO_WAIT2		0x17
#define	TESORO_ALRM		0x20
#define	TESORO_INDEF	0x25
#define	TESORO_INDEF1	0x26
#define	ROTU_IDLE		0x10
#define	ROTU_WAIT		0x12
#define	ROTU_ALRM		0x20

extern uint8_t fsm_rotrele485_state;
extern uint8_t fsm_rot485_state;
#define	FSM_ROT485_ENTRY	0x05
#define	FSM_ROT485_IDLE		0x10
#define	FSM_ROT485_WAIT		0x20
#define	FSM_ROT485_PWAIT1	0x21
#define	FSM_ROT485_PWAIT2	0x22
#define	FSM_ROT485_PWAIT3	0x23
#define	FSM_ROT485_ROT		0x30
#define	FSM_ROT485_ROT2		0x31
#define	FSM_ROT485_WAIT2    0x22

extern uint8_t fsm_pap_state;
#define FSMPAP_IDLE			0x10
#define FSMPAP_TXING		0x20
#define FSMPAP_WAIT  		0x30

extern uint16_t	Rot485_flag;
#define	ROT485_FLAG			0x0001
#define	ROT491_FLAG			0x0002
#define	ROT232_FLAG			0x0004
#define	ROTEVO_FLAG			0x0010
#define ROT485RELE85_FLAG   0x0020
#define ROT485PTM_FLAG      0x0040
#define ROT485CID_FLAG      0x0080
#define NOZSCAN_FLAG        0x0008
#define CIDRESET_FLAG       0x0100

extern time_t rot485timer;

extern time_t paptimerslot;
extern uint16_t paptslot;

extern uint8_t papalarmbyte;

extern uint8_t	ptxormask;

extern uint16_t paparst_timer;

extern uint32_t timer_llaveon, timer_contpoll, timer_llaveoff, timer_inhcpoll;
extern uint8_t fsm_llon_state;
#define	FSMLLON_IDLE			0x10
#define	FSMLLON_TXON			0x20
#define	FSMLLON_TXOFF			0x30
#define	FSMLLON_CPOLL_ON		0x40
#define	FSMLLON_CPOLL_OFF		0x50
#define	FSMLLON_TXDISP			0x60

extern uint8_t prLlaveState;
extern uint8_t PruebasFlags;
#define	StartCPOLL_flag		0x01
#define TXOFFCMD_flag		0x02
#define TXOFF2CMD_flag		0x04


extern	uint8_t FTXOFF_state;
#define FTXOFF_IDLE			0x10
#define FTXOFF_OFF			0x20
#define FTXOFF_WAIT			0x30

extern	int ftxoff_timer;

extern uint8_t FTXOFF2_state;
#define FTXOFF2_IDLE		0x10
#define FTXOFF2_OFF			0x20
#define FTXOFF2_WAIT		0x30
#define FTXOFF2_ON			0x40

extern uint8_t daper_state;
extern uint8_t daper2_state;
extern uint8_t daper_stateAP;
extern uint8_t dasa_state;
extern uint8_t df220_state;
extern uint8_t dinc_state, dinc2_state, dfinc2_state;
extern uint8_t drst_state;
extern uint8_t dteso_state;
extern uint8_t drotu_state[16];


extern int ftxoff2_timer;

extern int	n_asal, n_teso, n_ince, nmax;
extern int nmax_asal, nmax_teso, nmax_ince;
extern time_t last_ASAL, last_voluclose;

extern uint32_t timer_prueba, timpr_llaveon, timpr_llaveoff, timpr_gap, prretries;
extern uint32_t Timer_prueba, Timpr_llaveon, Timpr_llaveoff, Timpr_gap, Prretries;

#define	PRLLST_IDLE				0x10
#define	PRLLST_ON				0x20
#define	PRLLST_OFF				0x30
#define	PRLLST_WGAP				0x40

//*******************************************************************
//
//				 ---------------------------------------
//	1er byte	|			  	NUMABO					|	Numero de abonado
//				 ---------------------------------------
//	2do byte	|NORM|F220|LLOP|APER|ROTU|TESO|INCE|ASAL|	Alarma
//				 ---------------------------------------
//	3er byte	|EPT3|EPT2|EPT1|EPT0|EDSC|    |    |    |	Estado de dispositivos
//				 ---------------------------------------
//	4to byte	|TPT3|IPT3|TPT2|IPT2|TPT1|IPT1|TPT0|IPT0|	Memoria de disparos
//				 ---------------------------------------
//
//*******************************************************************


//defines de los bits de alarma
#define	NORMAL_bit	7	//0x80
#define	F220_bit	6	//0x40
#define	LLOP_bit	5	//0x20
#define	APER_bit	4	//0x10
#define	ROTU_bit	3	//0x08
#define	TESO_bit	2	//0x04
#define	INCE_bit	1	//0x02
#define	ASAL_bit	0	//0x01

#define	ZONA_ASALTO		0
#define	ZONA_INCENDIO	1
#define	ZONA_TESORO		2
#define ZONA_INCE2      6
#define ZONA_FALLAINCE2 7

//definiciones de los bits en SysInputs
#define	APER_sbit	0x08
#define	LLOP_sbit	0x04
#define	F220_sbit	0x02
#define	RSTA_sbit	0x01
#define DHCPM_sbit	0x10
#define APER2_sbit  0x20



#define	APER_POLL_COUNT		2

// Definicion de flags del sistema
//unsigned char SysFlag0;			//flag general del sistema
#define	R2K_POLL			0x01		//es hora de escanear las zonas
#define	STARTUP_flag		0x02		//a partir de ahora funcionamiento normal
#define	RSTALRM_flag		0x04		//indica que hay que resetear las alarmas
#define	TIMROT_flag			0x08		//cada cuanto debemos hacer deteccion de rotura
#define	RF_POLL_flag		0x10		//indica que llego una encuesta de RF
#define	AUTORSTDATA_flag	0x20
#define	FSMTX_flag			0x40		//indica que trasmita por RF las alarmas del abonado
#define	INPATTERN_flag		0x80		//indica que se esta dentro de un patron de encuesta valido

//uint8_t SysFlag1;			//flags para el modulo de ADC
#define	APER2_FLAG			0x01
#define	ABOMASUNO_flag		0x02
#define	INPPON_flag			0x04
#define SF220_flag          0x08
#define	PREVE_CENTRAL_TX	0x10
#define	PREVE_CENTRAL_RX	0x20
#define	APER13_FLAG 		0x40
#define	AP_APERLED_CTRL		0x80

//uint8_t	SysFlag3;			//flag para el modulo de aviso inmediato
#define	SEND_flag			0x01
#define FSTOP_flag			0x02
#define	FRFPOLL_flag		0x04
#define	RX201_flag			0x08
#define	FMODE_flag			0x10
#define	SENDM_flag			0x20
#define	INWIN_flag			0x40
#define	VALIDRXCHAR_flag	0x80

//uint8_t	SysFlag4;
#define	LOGICALPWRTXOFF		0x01
#define RFFILTER1           0x02
#define RFFILTER2           0x04
//#define AUTOPROT_NORMAL		0x02
#define	SERIALSTART_flag	0x10
#define ABONUMBER_flag      0x20
#define USEPPONWDOG_flag    0x40


//uint8_t	DebugFlag;
#define	DBGRF_ON_flag		0x01
#define DBGABORF_flag		0x02
#define MSGSYSUP_flag       0x04
#define NETRSTHAB_flag      0x08
#define HIGRSTHAB_flag      0x08
#define LAN485DBG_flag      0x20

#define	TIEMPO_PREVE		4200		//7 minutos para indicar preve

//estados de la maquina autoreset
#define	AUTR_NORMAL	0
#define	AUTR_ALRMED	1
#define	AUTR_MEMALR	2

//unsigned char pollsig_state;
#define	FSM_PS_NOISE	0x10
#define	FSM_PS_ABOSYNC	0x11
#define	FSM_PS_DARST	0x12
#define	FSM_PS_NABO		0x13

#define	FSMTX_IDLE		0x10
#define	FSMTX_WAIT1		0x11
#define	FSMTX_TXING		0x12
#define	FSMTX_WAIT2		0x13



#define	AUTORESET_BLINK_CAD		3*0x100 + 3
#define	AUTORESET_POLL_COUNT	1


#ifdef	AUTORST_PRUEBA
#define	AUTORST_TIMER_MIN		2
#define	AUTORST_TIMER_MAX		2
#define	AUTORST_TIMER_DEF		2
#else
#define	AUTORST_TIMER_MIN		3
#define	AUTORST_TIMER_MAX		240
#define	AUTORST_TIMER_DEF		15
#endif


void DeteccionEventos(void);
void AlarmDetectTask(void  *p_arg);
void autoreset_logo3d( void );
void RF_Poll( void );
void fsm_deteccion_f220( void );
void fsm_deteccion_apertura( void );
void fsm_deteccion_apertura2( void );
void fsm_deteccion_aperturaAP( void );
void fsm_deteccion_dispositivos( void );
void fsm_deteccion_rotura( void );
void fsm_deteccion_tesoro( void );
void fsm_deteccion_asalto( void );
void fsm_deteccion_incendio( void );
void fsm_deteccion_incendio2( void );
void fsm_rfwatchdog( void );
void fsm_ppon_wdog( void );

void load_buffer_tx(void);
void fsm_transmit_cmx( void );
void PollingSignature( unsigned char rxchar );
void MDM_IrqHandler( void );

void AlarmWriteHistory(void);
void AlarmReadHistory(void);

void fsm_fastmode(void);
void fsm_init_cmx_preve( void );
void fsm_event_on_preve( void );
void fsm_evimd_llavetx( void );

void fsm_rotura485( void );
void fsm_roturaEVO( void );
void fsm_pap(void);
void rf_cortex_signature( void );
void fsm_deteccion_lowbattery( void );
void fsm_deteccion_NP_umbral( void );

void PTM485NG_HistoryWrite(void);
void PTM485NG_HistoryRead(void);

void fsm_llaveon_wdog( void );
void PruebaLlaveWdog( void );
void LlaveOnEvent( void );

void fsm_txoff(void);
void fsm_txoff2(void);

void recharge_alarm(uint8_t alarm);
void read_nsignals(void);
void write_nsignals(void);
void recharge25min_alarm(uint8_t alarm, uint8_t etype);
void recharge5min_alarm(void);

void refresh_alrm_led( void );

void fsm_roturaRele485( void );

int rxabonum_prev(uint8_t rxchar, int delta);
int IsWrightTimePoll( void );

void FSM_WriteHistory(void);
void FSM_ReadHistory(void);

void Write_PTM_termic(void);
void Read_PTM_termic(void);

#endif /* FSM_DETECCION_H_ */
