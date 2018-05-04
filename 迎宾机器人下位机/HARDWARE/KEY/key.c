#include "key.h"
#include "delay.h"
//������ʼ��
//IO�ڳ����������ģʽ��0X0->ģ������ģʽ��ADC�ã�
//						0X3->�������ģʽ����������ã�50M���ʣ�
//						0X8->��/��������ģʽ����������ã�
//						0XB->���������ʹ��IO�ڵĵڶ����ܣ�50M����
void KEY_Init(void)
{
	RCC->APB2ENR|=1<<4;		//ʹ��PORTCʱ��
	GPIOC->CRL&=0XFFFFFFF0;	//���ó����룬Ĭ������
	GPIOC->CRL|=0X00000008;
	GPIOC->ODR|=1<<0;   	//PC0 ����
}
//����������
//���ؼ�ֵ
//mode��0����֧�ֳ���
//		1��֧�ֳ���
//0���ް�������
//1��KEY0����
//2��KEY1����
//3��KEY2����
//4��KEY3���� KEY_UP
//ע�⣺�˺�������Ӧ���ȼ���KEY0>KEY1>KEY2>KEY3������
u8 KEY_Scan(u8 mode)
{
	static u8 key_up=1;	//�������ɿ���־
	if(mode)key_up=1;	//֧������
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))
	{
		delay_ms(10);//ȥ����
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(KEY3==1)return 4;
	}
	else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)key_up=1;
	return 0;//�ް�������
}


























