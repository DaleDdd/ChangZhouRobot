/*==========================================================================
	光电开关避障
	1.中断输入脚
		INT1->PC0 LINE0(避障减速)
		INT2->PC1 LINE1(紧急停止)
	2.红外光电开关均为OC门输出
	3.LINE0/INT1中断,减速,有远距离障碍物
		LINE1/INT2中断,停车,有近距离障碍物
	2016.1.18	Li
==========================================================================*/
#include "barrier.h"
#include "delay.h"
#include "wireless.h"

/*****************************
	函数名称：void Barrier_Init( void )
	功		能：光电开关避障IO端口初始化
						PC0(INT1)、PC1(INT2)
	入		参：无
	返		回：无
*****************************/
void Barrier_Init( void )
{
	RCC->APB2ENR|=1<<4;			//使能PORTC时钟
	RCC->APB2ENR|=1<<3;			//使能PORTB时钟
	RCC->APB2ENR|=1<<2;			//使能PORTA时钟
	
	GPIOA->CRL&=0X00000000;	//PA0 1 2 3 4 5 6 7  浮空输入
	GPIOA->CRL|=0X88888888;
	GPIOB->CRL&=0XFFFFFF00;	//PB0 1  浮空输入
	GPIOB->CRL|=0X00000088;
	GPIOC->CRL&=0XFFFFFF00;	//PC0 1浮空输入
	GPIOC->CRL|=0X00000088;
	
#if	BARRIER_INTERRUPT_EN
	Ex_NVIC_Config(GPIO_C,0,FTIR|RTIR); //任意触发电平触发(上升沿、下降沿)
	Ex_NVIC_Config(GPIO_C,1,FTIR|RTIR); //任意触发电平触发(上升沿、下降沿)
	MY_NVIC_Init(2,3,EXTI0_IRQn,2); 		//抢占 3，子优先级 3，组 2
	MY_NVIC_Init(2,3,EXTI1_IRQn,2); 		//抢占 2，子优先级 3，组 2
	delay_ms(600);											//延时,避免以上中断对以下代码的影响
#endif
// 	/*	上电障碍物检测	*/
// 	if( BreakSignal1 == 0 || BreakSignal2 == 0 )				//近距离障碍物
// 	{
// 		BarrierFlag=NearBarrier;
// 	}else	if( LINE0 == 0 )	//远距离障碍物
// 	{
// 		BarrierFlag=FarBarrier;
// 	}else										//无障碍物
// 	{
// 		BarrierFlag=NoBarrier;
// 	}
}
/*****************************
	函数名称：void EXTI0_IRQHandler(void)
	功		能：外部中断(上升沿、下降沿) 0 服务程序
	入		参：无
	返		回：无【避障减速传感器输入】
*****************************/
unsigned char BarrierFlag=0;//障碍标志, NearBarrier(1):近距离障碍物, FarBarrier(2):远距离障碍物, NoBarrier(0):无障碍物
unsigned char BarrierFlag_last=NoBarrier;
// void EXTI0_IRQHandler(void)
// {
// 	if(LINE0==0)		//有远距离障碍物
// 	{
// 		if(LINE1==0)	//有近距离障碍物
// 			BarrierFlag=NearBarrier;
// 		else					//有远距离障碍物
// 			BarrierFlag=FarBarrier;
// 	}
// 	else						//无障碍物
// 	{
// 		if(LINE1==0)	//有近距离障碍物
// 			BarrierFlag=NearBarrier;
// 		else					//无障碍物
// 			BarrierFlag=NoBarrier;
// 	}
// 	EXTI->PR=1<<0; 	//清除 LINE0 上的中断标志位
// }
// /*****************************
// 	函数名称：void EXTI1_IRQHandler(void)
// 	功		能：外部中断(上升沿、下降沿) 1 服务程序
// 	入		参：无
// 	返		回：无【紧急停止避障传感器输入】
// *****************************/
// void EXTI1_IRQHandler(void)
// {
// 	if(LINE1==0)	//有近距离障碍物
// 	{
// 		BarrierFlag=NearBarrier;
// 	}
// 	else					//无近距离障碍物,判断是否有远距离障碍物
// 	{
// 		if(LINE0==0)		//有远距离障碍物
// 			BarrierFlag=FarBarrier;
// 		else						//无远距离障碍物
// 			BarrierFlag=NoBarrier;
// 	}
// 	EXTI->PR=1<<1; 	//清除 LINE1 上的中断标志位
// }
/*****************************
	函数名称：void BarrierScan( void )
	功		能：障碍物扫描
	入		参：无
	返		回：无
*****************************/
unsigned char BarrierStatus_Reg;
void BarrierScan( void )
{
		BarrierStatus_Reg=0x00;
		
		if((0==BreakSignal_up_foward) || (0==BreakSignal_dn_foward))
		{	BarrierStatus_Reg|=NearBarrier_Foward;			}
		
		//if((0==BreakSignal_up_backward) || (0==BreakSignal_dn_backward))
		//{	BarrierStatus_Reg|=NearBarrier_Backward;		}
		
		if(0==LowSpeedSignal_dn_foward)
		{	BarrierStatus_Reg|=FarBarrier_Foward;				}
		
		//(0==LowSpeedSignal_dn_backward)
		//{	BarrierStatus_Reg|=FarBarrier_Backward;			}

		if(0==TurnSignal_up_left) // (0==TurnSignal_dn_left)
		{	BarrierStatus_Reg|=Barrier_Left;						}
		
		if(0==TurnSignal_up_right) // (0==TurnSignal_dn_right)
		{	BarrierStatus_Reg|=Barrier_Right;						}
}
/*****************************
	函数名称：unsigned char TouchScan( void )
	功		能：触摸扫描
	入		参：无
	返		回：TouchStatus_Reg
*****************************/
unsigned char TouchScan( void )
{
		unsigned char TouchStatus_Reg=0x00;
		
		if(0==BreakSignal_dn_backward)//头
		{	TouchStatus_Reg|=Touch_Head;						}
				
		if(0==LowSpeedSignal_dn_backward)//肩
		{	TouchStatus_Reg|=Touch_Shoulder;				}

// 		if(0==TurnSignal_dn_left)//胸
// 		{	TouchStatus_Reg|=Touch_Chest;						}
// 		
// 		if(0==TurnSignal_dn_right)//腰
// 		{	TouchStatus_Reg|=Touch_Loin;						}
		
		return TouchStatus_Reg;
}
/*****************************
	函数名称：void BarrierTest( void )
	功		能：障碍物距离测试
	入		参：无
	返		回：无
*****************************/
void BarrierTest( void )
{
		if( BarrierFlag == NoBarrier )
		{
			USART5_SendString("No barrier!\r\n");
		}
		if( BarrierFlag == NearBarrier )
		{
			USART5_SendString("Near barrier!\r\n");
		}
		if( BarrierFlag == FarBarrier )
		{
			USART5_SendString("Far barrier!\r\n");
		}
}
//上+下避障检测人体
void HumanBodyDetect( unsigned char * barsta )
{
	unsigned char barstapre = *barsta;
	static unsigned char timecount=0,timecount1=0;//非0时不检测,时间长度：timecount/10 s
	static unsigned char detectdisenflag=0,detectdisenflag1=0;//检测禁止标志
	
	/*	倒计时	*/
	if(timecount)
	{	timecount--;	return;}
	if(timecount1)
	{	timecount1--;	return;}
	/*	检测		*/
	if(((BreakSignal_up_foward==0) || (BreakSignal_dn_foward==0)) && (barstapre!=BarrierSta_Man))//人
	{
		if(!detectdisenflag)
		{	detectdisenflag=1;	timecount=13;	*barsta = BarrierSta_NotMan;}
		else
		{ detectdisenflag=0;	*barsta = BarrierSta_Man;	}
		return;
	}
	if(detectdisenflag) detectdisenflag=0;
	if(((BreakSignal_up_foward==0) || (BreakSignal_dn_foward==0)) && (barstapre==BarrierSta_Man))//人
	{	*barsta = BarrierSta_Man;	 return;	}
	if(barstapre==BarrierSta_Man)//人
	{
		if(!detectdisenflag1)
		{	detectdisenflag1=1;	timecount1=13;	*barsta = BarrierSta_Man; return;}
		else
		{ detectdisenflag1=0;	*barsta = BarrierSta_NotMan;	}
	}
	if(detectdisenflag1) detectdisenflag1=0;
	if((BreakSignal_up_foward==1) && (BreakSignal_dn_foward==1) && (LowSpeedSignal_dn_foward==1))//无
	{	*barsta = BarrierSta_No; return;	}
	if((BreakSignal_up_foward==1) && (BreakSignal_dn_foward==1) && (LowSpeedSignal_dn_foward==0))//减速
	{	*barsta = BarrierSta_DecSpeed; return;	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
