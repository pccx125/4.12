#include "main.h"

float jingdu,weidu;
char s1,s2,s3,s4;
u8 Hour,Min,Sec,Day,Month,Year,GPS_VA,weidu_dir,jingdu_dir;
//  u8 flag_rx=0;
//GPS���ݽ���������
#define GPS_Buf_N   400	    			
u8 GPS_Buf[GPS_Buf_N]={0};	
volatile uint16_t GPSPoint_4 = 0;
volatile uint32_t Rec_Len = 0;

void GPS_NMEA(void)
{		
    IWDG_Feed();            
		if(GPS_Buf[3]=='R'&& GPS_Buf[4]=='M'&&GPS_Buf[5]=='C') 
                    GPRMC_DAT();     //����GPS NMEA GPRMCЭ�麯�� 
	
}

void GPRMC_DAT(void)
{	
   unsigned char i,i1=0,uf=0;
   float l_g;
   IWDG_Feed(); 
   for(i=0;i<Rec_Len;i++)
	{
		if(GPS_Buf[i]==0x2c)  //�ж��Ƿ�Ϊ����
		{				                       
			i1++;
			uf=0;
		}
    IWDG_Feed(); 
		if(i1==1&&uf==0) //GPRMCʱ��
		{	                                             
			Hour=(GPS_Buf[i+1]-0x30)*10+(GPS_Buf[i+2]-0x30);		 //ʱ
			Min=(GPS_Buf[i+3]-0x30)*10+(GPS_Buf[i+4]-0x30);		     //��
			Sec=(GPS_Buf[i+5]-0x30)*10+(GPS_Buf[i+6]-0x30);		     //��
			i=i+6;		
			uf=1;
    IWDG_Feed(); 
		}
                else if(i1==2&&uf==0) //GPRMC״̬��Ч��
		{	                                    
	  	if(GPS_Buf[i+1]=='A')	
                  GPS_VA=1; 		//��Ч
		else 
                  GPS_VA=0;												 //��Ч
                i++;	
                uf=1;
             IWDG_Feed(); 
   	}	
	  else if(i1==3&&uf==0) //GPRMC γ��
		{	                  
	  	if(GPS_Buf[i+1]==0x2c) weidu=0;
	  	else 
                {
                        weidu=((GPS_Buf[i+1]-0x30)*10+(GPS_Buf[i+2]-0x30)+	 //γ����������
                        (((((GPS_Buf[i+3]-0x30)*10)+(GPS_Buf[i+4]-0x30))/0.6)*0.01))*3600;
                       // s1 = GPS_Buf[i+1];
                       // s2 = GPS_Buf[i+2];
                       // weidu =  (((((GPS_Buf[i+3]-0x30)*10)+(GPS_Buf[i+4]-0x30))/0.6)*0.01)*3600;
                  IWDG_Feed();                        
                        l_g=(((GPS_Buf[i+6]-0x30)*1000)+((GPS_Buf[i+7]-0x30)*100)+     //γ��С������
                        ((GPS_Buf[i+8]-0x30)*10)+(GPS_Buf[i+9]-0x30))*0.006;
                        weidu=weidu+l_g;   //����γ�ȣ�����34.xxxx
                        i=i+9;
                }
                uf=1;
              IWDG_Feed(); 
	  }
	  else if(i1==4&&uf==0) //GPRMC γ���ϱ������ʾ
          {	                                    
                  if(GPS_Buf[i+1]==0x2c) jingdu_dir=0;
                  else if(GPS_Buf[i+1]=='N') weidu_dir=0;     //��γ
                  else if(GPS_Buf[i+1]=='S') weidu_dir=1;     //��γ
                  i++; 
                  uf=1;	
              IWDG_Feed(); 
	  } 
	  else if(i1==5&&uf==0)         //GPRMC ����
		{	                            
	  	if(GPS_Buf[i+1]==0x2c) jingdu=0; 
			else
			{
				jingdu=((GPS_Buf[i+1]-0x30)*100+(GPS_Buf[i+2]-0x30)*10+	   //???????????
				(GPS_Buf[i+3]-0x30)+(((((GPS_Buf[i+4]-0x30)*10)+(GPS_Buf[i+5]-0x30))/0.6)*0.01))*3600;
                                //jingdu=((GPS_Buf[i+1]-0x30)*100+(GPS_Buf[i+2]-0x30)*10+	   //???????????
				//(GPS_Buf[i+3]-0x30)+(((((GPS_Buf[i+4]-0x30)*10)+(GPS_Buf[i+5]-0x30))/0.6)*0.01));
            IWDG_Feed(); 
				l_g=(((GPS_Buf[i+7]-0x30)*1000)+((GPS_Buf[i+8]-0x30)*100)+  //??????????? 
						((GPS_Buf[i+9]-0x30)*10)+(GPS_Buf[i+10]-0x30))*0.006;
                               // l_g=((((GPS_Buf[i+7]-0x30)*1000)+((GPS_Buf[i+8]-0x30)*100)+  //??????????? 
				//		((GPS_Buf[i+9]-0x30)*10)+(GPS_Buf[i+10]-0x30))/0.6)*0.000001;

				jingdu=jingdu+l_g;  //?????? ??107.xxxx
				i=i+10;
			}
			uf=1;
            IWDG_Feed(); 
	  }         
	  else if(i1==6&&uf==0)   //GPRMC ���ȶ�������
		{	                   
			if(GPS_Buf[i+1]==0x2c) jingdu_dir=0;
			else if(GPS_Buf[i+1]=='E') jingdu_dir=0;   //??
			else if(GPS_Buf[i+1]=='W') jingdu_dir=1;   //??
			i++; 
			uf=1;	
           IWDG_Feed(); 
	  } 
	  else if(i1==9&&uf==0)  //GPRMC ����
		{	                                   
			Day=(GPS_Buf[i+1]-0x30)*10+(GPS_Buf[i+2]-0x30);			 //��
			Month=(GPS_Buf[i+3]-0x30)*10+(GPS_Buf[i+4]-0x30);		 //��
			Year=(GPS_Buf[i+5]-0x30)*10+(GPS_Buf[i+6]-0x30);		 //��
			i=i+6;
			uf=1;	
     IWDG_Feed(); 
		}
  }			
  UTCTOBJTIME();	
}


void UTCTOBJTIME(void)  //UTCתΪ����ʱ�䣬���δ����
{
					char days[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
         u8 i=0;
				if((Year%400==0)||((Year%100!=0)&&(Year%4==0))) // leap year
				days[2]=29;
        i=days[2];
				Hour+=8;
				
				if(Hour>24)
				{
						Hour-=24;
						Day++;				
						if(Month==2)
						{
							if(Day>i)
							{Day=1;
								Month+=1;
							}
						}
						
						if(Month==1||Month==3||Month==5||Month==7||Month==8||Month==10||Month==12)
						{
							if(Day>31)
							{Month+=1;
								Day=1;						
							}		
						}
							
						if(Month==4||Month==6||Month==9||Month==11)
						{
							if(Day>30)
							{Month+=1;
								Day=1;						
							}		
						}
							
						if(Month>12)
						{
						 Month-=12;
						 Year++;
						}	
				}
}
	
	


