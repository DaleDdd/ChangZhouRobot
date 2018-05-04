#ifndef __MOTIONCONTROL_H
#define __MOTIONCONTROL_H
#include "pwm.h"

/*	�˿ڶ���	*/
#define STOP_L						PBout(3)//׼����ʼ/��Ȼͣ��
#define BREAK_L						PBout(4)//�������/ɲ��
#define DIR_L							PBout(5)//����
#define STOP_R						PBout(8)//׼����ʼ/��Ȼͣ��
#define BREAK_R						PBout(9)//�������/ɲ��
#define DIR_R							PEout(0)//����
#define PWM_L							TIM4->CCR1//CH2 PWMռ�ձȸ���, 0-arr
#define PWM_R							TIM4->CCR2//CH1 PWMռ�ձȸ���, 0-arr

/*	ռ�ձ��޷�ֵ	*/
#define PWMDUTY_MAX				((signed int)  239)//ռ�ձ��������ֵ
#define PWMDUTY_MIN				((signed int) -239)//ռ�ձȷ������ֵ

void Motion_Init( void );																							//PWM/���塢����������˿ڳ�ʼ��
void MotorDriverVClControl(signed int dutyleft,signed int dutyright);	//����������ٶȸ���
#endif
