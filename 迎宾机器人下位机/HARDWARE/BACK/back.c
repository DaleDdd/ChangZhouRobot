/*==========================================================================
	���ذ�����ע�⣺���ذ���ʹ�ô��ư�ť������������⿪�أ���ʹ�ò�ͬ��ťʱ���ӿڵ�ƽ��һ�����жϳ���Ҫ����Ӧ�޸ġ�
	1.�����, NO����
	2.INT3->PC2
	3.INT4->PC3
	2016.1.18	Li
==========================================================================*/
#include "back.h"
/*****************************
	�������ƣ�void BackButton_Init( void )
	��		�ܣ����ذ�ť�˿ڳ�ʼ��
						PC2��������
						PC3��������
	��		�Σ���
	��		�أ���
*****************************/
void BackButton_Init( void )
{
	RCC->APB2ENR|=1<<4;			//ʹ��PORTCʱ��

	GPIOC->CRL&=0XFFFFF0FF;	//PC2 ��������(���ⴥ����ƽ�����ж�)
	GPIOC->CRL|=0X00000800;

	GPIOC->CRL&=0XFFFF0FFF;	//PC3 ��������
	GPIOC->CRL|=0X00008000;
	
	Ex_NVIC_Config(GPIO_C,2,FTIR|RTIR); //���ⴥ����ƽ����(�����ء��½���)
	MY_NVIC_Init(3,3,EXTI2_IRQn,2); 		//��ռ 3�������ȼ� 3���� 2
	Ex_NVIC_Config(GPIO_C,3,FTIR|RTIR);	//���ⴥ����ƽ����(�����ء��½���)
	MY_NVIC_Init(3,3,EXTI3_IRQn,2); 		//��ռ 3�������ȼ� 3���� 2
}
/*****************************
	�������ƣ�void EXTI2_IRQHandler(void)
	��		�ܣ��ⲿ�ж�(�����ء��½���) 2 �������
	��		�Σ���
	��		�أ���
*****************************/
unsigned char BackFlag = BackFlag_NULL;	//�Ͳͷ��ر�־
void EXTI2_IRQHandler(void)
{
	if( BackButton1 == 1 ) //���ذ�ť����
	{
		BackFlag = BackFlag_BACK;
		BackFlag = BackFlag_BACK;
	}else if( BackButton1 == 0 )//���ذ�ť�ɿ�
	{
		BackFlag = BackFlag_NULL;
		BackFlag = BackFlag_NULL;
	}
	EXTI->PR=1<<2; 	//��� LINE2 �ϵ��жϱ�־λ
}
/*****************************
	�������ƣ�void EXTI3_IRQHandler(void)
	��		�ܣ��ⲿ�ж�(�����ء��½���) 3 �������
	��		�Σ���
	��		�أ���
*****************************/
void EXTI3_IRQHandler(void)
{
	if( BackButton2 == 1 ) //���ذ�ť����
	{
		BackFlag = BackFlag_BACK;
		BackFlag = BackFlag_BACK;
	}else if( BackButton2 == 0 )//���ذ�ť�ɿ�
	{
		BackFlag = BackFlag_NULL;
		BackFlag = BackFlag_NULL;
	}
	EXTI->PR=1<<3; //��� LINE3 �ϵ��жϱ�־λ
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
