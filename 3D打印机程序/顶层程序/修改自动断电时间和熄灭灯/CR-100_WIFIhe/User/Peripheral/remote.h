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

/*ң������ʼ�� TIM1_CH1(PA8):Remote*/
void Remote_Configuration(void);

/*ң��������*/
void Remote_Capture(void);

/*���ң��������*/
void Remote_Clear(void);

/*ң��������ʹ��/ʧ��*/
void Remote_Enable(FunctionalState state);

/*ң����ɨ�谴��״̬ ����ֵ:0xFF,�ް�������;����,���������*/
unsigned char Remote_ScanPush(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
