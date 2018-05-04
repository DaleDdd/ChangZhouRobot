#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"

#define USART3_REC_LEN  	100   //定义串口3最大接收字节数 200
#define EN_USART3_RX			0			//使能（1）/禁止（0）串口3接收
extern u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符

extern unsigned char UartReceiveStatus;
extern unsigned char UartReceiveBuff[8];
extern u16 USART3_RX_STA;         				//接收状态标记

void USART3_SendByte(u8 data); // 发送一个字节数据
void USART3_Send(u8 *Pdata, u8 Length); // 发送Length长的字节数据
// 注意:串口1挂在APB2上,时钟源为PCLK2,最高72MHz;
// 			串口2-5挂在APB1上,时钟源为PCLK1,最高36MHz;
void uart3_init(u32 pclk1,u32 bound); //串口3初始化
#endif	   





