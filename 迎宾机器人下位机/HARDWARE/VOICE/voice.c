/*==========================================================================
	语音控制器
	1.USART2,485通信,MODBUS协议
	2016.1.18	Li
==========================================================================*/
#include "voice.h"
#include "delay.h"

/*****************************
	函数名称：void uart2_init(u32 pclk1,u32 bound)
	功		能：串口2初始化函数
	入		参：pclk1: PCLK1时钟频率(Mhz); 最高36MHz
						bound: 波特率
	返		回：无
*****************************/
void uart2_init(u32 pclk1,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk1*1000000)/(bound*16);	//得到USARTDIV
	mantissa=temp;				 									//得到整数部分
	fraction=(temp-mantissa)*16; 						//得到小数部分	 
  mantissa<<=4;
	mantissa+=fraction;
	
	RCC->APB2ENR|=1<<2;   	//使能PORTA口时钟
	RCC->APB1ENR|=1<<17;  	//使能串口2时钟 
	GPIOA->CRL&=0XFFFF00FF;	//IO状态设置
	GPIOA->CRL|=0X00008B00;	//IO状态设置

	RCC->APB1RSTR|=1<<17;   //复位串口2
	RCC->APB1RSTR&=~(1<<17);//停止复位
	//波特率设置
 	USART2->BRR=mantissa; 	//波特率设置	 
	USART2->CR1|=0X200C;  	//使能USART, 使能发送和接收
// 	USART2->CR2|=(2<<12);  	//2位停止,无校验位.

	//使能接收中断
	USART2->CR1|=1<<8;    	//PE中断使能
	USART2->CR1|=1<<5;    	//接收缓冲区非空中断使能
	MY_NVIC_Init(2,0,USART2_IRQn,2);//组2,抢占优先级2,响应优先级0
	
	//RS485_RE(PA1)引脚初始化
	RCC->APB2ENR|=1<<2; 		//使能 PORTA 时钟
	GPIOA->CRL&=0XFFFFFF0F;
	GPIOA->CRL|=0X00000030;	//PA.1 推挽输出

	USART2->CR1&=~(1<<2);	//禁止485接收
	USART2_RS485TxMod();	//发送
}
/****************************************************
	函数名称：void USART2_IRQHandler(void),串口2接收中断
	功		能：用于接收电机驱动器和语音控制器返回的数据帧.
						其中,语音控制器的485通讯关、ID返回关,即语音控制器无数据返回,只接受数据.
	入		参：无
	返		值：无
****************************************************/
u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART2_REC_LEN个字节.
u8  USART2_RX_STA=0;//接收状态
										//bit7,接收完成标志
										//bit6~0,接收到的有效字节数目
u8 USART2_TX_STA=0;	//发送状态
										//bit7,发送完成标志
										//bit6-0,发送的有效字节数目
unsigned char Tx_count=0;//已发送字节数目
void USART2_IRQHandler(void)
{
// 	u8 res;
// 	if(USART2->SR&(1<<5))//接收到数据
// 	{
// 		res=USART2->DR;
// 		USART2_RX_BUF[USART2_RX_STA]=res;
// 		USART2_RX_STA++;
// 		TIM2_Dis();				//关定时器2中断
// 		TIM2->CNT=0;			//清计数器值
// 		TIM2_En();				//开定时器2中断
// 	}
}
// 发送一个字节数据
void USART2_SendByte(u8 data)
{
	USART2->DR=data;
	while((USART2->SR&0x0040) == 0x00);//等待发送完毕
}
/****************************************************
	函数名称：void USART2_Send(u8 *Pdata, u8 Length)
	功		能：发送Length长的字节数据
	入		参：Pdata	 : 发送数据缓冲区
						bytenum: 发送的字节数
	返		值：无
****************************************************/
void USART2_Send(u8 *Pdata, u8 Length)
{
	u16 temp;
	USART2->CR1&=~(1<<2);    	//禁止485接收,发送数据时禁止485接收,否则会多接收到一个字节0x00,发送完毕后再使能接收
	USART2_RS485RE=1;					//发送
	while((USART2->SR&0x0040) == 0x00);//等待发送完毕,(必须加,否则第一次发送数据时将出错)
	for(temp=0;temp<Length;temp++)
	{
		USART2_SendByte(Pdata[temp]);
	}
	while((USART2->SR&0x0040) == 0x00);//等待最后一个字节数据发送完毕
// 	USART2->CR1|=1<<2;    		//使能485接收
// 	USART2_RS485RE=0;					//接收模式
}
/*********************************************
	语音控制器控制函数(bound:115200,ID:0x01)
	order:指令字0x04-0x10
	num	 :段号,音频文件编号(只针对0x04、0x0B指令)
	注:0x04、0x0B指令为6个字节,需先设定数据流(段号)和校验码;
		 其他指令均固定为5个字节
*********************************************/
// 语音控制器指令码
// 指令格式:开始字(0XF0)+ID(默认为0x01)+字节个数(5、6)+指令字(0x04-0x10)+数据流(段号)+校验码(字节和)
// 					有数据流的包含6个字节(0x04和0x0B指令)
//					无数据流的包含5个字节(第六个字节置零,在数据传输时不发送,即舍去)
unsigned char SpeechContrOrder[17][6]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00},//保留
	{0x00,0x00,0x00,0x01,0x00,0x00},//保留
	{0x00,0x00,0x00,0x02,0x00,0x00},//保留
	{0x00,0x00,0x00,0x03,0x00,0x00},//保留
	{0xF0,0X01,0x06,0x04,0x01,0xFC},//播放语音指令,设置段号与校验码(5、6字节)后发送给语音控制器
	{0xF0,0X01,0x05,0x05,0xFB,0x00},//音量增加指令
	{0xF0,0X01,0x05,0x06,0xFC,0x00},//音量减小指令
	{0xF0,0X01,0x05,0x07,0xFD,0x00},//下一曲指令
	{0xF0,0X01,0x05,0x08,0xFE,0x00},//上一曲指令
	{0xF0,0X01,0x05,0x09,0xFF,0x00},//暂停指令
	{0xF0,0X01,0x05,0x0A,0x00,0x00},//停止播放,清空列表指令
	{0xF0,0X01,0x06,0x0B,0x01,0x03},//循环播放指令,设置段号与校验码(5、6字节)后发送给语音控制器
	{0xF0,0X01,0x05,0x0C,0x02,0x00},//停止循环播放指令
	{0xF0,0X01,0x05,0x0D,0x03,0x00},//485通讯开指令
	{0xF0,0X01,0x05,0x0E,0x04,0x00},//485通讯关指令
	{0xF0,0X01,0x05,0x0F,0x05,0x00},//ID返回开指令
	{0xF0,0X01,0x05,0x10,0x06,0x00},//ID返回关指令
};
void SpeechControl(unsigned char order,unsigned char num)
{
	// SpCtrOrder_Play(0x04)、SpCtrOrder_Loop(0x0B)指令包含6个字节
	if((order == SpCtrOrder_Play)||(order == SpCtrOrder_Loop))
	{
		SpeechContrOrder[order][1]=SpeechController_ID;//设置ID
		SpeechContrOrder[order][4]=num;//设置段号
		SpeechContrOrder[order][5]=SpeechContrOrder[order][0]+SpeechContrOrder[order][1]
															+SpeechContrOrder[order][2]+SpeechContrOrder[order][3]
															+SpeechContrOrder[order][4];//设置校验码
		USART2_Send(SpeechContrOrder[order],6);
	}else{
		SpeechContrOrder[order][1]=SpeechController_ID;//设置ID
		SpeechContrOrder[order][4]=SpeechContrOrder[order][0]+SpeechContrOrder[order][1]
															+SpeechContrOrder[order][2]+SpeechContrOrder[order][3];//设置校验码
		USART2_Send(SpeechContrOrder[order],5);
	}
}
/*********************************************
	函数名称：void SpeechPlayControl(unsigned char voicenum)
	功		能：播放语音<底层调用>
	入		参：voicenum：语音编号
	返		值：无
*********************************************/
unsigned char VoicePlay_Flag =0;//播放语音标志,用于清楚控制信号用
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
	函数名称：void VoicePlay(unsigned char voicenum)
	功		能：播放语音<应用层调用>
	入		参：voicenum：语音编号
	返		值：无
*********************************************/
unsigned char VoicePlayFlag					=0;						//播放语音标志位
unsigned char StopFlag							=1;						//已停止播放语音标志，0：未停止,	1：已停止(用于障碍语音)
unsigned char VoiceNum							=0;						//要播放的语音编号
void VoicePlay(unsigned char voicenum)
{
	if(NULL != voicenum)//播放语音段
	{
		VoiceNum=voicenum;
		VoicePlayFlag=1;			
	}else{//停止循环播放障碍物语音段
		VoicePlayFlag=0;			
		VoiceNum=NULL;
	}
}
/*********************************************
	函数名称：void SpeechCtr_Init( void )
	功		能：语音控制器初始化操作,停止所有语音的循环播放,防止出错.
	入		参：无
	返		值：无
*********************************************/
void SpeechCtr_Init( void )
{
// 	uart2_init(36,115200);		//串口2初始化,8+n+1,用于语音控制器的485通讯
// 	delay_ms(5);
// 	SpeechControl(SpCtrOrder_StopLoop,StartupVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,BarrierVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,TakeMealVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,SendEndVoice);
// 	SpeechControl(SpCtrOrder_StopLoop,StartSendVoice);
	
	RCC->APB2ENR|=1<<6;//先使能外设IO PORTE时钟
	GPIOE->CRL&=0XF00000FF;	//PE2 3 4 5 6 推挽输出
	GPIOE->CRL|=0X03333300;
	
	RCC->APB2ENR|=1<<4;//先使能外设IO PORTC时钟
	GPIOC->CRH&=0X000FFFFF;	//PE13 14 15推挽输出
	GPIOC->CRH|=0X33300000;
	
	RCC->APB2ENR|=1<<7;//先使能外设IO PORTF时钟
	GPIOF->CRL&=0XFFFF0000;	//PF0 1 2 3推挽输出
	GPIOF->CRL|=0X00003333;
	
	PEout(2)=0;PEout(3)=0;PEout(4)=0;PEout(5)=0;PEout(6)=0;
	delay_ms(1000);									//必须有,等待语音控制器上电初始化完毕
	delay_ms(1000);
	VoicePlay(StartupVoice);				//播放启动语音
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
