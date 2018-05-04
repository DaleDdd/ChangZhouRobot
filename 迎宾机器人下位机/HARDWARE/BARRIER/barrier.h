#ifndef _BARRIER_H
#define _BARRIER_H
#include "sys.h"

#define BARRIER_INTERRUPT_EN 0 //障碍物检测中断使能，1:enable; 0:disable

#define LINE0	PCin(0)	//红外线光电开关中断输入
#define LINE1	PCin(1)	//红外线光电开关中断输入
#define BreakSignal1	PAin(0)//急停信号1
#define BreakSignal2	PCin(1)//急停信号2

#ifndef FarBarrier_Foward
#define FarBarrier_Foward			0x01
#define FarBarrier_Backward		0x02
#define NearBarrier_Foward		0x04
#define NearBarrier_Backward	0x08
#define Barrier_Left					0x10
#define Barrier_Right					0x20
#endif

#ifndef Touch_Head
#define Touch_Head						0x01//头部
#define Touch_Shoulder					0x02//肩部
#define Touch_Chest						0x04//胸部
#define Touch_Loin						0x08//腰部
#define Touch_5							0x10
#define Touch_6							0x20
#define Touch_7							0x40
#define Touch_8							0x80
#endif

#ifndef BreakSignal_up_foward
#define BreakSignal_up_foward				PAin(0)//上_前急停
#define BreakSignal_up_backward			PAin(1)//上_后急停
#define BreakSignal_dn_foward				PAin(4)//下_前急停
#define BreakSignal_dn_backward			PAin(6)//下_后急停
#define LowSpeedSignal_dn_foward		PAin(5)//下_前减速
#define LowSpeedSignal_dn_backward	PAin(7)//下_后减速
#define TurnSignal_up_left					PAin(2)//上_左转体
#define TurnSignal_up_right					PAin(3)//上_右转体
#define TurnSignal_dn_left					PBin(0)//下_左转体
#define TurnSignal_dn_right					PBin(1)//下_右转体
#endif

#define BarrierSta_No								0//无障碍物
#define BarrierSta_NotMan						1//非人障碍物
#define BarrierSta_Man							2//障碍物是人
#define BarrierSta_DecSpeed					3//减速标志

#ifndef	NoBarrier
#define NoBarrier		( ( unsigned char ) 0 )	//无障碍物标志
#define NearBarrier	( ( unsigned char ) 1 )	//近距离障碍物标志
#define FarBarrier	( ( unsigned char ) 2 )	//远距离障碍物标志
#endif

extern unsigned char BarrierFlag;
extern unsigned char BarrierFlag_last;
extern unsigned char BarrierStatus_Reg;
void HumanBodyDetect( unsigned char * barsta );
void Barrier_Init( void );
void BarrierTest( void );
void BarrierScan( void );
unsigned char TouchScan( void );
#endif
