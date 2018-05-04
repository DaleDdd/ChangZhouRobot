/*---------------------------------------------------------------------------------------
		Catering Robot V3.1,2016-8-31,Li
		
		���ܣ�ӭ�������������ý��棬��������1-10��ʱ�䡢����ģʽ������ģʽ���Լ��ٶȡ�
					��������ʼ����������ǰ����
					ֹͣ��ֹͣ����������ǰ����
					�˳����˳���ǰ���档
					
		һ���������ж����ȼ�����˵��:
			�ж�Դ						��ռ���ȼ�			��Ӧ���ȼ�			����
			USART1(HMI���豸)			1								3							2
			USART2(����������)		2								0							2
			USART4(ID��)					0								3							2
			USART5(����ģ��)			1								3							2
			TIM2(���ݳ�ʱ��ʱ)		2								3							2
			TIM3(�����ŷ�����)		3								2							2
			TIM4(PWM���)					
			TIM5(���ݳ�ʱ��ʱ)		2								3							2
			TIM6(���ݳ�ʱ��ʱ)		0								3							2
			LINE0(�����ⲿ�ж�)		2								3							2
			LINE1(�����ⲿ�ж�)		2								3							2
			LINE2(��ͣ�ⲿ�ж�)		3								3							2
			LINE3(�����ⲿ�ж�)		3								3							2
			
		����USART���ڷ���˵��:
			USART1->HMI�ӿ�,19200,8+n+1
			USART2->
			USART3->
			USART4->ID������,9600,8+n+1
			USART5->���ߴ���ģ��,19200��8+n+1

		�������ڵ�����:
			(1)��������ʱ,485���͵ĵ�һ��������Ч.<2015-8-28>
				�������:��485��������֮ǰ��һ���ȴ����,while((USART2->SR&0x0040) == 0x00),�ȴ����ݷ������.
				
			(2)ÿ�ν������������ص�����ʱ,�����ȶ����յ�һ��0x00�ֽ�.<2015-8-28>
				�������:��485�������ݵ�ʱ��,��ֹ����,�ȷ�����ɺ���ʹ�ܽ���.
				
			(3)�ڶ�ʱ��3���жϷ��������д�����PI�����㷨��,����ִ�г��ֲ���������.��ÿ���ŷ�����(20��40ms)
				�ڶ�ÿһ���������ٶȿ������(RS485 115200bps��,��Լ��Ҫ0.9ms)һ������,������(RS485 115200bps��,
				��Լ��Ҫ2.6ms),��ʵ������ʱ�����޷�����ִ��.<2015-9-29>
				�������:TIM3���ж����ȼ�����С�ڴ���2(USART2)�Ľ����ж����ȼ��Լ�TIM2���ж����ȼ�,������TIM3
								 ���ж�����봮��2�Լ�TIM2���ж�.<<����ԭ��>>
								 (ͬһ�����в�Ҫ������������,��ռ���ȼ�������Ӧ���ȼ�,��ֵԽС���ȼ�Խ��,����ռ���ȼ�
								 ���Դ�ϵ���ռ���ȼ���ִ��,����Ӧ���ȼ����ܴ�ϵ���Ӧ���ȼ���ִ��)
				
			(4)��������������485ͨѶʱ��������һ��0x00�ֽ�.<2015-9-29>
				�������:ȥ��������485оƬ������120������.һ�����ų�����ͨѶ���ȶ���������,�ſ��ǲ���120�����ն�
				����(�迹ƥ�����).
				
			(5)FLASH��дSECTOR������ԼҪ80ms,����ڶ�ʱ��3���ж��в��ɵ���,���򽫵��³����ܷ�.������Ͳ͹���
				�е�ת��ʱ�䲻��ʵʱ��¼,����,һ���ڼ�¼(д��FLASH)�����и�λ�����,�����³����ܷ�.<2015-11-7>

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

// �궨��*********************************************************************************************************************************
#define IWDEN							(1u)						//ʹ���ڲ����Ź�
#define SPEEDDOWNEN				(1u)						//�������1�����٣�0��������
#define INCOMPLETEPIDEN		(1u)						//����ȫ΢���㷨1�����ò���ȫ΢���㷨��0��������
#define DECACC_CHANGE_EN	(1u)						//���ٶȼ��ٶ����ٶȱ仯ʹ��
#define TRACKLESS_BACKEN	(0u)						//�޹�ģʽ�º���ʹ��
#define VOICEBARRIERTIME	(7u)						//���������趨ʱ��,��λ���롾ʵ������ʱ��+���ʱ��(Ĭ��Ϊ5��)��
#define BARRIERPLAYCONTEN	(0u)						//����������������ʹ��

// �˿ڶ���=============================================
#define ST								PFout(10)				//�ⲿ���Ź���λ����

// �ŵ��򴫸������Ŷ���=================================
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

// �˶�������غ�=========================================
#define NORMAL								0x01	//����ģʽ������PIDѰ��ģʽ
#define STOP									0x02	//����ģʽ��ͣ��ģʽ
#define	TURN_90								0x03	//����ģʽ��ת��ģʽ
#define ROUTESEL							0x04	//����ģʽ��·��ѡ��ģʽ
#define TURN_180							0x05	//����ģʽ��ԭ��ת��180��ģʽ
#define TURN_180_Trackless		0x06	//����ģʽ���޹�ת��180��
#define BACK_Trackless				0x07	//����ģʽ���޹����
#define	GOING									0x01	//����ִ��--ͣ����ת��ģʽ��ִ��״̬--
#define	END										0x02	//ִ�н���--ͣ����ת��ģʽ��ִ��״̬--
#define SENDING								0x01	//�Ͳ�״̬,ת���Ͳ�,����ת��90��,�����Ͳ�����
#define SENDBACK							0x02	//�Ͳ�״̬,�Ͳͷ���,����ת��90��,����������
#define STOPIDTIMING					0x01	//ֹͣ��ȡ�͵�ر���Ч��ʱ,��ʼ�Ͳͺ��ʱ������Ч
#define STOPIDVALID						0x02	//ֹͣ��ȡ�͵�ر���Ч
#define STOPIDUNVALID					0x03	//ֹͣ��ȡ�͵�ر���Ч,
#define RUNFLAG_NULL					0x00	//
#define RUNFLAG_RUNING				0x01	//����ǰ��
#define RUNFLAG_STOP					0x02	//ֹͣ(��ͣ)ǰ��
#define RUNFLAG_LOSTLINE			0x03	//����ֹͣ
#define TURN180_CONTINUE			0x01	//ת��180������ǰ��
#define TURN180_STOP					0x02	//ת��180���ֹͣ
#define TURN180_132NO					0x00	//132�ر� δ����
#define TURN180_132END				0x01	//132�ر� �Ѿ���,��ת��
#define CtrAreaOut						0x00	//�ڹ�������
#define CtrAreaIn							0x01	//���ڹ�������
#define RouteValid						((unsigned char)0x00)	//·��ת�����Ч
#define RouteInValid					((unsigned char)0x01)	//·��ת�����Ч
// ȫ�ֱ�������***************************************************************************************************************************

// ���Ʊ�־λ����=======================================
unsigned char RunFlag							=RUNFLAG_STOP;	//���б�־
unsigned char RunMod							=STOP;					//С��������ģʽ,NORMAL\STOP\TURN_90
unsigned char RunStatus							=END;						//�˶�ģʽ��ִ��״̬,GOING\END
unsigned char SendStatus						=0;							//�Ͳ�״̬,SENDING\SENDBACK\NULL
unsigned char Turn180Mod						=0;							//ԭ��ת��180��ģʽ��TURN180_CONTINUE(ת������ǰ��);TURN180_STOP(ת���ֹͣ)
unsigned char DIRBuf							=0;
unsigned char RunModBuf							=0;
unsigned int  TurnTimeCount						=0;							//ת��ʱ�����ֵ(ͨ��TIM3���м���),��λ5ms
unsigned char TurnTimeCountFlag					=0;							//ת��ʱ�俪ʼ������־,1:���м�����0:������
unsigned char BackTimeCount						=0;							//�Ͳͷ���ʱ�����ֵ(ͨ��TIM3���м���),��λ100ms
unsigned char Turn180TimeCount					=0;							//180��ת��ر���Ч����
unsigned int  RouteSelTime						=0;							//·��ѡ��ʱ���ʱ
unsigned char StopIDStatusFlag					=STOPIDUNVALID;	//ֹͣ��ȡ�͵�ر���Ч��־λ,��ֹ�������Ͳ�ʱ����ȡ�͵�ر��ֹͣ
unsigned char ReceiveOrder						=0;							//������ȫ�ֱ���,���ڱ������ߴ���(UART5)���յ���������
unsigned char Turn180_132_Flag					=TURN180_132NO;	//132�ر�ת��״̬,TURN180_132NO:δ����;TURN180_132END:�Ѿ���,��ת��
unsigned char CtrAreaFlag						=CtrAreaOut;		//�Ƿ��ڹ�������־,��ʼ�����ڹ�����
unsigned char RouteLFlag						=RouteValid;		//·����ת���,Ĭ����Ч
unsigned char RouteRFlag						=RouteValid;		//·����ת���,Ĭ����Ч

// ���Ʊ�������=======================================
signed		int SpeedMax						=PWMDUTY_MAX;/*�ٶȱջ���������ٶ�*/
signed 		int SpeedSet_HV						=0;//200
signed 		int SpeedSet_LV						=100;
signed 		int SpeedSet_BLV					=60;
signed 		int SpeedSearchLine					=20; /*Ѱ���ٶ�0-1000*/
signed 		int SpeedTurn						=20; /*ת���ٶ�0-1000*/
signed 		int SpeedRouteSel					=100;	/*·��ѡ���ٶ�*/
signed  	int SpeedIncA 						=2;  /*���ټ��ٶ�,ÿ40ms���ӵ��ٶ�*/
signed  	int SpeedDecA 						=4;  /*���ټ��ٶ�,ÿ40ms���ٵ��ٶ�*/
signed 		int SpeedTarget						=0;	 //������е�Ŀ���ٶ�
signed 		int SpeedNormal						=0;  //����ǰ���ٶ�,SpeedSet_HV��SpeedSet_LV��0
signed 		int SpeedReal 						=0;	 //ʵ���ٶ�(���ҵ��ʩ���ٶȵ�ƽ��ֵ)
signed 		int SpeedRealL						=0;	 //����ʵ���ٶ�
signed 		int SpeedRealR						=0;	 //�ҵ��ʵ���ٶ�

// float 				Kp=12.0;						//����ϵ��//10-16
// float 				Ki=0.080;						//����ϵ��//0.080
float 				Kp=0.0;							//����ϵ��//10-16
float 				Ki=0.001;						//����ϵ��//0.080
float 				Kd=0;								//����ϵ��//10-16
signed	 char EMAX=15;						//ƫ�����ֵ
signed	 char EMIN=-15;						//ƫ�����ֵ
signed	 int	MAXSUME=1500;				//ƫ������޷�ֵ
float 				Uk0=0;							//��ǰ�ŷ����ڵ��ٶ�PI���������
float 				Uk1=0;							//ǰһ�ŷ����ڵ��ٶ�PI���������
signed	 char	E0=0;								//��ǰʱ�̵�ƫ��
signed	 char	E1=0;								//ǰһʱ�̵�ƫ��
signed	 int	SumE=0;							//ƫ�����(�ۼ�)
#if SPEEDDOWNEN
float 				Q_e=0;							//������ٵ�ƫ��ϵ��
#endif
#if INCOMPLETEPIDEN
float					Alpha0=0.92;					//����ȫ΢��ϵ�� alpha0+Alpha1=1
float					Alpha1=0.08;					//����ȫ΢��ϵ�� alpha0+Alpha1=1
#endif

const unsigned int TURN90_180_TIME	=2400;//2400*5=12000ms=12s
unsigned int turn90_180_timing=0;
unsigned char turn_90_180_status=NULL;
#define turn_90_180_status_NULL				NULL
#define turn_90_180_status_lostline		(1)

#define VOICEPLAYEND 				0				//���Ž���
#define VOICEPLAYING 				1				//���ڲ���
#define VOICETIMEDELAY				1				//���ż��ʱ��,��λ����
#define TracklessNULL				0
#define Tracklessing				1
#define Tracklessend				2
#define TracklessTurn180			3
#define TracklessBack				4
#define TracklessbackNULL			0
#define Tracklessbacking			1
#define Tracklessbackend			2

#define TracklessDistQ			330//�޹����ת��ϵ��, �趨ֵ(0-9��) * TracklessDistQ = ����������(��)
unsigned char  BarrierEn=ENABLE;															//��������ʹ��λ
unsigned char  VoicePlayMod=VOICEMOD_NULL;										//��������ģʽ
unsigned char  VoicePlayNumList[10]={0,0,0,0,0,0,0,0,0,0};		//���������б�
unsigned short VoiceTimeCount=0;															//��ǰ��������ʱ��
unsigned char  VoicePlayStatus=VOICEPLAYEND;									//��ǰ��������״̬��VOICEPLAYEND:���Ž�����VOICEPLAYING:���ڲ���
unsigned char  ID_Last=0;																			//�����ϴεĶ�����ID����,��ֹ�ظ���ȡͬһID
unsigned char  RobotMod=ROBOTMOD_TRACK;												//������ģʽ(0:�й�,1:�޹�)
unsigned long  TracklessDistance;//L*280mm
unsigned long  TracklessbackDistance=70000;
unsigned long  TracklessOffset=0;															//�������򽫺��˾����趨Ϊǰ�������ƫ����
unsigned char	 TracklessCountFlag=TracklessNULL;//TracklessNULL��Tracklessing��Tracklessend��TracklessTurn180��TracklessBack
unsigned long	 TracklessCount=0;
unsigned char	 TracklessbackCountFlag=TracklessbackNULL;
unsigned long	 TracklessbackCount=0;
unsigned char  Turn180Trackless_DirBuf=MID;//LEFT��RIGHT��MID_BACK��MID
unsigned long  TracklessTurnCount=0;
unsigned long  TracklessTurnCountFlag=0;
 
unsigned char Turn_132_FLag;
// ��������************************************************************************************************
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
void GuidSensorPort_Init( void );		//�ŵ��򴫸����˿ڳ�ʼ��
signed char GetPos(void);						//��ȡ�յ���������ֵ,����ȡ��ǰ��λ����Ϣ
void BarrierDirDetect( void );			//����ʱƫ�뷽����
unsigned char LineDetect_Centre( void );		//ת��Ѱ��
unsigned char LineDetect_All( void );				//ת��Ѱ��
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
//			main����
//================================================================================================================================
int main(void)
{
	unsigned char index=0;
	#if SPEEDDOWNEN
	Q_e=(SpeedSet_HV-SpeedSet_LV)/15;//��ʼ������ϵ��
	#endif
	Stm32_Clock_Init(9); 				//72M��ʹ��8M�ⲿ���پ���HSE��9��Ƶ����Ϊϵͳʱ��
	JTAG_Set(SWD_ENABLE);				//ʹ��SWD, �ͷ�PA15��PB3 PB4Ϊ��ͨIO��
	delay_init(72); 						//��ʱ��ʼ��
	WD_Init( );									//��ʼ���ⲿ���Ź�����
	ST = 0;delay_ms(10);
	ST = 1;delay_ms(10);
	ST = 0;delay_ms(10);
	ST = 1;delay_ms(10);
	TIM3_Int_Init(49,7199);					//10Khz�ļ���Ƶ��,����50��Ϊ5ms
	LED_Init();								//LED�˿ڳ�ʼ��
	GuidSensorPort_Init();					//�յ���������ʼ��
	Barrier_Init();							//���ϴ�����IO�˿ڳ�ʼ��
	BackButton_Init();						//���ذ�ť(�̵�������)�˿ڳ�ʼ��
	Motion_Init( );							//�����������ʼ��
	RouteInfInit(0);						//FLASH���е���������ʼ������
	AT24CXX_Init( );						//��ʼ��IIC
#if IWDEN
	IWDG_Init(3,625);						//�������Ź���ʼ��,��Ƶ��prer=3,��װֵrlr=625,���ʱ��Tout=((4*2^prer)*rlr)/40 (ms)=500 (ms)
											//�ڶ�ʱ��3��,ÿ��250msι��һ��IWDG_Feed(),�Է�ϵͳ��λ
#endif
	uart3_init(36,19200);
	HMI_Init();									//HMI�ӿڳ�ʼ��
	Wireless_Init();						//����ģ��(����5)��ʼ��,19200,8+n+1,
	ID_Init();									//ID����ʼ��
	SpeechCtr_Init();						//������������ʼ��
	TIM2_Int_Init(49,7199);					//10Khz�ļ���Ƶ��,����50��Ϊ5ms
	TIM5_Int_Init(49,7199);					//10Khz�ļ���Ƶ��,����50��Ϊ5ms
															//ͬʱ����ά����ʹ���5����һ֡�������ݵĳ�ʱ���,����4ms����Ϊһ֡���ݽ������
	TIM6_Int_Init(49,7199);					//10Khz�ļ���Ƶ��,����50��Ϊ5ms
	delay_ms(10);
#if IWDEN
	IWDT_En( );									//�������Ź�ʹ��
#endif
	USART1_En( );								//USART1 ʹ��
	USART4_En( );								//USART4 ʹ��
	USART5_En( );								//USART5 ʹ��
	ReStartTaskDeal( );					//������������,������ǰ������,���������

	while(1)
	{
		/*	==============================	*/
		/*			���յ���������			*/
		/*	==============================	*/
		if( CommandReceiveStatus==OK )
		{
			switch( RouteMsg1.command )
			{
				case Com_Send://�Ͳ�ָ��
					
					/* ��ȡ�����Ĳ������С�ģʽ */
					//GetVoicePlayMsg();
					Init_Station_mode();
					ID_Last=0;
					Turn_132_FLag=0;
					/*	���������״̬�½�������	*/
					if( Task.taskstatus == TaskNo )
					{
						delay_ms(1000);												//�ȴ�
						delay_ms(1000);
						/*	��ʼ����ؼĴ�������,����־λ	*/
						WriteTaskStatus( TaskSending );								//�����������
						/* �ٶ�����	*/
						SpeedSet(RouteMsg_SpeedModGet(), &SpeedSet_HV);
						
						switch(RouteMsg1.Station_DIR)
						{
							case 0:break;
							case 1: Turn90Dir = LEFT;
									STOP_L	=	STOP_R	= 0;					//ʹ��������
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
									STOP_L	=	STOP_R	= 1;					//��������ֹ			
								break;
							case 2: Turn90Dir = RIGHT;
									STOP_L	=	STOP_R	= 0;					//ʹ��������
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
									STOP_L	=	STOP_R	= 1;					//��������ֹ		
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
						ID =0;														//��ID��,��ֹ�����Ͳ�����֮ǰ�ƶ����������ö�����Ӧ�ر굼�·�������Ͳ͵���
						RunFlag = RUNFLAG_RUNING;									//����ǰ��״̬��־
						StopIDStatusFlag = STOPIDTIMING;							//����ֹͣ��־����Ч��ʱ��־,16s��ʱ������ֹͣ����Ч,��ֹ��������ֹͣ
						Turn180_132_Flag = TURN180_132NO;							//180��ת��132�ŵر�δ����
						RouteLFlag	= RouteValid;									//·����ת�����Ч
						RouteRFlag	= RouteValid;									//·����ת�����Ч
					}
					BarrierFlag_last=NoBarrier;

					
					break;
				case Com_Stop://ָֹͣ��
					/*	����ֹͣ״̬ ��ֹͣǰ��	*/
					if( RunFlag == RUNFLAG_RUNING )
					{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
						StopIDStatusFlag = STOPIDVALID;
						if( RunModBuf == ROUTESEL ){RunMod = RunModBuf; RunModBuf = NULL;}  //У��ֹͣǰ״̬�Ƿ�Ϊ ROUTESEL,ȷ��ROUTESEL����
						WriteTaskStatus( TaskNo );											//��������״̬ΪTaskSendEnd
						RouteMsg.route_l=RouteMsg.route_r=NULL;								//���·������
						

					break;
				default:
					break;
			}
			Task.command = Com_NULL;
			CommandReceiveStatus = NULL;
		}
		/*	==============================	*/
		/*				��������				*/
		/*	==============================	*/
		if( (IDNumReceiveStatus==OK) && (Task.taskstatus!=TaskNo) )
		{
			if(RunFlag == RUNFLAG_RUNING)
			{
				/* ����ID����,��ȡ�ÿ�������(����㡢���š���ʼ�����������������������־�����ȥ�͵�) */
				if( ID > 0 && ID <= 20 && Turn_132_FLag==0 )//������
				{
						/* ����һ���µĲ���IDʱ,ִ��  ��ֹ�ظ���ȡ �ظ����� */
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
				}else if( ID == 132 )//180��ת���
				{
						Turn_132_FLag=1;
						if( Turn180_132_Flag == TURN180_132NO )//�������õ�һ��ʱ���ָ���Ч״̬��
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
					/*	����ֹͣ״̬ ��ֹͣǰ��	*/
					if( RunFlag == RUNFLAG_RUNING )
					{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
					
						switch(RouteMsg1.Station_DIR)
						{
							case 0:break;
							case 1: SendStatus=SENDING;	
									Turn90Dir = RIGHT;
									STOP_L	=	STOP_R	= 0;					//ʹ��������
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
									STOP_L	=	STOP_R	= 1;					//��������ֹ			
								break;
							case 2: SendStatus=SENDING;	
									Turn90Dir = LEFT;
									STOP_L	=	STOP_R	= 0;					//ʹ��������
									RunMod=TURN_90;
									RunStatus=GOING;
									while(RunStatus!=END);					//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
									STOP_L	=	STOP_R	= 1;					//��������ֹ		
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
						if( RunModBuf == ROUTESEL ){RunMod = RunModBuf; RunModBuf = NULL;}//У��ֹͣǰ״̬�Ƿ�Ϊ ROUTESEL,ȷ��ROUTESEL����
						WriteTaskStatus( TaskNo );					//��������״̬ΪTaskSendEnd
						RouteMsg.route_l=RouteMsg.route_r=NULL;	//���·������
				}
				else if(ID>100&&ID<=110)											//�ֲ��
				{
					
					if(RouteMsg1.Cross[ID-101]==0)									//��ת
					{
						RobotRunTypeCross(LEFT);
					}
					if(RouteMsg1.Cross[ID-101]==1)
					{
						RobotRunTypeCross(RIGHT);									//��ת
					}
				}
				
				ID_Last =ID;
			}
			/* ���־λ */
			ID = 0;
			IDNumReceiveStatus=NULL;//��ձ�־λ
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
	�������ƣ�SpeedSet
	��		�ܣ��ٶ�����
	��		�Σ���
	��		ֵ����
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
	�������ƣ�unsigned char GetVoicePlayMsg(void)
	��		�ܣ���ȡ�����Ĳ������С�ģʽ
	��		�Σ���
	��		ֵ��TRUE�� ��ȡ�ɹ�
						FLASE����ȡʧ��
*********************************************/
unsigned char  GetVoicePlayMsg(void)
{
		unsigned char tempdata;
		unsigned char index=0;
		/* ��ȡ1-10����������Ϣ */
		for(tempdata=1;tempdata<=10;tempdata++)
		{
				VoiceMsg[tempdata].time 	= VoiceTimeRead(tempdata);
				VoiceMsg[tempdata].mod  	= VoiceModRead(tempdata);
				VoiceMsg[tempdata].runmod = VoiceRunModRead(tempdata);
		}
		/* ��ȡ������ģʽ	*/
		RobotMod=RobotModTrackRead();
		
		if(ROBOTMOD_TRACKLESS==RobotMod)//�޹�ģʽ
		{
				/* ���û������޹�ģʽ���޹���� */
				VoicePlayMod = VOICEMOD_TRACKLESSLINE;
				//TracklessDistance = (unsigned long)TracklessDistRead( )*TracklessDistQ*1000;//mm
		}else{//�й�ģʽ
				/* ��ȡ1-10����������Ϣ */
				tempdata=1;
				while((VoiceMsg[tempdata].mod==VOICEMOD_NULL)&&(tempdata<=10)) tempdata++;
				
				if(tempdata<=10){
						VoicePlayMod = VoiceMsg[tempdata].mod;
						switch(VoicePlayMod)
						{
								/* �����㲥ģʽ �����б��е�һ������ */
								case VOICEMOD_Single:
									VoicePlayNumList[0]=0;
									break;
								
								/* ����ѭ��ģʽ �����б���ȫ������ */
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
								
									/* �޹�ֱ������ģʽ  */
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
	�������ƣ�void VoiceDeal(void)
	��		�ܣ����������ֲ���ģʽ����
	��		�Σ���
	��		ֵ����
*********************************************/
void VoiceDeal(void)
{
		static unsigned char index=0;//�����б�����
		switch(VoicePlayMod)
		{
				/* ֹͣ�������� */
				case VOICEMOD_NULL:
					break;
				
				/* �����㲥ģʽ �����б��е�һ������ */
				case VOICEMOD_Single:
					break;
				
				/* ����ѭ��ģʽ �����б���ȫ������ */
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
							index++;//ָ���б��е���һ������
							if(index>10) index=0;
					}
					break;
				
				/* �޹�ֱ������ģʽ  */
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
	�������ƣ�void VoiceStop(void)
	��		�ܣ�ֹͣ��������
	��		�Σ���
	��		ֵ����
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
	�������ƣ�void ReStartTaskDeal( void )
	��		�ܣ�������������,������ǰ������,���������
	��		�Σ���
	��		ֵ����
*********************************************/
void ReStartTaskDeal( void )
{
	#ifdef	RESTARTCONTINUE
		/* ����������			*/
		Task.ctrareanum=ReadCtrAreaNum( );
		if( (Task.ctrareanum>=CtrArea_1) && (Task.ctrareanum<=CtrArea_5) )
		{
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );
		}
		/* ���š�������	*/
		Task.tasktable=ReadTaskTable( );
		Task.taskstatus=ReadTaskStatus( );
		if( (Task.tasktable>0)&&(Task.tasktable<100) &&
			 ((Task.taskstatus==TaskSending)||(Task.taskstatus==TaskSendEnd)||(Task.taskstatus==TaskOver)) )
		{
			/*	��ȡ·����Ϣ	*/
			RouteMsg.robotid = RobotIDRead( );
			RouteMsg.tabledir= TableDIRRead( Task.tasktable );
			RouteMsg.route_l = RouteLeftRead( Task.tasktable ) - 100;
			RouteMsg.route_r = RouteRightRead( Task.tasktable );
			if(RouteMsg.tabledir == NULL) { RouteMsg.tabledir = MID; }
			// ��������,����ǰ��(��������ر�־λ)
			RobotRunTypeStart( SpeedSet_HV );
			RunFlag = RUNFLAG_RUNING;					//����ǰ��״̬��־
			StopIDStatusFlag = STOPIDTIMING;	//����ֹͣ��־����Ч��ʱ��־,2s��ʱ������ֹͣ����Ч,��ֹ��������ֹͣ
			Turn180_132_Flag = TURN180_132NO;	//180��ת��132�ŵر�δ����
		}else{
			WriteTaskTable( 0 );
			WriteTaskStatus( TaskNo );
		}
	#else
		/*������·���ݴ����*/
		WriteContrCommand(Com_NULL);	//�����ն˿�������Ϊ��
		WriteTaskTable(0);						//����Ŀ������Ϊ��
		WriteTaskStatus(TaskNo);			//����������״̬
		WriteCtrAreaNum(0);						//���ù��������Ϊ��
		Task.ctrareastatus=CtrArea_Idle;//���ù�����״̬����
	#endif
}
/*********************************************
	�������ƣ�void SetCtrAreaStatus( unsigned char ctrareanum, unsigned char status )
	��		�ܣ����ն˿��������ù�����״̬
	��		�Σ�ctrareanum�����������
						status		��״̬
												CtrArea_Idle ��������״̬
												CtrArea_Busy ������æ״̬
	��		ֵ��0:�ȴ���ʱ
						1:���óɹ�
*********************************************/
unsigned char SetCtrAreaStatus( unsigned char ctrareanum, unsigned char ctrareastatus )
{
	unsigned short crc16=0;
	unsigned long	 count=0x00200000;//�ȴ�Լ500ms
	GetCtrAreaStatusFlag = 0;
	USART5_TX_BUF[0]=RouteMsg.robotid;//�����˵�ַ
	USART5_TX_BUF[1]=Com_WCAS;				//������
	USART5_TX_BUF[2]=ctrareanum;			//���������
	USART5_TX_BUF[3]=ctrareastatus;		//������״̬
	crc16 = CRC16(USART5_TX_BUF,4);
	USART5_TX_BUF[4]=crc16;
	USART5_TX_BUF[5]=crc16>>8;
	USART5_Send(USART5_TX_BUF,6);			//���͸���λ��
	Task.ctrareastatus = ctrareastatus;
	while(!GetCtrAreaStatusFlag)			//�ȴ����յ������ն˷��ص�״̬
	{ count--; if(count==0)return (0); }//�ȴ���ʱ
	GetCtrAreaStatusFlag = 0;
	return (1);
}
/****************************************************
	�������ƣ�void GetCtrAreaStatus( unsigned char ctrareanum )
	��		�ܣ����ն˿�������ȡ������״̬
	��		�Σ�ctrareanum�����������
	��		ֵ��0:�ȴ���ʱ
						1:�ɹ���ȡ״̬
****************************************************/
unsigned char GetCtrAreaStatus( unsigned char ctrareanum )
{
	unsigned short crc16=0;
	unsigned long	 count=0x00200000;//�ȴ�Լ500ms
	GetCtrAreaStatusFlag = 0;
	USART5_TX_BUF[0]=RouteMsg.robotid;//�����˵�ַ
	USART5_TX_BUF[1]=Com_RCAS;				//������ ��������״̬
	USART5_TX_BUF[2]=ctrareanum;			//���������
	crc16 = CRC16(USART5_TX_BUF,3);		
	USART5_TX_BUF[3]=crc16;						//
	USART5_TX_BUF[4]=crc16>>8;				//
	USART5_Send(USART5_TX_BUF,5);			//���͸���λ��
	while(!GetCtrAreaStatusFlag)			//�ȴ����յ������ն˷��ص�״̬
	{ count--; if(count==0)return (0); }//�ȴ���ʱ
	GetCtrAreaStatusFlag = 0;
	return (1);
}
/****************************************************
	�������ƣ�void CtrAreaDeal( unsigned char ctrareaid )
	��		�ܣ����������״̬У��
								---��---																		---��---
						���ù�������š���״̬--->Idle--->����æ--->��æ״̬������
															 |
															 ------>Busy--->�ȴ�����--->����æ--->��æ״̬������
	��		�Σ�ctrareaid������������
	��		ֵ����
****************************************************/
void CtrAreaDeal( unsigned char ctrareaid )
{
	unsigned char status=0;
	switch( ctrareaid )
	{
		case 120://��1�Ź�����
			status = 1;
			WriteCtrAreaNum( CtrArea_1 );											//���ù��������
		break;
		case 121://��1�Ź�����
			status = 2;
		break;
		case 122://��2�Ź�����
			status = 1;
			WriteCtrAreaNum( CtrArea_2 );											//���ù��������
		break;
		case 123://��2�Ź�����
			status = 2;
		break;
		case 124://��3�Ź�����
			status = 1;
			WriteCtrAreaNum( CtrArea_3 );											//���ù��������
		break;
		case 125://��3�Ź�����
			status = 2;
		break;
		case 126://��4�Ź�����
			status = 1;
			WriteCtrAreaNum( CtrArea_4 );											//���ù��������
		break;
		case 127://��4�Ź�����
			status = 2;
		break;
		case 128://��5�Ź�����
			status = 1;
			WriteCtrAreaNum( CtrArea_5 );											//���ù��������
		break;
		case 129://��5�Ź�����
			status = 2;
		break;
		default:
		break;
	}
	if( (status==1) && (CtrAreaFlag==CtrAreaOut) )				//�����������Ҳ��ٹ�����
	{
			CtrAreaFlag = CtrAreaIn;													//����Ϊ�ѽ��������
			
			if(!GetCtrAreaStatus( Task.ctrareanum ))					//��ȡ������״̬,�ȴ���ʱ,���ٴζ�ȡһ��
			{
				if(!GetCtrAreaStatus( Task.ctrareanum ))				//�ȴ���ʱ,���ٴζ�ȡһ��
				{	GetCtrAreaStatus( Task.ctrareanum );	}				
			}
			if( Task.ctrareastatus == CtrArea_Idle )					//�������,������Ϊæ״̬
			{
				if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))	//�ȴ���ʱ,���ٴ�����һ��
				{
					if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))//�ȴ���ʱ,���ٴ�����һ��
						SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy );
				}
			}
			else{																							//���æ,��ֹͣ���ȴ�����
				/*	����ֹͣ״̬ ��ֹͣǰ��	*/
				if( RunFlag == RUNFLAG_RUNING )
				{	RunFlag = RUNFLAG_STOP;	RobotRunTypeStop( );}
				/*	�ȴ�����	*/
				while( Task.ctrareastatus == CtrArea_Busy )
				{ delay_ms(1000);delay_ms(1000);delay_ms(1000); GetCtrAreaStatus( Task.ctrareanum ); }
				/*	����Ϊæ״̬	*/
				if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))	//�ȴ���ʱ,���ٴ�����һ��
				{	
					if(!SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy ))//�ȴ���ʱ,���ٴ�����һ��
						SetCtrAreaStatus( Task.ctrareanum, CtrArea_Busy );
				}
				/*	��������	*/
				RobotRunTypeStart( SpeedSet_HV );
				RunFlag = RUNFLAG_RUNING;												//�ڱ��Ϻ����л���������
			}
	}else if( (status==2) && (CtrAreaFlag==CtrAreaIn) )	//������������֮ǰ�ڹ�������
// 	}else if( status==2 )	//��������
	{
			CtrAreaFlag = CtrAreaOut;													//����Ϊ�ѳ�������
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );//��æ״̬
			SetCtrAreaStatus( Task.ctrareanum, CtrArea_Idle );//��æ״̬
			WriteCtrAreaNum( CtrArea_NULL );									//����
	}
}
/*********************************************
	�������ƣ�void TouchServe( void )
	��		�ܣ����������������
	��		�Σ���
	��		ֵ����
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
	�������ƣ�void BarrierDeal( void )
	��		�ܣ�����
	��		�Σ���
	��		ֵ����
*********************************************/
void BarrierDeal( void )
{
			static unsigned char barriercount=0;
			static unsigned char barrrier_pre=BarrierSta_No;
			static unsigned char runmod_pre;
			
			if(RunFlag != RUNFLAG_RUNING) return;
		
			if(RouteMsg1.barrierstatus==BarrierSta_No)		//���ϰ���,�����н�
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
			}else if(RouteMsg1.barrierstatus==BarrierSta_DecSpeed)		//��Զ�����ϰ���,�����н�
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
			}else if(RouteMsg1.barrierstatus==BarrierSta_NotMan || RouteMsg1.barrierstatus==BarrierSta_Man)//�������ϰ���
			{
					barriercount++;
					if(barriercount>=10){
							barriercount=10;
							
							if(RunMod)
							{
								runmod_pre = RunMod;
								RouteMsg_VoiceNumSet(BarrierVoice);
							}
							
							RobotRunTypeBreak( );					//����ɲ����ע��:Break��,SpeedNormal��ȻΪ����н�ʱ���ٶ�ֵ��
							barrrier_pre=RouteMsg1.barrierstatus;
				}
			}
}
/*********************************************
	�������ƣ�void BarrierServe( void )
	��		�ܣ������ϰ�����Ľ������Ӧ��־λ
	��		�Σ���
	��		ֵ����
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
	�������ƣ�void BarrierServe_Trackless( void )
	��		�ܣ������ϰ�����Ľ������Ӧ��־λ
	��		�Σ���
	��		ֵ����
*********************************************/
void BarrierServe_Trackless( void )
{
		BarrierScan( );

		switch(TracklessCountFlag)
		{
				case Tracklessing ://��������
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
	�������ƣ�void BarrierDetect( void )
	��		�ܣ����ϼ��,����Ƿ����ϰ���,��ִ����ر��ϲ���
	��		�Σ���
	��		ֵ����
*********************************************/
void BarrierDetect( void )
{
			static unsigned char barriercount=0;
			
			if(BarrierFlag == NoBarrier)		//���ϰ���,�����н�
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
										else													//��������������,������ʻ
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case NearBarrier:
									VoicePlay(NULL);							//ֹͣѭ�������ϰ�����
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	RobotRunTypeStart( SpeedSet_HV );	}
										else													//��������������,������ʻ
										{	RobotRunTypeStart( SpeedSet_BLV );}
										
										if((TracklessCountFlag==TracklessTurn180)&&(RunStatus==GOING))
											RunMod = TURN_180_Trackless;
									}
									break;
									default:
										break;
						}
						BarrierFlag_last=NoBarrier;
			}else if(BarrierFlag == FarBarrier)		//��Զ�����ϰ���,�����н�
			{
						barriercount=0;
						
						switch(BarrierFlag_last)
						{
								case NoBarrier:
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	SpeedNormal= SpeedSet_LV ;	}
										else													//��������������,������ʻ
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case FarBarrier:
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	SpeedNormal= SpeedSet_LV ;	}
										else													//��������������,������ʻ
										{	SpeedNormal= SpeedSet_BLV ;}
									}
									break;
								case NearBarrier:
									VoicePlay(NULL);							//ֹͣѭ�������ϰ�����
									if(RunFlag == RUNFLAG_RUNING)
									{
										if(Task.taskstatus!=TaskOver)
										{	RobotRunTypeStart( SpeedSet_LV );	}
										else													//��������������,������ʻ
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
										RobotRunTypeBreak( );					//����ɲ����ע��:Break��,SpeedNormal��ȻΪ����н�ʱ���ٶ�ֵ��
									break;
								case FarBarrier:
									if(RunFlag == RUNFLAG_RUNING)
										RobotRunTypeBreak( );					//����ɲ����ע��:Break��,SpeedNormal��ȻΪ����н�ʱ���ٶ�ֵ��
									break;
								case NearBarrier:
									if(RunFlag == RUNFLAG_RUNING)
										RobotRunTypeBreak( );					//����ɲ����ע��:Break��,SpeedNormal��ȻΪ����н�ʱ���ٶ�ֵ��
									break;
								default:
										break;
						}
//						/*	���ű�������	*/
//						if(BarrierEn==ENABLE)
//						{
//#if BARRIERPLAYCONTEN
//								if(0==VoicePlay_Flag) VoicePlay(BarrierVoice);//�����ϰ�����
//#else
//								if(BarrierFlag_last!=NearBarrier) VoicePlay(BarrierVoice);//�����ϰ�����
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
	�������ƣ�void TIM3_IRQHandler(void)
	��		�ܣ���ʱ��3�жϷ������,5ms�ж�.
						ÿ5ms��ȡһ�γ���λ��,��ͨ��PID�����㷨�������ٽ��н�������״̬(4��״̬)
														----�ع켣����----		 ----Ѱ��----
														�ȼ��ٺ�����	����	�ȼ��ٺ�����	����
						SearchLineFlag	 		0		 				0				1		 				1
						LostLineFlag		 		0		 				1				1		 				0	
	��		�Σ���
	��		�أ���
*********************************************/
void TIM3_IRQHandler(void)
{
	unsigned int  temp=0;
	static unsigned int timer3count=0;
	static unsigned short StopIDcount=0;
	if(TIM3->SR&0X0001)//����ж�
	{
		timer3count++;
		TIM3->SR&=~(1<<0);//����жϱ�־λ
#if IWDEN
		IWDG_Feed();//ι��
#endif
		ST = ~ST;		//��λ�ⲿ���Ź�
		//==============================================================
		/*	����ŷ����� 5ms	*/
		if(Task.taskstatus!=TaskNo)//5ms,��������
		{
			switch(RunMod)
			{
					case NORMAL ://����Ѱ������
						MotorDriverCtrPID();					//��Ҫ0.1ms
						break;
					case STOP   ://ֹͣ����
						MotorDriverStop();						//ͣ��
						break;
					case TURN_90://ת��
						MotorDriverTurn_90Angle(Turn90Dir,SendStatus);//����Turn90Dir��SendStatus����ת�����
						break;
					case ROUTESEL://·��ѡ��
						MotorDriverRouteSel();				//·��ѡ��
						break;
					case TURN_180://ԭ��ת��180��
						MotorDriverTurn_180Angle(Turn180Dir,Turn180Mod);//����Turn180Dir��Turn180Mod����ԭ��ת��180�����
						break;
					default			:
						break;
			}
		}

		/*	���ϼ��	*/
// 		if(Task.taskstatus!=TaskNo)
// 		{
//				BarrierServe();
//				
//				BarrierDetect();	//����������ʱִ�б��ϲ���
// 		}
		
		
		BarrierDeal();				//���ϴ���
		/*	����״̬�·����ж�	*/
		BarrierDirDetect( );
		/*	ת��ʱ���ʱ	*/
		if(TurnTimeCountFlag)
		{
			TurnTimeCount++;
			if(TurnTimeCount>=2000)	TurnTimeCount=2000;
		}
		//==============================================================
		if(0==timer3count%8)//40ms
		{
			/*	����ٶ� �ȼӼ��ٿ���	*/
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
				/*	ֹͣ����Ч16s��ʱ	*/
				if( StopIDStatusFlag == STOPIDTIMING )
				{
					StopIDcount ++;
					if( StopIDcount >= 160 )//16s��ʱ
					{	StopIDcount = 0; StopIDStatusFlag = STOPIDVALID;	}
				}
				
				/*	180��ر���Ч14s��ʱ	*/
				if( Turn180_132_Flag == TURN180_132END )
				{
						Turn180TimeCount++;
						if( Turn180TimeCount>200 )
						{ Turn180TimeCount = 0; Turn180_132_Flag = TURN180_132NO; }
				}
				HumanBodyDetect(&RouteMsg1.barrierstatus);		//������
				WelcomeVoiceDeal();								//��ӭ����

				
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
	ͨ�ö�ʱ��3�жϳ�ʼ��
	����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36MHz
	arr���Զ���װֵ��
	psc��ʱ��Ԥ��Ƶ��
*********************************************/
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3ʱ��ʹ��    
 	TIM3->ARR=arr;  		//�趨�������Զ���װֵ//�պ�1ms    
	TIM3->PSC=psc;  		//Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��
	TIM3->DIER|=1<<0;   //��������ж�
	TIM3->CR1|=0x01;    //ʹ�ܶ�ʱ��3
  MY_NVIC_Init(3,2,TIM3_IRQn,2);//��ռ3,�����ȼ�2,��2
}
/*********************************************
	ʹ��ͨ�ö�ʱ��3
*********************************************/
void TIM3_En( void )
{
	TIM3->CR1|=0x01;    //ʹ�ܶ�ʱ��3
}
/*****************************
	�������ƣ�void WD_Init( void )
	��		�ܣ��ⲿ���Ź���ʼ��,ST��������
	��		�Σ���
	��		�أ���
TD->GND:150ms
*****************************/
void WD_Init( void )
{
	RCC->APB2ENR|=1<<7;//��ʹ������IO PORTFʱ��
	GPIOF->CRH&=0XFFFFF0FF;	//PF10�������
	GPIOF->CRH|=0X00000300;
	ST = 1;
}
/*****************************
	�������ƣ�void GuidSensorPort_Init( void )
	��		�ܣ��ŵ��򴫸����˿ڳ�ʼ��
	��		�Σ���
	��		�أ���
*****************************/
void GuidSensorPort_Init( void )
{
	RCC->APB2ENR|=1<<3; 		//ʹ�� PORTB ʱ��
	RCC->APB2ENR|=1<<5; 		//ʹ�� PORTD ʱ��
	RCC->APB2ENR|=1<<8; 		//ʹ�� PORTG ʱ��
	
	GPIOB->CRH&=0X0FFFFFFF;	//PB 15 ��������
	GPIOB->CRH|=0X80000000;
	GPIOB->ODR|=1<<15;
	
	GPIOD->CRH&=0X00000000;	//PD8 15 ��������
	GPIOD->CRH|=0X88888888;
	GPIOD->ODR|=0XFF<<8;
	
	GPIOG->CRL&=0X000000FF;	//PG2 8 ��������
	GPIOG->CRH&=0XFFFFFFF0;
	GPIOG->CRL|=0X88888800;
	GPIOG->CRH|=0X00000008;
	GPIOG->ODR|=0X1FC;
}
/*********************************************
	�������ƣ�char GetPos(void)
	��		�ܣ���ȡ�յ���������ֵ,����ȡ��ǰ��λ����Ϣ
															��						 |						��
						 D16	D15	D14	D13	D12	D11	D10	D9 | D8	D7	D6	D5	D4	D3	D2	D1
						 16		15	14	13	12	11	10	9		 8	7		6		5		4		3		2		1
						������Ҫ����ȷ�����²�����ֵ,�����ڱ�����ǰλ��״̬
						pos				:��ǰ��λ��
						DirFlag		:�����־,1(��ƫ),	0,			-1(��ƫ)
	��		�Σ���
	��		ֵ��char pos:��ǰλ����Ϣ15~-15
*********************************************/
const unsigned char GuideSensorDir	=0;		//���򴫸�����װλ�ñ�־. 0:��װ(16��-1��); 1:��װ(1��-16��)
signed 	 char DirFlag								=0;		//�����־λ,1:��ƫ;0:�м�(��ƫ);-1:��ƫ
unsigned char DirCountL							=0;	 	//��ƫ����
unsigned char DirCountM							=0;	 	//0 ƫ����
unsigned char DirCountR							=0;	 	//��ƫ����
unsigned char DirCountThreshold			=5;	 	//ƫ�������ֵ
unsigned char LostLineFlag					=0;  	//���߱�־,δ��⵽�����·
unsigned char LostLineCount					=0;	 	//���߼���
unsigned char NoLostLineCount				=0;  	//�Ƕ��߼���
unsigned char LostLineCountThreshold=50;	//С�����ߺ�Ѳ�߼�����ֵ,����ֵ������ֵ��Ϊ���߻�Ѱ����
#define posbufsize	30										//λ�û���������
signed	 int  pos_sum								=0;		//λ�����
signed	 char pos_buf[posbufsize]={0,0,0,0,0,0,0,0,0,0,//λ�û�����,ǰposbufsize��λ����Ϣ
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
		 D9==1 && D10==1 && D11==1 && D12==1 && D13==1 && D14==1 && D15==1 && D16==1	)//����,δ��⵽����
	{
			LostLineCount++;//���߼�����1
			NoLostLineCount=0;
			if(LostLineCount>=LostLineCountThreshold)//���ߴ���������ֵLAndSLineCountThresholdȷ��Ϊ����,�ö��߱�־λ
			{
				LostLineFlag=1;
				LostLineCount=LostLineCountThreshold;
			}
	}else//δ����,��⵽����
	{
		if( D5 ==0 || D6 ==0 || D7 ==0 || D8 ==0 || D9==0 || D10==0 || D11==0 || D12==0 )
		NoLostLineCount++;
		LostLineCount=0;
		if(NoLostLineCount>=LostLineCountThreshold)//�Ƕ����ߴ���������ֵLAndSLineCountThresholdȷ��Ϊδ����,�嶪�߱�־λ
		{
			LostLineFlag=0;
			NoLostLineCount=LostLineCountThreshold;
			if( RunMod == NORMAL )//����Ѱ��λ�ü���
			{
					/*	�������ĵ�ƫ����pos	*/
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
						pos	=	( signed char )((tempsum*1.0/tempcount-8.5)*2);//���㴫�������е�ƫ����, �е�ֵΪ8.5
						pos	=	( GuideSensorDir ? -pos : pos );
						/* ����λ�û�����	*/
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
// 					if( i<=16 )			//���ڵ�һ����Ч����
// 					{
// 						tempcount=0;
// 						while( (D[i]==0) && (i<=16) ) { pos_1+=i;	tempcount++;	i++;}
// 						pos_1	=	( signed char )((pos_1*1.0/tempcount-8.5)*2);
// 						pos_1	=	( GuideSensorDir ? -pos_1 : pos_1 );
// 						if( i<=16 )
// 						{
// 								while( D[i]==1 ) i++;
// 								if( i<=16 )//���ڵڶ�����Ч����
// 								{
// 									tempcount=0;
// 									while( (D[i]==0) && (i<=16) ) { pos_2+=i;	tempcount++;	i++;}
// 									pos_2	=	( signed char )((pos_2*1.0/tempcount-8.5)*2);
// 									pos_2	=	( GuideSensorDir ? -pos_2 : pos_2 );
// 									pos = (fabs(pos-pos_1)-fabs(pos-pos_2)) > 0 ? pos_2 : pos_1;
// 								}else			//�ڶ�����Ч���򲻴���
// 								{
// 									pos = pos_1;
// 								}
// 						}
// 					}else						//��һ����Ч���򲻴���,������
// 					{
// 						pos = 0;
// 					}

			}else if( RunMod == ROUTESEL )//ת��㴦λ�ü���
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
// 								pos = ( signed char )((pos-11)*2);//���㴫�������е�ƫ����, �е�ֵΪ11
								pos = ( signed char )((pos-9.5)*2);//���㴫�������е�ƫ����, �е�ֵΪ10
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
// 								pos = ( signed char )((6-pos)*2);//���㴫�������е�ƫ����, �е�ֵΪ6
								pos = ( signed char )((7.5-pos)*2);//���㴫�������е�ƫ����, �е�ֵΪ7
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
// 								pos = ( signed char )((pos-6)*2);//���㴫�������е�ƫ����, �е�ֵΪ6
								pos = ( signed char )((pos-7.5)*2);//���㴫�������е�ƫ����, �е�ֵΪ7
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
// 								pos = ( signed char )((11-pos)*2);//���㴫�������е�ƫ����, �е�ֵΪ11
								pos = ( signed char )((9.5-pos)*2);//���㴫�������е�ƫ����, �е�ֵΪ10
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
								pos	=	( signed char )((tempsum*1.0/tempcount-8.5)*2);//���㴫�������е�ƫ����, �е�ֵΪ8.5
								pos	=	( GuideSensorDir ? -pos : pos );
							}else
							{
								pos = 0;
							}
							break;
					}
			}
			
			/*	ƫ�뷽���ж�	*/
			if(pos==0)
			{
				DirCountM++;//0ƫ������1
				DirCountL=0;//������ƫ����
				DirCountR=0;
				if(DirCountM>=DirCountThreshold)
				{
					DirFlag=0;										//����Ϊ�м�
					DirCountM=DirCountThreshold;	//�����
				}
			}else{
				if(pos<0)//��ƫ
				{
					DirCountL++;//��ƫ������1
					DirCountM=0;////��0����ƫ����
					DirCountR=0;
					if(DirCountL>=DirCountThreshold)
					{	
						DirFlag=1;										//����Ϊ��ƫ
						DirCountL=DirCountThreshold;	//�����
					}
				}else//��ƫ
				{
					DirCountR++;//��ƫ������1
					DirCountM=0;//��0����ƫ����
					DirCountL=0;
					
					if(DirCountR>=DirCountThreshold)
					{
						DirFlag=-1;										//����Ϊ��ƫ
						DirCountR=DirCountThreshold;	//�����
					}
				}
			}
		}
	}
	return pos;
}
/*********************************************
	�������ƣ�void BarrierDirDetect( void )
	��		�ܣ�����ʱƫ�뷽����, �Է����ϼ��ٶ���ʱ, Ѱ�߷������
	��		�Σ���
	��		ֵ����
*********************************************/
void BarrierDirDetect( void )
{
	if( (GUIDESENSOR_D1 ==0)||(GUIDESENSOR_D2 ==0)||(GUIDESENSOR_D3 ==0) )
	{	DirFlag = (GuideSensorDir ?-1 : 1);	}				//����Ϊ��ƫ
	if( (GUIDESENSOR_D14==0)||(GUIDESENSOR_D15==0)||(GUIDESENSOR_D16==0) )
	{	DirFlag = (GuideSensorDir ? 1 :-1); }				//����Ϊ��ƫ
}
/*********************************************
	�������ƣ�unsigned char LineDetect_Centre( void )
	��		�ܣ�ת������Ѱ��,���ĵ�7 8 9 10�����ϼ�Ϊ��Ч
	��		�Σ���
	��		ֵ��1��δ��⵽�ߣ�0����⵽��
*********************************************/
unsigned char LineDetect_Centre( void )
{
	if((GUIDESENSOR_D7==1)&&(GUIDESENSOR_D8==1)&&(GUIDESENSOR_D9==1)&&(GUIDESENSOR_D10==1))
	{	return ((unsigned char)1);	}/*	δ��⵽��	*/
	else
	{	return ((unsigned char)0);	}/*	  ��⵽��	*/
}
/*********************************************
	�������ƣ�unsigned char LineDetect_All( void )
	��		�ܣ�ת��Ѱ��,�������ϵĵ㼴Ϊ��Ч
	��		�Σ���
	��		ֵ��1��δ��⵽�ߣ�0����⵽��
*********************************************/
unsigned char LineDetect_All( void )
{
	if((GUIDESENSOR_D1 ==1)&&(GUIDESENSOR_D2 ==1)&&(GUIDESENSOR_D3 ==1)&&(GUIDESENSOR_D4 ==1)&&
		 (GUIDESENSOR_D5 ==1)&&(GUIDESENSOR_D6 ==1)&&(GUIDESENSOR_D7 ==1)&&(GUIDESENSOR_D8 ==1)&&
		 (GUIDESENSOR_D9 ==1)&&(GUIDESENSOR_D10==1)&&(GUIDESENSOR_D11==1)&&(GUIDESENSOR_D12==1)&&
		 (GUIDESENSOR_D13==1)&&(GUIDESENSOR_D14==1)&&(GUIDESENSOR_D15==1)&&(GUIDESENSOR_D16==1)  )
	{	return ((unsigned char)1);	}/*	δ��⵽��	*/
	else
	{	return ((unsigned char)0);	}/*	  ��⵽��	*/
}
/*********************************************
	�������ƣ�unsigned char RobotRunTypeTable( unsigned char tabledir )
	��		�ܣ���������ID��ִ���ͲͲ���,����˳��:
						STOP->TURN_90(ԭ��ת��)->�ȴ���������->TURN_90(ԭ�ط���ת��)->NORMAL
	��		�Σ���
	��		ֵ����
*********************************************/
void RobotRunTypeTable( unsigned char tabledir,unsigned char ID )
{
	BarrierEn=DISABLE;
	/*	ͣ��	*/
	RunFlag = RUNFLAG_STOP;
	RobotRunTypeStop( );//ͣ��
	delay_ms(500);
	
	/*	ת��	*/
	if( tabledir==MID )							Turn90Dir = MID;			//���ò�������
	else if( tabledir==LEFT )				Turn90Dir = LEFT;
	else if( tabledir==RIGHT )			Turn90Dir = RIGHT;
	else if( tabledir==MID_BACK )		Turn90Dir = MID;
	else if( tabledir==LEFT_BACK )	Turn90Dir = LEFT;
	else if( tabledir==RIGHT_BACK )	Turn90Dir = RIGHT;
	SendStatus=SENDING;								//����ת��״̬λΪSENDING
	if(Turn90Dir!=MID)								//��Ҫת��
	{
		STOP_L	=	STOP_R	= 0;					//ʹ��������
		RunMod=TURN_90;
		RunStatus=GOING;
		while(RunStatus!=END);						//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
		STOP_L	=	STOP_R	= 1;					//��������ֹ
	}
	delay_ms(100);
	
	/*	��������			*/
	RouteMsg_VoiceNumSet(ID+12);
	
	/*	�ȴ���������ȴ�������ϣ�������Ըĳɵȴ�ָ��	*/
	while(Voice_ok!=RouteMsg1.Station_tim[ID-1])
	{
		delay_ms(1000);
		Voice_ok++;
	}
	Voice_ok=0;

	delay_ms(10);			//�ȴ�10s
	/*	����\ͬ��ת��(Ѱ��)	*/
	if( tabledir == MID )							Turn90Dir = MID;				//���ݲ�����������ת�巽��Ϊ����
	else if( tabledir == LEFT )				Turn90Dir = RIGHT;					
	else if( tabledir == RIGHT )			Turn90Dir = LEFT;
	else if( tabledir == LEFT_BACK )	Turn90Dir = LEFT;			//���ݲ�����������ת�巽��Ϊͬ��
	else if( tabledir == RIGHT_BACK )	Turn90Dir = RIGHT;
	
	SendStatus=SENDBACK;							//����ת��״̬λΪSENDBACK
	STOP_L	=	STOP_R	= 0;						//ʹ��������
	turn_90_180_status=turn_90_180_status_NULL;
	if( tabledir == MID_BACK )				//ת��180���ֱ��ǰ��
	{	RobotRunTypeTurn180( LEFT, TURN180_CONTINUE );	}
	else if(Turn90Dir!=MID)						//ת��90��ǰ��
	{
		RunMod=TURN_90;
		RunStatus=GOING;
		while(RunStatus!=END);					//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
	}
	SendStatus=NULL;									//����ת��״̬λΪNULL
	
	if(turn_90_180_status==turn_90_180_status_lostline)
		turn_90_180_status=turn_90_180_status_NULL;
	else{
		/*	��������(����PID����)	*/
		RobotRunTypeStart( SpeedSet_HV );
		RunFlag = RUNFLAG_RUNING;
	}
	BarrierEn=ENABLE;
}
/*********************************************
	�������ƣ�void RobotRunType180Trackless( unsigned char dir )
	��		�ܣ��޹�ת��180��
	��		�Σ���
	��		ֵ����
*********************************************/
void RobotRunType180Trackless( unsigned char dir )
{
	/*	ת��	*/
	Turn180Dir = dir;									//���ò�������
	STOP_L	=	STOP_R	= 0;						//ʹ��������
	RunMod=TURN_180_Trackless;
	RunStatus=GOING;
	while(RunStatus!=END);						//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
	STOP_L	=	STOP_R	= 1;						//��������ֹ
}
/*********************************************
	�������ƣ�void RobotRunTypeTurn180( unsigned char dir, unsigned char turn180mod )
	��		�ܣ�ԭ��ת��180��
	��		�Σ�dir��				ת�巽��(LEFT��RIGHT)
						turn180mod��ת��ģʽ
												TURN180_CONTINUE(ת������ǰ��);
												TURN180_STOP(ת���ֹͣ)
	��		ֵ����
*********************************************/
void RobotRunTypeTurn180( unsigned char dir, unsigned char turn180mod )
{
	/*	ͣ��	*/
	RunFlag = RUNFLAG_STOP;
	RobotRunTypeStop( );//ͣ��
	delay_ms(500);
	
	/*	ԭ��ת��180��	*/
	Turn180Dir=dir;					//����ԭ��ת��ķ���
	Turn180Mod=turn180mod;	//����ԭ��ת���ģʽ
	STOP_L	=	STOP_R	= 0;	//ʹ��������
	RunMod=TURN_180;
	RunStatus=GOING;
	while(RunStatus!=END);	//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
	STOP_L	=	STOP_R	= 1;	//��������ֹ
	
	/*	��������(����PID����)	*/
	if(( turn180mod == TURN180_CONTINUE )&&(turn_90_180_status==turn_90_180_status_NULL))
	{	RobotRunTypeStart( SpeedSet_HV ); RunFlag = RUNFLAG_RUNING;	}
}
/*********************************************
	�������ƣ�void RobotRunTypeCross( unsigned char dir )
	��		�ܣ�����㴦·��ѡ��
	��		�Σ���
	��		ֵ����
*********************************************/
void RobotRunTypeCross( unsigned char dir )
{
	/*	ѡ��·��	*/
	RouteSelDir=dir;		 						//����·������		<<��������,�д�����>>
	DIRBuf=RouteSelDir;
	RunMod=ROUTESEL;
	RunStatus=GOING;
}
/*********************************************
	�������ƣ�void RobotRunTypeStop( void )
	��		�ܣ�ͣ��
	��		�Σ���
	��		ֵ����
*********************************************/
void RobotRunTypeStop( void )
{
	/*	����ͣ��	*/
	RunModBuf = RunMod;		//��¼ֹͣǰ��ģʽ,������ʱУ���Ƿ�Ϊ ROUTESE ģʽ.
	RunMod=STOP;
	RunStatus=GOING;
	while(RunStatus!=END);//�ȴ�ִ�����,����ִ���ڶ�ʱ��3��
	STOP_L	=	STOP_R	= 1;//��������ֹ
	STOP_L	=	STOP_R	= 1;//��������ֹ
}
/*********************************************
	�������ƣ�void RobotRunTypeBreak( void )
	��		�ܣ�����ɲ��
	��		�Σ���
	��		ֵ����
*********************************************/
void RobotRunTypeBreak( void )
{
	/*	����ɲ��	*/
	RunMod=NULL;	//����˶�ģʽ
// 	RunStatus=END;//ʵ���ٶȼ�����0,ͣ������
	SpeedTarget	= SpeedReal = 0;
	MotorDriverVClControl(SpeedReal,SpeedReal);
	STOP_L	=	STOP_R	= 1;//��������ֹ
	STOP_L	=	STOP_R	= 1;//��������ֹ
}
/*********************************************
	�������ƣ�void RobotRunTypeStart( signed int runspeed )
	��		�ܣ�ʹ��������, ��������, ��ʼѰ��ǰ��
	��		�Σ�runspeed�������ٶȣ�SpeedSet_HV��SpeedSet_LV
	��		ֵ����
*********************************************/
void RobotRunTypeStart( signed int runspeed )
{
	SpeedNormal = runspeed;		//�趨�ٶȻ�����SpeedSet_HV��SpeedSet_LV
	STOP_L	=	STOP_R	= 0;		//ʹ��������
	STOP_L	=	STOP_R	= 0;		//ʹ��������

	RunMod=NORMAL;						//����
}
/*********************************************
	�������ƣ�void MotorDriverCtrPID( void )(Timer3��ÿ20ms����һ��)
	��		�ܣ�ʵ������ģʽ�µ��˶�����(����PI����)
						===					ͨ������RunMod=TURN_90ִ�иò���			===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===					����ִ����Ϻ�,����RunMod=NULL				===
	��		�Σ���
	��		ֵ����
*********************************************/
unsigned char SearchLineFlag				=0;	 		//Ѱ�߱�־,���ڽ���Ѱ�߲���
const unsigned  int SearchLineTime	=1000;		//Ѱ��ʱ��,����ʱ����ֹͣѰ��
float k=0.7,b=4.0;//���������֮�����Ի�ϵ��
void MotorDriverCtrPID( void )
{
	static unsigned  int SearchLineCount=0;
	/*	��ȡ����״̬	*/
	E1=E0;
	E0=GetPos();
// 	USART5_SendByte((unsigned char)fabs(E0));
	if( RunMod == ROUTESEL )
		Kp = 0.8*fabs(E0)+5;
	else
		Kp = k*fabs(E0)+b;
	if(Kp>10)Kp=10;
// 	/*���ٻ���	��Ч�������ԡ�*/
// 	if(E0>EMAX*0.4)
// 		SumE+=(E0*(Ki/0.6*(1-E0/EMAX)));
// 	else
// 		if(E0<EMIN*0.4)
// 			SumE+=(E0*(Ki/0.6*(1-E0/EMIN)));
// 		else
			SumE+=E0;
		
	/*	ƫ������޷�	*/
	if(SumE>=MAXSUME)
		SumE=MAXSUME;
	if(SumE<=-MAXSUME)
		SumE=-MAXSUME;
	
	/*	�������������������	*/
	if(LostLineFlag==0)//������ʻ״̬,�ȼ������趨�ٶ�,������Ѱ������
	{
			if(SearchLineFlag==1) 
			{
				SearchLineFlag=0;
				BREAK_L	=	BREAK_R = 0;
				RunFlag = RUNFLAG_RUNING;
			}//ʹ������������������״̬
			
			if( RunMod == NORMAL )	//����Ѱ���ٶ�
			{
				#if	SPEEDDOWNEN				//�������
					if(E0>1 || E0<-1)		//ģ����-1 0 1,������ֱ������
					{
					SpeedTarget	= SpeedNormal-((unsigned char)fabs(E0)-1)*Q_e;
					if(SpeedTarget <SpeedSet_LV )
						SpeedTarget=SpeedSet_LV;
					}else
						SpeedTarget	= SpeedNormal;
				#else									//���������
					SpeedTarget	= SpeedNormal;
				#endif
			}
			else if( RunMod == ROUTESEL )			//·��ѡ��(ת��)�ٶ�
			{	SpeedTarget	= SpeedRouteSel;	}
			else if( RunMod == STOP )					//����ͣ��
			{	SpeedTarget	= 0;				 			}
		
			/*	PI����	*/
			Uk0=E0*Kp+SumE*Ki;			//�ٶ�PI����
			#if INCOMPLETEPIDEN
			Uk0=Alpha0*Uk1+Alpha1*Uk0;
			Uk1=Uk0;
			#endif
			SpeedRealL=SpeedReal-Uk0;
			SpeedRealR=SpeedReal+Uk0;

			/*	�ٶ��޷�	*/
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
			MotorDriverVClControl(SpeedRealL,SpeedRealR);	//�ٶ����
	}else
	if((SearchLineFlag==0)&&(LostLineFlag==1))//����,������0,�ٽ���Ѱ�߲���
	{
			SpeedTarget=0;

			SpeedRealL=SpeedReal;
			SpeedRealR=SpeedReal;
			if(DirFlag>=0)//��ƫ
					MotorDriverVClControl(SpeedRealL,0);//�ٶ����
			else					//��ƫ
					MotorDriverVClControl(0,SpeedRealR);//�ٶ����
			if(SpeedReal==0)//ʵ���ٶȼ�����0,��Ѱ�߱�־λSearchLineFlag
			{
				SearchLineFlag		=	1;
				SearchLineCount 	= 0;							//��Ѱ��ʱ��
			}
	}else
	if((SearchLineFlag==1)&&(LostLineFlag==1))//Ѱ��,�ȼ�����Ѱ���ٶ�
	{
			SearchLineCount++;//Ѱ��ʱ���ʱ, ����ʱ��δѰ����, ��ֹͣѰ��
			if( SearchLineCount >= SearchLineTime)
			{
				SearchLineCount	= SearchLineTime;		//Ѱ��ʱ���޷�
				SpeedTarget			= 0;								//Ѱ���ٶ�����Ϊ��
			}else{
				SpeedTarget=SpeedSearchLine;
			}

			if( (SpeedTarget==0) && (SpeedReal==0) )		//Ѱ�߳�ʱ,������ɲ��
			{
				MotorDriverVClControl(0,0);
				BREAK_L = BREAK_R = 1;										//������ɲ��
				RunFlag = RUNFLAG_LOSTLINE;
			}else{																			//������Ѱ��
				SpeedRealL=SpeedReal;
				SpeedRealR=SpeedReal;
				if(DirFlag>=0)//��ƫ
				{ MotorDriverVClControl(SpeedRealL,-SpeedRealR); }//BREAK_R = 1; }//�ٶ��������������ɲ��
				else					//��ƫ
				{	MotorDriverVClControl(-SpeedRealL,SpeedRealR); }//BREAK_L = 1; }//�ٶ��������������ɲ��
			}
	}
}
/*********************************************
	�������ƣ�void MotorDriverCtrTracklessLine( void )
	��		�ܣ�ֱ���޹�ģʽ��,�ٶ����
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverCtrTracklessLine( void )
{
		SpeedTarget	= SpeedNormal;
		MotorDriverVClControl(SpeedReal,SpeedReal);	//�ٶ����
}
/*********************************************
	�������ƣ�void MotorDriverBackTracklessLine( void )
	��		�ܣ�ֱ���޹�ģʽ��,�����ٶ����
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverBackTracklessLine( void )
{
		SpeedTarget	= -SpeedNormal;
		MotorDriverVClControl(SpeedReal,SpeedReal);	//�ٶ����
}
/*********************************************
	�������ƣ�void MotorDriverStop( void )(Timer3��ÿ20ms����һ��)
	��		�ܣ���ˢ���ֹͣ,��������,��������״̬
						λΪEND,����RunMod.
						===ͨ������RunMod=STOP��RunStatus==GOINGִ�иò���===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===����ִ����Ϻ�,����RunMod=NULL��RunStatus=END	===
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverStop( void )
{
	MotorDriverCtrPID( );
	if(SpeedReal==0)
	{
		RunStatus=END;//ʵ���ٶȼ�����0,ͣ������
		RunMod=NULL;	//����˶�ģʽ
	}
}
/*********************************************
	�������ƣ�void MotorDriverStop_Trackless( void )(Timer3��ÿ20ms����һ��)
	��		�ܣ���ˢ���ֹͣ,��������,��������״̬
						λΪEND,����RunMod.
						===ͨ������RunMod=STOP��RunStatus==GOINGִ�иò���===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===����ִ����Ϻ�,����RunMod=NULL��RunStatus=END	===
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverStop_Trackless( void )
{
	SpeedTarget	= 0;
	MotorDriverVClControl(SpeedReal,SpeedReal);	//�ٶ����
	if(SpeedReal==0)
	{
		RunStatus=END;//ʵ���ٶȼ�����0,ͣ������
		RunMod=NULL;	//����˶�ģʽ
	}
}
/*********************************************
	�������ƣ�void MotorDriverTurn_90Angle( unsigned char dir, unsigned char sendstatus )(Timer3��ÿ20ms����һ��)
	��		�ܣ�ת��90��,��������״̬λΪEND,����RunMod.
						===����RunMod=TURN_90��RunStatus=GOINGִ�иò���	===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===����ִ����Ϻ�,����RunMod=NULL��RunStatus=END	===
	��		�Σ�dir					:ת�巽��(LEFT��RIGHT)
						sendstatus	:�Ͳ�״̬
												 SENDING,ת���Ͳ�(����ת��90��),ͨ����ʱ����ʱת��90��.
												 SENDBACK,�Ͳͷ���(����ת��90��),ͨ��Ѱ��ת��.
						ע:1.����dir��sendstatusȷʵת�巽���Լ��Ƿ񲥷��Ͳ�����
									SENDING  LEFT  	:��ת��,��������
									SENDING  RIGHT 	:��ת��,��������
									SENDBACK LEFT  	:��ת��,����������
									SENDBACK RIGHT	:��ת��,����������
	��		ֵ����
*********************************************/
void MotorDriverTurn_90Angle( unsigned char dir, unsigned char sendstatus )
{
	static unsigned char turn90count=0;
	if(sendstatus==SENDING)//�Ͳ�״̬,ת���Ͳ�,����ת��90��,�����Ͳ�����
	{
			TurnTimeCountFlag=1;		//��ʼ����ת�������ʱ(�ڶ�ʱ��3��10ms�ж��н��м���)
			if((TurnTimeCount<=560) && (dir!=MID))		//˫��ת,����Ϊ560*0.005=2.8s,�ٶ�SpeedTurn=30.
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//����Ŀ���ٶ�Ϊת���ٶ�SpeedTurn
					/*	˫��ת��	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
			}else{									//ת��ʱ�䵽,ͣ��
					if(SpeedTarget!=0)
						SpeedTarget=0;		//����Ŀ���ٶ�Ϊ0
					/*	˫��ת��	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
					if(SpeedReal==0)
					{
						TurnTimeCountFlag=0;			//��ת���ʱ��־
						RunStatus=END;						//ʵ���ٶȼ�����0,ͣ������
						RunMod=NULL;							//����˶�ģʽ
// 						VoicePlay(TakeMealVoice);	//���ñ�־λ,����ȡ������
					}
			}
	}else//�Ͳͷ���,����\ͬ��ת��90��,����������
	{
			if( !LineDetect_Centre( ) )	//��⵽��
			{	turn90count++;if(turn90count>100)turn90count=100;	}//100*5ms=500ms
			else												//δ��⵽��
			{ 
				turn90count=0;
				
				turn90_180_timing++;
				if(turn90_180_timing>=TURN90_180_TIME)
					turn90_180_timing=TURN90_180_TIME;
			}
			
			if( ( turn90count == 0 ) && (dir!=MID))//����,δ��⵽����
			{
					if(turn90_180_timing>=TURN90_180_TIME)
					{
						SpeedTarget	= 0;
						if(0==SpeedReal) turn_90_180_status=turn_90_180_status_lostline;
					}else	if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//����Ŀ���ٶ�Ϊת���ٶ�SpeedTurn
					/*	˫��ת��	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
// 					else if(dir==MID)
// 					{
// 						if(DirFlag>=0)	//��ƫ
// 						{ MotorDriverVClControl(SpeedReal,0); BREAK_R = 1; }//�ٶ��������������ɲ��
// 						else						//��ƫ
// 						{	MotorDriverVClControl(0,SpeedReal); BREAK_L	=	1; }//�ٶ��������������ɲ��
// 					}
			}else if( turn90count >= 10 ){									//Ѱ����,ͣ��
					// ����ת��ģʽ����Ѱ���ߺ�ֱ���л�����ģʽ,����ǰ��.�������Ƚ���˳-------------------------------------
					/*	����ת����ͣת,��ֹ��һʱ�̵�����ʱʹ��������ƶ�	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(0,SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(SpeedReal,0);//�ٶ����
					RunStatus=END;			//ʵ���ٶȼ�����0,ͣ������
					RunMod=NULL;				//����˶�ģʽ
					TurnTimeCountFlag=0;//��ת���ʱ��־
					TurnTimeCount=0;		//��ת�����ֵ
					turn90count=0;			//�����״̬
					turn90_180_timing=0;
			}
			
			if(turn_90_180_status==turn_90_180_status_lostline)
			{
					RunStatus=END;			//ʵ���ٶȼ�����0,ͣ������
					RunMod=NULL;				//����˶�ģʽ
					TurnTimeCountFlag=0;//��ת���ʱ��־
					TurnTimeCount=0;		//��ת�����ֵ
					turn90count=0;			//�����״̬
					
					turn90_180_timing=0;
					MotorDriverVClControl(0,0);
					BREAK_L = BREAK_R = 1;//������ɲ��
			}
	}
}
/*********************************************
	�������ƣ�void MotorDriverTurn_90Angle( unsigned char dir )(Timer3��ÿ20ms����һ��)
	��		�ܣ�ת��180��,��������״̬λΪEND,����RunMod.
						===����RunMod=TURN_180_Trackless��RunStatus=GOING	===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===����ִ����Ϻ�,����RunMod=NULL��RunStatus=END	===
	��		�Σ�dir					:ת�巽��(LEFT��RIGHT)
						sendstatus	:�Ͳ�״̬
												 SENDING,ת���Ͳ�(����ת��90��),ͨ����ʱ����ʱת��90��.
												 SENDBACK,�Ͳͷ���(����ת��90��),ͨ��Ѱ��ת��.
						ע:1.����dir��sendstatusȷʵת�巽���Լ��Ƿ񲥷��Ͳ�����
									SENDING  LEFT  	:��ת��,��������
									SENDING  RIGHT 	:��ת��,��������
									SENDBACK LEFT  	:��ת��,����������
									SENDBACK RIGHT	:��ת��,����������
	��		ֵ����
*********************************************/
void MotorDriverTurn_180Angle_Trackless( unsigned char dir )
{
	static unsigned char turn180count=0;
			TracklessTurnCountFlag=1;		//��ʼ����ת�������ʱ(�ڶ�ʱ��3��10ms�ж��н��м���)
			if((TracklessTurnCount<=145000) && (dir!=MID))		//137000
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//����Ŀ���ٶ�Ϊת���ٶ�SpeedTurn
					/*	˫��ת��	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
			}else{									//ת��ʱ�䵽,ͣ��
					if(SpeedTarget!=0)
						SpeedTarget=0;		//����Ŀ���ٶ�Ϊ0
					/*	˫��ת��	*/
					if(dir==RIGHT)			//��������,��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else if(dir==LEFT)	//��������,��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
					if(SpeedReal==0)
					{
						RunStatus=END;						//ʵ���ٶȼ�����0,ͣ������
						RunMod=NULL;							//����˶�ģʽ
						TracklessTurnCountFlag=0;	//��ת���ʱ��־
						TracklessTurnCount=0;			//��ת�����ֵ
					}
			}
}
/*********************************************
	�������ƣ�void MotorDriverTurn_180Angle( unsigned char dir, unsigned char turn180mod )(Timer3��ÿ20ms����һ��)
	��		�ܣ�ת��180��,��������״̬λΪEND,����RunMod.
						===����RunMod=TURN_180��RunStatus=GOINGִ�иò��� ===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===����ִ����Ϻ�,����RunMod=NULL��RunStatus=END	===
	��		�Σ�dir					:ת�巽��(LEFT��RIGHT)
						turn180mod	:ת��ģʽ,
												 TURN180_CONTINUE(ת������ǰ��);
												 TURN180_STOP(ת���ֹͣ)
	��		ֵ����
*********************************************/
void MotorDriverTurn_180Angle( unsigned char dir, unsigned char turn180mod )
{
	static unsigned char turn180status =0;
	static unsigned char turn180count	 =0;
	if(turn180status==0)//����δ����״̬
	{
			if(!LineDetect_All( ))//��⵽��
			{
					if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//����Ŀ���ٶ�Ϊת���ٶ�SpeedTurn
					/*	˫��ת��	*/
					if(dir==RIGHT)		//��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else							//��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
			}else{								//δ��⵽��
					/*	˫��ת��	*/
					if(dir==RIGHT)		//��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else							//��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
					turn180status = 1;														//����״̬λ
			}
	}else//�Ѷ���,����Ѱ��ģʽ
	{
			if( !LineDetect_Centre( ) )	//��⵽��
			{	turn180count++;if(turn180count>100)turn180count=100;	}//100*5ms=500ms
			else												//δ��⵽��
			{ 
				turn180count=0;
				
				turn90_180_timing++;
				if(turn90_180_timing>=TURN90_180_TIME)
					turn90_180_timing=TURN90_180_TIME;
			}
			
			if( turn180count == 0 )//����,δ��⵽��
			{
					/*	��ֹͣ���Ѷ��ߵ������(��180��ת��֮ǰ�Ѿ�����),�趨ת���ٶȣ�����ʡ�ԣ�	*/
					if(turn90_180_timing>=TURN90_180_TIME)
					{
						SpeedTarget	= 0;
						if(0==SpeedReal) turn_90_180_status=turn_90_180_status_lostline;
					}else if(SpeedTarget!=SpeedTurn)
						SpeedTarget=SpeedTurn;//����Ŀ���ٶ�Ϊת���ٶ�SpeedTurn
					/*	˫��ת��	*/
					if(dir==RIGHT)	//��ת��
						MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
					else						//��ת��
						MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
			}else if( turn180count >= 10 ){							//��⵽��
					switch( turn180mod )
					{
						case TURN180_STOP:
							// ת��ģʽһ��Ѱ���ߺ�,ͣ��-------------------------------------
							if(SpeedTarget!=0)
								SpeedTarget=0;//����Ŀ���ٶ�Ϊ0
							/*	˫��ת��	*/
							if(dir==RIGHT)	//��ת��
								MotorDriverVClControl(SpeedReal,-SpeedReal);//�ٶ����
							else						//��ת��
								MotorDriverVClControl(-SpeedReal,SpeedReal);//�ٶ����
							if(SpeedReal==0)//�ٶȼ���0
							{
								RunStatus=END;			//ʵ���ٶȼ�����0,ͣ������
								RunMod=NULL;				//����˶�ģʽ
								turn180status = 0;	//����״̬λ
								turn180count=0;			//�����״̬
							}
						break;
						case TURN180_CONTINUE:
							// ת��ģʽ����Ѱ���ߺ�ֱ���л�����ģʽ,����ǰ��.�Ƚ���˳-------------------------------------
							/*	����ת����ͣת,��ֹ��һʱ�̵�����ʱʹ��������ƶ�	*/
							if(dir==RIGHT)//��ת��
								MotorDriverVClControl(SpeedReal,0);//�ٶ����
							else					//��ת��
								MotorDriverVClControl(0,SpeedReal);//�ٶ����
							RunStatus=END;			//ʵ���ٶȼ�����0,ͣ������
							RunMod=NULL;				//����˶�ģʽ
							turn180status = 0;	//����״̬λ
							turn180count=0;			//�����״̬
						break;
					}
					turn90_180_timing=0;
			}
			
			if(turn_90_180_status==turn_90_180_status_lostline)
			{
					RunStatus=END;			//ʵ���ٶȼ�����0,ͣ������
					RunMod=NULL;				//����˶�ģʽ
					turn180status = 0;	//����״̬λ
					turn180count=0;			//�����״̬
					
					turn90_180_timing=0;
					MotorDriverVClControl(0,0);
					BREAK_L = BREAK_R = 1;//������ɲ��
			}
	}
}
/*********************************************
	�������ƣ�void MotorDriverRouteSel( void )(Timer3��ÿ5ms����һ��)
	��		�ܣ��ڽ���㴦ת��,��ʵ��·�ߵ�ѡ����
						===					ͨ������RunMod=ROUTESELִ�иò���			===
						===					����ִ���ڶ�ʱ��3���ж������					===
						===					����ִ����Ϻ�,����RunMod=NULL				===
						===					��ִ��8����,��Լ��ǰ��2.7m���ٶ�=100��===
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverRouteSel( void )
{
	RouteSelTime++;

	if(RouteSelTime<=1600)
	{
		MotorDriverCtrPID();
	}else{
		RunStatus=END;//ʵ���ٶȼ�����0,ͣ������
		RunMod=NULL;	//����˶�ģʽ
		RouteSelTime=0;
	
		/*	��������(����PID����)	*/
		RobotRunTypeStart( SpeedSet_HV );
	}
}
/*********************************************
	�������ƣ�void MotorDriverSpeedIncDecA( void )
	��		�ܣ�ʵ�ֵ���ٶȵ��ȼӼ���
	��		�Σ���
	��		ֵ����
*********************************************/
void MotorDriverSpeedIncDecA( void )
{
	if(SpeedTarget!=SpeedReal)//ʵ���ٶ�δ�ﵽĿ���ٶ�
	{
			if(SpeedTarget>SpeedReal)//����
			{
				if(fabs(SpeedTarget-SpeedReal)>=SpeedIncA)
					SpeedReal+=SpeedIncA;
				else
					SpeedReal=SpeedTarget;
			}else//����
			{
#if DECACC_CHANGE_EN
				if( RunModBuf == NORMAL )					//����Ѱ��
				{
					if(SpeedNormal>=178)			SpeedDecA=6;
					else if(SpeedNormal>=150)	SpeedDecA=5;
					else if(SpeedNormal>=140)	SpeedDecA=4;
					else if(SpeedNormal>=120)	SpeedDecA=3;
					else 											SpeedDecA=2;
				}else if( RunModBuf == ROUTESEL )	//·��ѡ��(ת��)
				{	SpeedDecA=2;								}
				else
				{	SpeedDecA=5;								}		//����ģʽ
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
