/*
 * utils.h
 *
 *  Created on: Oct 31, 2011
 *      Author: IronMan
 */

#ifndef UTILS_H_
#define UTILS_H_

#define	NORMAL_led		7	//0x80
#define	F220_led		6	//0x40
#define	LLOP_led		5	//0x20
#define	APER_led		4	//0x10
#define	ROTU_led		3	//0x08
#define	TESO_led		2	//0x04
#define	INCE_led		1	//0x02
#define	ASAL_led		0	//0x01

#define	NORMAL_led_mask		0x80
#define	F220_led_mask		0x40
#define	LLOP_led_mask		0x20
#define	APER_led_mask		0x10
#define	ROTU_led_mask		0x08
#define	TESO_led_mask		0x04
#define	INCE_led_mask		0x02
#define	ASAL_led_mask		0x01

#define	BLINK_FOREVER		0xFF

//de la fsm que maneja los leds
#define	LED_IDLE		0
#define	LED_ON		1
#define	LED_OFF		2


typedef struct	 {
	unsigned char	led_state;
	unsigned int	led_cad;
	unsigned int	led_tim_on;
	unsigned int	led_tim_off;
	unsigned char	led_mask;
	unsigned char	led_blink;
	unsigned char	led_flags;
} Led_Struct;

extern	OS_TCB		FsmLedsTask_TCB;
extern	CPU_STK		FsmLedsTask_Stk[FsmLedsTask_STK_SIZE];
extern	Led_Struct 	led_dcb[8];
extern	Led_Struct 	led_swp[8];
extern 	Led_Struct 	AP_Aper_led_dcb;
extern	Led_Struct 	Buzzer_dcb;
extern	uint8_t bledShadow, bledShadow2;

extern Led_Struct led_screen[12][8];
extern uint8_t SLedShadow[12];
extern uint16_t DiagEventFlags;

void SetLeds( uint8_t imageled );

void FsmLedsTask( void );
void fsm_AP_aper_led( void );
void fsm_buzzer( void );
void ScreenLedBlink(uint8_t screen, uint8_t led, uint8_t ton, uint8_t toff, uint8_t blink);

void printHexWord(unsigned int mydata);
void sprintHexWord(char buffer[], unsigned int mydata);
void sprintHexDWord(char buffer[], unsigned int mydata);
void printByte(unsigned char mydata);
void sprintByte(char buffer[], unsigned char mydata);
void dumpMemory( uint8_t *address, uint16_t len );
void conio_printHexWord(ConsoleState* state, unsigned int mydata);
void conio_printByte(ConsoleState* state, unsigned char mydata);

void delay_us( uint32_t microseconds);

int i2ctest_wr (void);
int i2ctest_rd (void);



char *itoa(int i);
uint8_t htoi_nibble( char data);
int ChrIndexOf( uint8_t *string, uint8_t key,  int start);
uint8_t *SubString(uint8_t *string1, uint8_t *string2, int start, int end);
uint8_t *SubStringEnd(uint8_t *string1, uint8_t *string2, int start);
int str_starts_with(const char *str, const char *start);
char *strnext(char **sp, const char *delim);
int StrIndexOf(uint8_t s1[], uint8_t s2[]);
int StrLastIndexOf(uint8_t s1[], uint8_t s2[]);
int atoi(const char *s);
uint16_t atoh(char *String);
void BuffPutHex (char *buffer, uint8_t hexnum);
uint16_t BCD_Word_to_int( uint16_t data);

void printBuffByte( uint8_t *data, uint8_t len);
void printBuffByteRow( uint8_t *data, uint8_t len);
void printByte2(unsigned char mydata);





#endif /* UTILS_H_ */
