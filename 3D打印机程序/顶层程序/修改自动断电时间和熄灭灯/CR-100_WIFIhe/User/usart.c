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
#include "printer.h"

unsigned char USART1_RxData[1];
UART_HandleTypeDef USART1_HandleStruct, USART2_HandleStruct, USART3_HandleStruct;
USART1_RxTypeDef USART1_RxStruct = {{0}, 0, 0};

/*串口1初始化 PA8:DTR PA9:Tx PA10:Rx baudRate:波特率*/
void USART1_Configuration(unsigned int baudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  USART1_HandleStruct.Instance = USART1;
  USART1_HandleStruct.Init.BaudRate = baudRate;
  USART1_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
  USART1_HandleStruct.Init.StopBits = UART_STOPBITS_1;
  USART1_HandleStruct.Init.Parity = UART_PARITY_NONE;
  USART1_HandleStruct.Init.Mode = UART_MODE_TX_RX;
  USART1_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	HAL_UART_Init(&USART1_HandleStruct);
	HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&USART1_HandleStruct, USART1_RxData, 1);
}

/*串口2初始化 PA2:Tx PA3:Rx baudRate:波特率*/
void USART2_Configuration(unsigned int baudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  USART2_HandleStruct.Instance = USART2;
  USART2_HandleStruct.Init.BaudRate = baudRate;
  USART2_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
  USART2_HandleStruct.Init.StopBits = UART_STOPBITS_1;
  USART2_HandleStruct.Init.Parity = UART_PARITY_NONE;
  USART2_HandleStruct.Init.Mode = UART_MODE_TX_RX;
  USART2_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  USART2_HandleStruct.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&USART2_HandleStruct);
}

/*串口3初始化 PB10:Tx PB11:Rx baudRate:波特率*/
void USART3_Configuration(unsigned int baudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_USART3_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  USART3_HandleStruct.Instance = USART3;
  USART3_HandleStruct.Init.BaudRate = baudRate;
  USART3_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
  USART3_HandleStruct.Init.StopBits = UART_STOPBITS_1;
  USART3_HandleStruct.Init.Parity = UART_PARITY_NONE;
  USART3_HandleStruct.Init.Mode = UART_MODE_TX_RX;
  USART3_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  USART3_HandleStruct.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&USART3_HandleStruct);
}

/*检查串口1数据终端就绪状态 PA8:DTR*/
void USART1_CheckDTR(void)
{
	static bool USART1_DTR_State = true;
	static bool USART1_DTR_OldState = true;
	USART1_DTR_State = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
	if(!USART1_DTR_State && USART1_DTR_OldState) Config_PrintSettings();
	USART1_DTR_OldState = USART1_DTR_State;
}

/*串口1读取数据*/
unsigned char USART1_ReadData(void)
{
	unsigned char byte;
  if(USART1_RxStruct.Head == USART1_RxStruct.Tail) return 0;
	else 
	{
		byte = USART1_RxStruct.Data[USART1_RxStruct.Tail];
		USART1_RxStruct.Tail = (unsigned int)(USART1_RxStruct.Tail+1) % USART1_DATA_SIZE;
		return byte;
  }
}

/*串口1数据就绪状态*/
unsigned int USART1_IsAvailable(void)
{
  return (USART1_DATA_SIZE+USART1_RxStruct.Head-USART1_RxStruct.Tail) % USART1_DATA_SIZE;
}

/*串口1清空数据缓存*/
void USART1_Flush(void)
{
	USART1_RxStruct.Head = USART1_RxStruct.Tail;
}

/*串口1保存数据*/
void USART1_SaveChar(unsigned char byte)
{
  int remainSize = (unsigned int)(USART1_RxStruct.Head+1) % USART1_DATA_SIZE;
  if(remainSize != USART1_RxStruct.Tail)
	{
    USART1_RxStruct.Data[USART1_RxStruct.Head] = byte;
    USART1_RxStruct.Head = remainSize;
  }
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

/*串口1中断服务函数*/
void USART1_IRQHandler(void)
{
	unsigned int timeOut = 0;
	unsigned int maxDelay = 0x1FFFF;
	HAL_UART_IRQHandler(&USART1_HandleStruct);
	timeOut = 0;
	while(HAL_UART_GetState(&USART1_HandleStruct) != HAL_UART_STATE_READY)
	{
		if(timeOut++ > maxDelay) break;
	}
	timeOut = 0;
	while(HAL_UART_Receive_IT(&USART1_HandleStruct, USART1_RxData, 1) != HAL_OK)
	{
		if(timeOut++ > maxDelay) break;
	}
}

/*串口接收完成回调函数*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		USART1_SaveChar(*USART1_RxData);
	}
}
