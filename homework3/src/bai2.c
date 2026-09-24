#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

// Ð?nh nghia chân CS (Chip Select / Load)
#define MAX7219_CS_PORT GPIOA
#define MAX7219_CS_PIN  GPIO_Pin_4

#define MAX7219_CS_LOW()  GPIO_ResetBits(MAX7219_CS_PORT, MAX7219_CS_PIN)
#define MAX7219_CS_HIGH() GPIO_SetBits(MAX7219_CS_PORT, MAX7219_CS_PIN)

// Các thanh ghi c?u hình c?a MAX7219
#define MAX7219_REG_NOOP        0x00
#define MAX7219_REG_DIGIT0      0x01
#define MAX7219_REG_DIGIT1      0x02
#define MAX7219_REG_DIGIT2      0x03
#define MAX7219_REG_DIGIT3      0x04
#define MAX7219_REG_DIGIT4      0x05
#define MAX7219_REG_DIGIT5      0x06
#define MAX7219_REG_DIGIT6      0x07
#define MAX7219_REG_DIGIT7      0x08
#define MAX7219_REG_DECODEMODE  0x09
#define MAX7219_REG_INTENSITY   0x0A
#define MAX7219_REG_SCANLIMIT   0x0B
#define MAX7219_REG_SHUTDOWN    0x0C
#define MAX7219_REG_DISPLAYTEST 0x0F

// Hình trái tim hi?n th? trên LED matrix 8x8 (Font 8 byte)
const uint8_t HEART_PATTERN[8] = {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
};

// Hàm Delay don gi?n b?ng SysTick
void Delay_ms(uint32_t ms)
{
    SysTick->LOAD = (72000000 / 8 / 1000) * ms;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

// Hàm g?i 1 byte qua SPI1
uint8_t SPI1_SendByte(uint8_t byte)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, byte);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

// C?u hình SPI1 (Master mode)
void SPI1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    // PA5 (SCK), PA7 (MOSI) -> Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA4 (CS) -> Output Push-Pull
    GPIO_InitStructure.GPIO_Pin = MAX7219_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MAX7219_CS_PORT, &GPIO_InitStructure);

    MAX7219_CS_HIGH(); // Kh?i t?o CS ? m?c cao

    // C?u hình thông s? SPI1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // 72MHz/16 = 4.5MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

// G?i 1 gói tin 16-bit (Address + Data) t?i MAX7219
void MAX7219_Write(uint8_t address, uint8_t data)
{
    MAX7219_CS_LOW();
    SPI1_SendByte(address);
    SPI1_SendByte(data);
    MAX7219_CS_HIGH();
}

// Kh?i t?o các thanh ghi c?u hình MAX7219
void MAX7219_Init(void)
{
    MAX7219_Write(MAX7219_REG_DISPLAYTEST, 0x00); // T?t ch? d? test
    MAX7219_Write(MAX7219_REG_DECODEMODE,  0x00); // T?t decode (dùng No Decode cho Matrix 8x8)
    MAX7219_Write(MAX7219_REG_SCANLIMIT,   0x07); // Hi?n th? d? 8 hàng (Digit 0-7)
    MAX7219_Write(MAX7219_REG_INTENSITY,   0x03); // Ð? sáng trung bình (0x00 d?n 0x0F)
    MAX7219_Write(MAX7219_REG_SHUTDOWN,    0x01); // B?t IC ho?t d?ng
}

// Hi?n th? m?ng d? li?u 8 byte lên Ma tr?n LED
void MAX7219_DisplayPattern(const uint8_t *pattern)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        MAX7219_Write(i + 1, pattern[i]);
    }
}

// Xóa màn hình LED
void MAX7219_Clear(void)
{
    uint8_t i;
    for (i = 1; i <= 8; i++)
    {
        MAX7219_Write(i, 0x00);
    }
}

int main(void)
{
    SystemInit();
    
    SPI1_Config();
    MAX7219_Init();
    MAX7219_Clear();

    while (1)
    {
        // Hi?n th? hình trái tim
        MAX7219_DisplayPattern(HEART_PATTERN);
        Delay_ms(1000);

        // Nh?p nháy b?ng cách xóa màn hình
        MAX7219_Clear();
        Delay_ms(1000);
    }
}