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

/*系统时钟配置*/
void SystemClock_Configuration(void);

/*延时函数初始化 sysclk:系统时钟频率*/
void Delay_Configuration(unsigned short int sysclk);

/*延时us nus:延时微秒*/
void Delay_Us(unsigned int nus);

/*延时ms nms:延时毫秒*/
void Delay_Ms(unsigned short int nms);

/*简单延时us nus:延时微秒*/
void EasyDelay_Us(unsigned int nus);

/*简单延时ms nms:延时毫秒*/
void EasyDelay_Ms(unsigned short int nms);

#ifdef __cplusplus
}
#endif

#endif
