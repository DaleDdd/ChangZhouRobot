#ifndef __TIME5_H
#define __TIME5_H
#include "sys.h"

/*	��λ��ͨѶָ���붨��	*/

/*	���ظ���λ����״̬�붨��	*/
#define ReceiveError					(unsigned char) 0x00 //������λ������ʧ��
#define ReceiveOk							(unsigned char) 0x01 //������λ�����ݳɹ�
extern unsigned char CommandReceiveStatus;
extern unsigned char TaskTableBuf;//�Ͳ����Ż�����
extern unsigned char GetCtrAreaStatusFlag;//�õ�������״̬��־λ
extern unsigned char 			 TIM2_IRQSource;
extern unsigned char 			 TIM5_IRQSource;
extern const unsigned char ID_TRIG;
extern const unsigned char WIRELESS_TRIG;
extern const unsigned char HMI_TRIG;
extern signed 		int SpeedSet_HV;
extern signed 		int SpeedSearchLine;
extern signed  		int SpeedIncA;  /*���ټ��ٶ�,ÿ100ms���ӵ��ٶ�*/
extern signed  		int SpeedDecA;
extern float Kp;											 //����
extern float Ki;
extern float Kd;
extern float k,b;//���������֮�����Ի�ϵ��
extern unsigned char STATMOD_FLAG;
extern unsigned char STATIM_FLAG;
extern unsigned char CROSS_FLAG;
extern unsigned char Recive_data[50];
extern unsigned char Recive_num;
void TIM5_Int_Init(u16 arr,u16 psc);//ͨ�ö�ʱ��5�жϳ�ʼ��
void TIM5_ClrCont( void );					//�嶨ʱ��5�ļ�����
void TIM5_En( void );								//ʹ��ͨ�ö�ʱ��5
void TIM5_Dis( void );							//��ֹͨ�ö�ʱ��5

void TIM2_Int_Init(u16 arr,u16 psc);//ͨ�ö�ʱ��2�жϳ�ʼ��
void TIM2_ClrCont( void );					//�嶨ʱ��2�ļ�����
void TIM2_En( void );								//ʹ��ͨ�ö�ʱ��2
void TIM2_Dis( void );							//��ֹͨ�ö�ʱ��2

void TIM6_Int_Init(u16 arr,u16 psc);//������ʱ��6�жϳ�ʼ��
void TIM6_ClrCont( void );					//�嶨ʱ��6�ļ�����
void TIM6_En( void );								//ʹ�ܻ�����ʱ��6
void TIM6_Dis( void );							//��ֹ������ʱ��6
#endif
