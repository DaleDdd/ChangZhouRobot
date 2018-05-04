/*---------------------------------------------------------------------------------------
		Catering Robot V3.1,2016-8-31,Li
		
		功能：迎宾，单独的设置界面，可设语音1-10的时间、播放模式、行走模式，以及速度。
					启动：开始播报语音、前进；
					停止：停止播报语音、前进；
					退出：退出当前界面。
					
		一、可屏蔽中断优先级配置说明:
			中断源						抢占优先级			响应优先级			分组
			USART1(HMI主设备)			1								3							2
			USART2(语音控制器)		2								0							2
			USART4(ID卡)					0								3							2
			USART5(无线模块)			1								3							2
			TIM2(数据超时定时)		2								3							2
			TIM3(基本伺服周期)		3								2							2
			TIM4(PWM输出)					
			TIM5(数据超时定时)		2								3							2
			TIM6(数据超时定时)		0								3							2
			LINE0(避障外部中断)		2								3							2
			LINE1(避障外部中断)		2								3							2
			LINE2(暂停外部中断)		3								3							2
			LINE3(返回外部中断)		3								3							2
			
		二、USART串口分配说明:
			USART1->HMI接口,19200,8+n+1
			USART2->
			USART3->
			USART4->ID读卡器,9600,8+n+1
			USART5->无线串口模块,19200、8+n+1

		三、存在的问题:
			(1)开机运行时,485发送的第一条命令无效.<2015-8-28>
				解决方法:在485发送数据之前加一条等待语句,while((USART2->SR&0x0040) == 0x00),等待数据发送完毕.
				
			(2)每次接收驱动器返回的数据时,都会先额外收到一个0x00字节.<2015-8-28>
				解决方法:在485发送数据的时候,禁止接收,等发送完成后在使能接收.
				
			(3)在定时器3的中断服务程序中写入控制PI控制算法后,程序执行出现不正常运行.在每个伺服周期(20或40ms)
				内对每一个驱动器速度控制输出(RS485 115200bps下,大约需要0.9ms)一次命令,并接收(RS485 115200bps下,
				大约需要2.6ms),但实际运行时程序无法正常执行.<2015-9-29>
				解决方法:TIM3的中断优先级必须小于串口2(USART2)的接收中断优先级以及TIM2的中断优先级,才能在TIM3
								 的中断里进入串口2以及TIM2的中断.<<根本原因>>
								 (同一程序中不要设置两个分组,抢占优先级大于响应优先级,数值越小优先级越高,高抢占优先级
								 可以打断低抢占优先级的执行,高响应优先级不能打断低响应优先级的执行)
				
			(4)控制器与驱动器485通讯时在最后多收一个0x00字节.<2015-9-29>
				解决方法:去掉控制器485芯片并联的120Ω电阻.一般在排除其他通讯不稳定的因素下,才考虑并联120Ω的终端
				电阻(阻抗匹配电阻).
				
			(5)FLASH的写SECTOR操作大约要80ms,因此在定时器3的中断中不可调用,否则将导致程序跑飞.因此在送餐过程
				中的转体时间不可实时记录,否则,一但在记录(写入FLASH)过程中复位或掉电,将导致程序跑飞.<2015-11-7>

---------------------------------------------------------------------------------------*/
#include <math.h>
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "id.h"
#include "wdg.h"
#include "stmflash.h"
#include "24cxx.h"
#include "crc16.h"
#include "include.h"
#include "barrier.h"
#include "back.h"
#include "voice.h"
#include "motioncontrol.h"
#include "time5.h"
#include "wireless.h"
#include "hmi.h"
#include "routemsg.h"

// 宏定义*********************************************************************************************************************************
#define IWDEN							(1u)						//使能内部看门狗
#define SPEEDDOWNEN				(1u)						//弯道降速1：降速；0：不降速
#define INCOMPLETEPIDEN		(1u)						//不完全微分算法1：采用不完全微分算法；0：不采用
#define DECACC_CHANGE_EN	(1u)						//减速度加速度随速度变化使能
#define TRACKLESS_BACKEN	(0u)						//无轨模式下后退使能
#define VOICEBARRIERTIME	(7u)						//避障语音设定时间,单位：秒【实际语音时长+间隔时长(默认为5秒)】
#define BARRIERPLAYCONTEN	(0u)						//避障语音连续播放使能

// 端口定义=============================================
#define ST								PFout(10)				//外部看门狗复位引脚

// 磁导向传感器引脚定义=================================
#define GUIDESENSOR_D1		PDin(10)
#define GUIDESENSOR_D2		PDin(9)
#define GUIDESENSOR_D3		PDin(8)
#define GUIDESENSOR_D4		PBin(15)
#define GUIDESENSOR_D5		PDin(11)
#define GUIDESENSOR_D6		PDin(12)
#define GUIDESENSOR_D7		PDin(13)
#define GUIDESENSOR_D8		PDin(14)
#define GUIDESENSOR_D9		PGin(4)
#define GUIDESENSOR_D10		PGin(3)
#define GUIDESENSOR_D11		PGin(2)
#define GUIDESENSOR_D12 	PDin(15)
#define GUIDESENSOR_D13		PGin(5)
#define GUIDESENSOR_D14		PGin(6)
#define GUIDESENSOR_D15		PGin(7)
#define GUIDESENSOR_D16		PGin(8)

// 运动控制相关宏=========================================
#define NORMAL								0x01	//运行模式：正常PID寻线模式
#define STOP									0x02	//运行模式：停车模式
#define	TURN_90								0x03	//运行模式：转体模式
#define ROUTESEL							0x04	//运行模式：路径选择模式
#define TURN_180							0x05	//运行模式：原地转体180°模式
#define TURN_180_Trackless		0x06	//运行模式：无轨转体180°
#define BACK_Trackless				0x07	//运行模式：无轨后退
#define	GOING									0x01	//正在执行--停车和转体模式的执行状态--
#define	END										0x02	//执行结束--停车和转体模式的执行状态--
#define SENDING								0x01	//送餐状态,转体送餐,正向转体90°,播放送餐语音
#define SENDBACK							0x02	//送餐状态,送餐返回,反向转体90°,不播放语音
#define STOPIDTIMING					0x01	//停止、取餐点地标有效计时,开始送餐后计时两秒有效
#define STOPIDVALID						0x02	//停止、取餐点地标有效
#define STOPIDUNVALID					0x03	//停止、取餐点地标无效,
#define RUNFLAG_NULL					0x00	//
#define RUNFLAG_RUNING				0x01	//正在前进
#define RUNFLAG_STOP					0x02	//停止(暂停)前进
#define RUNFLAG_LOSTLINE			0x03	//丢线停止
#define TURN180_CONTINUE			0x01	//转体180°后继续前进
#define TURN180_STOP					0x02	//转体180°后停止
#define TURN180_132NO					0x00	//132地标 未经过
#define TURN180_132END				0x01	//132地标 已经过,已转体
#define CtrAreaOut						0x00	//在管制区内
#define CtrAreaIn							0x01	//不在管制区内
#define RouteValid						((unsigned char)0x00)	//路径转弯点有效
#define RouteInValid					((unsigned char)0x01)	//路径转弯点无效
// 全局变量定义***************************************************************************************************************************

// 控制标志位定义=======================================
unsigned char RunFlag							=RUNFLAG_STOP;	//运行标志
unsigned char RunMod							=STOP;					//小车的运行模式,NORMAL\STOP\TURN_90
unsigned char RunStatus							=END;						//运动模式的执行状态,GOING\END
unsigned char SendStatus						=0;							//送餐状态,SENDING\SENDBACK\NULL
unsigned char Turn180Mod						=0;							//原地转体180°模式，TURN180_CONTINUE(转体后继续前进);TURN180_STOP(转体后停止)
unsigned char DIRBuf							=0;
unsigned char RunModBuf							=0;
unsigned int  TurnTimeCount						=0;							//转体时间计数值(通过TIM3进行计数),单位5ms
unsigned char TurnTimeCountFlag					=0;							//转体时间开始计数标志,1:进行计数、0:不计数
unsigned char BackTimeCount						=0;							//送餐返回时间计数值(通过TIM3进行计数),单位100ms
unsigned char Turn180TimeCount					=0;							//180°转体地标有效计数
unsigned int  RouteSelTime						=0;							//路径选择时间计时
unsigned char StopIDStatusFlag					=STOPIDUNVALID;	//停止、取餐点地标有效标志位,防止刚启动送餐时读到取餐点地标而停止
unsigned char ReceiveOrder						=0;							//调试用全局变量,用于保存无线串口(UART5)接收到的命令字
unsigned char Turn180_132_Flag					=TURN180_132NO;	//132地标转体状态,TURN180_132NO:未经过;TURN180_132END:已经过,已转体
unsigned char CtrAreaFlag						=CtrAreaOut;		//是否在管制区标志,初始化不在管制区
unsigned char RouteLFlag						=RouteValid;		//路径左转弯点,默认有效
unsigned char RouteRFlag						=RouteValid;		//路径右转弯点,默认有效

// 控制变量定义=======================================
signed		int SpeedMax						=PWMDUTY_MAX;/*速度闭环调速最大速度*/
signed 		int SpeedSet_HV						=0;//200
signed 		int SpeedSet_LV						=100;
signed 		int SpeedSet_BLV					=60;
signed 		int SpeedSearchLine					=20; /*寻线速度0-1000*/
signed 		int SpeedTurn						=20; /*转体速度0-1000*/
signed 		int SpeedRouteSel					=100;	/*路径选择速度*/
signed  	int SpeedIncA 						=2;  /*加速加速度,每40ms增加的速度*/
signed  	int SpeedDecA 						=4;  /*减速加速度,每40ms减少的速度*/
signed 		int SpeedTarget						=0;	 //电机运行的目标速度
signed 		int SpeedNormal						=0;  //正常前进速度,SpeedSet_HV、SpeedSet_LV、0
signed 		int SpeedReal 						=0;	 //实际速度(左右电机施加速度的平均值)
signed 		int SpeedRealL						=0;	 //左电机实际速度
signed 		int SpeedRealR						=0;	 //右电机实际速度

// float 				Kp=12.0;						//比例系数//10-16
// float 				Ki=0.080;						//积分系数//0.080
float 				Kp=0.0;							//比例系数//10-16
float 				Ki=0.001;						//积分系数//0.080
float 				Kd=0;								//比例系数//10-16
signed	 char EMAX=15;						//偏差最大值
signed	 char EMIN=-15;						//偏差最大值
signed	 int	MAXSUME=1500;				//偏差绝对限幅值
float 				Uk0=0;							//当前伺服周期的速度PI控制输出量
float 				Uk1=0;							//前一伺服周期的速度PI控制输出量
signed	 char	E0=0;								//当前时刻的偏差
signed	 char	E1=0;								//前一时刻的偏差
signed	 int	SumE=0;							//偏差积分(累加)
#if SPEEDDOWNEN
float 				Q_e=0;							//弯道降速的偏差系数
#endif
#if INCOMPLETEPIDEN
float					Alpha0=0.92;					//不完全微分系数 alpha0+Alpha1=1
float					Alpha1=0.08;					//不完全微分系数 alpha0+Alpha1=1
#endif

const unsigned int TURN90_180_TIME	=2400;//2400*5=12000ms=12s
unsigned int turn90_180_timing=0;
unsigned char turn_90_180_status=NULL;
#define turn_90_180_status_NULL				NULL
#define turn_90_180_status_lostline		(1)

#define VOICEPLAYEND 				0				//播放结束
#define VOICEPLAYING 				1				//正在播放
#define VOICETIMEDELAY				1				//播放间隔时间,单位：秒
#define TracklessNULL				0
#define Tracklessing				1
#define Tracklessend				2
#define TracklessTurn180			3
#define TracklessBack				4
#define TracklessbackNULL			0
#define Tracklessbacking			1
#define Tracklessbackend			2

#define TracklessDistQ			330//无轨距离转换系数, 设定值(0-9米) * TracklessDistQ = 程序估算距离(米)
unsigned char  BarrierEn=ENABLE;															//避障语音使能位
unsigned char  VoicePlayMod=VOICEMOD_NULL;										//语音播放模式
unsigned char  VoicePlayNumList[10]={0,0,0,0,0,0,0,0,0,0};		//语音播放列表
unsigned short VoiceTimeCount=0;															//当前语音倒计时长
unsigned char  VoicePlayStatus=VOICEPLAYEND;									//当前语音播放状态，VOICEPLAYEND:播放结束；VOICEPLAYING:正在播放
unsigned char  ID_Last=0;																			//保存上次的读到的ID卡号,防止重复读取同一ID
unsigned char  RobotMod=ROBOTMOD_TRACK;												//机器人模式(0:有轨,1:无轨)
unsigned long  TracklessDistance;//L*280mm
unsigned long  TracklessbackDistance=70000;
unsigned long  TracklessOffset=0;															//若后退则将后退距离设定为前进距离的偏置量
unsigned char	 TracklessCountFlag=TracklessNULL;//TracklessNULL、Tracklessing、Tracklessend、TracklessTurn180、TracklessBack
unsigned long	 TracklessCount=0;
unsigned char	 TracklessbackCountFlag=TracklessbackNULL;
unsigned long	 TracklessbackCount=0;
unsigned char  Turn180Trackless_DirBuf=MID;//LEFT、RIGHT、MID_BACK、MID
unsigned long  TracklessTurnCount=0;
unsigned long  TracklessTurnCountFlag=0;
 
unsigned char Turn_132_FLag;
// 函数声明************************************************************************************************
void IDNum_Operate(void);
void AT24CXX_Test( void );
void Send_Test( void );
unsigned char  GetVoicePlayMsg(void);
void VoiceDeal(void);
void VoiceStop(void);
void ReStartTaskDeal( void );
unsigned char SetCtrAreaStatus( unsigned char ctrareanum, unsigned char ctrareastatus );
unsigned char GetCtrAreaStatus( unsigned char ctrareanum );
void CtrAreaDeal( unsigned char ctrareaid );
void TouchServe( void );
void BarrierServe( void );
void BarrierServe_Trackless( void );
void BarrierDetect( void );
void PauseKeyDetect( void );
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_En( void );
void WD_Init( void );
void TIM4_PWM_Init(u16 arr,u16 psc);
void SpeedSet(unsigned char speedmod, signed int *speed);
void MotorDriverPort_Init( void );
void GuidSensorPort_Init( void );		//磁导向传感器端口初始化
signed char GetPos(void);						//读取诱导传感器的值,并获取当前的位置信息
void BarrierDirDetect( void );			//避障时偏离方向检测
unsigned char LineDetect_Centre( void );		//转体寻线
unsigned char LineDetect_All( void );				//转体寻线
void RobotRunTypeTable( unsigned char tabledir,unsigned char ID );
void RobotRunType180Trackless( unsigned char dir );
void RobotRunTypeTurn180( unsigned char dir, unsigned char turn180mod );
void RobotRunTypeCross( unsigned char dir );
void RobotRunTypeStop( void );
void RobotRunTypeBreak( void );
void RobotRunTypeStart( signed int runspeed );
void MotorDriverCtrPID( void );
void MotorDriverCtrTracklessLine( void );
void MotorDriverBackTracklessLine( void );
void MotorDriverStop( void );
void MotorDriverStop_Trackless( void );
void MotorDriverTurn_90Angle( unsigned char dir, unsigned char sendstatus );
void MotorDriverTurn_180Angle_Trackless( unsigned char dir );
void MotorDriverTurn_180Angle( unsigned char dir, unsigned char turn180mod );
void MotorDriverRouteSel( void );
void MotorDriverSpeedIncDecA( void );
void MotorDriverVClControl(signed int targetvleft,signed int targetvright);
void Init_Station_mode();
//================================================================================================================================
//			main函数
//================================================================================================================================
int main(void)
{
	unsigned char index=0;
	#if SPEEDDOWNEN
	Q_e=(SpeedSet_HV-SpeedSet_LV)/15;//初始化降速系数
	#endif
	Stm32_Clock_Init(9); 				//72M，使能8M外部高速晶振HSE，9倍频后作为系统时钟
	JTAG_Set(SWD_ENABLE);				//使能SWD, 释放PA15、PB3 PB4为普通IO口
	delay_init(72); 						//延时初始化
	WD_Init( );									//初始化外部看门狗引脚
	ST = 0;delay_ms(10);
	ST = 1;delay_ms(10);
	ST = 0;delay_ms(10);
	ST = 1;delay_ms(10);
	TIM3_Int_Init(49,7199);					//10Khz的计数频率,计数50次为5ms
	LED_Init();								//LED端口初始化
	GuidSensorPort_Init();					//诱导传感器初始化
	Barrier_Init();							//避障传感器IO端口初始化
	BackButton_Init();						//返回按钮(继电器常开)端口初始化
	Motion_Init( );							//电机驱动器初始化
	RouteInfInit(0);						//FLASH所有的数据区初始化清零
	AT24CXX_Init( );						//初始化IIC
#if IWDEN
	IWDG_Init(3,625);						//独立看门狗初始化,分频数prer=3,重装值rlr=625,溢出时间Tout=((4*2^prer)*rlr)/40 (ms)=500 (ms)
											//在定时器3中,每隔250ms喂狗一次IWDG_Feed(),以防系统复位
#endif
	uart3_init(36,19200);
	HMI_Init();									//HMI接口初始化
	Wireless_Init();						//无线模块(串口5)初始化,19200,8+n+1,
	ID_Init();									//ID卡初始化
	SpeechCtr_Init();						//语音控制器初始化
	TIM2_Int_Init(49,7199);					//10Khz的计数频率,计数50次为5ms
	TIM5_Int_Init(49,7199);					//10Khz的计数频率,计数50次为5ms
															//同时用于维根码和串口5接收一帧完整数据的超时检测,超出4ms则认为一帧数据接收完毕
	TIM6_Int_Init(49,7199);					//10Khz的计数频率,计数50次为5ms
	delay_ms(10);
#if IWDEN
	IWDT_En( );									//独立看门狗使能
#endif
	USART1_En( );								//USART1 使能
	USART4_En( );								//USART4 使能
	USART5_En( );								//USART5 使能
	ReStartTaskDeal( );					//重启后任务处理,若掉电前有任务,则继续任务

	while(1)
	{
		/*	==============================	*/
		/*			接收到控制命令			*/
		/*	==============================	*/
		if( CommandReceiveStatus==OK )
		{
			switch( RouteMsg1.command )
			{
				case Com_Send://送餐指令
					
					/* 获取语音的播放序列、模式 */
					//GetVoicePlayMsg();
					Init_Station_mode();
					ID_Last=0;
					Turn_132_FLag=0;
					/*	在无任务的状态下接受任务	*/
					if( Task.taskstatus == TaskNo )
					{
						delay_ms(1000);												//等待
						delay_ms(1000);
						/*	初始化相关寄存器变量,及标志位	*/
						WriteTaskStatus( TaskSending );								//设置任务进度
						/* 速度设置	*/
						SpeedSet(RouteMsg_SpeedModGet(), &SpeedSet_HV);
						
						switch(RouteMsg1.Station_DIR)
						{
							case 0:break;
							case 1: Turn90Dir = LEFT;
									STOP_L	=	STOP_R	= 0;					//使能驱动器
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//等待执行完毕,具体执行在定时器3中
									STOP_L	=	STOP_R	= 1;					//驱动器禁止			
								break;
							case 2: Turn90Dir = RIGHT;
									STOP_L	=	STOP_R	= 0;					//使能驱动器
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//等待执行完毕,具体执行在定时器3中
									STOP_L	=	STOP_R	= 1;					//驱动器禁止		
								break;
							case 3:
									BarrierEn=DISABLE;
									Turn180_132_Flag = TURN180_132END;
									RobotRunTypeTurn180( LEFT, TURN180_CONTINUE );
									turn_90_180_status=turn_90_180_status_NULL;
									BarrierEn=ENABLE;									
								break;
						}
						
						
						
						
						RobotRunTypeStart( SpeedSet_HV );
						ID =0;														//清ID号,防止发送送餐命令之前推动机器人正好读到相应地标导致发送命令即送餐到达
						RunFlag = RUNFLAG_RUNING;									//设置前进状态标志
						StopIDStatusFlag = STOPIDTIMING;							//设置停止标志点有效计时标志,16s计时结束后停止点有效,防止刚启动就停止
						Turn180_132_Flag = TURN180_132NO;							//180°转体132号地标未经过
						RouteLFlag	= RouteValid;									//路径左转弯点有效
						RouteRFlag	= RouteValid;									//路径右转弯点有效
					}
					BarrierFlag_last=NoBarrier;

					
					break;
				case Com_Stop://停止指令
					/*	设置停止状态 并停止前进	*/
					if( RunFlag == RUNFLAG_RUNING )
					{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
						StopIDStatusFlag = STOPIDVALID;
						if( RunModBuf == ROUTESEL ){RunMod = RunModBuf; RunModBuf = NULL;}  //校验停止前状态是否为 ROUTESEL,确保ROUTESEL继续
						WriteTaskStatus( TaskNo );											//设置任务状态为TaskSendEnd
						RouteMsg.route_l=RouteMsg.route_r=NULL;								//清空路径参数
						

					break;
				default:
					break;
			}
			Task.command = Com_NULL;
			CommandReceiveStatus = NULL;
		}
		/*	==============================	*/
		/*				读到卡号				*/
		/*	==============================	*/
		if( (IDNumReceiveStatus==OK) && (Task.taskstatus!=TaskNo) )
		{
			if(RunFlag == RUNFLAG_RUNING)
			{
				/* 遍历ID卡号,获取该卡的类型(交叉点、桌号、起始弯道、结束弯道、任务结束标志、后厨去餐点) */
				if( ID > 0 && ID <= 20 && Turn_132_FLag==0 )//餐桌号
				{
						/* 读到一个新的餐桌ID时,执行  防止重复读取 重复操作 */
						if(ID!=ID_Last)
						{
							switch(RouteMsg1.Station_mode[ID-1])
							{
								case 0:RobotRunTypeTable(MID,ID);break;
								case 1:RobotRunTypeTable(LEFT,ID);break;
								case 2:RobotRunTypeTable(RIGHT,ID);break;
								default:RobotRunTypeTable(MID,ID);
								
							}										
						}
				}else if( ID == 132 )//180°转体点
				{
						Turn_132_FLag=1;
						if( Turn180_132_Flag == TURN180_132NO )//【读到该点一段时间后恢复有效状态】
						{
							BarrierEn=DISABLE;
							Turn180_132_Flag = TURN180_132END;
							RobotRunTypeTurn180( LEFT, TURN180_CONTINUE );
							turn_90_180_status=turn_90_180_status_NULL;
							BarrierEn=ENABLE;	
						}
				}
				else if(ID==131 && ID_Last!=0 && ID!=ID_Last)
				{
					/*	设置停止状态 并停止前进	*/
					if( RunFlag == RUNFLAG_RUNING )
					{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
					
						switch(RouteMsg1.Station_DIR)
						{
							case 0:break;
							case 1: SendStatus=SENDING;	
									Turn90Dir = RIGHT;
									STOP_L	=	STOP_R	= 0;					//使能驱动器
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//等待执行完毕,具体执行在定时器3中
									STOP_L	=	STOP_R	= 1;					//驱动器禁止			
								break;
							case 2: SendStatus=SENDING;	
									Turn90Dir = LEFT;
									STOP_L	=	STOP_R	= 0;					//使能驱动器
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//等待执行完毕,具体执行在定时器3中
									STOP_L	=	STOP_R	= 1;					//驱动器禁止		
								break;
							case 3:
									BarrierEn=DISABLE;
									Turn180_132_Flag = TURN180_132END;
									RobotRunTypeTurn180( LEFT, TURN180_STOP );
									turn_90_180_status=turn_90_180_status_NULL;
									BarrierEn=ENABLE;									
								break;
						}
					
					
						StopIDStatusFlag = STOPIDVALID;
						if( RunModBuf == ROUTESEL ){RunMod = RunModBuf; RunModBuf = NULL;}//校验停止前状态是否为 ROUTESEL,确保ROUTESEL继续
						WriteTaskStatus( TaskNo );					//设置任务状态为TaskSendEnd
						RouteMsg.route_l=RouteMsg.route_r=NULL;	//清空路径参数
				}
				else if(ID>100&&ID<=110)											//分叉点
				{
					
					if(RouteMsg1.Cross[ID-101]==0)									//左转
					{
						RobotRunTypeCross(LEFT);
					}
					if(RouteMsg1.Cross[ID-101]==1)
					{
						RobotRunTypeCross(RIGHT);									//右转
					}
				}
				
				ID_Last =ID;
			}
			/* 清标志位 */
			ID = 0;
			IDNumReceiveStatus=NULL;//清空标志位
		}
		
		if(STATMOD_FLAG==1)
		{
			for(index=0;index<20;index++)
			{
				Station_modeSet(index,Recive_data[6+index]);
			}
			USART1_Send(Recive_data,Recive_num);
			STATMOD_FLAG=0;
		}
		if(STATIM_FLAG==1)
		{
			for(index=0;index<20;index++)
			{
				Station_timSet(index,Recive_data[6+index]);
			}
			USART1_Send(Recive_data,Recive_num);
			STATIM_FLAG=0;
		}
		if(CROSS_FLAG==1)
		{
			for(index=0;index<10;index++)
			{
				Cross_Set(index,Recive_data[6+index]);
			}
			USART1_Send(Recive_data,Recive_num);
			CROSS_FLAG=0;
		}
				
	}
}
/*********************************************
	函数名称：SpeedSet
	功		能：速度设置
	入		参：无
	返		值：无
*********************************************/
void SpeedSet(unsigned char speedmod, signed int *speed)
{
	switch(speedmod)
	{/* ********************
		* version : GGM V2.5
		* 170 = 0.60m/s;
		* 140 = 0.50m/s;
		* 125 = 0.40m/s;
		* 
		* version : GGM V1.0
		* 185 = 0.60m/s;
		* 170 = 0.53m/s;
		* 160 = 0.50m/s;
		* 150 = 0.46m/s;
		* 135 = 0.39m/s;
		* *******************/
		case SPEEDMOD_H:
			*speed=165;
			break;
		case SPEEDMOD_M:
			*speed=140;
			break;
		case SPEEDMOD_L:
			*speed=115;
			break;
		default:
			*speed=140;
			break;
	}
}
/*********************************************
	函数名称：unsigned char GetVoicePlayMsg(void)
	功		能：获取语音的播放序列、模式
	入		参：无
	返		值：TRUE： 获取成功
						FLASE：获取失败
*********************************************/
unsigned char  GetVoicePlayMsg(void)
{
		unsigned char tempdata;
		unsigned char index=0;
		/* 读取1-10段语音的信息 */
		for(tempdata=1;tempdata<=10;tempdata++)
		{
				VoiceMsg[tempdata].time 	= VoiceTimeRead(tempdata);
				VoiceMsg[tempdata].mod  	= VoiceModRead(tempdata);
				VoiceMsg[tempdata].runmod = VoiceRunModRead(tempdata);
		}
		/* 读取机器人模式	*/
		RobotMod=RobotModTrackRead();
		
		if(ROBOTMOD_TRACKLESS==RobotMod)//无轨模式
		{
				/* 设置机器人无轨模式、无轨距离 */
				VoicePlayMod = VOICEMOD_TRACKLESSLINE;
				//TracklessDistance = (unsigned long)TracklessDistRead( )*TracklessDistQ*1000;//mm
		}else{//有轨模式
				/* 读取1-10段语音的信息 */
				tempdata=1;
				while((VoiceMsg[tempdata].mod==VOICEMOD_NULL)&&(tempdata<=10)) tempdata++;
				
				if(tempdata<=10){
						VoicePlayMod = VoiceMsg[tempdata].mod;
						switch(VoicePlayMod)
						{
								/* 单曲点播模式 播放列表中第一段语音 */
								case VOICEMOD_Single:
									VoicePlayNumList[0]=0;
									break;
								
								/* 多曲循环模式 播放列表中全部语音 */
								case VOICEMOD_MULLOOP:
									while(tempdata<=10)
									{
											if(VoiceMsg[tempdata].mod==VOICEMOD_MULLOOP) VoicePlayNumList[index++]=tempdata;
											tempdata++;
									}
									while(index<=10)
									{
											VoicePlayNumList[index++]=0;
									}
									break;
								
									/* 无轨直线行走模式  */
								case VOICEMOD_TRACKLESSLINE:
									
									break;
									
								default :
									break;						
						}		
						return TRUE;
				}else{
						VoicePlayMod=VOICEMOD_NULL;
						return FALSE;
				}
		}
}
void Init_Station_mode()
{
	unsigned char index;
	for(index=0;index<20;index++)
	{
		RouteMsg1.Station_mode[index]=Station_modeRead(index);
		RouteMsg1.Station_tim[index]=Station_timRead(index);
			
	}
	for(index=0;index<10;index++)
	{
		RouteMsg1.Cross[index]=Cross_Read(index);
	}
	RouteMsg1.Station_DIR=StationDirRead();

}
/*********************************************
	函数名称：void VoiceDeal(void)
	功		能：语音段四种播放模式处理
	入		参：无
	返		值：无
*********************************************/
void VoiceDeal(void)
{
		static unsigned char index=0;//播放列表索引
		switch(VoicePlayMod)
		{
				/* 停止播放语音 */
				case VOICEMOD_NULL:
					break;
				
				/* 单曲点播模式 播放列表中第一段语音 */
				case VOICEMOD_Single:
					break;
				
				/* 多曲循环模式 播放列表中全部语音 */
				case VOICEMOD_MULLOOP:
					if(VoicePlayStatus == VOICEPLAYEND)
					{
							if( (index!=0) && (VoiceMsg[VoicePlayNumList[index]].mod == VOICEMOD_NULL) )
							{
									index=0;
							}
							VoicePlay(VoicePlayNumList[index]+2);
							VoiceTimeCount = VoiceMsg[VoicePlayNumList[index]].time+VOICETIMEDELAY;
							VoicePlayStatus= VOICEPLAYING;
							index++;//指向列表中的下一个对象
							if(index>10) index=0;
					}
					break;
				
				/* 无轨直线行走模式  */
				case VOICEMOD_TRACKLESSLINE:
					if(VoicePlayStatus == VOICEPLAYEND)
					{
							VoicePlay(TracklessLineVoice);
							VoiceTimeCount = VoiceMsg[TracklessLineVoice-2].time+6;
							VoicePlayStatus= VOICEPLAYING;
					}
					break;
					
				default :
					break;						
		}
}
/*********************************************
	函数名称：void VoiceStop(void)
	功		能：停止播放语音
	入		参：无
	返		值：无
*********************************************/
void VoiceStop(void)
{
		VoicePlayMod	 = VOICEMOD_NULL;
		VoicePlayStatus= VOICEPLAYEND;
		VoiceTimeCount = 0;
		TracklessCountFlag=TracklessNULL;
		TracklessCount=0;
		TracklessbackCount=0;
		TracklessDistance=0;
}
/*********************************************
	函数名称：void ReStartTaskDeal( void )
	功		能：重启后任务处理,若掉电前有任务,则继续任务
	入		参：无
	返		值：无
*********************************************/
void ReStartTaskDeal( void )
{
	#ifdef	RESTARTCONTINUE
		/* 管制区处理			*/
		Task.ctrareanum=ReadCtrAreaNum( );
		if( (Task.ctrareanum>=CtrArea_1) && (Task.ctrareanum<=CtrArea_5) )
		{
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );
		}
		/* 桌号、任务处理	*/
		Task.tasktable=ReadTaskTable( );
		Task.taskstatus=ReadTaskStatus( );
		if( (Task.tasktable>0)&&(Task.tasktable<100) &&
			 ((Task.taskstatus==TaskSending)||(Task.taskstatus==TaskSendEnd)||(Task.taskstatus==TaskOver)) )
		{
			/*	获取路径信息	*/
			RouteMsg.robotid = RobotIDRead( );
			RouteMsg.tabledir= TableDIRRead( Task.tasktable );
			RouteMsg.route_l = RouteLeftRead( Task.tasktable ) - 100;
			RouteMsg.route_r = RouteRightRead( Task.tasktable );
			if(RouteMsg.tabledir == NULL) { RouteMsg.tabledir = MID; }
			// 启动运行,继续前进(并设置相关标志位)
			RobotRunTypeStart( SpeedSet_HV );
			RunFlag = RUNFLAG_RUNING;					//设置前进状态标志
			StopIDStatusFlag = STOPIDTIMING;	//设置停止标志点有效计时标志,2s计时结束后停止点有效,防止刚启动就停止
			Turn180_132_Flag = TURN180_132NO;	//180°转体132号地标未经过
		}else{
			WriteTaskTable( 0 );
			WriteTaskStatus( TaskNo );
		}
	#else
		/*清任务、路径暂存变量*/
		WriteContrCommand(Com_NULL);	//设置终端控制命令为空
		WriteTaskTable(0);						//设置目标桌号为空
		WriteTaskStatus(TaskNo);			//设置无任务状态
		WriteCtrAreaNum(0);						//设置管制区编号为零
		Task.ctrareastatus=CtrArea_Idle;//设置管制区状态空闲
	#endif
}
/*********************************************
	函数名称：void SetCtrAreaStatus( unsigned char ctrareanum, unsigned char status )
	功		能：向终端控制器设置管制区状态
	入		参：ctrareanum：管制区编号
						status		：状态
												CtrArea_Idle 管制区闲状态
												CtrArea_Busy 管制区忙状态
	返		值：0:等待超时
						1:设置成功
*********************************************/
unsigned char SetCtrAreaStatus( unsigned char ctrareanum, unsigned char ctrareastatus )
{
	unsigned short crc16=0;
	unsigned long	 count=0x00200000;//等待约500ms
	GetCtrAreaStatusFlag = 0;
	USART5_TX_BUF[0]=RouteMsg.robotid;//机器人地址
	USART5_TX_BUF[1]=Com_WCAS;				//功能码
	USART5_TX_BUF[2]=ctrareanum;			//管制区编号
	USART5_TX_BUF[3]=ctrareastatus;		//管制区状态
	crc16 = CRC16(USART5_TX_BUF,4);
	USART5_TX_BUF[4]=crc16;
	USART5_TX_BUF[5]=crc16>>8;
	USART5_Send(USART5_TX_BUF,6);			//发送给上位机
	Task.ctrareastatus = ctrareastatus;
	while(!GetCtrAreaStatusFlag)			//等待接收到控制终端返回的状态
	{ count--; if(count==0)return (0); }//等待超时
	GetCtrAreaStatusFlag = 0;
	return (1);
}
/****************************************************
	函数名称：void GetCtrAreaStatus( unsigned char ctrareanum )
	功		能：从终端控制器读取管制区状态
	入		参：ctrareanum：管制区编号
	返		值：0:等待超时
						1:成功获取状态
****************************************************/
unsigned char GetCtrAreaStatus( unsigned char ctrareanum )
{
	unsigned short crc16=0;
	unsigned long	 count=0x00200000;//等待约500ms
	GetCtrAreaStatusFlag = 0;
	USART5_TX_BUF[0]=RouteMsg.robotid;//机器人地址
	USART5_TX_BUF[1]=Com_RCAS;				//功能码 读管制区状态
	USART5_TX_BUF[2]=ctrareanum;			//管制区编号
	crc16 = CRC16(USART5_TX_BUF,3);		
	USART5_TX_BUF[3]=crc16;						//
	USART5_TX_BUF[4]=crc16>>8;				//
	USART5_Send(USART5_TX_BUF,5);			//发送给上位机
	while(!GetCtrAreaStatusFlag)			//等待接收到控制终端返回的状态
	{ count--; if(count==0)return (0); }//等待超时
	GetCtrAreaStatusFlag = 0;
	return (1);
}
/****************************************************
	函数名称：void CtrAreaDeal( unsigned char ctrareaid )
	功		能：进入管制区状态校验
								---进---																		---出---
						设置管制区编号、读状态--->Idle--->设置忙--->清忙状态、清编号
															 |
															 ------>Busy--->等待空闲--->设置忙--->清忙状态、清编号
	入		参：ctrareaid：管制区卡号
	返		值：无
****************************************************/
void CtrAreaDeal( unsigned char ctrareaid )
{
	unsigned char status=0;
	switch( ctrareaid )
	{
		case 120://进1号管制区
			status = 1;
			WriteCtrAreaNum( CtrArea_1 );											//设置管制区编号
		break;
		case 121://出1号管制区
			status = 2;
		break;
		case 122://进2号管制区
			status = 1;
			WriteCtrAreaNum( CtrArea_2 );											//设置管制区编号
		break;
		case 123://出2号管制区
			status = 2;
		break;
		case 124://进3号管制区
			status = 1;
			WriteCtrAreaNum( CtrArea_3 );											//设置管制区编号
		break;
		case 125://出3号管制区
			status = 2;
		break;
		case 126://进4号管制区
			status = 1;
			WriteCtrAreaNum( CtrArea_4 );											//设置管制区编号
		break;
		case 127://出4号管制区
			status = 2;
		break;
		case 128://进5号管制区
			status = 1;
			WriteCtrAreaNum( CtrArea_5 );											//设置管制区编号
		break;
		case 129://出5号管制区
			status = 2;
		break;
		default:
		break;
	}
	if( (status==1) && (CtrAreaFlag==CtrAreaOut) )				//进管制区，且不再管制区
	{
			CtrAreaFlag = CtrAreaIn;													//设置为已进入管制区
			
			if(!GetCtrAreaStatus( Task.ctrareanum ))					//获取管制区状态,等待超时,则再次读取一次
			{
				if(!GetCtrAreaStatus( Task.ctrareanum ))				//等待超时,则再次读取一次
				{	GetCtrAreaStatus( Task.ctrareanum );	}				
			}
			if( Task.ctrareastatus == CtrArea_Idle )					//如果空闲,则设置为忙状态
			{
				if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))	//等待超时,则再次设置一次
				{
					if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))//等待超时,则再次设置一次
						SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy );
				}
			}
			else{																							//如果忙,则停止并等待空闲
				/*	设置停止状态 并停止前进	*/
				if( RunFlag == RUNFLAG_RUNING )
				{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
				/*	等待空闲	*/
				while( Task.ctrareastatus == CtrArea_Busy )
				{ delay_ms(1000);delay_ms(1000);delay_ms(1000); GetCtrAreaStatus( Task.ctrareanum ); }
				/*	设置为忙状态	*/
				if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))	//等待超时,则再次设置一次
				{	
					if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))//等待超时,则再次设置一次
						SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy );
				}
				/*	启动运行	*/
				RobotRunTypeStart( SpeedSet_HV );
				RunFlag = RUNFLAG_RUNING;												//在避障函数中会启动运行
			}
	}else if( (status==2) && (CtrAreaFlag==CtrAreaIn) )	//出管制区，且之前在管制区内
// 	}else if( status==2 )	//出管制区
	{
			CtrAreaFlag = CtrAreaOut;													//设置为已出管制区
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );//清忙状态
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );//清忙状态
			WriteCtrAreaNum( CtrArea_NULL );									//清编号
	}
}
/*********************************************
	函数名称：void TouchServe( void )
	功		能：触摸按键服务程序
	入		参：无
	返		值：无
*********************************************/
void TouchServe( void )
{
		unsigned char touchstatus=0;
		touchstatus=TouchScan( );

		if(touchstatus & Touch_Head)
		{	VoicePlay(HeadVoice);								}
		else if(touchstatus & Touch_Shoulder)
		{	VoicePlay(LoinVoice);						}
// 		else if(touchstatus & Touch_Chest)
// 		{	VoicePlay(ChestVoice);							}
// 		else if(touchstatus & Touch_Loin)
// 		{	VoicePlay(LoinVoice);								}
}
/*********************************************
	函数名称：void BarrierDeal( void )
	功		能：避障
	入		参：无
	返		值：无
*********************************************/
void BarrierDeal( void )
{
			static unsigned char barriercount=0;
			static unsigned char barrrier_pre=BarrierSta_No;
			static unsigned char runmod_pre;
			
			if(RunFlag != RUNFLAG_RUNING) return;
		
			if(RouteMsg1.barrierstatus==BarrierSta_No)		//无障碍物,高速行进
			{
					if(barrrier_pre==BarrierSta_NotMan || barrrier_pre==BarrierSta_Man){
							if(runmod_pre==NORMAL)
								RobotRunTypeStart( SpeedSet_HV );
							else if(runmod_pre==STOP)
								RunMod=STOP;
					}else if(barrrier_pre==BarrierSta_DecSpeed){
							SpeedNormal= SpeedSet_HV;
					}
					barriercount=0;
					barrrier_pre=BarrierSta_No;
			}else if(RouteMsg1.barrierstatus==BarrierSta_DecSpeed)		//有远距离障碍物,低速行进
			{
					if(barrrier_pre==BarrierSta_NotMan || barrrier_pre==BarrierSta_Man){
							if(runmod_pre==NORMAL)
								RobotRunTypeStart( SpeedSet_LV );
							else if(runmod_pre==STOP)
								RunMod=STOP;
					}else if(barrrier_pre==BarrierSta_No){
							SpeedNormal= SpeedSet_LV ;
					}else{
							SpeedNormal= SpeedSet_LV ;
					}
					barriercount=0;
					barrrier_pre=BarrierSta_DecSpeed;
			}else if(RouteMsg1.barrierstatus==BarrierSta_NotMan || RouteMsg1.barrierstatus==BarrierSta_Man)//近距离障碍区
			{
					barriercount++;
					if(barriercount>=10){
							barriercount=10;
							
							if(RunMod)
							{
								runmod_pre = RunMod;
								RouteMsg_VoiceNumSet(BarrierVoice);
							}
							
							RobotRunTypeBreak( );					//紧急刹车【注意:Break后,SpeedNormal依然为最后行进时的速度值】
							barrrier_pre=RouteMsg1.barrierstatus;
				}
			}
}
/*********************************************
	函数名称：void BarrierServe( void )
	功		能：根据障碍物检测的结果置相应标志位
	入		参：无
	返		值：无
*********************************************/
void BarrierServe( void )
{
		BarrierScan( );

		if(BarrierStatus_Reg & NearBarrier_Foward)
		{	BarrierFlag = NearBarrier;					}
		else if(BarrierStatus_Reg & FarBarrier_Foward)
		{	BarrierFlag = FarBarrier;						}
		else
		{	BarrierFlag = NoBarrier;						}
}
/*********************************************
	函数名称：void BarrierServe_Trackless( void )
	功		能：根据障碍物检测的结果置相应标志位
	入		参：无
	返		值：无
*********************************************/
void BarrierServe_Trackless( void )
{
		BarrierScan( );

		switch(TracklessCountFlag)
		{
				case Tracklessing ://正常行走
					if(BarrierStatus_Reg & NearBarrier_Foward)
					{	BarrierFlag = NearBarrier;					}
					else if(BarrierStatus_Reg & FarBarrier_Foward)
					{	BarrierFlag = FarBarrier;						}
					else
					{	BarrierFlag = NoBarrier;						}
				break;

				case TracklessTurn180:
							switch(Turn180Trackless_DirBuf)
							{
								case LEFT:
									if(BarrierStatus_Reg & Barrier_Left)
									{	BarrierFlag = NearBarrier;					}
									else
									{	BarrierFlag = NoBarrier;						}
								break;
								
								case RIGHT:
									if(BarrierStatus_Reg & Barrier_Right)
									{	BarrierFlag = NearBarrier;					}
									else
									{	BarrierFlag = NoBarrier;						}
								break;
								
								case MID_BACK:
									if(BarrierStatus_Reg & NearBarrier_Backward)
									{	BarrierFlag = NearBarrier;					}
									else if(BarrierStatus_Reg & FarBarrier_Backward)
									{	BarrierFlag = FarBarrier;						}
									else
									{	BarrierFlag = NoBarrier;						}
								break;
								
								default:
									BarrierFlag = NoBarrier;
								break;
							}
				break;
				case TracklessBack:
					if(BarrierStatus_Reg & NearBarrier_Backward)
					{	BarrierFlag = NearBarrier;					}
					else if(BarrierStatus_Reg & FarBarrier_Backward)
					{	BarrierFlag = FarBarrier;						}
					else
					{	BarrierFlag = NoBarrier;						}
				break;
				default			:
					BarrierFlag = NoBarrier;
				break;
		}
}
/*********************************************
	函数名称：void BarrierDetect( void )
	功		能：避障检测,检查是否有障碍物,并执行相关避障操作
	入		参：无
	返		值：无
*********************************************/
void BarrierDetect( void )
{
			static unsigned char barriercount=0;
			
			if(BarrierFlag == NoBarrier)		//无障碍物,高速行进
			{
						barriercount=0;
						
						switch(BarrierFlag_last)
						{
								case NoBarrier:
									break;
								case FarBarrier:
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	SpeedNormal= SpeedSet_HV ;	}
										else													//经过任务结束点后,低速行驶
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case NearBarrier:
									VoicePlay(NULL);							//停止循环播放障碍语音
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	RobotRunTypeStart( SpeedSet_HV );	}
										else													//经过任务结束点后,低速行驶
										{	RobotRunTypeStart( SpeedSet_BLV );}
										
										if((TracklessCountFlag==TracklessTurn180)&&(RunStatus==GOING))
											RunMod = TURN_180_Trackless;
									}
									break;
									default:
										break;
						}
						BarrierFlag_last=NoBarrier;
			}else if(BarrierFlag == FarBarrier)		//有远距离障碍物,低速行进
			{
						barriercount=0;
						
						switch(BarrierFlag_last)
						{
								case NoBarrier:
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	SpeedNormal= SpeedSet_LV ;	}
										else													//经过任务结束点后,低速行驶
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case FarBarrier:
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	SpeedNormal= SpeedSet_LV ;	}
										else													//经过任务结束点后,低速行驶
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case NearBarrier:
									VoicePlay(NULL);							//停止循环播放障碍语音
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	RobotRunTypeStart( SpeedSet_LV );	}
										else													//经过任务结束点后,低速行驶
										{	RobotRunTypeStart( SpeedSet_BLV );}
									}
									break;
									default:
										break;
						}
						BarrierFlag_last=FarBarrier;
			}else
			{
						barriercount++;
						if(barriercount>=10){
							barriercount=10;
							
						switch(BarrierFlag_last)
						{
								case NoBarrier:
									if(RunFlag == RUNFLAG_RUNING)
										RobotRunTypeBreak( );					//紧急刹车【注意:Break后,SpeedNormal依然为最后行进时的速度值】
									break;
								case FarBarrier:
									if(RunFlag == RUNFLAG_RUNING)
										RobotRunTypeBreak( );					//紧急刹车【注意:Break后,SpeedNormal依然为最后行进时的速度值】
									break;
								case NearBarrier:
									if(RunFlag == RUNFLAG_RUNING)
										RobotRunTypeBreak( );					//紧急刹车【注意:Break后,SpeedNormal依然为最后行进时的速度值】
									break;
								default:
										break;
						}
//						/*	播放避障语音	*/
//						if(BarrierEn==ENABLE)
//						{
//#if BARRIERPLAYCONTEN
//								if(0==VoicePlay_Flag) VoicePlay(BarrierVoice);//播放障碍语音
//#else
//								if(BarrierFlag_last!=NearBarrier) VoicePlay(BarrierVoice);//播放障碍语音
//#endif
//						}
						BarrierFlag_last=NearBarrier;
					}
			}
}
void WelcomeVoiceDeal( void )
{
		unsigned char barrrier=RouteMsg1.barrierstatus;
		static unsigned char barrrier_pre;
		if(Task.taskstatus != TaskNo) return;
		if((barrrier==BarrierSta_NotMan || barrrier==BarrierSta_Man) && barrrier_pre!=BarrierSta_NotMan && barrrier_pre!=BarrierSta_Man)
			RouteMsg_VoiceNumSet(3);
		barrrier_pre = barrrier;
}
/*********************************************
	函数名称：void TIM3_IRQHandler(void)
	功		能：定时器3中断服务程序,5ms中断.
						每5ms读取一次车身位置,并通过PID控制算法调整车速进行矫正车身状态(4种状态)
														----沿轨迹行走----		 ----寻线----
														先加速后匀速	减速	先加速后匀速	减速
						SearchLineFlag	 		0		 				0				1		 				1
						LostLineFlag		 		0		 				1				1		 				0	
	入		参：无
	返		回：无
*********************************************/
void TIM3_IRQHandler(void)
{
	unsigned int  temp=0;
	static unsigned int timer3count=0;
	static unsigned short StopIDcount=0;
	if(TIM3->SR&0X0001)//溢出中断
	{
		timer3count++;
		TIM3->SR&=~(1<<0);//清除中断标志位
#if IWDEN
		IWDG_Feed();//喂狗
#endif
		ST = ~ST;		//复位外部看门狗
		//==============================================================
		/*	电机伺服周期 5ms	*/
		if(Task.taskstatus!=TaskNo)//5ms,且有任务
		{
			switch(RunMod)
			{
					case NORMAL ://正常寻线行走
						MotorDriverCtrPID();					//需要0.1ms
						break;
					case STOP   ://停止行走
						MotorDriverStop();						//停车
						break;
					case TURN_90://转体
						MotorDriverTurn_90Angle(Turn90Dir,SendStatus);//根据Turn90Dir和SendStatus进行转体操作
						break;
					case ROUTESEL://路径选择
						MotorDriverRouteSel();				//路径选择
						break;
					case TURN_180://原地转体180°
						MotorDriverTurn_180Angle(Turn180Dir,Turn180Mod);//根据Turn180Dir和Turn180Mod进行原地转体180°操作
						break;
					default			:
						break;
			}
		}

		/*	避障检测	*/
// 		if(Task.taskstatus!=TaskNo)
// 		{
//				BarrierServe();
//				
//				BarrierDetect();	//仅在有任务时执行避障操作
// 		}
		
		
		BarrierDeal();				//避障处理
		/*	避障状态下方向判断	*/
		BarrierDirDetect( );
		/*	转体时间计时	*/
		if(TurnTimeCountFlag)
		{
			TurnTimeCount++;
			if(TurnTimeCount>=2000)	TurnTimeCount=2000;
		}
		//==============================================================
		if(0==timer3count%8)//40ms
		{
			/*	电机速度 匀加减速控制	*/
			if((Task.taskstatus!=TaskNo)&&(Task.taskstatus!=NULL))
			{	MotorDriverSpeedIncDecA();	}
		}
		//==============================================================
		if(0==timer3count%10)//50ms
		{
			
		}
		//==============================================================
		if(0==timer3count%20)//100ms
		{
				/*	停止点有效16s计时	*/
				if( StopIDStatusFlag == STOPIDTIMING )
				{
					StopIDcount ++;
					if( StopIDcount >= 160 )//16s计时
					{	StopIDcount = 0; StopIDStatusFlag = STOPIDVALID;	}
				}
				
				/*	180°地标有效14s计时	*/
				if( Turn180_132_Flag == TURN180_132END )
				{
						Turn180TimeCount++;
						if( Turn180TimeCount>200 )
						{ Turn180TimeCount = 0; Turn180_132_Flag = TURN180_132NO; }
				}
				HumanBodyDetect(&RouteMsg1.barrierstatus);		//人体检测
				WelcomeVoiceDeal();								//欢迎光临

				
		}
		//==============================================================
		if(timer3count>=200)//1000ms
		{
			
			LED		=!LED;
			timer3count=0;
		}
	}
}
/*********************************************
	通用定时器3中断初始化
	这里时钟选择为APB1的2倍，而APB1为36MHz
	arr：自动重装值。
	psc：时钟预分频数
*********************************************/
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能    
 	TIM3->ARR=arr;  		//设定计数器自动重装值//刚好1ms    
	TIM3->PSC=psc;  		//预分频器7200,得到10Khz的计数时钟
	TIM3->DIER|=1<<0;   //允许更新中断
	TIM3->CR1|=0x01;    //使能定时器3
  MY_NVIC_Init(3,2,TIM3_IRQn,2);//抢占3,子优先级2,组2
}
/*********************************************
	使能通用定时器3
*********************************************/
void TIM3_En( void )
{
	TIM3->CR1|=0x01;    //使能定时器3
}
/*****************************
	函数名称：void WD_Init( void )
	功		能：外部看门狗初始化,ST引脚拉低
	入		参：无
	返		回：无
TD->GND:150ms
*****************************/
void WD_Init( void )
{
	RCC->APB2ENR|=1<<7;//先使能外设IO PORTF时钟
	GPIOF->CRH&=0XFFFFF0FF;	//PF10推挽输出
	GPIOF->CRH|=0X00000300;
	ST = 1;
}
/*****************************
	函数名称：void GuidSensorPort_Init( void )
	功		能：磁导向传感器端口初始化
	入		参：无
	返		回：无
*****************************/
void GuidSensorPort_Init( void )
{
	RCC->APB2ENR|=1<<3; 		//使能 PORTB 时钟
	RCC->APB2ENR|=1<<5; 		//使能 PORTD 时钟
	RCC->APB2ENR|=1<<8; 		//使能 PORTG 时钟
	
	GPIOB->CRH&=0X0FFFFFFF;	//PB 15 上拉输入
	GPIOB->CRH|=0X80000000;
	GPIOB->ODR|=1<<15;
	
	GPIOD->CRH&=0X00000000;	//PD8 15 上拉输入
	GPIOD->CRH|=0X88888888;
	GPIOD->ODR|=0XFF<<8;
	
	GPIOG->CRL&=0X000000FF;	//PG2 8 上拉输入
	GPIOG->CRH&=0XFFFFFFF0;
	GPIOG->CRL|=0X88888800;
	GPIOG->CRH|=0X00000008;
	GPIOG->ODR|=0X1FC;
}
/*********************************************
	函数名称：char GetPos(void)
	功		能：读取诱导传感器的值,并获取当前的位置信息
															左						 |						右
						 D16	D15	D14	D13	D12	D11	D10	D9 | D8	D7	D6	D5	D4	D3	D2	D1
						 16		15	14	13	12	11	10	9		 8	7		6		5		4		3		2		1
						函数主要用来确定以下参数的值,并用于表征当前位置状态
						pos				:当前的位置
						DirFlag		:方向标志,1(左偏),	0,			-1(右偏)
	入		参：无
	返		值：char pos:当前位置信息15~-15
*********************************************/
const unsigned char GuideSensorDir	=0;		//导向传感器安装位置标志. 0:正装(16左-1右); 1:反装(1左-16右)
signed 	 char DirFlag								=0;		//方向标志位,1:左偏;0:中间(不偏);-1:右偏
unsigned char DirCountL							=0;	 	//左偏计数
unsigned char DirCountM							=0;	 	//0 偏计数
unsigned char DirCountR							=0;	 	//右偏计数
unsigned char DirCountThreshold			=5;	 	//偏离计数阈值
unsigned char LostLineFlag					=0;  	//丢线标志,未检测到轨道线路
unsigned char LostLineCount					=0;	 	//丢线计数
unsigned char NoLostLineCount				=0;  	//非丢线计数
unsigned char LostLineCountThreshold=50;	//小车丢线和巡线计数阈值,计数值超过该值则为丢线或寻到线
#define posbufsize	30										//位置缓冲区容量
signed	 int  pos_sum								=0;		//位置求和
signed	 char pos_buf[posbufsize]={0,0,0,0,0,0,0,0,0,0,//位置缓冲区,前posbufsize个位置信息
																	 0,0,0,0,0,0,0,0,0,0,
																	 0,0,0,0,0,0,0,0,0,0};
signed char GetPos(void)
{
	unsigned	char	i=1;
	static signed	char pos=0;
	signed		char	tempsum=0;
	unsigned	char	tempcount=0;
	unsigned	char	D1=0,D2	=0,D3	=0,D4	=0,D5	=0,D6	=0,D7	=0,D8	=0,
									D9=0,D10=0,D11=0,D12=0,D13=0,D14=0,D15=0,D16=0;
	D1 =GUIDESENSOR_D1;	D2 =GUIDESENSOR_D2;	D3 =GUIDESENSOR_D3;	D4 =GUIDESENSOR_D4;
	D5 =GUIDESENSOR_D5;	D6 =GUIDESENSOR_D6;	D7 =GUIDESENSOR_D7;	D8 =GUIDESENSOR_D8;
	D9 =GUIDESENSOR_D9;	D10=GUIDESENSOR_D10;D11=GUIDESENSOR_D11;D12=GUIDESENSOR_D12;
	D13=GUIDESENSOR_D13;D14=GUIDESENSOR_D14;D15=GUIDESENSOR_D15;D16=GUIDESENSOR_D16;
	
	if(D1==1 && D2 ==1 && D3 ==1 && D4 ==1 && D5 ==1 && D6 ==1 && D7 ==1 && D8 ==1&&
		 D9==1 && D10==1 && D11==1 && D12==1 && D13==1 && D14==1 && D15==1 && D16==1	)//丢线,未检测到导轨
	{
			LostLineCount++;//丢线计数加1
			NoLostLineCount=0;
			if(LostLineCount>=LostLineCountThreshold)//丢线次数超过阈值LAndSLineCountThreshold确定为丢线,置丢线标志位
			{
				LostLineFlag=1;
				LostLineCount=LostLineCountThreshold;
			}
	}else//未丢线,检测到导轨
	{
		if( D5 ==0 || D6 ==0 || D7 ==0 || D8 ==0 || D9==0 || D10==0 || D11==0 || D12==0 )
		NoLostLineCount++;
		LostLineCount=0;
		if(NoLostLineCount>=LostLineCountThreshold)//非丢线线次数超过阈值LAndSLineCountThreshold确定为未丢线,清丢线标志位
		{
			LostLineFlag=0;
			NoLostLineCount=LostLineCountThreshold;
			if( RunMod == NORMAL )//正常寻线位置计算
			{
					/*	计算中心点偏移量pos	*/
					if( D1 	== 0 ){	tempsum+=1;		tempcount++;	}
					if( D2 	== 0 ){	tempsum+=2;		tempcount++;	}
					if( D3 	== 0 ){	tempsum+=3;		tempcount++;	}
					if( D4 	== 0 ){	tempsum+=4;		tempcount++;	}
					if( D5 	== 0 ){	tempsum+=5;		tempcount++;	}
					if( D6 	== 0 ){	tempsum+=6;		tempcount++;	}
					if( D7 	== 0 ){	tempsum+=7;		tempcount++;	}
					if( D8 	== 0 ){	tempsum+=8;		tempcount++;	}
					if( D9 	== 0 ){	tempsum+=9;		tempcount++;	}
					if( D10 == 0 ){	tempsum+=10;	tempcount++;	}
					if( D11 == 0 ){	tempsum+=11;	tempcount++;	}
					if( D12 == 0 ){	tempsum+=12;	tempcount++;	}
					if( D13 == 0 ){	tempsum+=13;	tempcount++;	}
					if( D14 == 0 ){	tempsum+=14;	tempcount++;	}
					if( D15 == 0 ){	tempsum+=15;	tempcount++;	}
					if( D16 == 0 ){	tempsum+=16;	tempcount++;	}
					if( tempcount <= 7 )
					{
						pos	=	( signed char )((tempsum*1.0/tempcount-8.5)*2);//计算传感器的中点偏移量, 中点值为8.5
						pos	=	( GuideSensorDir ? -pos : pos );
						/* 更新位置缓冲区	*/
						pos_sum=pos_sum-pos_buf[posbufsize-1]+pos;
						for( i=posbufsize-1; i>0; i-- )
						{
							pos_buf[i]=pos_buf[i-1];
						}
						pos_buf[0]=pos;
					}else
					{
						pos = pos_sum/posbufsize;
					}
					
					
// 					D[1] =D1;	D[2] =D2;	D[3] =D3;	D[4] =D4;
// 					D[5] =D5;	D[6] =D6;	D[7] =D7;	D[8] =D8;
// 					D[9] =D9;	D[10]=D10;D[11]=D11;D[12]=D12;
// 					D[13]=D13;D[14]=D14;D[15]=D15;D[16]=D16;
// 					
// 					while( D[i]==1 ) i++;
// 					if( i<=16 )			//存在第一个有效区域
// 					{
// 						tempcount=0;
// 						while( (D[i]==0) && (i<=16) ) { pos_1+=i;	tempcount++;	i++;}
// 						pos_1	=	( signed char )((pos_1*1.0/tempcount-8.5)*2);
// 						pos_1	=	( GuideSensorDir ? -pos_1 : pos_1 );
// 						if( i<=16 )
// 						{
// 								while( D[i]==1 ) i++;
// 								if( i<=16 )//存在第二个有效区域
// 								{
// 									tempcount=0;
// 									while( (D[i]==0) && (i<=16) ) { pos_2+=i;	tempcount++;	i++;}
// 									pos_2	=	( signed char )((pos_2*1.0/tempcount-8.5)*2);
// 									pos_2	=	( GuideSensorDir ? -pos_2 : pos_2 );
// 									pos = (fabs(pos-pos_1)-fabs(pos-pos_2)) > 0 ? pos_2 : pos_1;
// 								}else			//第二个有效区域不存在
// 								{
// 									pos = pos_1;
// 								}
// 						}
// 					}else						//第一个有效区域不存在,即丢线
// 					{
// 						pos = 0;
// 					}

			}else if( RunMod == ROUTESEL )//转弯点处位置计算
			{
					switch( RouteSelDir )
					{
						case LEFT:
							if( GuideSensorDir == 0 )
							{
								if( D1 	== 0 ){	pos = 1;	}
								if( D2 	== 0 ){	pos = 2;	}
								if( D3 	== 0 ){	pos = 3;	}
								if( D4 	== 0 ){	pos = 4;	}
								if( D5 	== 0 ){	pos = 5;	}
								if( D6 	== 0 ){	pos = 6;	}
								if( D7 	== 0 ){	pos = 7;	}
								if( D8 	== 0 ){	pos = 8;	}
								if( D9 	== 0 ){	pos = 9;	}
								if( D10 == 0 ){	pos = 10;	}
								if( D11 == 0 ){	pos = 11;	}
								if( D12 == 0 ){	pos = 12;	}
								if( D13 == 0 ){	pos = 13;	}
								if( D14 == 0 ){	pos = 14;	}
								if( D15 == 0 ){	pos = 15;	}
								if( D16 == 0 ){	pos = 16;	}
// 								pos = ( signed char )((pos-11)*2);//计算传感器的中点偏移量, 中点值为11
								pos = ( signed char )((pos-9.5)*2);//计算传感器的中点偏移量, 中点值为10
							}else
							{
								if( D16 == 0 ){	pos = 16;	}
								if( D15 == 0 ){	pos = 15;	}
								if( D14 == 0 ){	pos = 14;	}
								if( D13 == 0 ){	pos = 13;	}
								if( D12 == 0 ){	pos = 12;	}
								if( D11 == 0 ){	pos = 11;	}
								if( D10 == 0 ){	pos = 10;	}
								if( D9 	== 0 ){	pos = 9;	}
								if( D8 	== 0 ){	pos = 8;	}
								if( D7 	== 0 ){	pos = 7;	}
								if( D6 	== 0 ){	pos = 6;	}
								if( D5 	== 0 ){	pos = 5;	}
								if( D4 	== 0 ){	pos = 4;	}
								if( D3 	== 0 ){	pos = 3;	}
								if( D2 	== 0 ){	pos = 2;	}
								if( D1 	== 0 ){	pos = 1;	}
// 								pos = ( signed char )((6-pos)*2);//计算传感器的中点偏移量, 中点值为6
								pos = ( signed char )((7.5-pos)*2);//计算传感器的中点偏移量, 中点值为7
							}
							break;
						case RIGHT:
							if( GuideSensorDir == 0 )
							{
								if( D16 == 0 ){	pos = 16;	}
								if( D15 == 0 ){	pos = 15;	}
								if( D14 == 0 ){	pos = 14;	}
								if( D13 == 0 ){	pos = 13;	}
								if( D12 == 0 ){	pos = 12;	}
								if( D11 == 0 ){	pos = 11;	}
								if( D10 == 0 ){	pos = 10;	}
								if( D9 	== 0 ){	pos = 9;	}
								if( D8 	== 0 ){	pos = 8;	}
								if( D7 	== 0 ){	pos = 7;	}
								if( D6 	== 0 ){	pos = 6;	}
								if( D5 	== 0 ){	pos = 5;	}
								if( D4 	== 0 ){	pos = 4;	}
								if( D3 	== 0 ){	pos = 3;	}
								if( D2 	== 0 ){	pos = 2;	}
								if( D1 	== 0 ){	pos = 1;	}
// 								pos = ( signed char )((pos-6)*2);//计算传感器的中点偏移量, 中点值为6
								pos = ( signed char )((pos-7.5)*2);//计算传感器的中点偏移量, 中点值为7
							}else
							{
								if( D1 	== 0 ){	pos = 1;	}
								if( D2 	== 0 ){	pos = 2;	}
								if( D3 	== 0 ){	pos = 3;	}
								if( D4 	== 0 ){	pos = 4;	}
								if( D5 	== 0 ){	pos = 5;	}
								if( D6 	== 0 ){	pos = 6;	}
								if( D7 	== 0 ){	pos = 7;	}
								if( D8 	== 0 ){	pos = 8;	}
								if( D9 	== 0 ){	pos = 9;	}
								if( D10 == 0 ){	pos = 10;	}
								if( D11 == 0 ){	pos = 11;	}
								if( D12 == 0 ){	pos = 12;	}
								if( D13 == 0 ){	pos = 13;	}
								if( D14 == 0 ){	pos = 14;	}
								if( D15 == 0 ){	pos = 15;	}
								if( D16 == 0 ){	pos = 16;	}
// 								pos = ( signed char )((11-pos)*2);//计算传感器的中点偏移量, 中点值为11
								pos = ( signed char )((9.5-pos)*2);//计算传感器的中点偏移量, 中点值为10
							}
							break;
							
						case MID:
							if( D1 	== 0 ){	tempsum+=1;		tempcount++;	}
							if( D2 	== 0 ){	tempsum+=2;		tempcount++;	}
							if( D3 	== 0 ){	tempsum+=3;		tempcount++;	}
							if( D4 	== 0 ){	tempsum+=4;		tempcount++;	}
							if( D5 	== 0 ){	tempsum+=5;		tempcount++;	}
							if( D6 	== 0 ){	tempsum+=6;		tempcount++;	}
							if( D7 	== 0 ){	tempsum+=7;		tempcount++;	}
							if( D8 	== 0 ){	tempsum+=8;		tempcount++;	}
							if( D9 	== 0 ){	tempsum+=9;		tempcount++;	}
							if( D10 == 0 ){	tempsum+=10;	tempcount++;	}
							if( D11 == 0 ){	tempsum+=11;	tempcount++;	}
							if( D12 == 0 ){	tempsum+=12;	tempcount++;	}
							if( D13 == 0 ){	tempsum+=13;	tempcount++;	}
							if( D14 == 0 ){	tempsum+=14;	tempcount++;	}
							if( D15 == 0 ){	tempsum+=15;	tempcount++;	}
							if( D16 == 0 ){	tempsum+=16;	tempcount++;	}
							if( tempcount <= 7  )
							{
								pos	=	( signed char )((tempsum*1.0/tempcount-8.5)*2);//计算传感器的中点偏移量, 中点值为8.5
								pos	=	( GuideSensorDir ? -pos : pos );
							}else
							{
								pos = 0;
							}
							break;
					}
			}
			
			/*	偏离方向判断	*/
			if(pos==0)
			{
				DirCountM++;//0偏计数加1
				DirCountL=0;//清左、右偏计数
				DirCountR=0;
				if(DirCountM>=DirCountThreshold)
				{
					DirFlag=0;										//方向为中间
					DirCountM=DirCountThreshold;	//防溢出
				}
			}else{
				if(pos<0)//左偏
				{
					DirCountL++;//左偏计数加1
					DirCountM=0;////清0、右偏计数
					DirCountR=0;
					if(DirCountL>=DirCountThreshold)
					{	
						DirFlag=1;										//方向为左偏
						DirCountL=DirCountThreshold;	//防溢出
					}
				}else//右偏
				{
					DirCountR++;//右偏计数加1
					DirCountM=0;//清0、左偏计数
					DirCountL=0;
					
					if(DirCountR>=DirCountThreshold)
					{
						DirFlag=-1;										//方向为右偏
						DirCountR=DirCountThreshold;	//防溢出
					}
				}
			}
		}
	}
	return pos;
}
/*********************************************
	函数名称：void BarrierDirDetect( void )
	功		能：避障时偏离方向检测, 以防避障减速丢线时, 寻线方向错误
	入		参：无
	返		值：无
*********************************************/
void BarrierDirDetect( void )
{
	if( (GUIDESENSOR_D1 ==0)||(GUIDESENSOR_D2 ==0)||(GUIDESENSOR_D3 ==0) )
	{	DirFlag = (GuideSensorDir ?-1 : 1);	}				//方向为左偏
	if( (GUIDESENSOR_D14==0)||(GUIDESENSOR_D15==0)||(GUIDESENSOR_D16==0) )
	{	DirFlag = (GuideSensorDir ? 1 :-1); }				//方向为右偏
}
/*********************************************
	函数名称：unsigned char LineDetect_Centre( void )
	功		能：转体中心寻线,中心点7 8 9 10在线上即为有效
	入		参：无
	返		值：1：未检测到线；0：检测到线
*********************************************/
unsigned char LineDetect_Centre( void )
{
	if((GUIDESENSOR_D7==1)&&(GUIDESENSOR_D8==1)&&(GUIDESENSOR_D9==1)&&(GUIDESENSOR_D10==1))
	{	return ((unsigned char)1);	}/*	未检测到线	*/
	else
	{	return ((unsigned char)0);	}/*	  检测到线	*/
}
/*********************************************
	函数名称：unsigned char LineDetect_All( void )
	功		能：转体寻线,有在线上的点即为有效
	入		参：无
	返		值：1：未检测到线；0：检测到线
*********************************************/
unsigned char LineDetect_All( void )
{
	if((GUIDESENSOR_D1 ==1)&&(GUIDESENSOR_D2 ==1)&&(GUIDESENSOR_D3 ==1)&&(GUIDESENSOR_D4 ==1)&&
		 (GUIDESENSOR_D5 ==1)&&(GUIDESENSOR_D6 ==1)&&(GUIDESENSOR_D7 ==1)&&(GUIDESENSOR_D8 ==1)&&
		 (GUIDESENSOR_D9 ==1)&&(GUIDESENSOR_D10==1)&&(GUIDESENSOR_D11==1)&&(GUIDESENSOR_D12==1)&&
		 (GUIDESENSOR_D13==1)&&(GUIDESENSOR_D14==1)&&(GUIDESENSOR_D15==1)&&(GUIDESENSOR_D16==1)  )
	{	return ((unsigned char)1);	}/*	未检测到线	*/
	else
	{	return ((unsigned char)0);	}/*	  检测到线	*/
}
/*********************************************
	函数名称：unsigned char RobotRunTypeTable( unsigned char tabledir )
	功		能：读到桌号ID后执行送餐操作,动作顺序:
						STOP->TURN_90(原地转体)->等待返回命令->TURN_90(原地反向转体)->NORMAL
	入		参：无
	返		值：无
*********************************************/
void RobotRunTypeTable( unsigned char tabledir,unsigned char ID )
{
	BarrierEn=DISABLE;
	/*	停车	*/
	RunFlag = RUNFLAG_STOP;
	RobotRunTypeStop( );//停车
	delay_ms(500);
	
	/*	转体	*/
	if( tabledir==MID )							Turn90Dir = MID;			//设置餐桌方向
	else if( tabledir==LEFT )				Turn90Dir = LEFT;
	else if( tabledir==RIGHT )			Turn90Dir = RIGHT;
	else if( tabledir==MID_BACK )		Turn90Dir = MID;
	else if( tabledir==LEFT_BACK )	Turn90Dir = LEFT;
	else if( tabledir==RIGHT_BACK )	Turn90Dir = RIGHT;
	SendStatus=SENDING;								//设置转体状态位为SENDING
	if(Turn90Dir!=MID)								//需要转体
	{
		STOP_L	=	STOP_R	= 0;					//使能驱动器
		RunMod=TURN_90;
		RunStatus=GOING;
		while(RunStatus!=END);						//等待执行完毕,具体执行在定时器3中
		STOP_L	=	STOP_R	= 1;					//驱动器禁止
	}
	delay_ms(100);
	
	/*	播报语音			*/
	RouteMsg_VoiceNumSet(ID+12);
	
	/*	等待返回命令，等待播放完毕，这里可以改成等待指令	*/
	while(Voice_ok!=RouteMsg1.Station_tim[ID-1])
	{
		delay_ms(1000);
		Voice_ok++;
	}
	Voice_ok=0;

	delay_ms(10);			//等待10s
	/*	反向\同向转体(寻线)	*/
	if( tabledir == MID )							Turn90Dir = MID;				//根据餐桌方向设置转体方向为反向
	else if( tabledir == LEFT )				Turn90Dir = RIGHT;					
	else if( tabledir == RIGHT )			Turn90Dir = LEFT;
	else if( tabledir == LEFT_BACK )	Turn90Dir = LEFT;			//根据餐桌方向设置转体方向为同向
	else if( tabledir == RIGHT_BACK )	Turn90Dir = RIGHT;
	
	SendStatus=SENDBACK;							//设置转体状态位为SENDBACK
	STOP_L	=	STOP_R	= 0;						//使能驱动器
	turn_90_180_status=turn_90_180_status_NULL;
	if( tabledir == MID_BACK )				//转体180°后直接前进
	{	RobotRunTypeTurn180( LEFT, TURN180_CONTINUE );	}
	else if(Turn90Dir!=MID)						//转体90°前进
	{
		RunMod=TURN_90;
		RunStatus=GOING;
		while(RunStatus!=END);					//等待执行完毕,具体执行在定时器3中
	}
	SendStatus=NULL;									//设置转体状态位为NULL
	
	if(turn_90_180_status==turn_90_180_status_lostline)
		turn_90_180_status=turn_90_180_status_NULL;
	else{
		/*	正常行走(方向PID控制)	*/
		RobotRunTypeStart( SpeedSet_HV );
		RunFlag = RUNFLAG_RUNING;
	}
	BarrierEn=ENABLE;
}
/*********************************************
	函数名称：void RobotRunType180Trackless( unsigned char dir )
	功		能：无轨转体180°
	入		参：无
	返		值：无
*********************************************/
void RobotRunType180Trackless( unsigned char dir )
{
	/*	转体	*/
	Turn180Dir = dir;									//设置餐桌方向
	STOP_L	=	STOP_R	= 0;						//使能驱动器
	RunMod=TURN_180_Trackless;
	RunStatus=GOING;
	while(RunStatus!=END);						//等待执行完毕,具体执行在定时器3中
	STOP_L	=	STOP_R	= 1;						//驱动器禁止
}
/*********************************************
	函数名称：void RobotRunTypeTurn180( unsigned char dir, unsigned char turn180mod )
	功		能：原地转体180°
	入		参：dir：				转体方向(LEFT、RIGHT)
						turn180mod：转体模式
												TURN180_CONTINUE(转体后继续前进);
												TURN180_STOP(转体后停止)
	返		值：无
*********************************************/
void RobotRunTypeTurn180( unsigned char dir, unsigned char turn180mod )
{
	/*	停车	*/
	RunFlag = RUNFLAG_STOP;
	RobotRunTypeStop( );//停车
	delay_ms(500);
	
	/*	原地转体180°	*/
	Turn180Dir=dir;					//设置原地转体的方向
	Turn180Mod=turn180mod;	//设置原地转体的模式
	STOP_L	=	STOP_R	= 0;	//使能驱动器
	RunMod=TURN_180;
	RunStatus=GOING;
	while(RunStatus!=END);	//等待执行完毕,具体执行在定时器3中
	STOP_L	=	STOP_R	= 1;	//驱动器禁止
	
	/*	正常行走(方向PID控制)	*/
	if(( turn180mod == TURN180_CONTINUE )&&(turn_90_180_status==turn_90_180_status_NULL))
	{	RobotRunTypeStart( SpeedSet_HV ); RunFlag = RUNFLAG_RUNING;	}
}
/*********************************************
	函数名称：void RobotRunTypeCross( unsigned char dir )
	功		能：交叉点处路径选择
	入		参：无
	返		值：无
*********************************************/
void RobotRunTypeCross( unsigned char dir )
{
	/*	选择路径	*/
	RouteSelDir=dir;		 						//设置路径方向		<<调试设置,有待完善>>
	DIRBuf=RouteSelDir;
	RunMod=ROUTESEL;
	RunStatus=GOING;
}
/*********************************************
	函数名称：void RobotRunTypeStop( void )
	功		能：停车
	入		参：无
	返		值：无
*********************************************/
void RobotRunTypeStop( void )
{
	/*	减速停车	*/
	RunModBuf = RunMod;		//记录停止前的模式,在启动时校验是否为 ROUTESE 模式.
	RunMod=STOP;
	RunStatus=GOING;
	while(RunStatus!=END);//等待执行完毕,具体执行在定时器3中
	STOP_L	=	STOP_R	= 1;//驱动器禁止
	STOP_L	=	STOP_R	= 1;//驱动器禁止
}
/*********************************************
	函数名称：void RobotRunTypeBreak( void )
	功		能：紧急刹车
	入		参：无
	返		值：无
*********************************************/
void RobotRunTypeBreak( void )
{
	/*	紧急刹车	*/
	RunMod=NULL;	//清空运动模式
// 	RunStatus=END;//实际速度减速至0,停车结束
	SpeedTarget	= SpeedReal = 0;
	MotorDriverVClControl(SpeedReal,SpeedReal);
	STOP_L	=	STOP_R	= 1;//驱动器禁止
	STOP_L	=	STOP_R	= 1;//驱动器禁止
}
/*********************************************
	函数名称：void RobotRunTypeStart( signed int runspeed )
	功		能：使能驱动器, 启动运行, 开始寻线前进
	入		参：runspeed：启动速度，SpeedSet_HV、SpeedSet_LV
	返		值：无
*********************************************/
void RobotRunTypeStart( signed int runspeed )
{
	SpeedNormal = runspeed;		//设定速度缓冲区SpeedSet_HV、SpeedSet_LV
	STOP_L	=	STOP_R	= 0;		//使能驱动器
	STOP_L	=	STOP_R	= 0;		//使能驱动器

	RunMod=NORMAL;						//启动
}
/*********************************************
	函数名称：void MotorDriverCtrPID( void )(Timer3中每20ms调用一次)
	功		能：实现正常模式下的运动控制(方向PI控制)
						===					通过设置RunMod=TURN_90执行该操作			===
						===					程序执行在定时器3的中断中完成					===
						===					程序执行完毕后,设置RunMod=NULL				===
	入		参：无
	返		值：无
*********************************************/
unsigned char SearchLineFlag				=0;	 		//寻线标志,正在进行寻线操作
const unsigned  int SearchLineTime	=1000;		//寻线时间,超出时间则停止寻线
float k=0.7,b=4.0;//比例与误差之间线性化系数
void MotorDriverCtrPID( void )
{
	static unsigned  int SearchLineCount=0;
	/*	获取车身状态	*/
	E1=E0;
	E0=GetPos();
// 	USART5_SendByte((unsigned char)fabs(E0));
	if( RunMod == ROUTESEL )
		Kp = 0.8*fabs(E0)+5;
	else
		Kp = k*fabs(E0)+b;
	if(Kp>10)Kp=10;
// 	/*变速积分	【效果不明显】*/
// 	if(E0>EMAX*0.4)
// 		SumE+=(E0*(Ki/0.6*(1-E0/EMAX)));
// 	else
// 		if(E0<EMIN*0.4)
// 			SumE+=(E0*(Ki/0.6*(1-E0/EMIN)));
// 		else
			SumE+=E0;
		
	/*	偏差积分限幅	*/
	if(SumE>=MAXSUME)
		SumE=MAXSUME;
	if(SumE<=-MAXSUME)
		SumE=-MAXSUME;
	
	/*	根据四种情况调整车速	*/
	if(LostLineFlag==0)//正常行驶状态,先加速至设定速度,在匀速寻线行走
	{
			if(SearchLineFlag==1) 
			{
				SearchLineFlag=0;
				BREAK_L	=	BREAK_R = 0;
				RunFlag = RUNFLAG_RUNING;
			}//使能驱动器、设置运行状态
			
			if( RunMod == NORMAL )	//正常寻线速度
			{
				#if	SPEEDDOWNEN				//弯道降速
					if(E0>1 || E0<-1)		//模糊化-1 0 1,避免在直道降速
					{
					SpeedTarget	= SpeedNormal-((unsigned char)fabs(E0)-1)*Q_e;
					if(SpeedTarget <SpeedSet_LV )
						SpeedTarget=SpeedSet_LV;
					}else
						SpeedTarget	= SpeedNormal;
				#else									//弯道不降速
					SpeedTarget	= SpeedNormal;
				#endif
			}
			else if( RunMod == ROUTESEL )			//路径选择(转弯)速度
			{	SpeedTarget	= SpeedRouteSel;	}
			else if( RunMod == STOP )					//减速停车
			{	SpeedTarget	= 0;				 			}
		
			/*	PI控制	*/
			Uk0=E0*Kp+SumE*Ki;			//速度PI控制
			#if INCOMPLETEPIDEN
			Uk0=Alpha0*Uk1+Alpha1*Uk0;
			Uk1=Uk0;
			#endif
			SpeedRealL=SpeedReal-Uk0;
			SpeedRealR=SpeedReal+Uk0;

			/*	速度限幅	*/
			if(SpeedRealL>SpeedReal)
			{
				SpeedRealL=SpeedReal;
			}
			if(SpeedRealR>SpeedReal)
			{
				SpeedRealR=SpeedReal;
			}
			if(SpeedRealR<0)
			{
				SpeedRealR=0;
			}
			if(SpeedRealL<0)
			{
				SpeedRealL=0;
			}
			MotorDriverVClControl(SpeedRealL,SpeedRealR);	//速度输出
	}else
	if((SearchLineFlag==0)&&(LostLineFlag==1))//丢线,减速至0,再进行寻线操作
	{
			SpeedTarget=0;

			SpeedRealL=SpeedReal;
			SpeedRealR=SpeedReal;
			if(DirFlag>=0)//左偏
					MotorDriverVClControl(SpeedRealL,0);//速度输出
			else					//右偏
					MotorDriverVClControl(0,SpeedRealR);//速度输出
			if(SpeedReal==0)//实际速度减速至0,置寻线标志位SearchLineFlag
			{
				SearchLineFlag		=	1;
				SearchLineCount 	= 0;							//清寻线时间
			}
	}else
	if((SearchLineFlag==1)&&(LostLineFlag==1))//寻线,先加速至寻线速度
	{
			SearchLineCount++;//寻线时间计时, 超出时间未寻到线, 则停止寻线
			if( SearchLineCount >= SearchLineTime)
			{
				SearchLineCount	= SearchLineTime;		//寻线时间限幅
				SpeedTarget			= 0;								//寻线速度设置为零
			}else{
				SpeedTarget=SpeedSearchLine;
			}

			if( (SpeedTarget==0) && (SpeedReal==0) )		//寻线超时,机器人刹车
			{
				MotorDriverVClControl(0,0);
				BREAK_L = BREAK_R = 1;										//驱动器刹车
				RunFlag = RUNFLAG_LOSTLINE;
			}else{																			//机器人寻线
				SpeedRealL=SpeedReal;
				SpeedRealR=SpeedReal;
				if(DirFlag>=0)//左偏
				{ MotorDriverVClControl(SpeedRealL,-SpeedRealR); }//BREAK_R = 1; }//速度输出、右驱动器刹车
				else					//右偏
				{	MotorDriverVClControl(-SpeedRealL,SpeedRealR); }//BREAK_L = 1; }//速度输出、左驱动器刹车
			}
	}
}
/*********************************************
	函数名称：void MotorDriverCtrTracklessLine( void )
	功		能：直线无轨模式下,速度输出
	入		参：无
	返		值：无
*********************************************/
void MotorDriverCtrTracklessLine( void )
{
		SpeedTarget	= SpeedNormal;
		MotorDriverVClControl(SpeedReal,SpeedReal);	//速度输出
}
/*********************************************
	函数名称：void MotorDriverBackTracklessLine( void )
	功		能：直线无轨模式下,后退速度输出
	入		参：无
	返		值：无
*********************************************/
void MotorDriverBackTracklessLine( void )
{
		SpeedTarget	= -SpeedNormal;
		MotorDriverVClControl(SpeedReal,SpeedReal);	//速度输出
}
/*********************************************
	函数名称：void MotorDriverStop( void )(Timer3中每20ms调用一次)
	功		能：无刷电机停止,减速至零,结束后置状态
						位为END,并清RunMod.
						===通过设置RunMod=STOP、RunStatus==GOING执行该操作===
						===					程序执行在定时器3的中断中完成					===
						===程序执行完毕后,设置RunMod=NULL、RunStatus=END	===
	入		参：无
	返		值：无
*********************************************/
void MotorDriverStop( void )
{
	MotorDriverCtrPID( );
	if(SpeedReal==0)
	{
		RunStatus=END;//实际速度减速至0,停车结束
		RunMod=NULL;	//清空运动模式
	}
}
/*********************************************
	函数名称：void MotorDriverStop_Trackless( void )(Timer3中每20ms调用一次)
	功		能：无刷电机停止,减速至零,结束后置状态
						位为END,并清RunMod.
						===通过设置RunMod=STOP、RunStatus==GOING执行该操作===
						===					程序执行在定时器3的中断中完成					===
						===程序执行完毕后,设置RunMod=NULL、RunStatus=END	===
	入		参：无
	返		值：无
*********************************************/
void MotorDriverStop_Trackless( void )
{
	SpeedTarget	= 0;
	MotorDriverVClControl(SpeedReal,SpeedReal);	//速度输出
	if(SpeedReal==0)
	{
		RunStatus=END;//实际速度减速至0,停车结束
		RunMod=NULL;	//清空运动模式
	}
}
/*********************************************
	函数名称：void MotorDriverTurn_90Angle( unsigned char dir, unsigned char sendstatus )(Timer3中每20ms调用一次)
	功		能：转体90°,结束后置状态位为END,并清RunMod.
						===设置RunMod=TURN_90、RunStatus=GOING执行该操作	===
						===					程序执行在定时器3的中断中完成					===
						===程序执行完毕后,设置RunMod=NULL、RunStatus=END	===
	入		参：dir					:转体方向(LEFT、RIGHT)
						sendstatus	:送餐状态
												 SENDING,转体送餐(正向转体90°),通过定时器计时转体90°.
												 SENDBACK,送餐返回(反向转体90°),通过寻线转体.
						注:1.根据dir和sendstatus确实转体方向以及是否播放送餐语音
									SENDING  LEFT  	:左转体,播放语音
									SENDING  RIGHT 	:右转体,播放语音
									SENDBACK LEFT  	:右转体,不播放语音
									SENDBACK RIGHT	:左转体,不播放语音
	返		值：无
*********************************************/
void MotorDriverTurn_90Angle( unsigned char dir, unsigned char sendstatus )
{
	static unsigned char turn90count=0;
	if(sendstatus==SENDING)//送餐状态,转体送餐,正向转体90°,播放送餐语音
	{
			TurnTimeCountFlag=1;		//开始进行转体操作计时(在定时器3的10ms中断中进行计数)
			if((TurnTimeCount<=560) && (dir!=MID))		//双轮转,设置为560*0.005=2.8s,速度SpeedTurn=30.
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//设置目标速度为转体速度SpeedTurn
					/*	双轮转体	*/
					if(dir==RIGHT)			//餐桌在右,右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
			}else{									//转体时间到,停机
					if(SpeedTarget!=0)
						SpeedTarget=0;		//设置目标速度为0
					/*	双轮转体	*/
					if(dir==RIGHT)			//餐桌在右,右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
					if(SpeedReal==0)
					{
						TurnTimeCountFlag=0;			//清转体计时标志
						RunStatus=END;						//实际速度减速至0,停车结束
						RunMod=NULL;							//清空运动模式
// 						VoicePlay(TakeMealVoice);	//设置标志位,播放取餐语音
					}
			}
	}else//送餐返回,反向\同向转体90°,不播放语音
	{
			if( !LineDetect_Centre( ) )	//检测到线
			{	turn90count++;if(turn90count>100)turn90count=100;	}//100*5ms=500ms
			else												//未检测到线
			{ 
				turn90count=0;
				
				turn90_180_timing++;
				if(turn90_180_timing>=TURN90_180_TIME)
					turn90_180_timing=TURN90_180_TIME;
			}
			
			if( ( turn90count == 0 ) && (dir!=MID))//丢线,未检测到导轨
			{
					if(turn90_180_timing>=TURN90_180_TIME)
					{
						SpeedTarget	= 0;
						if(0==SpeedReal) turn_90_180_status=turn_90_180_status_lostline;
					}else	if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//设置目标速度为转体速度SpeedTurn
					/*	双轮转体	*/
					if(dir==RIGHT)			//餐桌在右,左转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,右转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
// 					else if(dir==MID)
// 					{
// 						if(DirFlag>=0)	//左偏
// 						{ MotorDriverVClControl(SpeedReal,0); BREAK_R = 1; }//速度输出、右驱动器刹车
// 						else						//右偏
// 						{	MotorDriverVClControl(0,SpeedReal); BREAK_L	=	1; }//速度输出、左驱动器刹车
// 					}
			}else if( turn90count >= 10 ){									//寻到线,停机
					// 反向转体模式二：寻到线后直接切换运行模式,继续前进.看起来比较柔顺-------------------------------------
					/*	将反转车轮停转,防止下一时刻电机输出时使电机方向制动	*/
					if(dir==RIGHT)			//餐桌在右,左转体
						MotorDriverVClControl(0,SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,右转体
						MotorDriverVClControl(SpeedReal,0);//速度输出
					RunStatus=END;			//实际速度减速至0,停车结束
					RunMod=NULL;				//清空运动模式
					TurnTimeCountFlag=0;//清转体计时标志
					TurnTimeCount=0;		//清转体计数值
					turn90count=0;			//清计数状态
					turn90_180_timing=0;
			}
			
			if(turn_90_180_status==turn_90_180_status_lostline)
			{
					RunStatus=END;			//实际速度减速至0,停车结束
					RunMod=NULL;				//清空运动模式
					TurnTimeCountFlag=0;//清转体计时标志
					TurnTimeCount=0;		//清转体计数值
					turn90count=0;			//清计数状态
					
					turn90_180_timing=0;
					MotorDriverVClControl(0,0);
					BREAK_L = BREAK_R = 1;//驱动器刹车
			}
	}
}
/*********************************************
	函数名称：void MotorDriverTurn_90Angle( unsigned char dir )(Timer3中每20ms调用一次)
	功		能：转体180°,结束后置状态位为END,并清RunMod.
						===设置RunMod=TURN_180_Trackless、RunStatus=GOING	===
						===					程序执行在定时器3的中断中完成					===
						===程序执行完毕后,设置RunMod=NULL、RunStatus=END	===
	入		参：dir					:转体方向(LEFT、RIGHT)
						sendstatus	:送餐状态
												 SENDING,转体送餐(正向转体90°),通过定时器计时转体90°.
												 SENDBACK,送餐返回(反向转体90°),通过寻线转体.
						注:1.根据dir和sendstatus确实转体方向以及是否播放送餐语音
									SENDING  LEFT  	:左转体,播放语音
									SENDING  RIGHT 	:右转体,播放语音
									SENDBACK LEFT  	:右转体,不播放语音
									SENDBACK RIGHT	:左转体,不播放语音
	返		值：无
*********************************************/
void MotorDriverTurn_180Angle_Trackless( unsigned char dir )
{
	static unsigned char turn180count=0;
			TracklessTurnCountFlag=1;		//开始进行转体操作计时(在定时器3的10ms中断中进行计数)
			if((TracklessTurnCount<=145000) && (dir!=MID))		//137000
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//设置目标速度为转体速度SpeedTurn
					/*	双轮转体	*/
					if(dir==RIGHT)			//餐桌在右,右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
			}else{									//转体时间到,停机
					if(SpeedTarget!=0)
						SpeedTarget=0;		//设置目标速度为0
					/*	双轮转体	*/
					if(dir==RIGHT)			//餐桌在右,右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else if(dir==LEFT)	//餐桌在左,左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
					if(SpeedReal==0)
					{
						RunStatus=END;						//实际速度减速至0,停车结束
						RunMod=NULL;							//清空运动模式
						TracklessTurnCountFlag=0;	//清转体计时标志
						TracklessTurnCount=0;			//清转体计数值
					}
			}
}
/*********************************************
	函数名称：void MotorDriverTurn_180Angle( unsigned char dir, unsigned char turn180mod )(Timer3中每20ms调用一次)
	功		能：转体180°,结束后置状态位为END,并清RunMod.
						===设置RunMod=TURN_180、RunStatus=GOING执行该操作 ===
						===					程序执行在定时器3的中断中完成					===
						===程序执行完毕后,设置RunMod=NULL、RunStatus=END	===
	入		参：dir					:转体方向(LEFT、RIGHT)
						turn180mod	:转体模式,
												 TURN180_CONTINUE(转体后继续前进);
												 TURN180_STOP(转体后停止)
	返		值：无
*********************************************/
void MotorDriverTurn_180Angle( unsigned char dir, unsigned char turn180mod )
{
	static unsigned char turn180status =0;
	static unsigned char turn180count	 =0;
	if(turn180status==0)//处于未丢线状态
	{
			if(!LineDetect_All( ))//检测到线
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//设置目标速度为转体速度SpeedTurn
					/*	双轮转体	*/
					if(dir==RIGHT)		//右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else							//左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
			}else{								//未检测到线
					/*	双轮转体	*/
					if(dir==RIGHT)		//右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else							//左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
					turn180status = 1;														//设置状态位
			}
	}else//已丢线,进入寻线模式
	{
			if( !LineDetect_Centre( ) )	//检测到线
			{	turn180count++;if(turn180count>100)turn180count=100;	}//100*5ms=500ms
			else												//未检测到线
			{ 
				turn180count=0;
				
				turn90_180_timing++;
				if(turn90_180_timing>=TURN90_180_TIME)
					turn90_180_timing=TURN90_180_TIME;
			}
			
			if( turn180count == 0 )//丢线,未检测到线
			{
					/*	在停止后已丢线的情况下(即180°转体之前已经丢线),设定转体速度（不可省略）	*/
					if(turn90_180_timing>=TURN90_180_TIME)
					{
						SpeedTarget	= 0;
						if(0==SpeedReal) turn_90_180_status=turn_90_180_status_lostline;
					}else if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//设置目标速度为转体速度SpeedTurn
					/*	双轮转体	*/
					if(dir==RIGHT)	//右转体
						MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
					else						//左转体
						MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
			}else if( turn180count >= 10 ){							//检测到线
					switch( turn180mod )
					{
						case TURN180_STOP:
							// 转体模式一：寻到线后,停车-------------------------------------
							if(SpeedTarget!=0)
								SpeedTarget=0;//设置目标速度为0
							/*	双轮转体	*/
							if(dir==RIGHT)	//右转体
								MotorDriverVClControl(SpeedReal,-SpeedReal);//速度输出
							else						//左转体
								MotorDriverVClControl(-SpeedReal,SpeedReal);//速度输出
							if(SpeedReal==0)//速度减至0
							{
								RunStatus=END;			//实际速度减速至0,停车结束
								RunMod=NULL;				//清空运动模式
								turn180status = 0;	//设置状态位
								turn180count=0;			//清计数状态
							}
						break;
						case TURN180_CONTINUE:
							// 转体模式二：寻到线后直接切换运行模式,继续前进.比较柔顺-------------------------------------
							/*	将反转车轮停转,防止下一时刻电机输出时使电机反向制动	*/
							if(dir==RIGHT)//右转体
								MotorDriverVClControl(SpeedReal,0);//速度输出
							else					//左转体
								MotorDriverVClControl(0,SpeedReal);//速度输出
							RunStatus=END;			//实际速度减速至0,停车结束
							RunMod=NULL;				//清空运动模式
							turn180status = 0;	//设置状态位
							turn180count=0;			//清计数状态
						break;
					}
					turn90_180_timing=0;
			}
			
			if(turn_90_180_status==turn_90_180_status_lostline)
			{
					RunStatus=END;			//实际速度减速至0,停车结束
					RunMod=NULL;				//清空运动模式
					turn180status = 0;	//设置状态位
					turn180count=0;			//清计数状态
					
					turn90_180_timing=0;
					MotorDriverVClControl(0,0);
					BREAK_L = BREAK_R = 1;//驱动器刹车
			}
	}
}
/*********************************************
	函数名称：void MotorDriverRouteSel( void )(Timer3中每5ms调用一次)
	功		能：在交叉点处转弯,以实现路线的选择功能
						===					通过设置RunMod=ROUTESEL执行该操作			===
						===					程序执行在定时器3的中断中完成					===
						===					程序执行完毕后,设置RunMod=NULL				===
						===					共执行8秒钟,大约向前走2.7m（速度=100）===
	入		参：无
	返		值：无
*********************************************/
void MotorDriverRouteSel( void )
{
	RouteSelTime++;

	if(RouteSelTime<=1600)
	{
		MotorDriverCtrPID();
	}else{
		RunStatus=END;//实际速度减速至0,停车结束
		RunMod=NULL;	//清空运动模式
		RouteSelTime=0;
	
		/*	正常行走(方向PID控制)	*/
		RobotRunTypeStart( SpeedSet_HV );
	}
}
/*********************************************
	函数名称：void MotorDriverSpeedIncDecA( void )
	功		能：实现电机速度的匀加减速
	入		参：无
	返		值：无
*********************************************/
void MotorDriverSpeedIncDecA( void )
{
	if(SpeedTarget!=SpeedReal)//实际速度未达到目标速度
	{
			if(SpeedTarget>SpeedReal)//加速
			{
				if(fabs(SpeedTarget-SpeedReal)>=SpeedIncA)
					SpeedReal+=SpeedIncA;
				else
					SpeedReal=SpeedTarget;
			}else//减速
			{
#if DECACC_CHANGE_EN
				if( RunModBuf == NORMAL )					//正常寻线
				{
					if(SpeedNormal>=178)			SpeedDecA=6;
					else if(SpeedNormal>=150)	SpeedDecA=5;
					else if(SpeedNormal>=140)	SpeedDecA=4;
					else if(SpeedNormal>=120)	SpeedDecA=3;
					else 											SpeedDecA=2;
				}else if( RunModBuf == ROUTESEL )	//路径选择(转弯)
				{	SpeedDecA=2;								}
				else
				{	SpeedDecA=5;								}		//其他模式
#else
				SpeedDecA=5;
#endif
				if(fabs(SpeedReal-SpeedTarget)>=SpeedDecA)
					SpeedReal-=SpeedDecA;
				else
					SpeedReal=SpeedTarget;
			}
	}
}


//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
