#include "main.h"
/*��nand flash������API����*/
u16 log_len;
extern struct LC_Data lc_data;
extern struct DP_Data dp_data;
extern u8 tongbu,zttimeflag;
unsigned char  RTC_Time[5]={0};     //B4change 20161025
void CreateNewArea(u8 flag)
{
	u8 blocknum;
	u8 pagenum;
	NAND_ADDRESS WriteReadAddr;
// 	unsigned char  RTC_ARR[8],RTC_Time[5]={0};
	unsigned char  RTC_ARR[8]={0};
	IWDG_Feed();   //ι��
	/*����NAND FLASH��д��ַ��һ����ĵ�һҳ��ͬ�����ٴ�*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);//���ж��ٴ�ͬ������flash
	/*ÿͬ��һ����������������������ݣ���һ�����©������ռ����ҳ ������ pagenum���ڶ���ĵ�һҳ��ʱ�䣬����ҳ�����η�©�����ݣ�������ŵڶ�����
	��Ӧҳ����������Ҳ�Ǳ�����ÿ��13���ֽڣ�*/
	/*����NAND FLASH��д��ַ,��ʱ����Ϣд����һ��ĵ�һҳ*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	IWDG_Feed();   //ι��
	/*������д�����ݵĿ�*/
	NAND_EraseBlock(WriteReadAddr);

//ע�͵���Ӳͬ��֮���������ͬ��ʱ�䣬�����ǽ��յ�A6֮��
// 	ReadRTC(&RTC_ARR[0],0x00,8); 
// 	RTC_Time[0]=BCD2HEX(RTC_ARR[5]);
// 	RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
// 	RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
// 	RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
// 	RTC_Time[4]=BCD2HEX(RTC_ARR[0]);

	NAND_WriteBuffer(RTC_Time, WriteReadAddr, 5);
	IWDG_Feed();   //ι��
	pagenum=1;//�ڴ˿���ջ��ҳ
	/*����NAND FLASH�ĵ�ַ,*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+1+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*������д�����ݵĿ�*/
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&pagenum, WriteReadAddr, 1);
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+3+0x200*flag;
	WriteReadAddr.Page = 0x00;
	IWDG_Feed();   //ι��
	/*������д�����ݵĿ�*/
	NAND_EraseBlock(WriteReadAddr);
	blocknum++;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*������д�����ݵĿ�*/
	NAND_EraseBlock(WriteReadAddr);
	IWDG_Feed();   //ι��
	NAND_WriteBuffer(&blocknum, WriteReadAddr, 1);
	IWDG_Feed();   //ι��
}
void WriteDatatoFlash(u8 *tempbuffer,u8 NumByteToWrite,u8 flag)//flagΪ1��ʾд©�����ݣ�Ϊ0��ʾд����Ƶ����
{
	u8 blocknum;
	u8 pagenum;
	u8 datagroupcount;
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();   //ι��
	/*����NAND FLASH��д��ַ*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);//�ó��ж��ٸ���
	/*����NAND FLASH��д��ַ*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);//�ó��������д�˼�ҳ������
	IWDG_Feed();   //ι��
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-1+0x200*flag;
	WriteReadAddr.Page = pagenum;
	NAND_WriteBuffer(tempbuffer, WriteReadAddr,NumByteToWrite);//д����
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+0x200*flag;
	WriteReadAddr.Page = pagenum;
	datagroupcount=NumByteToWrite/13;
	IWDG_Feed();   //ι��
	NAND_WriteBuffer(&datagroupcount, WriteReadAddr,1);
	pagenum=pagenum+1;   //ҳ����1
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*������д�����ݵĿ�*/
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&pagenum, WriteReadAddr,1);	//�洢ҳ��
	if(pagenum>=62)//ÿ������64ҳ��ͬ��һ��ռ��һ�飬����һ���ҳ��ʱ�����Ų������ϵ�ͬ��
	{
		Set_Log(0,0xAC,1);
		log_len++;
		Set_Log_len(log_len);
		pagenum = 0;
		CreateNewArea(1);//ͬ����ɺ���NandFlash�н���һ��������������ͬ���������Ϊ1 ��ʾ��������Ƶ��
		CreateNewArea(0);//����©����
		CreateNewFile(1);//ͬ����ɺ���sd���н���һ�����ļ���������ͬ�����  ����Ƶ���� 
		CreateNewFile(0);//ͬ����ɺ���sd���н���һ�����ļ���������ͬ����� ©������	
		CreateNewFile(2);	//ͬ����ɺ���sd���н���һ�����ļ��������̬����			
		lc_data.LC_DataByteCnt=0;//��¼©�������ֽ�������0
		dp_data.DP_DataByteCnt=0;//��¼����Ƶ�����ֽ�������0
		Set_Log(0,0xAB,1);
		log_len++;
		Set_Log_len(log_len);
		
		//QueryToMeg(0xA5);
		if(flag==1){
																																	//WriteDatatoFlash(buffertemp,DipinData,1);
			WriteDatatoFlash(tempbuffer,NumByteToWrite,1);
    }
		else{
																																	//WriteDatatoFlash(buffertemp,LouciData,0);
			WriteDatatoFlash(tempbuffer,NumByteToWrite,0);
    }
		BeepON;//��������
		Delay_ms(1000);
		IWDG_Feed();   //ι��
		Delay_ms(1000);
		IWDG_Feed();   //ι��
		BeepOFF;

	}
	IWDG_Feed();   //ι��
}


