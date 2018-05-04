#ifndef __ID_H
#define __ID_H
#include "sys.h"

// ID卡类型枚举定义
#define crosspoint	(unsigned char) 0x01//交叉点
#define table				(unsigned char) 0x02//桌号
#define curvestart	(unsigned char) 0x03//起始弯道
#define curvestop		(unsigned char) 0x04//结束弯道
#define taskover		(unsigned char) 0x05//任务结束标志
#define takefood		(unsigned char) 0x06//后厨取餐点

extern unsigned char ID;									//ID卡号,0-255
extern unsigned char	IDNumReceiveStatus;	//接收状态标志(NULL:未接受到；OK:接收正确；ERROR:接收错误),需手动清零
extern const unsigned char ID_TRIG;
extern unsigned char 			 TIM5_IRQSource;
extern u8  USART4_RX_BUF[20]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u8  USART4_RX_STA;         				//接收状态标记

extern void TIM6_ClrCont( void );					//清定时器6的计数器
extern void TIM6_En( void );							//使能通用定时器6
extern void TIM6_Dis( void );							//禁止通用定时器6
void ID_Init(void);												//ID卡接口初始化
void uart4_init(u32 pclk1,u32 bound); 		//串口3初始化
void USART4_En( void );										//USART4 使能
void USART4_SendByte(u8 data); 						// 发送一个字节数据
void USART4_Send(u8 *Pdata, u8 Length); 	// 发送Length长的字节数据
#endif

