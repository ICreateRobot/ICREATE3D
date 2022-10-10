#ifndef __PRINTER_H
#define __PRINTER_H

//#define  FORCE_INLINE __attribute__((always_inline)) inline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "printer_conf.h"
#include "rtc.h"

//#define OK_KEY (Get_Adc_Average(ADC_Channel_15,10)>3250)

/*打印执行进程*/
void Printer_Process(void);
/*打印获取指令*/
void Printer_GetCommand(void);
void process_commands(void);
void manage_inactivity(void);

void FlushSerialRequestResend(void);
void ClearToSend(void);

void get_coordinates(void);
void prepare_move(void);
void kill(void);
void Stop(void);

bool IsStopped(void);

void enquecommand(const char *cmd); //put an ascii command at the end of the current buffer.
//void enquecommand_P(const char *cmd); //put an ascii command at the end of the current buffer, read from flash
void mc_arc(float *position, float *target, float *offset, unsigned char axis_0, unsigned char axis_1,
  unsigned char axis_linear, float feed_rate, float radius, unsigned char isclockwise, uint8_t extruder);
void prepare_arc_move(unsigned char isclockwise);

void clamp_to_software_endstops(float target[3]);

//#ifdef EEPROM_CHITCHAT
void Config_PrintSettings(void);
//#else
//#endif
//#ifdef EEPROM_SETTINGS
//void Config_StoreSettings(void);
//void Config_RetrieveSettings(void);
//#else

//#endif

#define CRITICAL_SECTION_START  __disable_irq();
#define CRITICAL_SECTION_END    __enable_irq();
extern float homing_feedrate[];
extern bool axis_relative_modes[];
extern int feedmultiply;
extern int extrudemultiply; // Sets extrude multiply factor (in percent)
extern float current_position[NUM_AXIS] ;
extern float add_homeing[3];
extern float min_pos[3];
extern float max_pos[3];
extern int fanSpeed;
#ifdef BARICUDA
extern int ValvePressure;
extern int EtoPPressure;
#endif

#if FWRETRACT
extern bool autoretract_enabled;
extern bool retracted;
extern float retract_length, retract_feedrate, retract_zlift;
extern float retract_recover_length, retract_recover_feedrate;
#endif

extern RTC_DurationTypeDef Duration_Time;

// Handling multiple extruders pins
extern uint8_t active_extruder;
#endif
