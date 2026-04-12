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
// uint8_t usart_text[20];     //所发送的数据
// uint8_t rx_data,rx_count;   //接收的单个数据，已接收的数据个数
// uint8_t rx_flag;    //正在接收中断标志，接收不定长数据
// uint8_t rx_buff[20];//所接收的不定长数据


/* RTC专用变量 */
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

/**************************************** 分割线 ****************************************/


/**KEY引脚设置为上拉(Pull-up)
*   LED引脚设置为高电平
*/


/** 若需要长按：选择一个定时器（TIM3）、内部时钟源、PSC ARR(7999 max)
*   main()添加：HAL_TIM_Base_Start(&htim3);
*/
void key_scan(void)	//此函数添加到while中
{
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

    if(B1_state == 0 && B1_last_state ==1)		//按键B1按下
    {
		TIM3->CNT=0;	
    }
    else if(B1_state ==0 && B1_last_state==0)	//按键B1一直按着
    {
        if(TIM3->CNT >=10000)	//按键B1长按1s
        {
        }
    }
    else if(B1_state ==1 && B1_last_state ==0)	//按键B1松开
    {
        if(TIM3->CNT <10000)	//按键B1短按1S
        {
        }
    }
	if(B2_state ==0 && B2_last_state ==1)	//B2按下
	{
		lcd_page++;				//lcd回到首页
		if(lcd_page>2) lcd_page=0;
	}
	if(B3_state ==0 && B3_last_state ==1)
	{
	}
	if(B4_state ==0 && B4_last_state ==1)
	{
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
			sprintf((char *)lcd_text,"",test);
			LCD_DisplayStringLine(Line0,lcd_text);
		}
		break;
		case 1:
		{
		}
		break;
		case 2:
		{
		}
		break;
		default:
		{
		}
		break;
	}
}



/** ADC输入(模拟信号输入) 来源于 R37(PB15) R38(PB12)
*   PB15设为ADC2_IN15   PB12设为ADC1_IN11
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
{   //串口发送使用方法
	//固定字符串+固定长度：
	HAL_UART_Transmit(&huart1, (uint8_t *)"ERROR\r\n", 7, 50);
	//先 sprintf()，再用返回值
	int len = sprintf((char *)usart_text, "ERROR\r\n");
	HAL_UART_Transmit(&huart1, usart_text, len, 50);
	//先 sprintf()，再 strlen()
	sprintf((char *)usart_text, "ERROR\r\n");
	HAL_UART_Transmit(&huart1, usart_text, strlen((char *)usart_text), 50);
	//最后
    sprintf(usart_text,"HelloWorld\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t *)usart_text,sizeof(usart_text),50);	//(串口号,内容,大小,限时)或使用strlen)()
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)	//串口接收回调函数***************************
{
	if(huart->Instance == USART1)
	{	
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
			if(rx_buff[0]=='H' && rx_buff[1]=='E' && rx_buff[2]=='l'){}
			else if(条件){}
			else{输入有error}

			//条件清零
			rx_flag=0;
			for(uint8_t i=0; i<rx_count;i++) rx_buff[i]=0;
			rx_count=0;
		}
	}
}



/** RTC配置界面勾选两个复选框
*   是否选择闹钟（Internal Alarm A） ，选择闹钟要打开中断
*   使用二进制数据格式
*   自行配置初始时间和闹钟
*/
{   //RTC使用方法
    HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
    sprintf(test,"%2d:E%2d:",sTime.Hours,sTime.Minutes);
}
//闹钟中断回调函数
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
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

{//减速部分
	__IO uint32_t uwTick_Key_Set_Point=0;
	if((uwTick - uwTick_Key_Set_Point)<1000) return;//1S
	uwTick_Key_Set_Point =uwTick;
}
{//字符串注意
char a[11];
char b[]='';
strcpy(b, "T");	//修改字符串的值
strcat(a,b);	//串拼接：要求两者都是串
strcmp(a, b) == 0	//为0代表两串相等
}
{//LCD初始化
    LCD_Init();

    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
	LCD_Clear(Black);
}

