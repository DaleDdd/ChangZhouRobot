/*==========================================================================
	路径信息存储置FLASH中
	1.仅在设置路径时,写入一次.
	2.STM32F103ZET6大容量主存储块(FLASH程序存储器)地址:0x08000000-0x0807FFFF,页(sector)大小为2KByte.
	程序存储器分配:
			0x08000000-0x0807D7FF:502K,用户程序存储区
			0x0807D800-0x0807FFFF:	2K,路径存储区
	2016.2.21	Li
==========================================================================*/
#include "routemsg.h"
#include "time5.h"

unsigned char RouteSelDir = MID;		//路径选择方向，MID、LEFT、RIGHT
unsigned char Turn90Dir	 = MID;			//90°转体方向，MID、LEFT、RIGHT
unsigned char Turn180Dir = LEFT;		//180°转体方向，LEFT、RIGHT
struct ROUTEMSG RouteMsg={0,0,0,0,0};	//路径暂存变量
struct VOICEMSG VoiceMsg[11];//语音段时长 模式信息
ROUTEMSG1 RouteMsg1;
unsigned char Voice_ok=0;
/*******************************************
	函数名称：void RobotIDSet(unsigned short robotid)
	功		能：设置机器人编码
	入		参：robotid,机器人编码
	返		回：无
*******************************************/
void RobotIDSet(unsigned short robotid)
{
// 	RouteMsg.robotid = robotid;
// 	STMFLASH_Write(ROBOTID_ADDR,&robotid,1);//存储机器人编码
}
/*******************************************
	函数名称：void TableDIRSet(unsigned char tablenum, unsigned short tabledir)
	功		能：设置餐桌方向
	入		参：tablenum,桌号
						tabledir,送餐时的餐桌方向(LEFT、RIGHT)
	返		回：无
*******************************************/
void TableDIRSet(unsigned char tablenum, unsigned short tabledir)
{
// 	STMFLASH_Write(TABLEDIR_ADDR(tablenum),&tabledir,1);//存储餐桌方向(LEFT、RIGHT)
}
/*******************************************
	函数名称：void RouteLeftSet(unsigned char tablenum, unsigned short routel)
	功		能：设置餐桌的左转弯点路径（卡号）
	入		参：tablenum,桌号
						routel,左转弯点卡号
	返		回：无
*******************************************/
void RouteLeftSet(unsigned char tablenum, unsigned short routel)
{
// 	STMFLASH_Write(ROUTE_ADDR(tablenum),&routel,1);//存储餐桌的左转弯点卡号
}
/*******************************************
	函数名称：void RouteRightSet(unsigned char tablenum, unsigned short router)
	功		能：设置餐桌的右转弯点路径（卡号）
	入		参：tablenum,桌号
						routel,右转弯点卡号
	返		回：无
*******************************************/
void RouteRightSet(unsigned char tablenum, unsigned short router)
{
// 	STMFLASH_Write(ROUTE_ADDR(tablenum)+2,&router,1);//存储餐桌的右转弯点卡号
}
/*******************************************
	函数名称：void RouteRightSet(unsigned char tablenum, unsigned short router)
	功		能：设置速度模式
	入		参：speedmod:SPEEDMOD_H(1)、SPEEDMOD_M(0)、SPEEDMOD_L(2)
	返		回：无
*******************************************/
void SpeedModSet(unsigned short speedmod)
{
	STMFLASH_Write(SPEEDMOD_BASEADDR,&speedmod,1);//存储餐桌的右转弯点卡号
}
/*******************************************
	函数名称：void RobotModTrackSet(unsigned short robotmod)
	功		能：设置机器人模式
	入		参：(0:有轨,1:无轨)
	返		回：无
*******************************************/
void RobotModTrackSet(unsigned short robotmod)
{
	STMFLASH_Write(ROBOTMOD_TRACK_BASEADDR,&robotmod,1);
}
/*******************************************
	函数名称：void TracklessDistSet(unsigned short speedmod)
	功		能：设置无轨距离
	入		参：0-9
	返		回：无
*******************************************/
void StationDirSet(unsigned short tracklessdist)
{
	RouteMsg1.Station_DIR=tracklessdist;
	STMFLASH_Write(StationDIR_BASEADDR,&tracklessdist,1);
	
	
}
/*******************************************
	函数名称：void VoiceTimeSet(unsigned char voicenum,unsigned short voicetime)
	功		能：设置语音段n的时长
	入		参：voicenum：语音段号
						voicetime：单位秒
	返		回：无
*******************************************/
void VoiceTimeSet(unsigned char voicenum,unsigned short voicetime)
{
	STMFLASH_Write(VOICE_TIME_ADDR(voicenum),&voicetime,1);
}
/*******************************************
	函数名称：void VoiceModSet(unsigned char voicenum,unsigned short voicemod)
	功		能：设置语音段n的模式
	入		参：voicenum：语音段号
						voicemod：模式
	返		回：无
*******************************************/
void VoiceModSet(unsigned char voicenum,unsigned short voicemod)
{
	STMFLASH_Write(VOICE_MOD_ADDR(voicenum),&voicemod,1);
}
void VoiceRunModSet(unsigned char voicenum,unsigned short voicerunmod)
{
	STMFLASH_Write(VOICE_RMOD_ADDR(voicenum),&voicerunmod,1);
}


void Station_modeSet(unsigned char station_id,unsigned short mode)
{
	
	STMFLASH_Write(STATION_MODE(station_id),&mode,1);
}

unsigned short Station_modeRead(unsigned char station_id)
{
	return  (unsigned char)( *(unsigned short*) (STATION_MODE(station_id)) );
}

void Station_timSet(unsigned char station_id,unsigned short tim)
{
	
	STMFLASH_Write(STATION_TIM(station_id),&tim,1);
}

unsigned short Station_timRead(unsigned char station_id)
{
	return  (unsigned char)( *(unsigned short*) (STATION_TIM(station_id)) );
}

void Cross_Set(unsigned cross_id,unsigned short dir)
{
	STMFLASH_Write(CROSS_SET(cross_id),&dir,1);
}
unsigned short Cross_Read(unsigned char cross_id)
{
	return  (unsigned char)( *(unsigned short*) (CROSS_SET(cross_id)) );
}
/*******************************************
	函数名称：unsigned char RobotIDRead( void )
	功		能：获取机器人编码
	入		参：无
	返		回：机器人编码
*******************************************/
unsigned char RobotIDRead( void )
{
// 	return (unsigned char)( *(unsigned short*) ROBOTID_ADDR );
}
/*******************************************
	函数名称：unsigned char TableDIRRead(unsigned char tablenum)
	功		能：获取餐桌方向
	入		参：tablenum,桌号
	返		回：餐桌方向
*******************************************/
unsigned char TableDIRRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) TABLEDIR_ADDR(tablenum) );
}
/*******************************************
	函数名称：unsigned char RouteLeftRead(unsigned char tablenum)
	功		能：获取餐桌的左转弯点路径（卡号）
	入		参：tablenum,桌号
	返		回：餐桌的左转弯点路径（卡号）
*******************************************/
unsigned char RouteLeftRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) ROUTE_ADDR(tablenum) );
}
/*******************************************
	函数名称：unsigned char RouteRightRead(unsigned char tablenum)
	功		能：获取餐桌的右转弯点路径（卡号）
	入		参：tablenum,桌号
	返		回：餐桌的右转弯点路径（卡号）
*******************************************/
unsigned char RouteRightRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) (ROUTE_ADDR(tablenum)+2) );
}
/*******************************************
	函数名称：unsigned char RouteRightRead(unsigned char tablenum)
	功		能：获取速度模式
	入		参：无
	返		回：速度模式
*******************************************/
unsigned char SpeedModRead(void)
{
	return (unsigned char)( *(unsigned short*) (SPEEDMOD_BASEADDR) );
}
/*******************************************
	函数名称：unsigned char RobotModTrackRead(unsigned char tablenum)
	功		能：获取机器人模式(0:有轨,1:无轨)
	入		参：无
	返		回：机器人模式(0:有轨,1:无轨)
*******************************************/
unsigned char RobotModTrackRead(void)
{
	return (unsigned char)( *(unsigned short*) (ROBOTMOD_TRACK_BASEADDR) );
}
/*******************************************
	函数名称：unsigned char TracklessDistRead(unsigned char tablenum)
	功		能：获取初试点转向
	入		参：无
	返		回：无轨距离(0-9米)
*******************************************/
unsigned char StationDirRead(void)
{
	return (unsigned char)( *(unsigned short*) (StationDIR_BASEADDR) );
}
/*******************************************
	函数名称：unsigned short VoiceTimeRead(unsigned char voicenum)
	功		能：获取语音段n的时长
	入		参：voicenum：语音段号
	返		回：时长
*******************************************/
unsigned short VoiceTimeRead(unsigned char voicenum)
{
	return ( *(unsigned short*) (VOICE_TIME_ADDR(voicenum)) );
}
/*******************************************
	函数名称：unsigned char VoiceModRead(unsigned char voicenum)
	功		能：获取语音段n的模式
	入		参：voicenum：语音段号
	返		回：模式
*******************************************/
unsigned char VoiceModRead(unsigned char voicenum)
{
	return (unsigned char)( *(unsigned short*) (VOICE_MOD_ADDR(voicenum)) );
}
unsigned char VoiceRunModRead(unsigned char voicenum)
{
	return (unsigned char)( *(unsigned short*) (VOICE_RMOD_ADDR(voicenum)) );
}
/*******************************************
	函数名称：void RouteInfInit(unsigned char mod)
	功		能：所有的数据区初始化清零
	入		参：mod,1:强制清零;0:条件清零(检测是否已清零)
	返		回：无
*******************************************/
void RouteInfInit(unsigned char mod)
{
	unsigned short ClearFlag=0x55AA;
	unsigned char  buff[1024];
	unsigned int   i=0;
	unsigned long  addr=0;
	if(ClearFlag !=(*(unsigned short *)INFALLCLEAR_BASE) || mod==1)//系统首次运行,还未清空FLASH数据区
	{
		for(i=0;i<1024;i++)
			buff[i]=0;
		addr=DATA_BASE;
		for(i=0;i<10;i++)//清空FLASH最后10K的空间
		{
			STMFLASH_Write(addr,(unsigned short *)buff,512);
			addr=addr+1024;
		}
		
		STMFLASH_Write(INFALLCLEAR_BASE,&ClearFlag,1);//在最后两个字节写入半字0x55AA,标志已清零.
	}
	switch(RouteMsg.speedmod= SpeedModRead())//获取设定速度
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
			SpeedSet_HV=185;
			break;
		case SPEEDMOD_M:
			SpeedSet_HV=160;
			break;
		case SPEEDMOD_L:
			SpeedSet_HV=135;
			break;
		default:
			SpeedSet_HV=160;
			break;
	}
}

//设置指令
void RouteMsg_CommandSet( unsigned char command )
{
	RouteMsg1.command = command;
}
//读取指令
unsigned char RouteMsg_CommandGet( void )
{
	return RouteMsg1.command;
}
//设置放音段
void RouteMsg_VoiceNumSet( unsigned char voicenum )
{
	RouteMsg1.playvoicenume = voicenum;
}
//读取放音段
unsigned char RouteMsg_VoiceNumGet( void )
{
	return RouteMsg1.playvoicenume;
}
//设置障碍物状态
void RouteMsg_BarrierStaSet( unsigned char barriersta )
{
	RouteMsg1.barrierstatus = barriersta;
}
//获取障碍物状态
unsigned char RouteMsg_BarrierStaGet( void )
{
	return RouteMsg1.barrierstatus;
}
//设置速度等级
void RouteMsg_SpeedModSet( unsigned char speedmod )
{
	RouteMsg1.speedmod = speedmod;
	STMFLASH_Write(SPEEDMOD_BASEADDR,(unsigned short*)&speedmod,1);
}
//获取速度等级
unsigned char RouteMsg_SpeedModGet( void )
{
	return RouteMsg1.speedmod  = (unsigned char) ( *(unsigned short*) (SPEEDMOD_BASEADDR) );
}
//写入寄存器数据,串口通讯中断使用
void WriteRegData( unsigned short regaddr, unsigned short regdata )
{
	unsigned  char turndir;
	unsigned short distance;
	unsigned  char angle;
	switch(regaddr)
	{
		case COMMAND_REG:		RouteMsg_CommandSet(regdata);
								CommandReceiveStatus = OK;
			break;
		case VOICENUM_REG:		RouteMsg_VoiceNumSet(regdata);
			break;
		case BARRIERSTA_REG:	RouteMsg_BarrierStaSet(regdata);
			break;
		case SPEEDMOD_REG:		RouteMsg_SpeedModSet(regdata);
			break;
		case ROUTEMOD_REG:      StationDirSet(regdata);   
			break;
		default:
			break;
	}
}
//获取寄存器数据,串口通讯中断使用
unsigned short ReadRegData( unsigned short regaddr )
{
	unsigned short tempdata;
	switch(regaddr)
	{
		case COMMAND_REG:		tempdata = RouteMsg_CommandGet();
			break;
		case VOICENUM_REG:		tempdata = RouteMsg_VoiceNumGet();	RouteMsg_VoiceNumSet(NULL);//读取放音语音段后,清空放音段寄存器
			break;
		case BARRIERSTA_REG:	tempdata = RouteMsg_BarrierStaGet();
			break;
		case SPEEDMOD_REG:		tempdata = RouteMsg_SpeedModGet();
			break;
		case ROUTEMOD_REG:      tempdata = StationDirRead();
		default:
			break;
	}
	return tempdata;
}


//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
