#ifndef __IAP_H
#define __IAP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"
#include "fatfs.h"

/*BootLoader代码区起始地址*/
#define BOOTLOARDER_CODE_ADDRESS  FLASH_BASE

/*BootLoader代码区大小(KB)*/
#define BOOTLOADER_CODE_SIZE      (100)

/*APP代码区起始地址*/
#define APP_CODE_ADDRESS          (BOOTLOARDER_CODE_ADDRESS + BOOTLOADER_CODE_SIZE*1024)

/*APP代码区大小(KB)*/
#define APP_CODE_SIZE             (400)

/*信息存储区起始地址*/
#define INFO_SAVE_ADDRESS         (APP_CODE_ADDRESS + (APP_CODE_SIZE+10)*1024)

/*信息存储区大小(KB)*/
#define INFO_SAVE_SIZE            (2)

/*最长APP文件名*/
#define APP_NAME_MAX_SIZE         (128)

/*APP有效标志位地址*/
#define APP_VALID_FLAG_ADDRESS    (INFO_SAVE_ADDRESS + APP_NAME_MAX_SIZE)

/*F1大容量系列Flash页大小(KB)*/
#define ST_FLASH_PAGE_SIZE_KB     (FLASH_PAGE_SIZE / 1024)

/*IAP错误类型定义*/
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

/*获取当前APP名称*/
void IAP_Get_CurrentAppName(void);

/*获取升级APP名称*/
FRESULT IAP_Get_UpdateAppName(void);

/*检查当前APP是否需要升级*/
bool IAP_IsNeedUpdate(void);

/*APP升级进程 *name:APP名称*/
void IAP_Update_Process(char *name);

/*设置当前APP名称 *name:APP名称*/
HAL_StatusTypeDef IAP_Set_CurrentAppName(char *name);

/*获取当前APP有效性*/
bool IAP_Get_CurrentAppValid(void);

/*设置当前APP有效性*/
HAL_StatusTypeDef IAP_Set_CurrentAppValid(void);

/*APP编程 *name:APP名称*/
HAL_StatusTypeDef IAP_AppProgram(char *name);

/*APP校验 *name:APP名称*/
HAL_StatusTypeDef IAP_AppVerify(char *name);

/*擦除信息储存区*/
HAL_StatusTypeDef IAP_Erase_InfoArea(void);

/*擦除APP储存区*/
HAL_StatusTypeDef IAP_Erase_AppArea(void);

/*运行当前APP appAddr:运行地址*/
void IAP_Run_CurrentApp(unsigned int appAddr);

/*IAP显示进度 percent:百分比*100*/
void IAP_ShowProgressBar(unsigned char percent);

/*IAP错误处理 errorType:错误类型*/
void IAP_ErrorHandle(unsigned char errorType);

/*定义函数指针类型用于实现代码跳转*/
typedef void(*ptr_code)(void);

#ifdef __cplusplus
}
#endif

#endif
