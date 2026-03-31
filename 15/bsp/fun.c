#include "fun.h"

uint8_t test;     //用于测试
uint8_t lcd_text[25];
uint8_t lcd_page;	//LCD分页
uint8_t lcd_data;
uint8_t lcd_para;

uint8_t B1_state;
uint8_t B2_state;
uint8_t B3_state;
uint8_t B4_state;
uint8_t B1_last_state=1;//没被按下时，默认高电平
uint8_t B2_last_state=1;
uint8_t B3_last_state=1;
uint8_t B4_last_state=1;

__IO uint32_t uwTick_fre_Set_Point=0;
uint32_t fre_A,capture_value_A,fre_B,capture_value_B;	//频率，捕获值(A-R40-TIM2 B-R39-TIM3)

double A_Hz,B_Hz,A_uS,B_uS;

uint16_t PD=1000 ,PH = 5000;
int PX=0;
uint8_t fmaxmini_start;//初始化频率突变

uint16_t NDA,NDB,NHA,NHB;
uint8_t NHA_flag,NHB_flag;

	__IO uint32_t uwTick_f_Set_Point=0;		//频率突变窗口
	uint16_t max_fA , mini_fA,max_fB , mini_fB;		//频率突变上下限



/**************************************** 分割线 ****************************************/



void key_scan(void)	//此函数添加到while中
{
	B1_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
	B2_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1);
	B3_state = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
	B4_state = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);

	if(B1_state ==0 && B1_last_state ==1)
	{
		switch(lcd_para)
		{
			case 0:
				PD+=100;
				if(PD>1000) PD=1000;
			break;
			
			case 1:
				PH+=100;
				if(PH>10000) PH=10000;
			break;
			
			default:
				PX+=100;
				if(PX>1000) PX=1000;
			break;
		}
	}
	if(B2_state ==0 && B2_last_state ==1)	
	{
		switch(lcd_para)
		{
			case 0:
				PD-=100;
				if(PD<100) PD=100;
			break;
			
			case 1:
				PH-=100;
				if(PH<1000) PH=1000;
			break;
			
			default:
				PX-=100;
				if(PX<-1000) PX=-1000;
			break;
		}
	}
	
	if(lcd_page==0)
	{
		if(B3_state ==0 && B3_last_state ==1)
		{
			lcd_data=!lcd_data;
		}
	}
	else if(lcd_page==1)
	{
		if(B3_state ==0 && B3_last_state ==1)
		{
			lcd_para++;
			if(lcd_para>2)lcd_para=0;
		}
	}
	else
	{
    if(B3_state == 0 && B3_last_state ==1)		//按键B1按下
    {
			TIM4->CNT=0;	
    }
    else if(B3_state ==0 && B3_last_state==0)	//按键B3一直按着
    {
			if(TIM4->CNT >=10000)	//按键B3长按1s
			{
				NDA=0;//频率突变
				NDB=0;
				NHA=0;//频率超限
				NHB=0;
			}
    }
	}
	
	
	if(B4_state ==0 && B4_last_state ==1)
	{
		lcd_data=0;
		lcd_para=0;
		lcd_page++;				//翻页
		if(lcd_page>2) lcd_page=0;
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
				if(NDA>2 || NDB>2)
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x87);
					else if(A_Hz>PH)
						led_Disp(0x83);
					else if(B_Hz>PH)
						led_Disp(0x85);
					else
						led_Disp(0x81);
				}
				else
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x07);
					else if(A_Hz>PH)
						led_Disp(0x03);
					else if(B_Hz>PH)
						led_Disp(0x05);
					else
						led_Disp(0x01);
				}			
				
				LCD_DisplayStringLine(Line1,(uint8_t *)"        DATA        ");
				
				if(!lcd_data)	//频率页面
				{
					if(A_Hz<0)
					{
						LCD_DisplayStringLine(Line3,(uint8_t *)"     A=NULL          ");
						
					}
					else if(A_Hz<1000)
					{
						sprintf((char *)lcd_text,"     A=%1dHz          ",(uint16_t)A_Hz);
						LCD_DisplayStringLine(Line3,lcd_text);
					}
					else
					{
						sprintf((char *)lcd_text,"     A=%.2fKHz        ",(float)A_Hz/1000);
						LCD_DisplayStringLine(Line3,lcd_text);
					}
					
					if(B_Hz<0)
					{
						LCD_DisplayStringLine(Line4,(uint8_t *)"     B=NULL          ");
					}
					else if(B_Hz<1000)
					{
						sprintf((char *)lcd_text,"     B=%1dHz          ",(uint16_t)B_Hz);
						LCD_DisplayStringLine(Line4,lcd_text);
					}
					else
					{
						sprintf((char *)lcd_text,"     B=%.2fKHz        ",(float)B_Hz/1000);
						LCD_DisplayStringLine(Line4,lcd_text);
					}
				}
				else		//周期页面
				{
					if(A_Hz<0)
					{
						LCD_DisplayStringLine(Line3,(uint8_t *)"     A=NULL          ");
					}
					else if(A_uS<1000)
					{
						sprintf((char *)lcd_text,"     A=%1duS          ",(uint16_t)A_uS);
						LCD_DisplayStringLine(Line3,lcd_text);
					}
					else
					{
						sprintf((char *)lcd_text,"     A=%.2fmS      ",(float)A_uS/1000);
						LCD_DisplayStringLine(Line3,lcd_text);
					}
					
					if(B_Hz<0)
					{
						LCD_DisplayStringLine(Line4,(uint8_t *)"     B=NULL          ");
					}
					else if(B_uS<1000)
					{
						sprintf((char *)lcd_text,"     B=%1duS          ",(uint16_t)B_uS);
						LCD_DisplayStringLine(Line4,lcd_text);
					}
					else
					{
						sprintf((char *)lcd_text,"     B=%.2fmS      ",(float)B_uS/1000);
						LCD_DisplayStringLine(Line4,lcd_text);
					}
				}
				LCD_DisplayStringLine(Line5,(uint8_t *)"                    ");
				LCD_DisplayStringLine(Line6,(uint8_t *)"                    ");
			}
			break;
			
			case 1:
			{
				if(NDA>2 || NDB>2)
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x86);
					else if(A_Hz>PH)
						led_Disp(0x82);
					else if(B_Hz>PH)
						led_Disp(0x84);
					else
						led_Disp(0x80);
				}
				else
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x06);
					else if(A_Hz>PH)
						led_Disp(0x02);
					else if(B_Hz>PH)
						led_Disp(0x04);
					else
						led_Disp(0x00);
				}
				
				LCD_DisplayStringLine(Line1,(uint8_t *)"        PARA        ");
				
				sprintf((char *)lcd_text,"     PD=%1dHz         ",PD);
				LCD_DisplayStringLine(Line3,lcd_text);
				sprintf((char *)lcd_text,"     PH=%1dHz         ",PH);
				LCD_DisplayStringLine(Line4,lcd_text);
				sprintf((char *)lcd_text,"     PX=%1dHz         ",PX);
				LCD_DisplayStringLine(Line5,lcd_text);
			}
			break;
			
			default:
			{
				if(NDA>2 || NDB>2)
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x86);
					else if(A_Hz>PH)
						led_Disp(0x82);
					else if(B_Hz>PH)
						led_Disp(0x84);
					else
						led_Disp(0x80);
				}
				else
				{
					if(A_Hz>PH && B_Hz>PH)
						led_Disp(0x06);
					else if(A_Hz>PH)
						led_Disp(0x02);
					else if(B_Hz>PH)
						led_Disp(0x04);
					else
						led_Disp(0x00);
				}
			
				LCD_DisplayStringLine(Line1,(uint8_t *)"        RECD        ");
				
				sprintf((char *)lcd_text,"     NDA=%1d          ",NDA);
				LCD_DisplayStringLine(Line3,lcd_text);
				sprintf((char *)lcd_text,"     NDB=%1d          ",NDB);
				LCD_DisplayStringLine(Line4,lcd_text);
				sprintf((char *)lcd_text,"     NHA=%1d         ",NHA);
				LCD_DisplayStringLine(Line5,lcd_text);
				sprintf((char *)lcd_text,"     NHB=%1d         ",NHB);
				LCD_DisplayStringLine(Line6,lcd_text);
			}
			break;
		}
}

void fre(void)
{
	if(uwTick>100 && (uwTick - uwTick_fre_Set_Point)<100) return;
	uwTick_fre_Set_Point =uwTick;
	
	A_Hz=(int)fre_A+PX;
	B_Hz=(long)fre_B+PX;
	A_uS=(double)1000000/A_Hz;
	B_uS=(double)1000000/B_Hz;
	
	if(A_Hz >PH) 		//判断是否频率超限NHA
	{
		if(!NHA_flag) 
		{
			NHA_flag=1;
			NHA++;
		}
	}
	else
	{
		NHA_flag=0;
	}
		 
		if(B_Hz >PH) 	//判断是否频率超限NHB
	{
		if(!NHB_flag) 
		{
			NHB_flag=1;
			NHB++;
		}
	}
	else
	{
		NHB_flag=0;
	}
	
}

void fmaxmini(void)
{
	if(uwTick < 500) return;
	if(! fmaxmini_start)
	{
		max_fA =fre_A;
		mini_fA = fre_A;
		max_fB	= fre_B;
		mini_fB =fre_B;
		uwTick_f_Set_Point=uwTick;
		fmaxmini_start=1;
	}
	
	if(max_fA<fre_A)		//检测脉冲A窗口突变
	{
		max_fA=fre_A;
	}
	if(mini_fA>fre_A)
	{
		mini_fA=fre_A;
	}
		
	if(max_fB<fre_B)		//检测脉冲B窗口突变
	{
		max_fB=fre_B;
	}
	if(mini_fB>fre_B)
	{
		mini_fB=fre_B;
	}
	if( uwTick - uwTick_f_Set_Point>=3000)	//每3秒检测脉冲突变
	{
		uwTick_f_Set_Point=uwTick;
		
		if(max_fA - mini_fA > PD)
		{
			NDA++;
			max_fA	= fre_A;
			mini_fA =fre_A;
		}
		
		if(max_fB - mini_fB > PD)
		{
			NDB++;
			max_fB	= fre_B;
			mini_fB =fre_B;
		}
		
	}
	
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim -> Instance== TIM2)
	{
		capture_value_A = TIM2->CCR1;			//在捕获到上升沿时，会将CNT赋值给CCR
		TIM2->CNT =0;							//计数器清零
		fre_A = 80000000 / (80 * capture_value_A);	//输入捕获频率
	}
		if(htim -> Instance== TIM3)
	{
		capture_value_B = TIM3->CCR1;			
		TIM3->CNT =0;							
		fre_B = 80000000 / (80 * capture_value_B);	
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

