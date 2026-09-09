#include "stm32f10x.h"

void delay_ms(uint16_t t){
    int i,j;
    for(i = 0; i < t; i++){
        for (j = 0; j <= 0x2AFF; j++){
        }
    }
}

int main(void) {
    uint8_t current_state; // bien luu trang thai hien tai cua nut nhan
    uint8_t old_state = 1; // bien luu trang thai truoc do cua nut nhan
    uint8_t led_status = 0; // bien luu trang thai led

    // Cap clock cho Port A
    RCC->APB2ENR |= 0x00000004; 

    // Cau hinh PA0 la input pull up, noi nut nhan va PA1 la chan noi voi led
    GPIOA->CRL &= ~0x000000FF; // Xoa cau hinh cu cua PA0 va PA1
    GPIOA->CRL |=  0x00000038; // PA0 la input pull up va pa1 la outpull push pull toc do 50MHZ
    while(1){
        // Doc gia tri hien tai tai chan PA0
        current_state = (GPIOA->IDR & 0x00000001) ? 1 : 0;

        // Phat hien nut nhan, neu hien tai doc duoc la 1 va truoc do la 0, thi la su kien nhan nut roi nha
        if ((old_state == 0) && (current_state == 1)) {
            led_status = !led_status; // Neu thoa man thi Dao trang thai
            if (led_status){
                GPIOA->BSRR =  (1 << 1); // Bat LED
            } 
						else{
                GPIOA->BRR = (1 << 1); // Tat LED
            }
        }
        old_state = current_state; // Cap nhat trang thai cu
        delay_ms(2);              // Chong doi phim
    }
}