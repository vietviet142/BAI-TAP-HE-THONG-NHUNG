#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "core_cm3.h"

// Khai báo biến toàn cục dùng cho đếm ngắt
volatile uint32_t counter_10Hz = 0;
volatile uint32_t counter_1Hz = 0;
volatile uint32_t counter_0_1Hz = 0;

// Khai báo hàm
void GPIO_Config(void);
void SysTick_Config_1ms(void);

int main(void)
{
    // 1. Cấu hình GPIO cho các chân LED (PA0, PA1, PA2)
    GPIO_Config();
    
    // 2. Cấu hình SysTick ngắt mỗi 1ms
    SysTick_Config_1ms();
    
    while (1)
    {
        // CPU rảnh rỗi, mọi việc nháy LED được xử lý ngầm trong ngắt
    }
}


//Cấu hình chân xuất tín hiệu cho 3 LED
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Chỉ cần cấp xung nhịp cho PORT A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // Cấu hình đồng thời 3 chân PA0, PA1, PA2 là Output Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // Tắt cả 3 LED khi khởi động bằng cách kéo mức logic về 0
    GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
}

//Cấu hình SysTick tạo ngắt mỗi 1ms (1000Hz)
void SysTick_Config_1ms(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    { 
        while (1); // Treo máy nếu cấu hình lỗi
    }
}

void SysTick_Handler(void)
{
    counter_10Hz++;
    counter_1Hz++;
    counter_0_1Hz++;
    
    //  Xử lý LED 1: 10Hz (PA0) 
    // Chu kỳ 0.1s = 100ms -> Sáng 50ms, tắt 50ms
    if (counter_10Hz >= 50) 
    {
        GPIOA->ODR ^= GPIO_Pin_0; // Đảo trạng thái chân PA0
        counter_10Hz = 0;
    }
    
    //  Xử lý LED 2: 1Hz (PA1) 
    // Chu kỳ 1s = 1000ms -> Sáng 500ms, tắt 500ms
    if (counter_1Hz >= 500) 
    {
        GPIOA->ODR ^= GPIO_Pin_1; // Đảo trạng thái chân PA1
        counter_1Hz = 0;
    }
    
    //  Xử lý LED 3: 0.1Hz (PA2) 
    // Chu kỳ 10s = 10000ms -> Sáng 5000ms, tắt 5000ms
    if (counter_0_1Hz >= 5000) 
    {
        GPIOA->ODR ^= GPIO_Pin_2; // Đảo trạng thái chân PA2
        counter_0_1Hz = 0;
    }
}