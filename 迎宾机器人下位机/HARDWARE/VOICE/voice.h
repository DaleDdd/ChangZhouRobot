#ifndef __VOICE_H
#define __VOICE_H
#include "sys.h"


// 485通讯的从机ID========================================
#define SpeechController_ID		0x01//语音控制器ID号
#define MotorDriverLeft_ID		0x02//左电机驱动器ID号
#define MotorDriverRight_ID		0x03//右电机驱动器ID号

// 语音控制器指令字=======================================
#define SpCtrOrder_Play				0x04//播放语音指令字	(6个字节)
#define SpCtrOrder_VolInc			0x05//音量增加指令字	(5个字节)
#define SpCtrOrder_VolDec			0x06//音量减小指令字	(5个字节)
#define SpCtrOrder_Next				0x07//下一曲指令字		(5个字节)
#define SpCtrOrder_Prev				0x08//上一曲指令字		(5个字节)
#define SpCtrOrder_Pause			0x09//暂停指令字			(5个字节)
#define SpCtrOrder_Stop				0x0A//停止指令字			(5个字节)
#define SpCtrOrder_Loop				0x0B//循环播放指令字	(6个字节)
#define SpCtrOrder_StopLoop		0x0C//停止循环指令字	(5个字节)
#define SpCtrOrder_485Op			0x0D//485通讯开指令字	(5个字节)
#define SpCtrOrder_485Cl			0x0E//485通讯关指令字	(5个字节)
#define SpCtrOrder_IDBackOp		0x0F//ID返回开指令字	(5个字节)
#define SpCtrOrder_IDBackCl		0x10//ID返回关指令字	(5个字节)
// 语音库编号
#define VOICEPLAY_NUM01				(unsigned char) 1
#define VOICEPLAY_NUM02				(unsigned char) 2
#define VOICEPLAY_NUM03				(unsigned char) 3
#define VOICEPLAY_NUM04				(unsigned char) 4
#define VOICEPLAY_NUM05				(unsigned char) 5
#define VOICEPLAY_NUM06				(unsigned char) 6
#define VOICEPLAY_NUM07				(unsigned char) 7
#define VOICEPLAY_NUM08				(unsigned char) 8
#define VOICEPLAY_NUM09				(unsigned char) 9
#define VOICEPLAY_NUM10				(unsigned char) 10
#define VOICEPLAY_NUM11				(unsigned char) 11
#define VOICEPLAY_NUM12				(unsigned char) 12

#define StartupVoice					VOICEPLAY_NUM01			//启动语音
#define BarrierVoice					VOICEPLAY_NUM02			//障碍语音
#define HeadVoice							VOICEPLAY_NUM11			//头
#define LoinVoice							VOICEPLAY_NUM12			//腰
// #define ShoulderVoice					VOICEPLAY_NUM12			//肩
// #define ChestVoice						VOICEPLAY_NUM05			//胸

#define TracklessLineVoice		VOICEPLAY_NUM03			//无轨直线行走时循环播放语音段
// 语音播放控制变量
extern unsigned char VoicePlayFlag;				//播放语音标志位
extern unsigned char StopFlag;						//已停止播放语音标志，0：未停止,	1：已停止(用于障碍语音)
extern unsigned char VoiceNum;						//要播放的语音编号
extern unsigned char VoicePlay_Flag;			//播放语音标志,用于清楚控制信号用
#define USART2_RS485RE		PAout(1)//RS485输出使能,拉低为接收模式,拉高为发送模式
#define USART2_RS485RxMod()	{USART2_RS485RE = 0;}//接收模式
#define USART2_RS485TxMod()	{USART2_RS485RE = 1;}//发送模式
#define USART2_REC_LEN  	10   //定义串口2最大接收字节数 10
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u8  USART2_RX_STA;         				//接收状态标记

extern void TIM2_ClrCont( void );
extern void TIM2_En( void );
extern void TIM2_Dis( void );
void USART2_SendByte(u8 data); // 发送一个字节数据
void USART2_Send(u8 *Pdata, u8 Length); // 串口2发送数据

void SpeechControl(unsigned char order,unsigned char num);//语音控制器控制函数(bound:115200,ID:0x01)
void SpeechPlayControl(unsigned char voicenum);//播放语音<底层调用>
void VoicePlay(unsigned char voicenum);//播放语音<应用层调用>
void SpeechCtr_Init( void );//语音控制器初始化操作,停止所有语音的循环播放,防止出错.
#endif
