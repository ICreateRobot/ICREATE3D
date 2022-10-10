/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sd_card.h"

/* Definitions of physical drive number for each drive */
#define SD_CARD		0	/* Example: Map SD card to physical drive 0 */

/*获得磁盘状态*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}

/*初始化磁盘*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	unsigned char tempRes = 0;
	switch(pdrv)
	{
		case SD_CARD:
		{
			tempRes = SD_Card_Configuration();
			break;
		}
		default: tempRes = RES_ERROR;
	}
	if(tempRes == 0) return RES_OK;
	else return STA_NOINIT;
}

/*读扇区 pdrv:磁盘编号0~9 *buff:数据接收缓冲首地址 sector:扇区地址 count:需要读取的扇区数*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	unsigned char tempRes = 0;
	if(!count) return RES_PARERR;
	switch(pdrv)
	{
		case SD_CARD:
		{
			tempRes = SD_Card_Read(buff, sector, count);
			break;
		}
		default: tempRes = RES_ERROR;
	}
	if(tempRes == 0) return RES_OK;
	else return RES_ERROR;
}

/*写扇区 pdrv:磁盘编号0~9 *buff:发送数据首地址 sector:扇区地址 count:需要写入的扇区数*/
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	unsigned char tempRes = 0;
	if(!count) return RES_PARERR;
	switch(pdrv)
	{
		case SD_CARD:
		{
			tempRes = SD_Card_Write((unsigned char *)buff, sector, count);
			break;
		}
		default: tempRes = RES_ERROR;
	}
	if(tempRes == 0) return RES_OK;
	else return RES_ERROR;
}

/*获取综合参数 pdrv:磁盘编号0~9 ctrl:控制代码 *buff:发送/接收缓冲区指针*/
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	switch(pdrv)
	{
		case SD_CARD:
		{
			switch(cmd)
			{
				case CTRL_SYNC:
				{
					res = RES_OK;
					break;
				}
				case GET_SECTOR_SIZE:
				{
					*(DWORD*)buff = 512;
					res = RES_OK;
					break;
				}
				case GET_BLOCK_SIZE:
				{
					*(WORD*)buff = 8;
					res = RES_OK;
					break;
				}
				case GET_SECTOR_COUNT:
				{
					//*(DWORD*)buff = W25Qxx_InitStruct.Sector_Count;
					res = RES_OK;
					break;
				}
				default: res = RES_PARERR;
			}
			return res;
		}
	}
	return res;
}

/*获得时间*/
DWORD get_fattime(void)
{				 
	return 0;
}
