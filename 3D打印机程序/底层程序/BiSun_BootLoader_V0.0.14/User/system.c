/**
  ******************************************************************************
  * @file     system.c
  * @author   LEO
	* @date     2018/11/16
	* @version  0.1.1
  * @brief    时钟配置,延时配置
	* @update   修正EasyDelay_Us函数
	******************************************************************************
**/
#include "system.h"

static unsigned int fac_us = 0;

/*系统时钟配置*/
void SystemClock_Configuration(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) while(1);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) while(1);
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_RTC;
  RCC_PeriphCLKInitStruct.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	RCC_PeriphCLKInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  if(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK) while(1);
	 __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

/*延时函数初始化 sysclk:系统时钟频率*/
void Delay_Configuration(unsigned short int sysclk)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	fac_us = sysclk;
}

/*延时us nus:延时微秒*/
void Delay_Us(unsigned int nus)
{
	unsigned int ticks;
	unsigned int told, tnow, tcnt=0;
	unsigned int reload = SysTick->LOAD;
	ticks = nus * fac_us;
	told = SysTick->VAL;
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told) tcnt += told-tnow;
			else tcnt += reload-tnow+told;
			told = tnow;
			if(tcnt >= ticks) break;	
		}
	}
}

/*延时ms nms:延时毫秒*/
void Delay_Ms(unsigned short int nms)
{
	unsigned int counter;
	for(counter=0; counter<nms; counter++) Delay_Us(1000);
}

/*简单延时us nus:延时微秒*/
void EasyDelay_Us(unsigned int nus)
{
	unsigned int counter = 12 * nus;
	while(counter--){};
}

/*简单延时ms nms:延时毫秒*/
void EasyDelay_Ms(unsigned short int nms)
{
	unsigned int counter;
	for(counter=0; counter<nms; counter++) EasyDelay_Us(1000);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	printf("%S,  %d", (unsigned short *)file, line);
}
#endif
