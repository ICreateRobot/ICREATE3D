#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

#define USART1_DATA_SIZE  128

typedef struct
{
  unsigned char Data[USART1_DATA_SIZE];
  int Head;
  int Tail;
}USART1_RxTypeDef;

/*����1��ʼ�� PA8:DTR PA9:Tx PA10:Rx baudRate:������*/
void USART1_Configuration(unsigned int baudRate);

/*����2��ʼ�� PA2:Tx PA3:Rx baudRate:������*/
void USART2_Configuration(unsigned int baudRate);

/*����3��ʼ�� PB10:Tx PB11:Rx baudRate:������*/
void USART3_Configuration(unsigned int baudRate);

/*��鴮��1�����ն˾���״̬ PA8:DTR*/
void USART1_CheckDTR(void);

/*����1��ȡ����*/
unsigned char USART1_ReadData(void);

/*����1���ݾ���״̬*/
unsigned int USART1_IsAvailable(void);

/*����1������ݻ���*/
void USART1_Flush(void);

/*����1��������*/
void USART1_SaveChar(unsigned char byte);

#ifdef __cplusplus
}
#endif

#endif
