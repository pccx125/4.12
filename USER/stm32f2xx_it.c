/**
  ******************************************************************************
  * @file    SDIO/uSDCard/stm32f2xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "main.h"

extern void TimingDelay_Decrement(void);

#define GPS_Buf_N   400	    			
extern u8 GPS_Buf[GPS_Buf_N];
extern volatile uint32_t Rec_Len;
u8 flag_rx = 0; //���ճɹ���־λ
extern __IO uint16_t ADCConvertedValue[12];//3��ͨ�����ֱ�ȡ�Ĵ�ȡƽ��ֵ
u16 pressuretest,batterytest,currenttest;//ѹ�����͵�ص�ѹ���
u8 inputmegstr[137],inputwaterstr[9],input485str[14];//���մ�������
u8 REC485[400]={0},REC485CNT=0;//uart5 485��������
u8 usart1data,usart3data;//���մ�������
u8 usart1count=0,usart3count=0,uart4count=0;//���ڽ������ݼ���
u8 megdatalen=0,data485len=0,waterdatalen=0;//���ݶγ���
u8 Rxmegflag=0;//��1��ʾ���ڽ������
u8 Rxwaterflag=0;//Rxwaterflag��1��ʾ���ڽ������
           
u8 GPS_Counter = 0 ; 	//GPS�����˳�������
u8 Data_correctly_flag = 0;  //GPS������ȷ��־λ
u8 i=0;

u8 watersendcomplet=0,watersendfall=0;//�ۿ�ͨ��ˮ����װ嵥Ԫ������Ϣ�����������Σ�ÿ��watersendcomplet+1.watersendfall=1ʱ��ʾ�ű�ˮ������ˮ��������Ϣʧ��
u8 RX_uart4flag=0;//����4������ɱ�־
extern u8 ControlBoardID;//�ű��豸ʶ��
extern u16 time;//���������ǵ�1000����ʾ����1ms
extern u32 ZTtime;//����������ʾ����ZTtime ��ms
extern u8 zttimeflag;//Ϊ1ʱ��time��ʼ����
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
void delay(uint32_t count)
{
	while(count--);
}
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	TimingDelay_Decrement();
	if(zttimeflag)
	{
		time++;
		if(time==0x03E8)//1000us=1ms
		{
			time=0;
			ZTtime++;
		}
	}
}

/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/******************************************************************************/


/* dma�жϴ�����*/
void DMA2_Stream0_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0))    //�ж���DMA��������ж�
  {
		DMA_ClearITPendingBit(DMA2_Stream0,DMA_IT_TCIF0);    //���DMA��������жϱ�־λ		
		ADC_DMACmd(ADC1, DISABLE);                           //ֹͣadc��dmaת��
		ADC_Cmd(ADC1, DISABLE);
		IWDG_Feed();  //ι��
		pressuretest=((ADCConvertedValue[1]+ADCConvertedValue[4]+ADCConvertedValue[7]+ADCConvertedValue[10])*16/4/0x1000);
		pressuretest=pressuretest*33;
		batterytest=((ADCConvertedValue[0]+ADCConvertedValue[3]+ADCConvertedValue[6]+ADCConvertedValue[9])*110/4/0x1000);//0729zby���ģ�0x1000Ϊ4096����Ϊ���㣩
		batterytest=batterytest*33;
		IWDG_Feed();  //ι��
		currenttest=((ADCConvertedValue[2]+ADCConvertedValue[5]+ADCConvertedValue[8]+ADCConvertedValue[11])*1000/4/0x1000);
		currenttest=currenttest*33;
	}
}
void USART1_IRQHandler(void)//ˮ��ģ������ж�
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
		USART_ClearFlag(USART1, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    usart1data = USART_ReceiveData(USART1);
    inputwaterstr[usart1count++]=usart1data;
		if(inputwaterstr[0]==0x7E)//�ж�����ĵ�һ���ֽ��Ƿ�Ϊ��ͷ
		{
			if(usart1count>=5)
			{
				if((inputwaterstr[1]==ControlBoardID)&&(inputwaterstr[2]==0x00))   //˵��Ϊ�ű�ˮ��ģ��ķ�����Ϣ
				{
					if(usart1count==8)
					{
						usart1count=0;
						if((inputwaterstr[3]!=0x0C)&&(inputwaterstr[3]!=0x0D))
						{
							watersendcomplet++;
							if(inputwaterstr[5]==0x00)
								watersendfall=1;
						}
					}
				}
				else if(inputwaterstr[2]==ControlBoardID)
				{
					if(usart1count==5)
						waterdatalen=inputwaterstr[4];
					if(usart1count==waterdatalen+7)
					{
						usart1count=0;
						Rxwaterflag=1;
					}
				}
				else
					usart1count=0;
			}		
		}
		else   //����������ĵ�һ���ֽڲ��Ǳ�ͷ������������usart1countΪ0
			usart1count=0;
  }
}
void USART3_IRQHandler(void)//RS232�����жϣ��Ų�ģ��
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {
		USART_ClearFlag(USART3, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    usart3data = USART_ReceiveData(USART3);
 		inputmegstr[usart3count++]=usart3data;
		if(usart3count>=5)
		{
			//if((inputmegstr[0]==0x7E)&&(inputmegstr[1]==ControlBoardID))
			if(inputmegstr[0]==0x7E)
			{
				if(usart3count>=5)
				{
					if(usart3count==5)
						megdatalen=inputmegstr[4];
					if(usart3count==megdatalen+7)
					{
						Rxmegflag=1;
						usart3count=0;
					}
				}	
			}
			else
				usart3count=0;
		}
  }
	
}
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13)!= RESET)
	{
		delay(10000);	
    if(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)) 
		{			
			BeepON;
			delay(1000000);
			delay(1000000);
			BeepOFF;
		}				
		EXTI_ClearITPendingBit(EXTI_Line13);     //����жϱ�־λ  
	}
}
void UART4_IRQHandler(void)//����ģ������ж�
{
	u8 uart4data;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
  {
		USART_ClearFlag(UART4, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    uart4data = USART_ReceiveData(UART4);
		input485str[uart4count++]=uart4data;
		if(uart4count>=3)
		{
			if((input485str[0]==0x7E)&&(input485str[1]==0x7E)&&(input485str[2]!=0x7E))
			{
				if(uart4count>=4)
				{
					if(uart4count==4)
						data485len=input485str[3];
					if(uart4count==data485len+6)
					{
						RX_uart4flag=1;
						uart4count=0;
					}
					if(data485len+6>14)
						uart4count=0;
				}	
			}
			else
				uart4count=0;
		}
  }
}

void UART5_IRQHandler(void)//485��ʱ�����ж�
{

	if( (USART_GetFlagStatus(UART5, USART_IT_RXNE) != RESET) )
	{
		USART_ClearFlag(UART5, USART_IT_RXNE);
	

		 REC485[REC485CNT++] = (uint8_t)USART_ReceiveData(UART5);   //?????????????????
      if(REC485[REC485CNT-1]=='$')      //�����յ����ǲ��Ǳ�ͷ$
      {
            
              REC485[0]='$'; 
              REC485CNT = 1; 
              GPS_Counter = 0;
              Data_correctly_flag = 1;
      }
      else if(GPS_Counter < 3 && Data_correctly_flag == 1)
      {
           
              GPS_Counter++;
      }
      if(GPS_Counter == 3 && (REC485[GPS_Counter] != 'R'))  //���Ĵν��ж�
      {
              
              GPS_Counter = 0;
              REC485CNT = 0;
              Data_correctly_flag = 0;
      }
      if((REC485[REC485CNT-1]==0x0a) && (Data_correctly_flag == 1))                     //���Ľ�β�س����з�0x0d 0x0a
      {	  			
             
              for(i=0; i< REC485CNT; i++) 
                GPS_Buf[i]= REC485[i]; 	     //????????????????
              flag_rx= 1;	//
		
              Rec_Len = REC485CNT;
              REC485CNT = 0;
              GPS_Counter = 0;
              Data_correctly_flag = 0;
      }
	}
}
/******************************************************************************/
/*                 STM32F2xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f2xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
