#ifndef __REMOTE_H
#define __REMOTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "printer_conf.h"

#if REMOTE_SUPPORT

#define Remote_PinRead()  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)

typedef struct
{
    unsigned int Receive_Data;
    FlagStatus ReceiveFinish_Flag;
    FlagStatus CaptureRise_Flag;
    unsigned short int Capture_Time;
    unsigned char Bit_Counter;
} Remote_StatusTypeDef;

extern Remote_StatusTypeDef Remote_StatusStruct;

/*遥控器初始化 TIM1_CH1(PA8):Remote*/
void Remote_Configuration(void);

/*遥控器捕获*/
void Remote_Capture(void);

/*清除遥控器按键*/
void Remote_Clear(void);

/*遥控器按键使能/失能*/
void Remote_Enable(FunctionalState state);

/*遥控器扫描按键状态 返回值:0xFF,无按键按下;其他,被按键序号*/
unsigned char Remote_ScanPush(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
