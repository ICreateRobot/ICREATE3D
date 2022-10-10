#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "printer_conf.h"

#if HMI_OLED

#define OLED_COMMAND  0x00
#define OLED_DATA  0x01

#define SET_CONTRAST_CONTROL                  0x81
#define ENTIRE_DISPLAY_ON                     0xA4
#define ENTIRE_DISPLAY_OFF                    0xA5
#define SET_NORMAL_DISPLAY                    0xA6
#define SET_INVERSE_DISPLAY                   0xA7
#define SET_DISPLAY_OFF                       0xAE
#define SET_DISPLAY_ON                        0xAF

#define SET_RIGHT_HORIZONTAL_SCROLL           0x26
#define SET_LEFT_HORIZONTAL_SCROLL            0x27
#define SET_VERTICAL_RIGHT_HORIZONTAL_SCROLL  0x29
#define SET_VERTICAL_LEFT_HORIZONTAL_SCROLL   0x2A
#define DEACTIVATE_SCROLL                     0x2E
#define ACTIVATE_SCROLL                       0x2F
#define SET_VERTICAL_SCROLL_AREA              0xA3

#define SET_LOWER_COLUMN_START_ADDRESS        0x00
#define SET_HIGHER_COLUMN_START_ADDRESS       0x10
#define SET_MEMORY_ADDRESSING_MODE            0x20
#define SET_COLUMN_ADDRESS                    0x21
#define SET_PAGE_ADDRESS                      0x22
#define SET_PAGE_START_ADDRESS                0xB0

#define SET_DISPLAY_START_LINE                0x40
#define SET_COLUMN_ADDRESS0_TO_SEG0           0xA0
#define SET_COLUMN_ADDRESS127_TO_SEG0         0xA1
#define SET_MULTIPLEX_RATIO                   0xA8
#define SET_COM_SCAN_DIRECTION_0_TO_N         0xC0
#define SET_COM_SCAN_DIRECTION_N_TO_0         0xC8
#define SET_DISPLAY_OFFSET                    0xD3
#define SET_COM_PIN_CONFIGURATION             0xDA

#define SET_DISPLAY_CLOCK_DIVIDE              0xD5
#define SET_PRECHARGE_PERIOD                  0xD9
#define SET_VCOMH_DESELECT_LEVEL              0xDB
#define SET_CHARGE_PUMP                       0x8D

/*OLED初始化*/
void OLED_Configuration(void);

/*OLED显示开关控制*/
void OLED_DisplaySwitch(FunctionalState state);

/*OLED写命令 data:数据 type:OLED_COMMAND,写命令;OLED_DATA,写数据*/
void OLED_WriteByte(unsigned char data, unsigned char type);

/*OLED坐标设置*/
void OLED_SetPosition(unsigned char x, unsigned char y);

/*OLED清屏*/
void OLED_Clear(void);

/*OLED显示字符 x:行坐标 y:列坐标 ch:字符 size:字符大小*/
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char size);

/*OLED显示数字 x:行坐标 y:列坐标 num:数字变量 size:字符大小 digits:位(1~5) zeroize:补零*/
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char size, unsigned char digits, bool zeroize);

/*OLED显示汉字 x:行坐标 y:列坐标 num:汉字对应字库中的位置*/
void OLED_ShowChinese(unsigned char x, unsigned char y, unsigned char num);

/*OLED显示文本 x:行坐标 y:列坐标 *text:文本 interval:文本间隔(仅中文) size:文本大小(仅字符)*/
void OLED_ShowText(unsigned char x, unsigned char y, char *text, unsigned char interval, unsigned char size);

/*OLED显示进度条 x:行坐标 y:列坐标 length:矩形长度 wide:矩形宽度 percent:百分比*100*/
void OLED_ShowProgressBar(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, unsigned char percent);

/*OLED显示图片 x:行坐标 y:列坐标 length:图片长度 wide:图片宽度 pic:图片地址*/
void OLED_ShowPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic);

/*OLED显示特效图片 x:行坐标 y:列坐标 length:图片长度 wide:图片宽度 pic:图片地址 specialMode:特效类型*/
void OLED_ShowSpecialPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic, unsigned char specialMode);

/*OLED屏幕横向滚动 startX:起始行 startY:起始列 endX:结束行 endY:结束列 dir:方向(0,左;1,右) speed:速度(1~8)*/
void OLED_HorizontalScroll(unsigned char startX, unsigned char startY, unsigned char endX, unsigned char endY, unsigned char dir, unsigned char speed);

/*OLED屏幕滚动控制 state:状态*/
void OLED_ScrollControl(FunctionalState state);

#endif

#ifdef __cplusplus
}
#endif

#endif
