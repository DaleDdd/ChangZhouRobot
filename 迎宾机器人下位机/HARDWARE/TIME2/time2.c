/*==========================================================================
	timer2设置
	1.语音控制器485 1ms通信超时计时
	2.
	2016.1.18	Li
==========================================================================*/
#include "time2.h"

/*********************************************
	通用定时器2中断初始化
	这里时钟选择为APB1的2倍，而APB1为36MHz
	arr：自动重装值。
	psc：时钟预分频数
*********************************************/
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2时钟使能 
 	TIM2->ARR=arr;  		//设定计数器自动重装值
	TIM2->PSC=psc;  		//预分频器7200,得到10Khz的计数时钟	
	
	/*产生一个更新事件,使预转载寄存器的内容传送至影子寄存器,
		确保第一次RS485接收数据定时时间的准确性.*/
	TIM2_En();
	TIM2->EGR|=1<<0;		//产生一个更新事件
	TIM2_Dis();
	
	TIM2->DIER|=1<<0;   //允许更新中断
  MY_NVIC_Init(2,0,TIM2_IRQn,2);//抢占2,子优先级0,组2
	
	TIM2_Dis();
}
/*********************************************
	清定时器2的计数器
*********************************************/
void TIM2_ClrCont( void )
{
	TIM2->CNT=0;    //清定时器2的计数器
}
/*********************************************
	使能通用定时器2
*********************************************/
void TIM2_En( void )
{
	TIM2->CR1|=0x01;    //使能定时器2
}
/*********************************************
	禁止通用定时器2
*********************************************/
void TIM2_Dis( void )
{
	TIM2->CR1&=~0x01;    //禁止定时器2
}
/*********************************************
	函数名称：void TIM2_IRQHandler(void)定时器2中断服务程序,1ms中断.
	功		能：485接收数据超时中断.进中断说明接收数据超时,一帧数据结束.
	入		参：无
	返		回：无
*********************************************/
u8 RS485_Rx_Flag	=0;	//485接收到一帧数据标志,0:无数据,1:接收完成一帧数据
void TIM2_IRQHandler(void)
{
	if(TIM2->SR&0X0001)//溢出中断
	{
		TIM2->SR&=~(1<<0);//清除中断标志位
		RS485_Rx_Flag=1;	//接收完成
		TIM2_Dis();				//关中断
	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//


