/**
  ******************************************************************************
  * @file     sd_card.c
  * @author   LEO
	* @date     2018/10/20
	* @version  0.1.1
  * @brief    DMA方式SD卡操作底层
	* @update   适应CR-100 SD_CD更换引脚
	******************************************************************************
**/

#include "sd_card.h"

#if SD_SUPPORT

__align(4) unsigned char SD_Card_DataBuff[512];

/*SD卡初始化 PC7:SD_CD 返回值:0,正确; 其他值,错误*/
HAL_SD_ErrorTypedef SD_Card_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    HAL_SD_ErrorTypedef state = SD_OK;
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    state = SDIO_Configuration();
    if(state == SD_OK)
    {
        state = HAL_SD_WideBusOperation_Config(&SD_HandleStruct, SDIO_BUS_WIDE_4B);
    }
    return state;
}

/*SD卡读数据(DMA) *pData:数据缓存区 ReadAddr:扇区地址 NumOfBlocks:扇区数量*/
HAL_SD_ErrorTypedef SD_Card_Read(unsigned char *pData, unsigned int ReadAddr, unsigned int NumOfBlocks)
{
    HAL_SD_ErrorTypedef state = SD_OK;
    unsigned char counter;
    ReadAddr <<= 9;
    SDIO_DMA_SetDirection(DMA_PERIPH_TO_MEMORY);
    if((unsigned int)pData%4 != 0)
    {
        for(counter=0; counter<NumOfBlocks; counter++)
        {
            state = HAL_SD_ReadBlocks_DMA(&SD_HandleStruct, (unsigned int *)SD_Card_DataBuff, ReadAddr+512*counter, 512, 1);
            if(state==SD_OK) state = HAL_SD_CheckReadOperation(&SD_HandleStruct, 0xFFFFFFFF);
            memcpy(pData, SD_Card_DataBuff, 512);
            pData += 512;
        }
    }
    else
    {
        state = HAL_SD_ReadBlocks_DMA(&SD_HandleStruct, (unsigned int *)pData, ReadAddr, 512, NumOfBlocks);
        if(state == SD_OK) state = HAL_SD_CheckReadOperation(&SD_HandleStruct, 0xFFFFFFFF);
    }
    return state;
}

/*SD卡写数据(DMA) *pData:数据缓存区 WriteAddr:扇区地址 NumOfBlocks:扇区数量*/
HAL_SD_ErrorTypedef SD_Card_Write(unsigned char *pData, unsigned int WriteAddr, unsigned int NumOfBlocks)
{
    HAL_SD_ErrorTypedef state = SD_OK;
    unsigned char counter;
    WriteAddr <<= 9;
    SDIO_DMA_SetDirection(DMA_MEMORY_TO_PERIPH);
    if((unsigned int)pData%4 != 0)
    {
        for(counter=0; counter<NumOfBlocks; counter++)
        {
            memcpy(pData, SD_Card_DataBuff, 512);
            state = HAL_SD_WriteBlocks_DMA(&SD_HandleStruct, (unsigned int *)SD_Card_DataBuff, WriteAddr+512*counter, 512, 1);
            if(state==SD_OK) state = HAL_SD_CheckWriteOperation(&SD_HandleStruct, 0xFFFFFFFF);
            pData += 512;
        }
    }
    else
    {
        state = HAL_SD_WriteBlocks_DMA(&SD_HandleStruct, (unsigned int *)pData, WriteAddr, 512, NumOfBlocks);
        if(state == SD_OK) state = HAL_SD_CheckWriteOperation(&SD_HandleStruct, 0xFFFFFFFF);
    }
    return state;
}

/*SD卡获取卡信息*/
void SD_Card_GetCardInfo(HAL_SD_CardInfoTypedef *cardInfo)
{
    HAL_SD_Get_CardInfo(&SD_HandleStruct, cardInfo);
}

/*SD卡插入检测*/
GPIO_PinState SD_Card_Detect(void)
{
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7);
}

#endif
