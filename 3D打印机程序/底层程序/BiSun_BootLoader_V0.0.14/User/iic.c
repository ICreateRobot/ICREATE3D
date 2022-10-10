/**
  ******************************************************************************
  * @file     iic.c
  * @author   LEO
	* @date     2018/11/20
	* @version  0.1.3
  * @brief    IIC2:OLED
	* @update   IIC1->IIC2 EEPROM->OLED
							加入DMA驱动方式可通过IIC_OLED_DMA使能
							加入模拟IIC驱动方式通过OLED_IIC_MODE选择
	******************************************************************************
**/
#include "iic.h"

I2C_HandleTypeDef IIC2_HandleStruct;

/*IIC2初始化 clockSpeed:时钟频率(<400KHz) PB10:IIC2_SCL PB11:IIC2_SDA*/
HAL_StatusTypeDef IIC2_Configuration(unsigned int clockSpeed)
{
	HAL_StatusTypeDef state;
	GPIO_InitTypeDef GPIO_InitStruct;
#if ((OLED_IIC_MODE==0) || (OLED_IIC_MODE==1))
	__HAL_RCC_I2C2_CLK_ENABLE();
#if (OLED_IIC_MODE == 1)
	DMA_HandleTypeDef IIC2_DMA_HandleStruct;
	__HAL_RCC_DMA1_CLK_ENABLE();
#endif
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  IIC2_HandleStruct.Instance = I2C2;
  IIC2_HandleStruct.Init.ClockSpeed = clockSpeed;
  IIC2_HandleStruct.Init.DutyCycle = I2C_DUTYCYCLE_2;
  IIC2_HandleStruct.Init.OwnAddress1 = 0;
  IIC2_HandleStruct.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  IIC2_HandleStruct.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  IIC2_HandleStruct.Init.OwnAddress2 = 0;
  IIC2_HandleStruct.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  IIC2_HandleStruct.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  state = HAL_I2C_Init(&IIC2_HandleStruct);
#if (OLED_IIC_MODE == 1)
	HAL_NVIC_SetPriority(I2C2_EV_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
	IIC2_DMA_HandleStruct.Instance = DMA1_Channel4;
	IIC2_DMA_HandleStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
	IIC2_DMA_HandleStruct.Init.PeriphInc = DMA_PINC_DISABLE;
	IIC2_DMA_HandleStruct.Init.MemInc = DMA_MINC_ENABLE;
	IIC2_DMA_HandleStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	IIC2_DMA_HandleStruct.Init.MemDataAlignment = DMA_PDATAALIGN_BYTE;
	IIC2_DMA_HandleStruct.Init.Mode = DMA_NORMAL;
	IIC2_DMA_HandleStruct.Init.Priority = DMA_PRIORITY_MEDIUM;
	__HAL_LINKDMA(&IIC2_HandleStruct, hdmatx, IIC2_DMA_HandleStruct);
	if(HAL_DMA_Init(&IIC2_DMA_HandleStruct) != HAL_OK) return HAL_ERROR;
	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 3, 1);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
#endif
#else
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	state = HAL_OK;
#endif
	return state;
}

#if (OLED_IIC_MODE == 2)
/*模拟IIC发送起始信号*/
void Simulate_I2C_Start(void)
{
	OLED_IIC_SCL(GPIO_PIN_SET);
	OLED_IIC_SDA(GPIO_PIN_SET);
	OLED_IIC_SDA(GPIO_PIN_RESET);
	OLED_IIC_SCL(GPIO_PIN_RESET);
}

/*模拟IIC发送停止信号*/
void Simulate_I2C_Stop(void)
{
	OLED_IIC_SCL(GPIO_PIN_SET);
	OLED_IIC_SDA(GPIO_PIN_RESET);
	OLED_IIC_SDA(GPIO_PIN_SET);
}

/*模拟IIC等待信号*/
void Simulate_I2C_WaitAck(void)
{
	OLED_IIC_SCL(GPIO_PIN_SET);
	OLED_IIC_SCL(GPIO_PIN_RESET);
}

/*模拟IIC写字节*/
void Simulate_I2C_WriteByte(unsigned char ch)
{
	unsigned char counter;
	OLED_IIC_SCL(GPIO_PIN_RESET);
	for(counter=0; counter<8; counter++)
	{
		if(ch & 0x80) OLED_IIC_SDA(GPIO_PIN_SET);
		else OLED_IIC_SDA(GPIO_PIN_RESET);
		ch <<= 1;
		OLED_IIC_SCL(GPIO_PIN_SET);
		OLED_IIC_SCL(GPIO_PIN_RESET);
	}
}

/*模拟IIC发送数据*/
void Simulate_I2C_Master_Transmit(unsigned char devAddress, unsigned char *pData, unsigned short int size)
{
	unsigned short int counter;
	Simulate_I2C_Start();
	Simulate_I2C_WriteByte(devAddress);
	Simulate_I2C_WaitAck();
	for(counter=0; counter<size; counter++)
	{
		Simulate_I2C_WriteByte(*pData);
		Simulate_I2C_WaitAck();
		pData++;
	}
	Simulate_I2C_Stop();
}
#endif

#if (OLED_IIC_MODE == 1)
/*IIC2事件中断服务函数*/
void I2C2_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&IIC2_HandleStruct);
}

/*DMA1通道4中断服务函数*/
void DMA1_Channel4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(IIC2_HandleStruct.hdmatx);
}
#endif
