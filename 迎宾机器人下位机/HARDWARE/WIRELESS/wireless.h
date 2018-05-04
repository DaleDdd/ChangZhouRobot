#ifndef __WIRELESS_H
#define __WIRELESS_H
#include "sys.h"

extern u8  USART5_TX_BUF[20];	//发送缓冲区
extern u8  USART5_RX_BUF[20]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u8  USART5_RX_STA;     //接收状态标记
extern const unsigned char WIRELESS_TRIG;
extern unsigned char 			 TIM5_IRQSource;

extern void TIM5_ClrCont( void );					//清定时器5的计数器
extern void TIM5_En( void );							//使能通用定时器5
extern void TIM5_Dis( void );							//禁止通用定时器5
void Wireless_Init( void );								//串口5初始化,8+n+1,用于无线串口模块的通讯
void USART5_SendByte(u8 data); 						// 发送一个字节数据
void USART5_SendString(u8 *Pdata);				// 发送字符串
void USART5_Send(u8 *Pdata, u8 Length); 	// 发送Length长的字节数据
void uart5_init(u32 pclk1,u32 bound); 		//串口5初始化
void USART5_En( void );										//USART5 使能
#endif
