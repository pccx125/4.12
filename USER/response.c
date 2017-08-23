#include "main.h"
extern u16 log_len;
extern struct LC_Data lc_data;
extern struct DP_Data dp_data;
extern u8 Water_power;//记录水声发射功率
extern u8 inputmegstr[136],inputwaterstr[8],input485str[14];//串口接收数据的数组
extern u8 Rxmegflag;//置1表示串口接收完成
extern u8 tongbu;//同步标志位为1，表示其他操作完成，可以发送同步信号 
u8 FromJiaBanA=0;//来自甲板单元的命令标志   为1表示A0，为2表示A1，为3表示A3，为4表示A4
extern u8 watersendcomplet,watersendfall;//信标水声发送完成标志    信标水声发送失败标志
extern u16 pressuretest,batterytest;//电池电压值和压力电压值
u8 sendtowater;//一切准备就绪，可以向甲板单元发送数据标志
FIL fsrc;//文件指针
u8 fileztp[25];//存储姿态数据建立的文件名
u32 ZTtime=0;//计数器，表示过了ZTtime 个ms
u8 zttimeflag=0;//为1时，time开始计数
u16 time=0;//计数器，记到1000，表示过了1ms
u8 tongbucount=0;//同步次数计数
extern u8 ControlBoardID;//信标设备识别码
u8 ComFromJiaBan=0;//为1表示命令来自于甲板单元
extern u32 waterontime,megontime;//记录水声和磁测上电时间
extern u32 wateron,megon;//计数器
extern u8 detectfeedback,feedcom,detect;//反馈标志位    命令字    计数器
u8 storagecnt=0;//表示检测信标状态的次数，检测5次时，就存储到SD卡中，计数不到5，不存储状态数据
u8 release=0;//为1表示压载释放，读压力传感器，GPS供电
extern unsigned char  RTC_Time[5];
	unsigned char  RTC_ARR[8];
u8 read_log_buffer[28] = {0}; 
//查询磁测模块
void QueryToMeg(u8 KeyWord)
{
	u8 i;
	u8 NoData[7]={0x7E,0x01,0x01,0xA0,0x00,0x00,0x00};
	u8 Data[8]={0x7E,0x01,0x01,0xA7,0x01,0x01,0x91,0x47};
	u8 Data1[8]={0x7E,0x01,0x01,0xA8,0x01,0x01,0x92,0x77};
	u16 wCRC;
	NoData[1]=ControlBoardID;
	NoData[2]=ControlBoardID;
	Data[1]=ControlBoardID;
	Data[2]=ControlBoardID;
	Data1[1]=ControlBoardID;
	Data1[2]=ControlBoardID;
	IWDG_Feed();   //喂狗
	switch(KeyWord)
	{
		case 0xA0:    //甚低频数据条数查询
		case 0xA1:    //漏磁数据条数查询
		case 0xA5:    //通知磁测模块自检
		case 0xA6:    //检测同步是否成功
		case 0xAD:    //读取漏磁阈值
		case 0xAF:    //读取甚低频阈值
			NoData[3]=KeyWord;
		  wCRC=chkcrc(NoData,5);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
		 IWDG_Feed();   //喂狗
			for(i=0;i<7;i++)
				RS232_MegSend(NoData[i]);//A0-AF都会执行这个，因为没有break
		  break;
		case 0xA7:    //信标接收到磁测模块传来的漏磁数据
			for(i=0;i<8;i++)
				RS232_MegSend(Data[i]);
		  break;
		case 0xA8:    //信标接收到磁测模块传来的甚低频数据
			for(i=0;i<8;i++)
				RS232_MegSend(Data1[i]);
			break;
		case 0xA3:    //甚低频数据读取
			NoData[3]=0xA3;
		  wCRC=chkcrc(NoData,5);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
			for(i=0;i<7;i++)
				RS232_MegSend(NoData[i]);
		  break;
		case 0xA4:    //漏磁数据读取
			NoData[3]=0xA4;
		  wCRC=chkcrc(NoData,5);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
			for(i=0;i<7;i++)
				RS232_MegSend(NoData[i]);
		  break;
		default:
			break;
	}
	IWDG_Feed();   //喂狗
	if((KeyWord!=0xA7)&&(KeyWord!=0xA8))
	{
		detectfeedback=1;
		feedcom=KeyWord;
	}
}

//处理磁测模块传过来的数据
void ExecutMegCommand(void)
{
	u8 count=0;
	u8 i,j=0;
	u8 buffertemp[130];
	u8 DipinData,LouciData;
	u16 wCRC;
	u8 sendbuffto485[11]={0x00,0x00,0x50,0x7E,0x7E,0xAD,0x02,0x00,0x00,0x00,0x00};//通过无线模块发送消息
	u8 sendbufftowater[9]={0x7E,0x01,0x00,0xAD,0x02,0x00,0x00,0x00,0x00};
	sendbufftowater[1]=ControlBoardID;
// 	detectfeedback=0;//表示收到磁测发过来的信息
	detect=0;
	IWDG_Feed();   //喂狗
	switch(inputmegstr[3])
	{
		case 0xA0:
		detectfeedback=0;//表示收到磁测回应信息，否则重发命令
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
			if(inputmegstr[5]!=0)//甚低频数据条数不为0，读数据
			{
				dp_data.dp_rdgroupcnt=inputmegstr[5];//获得甚低频数据条数
				dp_data.DP_AllGroupCnt+=dp_data.dp_rdgroupcnt;	//甚低频数据条数累加
				QueryToMeg(0xA3);//有甚低频数据条数，发A3命令读取甚低频数据
			}
			else    //如果磁测模块此时没有甚低频数据，并且是甲板单元要读取甚低频数据，置标志为1，综控板可以开始发数据了
			{
				Set_Log(1,0xA0,1);//记录工作日志信息				
				log_len++;//记录SD卡log数据的条数
				Set_Log_len(log_len);
				
				if(FromJiaBanA==3)//变量为3表示甲板单元要读甚低频数据
					sendtowater=1;
				 if (FromJiaBanA==6)//如果磁测无数据，且无线要读取命令
				 sendtowater=1;//无线发送就绪
			}
			if(FromJiaBanA==1)//如果此时甲板单元要询问甚低频数据条数，则发送
			{
				FromJiaBanA=0;							
				SendDataNum(dp_data.DP_AllGroupCnt,0xA0);	//发送甚低频数据条数					
			}
			break;
		case 0xA1:
			detectfeedback=0;//表示收到磁测发过来的信息
		  Set_Log(0,0x1F,1);
				log_len++;//记录SD卡log数据的条数
				Set_Log_len(log_len);
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
			if(inputmegstr[5]!=0)//漏磁数据条数不为0，读数据
			{
				
				lc_data.lc_rdgroupcnt=inputmegstr[5];
				lc_data.LC_AllGroupCnt+=lc_data.lc_rdgroupcnt;		//漏磁数据条数累加				
				QueryToMeg(0xA4);  //如果磁测模块内有数据，则发送读取数据命令
			}
			else //如果磁测模块此时没有漏磁数据，并且是甲板单元要读取漏磁数据，置标志为1，综控板可以开始发数据了
			{
				Set_Log(1,0xA1,1);
				log_len++;//记录SD卡log数据的条数
				Set_Log_len(log_len);
				if(FromJiaBanA==4)//变量为4表示甲板单元要读漏磁数据
					sendtowater=1;
				 if (FromJiaBanA==5)//如果磁测无数据，且无线要读取命令
				 sendtowater=1;//无线发送就绪

			} 
			
			
			if(FromJiaBanA==2)//如果此时甲板单元要询问漏磁数据条数，则发送
			{
				FromJiaBanA=0;							
				SendDataNum(lc_data.LC_AllGroupCnt,0xA1);		//发送漏磁数据条数				
			}
			break;
		case 0xA3:
      detectfeedback=0;//表示收到磁测发过来的信?			
			DipinData=inputmegstr[4];    //获得数据字节数
// 		  wCRC=chkcrc(inputmegstr,DipinData+5);
// 		  if((inputmegstr[DipinData+5]!=wCRC/256)||(inputmegstr[DipinData+6]!=wCRC%256))
// 				break;
		  for(i=5;i<DipinData+5;i++)       //把有效数据读取出来        
			{
				buffertemp[j++]=inputmegstr[i];
			}
			WriteDatatoFlash(buffertemp,DipinData,1);//把数据存入NandFlash中
			WriteDatatosd(dp_data.dp_filep,buffertemp,DipinData);//把甚低频数据存入sd卡中
			j=0;
			QueryToMeg(0xA7);  //发送A7表示数据接收完成
			dp_data.DP_DataByteCnt+=DipinData;//用于记录NandFlash中甚低频数据的字节数
			if(dp_data.dp_rdgroupcnt>10)//每次综控板最多从磁测模块接收10组数据，若组数大于10 ，则分批读取
			{
				QueryToMeg(0xA3);
				dp_data.dp_rdgroupcnt=dp_data.dp_rdgroupcnt-10;
			}
			else  //读取完数据后置1,表示数据读完可以向水声发送数据
			{
				sendtowater=1;
			}
			Set_Log(1,0xA3,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0xA4:
			detectfeedback=0;//表示收到磁测发过来的信?
			LouciData=inputmegstr[4];
// 		  wCRC=chkcrc(inputmegstr,LouciData+5);
// 		  if((inputmegstr[LouciData+5]!=wCRC/256)||(inputmegstr[LouciData+6]!=wCRC%256))
// 				break;
			for(i=5;i<LouciData+5;i++)    //把有效数据读取出来                  
			{
				buffertemp[j++]=inputmegstr[i];
			}
 			WriteDatatoFlash(buffertemp,LouciData,0);//向flash中写数据
			WriteDatatosd(lc_data.lc_filep,buffertemp,LouciData);//向sd卡中写数据
			j=0;
			QueryToMeg(0xA8);//发送A8表示数据接收完成
   		lc_data.LC_DataByteCnt+=LouciData;//用于记录NandFlash中漏磁数据的字节数
			if(lc_data.lc_rdgroupcnt>10)//每次综控板最多从磁测模块接收10组数据，若组数大于10 ，则分批读取
			{
				QueryToMeg(0xA4);
				lc_data.lc_rdgroupcnt=lc_data.lc_rdgroupcnt-10;
			}
			else   //读取完数据后置1,表示数据读完可以向水声发送数据
			{
				sendtowater=1;
			}
			Set_Log(1,0xA4,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0xA5://磁测模块自检的反馈
			Set_Log(1,0xA5,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0xA6:
			detectfeedback=0;//表示收到磁测发过来的信息
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
	
			if(inputmegstr[5]!=0)
			{
 				CreateNewArea(1);//同步完成后，在NandFlash中建立一个新区来存放这次同步后的数据为1 表示建立甚低频区
				CreateNewArea(0);//建立漏磁区
				CreateNewFile(1);//同步完成后，在sd卡中建立一个新文件来存放这次同步后的  甚低频数据 
				CreateNewFile(0);//同步完成后，在sd卡中建立一个新文件来存放这次同步后的 漏磁数据	
        CreateNewFile(2);	//同步完成后，在sd卡中建立一个新文件来存放姿态数据			
				lc_data.LC_DataByteCnt=0;//记录漏磁数据字节总数置0
				dp_data.DP_DataByteCnt=0;//记录甚低频数据字节总数置0
				
			}
			else
			{
				tongbucount++;//若同步不成功，则再发送两次同步信号
				GPIO_SetBits(GPIOG,GPIO_Pin_15);//同步信号给一个高电平的脉冲
				Delay_ms(1000);	
				IWDG_Feed();  //喂狗
				Delay_ms(1000);	
				IWDG_Feed();   //喂狗
				GPIO_ResetBits(GPIOG,GPIO_Pin_15);
				Delay_ms(1000);	
				IWDG_Feed();  //喂狗
				Delay_ms(1000);	
				IWDG_Feed();   //喂狗
				if(tongbucount<2)	//若磁测模块没有接收到同步信号，则连续发送三次同步信号	
					QueryToMeg(0xA6);
				else
					tongbucount=0;
			}	
			Set_Log(1,0xA6,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0xA9://表示磁测模块存储的甚低频数据达到25组，主动要求综控板读取
			Set_Log(1,0xA9,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
		 
			QueryToMeg(0xA0);
		 
			break;
		case 0xAA://表示磁测模块存储的漏磁数据达到25组，主动要求综控板读取
			Set_Log(1,0xAA,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			QueryToMeg(0xA1);
			break;
		case 0xAD:  //读取漏磁阈值后，磁测模块返回漏磁阈值数，综控板上传
		case 0xAF:  //读取甚低频阈值后，磁测模块返回漏磁阈值数，综控板上传
			detectfeedback=0;//表示收到磁测发过来的信?
			Set_Log(1,inputmegstr[3],1);
			if(ComFromJiaBan)//来自甲板单元的命令
			{
				sendbufftowater[3]=inputmegstr[3];
				sendbufftowater[4]=0x02;
				sendbufftowater[5]=inputmegstr[5];
				sendbufftowater[6]=inputmegstr[6];
				wCRC=chkcrc(sendbufftowater,7);
				sendbufftowater[7]=wCRC/256;
				sendbufftowater[8]=wCRC%256;
				IWDG_Feed();  //喂狗
				if(watersendcomplet!=0)
					watersendcomplet=0;
				
				for(i=0;i<9;i++)
					RS232_WaterSend(sendbufftowater[i]);//通过水声模块发送信息
				
				while((watersendcomplet!=2)&&(count<15))//等待水声通信模块两次反馈，最多等待30s
				{
					count++;
					Delay_ms(1000);	
					IWDG_Feed();  //喂狗
					Delay_ms(1000);	
					IWDG_Feed();   //喂狗
				}
				count=0;				
				watersendcomplet=0;
			}
			else//来自无线模块的命令
			{
				sendbuffto485[5]=inputmegstr[3];
				sendbuffto485[6]=0x02;
				sendbuffto485[7]=inputmegstr[5];
				sendbuffto485[8]=inputmegstr[6];
				wCRC=chkcrc(sendbuffto485,9);
				sendbuffto485[9]=wCRC/256;
				sendbuffto485[10]=wCRC%256;
				IWDG_Feed();  //喂狗
				for(i=0;i<11;i++)
					WirelessSend(sendbuffto485[i]);//通过无线模块发送
			}
	}
}
//设置水声模块的状态
void QueryToWater(u8 KeyWord)
{
	u8 i;
	u8 NoData[7]={0x7E,0x01,0x01,0x12,0x00,0x00,0x00};
	u16 wCRC;
	NoData[1]=ControlBoardID;
	NoData[2]=ControlBoardID;
	IWDG_Feed();  //喂狗
	switch(KeyWord)
	{
		case 0x12:    //使水声通信模块进入低功耗
		case 0x14:  //唤醒信标水声模块
		case 0x16:  //信标水声模块自检
		case 0x1C:   //读取信标水声的发射功率
			NoData[3]=KeyWord;
		  wCRC=chkcrc(NoData,4);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
			for(i=0;i<7;i++)
				RS232_WaterSend(NoData[i]);
		  break;
		default:
			break;
	}
	IWDG_Feed();  //喂狗
}
void ExecutWaterCommand(void)//执行信标水声传送过来的命令
{
	u8 i,j;
	u8 num=0;
	u8 FeedBack[8];
	u8 sendbuffto485[10]={0x00,0x00,0x50,0x7E,0x7E,0x1C,0x01,0x00,0x00,0x00};
	u16 wCRC;
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();  //喂狗
	if((inputwaterstr[1]==0x00)&&(inputwaterstr[2]==ControlBoardID))//收到水声模块发送过来的数据后，综控向水声模块反馈
	 {
			for(i=0;i<4;i++)
				FeedBack[i]=inputwaterstr[i];
			FeedBack[4]=0x01;
			FeedBack[5]=0x01;
			wCRC=chkcrc(FeedBack,6);
			FeedBack[6]=wCRC/256;
			FeedBack[7]=wCRC%256;
			IWDG_Feed();  //喂狗
			for(i=0;i<4;i++)
			{
				Delay_ms(1000);	
				IWDG_Feed();  //喂狗
			}
			for(i=0;i<8;i++)
				RS232_WaterSend(FeedBack[i]);
	 } 
	switch(inputwaterstr[3])
	{
		case 0x12:    //使水声通信模块进入低功耗
			Set_Log(0,0x12,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0x14:  //唤醒信标水声模块
			Set_Log(0,0x14,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0x16:  //信标水声模块自检
			Set_Log(0,0x16,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			break;
		case 0x1C:   //读取信标水声的发射功率
			Set_Log(0,0x1C,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			sendbuffto485[5]=0x1C;
		  sendbuffto485[7]=inputwaterstr[5];
		  wCRC=chkcrc(sendbuffto485,8);
		  sendbuffto485[8]=wCRC/256;
		  sendbuffto485[9]=wCRC%256;
		  IWDG_Feed();  //喂狗
			for(i=0;i<10;i++)
				WirelessSend(sendbuffto485[i]);
		  break;
		case 0xC0://压载释放
			Set_Log(0,0xC0,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			i=0;
		  MegDriver_Config();//驱动BTN7971
			Delay_ms(600); 
			MegDriver_Stop();
			IWDG_Feed();  //喂狗
			while((i<3)&&(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)))//判断释放开关是否释放，如果没有释放则驱动3次
			{
				MegDriver_Config();//驱动BTN7971
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //喂狗
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);	
					IWDG_Feed();  //喂狗
				}				
			}  
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
				release=1; 			
		  break;
		case 0xA0:
			Set_Log(0,0xA0,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
		  FromJiaBanA=1;
		  QueryToMeg(0xA0);
		  break;
		case 0xA1:
			Set_Log(0,0xA1,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
		  FromJiaBanA=2;
		  QueryToMeg(0xA1);
		  break;
		case 0xA3:
			Set_Log(0,0xA3,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
		  FromJiaBanA=3;
		  QueryToMeg(0xA0);
		  break;
		case 0xA4:
			Set_Log(0,0xA4,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
		  FromJiaBanA=4;
		  QueryToMeg(0xA1);
		  break;
		case 0xAB://给磁测上电
			Set_Log(0,0xAB,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			MEG_Enable;
		  SendREQToWater(0xAB);//向甲板单元反馈
		  tongbu=1;//同步标志置1
// 		  QueryToMeg(0xA5);
// 		  Delay_ms(1000);
// 			SetMegthreshold(0xAE);//设置漏磁阈值
			Delay_ms(1000);
// 			SetMegthreshold(0xB0);//设置甚低频阈值
		  break;
		case 0xAC:
			Set_Log(0,0xAC,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			MEG_Disable;
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//添加的，给磁测断电就要关闭晶振信号的输出。
		
		  zttimeflag=0;
		  SendREQToWater(0xAC);
		  break;
		case 0xB4://水声读取综控板状态信息
			Set_Log(0,0xB4,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			SendStatus();
		  break;
		case 0xB1://清空NandFlash数据
			Set_Log(0,0xB1,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
				/*设置NAND FLASH的写地址*/
			WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x00;
			WriteReadAddr.Page = 0x00;
		  IWDG_Feed();  //喂狗
			/*擦除待写入数据的块*/
			NAND_EraseBlock(WriteReadAddr);//清除漏磁数据存储的falsh地址
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  IWDG_Feed();  //喂狗
		  WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x200;
			WriteReadAddr.Page = 0x00;
			/*擦除待写入数据的块*/
			NAND_EraseBlock(WriteReadAddr);//清除甚低频数据存储的falsh地址
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  lc_data.LC_AllGroupCnt=0;
		  dp_data.DP_AllGroupCnt=0;
		  IWDG_Feed();  //喂狗
		  SendREQToWater(0xB1);//向甲板单元反馈
		Delay_ms(5000); //添加长延时，迫使程序复位// 		//张炳洋20161027添加开始
		break;
		case 0xAD:    //读取磁测模块漏磁阈值
			Set_Log(0,0xAD,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  QueryToMeg(0xAD);
		  break;
		case 0xAF:    //读取磁测模块甚低频阈值
			Set_Log(0,0xAF,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  QueryToMeg(0xAF);
		  break;
		case 0xAE:    //设置磁测模块漏磁阈值
			lc_data.lc_threshold[0]=inputwaterstr[5];
			lc_data.lc_threshold[1]=inputwaterstr[6];
		  Set_Log(0,0xAE,1);
			log_len++;//记录SD卡log数据的条数
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  SetMegthreshold(0xAE);
		  SendREQToWater(inputwaterstr[3]);
		break;
		case 0xB0:    //设置磁测模块甚低频阈值
			dp_data.dp_threshold[0]=inputwaterstr[5];
	    dp_data.dp_threshold[1]=inputwaterstr[6];
			Set_Log(0,0xB0,1);
			ComFromJiaBan=1;
		  SetMegthreshold(0xB0);
		  SendREQToWater(inputwaterstr[3]);
		  break;
		default:
			break;
	}
	IWDG_Feed();  //喂狗
}
//发送状态信息
void SendStatus(void)
{
	u8 count=0;
	u8 i;
	u16 wCRC;
	u8 buffer[25]={0x7E,0x01,0x00,0xB4,0x12};
	DHT11_Data_TypeDef DHT11_Data;
	unsigned char  RTC_ARR[8];
	buffer[1]=ControlBoardID;
	buffer[2]=0x00;
	ReadRTC(&RTC_ARR[0],0x00,8); //时间  日 时 分 秒 毫秒（10ms）
	buffer[5]=BCD2HEX(RTC_ARR[6]);
	buffer[6]=BCD2HEX(RTC_ARR[5]);
	buffer[7]=BCD2HEX(RTC_ARR[3]);
	buffer[8]=BCD2HEX(RTC_ARR[2]);
	buffer[9]=BCD2HEX(RTC_ARR[1]);
	buffer[10]=BCD2HEX(RTC_ARR[0]);
	IWDG_Feed();  //喂狗
	ADC_DMA_Config();
	Delay_us(500);
	buffer[11]=batterytest/100;//电池电压整数位
	buffer[12]=batterytest%100;//电池电压小数位
//添加检测电压语句，如果大于31V，则认为是错误采集，改值为0.
  
 if(buffer[11]>=0x20)//若大于31V，则认为出错，置0
	{
		buffer[11]=0;
		buffer[12]=0;
	}
	
	
// 	if(buffer[12]>=0x28)//一般小数位大0.4V，故手动校正
// 		buffer[12]-=0x28;
// 	 else 
// 	 {
// 		 buffer[11]=buffer[11]-1;
// 		 buffer[12]=buffer[12]+0x3C;
// 	 }
	 
	buffer[13]=pressuretest/100;//压力
	buffer[14]=pressuretest%100;
	IWDG_Feed();  //喂狗
	if( Read_DHT11(&DHT11_Data)==SUCCESS)	
	{
		buffer[15]=DHT11_Data.temp_int;//温度的整数部分
		buffer[16]=DHT11_Data.humi_int;//湿度的整数部分
	}
	else
	{
		buffer[15]=0x00;//温度的整数部分
		buffer[16]=0x00;//湿度的整数部分
	}
// 	ADXL345_Angle(&buffer[17]);	//读取X,Y,Z三个方向的加速度值 
	
	buffer[17]=RTC_Time[0];
	buffer[18]=RTC_Time[1];
	buffer[19]=RTC_Time[2];
	buffer[20]=RTC_Time[3];
	buffer[21]=RTC_Time[4];//B4将姿态数据改为同步时间
	buffer[22]=0x00;//为保证甲板单元测试界面，解析正确，添加一个00
	
	wCRC=chkcrc(buffer,23);
	buffer[23]=wCRC/256;
	buffer[24]=wCRC%256;	
	IWDG_Feed();  //喂狗
	if(watersendcomplet!=0)
		watersendcomplet=0;
	
	for(i=0;i<25;i++)
		RS232_WaterSend(buffer[i]);	
	while((watersendcomplet!=2)&&(count<15))
	{
		count++;
		Delay_ms(1000);	
		IWDG_Feed();  //喂狗
		Delay_ms(1000);	
		IWDG_Feed();  //喂狗
	}
	count=0;
	watersendcomplet=0;
}
//通过水声向甲板单元反馈信息
void SendREQToWater(u8 com)
{
	u8 count=0;
	u8 i;
	u16 wCRC;
	u8 buffer[8]={0x7E,0x01,0x00,0xB0,0x01,0x01};
	buffer[1]=ControlBoardID;
	buffer[3]=com;
	wCRC=chkcrc(buffer,6);
	buffer[6]=wCRC/256;
	buffer[7]=wCRC%256;
	
	if(watersendcomplet!=0)
		watersendcomplet=0;
	
	for(i=0;i<8;i++)
		RS232_WaterSend(buffer[i]);	
	
	while((watersendcomplet!=2)&&(count<15))
	{
		count++;
		Delay_ms(1000);	
		IWDG_Feed();  //喂狗
		Delay_ms(1000);	
		IWDG_Feed();  //喂狗
	}
	count=0;
	watersendcomplet=0;
}

//发送数据条数
void SendDataNum(u16 Groupcount,u8 com)
{
	u8 count=0;
	u8 i;
	u16 wCRC;
	u8 JiaBanFeedBack[8];
	JiaBanFeedBack[0]=0x7E;
	JiaBanFeedBack[1]=ControlBoardID;
	JiaBanFeedBack[2]=0x00;
	JiaBanFeedBack[3]=com;
	JiaBanFeedBack[4]=0x01;
	JiaBanFeedBack[5]=Groupcount;
	wCRC=chkcrc(JiaBanFeedBack,6);
	JiaBanFeedBack[6]=wCRC/256;
	JiaBanFeedBack[7]=wCRC%256;	
  IWDG_Feed();  //喂狗
	if(watersendcomplet!=0)
		watersendcomplet=0;
	
	for(i=0;i<8;i++)
		RS232_WaterSend(JiaBanFeedBack[i]);
	
	while((watersendcomplet!=2)&&(count<15))
	{
		count++;
		Delay_ms(1000);
		IWDG_Feed();  //喂狗
		Delay_ms(1000);
		IWDG_Feed();  //喂狗
	}
	count=0;
	watersendcomplet=0;
}
//从NandFlash的一个块中的页中读取数据发出
void ReadDataFromPage(u16 block,u8 page,u8 *timebuffer,u8 com)
{
	u8 count=0;
	u8 tempbuff[130];
	u8 readbuffer[70]={0x7E,0x01,0x00,0xA4,0x00};
	u8 datagroupcount;
	u8 sendyushu,sendcount;
	u8 *buff;
	u16 wCRC;
	u8 i;
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();  //喂狗
	readbuffer[1]=ControlBoardID;
	readbuffer[3]=com;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(&datagroupcount, WriteReadAddr,1);//得到这一页的数据组数
	IWDG_Feed();  //喂狗
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block-1;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(tempbuff, WriteReadAddr,datagroupcount*13);//得到这一页的数据
	IWDG_Feed();  //喂狗
	buff=tempbuff;
	sendcount=datagroupcount*13/52;//每次最多发送的磁测数据字节数为52个字节 4组 多于的分批发出
	sendyushu=datagroupcount*13%52;
	while(sendcount--)
	{
		for(i=0;i<52;i++)
			readbuffer[10+i]=*buff++;
		for(i=0;i<5;i++)
			readbuffer[5+i]=timebuffer[i];
		readbuffer[4]=57;
		wCRC=chkcrc(readbuffer,62);
		readbuffer[62]=wCRC/256;
		readbuffer[63]=wCRC%256;
		IWDG_Feed();  //喂狗
		if(watersendcomplet!=0)
		watersendcomplet=0;
		
		for(i=0;i<64;i++)
			RS232_WaterSend(readbuffer[i]);
		IWDG_Feed();  //喂狗
		while((watersendcomplet!=2)&&(count<15))
		{
			count++;
			Delay_ms(1000);
			IWDG_Feed();  //喂狗
			Delay_ms(1000);
			IWDG_Feed();  //喂狗
		}
		count=0;
		
		watersendcomplet=0;
	}
	if(sendyushu)
	{
		for(i=0;i<sendyushu;i++)
			readbuffer[10+i]=*buff++;
		for(i=0;i<5;i++)
			readbuffer[5+i]=timebuffer[i];
		readbuffer[4]=sendyushu+5;
		wCRC=chkcrc(readbuffer,sendyushu+10);
		readbuffer[sendyushu+10]=wCRC/256;
		readbuffer[sendyushu+11]=wCRC%256;
		IWDG_Feed();  //喂狗
		
		if(watersendcomplet!=0)
		watersendcomplet=0;
		
		for(i=0;i<sendyushu+12;i++)
			RS232_WaterSend(readbuffer[i]);
		IWDG_Feed();  //喂狗
		while((watersendcomplet!=2)&&(count<15))
		{
			count++;
			Delay_ms(1000);
			IWDG_Feed();  //喂狗
			Delay_ms(1000);
			IWDG_Feed();  //喂狗
		}
		count=0;	
		watersendcomplet=0;
	}	
}

//发送数据
void SendData(u16 rdblocknum,u32 fsizebyte,u8 com)
{
	u8 flag;
	u8 pagenum,blocknum;
	u8 i;
	u8 datagroupcount;
	u16 datacount=0;
	u8 timebuffer[5];
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();  //喂狗
	if(com==0xA4)
		flag=0;
	else if(com==0xA3)
		flag=1;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);
	IWDG_Feed();  //喂狗
	while(rdblocknum<=blocknum)//读取过数据的flash块数小于已有数据的flash块数
	{
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = rdblocknum*3-1+0x200*flag;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(timebuffer, WriteReadAddr, 5);//得到时间信息
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = rdblocknum*3-2+0x200*flag;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);//得出页数
		IWDG_Feed();  //喂狗
		if(fsizebyte==0)//偏移等于0
		{
			for(i=1;i<pagenum;i++)
				ReadDataFromPage(rdblocknum*3+0x200*flag,i,timebuffer,com);
		}
		else 
		{
			 for(i=1;i<pagenum;i++)//找出没有读过的页数
			 {
					WriteReadAddr.Zone = 0x00;
					WriteReadAddr.Block = rdblocknum*3+0x200*flag;
					WriteReadAddr.Page = i;
					NAND_ReadBuffer(&datagroupcount, WriteReadAddr, 1);  
					IWDG_Feed();  //喂狗
					datacount+=datagroupcount*13;
					if(datacount==fsizebyte)
						break;		 
			 }
			 for(i++;i<pagenum;i++)//发送数据
					ReadDataFromPage(rdblocknum*3+0x200*flag,i,timebuffer,com);
	   }
		 if(rdblocknum<blocknum)
				fsizebyte=0;
		 rdblocknum++;
	 }
	 if(flag==0)//表示漏磁数据有关
	 {
			lc_data.lc_fsizebyte=lc_data.LC_DataByteCnt;
		  lc_data.lc_blocknum=blocknum;
	 }	
   else
   {
		  dp_data.dp_fsizebyte=dp_data.DP_DataByteCnt;
		  dp_data.dp_blocknum=blocknum;
	 }		 
}

void CreateNewFile(u8 flag)//建立一个新的文件
{
	unsigned char  RTC_ARR[8],RTC_Time[5];
	u8 fname[9] = {0};
	IWDG_Feed();  //喂狗
	ReadRTC(&RTC_ARR[0],0x00,8); 
	fname[0]=BCD2HEX(RTC_ARR[6])/10+0x30;//月
	fname[1]=BCD2HEX(RTC_ARR[6])%10+0x30;
	fname[2]=BCD2HEX(RTC_ARR[5])/10+0x30;//日
	fname[3]=BCD2HEX(RTC_ARR[5])%10+0x30;
	fname[4]=BCD2HEX(RTC_ARR[3])/10+0x30;//时
	fname[5]=BCD2HEX(RTC_ARR[3])%10+0x30;
	fname[6]=BCD2HEX(RTC_ARR[2])/10+0x30;//分
	fname[7]=BCD2HEX(RTC_ARR[2])%10+0x30;
	fname[8]='\0';
	IWDG_Feed();  //喂狗
	RTC_Time[0]=BCD2HEX(RTC_ARR[5]);//日
	RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
	RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
	RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
	RTC_Time[4]=BCD2HEX(RTC_ARR[0]);
	switch(flag)
	{
		case 0://漏磁数据文件
			sprintf((char*)lc_data.lc_filep,"0:/lc/%s.bin",fname);
			IWDG_Feed();  //喂狗
			WriteDatatosd(lc_data.lc_filep,RTC_Time,5);
		break;
		case 1://甚低频数据文件
			sprintf((char*)dp_data.dp_filep,"0:/dp/%s.bin",fname);
			IWDG_Feed();  //喂狗
			WriteDatatosd(dp_data.dp_filep,RTC_Time,5);		
		break;
		case 2://姿态数据文件
			sprintf((char*)fileztp,"0:/zt/%s.bin",fname);
		  IWDG_Feed();  //喂狗
			WriteDatatosd(fileztp,RTC_Time,5);
			ZTtime=0;
			time=0;
			zttimeflag=1;
		break;
	}
}
void WriteDatatosd(u8 *tmp_name,u8 *buffertemp,u8 datalen)//把数据写入SD卡中
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 

	res = f_open(&fsrc, (char*)tmp_name, FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		res = f_lseek(&fsrc, f_size(&fsrc)); 
		IWDG_Feed();  //喂狗
		res = f_write(&fsrc, buffertemp,datalen, &bw);
	}
	else
	{
		f_close(&fsrc);
		IWDG_Feed();  //喂狗
		Delay_ms(1000);
		res = f_open(&fsrc, (char*)tmp_name, FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			res = f_lseek(&fsrc, f_size(&fsrc)); 
			IWDG_Feed();  //喂狗
			res = f_write(&fsrc, buffertemp,datalen, &bw);
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();  //喂狗
}

//从NandFlash的一个块中的页中读取数据通过无线模块发出
void ReadDataFromPageToPC(u16 block,u8 page,u8 *timebuffer,u8 com)
{
	u8 tempbuff[141]={0x7E,0x7E,0xB2,0x00,0x00,0x00};
	u8 readbuff[58]={0x00,0x00,0x50};
	u8 datagroupcount;
	u16 wCRC;
	u8 i;
	u8 *buff;
	u8 sendyushu,sendcount;
	NAND_ADDRESS WriteReadAddr;
	tempbuff[2]=com;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(&datagroupcount, WriteReadAddr,1);//得到这一页的数据组数
	tempbuff[3]=datagroupcount*13+5;//漏磁数据字节数+时间5个字节
	IWDG_Feed();  //喂狗
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block-1;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(&tempbuff[9], WriteReadAddr,datagroupcount*13);//得到这一页的数据
	IWDG_Feed();  //喂狗
	for(i=0;i<5;i++)
			tempbuff[4+i]=timebuffer[i];
	wCRC=chkcrc(tempbuff,datagroupcount*13+9);
	tempbuff[datagroupcount*13+9]=wCRC/256;
	tempbuff[datagroupcount*13+10]=wCRC%256;
	
	buff=tempbuff;
	sendcount=(datagroupcount*13+11)/58;//55更改为58. 无线模块一次传58字节
	sendyushu=(datagroupcount*13+11)%58;
	while(sendcount--)
	{
		for(i=0;i<58;i++)
			readbuff[3+i]=*buff++;
		IWDG_Feed();  //喂狗		
		for(i=0;i<61;i++)
			WirelessSend(readbuff[i]);
		Delay_ms(50);//原来延时30ms
		IWDG_Feed();  //喂狗
	}
	if(sendyushu)
	{
		for(i=0;i<sendyushu;i++)
			readbuff[3+i]=*buff++;
		IWDG_Feed();  //喂狗
		for(i=0;i<sendyushu+3;i++)
			WirelessSend(readbuff[i]);
		IWDG_Feed();  //喂狗
	}	
}
void ReadAllDataToPC(u8 com)//通过无线发送数据时使用
{
	u8 pagenum,blocknum;
	u8 i,j;
	u8 flag=0;
	u8 timebuffer[5];
	NAND_ADDRESS WriteReadAddr;
	if(com==0xB2)
		flag=0;
	else if(com==0xB3)
		flag=1;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+flag*0x200;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);
	IWDG_Feed();  //喂狗
	for(i=1;i<=blocknum;i++)
	{
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = i*3-1+flag*0x200;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(timebuffer, WriteReadAddr, 5);//获得时间信息
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block =i*3-2+flag*0x200;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);	//获得数据有多少页
		IWDG_Feed();  //喂狗
		for(j=1;j<pagenum;j++)
		{
			ReadDataFromPageToPC(i*3+flag*0x200,j,timebuffer,com);
			Delay_ms(100);
		}			
	}
}
//通过无线模块向甲板单元发送状态信息
void SendStatusToPC(void)
{
	u8 i;
	u16 wCRC;
	u8 buffer[27]={0x00,0x00,0x50,0x7E,0x7E,0xB4,0x12};
	DHT11_Data_TypeDef DHT11_Data;
	unsigned char  RTC_ARR[8];
	ReadRTC(&RTC_ARR[0],0x00,8); //获取时间信息
	buffer[7]=BCD2HEX(RTC_ARR[6]);
	buffer[8]=BCD2HEX(RTC_ARR[5]);
	buffer[9]=BCD2HEX(RTC_ARR[3]);
	buffer[10]=BCD2HEX(RTC_ARR[2]);
	buffer[11]=BCD2HEX(RTC_ARR[1]);
	buffer[12]=BCD2HEX(RTC_ARR[0]);
	IWDG_Feed();  //喂    狗
	ADC_DMA_Config();
	Delay_us(500); 
	buffer[13]=batterytest/100;//电池电压整数位
	buffer[14]=batterytest%100;//电池电压小数位

 if(buffer[13]>=0x20)//若大于31V，则认为出错，置0
	{
		buffer[13]=0;
		buffer[14]=0;
	}
	
// 	
// 	if(buffer[14]>=0x28)//一般小数位大0.4V，故手动校正
// 		buffer[14]-=0x28;
// 	 else 
// 	 {
// 		 buffer[13]=buffer[13]-1;
// 		 buffer[14]=buffer[14]+0x3C;
// 	 }
	 
	buffer[15]=pressuretest/100;
	buffer[16]=pressuretest%100;
	if( Read_DHT11(&DHT11_Data)==SUCCESS)	
	{
		buffer[17]=DHT11_Data.temp_int;//温度的整数部分
		buffer[18]=DHT11_Data.humi_int;//湿度的整数部分
	}
	else
	{
		buffer[17]=0x00;//温度的整数部分
		buffer[18]=0x00;//湿度的整数部分
	}
	IWDG_Feed();  //喂狗
// 	ADXL345_Angle(&buffer[19]);	//读取X,Y,Z三个方向的加速度值 
	
	buffer[19]=RTC_Time[0];
	buffer[20]=RTC_Time[1];
	buffer[21]=RTC_Time[2];
	buffer[22]=RTC_Time[3];
	buffer[23]=RTC_Time[4];//B4将姿态数据改为同步时间
	buffer[24]=0x00;//
	
	wCRC=chkcrc(buffer,25);
	buffer[25]=wCRC/256;
	buffer[26]=wCRC%256;	
	IWDG_Feed();  //喂狗
	for(i=0;i<27;i++)
		WirelessSend(buffer[i]);	
}
//通过无线向甲板单元反馈
void SendREQToPC(u8 keyword)
{
  u8 i;	
	u16 wCRC;
	u8 sendbuffto485[10]={0x00,0x00,0x50,0x7E,0x7E,0xAC,0x01,0x01,0x00,0x00};
	sendbuffto485[5]=keyword;
	sendbuffto485[6]=0x01;
	sendbuffto485[7]=0x01;
	wCRC=chkcrc(sendbuffto485,8);
	sendbuffto485[8]=wCRC/256;
	sendbuffto485[9]=wCRC%256;
	for(i=0;i<10;i++)
		WirelessSend(sendbuffto485[i]);
}
//处理由甲板单元通过无线收到的数据
void ExecutPCCommand(void)
{
	u8 i,j;
	u8 num=0;
	u16 wCRC;
	NAND_ADDRESS WriteReadAddr;
	unsigned char  RTC_ARR[8];
	u8 sendbuffto485[10]={0x00,0x00,0x50,0x7E,0x7E,0xAC,0x01,0x01,0x00,0x00};// 0730 给主机发送，前面都有这个，主机默认为00 00 50 
	IWDG_Feed();  //喂狗
	switch(input485str[2])
	{
		case 0xAC://关闭磁测模块电源
			MEG_Disable;
		
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//添加的，给磁测断电就要关闭晶振信号的输出。
		
		  zttimeflag=0;
		  SendREQToPC(0xAC);
		  break;
		case 0xAB://开启磁测模块电源
			MEG_Enable;
      SendREQToPC(0xAB);
		  tongbu=1;
		  break;
		case 0xBA://开启磁测模块电源,不加同步信号
			MEG_Enable;
      SendREQToPC(0xBA);
		  break;
		case 0xC0:    //压载释放
			i=0;
		  MegDriver_Config();
			Delay_ms(600); 
			MegDriver_Stop();
				IWDG_Feed();  //喂狗
			while((i<3)&&(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)))
			{
// 				MegDriver_EN();
				MegDriver_Config();
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //喂狗
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);
					IWDG_Feed();  //喂狗
				}
			} 
      if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
				release=1; 			
		  break;
		case 0xAD:    //读取磁测模块漏磁阈值
			ComFromJiaBan=0;
		  QueryToMeg(0xAD);
		  break;
		case 0xAF:    //读取磁测模块甚低频阈值
			ComFromJiaBan=0;
		  QueryToMeg(0xAF);
		  break;
		case 0xAE:    //设置磁测模块漏磁阈值
			lc_data.lc_threshold[0]=input485str[4];
			lc_data.lc_threshold[1]=input485str[5];
		  ComFromJiaBan=0;
		  SetMegthreshold(0xAE);
		  SendREQToPC(input485str[2]);
		  break;
		case 0xB0:    //设置磁测模块甚低频阈值
			dp_data.dp_threshold[0]=input485str[4];
			dp_data.dp_threshold[1]=input485str[5];
			ComFromJiaBan=0;
		  SetMegthreshold(0xB0);
		  SendREQToPC(input485str[2]);
		  break;
		case 0x1C:    //查询信标水声发射功率
			QueryToWater(0x1C);
		  break;
		case 0x1D:    //设置信标水声发射功率
			Water_power=input485str[4];
			SetWaterPower();
		  IWDG_Feed();  //喂狗
		  SendREQToPC(0x1D);
		  break;
		case 0xB1://清空NandFlash数据
				/*设置NAND FLASH的写地址*/
			WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x00;
			WriteReadAddr.Page = 0x00;
			/*擦除待写入数据的块*/
			NAND_EraseBlock(WriteReadAddr);
		  IWDG_Feed();  //喂狗
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x200;
			WriteReadAddr.Page = 0x00;
			/*擦除待写入数据的块*/
			NAND_EraseBlock(WriteReadAddr);
		  IWDG_Feed();  //喂狗
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		
		  SendREQToPC(0xB1);
		
			Delay_ms(5000); //添加长延时，迫使程序复位// 		//张炳洋20161027添加开始
		
		
		break;
		case 0xB2:   //读取flash中的全部漏磁数据
		  	FromJiaBanA= 5 ;//?????????
	  	 QueryToMeg(0xA1);//?????????

		break;
		case 0xB3:   //读取flash中的全部甚低频数据
			 FromJiaBanA= 6 ;//标志位
	  	 QueryToMeg(0xA0);//查询磁测有无低频数据
		break;
		case 0xB4:   //无线读取信标状态信息
			SendStatusToPC();
		break;
		/*******无线读取工作日志**********/
		case 0xB7:
			  Read_Log();
		break;
		/*******删除工作日志**********/
		case 0xe1:
			Delete_file();
		break;
		
    case 0xB5:   //读取信标识别码
			ControlBoardID=6;
			sendbuffto485[5]=0xB5;
		  sendbuffto485[6]=0x01;
		  sendbuffto485[7]=ControlBoardID;
		  wCRC=chkcrc(sendbuffto485,8);
		  sendbuffto485[8]=wCRC/256;
		  sendbuffto485[9]=wCRC%256;
		  for(i=0;i<10;i++)
				WirelessSend(sendbuffto485[i]);
		break;
		case 0xB6:   //设置信标识别码
// 			ControlBoardID=input485str[4];
// 		  SetBoardID(ControlBoardID);//把设备识别码保存到flash,改综控板的id
// 		  SetWirelessAddr(ControlBoardID);//改变无线模块的识别码和信标的识别码相同
// 		  SendREQToPC(0xB6);//回复
		break;
		case 0xC1:
			/*
		  // input485str[4]是年份17年....input485str[5] 是月份....input485str[6]是日期...input485str[7]保留位00
		  // input485str[8]是小时....input485str[9]是分钟.... input485str[10]是秒钟..... input485str[11]是毫秒
		  */
		  RTC_ARR[0] = HEX2BCD(input485str[11]);	// enable oscillator (bit 7=0)
			RTC_ARR[1] = HEX2BCD(input485str[10]);	// enable oscillator (bit 7=0)
			RTC_ARR[2] = HEX2BCD(input485str[9]);	// minute = 59
			RTC_ARR[3] = HEX2BCD(input485str[8]);	// hour = 05 ,24-hour mode(bit 6=0)
			RTC_ARR[4] = HEX2BCD(input485str[7]);	// Day = 1 or sunday
			RTC_ARR[5] = HEX2BCD(input485str[6]);	// Date = 30
			RTC_ARR[6] = HEX2BCD(input485str[5]);	// month = August
			RTC_ARR[7] = HEX2BCD(input485str[4]);	// year = 05 or 200
		
      WriteRTC(&RTC_ARR[0],0x00,8);	// Set RTC	  
	  	USART_ITConfig(UART5,USART_IT_RXNE,ENABLE);//接收中断使能
		 
		  IWDG_Feed();  //喂狗
		  SendREQToPC(0xC1);
			
		break;
		case 0xC2://设置磁测模块上电延时时间
			megontime=(megontime&0x00000000)|input485str[4];
		  megontime=(megontime<<8)|input485str[5];
		  megontime=(megontime<<8)|input485str[6];
			MEG_Disable;
		
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//添加的，给磁测断电就要关闭晶振信号的输出。
		
		  zttimeflag=0;
		  megon=0;
		  SendREQToPC(0xC2);
		break;
		case 0xC3://设置水声模块上电延时时间
			waterontime=(waterontime&0x00000000)|input485str[4];
		  waterontime=(waterontime<<8)|input485str[5];
		  waterontime=(waterontime<<8)|input485str[6];
			OutputSourceOFF;
		  wateron=0;
		  SendREQToPC(0xC3);
		break;
		case 0x1E://关闭水声模块供电
			OutputSourceOFF;
		  SendREQToPC(0x1E);
		break;
		case 0x1F://给水声模块供电
			OutputSourceON;
		  Delay_ms(200);
		  QueryToWater(0x16);
		  SendREQToPC(0x1F);
		break;
		default:
			break;
	}
	IWDG_Feed();  //喂狗
}
//记录工作日志
void Set_Log(u8 name,u8 com,u8 b)
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 
  unsigned char  RTC_ARR[8];
	u8 buffer[26] = {0};
	IWDG_Feed();   //喂狗
	ReadRTC(&RTC_ARR[0],0x00,8); 
	buffer[0]=BCD2HEX(RTC_ARR[6])/10+0x30;
	buffer[1]=BCD2HEX(RTC_ARR[6])%10+0x30;
	buffer[2]='-';
	buffer[3]=BCD2HEX(RTC_ARR[5])/10+0x30;
	buffer[4]=BCD2HEX(RTC_ARR[5])%10+0x30;
	buffer[5]=' ';
	buffer[6]=BCD2HEX(RTC_ARR[3])/10+0x30;
	buffer[7]=BCD2HEX(RTC_ARR[3])%10+0x30;
	buffer[8]=':';
	buffer[9]=BCD2HEX(RTC_ARR[2])/10+0x30;
	buffer[10]=BCD2HEX(RTC_ARR[2])%10+0x30;
	buffer[11]=':';
	buffer[12]=BCD2HEX(RTC_ARR[1])/10+0x30;
	buffer[13]=BCD2HEX(RTC_ARR[1])%10+0x30;
	buffer[14]=':';
	buffer[15]=BCD2HEX(RTC_ARR[0])/10+0x30;
	buffer[16]=BCD2HEX(RTC_ARR[0])%10+0x30;
	buffer[17]=' ';
	if(name==0)
		buffer[18]='W';
	else 
		buffer[18]='M';
	buffer[19]=' ';
	if((buffer[20]=((u8)com>>4)&0x0F)>0x09)
		buffer[20]+=0x37;
	else
		buffer[20]+=0x30;
	if((buffer[21]=com&0x0F)>0x09)
		buffer[21]+=0x37;
	else
		buffer[21]+=0x30;
	buffer[22]=' ';
	if(b)
		buffer[23]='T';
	else 
		buffer[23]='F';
	buffer[24]='\r';
	buffer[25]='\n';
	IWDG_Feed();   //喂狗
	res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		res = f_lseek(&fsrc, f_size(&fsrc)); 
		res = f_write(&fsrc, buffer,26, &bw);
		IWDG_Feed();   //喂狗
		Delay_ms(500);
	}
	else
	{
		f_close(&fsrc);
		Delay_ms(1000);
		res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			res = f_lseek(&fsrc, f_size(&fsrc)); 
			res = f_write(&fsrc, buffer,26, &bw);
			IWDG_Feed();   //喂狗
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();   //喂狗
}

/***************设置数据长度*************************************/
/***************放在SD卡中每次都会覆盖掉上一次内容***************/
/***************核心还是上面的setlog函数*************************/
/***************读取其中数据用readloglen*************************/
void Set_Log_len(u16 loglen)
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 
	u8 Hvalue = 0,Lvalue = 0;

	u8 buffer[2] = {0};
  Hvalue = loglen>>8;
	Lvalue = loglen&0x00ff;
	IWDG_Feed();   //喂狗
  buffer[0] = Lvalue;
  buffer[1] = Hvalue;
	IWDG_Feed();   //喂狗
	res = f_open(&fsrc, "0:/log/loglen.txt", FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		                                                     //res = f_lseek(&fsrc, f_size(&fsrc)); 
		res = f_write(&fsrc, buffer,2, &bw);
		IWDG_Feed();   //喂狗
		Delay_ms(500);
	}
	else
	{
		f_close(&fsrc);
		Delay_ms(1000);
		res = f_open(&fsrc, "0:/log/loglen.txt", FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			                                                    //res = f_lseek(&fsrc, f_size(&fsrc)); 
			res = f_write(&fsrc, buffer,2, &bw);
			
			IWDG_Feed();   //喂狗
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();   //喂狗
}
/***************读取数据长度***********************************************************/
/***************并将读取的十六位数据返回，但很有可能确实存在没删除的情况***************/
/***************与上面的setloglen是一对函数********************************************/
/***************写其中数据用setloglen**************************************************/
u16 Get_Log_len(void)            //u8 *buffer1
{
	FRESULT res;                 // FatFs function common result code
	UINT bw;
	u8 id8_H = 0x00,id8_L = 0x00;
	u16 data_len;
	u8 read_log_len[2] = {0};
	IWDG_Feed();                 //喂狗
	res = f_open(&fsrc, "0:/log/loglen.txt", FA_OPEN_ALWAYS | FA_READ);	
	res = f_read(&fsrc,read_log_len,2, &bw);
  f_close(&fsrc);
  id8_L = read_log_len[0];
  id8_H = read_log_len[1];
  data_len=id8_H;
	data_len<<=8;
	data_len=id8_L;
	return data_len;
  	
					
}


////////////////////调试读取数据//////////////////////////////
/*********************************---------****************************************/
/*********************************---------****************************************/
/*                         函数名 Read_Log(void)                                  */ 
/*                         作用为读取SD卡中的log文件的数据                        */
/*                         调用函数为 f_read()                                    */
/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
void Read_Log(void)            //u8 *buffer1
{
	FRESULT res;                 // FatFs function common result code
	UINT bw;
	int i,j;
	u16 wCRC;
	u16 data_len;
	u8 read_log_buffer[34]={0x00,0x00,0x50,0x7E,0x7E,0xb7};
  u8 read_zero[8]={0x00,0x00,0x50,0x7E,0x7E,0xb7,0x00,0x00};
  data_len=Get_Log_len();
  if(data_len==0x0000){
		for(i=0;i<8;i++){	
			WirelessSend(read_zero[i]);				
			}
  }  
	IWDG_Feed();                 //喂狗
	res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_ALWAYS | FA_READ);
 if(res==FR_OK)
	{
		
		for(j=0;j<data_len;j++)
		{
			res = f_read(&fsrc,&read_log_buffer[6],26, &bw);
				wCRC=chkcrc(read_log_buffer,26);
				read_log_buffer[32]=wCRC/256;
				read_log_buffer[33]=wCRC%256;	
			for(i=0;i<34;i++){	
				WirelessSend(read_log_buffer[i]);
				
			}
			IWDG_Feed();
			Delay_ms(100);
		}
		IWDG_Feed();               //喂狗
		Delay_ms(500);
	}
	else
	{
		f_close(&fsrc);
		Delay_ms(1000);
		res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_EXISTING | FA_READ);
		if(res==FR_OK)
		{
			
		for(j=0;j<data_len;j++)
			{
					res = f_read(&fsrc,&read_log_buffer[6],26, &bw);
					wCRC=chkcrc(read_log_buffer,26);
					read_log_buffer[32]=wCRC/256;
						read_log_buffer[33]=wCRC%256;	
				for(i=0;i<34;i++){	
						WirelessSend(read_log_buffer[i]);
				f_close(&fsrc);	
			}
				IWDG_Feed();
				Delay_ms(100);
		}
		IWDG_Feed();               //喂狗
		Delay_ms(500);
			}
																									
			IWDG_Feed();               //喂狗
			Delay_ms(500);
		}
		
		//log_len=0;		
		//SetBoardID(log_len);	
																									//	return read_log_buffer;
																									//buffer1=buffer;
}
/////////////////////////////////调试读取数据结束//////////////////////////////	

/*********************************---------****************************************/
/*********************************---------****************************************/
/*                         函数名 Delete_file(void)                               */ 
/*                         作用为删除SD卡中的log文件及其数据                      */
/*                         调用函数为unlink()                                     */
/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
void Delete_file(void){
	//FRESULT res;
	u16 wCRC;
	int i;
	u8 Delete_log_buffer[10]={0x00,0x00,0x50,0x7E,0x7E,0xE1};		
	//res = 
	f_unlink("0:/log/log.txt");                 /* Pointer to the file or directory path */
	/*if(res==FR_OK){
		res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_ALWAYS | FA_READ);
	  f_close(&fsrc);
  }	*/
  
	wCRC=chkcrc(read_log_buffer,26);
	Delete_log_buffer[6]=0x66;
	Delete_log_buffer[7]=0x66;
	Delete_log_buffer[8]=wCRC/256;
	Delete_log_buffer[9]=wCRC%256;	
	for(i=0;i<10;i++){	
		WirelessSend(Delete_log_buffer[i]);		
	}
	log_len=0x0000;
	//SetBoardID(log_len);	
  Set_Log_len(0x0000);	
	
}
u8 GetBoardID(void)//得到综控板的id
{
	u8 id;
	NAND_ADDRESS WriteReadAddr;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x3FF;//最后一个块1023
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&id, WriteReadAddr, 1);
	return id;
}
void SetBoardID(u8 id)//设置综控板的ID
{
	NAND_ADDRESS WriteReadAddr;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x3FF;//最后一个块1023
	WriteReadAddr.Page = 0x00;
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&id, WriteReadAddr,1);
}
void SetWirelessAddr(u8 id)//设置无线模块信道和地址(模式3下必须为9600波特率，因此在设置信标标识码时先把波特率改为9600
{
	  GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
	
	u8 i;
	u8 temp[6]={0xC0,0x00,0x00,0x3E,0x50,0xC4};  //地址修改 信道为
	SetMode(3);

	IWDG_Feed();   //喂狗
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	USART_InitStructure.USART_BaudRate = 9600;
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
	
	
	Delay_ms(50);
	IWDG_Feed();   //喂狗
	temp[2]=id;
	temp[4]=temp[4]+id;
	for(i=0;i<6;i++)
		WirelessSend(temp[i]);
	Delay_ms(100);

IWDG_Feed();   //喂狗
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
	IWDG_Feed();   //喂狗
	
	SetMode(0);
	Delay_ms(20);
}
void SetMegthreshold(u8 com)//设置磁测模块阈值
{
	u8 i;
	u16 wCRC;
	u8 buffer[9]={0x7E,0x01,0x01,0xAE,0x02,0x00,0x00,0x00,0x00};
	buffer[1]=ControlBoardID;
	buffer[2]=ControlBoardID;
	buffer[3]=com;
	if(com==0xAE)//漏磁阈值
	{
		buffer[5]=lc_data.lc_threshold[0];
		buffer[6]=lc_data.lc_threshold[1];
	}
	else if(com==0xB0)//极低频阈值
	{
		buffer[5]=dp_data.dp_threshold[0];
		buffer[6]=dp_data.dp_threshold[1];
	}
	wCRC=chkcrc(buffer,7);
	buffer[7]=wCRC/256;
	buffer[8]=wCRC%256;
	IWDG_Feed();  //喂狗
	for(i=0;i<9;i++)
		RS232_MegSend(buffer[i]);
}
void SetWaterPower(void)//设置水声发射功率
{
	u8 i;
	u16 wCRC;
	u8 buffer[8]={0x7E,0x01,0x01,0x1D,0x01,0x00,0x00,0x00};
	buffer[1]=ControlBoardID;
	buffer[2]=ControlBoardID;
	buffer[5]=Water_power;
	wCRC=chkcrc(buffer,6);
	buffer[6]=wCRC/256;
	buffer[7]=wCRC%256;
	for(i=0;i<8;i++)
		RS232_WaterSend(buffer[i]);
}
void Statusmonitor(void)//状态检测与存储
{
	u8 i,j;
	FRESULT res;               // FatFs function common result code
	UINT bw; 
	u8 buffer[31];
	DHT11_Data_TypeDef DHT11_Data;
	unsigned char  RTC_ARR[8];
// 	ReadRTC(&RTC_ARR[0],0x00,8); //时间  日 时 分 秒 毫秒（10ms）
// 	buffer[0]=BCD2HEX(RTC_ARR[6]);
// 	buffer[1]=BCD2HEX(RTC_ARR[5]);
// 	buffer[2]=BCD2HEX(RTC_ARR[3]);
// 	buffer[3]=BCD2HEX(RTC_ARR[2]);
// 	buffer[4]=BCD2HEX(RTC_ARR[1]);
// 	buffer[5]=BCD2HEX(RTC_ARR[0]);
	ReadRTC(&RTC_ARR[0],0x00,8); 
	buffer[0]=BCD2HEX(RTC_ARR[6])/10+0x30;
	buffer[1]=BCD2HEX(RTC_ARR[6])%10+0x30;
	buffer[2]='-';
	buffer[3]=BCD2HEX(RTC_ARR[5])/10+0x30;
	buffer[4]=BCD2HEX(RTC_ARR[5])%10+0x30;
	buffer[5]=' ';
	buffer[6]=BCD2HEX(RTC_ARR[3])/10+0x30;
	buffer[7]=BCD2HEX(RTC_ARR[3])%10+0x30;
	buffer[8]=':';
	buffer[9]=BCD2HEX(RTC_ARR[2])/10+0x30;
	buffer[10]=BCD2HEX(RTC_ARR[2])%10+0x30;
	buffer[11]=':';
	buffer[12]=BCD2HEX(RTC_ARR[1])/10+0x30;
	buffer[13]=BCD2HEX(RTC_ARR[1])%10+0x30;
	buffer[14]=' ';
	IWDG_Feed();  //喂狗
	ADC_DMA_Config();
	Delay_us(500);
	buffer[15]=batterytest/1000+0x30;//电池电压整数位
	buffer[16]=batterytest/100%10+0x30;//电池电压整数位
	buffer[17]='.';
	buffer[18]=batterytest%100/10+0x30;//电池电压小数位
	buffer[19]=batterytest%10+0x30;//电池电压小数位
	buffer[20]='V';
	buffer[21]=' ';
	IWDG_Feed();  //喂狗
	if( Read_DHT11(&DHT11_Data)==SUCCESS)	
	{
		buffer[22]=DHT11_Data.temp_int/10+0x30;//温度的整数部分
		buffer[23]=DHT11_Data.temp_int%10+0x30;//温度的整数部分
		buffer[24]='C';
		buffer[25]=' ';
		buffer[26]=DHT11_Data.humi_int/10+0x30;//湿度的整数部分
		buffer[27]=DHT11_Data.humi_int%10+0x30;
		buffer[28]='%';
	}
	else
	{
		buffer[22]=0x30;//温度的整数部分
		buffer[23]=0x30;//温度的整数部分
		buffer[24]='C';
		buffer[25]=' ';
		buffer[26]=0x30;//湿度的整数部分
		buffer[27]=0x30;
		buffer[28]='%';
	}
	buffer[29]='\r';
	buffer[30]='\n';
	if((batterytest/100)<0x16)//电压低于22V解锁
	{
		i=0;
		if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
		{
			while(i<3)
			{
				MegDriver_Config();
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //喂狗
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);
					IWDG_Feed();  //喂狗
				}
			} 
		}
	}	
	storagecnt++;
	if(storagecnt==5)
	{
		storagecnt=0;
		IWDG_Feed();   //喂狗
		res = f_open(&fsrc, "0:/log/status.txt", FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			res = f_lseek(&fsrc, f_size(&fsrc)); 
			res = f_write(&fsrc, buffer,31, &bw);
			IWDG_Feed();   //喂狗
		}
		f_close(&fsrc);	
	}
}

