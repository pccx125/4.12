#ifndef __ADXL345_H
#define __ADXL345_H


#define DEVICE_ID		0X00 	//Æ÷¼þID,0XE5
#define THRESH_TAP		0X1D   	//ÇÃ»÷·§Öµ
#define OFSX			0X1E
#define OFSY			0X1F
#define OFSZ			0X20
#define DUR				0X21
#define Latent			0X22
#define Window  		0X23 
#define THRESH_ACK		0X24
#define THRESH_INACT	0X25 
#define TIME_INACT		0X26
#define ACT_INACT_CTL	0X27	 
#define THRESH_FF		0X28	
#define TIME_FF			0X29 
#define TAP_AXES		0X2A  
#define ACT_TAP_STATUS  0X2B 
#define BW_RATE			0X2C 
#define POWER_CTL		0X2D 

#define INT_ENABLE		0X2E
#define INT_MAP			0X2F
#define INT_SOURCE  	0X30
#define DATA_FORMAT	    0X31
#define DATA_X0			0X32
#define DATA_X1			0X33
#define DATA_Y0			0X34
#define DATA_Y1			0X35
#define DATA_Z0			0X36
#define DATA_Z1			0X37
#define FIFO_CTL		0X38
#define FIFO_STATUS		0X39





#define	SlaveAddress   0xA6


#define Set_SCL_Low  GPIO_ResetBits(GPIOC,GPIO_Pin_6)
#define Set_SCL_High GPIO_SetBits(GPIOC,GPIO_Pin_6)
#define Set_SDA_Low  GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define Set_SDA_High GPIO_SetBits(GPIOA,GPIO_Pin_8)
void conversion(u16 temp_data);
void Single_Write_ADXL345(u8 REG_Address,u8 REG_data);
u8 Single_Read_ADXL345(u8 REG_Address);
void Multiple_Read_ADXL345(void);
void Init_ADXL345(void);
void ADXL345_RD(short *tempbuf);
void ADXL345_RD_XYZ2(u8 *buf);
void GlideFilter(short *value_buf,int n,short *filterbuff,u8 *buffer);
void ADXL345_RD_XYZ(short *x,short *y,short *z);
void ADXL345_Read_Average(short *x,short *y,short *z,u8 times);
void ADXL345_RD_Avval(short *x,short *y,short *z);
short ADXL345_Get_Angle(float x,float y,float z,u8 dir);
u8 ADXL345_Get_Angle1(float x,float y,float z,u8 dir);
void ADXL345_Angle(u8 *buff);

#endif

