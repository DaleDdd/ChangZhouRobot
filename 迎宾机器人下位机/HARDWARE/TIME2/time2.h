#ifndef __TIME2_H
#define __TIME2_H
#include "sys.h"

extern u8  RS485_Rx_Flag;									//485���յ�һ֡���ݱ�־,0:������,1:�������һ֡����

void TIM2_Int_Init(u16 arr,u16 psc);//ͨ�ö�ʱ��2�жϳ�ʼ��
void TIM2_ClrCont( void );//�嶨ʱ��2�ļ�����
void TIM2_En( void );//ʹ��ͨ�ö�ʱ��2
void TIM2_Dis( void );//��ֹͨ�ö�ʱ��2
void TIM2_IRQHandler(void);//485�������ݳ�ʱ�ж�.���ж�˵���������ݳ�ʱ,һ֡���ݽ���.

#endif
