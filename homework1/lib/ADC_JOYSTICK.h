#ifndef ADC_JOYSTICK
#define ADC_JOYSTICK
#include "stm32f10x.h"

void ADC_Config(uint8_t channel);
void GPIO_Init_JOYSTICK(void);
uint16_t ADC_Read(uint8_t channel);
#endif