#include "main.h"
extern u16 log_len;
extern struct LC_Data lc_data;
extern struct DP_Data dp_data;
extern u8 Water_power;//��¼ˮ�����书��
extern u8 inputmegstr[136],inputwaterstr[8],input485str[14];//���ڽ������ݵ�����
extern u8 Rxmegflag;//��1��ʾ���ڽ������
extern u8 tongbu;//ͬ����־λΪ1����ʾ����������ɣ����Է���ͬ���ź� 
u8 FromJiaBanA=0;//���Լװ嵥Ԫ�������־   Ϊ1��ʾA0��Ϊ2��ʾA1��Ϊ3��ʾA3��Ϊ4��ʾA4
extern u8 watersendcomplet,watersendfall;//�ű�ˮ��������ɱ�־    �ű�ˮ������ʧ�ܱ�־
extern u16 pressuretest,batterytest;//��ص�ѹֵ��ѹ����ѹֵ
u8 sendtowater;//һ��׼��������������װ嵥Ԫ�������ݱ�־
FIL fsrc;//�ļ�ָ��
u8 fileztp[25];//�洢��̬���ݽ������ļ���
u32 ZTtime=0;//����������ʾ����ZTtime ��ms
u8 zttimeflag=0;//Ϊ1ʱ��time��ʼ����
u16 time=0;//���������ǵ�1000����ʾ����1ms
u8 tongbucount=0;//ͬ����������
extern u8 ControlBoardID;//�ű��豸ʶ����
u8 ComFromJiaBan=0;//Ϊ1��ʾ���������ڼװ嵥Ԫ
extern u32 waterontime,megontime;//��¼ˮ���ʹŲ��ϵ�ʱ��
extern u32 wateron,megon;//������
extern u8 detectfeedback,feedcom,detect;//������־λ    ������    ������
u8 storagecnt=0;//��ʾ����ű�״̬�Ĵ��������5��ʱ���ʹ洢��SD���У���������5�����洢״̬����
u8 release=0;//Ϊ1��ʾѹ���ͷţ���ѹ����������GPS����
extern unsigned char  RTC_Time[5];
	unsigned char  RTC_ARR[8];
u8 read_log_buffer[28] = {0}; 
//��ѯ�Ų�ģ��
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
	IWDG_Feed();   //ι��
	switch(KeyWord)
	{
		case 0xA0:    //����Ƶ����������ѯ
		case 0xA1:    //©������������ѯ
		case 0xA5:    //֪ͨ�Ų�ģ���Լ�
		case 0xA6:    //���ͬ���Ƿ�ɹ�
		case 0xAD:    //��ȡ©����ֵ
		case 0xAF:    //��ȡ����Ƶ��ֵ
			NoData[3]=KeyWord;
		  wCRC=chkcrc(NoData,5);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
		 IWDG_Feed();   //ι��
			for(i=0;i<7;i++)
				RS232_MegSend(NoData[i]);//A0-AF����ִ���������Ϊû��break
		  break;
		case 0xA7:    //�ű���յ��Ų�ģ�鴫����©������
			for(i=0;i<8;i++)
				RS232_MegSend(Data[i]);
		  break;
		case 0xA8:    //�ű���յ��Ų�ģ�鴫��������Ƶ����
			for(i=0;i<8;i++)
				RS232_MegSend(Data1[i]);
			break;
		case 0xA3:    //����Ƶ���ݶ�ȡ
			NoData[3]=0xA3;
		  wCRC=chkcrc(NoData,5);
		  NoData[5]=wCRC/256;
		  NoData[6]=wCRC%256;
			for(i=0;i<7;i++)
				RS232_MegSend(NoData[i]);
		  break;
		case 0xA4:    //©�����ݶ�ȡ
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
	IWDG_Feed();   //ι��
	if((KeyWord!=0xA7)&&(KeyWord!=0xA8))
	{
		detectfeedback=1;
		feedcom=KeyWord;
	}
}

//����Ų�ģ�鴫����������
void ExecutMegCommand(void)
{
	u8 count=0;
	u8 i,j=0;
	u8 buffertemp[130];
	u8 DipinData,LouciData;
	u16 wCRC;
	u8 sendbuffto485[11]={0x00,0x00,0x50,0x7E,0x7E,0xAD,0x02,0x00,0x00,0x00,0x00};//ͨ������ģ�鷢����Ϣ
	u8 sendbufftowater[9]={0x7E,0x01,0x00,0xAD,0x02,0x00,0x00,0x00,0x00};
	sendbufftowater[1]=ControlBoardID;
// 	detectfeedback=0;//��ʾ�յ��Ųⷢ��������Ϣ
	detect=0;
	IWDG_Feed();   //ι��
	switch(inputmegstr[3])
	{
		case 0xA0:
		detectfeedback=0;//��ʾ�յ��Ų��Ӧ��Ϣ�������ط�����
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
			if(inputmegstr[5]!=0)//����Ƶ����������Ϊ0��������
			{
				dp_data.dp_rdgroupcnt=inputmegstr[5];//�������Ƶ��������
				dp_data.DP_AllGroupCnt+=dp_data.dp_rdgroupcnt;	//����Ƶ���������ۼ�
				QueryToMeg(0xA3);//������Ƶ������������A3�����ȡ����Ƶ����
			}
			else    //����Ų�ģ���ʱû������Ƶ���ݣ������Ǽװ嵥ԪҪ��ȡ����Ƶ���ݣ��ñ�־Ϊ1���ۿذ���Կ�ʼ��������
			{
				Set_Log(1,0xA0,1);//��¼������־��Ϣ				
				log_len++;//��¼SD��log���ݵ�����
				Set_Log_len(log_len);
				
				if(FromJiaBanA==3)//����Ϊ3��ʾ�װ嵥ԪҪ������Ƶ����
					sendtowater=1;
				 if (FromJiaBanA==6)//����Ų������ݣ�������Ҫ��ȡ����
				 sendtowater=1;//���߷��;���
			}
			if(FromJiaBanA==1)//�����ʱ�װ嵥ԪҪѯ������Ƶ��������������
			{
				FromJiaBanA=0;							
				SendDataNum(dp_data.DP_AllGroupCnt,0xA0);	//��������Ƶ��������					
			}
			break;
		case 0xA1:
			detectfeedback=0;//��ʾ�յ��Ųⷢ��������Ϣ
		  Set_Log(0,0x1F,1);
				log_len++;//��¼SD��log���ݵ�����
				Set_Log_len(log_len);
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
			if(inputmegstr[5]!=0)//©������������Ϊ0��������
			{
				
				lc_data.lc_rdgroupcnt=inputmegstr[5];
				lc_data.LC_AllGroupCnt+=lc_data.lc_rdgroupcnt;		//©�����������ۼ�				
				QueryToMeg(0xA4);  //����Ų�ģ���������ݣ����Ͷ�ȡ��������
			}
			else //����Ų�ģ���ʱû��©�����ݣ������Ǽװ嵥ԪҪ��ȡ©�����ݣ��ñ�־Ϊ1���ۿذ���Կ�ʼ��������
			{
				Set_Log(1,0xA1,1);
				log_len++;//��¼SD��log���ݵ�����
				Set_Log_len(log_len);
				if(FromJiaBanA==4)//����Ϊ4��ʾ�װ嵥ԪҪ��©������
					sendtowater=1;
				 if (FromJiaBanA==5)//����Ų������ݣ�������Ҫ��ȡ����
				 sendtowater=1;//���߷��;���

			} 
			
			
			if(FromJiaBanA==2)//�����ʱ�װ嵥ԪҪѯ��©����������������
			{
				FromJiaBanA=0;							
				SendDataNum(lc_data.LC_AllGroupCnt,0xA1);		//����©����������				
			}
			break;
		case 0xA3:
      detectfeedback=0;//��ʾ�յ��Ųⷢ��������?			
			DipinData=inputmegstr[4];    //��������ֽ���
// 		  wCRC=chkcrc(inputmegstr,DipinData+5);
// 		  if((inputmegstr[DipinData+5]!=wCRC/256)||(inputmegstr[DipinData+6]!=wCRC%256))
// 				break;
		  for(i=5;i<DipinData+5;i++)       //����Ч���ݶ�ȡ����        
			{
				buffertemp[j++]=inputmegstr[i];
			}
			WriteDatatoFlash(buffertemp,DipinData,1);//�����ݴ���NandFlash��
			WriteDatatosd(dp_data.dp_filep,buffertemp,DipinData);//������Ƶ���ݴ���sd����
			j=0;
			QueryToMeg(0xA7);  //����A7��ʾ���ݽ������
			dp_data.DP_DataByteCnt+=DipinData;//���ڼ�¼NandFlash������Ƶ���ݵ��ֽ���
			if(dp_data.dp_rdgroupcnt>10)//ÿ���ۿذ����ӴŲ�ģ�����10�����ݣ�����������10 ���������ȡ
			{
				QueryToMeg(0xA3);
				dp_data.dp_rdgroupcnt=dp_data.dp_rdgroupcnt-10;
			}
			else  //��ȡ�����ݺ���1,��ʾ���ݶ��������ˮ����������
			{
				sendtowater=1;
			}
			Set_Log(1,0xA3,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0xA4:
			detectfeedback=0;//��ʾ�յ��Ųⷢ��������?
			LouciData=inputmegstr[4];
// 		  wCRC=chkcrc(inputmegstr,LouciData+5);
// 		  if((inputmegstr[LouciData+5]!=wCRC/256)||(inputmegstr[LouciData+6]!=wCRC%256))
// 				break;
			for(i=5;i<LouciData+5;i++)    //����Ч���ݶ�ȡ����                  
			{
				buffertemp[j++]=inputmegstr[i];
			}
 			WriteDatatoFlash(buffertemp,LouciData,0);//��flash��д����
			WriteDatatosd(lc_data.lc_filep,buffertemp,LouciData);//��sd����д����
			j=0;
			QueryToMeg(0xA8);//����A8��ʾ���ݽ������
   		lc_data.LC_DataByteCnt+=LouciData;//���ڼ�¼NandFlash��©�����ݵ��ֽ���
			if(lc_data.lc_rdgroupcnt>10)//ÿ���ۿذ����ӴŲ�ģ�����10�����ݣ�����������10 ���������ȡ
			{
				QueryToMeg(0xA4);
				lc_data.lc_rdgroupcnt=lc_data.lc_rdgroupcnt-10;
			}
			else   //��ȡ�����ݺ���1,��ʾ���ݶ��������ˮ����������
			{
				sendtowater=1;
			}
			Set_Log(1,0xA4,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0xA5://�Ų�ģ���Լ�ķ���
			Set_Log(1,0xA5,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0xA6:
			detectfeedback=0;//��ʾ�յ��Ųⷢ��������Ϣ
// 			wCRC=chkcrc(inputmegstr,6);
// 		  if((inputmegstr[6]!=wCRC/256)||(inputmegstr[7]!=wCRC%256))
// 				break;
	
			if(inputmegstr[5]!=0)
			{
 				CreateNewArea(1);//ͬ����ɺ���NandFlash�н���һ��������������ͬ���������Ϊ1 ��ʾ��������Ƶ��
				CreateNewArea(0);//����©����
				CreateNewFile(1);//ͬ����ɺ���sd���н���һ�����ļ���������ͬ�����  ����Ƶ���� 
				CreateNewFile(0);//ͬ����ɺ���sd���н���һ�����ļ���������ͬ����� ©������	
        CreateNewFile(2);	//ͬ����ɺ���sd���н���һ�����ļ��������̬����			
				lc_data.LC_DataByteCnt=0;//��¼©�������ֽ�������0
				dp_data.DP_DataByteCnt=0;//��¼����Ƶ�����ֽ�������0
				
			}
			else
			{
				tongbucount++;//��ͬ�����ɹ������ٷ�������ͬ���ź�
				GPIO_SetBits(GPIOG,GPIO_Pin_15);//ͬ���źŸ�һ���ߵ�ƽ������
				Delay_ms(1000);	
				IWDG_Feed();  //ι��
				Delay_ms(1000);	
				IWDG_Feed();   //ι��
				GPIO_ResetBits(GPIOG,GPIO_Pin_15);
				Delay_ms(1000);	
				IWDG_Feed();  //ι��
				Delay_ms(1000);	
				IWDG_Feed();   //ι��
				if(tongbucount<2)	//���Ų�ģ��û�н��յ�ͬ���źţ���������������ͬ���ź�	
					QueryToMeg(0xA6);
				else
					tongbucount=0;
			}	
			Set_Log(1,0xA6,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0xA9://��ʾ�Ų�ģ��洢������Ƶ���ݴﵽ25�飬����Ҫ���ۿذ��ȡ
			Set_Log(1,0xA9,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
		 
			QueryToMeg(0xA0);
		 
			break;
		case 0xAA://��ʾ�Ų�ģ��洢��©�����ݴﵽ25�飬����Ҫ���ۿذ��ȡ
			Set_Log(1,0xAA,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			QueryToMeg(0xA1);
			break;
		case 0xAD:  //��ȡ©����ֵ�󣬴Ų�ģ�鷵��©����ֵ�����ۿذ��ϴ�
		case 0xAF:  //��ȡ����Ƶ��ֵ�󣬴Ų�ģ�鷵��©����ֵ�����ۿذ��ϴ�
			detectfeedback=0;//��ʾ�յ��Ųⷢ��������?
			Set_Log(1,inputmegstr[3],1);
			if(ComFromJiaBan)//���Լװ嵥Ԫ������
			{
				sendbufftowater[3]=inputmegstr[3];
				sendbufftowater[4]=0x02;
				sendbufftowater[5]=inputmegstr[5];
				sendbufftowater[6]=inputmegstr[6];
				wCRC=chkcrc(sendbufftowater,7);
				sendbufftowater[7]=wCRC/256;
				sendbufftowater[8]=wCRC%256;
				IWDG_Feed();  //ι��
				if(watersendcomplet!=0)
					watersendcomplet=0;
				
				for(i=0;i<9;i++)
					RS232_WaterSend(sendbufftowater[i]);//ͨ��ˮ��ģ�鷢����Ϣ
				
				while((watersendcomplet!=2)&&(count<15))//�ȴ�ˮ��ͨ��ģ�����η��������ȴ�30s
				{
					count++;
					Delay_ms(1000);	
					IWDG_Feed();  //ι��
					Delay_ms(1000);	
					IWDG_Feed();   //ι��
				}
				count=0;				
				watersendcomplet=0;
			}
			else//��������ģ�������
			{
				sendbuffto485[5]=inputmegstr[3];
				sendbuffto485[6]=0x02;
				sendbuffto485[7]=inputmegstr[5];
				sendbuffto485[8]=inputmegstr[6];
				wCRC=chkcrc(sendbuffto485,9);
				sendbuffto485[9]=wCRC/256;
				sendbuffto485[10]=wCRC%256;
				IWDG_Feed();  //ι��
				for(i=0;i<11;i++)
					WirelessSend(sendbuffto485[i]);//ͨ������ģ�鷢��
			}
	}
}
//����ˮ��ģ���״̬
void QueryToWater(u8 KeyWord)
{
	u8 i;
	u8 NoData[7]={0x7E,0x01,0x01,0x12,0x00,0x00,0x00};
	u16 wCRC;
	NoData[1]=ControlBoardID;
	NoData[2]=ControlBoardID;
	IWDG_Feed();  //ι��
	switch(KeyWord)
	{
		case 0x12:    //ʹˮ��ͨ��ģ�����͹���
		case 0x14:  //�����ű�ˮ��ģ��
		case 0x16:  //�ű�ˮ��ģ���Լ�
		case 0x1C:   //��ȡ�ű�ˮ���ķ��书��
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
	IWDG_Feed();  //ι��
}
void ExecutWaterCommand(void)//ִ���ű�ˮ�����͹���������
{
	u8 i,j;
	u8 num=0;
	u8 FeedBack[8];
	u8 sendbuffto485[10]={0x00,0x00,0x50,0x7E,0x7E,0x1C,0x01,0x00,0x00,0x00};
	u16 wCRC;
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();  //ι��
	if((inputwaterstr[1]==0x00)&&(inputwaterstr[2]==ControlBoardID))//�յ�ˮ��ģ�鷢�͹��������ݺ��ۿ���ˮ��ģ�鷴��
	 {
			for(i=0;i<4;i++)
				FeedBack[i]=inputwaterstr[i];
			FeedBack[4]=0x01;
			FeedBack[5]=0x01;
			wCRC=chkcrc(FeedBack,6);
			FeedBack[6]=wCRC/256;
			FeedBack[7]=wCRC%256;
			IWDG_Feed();  //ι��
			for(i=0;i<4;i++)
			{
				Delay_ms(1000);	
				IWDG_Feed();  //ι��
			}
			for(i=0;i<8;i++)
				RS232_WaterSend(FeedBack[i]);
	 } 
	switch(inputwaterstr[3])
	{
		case 0x12:    //ʹˮ��ͨ��ģ�����͹���
			Set_Log(0,0x12,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0x14:  //�����ű�ˮ��ģ��
			Set_Log(0,0x14,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0x16:  //�ű�ˮ��ģ���Լ�
			Set_Log(0,0x16,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			break;
		case 0x1C:   //��ȡ�ű�ˮ���ķ��书��
			Set_Log(0,0x1C,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			sendbuffto485[5]=0x1C;
		  sendbuffto485[7]=inputwaterstr[5];
		  wCRC=chkcrc(sendbuffto485,8);
		  sendbuffto485[8]=wCRC/256;
		  sendbuffto485[9]=wCRC%256;
		  IWDG_Feed();  //ι��
			for(i=0;i<10;i++)
				WirelessSend(sendbuffto485[i]);
		  break;
		case 0xC0://ѹ���ͷ�
			Set_Log(0,0xC0,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			i=0;
		  MegDriver_Config();//����BTN7971
			Delay_ms(600); 
			MegDriver_Stop();
			IWDG_Feed();  //ι��
			while((i<3)&&(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)))//�ж��ͷſ����Ƿ��ͷţ����û���ͷ�������3��
			{
				MegDriver_Config();//����BTN7971
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //ι��
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);	
					IWDG_Feed();  //ι��
				}				
			}  
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
				release=1; 			
		  break;
		case 0xA0:
			Set_Log(0,0xA0,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
		  FromJiaBanA=1;
		  QueryToMeg(0xA0);
		  break;
		case 0xA1:
			Set_Log(0,0xA1,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
		  FromJiaBanA=2;
		  QueryToMeg(0xA1);
		  break;
		case 0xA3:
			Set_Log(0,0xA3,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
		  FromJiaBanA=3;
		  QueryToMeg(0xA0);
		  break;
		case 0xA4:
			Set_Log(0,0xA4,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
		  FromJiaBanA=4;
		  QueryToMeg(0xA1);
		  break;
		case 0xAB://���Ų��ϵ�
			Set_Log(0,0xAB,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			MEG_Enable;
		  SendREQToWater(0xAB);//��װ嵥Ԫ����
		  tongbu=1;//ͬ����־��1
// 		  QueryToMeg(0xA5);
// 		  Delay_ms(1000);
// 			SetMegthreshold(0xAE);//����©����ֵ
			Delay_ms(1000);
// 			SetMegthreshold(0xB0);//��������Ƶ��ֵ
		  break;
		case 0xAC:
			Set_Log(0,0xAC,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			MEG_Disable;
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//��ӵģ����Ų�ϵ��Ҫ�رվ����źŵ������
		
		  zttimeflag=0;
		  SendREQToWater(0xAC);
		  break;
		case 0xB4://ˮ����ȡ�ۿذ�״̬��Ϣ
			Set_Log(0,0xB4,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			SendStatus();
		  break;
		case 0xB1://���NandFlash����
			Set_Log(0,0xB1,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
				/*����NAND FLASH��д��ַ*/
			WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x00;
			WriteReadAddr.Page = 0x00;
		  IWDG_Feed();  //ι��
			/*������д�����ݵĿ�*/
			NAND_EraseBlock(WriteReadAddr);//���©�����ݴ洢��falsh��ַ
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  IWDG_Feed();  //ι��
		  WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x200;
			WriteReadAddr.Page = 0x00;
			/*������д�����ݵĿ�*/
			NAND_EraseBlock(WriteReadAddr);//�������Ƶ���ݴ洢��falsh��ַ
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  lc_data.LC_AllGroupCnt=0;
		  dp_data.DP_AllGroupCnt=0;
		  IWDG_Feed();  //ι��
		  SendREQToWater(0xB1);//��װ嵥Ԫ����
		Delay_ms(5000); //��ӳ���ʱ����ʹ����λ// 		//�ű���20161027��ӿ�ʼ
		break;
		case 0xAD:    //��ȡ�Ų�ģ��©����ֵ
			Set_Log(0,0xAD,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  QueryToMeg(0xAD);
		  break;
		case 0xAF:    //��ȡ�Ų�ģ������Ƶ��ֵ
			Set_Log(0,0xAF,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  QueryToMeg(0xAF);
		  break;
		case 0xAE:    //���ôŲ�ģ��©����ֵ
			lc_data.lc_threshold[0]=inputwaterstr[5];
			lc_data.lc_threshold[1]=inputwaterstr[6];
		  Set_Log(0,0xAE,1);
			log_len++;//��¼SD��log���ݵ�����
			Set_Log_len(log_len);
			ComFromJiaBan=1;
		  SetMegthreshold(0xAE);
		  SendREQToWater(inputwaterstr[3]);
		break;
		case 0xB0:    //���ôŲ�ģ������Ƶ��ֵ
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
	IWDG_Feed();  //ι��
}
//����״̬��Ϣ
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
	ReadRTC(&RTC_ARR[0],0x00,8); //ʱ��  �� ʱ �� �� ���루10ms��
	buffer[5]=BCD2HEX(RTC_ARR[6]);
	buffer[6]=BCD2HEX(RTC_ARR[5]);
	buffer[7]=BCD2HEX(RTC_ARR[3]);
	buffer[8]=BCD2HEX(RTC_ARR[2]);
	buffer[9]=BCD2HEX(RTC_ARR[1]);
	buffer[10]=BCD2HEX(RTC_ARR[0]);
	IWDG_Feed();  //ι��
	ADC_DMA_Config();
	Delay_us(500);
	buffer[11]=batterytest/100;//��ص�ѹ����λ
	buffer[12]=batterytest%100;//��ص�ѹС��λ
//��Ӽ���ѹ��䣬�������31V������Ϊ�Ǵ���ɼ�����ֵΪ0.
  
 if(buffer[11]>=0x20)//������31V������Ϊ������0
	{
		buffer[11]=0;
		buffer[12]=0;
	}
	
	
// 	if(buffer[12]>=0x28)//һ��С��λ��0.4V�����ֶ�У��
// 		buffer[12]-=0x28;
// 	 else 
// 	 {
// 		 buffer[11]=buffer[11]-1;
// 		 buffer[12]=buffer[12]+0x3C;
// 	 }
	 
	buffer[13]=pressuretest/100;//ѹ��
	buffer[14]=pressuretest%100;
	IWDG_Feed();  //ι��
	if( Read_DHT11(&DHT11_Data)==SUCCESS)	
	{
		buffer[15]=DHT11_Data.temp_int;//�¶ȵ���������
		buffer[16]=DHT11_Data.humi_int;//ʪ�ȵ���������
	}
	else
	{
		buffer[15]=0x00;//�¶ȵ���������
		buffer[16]=0x00;//ʪ�ȵ���������
	}
// 	ADXL345_Angle(&buffer[17]);	//��ȡX,Y,Z��������ļ��ٶ�ֵ 
	
	buffer[17]=RTC_Time[0];
	buffer[18]=RTC_Time[1];
	buffer[19]=RTC_Time[2];
	buffer[20]=RTC_Time[3];
	buffer[21]=RTC_Time[4];//B4����̬���ݸ�Ϊͬ��ʱ��
	buffer[22]=0x00;//Ϊ��֤�װ嵥Ԫ���Խ��棬������ȷ�����һ��00
	
	wCRC=chkcrc(buffer,23);
	buffer[23]=wCRC/256;
	buffer[24]=wCRC%256;	
	IWDG_Feed();  //ι��
	if(watersendcomplet!=0)
		watersendcomplet=0;
	
	for(i=0;i<25;i++)
		RS232_WaterSend(buffer[i]);	
	while((watersendcomplet!=2)&&(count<15))
	{
		count++;
		Delay_ms(1000);	
		IWDG_Feed();  //ι��
		Delay_ms(1000);	
		IWDG_Feed();  //ι��
	}
	count=0;
	watersendcomplet=0;
}
//ͨ��ˮ����װ嵥Ԫ������Ϣ
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
		IWDG_Feed();  //ι��
		Delay_ms(1000);	
		IWDG_Feed();  //ι��
	}
	count=0;
	watersendcomplet=0;
}

//������������
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
  IWDG_Feed();  //ι��
	if(watersendcomplet!=0)
		watersendcomplet=0;
	
	for(i=0;i<8;i++)
		RS232_WaterSend(JiaBanFeedBack[i]);
	
	while((watersendcomplet!=2)&&(count<15))
	{
		count++;
		Delay_ms(1000);
		IWDG_Feed();  //ι��
		Delay_ms(1000);
		IWDG_Feed();  //ι��
	}
	count=0;
	watersendcomplet=0;
}
//��NandFlash��һ�����е�ҳ�ж�ȡ���ݷ���
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
	IWDG_Feed();  //ι��
	readbuffer[1]=ControlBoardID;
	readbuffer[3]=com;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(&datagroupcount, WriteReadAddr,1);//�õ���һҳ����������
	IWDG_Feed();  //ι��
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block-1;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(tempbuff, WriteReadAddr,datagroupcount*13);//�õ���һҳ������
	IWDG_Feed();  //ι��
	buff=tempbuff;
	sendcount=datagroupcount*13/52;//ÿ����෢�͵ĴŲ������ֽ���Ϊ52���ֽ� 4�� ���ڵķ�������
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
		IWDG_Feed();  //ι��
		if(watersendcomplet!=0)
		watersendcomplet=0;
		
		for(i=0;i<64;i++)
			RS232_WaterSend(readbuffer[i]);
		IWDG_Feed();  //ι��
		while((watersendcomplet!=2)&&(count<15))
		{
			count++;
			Delay_ms(1000);
			IWDG_Feed();  //ι��
			Delay_ms(1000);
			IWDG_Feed();  //ι��
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
		IWDG_Feed();  //ι��
		
		if(watersendcomplet!=0)
		watersendcomplet=0;
		
		for(i=0;i<sendyushu+12;i++)
			RS232_WaterSend(readbuffer[i]);
		IWDG_Feed();  //ι��
		while((watersendcomplet!=2)&&(count<15))
		{
			count++;
			Delay_ms(1000);
			IWDG_Feed();  //ι��
			Delay_ms(1000);
			IWDG_Feed();  //ι��
		}
		count=0;	
		watersendcomplet=0;
	}	
}

//��������
void SendData(u16 rdblocknum,u32 fsizebyte,u8 com)
{
	u8 flag;
	u8 pagenum,blocknum;
	u8 i;
	u8 datagroupcount;
	u16 datacount=0;
	u8 timebuffer[5];
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();  //ι��
	if(com==0xA4)
		flag=0;
	else if(com==0xA3)
		flag=1;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);
	IWDG_Feed();  //ι��
	while(rdblocknum<=blocknum)//��ȡ�����ݵ�flash����С���������ݵ�flash����
	{
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = rdblocknum*3-1+0x200*flag;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(timebuffer, WriteReadAddr, 5);//�õ�ʱ����Ϣ
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = rdblocknum*3-2+0x200*flag;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);//�ó�ҳ��
		IWDG_Feed();  //ι��
		if(fsizebyte==0)//ƫ�Ƶ���0
		{
			for(i=1;i<pagenum;i++)
				ReadDataFromPage(rdblocknum*3+0x200*flag,i,timebuffer,com);
		}
		else 
		{
			 for(i=1;i<pagenum;i++)//�ҳ�û�ж�����ҳ��
			 {
					WriteReadAddr.Zone = 0x00;
					WriteReadAddr.Block = rdblocknum*3+0x200*flag;
					WriteReadAddr.Page = i;
					NAND_ReadBuffer(&datagroupcount, WriteReadAddr, 1);  
					IWDG_Feed();  //ι��
					datacount+=datagroupcount*13;
					if(datacount==fsizebyte)
						break;		 
			 }
			 for(i++;i<pagenum;i++)//��������
					ReadDataFromPage(rdblocknum*3+0x200*flag,i,timebuffer,com);
	   }
		 if(rdblocknum<blocknum)
				fsizebyte=0;
		 rdblocknum++;
	 }
	 if(flag==0)//��ʾ©�������й�
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

void CreateNewFile(u8 flag)//����һ���µ��ļ�
{
	unsigned char  RTC_ARR[8],RTC_Time[5];
	u8 fname[9] = {0};
	IWDG_Feed();  //ι��
	ReadRTC(&RTC_ARR[0],0x00,8); 
	fname[0]=BCD2HEX(RTC_ARR[6])/10+0x30;//��
	fname[1]=BCD2HEX(RTC_ARR[6])%10+0x30;
	fname[2]=BCD2HEX(RTC_ARR[5])/10+0x30;//��
	fname[3]=BCD2HEX(RTC_ARR[5])%10+0x30;
	fname[4]=BCD2HEX(RTC_ARR[3])/10+0x30;//ʱ
	fname[5]=BCD2HEX(RTC_ARR[3])%10+0x30;
	fname[6]=BCD2HEX(RTC_ARR[2])/10+0x30;//��
	fname[7]=BCD2HEX(RTC_ARR[2])%10+0x30;
	fname[8]='\0';
	IWDG_Feed();  //ι��
	RTC_Time[0]=BCD2HEX(RTC_ARR[5]);//��
	RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
	RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
	RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
	RTC_Time[4]=BCD2HEX(RTC_ARR[0]);
	switch(flag)
	{
		case 0://©�������ļ�
			sprintf((char*)lc_data.lc_filep,"0:/lc/%s.bin",fname);
			IWDG_Feed();  //ι��
			WriteDatatosd(lc_data.lc_filep,RTC_Time,5);
		break;
		case 1://����Ƶ�����ļ�
			sprintf((char*)dp_data.dp_filep,"0:/dp/%s.bin",fname);
			IWDG_Feed();  //ι��
			WriteDatatosd(dp_data.dp_filep,RTC_Time,5);		
		break;
		case 2://��̬�����ļ�
			sprintf((char*)fileztp,"0:/zt/%s.bin",fname);
		  IWDG_Feed();  //ι��
			WriteDatatosd(fileztp,RTC_Time,5);
			ZTtime=0;
			time=0;
			zttimeflag=1;
		break;
	}
}
void WriteDatatosd(u8 *tmp_name,u8 *buffertemp,u8 datalen)//������д��SD����
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 

	res = f_open(&fsrc, (char*)tmp_name, FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		res = f_lseek(&fsrc, f_size(&fsrc)); 
		IWDG_Feed();  //ι��
		res = f_write(&fsrc, buffertemp,datalen, &bw);
	}
	else
	{
		f_close(&fsrc);
		IWDG_Feed();  //ι��
		Delay_ms(1000);
		res = f_open(&fsrc, (char*)tmp_name, FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			res = f_lseek(&fsrc, f_size(&fsrc)); 
			IWDG_Feed();  //ι��
			res = f_write(&fsrc, buffertemp,datalen, &bw);
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();  //ι��
}

//��NandFlash��һ�����е�ҳ�ж�ȡ����ͨ������ģ�鷢��
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
	NAND_ReadBuffer(&datagroupcount, WriteReadAddr,1);//�õ���һҳ����������
	tempbuff[3]=datagroupcount*13+5;//©�������ֽ���+ʱ��5���ֽ�
	IWDG_Feed();  //ι��
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = block-1;
	WriteReadAddr.Page = page;
	NAND_ReadBuffer(&tempbuff[9], WriteReadAddr,datagroupcount*13);//�õ���һҳ������
	IWDG_Feed();  //ι��
	for(i=0;i<5;i++)
			tempbuff[4+i]=timebuffer[i];
	wCRC=chkcrc(tempbuff,datagroupcount*13+9);
	tempbuff[datagroupcount*13+9]=wCRC/256;
	tempbuff[datagroupcount*13+10]=wCRC%256;
	
	buff=tempbuff;
	sendcount=(datagroupcount*13+11)/58;//55����Ϊ58. ����ģ��һ�δ�58�ֽ�
	sendyushu=(datagroupcount*13+11)%58;
	while(sendcount--)
	{
		for(i=0;i<58;i++)
			readbuff[3+i]=*buff++;
		IWDG_Feed();  //ι��		
		for(i=0;i<61;i++)
			WirelessSend(readbuff[i]);
		Delay_ms(50);//ԭ����ʱ30ms
		IWDG_Feed();  //ι��
	}
	if(sendyushu)
	{
		for(i=0;i<sendyushu;i++)
			readbuff[3+i]=*buff++;
		IWDG_Feed();  //ι��
		for(i=0;i<sendyushu+3;i++)
			WirelessSend(readbuff[i]);
		IWDG_Feed();  //ι��
	}	
}
void ReadAllDataToPC(u8 com)//ͨ�����߷�������ʱʹ��
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
	IWDG_Feed();  //ι��
	for(i=1;i<=blocknum;i++)
	{
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block = i*3-1+flag*0x200;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(timebuffer, WriteReadAddr, 5);//���ʱ����Ϣ
		WriteReadAddr.Zone = 0x00;
		WriteReadAddr.Block =i*3-2+flag*0x200;
		WriteReadAddr.Page = 0x00;
		NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);	//��������ж���ҳ
		IWDG_Feed();  //ι��
		for(j=1;j<pagenum;j++)
		{
			ReadDataFromPageToPC(i*3+flag*0x200,j,timebuffer,com);
			Delay_ms(100);
		}			
	}
}
//ͨ������ģ����װ嵥Ԫ����״̬��Ϣ
void SendStatusToPC(void)
{
	u8 i;
	u16 wCRC;
	u8 buffer[27]={0x00,0x00,0x50,0x7E,0x7E,0xB4,0x12};
	DHT11_Data_TypeDef DHT11_Data;
	unsigned char  RTC_ARR[8];
	ReadRTC(&RTC_ARR[0],0x00,8); //��ȡʱ����Ϣ
	buffer[7]=BCD2HEX(RTC_ARR[6]);
	buffer[8]=BCD2HEX(RTC_ARR[5]);
	buffer[9]=BCD2HEX(RTC_ARR[3]);
	buffer[10]=BCD2HEX(RTC_ARR[2]);
	buffer[11]=BCD2HEX(RTC_ARR[1]);
	buffer[12]=BCD2HEX(RTC_ARR[0]);
	IWDG_Feed();  //ι    ��
	ADC_DMA_Config();
	Delay_us(500); 
	buffer[13]=batterytest/100;//��ص�ѹ����λ
	buffer[14]=batterytest%100;//��ص�ѹС��λ

 if(buffer[13]>=0x20)//������31V������Ϊ������0
	{
		buffer[13]=0;
		buffer[14]=0;
	}
	
// 	
// 	if(buffer[14]>=0x28)//һ��С��λ��0.4V�����ֶ�У��
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
		buffer[17]=DHT11_Data.temp_int;//�¶ȵ���������
		buffer[18]=DHT11_Data.humi_int;//ʪ�ȵ���������
	}
	else
	{
		buffer[17]=0x00;//�¶ȵ���������
		buffer[18]=0x00;//ʪ�ȵ���������
	}
	IWDG_Feed();  //ι��
// 	ADXL345_Angle(&buffer[19]);	//��ȡX,Y,Z��������ļ��ٶ�ֵ 
	
	buffer[19]=RTC_Time[0];
	buffer[20]=RTC_Time[1];
	buffer[21]=RTC_Time[2];
	buffer[22]=RTC_Time[3];
	buffer[23]=RTC_Time[4];//B4����̬���ݸ�Ϊͬ��ʱ��
	buffer[24]=0x00;//
	
	wCRC=chkcrc(buffer,25);
	buffer[25]=wCRC/256;
	buffer[26]=wCRC%256;	
	IWDG_Feed();  //ι��
	for(i=0;i<27;i++)
		WirelessSend(buffer[i]);	
}
//ͨ��������װ嵥Ԫ����
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
//�����ɼװ嵥Ԫͨ�������յ�������
void ExecutPCCommand(void)
{
	u8 i,j;
	u8 num=0;
	u16 wCRC;
	NAND_ADDRESS WriteReadAddr;
	unsigned char  RTC_ARR[8];
	u8 sendbuffto485[10]={0x00,0x00,0x50,0x7E,0x7E,0xAC,0x01,0x01,0x00,0x00};// 0730 ���������ͣ�ǰ�涼�����������Ĭ��Ϊ00 00 50 
	IWDG_Feed();  //ι��
	switch(input485str[2])
	{
		case 0xAC://�رմŲ�ģ���Դ
			MEG_Disable;
		
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//��ӵģ����Ų�ϵ��Ҫ�رվ����źŵ������
		
		  zttimeflag=0;
		  SendREQToPC(0xAC);
		  break;
		case 0xAB://�����Ų�ģ���Դ
			MEG_Enable;
      SendREQToPC(0xAB);
		  tongbu=1;
		  break;
		case 0xBA://�����Ų�ģ���Դ,����ͬ���ź�
			MEG_Enable;
      SendREQToPC(0xBA);
		  break;
		case 0xC0:    //ѹ���ͷ�
			i=0;
		  MegDriver_Config();
			Delay_ms(600); 
			MegDriver_Stop();
				IWDG_Feed();  //ι��
			while((i<3)&&(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)))
			{
// 				MegDriver_EN();
				MegDriver_Config();
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //ι��
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);
					IWDG_Feed();  //ι��
				}
			} 
      if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
				release=1; 			
		  break;
		case 0xAD:    //��ȡ�Ų�ģ��©����ֵ
			ComFromJiaBan=0;
		  QueryToMeg(0xAD);
		  break;
		case 0xAF:    //��ȡ�Ų�ģ������Ƶ��ֵ
			ComFromJiaBan=0;
		  QueryToMeg(0xAF);
		  break;
		case 0xAE:    //���ôŲ�ģ��©����ֵ
			lc_data.lc_threshold[0]=input485str[4];
			lc_data.lc_threshold[1]=input485str[5];
		  ComFromJiaBan=0;
		  SetMegthreshold(0xAE);
		  SendREQToPC(input485str[2]);
		  break;
		case 0xB0:    //���ôŲ�ģ������Ƶ��ֵ
			dp_data.dp_threshold[0]=input485str[4];
			dp_data.dp_threshold[1]=input485str[5];
			ComFromJiaBan=0;
		  SetMegthreshold(0xB0);
		  SendREQToPC(input485str[2]);
		  break;
		case 0x1C:    //��ѯ�ű�ˮ�����书��
			QueryToWater(0x1C);
		  break;
		case 0x1D:    //�����ű�ˮ�����书��
			Water_power=input485str[4];
			SetWaterPower();
		  IWDG_Feed();  //ι��
		  SendREQToPC(0x1D);
		  break;
		case 0xB1://���NandFlash����
				/*����NAND FLASH��д��ַ*/
			WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x00;
			WriteReadAddr.Page = 0x00;
			/*������д�����ݵĿ�*/
			NAND_EraseBlock(WriteReadAddr);
		  IWDG_Feed();  //ι��
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		  WriteReadAddr.Zone = 0x00;
			WriteReadAddr.Block = 0x200;
			WriteReadAddr.Page = 0x00;
			/*������д�����ݵĿ�*/
			NAND_EraseBlock(WriteReadAddr);
		  IWDG_Feed();  //ι��
			NAND_WriteBuffer(&num, WriteReadAddr, 1);
		
		  SendREQToPC(0xB1);
		
			Delay_ms(5000); //��ӳ���ʱ����ʹ����λ// 		//�ű���20161027��ӿ�ʼ
		
		
		break;
		case 0xB2:   //��ȡflash�е�ȫ��©������
		  	FromJiaBanA= 5 ;//?????????
	  	 QueryToMeg(0xA1);//?????????

		break;
		case 0xB3:   //��ȡflash�е�ȫ������Ƶ����
			 FromJiaBanA= 6 ;//��־λ
	  	 QueryToMeg(0xA0);//��ѯ�Ų����޵�Ƶ����
		break;
		case 0xB4:   //���߶�ȡ�ű�״̬��Ϣ
			SendStatusToPC();
		break;
		/*******���߶�ȡ������־**********/
		case 0xB7:
			  Read_Log();
		break;
		/*******ɾ��������־**********/
		case 0xe1:
			Delete_file();
		break;
		
    case 0xB5:   //��ȡ�ű�ʶ����
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
		case 0xB6:   //�����ű�ʶ����
// 			ControlBoardID=input485str[4];
// 		  SetBoardID(ControlBoardID);//���豸ʶ���뱣�浽flash,���ۿذ��id
// 		  SetWirelessAddr(ControlBoardID);//�ı�����ģ���ʶ������ű��ʶ������ͬ
// 		  SendREQToPC(0xB6);//�ظ�
		break;
		case 0xC1:
			/*
		  // input485str[4]�����17��....input485str[5] ���·�....input485str[6]������...input485str[7]����λ00
		  // input485str[8]��Сʱ....input485str[9]�Ƿ���.... input485str[10]������..... input485str[11]�Ǻ���
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
	  	USART_ITConfig(UART5,USART_IT_RXNE,ENABLE);//�����ж�ʹ��
		 
		  IWDG_Feed();  //ι��
		  SendREQToPC(0xC1);
			
		break;
		case 0xC2://���ôŲ�ģ���ϵ���ʱʱ��
			megontime=(megontime&0x00000000)|input485str[4];
		  megontime=(megontime<<8)|input485str[5];
		  megontime=(megontime<<8)|input485str[6];
			MEG_Disable;
		
		GPIO_ResetBits(GPIOG,GPIO_Pin_15);//��ӵģ����Ų�ϵ��Ҫ�رվ����źŵ������
		
		  zttimeflag=0;
		  megon=0;
		  SendREQToPC(0xC2);
		break;
		case 0xC3://����ˮ��ģ���ϵ���ʱʱ��
			waterontime=(waterontime&0x00000000)|input485str[4];
		  waterontime=(waterontime<<8)|input485str[5];
		  waterontime=(waterontime<<8)|input485str[6];
			OutputSourceOFF;
		  wateron=0;
		  SendREQToPC(0xC3);
		break;
		case 0x1E://�ر�ˮ��ģ�鹩��
			OutputSourceOFF;
		  SendREQToPC(0x1E);
		break;
		case 0x1F://��ˮ��ģ�鹩��
			OutputSourceON;
		  Delay_ms(200);
		  QueryToWater(0x16);
		  SendREQToPC(0x1F);
		break;
		default:
			break;
	}
	IWDG_Feed();  //ι��
}
//��¼������־
void Set_Log(u8 name,u8 com,u8 b)
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 
  unsigned char  RTC_ARR[8];
	u8 buffer[26] = {0};
	IWDG_Feed();   //ι��
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
	IWDG_Feed();   //ι��
	res = f_open(&fsrc, "0:/log/log.txt", FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		res = f_lseek(&fsrc, f_size(&fsrc)); 
		res = f_write(&fsrc, buffer,26, &bw);
		IWDG_Feed();   //ι��
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
			IWDG_Feed();   //ι��
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();   //ι��
}

/***************�������ݳ���*************************************/
/***************����SD����ÿ�ζ��Ḳ�ǵ���һ������***************/
/***************���Ļ��������setlog����*************************/
/***************��ȡ����������readloglen*************************/
void Set_Log_len(u16 loglen)
{
	FRESULT res;               // FatFs function common result code
	UINT bw; 
	u8 Hvalue = 0,Lvalue = 0;

	u8 buffer[2] = {0};
  Hvalue = loglen>>8;
	Lvalue = loglen&0x00ff;
	IWDG_Feed();   //ι��
  buffer[0] = Lvalue;
  buffer[1] = Hvalue;
	IWDG_Feed();   //ι��
	res = f_open(&fsrc, "0:/log/loglen.txt", FA_OPEN_ALWAYS | FA_WRITE);
	
	if(res==FR_OK)
	{
		                                                     //res = f_lseek(&fsrc, f_size(&fsrc)); 
		res = f_write(&fsrc, buffer,2, &bw);
		IWDG_Feed();   //ι��
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
			
			IWDG_Feed();   //ι��
			Delay_ms(500);
		}
	}
	f_close(&fsrc);	
	IWDG_Feed();   //ι��
}
/***************��ȡ���ݳ���***********************************************************/
/***************������ȡ��ʮ��λ���ݷ��أ������п���ȷʵ����ûɾ�������***************/
/***************�������setloglen��һ�Ժ���********************************************/
/***************д����������setloglen**************************************************/
u16 Get_Log_len(void)            //u8 *buffer1
{
	FRESULT res;                 // FatFs function common result code
	UINT bw;
	u8 id8_H = 0x00,id8_L = 0x00;
	u16 data_len;
	u8 read_log_len[2] = {0};
	IWDG_Feed();                 //ι��
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


////////////////////���Զ�ȡ����//////////////////////////////
/*********************************---------****************************************/
/*********************************---------****************************************/
/*                         ������ Read_Log(void)                                  */ 
/*                         ����Ϊ��ȡSD���е�log�ļ�������                        */
/*                         ���ú���Ϊ f_read()                                    */
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
	IWDG_Feed();                 //ι��
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
		IWDG_Feed();               //ι��
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
		IWDG_Feed();               //ι��
		Delay_ms(500);
			}
																									
			IWDG_Feed();               //ι��
			Delay_ms(500);
		}
		
		//log_len=0;		
		//SetBoardID(log_len);	
																									//	return read_log_buffer;
																									//buffer1=buffer;
}
/////////////////////////////////���Զ�ȡ���ݽ���//////////////////////////////	

/*********************************---------****************************************/
/*********************************---------****************************************/
/*                         ������ Delete_file(void)                               */ 
/*                         ����Ϊɾ��SD���е�log�ļ���������                      */
/*                         ���ú���Ϊunlink()                                     */
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
u8 GetBoardID(void)//�õ��ۿذ��id
{
	u8 id;
	NAND_ADDRESS WriteReadAddr;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x3FF;//���һ����1023
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&id, WriteReadAddr, 1);
	return id;
}
void SetBoardID(u8 id)//�����ۿذ��ID
{
	NAND_ADDRESS WriteReadAddr;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x3FF;//���һ����1023
	WriteReadAddr.Page = 0x00;
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&id, WriteReadAddr,1);
}
void SetWirelessAddr(u8 id)//��������ģ���ŵ��͵�ַ(ģʽ3�±���Ϊ9600�����ʣ�����������ű��ʶ��ʱ�ȰѲ����ʸ�Ϊ9600
{
	  GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
	
	u8 i;
	u8 temp[6]={0xC0,0x00,0x00,0x3E,0x50,0xC4};  //��ַ�޸� �ŵ�Ϊ
	SetMode(3);

	IWDG_Feed();   //ι��
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
	IWDG_Feed();   //ι��
	temp[2]=id;
	temp[4]=temp[4]+id;
	for(i=0;i<6;i++)
		WirelessSend(temp[i]);
	Delay_ms(100);

IWDG_Feed();   //ι��
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
	IWDG_Feed();   //ι��
	
	SetMode(0);
	Delay_ms(20);
}
void SetMegthreshold(u8 com)//���ôŲ�ģ����ֵ
{
	u8 i;
	u16 wCRC;
	u8 buffer[9]={0x7E,0x01,0x01,0xAE,0x02,0x00,0x00,0x00,0x00};
	buffer[1]=ControlBoardID;
	buffer[2]=ControlBoardID;
	buffer[3]=com;
	if(com==0xAE)//©����ֵ
	{
		buffer[5]=lc_data.lc_threshold[0];
		buffer[6]=lc_data.lc_threshold[1];
	}
	else if(com==0xB0)//����Ƶ��ֵ
	{
		buffer[5]=dp_data.dp_threshold[0];
		buffer[6]=dp_data.dp_threshold[1];
	}
	wCRC=chkcrc(buffer,7);
	buffer[7]=wCRC/256;
	buffer[8]=wCRC%256;
	IWDG_Feed();  //ι��
	for(i=0;i<9;i++)
		RS232_MegSend(buffer[i]);
}
void SetWaterPower(void)//����ˮ�����书��
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
void Statusmonitor(void)//״̬�����洢
{
	u8 i,j;
	FRESULT res;               // FatFs function common result code
	UINT bw; 
	u8 buffer[31];
	DHT11_Data_TypeDef DHT11_Data;
	unsigned char  RTC_ARR[8];
// 	ReadRTC(&RTC_ARR[0],0x00,8); //ʱ��  �� ʱ �� �� ���루10ms��
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
	IWDG_Feed();  //ι��
	ADC_DMA_Config();
	Delay_us(500);
	buffer[15]=batterytest/1000+0x30;//��ص�ѹ����λ
	buffer[16]=batterytest/100%10+0x30;//��ص�ѹ����λ
	buffer[17]='.';
	buffer[18]=batterytest%100/10+0x30;//��ص�ѹС��λ
	buffer[19]=batterytest%10+0x30;//��ص�ѹС��λ
	buffer[20]='V';
	buffer[21]=' ';
	IWDG_Feed();  //ι��
	if( Read_DHT11(&DHT11_Data)==SUCCESS)	
	{
		buffer[22]=DHT11_Data.temp_int/10+0x30;//�¶ȵ���������
		buffer[23]=DHT11_Data.temp_int%10+0x30;//�¶ȵ���������
		buffer[24]='C';
		buffer[25]=' ';
		buffer[26]=DHT11_Data.humi_int/10+0x30;//ʪ�ȵ���������
		buffer[27]=DHT11_Data.humi_int%10+0x30;
		buffer[28]='%';
	}
	else
	{
		buffer[22]=0x30;//�¶ȵ���������
		buffer[23]=0x30;//�¶ȵ���������
		buffer[24]='C';
		buffer[25]=' ';
		buffer[26]=0x30;//ʪ�ȵ���������
		buffer[27]=0x30;
		buffer[28]='%';
	}
	buffer[29]='\r';
	buffer[30]='\n';
	if((batterytest/100)<0x16)//��ѹ����22V����
	{
		i=0;
		if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
		{
			while(i<3)
			{
				MegDriver_Config();
				Delay_ms(600); 
				MegDriver_Stop();
				IWDG_Feed();  //ι��
				i++;
				for(j=0;j<3;j++)
				{
					Delay_ms(1000);
					IWDG_Feed();  //ι��
				}
			} 
		}
	}	
	storagecnt++;
	if(storagecnt==5)
	{
		storagecnt=0;
		IWDG_Feed();   //ι��
		res = f_open(&fsrc, "0:/log/status.txt", FA_OPEN_ALWAYS | FA_WRITE);
		if(res==FR_OK)
		{
			res = f_lseek(&fsrc, f_size(&fsrc)); 
			res = f_write(&fsrc, buffer,31, &bw);
			IWDG_Feed();   //ι��
		}
		f_close(&fsrc);	
	}
}

