/**
  ******************************************************************************
  * @file     storage_device.c
  * @author   LEO
	* @date     2018/12/18
	* @version  2.0.1
  * @brief    Storage equipment operating layer
	******************************************************************************
**/

#include "storage_device.h"
#include "sd_card.h"
#include "printer.h"
#include "usart.h"
#include "fatfs.h"
#include "malloc.h"
#include "string.h"
#include "language.h"
#include "temperature.h"
#include "stepper_motor.h"
#include "hmi.h"

#if SD_SUPPORT
SD_ReaderTypeDef SD_Reader;
#endif
#if USB_SUPPORT
USB_ReaderTypeDef USB_Reader;
#endif
Storage_DeviceTypeDef Storage_Device = Storage_SD;

// Storage device execution process
void StorageDevice_Process(void)
{
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		if(SD_Card_Detect())
		{
			if(SD_Reader.Printing == true)
			{
				StorageDevice_FinishPrint();
				HMI_MemberStruct.Status = HMI_SD_ERROR;
			}
			else if(HMI_MemberStruct.Status != HMI_SD_ERROR)
			{
				HMI_MemberStruct.Status = HMI_NO_SD;
				SD_Reader.Ready = false;
			}
		}
#if SD_AUTO_MOUNT
		StorageDevice_AutoMount();
#endif
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
		USB_Host_Process();
		if(USB_Host_GetState() == USBH_OK) USB_Reader.Ready = true;
		else USB_Reader.Ready = false;
	}
#endif
}

// Automatic mounting of storage device
void StorageDevice_AutoMount(void)
{
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		if(!SD_Reader.Ready && !SD_Card_Detect()) StorageDevice_Mount();
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
	}
#endif
}

// 储存设备打印文件清单
void StorageDevice_ListFiles(void)
{
	TCHAR *path;
	FRESULT res;
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) path = ((TCHAR*)"0:");
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) path = ((TCHAR*)"2:");
#endif
	res = f_opendir(&dir, (const TCHAR*)path);
	if(res != FR_OK)
	{
	#if MANUAL_DEBUG_PRINTF
		printf("SD_ERROR_2 :%d\r\n", res);
	#endif
		return;
	}
	while(1)
	{
		res = f_readdir(&dir, &fileInfo);
		if(res!=FR_OK || fileInfo.fname[0]==0)
		{
		#if MANUAL_DEBUG_PRINTF
			if(res != FR_OK) printf("SD_ERROR_3 :%d\r\n", res);
		#endif
			break;
		}
		if((Exf_GetType((unsigned char*)fileInfo.fname)&0xF0) == 0x60) printf("%s\r\n", (unsigned char*)fileInfo.fname);
	}
}

// 储存设备获取第一个g文件名称 *fname:g文件名称
FRESULT StorageDevice_GetFirstFile(char *fname)
{
	TCHAR *path;
	FRESULT res;
	unsigned char *pname;
	FlagStatus isFindFile = RESET;
	pname = Memory_Malloc(MEMORY_IN_BANK, FF_MAX_LFN);
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) path = ((TCHAR*)"0:");
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) path = ((TCHAR*)"2:");
#endif
	res = f_opendir(&dir, (const TCHAR*)path);
	if(res == FR_OK) 
	{
		while(1)
		{
			res = f_readdir(&dir, &fileInfo);
			if(res!=FR_OK || fileInfo.fname[0]==0)
			{
				if(isFindFile == RESET)
				{
					res = FR_NO_FILE;
					break;
				}
				else
				{
					strcpy(fname, (const char*)pname);
					res = FR_OK;
					break;
				}
			}
			if((Exf_GetType((unsigned char*)fileInfo.fname)&0xF0) == 0x60)
			{
				strcpy((char*)pname, fileInfo.fname);
				isFindFile = SET;
			}
		} 
	}
	Memory_Free(MEMORY_IN_BANK, pname);
	return res;
}

// 储存设备打开文件 *fname:文件名 mode:打开方式(1,读;0,写)
void StorageDevice_OpenFile(char *fname, unsigned char mode)
{
	FRESULT res;
  unsigned char *pname;
#if SD_SUPPORT
	if((Storage_Device==Storage_SD) && !SD_Reader.Ready) return;
	if(Storage_Device == Storage_SD) SD_Reader.Printing = false;
#endif
#if USB_SUPPORT
	if((Storage_Device==Storage_USB) && !USB_Reader.Ready) return;
	if(Storage_Device == Storage_USB) USB_Reader.Printing = false;
#endif
	pname = Memory_Malloc(MEMORY_IN_BANK, FF_MAX_LFN*2+1);
  if(mode)
  {
		if(Storage_Device == Storage_SD) strcpy((char*)pname, "0:/");
		else if(Storage_Device == Storage_USB) strcpy((char*)pname, "2:/");
		strcat((char*)pname, (const char*)fname);
#if SD_SUPPORT
		if(Storage_Device == Storage_SD) res = f_open(&SD_Reader.G_File, (const TCHAR*)pname, FA_READ);
#endif
#if USB_SUPPORT
		if(Storage_Device == Storage_USB) res = f_open(&USB_Reader.G_File, (const TCHAR*)pname, FA_READ);
#endif
		if(res == FR_OK)
		{
			printf(MSG_FILE_OPENED);
			printf(fname);
			printf(MSG_FILE_SIZE);
#if SD_SUPPORT
			if(Storage_Device == Storage_SD) printf("%ld\n", f_size(&SD_Reader.G_File));
#endif
#if USB_SUPPORT
			if(Storage_Device == Storage_USB) printf("%ld\n", f_size(&USB_Reader.G_File));
#endif
			printf(MSG_FILE_SELECTED);
		}
		else
		{
			printf(MSG_OPEN_FILE_FAIL);
			printf(fname);
			printf(".");
		}
		printf("\n");
	}
  else
  {
#if SD_SUPPORT
		if(Storage_Device == Storage_SD) res = f_open(&SD_Reader.G_File, (const TCHAR*)fname, FA_WRITE|FA_OPEN_ALWAYS);
#endif
#if USB_SUPPORT
		if(Storage_Device == Storage_USB) res = f_open(&USB_Reader.G_File, (const TCHAR*)fname, FA_WRITE|FA_OPEN_ALWAYS);
#endif
    if(res == FR_OK)
    {
#if SD_SUPPORT
			if(Storage_Device == Storage_SD) SD_Reader.Saving = true;
#endif
#if USB_SUPPORT
			if(Storage_Device == Storage_USB) USB_Reader.Saving = true;
#endif
			printf(MSG_WRITE_TO_FILE);
			printf(fname);
    }
    else
    {
      printf(MSG_OPEN_FILE_FAIL);
      printf(fname);
      printf(".");
    }
		printf("\n");
  }
	Memory_Free(MEMORY_IN_BANK, pname);
}

// 储存设备关闭当前文件
void StorageDevice_CloseFile(void)
{
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		f_close(&SD_Reader.G_File);
		SD_Reader.Saving = false;
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
		f_close(&USB_Reader.G_File);
		USB_Reader.Saving = false;
	}
#endif
}

// 储存设备删除文件 *fname:文件名
void StorageDevice_DeleteFile(char *fname)
{
	FRESULT res;
  res = f_unlink(fname);
  if(res == FR_OK)
	{
		printf("File deleted:");
		printf(fname);
	}
	else
	{
		printf("Deletion failed, File: ");
		printf(fname);
		printf(".");
	}
	printf("\n");
}

/*储存设备读取文件*/
signed short int StorageDevice_ReadFile(void)
{
	unsigned char buffer[128];
	FRESULT res;
	UINT br;
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) res = f_read(&SD_Reader.G_File, buffer, 1, &br);
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) res = f_read(&USB_Reader.G_File, buffer, 1, &br);
#endif
	if(res == FR_OK) return *buffer;
	else
	{
	#if MANUAL_DEBUG_PRINTF
		printf("SD_ERROR_1 :%d\r\n", res);
	#endif
		return -1;
	}
}

/*储存设备判断文件是否结束*/
bool StorageDevice_IsFileEnd(void)
{
	int res;
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) res = f_eof(&SD_Reader.G_File);
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) res = f_eof(&USB_Reader.G_File);
#endif
	return res;
}

/*储存设备挂载*/
void StorageDevice_Mount(void)
{
  printf(SERIAL_ECHO_START);
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		if(f_mount(fatfs[0], "0:", 1) == FR_OK)
		{
			SD_Reader.Ready = true;
			HMI_MemberStruct.Status = HMI_READY;
			printf(MSG_SD_CARD_OK);
		}
		else 
		{
			SD_Reader.Ready = false;
			HMI_MemberStruct.Status = HMI_NO_SD;
			printf(MSG_SD_INIT_FAIL);
		}
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
	}
#endif
}

/*储存设备释放*/
void StorageDevice_Release(void)
{
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		f_mount(NULL, "0:", 1);
		SD_Reader.Printing = false;
		SD_Reader.Ready = false;
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
		f_mount(NULL, "2:", 0);
		USB_Reader.Printing = false;
		USB_Reader.Ready = false;
	}
#endif
}

/*储存设备开始打印*/
void StorageDevice_StartPrint(void)
{
#if SD_SUPPORT
	if(Storage_Device==Storage_SD && SD_Reader.Ready) SD_Reader.Printing = true;
#endif
#if USB_SUPPORT
	if(Storage_Device==Storage_USB && USB_Reader.Ready) USB_Reader.Printing = true;
#endif
}

/*储存设备暂停打印*/
void StorageDevice_PausePrint(void)
{
#if SD_SUPPORT
	if(Storage_Device==Storage_SD && SD_Reader.Printing) SD_Reader.Printing = false;
#endif
#if USB_SUPPORT
	if(Storage_Device==Storage_USB && USB_Reader.Printing) USB_Reader.Printing = false;
#endif
}

/*储存设备设置文件指针 pointer:指针位置*/
void StorageDevice_SetPointer(long pointer)
{
#if SD_SUPPORT
	if(Storage_Device==Storage_SD && SD_Reader.Printing) f_lseek(&SD_Reader.G_File, pointer);
#endif
#if USB_SUPPORT
	if(Storage_Device==Storage_USB && USB_Reader.Printing) f_lseek(&USB_Reader.G_File, pointer);
#endif
}

/*储存设备获取打印进度*/
void StorageDevice_GetProgress(void)
{
#if SD_SUPPORT
	if(Storage_Device == Storage_SD)
	{
		if(SD_Reader.Ready)
		{
			printf(MSG_FILE_PRINTING_BYTE);
			printf("%d / %d", f_tell(&SD_Reader.G_File), f_size(&SD_Reader.G_File));
		}
		else
		{
			printf(MSG_FILE_NOT_PRINTING);
		}
	}
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB)
	{
		if(USB_Reader.Ready)
		{
			printf(MSG_FILE_PRINTING_BYTE);
			printf("%d / %d", f_tell(&USB_Reader.G_File), f_size(&USB_Reader.G_File));
		}
		else
		{
			printf(MSG_FILE_NOT_PRINTING);
		}
	}
#endif
	printf("\n");
}

/*储存设备完成打印*/
void StorageDevice_FinishPrint(void)
{
	Stepper_Synchronize();
	Stepper_QuickStop();
	StorageDevice_CloseFile();
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) SD_Reader.Printing = false;
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) USB_Reader.Printing = false;
#endif
	if(FILE_FINISHED_STEPPERRELEASE)
	{
		enquecommand((const char*)(FILE_FINISHED_RELEASECOMMAND));
	}
	Heater_AutoShutdown();
}

/*储存设备拼接文件名及盘符地址 *pname:拼接后存放地址 *name:文件名*/
char* StorageDevice_PathCatName(char *pname, char *name)
{
	const char chgchar[2] = {0x5C, 0x00}; //等效"\"
#if SD_SUPPORT
	if(Storage_Device == Storage_SD) strcpy((char *)pname, (const char*)"0:");
#endif
#if USB_SUPPORT
	if(Storage_Device == Storage_USB) strcpy((char *)pname, (const char*)"2:");
#endif
	strcat((char*)pname, (const char*)chgchar);
	strcat((char*)pname, (const char*)name);
	return pname;
}
