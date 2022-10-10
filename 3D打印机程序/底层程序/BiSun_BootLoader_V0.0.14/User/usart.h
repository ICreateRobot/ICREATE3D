#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*串口1初始化 PA8:DTR PA9:Tx baudRate:波特率*/
void USART1_Configuration(unsigned int baudRate);

#ifdef __cplusplus
}
#endif

#endif
