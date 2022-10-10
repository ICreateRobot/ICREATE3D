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

/*��������pwm�Ƚ�ֵ fan:�������� compare:�Ƚ�ֵ*/
void Fan_SetCompare(Fan_TypeDef fan, unsigned short int compare);

/*����Ԫ������pwm�Ƚ�ֵ heater:����Ԫ�� compare:�Ƚ�ֵ*/
void Heater_SetCompare(Heater_TypeDef heater, unsigned short int compare);

/*����Ԫ����ȡpwm�Ƚ�ֵ heater:����Ԫ��*/
int Heater_GetCompare(Heater_TypeDef heater);

/*����Ԫ����ȡ��ֵ heater:����Ԫ��*/
unsigned short int Heater_GetValue(Heater_TypeDef heater);

/*����Ԫ���趨�¶�ֵ(��) heater:����Ԫ��*/
void Heater_SetTargetCelsius(const float celsius, Heater_TypeDef heater);

/*����Ԫ����ȡ��ǰ�¶�ֵ(��) heater:����Ԫ��*/
float Heater_GetCurrentCelsius(Heater_TypeDef heater);

/*����Ԫ����ȡ�趨�¶�ֵ(��) heater:����Ԫ��*/
float Heater_GetTargetCelsius(Heater_TypeDef heater);

/*�жϼ���Ԫ���Ƿ����ڼ��� heater:����Ԫ��*/
bool Heater_IsHeating(Heater_TypeDef heater);

/*�жϼ���Ԫ���Ƿ�������ȴ heater:����Ԫ��*/
bool Heater_IsCooling(Heater_TypeDef heater);

/*����Ԫ��PID�Զ���г temp:�趨�¶� heater:����Ԫ�� ncycles:ѭ������*/
void Heater_PID_AutoTune(float temp, int heater, int ncycles);

/*����Ԫ���Զ�ֹͣ*/
void Heater_AutoShutdown(void);

/*����Ԫ�������� tempError:�¶ȴ������� heater:����Ԫ��*/
void Heater_TemperatureError(Temp_ErrorTypeDef tempError, Heater_TypeDef heater);

/*����Ԫ������ǰ���*/
void Heater_SetWatch(void);

/*����Ԫ��ʧ��*/
void Heater_Disable(void);

/*����Ԫ������PID*/
void Heater_SetPID(void);

/*�¶ȿ��Ƴ�ʼ��*/
void Temperature_Configuration(void);

/*�¶ȿ��ƹ���*/
void Temperature_Manage(void);

/*�¶ȿ���ִ�н���*/
void Temperature_Process(void);

#ifdef PIDTEMP
/*�¶�PID�������� parameter:����(0,I;1,D) value:ֵ scaleType:��������(0,�Ŵ�;1,��С)*/
float Temperature_ScalePID(char parameter, float value, unsigned char scaleType);
#endif

#ifdef __cplusplus
}
#endif

#endif
