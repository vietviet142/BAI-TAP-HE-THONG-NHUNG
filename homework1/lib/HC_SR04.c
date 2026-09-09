#include "HC_SR04.h"
#include "GPIO.h"
// PA8 = ECHO (input pull-up)
// PA9 = TRIG (output)



void GPIO_Init_HC_SR04(void) {
    RCC->APB2ENR |= (1 << 2);   // b?t clock GPIOA

    // PA8: input pull-up (CNF=10, MODE=00)
    // PA9: output push-pull 50MHz (CNF=00, MODE=11)
    GPIOA->CRH &= ~(0x000000FF);
    GPIOA->CRH |=  (0x00000038);   // PA8=0x8, PA9=0x30

          // pull-up cho PA8
    GPIOA->BRR  = (1 << 8);        // TRIG = 0 ban d?u
}

/* ---- Ð?c HC-SR04 ---- */
uint32_t HCSR04_Read(void) {
    uint32_t time_us = 0;

    // phát xung TRIG
    GPIOA->BRR  = (1 << 9);
    delay_us(2);
    GPIOA->BSRR = (1 << 9);
    delay_us(10);
    GPIOA->BRR  = (1 << 9);
    // ch? ECHO lên
    while (!(GPIOA->IDR & (1 << 8)));
    // do th?i gian ECHO
    while (GPIOA->IDR & (1 << 8)) {
        delay_us(1);
        time_us++;
    }
    return time_us;
}




/* ---- Main ---- */
/*
float distance;
uint32_t duration ;

int main(void) {

	
    GPIO_Init_All();
    while (1) {
        duration = HCSR04_Read();
        distance = (float)duration / 58.0f;   // cm
        // ch? do, không di?u khi?n gì n?a
        delay_ms(60);
    }
}

*/
