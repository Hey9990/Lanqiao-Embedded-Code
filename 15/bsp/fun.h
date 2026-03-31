#ifndef __FUN_H__
#define __FUN_H__

#include "stm32g4xx.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"

#include "main.h"
#include "gpio.h"
// #include "tim.h"
// #include "adc.h"
// #include "usart.h"
// #include "rtc.h"

#include "fun.h"
#include "lcd.h"
// #include "i2c.h"

void led_Disp(unsigned char ucled);
void key_scan(void);
void lcd_show(void);
void fre(void);
void fmaxmini(void);

//uint32_t fre_A,fre_B;
//extern uint16_t max_fA , mini_fA,max_fB , mini_fB;

#endif
