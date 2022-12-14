/**
  ******************************************************************************
  * @file     keypad.c
  * @author   LEO
	* @date     2018/11/19
	* @version  0.1.1
  * @brief    按键操作驱动
	******************************************************************************
**/
#include "keypad.h"

#if KEYPAD_SUPPORT

FunctionalState KeyPad_EnableState = DISABLE;
FlagStatus KeyPad_PushFlag[KEY_NUM];

/*按键初始化 PB12:KEY2 PB13:KEY1 PB14:KEY0*/
void KeyPad_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    KeyPad_Enable(DISABLE);
}

/*清除按键标志*/
void KeyPad_ClearFlag(void)
{
    unsigned char counter;
    for(counter=0; counter<KEY_NUM; counter++) KeyPad_PushFlag[counter] = RESET;
}

/*按键使能/失能*/
void KeyPad_Enable(FunctionalState state)
{
    if(state) KeyPad_EnableState = ENABLE;
    else KeyPad_EnableState = DISABLE;
    KeyPad_ClearFlag();
}

/*扫描按键状态 返回值:0xFF,无按键按下;其他,被按键序号*/
unsigned char KeyPad_ScanPush(void)
{
    unsigned char counter;
    for(counter=0; counter<KEY_NUM; counter++)
    {
        if(KeyPad_PushFlag[counter] == SET) return counter;
    }
    return 0xFF;
}

/*中断线10-15中断函数*/
void EXTI15_10_IRQHandler(void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET) HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET) HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
}

/*外部中断回调函数*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(KeyPad_EnableState)
    {
        EasyDelay_Ms(10);
        if(!HAL_GPIO_ReadPin(GPIOB, GPIO_Pin))
        {
            switch(GPIO_Pin)
            {
            case GPIO_PIN_12:
                KeyPad_PushFlag[2] = SET;
                break;
            case GPIO_PIN_13:
                KeyPad_PushFlag[1] = SET;
                break;
            case GPIO_PIN_14:
                KeyPad_PushFlag[0] = SET;
                break;
            }
        }
    }
}

#endif
