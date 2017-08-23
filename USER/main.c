#include "main.h"

// #define FLASH_BASE  ((uint32_t)0x08000000)//iapÏÂÔØ  485ÏÂÔØÌí¼Ó
#define BUFFER_SIZE         512
struct LC_Data lc_data;
struct DP_Data dp_data;
u8 Water_power=0x03;//ÉèÖÃË®ÉùÉÏµç¹¦ÂÊÎª03
extern u16 log_len;//¼ÇÂ¼SD¿¨logÊı¾İµÄÌõÊı
FATFS fs;   
u8 ControlBoardID;//ĞÅ±êÉè±¸Ê¶±ğÂë
extern u8 fileztp[25];//¼ÇÂ¼ÎÄ¼şÃû
u8 FileNum=0;//¼ÇÂ¼ÎÄ¼şµÄ×ÜÊı
extern u32 ZTtime;//¼ÇÂ¼×ËÌ¬Êı¾İÏà¶ÔÓÚÍ¬²½Ê±¼äµÄÏà¶ÔÊ±¼ä
extern u8 zttimeflag;//Îª1±íÊ¾ÒÑ¾­Í¬²½£¬ZTtime¿ªÊ¼¼ÆÊı
NAND_ADDRESS WriteReadAddr;//flashµØÖ·±äÁ¿
extern u8 Rxmegflag,Rxwaterflag;//´®¿Ú½ÓÊÕÍê³É±êÖ¾
extern u8 sendtowater;//Îª1±íÊ¾¿ÉÒÔÏòË®Éù·¢ËÍÊı¾İ
u8 tongbu;//Í¬²½±êÖ¾£¬Îª1£¬¿ªÊ¼·¢Í¬²½ĞÅºÅ
extern u8 FromJiaBanA;//¼ÇÂ¼¼×°åµ¥Ôª·¢¹ıÀ´µÄÃüÁî  Îª1±íÊ¾A0£¬Îª2±íÊ¾A1£¬Îª3±íÊ¾A3£¬Îª4±íÊ¾A4£¬5±íÊ¾B2£¬6±íÊ¾B3
extern u8 RX_uart4flag;//´®¿Ú4½ÓÊÕÍê³É±êÖ¾
extern u8 release;//Îª1±íÊ¾Ñ¹ÔØÊÍ·Å
void NVIC_Configuration(void);        //ÖĞ¶ÏÅäÖÃº¯Êı
void GPIO_EXTI_Config(void);           //IO¿Ú³õÊ¼»¯
u32 waterontime=10,megontime=10;//¼ÇÂ¼Ë®ÉùºÍ´Å²âÉÏµçÊ±¼ä

u32 wateron=0,megon=0;//¼ÆÊıÆ÷
u8 detectfeedback=0,feedcom,detect=0;//ÓÃÓÚ¼ì²â×Û¿Ø°åÏò´Å²âÄ£¿é·¢ËÍÊı¾İºó£¬´Å²âÄ£¿éÊÇ·ñÓĞ·´À¡¡£detectfeedbackÅĞ¶Ï±êÖ¾£¬feedcom·´À¡µÄÃüÁî£¬detect¼ÆÊ±±äÁ¿

extern unsigned char  RTC_Time[5];
extern float jingdu,weidu;//¾­Î³¶È
extern char s1,s2,s3,s4;
extern u8  Hour,Min,Sec,Day,Month,Year,GPS_VA,weidu_dir,jingdu_dir;//ÈÕÆÚĞÅÏ¢¼°ÓĞĞ§ÎŞĞ§±êÖ¾Î»
extern unsigned char  RTC_ARR[8];//ÊµÊ±Ê±ÖÓËùÓÃÊ±¼äĞÅÏ¢ 
extern u8 flag_rx;//¶¨Î»Êı¾İ½ÓÊÕÍê±Ï±êÖ¾Î»
/*   Ö÷º¯Êı    */
int main(void)
{	   	
	u8 meg_syn=0;
	FRESULT res;
	UINT bw;
  FIL fsrc;//ÎÄ¼şÖ¸Õë	
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
	SysTick_Configuration();//µÎ´ğÊ±ÖÓ³õÊ¼»¯
	
// 	SCB->VTOR = FLASH_BASE | 0x10000;//IAPÏÂÔØ  485ÏÂÔØÌí¼Ó£¨¸ÄÏÂÔØµ½µÄFLASHµØÖ·Îª8010000  £©
	
  GPIO_EXTI_Config();//IO¿Ú£¬Íâ²¿ÖĞ¶Ï³õÊ¼»¯
	NVIC_Configuration();//ÖĞ¶ÏÓÅÏÈ¼¶´¦Àí
	IWDG_Init(IWDG_Prescaler_32,0xBB8);//0xBB8=3000  °´40KHzÔ¼Îª2.4s ¿´ÃÅ¹·³õÊ¼»¯    32*3000/40K
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  //Èô²úÉúÁË¸´Î»£¬Çå³ı¸´Î»±êÖ¾Î»
    RCC_ClearFlag();
// 	GPSSource24V_OUT();//GPS¶¨Î»ÉÏµç
	BeepON;//·äÃùÆ÷Ïì
	Delay_ms(1000);
	IWDG_Feed();   //Î¹¹·
	Delay_ms(1000);
	IWDG_Feed();   //Î¹¹·
	BeepOFF;
	MEG_Disable;
// 	GPSSource_Stop();
	Led_GPIO_Config();//ledµÆ³õÊ¼»¯
	I2C_M41T81_Config();//ÊµÊ±Ê±ÖÓ³õÊ¼»¯
  disk_initialize(0);	//ÎÄ¼şÏµÍ³³õÊ¼»¯
	DHT11_GPIO_Config();//ÎÂÊª¶È´«¸ĞÆ÷³õÊ¼»¯
  Init_ADXL345();                 //³õÊ¼»¯ADXL345 
	WirelessUsart_Init();//ÎŞÏßÄ£¿é´®¿Ú³õÊ¼»¯
	WirelessGPIO_Config();//ÎŞÏßÄ£¿éIO¿Ú³õÊ¼»¯
	SetMode(0);//ÉèÖÃÎŞÏßÄ£¿éÎªÄ£Ê½0
	ADC_GPIO_Configuration();//ADC³õÊ¼»¯
  NAND_Init();/*ÅäÖÃÓëSRAMÁ¬½ÓµÄFSMC BANK2 NAND*/
  NAND_Reset();//¸´Î»Ò»ÏÂNandFlash
  RS232_Init();//RS232´®¿Ú³õÊ¼»¯
	RS485_Init();
	IWDG_Feed();    //Î¹¹·
	ReadRTC(&RTC_Alarm_ARR[0],0x0A,5);	//ÊµÊ±Ê±ÖÓÄÚ²¿¼Ä´æÆ÷HTÎ»ÖÃ0 £¬ÊµÊ±Ê±ÖÓ¿ªÊ¼¼ÆÊ±£¬´Ó0A¿ªÊ¼¶ÁÈ¡ 5¸ö
	RTC_Alarm_ARR[2]&=0xBF;//ÏàÓëÈÃÊı×éµÄµÚÈı¸öÔªËØµÄD6ÖÃ0   µÚÈı¸ö¼´0CÓëBF¼´10111111 ÏàÓë£¬ÈÃHTÎª0
	WriteByte(RTC_Alarm_ARR[2],0x0C);   //Ğ´Èë
	res = f_mount(0, &fs);//×¢²á
	ADXL345_Read_Average(&x,&y,&z,5);//»ñµÃ¼ÓËÙ¶ÈÊı¾İ
// 	res=f_mkdir((char *)filename);
//	SetWirelessAddr(0x07);
	LED1( 1 ); 
	Delay_ms(800);     
	IWDG_Feed();    //Î¹¹·
	
	lc_data.LC_AllGroupCnt=0;//Â©´ÅÓĞ¹ØµÄ±äÁ¿³õÊ¼»¯
	lc_data.LC_DataByteCnt=0;
	lc_data.lc_fsizebyte=0;
	lc_data.lc_rdgroupcnt=0;
	lc_data.lc_blocknum=1;
	lc_data.lc_threshold[0]=0x02;
	lc_data.lc_threshold[1]=0xCC;
	
	dp_data.DP_AllGroupCnt=0;//ÉõµÍÆµÓĞ¹ØµÄ±äÁ¿³õÊ¼»¯
	dp_data.DP_DataByteCnt=0;
	dp_data.dp_fsizebyte=0;
	dp_data.dp_rdgroupcnt=0;
	dp_data.dp_blocknum=1;
	dp_data.dp_threshold[0]=0x02;
	dp_data.dp_threshold[1]=0xCC;
	
// 	ControlBoardID=01;//´ÓflashÖĞ»ñµÃÉè±¸Ê¶±ğÂë
	
	log_len=Get_Log_len();
	while(1)
	{ 
		
  
		
		ControlBoardID=3;//´ÓflashÖĞ»ñµÃÉè±¸Ê¶±ğÂë
		if(flag_rx==0)//Èç¹û¶¨Î»Êı¾İÎ´ÊÕµ½£¬ÔòÕı³£Ö´ĞĞ
		{
					if(wateron<=waterontime)//waterontimeÖ¸¶¨Ë®ÉùÉÏµçÊ±¼ä£¬µ¥Î»ÎªÃë
						wateron++;
					
					if(wateron==waterontime)//¼ÆÊıÆ÷¼ÆÊıµ½Ö¸¶¨µÄÖµ£¬¸øË®ÉùÉÏµç
					{			
						OutputSourceON;//¿ØÖÆBTS555¸øË®ÉùÄ£¿éÉÏµç
						Delay_ms(1000);
						QueryToWater(0x16);//·¢ËÍ×Ô¼ìÃüÁî£¬Ê¹Ë®ÉùÄ£¿éÖªµÀ×Û¿ØµÄÊ¶±ğÂë
						IWDG_Feed();   //Î¹¹·
						SetWaterPower();//Ë®ÉùÉÏµçºó£¬Éè±¸Ë®Éù·¢Éä¹¦ÂÊ
			// 			GPSSource_Config();
					}
					
					if(megon<=megontime)//megontimeÖ¸¶¨´Å²âÉÏµçÊ±¼ä£¬µ¥Î»ÎªÃë
						megon++;
					
					if(megon==megontime)//¼ÆÊıÆ÷¼ÆÊıµ½Ö¸¶¨µÄÖµ£¬¸ø´Å²âÉÏµç
					{				
						MEG_Enable;//´Å²âÄ£¿éÉÏµç
						tongbu=1;//Í¬²½ºó£¬tongbu=0£¬´Å²â°åºìµÆÔÚÁÁ£¬ÂÌµÆÔÚÉÁ¡£
						Delay_ms(1000);
// 						SetMegthreshold(0xAE);//ÉèÖÃ´Å²âÄ£¿éµÄÂ©´ÅãĞÖµ//´Å²âÒÑ¾­Ìí¼Ó´æ´¢ãĞÖµµÄ¹¦ÄÜ
						Delay_ms(1000);
// 						SetMegthreshold(0xB0);//ÉèÖÃ´Å²âÄ£¿éµÄÉõµÍÆµãĞÖµ
					}		
					IWDG_Feed();   //Î¹¹·		
					LED2( 0 );
					Delay_ms(1000);		// 1000ms   1s
					LED2( 1 );

					if(zttimeflag)//±íÊ¾ÒÑ¾­Í¬²½£¬¼ÇÂ¼×ËÌ¬Êı¾İ£¬²»Í¬²½Ê±£¬ĞÅ±êµÄÇã°Ú¶Ô´Å²âÊı¾İÃ»ÓĞÓ°Ïì£¬²»Ğè¼ÇÂ¼
					{
						ztcnt++;
						if(ztcnt>=30)
						{
							ztcnt=0;
							IWDG_Feed();    //Î¹¹·
							res = f_open(&fsrc,(char *)fileztp, FA_OPEN_ALWAYS | FA_WRITE);
							res = f_lseek(&fsrc, f_size(&fsrc));
							IWDG_Feed();    //Î¹¹·
							ADXL345_RD(tempbuf);	//¶ÁÈ¡10×é×ËÌ¬Êı¾İ£¬×öÈ¥³ıÌøµãµÄ´¦Àí		
							while(1)
							{
								ADXL345_RD_XYZ2(buf);//»ñµÃÒ»×é×ËÌ¬Êı¾İ
								/*»¬¶¯ÂË²¨´¦Àí*/
								tempbuf[Filter++]=(short)(((u16)buf[1]<<8)+buf[0]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[3]<<8)+buf[2]); 	    
								tempbuf[Filter++]=(short)(((u16)buf[5]<<8)+buf[4]); 
								IWDG_Feed();    //Î¹¹·
								if(Filter==30)
									Filter=0; //ÏÈ½øÏÈ³ö£¬ÔÙÇóÆ½¾ùÖµ
								
								a=tempbuf[i%30]-tempbuf[(i-3)%30];//XÖáÈ¥³ıĞÂ»ñÈ¡µÄÊı¾İµÄÌøµã
								b=tempbuf[(i+3)%30]-tempbuf[i%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[i%30]=(tempbuf[(i-3)%30]+tempbuf[(i+3)%30])/2;
								IWDG_Feed();    //Î¹¹·
								a=tempbuf[j%30]-tempbuf[(j-3)%30];//YÖáÈ¥³ıĞÂ»ñÈ¡µÄÊı¾İµÄÌøµã
								b=tempbuf[(j+3)%30]-tempbuf[j%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[j%30]=(tempbuf[(j-3)%30]+tempbuf[(j+3)%30])/2;
								
								a=tempbuf[k%30]-tempbuf[(k-3)%30];//ZÖáÈ¥³ıĞÂ»ñÈ¡µÄÊı¾İµÄÌøµã
								b=tempbuf[(k+3)%30]-tempbuf[k%30];
								if((a*b<0.00001)&&(fabs(a)>30)&&(fabs(b)>30))
									tempbuf[k%30]=(tempbuf[(k-3)%30]+tempbuf[(k+3)%30])/2;
								IWDG_Feed();    //Î¹¹·
								for(count=0;count<30;)
								{
									tx+=tempbuf[count++];
									ty+=tempbuf[count++];
									tz+=tempbuf[count++];
								}
								tx=tx/10;//È¡Æ½¾ùÖµ×÷ÎªÒ»×éÊı¾İ
								ty=ty/10;
								tz=tz/10;
								buffer[0]=(u8)(tx&0x00FF);
								buffer[1]=(u8)((tx>>8)&0x00FF);
								buffer[2]=(u8)(ty&0x00FF);
								buffer[3]=(u8)((ty>>8)&0x00FF);
								IWDG_Feed();    //Î¹¹·
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
								IWDG_Feed();    //Î¹¹·
								res = f_write(&fsrc,buffer,10, &bw);
								ztgroupcnt++;/////////////////
								if(ztgroupcnt==200)//////Ã¿´Î¼ÇÂ¼200×éÊı¾İºóÍË³ö
								{
									ztgroupcnt=0;/////
									IWDG_Feed();    //Î¹¹·
									f_close(&fsrc);	
									break;
								}
								IWDG_Feed();    //Î¹¹·		
								if(detectfeedback||RX_uart4flag||Rxwaterflag||Rxmegflag||tongbu)//ÈôÓĞÆäËûĞÅºÅµ½À´£¬ÔòÍË³ö
								{
									f_close(&fsrc);	
									break;
								}
							}
						}
					}
					
					
					statuscount++;
					if(statuscount==10)//Ã¿10Ãë´æ´¢Ò»´Î×´Ì¬ĞÅÏ¢
					{
						statuscount=0;
						Statusmonitor();
					}
					if(detectfeedback)//Îª1±íÊ¾×Û¿Ø°åÏò´Å²âÄ£¿é·¢ËÍÊı¾İÁË
					{
						detect++;
						if(detect==6)//¼ÆÊ±±äÁ¿µ½6±íÊ¾¹ıÁË6s»¹Ã»ÓĞÊÕµ½´Å²âµÄ·´À¡£¨±ÈÈç´Å²â¶Ïµç£©
						{
							detect=0;
							detectcount++;
							detectfeedback=0;
							IWDG_Feed();   //Î¹¹·
							Set_Log(1,feedcom,0);//¼ÇÂ¼´Ë´ÎÃüÁî×Ö·¢ËÍÊ§°Ü
							log_len++;//¼ÇÂ¼SD¿¨logÊı¾İµÄÌõÊı
			        Set_Log_len(log_len);
							IWDG_Feed();   //Î¹¹·
							if(detectcount==3)//Á¬ĞøÈı´Î·¢ËÍÊ§°Üºó£¬¸ø´Å²âÄ£¿é¹Øµç£¬ÖØĞÂÉÏµç
							{
								detectcount=0;
								MEG_Disable;
								
								GPIO_ResetBits(GPIOG,GPIO_Pin_15);//Ìí¼ÓµÄ£¬¸ø´Å²â¶Ïµç¾ÍÒª¹Ø±Õ¾§ÕñĞÅºÅµÄÊä³ö¡£
								
								Delay_ms(1000);	
								IWDG_Feed();   //Î¹¹·
								Delay_ms(1000);	
								IWDG_Feed();   //Î¹¹·
								Delay_ms(1000);	
								IWDG_Feed();   //Î¹¹·
								Delay_ms(1000);	
								IWDG_Feed();   //Î¹¹·
								Delay_ms(1000);	
								IWDG_Feed();   //Î¹¹·
								
								
								MEG_Enable;
								tongbu=1;
							}
							else
								QueryToMeg(feedcom);//Ã»ÓĞÊÕµ½´Å²âµÄ·´À¡£¬ÔòÔÙ´Î·¢ËÍÃüÁî£
						}
					}
					if(RX_uart4flag)//´®¿Ú4½ÓÊÕÊı¾İÍê³É£¬¼´ÎŞÏß´«ÊäÍê³É
				 {
					 RX_uart4flag=0;
					 ExecutPCCommand();//´¦ÀíÎŞÏß´«ÊäµÄÊı¾İ
				 }
					if(Rxwaterflag)//Ë®ÉùÊı¾İ½ÓÊÕÍê³É
					{
						IWDG_Feed();   //Î¹¹·
						Rxwaterflag=0;
						ExecutWaterCommand();	 //´¦ÀíË®ÉùÊı¾İ
					}
					if(Rxmegflag)//´Å²âÊı¾İ½ÓÊÕÍê³É
					{
						Rxmegflag=0;
						ExecutMegCommand();//´¦Àí´Å²âÊı¾İ
					}
					if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))//ÉÏËø
							GPSSource_Stop(); 
					if(release)//Ñ¹ÔØÊÍ·ÅÁË
					{
						release=0;
						Delay_ms(1000);	
						IWDG_Feed();   //Î¹¹·
						Delay_ms(1000);	
						IWDG_Feed();   //Î¹¹·
// 						MEG_Disable;//0702¸ü¸Ä£¬´Å²â¶ÏµçÒÔ±ãÓÚgps½ÓÊÕµ½¾­Î³¶ÈĞÅºÅ
						zttimeflag=0;
						tongbu=0;
						Delay_ms(1000);	
						IWDG_Feed();   //Î¹¹·
// 						GPSSource24V_OUT();//
					}
					
					if(tongbu)//Í¬²½±êÖ¾Î»Îª1£¬±íÊ¾ÆäËû²Ù×÷Íê³É£¬¿ÉÒÔ·¢ËÍÍ¬²½ĞÅºÅ  
					{ 
						Delay_ms(1000);	//Ìí¼ÓÑÓÊ±
						IWDG_Feed();  //Î¹¹·
						if(meg_syn<=30)//¸ø´Å²âÉÏµçºó£¬¹ı30ÃëºóÔÙ¸øÍ¬²½ĞÅºÅ
							meg_syn++;
						if(meg_syn==30)
						{	
							meg_syn=0;
							tongbu=0;	//Èô´Å²âÎ´Í¬²½Ê±¶Ïµç£¬ÔòtongbuÈÔÈ»Îª1£¬³ÌĞò»á·¢Í¬²½ĞÅºÅ£¬È»ºó¼ì²â·´À¡£¬ÒòÎª´Å²â¶ÏµçËùÒÔÎŞ·´À¡£¬Òò´Ë³ÌĞò»áÈÃ´Å²âÖØÆô		
							GPIO_SetBits(GPIOG,GPIO_Pin_15);//¸ø´Å²âÒ»¸öÉÏÉıÑØ£¬×÷ÎªÍ¬²½ĞÅºÅ
							Set_Log(1,0xD0,1);//ĞÂ¼ÓµÄDO,ÎÄ¼ş¼ÇÂ¼
							log_len++;//¼ÇÂ¼SD¿¨logÊı¾İµÄÌõÊı
			        Set_Log_len(log_len);
							ReadRTC(&RTC_ARR[0],0x00,8); 
							RTC_Time[0]=BCD2HEX(RTC_ARR[5]);
							RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
							RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
							RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
							RTC_Time[4]=BCD2HEX(RTC_ARR[0]);//2017¡¢1¡¢6Ìí¼Ó£¬Í¬²½Ö®ºó²Å¿ªÊ¼,»áÓĞ0.5SÑÓ³Ù
							
							Delay_ms(1000);	
							IWDG_Feed();  //Î¹¹·

							Delay_ms(1000);	
							IWDG_Feed();   //Î¹¹·
// 							GPIO_ResetBits(GPIOG,GPIO_Pin_15); //×¢ÊÍµô£¬ÒòÎª¸ÄÎªÍ¬²½ĞÅºÅºÍ¾§ÕñÏàÓë×÷Îª´Å²âÊ±ÖÓµÄÊäÈë£¬¿ªÊ¼Í¬²½Ö®ºóÒª±£³Ö¸ßµçÆ½²ÅÓĞĞÅºÅÊä³ö¸ø´Å²â¡£
							
							
							
							Delay_ms(1000);	
							IWDG_Feed();   //Î¹¹·
							Delay_ms(1000);	
							IWDG_Feed();   //Î¹¹·
							//QueryToMeg(0xA6);//²éÑ¯Í¬²½ÊÇ·ñ³É¹¦
						}			
					}	
					
					IWDG_Feed();    //Î¹¹·
					if(sendtowater)//Îª1 ±íÊ¾¿ÉÒÔÏòË®ÉùÄ£¿é·¢ËÍÊı¾İ
					{
						sendtowater=0;
						if(FromJiaBanA==4)//ÃüÁîÎªA4Ë®Éù
						{
							FromJiaBanA=0;
							SendData(lc_data.lc_blocknum,lc_data.lc_fsizebyte,0xA4);	//Í¨¹ıË®ÉùÍ¨ĞÅÄ£¿é·¢ËÍÊı¾İ								
						}
						if(FromJiaBanA==3)//ÃüÁîÎªA3
						{
							FromJiaBanA=0;
							SendData(dp_data.dp_blocknum,dp_data.dp_fsizebyte,0xA3);	
						}

            if(FromJiaBanA==5)//ÎŞÏß¶ÁÈ¡´Å²âÂ©´ÅÊı¾İ
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB2);//Í¨¹ıÎŞÏß·¢³öflashÊı¾İ
		      	}

           if(FromJiaBanA==6)//ÎŞÏß¶ÁÈ¡´Å²âµÍÆµÊı¾İ
		      	{
			       	FromJiaBanA=0;
		        	ReadAllDataToPC(0xB3);//Í¨¹ıÎŞÏß·¢³öflashÊı¾İ
		      	}
					}
		
				}
		
		if(flag_rx==1)//Èç¹û½ÓÊÜ¶¨Î»ÏûÏ¢³É¹¦±êÖ¾Î»Îª1£¬Ôò½øÈë´¦Àíº¯Êı£¬²»½øÈëÆäËûº¯ÊıÒÔÃâÓ°ÏìÊÚÊ±¾«¶È
		{  flag_rx = 0;
			 GPS_NMEA();//½ÓÊÕµ½µÄGPSÊı¾İ½øĞĞ×ª»»
				if(GPS_VA)//½ÓÊÕµÄ¶¨Î»Êı¾İÓĞĞ§
				{	
					GPS_VA=0;
					USART_ITConfig(UART5,USART_IT_RXNE,DISABLE);//½ÓÊÕÖĞ¶ÏÊ§ÄÜ
					RTC_ARR[0] = 0;	// enable oscillator (bit 7=0)
					RTC_ARR[1] = HEX2BCD(Sec);	// enable oscillator (bit 7=0)
					RTC_ARR[2] = HEX2BCD(Min);	// minute = 59
					RTC_ARR[3] = HEX2BCD(Hour);	// hour = 05 ,24-hour mode(bit 6=0)
					RTC_ARR[4] = 1;	// Day = 1 or sunday
					RTC_ARR[5] = HEX2BCD(Day);	// Date = 30
					RTC_ARR[6] = HEX2BCD(Month);	// month = August
					RTC_ARR[7] = HEX2BCD(Year);	// year = 05 or 200
					WriteRTC(&RTC_ARR[0],0x00,8);	// Set RTC	  
					
					IWDG_Feed();  //Î¹¹·
					SendREQToPC(0xC1);//»Ø·¢ÊÚÊ±³É¹¦				
				}
				
			}
	}
}

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority BitsÏÈÕ¼ÓÅÏÈ¼¶1Î»£¬´ÓÓÅÏÈ¼¶3Î» */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* ´®¿Ú1ÖĞ¶ÏÅäÖÃ */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//ÏÈÕ¼ÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//´ÓÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* ´®¿Ú3ÖĞ¶ÏÅäÖÃ */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//ÏÈÕ¼ÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//´ÓÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
	/* DMAÖĞ¶ÏÅäÖÃ */
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
	/* ´®¿Ú4ÖĞ¶ÏÅäÖÃ */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//ÏÈÕ¼ÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//´ÓÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	/* ´®¿Ú5ÖĞ¶ÏÅäÖÃ */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;       
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//ÏÈÕ¼ÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//´ÓÓÅÏÈ¼¶
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void GPIO_EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;//ÅäÖÃ´Å²âÄ£¿éÍ¬²½IO¿Ú£¬ÍÆÍìÊä³ö
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//BTS555¿ØÖÆ
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//ÅäÖÃË®Éù¶Ë¿ÚµÄÊ¹ÄÜ¶ËIO¿Ú
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//ÅäÖÃ·äÃùÆ÷¿ØÖÆIO¿Ú
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  //°´¼üIO¿Ú³õÊ¼»¯  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    
  GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//µç´ÅÌúÎ»ÖÃ¼ì²âIO¿ÚÅäÖÃÖĞ¶Ï
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);//ÅäÖÃÖĞ¶ÏÏß
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);//ÅäÖÃÖĞ¶ÏÏß
	
	
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



