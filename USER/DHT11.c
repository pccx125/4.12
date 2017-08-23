 /********************************************************************************
 * �ļ���  ��DHT11
 * ����    ��DHT11 Ӧ�ú�����         
 * ʵ��ƽ̨��Ұ��STM32������
 * Ӳ�����ӣ�----------------------
 *          |   PD12 - DHT11-DATA   |
 *           ---------------------- 
 * ��汾  ��ST3.5.0
 * ����    ������ 
 * ��̳    ��http://www.amobbs.com/forum-1008-1.html
 * �Ա�    ��http://firestm32.taobao.com
**********************************************************************************/
#include "main.h"

/*
 * ��������DHT11_GPIO_Config
 * ����  ������DHT11�õ���I/O��
 * ����  ����
 * ���  ����
 */
void DHT11_GPIO_Config(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOF, &GPIO_InitStructure);  


	GPIO_SetBits(GPIOF, GPIO_Pin_7);	 
}

/*
 * ��������DHT11_Mode_IPU
 * ����  ��ʹDHT11-DATA���ű�Ϊ����ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_IPU(void)
{
 	 GPIO_InitTypeDef GPIO_InitStructure; 
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            //��������
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	 GPIO_Init(GPIOF, &GPIO_InitStructure);
}

/*
 * ��������DHT11_Mode_Out_PP
 * ����  ��ʹDHT11-DATA���ű�Ϊ���ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOF, &GPIO_InitStructure);   	 
}


static uint8_t Read_Byte(void)
{	  

   	 uint8_t i, temp=0;

	 for(i=0;i<8;i++)    
	 {
	 
	   /*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/  
    	while(DHT11_DATA_IN()==Bit_RESET);
		 
		 /*DHT11 ��27~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
	 		 ͨ�����60us��ĵ�ƽ��������������״̬*/

		Delay_us(60); //��ʱ60us		   	  
	
		  if(DHT11_DATA_IN()==Bit_SET)//60us����Ϊ�ߵ�ƽ��ʾ���ݡ�1��
		   {
		   	/*��ѯֱ���ӻ�������ʣ��� 30us �ߵ�ƽ����*/
   				 while(DHT11_DATA_IN()==Bit_SET);

				 temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1 
			
		   }
		   else	 //60us��Ϊ�͵�ƽ��ʾ���ݡ�0��
		   {			   
		   	  temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0
		   }
	 }
	  return temp;
}

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  
	/*���ģʽ*/
   DHT11_Mode_Out_PP();
   /*��������*/
   DHT11_DATA_OUT(LOW);
   /*��ʱ18ms*/
   Delay_ms(18);

   /*�������� ������ʱ30us*/
   DHT11_DATA_OUT(HIGH); 

   Delay_us(30);   //��ʱ30us
   
 	/*������Ϊ���� �жϴӻ���Ӧ�ź�*/ 
   DHT11_Mode_IPU();

 /*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/   
   if(DHT11_DATA_IN()==Bit_RESET)   //T !   
    {
  
	
	  /*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/  
	    while(DHT11_DATA_IN()==Bit_RESET);
	  	 
	  /*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/
	    while(DHT11_DATA_IN()==Bit_SET);

	  /*��ʼ��������*/   
	 	 DHT11_Data->humi_int= Read_Byte();
								
		 DHT11_Data->humi_deci= Read_Byte();
				
		 DHT11_Data->temp_int= Read_Byte();

		 DHT11_Data->temp_deci= Read_Byte();
				 		
		 DHT11_Data->check_sum= Read_Byte();
		 								  

		 	/*��ȡ���������Ÿ�Ϊ���ģʽ*/
  		 DHT11_Mode_Out_PP();
		   /*��������*/
		 DHT11_DATA_OUT(HIGH);

		   /*����ȡ�������Ƿ���ȷ*/
		 if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
		  	return SUCCESS;
		  else 
		  	return ERROR;
   }
   else
   	{		
			return ERROR;
		}
   
}

	  


/*************************************END OF FILE******************************/
