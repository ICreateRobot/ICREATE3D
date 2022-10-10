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

/*�����豸ִ�н���*/
void StorageDevice_Process(void);

/*�����豸�Զ�����*/
void StorageDevice_AutoMount(void);

/*�����豸��ӡ�ļ��嵥*/
void StorageDevice_ListFiles(void);

/*�����豸��ȡ��һ��g�ļ����� *fname:g�ļ�����*/
FRESULT StorageDevice_GetFirstFile(char *fname);

/*�����豸���ļ� *fname:�ļ��� mode:�򿪷�ʽ(1,��;0,д)*/
void StorageDevice_OpenFile(char *fname, unsigned char mode);

/*�����豸�رյ�ǰ�ļ�*/
void StorageDevice_CloseFile(void);

/*�����豸ɾ���ļ� *fname:�ļ���*/
void StorageDevice_DeleteFile(char *fname);

/*�����豸��ȡ�ļ�*/
signed short int StorageDevice_ReadFile(void);

/*�����豸�ж��ļ��Ƿ����*/
bool StorageDevice_IsFileEnd(void);

/*�����豸����*/
void StorageDevice_Mount(void);

/*�����豸�ͷ�*/
void StorageDevice_Release(void);

/*�����豸��ʼ��ӡ*/
void StorageDevice_StartPrint(void);

/*�����豸��ͣ��ӡ*/
void StorageDevice_PausePrint(void);

/*�����豸�����ļ�ָ�� pointer:ָ��λ��*/
void StorageDevice_SetPointer(long pointer);

/*�����豸��ȡ��ӡ����*/
void StorageDevice_GetProgress(void);

/*�����豸��ɴ�ӡ*/
void StorageDevice_FinishPrint(void);

/*�����豸ƴ���ļ������̷���ַ *pname:ƴ�Ӻ��ŵ�ַ *name:�ļ���*/
char* StorageDevice_PathCatName(char *pname, char *name);

#ifdef __cplusplus
}
#endif

#endif
