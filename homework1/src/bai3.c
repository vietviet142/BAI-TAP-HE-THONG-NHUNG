#include "stm32f10x.h"

void delay_ms(uint16_t t) {
    int i,j;
		for(i=0;i<t;i++){
			for(j=0;j<=0x2AFF;j++){
			}
		}
}

// Khong co chan pa13 pa14 nen em xin phep chuyen data doc duoc sang pb0-pb7
int main(void){
		uint16_t input_res, output_res;
    // Cap clk cho port A
    RCC->APB2ENR |= 0x0000000C; 
		// Cau hinh chan PA0-PA7 o che do input pull up
		GPIOA->CRL &= ~0xFFFFFFFF;
		GPIOA->CRL |= 0x88888888;
		
		// KICK-OFF PULL-UP: Ghi bit 1 vao 8 bit thap ODR de kich hoat tro keo len VCC
    GPIOA->ODR |= 0x00FF;
		// cau hinh chan pb0-pb7 che do output push pull 50MHZ
		GPIOB->CRH &= ~0xFFFFFFFF;
    GPIOB->CRH |=  0x33333333; 
		
		while(1){
				// doc 8 bit dau vao tu pa0->pa7
				input_res = (GPIOA->IDR & 0x000000FF);
				// Dao trang thai
				output_res = ((~input_res) & 0x00FF) << 8;
				// Cap nhat 8 bit cao (PB8-PB15) va giu nguyen 8 bit thap cua GPIOB
        GPIOB->ODR = (GPIOB->ODR & 0x00FF) | output_res;
				delay_ms(10);
		}
}