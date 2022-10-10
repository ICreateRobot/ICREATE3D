#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

typedef struct
{
  unsigned short int Hours;
  unsigned char Minutes;
  unsigned char Seconds;
}RTC_DurationTypeDef;

/*实时时钟初始化*/
void RTC_Configuration(void);

/*设置时钟*/
void RTC_SetTime(unsigned short int Hours, unsigned char Minutes, unsigned char Seconds);

/*获取持续时间*/
RTC_DurationTypeDef RTC_Get_DurationTime(void);

#ifdef __cplusplus
}
#endif

#endif
