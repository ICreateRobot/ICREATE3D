#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*����1��ʼ�� PA8:DTR PA9:Tx baudRate:������*/
void USART1_Configuration(unsigned int baudRate);

#ifdef __cplusplus
}
#endif

#endif
