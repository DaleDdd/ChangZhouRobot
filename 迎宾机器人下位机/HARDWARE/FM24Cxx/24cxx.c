/*==========================================================================
	EEPROM��FRAM�洢��IIC��д
	1.�洢���ͺ�: AT24C01,
	2.�洢������: 1Kbit = 128Byte
	3.��ַ��Χ	: 0-127
	4.�洢����ַ: 0xA0
	5.AT24ϵ��EEPROMд���ڡ�5ms,�������ʱ
	2016.1.18	Li
==========================================================================*/
#include "24cxx.h"
#include "usart.h"
#include "delay.h"

//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
	u8 temp=0;		  	    																 
  IIC_Start();  
	IIC_Send_Byte(EE_ADDR);	   	//����д����
	IIC_Wait_Ack();
	if(EE_TYPE > AT24C02)
	{
		IIC_Send_Byte(ReadAddr>>8);		//���͸ߵ�ַ	  
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr%256);  //���͵͵�ַ
		IIC_Wait_Ack();
	}else
	{
		IIC_Send_Byte(ReadAddr%256);	//���͵�ַ
		IIC_Wait_Ack();
	}
	IIC_Start();  	 	   
	IIC_Send_Byte(EE_ADDR+1);    //�������ģʽ			   
	IIC_Wait_Ack();	 
	temp=IIC_Read_Byte(0);
	IIC_Stop();//����һ��ֹͣ����
	return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
	IIC_Start();  
	IIC_Send_Byte(EE_ADDR);	    //����д����
	IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr);				//���͵�ַ
		IIC_Wait_Ack();
	IIC_Send_Byte(DataToWrite);     //�����ֽ�							   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();//����һ��ֹͣ���� 
	delay_ms(5);//����AT24Cxx,��ʱ�Ǳ����,д����Ϊ5ms
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}
}
//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
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
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(8191)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
u8 AT24CXX_Check(void)
{
	u8 temp;
	temp=AT24CXX_ReadOneByte(AT24C64);//����ÿ�ο�����дAT24CXX			   
	if(temp==0X55)return 0;
	else//�ų���һ�γ�ʼ�������
	{
		AT24CXX_WriteOneByte(AT24C64,0X55);
	    temp=AT24CXX_ReadOneByte(255);
		if(temp==0X55)return 0;
	}
	return 1;
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
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
// ����Ϊ������Ϣ�洢����
// ==================================================================================
/****************************************************
	�������ƣ�void WriteTaskTable(unsigned char tasktable)
	��		�ܣ�д�������Ŀ������
	��		�Σ�tasktable:	�����Ŀ������
	��		ֵ����
****************************************************/
struct TASK Task={Com_NULL,NULL,TaskNo,CtrArea_NULL,CtrArea_Idle};	//����ṹ������
void WriteTaskTable(unsigned char tasktable)
{
	Task.tasktable = tasktable;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(TaskTable_Addr, tasktable);
	#endif
}
/****************************************************
	�������ƣ�unsigned char ReadTaskTable( void )
	��		�ܣ���ȡ�����Ŀ������
	��		�Σ���
	��		ֵ�������Ŀ������
****************************************************/
unsigned char ReadTaskTable( void )
{
	return (AT24CXX_ReadOneByte(TaskTable_Addr));
}
/****************************************************
	�������ƣ�void WriteTaskTable(unsigned char tasktable)
	��		�ܣ�д��ң���ն˵Ŀ���ָ��
	��		�Σ�contrcommand:	ң���ն˵Ŀ���ָ��
	��		ֵ����
****************************************************/
void WriteContrCommand(unsigned char contrcommand)
{
	Task.command = contrcommand;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(ContrCommand_Addr, contrcommand);
	#endif
}
/****************************************************
	�������ƣ�unsigned char ReadTaskTable( void )
	��		�ܣ���ȡң���ն˵Ŀ���ָ��
	��		�Σ���
	��		ֵ��ң���ն˵Ŀ���ָ��
****************************************************/
unsigned char ReadContrCommand( void )
{
	return (AT24CXX_ReadOneByte(ContrCommand_Addr));
}
/****************************************************
	�������ƣ�void WriteTaskStatus(unsigned char taskstatus)
	��		�ܣ�д����������״̬
	��		�Σ�taskstatus:	��������״̬
	��		ֵ����
****************************************************/
void WriteTaskStatus(unsigned char taskstatus)
{
	Task.taskstatus = taskstatus;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(TaskStatus_Addr, taskstatus);
	#endif
}
/****************************************************
	�������ƣ�unsigned char ReadTaskStatus( void )
	��		�ܣ���ȡ��������״̬
	��		�Σ���
	��		ֵ����������״̬
****************************************************/
unsigned char ReadTaskStatus( void )
{
	return (AT24CXX_ReadOneByte(TaskStatus_Addr));
}
/****************************************************
	�������ƣ�void WriteCtrAreaNum(unsigned char ctrareanum)
	��		�ܣ�д����������
	��		�Σ�ctrareanum:	���������
	��		ֵ����
****************************************************/
void WriteCtrAreaNum(unsigned char ctrareanum)
{
	Task.ctrareanum = ctrareanum;
	#ifdef	RESTARTCONTINUE
	AT24CXX_WriteOneByte(CtrAreaNum_Addr, ctrareanum);
	#endif
}
/****************************************************
	�������ƣ�unsigned char ReadCtrAreaNum( void )
	��		�ܣ���ȡ���������
	��		�Σ���
	��		ֵ�����������
****************************************************/
unsigned char ReadCtrAreaNum( void )
{
	return (AT24CXX_ReadOneByte(CtrAreaNum_Addr));
}
//��ʼ��IIC�ӿ�
void AT24CXX_Init(void)
{
	IIC_Init();
	if( AT24CXX_ReadOneByte(AT24CXXInitFlag_Addr) != 0x55 )
	{
		WriteContrCommand(Com_NULL);	//�����ն˿�������Ϊ��
		WriteTaskTable(0);						//����Ŀ������Ϊ��
		WriteTaskStatus(TaskNo);			//����������״̬
		WriteCtrAreaNum(0);						//���ù��������Ϊ��
		AT24CXX_WriteOneByte( AT24CXXInitFlag_Addr, 0x55 );//д���ʼ����־λ
	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
