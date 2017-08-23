#include "SysTick.h"
/*配置系统时钟，最小为1us*/
static __IO u32 TimingDelay;

void Delay_us(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;
 
  while(TimingDelay != 0);
}

void SysTick_Configuration(void)
{
  /* SystemFrequency / 1000    1ms中断一次
	 * SystemFrequency / 100000	 10us中断一次
	 * SystemFrequency / 1000000 1us中断一次
	 */
  if (SysTick_Config(SystemCoreClock / 1000000)) //SysTick配置函数
  { 
    /* Capture error */ 
    while (1);
  }  
 /* Configure the SysTick handler priority */
  NVIC_SetPriority(SysTick_IRQn, 0x0);//SysTick中断优先级设置
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
