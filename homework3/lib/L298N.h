#ifndef _L298N
#define _L298N
#include "stm32f10x.h"

#define cycle 1000
#define ENA GPIO_Pin_10
#define IN1 GPIO_Pin_1
#define IN2 GPIO_Pin_0
#define ENB GPIO_Pin_9 
#define IN3 GPIO_Pin_8
#define IN4 GPIO_Pin_7

void pwm(int duty);
void pwm_dutyAB(int dutyA,int dutyB);
void GPIO_L298N_Config(void);
void motorA_forward();
void motorA_backward();
void motorA_stop();
void motorB_forward();
void motorB_backward();
void motorB_stop();
void motor_enable();
void turn_right();
void turn_left();
void right_tend();
void left_tend();
void forward_tend();
void backward_tend();
#endif