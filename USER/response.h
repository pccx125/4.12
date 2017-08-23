#ifndef __RESPONSE_H
#define __RESPONSE_H


#include "main.h"
#include "ff.h"
typedef enum {
	Data_NO=0,
	Data_OK,
} STATUS;
struct LC_Data{
	u16 LC_AllGroupCnt;//所有的漏磁数据数据组数
	u32 LC_DataByteCnt;//flash中一次同步所存储的数据字节数  
	u32 lc_fsizebyte;//目前此组同步数据中，已经读出的数据组数
	u8 lc_rdgroupcnt;//本次所读数据的漏磁数据条数
	u8 lc_filep[25];//指针数据用于存储在sd卡中建立的文件名
	u16 lc_blocknum;//记录已经读出的块数
	u8 lc_threshold[2];//暂存漏磁检测阈值，磁测上电时给磁测模块
};
struct DP_Data{
	u16 DP_AllGroupCnt;//甚低频数据组数
	u32 DP_DataByteCnt;//flash中一次同步所存储的数据字节数  
	u32 dp_fsizebyte;//目前此组同步数据中，已经读出的数据字节数
	u8 dp_rdgroupcnt;//甚低频数据条数
	u8 dp_filep[25];//指针数据用于存储在sd卡中建立的文件名
	u16 dp_blocknum;//记录已经读出的块数
	u8 dp_threshold[2];//暂存甚低频检测阈值，磁测上电时给磁测模块
};



void QueryToMeg(u8 KeyWord);
void WriteData(u8 *tempbuffer,u8 NumByteToWrite);
void ExecutMegCommand(void);
void QueryToWater(u8 KeyWord);
void ExecutWaterCommand(void);
void SendStatus(void);
void SendREQToWater(u8 com);
void SendDataNum(u16 Groupcount,u8 com);
void SendData(u16 rdblocknum,u32 fsizebyte,u8 com);
void CreateNewFile(u8 flag);
void WriteDatatosd(u8 *tmp_name,u8 *buffertemp,u8 datalen);
void ReadAllDataToPC(u8 com);//通过485发送数据时使用
void ExecutPCCommand(void);
void SendStatusToPC(void);
void Set_Log(u8 name,u8 com,u8 b);
u8 GetBoardID(void);//得到综控板的id
void SetBoardID(u8 id);//设置综控板的ID
void SetWirelessAddr(u8 id);
void SetMegthreshold(u8 com);
void SetWaterPower(void);
void Statusmonitor(void);
void Read_Log(void);
void Delete_file(void);
u16 Get_Log_len(void) ;
void Set_Log_len(u16 loglen);
#endif

