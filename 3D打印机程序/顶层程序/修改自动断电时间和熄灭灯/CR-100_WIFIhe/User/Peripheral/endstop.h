#ifndef __ENDSTOP_H
#define __ENDSTOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "stepper_motor.h"
#include "printer.h"

extern volatile long EndStops_TrigSteps[];
extern volatile long Count_Position[];
extern volatile signed char Count_Direction[];
extern volatile bool EndStop_X_Hit;
extern volatile bool EndStop_Y_Hit;
extern volatile bool EndStop_Z_Hit;
extern bool EndStops_Check;

#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED
extern bool abort_on_endstop_hit;
#endif
#if X_MIN_PIN_USED
extern bool EndStop_X_Min_Old;
#endif
#if X_MAX_PIN_USED
extern bool EndStop_X_Max_Old;
#endif
#if Y_MIN_PIN_USED
extern bool EndStop_Y_Min_Old;
#endif
#if Y_MAX_PIN_USED
extern bool EndStop_Y_Max_Old;
#endif
#if Z_MIN_PIN_USED
extern bool EndStop_Z_Min_Old;
#endif
#if Z_MAX_PIN_USED
extern bool EndStop_Z_Max_Old;
#endif

typedef enum
{
    MIN_POSITION  = 0,
    MAX_POSITION  = 1
} EndStop_PositionTypeDef;
/*自动关机初始化，PC1*/
void Automatic_Power_Off_Configuration(void);//APO_Configuration
void Automatic_Power_Off(void);

/*限位开关初始化 PA5:X_Min PA6:Y_Min PA7:Z_Min*/
void EndStops_Configuration(void);

/*限位开关检测开关状态 axis:指定轴 position:开关位置*/
bool EndStops_CheckState(Axis_EnumTypeDef axis, EndStop_PositionTypeDef position);

/*限位开关报告状态*/
void EndStops_ReportState(void);

/*限位开关清除触发状态*/
void EndStops_Clear(void);

/*限位开关设置检测状态*/
void EndStops_Enable(bool check);

#ifdef __cplusplus
}
#endif

#endif
