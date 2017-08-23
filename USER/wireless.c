#include"main.h"

void WirelessUsart_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);    

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	USART_Init(UART4, &USART_InitStructure);
	USART_ITConfig(UART4,USART_IT_RXNE,ENABLE);
	USART_Cmd(UART4, ENABLE);
	USART_GetFlagStatus(UART4, USART_FLAG_TC);
}
void WirelessGPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOA,ENABLE);
	//M1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//M0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//AUX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            //上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void SetMode(u8 mode)
{
	switch(mode)
	{
		case 0://一般模式M1=0,M0=0
			GPIO_ResetBits(GPIOC,GPIO_Pin_9);
		  GPIO_ResetBits(GPIOA,GPIO_Pin_11);
			break;
		case 1://唤醒模式M1=0,M0=1
			GPIO_ResetBits(GPIOC,GPIO_Pin_9);
		  GPIO_SetBits(GPIOA,GPIO_Pin_11);
			break;
		case 2://省电模式M1=1,M0=0
			GPIO_SetBits(GPIOC,GPIO_Pin_9);
		  GPIO_ResetBits(GPIOA,GPIO_Pin_11);
			break;
		case 3://休眠模式M1=1,M0=1
			GPIO_SetBits(GPIOC,GPIO_Pin_9);
		  GPIO_SetBits(GPIOA,GPIO_Pin_11);
			break;
		default:
			break;
	}
}
void WirelessSend(u8 data)
{
	USART_SendData(UART4, data);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET)
  {}
}

