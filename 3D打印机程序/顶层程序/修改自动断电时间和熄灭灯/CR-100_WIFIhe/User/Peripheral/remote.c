/**
  ******************************************************************************
  * @file     remote.c
  * @author   LEO
	* @date     2018/11/23
	* @version  0.1.1
  * @brief    Ò£¿ØÆ÷²Ù×÷Çý¶¯
	******************************************************************************
**/
#include "remote.h"
#include "timer.h"
#include "keypad.h"

#if REMOTE_SUPPORT

Remote_StatusTypeDef Remote_StatusStruct = {0, RESET, RESET, 0, 0};

/*Ò£¿ØÆ÷³õÊ¼»¯ TIM1_CH1(PA8):Remote*/
void Remote_Configuration(void)
{
    TIM1_Configuration(71, 0xFFFF);
}

/*Ò£¿ØÆ÷²¶»ñ*/
void Remote_Capture(void)
{
    if(Remote_PinRead())
    {
        TIM_RESET_CAPTUREPOLARITY(&TIM1_HandleStruct, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&TIM1_HandleStruct, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
        __HAL_TIM_SET_COUNTER(&TIM1_HandleStruct, 0);
        Remote_StatusStruct.CaptureRise_Flag = SET;
    }
    else
    {
        Remote_StatusStruct.Capture_Time = HAL_TIM_ReadCapturedValue(&TIM1_HandleStruct, TIM_CHANNEL_1);
        TIM_RESET_CAPTUREPOLARITY(&TIM1_HandleStruct, TIM_CHANNEL_1);
        TIM_SET_CAPTUREPOLARITY(&TIM1_HandleStruct, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);
        if(Remote_StatusStruct.CaptureRise_Flag && !Remote_StatusStruct.ReceiveFinish_Flag)
        {
            if(Remote_StatusStruct.Capture_Time>360 && Remote_StatusStruct.Capture_Time<760)
            {
                Remote_StatusStruct.Receive_Data <<= 1;
                Remote_StatusStruct.Bit_Counter++;
            }
            else if(Remote_StatusStruct.Capture_Time>1480 && Remote_StatusStruct.Capture_Time<1880)
            {
                Remote_StatusStruct.Receive_Data <<= 1;
                Remote_StatusStruct.Receive_Data |= 1;
                Remote_StatusStruct.Bit_Counter++;
            }
            else if(Remote_StatusStruct.Capture_Time>4300 && Remote_StatusStruct.Capture_Time<4700)
            {
                Remote_StatusStruct.Receive_Data = 0;
                Remote_StatusStruct.Bit_Counter = 0;
            }
            if(Remote_StatusStruct.Bit_Counter == 32)
            {
                Remote_StatusStruct.ReceiveFinish_Flag = SET;
            }
        }
        Remote_StatusStruct.CaptureRise_Flag = RESET;
    }
}

/*Çå³ýÒ£¿ØÆ÷°´¼ü*/
void Remote_Clear(void)
{
    Remote_StatusStruct.ReceiveFinish_Flag = RESET;
    Remote_StatusStruct.CaptureRise_Flag = RESET;
    Remote_StatusStruct.Receive_Data = 0;
}

/*Ò£¿ØÆ÷°´¼üÊ¹ÄÜ/Ê§ÄÜ*/
void Remote_Enable(FunctionalState state)
{
    if(state) HAL_TIM_IC_Start_IT(&TIM1_HandleStruct, TIM_CHANNEL_1);
    else HAL_TIM_IC_Stop_IT(&TIM1_HandleStruct, TIM_CHANNEL_1);
}

/*Ò£¿ØÆ÷É¨Ãè°´¼ü×´Ì¬ ·µ»ØÖµ:0xFF,ÎÞ°´¼ü°´ÏÂ;ÆäËû,±»°´¼üÐòºÅ*/
unsigned char Remote_ScanPush(void)
{
    unsigned char add, add_B, cmd, cmd_B;
    if(Remote_StatusStruct.ReceiveFinish_Flag == SET)
    {
        Remote_StatusStruct.ReceiveFinish_Flag = RESET;
        add = (Remote_StatusStruct.Receive_Data&0xFF000000) >> 24;
        add_B = (Remote_StatusStruct.Receive_Data&0x00FF0000) >> 16;
        cmd = (Remote_StatusStruct.Receive_Data&0x0000FF00) >> 8;
        cmd_B = Remote_StatusStruct.Receive_Data;
        if((add==(unsigned char)~add_B) && (add==0x00) && (cmd==(unsigned char)~cmd_B))
        {
            switch(cmd)
            {
            case 0x62:
                return 0;
            case 0xA8:
                return 1;
            case 0x18:
                return 2;
            default:
                return 0xFF;
            }
        }
    }
    return 0xFF;
}

#endif
