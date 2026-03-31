#ifndef __FUN_H__
#define __FUN_H__

#include "stm32g4xx.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

#include "main.h"
#include "gpio.h"
 #include "tim.h"
// #include "adc.h"
 #include "usart.h"
// #include "rtc.h"

#include "fun.h"
#include "lcd.h"
#include "i2c_hal.h"

void led_Disp(unsigned char ucled);
void key_scan(void);
void lcd_show(void);
void uart_data_rec(void);
void pwm_set_duty(float Duty);
void yes_shop(void);
void led0(void);
void  eeprom_init(void);

extern uint8_t test;
extern uint8_t rx_data;
extern uint8_t usart_test[20];

#endif
