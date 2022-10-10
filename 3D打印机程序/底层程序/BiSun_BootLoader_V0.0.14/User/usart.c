/**
  ******************************************************************************
  * @file     usart.c
  * @author   LEO
	* @date     2018/09/28
	* @version  0.0.1
  * @brief    USART1:PC通讯 USART2:串口屏 USART3:WIFI通讯
	******************************************************************************
**/
#include "usart.h"

UART_HandleTypeDef USART1_HandleStruct;

/*串口1初始化 PA9:Tx baudRate:波特率*/
void USART1_Configuration(unsigned int baudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  USART1_HandleStruct.Instance = USART1;
  USART1_HandleStruct.Init.BaudRate = baudRate;
  USART1_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
  USART1_HandleStruct.Init.StopBits = UART_STOPBITS_1;
  USART1_HandleStruct.Init.Parity = UART_PARITY_NONE;
  USART1_HandleStruct.Init.Mode = UART_MODE_TX;
  USART1_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	HAL_UART_Init(&USART1_HandleStruct);
}

#pragma import(__use_no_semihosting)
/*标准库需要的支持函数*/
struct __FILE
{
	int handle;
};

FILE __stdout;

/*定义_sys_exit()以避免使用半主机模式*/
void _sys_exit(int x)
{
	x = x;
}

/*重构造printf函数指定USART1*/
int fputc(int byte, FILE *f)
{
	while((USART1->SR&0x40) == 0){};
	USART1->DR = (unsigned char)byte;
	return byte;
}
