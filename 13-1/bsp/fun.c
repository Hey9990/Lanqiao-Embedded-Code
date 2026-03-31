#include "fun.h"

uint8_t test;       //用于测试
uint8_t lcd_text[25];
uint8_t lcd_page=0;	//LCD分页

uint8_t B1_state;
uint8_t B2_state;
uint8_t B3_state;
uint8_t B4_state;
uint8_t B1_last_state=1;//没被按下时，默认高电平
uint8_t B2_last_state=1;
uint8_t B3_last_state=1;
uint8_t B4_last_state=1;


/* USART专用变量 */
 uint8_t rt_buff[20];     //所发送的数据
 uint8_t rx_data,rx_count;   //接收的单个数据，已接收的数据个数
 uint8_t rx_flag;    //正在接收中断标志，接收不定长数据
 uint8_t rx_buff[20];//所接收的不定长数据

uint8_t B1='1';//三位密码数值
uint8_t B2='2';
uint8_t B3='3';
uint8_t B1at=1;//是否处于@加密状态1是 0否
uint8_t B2at=1;
uint8_t B3at=1;

uint16_t F;	//频率
uint8_t D;	//占空比

uint8_t B1_t=9;//三位密码数值
uint8_t B2_t=9;
uint8_t B3_t=9;


__IO uint32_t uwTick_Lcd_ture_Point=0;
uint8_t lcd_ture_i=0;
__IO uint32_t uwTick_Led_false_Point=0;
uint8_t pass_err;
uint8_t Led_false_i=0;
uint8_t start;

/**************************************** 分割线 ****************************************/



void key_scan(void)	//此函数添加到while中
{
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

	if(B1_state == 0 && B1_last_state ==1)		
	{
		B1at=0;
		if(++B1_t>=10) B1_t=0;
	}
	if(B2_state ==0 && B2_last_state ==1)	
	{
		B2at=0;
		if(++B2_t>=10) B2_t=0;
	}
	if(B3_state ==0 && B3_last_state ==1)
	{
		B3at=0;
		if(++B3_t>=10) B3_t=0;
	}
	if(B4_state ==0 && B4_last_state ==1)
	{
		if(B1==(B1_t+'0') && (B2==B2_t+'0') && B3==(B3_t+'0'))
		{
			lcd_page=1;
		}
		else 
		{
			pass_err++;
				B1at=1;
				B2at=1;
				B3at=1;
				B1_t=9;
				B2_t=9;
				B3_t=9;
		}
	}
	B1_last_state = B1_state;
	B2_last_state = B2_state;
	B3_last_state = B3_state;
	B4_last_state = B4_state;
}

void lcd_show(void)
{

	switch(lcd_page)
	{
		case 0:
		{
			LCD_DisplayStringLine(Line1,(uint8_t *)"       PSD          ");
			
			if(B1at)	
			{
				LCD_DisplayStringLine(Line3,(uint8_t *)"    B1:@            ");
			}
			else
			{
				sprintf((char *)lcd_text,"    B1:%1d            ",B1_t);
				LCD_DisplayStringLine(Line3,lcd_text);
			}
			
			if(B2at)	
			{
				LCD_DisplayStringLine(Line4,(uint8_t *)"    B2:@            ");
			}
			else
			{
			sprintf((char *)lcd_text,"    B2:%1d            ",B2_t);
			LCD_DisplayStringLine(Line4,lcd_text);
			}
			
			if(B3at)	
			{
				LCD_DisplayStringLine(Line5,(uint8_t *)"    B3:@            ");
			}
			else
			{
			sprintf((char *)lcd_text,"    B3:%1d            ",B3_t);
			LCD_DisplayStringLine(Line5,lcd_text);
			}
			
			
		}
		break;

		default:
		{
			lcd_true();
			
			LCD_DisplayStringLine(Line1,(uint8_t *)"       STA          ");
			sprintf((char *)lcd_text,"    F:%1dHz           ",F);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"    D:%1d%%            ",D);
			LCD_DisplayStringLine(Line4,lcd_text);

			LCD_DisplayStringLine(Line5,(uint8_t *)"                    ");
		}
		break;
	}
}

void lcd_true(void)
{
	if((uwTick - uwTick_Lcd_ture_Point)<100) return;//0.1S
	uwTick_Lcd_ture_Point =uwTick;
	if(++lcd_ture_i >=50)
	{
		B1at=1;
		B2at=1;
		B3at=1;
		B1_t=9;
		B2_t=9;
		B3_t=9;
		TIM2->PSC = 80000000/(100000);
		TIM2->CCR2=test;
		
		lcd_page=0;
		lcd_ture_i=0;
	}
	else if(lcd_ture_i==1)
	{	
		F=2000;
		D=10;
		test=TIM2->CCR2;
		TIM2->PSC = 80000000/(200000);
		TIM2->CCR2 = (TIM2->ARR + 1) * (10 / 100.0f);
	}
}

void led_start(void)							//LED闪烁
{
	
	if(lcd_ture_i && pass_err>=3)
	{
		if((uwTick - uwTick_Led_false_Point)<100) return;//0.1S
		uwTick_Led_false_Point =uwTick;
		if(++Led_false_i>=50)
		{
			Led_false_i=0;
			led_Disp(0x01);
			pass_err=0;
		}
		if(Led_false_i % 2) led_Disp(0x03);
		else	led_Disp(0x01);
		
	}
	else if(lcd_ture_i )
	{
		led_Disp(0x01);
	}
	else if(pass_err >=3)
	{
		if((uwTick - uwTick_Led_false_Point)<100) return;//0.1S
		uwTick_Led_false_Point =uwTick;
		if(++Led_false_i>=50)
		{
			Led_false_i=0;
			led_Disp(0x00);
			pass_err=0;
		}
		if(Led_false_i % 2) led_Disp(0x02);
		else	led_Disp(0x00);
	}
	else
	{
		led_Disp(0x00);
	}
}



/** USART串口发送和接收
*   两引脚设置：PA10(USART1_RX)	PA9(USART1_TX)
*   USART1配置：异步（Mode:Asynchronous）  
*               修改波特率（Parameter Settings ：Baud Rate  9600）
*   打开中断使能
*   main()添加发送中断使能：HAL_UART_Receive_IT(&huart1,&rec_data,1);
*
*   不定长数据接收设置：
*   添加一个不中断TIM，PSC(7999), main()添加HAL_TIM_Base_Start(&htim4);
*/
//{   //串口发送使用方法(执行慢，不要放到串口中断)
//    sprintf(usart_test,"HelloWorld\r\n");
//    HAL_UART_Transmit(&huart1,(uint8_t *)usart_test,sizeof(usart_text),50);	//(串口号,内容,大小,限时)
//}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)	//串口接收回调函数***************************
{
	if(huart->Instance == USART1)
	{	
				B1at=1;
				B2at=1;
				B3at=1;
				B1_t=9;
				B2_t=9;
				B3_t=9;
	
		TIM4->CNT=0;			//计数器清零
		rx_flag=1;				//进入中断打开
		rx_buff[rx_count]=rx_data;	//
		rx_count++;
		
		HAL_UART_Receive_IT(&huart1,&rx_data,1);	//此处设置字节数，若不需要不定长数据接收，可以只保留此条，其他删掉
	}
}
void uart_data_rec(void)                                //此函数放到while,若不需要不定长数据接收，可删掉
{
	if(rx_flag)
	{
		if(TIM4->CNT >15)	//数据接收完成
		{
			
			//处理数据，根据题意编写条件
			if( (rx_buff[0]==B1) && (rx_buff[1]==B2) && (rx_buff[2]==B3) && rx_buff[3]=='-' && rx_buff[4]>='0' && rx_buff[4]<='9' && rx_buff[5]>='0' && rx_buff[5]<='9' && rx_buff[6]>='0' && rx_buff[6]<='9')
			{
				B1=rx_buff[4];
				B2=rx_buff[5];
				B3=rx_buff[6];
			}
			
// 

			//条件清零
			rx_flag=0;
			for(uint8_t i=0; i<rx_count;i++) rx_buff[i]=0;
			rx_count=0;
		}
	}
}






void pwm_set_duty(float Duty)   //设置占空比，如50%形参50
{
    TIM2->CCR2 = (TIM2->ARR + 1) * (Duty / 100.0f);
}


void led_Disp(unsigned char ucled)
{
	//**将所有灯熄灭
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);//不是GPIO_PIN_ALL（控制高8位的LED）
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	//根据ucled的数值点亮相应的灯
	HAL_GPIO_WritePin(GPIOC,ucled<<8,GPIO_PIN_RESET);//控制的LED处于高8位（8~18），所以需要将ucled左移8位
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}


