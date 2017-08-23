#ifndef __USART_H
#define __USART_H

#include"main.h"

void RS485_Init(void);
void RS232_Init(void);
void RS232_MegSend(u8 data);
void RS232_WaterSend(u8 data);
void RS485_Send(u8 *str);
#endif


