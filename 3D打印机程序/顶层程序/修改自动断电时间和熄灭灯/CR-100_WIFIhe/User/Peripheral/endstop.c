/**
  ******************************************************************************
  * @file     endstop.c
  * @author   LEO
	* @date     2018/11/20
	* @version  0.1.1
  * @brief    ��λ���ز�������
	* @update   ��ӦCR-100
	******************************************************************************
**/
#include "endstop.h"
#include "language.h"
#include "planner.h"
#include "hmi.h"
#include "ws2812.h"
#include "temperature.h"

volatile long EndStops_TrigSteps[3] = {0, 0, 0};
volatile long Count_Position[NUM_AXIS] = {0, 0, 0, 0};
volatile signed char Count_Direction[NUM_AXIS] = {1, 1, 1, 1};
volatile bool EndStop_X_Hit = false;
volatile bool EndStop_Y_Hit = false;
volatile bool EndStop_Z_Hit = false;
bool EndStops_Check = true;
#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED
bool abort_on_endstop_hit = false;
#endif
#if X_MIN_PIN_USED
bool EndStop_X_Min_Old = true;
#endif
#if X_MAX_PIN_USED
bool EndStop_X_Max_Old = true;
#endif
#if Y_MIN_PIN_USED
bool EndStop_Y_Min_Old = true;
#endif
#if Y_MAX_PIN_USED
bool EndStop_Y_Max_Old = true;
#endif
#if Z_MIN_PIN_USED
bool EndStop_Z_Min_Old = true;
#endif
#if Z_MAX_PIN_USED
bool EndStop_Z_Max_Old = true;
#endif
/*�Զ��ϵ��ʼ�� PC1*/
void Automatic_Power_Off_Configuration(void)//APO_Configuration
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed= GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
}
/*�Զ��ϵ�*/
uint16_t Key_Down_Time=0;
uint8_t Key_Down_flag_Last=0;
uint8_t No_Operation_flag=0;
void Automatic_Power_Off(void)  //�Զ��ϵ���
{
    if(Print_Finished_flag==0)
    {
        RGB_LED_WHITE();
    }
    if(Printing_flag==1)   //���ڴ�ӡ����
    {
        Print_Finished_flag=0;
        Key_Down_flag=0;
        No_Operation_flag=0;
			  Key_Down_Time=0;
    }
    if(Heater_GetCurrentCelsius(Heater_Head0)<=40)   //�¶�С��40��
    {
        if(Key_Down_flag!=0)
        {
            Key_Down_Time++;
            if(Key_Down_flag_Last!=Key_Down_flag)
            {
                Key_Down_Time=0;
            }
            Key_Down_flag_Last=Key_Down_flag;
            if(Key_Down_Time>255)                  //��������֮��ȴ�һ��ʱ��֮���޲����ڶϵ�
            {
                Key_Down_Time=0;
                Key_Down_flag=0;
                No_Operation_flag=1;
            }
        }
        else
        {
            Key_Down_Time++;
            if(Key_Down_Time>255)                  //�ȴ�һ��ʱ��֮���޲����ڶϵ�
            {
                Key_Down_Time=0;
                Key_Down_flag=0;
                No_Operation_flag=1;
            }
        }
    }
    else
    {
        No_Operation_flag=0;
    }
    if((Print_Finished_flag==1)&&(Heater_GetCurrentCelsius(Heater_Head0)<=50)&&(No_Operation_flag==1))//&&Key_Down_flag==0  �¶ȵ���
    {
        RGB_LED_OFF();                                       //Ϩ��ƴ�
        Delay_Ms(100);
			  RGB_LED_OFF();                                       //Ϩ��ƴ�
        Delay_Ms(100);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);  //�Զ��ػ���������ߵ�ƽ��
    }
}
/*��λ���س�ʼ�� PA5:X_Min PA6:Y_Min PA7:Z_Min*/
void EndStops_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    EndStops_Enable(true);
}

/*��λ���ؼ�⿪��״̬ axis:ָ���� position:����λ��*/
bool EndStops_CheckState(Axis_EnumTypeDef axis, EndStop_PositionTypeDef position)
{
    GPIO_PinState state;
    if(axis == X_AXIS)
    {
        if(position == MIN_POSITION) state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
        else state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
    }
    else if(axis == Y_AXIS)
    {
        if(position == MIN_POSITION) state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
        else state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
    }
    else if(axis == Z_AXIS)
    {
        if(position == MIN_POSITION) state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
        else state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
    }
    if(state == GPIO_PIN_SET) return true;
    return false;
}

/*��λ���ر���״̬*/
void EndStops_ReportState(void)
{
    if(EndStop_X_Hit || EndStop_Y_Hit || EndStop_Z_Hit)
    {
        printf(SERIAL_ECHO_START);
        printf(MSG_ENDSTOPS_HIT);
        if(EndStop_X_Hit) printf(" X:%f", (float)EndStops_TrigSteps[X_AXIS] / axis_steps_per_unit[X_AXIS]);
        if(EndStop_Y_Hit) printf(" Y:%f", (float)EndStops_TrigSteps[Y_AXIS] / axis_steps_per_unit[Y_AXIS]);
        if(EndStop_Z_Hit) printf(" Z:%f", (float)EndStops_TrigSteps[Z_AXIS] / axis_steps_per_unit[Z_AXIS]);
        printf("\n");
        EndStop_X_Hit = false;
        EndStop_Y_Hit = false;
        EndStop_Z_Hit = false;
#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED
        if(abort_on_endstop_hit)
        {
            card.sdprinting = false;
            card.closefile();
            quickStop();
            Heater_SetTargetCelsius(0, Heater_Head0);
            Heater_SetTargetCelsius(0, Heater_Head1);
        }
#endif
    }
}

/*��λ�����������״̬*/
void EndStops_Clear(void)
{
    EndStop_X_Hit = false;
    EndStop_Y_Hit = false;
    EndStop_Z_Hit = false;
}

/*��λ�������ü��״̬*/
void EndStops_Enable(bool check)
{
    EndStops_Check = check;
}
