
/*
*********************************************************************************************************
*                                             INCLUDE FILES
.metadata/
*.c~
RemoteSystemsTempFiles/
nbproject/
.settings/
Debug/
.DS_store
Release/
abonado.*
.project
.cproject
._.DS_store
settings/
*********************************************************************************************************
*/
#include "includes.h"



uint8_t SerialNumData[8];

//uint8_t e2promBuffer[128], dflashBuffer[128];

time_t SEC_TIMER;

OS_SEM LogEventRdyPtr;

#define	RADAR_LEN 533
uint8_t	RADAR_count, RADAR_count1, RADAR_flags, RADAR2_flags;

#define	RADAR_COUNT	4

uint8_t CentralPollTimer, dhcpinuse;
time_t startserialtimer;

uint8_t fsm_conent_state;
#define FCS_IDLE    0x10
#define FCS_CONIN   0x20
#define FCS_CONOUT  0x30
#define FCS_PREALRM 0x40
#define FCS_ALRM    0x50
#define FCS_NORMAL  0x60


uint8_t	fsm_radar_state;
#define	ST_RADAR_IDLE           0x10
#define	ST_RADAR_CLOSED	        0x20
#define	ST_RADAR_OPEN           0x30
#define	ST_RADAR_OPEN_F	        0x35
#define	ST_RADAR_PREOPEN        0x40
#define ST_RADAR_NOT_PRESENT    0x45
#define	ST_RADAR_BADMEAS        0x50
#define	ST_RADAR_PRECLOSED      0x55

const unsigned char AES_key[]   = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                          	  	  	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

unsigned char AES_state[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                              0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

#define	WDOG_EVO_TIMEOUT	60*30
uint16_t wdtimer, RHB_Time, RHBCAST_Time;
time_t wdogevotimer[3];
uint8_t fsmwdogevo_state[3];
#define	FSMWDEVO_ENTRY	0x05
#define	FSMWDEVO_IDLE	0x10
#define	FSMWDEVO_ALARM	0x20

uint8_t fsmpgm1_state;
#define	FSMPGM1_OFF		0x10
#define	FSMPGM1_ON		0x20

uint8_t fsmsir1_state;
#define	FSMSIR1_OFF		0x10
#define	FSMSIR1_ON		0x20

uint8_t fsmsir2_state;
#define	FSMSIR2_OFF		0x10
#define	FSMSIR2_ON		0x20

uint8_t	fsmstk1_state;
//#define	FSMSTK_OFF		0x10
//#define	FSMSTK_WAIT1	0x20
//#define	FSMSTK_ON		0x30
//#define	FSMSTK_WAIT2	0x40

uint8_t	fsmstk2_state;


unsigned char TasFlags;
unsigned char fsmtas_state;
#define	FSMTAS_STARTUP		0x05
#define	FSMTAS_IDLE			0x10
#define	FSMTAS_PULSEEND		0x15
#define	FSMTAS_WAIT220OFF	0x20
#define	FSMTAS_220OFF		0x30
#define	FSMTAS_WAIT220ON	0x35
#define	FSMTAS_PULSEON		0x40

uint8_t fsmr3kcid_state;
#define	FR3KCID_IDLE		0x10
#define	FR3KCID_WAITACK		0x20
#define	FR3KCID_WAITACK06	0x25
#define	FR3KCID_WAITCONSOLE	0x30

time_t r3kSendCidTimer, r3khbtimer, r3khb06timer;
uint16_t r3kaccount;

unsigned char sbtimeout;

uint8_t ibuttonid, lic_ibuttonid;

unsigned char sbfsm_state;
#define SBFSM_IDLE			0x10
#define SBFSM_CONECTADO		0x15
#define SBFSM_INPKT			0x20
#define SBFSM_WIBUTTON		0x25
#define SBFSM_WIBUFREE		0x30

void fsm_sboardconf( void );
int sbpgm( unsigned char rxpkt[], uint16_t id);
void fsm_pgm1( void );

//Watchodog time out in 1.5 seconds
#define WDT_TIMEOUT 	5000000

uint8_t SystemFlag, SystemFlag1, SystemFlag2, SystemFlag3, SystemFlag4, SystemFlag5, SystemFlag6;
uint8_t SystemFlag7, SystemFlag8, SystemFlag10;
uint32_t SystemFlag9, SystemFlag11;
uint8_t	EVOWD_Flag;
uint8_t SIRENA_Flag, STRIKE_Flag;


//**************************************************************************
//* Estas variables son para la transmision seria CID a rabbit
EventRecord R3KeventRecord[R3KSERIALBUFFLEN];
int R3KeventRec_writeptr;
int R3KeventRec_readptr;
int R3KeventRec_count;
//**************************************************************************

//**************************************************************************
//* Estas variables son para la transmision seria CID a rabbit
EventRecord LogT_eventRecord[LogT_BUFFLEN];
int LogT_eventRec_writeptr;
int LogT_eventRec_readptr;
int LogT_eventRec_count;
//**************************************************************************

/***************************************************************************
 **
 **  NVIC Interrupt channels
 **
 ***************************************************************************/
#define MAIN_STACK             0  /* Main Stack                                             */
#define RESETI                 1  /* Reset                                                  */
#define NMII                   2  /* Non-maskable Interrupt                                 */
#define HFI                    3  /* Hard Fault                                             */
#define MMI                    4  /* Memory Management                                      */
#define BFI                    5  /* Bus Fault                                              */
#define UFI                    6  /* Usage Fault                                            */
#define SVCI                  11  /* SVCall                                                 */
#define DMI                   12  /* Debug Monitor                                          */
#define PSI                   14  /* PendSV                                                 */
#define STI                   15  /* SysTick                                                */
#define NVIC_WDT              16  /* Watchdog Interrupt (WDINT)                             */
#define NVIC_TIMER0           17  /* Match 0 - 1 (MR0, MR1), Capture 0 - 1 (CR0, CR1)       */
#define NVIC_TIMER1           18  /* Match 0 - 2 (MR0, MR1, MR2), Capture 0 - 1 (CR0, CR1)  */
#define NVIC_TIMER2           19  /* Match 0-3, Capture 0-1                                 */
#define NVIC_TIMER3           20  /* Match 0-3, Capture 0-1                                 */
#define NVIC_UART0            21  /* UART0                                                  */
#define NVIC_UART1            22  /* UART1                                                  */
#define NVIC_UART2            23  /* UART2                                                  */
#define NVIC_UART3            24  /* UART3                                                  */
#define NVIC_PWM1             25  /* Match 0 - 6 of PWM1, Capture 0-1 of PWM1               */
#define NVIC_I2C0             26  /* SI (state change)                                      */
#define NVIC_I2C1             27  /* SI (state change)                                      */
#define NVIC_I2C2             28  /* SI (state change)                                      */
#define NVIC_SPI              29  /* SPI Interrupt Flag (SPIF), Mode Fault (MODF)           */
#define NVIC_SSP0             30  /* SSP0                                                   */
#define NVIC_SSP1             31  /* SSP1                                                   */
#define NVIC_PLL0             32  /* PLL0 Lock                                              */
#define NVIC_RTC              33  /* Counter Increment (RTCCIF), Alarm (RTCALF)             */
#define NVIC_EINT0            34  /* External Interrupt 0 (EINT0)                           */
#define NVIC_EINT1            35  /* External Interrupt 1 (EINT1)                           */
#define NVIC_EINT2            36  /* External Interrupt 2 (EINT2)                           */
#define NVIC_EINT3            37  /* External Interrupt 3 (EINT3)                           */
#define NVIC_ADC              38  /* A/D Converter end of conversion                        */
#define NVIC_BOD              39  /* Brown Out detect                                       */
#define NVIC_USB              40  /* USB                                                    */
#define NVIC_CAN              41  /* CAN Common, CAN 0 Tx, CAN 0 Rx, CAN 1 Tx, CAN 1 Rx     */
#define NVIC_GP_DMA           42  /* IntStatus of DMA channel 0, IntStatus of DMA channel 1 */
#define NVIC_I2S              43  /* irq, dmareq1, dmareq2                                  */
#define NVIC_ETHR             44  /* Ethernet                                               */
#define NVIC_RIT              45  /* Repetitive Interrupt Timer                             */
#define NVIC_MC               46  /* Motor Control PWM                                      */
#define NVIC_QE               47  /* Quadrature Encoder                                     */
#define NVIC_PLL1             48  /* PLL1 Lock                                              */
#define NVIC_USB_ACT          49  /* USB Activity Interrupt                                 */
#define NVIC_CAN_ACT          50  /* CAN Activity Interrupt                                 */
/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB		AppTaskStartTCB; 
static  CPU_STK		AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static	OS_TCB		App_Task1_TCB;
static	CPU_STK		App_Task1_Stk[APP_OS_CFG_TASK_1_STK_SIZE];



NET_IP_ADDR  App_IP_Addr;
NET_IP_ADDR  App_IP_Mask;
NET_IP_ADDR  App_IP_DfltGateway;
NET_IP_ADDR  App_IP_DNS_Srvr;
NET_IP_ADDR  App_IP_NTP_Srvr;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void SoftwareTimersHandler(void);

void AboBoardInit(void);

static  void  App_TaskCreate(void);

static  void  AppTaskStart	(void *p_arg);
static  void  App_Task_1    (void  *p_arg);


uint8_t	fsm_sndpreve;
#define FSM_SNDP_IDLE	0x10
#define	FSM_SNDP_WIND	0x20

uint8_t sndptimer;



void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
{
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;

}
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  err;
    
    

    SysFlag_AP_Apertura = 0;
    SysFlag_AP_Reset = 0;
    SysFlag_AP_GenAlarm = 0;
    SystemFlag = 0;
    SystemFlag1 = 0x00;
    SystemFlag2 = 0x00;
    SystemFlag3 = 0x00;
    SystemFlag4 = 0x00;
    SystemFlag4 |= ARSTOK_FLAG;
    SystemFlag5 = 0x00;
    SystemFlag6 = 0x00;
    SystemFlag7 |= IP150_ALIVE;
    SystemFlag8 = 0x00;
    SystemFlag11 = 0x00;

#ifdef NOEVENTS
    SystemFlag11 |= DONTSENDEVENTS;
#endif

    FSM_FLAG_1 = 0x00;
    EVOWD_Flag = 0x00;
    SIRENA_Flag = 0x00;
    STRIKE_Flag = 0x00;
    SysFlag0 = 0x00;
    SysFlag0 |= STARTUP_flag;
    SysFlag1 = 0x00;
    SysFlag2 = 0x00;
    SysFlag3 = 0x00;
    SysFlag4 = 0x00;

    PruebasFlags = 0x00;
   
    epreve_state = PRV_NORMAL;
    RADAR_t0 = 0;
    RADAR_tickcount = 0;
    CentralPollTimer = 75;
    paptslot = 0;

    startserialtimer = 45;

    GPIO_SetDir(1, (1 << 19), 1);
    GPIO_SetDir(0, (1 << 27), 1);
    PGM1_OFF();

    fsm_radar_state = ST_RADAR_IDLE;
    RADAR_distance = 0;
    RADAR_distance0 = 0;
    RADAR_distance1 = 0;
    RADAR_distance2 = 0;
    RADAR_distance3 = 0;
    RADAR_flags = 0x00;
	RADAR2_flags = 0x00;

    PDX_dev_alarm[0] = 0x00;
    PDX_dev_alarm[1] = 0x00;
    PDX_dev_alarm[2] = 0x00;

    ppon_state = FSM_PPON_IDLE;
    ppon_wdog_timer = SEC_TIMER;

    //determinacion de la causa de reset
//    SystemFlag |= PORRESET_FLAG;
//    SysFlag_AP_Reset |= AP_RST_SYSRESET;
//    SysFlag_AP_Apertura |= AP_APR_SYSRESET;
//    LPC_SC->RSID |= 0x0F;

    if(LPC_SC->RSID & 0x01)	{	//reset por perdida de energia POR
    	SystemFlag |= PORRESET_FLAG;
    	SysFlag_AP_Reset |= AP_RST_SYSRESET;
    	SysFlag_AP_Apertura |= AP_APR_SYSRESET;
    	LPC_SC->RSID |= 0x01;
    }
    if(LPC_SC->RSID & 0x02)	{	//reset por signal RESET
    	SystemFlag |= RESETRESET_FLAG;
    	SysFlag_AP_Reset |= AP_RST_SYSRESET;
    	SysFlag_AP_Apertura |= AP_APR_SYSRESET;
    	LPC_SC->RSID |= 0x02;
    }
    if(LPC_SC->RSID & 0x04)	{	//reset por watchdog
    	SystemFlag |= WDTRESET_FLAG;
    	SystemFlag2 |= APE2WDOG_FLAG;
    	SystemFlag2 |= APE1WDOG_FLAG;
    	WDT_ClrTimeOutFlag();
    	LPC_SC->RSID |= 0x04;
    	SysFlag_AP_Reset &= ~AP_RST_SYSRESET;
    	SysFlag_AP_Apertura &= ~AP_APR_SYSRESET;
    }
    if(LPC_SC->RSID & 0x08)	{	//reset por BOD
    	SystemFlag |= BODRESET_FLAG;
    	LPC_SC->RSID |= 0x08;
    }


    
    
    // - - - - - - - - - - - - - - - - -

//    if(WDT_ReadTimeOutFlag())	{
//    	WDT_ClrTimeOutFlag();
//    	SystemFlag |= WDTRESET_FLAG;
//    	//GPIO_SetValue( 1, 1 << 19);
//    } else	{
//    	GPIO_ClearValue( 1, 1 << 19);
//    	if(LPC_SC->RSID & 0x03)	{
//    		SysFlag_AP_Reset |= AP_RST_SYSRESET;			//notificamos al sistema de autoproteccion que ocurrio un reset
//    		SysFlag_AP_Apertura |= AP_APR_SYSRESET;
//    	}
//    }


    SerialConsoleFlag = 0;
    inconsole_timer = 0;
    fsmpgm1_state = FSMPGM1_OFF;
    fsmsir1_state = FSMSIR1_OFF;
    fsmsir2_state = FSMSIR2_OFF;
    fsmstk1_state = FSMSTK_OFF;
    fsmstk2_state = FSMSTK_OFF;
    fsmtas_state = FSMTAS_STARTUP;
    TasFlags = 0x00;

    fsm_conent_state = FCS_IDLE;


	
    //BSP_IntDisAll();                                            /* Disable all interrupts.                            */
    
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR )AppTaskStart, 
                 (void       *)0,
                 (OS_PRIO     )APP_OS_CFG_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
            
    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */        
    return 1;
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
//#define	ETHLNK_CONNECTED	0x10
//#define	ETHLNK_DISCONNECTED	0x20
uint8_t ethlink_state;

static  void  AppTaskStart (void *p_arg)
{

	OS_ERR      err;
    uint8_t ibutton_id[8];

	
	(void)p_arg;

	ethlink_state = ETHLNK_DISCONNECTED;
	OS_CSP_TickInit();                                          /* Initialize the Tick interrupt.                       */
   
	Mem_Init();
	AboBoardInit();

//	//caca
//	crc = flash0_read(mybuffer, DF_ALARMHISTORY_OFFSET, 16);
//	for(i = 0; i < crc; i++)	{
//		CommSendString(DEBUG_COMM, "{");
//		printByte(mybuffer[i]);													//1
//		CommSendString(DEBUG_COMM, "} ");
//		delay_us(200);
//	}
//	ComPutChar(DEBUG_COMM, 0x0A);
//	ComPutChar(DEBUG_COMM, 0x0D);
//	//fin caca


    InitMonitoreoStruct();
	App_TaskCreate();


	while (DEF_YES) {                                           /* Task body, always written as an infinite loop.       */
		WDT_Feed();
		OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
		//fsm_event_preve();
//		//-------------------------------------------------------------------
//		//Chequeo el estado de la red para ver si es necesario reinicializar
//		retval = NetNIC_PhyLinkState();
//		switch( ethlink_state )	{
//		case ETHLNK_CONNECTED :
//			if(retval == DEF_NO)	{
//				ethlink_state = ETHLNK_DISCONNECTED;
//			}
//			break;
//		case ETHLNK_DISCONNECTED :
//			if(retval == DEF_YES)	{
//				ethlink_state = ETHLNK_CONNECTED;
//				App_InitTCPIP();
//			}
//			break;
//		}
//		//-------------------------------------------------------------------
	}
}

/*
*********************************************************************************************************
*                                       CREATE APPLICATION TASKS
*
* Description : This function creates the application tasks.
*
* Arguments   : None.
*********************************************************************************************************
*/

static  void  App_TaskCreate (void)
{
    OS_ERR  err;

    OSSemCreate(&LogEventRdyPtr, "LogEventRdy", 1, &err);

	OSTaskCreate((OS_TCB     *)&ZoneScanTask_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"ZoneScanTask",
				 (OS_TASK_PTR )ZoneScanTask,
				 (void       *)0,
				 (OS_PRIO     )ZoneScanTask_PRIO,
				 (CPU_STK    *)&ZoneScanTask_Stk[0],
				 (CPU_STK_SIZE)ZoneScanTask_STK_SIZE / 10,
				 (CPU_STK_SIZE)ZoneScanTask_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&FsmLedsTask_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"FsmLedsTask",
				 (OS_TASK_PTR )FsmLedsTask,
				 (void       *)0,
				 (OS_PRIO     )FsmLedsTask_PRIO,
				 (CPU_STK    *)&FsmLedsTask_Stk[0],
				 (CPU_STK_SIZE)FsmLedsTask_STK_SIZE / 10,
				 (CPU_STK_SIZE)FsmLedsTask_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&AlarmDetectTask_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"AlarmDetectTask",
				 (OS_TASK_PTR )AlarmDetectTask,
				 (void       *)0,
				 (OS_PRIO     )AlarmDetectTask_PRIO,
				 (CPU_STK    *)&AlarmDetectTask_Stk[0],
				 (CPU_STK_SIZE)AlarmDetectTask_STK_SIZE / 10,
				 (CPU_STK_SIZE)AlarmDetectTask_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&App_Task1_TCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task1",
                 (OS_TASK_PTR )App_Task_1,
                 (void       *)0,
                 (OS_PRIO     )APP_OS_CFG_TASK_1_PRIO,
                 (CPU_STK    *)&App_Task1_Stk[0],
                 (CPU_STK_SIZE)APP_OS_CFG_TASK_1_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_OS_CFG_TASK_1_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

//	OSTaskCreate((OS_TCB     *)&TelnetServerTask_TCB,                /* Create the start task                                */
//				 (CPU_CHAR   *)"TelnetServerTask",
//				 (OS_TASK_PTR )TelnetServerTask,
//				 (void       *)0,
//				 (OS_PRIO     )TelnetServerTask_PRIO,
//				 (CPU_STK    *)&TelnetServerTask_Stk[0],
//				 (CPU_STK_SIZE)TelnetServerTask_STK_SIZE / 10,
//				 (CPU_STK_SIZE)TelnetServerTask_STK_SIZE,
//				 (OS_MSG_QTY  )0,
//				 (OS_TICK     )0,
//				 (void       *)0,
//				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
//				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&AutoprotectTask_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"AutoprotectTask",
				 (OS_TASK_PTR )AutoprotectTask,
				 (void       *)0,
				 (OS_PRIO     )AutoprotectTask_PRIO,
				 (CPU_STK    *)&AutoprotectTask_Stk[0],
				 (CPU_STK_SIZE)AutoprotectTask_STK_SIZE / 10,
				 (CPU_STK_SIZE)AutoprotectTask_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&RabbitTask_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"RabbitTask",
				 (OS_TASK_PTR )RabbitTask,
				 (void       *)0,
				 (OS_PRIO     )RabbitTask_PRIO,
				 (CPU_STK    *)&RabbitTask_Stk[0],
				 (CPU_STK_SIZE)RabbitTask_STK_SIZE / 10,
				 (CPU_STK_SIZE)RabbitTask_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
#ifdef LAN485_PTM
	OSTaskCreate((OS_TCB     *)&LAN485_Task_TCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"LAN485_Task",
				 (OS_TASK_PTR )LAN485_Task,
				 (void       *)0,
				 (OS_PRIO     )LAN485_Task_PRIO,
				 (CPU_STK    *)&LAN485_Task_Stk[0],
				 (CPU_STK_SIZE)LAN485_Task_STK_SIZE / 10,
				 (CPU_STK_SIZE)LAN485_Task_STK_SIZE,
				 (OS_MSG_QTY  )0,
				 (OS_TICK     )0,
				 (void       *)0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
#endif
    OSTaskCreate((OS_TCB     *)&LogT_Task_TCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"LogT_Task",
                 (OS_TASK_PTR )LogT_Task,
                 (void       *)0,
                 (OS_PRIO     )LogT_Task_PRIO,
                 (CPU_STK    *)&LogT_Task_Stk[0],
                 (CPU_STK_SIZE)LogT_Task_STK_SIZE / 10,
                 (CPU_STK_SIZE)LogT_Task_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

}

 
/*
*********************************************************************************************************
*                                        INITIALIZE TCP/IP STACK
*********************************************************************************************************
*/

void  App_InitTCPIP (void)
{
    NET_ERR      err;
    CPU_BOOLEAN retval;
    uint8_t buffer[16], jumperinuse;
    int error, i, len;
    OS_ERR os_err;
    uint8_t sysinput, sysinput1, sysinput2;

    //----------------------------------------------------
    // leo el ping para ver si tengo que arrancar con DHCP
    sysinput1 = 0;
    sysinput2 = 0;
    if (!(GPIO_ReadValue(1) & (1<<29)))
        sysinput1 |= DHCPM_sbit;
    else
        sysinput1 &= ~DHCPM_sbit;

    OSTimeDlyHMSM(0, 0, 0, 40, OS_OPT_TIME_HMSM_STRICT, &os_err);
    
    if (!(GPIO_ReadValue(1) & (1<<29)))
        sysinput2 |= DHCPM_sbit;
    else
        sysinput2 &= ~DHCPM_sbit;
    
    sysinput = sysinput1 & sysinput2;
    //----------------------------------------------------
    
    //NVIC_EnableIRQ( NVIC_ETHR - NVIC_WDT );

    //OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
    //EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
    //OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    //EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
    //for(i = 0; i < 6; i++ )	{
    //	NetIF_MAC_Addr[i] = buffer[i];
    //}


    check_fidu_dflash();
    
    GetMacaddress(buffer);

    WDT_Feed();
    err = Net_Init();
    if (err != NET_ERR_NONE) {
    	ethlink_state = ETHLNK_DISCONNECTED;
        return;
    }
    WDT_Feed();

    //------------------------------------------------------------
    //Analizo si debo arrancar con DHCP
    retval = EepromReadByte(DHCPUSED_E2P_ADDR, &error);
    switch(retval)	{
    case 0:
    	dhcpinuse = 0;
    	break;
    case 1:
    	dhcpinuse = 1;
    	break;
    default:
    	dhcpinuse = 0;
    	break;
    }

    retval = EepromReadByte(DHCPJUMPER_E2P_ADDR, &error);
    switch(retval)  {
    case 0:
        jumperinuse = 0;
        break;
    case 1:
        jumperinuse = 1;
        break;
    default:
        jumperinuse = 1;
        break;
    }

    //------------------------------------------------------------
    if((dhcpinuse == 1) || ((sysinput & DHCPM_sbit) && (jumperinuse == 1)))	{
        dhcpinuse = 1;
        EepromReadBuffer(DHCPIPADD_E2P_ADDR, buffer, 16, &error);
        App_IP_Addr = NetASCII_Str_to_IP(buffer, &err);
        if( err == NET_IP_ADDR_NONE)    {
            dhcpinuse = 0;
        }
        EepromReadBuffer(DHCPNMASK_E2P_ADDR, buffer, 16, &error);
        App_IP_Mask = NetASCII_Str_to_IP(buffer, &err);
        if( err == NET_IP_ADDR_NONE)    {
            dhcpinuse = 0;
        }
        EepromReadBuffer(DHCPGWADD_E2P_ADDR, buffer, 16, &error);
        App_IP_DfltGateway = NetASCII_Str_to_IP(buffer, &err);
        if( err == NET_IP_ADDR_NONE)    {
            dhcpinuse = 0;
        }
        //if( dhcpinuse == 1) {
        //    logCidEvent(account, 1, 980, 0, 0);
        //}
    }
    if( dhcpinuse == 0) {
		OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	//	EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, 16, &error);
	//	len = strlen(buffer);
	//	App_IP_Addr = NetASCII_Str_to_IP(buffer,    &err);
        App_IP_Addr = GetLocalIpAdd();
		WDT_Feed();

		OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	//	EepromReadBuffer(NETMASK_E2P_ADDR, buffer, 16, &error);
	//	len = strlen(buffer);
	//	App_IP_Mask = NetASCII_Str_to_IP(buffer,    &err);
        App_IP_Mask = GetNetmask();
		WDT_Feed();

		OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &os_err);
	//	EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, 16, &error);
	//	len = strlen(buffer);
	//	App_IP_DfltGateway = NetASCII_Str_to_IP(buffer,    &err);
        App_IP_DfltGateway = GetGateway();
		WDT_Feed();
    }

    //------------------------------------------------------------


//    App_IP_Addr        = NetASCII_Str_to_IP(APP_CFG_IP_ADDR_STR_THIS_HOST,    &err);
//    App_IP_Mask        = NetASCII_Str_to_IP(APP_CFG_IP_ADDR_STR_NET_MASK,     &err);
//    App_IP_DfltGateway = NetASCII_Str_to_IP(APP_CFG_IP_ADDR_STR_DFLT_GATEWAY, &err);
    App_IP_DNS_Srvr    = NetASCII_Str_to_IP(APP_CFG_IP_ADDR_STR_DNS_SRVR,     &err);
    App_IP_NTP_Srvr    = NetASCII_Str_to_IP(APP_CFG_IP_ADDR_STR_NTP_SRVR,     &err);

    NetIP_CfgAddrThisHost(App_IP_Addr, App_IP_Mask);
    NetIP_CfgAddrDfltGateway(App_IP_DfltGateway);
    WDT_Feed();

    retval = NetNIC_PhyLinkState();
    if(retval == DEF_YES)
    	ethlink_state = ETHLNK_CONNECTED;
    else
    	ethlink_state = ETHLNK_DISCONNECTED;

   (void)&App_IP_DNS_Srvr;
   (void)&App_IP_NTP_Srvr;
   NVIC_EnableIRQ( NVIC_ETHR - NVIC_WDT );
}

void account_time(int basehour, int baseminute, int * offset_hour, int * offset_min)
{
    int aminutes, ahours;

    aminutes = account % 100;
    aminutes += baseminute;

    ahours = basehour;

    if( (aminutes >= 60) && (aminutes <= 119)) {
        ahours++;
        if(ahours > 23)
            ahours = 0;
        aminutes -= 60;
    } else if( aminutes >= 120) {
        ahours += 2;
        aminutes -= 120;
    }

    *offset_hour = ahours;
    *offset_min = aminutes;
}


static  void  App_Task_1 (void  *p_arg)
{

	OS_ERR os_err;
	uint8_t ibutton_id[8], i, temp[9], temp2[9];
	uint8_t inchar;
	uint16_t crc;
	uint32_t len, ts_licvenc;
    int error, rndminute, tempint;
    struct tm currtime;
    time_t tmpSEC_TIMER;

    int E785_hour1, E785_minutes1, E785_hour2, E785_minutes2, E785_hour3, E785_minutes3, E785_hour4, E785_minutes4;
    int eveg1_hour, eveg1_minutes, eveg2_hour, eveg2_minutes, eveg3_hour, eveg3_minutes, eveg4_hour, eveg4_minutes;



	//uint8_t mybuffer[16];
	//uint16_t psz;
	//int32_t len = 0;
	//uint32_t offset = 240;

	(void)&p_arg;


    Math_Init();
    error = flash0_read(PartDec_group, DF_GROUPVOLU_OFFSET, 10);
    recuperar_PDX_status();
	App_InitTCPIP();
	WDT_Feed();
    //Si no esta la marca fiduciaria hay que configurar de fabrica
    i = (uint8_t)EepromReadByte(FIDUMARK_E2P_ADDR , &error);
    inchar = (uint8_t)EepromReadByte(FIDUMARK_E2P_ADDR + 1, &error);
    if((i != 0x5A) || (inchar != 0xA5)) {
        FactoryPgm();
    }
    
	R3KeventRec_writeptr = 0;
	R3KeventRec_readptr = 0;
	R3KeventRec_count = 0;

	fsmwdogevo_state[0] = FSMWDEVO_ENTRY;
	fsmwdogevo_state[1] = FSMWDEVO_ENTRY;
	fsmwdogevo_state[2] = FSMWDEVO_ENTRY;

    fsmwdip150_state = FSMWDIP150_ENTRY;
	fsm_gprsdet_state = FSM_GPRSD_INIT;

	r3kSendCidTimer = SEC_TIMER;
	r3khbtimer = SEC_TIMER;
	r3khb06timer = SEC_TIMER;
	fsmr3kcid_state = FR3KCID_IDLE;

	SerialConsoleFlag = 0x00;
	sbfsm_state = SBFSM_IDLE;
	fsm_sndpreve = FSM_SNDP_IDLE;
	AlarmReadHistory();
	WDT_Feed();
    udpserver_init();

    DiagEventFlags = 0x0000;
    
    //freno habilitacion de licencia por memmoria en FF, le pongo en cero para que no tenga licencia
    flash0_read(temp, DF_LICTSVENC_OFFSET, 4);
    ts_licvenc  = temp[3];
    ts_licvenc += ((temp[2] << 8)  & 0x0000FF00);
    ts_licvenc += ((temp[1] << 16) & 0x00FF0000);
    ts_licvenc += ((temp[0] << 24) & 0xFF000000);
    if(ts_licvenc > 1956531661) {
        Mem_Clr(temp, 8);
        flash0_write(1, temp, DF_LICTSVENC_OFFSET, 4);
    }

    SystemFlag5 &= ~SERIALNUM_OK;
    Mem_Clr(temp, 8);
    Mem_Clr(temp2, 8);
    flash0_read(temp, DF_SERIALNUM_OFFSET, 8);
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    EepromReadBuffer(SERIALNUM_E2P_ADDR, temp2, 8, &error);



    for(i = 0; i < 8; i++)  {
        if((temp[i] < '0') || (temp[i] > '9'))  {
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
            flash0_read(temp, DF_SERIALNUM_OFFSET, 8);
            break;
        }
    }
    WDT_Feed();
    for(i = 0; i < 8; i++)  {
        if((temp2[i] < '0') || (temp2[i] > '9'))  {
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
            EepromReadBuffer(SERIALNUM_E2P_ADDR, temp2, 8, &error);
            break;
        }
    }


    WDT_Feed();


    for(i = 0; i < 8; i++)  {               //detecto que no sea igual a cero
        if( (temp[i] != '0') && (temp[i] != 0xFF) )  {
            SystemFlag5 |= SERIALNUM_OK;
        }
    }
    for(i = 0; i < 8; i++)  {               //detecto que no sea igual a cero
        if( (temp2[i] != '0') && (temp2[i] != 0xFF) )  {
            SystemFlag5 |= SERIALNUM_OK;
        }
    }

//    for(i = 0; i < 8; i++)  {               //detecto si todos los digitos son numeros validos
//        if((temp[i] < '0') || (temp[i] > '9'))
//            SystemFlag5 &= ~SERIALNUM_OK;
//    }

    for(i = 0; i < 8; i++)  {
       SerialNumData[i] = temp[i];
    }

    WDT_Feed();
    flash0_read(temp, DF_BLOCKCODE_OFFSET, 1);
    if(temp[0] == 0x5A) {
        SystemFlag5 |= SERIALNUM_OK;
    }

    flash0_read(temp, DF_LICFLAG_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag6 |= USE_LICENSE;
    } else  {
        SystemFlag6 &= ~USE_LICENSE;
    }

    flash0_read(temp, DF_ARADAR_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag6 |= USE_ARADAR;
    } else  {
        SystemFlag6 &= ~USE_ARADAR;
    }
    WDT_Feed();

    flash0_read(temp, DF_ENARHB_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag6 |= ENARHB_FLAG;
    } else  {
        SystemFlag6 &= ~ENARHB_FLAG;
    }

    flash0_read(temp, DF_EVSEND_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag10 |= UDPUSELIC_FLAG;
    } else  {
        SystemFlag10 &= ~UDPUSELIC_FLAG;
        SystemFlag10 |= UDPLICOK_FLAG;
    }

	//Cargo el tiempo de autoreset
	flash0_read(temp, DF_AUTORST_OFFSET, 2);
    if(temp[0] == 0x5A)  {
        autorst_timer = temp[1];
    } else  {
		temp[0] = 0x5A;
		temp[1] = AUTORST_TIMER_DEF;
		error = flash0_write(1, temp, DF_AUTORST_OFFSET, 2);
        autorst_timer = AUTORST_TIMER_DEF;
    }

    //Cargo el tiempo de retardo por bornera
    flash0_read(temp, DF_BORDLY_OFFSET, 2);
    if(temp[0] == 0x5A)  {
        DlyBor_time = temp[1];
    } else  {
        temp[0] = 0x5A;
        temp[1] = 30;
        error = flash0_write(1, temp, DF_BORDLY_OFFSET, 2);
        DlyBor_time = 30;
    }

    //Cargo el tiemo de retardo de los PTM
    flash0_read(temp, DF_PTMDLY_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] > 120))  {
        rfdly_time = temp[1];
    } else  {
        temp[0] = 0x5A;
        temp[1] = 60;
        error = flash0_write(1, temp, DF_PTMDLY_OFFSET, 2);
        rfdly_time = 60;
    }

    flash0_read(temp, DF_E393HAB_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        RFDLYBOR_flag |= RFDLYBOR_E393HAB_FLAG;
    } else
    if((temp[0] == 0xAA) && (temp[1] == 0xBB)) {
        RFDLYBOR_flag &= ~RFDLYBOR_E393HAB_FLAG;
    } else  {
        RFDLYBOR_flag |= RFDLYBOR_E393HAB_FLAG;
        temp[0] = 0x5A;
        temp[1] = 0xA5;
        error = flash0_write(1, temp, DF_E393HAB_OFFSET, 2);
    }


    flash0_read(temp, DF_PPONWDOG_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SysFlag4 |= USEPPONWDOG_flag;
    } else
    if((temp[0] == 0xAA) && (temp[1] == 0xBB)) {
        SysFlag4 &= ~USEPPONWDOG_flag;
    }


	read_nsignals();
	flash0_read(temp, DF_NMAX_OFFSET, 1);
	if(temp[0] < 100)
		nmax = temp[0];
	else {
		temp[0] = 3;
		error = flash0_write(1, temp, DF_NMAX_OFFSET, 1);
		nmax = 3;
	}

	flash0_read(temp, DF_NMAXASAL_OFFSET, 1);
	if(temp[0] < 200)
		nmax_asal = temp[0];
	else {
		temp[0] = 20;
		error = flash0_write(1, temp, DF_NMAXASAL_OFFSET, 1);
		nmax_asal = 20;
	}

	flash0_read(temp, DF_NMAXTESO_OFFSET, 1);
	if(temp[0] < 200)
		nmax_teso = temp[0];
	else {
		temp[0] = 60;
		error = flash0_write(1, temp, DF_NMAXTESO_OFFSET, 1);
		nmax_teso = 60;
	}

	flash0_read(temp, DF_NMAXINCE_OFFSET, 1);
	if(temp[0] < 200)
		nmax_ince = temp[0];
	else {
		temp[0] = 20;
		error = flash0_write(1, temp, DF_NMAXINCE_OFFSET, 1);
		nmax_ince = 20;
	}

	
	flash0_read(temp, DF_HBRSTRTRY_OFFSET, 1);
	if(temp[0] <= HBRESET_RETRIES)	{
		hbreset_retries = temp[0];
	} else if(temp[0] > HBRESET_RETRIES + 1)	{
		temp[0] = 0;
		error = flash0_write(1, temp, DF_HBRSTRTRY_OFFSET, 1);
		hbreset_retries = 0;
	}


	


#ifdef USAR_IRIDIUM
    IRIDIUM_flag &= ~IRI_USEIRIDIUM_FLAG;
	flash0_read(temp, DF_ENAIRI_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        IRIDIUM_flag |= IRI_USEIRIDIUM_FLAG;
    } else  {
        IRIDIUM_flag &= ~IRI_USEIRIDIUM_FLAG;
    }


	flash0_read(temp, DF_IRIMODE_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] >= 0) && (temp[1] <= 4)) {
        switch(temp[1])	{
			case 0:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRI_mode = 0;
				break;
			case 1:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRIDIUM_flag |= IRI_IPDWNHAB_FLAG;
				IRI_mode = 1;
				break;
			case 2:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRIDIUM_flag |= IRI_RFDWNHAB_FLAG;
				IRI_mode = 2;
				break;
			case 3:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRIDIUM_flag |= IRI_BOTHDWNHAB_FLAG;
				IRI_mode = 3;
				break;
			case 4:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRIDIUM_flag |= IRI_ALWAYSHAB_FLAG;
				IRI_mode = 4;
				break;
			case 5:
				IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
				IRIDIUM_flag |= IRI_BOTHDWNHAB_FLAG;
				IRIDIUM_flag |= IRI_GPRSDWN_FLAG;
				IRI_mode = 5;
				break;
			default:
				break;
		}
    } else  {
        IRIDIUM_flag &= ~(IRI_IPDWNHAB_FLAG | IRI_RFDWNHAB_FLAG | IRI_ALWAYSHAB_FLAG | IRI_BOTHDWNHAB_FLAG | IRI_GPRSDWN_FLAG);
		IRI_mode = 0;
    }
#endif


//    flash0_read(temp, DF_PTMPWD_OFFSET, 2);
//    if(temp[1] == 0x5A) {
//        ptm_pwd = temp[0] | 0x80;
//    } else {
//        new_ptm_pwd();
//    }

    r3kmode = EepromReadByte(R3KSENDMODE_E2P_ADDR, &error);
    if( (r3kmode > 4) || (r3kmode < 1) )	{
    	EepromWriteByte(R3KSENDMODE_E2P_ADDR, (uint8_t)1, &error);
    	r3kmode = 1;
    }

    WDT_Feed();
    i = (uint8_t)EepromReadByte(OPENPTM_E2P_ADDR , &error);
    if(i == 0x5A)   {
        SystemFlag5 |= OPENPTM;
    } else {
        SystemFlag5 &= ~OPENPTM;
    }


    i = (uint8_t)EepromReadByte(TASMODE_E2P_ADDR , &error);
    if(i == 0x5A)	{
    	TasFlags |= TASMODE_FLAG;
    } else {
    	TasFlags &= ~TASMODE_FLAG;
    }

    //activacion de modo radar
    i = (uint8_t)EepromReadByte(RADARMODE_E2P_ADDR , &error);
	if(i == 0x5A)	{
		SystemFlag3 |= RADAR_ENABLE;
	} else {
		SystemFlag3 &= ~RADAR_ENABLE;
	}
	WDT_Feed();

    //activacion del chequeo del IP150
    flash0_read(temp, DF_IP150CHK_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag7 |= IP150_CHECK;
    } else  {
        SystemFlag7 &= ~IP150_CHECK;
    }
    WDT_Feed();

    //activacion del chequeo de la firma de RF
    flash0_read(temp, DF_INPATTERN_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag7 |= INPATT_CHECK;
    } else
    if((temp[0] == 0xAA) && (temp[1] == 0xBB)) {
        SystemFlag7 &= ~INPATT_CHECK;
    } else  {
        temp[0] = 0xA5;
        temp[1] = 0x5A;
        error = flash0_write(1, temp, DF_INPATTERN_OFFSET, 2);
        SystemFlag7 &= ~INPATT_CHECK;
    }

    WDT_Feed();

	//activacion del envio de rotura por rf para rhb castillete
    flash0_read(temp, DF_CASTROTU_OFFSET, 2);
    if((temp[0] == 0x5A) && (temp[1] == 0xA5)) {
        SystemFlag10 |= CASTROTUSND_FLAG;
    } else  {
        SystemFlag10 &= ~CASTROTUSND_FLAG;
    }
    WDT_Feed();

    //habilitacion del reset higienico
    flash0_read(temp, DF_HRSTHAB_OFFSET, 2);
    if((temp[0] == 0xA5) && (temp[1] == 0x5A)) {
        DebugFlag &= ~HIGRSTHAB_flag;
    } else  {
        DebugFlag |= HIGRSTHAB_flag;
    }
    WDT_Feed();
    //habilitacion del reset por interrupcion de red
    flash0_read(temp, DF_NRSTHAB_OFFSET, 2);
    if((temp[0] == 0xA5) && (temp[1] == 0x5A)) {
        DebugFlag &= ~NETRSTHAB_flag;
    } else  {
        DebugFlag |= NETRSTHAB_flag;
    }
    WDT_Feed();

	//activacion automatica de wdog de evo
	for(i = 0; i < MAXQTYPTM; i++)	{
		if( (ptm_dcb[i].rtuaddr >= 240) && (ptm_dcb[i].rtuaddr <= 243) )	{
			EepromWriteByte(WDEVOMODE_E2P_ADDR, (uint8_t)0x5A, &error);
			break;
		}
	}


    i = (uint8_t)EepromReadByte(WDEVOMODE_E2P_ADDR , &error);
	if(i == 0x5A)	{
		SystemFlag3 |= WDOG_EVO_ENABLE;
	} else {
		SystemFlag3 &= ~WDOG_EVO_ENABLE;
	}


	i = (uint8_t)EepromReadByte(NPMEDMODE_E2P_ADDR , &error);
	if(i == 0x5A)	{
		SystemFlag4 |= NPMED_FLAG;
	} else {
		SystemFlag4 &= ~NPMED_FLAG;
	}

	wdtimer = EepromReadWord(WDEVOTIMER_E2P_ADDR, &error);
    
    if(wdtimer > 250)
        wdtimer = 120;

    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    RHB_Time = EepromReadWord(RHBTIMER_E2P_ADDR, &error);
    if(RHB_Time > 121)  {
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
        RHB_Time = EepromReadWord(RHBTIMER_E2P_ADDR, &error);
        if(RHB_Time > 121) 
            RHB_Time = 15;
    }
    timer_rhb = RHB_Time*60;

    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    RHBCAST_Time = EepromReadWord(RHBCTIMER_E2P_ADDR, &error);
    if(RHBCAST_Time > 121)  {
        OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
        RHBCAST_Time = EepromReadWord(RHBCTIMER_E2P_ADDR, &error);
        if(RHBCAST_Time > 121) 
            RHBCAST_Time = 15;
    }

	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();
	wdevo_event = EepromReadWord(WDEVOEVENT_E2P_ADDR, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	if(wdevo_event == 0xFFFF)
		wdevo_event = 0x602;

    paptslot = EepromReadWord(PAPTSLOT_E2P_ADDR, &error);
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    paparst_timer = EepromReadWord(PAPARST_TIMER_E2P_ADDR, &error);

	OSTimeDlyHMSM(0, 0, TIME_STARTUP_AUTOP, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();
	SystemFlag1 = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);
	if( SystemFlag1 == 0xFF)
		SystemFlag1 = EepromReadByte(SYSFLAG1_E2P_ADDR, &error);

	tachof220v = EepromReadWord(TACHOF220_E2P_ADDR, &error);

	//SystemFlag3 |= WDOG_EVO_ENABLE;

	//calculo de minutos aleatorios para el reset higienico
    currtime.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);

    srand(currtime.tm_mday);

    rndminute = rand();
	rndminute %= 60;

	WDT_Feed();

	EepromWriteByte(BOOTMARK1_E2P_ADDR, 0xA5, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
	WDT_Feed();
	EepromWriteByte(BOOTMARK2_E2P_ADDR, 0x5A, &error);
	OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);

	for(i = 0; i < 5; i++)	{
		PT_estado_particion[i] = 0x00;
	}
	error = flash0_read(temp, DF_PARTITION_OFFSET, 5);
	for(i = 0; i < 5; i++)	{
		if(temp[i] <= 0x99)	{
			PT_estado_particion[i] = temp[i];
		}
	}

	//redundancia de particiones
    for(i = 0; i < 5; i++)	{
        VolumetricRedundance[i] = 0x00;
    }
    error = flash0_read(temp, DF_VOLREDUNT_OFFSET, 5);
    for(i = 0; i < 5; i++)	{
        if(temp[i] <= 0x99)	{
            VolumetricRedundance[i] = temp[i];
        }
    }

	PTM485NG_HistoryRead();

    Get_audwrptr();

    currtime.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
    error = currtime.tm_year + 1900;
//    if(error > VERSION_YEAR)    {
//        logCidEvent(account, 1, 905, 0, 1);
//    }

    //if(VERSION_NUMBER >= 500)   {
    if(SystemFlag6 & USE_ARADAR)   {
        SystemFlag3 |= RADAR_ENABLE;
        i = EepromReadByte(RADARMODE_E2P_ADDR, &error);
        if(i != 0x5A)   {
            EepromWriteByte(RADARMODE_E2P_ADDR, 0x5A, &error);
        }
    }


    //si la placa es nueva, es decir una EEPROM borrada, le doy 3 horas de licencia para que reconfiguren
    i = (uint8_t)EepromReadByte(FIDUMARK_E2P_ADDR , &error);
    inchar = (uint8_t)EepromReadByte(FIDUMARK_E2P_ADDR + 1, &error);
    if((i != 0x5A) || (inchar != 0xA5)) {
        Mem_Clr(temp, 8);
        ts_licvenc = SEC_TIMER + 3*60*60;
        temp[0] = (ts_licvenc >> 24) & 0x000000FF;
        temp[1] = (ts_licvenc >> 16) & 0x000000FF;
        temp[2] = (ts_licvenc >> 8) & 0x000000FF;
        temp[3] = (ts_licvenc) & 0x000000FF;
        flash0_write(1, temp, DF_LICTSVENC_OFFSET, 4);
    }

    //timer_rhb = TMINRHB*60;
    fsmrhbstate = FSMRHB_IDLE;


    fsmhsbclock_state = FSMHBL_IDLE;
    fsmhlock_timer = 0;


    //flash0_read(temp, DF_RADARP_OFFSET, 1);
    //if((temp[0] == 0x5A) && (SystemFlag6 & USE_ARADAR) ) {
    if(SystemFlag6 & USE_ARADAR)  {
        SystemFlag6 |= USE_RADAR_FLAG;
        //fsm_radar_state = ST_RADAR_IDLE;
    } else {
        SystemFlag6 &= ~USE_RADAR_FLAG;
        //fsm_radar_state = ST_RADAR_NOT_PRESENT;
    }

#ifdef USAR_IRIDIUM
	//---------------------------------------------------
	// Inicializacion del iridium
    sbdixInterval = ISBD_USB_SBDIX_INTERVAL;
    atTimeout = ISBD_DEFAULT_AT_TIMEOUT;
    sendReceiveTimeout = ISBD_DEFAULT_SENDRECEIVE_TIME;
    remainingMessages = -1;
    asleep = FALSE;
    reentrant = FALSE;
    //sleepPin(sleepPinNo),
    //ringPin(ringPinNo),
    msstmWorkaroundRequested = TRUE;
    //ringAlertsEnabled(ringPinNo != -1),
    ringAsserted = FALSE;
    lastPowerOnTime = (0UL);
    head = SBDRING;
    tail = SBDRING;
    nextChar = -1 ;

	IRIDIUM_flag |= IRI_SENDHB_FLAG;
#endif

    //---------------------------------------------------
	last_voluclose = 0;

    account_time(1,0, &E785_hour1, &E785_minutes1);
    account_time(7,0, &E785_hour2, &E785_minutes2);
    account_time(13,0, &E785_hour3, &E785_minutes3);
    account_time(19,0, &E785_hour4, &E785_minutes4);

    account_time(21,0, &eveg1_hour, &eveg1_minutes);     //grupo a tirar el dia 1
    account_time(20,0, &eveg2_hour, &eveg2_minutes);    //grupo a tirar el dia 3
    account_time(22,0, &eveg3_hour, &eveg3_minutes);    //grupo a tirar el dia 5
    account_time(19,0, &eveg4_hour, &eveg4_minutes);    //grupo a tirar el dia 7


	while (DEF_ON) {                                          /* Task body, always written as an infinite loop.       */

		WDT_Feed();
		if(!(TasFlags & TASMODE_FLAG))	{
			fsm_pgm1();
		}

        if(SystemFlag6 & ENARHB_FLAG)   {
            fsm_rhb_wdog();
        }

		fsm_sir1();
		fsm_sir2();
		fsm_strike1();
		fsm_strike2();

#ifdef USAR_IRIDIUM
		//chequeo si tengo un modem gprs conectado en el serial
		fsm_gprs_detection();
#endif

		// a las dos de la maniana me reseteo si no hay eventos en los buffers de monitoreo
		currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
		currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
		currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
		currtime.tm_mon = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH);
        currtime.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
        currtime.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
		currtime.tm_wday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);

		//********************************************************************************************************************
		//transmision del estado del abonado
		if(((currtime.tm_hour == E785_hour1) && (currtime.tm_min == E785_minutes1) && (currtime.tm_sec == 0)) || \
		   ((currtime.tm_hour == E785_hour2) && (currtime.tm_min == E785_minutes2) && (currtime.tm_sec == 0)) || \
		   ((currtime.tm_hour == E785_hour3) && (currtime.tm_min == E785_minutes3) && (currtime.tm_sec == 0)) || \
		   ((currtime.tm_hour == E785_hour4) && (currtime.tm_min == E785_minutes4) && (currtime.tm_sec == 0))
		)    {
		    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		    if((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX))
		        logCidEvent(account, 1, 785, 0, (uint16_t)170);
		    else
		        logCidEvent(account, 1, 785, 0, (uint16_t)BaseAlarmPkt_alarm);
		}

		//transmision de eventos del grupo 1, dia domingo
		if((currtime.tm_hour == eveg1_hour) && (currtime.tm_min == eveg1_minutes) && (currtime.tm_sec == 0) && (currtime.tm_wday == 1))    {
		    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		    //-------------------------------------------------------------------------------------
		    log_nivel_portadora();
		    //-------------------------------------------------------------------------------------
		    for( i = 0; i < MAXQTYPTM; i++) {
		        if(ptm_dcb[i].rtuaddr == 0x00)
		            continue;
		        else {
		            if(PTM_dev_status[i] & 0x02)    {
		                GenerateCIDEventPTm(i, 'E', 917, 0);
		            }
		        }
		    }
		    //-------------------------------------------------------------------------------------
		}

		//transmision de eventos del grupo 2, dia martes
		if((currtime.tm_hour == eveg2_hour) && (currtime.tm_min == eveg2_minutes) && (currtime.tm_sec == 0) && (currtime.tm_wday == 3))    {
		    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		    //-------------------------------------------------------------------------------------
		    for( i = 0; i < MAXQTYPTM; i++) {
		        if(ptm_dcb[i].rtuaddr == 0x00)
		            continue;
		        else {
		            if(PTM_dev_status[i] & 0x40)    {
		                GenerateCIDEventPTm(i, 'E', 530, 1);
		            } else  {
		                GenerateCIDEventPTm(i, 'E', 530, 0);
		            }
		        }
		    }
		    //-------------------------------------------------------------------------------------
		    temp[0] = EepromReadByte(OPENPTM_E2P_ADDR, &error);
		    if(temp[0] != 0xA5)	{
		        logCidEvent(account, 1, 990, 0, 1 );
		    } else	{
		        logCidEvent(account, 1, 990, 0, 0 );
		    }
		    //-------------------------------------------------------------------------------------
		}

		//transmision de eventos del grupo 3, dia jueves
		if((currtime.tm_hour == eveg3_hour) && (currtime.tm_min == eveg3_minutes) && (currtime.tm_sec == 0) && (currtime.tm_wday == 5))    {
		    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		    //-------------------------------------------------------------------------------------
		    if(RADAR2_flags & RADAR_DETECTED_FLAG)	{
		        logCidEvent(account, 1, 995, 0, 1);
		        if(!(SystemFlag3 & RADAR_ENABLE))	{
		            logCidEvent(account, 1, 996, 0, 0);
		        }
		    } else {
		        logCidEvent(account, 1, 995, 0, 0);
		    }
		    //-------------------------------------------------------------------------------------
		    if(SystemFlag6 & ENARHB_FLAG)   {
		        logCidEvent(account, 1, 815, 0, 1);
		    } else  {
		        logCidEvent(account, 1, 815, 0, 0);
		    }
		    //-------------------------------------------------------------------------------------
		    //ahora envio la version de los ptm instalados
		    for( i = 0; i < MAXQTYPTM; i++) {
		        if( (ptm_dcb[i].rtuaddr != 0x00) && (ptm_dcb[i].rtuaddr != 240) && (ptm_dcb[i].rtuaddr != 241) && (ptm_dcb[i].rtuaddr != 242) && (ptm_dcb[i].rtuaddr != 230)) {
		            GenerateCIDEventPTm(i, 'E', 915, ptm_dcb[i].version);
		        }
		    }
		    //-------------------------------------------------------------------------------------
		}

		//transmision de eventos del grupo 4, dia sabado
		if((currtime.tm_hour == eveg4_hour) && (currtime.tm_min == eveg4_minutes) && (currtime.tm_sec == 0) && (currtime.tm_wday == 7))    {
		    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		    //-------------------------------------------------------------------------------------
		    if( dhcpinuse == 1) {
		        logCidEvent(account, 1, 980, 0, 1);
		    } else  {
		        logCidEvent(account, 1, 980, 0, 0);
		    }

		    if (!(GPIO_ReadValue(1) & (1<<29))) {
		        logCidEvent(account, 1, 980, 0, 11);
		    } else  {
		        logCidEvent(account, 1, 980, 0, 10);
		    }
		    //-------------------------------------------------------------------------------------
		    logCidEvent(account, 1, 981, 0, VERSION_NUMBER);
		    //-------------------------------------------------------------------------------------
		    //ahora envio los tres ultimos digitos del numero de serie
		    flash0_read(temp, DF_SERIALNUM_OFFSET, 8);
		    temp[8] = 0;
		    error = atoi(temp);
		    tempint = error % 1000;
		    logCidEvent(account, 1, 982, 0, tempint);
		    error /= 1000;
		    tempint = error % 1000;
		    logCidEvent(account, 1, 983, 0, tempint);
		    //-------------------------------------------------------------------------------------
		    //notificamos si el numero de serie en E2Prom es diferente del que esta en dflash
		    EepromReadBuffer(SERIALNUM_E2P_ADDR, temp2, 8, &error);
		    temp2[8] = 0;
		    tempint = atoi(temp2);
		    error = atoi(temp);
		    if(error != tempint)    {
		        error = tempint % 1000;
		        logCidEvent(account, 1, 984, 0, error);
		        tempint /= 1000;
		        error = tempint % 1000;
		        logCidEvent(account, 1, 985, 0, error);
		    }
		    //-------------------------------------------------------------------------------------
		}
		//********************************************************************************************************************

		//autoreseteo higienico
		if(DebugFlag & HIGRSTHAB_flag)  {
		    //if(!(((Monitoreo[0].eventRec_count > 0) && (Monitoreo[0].inuse != 0)) || ((Monitoreo[1].eventRec_count > 0) && (Monitoreo[1].inuse != 0))) )	{
		        if((currtime.tm_hour == 19) && (currtime.tm_min == rndminute) && (currtime.tm_sec == 0))	{
		            LLAVE_TX_OFF();
		            POWER_TX_OFF();
		            while(1);
		        }
		    //}
		}



#ifdef  ABOSOLITARIO
        if((currtime.tm_hour == 23) && (currtime.tm_min == 0) && (currtime.tm_sec == 0))	{
            LLAVE_TX_OFF();
            POWER_TX_OFF();
            while(1);
        }
        if((currtime.tm_hour == 3) && (currtime.tm_min == 0) && (currtime.tm_sec == 0))	{
            LLAVE_TX_OFF();
            POWER_TX_OFF();
            while(1);
        }
        if((currtime.tm_hour == 8) && (currtime.tm_min == 0) && (currtime.tm_sec == 0))	{
            LLAVE_TX_OFF();
            POWER_TX_OFF();
            while(1);
        }
        if((currtime.tm_hour == 13) && (currtime.tm_min == 0) && (currtime.tm_sec == 0))	{
            LLAVE_TX_OFF();
            POWER_TX_OFF();
            while(1);
        }
        if((currtime.tm_hour == 17) && (currtime.tm_min == 0) && (currtime.tm_sec == 0))	{
            LLAVE_TX_OFF();
            POWER_TX_OFF();
            while(1);
        }
#endif

#ifdef USAR_IRIDIUM6
		//Si iridium esta en uso mando los hb a horarios prefijados.
		if(IRIDIUM_flag & IRI_IRIDIUMOK_FLAG)	{
			if((currtime.tm_hour == 4) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag |= IRI_SENDHB_FLAG;
			}
			if((currtime.tm_hour == 4) && (currtime.tm_min == 1) && (currtime.tm_sec == 0))	{
				IRIDIUM_flag &= ~IRI_IRIHBTDONE_FLAG;
			}

			if((currtime.tm_hour == 10) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag |= IRI_SENDHB_FLAG;
			}
			if((currtime.tm_hour == 10) && (currtime.tm_min == 1) && (currtime.tm_sec == 0))	{
				IRIDIUM_flag &= ~IRI_IRIHBTDONE_FLAG;
			}

			if((currtime.tm_hour == 16) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag |= IRI_SENDHB_FLAG;
			}
			if((currtime.tm_hour == 16) && (currtime.tm_min == 1) && (currtime.tm_sec == 0))	{
				IRIDIUM_flag &= ~IRI_IRIHBTDONE_FLAG;
			}

			if((currtime.tm_hour == 22) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag |= IRI_SENDHB_FLAG;
			}
			if((currtime.tm_hour == 22) && (currtime.tm_min == 1) && (currtime.tm_sec == 0))	{
				IRIDIUM_flag &= ~IRI_IRIHBTDONE_FLAG;
			}
		}
#endif

#ifdef USAR_IRIDIUM1H
		if(IRIDIUM_flag & IRI_IRIDIUMOK_FLAG)	{
			//if(((currtime.tm_min == 0) || (currtime.tm_min == 13) || (currtime.tm_min == 28) || (currtime.tm_min == 43)) \
			//   && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
            if((currtime.tm_min == 13) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_PENDING_FLAG;
			}
			//if(((currtime.tm_min == 0) || (currtime.tm_min == 15) || (currtime.tm_min == 30) || (currtime.tm_min == 45)) \
			//   && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
            if((currtime.tm_min == 15) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_IRIHBTDONE_FLAG)))	{
				IRIDIUM_flag |= IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag |= IRI_SENDHB_FLAG;
			}
			//if(((currtime.tm_min == 2) || (currtime.tm_min == 17) || (currtime.tm_min == 32) || (currtime.tm_min == 47)) \
			//   && (currtime.tm_sec == 0))	{
            if((currtime.tm_min == 17) && (currtime.tm_sec == 0))	{
				IRIDIUM_flag &= ~IRI_IRIHBTDONE_FLAG;
				IRIDIUM_flag &= ~IRI_PENDING_FLAG;
			}
		}
#endif

		//Actualizacion del contador de horas para el intervalo de licencias
		if((currtime.tm_min == 59) && (currtime.tm_sec == 59) &&  (!(SystemFlag4 & LICACT_DONE)))	{
			SystemFlag4 |= LICACT_DONE;

			if(valid_license())    {
			    SystemFlag10 |= UDPLICOK_FLAG;
			} else  {
			    SystemFlag10 &= ~UDPLICOK_FLAG;
			}
            if(!valid_license()) {
                lic_ibuttonid = 0; 
                error = flash0_read(temp, LOGIN_ENABLED, 1);
                if(temp[0] == 0x5A) {
                    temp[0] = 0xA5;         //se acabo la licensia para el login
                    error = flash0_write(1, temp, LOGIN_ENABLED, 1);
                    RADAR_flags &= ~LIC_ENTER;
                }
            }

		}
		if((currtime.tm_min == 0) && (currtime.tm_sec == 10))	{
			SystemFlag4 &= ~LICACT_DONE;
		}



		WDT_Feed();

#ifdef USAR_IRIDIUM
		//Indicacion del estado del iridium
		if((currtime.tm_hour == 5) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(IRIDIUM_flag & IRI_MEASDONE_FLAG)))	{
			//Iridium activado pero no ok
			if((IRIDIUM_flag & IRI_USEIRIDIUM_FLAG) && (!(IRIDIUM_flag & IRI_IRIDIUMOK_FLAG)))	{
				logCidEvent(account, 1, 997, 0, 6); 
			}
			//Iridium no activado pero detectado ok
			if((!(IRIDIUM_flag & IRI_USEIRIDIUM_FLAG)) && (IRIDIUM_flag & IRI_IRIDIUMOK_FLAG))	{
				logCidEvent(account, 1, 997, 0, 7); 
			}

		}
		if((currtime.tm_hour == 5) && (currtime.tm_min == 1) && (currtime.tm_sec == 0))	{
			IRIDIUM_flag &= ~IRI_MEASDONE_FLAG;
		}

		WDT_Feed();

#endif

		//Sincronizacion de timestamp
        if((currtime.tm_hour == 8) && (currtime.tm_min == 30) && (currtime.tm_sec == 0))	{
			temp[0] = 0;
			error = flash0_write(1, temp, DF_HBRSTRTRY_OFFSET, 1);
			hbreset_retries = 0;


            currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
            currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
            currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
            currtime.tm_mon = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH);
            currtime.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
            currtime.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
            currtime.tm_wday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);
			currtime.tm_mon -= 1;
            tmpSEC_TIMER = mktime(&currtime);

            if((SEC_TIMER < (tmpSEC_TIMER - (5*60))) || (SEC_TIMER > (tmpSEC_TIMER + (5*60))))    {
                SEC_TIMER = mktime(&currtime);
                fsm_wdog_evo( 99, 0 );
                //SystemFlag3 |= NAPER_flag;
                //SystemFlag3 |= NAPER_F220V;
            }
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);

        }


		// Desactivacion de reles de control para Patagonia, electroiman y pir, si corresponde
		if((currtime.tm_hour == 7) && (currtime.tm_min == 0) && (currtime.tm_sec == 0) && (!(SystemFlag4 & DOORPAT_FLAG)))	{
			STRIKE_Flag |= STRIKE1_OFF_FLAG;
			STRIKE_Flag |= STRIKE2_OFF_FLAG;
			if((relestate[0] & bitpat[0]) || (relestate[0] & bitpat[1]))	{
				logCidEvent(account, 1, 978, 0, 0);		//Apertura automatica de puerta
			}
			relestate[0] &= ~bitpat[0];
			relestate[0] &= ~bitpat[4];
			relestate[0] &= ~bitpat[1];
			relestate[0] &= ~bitpat[5];
			len = flash0_write(1, relestate, DF_RELESTATES, 10);

			SystemFlag4 |= DOORPAT_FLAG;
		}
		if((currtime.tm_hour == 7) && (currtime.tm_min == 10) && (currtime.tm_sec == 0))	{
			SystemFlag4 &= ~DOORPAT_FLAG;
		}


		if(Monitoreo[0].inuse == 1)
			fsm_wdog_r3k(0);

//		if(Monitoreo[1].inuse == 1)
//			fsm_wdog_r3k(1);
		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		OSTimeDlyHMSM(0, 0, 0, 100,
                      OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
		WDT_Feed();
		if(SerialConsoleFlag & INSERCON_flag)	{
        	console_tick();
        	if(!inconsole_timer)	{
        		SerialConsoleFlag &= ~INSERCON_flag;
        		CommSendString(DEBUG_COMM, "\n***  END OF SERIAL CONSOLE MODE  ***\n\r");
                RADAR_flags |= CONSOLE_OUT;
        	}
        }

        // aca damos lugar a la maquina que maneja la placa de configuracion
        if(SerialConsoleFlag & INCONFBOARD_flag)	{
        	fsm_sboardconf();
        	if(!(SerialConsoleFlag & INCONFBOARD_flag))	{

        	}
        } else
        //chequeo si presentaron un iButton
        if(iButton_read(ibutton_id))	{
        	if( (ibuttonid = check_id_2(ibutton_id)) )	{
        		for( i = 0; i < 8; i++ )
        			ibutton_id[i] = 0;
        		SysFlag_AP_Apertura |= AP_APR_IBUTTON_OK;	//aviso a la maquina de autoproteccion por apertura, que pusieron un ibutton
        		SysFlag_AP_Reset |= AP_RST_IBUTTON_OK;		//aviso a la maquina de autoproteccion por reset, que pusieron un ibutton
        	}
        }
        WDT_Feed();
	//------------------------------------------------------------------------
	//Servidor UDP de comandos de configuracion remota
	UDP_cmd_server();
#ifdef R3K_SERIAL
	if( (r3kmode == 2) || (r3kmode == 3) || (r3kmode == 4))	{
		fsm_R3kSendCid();
	}
#endif

    //-----------------------------------------------------------------------
    // Cargo el estado de preve en los bits disponibles del estado de dispositivos
    BaseAlarmPkt_estado_dispositivos |= 0x04;   //se incluye estados de preve
    if(SysFlag1 & PREVE_CENTRAL_TX)
        BaseAlarmPkt_estado_dispositivos |= 0x01;
    else
        BaseAlarmPkt_estado_dispositivos &= ~0x01;
    if(SysFlag1 & PREVE_CENTRAL_RX)
        BaseAlarmPkt_estado_dispositivos |= 0x02;
    else
        BaseAlarmPkt_estado_dispositivos &= ~0x02;


        //------------------------------------------------------------------------
        // Interprete de comandos desde serial, para modelo que usa R3K
        while( !CommIsEmpty(DEBUG_COMM) && (!(SerialConsoleFlag & INSERCON_flag)) && (!(SerialConsoleFlag & INCONFBOARD_flag)) && (SysFlag4 & SERIALSTART_flag) && (OptoInputs != 0x84))	{

        	//-------------------------------------------------
        	//voy a controlar la ventana de indicacion de preve
        	switch(fsm_sndpreve)	{
        		case FSM_SNDP_IDLE :
        			if(SysFlag3 & SENDM_flag)	{
        				SysFlag3 &= ~SENDM_flag;
        				fsm_sndpreve = FSM_SNDP_WIND;
        				SysFlag3 |= INWIN_flag;
        				sndptimer = 5;
        			}
        			break;
        		case FSM_SNDP_WIND :
        			if(SysFlag3 & SENDM_flag)	{
        				SysFlag3 &= ~SENDM_flag;
        				SysFlag3 |= INWIN_flag;
        				sndptimer = 5;
        			} else
        			if(!sndptimer)	{
        				SysFlag3 &= ~INWIN_flag;
        				fsm_sndpreve = FSM_SNDP_IDLE;
        			}
        		    break;
        		default :
        			fsm_sndpreve = FSM_SNDP_IDLE;
        			break;
        	}
        	//-------------------------------------------------

        	inchar = ComGetChar(DEBUG_COMM);
        	if((inchar == ' ') && ((r3kmode == 1) || (r3kmode == 3)))	{		//esta es la enuesta de la R3K
#ifdef USAR_IRIDIUM
				IRIDIUM_flag |= IRI_GPRSKICK1_FLAG;
#endif
        		ComPutChar(DEBUG_COMM, BaseAlarmPkt_numabo);
        		delay_us(200);
        		crc = BaseAlarmPkt_numabo;
        		if((TypeAboAns == 5) || (TypeAboAns == 6) || (TypeAboAns == 7)) {
        		    ComPutChar(DEBUG_COMM, BaseAlarmPkt_alarm);										//1
        		    delay_us(200);
        		    crc += BaseAlarmPkt_alarm;
        		} else {
        		    if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
        		        ComPutChar(DEBUG_COMM, 0x80);;													//1
        		        delay_us(200);
        		        crc += 0x80;
        		    } else	{
        		        ComPutChar(DEBUG_COMM, BaseAlarmPkt_alarm);										//1
        		        delay_us(200);
        		        crc += BaseAlarmPkt_alarm;
        		    }
        		}
         		ComPutChar(DEBUG_COMM, BaseAlarmPkt_estado_dispositivos);
        		delay_us(200);
        		crc += BaseAlarmPkt_estado_dispositivos;
        		ComPutChar(DEBUG_COMM, BaseAlarmPkt_memoria_dispositivos);
        		delay_us(200);
        		crc +=  BaseAlarmPkt_memoria_dispositivos;
        		if((TypeAboAns == 5) || (TypeAboAns == 6) || (TypeAboAns == 7)) {
        		    ComPutChar(DEBUG_COMM, 0x00);									//1
        		    delay_us(200);
        		} else {
        		    if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
        		        if(SysFlag1 & PREVE_CENTRAL_TX)	{
        		            ComPutChar(DEBUG_COMM, 0x01);
        		            crc += 0x01;
        		        }
        		        else if(SysFlag1 & PREVE_CENTRAL_RX)	{
        		            ComPutChar(DEBUG_COMM, 0x02);													//4
        		            crc += 0x02;
        		        }
        		        delay_us(200);

        		    } else	{
        		        ComPutChar(DEBUG_COMM, 0x00);													//4
        		        delay_us(200);
        		    }
        		}
        		ComPutChar(DEBUG_COMM, (unsigned char)(crc & 0x00FF));
        		delay_us(200);
        		SystemFlag |= R3KSERSPACE;
        	} else
        	if(inchar == 0x06)	{	//0x06
        		SystemFlag |= R3KSERACK;
        	} else
        	if( inchar == '-')	{
        	    //***************************
                autoreset_data = 0xEF;
                SysFlag0 |= RF_POLL_flag;
                SysFlag0 |= FSMTX_flag;
                SystemFlag3 |= NAPER_RFPOLL;
                //***************************
        		// DEBUG ---------------------------------------------------------
        		CommSendString(DEBUG_COMM, "ABODATA: \t");
        		CommSendString(DEBUG_COMM, "[");
        		printByte(BaseAlarmPkt_numabo);											//0
        		CommSendString(DEBUG_COMM, "] ");
				delay_us(200);
				crc = BaseAlarmPkt_numabo;
				if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
					CommSendString(DEBUG_COMM, "[");
					printByte(0x80);													//1
					CommSendString(DEBUG_COMM, "] ");
					delay_us(200);
					crc += 0x80;
				} else	{
					CommSendString(DEBUG_COMM, "[");
					printByte(BaseAlarmPkt_alarm);										//1
					CommSendString(DEBUG_COMM, "] ");
					delay_us(200);
					crc += BaseAlarmPkt_alarm;
				}
				CommSendString(DEBUG_COMM, "[");
				printByte(BaseAlarmPkt_estado_dispositivos);							//2
				CommSendString(DEBUG_COMM, "] ");
				delay_us(200);
				crc += BaseAlarmPkt_estado_dispositivos;
				CommSendString(DEBUG_COMM, "[");
				printByte(BaseAlarmPkt_memoria_dispositivos);							//3
				CommSendString(DEBUG_COMM, "] ");
				delay_us(200);
				crc +=  BaseAlarmPkt_memoria_dispositivos;
				if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
					CommSendString(DEBUG_COMM, "[");
					if(SysFlag1 & PREVE_CENTRAL_TX)	{
						printByte(0x01);
						crc += 0x01;
					}
					else if(SysFlag1 & PREVE_CENTRAL_RX)	{
						printByte(0x02);													//4
						crc += 0x02;
					}
					CommSendString(DEBUG_COMM, "] ");
					delay_us(200);

				} else	{
					CommSendString(DEBUG_COMM, "[");
					printByte( 0x00);													//4
					CommSendString(DEBUG_COMM, "] ");
					delay_us(200);
				}
				CommSendString(DEBUG_COMM, "[");
				printByte((unsigned char)(crc & 0x00FF));								//5
				CommSendString(DEBUG_COMM, "] ");
				delay_us(200);
				ComPutChar(DEBUG_COMM, 0x0A);
				ComPutChar(DEBUG_COMM, 0x0D);
				SystemFlag |= R3KSERSPACE;
        		//----------------------------------------------------------------
        	} else if((inchar == CONSOLEIN_CTRLCHAR) || (inchar == CONSOLEIN_CTRLCHAR1))	{		//Ctrl-B, entra en modo consola
                if(SystemFlag6 & USE_LICENSE)   {
                    if(valid_license()) 
                        CommSendString(DEBUG_COMM, "LIC OK\n\r");
                    else
                        CommSendString(DEBUG_COMM, "LIC NG\n\r");
                } else
                    CommSendString(DEBUG_COMM, "LIC OK\n\r");
                
        		CommSendString(DEBUG_COMM, "\n\n\n***  START OF SERIAL CONSOLE MODE  ***\n\r");
        		console_init();
        		SerialConsoleFlag |= INSERCON_flag;
                RADAR_flags |= CONSOLE_ENTER;
                
                //-----------------------------------------------------------------------------------------
#ifndef  USAR_LICENSIA     
                if(!(SystemFlag6 & USE_LICENSE))    {          
                    RADAR_flags |= LIC_ENTER;       // !!!! OJO: para que no necesite licensia - Despues sacar.
                }
#endif                
                //-----------------------------------------------------------------------------------------
        		
                inconsole_timer = 3*60;		//arranco dandole tres minutos de consola
        	} else if((inchar == '.') && (SysFlag_AP_Apertura & AP_APR_VALID))	{
        		if(EepromReadByte(REDBOXHAB_E2P_ADDR, &error))	{
        			SerialConsoleFlag |= INCONFBOARD_flag;
        			ComPutChar(DEBUG_COMM, '{');
        			sbtimeout = 60;
        		}
        	} else if( inchar == 'w')   {
                EVOWD_Flag |= (1 << 0);
                EVOWD_Flag |= (1 << 1);
                EVOWD_Flag |= (1 << 2);
                CommSendString(DEBUG_COMM, "w_");
            }

        }
        //------------------------------------------------------------------------
	}
} 


void AboBoardInit(void)
{
	EXTI_InitTypeDef EXTICfg;
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;
	struct tm tmbuff;
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	
	uint8_t this, i, j;

	uint16_t post_err;
	uint32_t retval;

	//InitUART_Debug();
	//CommSendString(DEBUG_COMM, "\nEYSE\n\r");
	//CommSendString(DEBUG_COMM, "Cortex Subscriber Initializing ...\n\n\r");

	//************************************************************************************************************************************
	// LED3: P0.0, LED4: P0.1, LED5: P0.5, LED6: P0.22, LED7: P0.26, LED8: P1.23, LED1: P2.3, LED2: P2.4, salidas
	GPIO_SetDir(0, ((1 << 26)|(1<< 22)|(1 << 5)|(1 << 1)|(1 << 0)), 1);
	GPIO_SetDir(1, (1 << 23), 1);
	GPIO_SetDir(2, ((1 << 4)|(1 << 3)), 1);
	GPIO_ClearValue(0, ((1 << 26)|(1<< 22)|(1 << 5)|(1 << 1)|(1 << 0)));
	GPIO_ClearValue(1, (1 << 23));
	GPIO_ClearValue(2, ((1 << 4)|(1 << 3)));

	//RF_CTRL1 = P3.25, RF_CTRL2 = P3.26, salidas
	GPIO_SetDir(3, ((1<< 25)|(1 << 26)),1);
	LLAVE_TX_OFF();
	POWER_TX_OFF();


	//ECHO: P0.4, MDM_CS: P0.6, CS_DFLASH0: P0.16, CS_DFLASH1: P1.24, salidas
	GPIO_SetDir(0, ((1 << 6)|(1 << 16)), 1);
	GPIO_SetDir(1, (1 << 24), 1);
	GPIO_SetDir(0, (1 << 4), 0);

	//LAN1_EN: P0.21, LAN2_EN: P1.18, salidas
	GPIO_SetDir(0, (1 << 21), 1);
	GPIO_SetDir(1, (1 << 18), 1);

	//BUZZER: P0.30, salida
	GPIO_SetDir(0, (1 << 29), 1);
	GPIO_SetDir(0, (1 << 30), 1);
	//GPIO_SetValue(0, (1 << 30));


	//PGM1: P0.27, SIMPWRKEY: P1.20, IR_OUT: P1.21, SIRENA1: P1.25, SIRENA2: P1.31, PERIFPWR: P2.8, BATTCTRL: P2.13, salidas
	GPIO_SetDir(0, (1 << 27), 1);
	GPIO_SetDir(1, ((1 << 20)|(1 << 21)|(1 << 22)|(1 << 25)|(1 << 31)), 1);
	GPIO_SetDir(2, ((1 << 8)|(1 << 13)), 1);
	GPIO_ClearValue(1, (1 << 22));		//bajo el trigger del ultrasonico
	PGM1_OFF();
	PERPWR_ON();

	//APER: P0.28, NETLIGHT: P1.19, IR_IN: P1.22, LEDMODE: P1.29, F220: P2.9, IBUTTIN: P2.12, entradas
	GPIO_SetDir(0, (1 << 28), 0);
	GPIO_SetDir(1, (1 << 19), 1);
	GPIO_SetDir(1, (1 << 29), 0);
	GPIO_SetDir(2, ((1 << 9)|(1 << 12)), 0);

    //Le pongo pull-up al pin 1.29 para deteccion de DHCP mode
    PinCfg.Funcnum = PINSEL_FUNC_0;
    PinCfg.Portnum = 1;
    PinCfg.Pinnum = 29;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PINSEL_ConfigPin(&PinCfg);



	//************************************************************************************************************************************
	//Configuracion de SPI/SSP0 para DataFlash,   P0.15: DF_SCK, P0.17: DF_MISO, P0.18: DF_MOSI, P0.16: CS_DFLASH0, P1.24: CS_DFLASH1
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 15;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 17;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 18;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 16;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetValue(0, 1<<16);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 24;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetValue(1, 1<<24);
	

	SSP_ConfigStructInit(&SSP_ConfigStruct);
	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP0, &SSP_ConfigStruct);
	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP0, ENABLE);

	retval = flash0_init();
//	if (!retval) {
//		CommSendString(DEBUG_COMM, "DataFlash0: \t\t\tFailed to initialize\r\n");
//	} else
//		CommSendString(DEBUG_COMM, "DataFlash0: \t\t\tOK\r\n");

	//************************************************************************************************************************************
	//Configuracion de SPI/SSP1 para Modem,   P0.7: MDM_SCK, P0.8: MDM_MISO, P0.9: MDM_MOSI, P0.6: MDM_CS
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 6;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetValue(0, 1<<6);

	//SSP_ConfigStructInit(&SSP_ConfigStruct);

	SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST;
	SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
	SSP_ConfigStruct.ClockRate = 400000;
	SSP_ConfigStruct.Databit = SSP_DATABIT_8;
	SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
	SSP_ConfigStruct.FrameFormat = SSP_FRAME_SPI;

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

	//************************************************************************************************************************************
	// Inicializacion de modem CMX869
	//		Configuracion de la interrupcion de modem
	isrclear = 0;
	modem_error = 0;
	/* Setting P2.11 as EINT1 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 11;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	/* Initialize External 0 interrupt */
	EXTI_Init();
	EXTICfg.EXTI_Line = EXTI_EINT1;
	/* edge sensitive */
	EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&EXTICfg);
	//NVIC_SetPriorityGrouping(4); //sets group priorities: 8 - subpriorities: 3
	//NVIC_SetPriority(EINT1_IRQn, 2);  //000:10 (bit 7:3)  assign eint0 to group 0, sub-priority 2 within group 0
	NVIC_EnableIRQ(EINT1_IRQn);

	
	post_err = InitCMX869();
		
//	if ((post_err & (CMX_TXRDY | CMX_RXRDY))&&(!(post_err & 0x0200))) {
//		CommSendString(DEBUG_COMM, "MDM CMX869: \t\t\tOK\r\n");
//	} else
//		CommSendString(DEBUG_COMM, "MDM CMX869: \t\t\tFailed to initialize\r\n");

	//************************************************************************************************************************************
	// Inicializacion I2C para EEPROM
	retval = I2CInit( (uint32_t)I2CMASTER );
//	if(retval)
//		CommSendString(DEBUG_COMM, "I2C EEPROM: \t\t\tOK\r\n");
//	else
//		CommSendString(DEBUG_COMM, "I2C EEPROM: \t\t\tFailed to initialize\r\n");

	//************************************************************************************************************************************
	// Inicializacion del ADC

	// Configuracion del pin ADC0.0 en P0.23
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	// Configuracion del pin ADC0.1 en P0.24
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 24;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	// Configuracion del pin ADC0.2 en P0.25
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 25;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_0,DISABLE);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_1,DISABLE);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_2,DISABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_1,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_2,ENABLE);
	//Start burst conversion
#ifdef ADC_SCAN_VERSION_0
	ADC_BurstCmd(LPC_ADC,ENABLE);
#endif

	//Configuracion de los pines de addres del mux CD4051
	// Configuracion del pin AD_ADD0 en P1.26
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 2;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	// Configuracion del pin AD_ADD0 en P1.27
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 2;
	PinCfg.Pinnum = 27;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	// Configuracion del pin AD_ADD0 en P1.26
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 2;
	PinCfg.Pinnum = 28;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	InitZoneScan();

	//************************************************************************************************************************************
	// Configuracion del RTC
	// Init RTC module
	RTC_Init(LPC_RTC);

	/* Disable RTC interrupt */
	NVIC_DisableIRQ(RTC_IRQn);
	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(RTC_IRQn, ((0x01<<3)|0x01));

	/* Enable rtc (starts increase the tick counter and second counter register) */
	RTC_ResetClockTickCounter(LPC_RTC);
	RTC_Cmd(LPC_RTC, ENABLE);
	RTC_CalibCounterCmd(LPC_RTC, DISABLE);

	/* Set current time for RTC */
	tmbuff.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
	tmbuff.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
	tmbuff.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
	tmbuff.tm_mon = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH);
	tmbuff.tm_year = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
	tmbuff.tm_mday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
	tmbuff.tm_wday = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);


	tmbuff.tm_mon -= 1;
	SEC_TIMER = mktime(&tmbuff);


	/* Set the CIIR for second counter interrupt*/
	RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);

	/* Enable RTC interrupt */
	NVIC_EnableIRQ(RTC_IRQn);

	//************************************************************************************************************************************
	//
	//inicializacion de dcb de leds basicos

		led_dcb[NORMAL_led].led_mask = NORMAL_led_mask;
		led_dcb[APER_led].led_mask = APER_led_mask;
		led_dcb[LLOP_led].led_mask = LLOP_led_mask;
		led_dcb[F220_led].led_mask = F220_led_mask;
		led_dcb[ROTU_led].led_mask = ROTU_led_mask;
		led_dcb[TESO_led].led_mask = TESO_led_mask;
		led_dcb[INCE_led].led_mask = INCE_led_mask;
		led_dcb[ASAL_led].led_mask = ASAL_led_mask;
		for( this = 0; this < 8; this++ )	{
			led_dcb[this].led_state = LED_IDLE;
			led_dcb[this].led_cad = 0x0000;
			led_dcb[this].led_tim_off = 0x0000;
			led_dcb[this].led_tim_on = 0x0000;
			led_dcb[this].led_flags = 0x00;
			led_dcb[this].led_blink = 0;
		}
		bledShadow = 0x00;

		led_swp[NORMAL_led].led_mask = NORMAL_led_mask;
		led_swp[APER_led].led_mask = APER_led_mask;
		led_swp[LLOP_led].led_mask = LLOP_led_mask;
		led_swp[F220_led].led_mask = F220_led_mask;
		led_swp[ROTU_led].led_mask = ROTU_led_mask;
		led_swp[TESO_led].led_mask = TESO_led_mask;
		led_swp[INCE_led].led_mask = INCE_led_mask;
		led_swp[ASAL_led].led_mask = ASAL_led_mask;
		for( this = 0; this < 8; this++ )	{
			led_swp[this].led_state = LED_IDLE;
			led_swp[this].led_cad = 0x0000;
			led_swp[this].led_tim_off = 0x0000;
			led_swp[this].led_tim_on = 0x0000;
			led_swp[this].led_flags = 0x00;
			led_swp[this].led_blink = 0;
		}
		bledShadow2 = 0x00;


		AP_Aper_led_dcb.led_state = LED_IDLE;
		AP_Aper_led_dcb.led_cad = 0x0000;
		AP_Aper_led_dcb.led_tim_off = 0x0000;
		AP_Aper_led_dcb.led_tim_on = 0x0000;
		AP_Aper_led_dcb.led_blink = 0;

		Buzzer_dcb.led_state = LED_IDLE;
		Buzzer_dcb.led_cad = 0x0000;
		Buzzer_dcb.led_tim_off = 0x0000;
		Buzzer_dcb.led_tim_on = 0x0000;
		Buzzer_dcb.led_blink = 0;

		for( i = 0; i < 11; i++)	{
			for( j = 0; j < 8; j++)	{
				led_screen[i][j].led_mask = (0x80 >> j);
			}
			SLedShadow[i] = 0x00;
		}

	InitUART_Debug();
	InitUART_LAN1();
	InitUART_LAN2();

	// Inicializacion del estado de fsm
	AP_zvolt_state = AP_ZVOLT_ENTRY;


	//habilito la interrupcion del ultrasonico
	LPC_GPIOINT->IO0IntEnF |= (1<<4);
	NVIC_EnableIRQ(EINT3_IRQn);


	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1;
	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_ResetCounter(LPC_TIM1);
	TIM_Cmd(LPC_TIM1,ENABLE);


	LPC_I2C1->I2CONCLR = I2CONCLR_AAC;
	LPC_I2C1->I2CONCLR = I2CONCLR_SIC;

	//************************************************************************************************************************************
	// Inicializacion del watchdog
	// Initialize WDT, IRC OSC, interrupt mode, timeout = 5000000us = 5s
	WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);
	//Start watchdog with timeout given
	WDT_Start(WDT_TIMEOUT);
	WDT_Feed();
}


time_t MSEC_TIMER;
void SoftwareTimersHandler(void)
{
	static uint16_t timer1seg, timerRadar;
        static uint64_t radarcalctemp;
    uint8_t temp[8];

	int i, len;

	MSEC_TIMER++;
		//La dispara el systick desde el systick handler del ucos-iii
		//implementa una actualizacion de timers cada 1 msec.

        if(fsmtx_timer1)
            fsmtx_timer1--;

		//timer para maquina de transmision por rf
		if(fsmhlock_timer)
            fsmhlock_timer--;

		if(dbnc_ince_timer)
			dbnc_ince_timer--;
		if(dbnc_asal_timer)
			dbnc_asal_timer--;
		if(dbnc_teso_timer)
			dbnc_teso_timer--;
		if(dbnc_rotu_timer)
			dbnc_rotu_timer--;

		for( i = 0; i < 5; i++)	{
			if(apdisp_timer[i])
				apdisp_timer[i]--;
		}

		if(timerdbncaper)
			timerdbncaper--;
		if(timerdbncaperAP)
			timerdbncaperAP--;


		if(tapsttimer)
			tapsttimer--;

		if(fevptimer)
			fevptimer--;
		if(ftxontimer)
			ftxontimer--;

		if(timerInitCMX)
			timerInitCMX--;

        if(amr_timer)
            amr_timer--;

        timer_llaveon++;
        timer_contpoll++;
        timer_llaveoff++;
        timer_inhcpoll++;

        timer_prueba++;
        timpr_llaveon++;
        timpr_llaveoff++;
        timpr_gap++;

        //mido distancia a la puerta por ultrasonido con el HC-SR04, cada 60 mS
        if(timerRadar == 60)	{
            timerRadar = 0;
			//if( (!(SysFlag0 & STARTUP_flag)) && (SystemFlag3 & RADAR_ENABLE))   {
            if( !(SysFlag0 & STARTUP_flag) )   {
                RADAR_distance = ((RADAR_distance0 + RADAR_distance1 + RADAR_distance2 + RADAR_distance3) >> 2);
                RADAR_trigger();
            }
        }
        else timerRadar++;

		if(timer1seg == 1000)	{
			timer1seg = 0;

			//SEC_TIMER++;


            if(fsmnpp_timer)
                fsmnpp_timer--;
			if(NIC_timer)
				NIC_timer--;
			//actualizacion de timers cada 1seg
            ftxoff_timer++;
            ftxoff2_timer++;
            if(timer_rhb)
                timer_rhb--;
			if(timerdbncf220v)
				timerdbncf220v--;
			if(tout_AP_apertura)
				tout_AP_apertura--;
			if(tout_AP_reset)
				tout_AP_reset--;
			if(inconsole_timer)
				inconsole_timer--;
			if(rfw_timer)
				rfw_timer--;
			if(sndptimer)
				sndptimer--;
			if(timerIrqCMX)
				timerIrqCMX--;
			if(sbtimeout)
				sbtimeout--;
			if(signature_timer)
				signature_timer--;
			if(lowbatt_timer)
				lowbatt_timer--;
			if(npd_timer)
				npd_timer--;
			for( i = 0; i < MAXQTYPTM; i++ )	{
				if(ptm_dcb[i].timeout485)
					ptm_dcb[i].timeout485--;
			}
			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			//implemento la fsm de deteccion de apertura por ultrasonido
			//if( (!(SysFlag0 & STARTUP_flag)) && (SystemFlag3 & RADAR_ENABLE) )	{
			if( !(SysFlag0 & STARTUP_flag))	{
				switch(fsm_radar_state)	{
                    case ST_RADAR_IDLE:
                        if(RADAR_distance < RADAR_DISTANCE_MIN) {
                            fsm_radar_state = ST_RADAR_BADMEAS;
                        } else
                        if((RADAR_distance >= RADAR_DISTANCE_MIN) && (RADAR_distance <= RADAR_DISTANCE_MAX))    {
                            fsm_radar_state = ST_RADAR_PRECLOSED;
                        } else
                        if(RADAR_distance > RADAR_DISTANCE_MAX) {
                            fsm_radar_state = ST_RADAR_PREOPEN;
                        }
						if(SystemFlag3 & RADAR_ENABLE)
                        	RADAR_flags &= ~AP_RADAR_FLAG;
                        RADAR_count = 0;
                        break;
                    case ST_RADAR_PRECLOSED:
                        if((RADAR_distance >= RADAR_DISTANCE_MIN) && (RADAR_distance <= RADAR_DISTANCE_MAX))    {
                            RADAR_count++;
                        } else  {
                            fsm_radar_state = ST_RADAR_IDLE;
                            RADAR_count = 0;
                        }

                        if( RADAR_count > RADAR_COUNT)	{
                            fsm_radar_state = ST_RADAR_CLOSED;
							RADAR2_flags |= RADAR_DETECTED_FLAG;
							if(SystemFlag3 & RADAR_ENABLE)
                            	RADAR_flags &= ~AP_RADAR_FLAG;
                        }
                        break;
                    case ST_RADAR_PREOPEN:
                        if(RADAR_distance > RADAR_DISTANCE_MAX) {
                            RADAR_count++;
                        } else  {
                            fsm_radar_state = ST_RADAR_IDLE;
                            RADAR_count = 0;
                        }
                        if( RADAR_count > RADAR_COUNT)	{
                            fsm_radar_state = ST_RADAR_OPEN;
							if(SystemFlag3 & RADAR_ENABLE)
                            	RADAR_flags |= AP_RADAR_FLAG;
                        }
                        break;
                    case ST_RADAR_OPEN:
						if(SystemFlag3 & RADAR_ENABLE)
                        	RADAR_flags |= AP_RADAR_FLAG;
                        if(RADAR_distance < RADAR_DISTANCE_MIN) {
                            fsm_radar_state = ST_RADAR_BADMEAS;
                        } else
                        if((RADAR_distance >= RADAR_DISTANCE_MIN) && (RADAR_distance <= RADAR_DISTANCE_MAX))    {
                            fsm_radar_state = ST_RADAR_PRECLOSED;
                        }
                        break;
                    case ST_RADAR_CLOSED:
						if(SystemFlag3 & RADAR_ENABLE)
                        	RADAR_flags &= ~AP_RADAR_FLAG;
                        RADAR_count = 0;
						RADAR2_flags |= RADAR_DETECTED_FLAG;
                        if(RADAR_distance < RADAR_DISTANCE_MIN) {
                            fsm_radar_state = ST_RADAR_BADMEAS;
                        } else
                        if(RADAR_distance > RADAR_DISTANCE_MAX) {
                            fsm_radar_state = ST_RADAR_PREOPEN;
                        }
                        break;
                    default:
                        fsm_radar_state = ST_RADAR_IDLE;
                        break;
                    }
			}
			//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
		else timer1seg++;

		//acciones que se realizan cuando ocurre una encuesta de RFMICRF230
		if( SysFlag0 & RF_POLL_flag )	{
			RF_Poll();
			SysFlag0 &= ~RF_POLL_flag;
		}
		if( !(SysFlag0 & STARTUP_flag))	{
			fsm_transmit_cmx();
			if(SysFlag1 & PREVE_CENTRAL_RX)	{
				fsm_event_on_preve();
			} else	{
				fsm_evimd_llavetx();
			}
		}

        //PruebaLlaveWdog();
        fsm_llaveon_wdog();
		//vigilo el fraude por tension
//		if( !(SysFlag0 & STARTUP_flag))	{
//			fsm_AP_zvolt();
//		}
}



void RTC_IRQHandler(void)
{


	/* This is increment counter interrupt*/
	if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
	{
		SEC_TIMER++;

		if(startserialtimer)	{
			startserialtimer--;
		} else	{
			SysFlag4 |= SERIALSTART_flag;
		}

		if(CentralPollTimer)	{
			CentralPollTimer--;
		} else	{
			CentralPollTimer = 75;
			if(paptslot > 0)	{
				autoreset_data = 0xEF;
				SysFlag0 |= RF_POLL_flag;
				SysFlag0 |= FSMTX_flag;
			}
		}

        if(norm_asal_timer) {
            norm_asal_timer--;
        }

		// Clear pending interrupt
		RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	}


}


uint8_t epreve_state;

void fsm_event_preve(void)
{
	static time_t prevetimer;
	int error;

	switch(epreve_state)	{
	case PRV_NORMAL:
		if((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX))	{
			epreve_state = PRV_TACHO;
			AlarmWriteHistory();
			if(SysFlag_AP_Apertura & AP_APR_VALID)	{
				prevetimer = 0;
			} else	{
                if(SysFlag1 & PREVE_CENTRAL_TX) {
                    prevetimer = 0;
                } else {
                    prevetimer = (time_t) (EepromReadByte(PREVETIMER_E2P_ADDR, &error) * 60);
                    if ((prevetimer > (time_t) (15 * 60)) || (prevetimer < (time_t) (2 * 60))) {
                        prevetimer = (time_t) (7 * 60);
                    }
                }
			}
			prevetimer += SEC_TIMER;
		}
		break;
	case PRV_TACHO:
		if(!((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)))	{
			epreve_state = PRV_NORMAL;
			AlarmWriteHistory();
		} else
		if(prevetimer < SEC_TIMER)	{
			epreve_state = PRV_PREVE;
			AlarmWriteHistory();
			if(SysFlag1 & PREVE_CENTRAL_TX)
				logCidEvent(account, 1, 356, 0, 1);
			//else
			if(SysFlag1 & PREVE_CENTRAL_RX)
				logCidEvent(account, 1, 356, 0, 2);
		}
		break;
	case PRV_PREVE:
		if(!((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)))	{
			epreve_state = PRV_NORMAL;
			AlarmWriteHistory();
			if(SystemFlag3 & MEMAP_PREVE)	{
				SystemFlag3 &= ~MEMAP_PREVE;
				logCidEvent(account, 1, 942, 0, 0);
			} else	{
				logCidEvent(account, 3, 356, 0, 0);
			}
		}
		break;
	default:
		epreve_state = PRV_NORMAL;
		AlarmWriteHistory();
		break;
	}
}






void fsm_sboardconf( void )
{
	uint8_t inchar, i;
	uint8_t ibutton_id[8];
	static uint16_t id;
	static uint8_t rxpkt[10], rxcount;
	int retval;

	if( CommIsEmpty(DEBUG_COMM))
		inchar = 0x00;
	else
		inchar = ComGetChar(DEBUG_COMM);

	switch( sbfsm_state )	{
	case SBFSM_IDLE:
		if(inchar == '.')	{
			sbtimeout = 5;
			sbfsm_state = SBFSM_CONECTADO;
			ComPutChar(DEBUG_COMM, '>');
			led_swp[LLOP_led].led_cad = 3*0x100 + 3;
			led_swp[LLOP_led].led_state = LED_IDLE;
			led_swp[LLOP_led].led_blink = BLINK_FOREVER;
		} else
		if(!sbtimeout)	{
			SerialConsoleFlag &= ~INCONFBOARD_flag;
			ComPutChar(DEBUG_COMM, '}');
			led_swp[LLOP_led].led_state = LED_IDLE;
			led_swp[LLOP_led].led_cad = 0;


		}
		break;
	case SBFSM_CONECTADO:
		if(!sbtimeout)	{
			SerialConsoleFlag &= ~INCONFBOARD_flag;
			ComPutChar(DEBUG_COMM, '}');
			led_swp[LLOP_led].led_state = LED_IDLE;
			led_swp[LLOP_led].led_cad = 0;
			sbfsm_state = SBFSM_IDLE;
		} else
		if(inchar == '!')	{
			sbtimeout = 5;
			sbfsm_state = SBFSM_INPKT;
			for(i=0; i<10; i++)
				rxpkt[i] = 0x00;
			rxcount = 0;
			rxpkt[rxcount++] = inchar;
		} else
		if( inchar == '.')	{
			sbtimeout = 5;
		}
		break;
	case SBFSM_INPKT:
		if(!sbtimeout)	{
			SerialConsoleFlag &= ~INCONFBOARD_flag;
			ComPutChar(DEBUG_COMM, '}');
			led_swp[LLOP_led].led_state = LED_IDLE;
			led_swp[LLOP_led].led_cad = 0;
			sbfsm_state = SBFSM_IDLE;
		} else
		if( (inchar != 0x00) && (rxcount < 9) )	{
			rxpkt[rxcount++] = inchar;
			sbtimeout = 5;
		} else
		if(rxcount >= 9)	{
			sbtimeout = 5;
			ComPutChar(DEBUG_COMM, '<');
			sbfsm_state = SBFSM_WIBUTTON;
			id = 0;
			//----------------
			byteToledBlink( htoi_nibble(rxpkt[5])*0x10 + htoi_nibble(rxpkt[6]));
		}
		break;
	case SBFSM_WIBUTTON:
		if(iButton_read(ibutton_id))	{
			if( (id = check_id_2(ibutton_id)) )	{
				for( i = 0; i < 8; i++ )
					ibutton_id[i] = 0;
				sbtimeout = 5;
				sbfsm_state = SBFSM_WIBUFREE;
			}
		} else
		if(!sbtimeout)	{
			sbtimeout = 5;
			sbfsm_state = SBFSM_CONECTADO;
			ComPutChar(DEBUG_COMM, '>');
		}
		break;
	case SBFSM_WIBUFREE:
		if(!iButton_read(ibutton_id))	{
			sbtimeout = 5;
			sbfsm_state = SBFSM_CONECTADO;
			ComPutChar(DEBUG_COMM, '>');

			retval = sbpgm(rxpkt, id);
			if(retval == 1)	{
//				led_swp[NORMAL_led].led_blink = 3;
//				led_swp[NORMAL_led].led_cad = 1*0x100+1;
//				led_swp[NORMAL_led].led_state = LED_IDLE;
				Buzzer_dcb.led_cad = 2*0x100 + 2;
				Buzzer_dcb.led_state = LED_IDLE;
				Buzzer_dcb.led_blink = 5;
			}

		} else
		if(!sbtimeout)	{
			SerialConsoleFlag &= ~INCONFBOARD_flag;
			ComPutChar(DEBUG_COMM, '}');
			led_swp[LLOP_led].led_state = LED_IDLE;
			led_swp[LLOP_led].led_cad = 0;
			sbfsm_state = SBFSM_IDLE;
		}
		break;
	default:
		sbfsm_state = SBFSM_IDLE;
		break;
	}
}

int sbpgm( unsigned char rxpkt[], uint16_t id)
{
	uint8_t command, data, temp;
	int error, i;
	OS_ERR os_err;
	NET_IP_ADDR myipaddress, ipaddrtemp, mygwayaddress;
	char buffer[17], altbuff[17];
	NET_ERR      err;

	command = htoi_nibble(rxpkt[3]);
	data = htoi_nibble(rxpkt[5])*0x10 + htoi_nibble(rxpkt[6]);

	switch(command)	{
	//-----------------------------------------------------------------------
	//programacion del numero de abonado en forma traversada
	case 1:
		EepromWriteByte(RF_NUMABO_E2P_ADDR, data, &error);
		logCidEvent(account, 1, SPV_PGM_NUMABO, 0, id);

#ifdef TABLAS_ABO_ON
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		temp = (uint8_t)EepromReadByte(ZONE_E2P_ADDR, &error);
		switch(temp)	{
		case 1:
			BaseAlarmPkt_numabo = numabo_z1[data-1];
			break;
		case 2:
			BaseAlarmPkt_numabo = numabo_z2[data-1];
			break;
		case 3:
			BaseAlarmPkt_numabo = numabo_z3[data-1];
			break;
		case 4:
			BaseAlarmPkt_numabo = numabo_z4[data-1];
			break;
		case 5:
			BaseAlarmPkt_numabo = numabo_z5[data-1];
			break;
		case 6:
			BaseAlarmPkt_numabo = numabo_z6[data-1];
			break;
		}


		if(BaseAlarmPkt_numabo >= 80)	{
			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
			EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)4, &error);
			TypeAboAns = 4;
		} else	{
			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
			EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)3, &error);
			TypeAboAns = 3;
		}
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		EepromWriteByte(FMODEON_ADDR, (uint8_t)0x01, &error);
		SysFlag3 |= FMODE_flag;

		Buzzer_dcb.led_cad = 50*0x100 + 5;
		Buzzer_dcb.led_state = LED_IDLE;
		Buzzer_dcb.led_blink = 1;
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		temp = (uint8_t)EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
		ByteToledBlink(temp);

		while(Buzzer_dcb.led_blink);

#else
		BaseAlarmPkt_numabo = numabo;
#endif

		return 1;
		break;
	//-----------------------------------------------------------------------
	//programacion del RFANSW y el FMODE
	case 2:
		temp = (data & 0x07);
		EepromWriteByte(RF_ANSWER_TYPE, temp, &error);
		TypeAboAns = temp;
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
		temp = (data >>4) & 0x01;
		EepromWriteByte(FMODEON_ADDR, temp, &error);
		if(temp)
			SysFlag3 |= FMODE_flag;
		else
			SysFlag3 &= ~FMODE_flag;
		logCidEvent(account, 1, SPV_PGM_MODE, 0, id);
		return 1;
		break;
	case 3:
	//-----------------------------------------------------------------------
	//	Programacion del estado de los PT Eyse
		temp = (data & 0xF8);
		EepromWriteByte(PTXORMASK_E2P_ADDR, temp, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		ptxormask = temp;
		return 1;
		break;
	//-----------------------------------------------------------------------
	//	Programacion de la zona
	case 4:
		if(id < IDZONE1)	{
			//EepromWriteByte(ZONE_E2P_ADDR, data, &error);
			logCidEvent(account, 1, SPV_PGM_ZONE, 0, id);

#ifdef TABLAS_ABO_ON
		temp = (uint8_t)EepromReadByte(RF_NUMABO_E2P_ADDR, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		switch(data)	{
		case 0x01:
			EepromWriteByte(ZONE_E2P_ADDR, 1, &error);
			BaseAlarmPkt_numabo = numabo_z1[temp-1];
			break;
		case 0x02:
			EepromWriteByte(ZONE_E2P_ADDR, 2, &error);
			BaseAlarmPkt_numabo = numabo_z2[temp-1];
			break;
		case 0x04:
			EepromWriteByte(ZONE_E2P_ADDR, 3, &error);
			BaseAlarmPkt_numabo = numabo_z3[temp-1];
			break;
		case 0x08:
			EepromWriteByte(ZONE_E2P_ADDR, 4, &error);
			BaseAlarmPkt_numabo = numabo_z4[temp-1];
			break;
		case 0x10:
			EepromWriteByte(ZONE_E2P_ADDR, 5, &error);
			BaseAlarmPkt_numabo = numabo_z5[temp-1];
			break;
		case 0x20:
			EepromWriteByte(ZONE_E2P_ADDR, 6, &error);
			BaseAlarmPkt_numabo = numabo_z6[temp-1];
			break;
		}

		if(BaseAlarmPkt_numabo >= 80)	{
			EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)4, &error);
			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
			TypeAboAns = 4;
		} else	{
			EepromWriteByte(RF_ANSWER_TYPE, (uint8_t)3, &error);
			OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
			TypeAboAns = 3;
		}

		EepromWriteByte(FMODEON_ADDR, (uint8_t)0x01, &error);
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
		SysFlag3 |= FMODE_flag;
#endif
		return 1;
		} else
			return -1;
		break;
	// Programacion de IP1
	case 5:
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		ipaddrtemp <<= 24;
		myipaddress &= 0x00FFFFFF;
		myipaddress |= (ipaddrtemp & 0xFF000000);
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de IP2
	case 6:
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		ipaddrtemp <<= 16;
		myipaddress &= 0xFF00FFFF;
		myipaddress |= (ipaddrtemp & 0x00FF0000);
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de IP3
	case 7:
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		ipaddrtemp <<= 8;
		myipaddress &= 0xFFFF00FF;
		myipaddress |= (ipaddrtemp & 0x0000FF00);
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de IP4
	case 8:
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		//ipaddrtemp <<= 16;
		myipaddress &= 0xFFFFFF00;
		myipaddress |= (ipaddrtemp & 0x000000FF);
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de GW4
	case 9:
		EepromReadBuffer(GATEWAY_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)altbuff, 16, &error);
		myipaddress = NetASCII_Str_to_IP(altbuff,    &err);
		mygwayaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		mygwayaddress &= 0x0000FFFF;
		mygwayaddress |= (0xFFFF0000 & myipaddress);


		mygwayaddress &= 0xFFFFFF00;
		mygwayaddress |= (ipaddrtemp & 0x000000FF);
		NetASCII_IP_to_Str (mygwayaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(GATEWAY_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de GW3
	case 10:
		EepromReadBuffer(GATEWAY_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		EepromReadBuffer(LOCAL_IP_E2P_ADDR, (uint8_t *)altbuff, 16, &error);
		myipaddress = NetASCII_Str_to_IP(altbuff,    &err);
		mygwayaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		mygwayaddress &= 0x0000FFFF;
		mygwayaddress |= (0xFFFF0000 & myipaddress);

		ipaddrtemp <<= 8;
		mygwayaddress &= 0xFFFF00FF;
		mygwayaddress |= (ipaddrtemp & 0x0000FF00);
		NetASCII_IP_to_Str (mygwayaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(GATEWAY_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de MK4
	case 11:
		EepromReadBuffer(NETMASK_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		//ipaddrtemp <<= 16;
		myipaddress &= 0xFFFFFF00;
		myipaddress |= (ipaddrtemp & 0x000000FF);
		myipaddress |= 0xFFFF0000;
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(NETMASK_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	// Programacion de MK3
	case 12:
		EepromReadBuffer(NETMASK_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		myipaddress = NetASCII_Str_to_IP(buffer,    &err);
		ipaddrtemp = (NET_IP_ADDR)data;
		ipaddrtemp <<= 8;
		myipaddress &= 0xFFFF00FF;
		myipaddress |= (ipaddrtemp & 0x0000FF00);
		myipaddress |= 0xFFFF0000;
		NetASCII_IP_to_Str (myipaddress, buffer, DEF_YES, &err);
		buffer[15] = 0x00;
		EepromWriteBuffer(NETMASK_E2P_ADDR, (uint8_t *)buffer, 16, &error);
		return 1;
		break;
	case 13:
		if(id < IDZONE1)	{
			switch(data)	{
			case 1:
				for(i = 0; i < 256; i++)	{
					WDT_Feed();
					//EepromWriteByte(E2P_NABOTBL_OFFSET + i, i, &error);
					OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
				}
				return 1;
				break;
			case 2:
				FormatEventFlash();
				logCidEvent(account, 1, SPV_PGM_EFORMAT, 0, id);
				return 1;
				break;
			}
		} else return -1;
		break;
	case 14:	//programacion del numero de abonado en forma directa
		EepromWriteByte(RF_NUMABO_E2P_ADDR, data, &error);
		BaseAlarmPkt_numabo = data;
		logCidEvent(account, 1, SPV_PGM_NUMABO, 0, id);
		return 1;
		break;
	}
	return -1;
}



void fsm_R3kSendCid(void)
{
	EventRecord thisEvent;
	uint8_t	sendbuffer[40], i;
	uint16_t buf_len, lr3kaccount;
	uint16_t crc;
	OS_ERR os_err;

	switch(fsmr3kcid_state)	{
	case FR3KCID_IDLE:
		if( (SerialConsoleFlag & INSERCON_flag) || (SerialConsoleFlag & INCONFBOARD_flag) )	{
			fsmr3kcid_state = FR3KCID_WAITCONSOLE;
		} else
		if(R3KeventRec_count)	{
			R3KpreReadEvent(&thisEvent);
			if((r3kmode == 2) || (r3kmode == 3)	)	{
				buf_len = ProtEncoder_R3KCID(&thisEvent, sendbuffer);
			} else
			if(r3kmode == 4)	{
				buf_len = ProtEncoder_R3KCID_T(&thisEvent, sendbuffer);
			}
			for(i = 0; i < buf_len; i++)	{
				ComPutChar(DEBUG_COMM, sendbuffer[i]);
				delay_us(100);
			}
			fsmr3kcid_state = FR3KCID_WAITACK;
			r3kSendCidTimer = SEC_TIMER;
			//r3khbtimer = SEC_TIMER;
			//r3khb06timer = SEC_TIMER;
		} else
		if(SEC_TIMER >= (r3khb06timer + 11))	{
			ComPutChar(DEBUG_COMM, 0x06);
			delay_us(100);
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
			fsmr3kcid_state = FR3KCID_WAITACK06;
			r3kSendCidTimer = SEC_TIMER;
		} else
		if(SEC_TIMER >= (r3khbtimer + 60))	{
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
			fsmr3kcid_state = FR3KCID_WAITACK06;
			r3kSendCidTimer = SEC_TIMER;
			r3khbtimer = SEC_TIMER;
			crc = 0;
//---------------------------------------------
			CommSendString(DEBUG_COMM, "@@HB");
			delay_us(100);
			//mando el numero de cuenta1
			lr3kaccount = AccountToDigits(r3kaccount);
			CommSendString(DEBUG_COMM, "[");
			ComPutChar(DEBUG_COMM, ((lr3kaccount >> 12) & 0x0F) +'0');
			ComPutChar(DEBUG_COMM, ((lr3kaccount >> 8) & 0x0F) +'0');
			ComPutChar(DEBUG_COMM, ((lr3kaccount >> 4) & 0x0F) +'0');
			ComPutChar(DEBUG_COMM, (lr3kaccount & 0x0F) +'0');
			CommSendString(DEBUG_COMM, "]");

			CommSendString(DEBUG_COMM, "[");
			ComPutChar(DEBUG_COMM, BaseAlarmPkt_numabo);											//0
			crc += BaseAlarmPkt_numabo;
			CommSendString(DEBUG_COMM, "]");
			delay_us(100);
//			if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
//				CommSendString(DEBUG_COMM, "[");
//				ComPutChar(DEBUG_COMM, 0x80);													//1
//				CommSendString(DEBUG_COMM, "]");
//				delay_us(100);
//			} else	{
				CommSendString(DEBUG_COMM, "[");
				ComPutChar(DEBUG_COMM, BaseAlarmPkt_alarm);										//1
				crc += BaseAlarmPkt_alarm;
				CommSendString(DEBUG_COMM, "]");
				delay_us(100);
//			}
			CommSendString(DEBUG_COMM, "[");
			ComPutChar(DEBUG_COMM, BaseAlarmPkt_estado_dispositivos);							//2
			crc += BaseAlarmPkt_estado_dispositivos;
			CommSendString(DEBUG_COMM, "]");
			delay_us(100);
			CommSendString(DEBUG_COMM, "[");
			ComPutChar(DEBUG_COMM, BaseAlarmPkt_memoria_dispositivos);							//3
			crc += BaseAlarmPkt_memoria_dispositivos;
			CommSendString(DEBUG_COMM, "]");
			delay_us(100);
//			if( ((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (BaseAlarmPkt_alarm == 0x80) )	{
//				CommSendString(DEBUG_COMM, "[");
//				if(SysFlag1 & PREVE_CENTRAL_TX)	{
//					ComPutChar(DEBUG_COMM, 0x01);
//					crc += 0x01;
//				}
//				else if(SysFlag1 & PREVE_CENTRAL_RX)	{
//					ComPutChar(DEBUG_COMM, 0x02);												//4
//					crc += 0x02;
//				}
//				CommSendString(DEBUG_COMM, "]");
//				delay_us(100);
//
//			} else	{
//				CommSendString(DEBUG_COMM, "[");
//				ComPutChar(DEBUG_COMM,  0x00);													//4
//				crc += 0x00;
//				CommSendString(DEBUG_COMM, "]");
//				delay_us(100);
//			}
			CommSendString(DEBUG_COMM, "[");
			ComPutChar(DEBUG_COMM, (unsigned char)(crc & 0x00FF));
			CommSendString(DEBUG_COMM, "]");
			delay_us(100);
//---------------------------------------------

		}
		break;
	case FR3KCID_WAITACK:
		if( (SerialConsoleFlag & INSERCON_flag) || (SerialConsoleFlag & INCONFBOARD_flag) )	{
			fsmr3kcid_state = FR3KCID_WAITCONSOLE;
		} else
		if(SystemFlag & R3KSERACK)	{
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
#ifdef USAR_IRIDIUM
			IRIDIUM_flag |= IRI_GPRSKICK2_FLAG;
#endif
			R3KReadOutEvent(&thisEvent);
			OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
			fsmr3kcid_state = FR3KCID_IDLE;
			r3khbtimer = SEC_TIMER;
			r3khb06timer = SEC_TIMER;
		} else
		if(SEC_TIMER >= (r3kSendCidTimer + 10))	{
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
			fsmr3kcid_state = FR3KCID_IDLE;
			r3khbtimer = SEC_TIMER;
			r3khb06timer = SEC_TIMER;
		}
		break;
	case FR3KCID_WAITACK06:
		if( (SerialConsoleFlag & INSERCON_flag) || (SerialConsoleFlag & INCONFBOARD_flag) )	{
			fsmr3kcid_state = FR3KCID_WAITCONSOLE;
		} else
		if(SystemFlag & R3KSERACK)	{
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
#ifdef USAR_IRIDIUM
			IRIDIUM_flag |= IRI_GPRSKICK2_FLAG;
#endif
			fsmr3kcid_state = FR3KCID_IDLE;
			//r3khbtimer = SEC_TIMER;
			r3khb06timer = SEC_TIMER;
		} else
		if(SEC_TIMER >= (r3kSendCidTimer + 10))	{
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
			fsmr3kcid_state = FR3KCID_IDLE;
			//r3khbtimer = SEC_TIMER;
			r3khb06timer = SEC_TIMER;
		}
		break;
	case FR3KCID_WAITCONSOLE:
		if( !((SerialConsoleFlag & INSERCON_flag) || (SerialConsoleFlag & INCONFBOARD_flag)) )	{
			fsmr3kcid_state = FR3KCID_IDLE;
			SystemFlag &= ~R3KSERACK;
			SystemFlag &= ~R3KSERSPACE;
			r3khbtimer = SEC_TIMER;
			r3khb06timer = SEC_TIMER;
		}
		break;
	default:
		fsmr3kcid_state = FR3KCID_IDLE;
		break;
	}
}

void byteToledBlink( uint8_t data)
{
	if( data & 0x80)	{
		led_swp[ASAL_led].led_blink = BLINK_FOREVER;
		led_swp[ASAL_led].led_cad = 1*0x100+1;
		led_swp[ASAL_led].led_state = LED_IDLE;
	} else	{
		led_swp[ASAL_led].led_state = LED_IDLE;
		led_swp[ASAL_led].led_cad = 0;
	}

	if( data & 0x40)	{
		led_swp[INCE_led].led_blink = BLINK_FOREVER;
		led_swp[INCE_led].led_cad = 1*0x100+1;
		led_swp[INCE_led].led_state = LED_IDLE;
	} else	{
		led_swp[INCE_led].led_state = LED_IDLE;
		led_swp[INCE_led].led_cad = 0;
	}

	if( data & 0x20)	{
		led_swp[TESO_led].led_blink = BLINK_FOREVER;
		led_swp[TESO_led].led_cad = 1*0x100+1;
		led_swp[TESO_led].led_state = LED_IDLE;
	} else	{
		led_swp[TESO_led].led_state = LED_IDLE;
		led_swp[TESO_led].led_cad = 0;
	}

	if( data & 0x10)	{
		led_swp[ROTU_led].led_blink = BLINK_FOREVER;
		led_swp[ROTU_led].led_cad = 1*0x100+1;
		led_swp[ROTU_led].led_state = LED_IDLE;
	} else	{
		led_swp[ROTU_led].led_state = LED_IDLE;
		led_swp[ROTU_led].led_cad = 0;
	}

	if( data & 0x08)	{
		led_swp[APER_led].led_blink = BLINK_FOREVER;
		led_swp[APER_led].led_cad = 1*0x100+1;
		led_swp[APER_led].led_state = LED_IDLE;
	} else	{
		led_swp[APER_led].led_state = LED_IDLE;
		led_swp[APER_led].led_cad = 0;
	}

	if( data & 0x04)	{
		led_swp[LLOP_led].led_blink = BLINK_FOREVER;
		led_swp[LLOP_led].led_cad = 1*0x100+1;
		led_swp[LLOP_led].led_state = LED_IDLE;
	} else	{
		led_swp[LLOP_led].led_state = LED_IDLE;
		led_swp[LLOP_led].led_cad = 0;
	}

	if( data & 0x02)	{
		led_swp[F220_led].led_blink = BLINK_FOREVER;
		led_swp[F220_led].led_cad = 1*0x100+1;
		led_swp[F220_led].led_state = LED_IDLE;
	} else	{
		led_swp[F220_led].led_state = LED_IDLE;
		led_swp[F220_led].led_cad = 0;
	}

	if( data & 0x01)	{
		led_swp[NORMAL_led].led_blink = BLINK_FOREVER;
		led_swp[NORMAL_led].led_cad = 1*0x100+1;
		led_swp[NORMAL_led].led_state = LED_IDLE;
	} else	{
		led_swp[NORMAL_led].led_state = LED_IDLE;
		led_swp[NORMAL_led].led_cad = 0;
	}
}

void ByteToledBlink( uint8_t data)
{
	if( data & 0x80)	{
		led_swp[ASAL_led].led_blink = 1;
		led_swp[ASAL_led].led_cad = 50*0x100+1;
		led_swp[ASAL_led].led_state = LED_IDLE;
	} else	{
		led_swp[ASAL_led].led_state = LED_IDLE;
		led_swp[ASAL_led].led_cad = 0;
	}

	if( data & 0x40)	{
		led_swp[INCE_led].led_blink = 1;
		led_swp[INCE_led].led_cad = 50*0x100+1;
		led_swp[INCE_led].led_state = LED_IDLE;
	} else	{
		led_swp[INCE_led].led_state = LED_IDLE;
		led_swp[INCE_led].led_cad = 0;
	}

	if( data & 0x20)	{
		led_swp[TESO_led].led_blink = 1;
		led_swp[TESO_led].led_cad = 50*0x100+1;
		led_swp[TESO_led].led_state = LED_IDLE;
	} else	{
		led_swp[TESO_led].led_state = LED_IDLE;
		led_swp[TESO_led].led_cad = 0;
	}

	if( data & 0x10)	{
		led_swp[ROTU_led].led_blink = 1;
		led_swp[ROTU_led].led_cad = 50*0x100+1;
		led_swp[ROTU_led].led_state = LED_IDLE;
	} else	{
		led_swp[ROTU_led].led_state = LED_IDLE;
		led_swp[ROTU_led].led_cad = 0;
	}

	if( data & 0x08)	{
		led_swp[APER_led].led_blink = 1;
		led_swp[APER_led].led_cad = 50*0x100+1;
		led_swp[APER_led].led_state = LED_IDLE;
	} else	{
		led_swp[APER_led].led_state = LED_IDLE;
		led_swp[APER_led].led_cad = 0;
	}

	if( data & 0x04)	{
		led_swp[LLOP_led].led_blink = 1;
		led_swp[LLOP_led].led_cad = 50*0x100+1;
		led_swp[LLOP_led].led_state = LED_IDLE;
	} else	{
		led_swp[LLOP_led].led_state = LED_IDLE;
		led_swp[LLOP_led].led_cad = 0;
	}

	if( data & 0x02)	{
		led_swp[F220_led].led_blink = 1;
		led_swp[F220_led].led_cad = 50*0x100+1;
		led_swp[F220_led].led_state = LED_IDLE;
	} else	{
		led_swp[F220_led].led_state = LED_IDLE;
		led_swp[F220_led].led_cad = 0;
	}

	if( data & 0x01)	{
		led_swp[NORMAL_led].led_blink = 1;
		led_swp[NORMAL_led].led_cad = 50*0x100+1;
		led_swp[NORMAL_led].led_state = LED_IDLE;
	} else	{
		led_swp[NORMAL_led].led_state = LED_IDLE;
		led_swp[NORMAL_led].led_cad = 0;
	}
}



void fsm_pgm1( void )
{
	static time_t pgm1timer;

	switch(fsmpgm1_state)	{
	case FSMPGM1_OFF :
		if(SystemFlag & PGM1_ON_FLAG)	{
			SystemFlag &= ~PGM1_ON_FLAG;
			pgm1timer = SEC_TIMER + 20;
			fsmpgm1_state = FSMPGM1_ON;
			GPIO_SetValue(0, 1<<27);
			logCidEvent(account, 1, 930, 0, 0);		//genero evento cid de activacion pgm1
		}
		break;
	case FSMPGM1_ON :
		if(SEC_TIMER >= pgm1timer)	{
			SystemFlag &= ~PGM1_ON_FLAG;
			fsmpgm1_state = FSMPGM1_OFF;
			GPIO_ClearValue(0, 1<<27);
			logCidEvent(account, 3, 930, 0, 0);		//genero evento cid de desactivacion pgm1
		} else
		if(SystemFlag & PGM1_OFF_FLAG)	{
			SystemFlag &= ~PGM1_OFF_FLAG;
			fsmpgm1_state = FSMPGM1_OFF;
			GPIO_ClearValue(0, 1<<27);
		}
		break;
	default:
		fsmpgm1_state = FSMPGM1_OFF;
		GPIO_ClearValue(0, 1<<27);
		break;
	}

}

void fsm_sir1( void )
{
	static time_t sir1timer;
	uint32_t len;

	switch(fsmsir1_state)	{
	case FSMSIR1_OFF :
		if(SIRENA_Flag & SIR1_ON_FLAG)	{
			SIRENA_Flag &= ~SIR1_ON_FLAG;
			sir1timer = SEC_TIMER + 30;
			fsmsir1_state = FSMSIR1_ON;
			relestate[0] |= bitpat[2];
			logCidEvent(account, 1, 972, 0, 0);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSIR1_ON :
		if(SEC_TIMER >= sir1timer)	{
			SIRENA_Flag &= ~SIR1_ON_FLAG;
			fsmsir1_state = FSMSIR1_OFF;
			relestate[0] &= ~bitpat[2];
			logCidEvent(account, 3, 972, 0, 0);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		} else
		if(SIRENA_Flag & SIR1_OFF_FLAG)	{
			SIRENA_Flag &= ~SIR1_OFF_FLAG;
			fsmsir1_state = FSMSIR1_OFF;
			relestate[0] &= ~bitpat[2];
			logCidEvent(account, 3, 972, 0, 1);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	default:
		fsmsir1_state = FSMSIR1_OFF;
		relestate[0] &= ~bitpat[2];
		len = flash0_write(1, relestate, DF_RELESTATES, 10);
		break;
	}

}

void fsm_sir2( void )
{
	static time_t sir2timer;
	uint32_t len;

	switch(fsmsir2_state)	{
	case FSMSIR2_OFF :
		if(SIRENA_Flag & SIR2_ON_FLAG)	{
			SIRENA_Flag &= ~SIR2_ON_FLAG;
			sir2timer = SEC_TIMER + 30;
			fsmsir2_state = FSMSIR2_ON;
			relestate[0] |= bitpat[3];
			logCidEvent(account, 1, 972, 0, 2);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSIR2_ON :
		if(SEC_TIMER >= sir2timer)	{
			SIRENA_Flag &= ~SIR2_ON_FLAG;
			fsmsir2_state = FSMSIR2_OFF;
			relestate[0] &= ~bitpat[3];
			logCidEvent(account, 3, 972, 0, 2);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		} else
		if(SIRENA_Flag & SIR2_OFF_FLAG)	{
			SIRENA_Flag &= ~SIR2_OFF_FLAG;
			fsmsir2_state = FSMSIR2_OFF;
			relestate[0] &= ~bitpat[3];
			logCidEvent(account, 3, 972, 0, 3);
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	default:
		fsmsir2_state = FSMSIR2_OFF;
		relestate[0] &= ~bitpat[3];
		len = flash0_write(1, relestate, DF_RELESTATES, 10);
		break;
	}

}

void fsm_strike1( void)
{
	static time_t strike1timer;
	uint32_t len;

	switch(fsmstk1_state)	{
	case FSMSTK_OFF:
		if(STRIKE_Flag & STRIKE1_ON_FLAG)	{
			STRIKE_Flag &= ~STRIKE1_ON_FLAG;
			if(!(SIRENA_Flag & DOOR1_OC_FLAG))	{
				logCidEvent(account, 1, 973, 0, 1);
			}
			relestate[0] |= bitpat[0];
			fsmstk1_state = FSMSTK_WAIT1;
			strike1timer = SEC_TIMER + 10;
			SIRENA_Flag |= STRIKE1_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		if(STRIKE_Flag & STRIKE1_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE1_OFF_FLAG;
			relestate[0] &= ~bitpat[4];
			fsmstk1_state = FSMSTK_WAIT2;
			strike1timer = SEC_TIMER + 2;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_WAIT1:
		if(STRIKE_Flag & STRIKE1_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE1_OFF_FLAG;
			relestate[0] &= ~bitpat[0];
			fsmstk1_state = FSMSTK_OFF;
			SIRENA_Flag &= ~STRIKE1_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		} else
		if(SEC_TIMER >= strike1timer)	{
			relestate[0] |= bitpat[4];
			fsmstk1_state = FSMSTK_ON;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_ON:
		if(STRIKE_Flag & STRIKE1_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE1_OFF_FLAG;
			relestate[0] &= ~bitpat[4];
			fsmstk1_state = FSMSTK_WAIT2;
			strike1timer = SEC_TIMER + 2;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_WAIT2:
		if(SEC_TIMER >= strike1timer)	{
			relestate[0] &= ~bitpat[0];
			fsmstk1_state = FSMSTK_OFF;
			SIRENA_Flag &= ~STRIKE1_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	default:
		fsmstk1_state = FSMSTK_OFF;
		break;
	}
}

void fsm_strike2( void)
{
	static time_t strike2timer;
	uint32_t len;

	switch(fsmstk2_state)	{
	case FSMSTK_OFF:
		if(STRIKE_Flag & STRIKE2_ON_FLAG)	{
			STRIKE_Flag &= ~STRIKE2_ON_FLAG;
			if(!(SIRENA_Flag & DOOR2_OC_FLAG))	{
				logCidEvent(account, 1, 973, 0, 2);
			}
			relestate[0] |= bitpat[1];
			fsmstk2_state = FSMSTK_WAIT1;
			strike2timer = SEC_TIMER + 10;
			SIRENA_Flag |= STRIKE2_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		if(STRIKE_Flag & STRIKE2_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE2_OFF_FLAG;
			relestate[0] &= ~bitpat[5];
			fsmstk2_state = FSMSTK_WAIT2;
			strike2timer = SEC_TIMER + 2;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_WAIT1:
		if(STRIKE_Flag & STRIKE2_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE2_OFF_FLAG;
			relestate[0] &= ~bitpat[1];
			fsmstk2_state = FSMSTK_OFF;
			SIRENA_Flag &= ~STRIKE2_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		} else
		if(SEC_TIMER >= strike2timer)	{
			relestate[0] |= bitpat[5];
			fsmstk2_state = FSMSTK_ON;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_ON:
		if(STRIKE_Flag & STRIKE2_OFF_FLAG)	{
			STRIKE_Flag &= ~STRIKE2_OFF_FLAG;
			relestate[0] &= ~bitpat[5];
			fsmstk2_state = FSMSTK_WAIT2;
			strike2timer = SEC_TIMER + 2;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	case FSMSTK_WAIT2:
		if(SEC_TIMER >= strike2timer)	{
			relestate[0] &= ~bitpat[1];
			fsmstk2_state = FSMSTK_OFF;
			SIRENA_Flag &= ~STRIKE2_FLAG;
			len = flash0_write(1, relestate, DF_RELESTATES, 10);
		}
		break;
	default:
		fsmstk2_state = FSMSTK_OFF;
		break;
	}
}


void fsm_TAS( void )
{
	static time_t tastimer;
	OS_ERR	os_err;

	switch(fsmtas_state)	{
	case FSMTAS_STARTUP:
		OSTimeDlyHMSM(0, 0, 20, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
		if(!(TasFlags & TAS220ON_FLAG))	{
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEON;
			logCidEvent(account, 1, 934, 0, 0);		//aviso que no hay 220 en el TAS
		} else	{
			fsmtas_state = FSMTAS_IDLE;
		}
		break;
	case FSMTAS_IDLE:
		if( TasFlags & TASVANDAL_FLAG)	{
			TasFlags &= ~TASVANDAL_FLAG;
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEEND;
			logCidEvent(account, 1, 933, 0, 0);		//aviso vandalismo de tas al monitoreo
		} else
		if(!(TasFlags & TAS220ON_FLAG))	{
			fsmtas_state = FSMTAS_220OFF;
			logCidEvent(account, 1, 934, 0, 0);		//aviso que no hay 220 en el TAS
		} else
		if(TasFlags & TASCMDOFF_FLAG)	{
			TasFlags &= ~TASCMDOFF_FLAG;
			TasFlags &= ~TASCMDON_FLAG;
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEEND;
		} else
		if(TasFlags & TASCMDON_FLAG)	{
			//TasFlags &= ~TASCMDOFF_FLAG;
			TasFlags &= ~TASCMDON_FLAG;
		}
		break;
	case FSMTAS_PULSEEND:
		if(MSEC_TIMER >= tastimer )	{
			GPIO_ClearValue(0, 1<<27);
			fsmtas_state = FSMTAS_WAIT220OFF;
			tastimer = SEC_TIMER + 10;
		}
		break;
	case FSMTAS_WAIT220OFF:
		if(SEC_TIMER >= tastimer )	{
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEEND;
		} else
		if( !(TasFlags & TAS220ON_FLAG))	{
			fsmtas_state = FSMTAS_220OFF;
			logCidEvent(account, 1, 934, 0, 0);		//aviso que no hay 220 en el TAS
		} else
		if(TasFlags & TASCMDON_FLAG)	{
			TasFlags &= ~TASCMDON_FLAG;
			tastimer = SEC_TIMER + 10;
			fsmtas_state = FSMTAS_WAIT220ON;
		} else
		if(TasFlags & TASCMDOFF_FLAG)	{
			TasFlags &= ~TASCMDOFF_FLAG;
		}
		break;
	case FSMTAS_220OFF:
		if(TasFlags & TASCMDON_FLAG)	{
			TasFlags &= ~TASCMDON_FLAG;
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEON;
		} else
		if(TasFlags & TAS220ON_FLAG)	{
			fsmtas_state = FSMTAS_IDLE;
			logCidEvent(account, 3, 934, 0, 0);		//aviso que hay 220 en el TAS
		} else
		if( TasFlags & TASVANDAL_FLAG)	{
			TasFlags &= ~TASVANDAL_FLAG;
			logCidEvent(account, 1, 933, 0, 0);		//aviso vandalismo de tas al monitoreo
		} else
		if(TasFlags & TASCMDOFF_FLAG)	{
			TasFlags &= ~TASCMDOFF_FLAG;
			//TasFlags &= ~TASCMDON_FLAG;
		}
		break;
	case FSMTAS_PULSEON:
		if(MSEC_TIMER >= tastimer )	{
			GPIO_ClearValue(0, 1<<27);
			fsmtas_state = FSMTAS_WAIT220ON;
			tastimer = SEC_TIMER + 10;
		}
		break;
	case FSMTAS_WAIT220ON:
		if(SEC_TIMER >= tastimer )	{
			GPIO_SetValue(0, 1<<27);
			tastimer = MSEC_TIMER + 1500;
			fsmtas_state = FSMTAS_PULSEON;
		} else
		if( TasFlags & TAS220ON_FLAG)	{
			fsmtas_state = FSMTAS_IDLE;
			logCidEvent(account, 3, 934, 0, 0);		//aviso que hay 220 en el TAS
		} else
		if(TasFlags & TASCMDOFF_FLAG)	{
			TasFlags &= ~TASCMDOFF_FLAG;
			TasFlags &= ~TASCMDON_FLAG;
			fsmtas_state = FSMTAS_WAIT220OFF;
			tastimer = SEC_TIMER + 10;
		} else
		if(TasFlags & TASCMDON_FLAG)	{
			TasFlags &= ~TASCMDON_FLAG;
		}
		break;
	default:
		fsmtas_state = FSMTAS_STARTUP;
	}
}

time_t wdip150_timer;
uint8_t  fsmwdip150_state;
//#define	FSMWDIP150_ENTRY	0x05
//#define	FSMWDIP150_IDLE	    0x10
//#define	FSMWDIP150_ALARM	0x20
/**
void fsm_wdog_ip150(void)
{
    if(!(SystemFlag7 & IP150_CHECK))
        return;

    switch(fsmwdip150_state)    {
        case FSMWDIP150_ENTRY:
            fsmwdip150_state = FSMWDIP150_IDLE;
            SystemFlag7 |= IP150_ALIVE;
            wdip150_timer = SEC_TIMER + 10*60;
            break;
        case FSMWDIP150_IDLE:
            if(SEC_TIMER > wdip150_timer)   {
                fsmwdip150_state = FSMWDIP150_ALARM;
                SystemFlag7 &= ~IP150_ALIVE;
                logCidEvent(account, 1, 354, 99, 150);      //aviso que el IP150 esta en problemas
            } else
            if(SystemFlag7 & IP150_KICK)    {
                SystemFlag7 &= ~IP150_KICK;
                wdip150_timer = SEC_TIMER + 10*60;
            }
            break;
        case FSMWDIP150_ALARM:
            if(SystemFlag7 & IP150_KICK)    {
                SystemFlag7 &= ~IP150_KICK;
                wdip150_timer = SEC_TIMER + 10*60;
                logCidEvent(account, 3, 354, 99, 150);      //aviso que el IP150 se normalizo
                SystemFlag7 |= IP150_ALIVE;
                fsmwdip150_state = FSMWDIP150_IDLE;
            }
            break;
        default:
            fsmwdip150_state = FSMWDIP150_ENTRY;
            break;
    }
}
**/

time_t wdogevotimer[3], e602mon_timer;
uint8_t fsmwdogevo_state[3];
//#define	FSMWDEVO_ENTRY	0x05
//#define	FSMWDEVO_IDLE	0x10
//#define	FSMWDEVO_ALARM	0x20

void fsm_wdog_evo( uint8_t this, uint8_t partition )
{
	//OS_ERR	os_err;
	struct tm currtime;

	if(this == 99)  {   //reseteo la fsm
        fsmwdogevo_state[0] = FSMWDEVO_ENTRY;
        fsmwdogevo_state[1] = FSMWDEVO_ENTRY;
        fsmwdogevo_state[2] = FSMWDEVO_ENTRY;
        Rot485_flag &= ~ROTEVO_FLAG;
        diag485[6] &= ~(1 << (5 + 0));
        diag485[6] &= ~(1 << (5 + 1));
        diag485[6] &= ~(1 << (5 + 2));
        return;
	}

	if(this > 2)
		return;
	switch(fsmwdogevo_state[this])	{
        case FSMWDEVO_ENTRY :
            FSM_ReadHistory();
            switch(this)    {
                case 0:
                    if(FSM_FLAG_1 & WDEVO0_ALRM_FLAG)    {
                        fsmwdogevo_state[this] = FSMWDEVO_ALARM;
                        Rot485_flag |= ROTEVO_FLAG;
                        diag485[6] |= (1 << (5 + this));
                        FSM_FLAG_1 |= WDEVO0_ALRM_FLAG;
                    } else {
                        FSM_FLAG_1 &= ~WDEVO0_ALRM_FLAG;
                        fsmwdogevo_state[this] = FSMWDEVO_IDLE;
                        e602mon_timer = SEC_TIMER;
                        EVOWD_Flag &= ~0x80;
                        if (BaseAlarmPkt_alarm & bitpat[APER_bit]) {
                            wdogevotimer[this] = SEC_TIMER + 15 * 60;
                        } else {
                            wdogevotimer[this] = SEC_TIMER + wdtimer * 60;
                        }
                    }
                    break;
                case 1:
                    if(FSM_FLAG_1 & WDEVO1_ALRM_FLAG)    {
                        fsmwdogevo_state[this] = FSMWDEVO_ALARM;
                        Rot485_flag |= ROTEVO_FLAG;
                        diag485[6] |= (1 << (5 + this));
                        FSM_FLAG_1 |= WDEVO1_ALRM_FLAG;
                    } else {
                        FSM_FLAG_1 &= ~WDEVO1_ALRM_FLAG;
                        fsmwdogevo_state[this] = FSMWDEVO_IDLE;
                        e602mon_timer = SEC_TIMER;
                        EVOWD_Flag &= ~0x80;
                        if (BaseAlarmPkt_alarm & bitpat[APER_bit]) {
                            wdogevotimer[this] = SEC_TIMER + 15 * 60;
                        } else {
                            wdogevotimer[this] = SEC_TIMER + wdtimer * 60;
                        }
                    }
                    break;
                case 2:
                    if(FSM_FLAG_1 & WDEVO2_ALRM_FLAG)    {
                        fsmwdogevo_state[this] = FSMWDEVO_ALARM;
                        Rot485_flag |= ROTEVO_FLAG;
                        diag485[6] |= (1 << (5 + this));
                        FSM_FLAG_1 |= WDEVO2_ALRM_FLAG;
                    } else {
                        FSM_FLAG_1 &= ~WDEVO2_ALRM_FLAG;
                        fsmwdogevo_state[this] = FSMWDEVO_IDLE;
                        e602mon_timer = SEC_TIMER;
                        EVOWD_Flag &= ~0x80;
                        if (BaseAlarmPkt_alarm & bitpat[APER_bit]) {
                            wdogevotimer[this] = SEC_TIMER + 15 * 60;
                        } else {
                            wdogevotimer[this] = SEC_TIMER + wdtimer * 60;
                        }
                    }
                    break;
            }

            break;
        case FSMWDEVO_IDLE :
            if(SEC_TIMER > wdogevotimer[this])	{
                fsmwdogevo_state[this] = FSMWDEVO_ALARM;
                logCidEvent(account, 1, 943, partition, 0);
                Rot485_flag |= ROTEVO_FLAG;
                diag485[6] |= (1 << (5 + this));
                switch(this)    {
                    case 0:
                        FSM_FLAG_1 |= WDEVO0_ALRM_FLAG;
                        break;
                    case 1:
                        FSM_FLAG_1 |= WDEVO1_ALRM_FLAG;
                        break;
                    case 2:
                        FSM_FLAG_1 |= WDEVO2_ALRM_FLAG;
                        break;
                }

                FSM_WriteHistory();
            } else
            if( EVOWD_Flag & (1 << this))	{
                EVOWD_Flag &= ~(1 << this);
                if(BaseAlarmPkt_alarm & bitpat[APER_bit])	{
                    wdogevotimer[this] = SEC_TIMER + 15*60;
                } else	{
                    wdogevotimer[this] = SEC_TIMER + wdtimer*60;
                }
            }
            //le envio señal de watchdog de evo al monitoreo
            if((SEC_TIMER > e602mon_timer + 60*60) && (!(EVOWD_Flag & 0x80)))   {
                EVOWD_Flag |= 0x80;
                e602mon_timer = SEC_TIMER;
                logCidEvent(account, 1, 602, partition, 0);
            } else
            if(SEC_TIMER > e602mon_timer + 1)  {
                EVOWD_Flag &= ~0x80;
            }
            break;
        case FSMWDEVO_ALARM :
            // a las dos de la maniana me reseteo si no hay eventos en los buffers de monitoreo
            currtime.tm_sec = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
            currtime.tm_min = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
            currtime.tm_hour = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
            if( EVOWD_Flag & (1 << this))	{
                EVOWD_Flag &= ~(1 << this);
                if(BaseAlarmPkt_alarm & bitpat[APER_bit])	{
                    wdogevotimer[this] = SEC_TIMER + 15*60;
                } else	{
                    wdogevotimer[this] = SEC_TIMER + wdtimer*60;
                }
                fsmwdogevo_state[this] = FSMWDEVO_IDLE;
                logCidEvent(account, 3, 943, partition, 0);
                Rot485_flag &= ~ROTEVO_FLAG;
                diag485[6] &= ~(1 << (5 + this));

                switch(this)    {
                    case 0:
                        FSM_FLAG_1 &= ~WDEVO0_ALRM_FLAG;
                        break;
                    case 1:
                        FSM_FLAG_1 &= ~WDEVO1_ALRM_FLAG;
                        break;
                    case 2:
                        FSM_FLAG_1 &= ~WDEVO2_ALRM_FLAG;
                        break;
                }
                FSM_WriteHistory();
            } else
            if((currtime.tm_hour == 1) && (currtime.tm_min == 30) && ((currtime.tm_sec >= 0) && (currtime.tm_sec <= 7)))	{
                //logCidEvent(account, 1, 943, partition, 0);
                Rot485_flag |= ROTEVO_FLAG;
                diag485[6] |= (1 << (5 + this));
                if(!(SystemFlag4 & E943F220_DONE)) {
                    SystemFlag4 |= E943F220_DONE;
                    SystemFlag3 |= NAPER_flag;
                    SystemFlag3 |= NAPER_F220V;
                }
            }
            if ((SystemFlag4 & E943F220_DONE) && (currtime.tm_hour == 1) && (currtime.tm_min == 30) && ((currtime.tm_sec >= 10) && (currtime.tm_sec <= 15))) {
                SystemFlag4 &= ~E943F220_DONE;
            }
            break;
        default :
            fsmwdogevo_state[this] = FSMWDEVO_ENTRY;
            Rot485_flag &= ~ROTEVO_FLAG;
            diag485[6] &= ~(1 << (5 + this));
            break;
	}
}


NET_IP_ADDR GetLocalIpAdd( void )
{
    uint8_t e2p_data[16], df_data[16], temp[8];
    uint8_t e2p_chksum, df_chksum;
    int error, i;
    uint16_t chksum1, chksum2;
    OS_ERR os_err;
    NET_ERR      err;

    //leo la data de la data_flash
    flash0_read(df_data, DF_MIPADD_OFFSET, 16);
    flash0_read(temp, DF_MIPCHK_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 16, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(LOCAL_IP_E2P_ADDR, e2p_data, 16, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(IPADDCHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 16, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            return NetASCII_Str_to_IP(e2p_data, &err);
        } else
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
    } else {
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
        else
        if( chksum2 == e2p_chksum ) {
            return NetASCII_Str_to_IP(e2p_data, &err);
        }
    }

    return NetASCII_Str_to_IP(e2p_data, &err);


}

NET_IP_ADDR GetNetmask( void )
{
    uint8_t e2p_data[16], df_data[16], temp[8];
    uint8_t e2p_chksum, df_chksum;
    int error, i;
    uint16_t chksum1, chksum2;
    OS_ERR os_err;
    NET_ERR      err;

    //leo la data de la data_flash
    flash0_read(df_data, DF_MNMASK_OFFSET, 16);
    flash0_read(temp, DF_MNMCKH_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 16, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(NETMASK_E2P_ADDR, e2p_data, 16, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(NMASKCHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 16, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            return NetASCII_Str_to_IP(e2p_data, &err);
        } else
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
    } else {
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
        else
        if( chksum2 == e2p_chksum ) {
            return NetASCII_Str_to_IP(e2p_data, &err);
        }
    }

    return NetASCII_Str_to_IP(e2p_data, &err);


}

NET_IP_ADDR GetGateway( void )
{
    uint8_t e2p_data[16], df_data[16], temp[8];
    uint8_t e2p_chksum, df_chksum;
    int error, i;
    uint16_t chksum1, chksum2;
    OS_ERR os_err;
    NET_ERR      err;

    //leo la data de la data_flash
    flash0_read(df_data, DF_MGWADD_OFFSET, 16);
    flash0_read(temp, DF_MGWCHK_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 16, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(GATEWAY_E2P_ADDR, e2p_data, 16, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(GWADDCHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 16, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            return NetASCII_Str_to_IP(e2p_data, &err);
        } else
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
    } else {
        if( chksum1 == df_chksum )   {
            return NetASCII_Str_to_IP(df_data, &err);
        }
        else
        if( chksum2 == e2p_chksum ) {
            return NetASCII_Str_to_IP(e2p_data, &err);
        }
    }

    return NetASCII_Str_to_IP(e2p_data, &err);


}

int GetMacaddress( uint8_t *boardmac)
{
    uint8_t e2p_data[16], df_data[16], temp[8];
    uint8_t e2p_chksum, df_chksum;
    int error, i;
    uint16_t chksum1, chksum2;
    OS_ERR os_err;

    //leo la data de la data_flash
    flash0_read(df_data, DF_MACADD_OFFSET, 6);
    flash0_read(temp, DF_MACCHK_OFFSET, 1);
    df_chksum = temp[0];
    chksum1 = 0;
    for( i = 0; i < 6, df_data[i] != 0; i++)
        chksum1 += df_data[i];
    chksum1 &= 0x00FF;

    //leo la data de la e2prom
    EepromReadBuffer(MACADDR_E2P_ADDR, e2p_data, 6, &error); 
    OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
    e2p_chksum = EepromReadByte(MACADDCHK_E2P_ADDR, &error);  
    chksum2 = 0;
    for( i = 0; i < 6, e2p_data[i] != 0; i++)
        chksum2 += e2p_data[i];
    chksum2 &= 0x00FF;

    if( chksum1 == chksum2) {
        if( chksum2 == df_chksum )    {
            //los checksum dieron todos bien !!!
            for(i = 0; i < 6; i++ ) {
                NetIF_MAC_Addr[i] = e2p_data[i];
                boardmac[i] = e2p_data[i];
            }
            return 1;
        } else
        if( chksum1 == df_chksum )   {
            for(i = 0; i < 6; i++ ) {
                NetIF_MAC_Addr[i] = df_data[i];
                boardmac[i] = df_data[i];
            }
            return 1;
        }
    } else {
        if( chksum1 == df_chksum )   {
            for(i = 0; i < 6; i++ ) {
                NetIF_MAC_Addr[i] = df_data[i];
                boardmac[i] = df_data[i];
            }
            return 1;
        }
        else
        if( chksum2 == e2p_chksum ) {
            for(i = 0; i < 6; i++ ) {
                NetIF_MAC_Addr[i] = e2p_data[i];
                boardmac[i] = e2p_data[i];
            }
            return 1;
        }
    }

    for(i = 0; i < 6; i++ ) {
        NetIF_MAC_Addr[i] = e2p_data[i];
        boardmac[i] = e2p_data[i];
    }
    return 1;
}

void check_fidu_dflash( void )
{
    uint8_t temp[8], buffer[16], temp1[8];
    int i, error;
    uint16_t e2p_port1, e2p_port2, df_port1, df_port2;
    uint16_t chksum1, chksum2;
    OS_ERR	os_err;

    flash0_read(temp, DF_FIDUMK_OFFSET, 1);

    if(temp[0] == 0x5A) {
        //aca veo individualmente que falta inicializar
        flash0_read(temp, DF_FIDUDT_OFFSET, 1);
        if(!(temp[0] & 0x01))  {
            for(i=0; i<16; i++) { buffer[i] = 0; }
            EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, 16, &error);
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
            pgm_localip(buffer);
/*            temp1[0] = EepromReadByte(IPADDCHK_E2P_ADDR, &error);
            flash0_write(1, temp1, DF_MIPCHK_OFFSET, 1 );
            temp[0] |= 0x01;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );*/
            WDT_Feed();
        }
        if(!(temp[0] & 0x02))  {
            for(i=0; i<16; i++) { buffer[i] = 0; }
            EepromReadBuffer(NETMASK_E2P_ADDR, buffer, 16, &error);
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
            pgm_netmask(buffer);
/*            temp1[0] = EepromReadByte(NMASKCHK_E2P_ADDR, &error);
            flash0_write(1, temp1, DF_MNMCKH_OFFSET, 1 );
            temp[0] |= 0x02;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );*/
            WDT_Feed();
        }
        if(!(temp[0] & 0x04))  {
            for(i=0; i<16; i++) { buffer[i] = 0; }
            EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, 16, &error);
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
            pgm_gateway(buffer);
/*            temp1[0] = EepromReadByte(GWADDCHK_E2P_ADDR, &error);
            flash0_write(1, temp1, DF_MGWCHK_OFFSET, 1 );
            temp[0] |= 0x04;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );*/
            WDT_Feed();
        }
        if(!(temp[0] & 0x08))  {
            for(i=0; i<16; i++) { buffer[i] = 0; }
            EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
            flash0_write(1, buffer, DF_MACADD_OFFSET, 6 );
            temp1[0] = EepromReadByte(MACADDCHK_E2P_ADDR, &error);
            flash0_write(1, temp1, DF_MACCHK_OFFSET, 1 );
            temp[0] |= 0x08;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
            WDT_Feed();
        }
        if(!(temp[0] & 0x10))   {
            EepromReadBuffer(SERVER1_E2P_ADDR, buffer, 16, &error); 
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
            pgm_server1(buffer);
            temp[0] |= 0x10;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
            WDT_Feed();
        }
        if(!(temp[0] & 0x20))   {
            EepromReadBuffer(SERVER2_E2P_ADDR, buffer, 16, &error); 
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err); 
            pgm_server2(buffer);
            temp[0] |= 0x20;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
            WDT_Feed();
        }
        if(!(temp[0] & 0x40))   {
            e2p_port1 = EepromReadWord(PORT1_E2P_ADDR, &error);
            temp1[0] = (e2p_port1 >> 8) & 0x00FF;
            temp1[1] = e2p_port1 & 0x00FF;
            flash0_write(1, temp1, DF_PORT1_OFFSET, 2 );
            temp[0] |= 0x40;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
        }
        if(!(temp[0] & 0x80))   {
            e2p_port2 = EepromReadWord(PORT2_E2P_ADDR, &error);
            temp1[0] = (e2p_port2 >> 8) & 0x00FF;
            temp1[1] = e2p_port2 & 0x00FF;
            flash0_write(1, temp1, DF_PORT2_OFFSET, 2 );
            temp[0] |= 0x80;
            flash0_write(1, temp, DF_FIDUDT_OFFSET, 1 );
        }
        return;
    } else  {
        //aca hay que inicializar todo
        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(LOCAL_IP_E2P_ADDR, buffer, 16, &error);
        pgm_localip(buffer);
        temp[0] = EepromReadByte(IPADDCHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_MIPCHK_OFFSET, 1 );
        WDT_Feed();

        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(NETMASK_E2P_ADDR, buffer, 16, &error);
        pgm_netmask(buffer);
        temp[0] = EepromReadByte(NMASKCHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_MNMCKH_OFFSET, 1 );
        WDT_Feed();

        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(GATEWAY_E2P_ADDR, buffer, 16, &error);
        pgm_gateway(buffer);
        temp[0] = EepromReadByte(GWADDCHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_MGWCHK_OFFSET, 1 );
        WDT_Feed();

        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(MACADDR_E2P_ADDR, buffer, 6, &error);
        flash0_write(1, buffer, DF_MACADD_OFFSET, 6 );
        temp[0] = EepromReadByte(MACADDCHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_MACCHK_OFFSET, 1 );
        WDT_Feed();

        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(SERVER1_E2P_ADDR, buffer, 16, &error);
        pgm_server1(buffer);
        temp[0] = EepromReadByte(SRV1ACHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_SR1CHK_OFFSET, 1 );
        WDT_Feed();

        for(i=0; i<16; i++) { buffer[i] = 0; }
        EepromReadBuffer(SERVER2_E2P_ADDR, buffer, 16, &error);
        pgm_server2(buffer);
        temp[0] = EepromReadByte(SRV2ACHK_E2P_ADDR, &error);
        flash0_write(1, temp, DF_SR2CHK_OFFSET, 1 );
        WDT_Feed();

        e2p_port1 = EepromReadWord(PORT1_E2P_ADDR, &error);
        temp[0] = (e2p_port1 >> 8) & 0x00FF;
        temp[1] = e2p_port1 & 0x00FF;
        flash0_write(1, temp, DF_PORT1_OFFSET, 2 );

        e2p_port2 = EepromReadWord(PORT2_E2P_ADDR, &error);
        temp[0] = (e2p_port2 >> 8) & 0x00FF;
        temp[1] = e2p_port2 & 0x00FF;
        flash0_write(1, temp, DF_PORT2_OFFSET, 2 );

        buffer[0] = 0x5A;
        flash0_write(1, buffer, DF_FIDUMK_OFFSET, 1);
        buffer[0] = 0xFF;
        flash0_write(1, buffer, DF_FIDUDT_OFFSET, 1);
        WDT_Feed();

        return;
    }
}

uint8_t fsm_gprsdet_state;


//void fsm_gprs_detection(void)
//{
//	static uint32_t timeout;
//
//	if((IRIDIUM_flag & IRI_GPRSKICK1_FLAG) || (IRIDIUM_flag & IRI_GPRSKICK2_FLAG))	{
//		timeout = SEC_TIMER + 5*60;
//		IRIDIUM_flag &= ~IRI_GPRSKICK1_FLAG;
//		IRIDIUM_flag &= ~IRI_GPRSKICK2_FLAG;
//		IRIDIUM_flag &= ~IRI_GPRSNG_FLAG;
//		fsm_gprsdet_state = FSM_GPRSD_IDLE;
//	}
//
//	switch(fsm_gprsdet_state)	{
//		case FSM_GPRSD_IDLE:
//			if(timeout > SEC_TIMER)	{
//				IRIDIUM_flag |= IRI_GPRSNG_FLAG;
//				fsm_gprsdet_state = FSM_GPRSD_NG;
//			}
//			break;
//		case FSM_GPRSD_NG:
//			break;
//		default:
//			fsm_gprsdet_state = FSM_GPRSD_IDLE;
//			timeout = SEC_TIMER + 5*60;
//			IRIDIUM_flag &= ~IRI_GPRSKICK1_FLAG;
//			IRIDIUM_flag &= ~IRI_GPRSKICK2_FLAG;
//			break;
//	}
//}

void fsm_console_enter(void)
{
    switch(fsm_conent_state)    {
        case FCS_IDLE:
            if(RADAR_flags & CONSOLE_ENTER) {
                RADAR_flags &= ~CONSOLE_ENTER;
                fsm_conent_state = FCS_CONIN;
                logCidEvent(account, 1, 628, 0, 0);
                if(!(SysFlag_AP_Apertura & AP_APR_VALID))    {
                    SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                    SystemFlag11 |= CONSOLASAL_FLAG;
                    fsm_conent_state = FCS_ALRM;
                }
            }
            // OJO reactivar
#ifdef  USAR_LICENSIA
            if(SystemFlag6 & USE_LICENSE)   {
                if(RADAR_flags & LIC_ENTER) {
                    RADAR_flags &= ~LIC_ENTER;
                    fsm_conent_state = FCS_NORMAL;
                }
            }
#endif
            break;
        case FCS_CONIN:
            if(RADAR_flags & CONSOLE_OUT)   {
                RADAR_flags &= ~CONSOLE_OUT;
                fsm_conent_state = FCS_IDLE;
                RADAR_flags &= ~CONSOLE_CMDIN;
                logCidEvent(account, 3, 628, 0, 0);
            }
            // OJO reactivar
#ifdef  USAR_LICENSIA
            if((RADAR_flags & LIC_ENTER) && (SystemFlag6 & USE_LICENSE) ){
                RADAR_flags &= ~LIC_ENTER;
                fsm_conent_state = FCS_NORMAL;
            } else 
#endif
            if((RADAR_flags & CONSOLE_CMDIN) && (!(SystemFlag11 & FIRSTCMD_FLAG)))   {
                fsm_conent_state = FCS_ALRM;
                RADAR_flags &= ~CONSOLE_CMDIN;
                SysFlag_AP_GenAlarm |= bitpat[ASAL_bit];
                SystemFlag11 |= FIRSTCMD_FLAG;
            }
            break;
        case FCS_ALRM:
            if(!(SysFlag_AP_Apertura & AP_APR_VALID))  {
                if(asal_state == AUTR_ALRMED)   {
                   fsm_conent_state = FCS_CONIN;
                   //asal_autr_timer = (SEC_TIMER + 60*10 - asal_autorst_timer_min);
                }
            } else  {
               fsm_conent_state = FCS_CONIN;
               recharge5min_alarm();
            }
            break;
        case FCS_NORMAL:
            if(RADAR_flags & CONSOLE_OUT)   {
                RADAR_flags &= ~CONSOLE_OUT;
                fsm_conent_state = FCS_IDLE;
                logCidEvent(account, 3, 628, 0, 0);
            }
            if(RADAR_flags & CONSOLE_CMDIN)   {
                RADAR_flags &= ~CONSOLE_CMDIN;
            }
            break;
        default:
            fsm_conent_state = FCS_IDLE;
            break;
    }
}

void HardFault_Handler(void)
{
    CommSendString(DEBUG_COMM, "HARD_FAULT\n\r");
    while(1);
}

void NMI_Handler( void )
{
    CommSendString(DEBUG_COMM, "NMI_FAULT\n\r");
    while(1);
}

void MemManage_Handler( void )
{
    CommSendString(DEBUG_COMM, "MEMMANAGE_FAULT\n\r");
    while(1);
}

void BusFault_Handler( void )
{
    CommSendString(DEBUG_COMM, "BUS_FAULT\n\r");
    while(1);

}

void UsageFault_Handler( void )
{
    CommSendString(DEBUG_COMM, "USAGE_FAULT\n\r");
    while(1);
}

void RIT_IRQHandler( void )
{
    CommSendString(DEBUG_COMM, "RIT_IRQHandler\n\r");
    return;
}




