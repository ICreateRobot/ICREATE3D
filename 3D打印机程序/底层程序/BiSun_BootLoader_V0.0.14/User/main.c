/**
  ******************************************************************************
  * @project  BiSun_BootLoader
  * @author   T_B/LEO
	* @date     2019/01/02
	* @version  0.0.1
  * @brief    SD������IAP_BootLoader For CR-100
	******************************************************************************
**/

#include "system.h"
#include "sd_card.h"
#include "fatfs.h"
#include "usart.h"
#include "malloc.h"
#include "oled.h"
#include "iap.h"

int main(void)
{
  HAL_Init();
	Automatic_Power_Off_Configuration();
  SystemClock_Configuration();
	Delay_Configuration(72);
	USART1_Configuration(115200);
	OLED_Configuration();
	Delay_Ms(100);
  OLED_Configuration();
	Memory_Configuration(MEMORY_IN_BANK);
 	Exf_Configuration();
	if(IAP_Get_UpdateAppName() == FR_OK)  /*�п����벢����bin�ļ�*/
	{
		if(IAP_Get_CurrentAppValid() == true) /*��ǰAPP����,�ж��Ƿ���Ҫ����*/
		{
			IAP_Get_CurrentAppName();
			if(IAP_IsNeedUpdate() == true)  /*��Ҫ����*/
			{
			#if USART_DEBUG_USED
				printf("Current-App and Update-App is conflicting, updating.\r\n");
			#endif
				IAP_Update_Process(IAP_UpdateAppName);
				IAP_ErrorHandle(IAP_ERROR_APP_RUN_FAILED);
			}
			else  /*����Ҫ����,���е�ǰAPP*/
			{
			#if USART_DEBUG_USED
				printf("Current-App and Update-App is coincident, run Current-App.\r\n");
			#endif
				IAP_Run_CurrentApp(APP_CODE_ADDRESS);
				IAP_ErrorHandle(IAP_ERROR_APP_RUN_FAILED);
			}
		}
		else  /*��ǰAPP������,ֱ�Ӹ���*/
		{
		#if USART_DEBUG_USED
			printf("Current-App is invalid, updating.\r\n");
		#endif
			IAP_Update_Process(IAP_UpdateAppName);
			IAP_ErrorHandle(IAP_ERROR_APP_RUN_FAILED);
		}
	}
	else if(IAP_Get_CurrentAppValid() == true)  /*�޿ɸ����ļ�ֱ�����е�ǰAPP*/
	{
	#if USART_DEBUG_USED
		printf("Update-App is invalid, run Current-App\r\n");
	#endif
		IAP_Run_CurrentApp(APP_CODE_ADDRESS);
		IAP_ErrorHandle(IAP_ERROR_APP_RUN_FAILED);
	}
	else  /*�޿ɸ����ļ��ҵ�ǰAPP��Ч*/
	{
	#if USART_DEBUG_USED
		printf("Current-App and Update-App is invalid, need update.\r\n");
	#endif
		IAP_ErrorHandle(IAP_ERROR_APP_NEED_UPDATE);
	}
  while(1)
  {
  }
}
