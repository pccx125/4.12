#include"main.h"

void IWDG_Init(u8 prer,u16 rlr) 
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);		//使能对IWDG->PR和IWDG->RLR的写		 								  
  IWDG_SetPrescaler(prer);  //装载预分频寄存器
  IWDG_SetReload(rlr);  //装载重装载寄存器
	IWDG_ReloadCounter();		//复位									   
  IWDG_Enable();//使能看门狗	
}
//喂独立看门狗
void IWDG_Feed(void)
{
	IWDG_ReloadCounter(); //reload											   
}






