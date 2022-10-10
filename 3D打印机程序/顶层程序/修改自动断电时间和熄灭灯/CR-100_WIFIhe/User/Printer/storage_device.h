#ifndef __STORAGE_DEVICE_H
#define __STORAGE_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"
#include "ff.h"	  
#include "printer_conf.h"

typedef enum
{
	LS_SerialPrint = 0,
	LS_Count,
	LS_GetFilename
}LS_ActionTypeDef;

#if SD_SUPPORT
typedef struct
{
	bool Saving;
	bool Printing;
	bool Ready;
	unsigned long AutoStart_Atmillis;
	unsigned long Pos;
	bool AutoStart_StilltoCheck;
	LS_ActionTypeDef LS_Action; //stored for recursion.
	unsigned char FileName[32];
	FIL G_File;
}SD_ReaderTypeDef;

extern SD_ReaderTypeDef SD_Reader;
#endif

#if USB_SUPPORT
typedef struct
{
	bool Saving;
	bool Printing;
	bool Ready;
	unsigned long AutoStart_Atmillis;
	unsigned long Pos;
	bool AutoStart_StilltoCheck;
	LS_ActionTypeDef LS_Action; //stored for recursion.
	unsigned char FileName[32];
	FIL G_File;
}USB_ReaderTypeDef;

extern USB_ReaderTypeDef USB_Reader;
#endif

typedef enum
{
	Storage_SD = 0,
	Storage_USB
}Storage_DeviceTypeDef;

extern Storage_DeviceTypeDef Storage_Device;

/*储存设备执行进程*/
void StorageDevice_Process(void);

/*储存设备自动挂载*/
void StorageDevice_AutoMount(void);

/*储存设备打印文件清单*/
void StorageDevice_ListFiles(void);

/*储存设备获取第一个g文件名称 *fname:g文件名称*/
FRESULT StorageDevice_GetFirstFile(char *fname);

/*储存设备打开文件 *fname:文件名 mode:打开方式(1,读;0,写)*/
void StorageDevice_OpenFile(char *fname, unsigned char mode);

/*储存设备关闭当前文件*/
void StorageDevice_CloseFile(void);

/*储存设备删除文件 *fname:文件名*/
void StorageDevice_DeleteFile(char *fname);

/*储存设备读取文件*/
signed short int StorageDevice_ReadFile(void);

/*储存设备判断文件是否结束*/
bool StorageDevice_IsFileEnd(void);

/*储存设备挂载*/
void StorageDevice_Mount(void);

/*储存设备释放*/
void StorageDevice_Release(void);

/*储存设备开始打印*/
void StorageDevice_StartPrint(void);

/*储存设备暂停打印*/
void StorageDevice_PausePrint(void);

/*储存设备设置文件指针 pointer:指针位置*/
void StorageDevice_SetPointer(long pointer);

/*储存设备获取打印进度*/
void StorageDevice_GetProgress(void);

/*储存设备完成打印*/
void StorageDevice_FinishPrint(void);

/*储存设备拼接文件名及盘符地址 *pname:拼接后存放地址 *name:文件名*/
char* StorageDevice_PathCatName(char *pname, char *name);

#ifdef __cplusplus
}
#endif

#endif
