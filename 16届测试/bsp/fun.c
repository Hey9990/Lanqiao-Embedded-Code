#include "fun.h"

uint8_t test;       //用于测试
uint8_t lcd_text[25];
uint8_t lcd_page;	//LCD分页

uint8_t B1_state;
uint8_t B2_state;
uint8_t B3_state;
uint8_t B4_state;
uint8_t B1_last_state=1;//没被按下时，默认高电平
uint8_t B2_last_state=1;
uint8_t B3_last_state=1;
uint8_t B4_last_state=1;


/* USART专用变量 */
 uint8_t usart_text[20];     //所发送的数据
 uint8_t rx_data,rx_count;   //接收的单个数据，已接收的数据个数
 uint8_t rx_flag;    //正在接收中断标志，接收不定长数据
 uint8_t rx_buff[20];//所接收的不定长数据
 
 
 float r37;
 uint8_t mode;	//0-KEY 1-USART
 uint8_t count;
 
 float vmax=2.5;
 float vmin=1.2;

uint8_t r37_flag;	//R37采集的启动1  停止0
uint8_t vmax_flag;	//是否已超 0重置已超
uint8_t vmin_flag;	


/**************************************** 分割线 ****************************************/

void fun0(void)
{
	if(r37_flag) 
	{
		r37=get_vol(&hadc2);
		if(r37>vmax)
		{
			if(!vmax_flag)
			{
				count++;
			}
			vmax_flag=1;
		}
		else
		{
			vmax_flag=0;
		}
		
		if(r37<vmin )
		{
			if(!vmin_flag)
			{
				count+=2;
			}
			
			vmin_flag=1;
		}
		else
		{
			vmin_flag=0;
		}
	}
	
}


void fun_led(void)
{
	if(lcd_page==0 && count>3 && mode)	//DATA界面+ count + mode
	{
		led_Disp(0x85);
	}
	else if(lcd_page && count>3 && mode)	//PARA界面+ count + mode
	{
		led_Disp(0x86);
	}
	else if(lcd_page==0 && count>3)	//DATA界面+ count
	{
		led_Disp(0x05);
	}
	else if(lcd_page && count>3) //PARA界面+ count
	{
		led_Disp(0x06);
	}
	else if(lcd_page==0 && mode)	//DATA界面+ mode
	{
		led_Disp(0x81);
	}
	else if(lcd_page && mode)	//PARA界面+ mode
	{
		led_Disp(0x82);
	}
	else if(lcd_page)
	{
		led_Disp(0x02);
	}
	else if(!lcd_page)
	{
		led_Disp(0x01);
	}
}



void key_scan(void)	//此函数添加到while中
{
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

    if(B1_state == 0 && B1_last_state ==1)		//按键B1按下
    {
			lcd_page=!lcd_page;
    }

	if(B2_state ==0 && B2_last_state ==1)	//B2按下
	{
		mode= !mode;
	}
	if(B3_state ==0 && B3_last_state ==1)
	{
	}
	if(B4_state ==0 && B4_last_state ==1)
	{
		r37_flag =!r37_flag;
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
			
			LCD_DisplayStringLine(Line1,(uint8_t *)"        DATA        ");
			
			sprintf((char *)lcd_text,"        R37:%.2fV     ",r37);
			LCD_DisplayStringLine(Line3,lcd_text);
			if(mode)
			{
				LCD_DisplayStringLine(Line4,(uint8_t *)"        MODE:USART    ");
			}
			else
			{
				LCD_DisplayStringLine(Line4,(uint8_t *)"        MODE:KEY      ");
			}
			sprintf((char *)lcd_text,"        COUNT:%d       ",count);
			LCD_DisplayStringLine(Line5,lcd_text);
		}
		break;
		default:
		{
			LCD_DisplayStringLine(Line1,(uint8_t *)"        PARA        ");
			
			sprintf((char *)lcd_text,"        Vmax:%.1fV     ",vmax);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"        Vmax:%.1fV     ",vmin);
			LCD_DisplayStringLine(Line4,lcd_text);
			LCD_DisplayStringLine(Line5,(uint8_t *)"                    ");
		}
		break;
	}
}



/** ADC输入(模拟信号输入) 来源于 R37 R38
*   PB15设为ADC2_IN15   PB2设为ADC1_IN11
*   ADC——IN11设为 Single-ended
*/
double get_vol(ADC_HandleTypeDef *hadc)	//定义读取电压的函数
{
	HAL_ADC_Start(hadc);
	uint32_t adc_value = HAL_ADC_GetValue(hadc);	
	return 3.3 * adc_value /4096;
}


/** USART串口发送和接收
*   两引脚设置：PA10(USART1_RX)	PA9(USART1_TX)
*   USART1配置：异步（Mode:Asynchronous）  
*               修改波特率（Parameter Settings ：Baud Rate  9600）
*   打开中断使能
*   main()添加发送中断使能：HAL_UART_Receive_IT(&huart1,&rx_data,1);
*
*   不定长数据接收设置：
*   添加一个不中断TIM，PSC(7999), main()添加HAL_TIM_Base_Start(&htim4);
*/


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)	//串口接收回调函数***************************
{
	if(huart->Instance == USART1)
	{	
		TIM4->CNT=0;			//计数器清零
		if(mode)	//USART模式
		{
			rx_flag=1;				//进入中断打开
			rx_buff[rx_count]=rx_data;	//
			rx_count++;
		}
		else			//KEY模式
		{
			
			sprintf((char *)usart_text, "ERROR\r\n");
			HAL_UART_Transmit(&huart1, usart_text, strlen((char *)usart_text), 50);
		}

		HAL_UART_Receive_IT(&huart1,&rx_data,1);	//此处设置字节数，若不需要不定长数据接收，可以只保留此条，其他删掉
	}
}

void uart_data_rec(void)                                //此函数放到while,若不需要不定长数据接收，可删掉
{
	if(rx_flag)
	{
		if(TIM4->CNT >100)	//数据接收完成
		{
//			if(mode)	//USART模式
//			{
				if(rx_count==5 && rx_buff[0]=='S' && rx_buff[1]=='T' && rx_buff[2]=='A'&& rx_buff[3]=='T'&& rx_buff[4]=='E')
					r37_flag=1;
				else if( rx_buff[0]=='E' && rx_buff[1]=='N' && rx_buff[2]=='D')
					r37_flag=0;
				else if(rx_count==9 && rx_buff[3]=='V' && rx_buff[8]=='V' && rx_buff[1]=='.'&& rx_buff[4]==','&& rx_buff[6]=='.')
				{
					vmax=(float)(rx_buff[0]-'0')+ ((float)(rx_buff[2]-'0'))/10.0;
					vmin=(float)(rx_buff[5]-'0')+ ((float)(rx_buff[7]-'0'))/10.0;
				}
					
//			}
			
		

			//条件清零
			rx_flag=0;
			for(uint8_t i=0; i<rx_count;i++) rx_buff[i]=0;
			rx_count=0;
		}
	}
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



