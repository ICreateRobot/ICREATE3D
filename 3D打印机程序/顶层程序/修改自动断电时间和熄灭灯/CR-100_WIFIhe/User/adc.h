#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*ADC3初始化 PC0:ADC3_IN10*/
void ADC3_Configuration(void);

/*获取AD转换值 channel:通道号*/
unsigned short int ADC3_GetValue(unsigned int channel);

/*获取AD转换平均值 channel:通道号 times:平均次数*/
unsigned short int ADC3_GetAverageValue(unsigned int channel, unsigned char times);

#ifdef __cplusplus
}
#endif

#endif
