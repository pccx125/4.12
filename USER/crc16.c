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

/*1、定义一个初始值为FFFF的16位的变量，该变量称为CRC寄存器.
2、把欲发送或接收消息（char型，8位）和CRC寄存器的数值的低8位作异或运算，并把结果在赋到CRC寄存器。
3、CRC寄存器右移1位（朝最低位），同时最高位添零。取出并检查最低位是否为1。
4、如果为1，则CRC寄存器与多项式A001异或；如果为0，则重复第3步的动作
5、重复3和4直到完成了8次移位。这样完整的8位字节将完成处理了。
6、对于下一个8位字节的处理就是重复第2步到第5步了
7、把所有的欲发送或接收消息这样处理后， CRC寄存器里的值就是我们最终需要得到的CRC校验码。
*/
unsigned int calccrc(unsigned char crcbuf,unsigned int crc)

{

       unsigned char i;

       crc=crc ^ crcbuf;//把欲发送或接收消息和CRC寄存器的数值作异或运算，并把结果在赋到CRC寄存器。

       for(i=0;i<8;i++)

       {

       unsigned char chk;

       chk=crc&1;//CRC寄存器右移1位（朝最低位），同时最高位添零。取出并检查最低位是否为1。

       crc=crc>>1;

       crc=crc&0x7fff;

       if (chk==1)//如果最低位为1，则CRC寄存器与多项式A001异或

       crc=crc^0xa001;

       crc=crc&0xffff;

       }

       return crc;

}

unsigned int chkcrc(unsigned char *buf,unsigned int len)//buf为存放要发送的数据数组，len为数组的长度

{

   unsigned int i;

   unsigned int crc;

   crc=0x0000;//定义一个初始值为FFFF的16位的变量，该变量称为CRC寄存器.

   for (i=0;i<len;i++)

   {

   crc=calccrc(*buf,crc);

   buf++;

   }

  return crc;

}

