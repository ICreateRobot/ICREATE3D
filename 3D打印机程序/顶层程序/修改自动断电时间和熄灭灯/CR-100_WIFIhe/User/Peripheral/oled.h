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

/*OLED��ʼ��*/
void OLED_Configuration(void);

/*OLED��ʾ���ؿ���*/
void OLED_DisplaySwitch(FunctionalState state);

/*OLEDд���� data:���� type:OLED_COMMAND,д����;OLED_DATA,д����*/
void OLED_WriteByte(unsigned char data, unsigned char type);

/*OLED��������*/
void OLED_SetPosition(unsigned char x, unsigned char y);

/*OLED����*/
void OLED_Clear(void);

/*OLED��ʾ�ַ� x:������ y:������ ch:�ַ� size:�ַ���С*/
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char size);

/*OLED��ʾ���� x:������ y:������ num:���ֱ��� size:�ַ���С digits:λ(1~5) zeroize:����*/
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char size, unsigned char digits, bool zeroize);

/*OLED��ʾ���� x:������ y:������ num:���ֶ�Ӧ�ֿ��е�λ��*/
void OLED_ShowChinese(unsigned char x, unsigned char y, unsigned char num);

/*OLED��ʾ�ı� x:������ y:������ *text:�ı� interval:�ı����(������) size:�ı���С(���ַ�)*/
void OLED_ShowText(unsigned char x, unsigned char y, char *text, unsigned char interval, unsigned char size);

/*OLED��ʾ������ x:������ y:������ length:���γ��� wide:���ο�� percent:�ٷֱ�*100*/
void OLED_ShowProgressBar(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, unsigned char percent);

/*OLED��ʾͼƬ x:������ y:������ length:ͼƬ���� wide:ͼƬ��� pic:ͼƬ��ַ*/
void OLED_ShowPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic);

/*OLED��ʾ��ЧͼƬ x:������ y:������ length:ͼƬ���� wide:ͼƬ��� pic:ͼƬ��ַ specialMode:��Ч����*/
void OLED_ShowSpecialPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic, unsigned char specialMode);

/*OLED��Ļ������� startX:��ʼ�� startY:��ʼ�� endX:������ endY:������ dir:����(0,��;1,��) speed:�ٶ�(1~8)*/
void OLED_HorizontalScroll(unsigned char startX, unsigned char startY, unsigned char endX, unsigned char endY, unsigned char dir, unsigned char speed);

/*OLED��Ļ�������� state:״̬*/
void OLED_ScrollControl(FunctionalState state);

#endif

#ifdef __cplusplus
}
#endif

#endif
