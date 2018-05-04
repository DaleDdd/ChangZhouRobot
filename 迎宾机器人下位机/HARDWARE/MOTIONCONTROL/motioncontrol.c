/*==========================================================================
	ʵ�ָ����˶�ģʽ(������ֹͣ���Ӽ��١�Ѱ�ߡ�ת���)���˶����ƺ���ʵ��
	1.TIM4 
		CH1(PB6) ����
		CH2(PB7) �ҵ��
	2016.1.18	Li
==========================================================================*/
#include "motioncontrol.h"
#include "math.h"

/*********************************************
	�������ƣ�void Motion_Init( void )
	��		�ܣ�PWM/���塢����������˿ڳ�ʼ��
						PB3->start/stop L, ׼����ʼ/��Ȼͣ��
						PB4->run/break	L, �������/ɲ��
						PB5->dir				L, ����
						PB8->start/stop R, ׼����ʼ/��Ȼͣ��
						PB9->run/break	R, �������/ɲ��
						PE0->dir				R, ����
						
						PB6->speed_in		L, �ٶ����� 0-5V
						PB7->speed_in		R, �ٶ����� 0-5V
	��		�Σ���
	��		ֵ����
*********************************************/
void Motion_Init( void )
{
	/*	PWM/�����ʼ��	*/
	TIM4_PWM_Init(239,14);			//CH1(PB6) CH2(PB7)PWM���. PWMƵ��=72000/(14+1)/(239+1)=20Khz
	
	/*	�˿ڳ�ʼ��	*/
	RCC->APB2ENR|=1<<3;    	//ʹ��PORTBʱ��
	RCC->APB2ENR|=1<<6;    	//ʹ��PORTEʱ��
 	  	
	GPIOB->CRL&=0XFF000FFF;	//PB3 4 5 �������
	GPIOB->CRL|=0X00333000;	//
	
	GPIOB->CRH&=0XFFFFFF00;	//PB8 9 �������
	GPIOB->CRH|=0X00000033;	//
	
	GPIOE->CRL&=0XFFFFFFF0;	//PE0�������
	GPIOE->CRL|=0X00000003;	//
	
	STOP_L	=	STOP_R	= 1;	//��������ֹ
	
	BREAK_L	=	BREAK_R	= 0;	//�������
	DIR_L		=	1;						//��ʼ����
	DIR_R		=	0;	
	
	MotorDriverVClControl( 0 , 0 );
}
/*********************************************
	�������ƣ�void MotorDriverVClControl(signed int targetvleft,signed int targetvright)
	��		�ܣ�����������ٶȸ���
	��		�Σ�dutyleft	: ����ռ�ձ�
						dutyright	: �ҵ��ռ�ձ�
	��		ֵ����
*********************************************/
void MotorDriverVClControl(signed int dutyleft,signed int dutyright)
{
	/*	�޷�	*/
	dutyleft 	= (dutyleft>PWMDUTY_MAX) 	? PWMDUTY_MAX : dutyleft;
	dutyleft 	= (dutyleft<PWMDUTY_MIN) 	? PWMDUTY_MIN : dutyleft;
	dutyright = (dutyright>PWMDUTY_MAX) ? PWMDUTY_MAX : dutyright;
	dutyright = (dutyright<PWMDUTY_MIN) ? PWMDUTY_MIN : dutyright;
	/*	����	*/
	if(dutyleft>=0)//��ת
	{
		DIR_L	 =0;
		PWM_L = dutyleft;
	}else if(dutyleft<0)//��ת
	{
		DIR_L	 =1;
		PWM_L = fabs(dutyleft);
	}
	/*	�ҵ��	*/
	if(dutyright>=0)//��ת
	{
		DIR_R	 =1;
		PWM_R = dutyright;
	}else if(dutyright<0)//��ת
	{
		DIR_R	 =0;
		PWM_R = fabs(dutyright);
	}
}


//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
