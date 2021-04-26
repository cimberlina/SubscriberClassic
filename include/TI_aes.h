/*
 * TI_aes.h
 *
 *  Created on: Feb 10, 2014
 *      Author: Claudio
 */

#ifndef TI_AES_H_
#define TI_AES_H_

void aes_encrypt(unsigned char *state, unsigned char *key);
void aes_decrypt(unsigned char *state, unsigned char *key);

#endif /* TI_AES_H_ */
