#ifndef __IIC_H
#define __IIC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*OLED_IIC驱动方式选择 0:硬件IIC普通方式 1:硬件IIC-DMA方式 2:模拟IIC方式*/
#define OLED_IIC_MODE  2

#if (OLED_IIC_MODE == 2)
#define OLED_IIC_SCL(pinState) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, pinState)
#define OLED_IIC_SDA(pinState) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, pinState)
#endif

extern I2C_HandleTypeDef IIC2_HandleStruct;

/*IIC2初始化 clockSpeed:时钟频率(<400KHz) PB10:IIC2_SCL PB11:IIC2_SDA*/
HAL_StatusTypeDef IIC2_Configuration(unsigned int clockSpeed);

#if (OLED_IIC_MODE == 2)
/*模拟IIC发送数据*/
void Simulate_I2C_Master_Transmit(unsigned char devAddress, unsigned char *pData, unsigned short int size);
#endif

#ifdef __cplusplus
}
#endif

#endif
