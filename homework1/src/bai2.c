#include "stm32f10x.h"
uint16_t hieuung[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
void delay_ms(uint16_t t) {
    int i,j;
		for(i=0;i<t;i++){
			for(j=0;j<=0x2AFF;j++){
			}
		}
}

int main(void){
		int i;
    // Cap clk cho port A
    RCC->APB2ENR |= 0x00000004; 
		// Cau hinh chan PA0-PA7 o che do Output push pull 50MHZ
    GPIOA->CRL &= ~0xFFFFFFFF; // Xoa cau hinh cu cua cac chan PA0-PA7
    GPIOA->CRL |= 0x33333333; // Mode Output push pull, 50MHZ
    while (1){
				// hieu ung chay tu trai sang phai
        for(i=0;i<8;i++){
					GPIOA->ODR = hieuung[i];
					delay_ms(100);
				}
				delay_ms(100);
				// hieu ung chay tu phai sang trai
				for(i=7;i>=0;i--){
					GPIOA->ODR = hieuung[i];
					delay_ms(100);
				}
				delay_ms(100);
    }
}