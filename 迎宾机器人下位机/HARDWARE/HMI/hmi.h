#ifndef __HMI_H
#define __HMI_H
#include "sys.h"

#define RX_BUF_LEN	20
#define TX_BUF_LEN	70
extern u8  USART1_RX_BUF[RX_BUF_LEN]; //接收缓冲,最大20个字节.
extern u8  USART1_TX_BUF[TX_BUF_LEN]; //发送缓冲
extern u8  USART1_RX_STA;     //接收状态标记
extern unsigned char 			 TIM2_IRQSource;
extern const unsigned char HMI_TRIG;
extern void TIM2_ClrCont( void );					//清定时器5的计数器
extern void TIM2_En( void );							//使能通用定时器5
extern void TIM2_Dis( void );							//禁止通用定时器5
void HMI_Init( void );										//HMI接口初始化,19200,8+n+1
// 注意:串口1挂在APB2上,时钟源为PCLK2,最高72MHz;
// 			串口2-5挂在APB1上,时钟源为PCLK1,最高36MHz;
void USART1_En( void );										//串口1发送函数
void uart1_init(u32 pclk2,u32 bound); 		//串口1初始化
void USART1_SendByte(u8 data); 						// 发送一个字节数据
void USART1_Send(u8 *Pdata, u8 Length); 	// 发送Length长的字节数据
#endif
