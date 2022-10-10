#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <math.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define square(x) x*x
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/*ϵͳʱ������*/
void SystemClock_Configuration(void);

/*��ʱ������ʼ�� sysclk:ϵͳʱ��Ƶ��*/
void Delay_Configuration(unsigned short int sysclk);

/*��ʱus nus:��ʱ΢��*/
void Delay_Us(unsigned int nus);

/*��ʱms nms:��ʱ����*/
void Delay_Ms(unsigned short int nms);

/*����ʱus nus:��ʱ΢��*/
void EasyDelay_Us(unsigned int nus);

/*����ʱms nms:��ʱ����*/
void EasyDelay_Ms(unsigned short int nms);

#ifdef __cplusplus
}
#endif

#endif
