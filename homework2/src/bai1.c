#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

//  Cấu hình các thông số 
#define MAX_BUFFER_SIZE 256
#define CLASS_CODE      "HETHONGNHUNG"   // Thay đổi Mã Lớp của bạn ở đây
#define GROUP_CODE      "N03"   // Thay đổi Mã Nhóm của bạn ở đây

//  Khai báo biến toàn cục 
char rx_buffer[MAX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t msg_ready = 0;

// Khai báo hàm 
void UART1_Config(void);
void UART1_SendChar(char c);
void UART1_SendString(char* str);

int main(void)
{
    // Khởi tạo UART1
    UART1_Config();
    
    while (1)
    {
        // Kiểm tra xem đã nhận được ký tự '!' chưa
        if (msg_ready == 1)
        {
            // Gửi Mã lớp, Mã nhóm: 
            UART1_SendString(CLASS_CODE);
            UART1_SendString(GROUP_CODE);
            UART1_SendString(": ");
            
            // Gửi Bản tin đã nhận từ PC
            UART1_SendString(rx_buffer);
            
            // Gửi ký tự xuống dòng \n\r
            UART1_SendString("\n\r");
            
            // Reset lại bộ đệm và cờ báo để sẵn sàng nhận bản tin mới
            rx_index = 0;
            msg_ready = 0;
            
            // Xóa bộ đệm để đảm bảo an toàn cho lần nhận sau
            for(int i = 0; i < MAX_BUFFER_SIZE; i++) {
                rx_buffer[i] = '\0';
            }
        }
    }
}


// Cấu hình USART1 (TX: PA9, RX: PA10), Baudrate 115200, Ngắt RX
void UART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. Cấp xung nhịp cho USART1, GPIOA và AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // 2. Cấu hình chân GPIO
    // Cấu hình chân TX (PA9) - Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cấu hình chân RX (PA10) - Input Floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. Cấu hình các thông số USART1
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 4. Cấu hình Ngắt (Interrupt) cho USART1 khi nhận dữ liệu (RXNE)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 5. Cấu hình NVIC (Nested Vectored Interrupt Controller)
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 6. Cho phép USART1 hoạt động
    USART_Cmd(USART1, ENABLE);
}

//  Hàm gửi 1 ký tự qua UART1
void UART1_SendChar(char c)
{
    // Chờ đến khi thanh ghi truyền dữ liệu trống
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

//Hàm gửi 1 chuỗi ký tự qua UART1
void UART1_SendString(char* str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}

void USART1_IRQHandler(void)
{
    // Kiểm tra xem ngắt có phải do nhận dữ liệu không
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // Đọc dữ liệu nhận được
        char received_char = (char)USART_ReceiveData(USART1);
        
        // Kiểm tra ký tự kết thúc '!'
        if (received_char == '!')
        {
            rx_buffer[rx_index] = '\0'; // Kết thúc chuỗi
            msg_ready = 1;              // Bật cờ báo hiệu đã nhận xong
        }
        else
        {
            // Nếu chưa nhận được '!', tiếp tục lưu vào buffer
            // Chỉ lưu khi chưa vượt quá kích thước mảng để tránh tràn RAM
            if (rx_index < (MAX_BUFFER_SIZE - 1))
            {
                rx_buffer[rx_index++] = received_char;
            }
        }
        
        // Xóa cờ ngắt (với STM32, lệnh đọc thanh ghi DR ở trên đã tự động xóa cờ ngắt)
        // USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}