#include "key.h"
#include "delay.h"
//按键初始化
//IO口常用输入输出模式：0X0->模拟输入模式（ADC用）
//						0X3->推挽输出模式（做输出口用，50M速率）
//						0X8->上/下拉输入模式（做输入口用）
//						0XB->复用输出（使用IO口的第二功能，50M速率
void KEY_Init(void)
{
	RCC->APB2ENR|=1<<4;		//使能PORTC时钟
	GPIOC->CRL&=0XFFFFFFF0;	//设置成输入，默认下拉
	GPIOC->CRL|=0X00000008;
	GPIOC->ODR|=1<<0;   	//PC0 上拉
}
//按键处理函数
//返回键值
//mode：0，不支持长按
//		1，支持长按
//0，无按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下
//4，KEY3按下 KEY_UP
//注意：此函数有响应优先级，KEY0>KEY1>KEY2>KEY3！！！
u8 KEY_Scan(u8 mode)
{
	static u8 key_up=1;	//按键按松开标志
	if(mode)key_up=1;	//支持连按
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))
	{
		delay_ms(10);//去抖动
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(KEY3==1)return 4;
	}
	else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)key_up=1;
	return 0;//无按键按下
}


























