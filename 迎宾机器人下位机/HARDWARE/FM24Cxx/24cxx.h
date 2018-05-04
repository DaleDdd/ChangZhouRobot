#ifndef __24CXX_H
#define __24CXX_H
#include "iic.h"

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	  8191
#define AT24C128	16383
#define AT24C256	32767
//����EEPROM��FRAM������
#define EE_TYPE AT24C01
#define EE_ADDR	0xA0

// #define RESTARTCONTINUE			(1)//������������

/* **************
����ṹ�嶨�壺
	 �ն�ָ��
	 ����Ŀ������
	 ����״̬
	 ************** */
struct TASK{
	unsigned 	char command;				//�ն�ָ��
	unsigned  char tasktable;			//����Ŀ������
	unsigned 	char taskstatus;		//����״̬
	unsigned	char ctrareanum;		//���������
	unsigned 	char ctrareastatus;	//������״̬
																//CtrArea_Idle:��������״̬
																//CtrArea_Busy:������æ״̬
	unsigned  char playvoicenume; 		//�������1-12
	unsigned  char barrierstatus; 		//�ϰ���״̬
	unsigned  char speedmod; 			//�ٶ�ģʽ,SPEEDMOD_H(1)��SPEEDMOD_M(0)��SPEEDMOD_L(2)
};
extern struct TASK Task;				//����ṹ������

/*�洢��ַ����*/
#define AT24CXXInitFlag_Addr				((unsigned char)0)			//AT24CXX���ݵ�һ�����г�ʼ����־�洢��ַ
#define TaskTable_Addr							((unsigned char)1)			//�����Ŀ�����Ŵ洢��ַ
#define ContrCommand_Addr						((unsigned char)2)			//ң���ն˵Ŀ���ָ��洢��ַ
#define TaskStatus_Addr							((unsigned char)3)			//��������״̬�洢��ַ
#define CtrAreaNum_Addr							((unsigned char)4)			//��������Ŵ洢��ַ
// #define CtrAreaStatus_Addr					((unsigned char)5)			//������״̬�洢��ַ

/*�ն�ָ��*/
#define Com_NULL										((unsigned char)0)			//��ָ��
#define Com_Send										((unsigned char)1)			//�Ͳ�ָ��ն˿���ָ�
#define Com_Stop										((unsigned char)2)			//ָֹͣ��ն˿���ָ�
#define Com_Back										((unsigned char)3)			//����ָ��ն˿���ָ�
#define Com_RCAS										((unsigned char)4)			//��������״̬(read control area status)
#define Com_WCAS										((unsigned char)5)			//д������״̬(Write control area status)

/*��������š�״̬*/
#define CtrArea_NULL								((unsigned char)0x00)		//��
#define CtrArea_1										((unsigned char)0x01)		//1�Ź�����
#define CtrArea_2										((unsigned char)0x02)		//2�Ź�����
#define CtrArea_3										((unsigned char)0x03)		//3�Ź�����
#define CtrArea_4										((unsigned char)0x04)		//4�Ź�����
#define CtrArea_5										((unsigned char)0x05)		//5�Ź�����

#define CtrArea_Idle								((unsigned char)0x00)		//��������״̬
#define CtrArea_Busy								((unsigned char)0x01)		//������æ״̬

/*�Ͳ͹�����,�����־�л�˵����
	TaskNo-->�յ��Ͳ�����-->TaskSending-->������Ӧ����(ת���Ͳ�)-->TaskSendEnd
	-->�������������־��-->TaskOver-->�������ȡ�͵�(��ʼ��)-->TaskNo
*/
#define TaskNo											((unsigned short)0x01)	//������
#define TaskSending									((unsigned short)0x02)	//������,��δ�ʹ���Ӧ�Ĳ���
#define TaskSendEnd									((unsigned short)0x03)	//������,���ʹ���Ӧ�Ĳ���,��δ�������������־��
#define TaskOver										((unsigned short)0x04)	//������,�Ѿ��������������־��,��δ������ȡ�͵�

u8 AT24CXX_ReadOneByte(u16 ReadAddr);															//ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);					//ָ����ַд��һ���ֽ�
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);	//ָ����ַ��ʼд��ָ����������(�ú�������д��16bit����32bit������)
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);											//ָ����ַ��ʼ��ȡָ����������(�ú������ڶ���16bit����32bit������)
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);			//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   			//��ָ����ַ��ʼ����ָ�����ȵ�����

u8 AT24CXX_Check(void);  //�������
void AT24CXX_Init(void); //��ʼ��IIC

void WriteTaskTable(unsigned char tasktable);											//д�������Ŀ������
unsigned char ReadTaskTable( void );															//��ȡ�����Ŀ������
void WriteContrCommand(unsigned char contrcommand);								//д��ң���ն˵Ŀ���ָ��
unsigned char ReadContrCommand( void );														//��ȡң���ն˵Ŀ���ָ��
void WriteTaskStatus(unsigned char taskstatus);										//д����������״̬
unsigned char ReadTaskStatus( void );															//��ȡ��������״̬
void WriteCtrAreaNum(unsigned char ctrareanum);										//д����������
unsigned char ReadCtrAreaNum( void );															//��ȡ���������
#endif
