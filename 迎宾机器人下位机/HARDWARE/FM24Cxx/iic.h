#ifndef __IIC_H
#define __IIC_H
#include "sys.h"
  		   
//IO方向设置
#define SDA_IN()  {GPIOC->CRL&=0XFFF0FFFF;GPIOC->CRL|=0x00080000;}
#define SDA_OUT() {GPIOC->CRL&=0XFFF0FFFF;GPIOC->CRL|=0x00030000;}

//IO操作函数	 
#define IIC_SCL    PCout(5) //SCL
#define IIC_SDA    PCout(4) //SDA
#define READ_SDA   PCin(4) 	//输入SDA

//IIC所有操作函数
void IIC_Init(void);                	//初始化IIC的IO口				 
void IIC_Start(void);									//发送IIC开始信号
void IIC_Stop(void);	  							//发送IIC停止信号
void IIC_Send_Byte(u8 txd);						//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);	//IIC读取一个字节
u8 IIC_Wait_Ack(void); 								//IIC等待ACK信号
void IIC_Ack(void);										//IIC发送ACK信号
void IIC_NAck(void);									//IIC不发送ACK信号
  
#endif
















