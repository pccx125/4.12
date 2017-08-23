#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f2xx.h"

#include <stdio.h>
#include"adc.h"                       //AD转换的头文件
#include <string.h>
#include <stdio.h>
#include "SysTick.h" 
#include "usart.h"
#include "fsmc_nand.h"
#include "crc16.h"
#include "response.h"
#include "nand_wr.h"
#include "led.h"
#include "spi1.h"
#include "mmc_sd.h"
#include "diskio.h"
#include "ff.h"
#include "MegDriver.h"
#include "M41T81.h"
#include "DHT11.h"
#include "adxl345.h"
#include "GPSSource.h"
#include <math.h> 
#include "wireless.h"
#include "iwdg.h"
#include "GPSTIME.h"

/*蜂鸣器控制*/
#define BeepON GPIO_SetBits(GPIOF,GPIO_Pin_8)
#define BeepOFF GPIO_ResetBits(GPIOF,GPIO_Pin_8)
/*BTS555控制*/
#define OutputSourceON GPIO_SetBits(GPIOB,GPIO_Pin_9)
#define OutputSourceOFF GPIO_ResetBits(GPIOB,GPIO_Pin_9)

/*水声端口处的使能口控制*/
#define MEG_Disable GPIO_SetBits(GPIOF,GPIO_Pin_10)
#define MEG_Enable GPIO_ResetBits(GPIOF,GPIO_Pin_10)


// #define ControlBoardID 0x02
#endif


