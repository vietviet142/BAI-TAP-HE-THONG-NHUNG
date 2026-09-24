#include "L298N.h"
#include "GPIO.h"

//detect line


// joystick
void pwm(int duty){
	GPIO_SetBits(GPIOB, ENB | ENA);
	delay_ms(duty);
	GPIO_ResetBits(GPIOB, ENA | ENB);
	delay_ms(cycle-duty);
}
void pwm_dutyAB(int dutyA,int dutyB){
	GPIO_SetBits(GPIOB, ENB | ENA);
	if(dutyA>=dutyB){
		delay_ms(dutyB);
		GPIO_ResetBits(GPIOB,ENB);
		delay_ms(dutyA-dutyB);
		delay_us(10);
		GPIO_ResetBits(GPIOB, ENA);
		//delay_ms(cycle-dutyA);
	}
	else{
		delay_ms(dutyA);
		GPIO_ResetBits(GPIOB,ENA);
		delay_ms(dutyB-dutyA);
		delay_us(10);
		GPIO_ResetBits(GPIOB, ENB);
		//delay_ms(cycle-dutyB);
	}
}

void turn_left(){
	motorA_forward();
	motorB_backward();
	pwm(170);
};

void turn_right(){
	motorA_backward();
	motorB_forward();
	pwm(170);
};
void right_tend(){
	motorA_forward();
	motorB_backward();
}
void left_tend(	){
	motorA_backward();
	motorB_forward();
}
void forward_tend(){
	motorA_forward();
	motorB_forward();
}
void backward_tend(){
	motorA_backward();
	motorB_backward();
}

void GPIO_L298N_Config(void) {
	
//		GPIO_Cfg_PinLow_Output(GPIOB,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_7);
//		GPIO_Cfg_PinHigh_Output(GPIOB,GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
//		GPIO_ResetBits(GPIOB, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable clock GPIOB
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // Config PB0,1,6,7,10,11 là output
    GPIO_InitStructure.GPIO_Pin = IN1 | IN2 |
                                  IN3 | IN4 |
                                  ENA | ENB;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// ===== Motor A =====
void motorA_forward() {
    GPIO_SetBits(GPIOB, IN1);
    GPIO_ResetBits(GPIOB, IN2);
}

void motorA_backward() {
    GPIO_ResetBits(GPIOB, IN1);
    GPIO_SetBits(GPIOB, IN2);
}

void motorA_stop() {
    GPIO_ResetBits(GPIOB, IN1 | IN2);
}

// ===== Motor B =====
void motorB_forward() {
    GPIO_SetBits(GPIOB, IN4);
    GPIO_ResetBits(GPIOB, IN3);
}

void motorB_backward() {
    GPIO_ResetBits(GPIOB, IN4);
    GPIO_SetBits(GPIOB, IN3);
}

void motorB_stop() {
    GPIO_ResetBits(GPIOB, IN3 | IN4);
}

// Enable motor
void motor_enable() {
    GPIO_SetBits(GPIOB, ENB | ENA);
}