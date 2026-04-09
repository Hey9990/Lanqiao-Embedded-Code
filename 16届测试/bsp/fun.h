#ifndef __FUN_H__
#define __FUN_H__

#include "stm32g4xx.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

#include "main.h"
#include "gpio.h"
 #include "tim.h"
 #include "adc.h"
 #include "usart.h"
// #include "rtc.h"

#include "fun.h"
#include "lcd.h"
// #include "i2c.h"

void led_Disp(unsigned char ucled);
void key_scan(void);
void lcd_show(void);
void uart_data_rec(void);
double get_vol(ADC_HandleTypeDef *hadc);
void fun0(void);
void fun_led(void);

extern uint8_t test;
 extern  uint8_t rx_data;


// typedef struct {
//     int id;
//     char name[50];
//     float score;
// } Student;
// extern Student top_student;

//'0'=48

#endif
