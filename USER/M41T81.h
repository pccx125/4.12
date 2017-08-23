#ifndef   __M41T81_H
#define  __M41T81_H




void I2C_M41T81_Config(void);
void ReadRTC(unsigned char * buff,unsigned char addr,unsigned char NumByteToRead);
void WriteRTC(unsigned char *buff,unsigned char addr,unsigned char NumByteToWrite);	
unsigned char ReadByte(unsigned char addr);
void WriteByte(unsigned char Data,unsigned char addr);
unsigned char BCD2HEX(unsigned char bcd)	;
unsigned char HEX2BCD(unsigned char hex);



#endif


