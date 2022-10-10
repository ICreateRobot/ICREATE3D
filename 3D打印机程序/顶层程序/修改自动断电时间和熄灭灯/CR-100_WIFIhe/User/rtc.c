/**
  ******************************************************************************
  * @file     rtc.c
  * @author   LEO
	* @date     2018/11/19
	* @version  0.1.1
  * @brief    实时时钟操作底层
	******************************************************************************
**/
#include "rtc.h"

RTC_HandleTypeDef RTC_HandleStruct;

/*实时时钟初始化*/
void RTC_Configuration(void)
{
	RTC_TimeTypeDef RTC_TimeStruct = {0, 0, 0};
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_BKP_CLK_ENABLE();
	__HAL_RCC_RTC_ENABLE();
  RTC_HandleStruct.Instance = RTC;
  RTC_HandleStruct.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  RTC_HandleStruct.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  HAL_RTC_Init(&RTC_HandleStruct);
	HAL_RTC_SetTime(&RTC_HandleStruct, &RTC_TimeStruct, RTC_FORMAT_BIN);
}

/*设置时钟*/
void RTC_SetTime(unsigned short int Hours, unsigned char Minutes, unsigned char Seconds)
{
	RTC_TimeTypeDef RTC_TimeStruct = {0, 0, 0};
	RTC_TimeStruct.Hours = Hours;
	RTC_TimeStruct.Minutes = Minutes;
	RTC_TimeStruct.Seconds = Seconds;
	HAL_RTC_SetTime(&RTC_HandleStruct, &RTC_TimeStruct, RTC_FORMAT_BIN);
}

/*获取持续时间*/
RTC_DurationTypeDef RTC_Get_DurationTime(void)
{
	RTC_DurationTypeDef RTC_DurationStruct;
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	HAL_RTC_GetTime(&RTC_HandleStruct, &RTC_TimeStruct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_HandleStruct, &RTC_DateStruct, RTC_FORMAT_BIN);
	RTC_DurationStruct.Hours = (RTC_DateStruct.Date-1)*24 + RTC_TimeStruct.Hours;
	RTC_DurationStruct.Minutes = RTC_TimeStruct.Minutes;
	RTC_DurationStruct.Seconds = RTC_TimeStruct.Seconds;
	return RTC_DurationStruct;
}
