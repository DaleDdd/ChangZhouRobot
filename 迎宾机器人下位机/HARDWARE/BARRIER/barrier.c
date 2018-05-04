/*==========================================================================
	��翪�ر���
	1.�ж������
		INT1->PC0 LINE0(���ϼ���)
		INT2->PC1 LINE1(����ֹͣ)
	2.�����翪�ؾ�ΪOC�����
	3.LINE0/INT1�ж�,����,��Զ�����ϰ���
		LINE1/INT2�ж�,ͣ��,�н������ϰ���
	2016.1.18	Li
==========================================================================*/
#include "barrier.h"
#include "delay.h"
#include "wireless.h"

/*****************************
	�������ƣ�void Barrier_Init( void )
	��		�ܣ���翪�ر���IO�˿ڳ�ʼ��
						PC0(INT1)��PC1(INT2)
	��		�Σ���
	��		�أ���
*****************************/
void Barrier_Init( void )
{
	RCC->APB2ENR|=1<<4;			//ʹ��PORTCʱ��
	RCC->APB2ENR|=1<<3;			//ʹ��PORTBʱ��
	RCC->APB2ENR|=1<<2;			//ʹ��PORTAʱ��
	
	GPIOA->CRL&=0X00000000;	//PA0 1 2 3 4 5 6 7  ��������
	GPIOA->CRL|=0X88888888;
	GPIOB->CRL&=0XFFFFFF00;	//PB0 1  ��������
	GPIOB->CRL|=0X00000088;
	GPIOC->CRL&=0XFFFFFF00;	//PC0 1��������
	GPIOC->CRL|=0X00000088;
	
#if	BARRIER_INTERRUPT_EN
	Ex_NVIC_Config(GPIO_C,0,FTIR|RTIR); //���ⴥ����ƽ����(�����ء��½���)
	Ex_NVIC_Config(GPIO_C,1,FTIR|RTIR); //���ⴥ����ƽ����(�����ء��½���)
	MY_NVIC_Init(2,3,EXTI0_IRQn,2); 		//��ռ 3�������ȼ� 3���� 2
	MY_NVIC_Init(2,3,EXTI1_IRQn,2); 		//��ռ 2�������ȼ� 3���� 2
	delay_ms(600);											//��ʱ,���������ж϶����´����Ӱ��
#endif
// 	/*	�ϵ��ϰ�����	*/
// 	if( BreakSignal1 == 0 || BreakSignal2 == 0 )				//�������ϰ���
// 	{
// 		BarrierFlag=NearBarrier;
// 	}else	if( LINE0 == 0 )	//Զ�����ϰ���
// 	{
// 		BarrierFlag=FarBarrier;
// 	}else										//���ϰ���
// 	{
// 		BarrierFlag=NoBarrier;
// 	}
}
/*****************************
	�������ƣ�void EXTI0_IRQHandler(void)
	��		�ܣ��ⲿ�ж�(�����ء��½���) 0 �������
	��		�Σ���
	��		�أ��ޡ����ϼ��ٴ��������롿
*****************************/
unsigned char BarrierFlag=0;//�ϰ���־, NearBarrier(1):�������ϰ���, FarBarrier(2):Զ�����ϰ���, NoBarrier(0):���ϰ���
unsigned char BarrierFlag_last=NoBarrier;
// void EXTI0_IRQHandler(void)
// {
// 	if(LINE0==0)		//��Զ�����ϰ���
// 	{
// 		if(LINE1==0)	//�н������ϰ���
// 			BarrierFlag=NearBarrier;
// 		else					//��Զ�����ϰ���
// 			BarrierFlag=FarBarrier;
// 	}
// 	else						//���ϰ���
// 	{
// 		if(LINE1==0)	//�н������ϰ���
// 			BarrierFlag=NearBarrier;
// 		else					//���ϰ���
// 			BarrierFlag=NoBarrier;
// 	}
// 	EXTI->PR=1<<0; 	//��� LINE0 �ϵ��жϱ�־λ
// }
// /*****************************
// 	�������ƣ�void EXTI1_IRQHandler(void)
// 	��		�ܣ��ⲿ�ж�(�����ء��½���) 1 �������
// 	��		�Σ���
// 	��		�أ��ޡ�����ֹͣ���ϴ��������롿
// *****************************/
// void EXTI1_IRQHandler(void)
// {
// 	if(LINE1==0)	//�н������ϰ���
// 	{
// 		BarrierFlag=NearBarrier;
// 	}
// 	else					//�޽������ϰ���,�ж��Ƿ���Զ�����ϰ���
// 	{
// 		if(LINE0==0)		//��Զ�����ϰ���
// 			BarrierFlag=FarBarrier;
// 		else						//��Զ�����ϰ���
// 			BarrierFlag=NoBarrier;
// 	}
// 	EXTI->PR=1<<1; 	//��� LINE1 �ϵ��жϱ�־λ
// }
/*****************************
	�������ƣ�void BarrierScan( void )
	��		�ܣ��ϰ���ɨ��
	��		�Σ���
	��		�أ���
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
	�������ƣ�unsigned char TouchScan( void )
	��		�ܣ�����ɨ��
	��		�Σ���
	��		�أ�TouchStatus_Reg
*****************************/
unsigned char TouchScan( void )
{
		unsigned char TouchStatus_Reg=0x00;
		
		if(0==BreakSignal_dn_backward)//ͷ
		{	TouchStatus_Reg|=Touch_Head;						}
				
		if(0==LowSpeedSignal_dn_backward)//��
		{	TouchStatus_Reg|=Touch_Shoulder;				}

// 		if(0==TurnSignal_dn_left)//��
// 		{	TouchStatus_Reg|=Touch_Chest;						}
// 		
// 		if(0==TurnSignal_dn_right)//��
// 		{	TouchStatus_Reg|=Touch_Loin;						}
		
		return TouchStatus_Reg;
}
/*****************************
	�������ƣ�void BarrierTest( void )
	��		�ܣ��ϰ���������
	��		�Σ���
	��		�أ���
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
//��+�±��ϼ������
void HumanBodyDetect( unsigned char * barsta )
{
	unsigned char barstapre = *barsta;
	static unsigned char timecount=0,timecount1=0;//��0ʱ�����,ʱ�䳤�ȣ�timecount/10 s
	static unsigned char detectdisenflag=0,detectdisenflag1=0;//����ֹ��־
	
	/*	����ʱ	*/
	if(timecount)
	{	timecount--;	return;}
	if(timecount1)
	{	timecount1--;	return;}
	/*	���		*/
	if(((BreakSignal_up_foward==0) || (BreakSignal_dn_foward==0)) && (barstapre!=BarrierSta_Man))//��
	{
		if(!detectdisenflag)
		{	detectdisenflag=1;	timecount=13;	*barsta = BarrierSta_NotMan;}
		else
		{ detectdisenflag=0;	*barsta = BarrierSta_Man;	}
		return;
	}
	if(detectdisenflag) detectdisenflag=0;
	if(((BreakSignal_up_foward==0) || (BreakSignal_dn_foward==0)) && (barstapre==BarrierSta_Man))//��
	{	*barsta = BarrierSta_Man;	 return;	}
	if(barstapre==BarrierSta_Man)//��
	{
		if(!detectdisenflag1)
		{	detectdisenflag1=1;	timecount1=13;	*barsta = BarrierSta_Man; return;}
		else
		{ detectdisenflag1=0;	*barsta = BarrierSta_NotMan;	}
	}
	if(detectdisenflag1) detectdisenflag1=0;
	if((BreakSignal_up_foward==1) && (BreakSignal_dn_foward==1) && (LowSpeedSignal_dn_foward==1))//��
	{	*barsta = BarrierSta_No; return;	}
	if((BreakSignal_up_foward==1) && (BreakSignal_dn_foward==1) && (LowSpeedSignal_dn_foward==0))//����
	{	*barsta = BarrierSta_DecSpeed; return;	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
