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
 
 uint8_t x_shop;
 uint8_t y_shop;
 float z;
 
 float x_prise=1.0;
 float y_prise=1.0;
 uint8_t x_prise_t=10;
 uint8_t y_prise_t=10;
 
 uint8_t x_rep=10;
 uint8_t y_rep=10;

__IO uint32_t uwTick_yes_shop_Point=0;
__IO uint32_t uwTick_led_rep_Point=0;
uint8_t yes_shop_flag;		//是否进入确认购买函数 0 否  1 是
uint8_t yes_rep_flag;				//X和Y数量都为0  0否 1是
uint8_t led_rep_flag;				//led闪烁


uint8_t one_flag;	//用于判断当前设备是否是第一次上电(0x55)

/**************************************** 分割线 ****************************************/


void yes_shop(void)
{
if(yes_shop_flag){
	if((uwTick - uwTick_yes_shop_Point)<100) return;//0.1S
	uwTick_yes_shop_Point =uwTick;
	yes_shop_flag++;

	if(yes_shop_flag>50)
	{
		yes_shop_flag=0;
		pwm_set_duty(5);
	}
	
}
}

void led0(void)
{
	if((uwTick - uwTick_led_rep_Point)<100) return;//0.1S
	uwTick_led_rep_Point =uwTick;
	
	if(yes_shop_flag &&yes_rep_flag)
	{
		led_rep_flag = !led_rep_flag ;
		if(led_rep_flag) led_Disp(0x03);
		else	led_Disp(0x01);
	}
	else if(yes_rep_flag)
	{
		led_rep_flag = !led_rep_flag ;
		if(led_rep_flag) led_Disp(0x02);
		else	led_Disp(0x00);
	}
	else if(yes_shop_flag)
	{
		led_Disp(0x01);
	}
	else
	{
		led_Disp(0x00);
	}
}

void  eeprom_init(void)
{
eeprom_read(&one_flag,4,1);

	if(one_flag !=0x55)	//设备第一次上电
	{
		eeprom_write(&x_rep,0,1);	//x库存数量存入
		eeprom_write(&y_rep,1,1);	//y库存数量存入
		x_prise_t=x_prise * 10;				//x单价处理
		eeprom_write(&x_prise_t,2,1);	//x单价存入
		y_prise_t=y_prise * 10;				//y单价处理
		eeprom_write(&y_prise_t,3,1);	//y单价存入
		one_flag=0x55;
		eeprom_write(&one_flag,4,1);;
		return;
	}
	eeprom_read(&x_rep,0,1);	//x库存数量读取
	eeprom_read(&y_rep,1,1);	//y库存数量读取			
	eeprom_read(&x_prise_t,2,1);	//x单价读取
	x_prise=(float)x_prise_t/10.0;//x单价处理
	eeprom_read(&y_prise_t,3,1);	//y单价读取
	y_prise=(float)y_prise_t/10.0;//y单价处理

}


void key_scan(void)	//此函数添加到while中
{
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

	if(B1_state == 0 && B1_last_state ==1)		//按键B1按下
	{
		lcd_page++;				
		if(lcd_page>2) lcd_page=0;
	}
	if(B2_state ==0 && B2_last_state ==1)	//B2按下
	{
		switch (lcd_page)
		{
			case 0:
			{
				x_shop++;
				if(x_shop>x_rep) x_shop=0;
			}
			break;
			
			case 1:
			{
				x_prise+=0.1;
				if(x_prise>2.1)x_prise=1.0;
				x_prise_t=x_prise * 10;				//x单价处理
				eeprom_write(&x_prise_t,2,1);	//x单价存入
			}
			break;
			
			default:
			{
				x_rep++;
				if(!y_rep && !x_rep) yes_rep_flag=1;
				else yes_rep_flag=0;
				eeprom_write(&x_rep,0,1);
			}
		}
	}
	if(B3_state ==0 && B3_last_state ==1)
	{
		switch (lcd_page)
		{
			case 0:
			{
				y_shop++;
				if(y_shop>y_rep) y_shop=0;
			}
			break;
			
			case 1:
			{
				y_prise+=0.1;
				if(y_prise>2.1)y_prise=1.0;
				y_prise_t=y_prise * 10;				//y单价处理
				eeprom_write(&y_prise_t,3,1);	//y单价存入
			}
			break;
			
			default:
			{
				y_rep++;
				if(!y_rep && !x_rep) yes_rep_flag=1;
				else yes_rep_flag=0;
				eeprom_write(&y_rep,1,1);
			}
		}
	}
	if(B4_state ==0 && B4_last_state ==1)
	{
		if(lcd_page==0)
		{
			if(x_shop<=x_rep && y_shop<=y_rep)
			{
				x_rep=x_rep-x_shop;
				y_rep=y_rep-y_shop;
				yes_shop_flag=1;
				TIM2->PSC = 80000000/(2000*100);
				pwm_set_duty(30);
				z=x_shop* x_prise + y_shop * y_prise;
				sprintf(usart_text,"X:%1d,Y:%1d,Z:%.1f\r\n",x_shop,y_shop,z);
				HAL_UART_Transmit(&huart1,(uint8_t *)usart_text,sizeof(usart_text),50);
				
				eeprom_write(&x_rep,0,1);	//x库存数量存入
				eeprom_write(&y_rep,1,1);	//y库存数量存入
				x_prise_t=x_prise * 10;				//x单价处理
				eeprom_write(&x_prise_t,2,1);	//x单价存入
				y_prise_t=y_prise * 10;				//y单价处理
				eeprom_write(&y_prise_t,3,1);	//y单价存入
			}
				if(!y_rep && !x_rep) yes_rep_flag=1;
				else yes_rep_flag=0;
			x_shop=0;
			y_shop=0;
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
			LCD_DisplayStringLine(Line1,(uint8_t *)"        SHOP        ");
			
			sprintf((char *)lcd_text,"     X:%1d            ",x_shop);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"     Y:%1d            ",y_shop);
			LCD_DisplayStringLine(Line4,lcd_text);
		}
		break;
		
		case 1:
		{
			LCD_DisplayStringLine(Line1,(uint8_t *)"        PRICE       ");
			
			sprintf((char *)lcd_text,"     X:%.1f          ",x_prise);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"     Y:%.1f          ",y_prise);
			LCD_DisplayStringLine(Line4,lcd_text);
		}
		break;

		default:
		{
			LCD_DisplayStringLine(Line1,(uint8_t *)"        REP         ");
			
			sprintf((char *)lcd_text,"     X:%1d            ",x_rep);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"     Y:%1d            ",y_rep);
			LCD_DisplayStringLine(Line4,lcd_text);
		}
		break;
	}
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
//{   //串口发送使用方法
//    sprintf(usart_test,"HelloWorld\r\n");
//    HAL_UART_Transmit(&huart1,(uint8_t *)usart_test,sizeof(usart_text),50);	//(串口号,内容,大小,限时)或使用strlen)()
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)	//串口接收回调函数***************************
{
	if(huart->Instance == USART1)
	{	
		if(rx_data=='?')
		{
			sprintf(usart_text,"X:%.1f,Y:%.1f\r\n",x_prise,y_prise);
			HAL_UART_Transmit(&huart1,(uint8_t *)usart_text,sizeof(usart_text),50);
		}
		
		
		HAL_UART_Receive_IT(&huart1,&rx_data,1);	//此处设置字节数，若不需要不定长数据接收，可以只保留此条，其他删掉
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


