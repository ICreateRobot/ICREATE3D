#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*ADC3��ʼ�� PC0:ADC3_IN10*/
void ADC3_Configuration(void);

/*��ȡADת��ֵ channel:ͨ����*/
unsigned short int ADC3_GetValue(unsigned int channel);

/*��ȡADת��ƽ��ֵ channel:ͨ���� times:ƽ������*/
unsigned short int ADC3_GetAverageValue(unsigned int channel, unsigned char times);

#ifdef __cplusplus
}
#endif

#endif
