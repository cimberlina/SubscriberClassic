/*
 * DataFlash.h
 *
 *  Created on: Mar 1, 2012
 *      Author: ironman
 */

#ifndef DATAFLASH_H_
#define DATAFLASH_H_

uint32_t flash0_init (void);
uint32_t flash0_write(int dfbuff, uint8_t* buf, uint32_t offset, uint32_t len);
uint32_t flash0_read(uint8_t* buf, uint32_t offset, uint32_t len);
uint16_t flash0_getPageSize(void);
void flash0_setToBinaryPageSize(void);

#endif /* DATAFLASH_H_ */
