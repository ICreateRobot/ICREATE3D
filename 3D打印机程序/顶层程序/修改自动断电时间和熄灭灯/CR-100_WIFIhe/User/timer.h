#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

extern TIM_HandleTypeDef TIM1_HandleStruct, TIM3_HandleStruct, TIM5_HandleStruct, TIM6_HandleStruct;

/*��ʱ��1��ʼ�� prescaler:Ԥ��Ƶֵ period:��װ��ֵ TIM1_CH1_IC(PA8):Remote*/
void TIM1_Configuration(unsigned short int prescaler, unsigned short int period);

/*��ʱ��3��ʼ�� prescaler:Ԥ��Ƶֵ period:��װ��ֵ*/
void TIM3_Configuration(unsigned short int prescaler, unsigned short int period);

/*��ʱ��5��ʼ�� prescaler:Ԥ��Ƶֵ period:��װ��ֵ TIM5_CH2_PWM(PA1):E0_Heat*/
void TIM5_Configuration(unsigned short int prescaler, unsigned short int period);

/*��ʱ��6��ʼ�� prescaler:Ԥ��Ƶֵ period:��װ��ֵ*/
void TIM6_Configuration(unsigned short int prescaler, unsigned short int period);

#ifdef __cplusplus
}
#endif

#endif
