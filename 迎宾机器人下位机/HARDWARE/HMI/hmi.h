#ifndef __HMI_H
#define __HMI_H
#include "sys.h"

#define RX_BUF_LEN	20
#define TX_BUF_LEN	70
extern u8  USART1_RX_BUF[RX_BUF_LEN]; //���ջ���,���20���ֽ�.
extern u8  USART1_TX_BUF[TX_BUF_LEN]; //���ͻ���
extern u8  USART1_RX_STA;     //����״̬���
extern unsigned char 			 TIM2_IRQSource;
extern const unsigned char HMI_TRIG;
extern void TIM2_ClrCont( void );					//�嶨ʱ��5�ļ�����
extern void TIM2_En( void );							//ʹ��ͨ�ö�ʱ��5
extern void TIM2_Dis( void );							//��ֹͨ�ö�ʱ��5
void HMI_Init( void );										//HMI�ӿڳ�ʼ��,19200,8+n+1
// ע��:����1����APB2��,ʱ��ԴΪPCLK2,���72MHz;
// 			����2-5����APB1��,ʱ��ԴΪPCLK1,���36MHz;
void USART1_En( void );										//����1���ͺ���
void uart1_init(u32 pclk2,u32 bound); 		//����1��ʼ��
void USART1_SendByte(u8 data); 						// ����һ���ֽ�����
void USART1_Send(u8 *Pdata, u8 Length); 	// ����Length�����ֽ�����
#endif
