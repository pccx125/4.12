#include"main.h"

const u16 wCRCTalbeAbs[] =
{
0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 
};

u16 CRC16_2(u8* pchMsg, u16 wDataLen)
{
        u16 wCRC = 0xFFFF;
        u16 i;
        u8 chChar;
        u8 hi,lo;
        for (i = 0; i < wDataLen; i++)
        {
                chChar = *pchMsg++;
                wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
                wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
        }
				hi=wCRC%256;
        lo=wCRC/256;
        wCRC=(hi<<8)|lo;
        return wCRC;
}

/*1������һ����ʼֵΪFFFF��16λ�ı������ñ�����ΪCRC�Ĵ���.
2���������ͻ������Ϣ��char�ͣ�8λ����CRC�Ĵ�������ֵ�ĵ�8λ��������㣬���ѽ���ڸ���CRC�Ĵ�����
3��CRC�Ĵ�������1λ�������λ����ͬʱ���λ���㡣ȡ����������λ�Ƿ�Ϊ1��
4�����Ϊ1����CRC�Ĵ��������ʽA001������Ϊ0�����ظ���3���Ķ���
5���ظ�3��4ֱ�������8����λ������������8λ�ֽڽ���ɴ����ˡ�
6��������һ��8λ�ֽڵĴ�������ظ���2������5����
7�������е������ͻ������Ϣ��������� CRC�Ĵ������ֵ��������������Ҫ�õ���CRCУ���롣
*/
unsigned int calccrc(unsigned char crcbuf,unsigned int crc)

{

       unsigned char i;

       crc=crc ^ crcbuf;//�������ͻ������Ϣ��CRC�Ĵ�������ֵ��������㣬���ѽ���ڸ���CRC�Ĵ�����

       for(i=0;i<8;i++)

       {

       unsigned char chk;

       chk=crc&1;//CRC�Ĵ�������1λ�������λ����ͬʱ���λ���㡣ȡ����������λ�Ƿ�Ϊ1��

       crc=crc>>1;

       crc=crc&0x7fff;

       if (chk==1)//������λΪ1����CRC�Ĵ��������ʽA001���

       crc=crc^0xa001;

       crc=crc&0xffff;

       }

       return crc;

}

unsigned int chkcrc(unsigned char *buf,unsigned int len)//bufΪ���Ҫ���͵��������飬lenΪ����ĳ���

{

   unsigned int i;

   unsigned int crc;

   crc=0x0000;//����һ����ʼֵΪFFFF��16λ�ı������ñ�����ΪCRC�Ĵ���.

   for (i=0;i<len;i++)

   {

   crc=calccrc(*buf,crc);

   buf++;

   }

  return crc;

}

