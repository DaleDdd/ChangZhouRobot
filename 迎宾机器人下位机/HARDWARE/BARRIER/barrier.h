#ifndef _BARRIER_H
#define _BARRIER_H
#include "sys.h"

#define BARRIER_INTERRUPT_EN 0 //�ϰ������ж�ʹ�ܣ�1:enable; 0:disable

#define LINE0	PCin(0)	//�����߹�翪���ж�����
#define LINE1	PCin(1)	//�����߹�翪���ж�����
#define BreakSignal1	PAin(0)//��ͣ�ź�1
#define BreakSignal2	PCin(1)//��ͣ�ź�2

#ifndef FarBarrier_Foward
#define FarBarrier_Foward			0x01
#define FarBarrier_Backward		0x02
#define NearBarrier_Foward		0x04
#define NearBarrier_Backward	0x08
#define Barrier_Left					0x10
#define Barrier_Right					0x20
#endif

#ifndef Touch_Head
#define Touch_Head						0x01//ͷ��
#define Touch_Shoulder					0x02//�粿
#define Touch_Chest						0x04//�ز�
#define Touch_Loin						0x08//����
#define Touch_5							0x10
#define Touch_6							0x20
#define Touch_7							0x40
#define Touch_8							0x80
#endif

#ifndef BreakSignal_up_foward
#define BreakSignal_up_foward				PAin(0)//��_ǰ��ͣ
#define BreakSignal_up_backward			PAin(1)//��_��ͣ
#define BreakSignal_dn_foward				PAin(4)//��_ǰ��ͣ
#define BreakSignal_dn_backward			PAin(6)//��_��ͣ
#define LowSpeedSignal_dn_foward		PAin(5)//��_ǰ����
#define LowSpeedSignal_dn_backward	PAin(7)//��_�����
#define TurnSignal_up_left					PAin(2)//��_��ת��
#define TurnSignal_up_right					PAin(3)//��_��ת��
#define TurnSignal_dn_left					PBin(0)//��_��ת��
#define TurnSignal_dn_right					PBin(1)//��_��ת��
#endif

#define BarrierSta_No								0//���ϰ���
#define BarrierSta_NotMan						1//�����ϰ���
#define BarrierSta_Man							2//�ϰ�������
#define BarrierSta_DecSpeed					3//���ٱ�־

#ifndef	NoBarrier
#define NoBarrier		( ( unsigned char ) 0 )	//���ϰ����־
#define NearBarrier	( ( unsigned char ) 1 )	//�������ϰ����־
#define FarBarrier	( ( unsigned char ) 2 )	//Զ�����ϰ����־
#endif

extern unsigned char BarrierFlag;
extern unsigned char BarrierFlag_last;
extern unsigned char BarrierStatus_Reg;
void HumanBodyDetect( unsigned char * barsta );
void Barrier_Init( void );
void BarrierTest( void );
void BarrierScan( void );
unsigned char TouchScan( void );
#endif
