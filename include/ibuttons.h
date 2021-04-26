/*
 * ibuttons.h
 *
 *  Created on: Jan 4, 2012
 *      Author: IronMan
 */

#ifndef IBUTTONS_H_
#define IBUTTONS_H_

uint8_t DowCRC( uint8_t * buffer );
uint8_t iButton_rx_byte( void );
uint8_t iButton_read( uint8_t buffer[] );
uint8_t iButton_r_bit( void );
void iButton_tx_byte ( uint8_t data );
uint8_t iButton_cmd( uint8_t cmd );
void iButton_w_bit( int16_t bit );
uint8_t iButton_reset( void );
uint8_t check_id( uint8_t *idbuffer);
uint8_t check_id_2( uint8_t *idbuffer);


#endif /* IBUTTONS_H_ */
