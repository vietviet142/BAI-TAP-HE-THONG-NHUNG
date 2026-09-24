#ifndef __SSD1306_H
#define __SSD1306_H

#include "stm32f10x.h"

#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64
#define SSD1306_PAGES       (SSD1306_HEIGHT / 8)

void SSD1306_Init(void);
void SSD1306_Fill(uint8_t color);
void SSD1306_Clear(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_DrawBitmap(const uint8_t *bitmap);
void SSD1306_UpdateScreen(void);
void SSD1306_DrawTestPattern(void);

#endif /* __SSD1306_H */
