#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f2xx.h"

void SysTick_Configuration(void);
void Delay_us(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

#define Delay_ms(x) Delay_us(1000*x)	 //µ¥Î»ms


#endif /* __SYSTICK_H */


