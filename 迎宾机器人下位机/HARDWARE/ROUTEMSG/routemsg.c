/*==========================================================================
	·����Ϣ�洢��FLASH��
	1.��������·��ʱ,д��һ��.
	2.STM32F103ZET6���������洢��(FLASH����洢��)��ַ:0x08000000-0x0807FFFF,ҳ(sector)��СΪ2KByte.
	����洢������:
			0x08000000-0x0807D7FF:502K,�û�����洢��
			0x0807D800-0x0807FFFF:	2K,·���洢��
	2016.2.21	Li
==========================================================================*/
#include "routemsg.h"
#include "time5.h"

unsigned char RouteSelDir = MID;		//·��ѡ����MID��LEFT��RIGHT
unsigned char Turn90Dir	 = MID;			//90��ת�巽��MID��LEFT��RIGHT
unsigned char Turn180Dir = LEFT;		//180��ת�巽��LEFT��RIGHT
struct ROUTEMSG RouteMsg={0,0,0,0,0};	//·���ݴ����
struct VOICEMSG VoiceMsg[11];//������ʱ�� ģʽ��Ϣ
ROUTEMSG1 RouteMsg1;
unsigned char Voice_ok=0;
/*******************************************
	�������ƣ�void RobotIDSet(unsigned short robotid)
	��		�ܣ����û����˱���
	��		�Σ�robotid,�����˱���
	��		�أ���
*******************************************/
void RobotIDSet(unsigned short robotid)
{
// 	RouteMsg.robotid = robotid;
// 	STMFLASH_Write(ROBOTID_ADDR,&robotid,1);//�洢�����˱���
}
/*******************************************
	�������ƣ�void TableDIRSet(unsigned char tablenum, unsigned short tabledir)
	��		�ܣ����ò�������
	��		�Σ�tablenum,����
						tabledir,�Ͳ�ʱ�Ĳ�������(LEFT��RIGHT)
	��		�أ���
*******************************************/
void TableDIRSet(unsigned char tablenum, unsigned short tabledir)
{
// 	STMFLASH_Write(TABLEDIR_ADDR(tablenum),&tabledir,1);//�洢��������(LEFT��RIGHT)
}
/*******************************************
	�������ƣ�void RouteLeftSet(unsigned char tablenum, unsigned short routel)
	��		�ܣ����ò�������ת���·�������ţ�
	��		�Σ�tablenum,����
						routel,��ת��㿨��
	��		�أ���
*******************************************/
void RouteLeftSet(unsigned char tablenum, unsigned short routel)
{
// 	STMFLASH_Write(ROUTE_ADDR(tablenum),&routel,1);//�洢��������ת��㿨��
}
/*******************************************
	�������ƣ�void RouteRightSet(unsigned char tablenum, unsigned short router)
	��		�ܣ����ò�������ת���·�������ţ�
	��		�Σ�tablenum,����
						routel,��ת��㿨��
	��		�أ���
*******************************************/
void RouteRightSet(unsigned char tablenum, unsigned short router)
{
// 	STMFLASH_Write(ROUTE_ADDR(tablenum)+2,&router,1);//�洢��������ת��㿨��
}
/*******************************************
	�������ƣ�void RouteRightSet(unsigned char tablenum, unsigned short router)
	��		�ܣ������ٶ�ģʽ
	��		�Σ�speedmod:SPEEDMOD_H(1)��SPEEDMOD_M(0)��SPEEDMOD_L(2)
	��		�أ���
*******************************************/
void SpeedModSet(unsigned short speedmod)
{
	STMFLASH_Write(SPEEDMOD_BASEADDR,&speedmod,1);//�洢��������ת��㿨��
}
/*******************************************
	�������ƣ�void RobotModTrackSet(unsigned short robotmod)
	��		�ܣ����û�����ģʽ
	��		�Σ�(0:�й�,1:�޹�)
	��		�أ���
*******************************************/
void RobotModTrackSet(unsigned short robotmod)
{
	STMFLASH_Write(ROBOTMOD_TRACK_BASEADDR,&robotmod,1);
}
/*******************************************
	�������ƣ�void TracklessDistSet(unsigned short speedmod)
	��		�ܣ������޹����
	��		�Σ�0-9
	��		�أ���
*******************************************/
void StationDirSet(unsigned short tracklessdist)
{
	RouteMsg1.Station_DIR=tracklessdist;
	STMFLASH_Write(StationDIR_BASEADDR,&tracklessdist,1);
	
	
}
/*******************************************
	�������ƣ�void VoiceTimeSet(unsigned char voicenum,unsigned short voicetime)
	��		�ܣ�����������n��ʱ��
	��		�Σ�voicenum�������κ�
						voicetime����λ��
	��		�أ���
*******************************************/
void VoiceTimeSet(unsigned char voicenum,unsigned short voicetime)
{
	STMFLASH_Write(VOICE_TIME_ADDR(voicenum),&voicetime,1);
}
/*******************************************
	�������ƣ�void VoiceModSet(unsigned char voicenum,unsigned short voicemod)
	��		�ܣ�����������n��ģʽ
	��		�Σ�voicenum�������κ�
						voicemod��ģʽ
	��		�أ���
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
	�������ƣ�unsigned char RobotIDRead( void )
	��		�ܣ���ȡ�����˱���
	��		�Σ���
	��		�أ������˱���
*******************************************/
unsigned char RobotIDRead( void )
{
// 	return (unsigned char)( *(unsigned short*) ROBOTID_ADDR );
}
/*******************************************
	�������ƣ�unsigned char TableDIRRead(unsigned char tablenum)
	��		�ܣ���ȡ��������
	��		�Σ�tablenum,����
	��		�أ���������
*******************************************/
unsigned char TableDIRRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) TABLEDIR_ADDR(tablenum) );
}
/*******************************************
	�������ƣ�unsigned char RouteLeftRead(unsigned char tablenum)
	��		�ܣ���ȡ��������ת���·�������ţ�
	��		�Σ�tablenum,����
	��		�أ���������ת���·�������ţ�
*******************************************/
unsigned char RouteLeftRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) ROUTE_ADDR(tablenum) );
}
/*******************************************
	�������ƣ�unsigned char RouteRightRead(unsigned char tablenum)
	��		�ܣ���ȡ��������ת���·�������ţ�
	��		�Σ�tablenum,����
	��		�أ���������ת���·�������ţ�
*******************************************/
unsigned char RouteRightRead(unsigned char tablenum)
{
// 	return (unsigned char)( *(unsigned short*) (ROUTE_ADDR(tablenum)+2) );
}
/*******************************************
	�������ƣ�unsigned char RouteRightRead(unsigned char tablenum)
	��		�ܣ���ȡ�ٶ�ģʽ
	��		�Σ���
	��		�أ��ٶ�ģʽ
*******************************************/
unsigned char SpeedModRead(void)
{
	return (unsigned char)( *(unsigned short*) (SPEEDMOD_BASEADDR) );
}
/*******************************************
	�������ƣ�unsigned char RobotModTrackRead(unsigned char tablenum)
	��		�ܣ���ȡ������ģʽ(0:�й�,1:�޹�)
	��		�Σ���
	��		�أ�������ģʽ(0:�й�,1:�޹�)
*******************************************/
unsigned char RobotModTrackRead(void)
{
	return (unsigned char)( *(unsigned short*) (ROBOTMOD_TRACK_BASEADDR) );
}
/*******************************************
	�������ƣ�unsigned char TracklessDistRead(unsigned char tablenum)
	��		�ܣ���ȡ���Ե�ת��
	��		�Σ���
	��		�أ��޹����(0-9��)
*******************************************/
unsigned char StationDirRead(void)
{
	return (unsigned char)( *(unsigned short*) (StationDIR_BASEADDR) );
}
/*******************************************
	�������ƣ�unsigned short VoiceTimeRead(unsigned char voicenum)
	��		�ܣ���ȡ������n��ʱ��
	��		�Σ�voicenum�������κ�
	��		�أ�ʱ��
*******************************************/
unsigned short VoiceTimeRead(unsigned char voicenum)
{
	return ( *(unsigned short*) (VOICE_TIME_ADDR(voicenum)) );
}
/*******************************************
	�������ƣ�unsigned char VoiceModRead(unsigned char voicenum)
	��		�ܣ���ȡ������n��ģʽ
	��		�Σ�voicenum�������κ�
	��		�أ�ģʽ
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
	�������ƣ�void RouteInfInit(unsigned char mod)
	��		�ܣ����е���������ʼ������
	��		�Σ�mod,1:ǿ������;0:��������(����Ƿ�������)
	��		�أ���
*******************************************/
void RouteInfInit(unsigned char mod)
{
	unsigned short ClearFlag=0x55AA;
	unsigned char  buff[1024];
	unsigned int   i=0;
	unsigned long  addr=0;
	if(ClearFlag !=(*(unsigned short *)INFALLCLEAR_BASE) || mod==1)//ϵͳ�״�����,��δ���FLASH������
	{
		for(i=0;i<1024;i++)
			buff[i]=0;
		addr=DATA_BASE;
		for(i=0;i<10;i++)//���FLASH���10K�Ŀռ�
		{
			STMFLASH_Write(addr,(unsigned short *)buff,512);
			addr=addr+1024;
		}
		
		STMFLASH_Write(INFALLCLEAR_BASE,&ClearFlag,1);//����������ֽ�д�����0x55AA,��־������.
	}
	switch(RouteMsg.speedmod= SpeedModRead())//��ȡ�趨�ٶ�
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

//����ָ��
void RouteMsg_CommandSet( unsigned char command )
{
	RouteMsg1.command = command;
}
//��ȡָ��
unsigned char RouteMsg_CommandGet( void )
{
	return RouteMsg1.command;
}
//���÷�����
void RouteMsg_VoiceNumSet( unsigned char voicenum )
{
	RouteMsg1.playvoicenume = voicenum;
}
//��ȡ������
unsigned char RouteMsg_VoiceNumGet( void )
{
	return RouteMsg1.playvoicenume;
}
//�����ϰ���״̬
void RouteMsg_BarrierStaSet( unsigned char barriersta )
{
	RouteMsg1.barrierstatus = barriersta;
}
//��ȡ�ϰ���״̬
unsigned char RouteMsg_BarrierStaGet( void )
{
	return RouteMsg1.barrierstatus;
}
//�����ٶȵȼ�
void RouteMsg_SpeedModSet( unsigned char speedmod )
{
	RouteMsg1.speedmod = speedmod;
	STMFLASH_Write(SPEEDMOD_BASEADDR,(unsigned short*)&speedmod,1);
}
//��ȡ�ٶȵȼ�
unsigned char RouteMsg_SpeedModGet( void )
{
	return RouteMsg1.speedmod  = (unsigned char) ( *(unsigned short*) (SPEEDMOD_BASEADDR) );
}
//д��Ĵ�������,����ͨѶ�ж�ʹ��
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
//��ȡ�Ĵ�������,����ͨѶ�ж�ʹ��
unsigned short ReadRegData( unsigned short regaddr )
{
	unsigned short tempdata;
	switch(regaddr)
	{
		case COMMAND_REG:		tempdata = RouteMsg_CommandGet();
			break;
		case VOICENUM_REG:		tempdata = RouteMsg_VoiceNumGet();	RouteMsg_VoiceNumSet(NULL);//��ȡ���������κ�,��շ����μĴ���
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
