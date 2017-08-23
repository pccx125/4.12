#ifndef __NAND_WR_H
#define __NAND_WR_H

void CreateNewArea(u8 flag);
void WriteDatatoFlash(u8 *tempbuffer,u8 NumByteToWrite,u8 flag);



#endif

