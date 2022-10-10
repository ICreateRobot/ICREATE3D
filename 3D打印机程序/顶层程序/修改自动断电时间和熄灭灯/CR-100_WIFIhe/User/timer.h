#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

extern TIM_HandleTypeDef TIM1_HandleStruct, TIM3_HandleStruct, TIM5_HandleStruct, TIM6_HandleStruct;

/*定时器1初始化 prescaler:预分频值 period:重装载值 TIM1_CH1_IC(PA8):Remote*/
void TIM1_Configuration(unsigned short int prescaler, unsigned short int period);

/*定时器3初始化 prescaler:预分频值 period:重装载值*/
void TIM3_Configuration(unsigned short int prescaler, unsigned short int period);

/*定时器5初始化 prescaler:预分频值 period:重装载值 TIM5_CH2_PWM(PA1):E0_Heat*/
void TIM5_Configuration(unsigned short int prescaler, unsigned short int period);

/*定时器6初始化 prescaler:预分频值 period:重装载值*/
void TIM6_Configuration(unsigned short int prescaler, unsigned short int period);

#ifdef __cplusplus
}
#endif

#endif
