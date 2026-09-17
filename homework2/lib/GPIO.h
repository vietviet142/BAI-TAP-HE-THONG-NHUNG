#ifndef GPIO_H
#define GPIO_H
#include "stm32f10x.h"
#include "stdint.h"
#include "stdbool.h"
extern void SysTick_Init(void);
extern void delay_us(uint32_t us);
extern void delay_ms(uint32_t ms);
void GPIO_Cfg_PinLow_Output(GPIO_TypeDef *GPIOx, uint32_t Pin);
void GPIO_Cfg_PinHigh_Output(GPIO_TypeDef *GPIOx, uint32_t Pin);
void GPIO_Cfg_Pin_InOutPut(GPIO_TypeDef *GPIOx, uint32_t Pin, bool InOut);
void GPIO_Setbit(GPIO_TypeDef *GPIOx,uint32_t pin);
void GPIO_Resetbit(GPIO_TypeDef *GPIOx,uint32_t pin);
void GPIO_Toggle(GPIO_TypeDef *GPIOx,uint32_t pin);
uint8_t GPIO_ReadOutPut_DataBit(GPIO_TypeDef *GPIOx, uint32_t Pin);
uint8_t GPIO_ReadInPut_DataBit(GPIO_TypeDef *GPIOx, uint32_t Pin);

#endif