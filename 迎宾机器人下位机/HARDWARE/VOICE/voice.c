/*==========================================================================
	����������
	1.USART2,485ͨ��,MODBUSЭ��
	2016.1.18	Li
==========================================================================*/
#include "voice.h"
#include "delay.h"

/*****************************
	�������ƣ�void uart2_init(u32 pclk1,u32 bound)
	��		�ܣ�����2��ʼ������
	��		�Σ�pclk1: PCLK1ʱ��Ƶ��(Mhz); ���36MHz
						bound: ������
	��		�أ���
*****************************/
void uart2_init(u32 pclk1,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);	//�õ�USARTDIV
	mantissa=temp;				 									//�õ���������
	fraction=(temp-mantissa)*16; 						//�õ�С������	 
  mantissa<<=4;
	mantissa+=fraction;
	
	RCC->APB2ENR|=1<<2;   	//ʹ��PORTA��ʱ��
	RCC->APB1ENR|=1<<17;  	//ʹ�ܴ���2ʱ�� 
	GPIOA->CRL&=0XFFFF00FF;	//IO״̬����
	GPIOA->CRL|=0X00008B00;	//IO״̬����

	RCC->APB1RSTR|=1<<17;   //��λ����2
	RCC->APB1RSTR&=~(1<<17);//ֹͣ��λ
	//����������
 	USART2->BRR=mantissa; 	//����������	 
	USART2->CR1|=0X200C;  	//ʹ��USART, ʹ�ܷ��ͺͽ���
// 	USART2->CR2|=(2<<12);  	//2λֹͣ,��У��λ.

	//ʹ�ܽ����ж�
	USART2->CR1|=1<<8;    	//PE�ж�ʹ��
	USART2->CR1|=1<<5;    	//���ջ������ǿ��ж�ʹ��
	MY_NVIC_Init(2,0,USART2_IRQn,2);//��2,��ռ���ȼ�2,��Ӧ���ȼ�0
	
	//RS485_RE(PA1)���ų�ʼ��
	RCC->APB2ENR|=1<<2; 		//ʹ�� PORTA ʱ��
	GPIOA->CRL&=0XFFFFFF0F;
	GPIOA->CRL|=0X00000030;	//PA.1 �������

	USART2->CR1&=~(1<<2);	//��ֹ485����
	USART2_RS485TxMod();	//����
}
/****************************************************
	�������ƣ�void USART2_IRQHandler(void),����2�����ж�
	��		�ܣ����ڽ��յ�����������������������ص�����֡.
						����,������������485ͨѶ�ء�ID���ع�,�����������������ݷ���,ֻ��������.
	��		�Σ���
	��		ֵ����
****************************************************/
u8  USART2_RX_BUF[USART2_REC_LEN]; //���ջ���,���USART2_REC_LEN���ֽ�.
u8  USART2_RX_STA=0;//����״̬
										//bit7,������ɱ�־
										//bit6~0,���յ�����Ч�ֽ���Ŀ
u8 USART2_TX_STA=0;	//����״̬
										//bit7,������ɱ�־
										//bit6-0,���͵���Ч�ֽ���Ŀ
unsigned char Tx_count=0;//�ѷ����ֽ���Ŀ
void USART2_IRQHandler(void)
{
// 	u8 res;
// 	if(USART2->SR&(1<<5))//���յ�����
// 	{
// 		res=USART2->DR;
// 		USART2_RX_BUF[USART2_RX_STA]=res;
// 		USART2_RX_STA++;
// 		TIM2_Dis();				//�ض�ʱ��2�ж�
// 		TIM2->CNT=0;			//�������ֵ
// 		TIM2_En();				//����ʱ��2�ж�
// 	}
}
// ����һ���ֽ�����
void USART2_SendByte(u8 data)
{
	USART2->DR=data;
	while((USART2->SR&0x0040) == 0x00);//�ȴ��������
}
/****************************************************
	�������ƣ�void USART2_Send(u8 *Pdata, u8 Length)
	��		�ܣ�����Length�����ֽ�����
	��		�Σ�Pdata	 : �������ݻ�����
						bytenum: ���͵��ֽ���
	��		ֵ����
****************************************************/
void USART2_Send(u8 *Pdata, u8 Length)
{
	u16 temp;
	USART2->CR1&=~(1<<2);    	//��ֹ485����,��������ʱ��ֹ485����,��������յ�һ���ֽ�0x00,������Ϻ���ʹ�ܽ���
	USART2_RS485RE=1;					//����
	while((USART2->SR&0x0040) == 0x00);//�ȴ��������,(�����,�����һ�η�������ʱ������)
	for(temp=0;temp<Length;temp++)
	{
		USART2_SendByte(Pdata[temp]);
	}
	while((USART2->SR&0x0040) == 0x00);//�ȴ����һ���ֽ����ݷ������
// 	USART2->CR1|=1<<2;    		//ʹ��485����
// 	USART2_RS485RE=0;					//����ģʽ
}
/*********************************************
	�������������ƺ���(bound:115200,ID:0x01)
	order:ָ����0x04-0x10
	num	 :�κ�,��Ƶ�ļ����(ֻ���0x04��0x0Bָ��)
	ע:0x04��0x0Bָ��Ϊ6���ֽ�,�����趨������(�κ�)��У����;
		 ����ָ����̶�Ϊ5���ֽ�
*********************************************/
// ����������ָ����
// ָ���ʽ:��ʼ��(0XF0)+ID(Ĭ��Ϊ0x01)+�ֽڸ���(5��6)+ָ����(0x04-0x10)+������(�κ�)+У����(�ֽں�)
// 					���������İ���6���ֽ�(0x04��0x0Bָ��)
//					���������İ���5���ֽ�(�������ֽ�����,�����ݴ���ʱ������,����ȥ)
unsigned char SpeechContrOrder[17][6]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00},//����
	{0x00,0x00,0x00,0x01,0x00,0x00},//����
	{0x00,0x00,0x00,0x02,0x00,0x00},//����
	{0x00,0x00,0x00,0x03,0x00,0x00},//����
	{0xF0,0X01,0x06,0x04,0x01,0xFC},//��������ָ��,���öκ���У����(5��6�ֽ�)���͸�����������
	{0xF0,0X01,0x05,0x05,0xFB,0x00},//��������ָ��
	{0xF0,0X01,0x05,0x06,0xFC,0x00},//������Сָ��
	{0xF0,0X01,0x05,0x07,0xFD,0x00},//��һ��ָ��
	{0xF0,0X01,0x05,0x08,0xFE,0x00},//��һ��ָ��
	{0xF0,0X01,0x05,0x09,0xFF,0x00},//��ָͣ��
	{0xF0,0X01,0x05,0x0A,0x00,0x00},//ֹͣ����,����б�ָ��
	{0xF0,0X01,0x06,0x0B,0x01,0x03},//ѭ������ָ��,���öκ���У����(5��6�ֽ�)���͸�����������
	{0xF0,0X01,0x05,0x0C,0x02,0x00},//ֹͣѭ������ָ��
	{0xF0,0X01,0x05,0x0D,0x03,0x00},//485ͨѶ��ָ��
	{0xF0,0X01,0x05,0x0E,0x04,0x00},//485ͨѶ��ָ��
	{0xF0,0X01,0x05,0x0F,0x05,0x00},//ID���ؿ�ָ��
	{0xF0,0X01,0x05,0x10,0x06,0x00},//ID���ع�ָ��
};
void SpeechControl(unsigned char order,unsigned char num)
{
	// SpCtrOrder_Play(0x04)��SpCtrOrder_Loop(0x0B)ָ�����6���ֽ�
	if((order == SpCtrOrder_Play)||(order == SpCtrOrder_Loop))
	{
		SpeechContrOrder[order][1]=SpeechController_ID;//����ID
		SpeechContrOrder[order][4]=num;//���öκ�
		SpeechContrOrder[order][5]=SpeechContrOrder[order][0]+SpeechContrOrder[order][1]
															+SpeechContrOrder[order][2]+SpeechContrOrder[order][3]
															+SpeechContrOrder[order][4];//����У����
		USART2_Send(SpeechContrOrder[order],6);
	}else{
		SpeechContrOrder[order][1]=SpeechController_ID;//����ID
		SpeechContrOrder[order][4]=SpeechContrOrder[order][0]+SpeechContrOrder[order][1]
															+SpeechContrOrder[order][2]+SpeechContrOrder[order][3];//����У����
		USART2_Send(SpeechContrOrder[order],5);
	}
}
/*********************************************
	�������ƣ�void SpeechPlayControl(unsigned char voicenum)
	��		�ܣ���������<�ײ����>
	��		�Σ�voicenum���������
	��		ֵ����
*********************************************/
unsigned char VoicePlay_Flag =0;//����������־,������������ź���
void SpeechPlayControl(unsigned char voicenum)
{
	if(NULL != voicenum)
	{
			VoicePlay_Flag = 1;
			switch( voicenum )
			{
				case VOICEPLAY_NUM01:			PEout(2)=1;
					break;
				case VOICEPLAY_NUM02:			PEout(3)=1;
					break;
				case VOICEPLAY_NUM03:			PEout(4)=1;
					break;
				case VOICEPLAY_NUM04:			PEout(5)=1;
					break;
				case VOICEPLAY_NUM05:			PEout(6)=1;
					break;
				case VOICEPLAY_NUM06:			PCout(13)=1;
					break;
				case VOICEPLAY_NUM07:			PCout(14)=1;
					break;
				case VOICEPLAY_NUM08:			PCout(15)=1;
					break;
				case VOICEPLAY_NUM09:			PFout(0)=1;
					break;
				case VOICEPLAY_NUM10:			PFout(1)=1;
					break;
				case VOICEPLAY_NUM11:			PFout(2)=1;
					break;
				case VOICEPLAY_NUM12:			PFout(3)=1;
					break;
				default:
					break;
			}
	}
}
/*********************************************
	�������ƣ�void VoicePlay(unsigned char voicenum)
	��		�ܣ���������<Ӧ�ò����>
	��		�Σ�voicenum���������
	��		ֵ����
*********************************************/
unsigned char VoicePlayFlag					=0;						//����������־λ
unsigned char StopFlag							=1;						//��ֹͣ����������־��0��δֹͣ,	1����ֹͣ(�����ϰ�����)
unsigned char VoiceNum							=0;						//Ҫ���ŵ��������
void VoicePlay(unsigned char voicenum)
{
	if(NULL != voicenum)//����������
	{
		VoiceNum=voicenum;
		VoicePlayFlag=1;			
	}else{//ֹͣѭ�������ϰ���������
		VoicePlayFlag=0;			
		VoiceNum=NULL;
	}
}
/*********************************************
	�������ƣ�void SpeechCtr_Init( void )
	��		�ܣ�������������ʼ������,ֹͣ����������ѭ������,��ֹ����.
	��		�Σ���
	��		ֵ����
*********************************************/
void SpeechCtr_Init( void )
{
// 	uart2_init(36,115200);		//����2��ʼ��,8+n+1,����������������485ͨѶ
// 	delay_ms(5);
// 	SpeechControl(SpCtrOrder_StopLoop,StartupVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,BarrierVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,TakeMealVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,SendEndVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,StartSendVoice);
	
	RCC->APB2ENR|=1<<6;//��ʹ������IO PORTEʱ��
	GPIOE->CRL&=0XF00000FF;	//PE2 3 4 5 6 �������
	GPIOE->CRL|=0X03333300;
	
	RCC->APB2ENR|=1<<4;//��ʹ������IO PORTCʱ��
	GPIOC->CRH&=0X000FFFFF;	//PE13 14 15�������
	GPIOC->CRH|=0X33300000;
	
	RCC->APB2ENR|=1<<7;//��ʹ������IO PORTFʱ��
	GPIOF->CRL&=0XFFFF0000;	//PF0 1 2 3�������
	GPIOF->CRL|=0X00003333;
	
	PEout(2)=0;PEout(3)=0;PEout(4)=0;PEout(5)=0;PEout(6)=0;
	delay_ms(1000);									//������,�ȴ������������ϵ��ʼ�����
	delay_ms(1000);
	VoicePlay(StartupVoice);				//������������
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
