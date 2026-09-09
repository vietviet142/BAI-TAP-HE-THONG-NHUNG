#include "GPIO.h"
#include "ADC_JOYSTICK.h"
void GPIO_Init_JOYSTICK(void)
{
    RCC->APB2ENR |= (1 << 2); // Enable GPIOA

    // PA5, PA6 = Analog (ADC)
    GPIOA->CRL &= ~((0xF << 20) | (0xF << 24));

    // PA7 = Input pull-up
    GPIOA->CRL &= ~(0xF << 28);
    GPIOA->CRL |=  (0x8 << 28);
    GPIOA->ODR |= (1 << 7);
}

// ================= ADC =================
void ADC_Config(uint8_t channel)
{
    // Enable ADC1 clock
    RCC->APB2ENR |= (1 << 9);

    // ADC clock = PCLK2 / 6 (<=14MHz)
    RCC->CFGR |= (2 << 14);

     ADC1->SQR1 &= ~(0xF << 20);   // 1 conversion
    ADC1->SQR3 = channel;         // SQ1 = channel

    // Sample time (239.5 cycles cho ?n d?nh)
   // ADC1->SMPR2 |= (7 << (3 * 5)); // CH5
   // ADC1->SMPR2 |= (7 << (3 * 6)); // CH6
		
        ADC1->SMPR1 &= ~(7 << (channel * 3));
        ADC1->SMPR1 |=  (7 << (channel * 3));
	
		ADC1->CR2 &= ~(7 << 17);
    ADC1->CR2 |=  (7 << 17);      // EXTSEL = 111 -> SWSTART
    ADC1->CR2 |=  (1 << 20);      // EXTTRIG = 1  <-- THÊM DÒNG NÀY

    delay_ms(10);

    ADC1->CR2 |= (1 << 0);        // ADON
    delay_ms(1);
    // B?t ADC l?n 1
    ADC1->CR2 |= (1 << 0);
    for (int i = 0; i < 1000; i++);

    // B?t ADC l?n 2 (quan tr?ng STM32F1)
    ADC1->CR2 |= (1 << 0);

    // Enable SWSTART
    ADC1->CR2 |= (1 << 20);

    // Reset calibration
    ADC1->CR2 |= (1 << 3);
    while (ADC1->CR2 & (1 << 3));

    // Calibration
    ADC1->CR2 |= (1 << 2);
    while (ADC1->CR2 & (1 << 2));
}

// ================= READ ADC =================
uint16_t ADC_Read(uint8_t channel)
{
    // ch?n 1 conversion
    ADC1->SQR1 &= ~(0xF << 20);
    ADC1->SQR3 = channel;

    // clear EOC
    ADC1->SR &= ~(1 << 1);

    // start
    ADC1->CR2 |= (1 << 22);

    // ch? xong
    while (!(ADC1->SR & (1 << 1)));

    return ADC1->DR;
}
