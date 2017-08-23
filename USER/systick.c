#include "SysTick.h"
/*����ϵͳʱ�ӣ���СΪ1us*/
static __IO u32 TimingDelay;

void Delay_us(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;
 
  while(TimingDelay != 0);
}

void SysTick_Configuration(void)
{
  /* SystemFrequency / 1000    1ms�ж�һ��
	 * SystemFrequency / 100000	 10us�ж�һ��
	 * SystemFrequency / 1000000 1us�ж�һ��
	 */
  if (SysTick_Config(SystemCoreClock / 1000000)) //SysTick���ú���
  { 
    /* Capture error */ 
    while (1);
  }  
 /* Configure the SysTick handler priority */
  NVIC_SetPriority(SysTick_IRQn, 0x0);//SysTick�ж����ȼ�����
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
