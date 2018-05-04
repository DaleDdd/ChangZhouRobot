#ifndef __ROUTEMSG_H
#define __ROUTEMSG_H
#include "stmflash.h"
#include "24cxx.h"

#define SPEEDMOD_M	0
#define SPEEDMOD_H	1
#define SPEEDMOD_L	2

/* **************
路径结构体定义：
	 机器人ID/编号
	 餐桌方向
	 左转转弯点卡号
	 右转转弯点卡号
	 速度模式
	 ************** */
struct ROUTEMSG{
	unsigned char robotid;	//机器人编号
	unsigned char tabledir;	//餐桌方向
	unsigned char route_l;	//左转弯点卡号
	unsigned char route_r;	//右转弯点卡号
	unsigned char speedmod; //速度模式,SPEEDMOD_H(1)、SPEEDMOD_M(0)、SPEEDMOD_L(2)
};
typedef struct tagtoutemsg{
	unsigned  char command;				//控制指令码
	unsigned  char playvoicenume; 		//放音编号1-12
	unsigned  char barrierstatus; 		//障碍物状态
	unsigned  char speedmod; 			//速度模式,SPEEDMOD_H(1)、SPEEDMOD_M(0)、SPEEDMOD_L(2)
	unsigned  char Station_mode[20];	//20个站点转向模式
	unsigned  char Station_tim[20];	    //20个站点停靠时间
	unsigned  char Station_DIR;			//初始点转向设置
	unsigned char Cross[10];			//10个分叉点设置
}ROUTEMSG1;

struct VOICEMSG{
	unsigned short time;//语音时长
	unsigned char  mod; //语音模式
	unsigned char  runmod;//行走模式
};
extern unsigned char RouteSelDir;	//路径选择方向，MID、LEFT、RIGHT
extern unsigned char Turn90Dir;		//90°转体方向，MID、LEFT、RIGHT
extern unsigned char Turn180Dir;	//180°转体方向，LEFT、RIGHT
extern struct ROUTEMSG RouteMsg;	//路径暂存变量
extern struct VOICEMSG VoiceMsg[11];//语音段时长 模式信息
extern signed 		int SpeedSet_HV;
extern ROUTEMSG1 RouteMsg1;
extern unsigned char Voice_ok;
/* 基地址定义 */
#define DATA_BASE						(0x0807D800)								//数据区基地址
// #define ROBOTID_ADDR				(DATA_BASE)									//机器人ID/编号存储地址
#define INFALLCLEAR_BASE		(unsigned long)0x0807FFFE		//所有的数据区清零标志基地址

/*	寄存器地址定义	*/
#define COMMAND_REG				1
#define VOICENUM_REG			2
#define BARRIERSTA_REG			3
#define SPEEDMOD_REG			4
#define ROUTEMOD_REG			5
#define NODE_1_REG				6
#define NODE_2_REG				7
#define NODE_3_REG				8
#define NODE_4_REG				9
#define NODE_5_REG				10
#define DISTANCE_REG			11
#define WUDIALOGUE_REG		12//唤醒语音识别模块的对话功能
#define Voice_OK            14

/* 基地址定义（语音段设置） */
#define VOICE_TIME_SET_BASEADDR		DATA_BASE							//语音段1-10的时长存储基地址
#define VOICE_MOD_SET_BASEADDR		(VOICE_TIME_SET_BASEADDR+2*10)		//语音段1-10的播放模式存储基地址
#define VOICE_RMOD_SET_BASEADDR		(VOICE_MOD_SET_BASEADDR+2*10)		//语音段1-10的行走模式存储基地址
#define SPEEDMOD_BASEADDR			(VOICE_RMOD_SET_BASEADDR+2*10) 		//速度模式基地址
#define ROBOTMOD_TRACK_BASEADDR		(SPEEDMOD_BASEADDR+20)				//机器人模式(0:有轨,1:无轨)
#define StationDIR_BASEADDR			(ROBOTMOD_TRACK_BASEADDR+2)			//初始点转向模式
#define STATION_MODE_BASEADDR       (StationDIR_BASEADDR+20)			//站点转向模式
#define STATION_TIME_BASEADDR       (STATION_MODE_BASEADDR+60)			//站点停靠时间
#define CROSS_SET_BASEADDR          (STATION_TIME_BASEADDR+60)			//分叉点设置


#define VOICE_TIME_ADDR(n)	(VOICE_TIME_SET_BASEADDR+(n-1)*2)			//语音段n(1-10)的时长
#define VOICE_MOD_ADDR(n)		(VOICE_MOD_SET_BASEADDR+(n-1)*2)			//语音段n(1-10)的播放模式
#define VOICE_RMOD_ADDR(n)	(VOICE_RMOD_SET_BASEADDR+(n-1)*2)			//语音段n(1-10)的行走模式
#define STATION_MODE(n)		(STATION_MODE_BASEADDR+(n-1)*2)
#define STATION_TIM(n)		(STATION_TIME_BASEADDR+(n-1)*2)
#define CROSS_SET(n)		(CROSS_SET_BASEADDR+(n-1)*2)

/*	相关常量宏定义	*/
#define MID							((unsigned char) 0x00)	//方向中,表示在交叉点处直走、餐桌处不转体
#define LEFT						((unsigned char) 0x01)	//方向左,表示在交叉点处左转、餐桌在左侧
#define RIGHT						((unsigned char) 0x02)	//方向右,表示在交叉点处右转、餐桌在右侧
#define MID_BACK				((unsigned char) 0x03)	//表示餐桌处不转体，且送完餐后180°转体原路返回
#define LEFT_BACK				((unsigned char) 0x04)	//表示餐桌在左侧，且送完餐后左转体原路返回
#define RIGHT_BACK			((unsigned char) 0x05)	//表示餐桌在右侧，且送完餐后右转体原路返回
#define VOICEMOD_NULL							(0)//停止播放语音
#define VOICEMOD_Single 					(1)//单曲点播模式（读取地标播放）
#define VOICEMOD_MULLOOP 					(2)//多曲循环模式
#define VOICEMOD_TRACKLESSLINE		(3)//无轨直线行走模式
#define VOICERUNMOD_0							(0)//行走模式 0
#define VOICERUNMOD_1							(1)//行走模式 1
#define VOICERUNMOD_2							(2)//行走模式 2
#define VOICERUNMOD_3							(3)//行走模式 3
#define VOICERUNMOD_4							(4)//行走模式 4
#define VOICERUNMOD_5							(5)//行走模式 5
#define VOICERUNMOD_6							(6)//行走模式 6
#define ROBOTMOD_TRACK						(0)//机器人有轨模式
#define ROBOTMOD_TRACKLESS				(1)//机器人无轨模式
#define SumTableNum			(unsigned char) 99		//餐桌总数

void RobotIDSet(unsigned short robotid);														//设置机器人编号
void TableDIRSet(unsigned char tablenum, unsigned short tabledir);	//设置餐桌的餐桌方向
void RouteLeftSet(unsigned char tablenum, unsigned short routel);		//设置餐桌的左转弯点路径（卡号）
void RouteRightSet(unsigned char tablenum, unsigned short router);	//设置餐桌的右转弯点路径（卡号）
void SpeedModSet(unsigned short speedmod);													//设置速度模式
void RobotModTrackSet(unsigned short robotmod);											//设置机器人模式
void StationDirSet(unsigned short tracklessdist);								//设置无轨距离
void RouteInfInit(unsigned char mod);																//所有的数据区初始化清零
unsigned char RobotIDRead( void );																	//获取机器人编号
unsigned char TableDIRRead(unsigned char tablenum);									//获取餐桌的方向
unsigned char RouteLeftRead(unsigned char tablenum);								//获取餐桌的左转弯点路径（卡号）
unsigned char RouteRightRead(unsigned char tablenum);								//获取餐桌的右转弯点路径（卡号）
unsigned char SpeedModRead(void);																		//获取速度模式
unsigned char RobotModTrackRead(void);															//获取机器人模式(0:有轨,1:无轨)
unsigned char StationDirRead(void);															//获取无轨距离
unsigned short ReadRegData( unsigned short regaddr );
void WriteRegData( unsigned short regaddr, unsigned short regdata );
void VoiceTimeSet(unsigned char voicenum,unsigned short voicetime);
void VoiceModSet(unsigned char voicenum,unsigned short voicemod);
void VoiceRunModSet(unsigned char voicenum,unsigned short voicerunmod);
unsigned short VoiceTimeRead(unsigned char voicenum);
unsigned char VoiceModRead(unsigned char voicenum);
unsigned char VoiceRunModRead(unsigned char voicenum);
unsigned short Station_modeRead(unsigned char station_id);
void Station_modeSet(unsigned char station_id,unsigned short mode);
void Station_timSet(unsigned char station_id,unsigned short tim);
void Cross_Set(unsigned cross_id,unsigned short dir);
unsigned short Cross_Read(unsigned char cross_id);
unsigned short Station_timRead(unsigned char station_id);
unsigned char RouteMsg_SpeedModGet( void );
#endif
