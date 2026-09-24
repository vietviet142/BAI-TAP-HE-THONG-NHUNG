#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

#define BUFFER_SIZE 500
uint16_t adc_buffer[BUFFER_SIZE];
volatile uint8_t ht_flag = 0;
volatile uint8_t tc_flag = 0;

// 1. Hàm g?i chu?i UART
void UART_SendString(char *str)
{
    while (*str)
    {
        USART_SendData(USART1, *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}

// 2. Hàm g?i s? nguyên ADC siêu t?c (không làm ngh?n UART)
void UART_SendNumber(uint16_t num)
{
    char str[6];
    int i = 0;
    if (num == 0) {
        USART_SendData(USART1, '0');
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, '\r');
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, '\n');
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        return;
    }
    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        USART_SendData(USART1, str[--i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    USART_SendData(USART1, '\r');
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, '\n');
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

// 3. C?u hình UART1 (TX: PA9, RX: PA10)
void UART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

// 4. C?u hình Timer3 phát TRGO ? 100Hz

void TIM3_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Clock TIM3 = 72MHz / (719 + 1) = 100kHz
    // Period = 999 -> T?n s? ng?t/TRGO = 100kHz / 1000 = 100Hz (10ms/m?u)
    TIM_TimeBaseStructure.TIM_Prescaler = 719;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // B?t xu?t xung Trigger TRGO khi Timer Update
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
}

// 5. C?u hình ADC1 + DMA1 Ch1 + Ng?t HT/TC
void ADC1_DMA_Config(void){
		GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // PA0 -> ADC Ch0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // DMA1 Channel1 Configuration
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    // B?t ng?t Half-Transfer và Transfer-Complete
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // NVIC DMA Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // ADC1 Configuration
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // T?t liên t?c, dùng Trigger ph?n c?ng
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // Kích b?ng Timer3
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    // Hi?u chu?n ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

// 6. Trình ph?c v? ng?t DMA1 Channel 1 (B?T BU?C gi? attribute used)

__attribute__((used)) void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET)
    {
        ht_flag = 1; 
        DMA_ClearITPendingBit(DMA1_IT_HT1); 
    }
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        tc_flag = 1; 
        DMA_ClearITPendingBit(DMA1_IT_TC1); 
    }
}
int main(void)
{
    int i;
    SystemInit();
    SystemCoreClockUpdate(); 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    UART1_Config();
    UART_SendString("KHOI DONG THANH CONG! DU LIEU SE HIEN THI NGAY SAU DAY:\r\n");
    TIM3_Config();
    ADC1_DMA_Config();
    // B?t Timer3 ch?y

    TIM_Cmd(TIM3, ENABLE); 
    while (1)
    {
        // X? 250 m?u kh?i d?u tiên
        if (ht_flag == 1)
        {
            ht_flag = 0; 
            for (i = 0; i < BUFFER_SIZE / 2; i++)
            {
                UART_SendNumber(adc_buffer[i]); 

            }
        }
        // X? 250 m?u kh?i ti?p theo
        if (tc_flag == 1)
        {
            tc_flag = 0; 
            for (i = BUFFER_SIZE / 2; i < BUFFER_SIZE; i++)
            {
                UART_SendNumber(adc_buffer[i]); 
            }
        }
    }
}