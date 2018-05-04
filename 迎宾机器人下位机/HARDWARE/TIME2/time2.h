#ifndef __TIME2_H
#define __TIME2_H
#include "sys.h"

extern u8  RS485_Rx_Flag;									//485接收到一帧数据标志,0:无数据,1:接收完成一帧数据

void TIM2_Int_Init(u16 arr,u16 psc);//通用定时器2中断初始化
void TIM2_ClrCont( void );//清定时器2的计数器
void TIM2_En( void );//使能通用定时器2
void TIM2_Dis( void );//禁止通用定时器2
void TIM2_IRQHandler(void);//485接收数据超时中断.进中断说明接收数据超时,一帧数据结束.

#endif
