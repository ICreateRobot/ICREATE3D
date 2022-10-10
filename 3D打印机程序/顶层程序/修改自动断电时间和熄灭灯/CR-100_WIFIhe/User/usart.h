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

/*串口1初始化 PA8:DTR PA9:Tx PA10:Rx baudRate:波特率*/
void USART1_Configuration(unsigned int baudRate);

/*串口2初始化 PA2:Tx PA3:Rx baudRate:波特率*/
void USART2_Configuration(unsigned int baudRate);

/*串口3初始化 PB10:Tx PB11:Rx baudRate:波特率*/
void USART3_Configuration(unsigned int baudRate);

/*检查串口1数据终端就绪状态 PA8:DTR*/
void USART1_CheckDTR(void);

/*串口1读取数据*/
unsigned char USART1_ReadData(void);

/*串口1数据就绪状态*/
unsigned int USART1_IsAvailable(void);

/*串口1清空数据缓存*/
void USART1_Flush(void);

/*串口1保存数据*/
void USART1_SaveChar(unsigned char byte);

#ifdef __cplusplus
}
#endif

#endif
