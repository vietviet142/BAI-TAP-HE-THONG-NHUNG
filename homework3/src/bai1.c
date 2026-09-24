#include "stm32f10x.h"
#include "ssd1306.h"

int main(void)
{
    /* SystemInit() da duoc goi tu dong boi startup file truoc khi vao main(),
       cau hinh clock he thong (thuong 72MHz neu board dung HSE 8MHz). */

    SSD1306_Init();
    SSD1306_DrawTestPattern();

    while (1)
    {
        /* Man hinh da hien thi anh tinh (khung + duong cheo + hinh tron).
           Neu muon hien anh thuc te cua ban, thay dong tren bang:
             SSD1306_DrawBitmap(my_bitmap);
             SSD1306_UpdateScreen();
           voi my_bitmap la mang 1024 byte xuat tu cong cu image2cpp. */
    }
}
