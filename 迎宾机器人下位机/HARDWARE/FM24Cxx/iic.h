#ifndef __IIC_H
#define __IIC_H
#include "sys.h"
  		   
//IO��������
#define SDA_IN()  {GPIOC->CRL&=0XFFF0FFFF;GPIOC->CRL|=0x00080000;}
#define SDA_OUT() {GPIOC->CRL&=0XFFF0FFFF;GPIOC->CRL|=0x00030000;}

//IO��������	 
#define IIC_SCL    PCout(5) //SCL
#define IIC_SDA    PCout(4) //SDA
#define READ_SDA   PCin(4) 	//����SDA

//IIC���в�������
void IIC_Init(void);                	//��ʼ��IIC��IO��				 
void IIC_Start(void);									//����IIC��ʼ�ź�
void IIC_Stop(void);	  							//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);						//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);	//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 								//IIC�ȴ�ACK�ź�
void IIC_Ack(void);										//IIC����ACK�ź�
void IIC_NAck(void);									//IIC������ACK�ź�
  
#endif
















