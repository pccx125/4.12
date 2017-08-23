#include "main.h"

#define SLAVE_ADDRESS 0xD0
#define I2C1_ADDRESS7    0xA2
#define I2C_Speed              100000  //频率原来为400K，全速调试不成功，后改为300K

						
static void I2C_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 

	/* 使能与 I2C1 有关的时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);  
    
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);  
  
  /*!< Configure sEE_I2C pins: SCL */   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
static void I2C_Mode_Config(void)
{
  I2C_InitTypeDef  I2C_InitStructure; 

  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = I2C1_ADDRESS7;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;
  
  /* sEE_I2C Peripheral Enable */
  I2C_Cmd(I2C1, ENABLE);
  /* Apply sEE_I2C configuration after enabling it */
  I2C_Init(I2C1, &I2C_InitStructure);

	/*允许1字节1应答模式*/
	I2C_AcknowledgeConfig(I2C1, ENABLE);    
}
void I2C_M41T81_Config(void)
{
	I2C_GPIO_Config();
	I2C_Mode_Config();
}
//-------------------------------
// Read RTC (all real time)
//-------------------------------
void ReadRTC(unsigned char * buff,unsigned char addr,unsigned char NumByteToRead)
{
	u8 cnt=0;
	/* Send START condition */
	I2C_GenerateSTART(I2C1, ENABLE);
	
	/* Test on EV5 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(cnt<100))
	{
		Delay_us(5);
		cnt++;
	}
  cnt=0;
	/* Send  address for write */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);
	
	/* Test on EV6 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(cnt<100))
  {
		Delay_us(5);
		cnt++;
	}
  cnt=0;		

  /* Clear EV6 by setting again the PE bit */
  I2C_Cmd(I2C1, ENABLE);	
	
	/* Send the internal address to write to */    
  I2C_SendData(I2C1, addr);
	
	/* Test on EV8 and clear it */
  while((! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(cnt<100))
	{
		Delay_us(5);
		cnt++;
	}
  cnt=0;

  /* Send STRAT condition a second time */  
  I2C_GenerateSTART(I2C1, ENABLE);
	
	/* Test on EV5 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(cnt<100))
  {
		Delay_us(5);
		cnt++;
	}
  cnt=0;		

  /* Send EEPROM address for read */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS+1, I2C_Direction_Receiver);/////////0xD1读模式
	/* Test on EV6 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))&&(cnt<100))
	{
		Delay_us(5);
		cnt++;
	}
  cnt=0;

	/* While there is data to be read */
  while(NumByteToRead)  
  {
    if(NumByteToRead == 1)
    {
      /* Disable Acknowledgement */
      I2C_AcknowledgeConfig(I2C1, DISABLE);
      
      /* Send STOP Condition */
      I2C_GenerateSTOP(I2C1, ENABLE);
    }

    /* Test on EV7 and clear it */
    if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
    {      
      /* Read a byte from the EEPROM */
      *buff = I2C_ReceiveData(I2C1);

      /* Point to the next location where the byte read will be saved */
      buff++; 
      
      /* Decrement the read bytes counter */
      NumByteToRead--;        
    }   
  }

  /* Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig(I2C1, ENABLE);	
}

//-------------------------------
// Write RTC
//-------------------------------
void WriteRTC(unsigned char *buff,unsigned char addr,unsigned char NumByteToWrite)
{
	u8 cnt=0;
  /* Send START condition */
  I2C_GenerateSTART(I2C1, ENABLE);
  
  /* Test on EV5 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(cnt<100))
  {
		Delay_us(5);
		cnt++;
	}
  cnt=0;		
  
  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);
  
  /* Test on EV6 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))&&(cnt<100))
  {
		Delay_us(5);
		cnt++;
	}
  cnt=0;		

  /* Send the EEPROM's internal address to write to */    
  I2C_SendData(I2C1, addr); 

	/* Test on EV8 and clear it */
  while((! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(cnt<100))
	{
		Delay_us(5);
		cnt++;
	}
  cnt=0;
  /* While there is data to be written */
  while(NumByteToWrite--)  
  {
    /* Send the current byte */
    I2C_SendData(I2C1, *buff); 

    /* Point to the next byte to be written */
    buff++; 
  
    /* Test on EV8 and clear it */
    while ((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))&&(cnt<300))
		{
			Delay_us(2);
			cnt++;
		}
		cnt=0;
  }

  /* Send STOP condition */
  I2C_GenerateSTOP(I2C1, ENABLE);
}

unsigned char ReadByte(unsigned char addr)
{
	u8 cnt=0;
	unsigned char Data;
	/* Send START condition */
	I2C_GenerateSTART(I2C1, ENABLE);
	
	/* Test on EV5 and clear it */
  while((!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))&&(cnt<100))
	{
		Delay_us(5);
		cnt++;
	}		
	cnt=0;
	/* Send  address for write */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);
	
	/* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
	
  /* Clear EV6 by setting again the PE bit */
  I2C_Cmd(I2C1, ENABLE);	
	
	/* Send the internal address to write to */    
  I2C_SendData(I2C1, addr);
	
	/* Test on EV8 and clear it */
  while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
  /* Send STRAT condition a second time */  
  I2C_GenerateSTART(I2C1, ENABLE);
	
	/* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  
	
  /* Send EEPROM address for read */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Receiver);
	
	/* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  /* Disable Acknowledgement */
  I2C_AcknowledgeConfig(I2C1, DISABLE);
	if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) 
	{
		 Data = I2C_ReceiveData(I2C1);
	}

  /* Send STOP Condition */
  I2C_GenerateSTOP(I2C1, ENABLE);
  /* Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig(I2C1, ENABLE);	
	return(Data); 
}

//-------------------------------
// Write RTC a bit
//-------------------------------
void WriteByte(unsigned char Data,unsigned char addr)
{
  /* Send START condition */
  I2C_GenerateSTART(I2C1, ENABLE);
  
  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)); 
  
  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Transmitter);
  
  /* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));  

  /* Send the EEPROM's internal address to write to */    
  I2C_SendData(I2C1, addr); 

	/* Test on EV8 and clear it */
  while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  /* Send the current byte */
  I2C_SendData(I2C1, Data); 
  /* Test on EV8 and clear it */
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  /* Send STOP condition */
  I2C_GenerateSTOP(I2C1, ENABLE);
}

//-------------------------------
// Convert BCD 1 byte to HEX 1 byte
//-------------------------------
unsigned char BCD2HEX(unsigned char bcd)	 //把int型转成char型（bcd码转二进制码）
{
	unsigned char temp;
	temp=((bcd>>4)*10)+(bcd&0x0f);
	return temp;
	
}
unsigned char HEX2BCD(unsigned char hex)	 //把char型转成int型（二进制码转bcd码）
{
	unsigned char temp;
	temp=((hex/10)<<4)|(hex%10);
	return temp;
	
}



