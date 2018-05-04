/*==========================================================================
	返回按键【注意：返回按键使用带灯按钮或者漫反射红外开关，当使用不同按钮时，接口电平不一样，中断程序要做相应修改】
	1.输入脚, NO常开
	2.INT3->PC2
	3.INT4->PC3
	2016.1.18	Li
==========================================================================*/
#include "back.h"
/*****************************
	函数名称：void BackButton_Init( void )
	功		能：返回按钮端口初始化
						PC2浮空输入
						PC3浮空输入
	入		参：无
	返		回：无
*****************************/
void BackButton_Init( void )
{
	RCC->APB2ENR|=1<<4;			//使能PORTC时钟

	GPIOC->CRL&=0XFFFFF0FF;	//PC2 浮空输入(任意触发电平触发中断)
	GPIOC->CRL|=0X00000800;

	GPIOC->CRL&=0XFFFF0FFF;	//PC3 浮空输入
	GPIOC->CRL|=0X00008000;
	
	Ex_NVIC_Config(GPIO_C,2,FTIR|RTIR); //任意触发电平触发(上升沿、下降沿)
	MY_NVIC_Init(3,3,EXTI2_IRQn,2); 		//抢占 3，子优先级 3，组 2
	Ex_NVIC_Config(GPIO_C,3,FTIR|RTIR);	//任意触发电平触发(上升沿、下降沿)
	MY_NVIC_Init(3,3,EXTI3_IRQn,2); 		//抢占 3，子优先级 3，组 2
}
/*****************************
	函数名称：void EXTI2_IRQHandler(void)
	功		能：外部中断(上升沿、下降沿) 2 服务程序
	入		参：无
	返		回：无
*****************************/
unsigned char BackFlag = BackFlag_NULL;	//送餐返回标志
void EXTI2_IRQHandler(void)
{
	if( BackButton1 == 1 ) //返回按钮按下
	{
		BackFlag = BackFlag_BACK;
		BackFlag = BackFlag_BACK;
	}else if( BackButton1 == 0 )//返回按钮松开
	{
		BackFlag = BackFlag_NULL;
		BackFlag = BackFlag_NULL;
	}
	EXTI->PR=1<<2; 	//清除 LINE2 上的中断标志位
}
/*****************************
	函数名称：void EXTI3_IRQHandler(void)
	功		能：外部中断(上升沿、下降沿) 3 服务程序
	入		参：无
	返		回：无
*****************************/
void EXTI3_IRQHandler(void)
{
	if( BackButton2 == 1 ) //返回按钮按下
	{
		BackFlag = BackFlag_BACK;
		BackFlag = BackFlag_BACK;
	}else if( BackButton2 == 0 )//返回按钮松开
	{
		BackFlag = BackFlag_NULL;
		BackFlag = BackFlag_NULL;
	}
	EXTI->PR=1<<3; //清除 LINE3 上的中断标志位
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
