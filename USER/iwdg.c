#include"main.h"

void IWDG_Init(u8 prer,u16 rlr) 
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);		//ʹ�ܶ�IWDG->PR��IWDG->RLR��д		 								  
  IWDG_SetPrescaler(prer);  //װ��Ԥ��Ƶ�Ĵ���
  IWDG_SetReload(rlr);  //װ����װ�ؼĴ���
	IWDG_ReloadCounter();		//��λ									   
  IWDG_Enable();//ʹ�ܿ��Ź�	
}
//ι�������Ź�
void IWDG_Feed(void)
{
	IWDG_ReloadCounter(); //reload											   
}






