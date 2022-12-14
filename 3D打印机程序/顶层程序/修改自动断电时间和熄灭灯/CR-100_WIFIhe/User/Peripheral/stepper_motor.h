#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "timer.h"

#define ENABLE_STEPPER_DRIVER_INTERRUPT() HAL_TIM_Base_Start_IT(&TIM3_HandleStruct);
#define DISABLE_STEPPER_DRIVER_INTERRUPT() HAL_TIM_Base_Stop_IT(&TIM3_HandleStruct);

typedef enum
{
    X_AXIS  = 0,
    Y_AXIS  = 1,
    Z_AXIS  = 2,
    E0_AXIS = 3,
    E1_AXIS = 4
} Axis_EnumTypeDef;

typedef enum
{
    FORWARD_DIR  = 0,
    REVERSE_DIR  = 1
} Axis_DirTypeDef;

typedef enum
{
    ACTIVE_PULSE    = 0,
    INACTIVE_PULSE  = 1
} Axis_PulseTypeDef;

#if EXTRUDERS > 1  //挤压机
#define WRITE_E_STEP(v) { if(current_block->active_extruder == 1) { E1_STEP_PIN=v; } else { E0_STEP_PIN=v; }}
#define NORM_E_DIR() { if(current_block->active_extruder == 1) { E1_DIR_PIN=!INVERT_E1_DIR; } else {E0_DIR_PIN=!INVERT_E0_DIR; }}
#define REV_E_DIR()  { if(current_block->active_extruder == 1) { E1_DIR_PIN=INVERT_E1_DIR; }  else { E0_DIR_PIN=INVERT_E0_DIR; }}
#else
//LEO  #define WRITE_E_STEP(v) E0_STEP_PIN=v
//LEO  #define NORM_E_DIR() E0_DIR_PIN=!INVERT_E0_DIR
//LEO  #define REV_E_DIR()  E0_DIR_PIN=INVERT_E0_DIR
#endif

// The stepper subsystem goes to sleep when it runs out of things to execute. Call this
// to notify the subsystem that it is time to go to work.

void microstep_ms(uint8_t driver, int8_t ms1, int8_t ms2, int8_t ms3);
void microstep_mode(uint8_t driver, uint8_t stepping);

/*步进电机驱动初始化 PB0:X_EN PB2:X_DIR PB1:X_STEP PB8:Y_EN PB6:Y_DIR PB7:Y_STEP PB5:Z_EN PB3:Z_DIR PB4:Z_STEP PA15:E0_EN PB15:E0_DIR PC6:E0_STEP*/
void Stepper_Configuration(void);

/*步进电机同步控制(缓冲步数全部执行之前阻塞)*/
void Stepper_Synchronize(void);

/*步进电机设定位置*/
void Stepper_SetPosition(const long x, const long y, const long z, const long e);

/*步进电机设定E轴位置*/
void Stepper_SetPosition_E(const long e);

/*步进电机获取位置*/
long Stepper_GetPosition(Axis_EnumTypeDef axis);

/*步进电机完成操作并失能*/
void Stepper_Finish_Disable(void);

/*步进电机快速停止*/
void Stepper_QuickStop(void);

void microstep_readings(void);

void Stepper_Interrupt(void);

/*步进电机驱动使能控制 axis:指定轴 state:状态*/
void Stepper_EnPin_Control(Axis_EnumTypeDef axis, FunctionalState state);

/*步进电机驱动方向控制 axis:指定轴 dir:轴方向*/
void Stepper_DirPin_Control(Axis_EnumTypeDef axis, Axis_DirTypeDef dir);

/*步进电机驱动脉冲控制 axis:指定轴 pulse:脉冲状态*/
void Stepper_PulsePin_Control(Axis_EnumTypeDef axis, Axis_PulseTypeDef pulse);

#ifdef __cplusplus
}
#endif

#endif
