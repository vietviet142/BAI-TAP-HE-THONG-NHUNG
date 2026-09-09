#include "stm32f10x.h"

void delay_ms(uint16_t t) {
    int i,j;
		for(i=0;i<t;i++){
			for(j=0;j<=0x2AFF;j++){
			}
		}
}

int main(void){
    // Cap clk cho port C
    RCC->APB2ENR |= 0x00000010; 
		// Cau hinh chan PC13 o che do Output push pull
    GPIOC->CRH &= ~0x00F00000; // Xoa cau hinh cu cua chan PC13
    GPIOC->CRH |= 0x00300000; // Mode Output push pull, 50MHZ
    while (1){
        GPIOC->ODR ^= (1<<13); // Ðao trang thai chan PC13
        delay_ms(1000); // delay 1000ms = 1s
    }
}