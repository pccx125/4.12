#ifndef __SPI1_H
#define __SPI1_H
#include "stm32f2xx.h"


/* SPI�����ٶ�����*/
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1




																					  
void SPI1_Init(void);			 //��ʼ��SPI�� 
//void SPIx_SetSpeed(u8 SpeedSet); //����SPI�ٶ�
u8 SPI1_ReadWriteByte(u8 TxData);//SPI���߶�дһ���ֽ�
void SPI1_SetSpeed(u8 SpeedSet); 
#endif

