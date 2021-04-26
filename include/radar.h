/*
 * radar.h
 *
 *  Created on: Mar 12, 2014
 *      Author: Claudio
 */

#ifndef RADAR_H_
#define RADAR_H_

extern uint64_t RADAR_distance, RADAR_distance0, RADAR_distance1, RADAR_distance2, RADAR_distance3;
extern uint32_t RADAR_t0;
extern uint64_t RADAR_tickcount;
extern uint64_t new_radar_limit;

#define RADAR_DISTANCE_MIN  230
#define RADAR_DISTANCE_MAX  800


void RADAR_delayUs(uint32_t us);
uint64_t RADAR_getUsTime( void );
void RADAR_trigger(void);
void EINT3_IRQHandler(void);



#endif /* RADAR_H_ */
