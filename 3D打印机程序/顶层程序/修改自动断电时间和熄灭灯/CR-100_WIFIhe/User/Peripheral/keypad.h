#ifndef __KEYPAD_H
#define __KEYPAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "printer_conf.h"

#if KEYPAD_SUPPORT

#define KEY_NUM  3

typedef enum
{
    START_PAUSE = 0,
    HOME        = 1,
    HEAT_COOL   = 2
} KeyPad_StatusTypeDef;

/*按键初始化 PB12:KEY2 PB13:KEY1 PB14:KEY0*/
void KeyPad_Configuration(void);

/*清除按键标志*/
void KeyPad_ClearFlag(void);

/*按键使能/失能*/
void KeyPad_Enable(FunctionalState state);

/*扫描按键状态 返回值:0xFF,无按键按下;其他,被按键序号*/
unsigned char KeyPad_ScanPush(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
