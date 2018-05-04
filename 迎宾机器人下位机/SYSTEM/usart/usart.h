#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"

#define USART3_REC_LEN  	100   //���崮��3�������ֽ��� 200
#define EN_USART3_RX			0			//ʹ�ܣ�1��/��ֹ��0������3����
extern u8  USART3_RX_BUF[USART3_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�

extern unsigned char UartReceiveStatus;
extern unsigned char UartReceiveBuff[8];
extern u16 USART3_RX_STA;         				//����״̬���

void USART3_SendByte(u8 data); // ����һ���ֽ�����
void USART3_Send(u8 *Pdata, u8 Length); // ����Length�����ֽ�����
// ע��:����1����APB2��,ʱ��ԴΪPCLK2,���72MHz;
// 			����2-5����APB1��,ʱ��ԴΪPCLK1,���36MHz;
void uart3_init(u32 pclk1,u32 bound); //����3��ʼ��
#endif	   





