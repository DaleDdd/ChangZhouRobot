#ifndef __TIME5_H
#define __TIME5_H
#include "sys.h"

/*	上位机通讯指令码定义	*/

/*	返回给上位机的状态码定义	*/
#define ReceiveError					(unsigned char) 0x00 //接收上位机数据失败
#define ReceiveOk							(unsigned char) 0x01 //接收上位机数据成功
extern unsigned char CommandReceiveStatus;
extern unsigned char TaskTableBuf;//送餐桌号缓冲区
extern unsigned char GetCtrAreaStatusFlag;//得到管制区状态标志位
extern unsigned char 			 TIM2_IRQSource;
extern unsigned char 			 TIM5_IRQSource;
extern const unsigned char ID_TRIG;
extern const unsigned char WIRELESS_TRIG;
extern const unsigned char HMI_TRIG;
extern signed 		int SpeedSet_HV;
extern signed 		int SpeedSearchLine;
extern signed  		int SpeedIncA;  /*加速加速度,每100ms增加的速度*/
extern signed  		int SpeedDecA;
extern float Kp;											 //比例
extern float Ki;
extern float Kd;
extern float k,b;//比例与误差之间线性化系数
extern unsigned char STATMOD_FLAG;
extern unsigned char STATIM_FLAG;
extern unsigned char CROSS_FLAG;
extern unsigned char Recive_data[50];
extern unsigned char Recive_num;
void TIM5_Int_Init(u16 arr,u16 psc);//通用定时器5中断初始化
void TIM5_ClrCont( void );					//清定时器5的计数器
void TIM5_En( void );								//使能通用定时器5
void TIM5_Dis( void );							//禁止通用定时器5

void TIM2_Int_Init(u16 arr,u16 psc);//通用定时器2中断初始化
void TIM2_ClrCont( void );					//清定时器2的计数器
void TIM2_En( void );								//使能通用定时器2
void TIM2_Dis( void );							//禁止通用定时器2

void TIM6_Int_Init(u16 arr,u16 psc);//基本定时器6中断初始化
void TIM6_ClrCont( void );					//清定时器6的计数器
void TIM6_En( void );								//使能基本定时器6
void TIM6_Dis( void );							//禁止基本定时器6
#endif
