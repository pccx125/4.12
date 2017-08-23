#include"main.h"


u8 BUF[8];
u8 ge,shi,bai,qian,wan;           //��ʾ����
short  dis_data;                       //����
GPIO_InitTypeDef GPIO_InitStructure;
extern u32 ZTtime;
u8 FilterI=0;
//*********************************************************
void conversion(u16 temp_data)  
{  
    wan=temp_data/10000+0x30 ;
    temp_data=temp_data%10000;   //ȡ������
	  qian=temp_data/1000+0x30 ;
    temp_data=temp_data%1000;    //ȡ������
    bai=temp_data/100+0x30   ;
    temp_data=temp_data%100;     //ȡ������
    shi=temp_data/10+0x30    ;
    temp_data=temp_data%10;      //ȡ������
    ge=temp_data+0x30; 	
}
void ADXL345_GPIO_Cofig(void)
{	
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG,ENABLE);
	//SCL
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	//SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOG, &GPIO_InitStructure); 
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);
	//INT1  INT2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource11);//�����ж���
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;       
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
}
void SDA_IN(void)
{
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
}
void SDA_OUT(void) 
{
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
}
/**************************************
��ʼ�ź�
**************************************/
void ADXL345_Start()
{
	  SDA_OUT();
    Set_SDA_High;                    //����������
    Set_SCL_High;                    //����ʱ����
    Delay_us(20);                 //��ʱ
    Set_SDA_Low;                    //�����½���
    Delay_us(20);                //��ʱ
    Set_SCL_Low;                    //����ʱ����
}
/**************************************
ֹͣ�ź�
**************************************/
void ADXL345_Stop()
{
	  SDA_OUT();
    Set_SDA_Low;                    //����������
    Set_SCL_High;                     //����ʱ����
    Delay_us(20);                 //��ʱ
    Set_SDA_High;                    //����������
    Delay_us(20);                  //��ʱ
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	Set_SDA_High;Delay_us(5);	   
	Set_SCL_High;Delay_us(5);	 
	while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			ADXL345_Stop();
			return 1;
		}
	}
	Set_SCL_Low;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	Set_SCL_Low;
	SDA_OUT();
	Set_SDA_Low; 
	Delay_us(5);
	Set_SCL_High;
	Delay_us(5);
	Set_SCL_Low;
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	Set_SCL_Low;
	SDA_OUT();
	Set_SDA_High;
	Delay_us(5);
	Set_SCL_High;
	Delay_us(5);
	Set_SCL_Low;
}	

/**************************************
��IIC���߷���һ���ֽ�����
**************************************/
void ADXL345_SendByte(u8 dat)
{
    u8 i;
	  SDA_OUT();
    Set_SCL_Low;                   //����ʱ����
    for (i=0; i<8; i++)         //8λ������
    {
			if(dat&0x80)
				Set_SDA_High;
			else
				Set_SDA_Low;
			dat<<=1;
			Delay_us(10);
			Set_SCL_High;                //����ʱ����
			Delay_us(10);              //��ʱ
			Set_SCL_Low;              //����ʱ����
			Delay_us(10);             //��ʱ
    }
}

/**************************************
��IIC���߽���һ���ֽ�����
**************************************/
u8 ADXL345_RecvByte(u8 ack)
{
    u8 i;
    u8 dat = 0;
	  SDA_IN();
    
    for (i=0; i<8; i++)         //8λ������
    {
			Set_SCL_Low;
	    Delay_us(10);
			Set_SCL_High;                //����ʱ����
      Delay_us(10); 
			dat <<= 1; 
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8))
				dat|=0x01;				      
      Delay_us(10);			
    }
		if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK  
    return dat;
}

//******���ֽ�д��*******************************************

void Single_Write_ADXL345(u8 REG_Address,u8 REG_data)
{
    ADXL345_Start();                  //��ʼ�ź�
    ADXL345_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ����ο�����pdf22ҳ 
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_data);       //�ڲ��Ĵ������ݣ���ο�����pdf22ҳ 
	  IIC_Wait_Ack();	
    ADXL345_Stop();                   //����ֹͣ�ź�
}

//********���ֽڶ�ȡ*****************************************
u8 Single_Read_ADXL345(u8 REG_Address)
{   
	  u8 REG_data;
    ADXL345_Start();                          //��ʼ�ź�
    ADXL345_SendByte(SlaveAddress);           //�����豸��ַ+д�ź�
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_Address);            //���ʹ洢��Ԫ��ַ����0��ʼ	
	  IIC_Wait_Ack();	
    ADXL345_Start();                          //��ʼ�ź�
    ADXL345_SendByte(SlaveAddress+1);         //�����豸��ַ+���ź�
	  IIC_Wait_Ack();	
    REG_data=ADXL345_RecvByte(0);              //�����Ĵ�������   
	  ADXL345_Stop();                           //ֹͣ�ź�
    return REG_data; 
}
//*********************************************************
//
//��������ADXL345�ڲ����ٶ����ݣ���ַ��Χ0x32~0x37
//
//*********************************************************
void Multiple_Read_ADXL345(void)
{   u8 i;
    ADXL345_Start();                          //��ʼ�ź�
    ADXL345_SendByte(SlaveAddress);           //�����豸��ַ+д�ź�
	  IIC_Wait_Ack();	
    ADXL345_SendByte(0x32);                   //���ʹ洢��Ԫ��ַ����0x32��ʼ	
	  IIC_Wait_Ack();	
	
    ADXL345_Start();                          //��ʼ�ź�
    ADXL345_SendByte(SlaveAddress+1);         //�����豸��ַ+���ź�
	  IIC_Wait_Ack();	
	  for (i=0; i<6; i++)                      //������ȡ6����ַ���ݣ��洢��BUF
    {
        if(i==5)
					BUF[i] = ADXL345_RecvByte(0);          //BUF[0]�洢0x32��ַ�е�����
			  else
					BUF[i] = ADXL345_RecvByte(1);
    }
    ADXL345_Stop();                          //ֹͣ�ź�
    Delay_ms(5); ;
}


//*****************************************************************

//��ʼ��ADXL345��������Ҫ��ο�pdf�����޸�************************
void Init_ADXL345(void)
{
	 ADXL345_GPIO_Cofig();
	 Delay_ms(2);
   Single_Write_ADXL345(0x31,0x0B);   //������Χ,����16g��13λģʽ
   Single_Write_ADXL345(0x2C,0x0A);   //�����趨Ϊ50Hz 
   Single_Write_ADXL345(0x2D,0x08);   //ѡ���Դģʽ   
   Single_Write_ADXL345(0x2E,0x00);   //ʹ�� DATA_READY �ж�
   Single_Write_ADXL345(0x1E,0x00);   //X ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   Single_Write_ADXL345(0x1F,0x00);   //Y ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   Single_Write_ADXL345(0x20,0x05);   //Z ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
}



//��ȡ3���������
//x,y,z:��ȡ��������
void ADXL345_RD_XYZ(short *x,short *y,short *z)
{
	u8 buf[6];
	u8 i;
	ADXL345_Start();   				 
	ADXL345_SendByte(SlaveAddress);	//����д����ָ��	 
	IIC_Wait_Ack();	   
  ADXL345_SendByte(0x32);   		//���ͼĴ�����ַ(���ݻ������ʼ��ַΪ0X32)
	IIC_Wait_Ack(); 	 										  		   
 
 	ADXL345_Start();  	 	   		//��������
	ADXL345_SendByte(SlaveAddress+1);	//���Ͷ�����ָ��
	IIC_Wait_Ack();
	for(i=0;i<6;i++)
	{
		if(i==5)buf[i]=ADXL345_RecvByte(0);//��ȡһ���ֽ�,�������ٶ�,����NACK  
		else buf[i]=ADXL345_RecvByte(1);	//��ȡһ���ֽ�,������,����ACK 
 	}	        	   
   ADXL345_Stop();					//����һ��ֹͣ����
	*x=(short)(((u16)buf[1]<<8)+buf[0]); 	    
	*y=(short)(((u16)buf[3]<<8)+buf[2]); 	    
	*z=(short)(((u16)buf[5]<<8)+buf[4]); 	   
}
//��ȡADXL��ƽ��ֵ
//x,y,z:��ȡ10�κ�ȡƽ��ֵ
void ADXL345_RD_Avval(short *x,short *y,short *z)
{
	short tx=0,ty=0,tz=0;	   
	u8 i;  
	for(i=0;i<10;i++)
	{
		ADXL345_RD_XYZ(x,y,z);
// 		Delay_ms(10);
		tx+=(short)*x;
		ty+=(short)*y;
		tz+=(short)*z;	   
	}
	*x=tx/10;
	*y=ty/10;
	*z=tz/10;
} 
void ADXL345_RD_XYZ2(u8 *buf)
{
	u8 i;
	ADXL345_Start();   				 
	ADXL345_SendByte(SlaveAddress);	//����д����ָ��	 
	IIC_Wait_Ack();	   
  ADXL345_SendByte(0x32);   		//���ͼĴ�����ַ(���ݻ������ʼ��ַΪ0X32)
	IIC_Wait_Ack(); 	 										  		   
 
 	ADXL345_Start();  	 	   		//��������
	ADXL345_SendByte(SlaveAddress+1);	//���Ͷ�����ָ��
	IIC_Wait_Ack();
	for(i=0;i<6;i++)
	{
		if(i==5)*buf=ADXL345_RecvByte(0);//��ȡһ���ֽ�,�������ٶ�,����NACK  
		else *buf++=ADXL345_RecvByte(1);	//��ȡһ���ֽ�,������,����ACK 
 	}	        	   
   ADXL345_Stop();					//����һ��ֹͣ����
		   
}
void ADXL345_RD_Avval2(short *x,short *y,short *z,u8 *buffer)//�������������
{
	short tx=0,ty=0,tz=0;	   
	u8 i,j;  
	u8 buf[6];
	for(i=0;i<5;i++)
	{
		ADXL345_RD_XYZ2(buf);
		*x=(short)(((u16)buf[1]<<8)+buf[0]); 	    
		*y=(short)(((u16)buf[3]<<8)+buf[2]); 	    
		*z=(short)(((u16)buf[5]<<8)+buf[4]); 
		for(j=0;j<6;j++)
			*buffer++=buf[j];
		*buffer++=(u8)((ZTtime>>24)&0x000000FF);
		*buffer++=(u8)((ZTtime>>16)&0x000000FF);
		*buffer++=(u8)((ZTtime>>8)&0x000000FF);
		*buffer++=(u8)(ZTtime&0x000000FF);
		tx+=(short)*x;
		ty+=(short)*y;
		tz+=(short)*z;	   
	}
	*x=tx/5;
	*y=ty/5;
	*z=tz/5;
} 
// void ADXL345_RD_TD(short *x,short *y,short *z,u8 *buffer)//ֻȥ������
// {   
// 	u8 i,t;  
// 	short tx=0,ty=0,tz=0;	
// 	u8 buf[6];
// 	short tempbuf[60];
// 	short a,b;
// 	u32 time[20];
// 	for(i=0,t=0;i<20;i++,t+=3)
// 	{
// 		ADXL345_RD_XYZ2(buf);
// 		tempbuf[t]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
// 		tempbuf[t+1]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
// 		tempbuf[t+2]=(short)(((u16)buf[5]<<8)+buf[4]);   
// 		time[i]=ZTtime;
// 		tx+=tempbuf[t];
// 		ty+=tempbuf[t+1];
// 		tz+=tempbuf[t+2];
// 	}
// 	for(i=3;i<57;i++)//X
// 	{
// 		a=tempbuf[i]-tempbuf[i-3];
// 		b=tempbuf[i+3]-tempbuf[i];
// 		if((a*b<0.00001)&&(fabs(a)>40)&&(fabs(b)>40))
// 			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
// 	}
// 	for(i=4;i<58;i++)//Y
// 	{
// 		a=tempbuf[i]-tempbuf[i-3];
// 		b=tempbuf[i+3]-tempbuf[i];
// 		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
// 			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
// 	}
// 	for(i=5;i<59;i++)//Z
// 	{
// 		a=tempbuf[i]-tempbuf[i-3];
// 		b=tempbuf[i+3]-tempbuf[i];
// 		if((a*b<0.00001)&&(fabs(a)>40)&&(fabs(b)>40))
// 			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
// 	}
// 	for(i=0,t=0;i<20;i++,t+=3)
// 	{
// 		*buffer++=(u8)(tempbuf[t]&0x00FF);
// 		*buffer++=(u8)((tempbuf[t]>>8)&0x00FF);
// 		*buffer++=(u8)(tempbuf[t+1]&0x00FF);
// 		*buffer++=(u8)((tempbuf[t+1]>>8)&0x00FF);
// 		*buffer++=(u8)(tempbuf[t+2]&0x00FF);
// 		*buffer++=(u8)((tempbuf[t+2]>>8)&0x00FF);
// 		*buffer++=(u8)((time[i]>>24)&0x000000FF);
// 		*buffer++=(u8)((time[i]>>16)&0x000000FF);
// 		*buffer++=(u8)((time[i]>>8)&0x000000FF);
// 		*buffer++=(u8)(time[i]&0x000000FF);		
// 	}	
// 	*x=tx/20;
// 	*y=ty/20;
// 	*z=tz/20;
// } 
void ADXL345_RD(short *tempbuf)//ȥ������
{   
	u8 i,t;  
	u8 buf[6];
//  	short tempbuf[30];
	short a,b;
	
	for(i=0,t=0;i<10;i++,t+=3)
	{
		ADXL345_RD_XYZ2(buf);
		tempbuf[t]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
		tempbuf[t+1]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
		tempbuf[t+2]=(short)(((u16)buf[5]<<8)+buf[4]);   
	}
	for(i=3;i<27;i+=3)
	{
		a=tempbuf[i]-tempbuf[i-3];
		b=tempbuf[i+3]-tempbuf[i];
		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
	}
	IWDG_Feed();    //ι��
	for(i=4;i<28;i+=3)
	{
		a=tempbuf[i]-tempbuf[i-3];
		b=tempbuf[i+3]-tempbuf[i];
		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
	}
	for(i=5;i<29;i+=3)
	{
		a=tempbuf[i]-tempbuf[i-3];
		b=tempbuf[i+3]-tempbuf[i];
		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
			tempbuf[i]=(tempbuf[i-3]+tempbuf[i+3])/2;
	}
// 	GlideFilter(tempbuf,30,filterbuff,buffer); 
} 
// void GlideFilter(short *value_buf,int n,short *filterbuff,u8 *buffer)
// {
// 	u8 i,x=27,y=28,z=29;
// 	short tx=0,ty=0,tz=0;	
// 	u8 buf[6];
// 	u8 count;
// 	short a,b;
// 	for(i=0;i<2;i++)
// 	{
// 		ADXL345_RD_XYZ2(buf);
// 		value_buf[FilterI++]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
// 		value_buf[FilterI++]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
// 		value_buf[FilterI++]=(short)(((u16)buf[5]<<8)+buf[4]); 
// 		if(FilterI==n)
// 			FilterI=0; //�Ƚ��ȳ�������ƽ��ֵ
// 		
// 		a=value_buf[x%30]-value_buf[(x-3)%30];
// 		b=value_buf[(x+3)%30]-value_buf[x%30];
// 		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
// 			value_buf[x%30]=(value_buf[(x-3)%30]+value_buf[(x+3)%30])/2;
// 		
// 		a=value_buf[y%30]-value_buf[(y-3)%30];
// 		b=value_buf[(y+3)%30]-value_buf[y%30];
// 		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
// 			value_buf[y%30]=(value_buf[(y-3)%30]+value_buf[(y+3)%30])/2;
// 		
// 		a=value_buf[z%30]-value_buf[(z-3)%30];
// 		b=value_buf[(z+3)%30]-value_buf[z%30];
// 		if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
// 			value_buf[z%30]=(value_buf[(z-3)%30]+value_buf[(z+3)%30])/2;
// 		
// 		for(count=0;count<n;)
// 		{
// 			tx+=value_buf[count++];
// 			ty+=value_buf[count++];
// 			tz+=value_buf[count++];
// 		}
// 		tx=tx/10;
// 		ty=ty/10;
// 		tz=tz/10;
// 		*buffer++=(u8)(tx&0x00FF);
// 		*buffer++=(u8)((tx>>8)&0x00FF);
// 		*buffer++=(u8)(ty&0x00FF);
// 		*buffer++=(u8)((ty>>8)&0x00FF);
// 		*buffer++=(u8)(tz&0x00FF);
// 		*buffer++=(u8)((tz>>8)&0x00FF);
// 		*buffer++=(u8)((ZTtime>>24)&0x000000FF);
// 		*buffer++=(u8)((ZTtime>>16)&0x000000FF);
// 		*buffer++=(u8)((ZTtime>>8)&0x000000FF);
// 		*buffer++=(u8)(ZTtime&0x000000FF);
// 		x=x+3;
// 		y=y+3;
// 		z=z+3;
// 	}
// 	filterbuff[0]=tx;
// 	filterbuff[1]=ty;
// 	filterbuff[2]=tz;
// }
//��ȡADXL345������times��,��ȡƽ��
//x,y,z:����������
//times:��ȡ���ٴ�
void ADXL345_Read_Average(short *x,short *y,short *z,u8 times)
{
	u8 i;
	short tx,ty,tz;
	*x=0;
	*y=0;
	*z=0;
	if(times)//��ȡ������Ϊ0
	{
		for(i=0;i<times;i++)//������ȡtimes��
		{
			ADXL345_RD_XYZ(&tx,&ty,&tz);
			*x+=tx;
			*y+=ty;
			*z+=tz;
			Delay_ms(1);
		}
		*x/=times;
		*y/=times;
		*z/=times;
	}
}
//�õ��Ƕ�
//x,y,z:x,y,z������������ٶȷ���(����Ҫ��λ,ֱ����ֵ����)
//dir:Ҫ��õĽǶ�.0,��Z��ĽǶ�;1,��X��ĽǶ�;2,��Y��ĽǶ�.
//����ֵ:�Ƕ�ֵ.��λ0.1��.
short ADXL345_Get_Angle(float x,float y,float z,u8 dir)
{
	float temp;
 	float res=0;
	switch(dir)
	{
		case 0://����ȻZ��ĽǶ�
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://����ȻX��ĽǶ�
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://����ȻY��ĽǶ�
 			temp=y/sqrt((x*x+z*z));
 			res=atan(temp);
 			break;
 	}
	if(res<0.0000001)
		res=-res;
	return res*1800/3.14;
}
u8 ADXL345_Get_Angle1(float x,float y,float z,u8 dir)
{
	float temp;
 	float res=0;
	short angle=0;
	switch(dir)
	{
		case 0://����ȻZ��ĽǶ�
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://����ȻX��ĽǶ�
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://����ȻY��ĽǶ�
 			temp=y/sqrt((x*x+z*z));
 			res=atan(temp);
 			break;
 	}

	if(res<0.0000001)
	{
		res=-res;
		angle=res*180/3.14;
		angle|=0x80;
	}
	else
		angle=res*180/3.14;
	return angle;
}
void ADXL345_Angle(u8 *buff)
{
	float temp;
 	float res=0;
	short angle=0;
	short x,y,z; 
	IWDG_Feed();  //ι��
	ADXL345_Read_Average(&x,&y,&z,10);
	temp=x/sqrt((y*y+z*z));//����ȻX��ĽǶ�
 	res=atan(temp);
	if(res<0.0000001)
	{
		res=-res;
		angle=res*1800/3.14;
		buff[0]=angle/10;
		buff[0]|=0x80;
	}
	else
	{
		angle=res*1800/3.14;
		buff[0]=angle/10;
	}
	buff[1]=angle%10;
	
	temp=y/sqrt((x*x+z*z));//����ȻY��ĽǶ�
 	res=atan(temp);
	if(res<0.0000001)
	{
		res=-res;
		angle=res*1800/3.14;
		buff[2]=angle/10;
		buff[2]|=0x80;
	}
	else
	{
		angle=res*1800/3.14;
		buff[2]=angle/10;
	}
	buff[3]=angle%10;
	
	temp=sqrt((x*x+y*y))/z;//����ȻZ��ĽǶ�
 	res=atan(temp);
	if(res<0.0000001)
	{
		res=-res;
		angle=res*1800/3.14;
		buff[4]=angle/10;
		buff[4]|=0x80;
	}
	else
	{
		angle=res*1800/3.14;
		buff[4]=angle/10;
	}
	buff[5]=angle%10;
}

