/**
  ******************************************************************************
  * @file     stepper_motor.c
  * @author   LEO
	* @date     2018/10/13
	* @version  0.0.1
  * @brief    步进电机运动控制
	******************************************************************************
**/
#include "stepper_motor.h"
#include "planner.h"
#include "printer.h"
#include "printer_conf.h"
#include "usart.h"
#include "language.h"
#include "temperature.h"
#include "endstop.h"

static unsigned char subsection_x_value = 1;
static unsigned char subsection_y_value = 1;
static unsigned char subsection_z_value = 1;
static unsigned char subsection_e0_value = 1;
static unsigned char subsection_e1_value = 1;

block_t *current_block;  // A pointer to the block currently being traced

// Variables used by The Stepper Driver Interrupt
static unsigned char out_bits;        // The next stepping-bits to be output
static long counter_x, counter_y, counter_z, counter_e; // Counter variables for the bresenham line tracer
volatile static unsigned long step_events_completed; // The number of step events executed in the current block
#ifdef ADVANCE
static long advance_rate, advance, final_advance = 0;
static long old_advance = 0;
static long e_steps[3];
#endif
static long acceleration_time, deceleration_time;
//static unsigned long accelerate_until, decelerate_after, acceleration_rate, initial_rate, final_rate, nominal_rate;
static unsigned short acc_step_rate; // needed for deccelaration start point
static char step_loops;
static unsigned short TIME3_nominal;
//static unsigned short step_loops_nominal;

//===========================================================================
//=============================functions         ============================
//===========================================================================

#define MultiU24X24toH16(intRes, longIn1, longIn2) intRes= ((uint64_t)(longIn1) * (longIn2)) >> 24
#define MultiU16X8toH16(intRes, charIn1, intIn2) intRes = ((charIn1) * (intIn2)) >> 16

unsigned short calc_timer(unsigned short step_rate)
{
    unsigned short timer;
    if(step_rate > MAX_STEP_FREQUENCY) step_rate = MAX_STEP_FREQUENCY;
    if(step_rate > 20000)
    {
        step_rate = (step_rate >> 2)&0x3FFF;
        step_loops = 4;
    }
    else if(step_rate > 10000)
    {
        step_rate = (step_rate >> 1)&0x7FFF;
        step_loops = 2;
    }
    else
    {
        step_loops = 1;
    }
    if(step_rate < 32) step_rate = 32;
    timer = 2000000/step_rate - 1;
    if(timer < 100)//(20kHz this should never happen)
    {
        timer = 100;
        printf(MSG_STEPPER_TO_HIGH);
        printf("%d", step_rate);
    }
    return timer;
}

// Initializes the trapezoid generator from the current block. Called whenever a new
// block begins.
void trapezoid_generator_reset(void)
{
#ifdef ADVANCE
    advance = current_block->initial_advance;
    final_advance = current_block->final_advance;
    // Do E steps + advance steps
    e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
    old_advance = advance >>8;
#endif
    deceleration_time = 0;
    // step_rate to timer interval
    TIME3_nominal = calc_timer(current_block->nominal_rate);
    // make a note of the number of step loops required at nominal speed
    //step_loops_nominal = step_loops;
    acc_step_rate = current_block->initial_rate;
    acceleration_time = calc_timer(acc_step_rate);
    __HAL_TIM_SET_AUTORELOAD(&TIM3_HandleStruct, acceleration_time-1);
}

// "The Stepper Driver Interrupt" - This timer interrupt is the workhorse.
// It pops blocks from the block_buffer and executes them by pulsing the stepper pins appropriately.

void Stepper_Interrupt(void)
{
//printf("T3\n");
    // If there is no current block, attempt to pop one from the buffer
    if(current_block == NULL)
    {
        // Anything in the buffer?
        current_block = plan_get_current_block();
        if(current_block != NULL)
        {
            current_block->busy = true;
            trapezoid_generator_reset();
            counter_x = -(current_block->step_event_count >> 1);
            counter_y = counter_x;
            counter_z = counter_x;
            counter_e = counter_x;
            step_events_completed = 0;
#ifdef Z_LATE_ENABLE
            if(current_block->steps_z > 0) {
                enable_z();
                TIM_SetAutoreload(TIM3, 2000-1);//1ms wait
                return;
            }
#endif
//      #ifdef ADVANCE
//      e_steps[current_block->active_extruder] = 0;
//      #endif
        }
        else __HAL_TIM_SET_AUTORELOAD(&TIM3_HandleStruct, 2000-1);
    }
    if(current_block != NULL)
    {
        // Set directions TO DO This should be done once during init of trapezoid. Endstops -> interrupt
        out_bits = current_block->direction_bits;
        // Set direction en check limit switches
        if((out_bits & (1<<X_AXIS)) != 0) // stepping along -X axis
        {
#ifndef COREXY  //NOT COREXY
            Stepper_DirPin_Control(X_AXIS, REVERSE_DIR);
#endif
            Count_Direction[X_AXIS] = -1;
            if(EndStops_Check)
            {
#if X_MIN_PIN_USED
                bool x_min_endstop = EndStops_CheckState(X_AXIS, MIN_POSITION) != X_ENDSTOPS_INVERTING;
                if(x_min_endstop && EndStop_X_Min_Old && (current_block->steps_x > 0))
                {
                    EndStops_TrigSteps[X_AXIS] = Count_Position[X_AXIS];
                    EndStop_X_Hit = true;
                    step_events_completed = current_block->step_event_count;
                }
                EndStop_X_Min_Old = x_min_endstop;
#endif
            }
        }
        else// +direction
        {
#ifndef COREXY  //NOT COREXY
            Stepper_DirPin_Control(X_AXIS, FORWARD_DIR);
#endif
            Count_Direction[X_AXIS] = 1;
            if(EndStops_Check)
            {
#if X_MAX_PIN_USED
                bool x_max_endstop = EndStops_CheckState(X_AXIS, MAX_POSITION) != X_ENDSTOPS_INVERTING;
                if(x_max_endstop && EndStop_X_Max_Old && (current_block->steps_x > 0)) {
                    EndStops_TrigSteps[X_AXIS] = Count_Position[X_AXIS];
                    EndStop_X_Hit = true;
                    step_events_completed = current_block->step_event_count;
                }
                EndStop_X_Max_Old = x_max_endstop;
#endif
            }
        }
        if((out_bits & (1<<Y_AXIS)) != 0) // -direction
        {
#ifndef COREXY  //NOT COREXY
            Stepper_DirPin_Control(Y_AXIS, REVERSE_DIR);
#endif
            Count_Direction[Y_AXIS] = -1;
            if(EndStops_Check)
            {
#if Y_MIN_PIN_USED //&& Y_MIN_PIN > -1
                bool y_min_endstop = EndStops_CheckState(Y_AXIS, MIN_POSITION) != Y_ENDSTOPS_INVERTING;
                if(y_min_endstop && EndStop_Y_Min_Old && (current_block->steps_y > 0)) {
                    EndStops_TrigSteps[Y_AXIS] = Count_Position[Y_AXIS];
                    EndStop_Y_Hit = true;
                    step_events_completed = current_block->step_event_count;
                }
                EndStop_Y_Min_Old = y_min_endstop;
#endif
            }
        }
        else // +direction
        {
#ifndef COREXY  //NOT COREXY
            Stepper_DirPin_Control(Y_AXIS, FORWARD_DIR);
#endif
            Count_Direction[Y_AXIS] = 1;
            if(EndStops_Check)
            {
#if Y_MAX_PIN_USED// && Y_MAX_PIN > -1
                bool y_max_endstop = EndStops_CheckState(Y_AXIS, MAX_POSITION) != Y_ENDSTOPS_INVERTING;
                if(y_max_endstop && EndStop_Y_Max_Old && (current_block->steps_y > 0)) {
                    EndStops_TrigSteps[Y_AXIS] = Count_Position[Y_AXIS];
                    EndStop_Y_Hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                EndStop_Y_Max_Old = y_max_endstop;
#endif
            }
        }
        if((out_bits & (1<<Z_AXIS)) != 0)   // -direction
        {
            Stepper_DirPin_Control(Z_AXIS, REVERSE_DIR);
#ifdef Z_DUAL_STEPPER_DRIVERS
            // WRITE(Z2_DIR_PIN,INVERT_Z_DIR);
#endif
            Count_Direction[Z_AXIS] = -1;
            if(EndStops_Check)
            {
#if Z_MIN_PIN_USED
                bool z_min_endstop = EndStops_CheckState(Z_AXIS, MIN_POSITION) != Z_ENDSTOPS_INVERTING;
                if(z_min_endstop && EndStop_Z_Min_Old && (current_block->steps_z > 0)) {
                    EndStops_TrigSteps[Z_AXIS] = Count_Position[Z_AXIS];
                    EndStop_Z_Hit = true;
                    step_events_completed = current_block->step_event_count;
                }
                EndStop_Z_Min_Old = z_min_endstop;
#endif
            }
        }
        else // +direction
        {
            Stepper_DirPin_Control(Z_AXIS, FORWARD_DIR);
#ifdef Z_DUAL_STEPPER_DRIVERS
            // WRITE(Z2_DIR_PIN,!INVERT_Z_DIR);
#endif
            Count_Direction[Z_AXIS] = 1;
            if(EndStops_Check)
            {
#if Z_MAX_PIN_USED
                bool z_max_endstop = EndStops_CheckState(Z_AXIS, MAX_POSITION) != Z_ENDSTOPS_INVERTING;
                if(z_max_endstop && old_z_max_endstop && (current_block->steps_z > 0)) {
                    EndStops_TrigSteps[Z_AXIS] = Count_Position[Z_AXIS];
                    endstop_z_hit=true;
                    step_events_completed = current_block->step_event_count;
                }
                old_z_max_endstop = z_max_endstop;
#endif
            }
        }
#ifndef ADVANCE
        if((out_bits & (1<<E0_AXIS)) != 0)  // -direction
        {
            Stepper_DirPin_Control(E0_AXIS, REVERSE_DIR);
            Count_Direction[E0_AXIS] = -1;
        }
        else // +direction
        {
            Stepper_DirPin_Control(E0_AXIS, FORWARD_DIR);
            Count_Direction[E0_AXIS] = 1;
        }
#endif
        {
            unsigned char i;
            for(i=0; i<step_loops; i++) // Take multiple steps per interrupt (For high speed moves)
            {
                //  printf("1\n\r");
//	  #if !EN_USART1_RX
//      checkRx(); // Check for serial chars.//20160228
//     #endif
#ifdef ADVANCE
                counter_e += current_block->steps_e;
                if(counter_e > 0)
                {
                    counter_e -= current_block->step_event_count;
                    if((out_bits & (1<<E0_AXIS)) != 0) // - direction
                    {
                        e_steps[current_block->active_extruder]--;
                    }
                    else
                    {
                        e_steps[current_block->active_extruder]++;
                    }
                }
#endif
#ifndef COREXY
                counter_x += current_block->steps_x;
                if(counter_x > 0)
                {
                    Stepper_PulsePin_Control(X_AXIS, ACTIVE_PULSE);
                    counter_x -= current_block->step_event_count;
                    Count_Position[X_AXIS] += Count_Direction[X_AXIS];
                    Stepper_PulsePin_Control(X_AXIS, INACTIVE_PULSE);
                }
                counter_y += current_block->steps_y;
                if(counter_y > 0)
                {
                    Stepper_PulsePin_Control(Y_AXIS, ACTIVE_PULSE);
                    counter_y -= current_block->step_event_count;
                    Count_Position[Y_AXIS] += Count_Direction[Y_AXIS];
                    Stepper_PulsePin_Control(Y_AXIS, INACTIVE_PULSE);
                }
#endif
                counter_z += current_block->steps_z;
                if(counter_z > 0)
                {
                    Stepper_PulsePin_Control(Z_AXIS, ACTIVE_PULSE);
#ifdef Z_DUAL_STEPPER_DRIVERS
                    Z2_STEP_PIN= !INVERT_Z_STEP_PIN;
#endif
                    counter_z -= current_block->step_event_count;
                    Count_Position[Z_AXIS] += Count_Direction[Z_AXIS];
                    Stepper_PulsePin_Control(Z_AXIS, INACTIVE_PULSE);
#ifdef Z_DUAL_STEPPER_DRIVERS
                    Z2_STEP_PIN= INVERT_Z_STEP_PIN;
#endif
                }
#ifndef ADVANCE
                counter_e += current_block->steps_e;
                if(counter_e > 0)
                {
                    Stepper_PulsePin_Control(E0_AXIS, ACTIVE_PULSE);
                    counter_e -= current_block->step_event_count;
                    Count_Position[E0_AXIS] += Count_Direction[E0_AXIS];
                    Stepper_PulsePin_Control(E0_AXIS, INACTIVE_PULSE);
                }
#endif
                step_events_completed += 1;
                if(step_events_completed >= current_block->step_event_count) break;
            }
        }
        {   // Calculare new timer value
            unsigned short timer;
            unsigned short step_rate;
            if(step_events_completed <= (unsigned long int)current_block->accelerate_until)
            {
                MultiU24X24toH16(acc_step_rate, acceleration_time, current_block->acceleration_rate);
                acc_step_rate += current_block->initial_rate;
                // upper limit
                if(acc_step_rate > current_block->nominal_rate) acc_step_rate = current_block->nominal_rate;
                // step_rate to timer interval
                timer = calc_timer(acc_step_rate);
                //  printf("1:%ld\r\n",timer);//timer4_millis);
                __HAL_TIM_SET_AUTORELOAD(&TIM3_HandleStruct, timer-1);
                acceleration_time += timer;
#ifdef ADVANCE
                for(i=0; i < step_loops; i++) {
                    advance += advance_rate;
                }
                //if(advance > current_block->advance) advance = current_block->advance;
                // Do E steps + advance steps
                e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
                old_advance = advance >>8;
#endif
            }
            else if(step_events_completed > (unsigned long int)current_block->decelerate_after)
            {
                MultiU24X24toH16(step_rate, deceleration_time, current_block->acceleration_rate);
                if(step_rate > acc_step_rate) // Check step_rate stays positive
                {
                    step_rate = current_block->final_rate;
                }
                else // Decelerate from aceleration end point.
                {
                    step_rate = acc_step_rate - step_rate;
                }
                // lower limit
                if(step_rate < current_block->final_rate) step_rate = current_block->final_rate;
                // step_rate to timer interval
                timer = calc_timer(step_rate);
                //  printf("2:%ld\r\n",timer);
                __HAL_TIM_SET_AUTORELOAD(&TIM3_HandleStruct, timer-1);
                deceleration_time += timer;
#ifdef ADVANCE
                for(i=0; i < step_loops; i++)
                {
                    advance -= advance_rate;
                }
                if(advance < final_advance) advance = final_advance;
                // Do E steps + advance steps
                e_steps[current_block->active_extruder] += ((advance >>8) - old_advance);
                old_advance = advance >>8;
#endif
            }
            else
            {   // printf("3:%ld\r\n",TIME3_nominal);
                __HAL_TIM_SET_AUTORELOAD(&TIM3_HandleStruct, TIME3_nominal-1);
                // ensure we're running at the correct step rate, even if we just came off an acceleration
                //  step_loops = step_loops_nominal;
            }
        }     // Calculare new timer value
        // If current block is finished, reset pointer
        if(step_events_completed >= current_block->step_event_count)
        {
            current_block = NULL;
            plan_discard_current_block();
        }
    }
}

/*步进电机驱动初始化 PB0:X_EN PB2:X_DIR PB1:X_STEP PB8:Y_EN PB6:Y_DIR PB7:Y_STEP PB5:Z_EN PB3:Z_DIR PB4:Z_STEP PA15:E0_EN PB15:E0_DIR PC6:E0_STEP*/
void Stepper_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    Stepper_EnPin_Control(X_AXIS, DISABLE);
    Stepper_EnPin_Control(Y_AXIS, DISABLE);
    Stepper_EnPin_Control(Z_AXIS, DISABLE);
    Stepper_EnPin_Control(E0_AXIS, DISABLE);
    Stepper_EnPin_Control(E1_AXIS, DISABLE);
    Stepper_PulsePin_Control(X_AXIS, INACTIVE_PULSE);
    Stepper_PulsePin_Control(Y_AXIS, INACTIVE_PULSE);
    Stepper_PulsePin_Control(Z_AXIS, INACTIVE_PULSE);
    Stepper_PulsePin_Control(E0_AXIS, INACTIVE_PULSE);
    Stepper_PulsePin_Control(E1_AXIS, INACTIVE_PULSE);
    TIM3_Configuration(35, 0x4000);
    ENABLE_STEPPER_DRIVER_INTERRUPT();
}

/*步进电机同步控制(缓冲步数全部执行之前阻塞)*/
void Stepper_Synchronize(void)
{
    while(blocks_queued())
    {
        Temperature_Manage();
        manage_inactivity();
        //LEO	lcd_update();
    }
}

/*步进电机设定位置*/
void Stepper_SetPosition(const long x, const long y, const long z, const long e)
{
    CRITICAL_SECTION_START;
    Count_Position[X_AXIS] = x;
    Count_Position[Y_AXIS] = y;
    Count_Position[Z_AXIS] = z;
    Count_Position[E0_AXIS] = e;
    CRITICAL_SECTION_END;
}

/*步进电机设定E轴位置*/
void Stepper_SetPosition_E(const long e)
{
    CRITICAL_SECTION_START;
    Count_Position[E0_AXIS] = e;
    CRITICAL_SECTION_END;
}

/*步进电机获取位置*/
long Stepper_GetPosition(Axis_EnumTypeDef axis)
{
    long count_pos;
    CRITICAL_SECTION_START;
    count_pos = Count_Position[axis];
    CRITICAL_SECTION_END;
    return count_pos;
}

/*步进电机完成操作并失能*/
void Stepper_Finish_Disable(void)
{
    Stepper_Synchronize();
    Stepper_EnPin_Control(X_AXIS, DISABLE);
    Stepper_EnPin_Control(Y_AXIS, DISABLE);
    Stepper_EnPin_Control(Z_AXIS, DISABLE);
    Stepper_EnPin_Control(E0_AXIS, DISABLE);
    Stepper_EnPin_Control(E1_AXIS, DISABLE);
}

/*步进电机快速停止*/
void Stepper_QuickStop(void)
{
    DISABLE_STEPPER_DRIVER_INTERRUPT();
    while(blocks_queued()) plan_discard_current_block();
    current_block = NULL;
    ENABLE_STEPPER_DRIVER_INTERRUPT();
}

void microstep_ms(uint8_t driver, int8_t ms1, int8_t ms2, int8_t ms3)
{   /*
     if(ms1 > -1) switch(driver)
     {
       case 0:X_MS1_PIN=ms1 ; break;
       case 1:Y_MS1_PIN=ms1 ; break;
       case 2:Z_MS1_PIN=ms1 ; break;
       case 3:E0_MS1_PIN=ms1 ; break;
       case 4:E1_MS1_PIN=ms1 ; break;
    default:  break;
     }
     if(ms2 > -1)
     switch(driver)
     {
       case 0:X_MS2_PIN=ms2 ; break;
       case 1:Y_MS2_PIN=ms2 ; break;
       case 2:Z_MS2_PIN=ms2 ; break;
       case 3:E0_MS2_PIN=ms2 ; break;
       case 4:E1_MS2_PIN=ms2 ; break;
    default:  break;
     }
       if(ms3 > -1) switch(driver)
     {
       case 0:X_MS3_PIN=ms3 ; break;
       case 1:Y_MS3_PIN=ms3 ; break;
       case 2:Z_MS3_PIN=ms3 ; break;
       case 3:E0_MS3_PIN=ms3 ; break;
       case 4:E1_MS3_PIN=ms3 ; break;
    default:  break;
     }
    */
}

void microstep_mode(uint8_t driver, uint8_t stepping_mode)
{
    switch(driver)
    {
    case 0:
        subsection_x_value=stepping_mode;
        break;
    case 1:
        subsection_y_value=stepping_mode;
        break;
    case 2:
        subsection_z_value=stepping_mode;
        break;
    case 3:
        subsection_e0_value=stepping_mode;
        break;
    case 4:
        subsection_e1_value=stepping_mode;
        break;
    default:
        break;
    }
    switch(stepping_mode)
    {
//    case 1: microstep_ms(driver,MICROSTEP1); break;
//    case 2: microstep_ms(driver,MICROSTEP2); break;
//    case 4: microstep_ms(driver,MICROSTEP4); break;
//    case 8: microstep_ms(driver,MICROSTEP8); break;
//    case 16: microstep_ms(driver,MICROSTEP16); break;
//    case 32: microstep_ms(driver,MICROSTEP32); break;
//    case 64: microstep_ms(driver,MICROSTEP64); break;
//    case 128: microstep_ms(driver,MICROSTEP128); break;
    default:
        break;
    }
}

void microstep_readings(void)
{
    printf("Motor_Subsection \n");
    printf("X: %d\n", subsection_x_value);
    printf("Y: %d\n", subsection_y_value);
    printf("Z: %d\n", subsection_z_value);
    printf("E0: %d\n", subsection_e0_value);
    printf("E1: %d\n", subsection_e1_value);
}

/*步进电机驱动使能控制 axis:指定轴 state:状态*/
void Stepper_EnPin_Control(Axis_EnumTypeDef axis, FunctionalState state)
{
    if(axis == X_AXIS)
    {
        if(state == ENABLE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (GPIO_PinState)X_ENABLE_ON);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (GPIO_PinState)!X_ENABLE_ON);
    }
    else if(axis == Y_AXIS)
    {
        if(state == ENABLE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)Y_ENABLE_ON);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)!Y_ENABLE_ON);
    }
    else if(axis == Z_AXIS)
    {
        if(state == ENABLE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (GPIO_PinState)Z_ENABLE_ON);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (GPIO_PinState)!Z_ENABLE_ON);
    }
    else if(axis == E0_AXIS)
    {
        if(state == ENABLE) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)E_ENABLE_ON);
        else HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)!E_ENABLE_ON);
    }
    else if(axis == E1_AXIS)
    {
//		if(state == ENABLE) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)E_ENABLE_ON);
//		else HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)!E_ENABLE_ON);
    }
}

/*步进电机驱动方向控制 axis:指定轴 dir:轴方向*/
void Stepper_DirPin_Control(Axis_EnumTypeDef axis, Axis_DirTypeDef dir)
{
    if(axis == X_AXIS)
    {
        if(dir == FORWARD_DIR) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (GPIO_PinState)!INVERT_X_DIR);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, (GPIO_PinState)INVERT_X_DIR);
    }
    else if(axis == Y_AXIS)
    {
        if(dir == FORWARD_DIR) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (GPIO_PinState)!INVERT_Y_DIR);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (GPIO_PinState)INVERT_Y_DIR);
    }
    else if(axis == Z_AXIS)
    {
        if(dir == FORWARD_DIR) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, (GPIO_PinState)!INVERT_Z_DIR);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, (GPIO_PinState)INVERT_Z_DIR);
    }
    else if(axis == E0_AXIS)
    {
        if(dir == FORWARD_DIR) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (GPIO_PinState)!INVERT_E0_DIR);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (GPIO_PinState)INVERT_E0_DIR);
    }
    else if(axis == E1_AXIS)
    {
//		if(dir == FORWARD_DIR) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, (GPIO_PinState)!INVERT_E1_DIR);
//		else HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, (GPIO_PinState)INVERT_E1_DIR);
    }
}

/*步进电机驱动脉冲控制 axis:指定轴 pulse:脉冲状态*/
void Stepper_PulsePin_Control(Axis_EnumTypeDef axis, Axis_PulseTypeDef pulse)
{
    if(axis == X_AXIS)
    {
        if(pulse == ACTIVE_PULSE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (GPIO_PinState)!INVERT_X_STEP_PIN);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (GPIO_PinState)INVERT_X_STEP_PIN);
    }
    else if(axis == Y_AXIS)
    {
        if(pulse == ACTIVE_PULSE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (GPIO_PinState)!INVERT_Y_STEP_PIN);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (GPIO_PinState)INVERT_Y_STEP_PIN);
    }
    else if(axis == Z_AXIS)
    {
        if(pulse == ACTIVE_PULSE) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (GPIO_PinState)!INVERT_Z_STEP_PIN);
        else HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (GPIO_PinState)INVERT_Z_STEP_PIN);
    }
    else if(axis == E0_AXIS)
    {
        if(pulse == ACTIVE_PULSE) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (GPIO_PinState)!INVERT_E_STEP_PIN);
        else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (GPIO_PinState)INVERT_E_STEP_PIN);
    }
    else if(axis == E1_AXIS)
    {
//		if(pulse == ACTIVE_PULSE) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, (GPIO_PinState)!INVERT_E_STEP_PIN);
//		else HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, (GPIO_PinState)INVERT_E_STEP_PIN);
    }
}
