#include"main.h"


u8 BUF[8];
u8 ge,shi,bai,qian,wan;           //显示变量
short  dis_data;                       //变量
GPIO_InitTypeDef GPIO_InitStructure;
extern u32 ZTtime;
u8 FilterI=0;
//*********************************************************
void conversion(u16 temp_data)  
{  
    wan=temp_data/10000+0x30 ;
    temp_data=temp_data%10000;   //取余运算
	  qian=temp_data/1000+0x30 ;
    temp_data=temp_data%1000;    //取余运算
    bai=temp_data/100+0x30   ;
    temp_data=temp_data%100;     //取余运算
    shi=temp_data/10+0x30    ;
    temp_data=temp_data%10;      //取余运算
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
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource11);//配置中断线
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
起始信号
**************************************/
void ADXL345_Start()
{
	  SDA_OUT();
    Set_SDA_High;                    //拉高数据线
    Set_SCL_High;                    //拉高时钟线
    Delay_us(20);                 //延时
    Set_SDA_Low;                    //产生下降沿
    Delay_us(20);                //延时
    Set_SCL_Low;                    //拉低时钟线
}
/**************************************
停止信号
**************************************/
void ADXL345_Stop()
{
	  SDA_OUT();
    Set_SDA_Low;                    //拉低数据线
    Set_SCL_High;                     //拉高时钟线
    Delay_us(20);                 //延时
    Set_SDA_High;                    //产生上升沿
    Delay_us(20);                  //延时
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
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
	Set_SCL_Low;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
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
//不产生ACK应答		    
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
向IIC总线发送一个字节数据
**************************************/
void ADXL345_SendByte(u8 dat)
{
    u8 i;
	  SDA_OUT();
    Set_SCL_Low;                   //拉低时钟线
    for (i=0; i<8; i++)         //8位计数器
    {
			if(dat&0x80)
				Set_SDA_High;
			else
				Set_SDA_Low;
			dat<<=1;
			Delay_us(10);
			Set_SCL_High;                //拉高时钟线
			Delay_us(10);              //延时
			Set_SCL_Low;              //拉低时钟线
			Delay_us(10);             //延时
    }
}

/**************************************
从IIC总线接收一个字节数据
**************************************/
u8 ADXL345_RecvByte(u8 ack)
{
    u8 i;
    u8 dat = 0;
	  SDA_IN();
    
    for (i=0; i<8; i++)         //8位计数器
    {
			Set_SCL_Low;
	    Delay_us(10);
			Set_SCL_High;                //拉高时钟线
      Delay_us(10); 
			dat <<= 1; 
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8))
				dat|=0x01;				      
      Delay_us(10);			
    }
		if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK  
    return dat;
}

//******单字节写入*******************************************

void Single_Write_ADXL345(u8 REG_Address,u8 REG_data)
{
    ADXL345_Start();                  //起始信号
    ADXL345_SendByte(SlaveAddress);   //发送设备地址+写信号
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf22页 
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf22页 
	  IIC_Wait_Ack();	
    ADXL345_Stop();                   //发送停止信号
}

//********单字节读取*****************************************
u8 Single_Read_ADXL345(u8 REG_Address)
{   
	  u8 REG_data;
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress);           //发送设备地址+写信号
	  IIC_Wait_Ack();	
    ADXL345_SendByte(REG_Address);            //发送存储单元地址，从0开始	
	  IIC_Wait_Ack();	
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	  IIC_Wait_Ack();	
    REG_data=ADXL345_RecvByte(0);              //读出寄存器数据   
	  ADXL345_Stop();                           //停止信号
    return REG_data; 
}
//*********************************************************
//
//连续读出ADXL345内部加速度数据，地址范围0x32~0x37
//
//*********************************************************
void Multiple_Read_ADXL345(void)
{   u8 i;
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress);           //发送设备地址+写信号
	  IIC_Wait_Ack();	
    ADXL345_SendByte(0x32);                   //发送存储单元地址，从0x32开始	
	  IIC_Wait_Ack();	
	
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	  IIC_Wait_Ack();	
	  for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
    {
        if(i==5)
					BUF[i] = ADXL345_RecvByte(0);          //BUF[0]存储0x32地址中的数据
			  else
					BUF[i] = ADXL345_RecvByte(1);
    }
    ADXL345_Stop();                          //停止信号
    Delay_ms(5); ;
}


//*****************************************************************

//初始化ADXL345，根据需要请参考pdf进行修改************************
void Init_ADXL345(void)
{
	 ADXL345_GPIO_Cofig();
	 Delay_ms(2);
   Single_Write_ADXL345(0x31,0x0B);   //测量范围,正负16g，13位模式
   Single_Write_ADXL345(0x2C,0x0A);   //速率设定为50Hz 
   Single_Write_ADXL345(0x2D,0x08);   //选择电源模式   
   Single_Write_ADXL345(0x2E,0x00);   //使能 DATA_READY 中断
   Single_Write_ADXL345(0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
   Single_Write_ADXL345(0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页
}



//读取3个轴的数据
//x,y,z:读取到的数据
void ADXL345_RD_XYZ(short *x,short *y,short *z)
{
	u8 buf[6];
	u8 i;
	ADXL345_Start();   				 
	ADXL345_SendByte(SlaveAddress);	//发送写器件指令	 
	IIC_Wait_Ack();	   
  ADXL345_SendByte(0x32);   		//发送寄存器地址(数据缓存的起始地址为0X32)
	IIC_Wait_Ack(); 	 										  		   
 
 	ADXL345_Start();  	 	   		//重新启动
	ADXL345_SendByte(SlaveAddress+1);	//发送读器件指令
	IIC_Wait_Ack();
	for(i=0;i<6;i++)
	{
		if(i==5)buf[i]=ADXL345_RecvByte(0);//读取一个字节,不继续再读,发送NACK  
		else buf[i]=ADXL345_RecvByte(1);	//读取一个字节,继续读,发送ACK 
 	}	        	   
   ADXL345_Stop();					//产生一个停止条件
	*x=(short)(((u16)buf[1]<<8)+buf[0]); 	    
	*y=(short)(((u16)buf[3]<<8)+buf[2]); 	    
	*z=(short)(((u16)buf[5]<<8)+buf[4]); 	   
}
//读取ADXL的平均值
//x,y,z:读取10次后取平均值
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
	ADXL345_SendByte(SlaveAddress);	//发送写器件指令	 
	IIC_Wait_Ack();	   
  ADXL345_SendByte(0x32);   		//发送寄存器地址(数据缓存的起始地址为0X32)
	IIC_Wait_Ack(); 	 										  		   
 
 	ADXL345_Start();  	 	   		//重新启动
	ADXL345_SendByte(SlaveAddress+1);	//发送读器件指令
	IIC_Wait_Ack();
	for(i=0;i<6;i++)
	{
		if(i==5)*buf=ADXL345_RecvByte(0);//读取一个字节,不继续再读,发送NACK  
		else *buf++=ADXL345_RecvByte(1);	//读取一个字节,继续读,发送ACK 
 	}	        	   
   ADXL345_Stop();					//产生一个停止条件
		   
}
void ADXL345_RD_Avval2(short *x,short *y,short *z,u8 *buffer)//不做处理的数据
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
// void ADXL345_RD_TD(short *x,short *y,short *z,u8 *buffer)//只去除跳点
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
void ADXL345_RD(short *tempbuf)//去除跳点
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
	IWDG_Feed();    //喂狗
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
// 			FilterI=0; //先进先出，再求平均值
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
//读取ADXL345的数据times次,再取平均
//x,y,z:读到的数据
//times:读取多少次
void ADXL345_Read_Average(short *x,short *y,short *z,u8 times)
{
	u8 i;
	short tx,ty,tz;
	*x=0;
	*y=0;
	*z=0;
	if(times)//读取次数不为0
	{
		for(i=0;i<times;i++)//连续读取times次
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
//得到角度
//x,y,z:x,y,z方向的重力加速度分量(不需要单位,直接数值即可)
//dir:要获得的角度.0,与Z轴的角度;1,与X轴的角度;2,与Y轴的角度.
//返回值:角度值.单位0.1°.
short ADXL345_Get_Angle(float x,float y,float z,u8 dir)
{
	float temp;
 	float res=0;
	switch(dir)
	{
		case 0://与自然Z轴的角度
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://与自然X轴的角度
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://与自然Y轴的角度
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
		case 0://与自然Z轴的角度
 			temp=sqrt((x*x+y*y))/z;
 			res=atan(temp);
 			break;
		case 1://与自然X轴的角度
 			temp=x/sqrt((y*y+z*z));
 			res=atan(temp);
 			break;
 		case 2://与自然Y轴的角度
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
	IWDG_Feed();  //喂狗
	ADXL345_Read_Average(&x,&y,&z,10);
	temp=x/sqrt((y*y+z*z));//与自然X轴的角度
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
	
	temp=y/sqrt((x*x+z*z));//与自然Y轴的角度
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
	
	temp=sqrt((x*x+y*y))/z;//与自然Z轴的角度
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

