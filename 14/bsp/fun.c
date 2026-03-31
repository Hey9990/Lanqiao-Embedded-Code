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

uint32_t fre,capture_value;	//频率，捕获值

uint8_t M=0;	//PWM输出模式：1高频，0低频	
uint8_t P;		//实时占空比：
float V;			//实时速度：
__IO uint32_t uwTick_Key_M_Point=0;	//5秒内是否切换过模式
__IO uint32_t uwTick_Led_M_Point=0;	//模式切换时闪烁
uint8_t P_flag=1;											//占空比 0锁定 1开启
float duty_k=75.0/2.0;										//电压1~3时占空比比值
double vol_temp;
uint8_t M_temp=0;											//1正在模式切换中
uint8_t M_i=1;													//步长增加频率
uint16_t M_fre;												//临时频率

uint8_t R=1;		//有效范围1~10
uint8_t K=1;
uint8_t R_t=1;
uint8_t K_t=1;
short choose_RK;	//选择R0，K1

uint8_t N=0;		//PWM输出模式切换次数
float MH=0;			//高频模式下速度最大值
float ML=0;			//低频模式  速度最大值
float MH_temp , ML_temp;

__IO uint32_t uwTick_fre_M_Point=0;

uint8_t led_start;

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


	if(B1_state ==0 && B1_last_state ==1)	//B1按下
	{
		lcd_page++;								//切页
		if(lcd_page>2) lcd_page=0;//lcd回到首页
	}

	if(lcd_page==0)				//处于数据界面
	{
		if(B2_state == 0 && B2_last_state ==1 )
		{
			if(!M_temp) M_temp=1;
		}
		
		if(B4_state == 0 && B4_last_state ==1)		//按键B4按下
    {
			TIM3->CNT=0;	
    }
    else if(B4_state ==1 && B4_last_state ==0)	//按键B4松开
    {
        if(TIM3->CNT >20000)	//按键B4长按2S
        {
					P_flag=0;
        }
				else if(!P_flag)			//按键4短按且锁定
				{
					P_flag=1;
				}
    }
		
	}
	else if(lcd_page==1)	//处于参数界面
	{
		if(B2_state ==0 && B2_last_state ==1)	//按下按键2
		{
			choose_RK=!choose_RK;
		}
		if(B4_state ==0 && B4_last_state ==1)//按下按键4
		{
			if(choose_RK)
			{
				K_t--;
				if(K_t<1) K_t=10;
			}
			else
			{
				R_t--;
				if(R_t<1) R_t=10;
			}
		}
	}
	
	


	
	if(B3_state ==0 && B3_last_state ==1)
	{
		if(choose_RK)
		{
			K_t++;
			if(K_t>10) K_t=1;
		}
		else
		{
			R_t++;
			if(R_t>10) R_t=1;
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
			led0();
			//P=(double)TIM2->CCR2/(TIM2->ARR + 1);
			
			LCD_DisplayStringLine(Line1,(uint8_t *)"        DATA        ");
			
			if(M)
			{
				sprintf((char *)lcd_text,"     M=H            ");
				LCD_DisplayStringLine(Line3,lcd_text);
			}
			else
			{
				sprintf((char *)lcd_text,"     M=L            ");
				LCD_DisplayStringLine(Line3,lcd_text);
			}
			sprintf((char *)lcd_text,"     P=%1d%%           ",P);
			LCD_DisplayStringLine(Line4,lcd_text);
			sprintf((char *)lcd_text,"     V=%.1f          ",V);
			LCD_DisplayStringLine(Line5,lcd_text);
			
		}
		break;
		
		case 1:
		{
			led1();
			LCD_DisplayStringLine(Line1,(uint8_t *)"        PARA        ");
			
			sprintf((char *)lcd_text,"     R=%1d            ",R_t);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"     K=%1d            ",K_t);
			LCD_DisplayStringLine(Line4,lcd_text);
			LCD_DisplayStringLine(Line5,(uint8_t *)"                    ");
		}
		break;
		
		default:
		{
			choose_RK=0;
			R=R_t;
			K=K_t;
			led1();
			
			LCD_DisplayStringLine(Line1,(uint8_t *)"        RECD        ");
		
			sprintf((char *)lcd_text,"     N=%1d            ",N);
			LCD_DisplayStringLine(Line3,lcd_text);
			sprintf((char *)lcd_text,"     MH=%.1f         ",MH);
			LCD_DisplayStringLine(Line4,lcd_text);
			sprintf((char *)lcd_text,"     ML=%.1f         ",ML);
			LCD_DisplayStringLine(Line5,lcd_text);
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



/**
 * PWM输入、脉冲捕获
 * 引脚配置TIM通道(TIM17_ch1)
 * TIM通道设置输入捕获 (Input Capture direct mode)
 * PSC(79)
 * 打开中断、main()添加：HAL_TIM_IC_Start_IT(&htim17,TIM_CHANNEL_1);
 */
//输入捕获中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim -> Instance== TIM17)
	{
		capture_value = TIM17->CCR1;			//在捕获到上升沿时，会将CNT赋值给CCR
		TIM17->CNT =0;							//计数器清零
		fre = 80000000 / (80 * capture_value);	//输入捕获频率
		V=(fre *3.14 * 2.0 * R)/(100 * K);
	}
}


/**
 * PWM输出、频率、占空比
 * 引脚配置TIM通道(PA1--TIM_ch2)
 * TIM通道设置为PWM波 (PWM Generation CH2)
 * PSC ARR（799 99）
 * main()添加：HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
 */
void pwm_set_duty(float Duty)   //设置占空比，如50%形参50
{
  TIM2->CCR2 = (TIM2->ARR + 1) * (Duty / 100.0f);
}

void pwm_duty(void)						//R37对占空比的控制
{
	if(!P_flag) return ;
	vol_temp=get_vol(&hadc2);
	if(vol_temp<=1)
	{
		P=10;
	}
	else if(vol_temp>=3)
	{
		P=85;
	}
	else
	{
		P=(vol_temp-1)*duty_k+10;
	}
	pwm_set_duty(P);
}


void freM(void)	//PWM输出模式频率切换
{
	if(M_temp && M_i<41)	//40次代表5秒
	{
		if((uwTick - uwTick_Key_M_Point)<125) return;//1S进入8次
		uwTick_Key_M_Point =uwTick;
		M_i++;
		
		if(M)//高->低
		{
			M_fre=8000-M_i*100;
		}
		else//低->高
		{
			M_fre=4000+M_i*100;
		}
		TIM2->PSC = 80000000/(M_fre*100);
	}
	if(M_i>=40)
		{
			M=!M;
			N++;
			M_i=1;
			M_temp=0;
		}
}

void freV(void)		//统计最大速度
{
	if(M)	//统计高频最大值
	{
		if(MH_temp==V)	
		{
			if(uwTick-uwTick_fre_M_Point>=2000)	MH=MH_temp;
		}
		else if(MH_temp < V)
		{
			MH_temp=V;
			uwTick_fre_M_Point=uwTick;
			MH=MH_temp;
		}
	}
	else	//统计低频最大值
	{
		if(ML_temp==V)	
		{
			if(uwTick-uwTick_fre_M_Point>=1000)	MH=ML_temp;
		}
		else if(ML_temp < V)
		{
			ML_temp=V;
			uwTick_fre_M_Point=uwTick;
			MH=ML_temp;
		}
	}
}


void led0(void)	//处于数据页面，led状态
{
	if(!P_flag && M_temp)
	{
		if((uwTick - uwTick_Led_M_Point)<100) return;
		uwTick_Led_M_Point =uwTick;
		led_start = !led_start;
		if(led_start)	led_Disp(0x07);
		else	led_Disp(0x05);
	}
	else if(!P_flag)
	{
		led_Disp(0x05);
	}
	else if(M_temp)
	{
		if((uwTick - uwTick_Led_M_Point)<100) return;
		uwTick_Led_M_Point =uwTick;
		led_start = !led_start;
		if(led_start)	led_Disp(0x03);
		else	led_Disp(0x01);
	}
	else
	{
		led_Disp(0x01);
	}
}
void led1(void)	//参数界面，led状态
{
	if(!P_flag && M_temp)
	{
		if((uwTick - uwTick_Led_M_Point)<100) return;
		uwTick_Led_M_Point =uwTick;
		led_start = !led_start;
		if(led_start)	led_Disp(0x06);
		else	led_Disp(0x04);
	}
	else if(!P_flag)
	{
		led_Disp(0x04);
	}
	else if(M_temp)
	{
		if((uwTick - uwTick_Led_M_Point)<100) return;
		uwTick_Led_M_Point =uwTick;
		led_start = !led_start;
		if(led_start)	led_Disp(0x02);
		else	led_Disp(0x00);
	}
	else
	{
		led_Disp(0x00);
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




