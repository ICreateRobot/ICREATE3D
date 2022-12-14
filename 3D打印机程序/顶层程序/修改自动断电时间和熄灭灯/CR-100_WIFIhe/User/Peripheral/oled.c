/**
  ******************************************************************************
  * @file     oled.c
  * @author   LEO
	* @date     2018/11/14
	* @version  0.1.1
  * @brief    OLED驱动底层
	******************************************************************************
**/
#include "oled.h"
#include "iic.h"
#include "oled_font.h"

#if HMI_OLED

/*OLED初始化*/
void OLED_Configuration(void)
{
    IIC2_Configuration(I2C_FAST_MODE_MAX_CLK);
    OLED_WriteByte(SET_DISPLAY_OFF, OLED_COMMAND);
    OLED_WriteByte(SET_DISPLAY_CLOCK_DIVIDE, OLED_COMMAND);
    OLED_WriteByte(0x80, OLED_COMMAND);
    OLED_WriteByte(SET_MULTIPLEX_RATIO, OLED_COMMAND);
    OLED_WriteByte(0x3F, OLED_COMMAND);
    OLED_WriteByte(SET_DISPLAY_OFFSET, OLED_COMMAND);
    OLED_WriteByte(0x00, OLED_COMMAND);
    OLED_WriteByte(SET_DISPLAY_START_LINE, OLED_COMMAND);
    OLED_WriteByte(SET_CHARGE_PUMP, OLED_COMMAND);
    OLED_WriteByte(0x14, OLED_COMMAND);
    OLED_WriteByte(SET_MEMORY_ADDRESSING_MODE, OLED_COMMAND);
    OLED_WriteByte(0x02, OLED_COMMAND);
    OLED_WriteByte(SET_COLUMN_ADDRESS127_TO_SEG0, OLED_COMMAND);
    OLED_WriteByte(SET_COM_SCAN_DIRECTION_N_TO_0, OLED_COMMAND);
    OLED_WriteByte(SET_COM_PIN_CONFIGURATION, OLED_COMMAND);
    OLED_WriteByte(0x12, OLED_COMMAND);
    OLED_WriteByte(SET_CONTRAST_CONTROL, OLED_COMMAND);
    OLED_WriteByte(0x7F, OLED_COMMAND);
    OLED_WriteByte(SET_PRECHARGE_PERIOD, OLED_COMMAND);
    OLED_WriteByte(0xF1, OLED_COMMAND);
    OLED_WriteByte(SET_VCOMH_DESELECT_LEVEL, OLED_COMMAND);
    OLED_WriteByte(0x30, OLED_COMMAND);
    OLED_WriteByte(ENTIRE_DISPLAY_ON, OLED_COMMAND);
    OLED_WriteByte(SET_NORMAL_DISPLAY, OLED_COMMAND);
    OLED_WriteByte(SET_DISPLAY_ON, OLED_COMMAND);
    OLED_ScrollControl(DISABLE);
    OLED_Clear();
}

/*OLED显示开关控制*/
void OLED_DisplaySwitch(FunctionalState state)
{
    OLED_WriteByte(SET_CHARGE_PUMP, OLED_COMMAND);
    if(state)
    {
        OLED_WriteByte(0x14, OLED_COMMAND);
        OLED_WriteByte(SET_DISPLAY_ON, OLED_COMMAND);
    }
    else
    {
        OLED_WriteByte(0x10, OLED_COMMAND);
        OLED_WriteByte(SET_DISPLAY_OFF, OLED_COMMAND);
    }
}

/*OLED写命令 data:数据 type:OLED_COMMAND,写命令;OLED_DATA,写数据*/
void OLED_WriteByte(unsigned char data, unsigned char type)
{
    unsigned char tempData[2] = {0x00, 0x00};
    tempData[1] = data;
    if(type == OLED_DATA) tempData[0] = 0x40;
#if (OLED_IIC_MODE == 0)
    HAL_I2C_Master_Transmit(&IIC2_HandleStruct, 0x78, tempData, 2, 10);
#endif
#if (OLED_IIC_MODE == 1)
    HAL_I2C_Master_Transmit_DMA(&IIC2_HandleStruct, 0x78, tempData, 2);
#endif
#if (OLED_IIC_MODE == 2)
    Simulate_I2C_Master_Transmit(0x78, tempData, 2);
#endif
}

/*OLED坐标设置*/
void OLED_SetPosition(unsigned char x, unsigned char y)
{
    OLED_WriteByte(SET_PAGE_START_ADDRESS+y, OLED_COMMAND);
    OLED_WriteByte(((x&0xF0)>>4)|SET_HIGHER_COLUMN_START_ADDRESS, OLED_COMMAND);
    OLED_WriteByte((x&0x0F), OLED_COMMAND);
}

/*OLED清屏*/
void OLED_Clear(void)
{
    unsigned char page, column;
    for(page=0; page<8; page++)
    {
        OLED_SetPosition(0, page);
        for(column=0; column<128; column++) OLED_WriteByte(0x00, OLED_DATA);
    }
}

/*OLED显示字符 x:行坐标 y:列坐标 ch:字符 size:字符大小*/
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char size)
{
    unsigned char tempChar=0, counter=0;
    tempChar = ch - ' ';
    if(size == 16)
    {
        OLED_SetPosition(x, y);
        for(counter=0; counter<8; counter++) OLED_WriteByte(Ascii_8X16[tempChar][counter], OLED_DATA);
        OLED_SetPosition(x, y+1);
        for(counter=8; counter<16; counter++) OLED_WriteByte(Ascii_8X16[tempChar][counter], OLED_DATA);
    }
    else if(size == 12)
    {
        OLED_SetPosition(x, y);
        for(counter=0; counter<7; counter++) OLED_WriteByte(Ascii_7X12[tempChar][counter], OLED_DATA);
        OLED_SetPosition(x, y+1);
        for(counter=7; counter<14; counter++) OLED_WriteByte(Ascii_7X12[tempChar][counter], OLED_DATA);
    }
    else
    {
        OLED_SetPosition(x, y);
        for(counter=0; counter<6; counter++) OLED_WriteByte(Ascii_6X8[tempChar][counter], OLED_DATA);
    }
}

/*OLED显示数字 x:行坐标 y:列坐标 num:数字变量 size:字符大小 digits:位(1~5) zeroize:补零*/
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char size, unsigned char digits, bool zeroize)
{
    char data[20];
    unsigned char counter = 0;
    if(zeroize)
    {
        switch(digits)
        {
        case 5:
            sprintf(data, "%05d", num);
            break;
        case 4:
            sprintf(data, "%04d", num);
            break;
        case 3:
            sprintf(data, "%03d", num);
            break;
        case 2:
            sprintf(data, "%02d", num);
            break;
        case 1:
        default:
            sprintf(data, "%01d", num);
            break;
        }
    }
    else
    {
        switch(digits)
        {
        case 5:
            sprintf(data, "%5d", num);
            break;
        case 4:
            sprintf(data, "%4d", num);
            break;
        case 3:
            sprintf(data, "%3d", num);
            break;
        case 2:
            sprintf(data, "%2d", num);
            break;
        case 1:
        default:
            sprintf(data, "%1d", num);
            break;
        }
    }
    while(data[counter] != '\0')
    {
        OLED_ShowChar(x, y, data[counter], size);
        (size == 16) ? (x += 8) : (x += 6);
        counter++;
    }
}

/*OLED显示汉字 x:行坐标 y:列坐标 num:汉字对应字库中的位置*/
void OLED_ShowChinese(unsigned char x, unsigned char y, unsigned char num)
{
    unsigned char counter;
    OLED_SetPosition(x, y);
    for(counter=0; counter<16; counter++) OLED_WriteByte(GB_16[num].Msk[counter], OLED_DATA);
    OLED_SetPosition(x, y+1);
    for(counter=16; counter<32; counter++) OLED_WriteByte(GB_16[num].Msk[counter], OLED_DATA);
}

/*OLED显示文本 x:行坐标 y:列坐标 *text:文本 interval:文本间隔(仅中文) size:文本大小(仅字符)*/
void OLED_ShowText(unsigned char x, unsigned char y, char *text, unsigned char interval, unsigned char size)
{
    unsigned char tempNum;
    unsigned char *tempText = (unsigned char *)text;
    while(*tempText)
    {
        if(*tempText < 0x80)
        {
            OLED_ShowChar(x, y, *tempText, size);
            if(size == 16) x += 8;
            else if(size == 12) x += 7;
            else x += 6;
            tempText++;
        }
        else
        {
            for(tempNum=0; tempNum<0xFF; tempNum++)
            {
                if((GB_16[tempNum].Index[0]==*(tempText)) && (GB_16[tempNum].Index[1]==*(tempText+1)))
                {
                    OLED_ShowChinese(x, y, tempNum);
                    break;
                }
            }
            tempText += 2;
            x = x + 16 + interval;
        }
    }
}

/*OLED显示进度条 x:行坐标 y:列坐标 length:矩形长度 wide:矩形宽度 percent:百分比*100*/
void OLED_ShowProgressBar(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, unsigned char percent)
{
    unsigned char counter;
    unsigned char finishNum = (length-2) * percent / 100;
    OLED_SetPosition(x, y);
    OLED_WriteByte(0x3F, OLED_DATA);
    for(counter=0; counter<length-2; counter++)
    {
        if(counter < finishNum) OLED_WriteByte(0x2D, OLED_DATA);
        else OLED_WriteByte(0x21, OLED_DATA);
    }
    OLED_WriteByte(0x3F, OLED_DATA);
}

/*OLED显示图片 x:行坐标 y:列坐标 length:图片长度 wide:图片宽度 pic:图片地址*/
void OLED_ShowPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic)
{
    unsigned char counter;
    wide /= 8;
    while(wide)
    {
        wide--;
        OLED_SetPosition(x, y+wide);
        for(counter=0; counter<length; counter++) OLED_WriteByte(pic[counter + wide*length], OLED_DATA);
    }
}

/*OLED显示特效图片 x:行坐标 y:列坐标 length:图片长度 wide:图片宽度 pic:图片地址 specialMode:特效类型*/
void OLED_ShowSpecialPicture(unsigned char x, unsigned char y, unsigned char length, unsigned char wide, const unsigned char *pic, unsigned char specialMode)
{
    unsigned char tempLength=0, tempWide=0;
    switch(specialMode)
    {
    case 0:
    {
        while(tempLength != length-1)
        {
            for(tempWide=0; tempWide<(wide/8); tempWide++)
            {
                OLED_SetPosition(x+tempLength, y+tempWide);
                OLED_WriteByte(pic[tempLength + tempWide*length], OLED_DATA);
            }
            tempLength++;
            Delay_Ms(5);
        }
    }
    break;
    }
}

/*OLED屏幕横向滚动 startX:起始行 startY:起始列 endX:结束行 endY:结束列 dir:方向(0,左;1,右) speed:速度(1~8)*/
void OLED_HorizontalScroll(unsigned char startX, unsigned char startY, unsigned char endX, unsigned char endY, unsigned char dir, unsigned char speed)
{
    OLED_ScrollControl(DISABLE);
    OLED_WriteByte(dir ? SET_RIGHT_HORIZONTAL_SCROLL : SET_RIGHT_HORIZONTAL_SCROLL, OLED_COMMAND);
    OLED_WriteByte(0x00, OLED_COMMAND);
    OLED_WriteByte(startY, OLED_COMMAND);
    switch(speed)
    {
    case 1:
        OLED_WriteByte(0x03, OLED_COMMAND);
        break;
    case 2:
        OLED_WriteByte(0x02, OLED_COMMAND);
        break;
    case 3:
        OLED_WriteByte(0x01, OLED_COMMAND);
        break;
    case 4:
        OLED_WriteByte(0x00, OLED_COMMAND);
        break;
    case 5:
        OLED_WriteByte(0x06, OLED_COMMAND);
        break;
    case 6:
        OLED_WriteByte(0x05, OLED_COMMAND);
        break;
    case 7:
        OLED_WriteByte(0x04, OLED_COMMAND);
        break;
    case 8:
        OLED_WriteByte(0x07, OLED_COMMAND);
        break;
    default:
        break;
    }
    OLED_WriteByte(endY, OLED_COMMAND);
    OLED_WriteByte(startX, OLED_COMMAND);
    OLED_WriteByte(endX, OLED_COMMAND);
}

/*OLED屏幕滚动控制 state:状态*/
void OLED_ScrollControl(FunctionalState state)
{
    state ? OLED_WriteByte(ACTIVATE_SCROLL, OLED_COMMAND) : OLED_WriteByte(DEACTIVATE_SCROLL, OLED_COMMAND);
}

#endif
