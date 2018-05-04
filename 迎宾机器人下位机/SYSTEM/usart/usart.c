//===================================================
// ����1-5��ʼ��,�жϽ���
// ע:����1	 ����APB2��,ʱ��Ϊpclk2,���72MHz
//		����2-5����APB1��,ʱ��Ϊpclk1,���36MHz
// 2015-8-24,Li
//===================================================
#include "usart.h"
#include "delay.h"
#include	"id.h"

extern unsigned char ReceiveOrder;

/****************************************************
	����3��ʼ������
	pclk1:PCLK1ʱ��Ƶ��(Mhz); ���36MHz
	bound:������
****************************************************/
void uart3_init(u32 pclk1,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);	//�õ�USARTDIV
	mantissa=temp;				 									//�õ���������
	fraction=(temp-mantissa)*16; 						//�õ�С������	 
  mantissa<<=4;
	mantissa+=fraction;
	
	RCC->APB2ENR|=1<<3;   	//ʹ��PORTB��ʱ��
	RCC->APB1ENR|=1<<18;  	//ʹ�ܴ���3ʱ�� 
	GPIOB->CRH&=0XFFFF00FF;	//IO״̬����
	GPIOB->CRH|=0X00008B00;	//IO״̬����

	RCC->APB1RSTR|=1<<18;   //��λ����3
	RCC->APB1RSTR&=~(1<<18);//ֹͣ��λ
	//����������
 	USART3->BRR=mantissa; 	// ����������	 
	USART3->CR1|=0X200C;  	//1λֹͣ,��У��λ.
#if EN_USART3_RX		  		//���ʹ���˽���
	//ʹ�ܽ����ж�
	USART3->CR1|=1<<8;    	//PE�ж�ʹ��
	USART3->CR1|=1<<5;    	//���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(2,1,USART3_IRQn,2);//��2,��ռ���ȼ�2,��Ӧ���ȼ�1
#endif
}
/****************************************************
	����3���ͺ���
****************************************************/
// ����һ���ֽ�����
void USART3_SendByte(u8 data)
{
	USART3->DR=data;
	while((USART3->SR&0x0080) == 0x00);//�ȴ��������ݼĴ���Ϊ��
}
// ����Length�����ֽ�����
void USART3_Send(u8 *Pdata, u8 Length)
{
	u8 temp;
	for(temp=0;temp<Length;temp++)
	{
		USART3_SendByte(Pdata[temp]);
	}	
}
/****************************************************
	����3�����ж�
****************************************************/
#if EN_USART3_RX   //���ʹ���˽���
u8  USART3_RX_BUF[USART3_REC_LEN]; //���ջ���,���USART2_REC_LEN���ֽ�.
u16 USART3_RX_STA=0; //����״̬���
										//����״̬
										//bit15��	������ɱ�־
										//bit14��	���յ�0x0d
										//bit13~0��	���յ�����Ч�ֽ���Ŀ
void USART3_IRQHandler(void)
{
//	u8 res;
	if(USART3->SR&(1<<5))//���յ�����
	{	 
		//USART3_SendByte(res);
	}
}
#endif






















