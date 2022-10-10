#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "printer_conf.h"

extern int target_temperature[EXTRUDERS];
extern float current_temperature[EXTRUDERS];
extern int target_temperature_bed;
extern float current_temperature_bed;
#ifdef PIDTEMP
extern float Kp, Ki, Kd, Kc;
#endif
#ifdef PIDTEMPBED
extern float bedKp,bedKi,bedKd;
#endif

typedef enum
{
    Heater_Head0 = 0,
    Heater_Head1 = 1,
    Heater_Head2 = 2,
    Heater_Bed   = -1
} Heater_TypeDef;

typedef enum
{
    Fan_Model0 = 0,
    Fan_Model1 = 1,
    Fan_Model2 = 2,
    Fan_Head0  = 3,
    Fan_Head1  = 4,
    Fan_Head2  = 5,
    Fan_Bed    = 6,
    Fan_Power  = 7
} Fan_TypeDef;

typedef enum
{
    Temp_OverMax = 0,
    Temp_OverMin = 1
} Temp_ErrorTypeDef;

/*风扇设置pwm比较值 fan:风扇类型 compare:比较值*/
void Fan_SetCompare(Fan_TypeDef fan, unsigned short int compare);

/*加热元件设置pwm比较值 heater:加热元件 compare:比较值*/
void Heater_SetCompare(Heater_TypeDef heater, unsigned short int compare);

/*加热元件获取pwm比较值 heater:加热元件*/
int Heater_GetCompare(Heater_TypeDef heater);

/*加热元件获取数值 heater:加热元件*/
unsigned short int Heater_GetValue(Heater_TypeDef heater);

/*加热元件设定温度值(℃) heater:加热元件*/
void Heater_SetTargetCelsius(const float celsius, Heater_TypeDef heater);

/*加热元件获取当前温度值(℃) heater:加热元件*/
float Heater_GetCurrentCelsius(Heater_TypeDef heater);

/*加热元件获取设定温度值(℃) heater:加热元件*/
float Heater_GetTargetCelsius(Heater_TypeDef heater);

/*判断加热元件是否正在加热 heater:加热元件*/
bool Heater_IsHeating(Heater_TypeDef heater);

/*判断加热元件是否正在冷却 heater:加热元件*/
bool Heater_IsCooling(Heater_TypeDef heater);

/*加热元件PID自动调谐 temp:设定温度 heater:加热元件 ncycles:循环次数*/
void Heater_PID_AutoTune(float temp, int heater, int ncycles);

/*加热元件自动停止*/
void Heater_AutoShutdown(void);

/*加热元件错误处理 tempError:温度错误类型 heater:加热元件*/
void Heater_TemperatureError(Temp_ErrorTypeDef tempError, Heater_TypeDef heater);

/*加热元件加热前检查*/
void Heater_SetWatch(void);

/*加热元件失能*/
void Heater_Disable(void);

/*加热元件设置PID*/
void Heater_SetPID(void);

/*温度控制初始化*/
void Temperature_Configuration(void);

/*温度控制管理*/
void Temperature_Manage(void);

/*温度控制执行进程*/
void Temperature_Process(void);

#ifdef PIDTEMP
/*温度PID比例调制 parameter:参数(0,I;1,D) value:值 scaleType:比例类型(0,放大;1,缩小)*/
float Temperature_ScalePID(char parameter, float value, unsigned char scaleType);
#endif

#ifdef __cplusplus
}
#endif

#endif
