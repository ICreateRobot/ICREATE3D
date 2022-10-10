/**
  ******************************************************************************
  * @project  BiSun
  * @author   LEO/T_B/Talon
  * @date     2019/01/07
  * @version  0.1.5
  * @brief    STM32F103RE-CR-100(正式版v0.1.5)
  * @update   -优化中断优先级
              -增加了多个模块控制宏
              -偏移了原点坐标
              -整理温度相关函数
              -优化串口输出信息
              -屏蔽相关GCode功能
  ******************************************************************************
**/

#include "system.h"
#include "sd_card.h"
#include "fatfs.h"
#include "timer.h"
#include "usart.h"
#include "malloc.h"
#include "printer.h"
#include "oled.h"
#include "keypad.h"
#include "remote.h"
#include "hmi.h"
#include "planner.h"
#include "stepper_motor.h"
#include "temperature.h"
#include "endstop.h"
#include "storage_device.h"
#include "ws2812.h"
#ifdef HOST_KEEPALIVE_FEATURE
static uint8_t busy_state = 0;

void set_busy_state(uint8_t busy)
{
  busy_state = busy;
}

void host_keep_alive(void)
{
  static uint32_t millis_next_report;
  
  if(busy_state) {
    if(HAL_GetTick() > millis_next_report) {
      millis_next_report = HAL_GetTick() + 2*1000;
      printf("echo:busy: processing\n");
    }
  }
  else millis_next_report = 0;
}
#endif

#ifdef TEMPRATURE_AUTO_REPORT
static uint8_t temp_report_interval;

void set_temp_report_interval(uint8_t interval)
{
  temp_report_interval = interval;
}

void temprature_auto_report(void)
{
  static uint32_t millis_next_report;
  
  if(temp_report_interval) {
    if(HAL_GetTick() > millis_next_report) {
      millis_next_report = HAL_GetTick() + temp_report_interval*1000;
      printf("T:%.2f /%.2f B:0.00 /0.00 @:0 B@:0\n", Heater_GetCurrentCelsius((Heater_TypeDef)Heater_Head0), Heater_GetTargetCelsius((Heater_TypeDef)Heater_Head0));
    }
  }
  else millis_next_report = 0;
}
#endif

int main(void)
{
  SCB->VTOR = FLASH_BASE | 0x19000;
  HAL_Init();
	Automatic_Power_Off_Configuration();//自动断电初始化
	ws2812_GPIO_Init();
  SystemClock_Configuration();
  Delay_Configuration(72);
  RTC_Configuration();
  USART1_Configuration(115200);
#if KEYPAD_SUPPORT
  KeyPad_Configuration();
#endif
#if REMOTE_SUPPORT
  Remote_Configuration();
#endif
#if HMI_SUPPORT
#if HMI_OLED
  OLED_Configuration();
#endif
  HMI_StartFrame();
#endif
  Memory_Configuration(MEMORY_IN_BANK);
   Exf_Configuration();
//  Config_PrintSettings();
  reset_acceleration_rates();
  Stepper_Configuration();
  EndStops_Configuration();
  Temperature_Configuration();
  plan_init();
#if KEYPAD_SUPPORT
  KeyPad_Enable(ENABLE);
#endif
#if REMOTE_SUPPORT
  Remote_Enable(ENABLE);
#endif
  Delay_Ms(1500);
  while(1)
  {
		
    HMI_ProcessFrame();
    Printer_GetCommand();
    StorageDevice_Process();
    Printer_Process();
    Temperature_Manage();
    manage_inactivity();
    EndStops_ReportState();
		Automatic_Power_Off();//自动断电
//  USART1_CheckDTR();
  }
}
