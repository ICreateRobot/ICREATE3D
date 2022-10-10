#ifndef __SDIO_H
#define __SDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

extern SD_HandleTypeDef SD_HandleStruct;

/*SDIO��ʼ�� ����ֵ:0,��ȷ; ����ֵ,���� PC8:SDIO_D0 PC9:SDIO_D1 PC10:SDIO_D2 PC11:SDIO_D3 PC12:SDIO_CK PD2:SDIO_CMD */
HAL_SD_ErrorTypedef SDIO_Configuration(void);

/*����SDIO��DMA���� dir:DMA_PERIPH_TO_MEMORY,����->������;DMA_MEMORY_TO_PERIPH,������->����*/
HAL_SD_ErrorTypedef SDIO_DMA_SetDirection(unsigned char dir);

#ifdef __cplusplus
}
#endif

#endif
