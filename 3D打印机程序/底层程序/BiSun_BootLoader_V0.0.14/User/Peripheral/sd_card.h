#ifndef __SD_CARD_H
#define __SD_CARD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include "system.h"
#include "sdio.h"

#define MSD_OK     ((uint8_t)0x00)
#define MSD_ERROR  ((uint8_t)0x01)
   
#define SD_TRANSFER_OK    ((uint8_t)0x00)
#define SD_TRANSFER_BUSY  ((uint8_t)0x01)

#define SD_PRESENT      ((uint8_t)0x01)
#define SD_NOT_PRESENT  ((uint8_t)0x00)
#define SD_DATATIMEOUT  ((uint32_t)100000000)

/*SD����ʼ�� PC7:SD_CD ����ֵ:0,��ȷ; ����ֵ,����*/
HAL_SD_ErrorTypedef SD_Card_Configuration(void);

/*SD��������(DMA) *pData:���ݻ����� ReadAddr:������ַ NumOfBlocks:��������*/
HAL_SD_ErrorTypedef SD_Card_Read(unsigned char *pData, unsigned int ReadAddr, unsigned int NumOfBlocks);

/*SD��д����(DMA) *pData:���ݻ����� WriteAddr:������ַ NumOfBlocks:��������*/
HAL_SD_ErrorTypedef SD_Card_Write(unsigned char *pData, unsigned int WriteAddr, unsigned int NumOfBlocks);

/*SD����ȡ����Ϣ*/
void SD_Card_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo);

/*SD��������*/
GPIO_PinState SD_Card_Detect(void);

#ifdef __cplusplus
}
#endif

#endif
