/*
 * radar.c
 *
 *  Created on: Mar 10, 2014
 *      Author: Claudio
 */

#include "includes.h"

uint64_t RADAR_distance, RADAR_distance0, RADAR_distance1, RADAR_distance2, RADAR_distance3;
uint64_t new_radar_limit;
uint32_t RADAR_t0;
uint64_t RADAR_tickcount;


void RADAR_delayUs(uint32_t us)
{
	uint32_t t0;
	uint64_t t;

	t0 = LPC_TIM1->TC;
	for(;;)	{
		t = LPC_TIM1->TC;
		if (t < t0)
			t += 0x100000000;

		if (us < t - t0)
			return;

		us -= (t - t0);
		t0  = t;
	}

}

uint64_t RADAR_getUsTime( void )
{


	uint64_t t;

	t = LPC_TIM1->TC;
	if (t < RADAR_t0)
		t += 0x100000000;

	RADAR_tickcount = (t - RADAR_t0);
	//RADAR_t0 = t;

        
	return RADAR_tickcount;
}

void RADAR_trigger(void)
{

	GPIO_ClearValue(1, (1 << 22));
	RADAR_delayUs(2);
	GPIO_SetValue(1, (1 << 22));
	RADAR_delayUs(10);
	GPIO_ClearValue(1, (1 << 22));
	//t = RADAR_getUsTime();

	RADAR_t0 = LPC_TIM1->TC;
	//me preparo a esperar el rising edge
	LPC_GPIOINT->IO0IntEnF &= ~(1<<4);
	LPC_GPIOINT->IO0IntEnR |= (1<<4);
	RADAR_flags |= RADAR_IRQ_RISE;
    LPC_GPIOINT->IO0IntClr |= (1<<4);

	RADAR_flags |= RADAR_START;
	//RADAR_distance = 0;

}

void EINT3_IRQHandler(void)		//rutina de interrupcion del ultrasonico
{
	if(RADAR_flags & RADAR_IRQ_RISE)	{
		RADAR_t0 = LPC_TIM1->TC;
		LPC_GPIOINT->IO0IntEnR &= ~(1<<4);
		LPC_GPIOINT->IO0IntEnF |= (1<<4);
		RADAR_flags &= ~RADAR_IRQ_RISE;
        LPC_GPIOINT->IO0IntClr |= (1<<4);
		return;
	}

	RADAR_distance0 = RADAR_distance1;
	RADAR_distance1 = RADAR_distance2;
	RADAR_distance2 = RADAR_distance3;
	RADAR_distance3 = RADAR_getUsTime();		//-457
	RADAR_flags &= ~RADAR_START;
	LPC_GPIOINT->IO0IntClr |= (1<<4);
}
