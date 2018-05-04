#include"beep.h"
//初始化PB8为输出口，使能这个口的时钟
//蜂鸣器初始化
//IO口常用输入输出模式：0X0->模拟输入模式（ADC用）
//						0X3->推挽输出模式（做输出口用，50M速率）
//						0X8->上/下拉输入模式（做输入口用）
//						0XB->复用输出（使用IO口的第二功能，50M速率
void BEEP_Init(void)
{
	RCC->APB2ENR|=1<<3; 	//使能 PORTB 时钟  
	GPIOB->CRH&=0XFFFFFFF0;
	GPIOB->CRH|=0X00000003;	//PB.8 推挽输出
	BEEP=0;					//关闭蜂鸣器输出
}












