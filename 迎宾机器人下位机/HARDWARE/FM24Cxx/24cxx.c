/*==========================================================================
	EEPROM、FRAM存储器IIC读写
	1.存储器型号: AT24C01,
	2.存储器容量: 1Kbit = 128Byte
	3.地址范围	: 0-127
	4.存储器地址: 0xA0
	5.AT24系列EEPROM写周期≤5ms,必须加延时
	2016.1.18	Li
==========================================================================*/
#include "24cxx.h"
#include "usart.h"
#include "delay.h"

//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
	u8 temp=0;		  	    																 
  IIC_Start();  
	IIC_Send_Byte(EE_ADDR);	   	//发送写命令
	IIC_Wait_Ack();
	if(EE_TYPE > AT24C02)
	{
		IIC_Send_Byte(ReadAddr>>8);		//发送高地址	  
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr%256);  //发送低地址
		IIC_Wait_Ack();
	}else
	{
		IIC_Send_Byte(ReadAddr%256);	//发送地址
		IIC_Wait_Ack();
	}
	IIC_Start();  	 	   
	IIC_Send_Byte(EE_ADDR+1);    //进入接收模式			   
	IIC_Wait_Ack();	 
	temp=IIC_Read_Byte(0);
	IIC_Stop();//产生一个停止条件
	return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
	IIC_Start();  
	IIC_Send_Byte(EE_ADDR);	    //发送写命令
	IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr);				//发送地址
		IIC_Wait_Ack();
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();//产生一个停止条件 
	delay_ms(5);//对于AT24Cxx,延时是必须的,写周期为5ms
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}
}
//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(8191)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
u8 AT24CXX_Check(void)
{
	u8 temp;
	temp=AT24CXX_ReadOneByte(AT24C64);//避免每次开机都写AT24CXX			   
	if(temp==0X55)return 0;
	else//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(AT24C64,0X55);
	    temp=AT24CXX_ReadOneByte(255);
		if(temp==0X55)return 0;
	}
	return 1;
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);
		NumToRead--;
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

// ==================================================================================
// 以下为任务信息存储函数
// ==================================================================================
/****************************************************
	函数名称：void WriteTaskTable(unsigned char tasktable)
	功		能：写入任务的目标桌号
	入		参：tasktable:	任务的目标桌号
	返		值：无
****************************************************/
struct TASK Task={Com_NULL,NULL,TaskNo,CtrArea_NULL,CtrArea_Idle};	//任务结构体声明
void WriteTaskTable(unsigned char tasktable)
{
	Task.tasktable = tasktable;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(TaskTable_Addr, tasktable);
	#endif
}
/****************************************************
	函数名称：unsigned char ReadTaskTable( void )
	功		能：读取任务的目标桌号
	入		参：无
	返		值：任务的目标桌号
****************************************************/
unsigned char ReadTaskTable( void )
{
	return (AT24CXX_ReadOneByte(TaskTable_Addr));
}
/****************************************************
	函数名称：void WriteTaskTable(unsigned char tasktable)
	功		能：写入遥控终端的控制指令
	入		参：contrcommand:	遥控终端的控制指令
	返		值：无
****************************************************/
void WriteContrCommand(unsigned char contrcommand)
{
	Task.command = contrcommand;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(ContrCommand_Addr, contrcommand);
	#endif
}
/****************************************************
	函数名称：unsigned char ReadTaskTable( void )
	功		能：读取遥控终端的控制指令
	入		参：无
	返		值：遥控终端的控制指令
****************************************************/
unsigned char ReadContrCommand( void )
{
	return (AT24CXX_ReadOneByte(ContrCommand_Addr));
}
/****************************************************
	函数名称：void WriteTaskStatus(unsigned char taskstatus)
	功		能：写入任务的完成状态
	入		参：taskstatus:	任务的完成状态
	返		值：无
****************************************************/
void WriteTaskStatus(unsigned char taskstatus)
{
	Task.taskstatus = taskstatus;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(TaskStatus_Addr, taskstatus);
	#endif
}
/****************************************************
	函数名称：unsigned char ReadTaskStatus( void )
	功		能：读取任务的完成状态
	入		参：无
	返		值：任务的完成状态
****************************************************/
unsigned char ReadTaskStatus( void )
{
	return (AT24CXX_ReadOneByte(TaskStatus_Addr));
}
/****************************************************
	函数名称：void WriteCtrAreaNum(unsigned char ctrareanum)
	功		能：写入管制区编号
	入		参：ctrareanum:	管制区编号
	返		值：无
****************************************************/
void WriteCtrAreaNum(unsigned char ctrareanum)
{
	Task.ctrareanum = ctrareanum;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(CtrAreaNum_Addr, ctrareanum);
	#endif
}
/****************************************************
	函数名称：unsigned char ReadCtrAreaNum( void )
	功		能：读取管制区编号
	入		参：无
	返		值：管制区编号
****************************************************/
unsigned char ReadCtrAreaNum( void )
{
	return (AT24CXX_ReadOneByte(CtrAreaNum_Addr));
}
//初始化IIC接口
void AT24CXX_Init(void)
{
	IIC_Init();
	if( AT24CXX_ReadOneByte(AT24CXXInitFlag_Addr) != 0x55 )
	{
		WriteContrCommand(Com_NULL);	//设置终端控制命令为空
		WriteTaskTable(0);						//设置目标桌号为空
		WriteTaskStatus(TaskNo);			//设置无任务状态
		WriteCtrAreaNum(0);						//设置管制区编号为零
		AT24CXX_WriteOneByte( AT24CXXInitFlag_Addr, 0x55 );//写入初始化标志位
	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
