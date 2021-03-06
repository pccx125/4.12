#include "main.h"

// #define FLASH_BASE  ((uint32_t)0x08000000)//iap下载  485下载添加
#define BUFFER_SIZE         512
struct LC_Data lc_data;
struct DP_Data dp_data;
u8 Water_power=0x03;//设置水声上电功率为03
extern u16 log_len;//记录SD卡log数据的条数
FATFS fs;   
u8 ControlBoardID;//信标设备识别码
extern u8 fileztp[25];//记录文件名
u8 FileNum=0;//记录文件的总数
extern u32 ZTtime;//记录姿态数据相对于同步时间的相对时间
extern u8 zttimeflag;//为1表示已经同步，ZTtime开始计数
NAND_ADDRESS WriteReadAddr;//flash地址变量
extern u8 Rxmegflag,Rxwaterflag;//串口接收完成标志
extern u8 sendtowater;//为1表示可以向水声发送数据
u8 tongbu;//同步标志，为1，开始发同步信号
extern u8 FromJiaBanA;//记录甲板单元发过来的命令  为1表示A0，为2表示A1，为3表示A3，为4表示A4，5表示B2，6表示B3
extern u8 RX_uart4flag;//串口4接收完成标志
extern u8 release;//为1表示压载释放
void NVIC_Configuration(void);        //中断配置函数
void GPIO_EXTI_Config(void);           //IO口初始化
u32 waterontime=10,megontime=10;//记录水声和磁测上电时间

u32 wateron=0,megon=0;//计数器
u8 detectfeedback=0,feedcom,detect=0;//用于检测综控板向磁测模块发送数据后，磁测模块是否有反馈。detectfeedback判断标志，feedcom反馈的命令，detect计时变量

extern unsigned char  RTC_Time[5];
extern float jingdu,weidu;//经纬度
extern char s1,s2,s3,s4;
extern u8  Hour,Min,Sec,Day,Month,Year,GPS_VA,weidu_dir,jingdu_dir;//日期信息及有效无效标志位
extern unsigned char  RTC_ARR[8];//实时时钟所用时间信息 
extern u8 flag_rx;//定位数据接收完毕标志位
/*   主函数    */
int main(void)
{	   	
	u8 meg_syn=0;
	FRESULT res;
	UINT bw;
  FIL fsrc;//文件指针	
	u8 detectcount=0;
  unsigned char  RTC_Alarm_ARR[5];
	u8 buffer[10];
	short tempbuf[30];
	short a,b;
	short x,y,z;
	short tx=0,ty=0,tz=0;	
	u8 count=0,statuscount=0;
	u8 ztcnt=0;
	u8 i=30,j=31,k=32;
	u8 Filter=0;
	u8 buf[6];
	u8 ztgroupcnt=0;
	SystemInit();
	SysTick_Configuration();//滴答时钟初始化
	
// 	SCB->VTOR = FLASH_BASE | 0x10000;//IAP下载  485下载添加（改下载到的FLASH地址为8010000  ）
	
  GPIO_EXTI_Config();//IO口，外部中断初始化
	NVIC_Configuration();//中断优先级处理
	IWDG_Init(IWDG_Prescaler_32,0xBB8);//0xBB8=3000  按40KHz约为2.4s 看门狗初始化    32*3000/40K
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  //若产生了复位，清除复位标志位
    RCC_ClearFlag();
// 	GPSSource24V_OUT();//GPS定位上电
	BeepON;//蜂鸣器响
	Delay_ms(1000);
	IWDG_Feed();   //喂狗
	Delay_ms(1000);
	IWDG_Feed();   //喂狗
	BeepOFF;
	MEG_Disable;
// 	GPSSource_Stop();
	Led_GPIO_Config();//led灯初始化
	I2C_M41T81_Config();//实时时钟初始化
  disk_initialize(0);	//文件系统初始化
	DHT11_GPIO_Config();//温湿度传感器初始化
  Init_ADXL345();                 //初始化ADXL345 
	WirelessUsart_Init();//无线模块串口初始化
	WirelessGPIO_Config();//无线模块IO口初始化
	SetMode(0);//设置无线模块为模式0
	ADC_GPIO_Configuration();//ADC初始化
  NAND_Init();/*配置与SRAM连接的FSMC BANK2 NAND*/
  NAND_Reset();//复位一下NandFlash
  RS232_Init();//RS232串口初始化
	RS485_Init();
	IWDG_Feed();    //喂狗
	ReadRTC(&RTC_Alarm_ARR[0],0x0A,5);	//实时时钟内部寄存器HT位置0 ，实时时钟开始计时，从0A开始读取 5个
	RTC_Alarm_ARR[2]&=0xBF;//相与让数组的第三个元素的D6置0   第三个即0C与BF即10111111 相与，让HT为0
	WriteByte(RTC_Alarm_ARR[2],0x0C);   //写入
	res = f_mount(0, &fs);//注册
	ADXL345_Read_Average(&x,&y,&z,5);//获得加速度数据
// 	res=f_mkdir((char *)filename);
//	SetWirelessAddr(0x07);
	LED1( 1 ); 
	Delay_ms(800);     
	IWDG_Feed();    //喂狗
	
	lc_data.LC_AllGroupCnt=0;//漏磁有关的变量初始化
	lc_data.LC_DataByteCnt=0;
	lc_data.lc_fsizebyte=0;
	lc_data.lc_rdgroupcnt=0;
	lc_data.lc_blocknum=1;
	lc_data.lc_threshold[0]=0x02;
	lc_data.lc_threshold[1]=0xCC;
	
	dp_data.DP_AllGroupCnt=0;//甚低频有关的变量初始化
	dp_data.DP_DataByteCnt=0;
	dp_data.dp_fsizebyte=0;
	dp_data.dp_rdgroupcnt=0;
	dp_data.dp_blocknum=1;
	dp_data.dp_threshold[0]=0x02;
	dp_data.dp_threshold[1]=0xCC;
	
// 	ControlBoardID=01;//从flash中获得设备识别码
	
	log_len=Get_Log_len();
	while(1)
	{ 
		
  
		
		ControlBoardID=3;//从flash中获得设备识别码
		if(flag_rx==0)//如果定位数据未收到，则正常执行
		{
					if(wateron<=waterontime)//waterontime指定水声上电时间，单位为秒
						wateron++;
					
					if(wateron==waterontime)//计数器计数到指定的值，给水声上电
					{			
						OutputSourceON;//控制BTS555给水声模块上电
						Delay_ms(1000);
						QueryToWater(0x16);//发送自检命令，使水声模块知道综控的识别码
						IWDG_Feed();   //喂狗
						SetWaterPower();//水声上电后，设备水声发射功率
			// 			GPSSource_Config();
					}
					
					if(megon<=megontime)//megontime指定磁测上电时间，单位为秒
						megon++;
					
					if(megon==megontime)//计数器计数到指定的值，给磁测上电
					{				
						MEG_Enable;//磁测模块上电
						tongbu=1;//同步后，tongbu=0，磁测板红灯在亮，绿灯在闪。
						Delay_ms(1000);
// 						SetMegthreshold(0xAE);//设置磁测模块的漏磁阈值//磁测已经添加存储阈值的功能
						Delay_ms(1000);
// 						SetMegthreshold(0xB0);//设置磁测模块的甚低频阈值
					}		
					IWDG_Feed();   //喂狗		
					LED2( 0 );
					Delay_ms(1000);		// 1000ms   1s
					LED2( 1 );

					if(zttimeflag)//表示已经同步，记录姿态数据，不同步时，信标的倾摆对磁测数据没有影响，不需记录
					{
						ztcnt++;
						if(ztcnt>=30)
						{
							ztcnt=0;
							IWDG_Feed();    //喂狗
							res = f_open(&fsrc,(char *)fileztp, FA_OPEN_ALWAYS | FA_WRITE);
							res = f_lseek(&fsrc, f_size(&fsrc));
							IWDG_Feed();    //喂狗
							ADXL345_RD(tempbuf);	//读取10组姿态数据，做去除跳点的处理		
							while(1)
							{
								ADXL345_RD_XYZ2(buf);//获得一组姿态数据
								/*滑动滤波处理*/
								tempbuf[Filter++]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[5]<<8)+buf[4]); 
								IWDG_Feed();    //喂狗
								if(Filter==30)
									Filter=0; //先进先出，再求平均值
								
								a=tempbuf[i%30]-tempbuf[(i-3)%30];//X轴去除新获取的数据的跳点
								b=tempbuf[(i+3)%30]-tempbuf[i%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[i%30]=(tempbuf[(i-3)%30]+tempbuf[(i+3)%30])/2;
								IWDG_Feed();    //喂狗
								a=tempbuf[j%30]-tempbuf[(j-3)%30];//Y轴去除新获取的数据的跳点
								b=tempbuf[(j+3)%30]-tempbuf[j%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[j%30]=(tempbuf[(j-3)%30]+tempbuf[(j+3)%30])/2;
								
								a=tempbuf[k%30]-tempbuf[(k-3)%30];//Z轴去除新获取的数据的跳点
								b=tempbuf[(k+3)%30]-tempbuf[k%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[k%30]=(tempbuf[(k-3)%30]+tempbuf[(k+3)%30])/2;
								IWDG_Feed();    //喂狗
								for(count=0;count<30;)
								{
									tx+=tempbuf[count++];
									ty+=tempbuf[count++];
									tz+=tempbuf[count++];
								}
								tx=tx/10;//取平均值作为一组数据
								ty=ty/10;
								tz=tz/10;
								buffer[0]=(u8)(tx&0x00FF);
								buffer[1]=(u8)((tx>>8)&0x00FF);
								buffer[2]=(u8)(ty&0x00FF);
								buffer[3]=(u8)((ty>>8)&0x00FF);
								IWDG_Feed();    //喂狗
								buffer[4]=(u8)(tz&0x00FF);
								buffer[5]=(u8)((tz>>8)&0x00FF);
								buffer[6]=(u8)((ZTtime>>24)&0x000000FF);
								buffer[7]=(u8)((ZTtime>>16)&0x000000FF);
								buffer[8]=(u8)((ZTtime>>8)&0x000000FF);
								buffer[9]=(u8)(ZTtime&0x000000FF);
								i=i+3;
								j=j+3;
								k=k+3;
								if(i==33)
								{
									i=3;
									j=4;
									k=5;
								}
								IWDG_Feed();    //喂狗
								res = f_write(&fsrc,buffer,10, &bw);
								ztgroupcnt++;/////////////////
								if(ztgroupcnt==200)//////每次记录200组数据后退出
								{
									ztgroupcnt=0;/////
									IWDG_Feed();    //喂狗
									f_close(&fsrc);	
									break;
								}
								IWDG_Feed();    //喂狗		
								if(detectfeedback||RX_uart4flag||Rxwaterflag||Rxmegflag||tongbu)//若有其他信号到来，则退出
								{
									f_close(&fsrc);	
									break;
								}
							}
						}
					}
					
					
					statuscount++;
					if(statuscount==10)//每10秒存储一次状态信息
					{
						statuscount=0;
						Statusmonitor();
					}
					if(detectfeedback)//为1表示综控板向磁测模块发送数据了
					{
						detect++;
						if(detect==6)//计时变量到6表示过了6s还没有收到磁测的反馈（比如磁测断电）
						{
							detect=0;
							detectcount++;
							detectfeedback=0;
							IWDG_Feed();   //喂狗
							Set_Log(1,feedcom,0);//记录此次命令字发送失败
							log_len++;//记录SD卡log数据的条数
			        Set_Log_len(log_len);
							IWDG_Feed();   //喂狗
							if(detectcount==3)//连续三次发送失败后，给磁测模块关电，重新上电
							{
								detectcount=0;
								MEG_Disable;
								
								GPIO_ResetBits(GPIOG,GPIO_Pin_15);//添加的，给磁测断电就要关闭晶振信号的输出。
								
								Delay_ms(1000);	
								IWDG_Feed();   //喂狗
								Delay_ms(1000);	
								IWDG_Feed();   //喂狗
								Delay_ms(1000);	
								IWDG_Feed();   //喂狗
								Delay_ms(1000);	
								IWDG_Feed();   //喂狗
								Delay_ms(1000);	
								IWDG_Feed();   //喂狗
								
								
								MEG_Enable;
								tongbu=1;
							}
							else
								QueryToMeg(feedcom);//没有收到磁测的反馈，则再次发送命令�
						}
					}
					if(RX_uart4flag)//串口4接收数据完成，即无线传输完成
				 {
					 RX_uart4flag=0;
					 ExecutPCCommand();//处理无线传输的数据
				 }
					if(Rxwaterflag)//水声数据接收完成
					{
						IWDG_Feed();   //喂狗
						Rxwaterflag=0;
						ExecutWaterCommand();	 //处理水声数据
					}
					if(Rxmegflag)//磁测数据接收完成
					{
						Rxmegflag=0;
						ExecutMegCommand();//处理磁测数据
					}
					if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))//上锁
							GPSSource_Stop(); 
					if(release)//压载释放了
					{
						release=0;
						Delay_ms(1000);	
						IWDG_Feed();   //喂狗
						Delay_ms(1000);	
						IWDG_Feed();   //喂狗
// 						MEG_Disable;//0702更改，磁测断电以便于gps接收到经纬度信号
						zttimeflag=0;
						tongbu=0;
						Delay_ms(1000);	
						IWDG_Feed();   //喂狗
// 						GPSSource24V_OUT();//
					}
					
					if(tongbu)//同步标志位为1，表示其他操作完成，可以发送同步信号  
					{ 
						Delay_ms(1000);	//添加延时
						IWDG_Feed();  //喂狗
						if(meg_syn<=30)//给磁测上电后，过30秒后再给同步信号
							meg_syn++;
						if(meg_syn==30)
						{	
							meg_syn=0;
							tongbu=0;	//若磁测未同步时断电，则tongbu仍然为1，程序会发同步信号，然后检测反馈，因为磁测断电所以无反馈，因此程序会让磁测重启		
							GPIO_SetBits(GPIOG,GPIO_Pin_15);//给磁测一个上升沿，作为同步信号
							Set_Log(1,0xD0,1);//新加的DO,文件记录
							log_len++;//记录SD卡log数据的条数
			        Set_Log_len(log_len);
							ReadRTC(&RTC_ARR[0],0x00,8); 
							RTC_Time[0]=BCD2HEX(RTC_ARR[5]);
							RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
							RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
							RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
							RTC_Time[4]=BCD2HEX(RTC_ARR[0]);//2017、1、6添加，同步之后才开始,会有0.5S延迟
							
							Delay_ms(1000);	
							IWDG_Feed();  //喂狗

							Delay_ms(1000);	
							IWDG_Feed();   //喂狗
// 							GPIO_ResetBits(GPIOG,GPIO_Pin_15); //注释掉，因为改为同步信号和晶振相与作为磁测时钟的输入，开始同步之后要保持高电平才有信号输出给磁测。
							
							
							
							Delay_ms(1000);	
							IWDG_Feed();   //喂狗
							Delay_ms(1000);	
							IWDG_Feed();   //喂狗
							//QueryToMeg(0xA6);//查询同步是否成功
						}			
					}	
					
					IWDG_Feed();    //喂狗
					if(sendtowater)//为1 表示可以向水声模块发送数据
					{
						sendtowater=0;
						if(FromJiaBanA==4)//命令为A4水声
						{
							FromJiaBanA=0;
							SendData(lc_data.lc_blocknum,lc_data.lc_fsizebyte,0xA4);	//通过水声通信模块发送数据								
						}
						if(FromJiaBanA==3)//命令为A3
						{
							FromJiaBanA=0;
							SendData(dp_data.dp_blocknum,dp_data.dp_fsizebyte,0xA3);	
						}

            if(FromJiaBanA==5)//无线读取磁测漏磁数据
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB2);//通过无线发出flash数据
		      	}

           if(FromJiaBanA==6)//无线读取磁测低频数据
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB3);//通过无线发出flash数据
		      	}
					}
		
				}
		
		if(flag_rx==1)//如果接受定位消息成功标志位为1，则进入处理函数，不进入其他函数以免影响授时精度
		{  flag_rx = 0;
			 GPS_NMEA();//接收到的GPS数据进行转换
				if(GPS_VA)//接收的定位数据有效
				{	
					GPS_VA=0;
					USART_ITConfig(UART5,USART_IT_RXNE,DISABLE);//接收中断失能
					RTC_ARR[0] = 0;	// enable oscillator (bit 7=0)
					RTC_ARR[1] = HEX2BCD(Sec);	// enable oscillator (bit 7=0)
					RTC_ARR[2] = HEX2BCD(Min);	// minute = 59
					RTC_ARR[3] = HEX2BCD(Hour);	// hour = 05 ,24-hour mode(bit 6=0)
					RTC_ARR[4] = 1;	// Day = 1 or sunday
					RTC_ARR[5] = HEX2BCD(Day);	// Date = 30
					RTC_ARR[6] = HEX2BCD(Month);	// month = August
					RTC_ARR[7] = HEX2BCD(Year);	// year = 05 or 200
					WriteRTC(&RTC_ARR[0],0x00,8);	// Set RTC	  
					
					IWDG_Feed();  //喂狗
					SendREQToPC(0xC1);//回发授时成功				
				}
				
			}
	}
}

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits先占优先级1位，从优先级3位 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* 串口1中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//先占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* 串口3中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//先占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
	/* DMA中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* 串口4中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//先占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* 串口5中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//先占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void GPIO_EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//配置磁测模块同步IO口，推挽输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//BTS555控制
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//配置水声端口的使能端IO口
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//配置蜂鸣器控制IO口
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  //按键IO口初始化  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//电磁铁位置检测IO口配置中断
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);//配置中断线
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);//配置中断线
	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;       
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;       
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}



