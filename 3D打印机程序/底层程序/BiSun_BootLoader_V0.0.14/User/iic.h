#ifndef __IIC_H
#define __IIC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*OLED_IIC������ʽѡ�� 0:Ӳ��IIC��ͨ��ʽ 1:Ӳ��IIC-DMA��ʽ 2:ģ��IIC��ʽ*/
#define OLED_IIC_MODE  2

#if (OLED_IIC_MODE == 2)
#define OLED_IIC_SCL(pinState) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, pinState)
#define OLED_IIC_SDA(pinState) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, pinState)
#endif

extern I2C_HandleTypeDef IIC2_HandleStruct;

/*IIC2��ʼ�� clockSpeed:ʱ��Ƶ��(<400KHz) PB10:IIC2_SCL PB11:IIC2_SDA*/
HAL_StatusTypeDef IIC2_Configuration(unsigned int clockSpeed);

#if (OLED_IIC_MODE == 2)
/*ģ��IIC��������*/
void Simulate_I2C_Master_Transmit(unsigned char devAddress, unsigned char *pData, unsigned short int size);
#endif

#ifdef __cplusplus
}
#endif

#endif
