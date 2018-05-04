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
//定义EEPROM、FRAM的类型
#define EE_TYPE AT24C01
#define EE_ADDR	0xA0

// #define RESTARTCONTINUE			(1)//重启继续任务

/* **************
任务结构体定义：
	 终端指令
	 任务目标桌号
	 任务状态
	 ************** */
struct TASK{
	unsigned 	char command;				//终端指令
	unsigned  char tasktable;			//任务目标桌号
	unsigned 	char taskstatus;		//任务状态
	unsigned	char ctrareanum;		//管制区编号
	unsigned 	char ctrareastatus;	//管制区状态
																//CtrArea_Idle:管制区闲状态
																//CtrArea_Busy:管制区忙状态
	unsigned  char playvoicenume; 		//放音编号1-12
	unsigned  char barrierstatus; 		//障碍物状态
	unsigned  char speedmod; 			//速度模式,SPEEDMOD_H(1)、SPEEDMOD_M(0)、SPEEDMOD_L(2)
};
extern struct TASK Task;				//任务结构体声明

/*存储地址定义*/
#define AT24CXXInitFlag_Addr				((unsigned char)0)			//AT24CXX数据第一次运行初始化标志存储地址
#define TaskTable_Addr							((unsigned char)1)			//任务的目标桌号存储地址
#define ContrCommand_Addr						((unsigned char)2)			//遥控终端的控制指令存储地址
#define TaskStatus_Addr							((unsigned char)3)			//任务的完成状态存储地址
#define CtrAreaNum_Addr							((unsigned char)4)			//管制区编号存储地址
// #define CtrAreaStatus_Addr					((unsigned char)5)			//管制区状态存储地址

/*终端指令*/
#define Com_NULL										((unsigned char)0)			//空指令
#define Com_Send										((unsigned char)1)			//送餐指令【终端控制指令】
#define Com_Stop										((unsigned char)2)			//停止指令【终端控制指令】
#define Com_Back										((unsigned char)3)			//返回指令【终端控制指令】
#define Com_RCAS										((unsigned char)4)			//读管制区状态(read control area status)
#define Com_WCAS										((unsigned char)5)			//写管制区状态(Write control area status)

/*管制区编号、状态*/
#define CtrArea_NULL								((unsigned char)0x00)		//空
#define CtrArea_1										((unsigned char)0x01)		//1号管制区
#define CtrArea_2										((unsigned char)0x02)		//2号管制区
#define CtrArea_3										((unsigned char)0x03)		//3号管制区
#define CtrArea_4										((unsigned char)0x04)		//4号管制区
#define CtrArea_5										((unsigned char)0x05)		//5号管制区

#define CtrArea_Idle								((unsigned char)0x00)		//管制区闲状态
#define CtrArea_Busy								((unsigned char)0x01)		//管制区忙状态

/*送餐过程中,任务标志切换说明：
	TaskNo-->收到送餐命令-->TaskSending-->读到相应桌号(转体送餐)-->TaskSendEnd
	-->读到任务结束标志点-->TaskOver-->读到后厨取餐点(起始点)-->TaskNo
*/
#define TaskNo											((unsigned short)0x01)	//无任务
#define TaskSending									((unsigned short)0x02)	//有任务,还未送达相应的餐桌
#define TaskSendEnd									((unsigned short)0x03)	//有任务,已送达相应的餐桌,还未到达任务结束标志点
#define TaskOver										((unsigned short)0x04)	//有任务,已经到达任务结束标志点,还未到达后厨取餐点

u8 AT24CXX_ReadOneByte(u16 ReadAddr);															//指定地址读取一个字节
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);					//指定地址写入一个字节
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);	//指定地址开始写入指定长度数据(该函数用于写入16bit或者32bit的数据)
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);											//指定地址开始读取指定长度数据(该函数用于读出16bit或者32bit的数据)
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);			//从指定地址开始写入指定长度的数据
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   			//从指定地址开始读出指定长度的数据

u8 AT24CXX_Check(void);  //检查器件
void AT24CXX_Init(void); //初始化IIC

void WriteTaskTable(unsigned char tasktable);											//写入任务的目标桌号
unsigned char ReadTaskTable( void );															//读取任务的目标桌号
void WriteContrCommand(unsigned char contrcommand);								//写入遥控终端的控制指令
unsigned char ReadContrCommand( void );														//读取遥控终端的控制指令
void WriteTaskStatus(unsigned char taskstatus);										//写入任务的完成状态
unsigned char ReadTaskStatus( void );															//读取任务的完成状态
void WriteCtrAreaNum(unsigned char ctrareanum);										//写入管制区编号
unsigned char ReadCtrAreaNum( void );															//读取管制区编号
#endif
