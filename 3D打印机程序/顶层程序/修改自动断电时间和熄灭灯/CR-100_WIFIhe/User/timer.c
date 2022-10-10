/**
  ******************************************************************************
  * @file     timer.c
  * @author   LEO
	* @date     2018/11/23
	* @version  0.1.2
  * @brief    TIM1_CH1(IC):遥控器定时及捕获
							TIM3:电机中断
							TIM5_CH2(PWM):E0加热
							TIM6:温控中断
	* @update   去除TIM4改用滴答定时器HAL_IncTick()
							适应CR-100 去除热床加热及双风扇
	******************************************************************************
**/
#include "timer.h"
#include "stepper_motor.h"
#include "temperature.h"
#include "remote.h"

TIM_HandleTypeDef TIM1_HandleStruct, TIM3_HandleStruct, TIM5_HandleStruct, TIM6_HandleStruct;

/*定时器1初始化 prescaler:预分频值 period:重装载值 TIM1_CH1_IC(PA8):Remote*/
void TIM1_Configuration(unsigned short int prescaler, unsigned short int period)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_IC_InitTypeDef TIM1_IC_InitStruct;
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	TIM1_HandleStruct.Instance = TIM1;
	TIM1_HandleStruct.Init.Prescaler = prescaler;
	TIM1_HandleStruct.Init.Period = period;
	TIM1_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM1_HandleStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_IC_Init(&TIM1_HandleStruct);
	TIM1_IC_InitStruct.ICPolarity = TIM_ICPOLARITY_RISING;
	TIM1_IC_InitStruct.ICSelection = TIM_ICSELECTION_DIRECTTI;
	TIM1_IC_InitStruct.ICPrescaler = TIM_ICPSC_DIV1;
	TIM1_IC_InitStruct.ICFilter = 0x03;
	HAL_TIM_IC_ConfigChannel(&TIM1_HandleStruct, &TIM1_IC_InitStruct, TIM_CHANNEL_1);
	HAL_NVIC_SetPriority(TIM1_CC_IRQn, 6, 1);
	HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
	HAL_TIM_IC_Start_IT(&TIM1_HandleStruct, TIM_CHANNEL_1);
	__HAL_TIM_ENABLE_IT(&TIM1_HandleStruct, TIM_IT_UPDATE);
}

/*定时器3初始化 prescaler:预分频值 period:重装载值*/
void TIM3_Configuration(unsigned short int prescaler, unsigned short int period)
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_HandleStruct.Instance = TIM3;
	TIM3_HandleStruct.Init.Prescaler = prescaler;
	TIM3_HandleStruct.Init.Period = period;
	TIM3_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TIM3_HandleStruct);
	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Start_IT(&TIM3_HandleStruct);
}

/*定时器5初始化 prescaler:预分频值 period:重装载值 TIM5_CH2_PWM(PA1):E0_Heat*/
void TIM5_Configuration(unsigned short int prescaler, unsigned short int period)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  TIM_OC_InitTypeDef TIM_OC_InitStruct;
	__HAL_RCC_TIM5_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  TIM5_HandleStruct.Instance = TIM5;
  TIM5_HandleStruct.Init.Prescaler = prescaler;
  TIM5_HandleStruct.Init.Period = period;
	TIM5_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
  TIM5_HandleStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM5_HandleStruct.Init.RepetitionCounter = 0;
  HAL_TIM_PWM_Init(&TIM5_HandleStruct);
  TIM_OC_InitStruct.OCMode = TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.Pulse = 0;
  TIM_OC_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
  TIM_OC_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
	TIM_OC_InitStruct.OCIdleState = TIM_OCIDLESTATE_RESET;
	HAL_TIM_PWM_ConfigChannel(&TIM5_HandleStruct, &TIM_OC_InitStruct, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&TIM5_HandleStruct, TIM_CHANNEL_2);
}

/*定时器6初始化 prescaler:预分频值 period:重装载值*/
void TIM6_Configuration(unsigned short int prescaler, unsigned short int period)
{
	__HAL_RCC_TIM6_CLK_ENABLE();
	TIM6_HandleStruct.Instance = TIM6;
	TIM6_HandleStruct.Init.Prescaler = prescaler;
	TIM6_HandleStruct.Init.Period = period;
	TIM6_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM6_HandleStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TIM6_HandleStruct);
	HAL_NVIC_SetPriority(TIM6_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
	HAL_TIM_Base_Start_IT(&TIM6_HandleStruct);
}

/*定时器1捕获中断服务函数*/
void TIM1_CC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TIM1_HandleStruct);
}

/*定时器3中断服务函数*/
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM3_HandleStruct);
}

/*定时器6中断服务函数*/
void TIM6_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM6_HandleStruct);
}

/*定时器中断回调函数*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3) Stepper_Interrupt();
	else if(htim->Instance == TIM6) Temperature_Process();
}

#if REMOTE_SUPPORT
/*定时器捕获中断回调函数*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1) Remote_Capture();
}
#endif
