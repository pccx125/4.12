#include "main.h"

// #define FLASH_BASE  ((uint32_t)0x08000000)//iap����  485�������
#define BUFFER_SIZE         512
struct LC_Data lc_data;
struct DP_Data dp_data;
u8 Water_power=0x03;//����ˮ���ϵ繦��Ϊ03
extern u16 log_len;//��¼SD��log���ݵ�����
FATFS fs;   
u8 ControlBoardID;//�ű��豸ʶ����
extern u8 fileztp[25];//��¼�ļ���
u8 FileNum=0;//��¼�ļ�������
extern u32 ZTtime;//��¼��̬���������ͬ��ʱ������ʱ��
extern u8 zttimeflag;//Ϊ1��ʾ�Ѿ�ͬ����ZTtime��ʼ����
NAND_ADDRESS WriteReadAddr;//flash��ַ����
extern u8 Rxmegflag,Rxwaterflag;//���ڽ�����ɱ�־
extern u8 sendtowater;//Ϊ1��ʾ������ˮ����������
u8 tongbu;//ͬ����־��Ϊ1����ʼ��ͬ���ź�
extern u8 FromJiaBanA;//��¼�װ嵥Ԫ������������  Ϊ1��ʾA0��Ϊ2��ʾA1��Ϊ3��ʾA3��Ϊ4��ʾA4��5��ʾB2��6��ʾB3
extern u8 RX_uart4flag;//����4������ɱ�־
extern u8 release;//Ϊ1��ʾѹ���ͷ�
void NVIC_Configuration(void);        //�ж����ú���
void GPIO_EXTI_Config(void);           //IO�ڳ�ʼ��
u32 waterontime=10,megontime=10;//��¼ˮ���ʹŲ��ϵ�ʱ��

u32 wateron=0,megon=0;//������
u8 detectfeedback=0,feedcom,detect=0;//���ڼ���ۿذ���Ų�ģ�鷢�����ݺ󣬴Ų�ģ���Ƿ��з�����detectfeedback�жϱ�־��feedcom���������detect��ʱ����

extern unsigned char  RTC_Time[5];
extern float jingdu,weidu;//��γ��
extern char s1,s2,s3,s4;
extern u8  Hour,Min,Sec,Day,Month,Year,GPS_VA,weidu_dir,jingdu_dir;//������Ϣ����Ч��Ч��־λ
extern unsigned char  RTC_ARR[8];//ʵʱʱ������ʱ����Ϣ 
extern u8 flag_rx;//��λ���ݽ�����ϱ�־λ
/*   ������    */
int main(void)
{	   	
	u8 meg_syn=0;
	FRESULT res;
	UINT bw;
  FIL fsrc;//�ļ�ָ��	
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
	SysTick_Configuration();//�δ�ʱ�ӳ�ʼ��
	
// 	SCB->VTOR = FLASH_BASE | 0x10000;//IAP����  485������ӣ������ص���FLASH��ַΪ8010000  ��
	
  GPIO_EXTI_Config();//IO�ڣ��ⲿ�жϳ�ʼ��
	NVIC_Configuration();//�ж����ȼ�����
	IWDG_Init(IWDG_Prescaler_32,0xBB8);//0xBB8=3000  ��40KHzԼΪ2.4s ���Ź���ʼ��    32*3000/40K
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  //�������˸�λ�������λ��־λ
    RCC_ClearFlag();
// 	GPSSource24V_OUT();//GPS��λ�ϵ�
	BeepON;//��������
	Delay_ms(1000);
	IWDG_Feed();   //ι��
	Delay_ms(1000);
	IWDG_Feed();   //ι��
	BeepOFF;
	MEG_Disable;
// 	GPSSource_Stop();
	Led_GPIO_Config();//led�Ƴ�ʼ��
	I2C_M41T81_Config();//ʵʱʱ�ӳ�ʼ��
  disk_initialize(0);	//�ļ�ϵͳ��ʼ��
	DHT11_GPIO_Config();//��ʪ�ȴ�������ʼ��
  Init_ADXL345();                 //��ʼ��ADXL345 
	WirelessUsart_Init();//����ģ�鴮�ڳ�ʼ��
	WirelessGPIO_Config();//����ģ��IO�ڳ�ʼ��
	SetMode(0);//��������ģ��Ϊģʽ0
	ADC_GPIO_Configuration();//ADC��ʼ��
  NAND_Init();/*������SRAM���ӵ�FSMC BANK2 NAND*/
  NAND_Reset();//��λһ��NandFlash
  RS232_Init();//RS232���ڳ�ʼ��
	RS485_Init();
	IWDG_Feed();    //ι��
	ReadRTC(&RTC_Alarm_ARR[0],0x0A,5);	//ʵʱʱ���ڲ��Ĵ���HTλ��0 ��ʵʱʱ�ӿ�ʼ��ʱ����0A��ʼ��ȡ 5��
	RTC_Alarm_ARR[2]&=0xBF;//����������ĵ�����Ԫ�ص�D6��0   ��������0C��BF��10111111 ���룬��HTΪ0
	WriteByte(RTC_Alarm_ARR[2],0x0C);   //д��
	res = f_mount(0, &fs);//ע��
	ADXL345_Read_Average(&x,&y,&z,5);//��ü��ٶ�����
// 	res=f_mkdir((char *)filename);
//	SetWirelessAddr(0x07);
	LED1( 1 ); 
	Delay_ms(800);     
	IWDG_Feed();    //ι��
	
	lc_data.LC_AllGroupCnt=0;//©���йصı�����ʼ��
	lc_data.LC_DataByteCnt=0;
	lc_data.lc_fsizebyte=0;
	lc_data.lc_rdgroupcnt=0;
	lc_data.lc_blocknum=1;
	lc_data.lc_threshold[0]=0x02;
	lc_data.lc_threshold[1]=0xCC;
	
	dp_data.DP_AllGroupCnt=0;//����Ƶ�йصı�����ʼ��
	dp_data.DP_DataByteCnt=0;
	dp_data.dp_fsizebyte=0;
	dp_data.dp_rdgroupcnt=0;
	dp_data.dp_blocknum=1;
	dp_data.dp_threshold[0]=0x02;
	dp_data.dp_threshold[1]=0xCC;
	
// 	ControlBoardID=01;//��flash�л���豸ʶ����
	
	log_len=Get_Log_len();
	while(1)
	{ 
		
  
		
		ControlBoardID=3;//��flash�л���豸ʶ����
		if(flag_rx==0)//�����λ����δ�յ���������ִ��
		{
					if(wateron<=waterontime)//waterontimeָ��ˮ���ϵ�ʱ�䣬��λΪ��
						wateron++;
					
					if(wateron==waterontime)//������������ָ����ֵ����ˮ���ϵ�
					{			
						OutputSourceON;//����BTS555��ˮ��ģ���ϵ�
						Delay_ms(1000);
						QueryToWater(0x16);//�����Լ����ʹˮ��ģ��֪���ۿص�ʶ����
						IWDG_Feed();   //ι��
						SetWaterPower();//ˮ���ϵ���豸ˮ�����书��
			// 			GPSSource_Config();
					}
					
					if(megon<=megontime)//megontimeָ���Ų��ϵ�ʱ�䣬��λΪ��
						megon++;
					
					if(megon==megontime)//������������ָ����ֵ�����Ų��ϵ�
					{				
						MEG_Enable;//�Ų�ģ���ϵ�
						tongbu=1;//ͬ����tongbu=0���Ų�����������̵�������
						Delay_ms(1000);
// 						SetMegthreshold(0xAE);//���ôŲ�ģ���©����ֵ//�Ų��Ѿ���Ӵ洢��ֵ�Ĺ���
						Delay_ms(1000);
// 						SetMegthreshold(0xB0);//���ôŲ�ģ�������Ƶ��ֵ
					}		
					IWDG_Feed();   //ι��		
					LED2( 0 );
					Delay_ms(1000);		// 1000ms   1s
					LED2( 1 );

					if(zttimeflag)//��ʾ�Ѿ�ͬ������¼��̬���ݣ���ͬ��ʱ���ű����ڶԴŲ�����û��Ӱ�죬�����¼
					{
						ztcnt++;
						if(ztcnt>=30)
						{
							ztcnt=0;
							IWDG_Feed();    //ι��
							res = f_open(&fsrc,(char *)fileztp, FA_OPEN_ALWAYS | FA_WRITE);
							res = f_lseek(&fsrc, f_size(&fsrc));
							IWDG_Feed();    //ι��
							ADXL345_RD(tempbuf);	//��ȡ10����̬���ݣ���ȥ������Ĵ���		
							while(1)
							{
								ADXL345_RD_XYZ2(buf);//���һ����̬����
								/*�����˲�����*/
								tempbuf[Filter++]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[5]<<8)+buf[4]); 
								IWDG_Feed();    //ι��
								if(Filter==30)
									Filter=0; //�Ƚ��ȳ�������ƽ��ֵ
								
								a=tempbuf[i%30]-tempbuf[(i-3)%30];//X��ȥ���»�ȡ�����ݵ�����
								b=tempbuf[(i+3)%30]-tempbuf[i%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[i%30]=(tempbuf[(i-3)%30]+tempbuf[(i+3)%30])/2;
								IWDG_Feed();    //ι��
								a=tempbuf[j%30]-tempbuf[(j-3)%30];//Y��ȥ���»�ȡ�����ݵ�����
								b=tempbuf[(j+3)%30]-tempbuf[j%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[j%30]=(tempbuf[(j-3)%30]+tempbuf[(j+3)%30])/2;
								
								a=tempbuf[k%30]-tempbuf[(k-3)%30];//Z��ȥ���»�ȡ�����ݵ�����
								b=tempbuf[(k+3)%30]-tempbuf[k%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[k%30]=(tempbuf[(k-3)%30]+tempbuf[(k+3)%30])/2;
								IWDG_Feed();    //ι��
								for(count=0;count<30;)
								{
									tx+=tempbuf[count++];
									ty+=tempbuf[count++];
									tz+=tempbuf[count++];
								}
								tx=tx/10;//ȡƽ��ֵ��Ϊһ������
								ty=ty/10;
								tz=tz/10;
								buffer[0]=(u8)(tx&0x00FF);
								buffer[1]=(u8)((tx>>8)&0x00FF);
								buffer[2]=(u8)(ty&0x00FF);
								buffer[3]=(u8)((ty>>8)&0x00FF);
								IWDG_Feed();    //ι��
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
								IWDG_Feed();    //ι��
								res = f_write(&fsrc,buffer,10, &bw);
								ztgroupcnt++;/////////////////
								if(ztgroupcnt==200)//////ÿ�μ�¼200�����ݺ��˳�
								{
									ztgroupcnt=0;/////
									IWDG_Feed();    //ι��
									f_close(&fsrc);	
									break;
								}
								IWDG_Feed();    //ι��		
								if(detectfeedback||RX_uart4flag||Rxwaterflag||Rxmegflag||tongbu)//���������źŵ��������˳�
								{
									f_close(&fsrc);	
									break;
								}
							}
						}
					}
					
					
					statuscount++;
					if(statuscount==10)//ÿ10��洢һ��״̬��Ϣ
					{
						statuscount=0;
						Statusmonitor();
					}
					if(detectfeedback)//Ϊ1��ʾ�ۿذ���Ų�ģ�鷢��������
					{
						detect++;
						if(detect==6)//��ʱ������6��ʾ����6s��û���յ��Ų�ķ���������Ų�ϵ磩
						{
							detect=0;
							detectcount++;
							detectfeedback=0;
							IWDG_Feed();   //ι��
							Set_Log(1,feedcom,0);//��¼�˴������ַ���ʧ��
							log_len++;//��¼SD��log���ݵ�����
			        Set_Log_len(log_len);
							IWDG_Feed();   //ι��
							if(detectcount==3)//�������η���ʧ�ܺ󣬸��Ų�ģ��ص磬�����ϵ�
							{
								detectcount=0;
								MEG_Disable;
								
								GPIO_ResetBits(GPIOG,GPIO_Pin_15);//��ӵģ����Ų�ϵ��Ҫ�رվ����źŵ������
								
								Delay_ms(1000);	
								IWDG_Feed();   //ι��
								Delay_ms(1000);	
								IWDG_Feed();   //ι��
								Delay_ms(1000);	
								IWDG_Feed();   //ι��
								Delay_ms(1000);	
								IWDG_Feed();   //ι��
								Delay_ms(1000);	
								IWDG_Feed();   //ι��
								
								
								MEG_Enable;
								tongbu=1;
							}
							else
								QueryToMeg(feedcom);//û���յ��Ų�ķ��������ٴη�������
						}
					}
					if(RX_uart4flag)//����4����������ɣ������ߴ������
				 {
					 RX_uart4flag=0;
					 ExecutPCCommand();//�������ߴ��������
				 }
					if(Rxwaterflag)//ˮ�����ݽ������
					{
						IWDG_Feed();   //ι��
						Rxwaterflag=0;
						ExecutWaterCommand();	 //����ˮ������
					}
					if(Rxmegflag)//�Ų����ݽ������
					{
						Rxmegflag=0;
						ExecutMegCommand();//����Ų�����
					}
					if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))//����
							GPSSource_Stop(); 
					if(release)//ѹ���ͷ���
					{
						release=0;
						Delay_ms(1000);	
						IWDG_Feed();   //ι��
						Delay_ms(1000);	
						IWDG_Feed();   //ι��
// 						MEG_Disable;//0702���ģ��Ų�ϵ��Ա���gps���յ���γ���ź�
						zttimeflag=0;
						tongbu=0;
						Delay_ms(1000);	
						IWDG_Feed();   //ι��
// 						GPSSource24V_OUT();//
					}
					
					if(tongbu)//ͬ����־λΪ1����ʾ����������ɣ����Է���ͬ���ź�  
					{ 
						Delay_ms(1000);	//�����ʱ
						IWDG_Feed();  //ι��
						if(meg_syn<=30)//���Ų��ϵ�󣬹�30����ٸ�ͬ���ź�
							meg_syn++;
						if(meg_syn==30)
						{	
							meg_syn=0;
							tongbu=0;	//���Ų�δͬ��ʱ�ϵ磬��tongbu��ȻΪ1������ᷢͬ���źţ�Ȼ���ⷴ������Ϊ�Ų�ϵ������޷�������˳�����ôŲ�����		
							GPIO_SetBits(GPIOG,GPIO_Pin_15);//���Ų�һ�������أ���Ϊͬ���ź�
							Set_Log(1,0xD0,1);//�¼ӵ�DO,�ļ���¼
							log_len++;//��¼SD��log���ݵ�����
			        Set_Log_len(log_len);
							ReadRTC(&RTC_ARR[0],0x00,8); 
							RTC_Time[0]=BCD2HEX(RTC_ARR[5]);
							RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
							RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
							RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
							RTC_Time[4]=BCD2HEX(RTC_ARR[0]);//2017��1��6��ӣ�ͬ��֮��ſ�ʼ,����0.5S�ӳ�
							
							Delay_ms(1000);	
							IWDG_Feed();  //ι��

							Delay_ms(1000);	
							IWDG_Feed();   //ι��
// 							GPIO_ResetBits(GPIOG,GPIO_Pin_15); //ע�͵�����Ϊ��Ϊͬ���źź;���������Ϊ�Ų�ʱ�ӵ����룬��ʼͬ��֮��Ҫ���ָߵ�ƽ�����ź�������Ų⡣
							
							
							
							Delay_ms(1000);	
							IWDG_Feed();   //ι��
							Delay_ms(1000);	
							IWDG_Feed();   //ι��
							//QueryToMeg(0xA6);//��ѯͬ���Ƿ�ɹ�
						}			
					}	
					
					IWDG_Feed();    //ι��
					if(sendtowater)//Ϊ1 ��ʾ������ˮ��ģ�鷢������
					{
						sendtowater=0;
						if(FromJiaBanA==4)//����ΪA4ˮ��
						{
							FromJiaBanA=0;
							SendData(lc_data.lc_blocknum,lc_data.lc_fsizebyte,0xA4);	//ͨ��ˮ��ͨ��ģ�鷢������								
						}
						if(FromJiaBanA==3)//����ΪA3
						{
							FromJiaBanA=0;
							SendData(dp_data.dp_blocknum,dp_data.dp_fsizebyte,0xA3);	
						}

            if(FromJiaBanA==5)//���߶�ȡ�Ų�©������
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB2);//ͨ�����߷���flash����
		      	}

           if(FromJiaBanA==6)//���߶�ȡ�Ų��Ƶ����
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB3);//ͨ�����߷���flash����
		      	}
					}
		
				}
		
		if(flag_rx==1)//������ܶ�λ��Ϣ�ɹ���־λΪ1������봦������������������������Ӱ����ʱ����
		{  flag_rx = 0;
			 GPS_NMEA();//���յ���GPS���ݽ���ת��
				if(GPS_VA)//���յĶ�λ������Ч
				{	
					GPS_VA=0;
					USART_ITConfig(UART5,USART_IT_RXNE,DISABLE);//�����ж�ʧ��
					RTC_ARR[0] = 0;	// enable oscillator (bit 7=0)
					RTC_ARR[1] = HEX2BCD(Sec);	// enable oscillator (bit 7=0)
					RTC_ARR[2] = HEX2BCD(Min);	// minute = 59
					RTC_ARR[3] = HEX2BCD(Hour);	// hour = 05 ,24-hour mode(bit 6=0)
					RTC_ARR[4] = 1;	// Day = 1 or sunday
					RTC_ARR[5] = HEX2BCD(Day);	// Date = 30
					RTC_ARR[6] = HEX2BCD(Month);	// month = August
					RTC_ARR[7] = HEX2BCD(Year);	// year = 05 or 200
					WriteRTC(&RTC_ARR[0],0x00,8);	// Set RTC	  
					
					IWDG_Feed();  //ι��
					SendREQToPC(0xC1);//�ط���ʱ�ɹ�				
				}
				
			}
	}
}

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits��ռ���ȼ�1λ�������ȼ�3λ */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* ����1�ж����� */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* ����3�ж����� */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
	/* DMA�ж����� */
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
	/* ����4�ж����� */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* ����5�ж����� */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//�����ȼ�
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void GPIO_EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//���ôŲ�ģ��ͬ��IO�ڣ��������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//BTS555����
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//����ˮ���˿ڵ�ʹ�ܶ�IO��
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//���÷���������IO��
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  //����IO�ڳ�ʼ��  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//�����λ�ü��IO�������ж�
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);//�����ж���
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);//�����ж���
	
	
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



