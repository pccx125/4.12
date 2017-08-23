#include "spi1.h"
#include "stm32f2xx.h"

/*SD����SPI������غ���*/

//��������ӿ�SPI�ĳ�ʼ����SPI���ó���ģʽ							  

void SPI1_Init(void)
{	 
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable SPI2 and GPIO clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  /*!< SPI Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
      
  /*!< Configure  pins: SCK */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure  pins: MISO */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure  pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* SPI1 configuration *///��ʼ��SPI1�ṹ��
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI1����Ϊ����ȫ˫��
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //����SPI1Ϊ��ģʽ
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //SPI���ͽ���8λ֡�ṹ
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //����ʱ���ڲ�����ʱ��ʱ��Ϊ�͵�ƽ
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //��һ��ʱ���ؿ�ʼ��������
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS�ź��������ʹ��SSIλ������
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //SPI������Ԥ��ƵֵΪ8
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //���ݴ����MSBλ��ʼ
  SPI_InitStructure.SPI_CRCPolynomial = 7; //CRCֵ����Ķ���ʽ

	SPI_Init(SPI1, &SPI_InitStructure);                                 //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���
	
	/* Enable SPI2  */
	SPI_Cmd(SPI1, ENABLE);                                              //ʹ��SPI1����
	
	SPI1_ReadWriteByte(0xff);                                           //��������		 
}  

u8 SPI1_ReadWriteByte(u8 TxData)                                        //SPI��д���ݺ���
{		
	u8 retry=0;				 	
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)      //���ͻ����־λΪ��
		{
		retry++;
		if(retry>200)return 0;
		}			  
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI1, TxData);                                    //ͨ������SPI1����һ������
	retry=0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);   //���ջ����־λ��Ϊ��
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);                                 //ͨ��SPI1���ؽ�������				    
}

//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
  
void SPI1_SetSpeed(u8 SpeedSet)
{
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI1����Ϊ����ȫ˫��
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //����SPI1Ϊ��ģʽ
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //SPI���ͽ���8λ֡�ṹ
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; //����ʱ���ڲ�����ʱ��ʱ��Ϊ�͵�ƽ
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //��һ��ʱ���ؿ�ʼ��������
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS�ź��������ʹ��SSIλ������
    //����ٶ���������0�������ģʽ����0�����ģʽ
    if(SpeedSet==SPI_SPEED_LOW)
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    }
    else
    {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    }
	//moon.mp3: 4707774 Byte size Ŀ���ļ� ��Ϊbuffer[512]     
	//speed:ʵ��������ݣ�����ٶ� 392314 Byte/S��
	//Prescaler_128, 59592 Byte/S
	//Prescaler_64, 104617 Byte/S
	//Prescaler_32, 168134 Byte/S    162337 Byte/S
	//Prescaler_16, 261543 Byte/S    247777 Byte/S
	//Prescaler_8,  313851 Byte/S    336269 Byte/S
	//Prescaler_4,  392314 Byte/S    392314 Byte/S
	//Prescaler_2,  392314 Byte/S

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
    return;
}




















