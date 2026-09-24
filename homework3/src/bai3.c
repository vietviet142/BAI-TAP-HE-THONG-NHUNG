#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include <stdint.h>

volatile uint32_t button_count = 0;
volatile uint8_t button_event = 0;

static uint8_t dma_buffer[64];

// --- HÀM DELAY ĐƠN GIẢN (Tránh phụ thuộc SysTick) ---
void Delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 7200; i++)
    {
        __NOP();
    }
}

// --- CẤU HÌNH CLOCK 72MHZ ---
void Clock_Config_72MHz(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    
    if (RCC_WaitForHSEStartUp() == SUCCESS)
    {
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);
        
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08);
    }
}

// --- HÀM XỬ LÝ CHUỖI ---
uint32_t String_Append(char *dest, uint32_t index, const char *src)
{
    uint32_t i = 0;
    while (src[i] != '\0')
    {
        dest[index++] = src[i++];
    }
    return index;
}

uint32_t UInt_To_String(uint32_t value, char *buffer)
{
    char temp[12];
    uint32_t i = 0;
    uint32_t j = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    while (value > 0)
    {
        temp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0)
    {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';

    return j;
}

// --- TẠO VÀ GỬI MESSAGE QUA DMA ---
void Create_Message(uint32_t value)
{
    uint32_t index = 0;
    char number[12];

    // Chờ DMA gửi xong bản tin trước (nếu đang gửi)
    while (DMA1_Channel4->CNDTR != 0);

    index = String_Append((char *)dma_buffer, index, "C03G04:BTN:");
    UInt_To_String(value, number);
    index = String_Append((char *)dma_buffer, index, number);

    dma_buffer[index++] = '\n';
    dma_buffer[index++] = '\r';

    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_GL4 | DMA1_FLAG_TC4 | DMA1_FLAG_HT4 | DMA1_FLAG_TE4);
    
    DMA1_Channel4->CMAR = (uint32_t)dma_buffer;
    DMA1_Channel4->CNDTR = index;
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

// --- KHỞI TẠO PHẦN CỨNG ---
void Hardware_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. Cấp xung nhịp
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // 2. Cấu hình NVIC Group
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // 3. Cấu hình GPIO (Mặc định PA0: IPU cho nút ngoài nối GND)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA9: USART1 TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. Cấu hình USART1 (115200, 8N1)
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);

    // 5. Cấu hình DMA1 Channel 4 cho USART1_TX
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dma_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0; 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // 6. Cấu hình EXTI Line 0 (Cạnh xuống)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line0);

    // 7. Bật ngắt EXTI0 trong NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// --- NGẮT EXTI0 ---
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        button_event = 1; 
        EXTI_ClearITPendingBit(EXTI_Line0); 
    }
}

// --- HÀM MAIN ---
int main(void)
{
    Clock_Config_72MHz();
    Hardware_Init();


    while (1)
    {
        if (button_event)
        {
            button_event = 0; // Xóa cờ ngắt
            
            Delay_ms(20); // Chống dội phím (Debounce)

            // Kiểm tra nút có ĐANG ĐƯỢC NHẤN không (Mức LOW đối với IPU)
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
            {
                
                Create_Message(button_count);
				button_count++;
                // Chờ cho đến khi người dùng nhả tay ra hoàn toàn
                while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET);
                
                Delay_ms(20); // Chống dội khi nhả
                
                // Xóa cờ ngắt rác phát sinh trong lúc giữ/nhả nút
                EXTI_ClearITPendingBit(EXTI_Line0);
                button_event = 0;
            }
        }
    }
}