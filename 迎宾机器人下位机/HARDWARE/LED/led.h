#ifndef __LED_H
#define __LED_H
#include "sys.h"
//LED端口定义
//对单个IO口操作的位带操作方法（比较简单，常用）
#define LED  PEout(12)//DS0
//另一种对单个IO口操作的方法（此方法比较麻烦，一般用STM32的位带操作实现对单个IO口的输出操作）
//#define LED0 (1<<5) //led0  PB5
//#define LED1 (1<<5) //led1 PE5
//#define LED0_SET(x) GPIOB->ODR=(GPIOB->ODR&~LED0)|(x ? LED0: 0)
//#define LED1_SET(x) GPIOE->ODR=(GPIOE->ODR&~LED1)|(x ? LED1: 0)

void LED_Init(void);//LED端口初始化
#endif




