//===================================================
// 串口1-5初始化,中断接收
// 注:串口1	 挂在APB2上,时钟为pclk2,最高72MHz
//		串口2-5挂在APB1上,时钟为pclk1,最高36MHz
// 2015-8-24,Li
//===================================================
#include "usart.h"
#include "delay.h"
#include	"id.h"

extern unsigned char ReceiveOrder;

/****************************************************
	串口3初始化函数
	pclk1:PCLK1时钟频率(Mhz); 最高36MHz
	bound:波特率
****************************************************/
void uart3_init(u32 pclk1,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);	//得到USARTDIV
	mantissa=temp;				 									//得到整数部分
	fraction=(temp-mantissa)*16; 						//得到小数部分	 
  mantissa<<=4;
	mantissa+=fraction;
	
	RCC->APB2ENR|=1<<3;   	//使能PORTB口时钟
	RCC->APB1ENR|=1<<18;  	//使能串口3时钟 
	GPIOB->CRH&=0XFFFF00FF;	//IO状态设置
	GPIOB->CRH|=0X00008B00;	//IO状态设置

	RCC->APB1RSTR|=1<<18;   //复位串口3
	RCC->APB1RSTR&=~(1<<18);//停止复位
	//波特率设置
 	USART3->BRR=mantissa; 	// 波特率设置	 
	USART3->CR1|=0X200C;  	//1位停止,无校验位.
#if EN_USART3_RX		  		//如果使能了接收
	//使能接收中断
	USART3->CR1|=1<<8;    	//PE中断使能
	USART3->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(2,1,USART3_IRQn,2);//组2,抢占优先级2,响应优先级1
#endif
}
/****************************************************
	串口3发送函数
****************************************************/
// 发送一个字节数据
void USART3_SendByte(u8 data)
{
	USART3->DR=data;
	while((USART3->SR&0x0080) == 0x00);//等待发送数据寄存器为空
}
// 发送Length长的字节数据
void USART3_Send(u8 *Pdata, u8 Length)
{
	u8 temp;
	for(temp=0;temp<Length;temp++)
	{
		USART3_SendByte(Pdata[temp]);
	}	
}
/****************************************************
	串口3接收中断
****************************************************/
#if EN_USART3_RX   //如果使能了接收
u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART2_REC_LEN个字节.
u16 USART3_RX_STA=0; //接收状态标记
										//接收状态
										//bit15，	接收完成标志
										//bit14，	接收到0x0d
										//bit13~0，	接收到的有效字节数目
void USART3_IRQHandler(void)
{
//	u8 res;
	if(USART3->SR&(1<<5))//接收到数据
	{	 
		//USART3_SendByte(res);
	}
}
#endif






















