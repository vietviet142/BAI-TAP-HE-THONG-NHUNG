#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "misc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RX_BUFFER_SIZE 64

/*  BIEN TOAN CUC  */
static volatile char     rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t rx_index      = 0;
static volatile uint8_t  message_ready = 0;

/* trang thai LED va muc PWM duoc luu doc lap voi nhau
 * - led_status : LED dang BAT hay TAT
 * - current_pwm: muc do sang (%) duoc CAU HINH gan nhat,
 *                khong nhat thiet la muc dang xuat ra thuc te
 */
static uint8_t  led_status  = 0;    // 0: OFF, 1: ON
static uint16_t current_pwm = 100;  // 0 - 100 (%), mac dinh sang 100% khi chua tung cau hinh PWM

/*  KHOI TAO TIM2 - CH1 (PA0) LAM PWM  */
void TIM2_PWM_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* PA0 = TIM2_CH1, output alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* APB1 timer clock = 72MHz (khi APB1 prescaler = 2, TIMxCLK = 2*PCLK1)
     * f_PWM = 72MHz / (71+1) / (999+1) = 1kHz */
    TIM_TimeBaseStructure.TIM_Period        = 999;
    TIM_TimeBaseStructure.TIM_Prescaler     = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;      // bat dau voi duty = 0%
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

/*  KHOI TAO USART1 (PA9-TX, PA10-RX) VOI NGAT RX  */
void UART1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

    /* PA9 - TX1 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 - RX1 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = 9600;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/*  GUI 1 BYTE (CHO CO TXE DE TRANH TREO CHIP)  */
void UART1_SendByte(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)c);
}

void UART1_SendString(const char *s)
{
    while (*s) {
        UART1_SendByte(*s++);
    }
}

/*  NGAT USART1 RX 
 * Gom ky tu vao rx_buffer cho den khi gap '!'.
 * Khong reset rx_index tai day de tranh xung dot voi vong lap
 * main dang doc rx_buffer khi message_ready = 1 (xem main()).
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char c = (char)USART_ReceiveData(USART1);

        if (message_ready) {
            /* lenh truoc chua duoc main() xu ly xong, bo qua ky tu moi
             * de khong ghi de len du lieu dang cho xu ly */
            return;
        }

        if (c == '!') {
            rx_buffer[rx_index] = '\0';
            message_ready = 1;      // bao cho main() co lenh moi
            /* KHONG reset rx_index o day, main() se lam sau khi xu ly xong */
        } else if (c != '\r' && c != '\n') {
            if (rx_index < (RX_BUFFER_SIZE - 1)) {
                rx_buffer[rx_index++] = c;
            }
        }
    }
}

/*  AP DUNG RA PHAN CUNG (PWM) 
 * Chi khi led_status == 1 thi CCR moi duoc set theo current_pwm.
 * Neu dang OFF, CCR luon = 0 bat ke current_pwm la bao nhieu
 * -> thoa yeu cau: doi PWM luc OFF chi thay doi cau hinh,
 *    khong lam sang den, cho den khi co lenh ON.
 */
void Apply_LED_Hardware(void)
{
    if (led_status == 1) {
        uint16_t ccr_val = (uint16_t)((uint32_t)current_pwm * 1000 / 100); // 0-100% -> 0-999
        if (ccr_val > 999) ccr_val = 999;
        TIM_SetCompare1(TIM2, ccr_val);
    } else {
        TIM_SetCompare1(TIM2, 0);
    }
}

/*  XU LY LENH LOGIC  */
void Process_Command(char *cmd)
{
    if (strcmp(cmd, "ON") == 0) {
        led_status = 1;
Apply_LED_Hardware();       // sang lai theo current_pwm gan nhat
        UART1_SendString("OK: LED IS ON\r\n");
    }
    else if (strcmp(cmd, "OFF") == 0) {
        led_status = 0;
        Apply_LED_Hardware();       // tat den, current_pwm van duoc giu nguyen
        UART1_SendString("OK: LED IS OFF\r\n");
    }
    else if (strncmp(cmd, "PWM:", 4) == 0) {
        char *p = cmd + 4;
        int   i;
        int   len;
        int   valid;
        /* kiem tra chuoi so hop le truoc khi doi (atoi khong bao loi) */
        len   = strlen(p);
        valid = (len > 0);
        for (i = 0; i < len; i++) {
            /* cho phep dau '%' o cuoi chuoi, con lai phai la chu so */
            if (!( (p[i] >= '0' && p[i] <= '9') || (p[i] == '%' && i == len - 1) )) {
                valid = 0;
                break;
            }
        }

        if (valid) {
            int percent = atoi(p);
            if (percent >= 0 && percent <= 100) {
                current_pwm = (uint16_t)percent;
                Apply_LED_Hardware();   // neu dang OFF, ham nay se giu CCR = 0
                UART1_SendString("OK: PWM UPDATED\r\n");
            } else {
                UART1_SendString("ERROR: PWM OUT OF RANGE\r\n");
            }
        } else {
            UART1_SendString("ERROR: INVALID PWM FORMAT\r\n");
        }
    }
    else if (strcmp(cmd, "Status") == 0) {
        char msg[48];
        snprintf(msg, sizeof(msg), "STATUS: LED=%s, PWM=%u%%\r\n",
                 led_status ? "ON" : "OFF", current_pwm);
        UART1_SendString(msg);
    }
    else {
        UART1_SendString("ERROR: UNKNOWN COMMAND\r\n");
    }
}

/*  MAIN  */
int main(void)
{
    volatile int i;

    TIM2_PWM_Init();
    UART1_Init();

    /* delay ngan cho UART/PWM on dinh truoc khi bao san sang */
    for (i = 0; i < 500000; i++);
    UART1_SendString("SYSTEM READY!\r\n");

    while (1) {
        if (message_ready) {
            /* rx_buffer da duoc chot chuoi ket thuc bang '\0' trong ISR */
            Process_Command((char *)rx_buffer);

            /* xu ly xong moi cho phep ISR ghi de va nhan lenh tiep theo */
            rx_index      = 0;
            message_ready = 0;
        }
    }
}