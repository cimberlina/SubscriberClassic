/*
 * telnet_server.h
 *
 *  Created on: Oct 25, 2011
 *      Author: IronMan
 */

#ifndef TELNET_SERVER_H_
#define TELNET_SERVER_H_


extern OS_TCB		TelnetServerTask_TCB;
extern CPU_STK		TelnetServerTask_Stk[];



void  TelnetServerTask (void  *p_arg);


#endif /* TELNET_SERVER_H_ */
