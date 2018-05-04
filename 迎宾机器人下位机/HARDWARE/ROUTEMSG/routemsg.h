#ifndef __ROUTEMSG_H
#define __ROUTEMSG_H
#include "stmflash.h"
#include "24cxx.h"

#define SPEEDMOD_M	0
#define SPEEDMOD_H	1
#define SPEEDMOD_L	2

/* **************
·���ṹ�嶨�壺
	 ������ID/���
	 ��������
	 ��תת��㿨��
	 ��תת��㿨��
	 �ٶ�ģʽ
	 ************** */
struct ROUTEMSG{
	unsigned char robotid;	//�����˱��
	unsigned char tabledir;	//��������
	unsigned char route_l;	//��ת��㿨��
	unsigned char route_r;	//��ת��㿨��
	unsigned char speedmod; //�ٶ�ģʽ,SPEEDMOD_H(1)��SPEEDMOD_M(0)��SPEEDMOD_L(2)
};
typedef struct tagtoutemsg{
	unsigned  char command;				//����ָ����
	unsigned  char playvoicenume; 		//�������1-12
	unsigned  char barrierstatus; 		//�ϰ���״̬
	unsigned  char speedmod; 			//�ٶ�ģʽ,SPEEDMOD_H(1)��SPEEDMOD_M(0)��SPEEDMOD_L(2)
	unsigned  char Station_mode[20];	//20��վ��ת��ģʽ
	unsigned  char Station_tim[20];	    //20��վ��ͣ��ʱ��
	unsigned  char Station_DIR;			//��ʼ��ת������
	unsigned char Cross[10];			//10���ֲ������
}ROUTEMSG1;

struct VOICEMSG{
	unsigned short time;//����ʱ��
	unsigned char  mod; //����ģʽ
	unsigned char  runmod;//����ģʽ
};
extern unsigned char RouteSelDir;	//·��ѡ����MID��LEFT��RIGHT
extern unsigned char Turn90Dir;		//90��ת�巽��MID��LEFT��RIGHT
extern unsigned char Turn180Dir;	//180��ת�巽��LEFT��RIGHT
extern struct ROUTEMSG RouteMsg;	//·���ݴ����
extern struct VOICEMSG VoiceMsg[11];//������ʱ�� ģʽ��Ϣ
extern signed 		int SpeedSet_HV;
extern ROUTEMSG1 RouteMsg1;
extern unsigned char Voice_ok;
/* ����ַ���� */
#define DATA_BASE						(0x0807D800)								//����������ַ
// #define ROBOTID_ADDR				(DATA_BASE)									//������ID/��Ŵ洢��ַ
#define INFALLCLEAR_BASE		(unsigned long)0x0807FFFE		//���е������������־����ַ

/*	�Ĵ�����ַ����	*/
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
#define WUDIALOGUE_REG		12//��������ʶ��ģ��ĶԻ�����
#define Voice_OK            14

/* ����ַ���壨���������ã� */
#define VOICE_TIME_SET_BASEADDR		DATA_BASE							//������1-10��ʱ���洢����ַ
#define VOICE_MOD_SET_BASEADDR		(VOICE_TIME_SET_BASEADDR+2*10)		//������1-10�Ĳ���ģʽ�洢����ַ
#define VOICE_RMOD_SET_BASEADDR		(VOICE_MOD_SET_BASEADDR+2*10)		//������1-10������ģʽ�洢����ַ
#define SPEEDMOD_BASEADDR			(VOICE_RMOD_SET_BASEADDR+2*10) 		//�ٶ�ģʽ����ַ
#define ROBOTMOD_TRACK_BASEADDR		(SPEEDMOD_BASEADDR+20)				//������ģʽ(0:�й�,1:�޹�)
#define StationDIR_BASEADDR			(ROBOTMOD_TRACK_BASEADDR+2)			//��ʼ��ת��ģʽ
#define STATION_MODE_BASEADDR       (StationDIR_BASEADDR+20)			//վ��ת��ģʽ
#define STATION_TIME_BASEADDR       (STATION_MODE_BASEADDR+60)			//վ��ͣ��ʱ��
#define CROSS_SET_BASEADDR          (STATION_TIME_BASEADDR+60)			//�ֲ������


#define VOICE_TIME_ADDR(n)	(VOICE_TIME_SET_BASEADDR+(n-1)*2)			//������n(1-10)��ʱ��
#define VOICE_MOD_ADDR(n)		(VOICE_MOD_SET_BASEADDR+(n-1)*2)			//������n(1-10)�Ĳ���ģʽ
#define VOICE_RMOD_ADDR(n)	(VOICE_RMOD_SET_BASEADDR+(n-1)*2)			//������n(1-10)������ģʽ
#define STATION_MODE(n)		(STATION_MODE_BASEADDR+(n-1)*2)
#define STATION_TIM(n)		(STATION_TIME_BASEADDR+(n-1)*2)
#define CROSS_SET(n)		(CROSS_SET_BASEADDR+(n-1)*2)

/*	��س����궨��	*/
#define MID							((unsigned char) 0x00)	//������,��ʾ�ڽ���㴦ֱ�ߡ���������ת��
#define LEFT						((unsigned char) 0x01)	//������,��ʾ�ڽ���㴦��ת�����������
#define RIGHT						((unsigned char) 0x02)	//������,��ʾ�ڽ���㴦��ת���������Ҳ�
#define MID_BACK				((unsigned char) 0x03)	//��ʾ��������ת�壬������ͺ�180��ת��ԭ·����
#define LEFT_BACK				((unsigned char) 0x04)	//��ʾ��������࣬������ͺ���ת��ԭ·����
#define RIGHT_BACK			((unsigned char) 0x05)	//��ʾ�������Ҳ࣬������ͺ���ת��ԭ·����
#define VOICEMOD_NULL							(0)//ֹͣ��������
#define VOICEMOD_Single 					(1)//�����㲥ģʽ����ȡ�ر겥�ţ�
#define VOICEMOD_MULLOOP 					(2)//����ѭ��ģʽ
#define VOICEMOD_TRACKLESSLINE		(3)//�޹�ֱ������ģʽ
#define VOICERUNMOD_0							(0)//����ģʽ 0
#define VOICERUNMOD_1							(1)//����ģʽ 1
#define VOICERUNMOD_2							(2)//����ģʽ 2
#define VOICERUNMOD_3							(3)//����ģʽ 3
#define VOICERUNMOD_4							(4)//����ģʽ 4
#define VOICERUNMOD_5							(5)//����ģʽ 5
#define VOICERUNMOD_6							(6)//����ģʽ 6
#define ROBOTMOD_TRACK						(0)//�������й�ģʽ
#define ROBOTMOD_TRACKLESS				(1)//�������޹�ģʽ
#define SumTableNum			(unsigned char) 99		//��������

void RobotIDSet(unsigned short robotid);														//���û����˱��
void TableDIRSet(unsigned char tablenum, unsigned short tabledir);	//���ò����Ĳ�������
void RouteLeftSet(unsigned char tablenum, unsigned short routel);		//���ò�������ת���·�������ţ�
void RouteRightSet(unsigned char tablenum, unsigned short router);	//���ò�������ת���·�������ţ�
void SpeedModSet(unsigned short speedmod);													//�����ٶ�ģʽ
void RobotModTrackSet(unsigned short robotmod);											//���û�����ģʽ
void StationDirSet(unsigned short tracklessdist);								//�����޹����
void RouteInfInit(unsigned char mod);																//���е���������ʼ������
unsigned char RobotIDRead( void );																	//��ȡ�����˱��
unsigned char TableDIRRead(unsigned char tablenum);									//��ȡ�����ķ���
unsigned char RouteLeftRead(unsigned char tablenum);								//��ȡ��������ת���·�������ţ�
unsigned char RouteRightRead(unsigned char tablenum);								//��ȡ��������ת���·�������ţ�
unsigned char SpeedModRead(void);																		//��ȡ�ٶ�ģʽ
unsigned char RobotModTrackRead(void);															//��ȡ������ģʽ(0:�й�,1:�޹�)
unsigned char StationDirRead(void);															//��ȡ�޹����
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
