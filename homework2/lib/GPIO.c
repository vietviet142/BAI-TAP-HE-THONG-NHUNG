#include "GPIO.h"

void GPIO_Cfg_PinLow_Output(GPIO_TypeDef *GPIOx, uint32_t Pin)
{
	if(GPIOx == GPIOA)
			RCC -> APB2ENR |= 1 << 2;
	else if(GPIOx == GPIOB)
		RCC -> APB2ENR |= 1 << 3;
	else if(GPIOx == GPIOC)
		RCC -> APB2ENR |= 1 << 4;
	else if(GPIOx == GPIOD)
		RCC -> APB2ENR |= 1 << 5;
	
	GPIOx -> CRL &= ~(0xF << (4 * Pin));
	GPIOx -> CRL |= 0x3 << (4 * Pin); // Config Ouput Push Pull F = 50Hz
}
void GPIO_Cfg_PinHigh_Output(GPIO_TypeDef *GPIOx, uint32_t Pin)
{
	if(GPIOx == GPIOA)
		RCC -> APB2ENR |= 1 << 2;
	else if(GPIOx == GPIOB)
		RCC -> APB2ENR |= 1 << 3;
	else if(GPIOx == GPIOC)
		RCC -> APB2ENR |= 1 << 4;
	else if(GPIOx == GPIOD)
		RCC -> APB2ENR |= 1 << 5;
	
	GPIOx -> CRH &= ~(0xF << (4 * (Pin - 8)));
	GPIOx -> CRH |= 0x3 << (4 * (Pin - 8)); // Config Ouput Push Pull F = 50Hz
}
void GPIO_Cfg_Pin_InOutPut(GPIO_TypeDef *GPIOx, uint32_t Pin, bool InOut)
{
	if(GPIOx == GPIOA)
		RCC -> APB2ENR |= 1 << 2;
	else if(GPIOx == GPIOB)
		RCC -> APB2ENR |= 1 << 3;
	else if(GPIOx == GPIOC)
		RCC -> APB2ENR |= 1 << 4;
	else if(GPIOx == GPIOD)
		RCC -> APB2ENR |= 1 << 5;
	if(InOut) // true == OutPut
	{
		if(Pin <= 7)
		{
			GPIOx -> CRL &= ~(0xF << (4 * Pin));
			GPIOx -> CRL |= 0x3 << (4 * Pin);
		}
		else
		{
			GPIOx -> CRH &= ~(0xF << (4 * (Pin - 8)));
			GPIOx -> CRH |= 0x3 << (4 * (Pin - 8)); // Config Ouput Push Pull F = 50Hz
		}
	}
	else // false == InPut
	{
		if(Pin <= 7)
		{
			GPIOx -> CRL &= ~(0xF << (4 * Pin));
			GPIOx -> CRL |= 0x8 << (4 * Pin);
		}
		else
		{
			GPIOx -> CRH &= ~(0xF << 4 * ((Pin - 8)));
			GPIOx -> CRH |= 0x8 << (4 * (Pin - 8)); 
		}
	}
}
void GPIO_Setbit(GPIO_TypeDef *GPIOx,uint32_t pin)
{	
	GPIOx -> BSRR = 1 << pin;
}
void GPIO_Resetbit(GPIO_TypeDef *GPIOx,uint32_t pin)
{
	GPIOx -> BSRR = 1 << (pin + 16); 
}

uint8_t GPIO_ReadOutPut_DataBit(GPIO_TypeDef *GPIOx, uint32_t Pin)
{
	if((GPIOx -> ODR & (1 << Pin)) != 0)
		return (uint8_t)Bit_SET;
	else 
		return (uint8_t)Bit_RESET;
}
uint8_t GPIO_ReadInPut_DataBit(GPIO_TypeDef *GPIOx, uint32_t Pin)
{
	if((GPIOx -> IDR & (1 << Pin)) != 0)
		return (uint8_t)Bit_SET;
	else 
		return (uint8_t)Bit_RESET;
}
void GPIO_Toggle(GPIO_TypeDef *GPIOx,uint32_t pin) // doi trang thai led
{
	uint8_t bit_status = 0x00;
	bit_status = GPIO_ReadOutPut_DataBit(GPIOx, pin); // doc output cua chan Pin cua Portx
	if(bit_status == 0x00)
			GPIO_Setbit(GPIOx, pin);
	else 
			GPIO_Resetbit(GPIOx, pin);
}