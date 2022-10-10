/**
  ******************************************************************************
  * @file     iap.c
  * @author   LEO
	* @date     2019/01/02
	* @version  0.0.1
  * @brief    IAP驱动底层
	******************************************************************************
**/

#include "iap.h"
#include "sd_card.h"
#include "oled.h"
#include "usart.h"

ptr_code ptr_app = NULL;

char IAP_CurrentAppName[APP_NAME_MAX_SIZE];
char IAP_UpdateAppName[APP_NAME_MAX_SIZE];
unsigned char readBuf[FLASH_PAGE_SIZE];
unsigned char updatePercent = 0;

/*获取当前APP名称*/
void IAP_Get_CurrentAppName(void)
{
	unsigned char count;
	memset(IAP_CurrentAppName, 0, APP_NAME_MAX_SIZE);
	for(count=0; count<APP_NAME_MAX_SIZE-1; count++)
	{
		IAP_CurrentAppName[count] = (unsigned char)(*(volatile unsigned int*)(INFO_SAVE_ADDRESS + count));
	}
}

/*获取升级APP名称*/
FRESULT IAP_Get_UpdateAppName(void)
{
	FRESULT res = FR_DISK_ERR;
	memset(IAP_UpdateAppName, 0, APP_NAME_MAX_SIZE);
	if(SD_Card_Detect()) return res;
	else
	{
		res = f_mount(fatfs[0], "0:", 1);
		if(res == FR_OK)
		{
			res = f_opendir(&dir, "0:");
			if(res == FR_OK)
			{
				while(1)
				{
					res = f_readdir(&dir, &fileInfo);
					if(res!=FR_OK || fileInfo.fname[0]==0) return FR_NO_FILE;
					if((Exf_GetType((unsigned char*)fileInfo.fname)&0xF0) == 0x00)
					{
						strcpy((char*)IAP_UpdateAppName, fileInfo.fname);
						break;
					}
				}
			}
			f_closedir(&dir);
		}
		else f_unmount("0:");
	}
	return res;
}

/*检查当前APP是否需要升级*/
bool IAP_IsNeedUpdate(void)
{
	if(strcmp((char*)IAP_CurrentAppName, (char*)IAP_UpdateAppName) != 0) return true;
	return false;
}

/*APP升级进程 *name:APP名称*/
void IAP_Update_Process(char *name)
{
	OLED_ShowText(0, 0, "APP Updating", 0, 16);
	IAP_ShowProgressBar(updatePercent);
	OLED_ShowText(102, 4, "%", 0, 8);
	if(HAL_FLASH_Unlock() != HAL_OK) IAP_ErrorHandle(IAP_ERROR_FLASH_UNLOCK);
	Delay_Ms(300);
	if(IAP_Erase_InfoArea() != HAL_OK) IAP_ErrorHandle(IAP_ERROR_INFO_ERASE);
	Delay_Ms(300);
	if(IAP_Erase_AppArea() != HAL_OK) IAP_ErrorHandle(IAP_ERROR_APP_ERASE);
	if(IAP_AppProgram(name) != HAL_OK) IAP_ErrorHandle(IAP_ERROR_APP_PROGRAM);
	if(IAP_AppVerify(name) != HAL_OK) IAP_ErrorHandle(IAP_ERROR_APP_VERIFY);
	if(IAP_Set_CurrentAppName(IAP_UpdateAppName)) IAP_ErrorHandle(IAP_ERROR_SET_APP_NAME);
	if(IAP_Set_CurrentAppValid()) IAP_ErrorHandle(IAP_ERROR_SET_VAILD_FLAG);
	HAL_FLASH_Lock();
	if(IAP_Get_CurrentAppValid() != true) IAP_ErrorHandle(IAP_ERROR_GET_VAILD_FLAG);
	else
	{
		IAP_ShowProgressBar(100);
		OLED_ShowText(0, 6, "                    ", 0, 12);
		OLED_ShowText(0, 6, "Finish.", 0, 12);
		Delay_Ms(800);
		IAP_Run_CurrentApp(APP_CODE_ADDRESS);
	}
}

/*APP编程 *name:APP名称*/
HAL_StatusTypeDef IAP_AppProgram(char *name)
{
	char fileName[APP_NAME_MAX_SIZE + 4];
	unsigned int flashPage=0, br, count;
	HAL_StatusTypeDef status = HAL_OK;
	memset(fileName, 0, APP_NAME_MAX_SIZE+4);
	strcpy(fileName, "0:/");
	strcat(fileName, name);
	if(f_open(file, fileName, FA_READ|FA_OPEN_EXISTING) != FR_OK) status = HAL_ERROR;
	else
	{
		OLED_ShowText(0, 6, "                    ", 0, 12);
		OLED_ShowText(0, 6, "Programming.", 0, 12);
		while(!f_eof(file))
		{
			memset(readBuf, 0xFF, FLASH_PAGE_SIZE);
			if(f_read(file, readBuf, FLASH_PAGE_SIZE, &br) != FR_OK)
			{
				status = HAL_ERROR;
				break;
			}
			if(br!=FLASH_PAGE_SIZE && !f_eof(file))
			{
				status = HAL_ERROR;
				break;
			}
			for(count=0; count<(FLASH_PAGE_SIZE/2); count++)
			{
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_CODE_ADDRESS+flashPage*FLASH_PAGE_SIZE+count*2, (uint64_t)((readBuf[count*2+1]<<8)|readBuf[count*2]));
			}
			flashPage++;
			IAP_ShowProgressBar(updatePercent + (unsigned char)(f_tell(file)*40/f_size(file)));
		}
		updatePercent += 40;
	}
	f_close(file);
	return status;
}

/*APP校验 *name:APP名称*/
HAL_StatusTypeDef IAP_AppVerify(char *name)
{
	char fileName[APP_NAME_MAX_SIZE + 4];
	unsigned int br;
	unsigned long flashPage = 0;
	HAL_StatusTypeDef status = HAL_OK;
	memset(fileName, 0, APP_NAME_MAX_SIZE+4);
	strcpy(fileName, "0:/");
	strcat(fileName, name);
	if(f_open(file, fileName, FA_READ|FA_OPEN_EXISTING) != FR_OK) status = HAL_ERROR;
	else
	{
		OLED_ShowText(0, 6, "                    ", 0, 12);
		OLED_ShowText(0, 6, "Verifying.", 0, 12);
		while(!f_eof(file))
		{
			memset(readBuf, 0xFF, FLASH_PAGE_SIZE);
			if(f_read(file, readBuf, FLASH_PAGE_SIZE, &br) != FR_OK)
			{
				status = HAL_ERROR;
				break;
			}
			else if(memcmp(readBuf, (unsigned int*)(APP_CODE_ADDRESS + flashPage*FLASH_PAGE_SIZE), FLASH_PAGE_SIZE))
			{
				status = HAL_ERROR;
				break;
			}
			flashPage++;
			IAP_ShowProgressBar(updatePercent + (unsigned char)(f_tell(file)*40/f_size(file)));
		}
		updatePercent += 40;
	}
	f_close(file);
	return status;
}

/*设置当前APP名称 *name:APP名称*/
HAL_StatusTypeDef IAP_Set_CurrentAppName(char *name)
{
	HAL_StatusTypeDef status = HAL_OK;
	unsigned char count;
	unsigned char halfWord;
	if(strlen(name) > APP_NAME_MAX_SIZE-2) return HAL_ERROR;
	if(strlen(name)%2 == 0) halfWord = strlen(name) / 2;
	else halfWord = strlen(name)/2 + 1;
	for(count=0; count<halfWord; count++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, INFO_SAVE_ADDRESS+count*2, (uint64_t)((name[count*2+1]<<8)|name[count*2])) != HAL_OK) status = HAL_ERROR;
	}
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, INFO_SAVE_ADDRESS+count*2, (uint64_t)0x00) != HAL_OK) status = HAL_ERROR;
	return status;
}

/*获取当前APP有效性*/
bool IAP_Get_CurrentAppValid(void)
{
	bool vaild = false;
	if((unsigned char)(*(volatile unsigned int*)(APP_VALID_FLAG_ADDRESS))==0xAA && (unsigned char)(*(volatile unsigned int*)(APP_VALID_FLAG_ADDRESS+1))==0x55) vaild = true;
	return vaild;
}

/*设置当前APP有效性*/
HAL_StatusTypeDef IAP_Set_CurrentAppValid(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_VALID_FLAG_ADDRESS, (uint64_t)0x55AA);
	return status;
}

/*擦除信息储存区*/
HAL_StatusTypeDef IAP_Erase_InfoArea(void)
{
	HAL_StatusTypeDef status = HAL_OK;
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
	unsigned int pageError;
	IAP_ShowProgressBar(updatePercent += 2);
	OLED_ShowText(0, 6, "                    ", 0, 12);
	OLED_ShowText(0, 6, "Erase Info.", 0, 12);
	FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	FLASH_EraseInitStruct.PageAddress = INFO_SAVE_ADDRESS;
	FLASH_EraseInitStruct.NbPages = INFO_SAVE_SIZE / ST_FLASH_PAGE_SIZE_KB;
	status = HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &pageError);
	return status;
}

/*擦除APP储存区*/
HAL_StatusTypeDef IAP_Erase_AppArea(void)
{
	unsigned int pageError;
	HAL_StatusTypeDef status = HAL_OK;
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
	FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	FLASH_EraseInitStruct.PageAddress = APP_CODE_ADDRESS;
	FLASH_EraseInitStruct.NbPages = APP_CODE_SIZE / ST_FLASH_PAGE_SIZE_KB;
	IAP_ShowProgressBar(updatePercent += 8);
	OLED_ShowText(0, 6, "                    ", 0, 12);
	OLED_ShowText(0, 6, "Erase Flash.", 0, 12);
	status = HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &pageError);
	return status;
}

/*运行当前APP appAddr:运行地址*/
void IAP_Run_CurrentApp(unsigned int appAddr)
{
	if(((*(volatile unsigned int*)appAddr)&0x2FFE0000) == 0x20000000)
	{
		ptr_app = (ptr_code)*(volatile unsigned int*)(appAddr + 4);
		__set_MSP(*(volatile unsigned int*)appAddr);
		ptr_app();
	}
}

/*IAP显示进度 percent:百分比*100*/
void IAP_ShowProgressBar(unsigned char percent)
{
	OLED_ShowProgressBar(0, 4, 82, 8, percent);
	OLED_ShowNum(84, 4, percent, 8, 3, false);
}

/*IAP错误处理 errorType:错误类型*/
void IAP_ErrorHandle(unsigned char errorType)
{
	OLED_Clear();
	switch(errorType)
	{
		case IAP_ERROR_SET_VAILD_FLAG:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_SET_VAILD_FLAG");
		#endif
			OLED_ShowText(0, 0, "ERROR:SET_VAILD_FLAG", 0, 8);
		}break;
		case IAP_ERROR_GET_VAILD_FLAG:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_GET_VAILD_FLAG");
		#endif
			OLED_ShowText(0, 0, "ERROR:GET_VAILD_FLAG", 0, 8);
		}break;
		case IAP_ERROR_FLASH_UNLOCK:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_FLASH_UNLOCK");
		#endif
			OLED_ShowText(0, 0, "ERROR:FLASH_UNLOCK", 0, 8);
		}break;
		case IAP_ERROR_SET_APP_NAME:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_SET_APP_NAME");
		#endif
			OLED_ShowText(0, 0, "ERROR:SET_APP_NAME", 0, 8);
		}break;
		case IAP_ERROR_APP_RUN_FAILED:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_APP_RUN_FAILED");
		#endif
			OLED_ShowText(0, 0, "ERROR:APP_RUN_FAILED", 0, 8);
		}break;
		case IAP_ERROR_APP_NEED_UPDATE:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_APP_NEED_UPDATE");
		#endif
			OLED_ShowText(0, 0, "App is invalid!", 0, 16);
			OLED_ShowText(0, 3, "Please put *.bin in", 0, 8);
			OLED_ShowText(0, 5, "SD-Crad and restart", 0, 8);
		}break;
		case IAP_ERROR_INFO_ERASE:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_INFO_ERASE");
		#endif
			OLED_ShowText(0, 0, "ERROR:INFO_ERASE", 0, 8);
		}break;
		case IAP_ERROR_APP_ERASE:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_APP_ERASE");
		#endif
			OLED_ShowText(0, 0, "ERROR:APP_ERASE", 0, 8);
		}break;
		case IAP_ERROR_APP_PROGRAM:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_APP_PROGRAM");
		#endif
			OLED_ShowText(0, 0, "ERROR:APP_PROGRAM", 0, 8);
		}break;
		case IAP_ERROR_APP_VERIFY:
		{
		#if USART_DEBUG_USED
			printf("IAP_ERROR_APP_VERIFY");
		#endif
			OLED_ShowText(0, 0, "ERROR:APP_VERIFY", 0, 8);
		}break;
	}
	while(1){};
}
