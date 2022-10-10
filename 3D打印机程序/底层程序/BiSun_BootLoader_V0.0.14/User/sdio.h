#ifndef __SDIO_H
#define __SDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

extern SD_HandleTypeDef SD_HandleStruct;

/*SDIO初始化 返回值:0,正确; 其他值,错误 PC8:SDIO_D0 PC9:SDIO_D1 PC10:SDIO_D2 PC11:SDIO_D3 PC12:SDIO_CK PD2:SDIO_CMD */
HAL_SD_ErrorTypedef SDIO_Configuration(void);

/*设置SDIO的DMA方向 dir:DMA_PERIPH_TO_MEMORY,外设->储存器;DMA_MEMORY_TO_PERIPH,储存器->外设*/
HAL_SD_ErrorTypedef SDIO_DMA_SetDirection(unsigned char dir);

#ifdef __cplusplus
}
#endif

#endif
