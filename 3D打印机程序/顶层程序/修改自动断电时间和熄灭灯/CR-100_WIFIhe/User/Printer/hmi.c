/**
  ******************************************************************************
  * @file     hmi.c
  * @author   LEO
  * @date     2018/11/16
  * @version  0.1.1
  * @brief    人机交互界面底层
  ******************************************************************************
**/
#include <string.h>
#include "malloc.h"
#include "hmi.h"
#include "oled.h"
#include "oled_bmp.h"
#include "keypad.h"
#include "remote.h"
#include "temperature.h"
#include "rtc.h"
#include "stepper_motor.h"
#include "printer.h"
#include "storage_device.h"
#include "language.h"

HMI_MemberTypeDef HMI_MemberStruct = {DISABLE, 0, HMI_IDLE, true, 0, HEAD_IDLE};
RTC_DurationTypeDef RTC_DurationStruct;

uint8_t Print_Finished_flag=0;//打印完成标志位王
uint8_t Key_Down_flag=0;			//按键按下标志位王
uint8_t Printing_flag=0;      //正在打印标志位王

/*开机画面*/
void HMI_StartFrame(void)
{
#if HMI_OLED
//  OLED_ShowSpecialPicture(0, 0, 128, 16, gImage_logo, 0);
//  Delay_Ms(300);
//  OLED_ShowText(24, 3, "创想三", 5, 16);
	OLED_ShowText(0, 0, "I", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(8, 0, "C", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(16, 0, "r", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(24, 0, "e", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(32, 0, "a", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(40, 0, "t", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(48, 0, "e", 0, 16);
  Delay_Ms(50);
		OLED_ShowText(56, 0, " ", 0, 16);
  Delay_Ms(50);
			OLED_ShowText(64, 0, "R", 0, 16);
  Delay_Ms(50);
			OLED_ShowText(72, 0, "o", 0, 16);
  Delay_Ms(50);
			OLED_ShowText(80, 0, "b", 0, 16);
  Delay_Ms(50);
				OLED_ShowText(88, 0, "o", 0, 16);
  Delay_Ms(50);
				OLED_ShowText(96, 0, "t", 0, 16);
  Delay_Ms(50);
  OLED_ShowText(0, 6, "ICreate 3D", 0, 16);
	OLED_ShowChinese(20, 3, 0);
	OLED_ShowChinese(45, 3, 1);
	OLED_ShowChinese(70, 3, 2);
	OLED_ShowChinese(95, 3, 3);
  Delay_Ms(300);
  OLED_ShowText(0, 6, "ICreate 3D", 0, 16);
  OLED_ShowText(98, 7, PROTOCOL_VERSION, 0, 8);
#endif
}

/*主进程画面*/
void HMI_ProcessFrame(void)
{
  static unsigned char HMI_LastStatus = HMI_PRINT_FINISH;
  unsigned char percent;
  char *pName;
  if(HMI_MemberStruct.Need_ReDraw == true)
  {
    HMI_MemberStruct.Need_ReDraw = false;
  #if HMI_OLED
    OLED_Clear();
    OLED_ShowText(5, 0, "ICreate 3D", 0, 16);
    OLED_ShowPicture(100, 1, 8, 16, gImage_head);
    OLED_ShowNum(94, 0, 0, 8, 3, false);
    OLED_ShowNum(94, 3, 0, 8, 3, false);
    OLED_ShowPicture(112, 0, 8, 8, gImage_celsius);
    OLED_ShowPicture(112, 3, 8, 8, gImage_celsius);
    OLED_ShowText(12, 3, "--:--:--", 0, 12);
    OLED_ShowProgressBar(0, 5, 82, 8, 0);
  #endif
  }
  if(HMI_MemberStruct.NextUpdateTime < HAL_GetTick())
  {
    HMI_MemberStruct.NextUpdateTime = HAL_GetTick() + 1000;
  #if HMI_OLED
    OLED_ShowNum(94, 0, Heater_GetTargetCelsius(Heater_Head0), 8, 3, false);
    OLED_ShowNum(94, 3, Heater_GetCurrentCelsius(Heater_Head0), 8, 3, false);
  #endif
    if(HMI_MemberStruct.RunTime_Flag == ENABLE)
    {
      RTC_DurationStruct = RTC_Get_DurationTime();
    #if HMI_OLED
      OLED_ShowNum(5, 3, RTC_DurationStruct.Hours, 12, 3, false);
      OLED_ShowNum(33, 3, RTC_DurationStruct.Minutes, 12, 2, true);
      OLED_ShowNum(54, 3, RTC_DurationStruct.Seconds, 12, 2, true);
    #endif
    }
    if(HMI_MemberStruct.Status == HMI_PRINTING)
    {
      percent = f_tell(&SD_Reader.G_File) * 100 / f_size(&SD_Reader.G_File);
    #if HMI_OLED
      OLED_ShowProgressBar(0, 5, 82, 8, percent);
      OLED_ShowNum(84, 5, percent, 8, 3, false);
      OLED_ShowText(102, 5, "%", 0, 8);
    #endif
    }
    if(HMI_MemberStruct.Status == HMI_PRINT_FINISH)
    {
    #if HMI_OLED
      OLED_ShowProgressBar(0, 5, 82, 8, 100);
      OLED_ShowNum(84, 5, 100, 8, 3, false);
      OLED_ShowText(102, 5, "%", 0, 8);
    #endif
    }
  }
  if(HMI_MemberStruct.Status != HMI_LastStatus)
  {
    HMI_LastStatus = HMI_MemberStruct.Status;
  #if HMI_OLED
    OLED_ShowText(0, 6, "                 ", 0, 12);
    switch(HMI_LastStatus)
    {
      case HMI_NO_FILE: OLED_ShowText(0, 6, "No G-File.", 0, 12); break;
      case HMI_NO_SD: OLED_ShowText(0, 6, "No TF-Card.", 0, 12); break;
      case HMI_PRINTING: OLED_ShowText(0, 6, "Printing.", 0, 12);
			
					 break;
      case HMI_PRINT_FINISH: OLED_ShowText(0, 6, "Print Finished.", 0, 12); 
//					 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);  //自动关机引脚输出高电平王
		     	 Printing_flag=0;          //正在打印标志位清零
			     Print_Finished_flag=1;    //打印完成标志位王
					 break;
      case HMI_AXIS_HOME: OLED_ShowText(0, 6, "Axis Homing.", 0, 12); break;
      case HMI_PAUSE: OLED_ShowText(0, 6, "Paused.", 0, 12); break;
      case HMI_HEATING: OLED_ShowText(0, 6, "Heating Head.", 0, 12); break;
      case HMI_COOLING: OLED_ShowText(0, 6, "Cooling Head.", 0, 12); break;
      case HMI_READY: OLED_ShowText(0, 6, "Printer Ready.", 0, 12); break;
      case HMI_SD_ERROR: OLED_ShowText(0, 6, "Please Restart.", 0, 12); break;
    }
  #endif
  }
#if KEYPAD_SUPPORT
  if(KeyPad_ScanPush() != 0xFF)  //获取按键状态
  {
    switch(KeyPad_ScanPush())
    {
      case START_PAUSE:      //开始停止
      {
				if(Print_Finished_flag==1)
			{
			     Printing_flag=1;          //正在打印标志位王
			}
				Key_Down_flag=1;  //按键按下标志位王
        if(SD_Reader.Printing == false)
        {
          if(HMI_MemberStruct.Status==HMI_READY || HMI_MemberStruct.Status==HMI_PRINT_FINISH || HMI_MemberStruct.Status==HMI_HEATING || HMI_MemberStruct.Status==HMI_COOLING)
          {
            if(StorageDevice_GetFirstFile((char *)SD_Reader.FileName) != FR_OK) HMI_MemberStruct.Status = HMI_NO_FILE;
            else
            {
              pName = Memory_Malloc(MEMORY_IN_BANK, strlen((const char*)SD_Reader.FileName)+strlen((const char*)"0:")+2);
              pName = StorageDevice_PathCatName(pName, (char *)SD_Reader.FileName);
              if(f_open(&SD_Reader.G_File, (const char*)pName, FA_READ) == FR_OK)
              {
                StorageDevice_StartPrint();
                HMI_MemberStruct.Status = HMI_HEATING;
              }
              Memory_Free(MEMORY_IN_BANK, pName);
            }
          }
          else if(HMI_MemberStruct.Status == HMI_PAUSE)
          {
            HMI_MemberStruct.Status = HMI_PRINTING;
            SD_Reader.Printing = true;
            ENABLE_STEPPER_DRIVER_INTERRUPT();
          }
        }
        else
        {
          HMI_MemberStruct.Status = HMI_PAUSE;
          enquecommand("G1 F1500 X1.0 Y1.0");
          SD_Reader.Printing = false;
        }
      }break;
      case HOME:    //原点
      {
				Key_Down_flag=1;  //按键按下标志位王
        if((SD_Reader.Printing == false) && (HMI_MemberStruct.Status!=HMI_PAUSE) && (HMI_MemberStruct.Status!=HMI_SD_ERROR))
        {
          enquecommand("G28");
          HMI_MemberStruct.Status = HMI_AXIS_HOME;
        }
      }break;
      case HEAT_COOL:   //加热降温
      {
				Key_Down_flag=1;  //按键按下标志位王
        if((SD_Reader.Printing == false) && (HMI_MemberStruct.Status!=HMI_PAUSE) && (HMI_MemberStruct.Status!=HMI_SD_ERROR))
        {
          if((HMI_MemberStruct.Status!=HMI_HEATING) && (HMI_MemberStruct.Head_Status!=HEAD_HEATING))
          {
            enquecommand("M104 S180");
            HMI_MemberStruct.Status = HMI_HEATING;
            HMI_MemberStruct.Head_Status = HEAD_HEATING;
          }
          else
          {
            enquecommand("M104 S0");
            HMI_MemberStruct.Status = HMI_COOLING;
            HMI_MemberStruct.Head_Status = HEAD_COOLING;
          }
        }
      }break;
    }
    KeyPad_ClearFlag();
    return;
  }
#endif
#if REMOTE_SUPPORT
  if(Remote_StatusStruct.ReceiveFinish_Flag == SET)
  {
    Remote_Enable(DISABLE);
    switch(Remote_ScanPush())
    {
      case START_PAUSE:
      {
        if(SD_Reader.Printing == false)
        {
          if(HMI_MemberStruct.Status==HMI_READY || HMI_MemberStruct.Status==HMI_PRINT_FINISH || HMI_MemberStruct.Status==HMI_HEATING || HMI_MemberStruct.Status==HMI_COOLING)
          {
            if(StorageDevice_GetFirstFile((char *)SD_Reader.FileName) != FR_OK) HMI_MemberStruct.Status = HMI_NO_FILE;
            else
            {
              pName = Memory_Malloc(MEMORY_IN_BANK, strlen((const char*)SD_Reader.FileName)+strlen((const char*)"0:")+2);
              pName = StorageDevice_PathCatName(pName, (char *)SD_Reader.FileName);
              if(f_open(&SD_Reader.G_File, (const char*)pName, FA_READ) == FR_OK)
              {
                StorageDevice_StartPrint();
                HMI_MemberStruct.Status = HMI_HEATING;
              }
              Memory_Free(MEMORY_IN_BANK, pName);
            }
          }
          else if(HMI_MemberStruct.Status == HMI_PAUSE)
          {
            HMI_MemberStruct.Status = HMI_PRINTING;
            SD_Reader.Printing = true;
            ENABLE_STEPPER_DRIVER_INTERRUPT();
          }
        }
        else
        {
          HMI_MemberStruct.Status = HMI_PAUSE;
          enquecommand("G1 F1500 X1.0 Y1.0");
          SD_Reader.Printing = false;
        }
      }break;
      case HOME:
      {
        if((SD_Reader.Printing == false) && (HMI_MemberStruct.Status!=HMI_PAUSE) && (HMI_MemberStruct.Status!=HMI_SD_ERROR))
        {
          enquecommand("G28");
          HMI_MemberStruct.Status = HMI_AXIS_HOME;
        }
      }break;
      case HEAT_COOL:
      {
        if((SD_Reader.Printing == false) && (HMI_MemberStruct.Status!=HMI_PAUSE) && (HMI_MemberStruct.Status!=HMI_SD_ERROR))
        {
          if((HMI_MemberStruct.Status!=HMI_HEATING) && (HMI_MemberStruct.Head_Status!=HEAD_HEATING))
          {
            enquecommand("M104 S180");
            HMI_MemberStruct.Status = HMI_HEATING;
            HMI_MemberStruct.Head_Status = HEAD_HEATING;
          }
          else
          {
            enquecommand("M104 S0");
            HMI_MemberStruct.Status = HMI_COOLING;
            HMI_MemberStruct.Head_Status = HEAD_COOLING;
          }
        }
      }break;
    }
    Remote_Clear();
    Remote_Enable(ENABLE);
    return;
  }
#endif
}
