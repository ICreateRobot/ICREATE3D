/**
  ******************************************************************************
  * @file     adc.c
  * @author   LEO
	* @date     2018/11/20
	* @version  0.1.1
  * @brief    ADC3_IN4:E0温度
	* @update   ADC3_IN10:E0温度 去除热床温度
	******************************************************************************
**/
#include "adc.h"

ADC_HandleTypeDef ADC3_HandleStruct;

/*ADC3初始化 PC0:ADC3_IN10*/
void ADC3_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_ADC3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  ADC3_HandleStruct.Instance = ADC3;
  ADC3_HandleStruct.Init.ScanConvMode = ADC_SCAN_DISABLE;
  ADC3_HandleStruct.Init.ContinuousConvMode = DISABLE;
  ADC3_HandleStruct.Init.DiscontinuousConvMode = DISABLE;
  ADC3_HandleStruct.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  ADC3_HandleStruct.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  ADC3_HandleStruct.Init.NbrOfConversion = 1;
  HAL_ADC_Init(&ADC3_HandleStruct);
	HAL_ADCEx_Calibration_Start(&ADC3_HandleStruct);
}

/*获取AD转换值 channel:通道号*/
unsigned short int ADC3_GetValue(unsigned int channel)
{
	ADC_ChannelConfTypeDef ADC_ChannelStruct;
  ADC_ChannelStruct.Channel = channel;
  ADC_ChannelStruct.Rank = ADC_REGULAR_RANK_1;
  ADC_ChannelStruct.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  HAL_ADC_ConfigChannel(&ADC3_HandleStruct, &ADC_ChannelStruct);
	HAL_ADC_Start(&ADC3_HandleStruct);
	HAL_ADC_PollForConversion(&ADC3_HandleStruct, 100);
	return (unsigned short int)HAL_ADC_GetValue(&ADC3_HandleStruct);
}

/*获取AD转换平均值 channel:通道号 times:平均次数*/
unsigned short int ADC3_GetAverageValue(unsigned int channel, unsigned char times)
{
	unsigned int tempValue = 0;
	unsigned char tempCounter;
	for(tempCounter=0; tempCounter<times; tempCounter++) tempValue += ADC3_GetValue(channel);
	return (tempValue / times);
}
