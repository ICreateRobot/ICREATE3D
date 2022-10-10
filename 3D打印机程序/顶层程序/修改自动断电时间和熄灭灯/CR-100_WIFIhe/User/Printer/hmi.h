#ifndef __HMI_H
#define __HMI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

typedef enum
{
  HMI_NO_FILE      = 0,
  HMI_NO_SD        = 1,
  HMI_PRINTING     = 2,
  HMI_PRINT_FINISH = 3,
  HMI_AXIS_HOME    = 4,
  HMI_PAUSE        = 5,
  HMI_HEATING      = 6,
  HMI_COOLING      = 7,
  HMI_READY        = 8,
  HMI_IDLE         = 9,
  HMI_SD_ERROR     = 10
}HMI_StatusTypeDef;

typedef enum
{
  HEAD_HEATING      = 0,
  HEAD_COOLING      = 1,
  HEAD_IDLE         = 2
}Head_StatusTypeDef;

typedef struct
{
  FunctionalState RunTime_Flag;
  unsigned int File_TotalSize;
  HMI_StatusTypeDef Status;
  bool Need_ReDraw;
  unsigned int NextUpdateTime;
  Head_StatusTypeDef Head_Status;
}HMI_MemberTypeDef;

extern HMI_MemberTypeDef HMI_MemberStruct;
extern uint8_t Print_Finished_flag; //��ӡ��ɱ�־λ��
extern uint8_t Key_Down_flag;       //�������±�־λ��
extern uint8_t Printing_flag;       //���ڴ�ӡ��־λ��

/*��������*/
void HMI_StartFrame(void);

/*�����̻���*/
void HMI_ProcessFrame(void);

#ifdef __cplusplus
}
#endif

#endif
