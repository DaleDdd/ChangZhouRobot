#ifndef __LED_H
#define __LED_H
#include "sys.h"
//LED�˿ڶ���
//�Ե���IO�ڲ�����λ�������������Ƚϼ򵥣����ã�
#define LED  PEout(12)//DS0
//��һ�ֶԵ���IO�ڲ����ķ������˷����Ƚ��鷳��һ����STM32��λ������ʵ�ֶԵ���IO�ڵ����������
//#define LED0 (1<<5) //led0  PB5
//#define LED1 (1<<5) //led1 PE5
//#define LED0_SET(x) GPIOB->ODR=(GPIOB->ODR&~LED0)|(x ? LED0: 0)
//#define LED1_SET(x) GPIOE->ODR=(GPIOE->ODR&~LED1)|(x ? LED1: 0)

void LED_Init(void);//LED�˿ڳ�ʼ��
#endif




