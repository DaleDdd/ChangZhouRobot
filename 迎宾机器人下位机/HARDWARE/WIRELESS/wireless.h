#ifndef __WIRELESS_H
#define __WIRELESS_H
#include "sys.h"

extern u8  USART5_TX_BUF[20];	//���ͻ�����
extern u8  USART5_RX_BUF[20]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�
extern u8  USART5_RX_STA;     //����״̬���
extern const unsigned char WIRELESS_TRIG;
extern unsigned char 			 TIM5_IRQSource;

extern void TIM5_ClrCont( void );					//�嶨ʱ��5�ļ�����
extern void TIM5_En( void );							//ʹ��ͨ�ö�ʱ��5
extern void TIM5_Dis( void );							//��ֹͨ�ö�ʱ��5
void Wireless_Init( void );								//����5��ʼ��,8+n+1,�������ߴ���ģ���ͨѶ
void USART5_SendByte(u8 data); 						// ����һ���ֽ�����
void USART5_SendString(u8 *Pdata);				// �����ַ���
void USART5_Send(u8 *Pdata, u8 Length); 	// ����Length�����ֽ�����
void uart5_init(u32 pclk1,u32 bound); 		//����5��ʼ��
void USART5_En( void );										//USART5 ʹ��
#endif
