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

/*ʵʱʱ�ӳ�ʼ��*/
void RTC_Configuration(void);

/*����ʱ��*/
void RTC_SetTime(unsigned short int Hours, unsigned char Minutes, unsigned char Seconds);

/*��ȡ����ʱ��*/
RTC_DurationTypeDef RTC_Get_DurationTime(void);

#ifdef __cplusplus
}
#endif

#endif
