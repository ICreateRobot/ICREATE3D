#ifndef __IAP_H
#define __IAP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"
#include "fatfs.h"

/*BootLoader��������ʼ��ַ*/
#define BOOTLOARDER_CODE_ADDRESS  FLASH_BASE

/*BootLoader��������С(KB)*/
#define BOOTLOADER_CODE_SIZE      (100)

/*APP��������ʼ��ַ*/
#define APP_CODE_ADDRESS          (BOOTLOARDER_CODE_ADDRESS + BOOTLOADER_CODE_SIZE*1024)

/*APP��������С(KB)*/
#define APP_CODE_SIZE             (400)

/*��Ϣ�洢����ʼ��ַ*/
#define INFO_SAVE_ADDRESS         (APP_CODE_ADDRESS + (APP_CODE_SIZE+10)*1024)

/*��Ϣ�洢����С(KB)*/
#define INFO_SAVE_SIZE            (2)

/*�APP�ļ���*/
#define APP_NAME_MAX_SIZE         (128)

/*APP��Ч��־λ��ַ*/
#define APP_VALID_FLAG_ADDRESS    (INFO_SAVE_ADDRESS + APP_NAME_MAX_SIZE)

/*F1������ϵ��Flashҳ��С(KB)*/
#define ST_FLASH_PAGE_SIZE_KB     (FLASH_PAGE_SIZE / 1024)

/*IAP�������Ͷ���*/
#define IAP_ERROR_SET_VAILD_FLAG   0x01
#define IAP_ERROR_GET_VAILD_FLAG   0x02
#define IAP_ERROR_FLASH_UNLOCK     0x03
#define IAP_ERROR_SET_APP_NAME     0x04
#define IAP_ERROR_APP_RUN_FAILED   0x05
#define IAP_ERROR_APP_NEED_UPDATE  0x06
#define IAP_ERROR_INFO_ERASE       0x07
#define IAP_ERROR_APP_ERASE        0x08
#define IAP_ERROR_APP_PROGRAM      0x09
#define IAP_ERROR_APP_VERIFY       0x0A

#define USART_DEBUG_USED  false

extern char IAP_CurrentAppName[APP_NAME_MAX_SIZE];
extern char IAP_UpdateAppName[APP_NAME_MAX_SIZE];

/*��ȡ��ǰAPP����*/
void IAP_Get_CurrentAppName(void);

/*��ȡ����APP����*/
FRESULT IAP_Get_UpdateAppName(void);

/*��鵱ǰAPP�Ƿ���Ҫ����*/
bool IAP_IsNeedUpdate(void);

/*APP�������� *name:APP����*/
void IAP_Update_Process(char *name);

/*���õ�ǰAPP���� *name:APP����*/
HAL_StatusTypeDef IAP_Set_CurrentAppName(char *name);

/*��ȡ��ǰAPP��Ч��*/
bool IAP_Get_CurrentAppValid(void);

/*���õ�ǰAPP��Ч��*/
HAL_StatusTypeDef IAP_Set_CurrentAppValid(void);

/*APP��� *name:APP����*/
HAL_StatusTypeDef IAP_AppProgram(char *name);

/*APPУ�� *name:APP����*/
HAL_StatusTypeDef IAP_AppVerify(char *name);

/*������Ϣ������*/
HAL_StatusTypeDef IAP_Erase_InfoArea(void);

/*����APP������*/
HAL_StatusTypeDef IAP_Erase_AppArea(void);

/*���е�ǰAPP appAddr:���е�ַ*/
void IAP_Run_CurrentApp(unsigned int appAddr);

/*IAP��ʾ���� percent:�ٷֱ�*100*/
void IAP_ShowProgressBar(unsigned char percent);

/*IAP������ errorType:��������*/
void IAP_ErrorHandle(unsigned char errorType);

/*���庯��ָ����������ʵ�ִ�����ת*/
typedef void(*ptr_code)(void);

#ifdef __cplusplus
}
#endif

#endif
