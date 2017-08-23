#ifndef __ADC_H
#define __ADC_H

#include"main.h"




#define  ADC1_DR_ADDRESS                  ((uint32_t)0x4001204C) //ADC1 DR¼Ä´æÆ÷»ùµØÖ·
void ADC_GPIO_Configuration(void);

void ADC_DMA_Config(void);
#endif


