#include "led.h"
//初始化PE12为输出口 .并使能这个口的时钟
//LED IO 初始化
//IO口常用输入输出模式：0X0->模拟输入模式（ADC用）
//						0X3->推挽输出模式（做输出口用，50M速率）
//						0X8->上/下拉输入模式（做输入口用）
//						0XB->复用输出（使用IO口的第二功能，50M速率）
//端口配置寄存器CRL和CRH只能按32位字访问
void LED_Init(void)
{
	RCC->APB2ENR|=1<<6; 		//使能 PORTE 时钟
	
	GPIOE->CRH&=0XFFF0FFFF;
	GPIOE->CRH|=0X00030000;	//PE.12 推挽输出
	GPIOE->ODR|=1<<12; 			//PE.12 输出高
}




