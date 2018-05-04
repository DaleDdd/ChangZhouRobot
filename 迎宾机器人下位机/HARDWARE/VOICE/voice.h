#ifndef __VOICE_H
#define __VOICE_H
#include "sys.h"


// 485ͨѶ�Ĵӻ�ID========================================
#define SpeechController_ID		0x01//����������ID��
#define MotorDriverLeft_ID		0x02//����������ID��
#define MotorDriverRight_ID		0x03//�ҵ��������ID��

// ����������ָ����=======================================
#define SpCtrOrder_Play				0x04//��������ָ����	(6���ֽ�)
#define SpCtrOrder_VolInc			0x05//��������ָ����	(5���ֽ�)
#define SpCtrOrder_VolDec			0x06//������Сָ����	(5���ֽ�)
#define SpCtrOrder_Next				0x07//��һ��ָ����		(5���ֽ�)
#define SpCtrOrder_Prev				0x08//��һ��ָ����		(5���ֽ�)
#define SpCtrOrder_Pause			0x09//��ָͣ����			(5���ֽ�)
#define SpCtrOrder_Stop				0x0A//ָֹͣ����			(5���ֽ�)
#define SpCtrOrder_Loop				0x0B//ѭ������ָ����	(6���ֽ�)
#define SpCtrOrder_StopLoop		0x0C//ֹͣѭ��ָ����	(5���ֽ�)
#define SpCtrOrder_485Op			0x0D//485ͨѶ��ָ����	(5���ֽ�)
#define SpCtrOrder_485Cl			0x0E//485ͨѶ��ָ����	(5���ֽ�)
#define SpCtrOrder_IDBackOp		0x0F//ID���ؿ�ָ����	(5���ֽ�)
#define SpCtrOrder_IDBackCl		0x10//ID���ع�ָ����	(5���ֽ�)
// ��������
#define VOICEPLAY_NUM01				(unsigned char) 1
#define VOICEPLAY_NUM02				(unsigned char) 2
#define VOICEPLAY_NUM03				(unsigned char) 3
#define VOICEPLAY_NUM04				(unsigned char) 4
#define VOICEPLAY_NUM05				(unsigned char) 5
#define VOICEPLAY_NUM06				(unsigned char) 6
#define VOICEPLAY_NUM07				(unsigned char) 7
#define VOICEPLAY_NUM08				(unsigned char) 8
#define VOICEPLAY_NUM09				(unsigned char) 9
#define VOICEPLAY_NUM10				(unsigned char) 10
#define VOICEPLAY_NUM11				(unsigned char) 11
#define VOICEPLAY_NUM12				(unsigned char) 12

#define StartupVoice					VOICEPLAY_NUM01			//��������
#define BarrierVoice					VOICEPLAY_NUM02			//�ϰ�����
#define HeadVoice							VOICEPLAY_NUM11			//ͷ
#define LoinVoice							VOICEPLAY_NUM12			//��
// #define ShoulderVoice					VOICEPLAY_NUM12			//��
// #define ChestVoice						VOICEPLAY_NUM05			//��

#define TracklessLineVoice		VOICEPLAY_NUM03			//�޹�ֱ������ʱѭ������������
// �������ſ��Ʊ���
extern unsigned char VoicePlayFlag;				//����������־λ
extern unsigned char StopFlag;						//��ֹͣ����������־��0��δֹͣ,	1����ֹͣ(�����ϰ�����)
extern unsigned char VoiceNum;						//Ҫ���ŵ��������
extern unsigned char VoicePlay_Flag;			//����������־,������������ź���
#define USART2_RS485RE		PAout(1)//RS485���ʹ��,����Ϊ����ģʽ,����Ϊ����ģʽ
#define USART2_RS485RxMod()	{USART2_RS485RE = 0;}//����ģʽ
#define USART2_RS485TxMod()	{USART2_RS485RE = 1;}//����ģʽ
#define USART2_REC_LEN  	10   //���崮��2�������ֽ��� 10
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�
extern u8  USART2_RX_STA;         				//����״̬���

extern void TIM2_ClrCont( void );
extern void TIM2_En( void );
extern void TIM2_Dis( void );
void USART2_SendByte(u8 data); // ����һ���ֽ�����
void USART2_Send(u8 *Pdata, u8 Length); // ����2��������

void SpeechControl(unsigned char order,unsigned char num);//�������������ƺ���(bound:115200,ID:0x01)
void SpeechPlayControl(unsigned char voicenum);//��������<�ײ����>
void VoicePlay(unsigned char voicenum);//��������<Ӧ�ò����>
void SpeechCtr_Init( void );//������������ʼ������,ֹͣ����������ѭ������,��ֹ����.
#endif
