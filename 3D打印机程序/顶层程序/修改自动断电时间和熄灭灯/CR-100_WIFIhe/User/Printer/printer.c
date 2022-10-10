#include "printer.h"
#include "planner.h"
#include "stepper_motor.h"
#include "temperature.h"
#include "language.h"
#include "usart.h"
#include "sd_card.h"
#include "timer.h"
#include "endstop.h"
#include "hmi.h"
#include "keypad.h"
#include "Remote.h"
#include "storage_device.h"
#include "gcode.h"

#ifdef HOST_KEEPALIVE_FEATURE
void set_busy_state(uint8_t busy);
#endif
#ifdef TEMPRATURE_AUTO_REPORT
void set_temp_report_interval(uint8_t interval);
#endif

float homing_feedrate[] = HOMING_FEEDRATE;
bool axis_relative_modes[] = AXIS_RELATIVE_MODES;
int feedmultiply = 100; //100->1 200->2
int saved_feedmultiply;
int extrudemultiply = 100; //100->1 200->2
float current_position[NUM_AXIS] = {0.0, 0.0, 0.0, 0.0};
float add_homeing[3] = {0 ,0, 0};
float min_pos[3] = {X_MIN_POS, Y_MIN_POS, Z_MIN_POS};
float max_pos[3] = {X_MAX_POS, Y_MAX_POS, Z_MAX_POS};

static const float base_min_pos[3] = {X_MIN_POS, Y_MIN_POS, Z_MIN_POS};
static float base_max_pos[3] = {X_MAX_POS, Y_MAX_POS, Z_MAX_POS};
static const float base_home_pos[3] = {X_HOME_POS, Y_HOME_POS, Z_HOME_POS};
static float max_length[3] = {X_MAX_LENGTH, Y_MAX_LENGTH, Z_MAX_LENGTH};
static const float home_retract_mm[3]= {X_HOME_RETRACT_MM, Y_HOME_RETRACT_MM, Z_HOME_RETRACT_MM};
static const signed char home_dir[3] = {X_HOME_DIR, Y_HOME_DIR, Z_HOME_DIR};

// Extruder offset, only in XY plane
#if EXTRUDERS > 1
float extruder_offset[2][EXTRUDERS] = {
#if defined(EXTRUDER_OFFSET_X) && defined(EXTRUDER_OFFSET_Y)
  EXTRUDER_OFFSET_X, EXTRUDER_OFFSET_Y
#endif
};
#endif
uint8_t active_extruder = 0;
int fanSpeed = 0;
#ifdef BARICUDA
int ValvePressure=0;
int EtoPPressure=0;
#endif

#if FWRETRACT
bool autoretract_enabled=true;
bool retracted=false;
float retract_length=3, retract_feedrate=17*60, retract_zlift=0.8;
float retract_recover_length=0, retract_recover_feedrate=8*60;
#endif
//===========================================================================
//=============================private variables=============================
//===========================================================================
const char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
float destination[NUM_AXIS] = {0.0, 0.0, 0.0, 0.0};
float offset[3] = {0.0, 0.0, 0.0};
static bool home_all_axis = true;
static float feedrate = 1500.0, next_feedrate, saved_feedrate;
static long gcode_N, gcode_LastN, Stopped_gcode_LastN = 0;

volatile static bool relative_mode = false;  //Determines Absolute or Relative Coordinates

static char cmdbuffer[BUFSIZE][MAX_CMD_SIZE];
volatile static bool fromsd[BUFSIZE];
static int bufindr = 0;
static int bufindw = 0;
static int buflen = 0;
//static int i = 0;
static char serial_char;
static int serial_count = 0;
static bool comment_mode = false;
static char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc

//const int sensitive_pins[] = SENSITIVE_PINS; // Sensitive pin list for M42

//static float tt = 0;
//static float bt = 0;

//Inactivity shutdown variables
static unsigned long previous_millis_cmd = 0;
static unsigned long max_inactive_time = 0;
static unsigned long stepper_inactive_time = DEFAULT_STEPPER_DEACTIVE_TIME * 1000l;

RTC_DurationTypeDef Duration_Time;
static unsigned char tmp_extruder;

bool Stopped = false;

//#if NUM_SERVOS > 0
//  Servo servos[NUM_SERVOS];
//#endif
//===========================================================================
//=============================ROUTINES=============================
//===========================================================================

void get_arc_coordinates(void);
bool setTargetedHotend(int code);

// Print execution process
void Printer_Process(void)
{
  if(buflen)
  {
#if SD_SUPPORT
    if(Storage_Device == Storage_SD)
    {
      if(SD_Reader.Saving)
      {
        if(strstr(cmdbuffer[bufindr], (const char*)("M29")) == NULL)
        {
          // LEO
          // card_write_command(cmdbuffer[bufindr]);
          printf(MSG_OK);
        }
        else
        {
          StorageDevice_CloseFile();
          printf(MSG_FILE_SAVED);
        }
      }
      else 
      {
        // Adds the judgment "C \n" to reply to the gateway's request information
        if(strcmp(cmdbuffer[bufindr], "c") == 0) 
        {
          printf("%s", STARTUP_FIRMWARE_MESSAGE);
          printf("%s", STARTUP_AUTHOR_MESSAGE);
          printf("%s", STARTUP_MACHINE_MESSAGE);
        }
        else process_commands();
      }
    }
#else
    if(Storage_Device == Storage_SD) process_commands();
#endif
#if USB_SUPPORT
    if(Storage_Device == Storage_USB)
    {
      if(USB_Reader.Saving)
      {
        if(strstr(cmdbuffer[bufindr], (const char*)("M29")) == NULL)
        {
          //usbcmd
          printf(MSG_OK);
        }
        else
        {
          StorageDevice_CloseFile();
          printf(MSG_FILE_SAVED);
        }
      }
      else process_commands();   
    }
#else
    if(Storage_Device == Storage_USB) process_commands();
#endif
    if(buflen > 0) buflen--;
    bufindr = (bufindr+1) % BUFSIZE;
  }
}

void enquecommand(const char *cmd)
{
  if(buflen < BUFSIZE)
  {
    //this is dangerous if a mixing of serial and this happsens
    strcpy(&(cmdbuffer[bufindw][0]), cmd);
    printf(SERIAL_ECHO_START);
    printf("Enqueing \"%s\"", cmdbuffer[bufindw]);
    bufindw = (bufindw+1) % BUFSIZE;
    buflen++;
  }
}

/*打印获取指令*/
void Printer_GetCommand(void)
{
  signed short int n;
  char time[30];
  if(buflen >= (BUFSIZE-1)) return;
  while(USART1_IsAvailable()>0 && buflen<BUFSIZE)
  {
    //LCD_ShowString(5,5,240,320,12, ".1.");
    serial_char = USART1_ReadData();
    //printf(" serial_char: %c\n\r",serial_char);
    if(serial_char=='\n' || serial_char=='\r' || (serial_char==':' && comment_mode==false) || serial_count>=(MAX_CMD_SIZE-1))//sanse 冒号
    {
      if(!serial_count)  //if empty line
      {
        comment_mode = false; //for new command
        return;
      }
      cmdbuffer[bufindw][serial_count] = 0; //terminate string
      if(!comment_mode)
      {
        comment_mode = false; //for new command
        fromsd[bufindw] = false;
        if(strchr(cmdbuffer[bufindw], 'N') != NULL)
        {
          strchr_pointer = strchr(cmdbuffer[bufindw], 'N');
          gcode_N = (strtol(&cmdbuffer[bufindw][strchr_pointer-cmdbuffer[bufindw]+1], NULL, 10));
          if(gcode_N!=gcode_LastN+1 && (strstr(cmdbuffer[bufindw], (const char*)("M110")) == NULL))
          {                                                                          
            printf(SERIAL_ERROR_START);
            printf(MSG_ERR_LINE_NO);
            printf("%ld", gcode_LastN);
            FlushSerialRequestResend();
            serial_count = 0;
            return;
          }
          if(strchr(cmdbuffer[bufindw], '*') != NULL)
          {
            unsigned char checksum = 0;
            unsigned char count = 0;
            while(cmdbuffer[bufindw][count] != '*') checksum = checksum ^ cmdbuffer[bufindw][count++];
            strchr_pointer = strchr(cmdbuffer[bufindw], '*');
            if((unsigned char)(strtod(&cmdbuffer[bufindw][strchr_pointer-cmdbuffer[bufindw]+1], NULL)) != checksum)
            {                                                                 
              printf(SERIAL_ERROR_START);
              printf(MSG_ERR_CHECKSUM_MISMATCH);
              printf(" checksum: %d\n\r",checksum);
              count = 0;
              printf(" '");
              while(cmdbuffer[bufindw][count] != '*') printf("%c", cmdbuffer[bufindw][count++]);
              printf(" '\n\r ");
              checksum = 0;
              count = 0;
              while(cmdbuffer[bufindw][count] != '*')
              { 
                printf("cmdbuffer:%d;", cmdbuffer[bufindw][count]);
                checksum = checksum ^ cmdbuffer[bufindw][count++];
                printf(" checksum:%d\n\r", checksum);
              }
              printf("%ld", gcode_LastN);
              FlushSerialRequestResend();
              serial_count = 0;
              return;
            }
            //if no errors, continue parsing
          }
          else
          {
            printf(SERIAL_ERROR_START);
            printf(MSG_ERR_NO_CHECKSUM);
            printf("%ld", gcode_LastN);
            FlushSerialRequestResend();
            serial_count = 0;
            return;
          }
          gcode_LastN = gcode_N;
          //if no errors, continue parsing
        }
        else  // if we don't receive 'N' but still see '*'
        {
          if((strchr(cmdbuffer[bufindw], '*') != NULL))                                                            
          {
            printf(SERIAL_ERROR_START);
            printf(MSG_ERR_NO_LINENUMBER_WITH_CHECKSUM);
            printf("%ld", gcode_LastN);
            serial_count = 0;
            return;
          }
        }
        if((strchr(cmdbuffer[bufindw], 'G') != NULL))                                                                
        {
          strchr_pointer = strchr(cmdbuffer[bufindw], 'G');
          switch((int)((strtod(&cmdbuffer[bufindw][strchr_pointer-cmdbuffer[bufindw]+1], NULL))))
          {
            case 0:
            case 1:
            case 2:
            case 3:
              if(Stopped == false) // If printer is stopped by an error the G[0-3] codes are ignored.
              {
              #if SD_SUPPORT
                if(SD_Reader.Saving && Storage_Device==Storage_SD) break;
              #endif
              #if USB_SUPPORT
                if(USB_Reader.Saving && Storage_Device==Storage_USB) break;
              #endif
                printf(MSG_OK);
              }
              else
              {
                // printf(MSG_ERR_STOPPED);
                // LCD_MESSAGEPGM(MSG_STOPPED);
              }
            break;
            default:
            break;
          }
        }
        bufindw = (bufindw+1) % BUFSIZE;
        buflen++;//sanse
      }
      serial_count = 0; //clear buffer
    }
    else
    {
      if(serial_char == ';') comment_mode = true;
      if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
    }
  }
#if SD_SUPPORT   //sanse
  if((Storage_Device==Storage_SD) && (!SD_Reader.Printing||serial_count!=0))
  {
    // LCD_ShowString(20,5,240,320,12, ".2.");
    return;
  }
#endif
#if USB_SUPPORT
  if((Storage_Device==Storage_USB) && (!USB_Reader.Printing||serial_count!=0))
  {
  //  LCD_ShowString(20,5,240,320,12, ".2.");
    return;
  }
#endif
#if SD_SUPPORT || USB_SUPPORT
  while(!StorageDevice_IsFileEnd() && buflen<BUFSIZE)
  {
    n = StorageDevice_ReadFile();
    serial_char = (BYTE)n;
    if(serial_char == '\n' || serial_char == '\r' || (serial_char == ':' && comment_mode == false) || serial_count >= (MAX_CMD_SIZE-1) || n==-1)
    {
      if(StorageDevice_IsFileEnd())//sanse
      {
        printf(MSG_FILE_PRINTED);
        Duration_Time = RTC_Get_DurationTime();
        sprintf(time, (const char*)("%i hours %i minutes"), Duration_Time.Hours, Duration_Time.Minutes);
        printf(SERIAL_ECHO_START);
        printf("%s", time);
        HMI_MemberStruct.Status = HMI_PRINT_FINISH; //LEO
        HMI_MemberStruct.RunTime_Flag = DISABLE;
        StorageDevice_FinishPrint();
      }
      if(!serial_count)
      {
        comment_mode = false; //for new command
        return; //if empty line
      }
      cmdbuffer[bufindw][serial_count] = 0; //terminate string
//      if(!comment_mode){
      fromsd[bufindw] = true;//sanse
      buflen++;
      bufindw = (bufindw+1) % BUFSIZE;
//      }
      comment_mode = false; //for new command
      serial_count = 0; //clear buffer
    }
    else
    {
      if(serial_char == ';') comment_mode = true;
      if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
    }
  }
#endif
}

float code_value(void)
{
  return (strtod(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL));
}

long code_value_long(void)
{
  return (strtol(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL, 10));
}

int code_value_int(void)
{
  return atoi(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1]);
}

bool code_seen(char code)
{
  strchr_pointer = strchr(cmdbuffer[bufindr], code);
  return (strchr_pointer != NULL);
}

void axis_is_at_home(int axis) 
{
  current_position[axis] = base_home_pos[axis] + add_homeing[axis];
  min_pos[axis] = base_min_pos[axis] + add_homeing[axis];
  max_pos[axis] = base_max_pos[axis] + add_homeing[axis];
}

#define HOMEAXIS_DO(LETTER) ((LETTER##_HOME_DIR==-1) || (LETTER##_HOME_DIR==1))

static void homeaxis(int axis)
{
  if(axis == X_AXIS?HOMEAXIS_DO(X) : axis==Y_AXIS?HOMEAXIS_DO(Y) : axis==Z_AXIS?HOMEAXIS_DO(Z) : 0)
  {
    current_position[axis] = 0;
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
    destination[axis] = 1.5 * max_length[axis] * home_dir[axis];
    feedrate = homing_feedrate[axis];
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
    Stepper_Synchronize();
    current_position[axis] = 0;
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
    destination[axis] = -home_retract_mm[axis] * home_dir[axis];
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
    Stepper_Synchronize();
    destination[axis] = 2 * home_retract_mm[axis] * home_dir[axis];
    feedrate = homing_feedrate[axis] / 2;
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
    Stepper_Synchronize();
    axis_is_at_home(axis);
    destination[axis] = current_position[axis];
    feedrate = 0.0;
    EndStops_Clear();
  }
}

#define HOMEAXIS(LETTER) homeaxis(LETTER##_AXIS)

void process_commands(void)
{
  unsigned long codenum; //throw away variable
  char *starpos = NULL;
  int8_t i;
  
  if(code_seen('G'))
  {
    switch((int)code_value())
    {
      case 0: // G0 -> G1
      case 1: // G1
        if(Stopped == false)
        {
          get_coordinates(); // For X Y Z E F
          prepare_move();
          //printf("\n");
          //ClearToSend();
          return;
        }
      //break;
      case 2: // G2  - CW ARC
        if(Stopped == false)
        {
          get_arc_coordinates();
          prepare_arc_move(true);
          return;
        }
      case 3: // G3  - CCW ARC
        if(Stopped == false)
        {
          get_arc_coordinates();
          // prepare_arc_move(false); LEO
          prepare_arc_move(true);
          return;
        }
      case 4: // G4 dwell
        // LCD_MESSAGEPGM(MSG_DWELL);
        codenum = 0;
        if(code_seen('P')) codenum = code_value(); // milliseconds to wait
        if(code_seen('S')) codenum = code_value() * 1000; // seconds to wait
        Stepper_Synchronize();
        codenum += HAL_GetTick();  // keep track of when we started waiting
        previous_millis_cmd = HAL_GetTick();
        while(HAL_GetTick() < codenum)
        {
          Temperature_Manage();
          manage_inactivity();
          // LEO
          // lcd_update();
        }
        break;
    #if FWRETRACT
      case 10: // G10 retract
        if(!retracted)
        {
          destination[X_AXIS] = current_position[X_AXIS];
          destination[Y_AXIS] = current_position[Y_AXIS];
          destination[Z_AXIS] = current_position[Z_AXIS];
          current_position[Z_AXIS] += -retract_zlift;
          destination[E0_AXIS] = current_position[E0_AXIS] - retract_length;
          feedrate = retract_feedrate;
          retracted = true;
          prepare_move();
        }
      break;
      case 11: // G10 retract_recover
        if(!retracted)
        {
          destination[X_AXIS]=current_position[X_AXIS];
          destination[Y_AXIS]=current_position[Y_AXIS];
          destination[Z_AXIS]=current_position[Z_AXIS];

          current_position[Z_AXIS]+=retract_zlift;
          current_position[E0_AXIS]+=-retract_recover_length;
          feedrate=retract_recover_feedrate;
          retracted=false;
          prepare_move();
        }
      break;
    #endif

      case 28: //G28 Home all Axis one at a time
        #ifdef HOST_KEEPALIVE_FEATURE
          set_busy_state(1);
        #endif
        saved_feedrate = feedrate;
        saved_feedmultiply = feedmultiply;
        feedmultiply = 100;
        HMI_ProcessFrame(); //LEO
        previous_millis_cmd = HAL_GetTick();
        EndStops_Enable(true);
        for(i=0; i < NUM_AXIS; i++) destination[i] = current_position[i];
        feedrate = 0.0;
        home_all_axis = !((code_seen(axis_codes[0])) || (code_seen(axis_codes[1])) || (code_seen(axis_codes[2])));
      #if Z_HOME_DIR > 0                      // If homing away from BED do Z first
        if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) HOMEAXIS(Z);
      #endif
      #ifdef QUICK_HOME
        if((home_all_axis)||( code_seen(axis_codes[X_AXIS]) && code_seen(axis_codes[Y_AXIS])) )  //first diagonal move
        {
          current_position[X_AXIS] = 0;
          current_position[Y_AXIS] = 0;
          plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
          destination[X_AXIS] = 1.5 * X_MAX_LENGTH * X_HOME_DIR;destination[Y_AXIS] = 1.5 * Y_MAX_LENGTH * Y_HOME_DIR;
          feedrate = homing_feedrate[X_AXIS];
          if(homing_feedrate[Y_AXIS]<feedrate) feedrate =homing_feedrate[Y_AXIS];
          plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
          Stepper_Synchronize();
          axis_is_at_home(X_AXIS);
          axis_is_at_home(Y_AXIS);
          plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
          destination[X_AXIS] = current_position[X_AXIS];
          destination[Y_AXIS] = current_position[Y_AXIS];
          plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
          feedrate = 0.0;
          Stepper_Synchronize();
          EndStops_Clear();
        }
      #endif
        if((home_all_axis) || (code_seen(axis_codes[X_AXIS]))) HOMEAXIS(X);
        if((home_all_axis) || (code_seen(axis_codes[Y_AXIS]))) HOMEAXIS(Y);
      #if Z_HOME_DIR < 0                      // If homing towards BED do Z last
        if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) HOMEAXIS(Z);
      #endif
        if(code_seen(axis_codes[X_AXIS]))
        {
          if(code_value_long() != 0) current_position[X_AXIS] = code_value() + add_homeing[0];
        }
        if(code_seen(axis_codes[Y_AXIS]))
        {
          if(code_value_long() != 0) current_position[Y_AXIS] = code_value() + add_homeing[1];
        }
        if(code_seen(axis_codes[Z_AXIS]))
        {
          if(code_value_long() != 0) current_position[Z_AXIS] = code_value() + add_homeing[2];
        }
        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
      #ifdef ENDSTOPS_ONLY_FOR_HOMING
        EndStops_Enable(false);
      #endif
        feedrate = saved_feedrate;
        feedmultiply = saved_feedmultiply;
        previous_millis_cmd = HAL_GetTick();
        EndStops_Clear();
        if(HMI_MemberStruct.Status == HMI_AXIS_HOME) HMI_MemberStruct.Status = HMI_READY;
      break;
      case 90: // G90
        relative_mode = false;
      break;
      case 91: // G91
        relative_mode = true;
      break;
      case 92: // G92
        if(!code_seen(axis_codes[E0_AXIS])) Stepper_Synchronize();
        for(i=0; i<NUM_AXIS; i++)
        {
          if(code_seen(axis_codes[i]))
          {
            if(i == E0_AXIS)
            {
             current_position[i] = code_value();
             plan_set_e_position(current_position[E0_AXIS]);
            }
            else
            {
              current_position[i] = code_value() + add_homeing[i];
              plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
            }
          }
        }
      break;
    }
  }
  else if(code_seen('M'))
  {
    switch((int)code_value())
    {
      case 0: // M0 - Unconditional stop - Wait for user button press on LCD
      case 1: // M1 - Conditional stop - Wait for user button press on LCD
      {
      #ifdef ULTIPANEL
         // LCD_MESSAGEPGM(MSG_USERWAIT);  /////////////////////////////////////////////////
        codenum = 0;
        if(code_seen('P')) codenum = code_value(); // milliseconds to wait
        if(code_seen('S')) codenum = code_value() * 1000; // seconds to wait
        Stepper_Synchronize();
        previous_millis_cmd = millis();
        if(codenum > 0)
        {
          codenum += millis();  // keep track of when we started waiting
          while(millis()<codenum && !LCD_CLICKED)
          {
            Temperature_Manage();
            manage_inactivity();
            lcd_update();   ///////////////////////////////////
          }
        }
        else
        {
          while(!LCD_CLICKED)
          {
            Temperature_Manage();
            manage_inactivity();
            lcd_update();
          }
        }
     //   LCD_MESSAGEPGM(MSG_RESUMING);
      #endif
      }
      break;
      case 15:// M15  - update flash data (font data ; icon data and so on)
//LEO            LCD_Clear(WHITE);
//            if(update_font(5,0,12,0)==0)
//            {  LCD_ShowString(5,80,200,200,12, "UPDATE SUCCESS!");
//              delay_ms(500);
//            }
      break;
      case 16:// M16  - screen_adjust
//LEO            LCD_Clear(WHITE);
//            TP_Adjust();
//            //TP_Save_Adjdata();
//            printf("Adjust OK\r\n");
      break;
      case 17:
         // LCD_MESSAGEPGM(MSG_NO_MOVE);
        Stepper_EnPin_Control(X_AXIS, ENABLE);
        Stepper_EnPin_Control(Y_AXIS, ENABLE);
        Stepper_EnPin_Control(Z_AXIS, ENABLE);
        Stepper_EnPin_Control(E0_AXIS, ENABLE);
        Stepper_EnPin_Control(E1_AXIS, ENABLE);
      break;
    #if SD_SUPPORT || USB_SUPPORT
      case 20:
        printf(MSG_BEGIN_FILE_LIST);
        StorageDevice_ListFiles();
        printf(MSG_END_FILE_LIST);
      break;
      case 21:   
        StorageDevice_Mount();
      break;
      case 22: 
        StorageDevice_Release();
      break;
      case 23:
        //printf("--%s\n",strchr_pointer);
        starpos = (strchr(strchr_pointer + 4, '*'));
        if(starpos!=NULL) *(starpos-1) = '\0';
        //printf("--%s\n",strchr_pointer + 4);
        StorageDevice_OpenFile(strchr_pointer+4, true);
      break;
      case 24:
        StorageDevice_StartPrint();
        RTC_SetTime(0, 0, 0);
      break;
      case 25:
        StorageDevice_PausePrint();
      break;
      case 26:
      #if SD_SUPPORT
        if(Storage_Device == Storage_SD)
        {
          if(SD_Reader.Ready && code_seen('S')) StorageDevice_SetPointer(code_value_long());
        }
      #endif
      #if USB_SUPPORT
        if(Storage_Device == Storage_USB)
        {
          if(USB_Reader.Ready && code_seen('S')) StorageDevice_SetPointer(code_value_long());
        }
      #endif
      break;
      case 27: 
        StorageDevice_GetProgress();
      break;
      case 28:
        starpos = (strchr(strchr_pointer + 4, '*'));
        if(starpos != NULL)
        {
          char* npos = strchr(cmdbuffer[bufindr], 'N');
          strchr_pointer = strchr(npos, ' ') + 1;
          *(starpos-1) = '\0';
        }
        StorageDevice_OpenFile(strchr_pointer+4,false);
      break;
      case 29:
          //processed in write to file routine above
          //card,saving = false;
      break;
      case 30:
      #if SD_SUPPORT
        if(Storage_Device==Storage_SD && SD_Reader.Ready)
        {
          StorageDevice_CloseFile();
          starpos = (strchr(strchr_pointer + 4, '*'));
          if(starpos != NULL)
          {
            char *npos = strchr(cmdbuffer[bufindr], 'N');
            strchr_pointer = strchr(npos, ' ') + 1;
            *(starpos-1) = '\0';
          }
          StorageDevice_DeleteFile(strchr_pointer + 4);
        }
      #endif
      #if USB_SUPPORT
        if(Storage_Device==Storage_USB && USB_Reader.Ready)
        {
          StorageDevice_CloseFile();
          starpos = (strchr(strchr_pointer + 4, '*'));
          if(starpos != NULL)
          {
            char *npos = strchr(cmdbuffer[bufindr], 'N');
            strchr_pointer = strchr(npos, ' ') + 1;
            *(starpos-1) = '\0';
          }
          StorageDevice_DeleteFile(strchr_pointer + 4);
        }
      #endif
      break;
    #endif
      case 31: 
      {
        Duration_Time = RTC_Get_DurationTime();
        printf(SERIAL_ECHO_START);
        // tangbing
        printf("Print time: %dm %ds\n", Duration_Time.Hours*60 + Duration_Time.Minutes, Duration_Time.Seconds);
        // printf("%d hour, %d min, %d sec\n", Duration_Time.Hours, Duration_Time.Minutes, Duration_Time.Seconds);
        // printf("%s\n",time);
        // lcd_setstatus(time);
        Heater_AutoShutdown();
      }
      break;
      case 42: break;
      case 104: // M104
        if(setTargetedHotend(104)) break;
        if(code_seen('S')) Heater_SetTargetCelsius(code_value(), (Heater_TypeDef)tmp_extruder);
        printf(SERIAL_ECHO_START);
        printf("Set Head(%d) T:%.1f\n", tmp_extruder, Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder));
        Heater_SetWatch();
        break;
      case 140:
        if(code_seen('S')) Heater_SetTargetCelsius(code_value(), Heater_Bed);
        printf(SERIAL_ECHO_START);
        printf("Set Bed T:%.1f\n", Heater_GetTargetCelsius(Heater_Bed));
        break;
      case 105: 
        if(setTargetedHotend(105)) break;
      #if TEMP_HEAD_0_USED
      //ok T:100.09 /100.00 B:40.99 /40.00 @:26 B@:0
      //tangbing
        printf("ok T:%.2f /%.2f B:0.00 /0.00 @:26 B@:0\n", Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder), Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder));
      //printf(SERIAL_ECHO_START);
      //printf("Get Head(%d) T:%.1f/%.1f\n", tmp_extruder, Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder), Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder));
      #if TEMP_BED_USED
        printf(" B:%.1f /%.1f", Heater_GetCurrentCelsius(Heater_Bed), Heater_GetTargetCelsius(Heater_Bed));
      #endif
      #else
        printf(SERIAL_ERROR_START);
        printf(MSG_ERR_NO_THERMISTORS);
      #endif
        return;
      case 109:
      {
        bool target_direction;
        long residencyStart;

        if(setTargetedHotend(109)) break;
      #ifdef AUTOTEMP
        autotemp_enabled = false;
      #endif
        if(code_seen('S')) Heater_SetTargetCelsius(code_value(), (Heater_TypeDef)tmp_extruder);
      #ifdef AUTOTEMP
        if(code_seen('S')) autotemp_min = code_value();
        if(code_seen('B')) autotemp_max = code_value();  
        if(code_seen('F'))
        {
          autotemp_factor = code_value();
          autotemp_enabled = true;
        }
      #endif
      #if KEYPAD_SUPPORT
        KeyPad_Enable(DISABLE);
      #endif
      #if REMOTE_SUPPORT
        Remote_Enable(DISABLE);
      #endif

      #ifdef HOST_KEEPALIVE_FEATURE
        set_busy_state(1);
      #endif

        Heater_SetWatch();
        codenum = HAL_GetTick();
        /* See if we are heating up or cooling down */
        target_direction = Heater_IsHeating((Heater_TypeDef)tmp_extruder); // true if heating, false if cooling
      #ifdef TEMP_RESIDENCY_TIME
        residencyStart = -1;
        /* continue to loop until we have reached the target temp _and_ until TEMP_RESIDENCY_TIME hasn't passed since we reached it */
        while((residencyStart == -1) || (residencyStart >= 0 && (((unsigned int) (HAL_GetTick() - residencyStart)) < (TEMP_RESIDENCY_TIME * 1000UL))) ) 
        {
          if((HAL_GetTick()-codenum) > 1000UL)
          {
            // Print Temp Reading and remaining time every 1 second while heating up/cooling down
            // printf("T:%.1f E:%d", Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder), tmp_extruder);
            // There is no hot bed, set to 0
            //tangbing
            printf("T:%.2f /%.2f B:0.00 /0.00 @:0 B@:0", Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder), Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder));
          #ifdef TEMP_RESIDENCY_TIME
            printf(" W:");
            if(residencyStart > -1)
            {
              codenum = ((TEMP_RESIDENCY_TIME * 1000UL) - (HAL_GetTick() - residencyStart)) / 1000UL;
              printf("%ld\n", codenum );
            }
            else printf("?\n");
          #else
            printf("\n");
          #endif
            codenum = HAL_GetTick();
          }
          Temperature_Manage();
          manage_inactivity();
          HMI_ProcessFrame(); //LEO
        #ifdef TEMP_RESIDENCY_TIME
          /* start/restart the TEMP_RESIDENCY_TIME timer whenever we reach target temp for the first time or when current temp falls outside the hysteresis after target temp was reached */
          if((residencyStart == -1 &&  target_direction && (Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder) >= (Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder)-TEMP_WINDOW))) ||
              (residencyStart == -1 && !target_direction && (Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder) <= (Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder)+TEMP_WINDOW))) ||
              (residencyStart > -1 && labs(Heater_GetCurrentCelsius((Heater_TypeDef)tmp_extruder) - Heater_GetTargetCelsius((Heater_TypeDef)tmp_extruder)) > TEMP_HYSTERESIS) )
          {
            residencyStart = HAL_GetTick();
          }
        #endif
        }
      #else
        while(target_direction ? (Heater_IsHeating(tmp_extruder)) : (Heater_IsCooling(tmp_extruder)&&(CooldownNoWait==false)))
        {
          if((millis()-codenum) > 1000UL)
          {
            //Print Temp Reading and remaining time every 1 second while heating up/cooling down
            printf("T:%.1f E:%d", Heater_GetCurrentCelsius(tmp_extruder), tmp_extruder);
            #ifdef TEMP_RESIDENCY_TIME
            printf(" W:");
            if(residencyStart > -1)
            {
              codenum = ((TEMP_RESIDENCY_TIME * 1000UL) - (millis() - residencyStart)) / 1000UL;
              printf("%ld\n", codenum );
            }
            else printf("?\n");
          #else
            printf("\n");
          #endif
            codenum = millis();
          }
          Temperature_Manage();
          manage_inactivity();
          // lcd_update();
          #ifdef TEMP_RESIDENCY_TIME
                /* start/restart the TEMP_RESIDENCY_TIME timer whenever we reach target temp for the first time
                  or when current temp falls outside the hysteresis after target temp was reached */
          if((residencyStart == -1 &&  target_direction && (Heater_GetCurrentCelsius(tmp_extruder) >= (Heater_GetTargetCelsius(tmp_extruder)-TEMP_WINDOW))) ||
              (residencyStart == -1 && !target_direction && (Heater_GetCurrentCelsius(tmp_extruder) <= (Heater_GetTargetCelsius(tmp_extruder)+TEMP_WINDOW))) ||
              (residencyStart > -1 && labs(Heater_GetCurrentCelsius(tmp_extruder) - Heater_GetTargetCelsius(tmp_extruder)) > TEMP_HYSTERESIS) )
          {
            residencyStart = millis();
          }
        #endif
        }
      #endif
        if(SD_Reader.Printing == true)//LEO
        {
          RTC_SetTime(0, 0, 0);
          HMI_MemberStruct.RunTime_Flag = ENABLE;
          HMI_MemberStruct.Status = HMI_PRINTING;
        }
      #if KEYPAD_SUPPORT
        KeyPad_Enable(ENABLE);
      #endif
      #if REMOTE_SUPPORT
        Remote_Enable(ENABLE);
      #endif
        previous_millis_cmd = HAL_GetTick();
      }
      break;
      case 190: // M190 - Wait for bed heater to reach target.
      #if TEMP_BED_USED
        // LCD_MESSAGEPGM(MSG_BED_HEATING);
        if(code_seen('S')) Heater_SetTargetCelsius(code_value(), Heater_Bed);
        #ifdef HOST_KEEPALIVE_FEATURE
          set_busy_state(1);
        #endif
        codenum = HAL_GetTick();
        while(Heater_IsHeating(Heater_Bed))
        {
          if((HAL_GetTick()-codenum) > 1000 ) //Print Temp Reading every 1 second while heating up.
          {
            float tt = Heater_GetCurrentCelsius(active_extruder);
            printf("T:%.1f E:%d B:%.1f\n", tt, active_extruder, Heater_GetCurrentCelsius(Heater_Bed));
            codenum = HAL_GetTick();
          }
          Temperature_Manage();
          manage_inactivity();
          //LEO              lcd_update(); /////////////////////////////////////////////////
        }
        // LCD_MESSAGEPGM(MSG_BED_DONE);     //////////////////////////////////////////////////
        previous_millis_cmd = HAL_GetTick();
      #endif
      break;
      case 106: //M106 Fan On
      #if FAN_MODEL_0_USED
        if(code_seen('S')) fanSpeed = constrain(code_value(), 0, 255);
        else fanSpeed = 255;
      #endif
      break;
      case 107: //M107 Fan Off
        fanSpeed = 0;
        //test(0);
        //soft_pwm[0]=0;
      break;
      case 126: //M126 valve open
      #ifdef BARICUDA
        //PWM for HEATER_HEAD_1_USED
      #if HEATER_HEAD_1_USED
        if(code_seen('S')) ValvePressure = constrain(code_value(), 0, 255);
        else ValvePressure = 255;
      #endif
      #endif
      break;
      case 127: //M127 valve closed
      #ifdef BARICUDA
        //PWM for HEATER_HEAD_1_USED
      #if HEATER_HEAD_1_USED
        ValvePressure = 0;
      #endif
      #endif
      break;
      case 128://M128 valve open
      #ifdef BARICUDA
        //PWM for HEATER_HEAD_2_USED
      #if HEATER_HEAD_2_USED
        if(code_seen('S')) EtoPPressure=constrain(code_value(), 0, 255);
        else EtoPPressure = 255;
      #endif
      #endif
      break;
      case 129: //M129 valve closed
      #ifdef BARICUDA
        // PWM for HEATER_HEAD_2_USED
      #if HEATER_HEAD_2_USED
        EtoPPressure = 0;
      #endif
      #endif
      break;
      case 80: // M80 - ATX Power On
           // SET_OUTPUT(PS_ON_PIN); //GND  ////////////////////////////////////////////////////
           //  WRITE(PS_ON_PIN, PS_ON_AWAKE);////////////////////////////////////////////////////
      break;
      case 81: // M81 - ATX Power Off
       break;
      case 82:
        axis_relative_modes[3] = false;
      break;
      case 83:
        axis_relative_modes[3] = true;
      break;
      case 18: //compatibility
      case 84: // M84
        if(code_seen('S')) stepper_inactive_time = code_value() * 1000;
        else
        {
          bool all_axis = !((code_seen(axis_codes[0])) || (code_seen(axis_codes[1])) || (code_seen(axis_codes[2]))|| (code_seen(axis_codes[3])));
          if(all_axis)
          {
            Stepper_Synchronize();
            Stepper_EnPin_Control(E0_AXIS, DISABLE);
            Stepper_EnPin_Control(E1_AXIS, DISABLE);
            //  disable_e2();
            Stepper_Finish_Disable();
          }
          else
          {
            Stepper_Synchronize();
            if(code_seen('X')) Stepper_EnPin_Control(X_AXIS, DISABLE);
            if(code_seen('Y')) Stepper_EnPin_Control(Y_AXIS, DISABLE);
            if(code_seen('Z')) Stepper_EnPin_Control(Z_AXIS, DISABLE);
           // #if ((E0_ENABLE_PIN != X_ENABLE_PIN) && (E1_ENABLE_PIN != Y_ENABLE_PIN)) // Only enable on boards that have seperate ENABLE_PINS
            if(code_seen('E'))
            {
              Stepper_EnPin_Control(E0_AXIS, DISABLE);
              Stepper_EnPin_Control(E1_AXIS, DISABLE);
            //  disable_e2();
            }
          //  #endif
          }
        }
      break;
      case 85: // M85
        code_seen('S');
        max_inactive_time = code_value() * 1000;
      break;
      case 92: // M92
        for(i=0; i<NUM_AXIS; i++)
        {
          if(code_seen(axis_codes[i]))
          {
            if(i == 3) // E
            {
              float value = code_value();
              if(value < 20.0)
              {
                float factor = axis_steps_per_unit[i] / value; // increase e constants if M92 E14 is given for netfab.
                max_e_jerk *= factor;
                max_feedrate[i] *= factor;
                axis_steps_per_sqr_second[i] *= factor;
              }
              axis_steps_per_unit[i] = value;
            }
            else axis_steps_per_unit[i] = code_value();
          }
        }
      break;
      case 115: // M115
        printf(MSG_M115_REPORT);
      break;
      case 117: // M117 display message    /////////////////////////////////////////////////////
        starpos = (strchr(strchr_pointer+5,'*'));
        if(starpos!=NULL) *(starpos-1)='\0';
        //lcd_setstatus(strchr_pointer + 5);   //////////////////////////////////////////////////
      break;
      case 114: // M114   
        printf("X:%.2f Y:%.2f Z:%.2f E:%.2f ", current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
        printf("Count X:%d Y:%d Z:%d\n", (int)(Stepper_GetPosition(X_AXIS)/axis_steps_per_unit[X_AXIS]),(int)(Stepper_GetPosition(Y_AXIS)/axis_steps_per_unit[Y_AXIS]),(int)(Stepper_GetPosition(Z_AXIS)/axis_steps_per_unit[Z_AXIS]));
        //printf(MSG_COUNT_X);
        //printf("%f Y:%f Z:%f\n",((float)Stepper_GetPosition(X_AXIS))/axis_steps_per_unit[X_AXIS],((float)Stepper_GetPosition(Y_AXIS))/axis_steps_per_unit[Y_AXIS],((float)Stepper_GetPosition(Z_AXIS))/axis_steps_per_unit[Z_AXIS]);
      break; 
      case 120: // M120
        EndStops_Enable(false);
      break;
      case 121: // M121
        EndStops_Enable(true);
      break;
      case 119: // M119
        printf(MSG_M119_REPORT);
        printf("\n");
      #if X_MIN_PIN_USED
        printf(MSG_X_MIN);
        printf(((EndStops_CheckState(X_AXIS, MIN_POSITION) != X_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      #if X_MAX_PIN_USED
        printf(MSG_X_MAX);
        printf(((EndStops_CheckState(X_AXIS, MAX_POSITION) != X_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      #if Y_MIN_PIN_USED
        printf(MSG_Y_MIN);
        printf(((EndStops_CheckState(Y_AXIS, MIN_POSITION) != Y_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      #if Y_MAX_PIN_USED
        printf(MSG_Y_MAX);
        printf(((EndStops_CheckState(Y_AXIS, MAX_POSITION) != Y_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      #if Z_MIN_PIN_USED
        printf(MSG_Z_MIN);
        printf(((EndStops_CheckState(Z_AXIS, MIN_POSITION) != Z_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      #if Z_MAX_PIN_USED
        printf(MSG_Z_MAX);
        printf(((EndStops_CheckState(Z_AXIS, MAX_POSITION) != Z_ENDSTOPS_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
        printf("\n");
      #endif
      break;
        //TODO: update for all axis, use for loop
      case 201: // M201   
        for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) max_acceleration_units_per_sq_second[i] = code_value();
        // steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
        reset_acceleration_rates();
      break;
      case 202: // M202
      #if 0 // Not used for Sprinter/grbl gen6
        for(i=0; i < NUM_AXIS; i++) if(code_seen(axis_codes[i])) axis_travel_steps_per_sqr_second[i] = code_value() * axis_steps_per_unit[i];
      #endif
      break;
      case 203: // M203 max feedrate mm/sec
        for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) max_feedrate[i] = code_value();
      break; 
      case 204: // M204 acclereration S normal moves T filmanent only moves
      {
        if(code_seen('S')) acceleration = code_value();
        if(code_seen('T')) retract_acceleration = code_value();
      }
      break;
      case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
      {
        if(code_seen('S')) minimumfeedrate = code_value();
        if(code_seen('T')) mintravelfeedrate = code_value();
        if(code_seen('B')) minsegmenttime = code_value();
        if(code_seen('X')) max_xy_jerk = code_value();
        if(code_seen('Z')) max_z_jerk = code_value();
        if(code_seen('E')) max_e_jerk = code_value();
      }
      break;
      case 206: // M206 additional homeing offset
        for(i=0; i<3; i++) if(code_seen(axis_codes[i])) add_homeing[i] = code_value();
      break;
    #if FWRETRACT
      case 207: //M207 - set retract length S[positive mm] F[feedrate mm/sec] Z[additional zlift/hop]
      {
        if(code_seen('S')) retract_length = code_value();
        if(code_seen('F')) retract_feedrate = code_value();
        if(code_seen('Z')) retract_zlift = code_value();
      }
      break;
      case 208: // M208 - set retract recover length S[positive mm surplus to the M207 S*] F[feedrate mm/sec]
      {
        if(code_seen('S')) retract_recover_length = code_value();
        if(code_seen('F')) retract_recover_feedrate = code_value();
      }
      break;
      case 209: // M209 - S<1=true/0=false> enable automatic retract detect if the slicer did not support G10/11: every normal extrude-only move will be classified as retract depending on the direction.
      {
        if(code_seen('S'))
        {
          int t = code_value();
          switch(t)
          {
            case 0: autoretract_enabled = false; retracted = false; break;
            case 1: autoretract_enabled = true; retracted = false; break;
            default:
              printf(SERIAL_ECHO_START);
              printf(MSG_UNKNOWN_COMMAND);
              printf("%d", cmdbuffer[bufindr]);
              printf("\"");
          }
        }
      }
      break;
    #endif
    #if EXTRUDERS > 1
      case 218: // M218 - set hotend offset (in mm), T<extruder_number> X<offset_on_X> Y<offset_on_Y>
      {
        if(setTargetedHotend(218)) break;
        if(code_seen('X')) extruder_offset[X_AXIS][tmp_extruder] = code_value();
        if(code_seen('Y')) extruder_offset[Y_AXIS][tmp_extruder] = code_value();
        printf(SERIAL_ECHO_START);
        printf(MSG_HOTEND_OFFSET);
        for(tmp_extruder=0; tmp_extruder<EXTRUDERS; tmp_extruder++)
        {
          //SERIAL_ECHO(" ");
          printf(" %f,%f", extruder_offset[X_AXIS][tmp_extruder], extruder_offset[Y_AXIS][tmp_extruder]);
          //SERIAL_ECHO(",");
          //SERIAL_ECHO(extruder_offset[Y_AXIS][tmp_extruder]);
        }
        printf("\n");
      }
      break;
    #endif
      case 220: // M220 S<factor in percent>- set speed factor override percentage
      {
        if(code_seen('S')) feedmultiply = code_value();
      }
      break;
      case 221: // M221 S<factor in percent>- set extrude factor override percentage
      {
        if(code_seen('S')) extrudemultiply = code_value();
      }
      break;
    #if NUM_SERVOS > 0
      case 280: // M280 - set servo position absolute. P: servo index, S: angle or microseconds
      {
        int servo_index = -1;
        int servo_position = 0;
        if(code_seen('P')) servo_index = code_value();
        if(code_seen('S'))
        {
          servo_position = code_value();
          if((servo_index>=0) && (servo_index<NUM_SERVOS)) servos[servo_index].write(servo_position);
        }
      }
      break;
    #endif
       // #if LARGE_FLASH == true && ( BEEPER > 0 || defined(ULTRALCD) )
      case 300: // M300
      {
      //  int beepS = 400;
      //          int beepP = 1000;
      //if(code_seen('S')) beepS = code_value();
      //          if(code_seen('P')) beepP = code_value();
      //   #if BEEPER > 0
      //            Beep_Control(ENABLE);
      //            EasyDelay_Ms(beepP);
      //            Beep_Control(DISABLE);
      //   #elif defined(ULTRALCD)
      //    lcd_buzz(beepS, beepP);
      //  #endif
      }
      break;
     //   #endif
    #ifdef PIDTEMP
      case 301: // M301
      {
        if(code_seen('P')) Kp = code_value();
        if(code_seen('I')) Ki = Temperature_ScalePID(0, code_value(), 0);
        if(code_seen('D')) Kd = Temperature_ScalePID(1, code_value(), 0);
      #ifdef PID_ADD_EXTRUSION_RATE
        if(code_seen('C')) Kc = code_value();
      #endif
        Heater_SetPID();
        printf(MSG_OK);
        printf(" p:%f i:%f d:%f", Kp, Temperature_ScalePID(0, Ki, 1), Temperature_ScalePID(1, Kd, 1));
      #ifdef PID_ADD_EXTRUSION_RATE
        //Kc does not have scaling applied above, or in resetting defaults
        printf(" c:%f", Kc);
        //  SERIAL_PROTOCOL(Kc);
      #endif
        printf("\n");
      }
      break;
    #endif
    #ifdef PIDTEMPBED
      case 304: // M304
      {
        if(code_seen('P')) bedKp = code_value();
        if(code_seen('I')) bedKi = Temperature_ScalePID(0, code_value(), 0);
        if(code_seen('D')) bedKd = Temperature_ScalePID(1, code_value(), 0);
        Heater_SetPID();
        printf(MSG_OK);
        printf(" p:%f i:%f d:%f", Kp, Temperature_ScalePID(0, bedKi, 1), Temperature_ScalePID(1, bedKd, 1));
        printf("\n");
      }
      break;
    #endif
      case 240: // M240  Triggers a camera by emulating a Canon RC-1 : http://www.doc-diy.net/photo/rc-1_hacked/
      {
      #ifdef PHOTOGRAPH_PIN
      #endif
      }
      break;
      case 302: // allow cold extrudes
      {
        allow_cold_extrudes(true);
      }
      break;
      case 303: // M303 PID autotune
      {
        float temp = 150.0;
        int e=0;
        int c=5;
        if(code_seen('E')) e = code_value();
        if(e < 0) temp = 70;
        if(code_seen('S')) temp = code_value();
        if(code_seen('C')) c = code_value();
        Heater_PID_AutoTune(temp, e, c);
      }
      break;
      case 400: // M400 finish all moves
      {
        Stepper_Synchronize();
      }
      break;
      case 500: // M500 Store settings in EEPROM
      {
        //Config_StoreSettings();////////////////////////////////////////////////////////////////////////////////////////
      }
      break;
      case 501: // M501 Read settings from EEPROM
      {
        //Config_RetrieveSettings();////////////////////////////////////////////////////////////////////////////////////
      }
      break;
      case 502: // M502 Revert to default settings
      {
        //Config_ResetDefault();//////////////////////////////////////////////////////////////////////////////////////////
      }
      break;
      case 503: // M503 print settings currently in memory
      {
        Config_PrintSettings();
      }
      break;
    #ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED
      case 540:
      {
        if(code_seen('S')) abort_on_endstop_hit = code_value() > 0;
      }
      break;
    #endif
    #ifdef FILAMENTCHANGEENABLE
      case 600: //Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
      {
        float target[4];
        float lastpos[4];
        target[X_AXIS] = current_position[X_AXIS];
        target[Y_AXIS] = current_position[Y_AXIS];
        target[Z_AXIS] = current_position[Z_AXIS];
        target[E0_AXIS] = current_position[E0_AXIS];
        lastpos[X_AXIS] = current_position[X_AXIS];
        lastpos[Y_AXIS] = current_position[Y_AXIS];
        lastpos[Z_AXIS] = current_position[Z_AXIS];
        lastpos[E0_AXIS] = current_position[E0_AXIS];
        //retract by E
        if(code_seen('E')) target[E0_AXIS] += code_value();
        else
        {
        #ifdef FILAMENTCHANGE_FIRSTRETRACT
          target[E0_AXIS] += FILAMENTCHANGE_FIRSTRETRACT;
        #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder);
        //lift Z
        if(code_seen('Z')) target[Z_AXIS] += code_value();
        else
        {
        #ifdef FILAMENTCHANGE_ZADD
          target[Z_AXIS] += FILAMENTCHANGE_ZADD;
        #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder);
        //move xy
        if(code_seen('X')) target[X_AXIS] += code_value();
        else
        {
        #ifdef FILAMENTCHANGE_XPOS
          target[X_AXIS] = FILAMENTCHANGE_XPOS;
        #endif
        }
        if(code_seen('Y')) target[Y_AXIS] = code_value();
        else
        {
        #ifdef FILAMENTCHANGE_YPOS
          target[Y_AXIS] = FILAMENTCHANGE_YPOS;
        #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder);
        if(code_seen('L')) target[E0_AXIS] += code_value();
        else
        {
        #ifdef FILAMENTCHANGE_FINALRETRACT
          target[E0_AXIS] += FILAMENTCHANGE_FINALRETRACT;
        #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder);
        //finish moves
        Stepper_Synchronize();
        //disable extruder steppers so filament can be removed
        Stepper_EnPin_Control(E0_AXIS, ENABLE);
        Stepper_EnPin_Control(E1_AXIS, ENABLE);
        Stepper_EnPin_Control(E2_AXIS, ENABLE);
        delay_ms(100);
        cnt = 0;
        while(!lcd_clicked)
        {
          cnt++;
          Temperature_Manage();
          manage_inactivity();
          lcd_update();
          if(cnt==0) beep();
        }
        //return to normal
        if(code_seen('L')) target[E0_AXIS] += -code_value();
        else
        {
        #ifdef FILAMENTCHANGE_FINALRETRACT
          target[E0_AXIS] += (-1)*FILAMENTCHANGE_FINALRETRACT;
        #endif
        }
        current_position[E0_AXIS] = target[E0_AXIS]; //the long retract of L is compensated by manual filament feeding
        plan_set_e_position(current_position[E0_AXIS]);
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder); //should do nothing
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder); //move xy back
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], target[E0_AXIS], feedrate/60, active_extruder); //move z back
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], lastpos[E0_AXIS], feedrate/60, active_extruder); //final untretract
      }
      break;
    #endif
      case 907: // M907 Set digital trimpot motor current using axis codes.
      {
//LEO          for(i=0;i<NUM_AXIS;i++) if(code_seen(axis_codes[i])) digipot_current(i, code_value());
//          if(code_seen('B')) digipot_current(4,code_value());
//          if(code_seen('S')) for(i=0;i<=4;i++) digipot_current(i,code_value());
      }
      break;
      case 908: // M908 Control digital trimpot directly.
      {
//LEO          uint8_t channel,current;
//          if(code_seen('P')) channel=code_value();
//          if(code_seen('S')) current=code_value();
//          digipot_current(channel, current);
      }
      break;
      case 350: // M350 Set microstepping mode. Warning: Steps per unit remains unchanged. S code sets stepping mode for all drivers.
      { 
        if(code_seen('S')) for( i=0; i<=4; i++) microstep_mode(i, code_value());
        for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) microstep_mode(i, (uint8_t)code_value());
        if(code_seen('B')) microstep_mode(4, code_value());
        microstep_readings();
      }
      break;
      case 351: // M351 Toggle MS1 MS2 pins directly, S# determines MS1 or MS2, X# sets the pin high/low.
      {
        if(code_seen('S'))
        {
          switch((int)code_value())
          {
            case 1:
              for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) microstep_ms(i, code_value(), -1, -1);
              if(code_seen('B')) microstep_ms(4, code_value(), -1, -1);
            break;
            case 2:
              for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) microstep_ms(i, -1, code_value(), -1);
              if(code_seen('B')) microstep_ms(4, -1, code_value(), -1);
            break;
            case 3:
              for(i=0; i<NUM_AXIS; i++) if(code_seen(axis_codes[i])) microstep_ms(i, -1, -1, code_value());
              if(code_seen('B')) microstep_ms(4, -1, -1, code_value());
            break;
          }
        }
        microstep_readings();
      }
      break;
#ifdef TEMPRATURE_AUTO_REPORT
      case 155:  //设置定时上报温度间隔tangbing
      {
        if(code_seen('S'))
        {
          int param = code_value_int();
          if(param > 50) param = 50;
          if(param < 0)  param = 0;
          set_temp_report_interval((uint8_t)param);
        }
      }
      break;
#endif
      case 999: // M999: Restart after being stopped
        Stopped = false;
        //lcd_reset_alert_level();//////////////////////////////////////
        gcode_LastN = Stopped_gcode_LastN;
        FlushSerialRequestResend();
      break;
    }//end switch(int) code_value();
  }
  else if(code_seen('T'))
  {
    tmp_extruder = code_value();
    if(tmp_extruder >= EXTRUDERS)
    {
      printf(SERIAL_ECHO_START);
      printf("T%d",tmp_extruder);
      //SERIAL_ECHO(tmp_extruder);
      printf(MSG_INVALID_EXTRUDER);
    }
    else 
    {
      volatile bool make_move = false;
      if(code_seen('F')) 
      {
        make_move = true;
        next_feedrate = code_value();
        if(next_feedrate > 0.0) feedrate = next_feedrate;
      }
    #if EXTRUDERS > 1
      if(tmp_extruder != active_extruder) 
      {
        // Save current position to return to after applying extruder offset
        memcpy(destination, current_position, sizeof(destination));
        // Offset extruder (only by XY)
        for(i=0; i<2; i++) current_position[i] = current_position[i] - extruder_offset[i][active_extruder] + extruder_offset[i][tmp_extruder];
        // Set the new active extruder and position
        active_extruder = tmp_extruder;
        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E0_AXIS]);
        // Move to the old position if 'F' was in the parameters
        if(make_move && Stopped == false) prepare_move();
      }  //end   if(tmp_extruder != active_extruder) 
    #endif
      printf(SERIAL_ECHO_START);
      printf(MSG_ACTIVE_EXTRUDER);
      printf("%d",active_extruder);
      printf("\n");
    }
  }//end else if(code_seen('T'))
  else
  {
    printf(SERIAL_ECHO_START);
    printf(MSG_UNKNOWN_COMMAND);
    printf("%s",cmdbuffer[bufindr]);
    printf("\"");
  }
  #ifdef HOST_KEEPALIVE_FEATURE
  set_busy_state(0);
  #endif
  
  ClearToSend();
}

void FlushSerialRequestResend()
{
  USART1_Flush();
  printf(MSG_RESEND);
  printf("%d\n", gcode_LastN+1);
  ClearToSend();
}

void ClearToSend()
{
  previous_millis_cmd = HAL_GetTick();
#if SD_SUPPORT || USB_SUPPORT
  if(fromsd[bufindr]) return;
#endif
  printf(MSG_OK);
}

void get_coordinates()
{
  int8_t i;
  volatile bool seen[4] = {false, false, false, false};
  for(i=0; i<NUM_AXIS; i++)
  {
    if(code_seen(axis_codes[i]))
    {
      destination[i] = (float)code_value() + (axis_relative_modes[i]||relative_mode)*current_position[i];
      seen[i] = true;
    }
    else destination[i] = current_position[i]; //Are these else lines really needed?
  }
  if(code_seen('F'))
  {
    next_feedrate = code_value();
    if(next_feedrate > 0.0) feedrate = next_feedrate;
  }
#if FWRETRACT
  if(autoretract_enabled)
  if(!(seen[X_AXIS] || seen[Y_AXIS] || seen[Z_AXIS]) && seen[E0_AXIS])
  {
    float echange = destination[E0_AXIS] - current_position[E0_AXIS];
    if(echange<-MIN_RETRACT) //retract
    {
      if(!retracted)
      {
        destination[Z_AXIS]+=retract_zlift; //not sure why chaninging current_position negatively does not work.
        //if slicer retracted by echange=-1mm and you want to retract 3mm, corrrectede=-2mm additionally
        float correctede=-echange-retract_length;
        //to generate the additional steps, not the destination is changed, but inversely the current position
        current_position[E0_AXIS]+=-correctede;
        feedrate=retract_feedrate;
        retracted=true;
      }
    }
    else if(echange>MIN_RETRACT) //retract_recover
    {
      if(retracted)
      {
        //current_position[Z_AXIS]+=-retract_zlift;
        //if slicer retracted_recovered by echange=+1mm and you want to retract_recover 3mm, corrrectede=2mm additionally
        float correctede=-echange+1*retract_length+retract_recover_length; //total unretract=retract_length+retract_recover_length[surplus]
        current_position[E0_AXIS]+=correctede; //to generate the additional steps, not the destination is changed, but inversely the current position
        feedrate=retract_recover_feedrate;
        retracted=false;
      }
    }
  }
#endif
}

void get_arc_coordinates(void)
{
#ifdef SF_ARC_FIX
  bool relative_mode_backup = relative_mode;
  relative_mode = true;
#endif
  get_coordinates();
#ifdef SF_ARC_FIX
  relative_mode = relative_mode_backup;
#endif
   if(code_seen('I')) offset[0] = code_value();
   else offset[0] = 0.0;
   if(code_seen('J')) offset[1] = code_value();
   else offset[1] = 0.0;
}

void clamp_to_software_endstops(float target[3])
{
  if(min_software_endstops)
  {
    if(target[X_AXIS] < min_pos[X_AXIS]) target[X_AXIS] = min_pos[X_AXIS];
    if(target[Y_AXIS] < min_pos[Y_AXIS]) target[Y_AXIS] = min_pos[Y_AXIS];
    if(target[Z_AXIS] < min_pos[Z_AXIS]) target[Z_AXIS] = min_pos[Z_AXIS];
  }
  if(max_software_endstops)
  {
    if(target[X_AXIS] > max_pos[X_AXIS]) target[X_AXIS] = max_pos[X_AXIS];
    if(target[Y_AXIS] > max_pos[Y_AXIS]) target[Y_AXIS] = max_pos[Y_AXIS];
    if(target[Z_AXIS] > max_pos[Z_AXIS]) target[Z_AXIS] = max_pos[Z_AXIS];
  }
}

void prepare_move(void)
{
  int8_t i;
  clamp_to_software_endstops(destination);
  previous_millis_cmd = HAL_GetTick();
  // Do not use feedmultiply for E or Z only moves
  if((current_position[X_AXIS] == destination [X_AXIS]) && (current_position[Y_AXIS] == destination [Y_AXIS]))
  {
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate/60, active_extruder);
  }
  else
  {
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E0_AXIS], feedrate*feedmultiply/60/100.0, active_extruder);
  }
  for(i=0; i<NUM_AXIS; i++) current_position[i] = destination[i];
}

#define M_PI  3.14159265358979323846
// The arc is approximated by generating a huge number of tiny, linear segments. The length of each 
// segment is configured in settings.mm_per_arc_segment.  
void mc_arc(float *position, float *target, float *offset, uint8_t axis_0, uint8_t axis_1, 
  uint8_t axis_linear, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
{      
  //   int acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
  //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
  float center_axis0 = position[axis_0] + offset[axis_0];
  float center_axis1 = position[axis_1] + offset[axis_1];
  float linear_travel = target[axis_linear] - position[axis_linear];
  float extruder_travel = target[E0_AXIS] - position[E0_AXIS];
  float r_axis0 = -offset[axis_0];  // Radius vector from center to current location
  float r_axis1 = -offset[axis_1];
  float rt_axis0 = target[axis_0] - center_axis0;
  float rt_axis1 = target[axis_1] - center_axis1;
  float angular_travel;
  float millimeters_of_travel;
  uint16_t segments;
  float theta_per_segment;
  float linear_per_segment;
  float extruder_per_segment;
  float cos_T; // Small angle approximation
  float sin_T;
  float arc_target[4];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count;
  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  angular_travel = atan2(r_axis0*rt_axis1-r_axis1*rt_axis0, r_axis0*rt_axis0+r_axis1*rt_axis1);
  if(angular_travel < 0) {angular_travel += 2*M_PI;}
  if(isclockwise) {angular_travel -= 2*M_PI; }
  millimeters_of_travel = hypot(angular_travel*radius, fabs(linear_travel));
  if(millimeters_of_travel < 0.001f) return;  //single-precision
  segments = floor(millimeters_of_travel/MM_PER_ARC_SEGMENT);
  if(segments == 0) segments = 1;
  /*  
    // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
    // by a number of discrete segments. The inverse feed_rate should be correct for the sum of 
    // all segments.
    if (invert_feed_rate) { feed_rate *= segments; }
  */
  theta_per_segment = angular_travel / segments;
  linear_per_segment = linear_travel / segments;
  extruder_per_segment = extruder_travel / segments;
  /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
     and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
         r_T = [cos(phi) -sin(phi);
                sin(phi)  cos(phi] * r ;
     
     For arc generation, the center of the circle is the axis of rotation and the radius vector is 
     defined from the circle center to the initial position. Each line segment is formed by successive
     vector rotations. This requires only two cos() and sin() computations to form the rotation
     matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
     all double numbers are single precision on the Arduino. (True double precision will not have
     round off issues for CNC applications.) Single precision error can accumulate to be greater than
     tool precision in some cases. Therefore, arc path correction is implemented. 

     Small angle approximation may be used to reduce computation overhead further. This approximation
     holds for everything, but very small circles and large mm_per_arc_segment values. In other words,
     theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
     to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for 
     numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
     issue for CNC machines with the single precision Arduino calculations.
     
     This approximation also allows mc_arc to immediately insert a line segment into the planner 
     without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
     a correction, the planner should have caught up to the lag caused by the initial mc_arc overhead. 
     This is important when there are successive arc motions. 
  */
  // Vector rotation matrix values
  cos_T = 1-0.5f*theta_per_segment*theta_per_segment; // Small angle approximation  //single-precision
  sin_T = theta_per_segment;
  count = 0;
  // Initialize the linear axis
  arc_target[axis_linear] = position[axis_linear];
  // Initialize the extruder axis
  arc_target[E0_AXIS] = position[E0_AXIS];
  for(i = 1; i<segments; i++) // Increment (segments-1)
  {
    if(count < N_ARC_CORRECTION)
    {
      // Apply vector rotation matrix 
      r_axisi = r_axis0*sin_T + r_axis1*cos_T;
      r_axis0 = r_axis0*cos_T - r_axis1*sin_T;
      r_axis1 = r_axisi;
      count++;
    }
    else
    {
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      cos_Ti = cos(i*theta_per_segment);
      sin_Ti = sin(i*theta_per_segment);
      r_axis0 = -offset[axis_0]*cos_Ti + offset[axis_1]*sin_Ti;
      r_axis1 = -offset[axis_0]*sin_Ti - offset[axis_1]*cos_Ti;
      count = 0;
    }
    // Update arc_target location
    arc_target[axis_0] = center_axis0 + r_axis0;
    arc_target[axis_1] = center_axis1 + r_axis1;
    arc_target[axis_linear] += linear_per_segment;
    arc_target[E0_AXIS] += extruder_per_segment;
    clamp_to_software_endstops(arc_target);
    plan_buffer_line(arc_target[X_AXIS], arc_target[Y_AXIS], arc_target[Z_AXIS], arc_target[E0_AXIS], feed_rate, extruder);
  }
  // Ensure last segment arrives at target location.
    plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E0_AXIS], feed_rate, extruder);
  //   plan_set_acceleration_manager_enabled(acceleration_manager_was_enabled);
}

void prepare_arc_move(unsigned char isclockwise)
{
  int8_t i;
  float r = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc
  // Trace the arc
  mc_arc(current_position, destination, offset, X_AXIS, Y_AXIS, Z_AXIS, feedrate*feedmultiply/60/100.0, r, isclockwise, active_extruder);
  // As far as the parser is concerned, the position is now == target. In reality the
  // motion control system might still be processing the action and the real tool position
  // in any intermediate location.
  for(i=0; i < NUM_AXIS; i++) current_position[i] = destination[i];
  previous_millis_cmd = HAL_GetTick();
}

#ifdef HOST_KEEPALIVE_FEATURE
void host_keep_alive(void);
#endif
#ifdef TEMPRATURE_AUTO_REPORT
void temprature_auto_report(void);
#endif

void manage_inactivity(void)
{
  if((HAL_GetTick()-previous_millis_cmd) > max_inactive_time)
    if(max_inactive_time)
      kill();
  if(stepper_inactive_time)
  {
    if((HAL_GetTick()-previous_millis_cmd) > stepper_inactive_time)
    {
      if(blocks_queued() == false)
      {
        Stepper_EnPin_Control(X_AXIS, DISABLE);
        Stepper_EnPin_Control(Y_AXIS, DISABLE);
        Stepper_EnPin_Control(Z_AXIS, DISABLE);
        Stepper_EnPin_Control(E0_AXIS, DISABLE);
        Stepper_EnPin_Control(E1_AXIS, DISABLE);
     //   disable_e2();
      }
    }
  }
  check_axes_activity();
//处理GCODE命令时，输出busy信息
//tangbing
#ifdef HOST_KEEPALIVE_FEATURE
  host_keep_alive();
#endif
//增加温度自动上报功能
//tangbing
#ifdef TEMPRATURE_AUTO_REPORT
  temprature_auto_report();
#endif
}

void kill(void)
{
  CRITICAL_SECTION_START; // Stop interrupts
  Heater_Disable();
  Stepper_EnPin_Control(X_AXIS, DISABLE);
  Stepper_EnPin_Control(Y_AXIS, DISABLE);
  Stepper_EnPin_Control(Z_AXIS, DISABLE);
  Stepper_EnPin_Control(E0_AXIS, DISABLE);
  Stepper_EnPin_Control(E1_AXIS, DISABLE);
 // disable_e2();
//#if defined(PS_ON_PIN) && PS_ON_PIN > -1
//  pinMode(PS_ON_PIN,INPUT);
//#endif
  printf(SERIAL_ERROR_START);
  printf(MSG_ERR_KILLED);
 // LCD_ALERTMESSAGEPGM(MSG_KILLED);
//  suicide();
  while(1){ /* Intentionally left empty */ } // Wait for reset
}

void Stop(void)
{
  Heater_Disable();
  if(Stopped == false)
  {
    Stopped = true;
    Stopped_gcode_LastN = gcode_LastN; // Save last g_code for restart
  }
}

bool IsStopped(void) 
{
  return Stopped; 
}
bool setTargetedHotend(int code)
{
  tmp_extruder = active_extruder;
  if(code_seen('T')) 
  {
    tmp_extruder = code_value();
    if(tmp_extruder >= EXTRUDERS) 
    {
      printf(SERIAL_ECHO_START);
      switch(code)
      {
        case 104: printf(MSG_M104_INVALID_EXTRUDER); break;
        case 105: printf(MSG_M105_INVALID_EXTRUDER); break;
        case 109: printf(MSG_M109_INVALID_EXTRUDER); break;
        case 218: printf(MSG_M218_INVALID_EXTRUDER); break;
      }
      printf("%d", tmp_extruder);
      return true;
    }
  }
  return false;
}

void Config_PrintSettings()
{  // Always have this function, even with EEPROM_SETTINGS disabled, the current values will be shown
    printf(SERIAL_ECHO_START);
    printf("Steps per unit:");
    printf(SERIAL_ECHO_START);
    printf("  M92 X%f", axis_steps_per_unit[0]);
    printf(" Y%f", axis_steps_per_unit[1]);
    printf(" Z%f", axis_steps_per_unit[2]);
    printf(" E%f\r\n", axis_steps_per_unit[3]);
    printf(SERIAL_ECHO_START);
    printf("Maximum feedrates (mm/s):\r\n");
    printf(SERIAL_ECHO_START);
    printf("  M203 X%f", max_feedrate[0]);
    printf(" Y%f", max_feedrate[1]); 
    printf(" Z%f", max_feedrate[2]); 
    printf(" E%f\r\n", max_feedrate[3]);
    printf(SERIAL_ECHO_START);
    printf("Maximum Acceleration (mm/s2):\r\n");
    printf(SERIAL_ECHO_START);
    printf("  M201 X%ld", max_acceleration_units_per_sq_second[0]); 
    printf(" Y%ld", max_acceleration_units_per_sq_second[1]); 
    printf(" Z%ld", max_acceleration_units_per_sq_second[2]);
    printf(" E%ld\r\n", max_acceleration_units_per_sq_second[3]);
  //  SERIAL_ECHOLN("");
    printf(SERIAL_ECHO_START);
    printf("Acceleration: S=acceleration, T=retract acceleration\r\n");
    printf(SERIAL_ECHO_START);
    printf("  M204 S%f", acceleration);
    printf(" T%f\r\n", retract_acceleration);
 ///   SERIAL_ECHOLN("");
    printf(SERIAL_ECHO_START);
    printf("Advanced variables: S=Min feedrate (mm/s), T=Min travel feedrate (mm/s), B=minimum segment time (ms), X=maximum XY jerk (mm/s),  Z=maximum Z jerk (mm/s),  E=maximum E jerk (mm/s)\r\n");
    printf(SERIAL_ECHO_START);
    printf("  M205 S%f", minimumfeedrate);
    printf(" T%f", mintravelfeedrate);
    printf(" B%ld", minsegmenttime);
    printf(" X%f", max_xy_jerk );
    printf(" Z%f", max_z_jerk);
    printf(" E%f\r\n", max_e_jerk);
   // SERIAL_ECHOLN("");
    printf(SERIAL_ECHO_START);
    printf("Home offset (mm):\r\n");
    printf(SERIAL_ECHO_START);
    printf("  M206 X%f", add_homeing[0]);
    printf(" Y%f", add_homeing[1]);
    printf(" Z%f", add_homeing[2]);
  //  SERIAL_ECHOLN("");
#ifdef PIDTEMP
    printf(SERIAL_ECHO_START);
    printf("PID settings:\r\n");
    printf(SERIAL_ECHO_START);
    printf("   M301 P%f", Kp);
    printf(" I%f", Temperature_ScalePID(0, Ki, 1));
    printf(" D%f\r\n", Temperature_ScalePID(1, Kd, 1));
  //  SERIAL_ECHOLN(""); 
#endif
} 
