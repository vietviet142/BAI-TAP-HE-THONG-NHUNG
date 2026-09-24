#include "ssd1306.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include <string.h>
#include <math.h>

#define SSD1306_I2C          I2C1
#define SSD1306_I2C_ADDR     0x78    /* 0x3C << 1 */
#define I2C_TIMEOUT          100000  /* Chống treo loop */

static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_PAGES];

/* Cấu hình phần cứng I2C1: SCL = PB6, SDA = PB7 */
static void SSD1306_I2C_LowLevel_Init(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef  i2c;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    I2C_DeInit(SSD1306_I2C);
    i2c.I2C_Mode                = I2C_Mode_I2C;
    i2c.I2C_DutyCycle           = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1         = 0x00;
    i2c.I2C_Ack                 = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed          = 100000;
    I2C_Init(SSD1306_I2C, &i2c);

    I2C_Cmd(SSD1306_I2C, ENABLE);
}

/* Hàm gửi dữ liệu I2C an toàn có Timeout */
static uint8_t SSD1306_I2C_WriteBytes(uint8_t control, const uint8_t *data, uint16_t len)
{
    uint32_t timeout = I2C_TIMEOUT;
    uint16_t i;

    I2C_GenerateSTART(SSD1306_I2C, ENABLE);
    while (!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) return 1;
    }

    timeout = I2C_TIMEOUT;
    I2C_Send7bitAddress(SSD1306_I2C, SSD1306_I2C_ADDR, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if (--timeout == 0) return 1;
    }

    timeout = I2C_TIMEOUT;
    I2C_SendData(SSD1306_I2C, control);
    while (!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if (--timeout == 0) return 1;
    }

    for (i = 0; i < len; i++)
    {
        timeout = I2C_TIMEOUT;
        I2C_SendData(SSD1306_I2C, data[i]);
        while (!I2C_CheckEvent(SSD1306_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if (--timeout == 0) return 1;
        }
    }

    I2C_GenerateSTOP(SSD1306_I2C, ENABLE);
    return 0;
}

static void SSD1306_Delay(volatile uint32_t count)
{
    while (count--);
}

void SSD1306_Init(void)
{
    static const uint8_t init_commands[] = {
        0xAE,       // Display OFF
        0x20, 0x00, // Horizontal Addressing Mode
        0xB0,       // Page Start Address = 0
        0xC8,       // COM Output Scan Direction
        0x00,       // Column Low Nibble
        0x10,       // Column High Nibble
        0x40,       // Display Start Line = 0
        0x81, 0xFF, // Contrast Control (Max)
        0xA1,       // Segment Re-map
        0xA6,       // Normal Display
        0xA8, 0x3F, // Multiplex Ratio (1/64)
        0xA4,       // Output Follows RAM
        0xD3, 0x00, // Display Offset = 0
        0xD5, 0xF0, // Display Clock Divide Ratio
        0xD9, 0x22, // Pre-charge Period
        0xDA, 0x12, // COM Pins Hardware Config
        0xDB, 0x20, // VCOMH Deselect Level
        0x8D, 0x14, // Charge Pump ENABLE
        0xAF        // Display ON
    };

    SSD1306_I2C_LowLevel_Init();
    SSD1306_Delay(1000000);

    SSD1306_I2C_WriteBytes(0x00, init_commands, sizeof(init_commands));

    SSD1306_Clear();
    SSD1306_UpdateScreen();
}

void SSD1306_Fill(uint8_t color)
{
    memset(ssd1306_buffer, color ? 0xFF : 0x00, sizeof(ssd1306_buffer));
}

void SSD1306_Clear(void)
{
    SSD1306_Fill(0);
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    if (color)
        ssd1306_buffer[x + (y / 8) * SSD1306_WIDTH] |=  (1 << (y % 8));
    else
        ssd1306_buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

void SSD1306_DrawBitmap(const uint8_t *bitmap)
{
    memcpy(ssd1306_buffer, bitmap, sizeof(ssd1306_buffer));
}

void SSD1306_UpdateScreen(void)
{
    uint8_t page;
    for (page = 0; page < SSD1306_PAGES; page++)
    {
        uint8_t page_cmd[3];
        page_cmd[0] = (uint8_t)(0xB0 + page);
        page_cmd[1] = 0x00;
        page_cmd[2] = 0x10;
        
        SSD1306_I2C_WriteBytes(0x00, page_cmd, 3);
        SSD1306_I2C_WriteBytes(0x40, &ssd1306_buffer[SSD1306_WIDTH * page], SSD1306_WIDTH);
    }
}

void SSD1306_DrawTestPattern(void)
{
    uint8_t x, y;
    int16_t cx, cy, r, angle;

    SSD1306_Clear();

    for (x = 0; x < SSD1306_WIDTH; x++)
    {
        SSD1306_DrawPixel(x, 0, 1);
        SSD1306_DrawPixel(x, SSD1306_HEIGHT - 1, 1);
    }
    for (y = 0; y < SSD1306_HEIGHT; y++)
    {
        SSD1306_DrawPixel(0, y, 1);
        SSD1306_DrawPixel(SSD1306_WIDTH - 1, y, 1);
    }

    for (x = 0; x < SSD1306_WIDTH; x++)
    {
        uint8_t py = (uint8_t)((uint32_t)x * (SSD1306_HEIGHT - 1) / (SSD1306_WIDTH - 1));
        SSD1306_DrawPixel(x, py, 1);
        SSD1306_DrawPixel(x, SSD1306_HEIGHT - 1 - py, 1);
    }

    cx = SSD1306_WIDTH / 2;
    cy = SSD1306_HEIGHT / 2;
    r  = 20;
    for (angle = 0; angle < 360; angle++)
    {
        float rad = angle * 3.14159265f / 180.0f;
        int16_t px = cx + (int16_t)(r * cosf(rad));
        int16_t py = cy + (int16_t)(r * sinf(rad));
        if (px >= 0 && py >= 0)
            SSD1306_DrawPixel((uint8_t)px, (uint8_t)py, 1);
    }

    SSD1306_UpdateScreen();
}