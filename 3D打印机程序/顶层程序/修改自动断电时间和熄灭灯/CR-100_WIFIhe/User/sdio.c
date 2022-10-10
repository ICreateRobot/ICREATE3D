/**
  ******************************************************************************
  * @file     sdio.c
  * @author   LEO
	* @date     2018/10/08
	* @version  0.0.1
  * @brief    SDIO:SD卡
	******************************************************************************
**/
#include "sdio.h"

SD_HandleTypeDef SD_HandleStruct;
HAL_SD_CardInfoTypedef SD_CardInfoStruct;
DMA_HandleTypeDef SDIO_DMA_HandleStruct;

/*SDIO初始化 返回值:0,正确; 其他值,错误 PC8:SDIO_D0 PC9:SDIO_D1 PC10:SDIO_D2 PC11:SDIO_D3 PC12:SDIO_CK PD2:SDIO_CMD*/
HAL_SD_ErrorTypedef SDIO_Configuration(void)
{
	HAL_SD_ErrorTypedef state;
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_SDIO_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	SD_HandleStruct.Instance = SDIO;
  SD_HandleStruct.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  SD_HandleStruct.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  SD_HandleStruct.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  SD_HandleStruct.Init.BusWide = SDIO_BUS_WIDE_1B;
  SD_HandleStruct.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  SD_HandleStruct.Init.ClockDiv = SDIO_TRANSFER_CLK_DIV;
	HAL_SD_DeInit(&SD_HandleStruct);
  state = HAL_SD_Init(&SD_HandleStruct, &SD_CardInfoStruct);
	HAL_NVIC_SetPriority(SDIO_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(SDIO_IRQn);
	SDIO_DMA_HandleStruct.Instance = DMA2_Channel4;
	SDIO_DMA_HandleStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
	SDIO_DMA_HandleStruct.Init.PeriphInc = DMA_PINC_DISABLE;
	SDIO_DMA_HandleStruct.Init.MemInc = DMA_MINC_ENABLE;
	SDIO_DMA_HandleStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	SDIO_DMA_HandleStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	SDIO_DMA_HandleStruct.Init.Mode = DMA_NORMAL;
	SDIO_DMA_HandleStruct.Init.Priority = DMA_PRIORITY_HIGH;
	__HAL_LINKDMA(&SD_HandleStruct, hdmarx, SDIO_DMA_HandleStruct);
	__HAL_LINKDMA(&SD_HandleStruct, hdmatx, SDIO_DMA_HandleStruct);
	if(HAL_DMA_Init(&SDIO_DMA_HandleStruct) != HAL_OK) return SD_ERROR;
	HAL_NVIC_SetPriority(DMA2_Channel4_5_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);
	return state;
}

/*设置SDIO的DMA方向 dir:DMA_PERIPH_TO_MEMORY,外设->储存器;DMA_MEMORY_TO_PERIPH,储存器->外设*/
HAL_SD_ErrorTypedef SDIO_DMA_SetDirection(unsigned char dir)
{
	if(HAL_DMA_DeInit(&SDIO_DMA_HandleStruct) != HAL_OK) return SD_ERROR;
	SDIO_DMA_HandleStruct.Init.Direction = dir;
	if(HAL_DMA_Init(&SDIO_DMA_HandleStruct) != HAL_OK) return SD_ERROR;
	return SD_OK;
}

/*SDIO中断服务函数*/
void SDIO_IRQHandler(void)
{
	HAL_SD_IRQHandler(&SD_HandleStruct);
}

/*DMA2通道4&5中断服务函数*/
void DMA2_Channel4_5_IRQHandler(void)
{
	if(SDIO_DMA_HandleStruct.Init.Direction == DMA_MEMORY_TO_PERIPH) HAL_DMA_IRQHandler(SD_HandleStruct.hdmatx);
	else HAL_DMA_IRQHandler(SD_HandleStruct.hdmarx);
}
