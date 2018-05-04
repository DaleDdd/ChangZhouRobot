/*==========================================================================
	实现各种运动模式(启动、停止、加减速、寻线、转体等)的运动控制函数实现
	1.TIM4 
		CH1(PB6) 左电机
		CH2(PB7) 右电机
	2016.1.18	Li
==========================================================================*/
#include "motioncontrol.h"
#include "math.h"

/*********************************************
	函数名称：void Motion_Init( void )
	功		能：PWM/脉冲、电机驱动器端口初始化
						PB3->start/stop L, 准备开始/自然停车
						PB4->run/break	L, 电机启动/刹车
						PB5->dir				L, 方向
						PB8->start/stop R, 准备开始/自然停车
						PB9->run/break	R, 电机启动/刹车
						PE0->dir				R, 方向
						
						PB6->speed_in		L, 速度输入 0-5V
						PB7->speed_in		R, 速度输入 0-5V
	入		参：无
	返		值：无
*********************************************/
void Motion_Init( void )
{
	/*	PWM/脉冲初始化	*/
	TIM4_PWM_Init(239,14);			//CH1(PB6) CH2(PB7)PWM输出. PWM频率=72000/(14+1)/(239+1)=20Khz
	
	/*	端口初始化	*/
	RCC->APB2ENR|=1<<3;    	//使能PORTB时钟
	RCC->APB2ENR|=1<<6;    	//使能PORTE时钟
 	  	
	GPIOB->CRL&=0XFF000FFF;	//PB3 4 5 推挽输出
	GPIOB->CRL|=0X00333000;	//
	
	GPIOB->CRH&=0XFFFFFF00;	//PB8 9 推挽输出
	GPIOB->CRH|=0X00000033;	//
	
	GPIOE->CRL&=0XFFFFFFF0;	//PE0推挽输出
	GPIOE->CRL|=0X00000003;	//
	
	STOP_L	=	STOP_R	= 1;	//驱动器禁止
	
	BREAK_L	=	BREAK_R	= 0;	//电机启动
	DIR_L		=	1;						//初始方向
	DIR_R		=	0;	
	
	MotorDriverVClControl( 0 , 0 );
}
/*********************************************
	函数名称：void MotorDriverVClControl(signed int targetvleft,signed int targetvright)
	功		能：电机驱动器速度给定
	入		参：dutyleft	: 左电机占空比
						dutyright	: 右电机占空比
	返		值：无
*********************************************/
void MotorDriverVClControl(signed int dutyleft,signed int dutyright)
{
	/*	限幅	*/
	dutyleft 	= (dutyleft>PWMDUTY_MAX) 	? PWMDUTY_MAX : dutyleft;
	dutyleft 	= (dutyleft<PWMDUTY_MIN) 	? PWMDUTY_MIN : dutyleft;
	dutyright = (dutyright>PWMDUTY_MAX) ? PWMDUTY_MAX : dutyright;
	dutyright = (dutyright<PWMDUTY_MIN) ? PWMDUTY_MIN : dutyright;
	/*	左电机	*/
	if(dutyleft>=0)//正转
	{
		DIR_L	 =0;
		PWM_L = dutyleft;
	}else if(dutyleft<0)//反转
	{
		DIR_L	 =1;
		PWM_L = fabs(dutyleft);
	}
	/*	右电机	*/
	if(dutyright>=0)//正转
	{
		DIR_R	 =1;
		PWM_R = dutyright;
	}else if(dutyright<0)//反转
	{
		DIR_R	 =0;
		PWM_R = fabs(dutyright);
	}
}


//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
