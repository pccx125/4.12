/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "main.h"


#define BLOCK_SIZE            512 /* Block Size in Bytes */



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	u8  Status;
	/* Supports only single drive */
	if (drv)
	{
		return STA_NOINIT;
	}
/*-------------------------- SD Init ----------------------------- */
  Status = SD_Initialize();
	if (Status!=0 )
	{
		return STA_NOINIT;
	}
	else
	{
		return RES_OK;
	}

}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{

	if (count > 1)
	{
// 		SD_ReadMultiBlock(sector, buff, count);	
	  SD_ReadDisk(buff,sector,count);	 
// 			  /* Check if the Transfer is finished */
// 	     SD_WaitReadOperation();  //循环查询dma传输是否结束
// 	
// 	    /* Wait until end of DMA transfer */
// 	    while(SD_GetStatus() != SD_TRANSFER_OK);

	}
	else
	{
// 		
// 		SD_ReadSingleBlock(sector, buff);
    SD_ReadDisk(buff,sector,1);	 
// 			  /* Check if the Transfer is finished */
// 	     SD_WaitReadOperation();  //循环查询dma传输是否结束
// 	
// 	    /* Wait until end of DMA transfer */
// 	    while(SD_GetStatus() != SD_TRANSFER_OK);

	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{

	if (count > 1)
	{
// 		SD_WriteMultiBlock(sector, buff, count);
		SD_WriteDisk((u8*)buff,sector,count);
	}
	else
	{
// 		SD_WriteSingleBlock(sector, buff);
		SD_WriteDisk((u8*)buff,sector,1);
	}
	return RES_OK;
}
#endif /* _READONLY */




/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}
							 
/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/ 
DWORD get_fattime(void)
{

 	return 0;

} 
