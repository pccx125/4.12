#include "main.h"
/*对nand flash操作的API函数*/
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
	IWDG_Feed();   //喂狗
	/*设置NAND FLASH的写地址第一个块的第一页放同步多少次*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);//把有多少次同步存入flash
	/*每同步一次留出三个块来存相关数据，第一个块放漏磁数据占多少页 变量是 pagenum，第二块的第一页放时间，其他页放依次放漏磁数据，第三块放第二块中
	对应页的数据组数也是变量（每组13个字节）*/
	/*设置NAND FLASH的写地址,把时间信息写入下一块的第一页*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	IWDG_Feed();   //喂狗
	/*擦除待写入数据的块*/
	NAND_EraseBlock(WriteReadAddr);

//注释掉，硬同步之后立马存下同步时间，而不是接收到A6之后
// 	ReadRTC(&RTC_ARR[0],0x00,8); 
// 	RTC_Time[0]=BCD2HEX(RTC_ARR[5]);
// 	RTC_Time[1]=BCD2HEX(RTC_ARR[3]);
// 	RTC_Time[2]=BCD2HEX(RTC_ARR[2]);
// 	RTC_Time[3]=BCD2HEX(RTC_ARR[1]);
// 	RTC_Time[4]=BCD2HEX(RTC_ARR[0]);

	NAND_WriteBuffer(RTC_Time, WriteReadAddr, 5);
	IWDG_Feed();   //喂狗
	pagenum=1;//在此块中栈几页
	/*设置NAND FLASH的地址,*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+1+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*擦除待写入数据的块*/
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&pagenum, WriteReadAddr, 1);
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+3+0x200*flag;
	WriteReadAddr.Page = 0x00;
	IWDG_Feed();   //喂狗
	/*擦除待写入数据的块*/
	NAND_EraseBlock(WriteReadAddr);
	blocknum++;
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*擦除待写入数据的块*/
	NAND_EraseBlock(WriteReadAddr);
	IWDG_Feed();   //喂狗
	NAND_WriteBuffer(&blocknum, WriteReadAddr, 1);
	IWDG_Feed();   //喂狗
}
void WriteDatatoFlash(u8 *tempbuffer,u8 NumByteToWrite,u8 flag)//flag为1表示写漏磁数据，为0表示写甚低频数据
{
	u8 blocknum;
	u8 pagenum;
	u8 datagroupcount;
	NAND_ADDRESS WriteReadAddr;
	IWDG_Feed();   //喂狗
	/*设置NAND FLASH的写地址*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = 0x00+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&blocknum, WriteReadAddr, 1);//得出有多少个块
	/*设置NAND FLASH的写地址*/
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	NAND_ReadBuffer(&pagenum, WriteReadAddr, 1);//得出这个快中写了几页的数据
	IWDG_Feed();   //喂狗
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-1+0x200*flag;
	WriteReadAddr.Page = pagenum;
	NAND_WriteBuffer(tempbuffer, WriteReadAddr,NumByteToWrite);//写数据
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3+0x200*flag;
	WriteReadAddr.Page = pagenum;
	datagroupcount=NumByteToWrite/13;
	IWDG_Feed();   //喂狗
	NAND_WriteBuffer(&datagroupcount, WriteReadAddr,1);
	pagenum=pagenum+1;   //页数加1
	WriteReadAddr.Zone = 0x00;
	WriteReadAddr.Block = blocknum*3-2+0x200*flag;
	WriteReadAddr.Page = 0x00;
	/*擦除待写入数据的块*/
	NAND_EraseBlock(WriteReadAddr);
	NAND_WriteBuffer(&pagenum, WriteReadAddr,1);	//存储页数
	if(pagenum>=62)//每块中有64页，同步一次占用一块，超过一块的页数时，给磁测重新上电同步
	{
		Set_Log(0,0xAC,1);
		log_len++;
		Set_Log_len(log_len);
		pagenum = 0;
		CreateNewArea(1);//同步完成后，在NandFlash中建立一个新区来存放这次同步后的数据为1 表示建立甚低频区
		CreateNewArea(0);//建立漏磁区
		CreateNewFile(1);//同步完成后，在sd卡中建立一个新文件来存放这次同步后的  甚低频数据 
		CreateNewFile(0);//同步完成后，在sd卡中建立一个新文件来存放这次同步后的 漏磁数据	
		CreateNewFile(2);	//同步完成后，在sd卡中建立一个新文件来存放姿态数据			
		lc_data.LC_DataByteCnt=0;//记录漏磁数据字节总数置0
		dp_data.DP_DataByteCnt=0;//记录甚低频数据字节总数置0
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
		BeepON;//蜂鸣器响
		Delay_ms(1000);
		IWDG_Feed();   //喂狗
		Delay_ms(1000);
		IWDG_Feed();   //喂狗
		BeepOFF;

	}
	IWDG_Feed();   //喂狗
}


