#ifndef __ID_H
#define __ID_H
#include "sys.h"

// ID������ö�ٶ���
#define crosspoint	(unsigned char) 0x01//�����
#define table				(unsigned char) 0x02//����
#define curvestart	(unsigned char) 0x03//��ʼ���
#define curvestop		(unsigned char) 0x04//�������
#define taskover		(unsigned char) 0x05//���������־
#define takefood		(unsigned char) 0x06//���ȡ�͵�

extern unsigned char ID;									//ID����,0-255
extern unsigned char	IDNumReceiveStatus;	//����״̬��־(NULL:δ���ܵ���OK:������ȷ��ERROR:���մ���),���ֶ�����
extern const unsigned char ID_TRIG;
extern unsigned char 			 TIM5_IRQSource;
extern u8  USART4_RX_BUF[20]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�
extern u8  USART4_RX_STA;         				//����״̬���

extern void TIM6_ClrCont( void );					//�嶨ʱ��6�ļ�����
extern void TIM6_En( void );							//ʹ��ͨ�ö�ʱ��6
extern void TIM6_Dis( void );							//��ֹͨ�ö�ʱ��6
void ID_Init(void);												//ID���ӿڳ�ʼ��
void uart4_init(u32 pclk1,u32 bound); 		//����3��ʼ��
void USART4_En( void );										//USART4 ʹ��
void USART4_SendByte(u8 data); 						// ����һ���ֽ�����
void USART4_Send(u8 *Pdata, u8 Length); 	// ����Length�����ֽ�����
#endif

