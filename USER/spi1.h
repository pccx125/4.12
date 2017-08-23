#ifndef __SPI1_H
#define __SPI1_H
#include "stm32f2xx.h"


/* SPI总线速度设置*/
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1




																					  
void SPI1_Init(void);			 //初始化SPI口 
//void SPIx_SetSpeed(u8 SpeedSet); //设置SPI速度
u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
void SPI1_SetSpeed(u8 SpeedSet); 
#endif

