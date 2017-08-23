#ifndef __RESPONSE_H
#define __RESPONSE_H


#include "main.h"
#include "ff.h"
typedef enum {
	Data_NO=0,
	Data_OK,
} STATUS;
struct LC_Data{
	u16 LC_AllGroupCnt;//���е�©��������������
	u32 LC_DataByteCnt;//flash��һ��ͬ�����洢�������ֽ���  
	u32 lc_fsizebyte;//Ŀǰ����ͬ�������У��Ѿ���������������
	u8 lc_rdgroupcnt;//�����������ݵ�©����������
	u8 lc_filep[25];//ָ���������ڴ洢��sd���н������ļ���
	u16 lc_blocknum;//��¼�Ѿ������Ŀ���
	u8 lc_threshold[2];//�ݴ�©�ż����ֵ���Ų��ϵ�ʱ���Ų�ģ��
};
struct DP_Data{
	u16 DP_AllGroupCnt;//����Ƶ��������
	u32 DP_DataByteCnt;//flash��һ��ͬ�����洢�������ֽ���  
	u32 dp_fsizebyte;//Ŀǰ����ͬ�������У��Ѿ������������ֽ���
	u8 dp_rdgroupcnt;//����Ƶ��������
	u8 dp_filep[25];//ָ���������ڴ洢��sd���н������ļ���
	u16 dp_blocknum;//��¼�Ѿ������Ŀ���
	u8 dp_threshold[2];//�ݴ�����Ƶ�����ֵ���Ų��ϵ�ʱ���Ų�ģ��
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
void ReadAllDataToPC(u8 com);//ͨ��485��������ʱʹ��
void ExecutPCCommand(void);
void SendStatusToPC(void);
void Set_Log(u8 name,u8 com,u8 b);
u8 GetBoardID(void);//�õ��ۿذ��id
void SetBoardID(u8 id);//�����ۿذ��ID
void SetWirelessAddr(u8 id);
void SetMegthreshold(u8 com);
void SetWaterPower(void);
void Statusmonitor(void);
void Read_Log(void);
void Delete_file(void);
u16 Get_Log_len(void) ;
void Set_Log_len(u16 loglen);
#endif

