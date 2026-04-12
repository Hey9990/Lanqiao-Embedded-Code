#include "fun.h"

/**
 *  基本TIM使用：
 *  选用一个TIM(tim2)、选择内部时钟、设置PSC ARR(7999 9999)
 *  开启中断使能
 *  main()添加：HAL_TIM_Base_Start_IT(&htim2);
 */
//重重定义回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
	}
}



//R39-PB4	R40-PA15
/**
 * PWM输入、脉冲捕获
 * 引脚配置TIM通道(TIM17_ch1)
 * TIM通道设置输入捕获 (Input Capture direct mode)
 * PSC(79)
 * 打开中断、main()添加：HAL_TIM_IC_Start_IT(&htim17,TIM_CHANNEL_1);
 * /

uint32_t fre,capture_value;	//频率，捕获值
//输入捕获中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim -> Instance== TIM17)
	{
		//capture_value = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1);
		capture_value = TIM17->CCR1;			//在捕获到上升沿时，会将CNT赋值给CCR
		TIM17->CNT =0;							//计数器清零
		fre = 80000000 / (80 * capture_value);	//输入捕获频率
	}
}





/**
 * PWM输出、频率、占空比
 * 引脚配置TIM通道(PA1--TIM2_ch2)
 * TIM通道设置为PWM波 (PWM Generation CH2)
 * PSC ARR（799 99）
 * main()添加：HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
 * /
{    
    TIM2->PSC = 80000000/(test*100);    //设置PWM频率（固定ARR，动态PSC方法，此方案默认ARR已初始化为99）
}
void pwm_set_duty(float Duty)   //设置占空比，如50%形参50
{
    TIM2->CCR2 = (TIM2->ARR + 1) * (Duty / 100.0f);
}

