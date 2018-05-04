#ifndef __MOTIONCONTROL_H
#define __MOTIONCONTROL_H
#include "pwm.h"

/*	端口定义	*/
#define STOP_L						PBout(3)//准备开始/自然停车
#define BREAK_L						PBout(4)//电机启动/刹车
#define DIR_L							PBout(5)//方向
#define STOP_R						PBout(8)//准备开始/自然停车
#define BREAK_R						PBout(9)//电机启动/刹车
#define DIR_R							PEout(0)//方向
#define PWM_L							TIM4->CCR1//CH2 PWM占空比给定, 0-arr
#define PWM_R							TIM4->CCR2//CH1 PWM占空比给定, 0-arr

/*	占空比限幅值	*/
#define PWMDUTY_MAX				((signed int)  239)//占空比正向最大值
#define PWMDUTY_MIN				((signed int) -239)//占空比反向最大值

void Motion_Init( void );																							//PWM/脉冲、电机驱动器端口初始化
void MotorDriverVClControl(signed int dutyleft,signed int dutyright);	//电机驱动器速度给定
#endif
