/*==========================================================================
	PWM/脉冲设置
	1.TIM4 
		CH1(PB6) 
		CH2(PB7)
	2016.1.18	Li
==========================================================================*/
#include "pwm.h"

/*********************************************
	函数名称：void TIM4_PWM_Init(u16 arr,u16 psc)
	功		能：通用TIM4 CH1(PB6) CH2(PB7)PWM输出初始化
	入		参：arr：自动重装值。
						psc：时钟预分频数
	返		回：无
【说明】Frq = 72Mhz / ( psc + 1 ) / ( arr + 1 )
*********************************************/
void TIM4_PWM_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<2; 		//TIM4时钟使能    
	RCC->APB2ENR|=1<<3;    	//使能PORTB时钟	
 	  	
	GPIOB->CRL&=0X00FFFFFF;	//PB6 7 输出
	GPIOB->CRL|=0XBB000000;	//复用功能输出 	  
 
	TIM4->ARR=arr;				//设定计数器自动重装值 
	TIM4->PSC=psc;				//预分频器分频设置
	
	TIM4->CCMR1|=6<<4;  	//CH1 PWM1模式
												//6: PWM1模式, 正脉冲
												//7: PWM2模式, 负脉冲
	TIM4->CCMR1|=1<<3; 		//CH1 预装载使能	   
	TIM4->CCER|=1<<0;   	//OC1 输出使能	   
	
	TIM4->CCMR1|=6<<12;  	//CH2 PWM1模式
	TIM4->CCMR1|=1<<11; 	//CH2 预装载使能	   
	TIM4->CCER|=1<<4;   	//OC2 输出使能

	TIM4->CR1=0x0080;   	//ARPE使能 
	TIM4->CR1|=0x01;    	//使能定时器4
}

//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
