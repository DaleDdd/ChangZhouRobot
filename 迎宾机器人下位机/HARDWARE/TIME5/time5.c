/*==========================================================================
	��ʱ��5��ʼ��
	1.�ö�ʱ��5��Ϊ���ڽ���һ֡���ݵ�5ms��ʱ��ʱ
	2.ID�����Ž��յ�5ms��ʱ��ʱ
	2016.1.23	Li
==========================================================================*/
#include "time5.h"
#include "wireless.h"
#include "crc16.h"
#include "include.h"
#include "id.h"
#include "hmi.h"
#include "routemsg.h"
#include "24cxx.h"
#include "usart.h"
#include "sys.h"
unsigned char TIM2_IRQSource	=NULL; //timer 2�ж�Դ��־
unsigned char TIM5_IRQSource	=NULL; //timer 5�ж�Դ��־
unsigned char TIM6_IRQSource	=NULL; //timer 6�ж�Դ��־
unsigned char STATMOD_FLAG=0;
unsigned char STATIM_FLAG=0;
unsigned char CROSS_FLAG=0;
unsigned char Recive_data[50];
unsigned char Recive_num=0;
																		 //ID_TRIG(1):LINE9�ϵ��ⲿ�жϴ������ж�(ά�������)
																		 //WIRELESS_TRIG(2):����5�������ж�(������λ��������֡)
																		 //HMI_TRIG(3):HMI�˻��ӿڴ����ж�(�����˻��ӿڵ�Modbus����,������������Ӧ)
const unsigned char ID_TRIG					=1;
const unsigned char WIRELESS_TRIG		=2;
const unsigned char HMI_TRIG				=3;
/*********************************************
	ͨ�ö�ʱ��5�жϳ�ʼ��
	����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36MHz
	arr���Զ���װֵ��
	psc��ʱ��Ԥ��Ƶ��
*********************************************/
void TIM5_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<3;	//TIM5ʱ��ʹ�� 
 	TIM5->ARR=arr;  		//�趨�������Զ���װֵ
	TIM5->PSC=psc;  		//Ԥ��Ƶ��72,�õ�1MHz�ļ���ʱ��
	
	TIM5_En();
	TIM5->EGR|=1<<0;		//����һ�������¼�
	TIM5_Dis();
	
	TIM5->DIER|=1<<0;   //��������ж�
	MY_NVIC_Init(2,3,TIM5_IRQn,2);//��ռ2,�����ȼ�3,��2
	
	TIM5_Dis();
}
/*********************************************
	�嶨ʱ��5�ļ�����
*********************************************/
void TIM5_ClrCont( void )
{
	TIM5->CNT=0;    //�嶨ʱ��4�ļ�����
}
/*********************************************
	ʹ��ͨ�ö�ʱ��5
*********************************************/
void TIM5_En( void )
{
	TIM5->CR1|=0x01;    //ʹ�ܶ�ʱ��5
}
/*********************************************
	��ֹͨ�ö�ʱ��5
*********************************************/
void TIM5_Dis( void )
{
	TIM5->CR1&=~0x01;    //��ֹ��ʱ��5
}
/*********************************************
	�������ƣ�void TIM5_IRQHandler(void),��ʱ��5�жϷ������,5ms
	��		�ܣ�����ģ��(����5)���ݽ��ճ�ʱ�ж�,�շ����ݸ�ʽ�μ�����λ���������ͨѶЭ�顷

	��		�Σ���
	��		�أ���
*********************************************/
unsigned char CommandReceiveStatus=0;
unsigned char TaskTableBuf=0;//�Ͳ����Ż�����
unsigned char GetCtrAreaStatusFlag=0;//�õ�������״̬��־λ
void TIM5_IRQHandler(void)
{
	if(TIM5->SR&0X0001)//����ж�
	{
		unsigned short  tempdata=0;
		unsigned short  crccheck=0;
		unsigned char txbuf[5]={0xFE,0x01,0x01};
		unsigned short crc16;
		TIM5->SR&=~(1<<0);//����жϱ�־λ
		TIM5_Dis();//�ض�ʱ��
		
		switch(TIM5_IRQSource)
		{
			/*	���ߴ���5�������ж�(������λ��������֡)	*/
			case WIRELESS_TRIG :
					tempdata = USART5_RX_STA&0x7F;//������Ч�����ֽ���
					if(tempdata<2) break;
					crccheck = USART5_RX_BUF[tempdata-2]+(unsigned int)(USART5_RX_BUF[tempdata-1]<<8);//����CRC16У��ֵ
					if(crccheck == CRC16(USART5_RX_BUF,tempdata-2))//CRCУ��ɹ�
					{
						if(USART5_RX_BUF[0]==0xFE && USART5_RX_BUF[1]==0x01 && USART5_RX_BUF[2]==0x01)			//���յ���һ̨���Ѿ���������Ϣ
						{
							crc16=CRC16(txbuf,3);
							txbuf[3]=crc16;
							txbuf[4]=crc16>>8;
							USART1_Send(txbuf,5);
						}
					}
					
		}
		USART5_RX_STA  = 0;
		TIM5_IRQSource=NULL;				//��ձ�־
	}
}
//===========================================================================================================================//
//===========================================================================================================================//
/*********************************************
	ͨ�ö�ʱ��2�жϳ�ʼ��
	����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36MHz
	arr���Զ���װֵ��
	psc��ʱ��Ԥ��Ƶ��
*********************************************/
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2ʱ��ʹ�� 
 	TIM2->ARR=arr;  		//�趨�������Զ���װֵ
	TIM2->PSC=psc;  		//Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��	
	
	/*����һ�������¼�,ʹԤת�ؼĴ��������ݴ�����Ӱ�ӼĴ���,
		ȷ����һ��RS485�������ݶ�ʱʱ���׼ȷ��.*/
	TIM2_En();
	TIM2->EGR|=1<<0;		//����һ�������¼�
	TIM2_Dis();
	
	TIM2->DIER|=1<<0;   //��������ж�
  MY_NVIC_Init(2,3,TIM2_IRQn,2);//��ռ2,�����ȼ�3,��2
	
	TIM2_Dis();
}
/*********************************************
	�嶨ʱ��2�ļ�����
*********************************************/
void TIM2_ClrCont( void )
{
	TIM2->CNT=0;    //�嶨ʱ��2�ļ�����
}
/*********************************************
	ʹ��ͨ�ö�ʱ��2
*********************************************/
void TIM2_En( void )
{
	TIM2->CR1|=0x01;    //ʹ�ܶ�ʱ��2
}
/*********************************************
	��ֹͨ�ö�ʱ��2
*********************************************/
void TIM2_Dis( void )
{
	TIM2->CR1&=~0x01;   //��ֹ��ʱ��2
}
/*********************************************
	�������ƣ�void TIM2_IRQHandler(void)��ʱ��2�жϷ������,5ms�ж�.
	��		�ܣ�HMI�����豸����1�����ж�,���յ�Modbus����
	��		�Σ���
	��		�أ���
*********************************************/
void TIM2_IRQHandler(void)
{
	if(TIM2->SR&0X0001)//����ж�
	{
		unsigned char data=0;
		unsigned int  tempdata=0;			//�ֽڸ���
		unsigned int  tempreg	=0;
		unsigned int  tempaddr=0;
		unsigned int  tempnum	=0;
		unsigned int  templen =0;
		unsigned int  crccheck=0;			//У����
		static unsigned char flag[100];
		unsigned short tempregaddr;
		unsigned short tempregdata;
		unsigned char  txbuf[8];
		unsigned char  txbuf1[25];
		unsigned short crc16;
		unsigned char index;
		unsigned char txerr_03[]={0x01, 0x83, 0x03, 0x01, 0x31};//0x03���󷵻���
		unsigned char txerr_06[]={0x01, 0x86, 0x03, 0x02, 0x61};//0x06���󷵻���
		TIM2->SR&=~(1<<0);//����жϱ�־λ
		TIM2_Dis();				//�ض�ʱ��
		
		switch( TIM2_IRQSource )
		{
			
			
			/* HMI�����豸����1�����ж�,���յ�Modbus���� */
			//0x01 0x04 xx yy aa bb CRC16��8λ CRC16��8λ
			//0xxxyy: ��ʼ�Ĵ�����ַ
			//oxaabb: �Ĵ�������
			case HMI_TRIG :
					tempdata = USART1_RX_STA&0x7F;//������Ч�����ֽ���
					if(tempdata<3) break;
					crccheck = USART1_RX_BUF[tempdata-2]+(unsigned int)(USART1_RX_BUF[tempdata-1]<<8);//����CRC16У��ֵ
//					if(crccheck!=CRC16(USART1_RX_BUF,tempdata-2))
//					{	
//						if(USART1_RX_BUF[1]==0x03) USART1_Send(txerr_03, 5);
//						if(USART1_RX_BUF[1]==0x06) USART1_Send(txerr_06, 5);
//						return;				
//					}
					if(crccheck == CRC16(USART1_RX_BUF,tempdata-2))//CRCУ��ɹ�
					{
						switch(USART1_RX_BUF[1])
						{
							case 0x03:
								tempregaddr = ((unsigned short)USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];
								tempnum=((unsigned short)USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];			//�Ĵ�������
							    if(tempnum==20)
								{
									if(tempregaddr==13)									//��ȡվ��ͣ��ģʽ
									{
										txbuf1[0]=0x01;
										txbuf1[1]=0x03;
										txbuf1[2]=20;
										for(index=0;index<20;index++)
										{
											txbuf1[3+index]=Station_modeRead(index);
										}
										crc16=CRC16(txbuf1,23);
										txbuf1[23]=crc16;
										txbuf1[24]=crc16>>8;
										
										USART1_Send(txbuf1,25);
									}
									if(tempregaddr==14)									//��ȡվ��ͣ��ʱ��
									{
										txbuf1[0]=0x01;
										txbuf1[1]=0x03;
										txbuf1[2]=20;
										for(index=0;index<20;index++)
										{
											txbuf1[3+index]=Station_timRead(index);
										}
										crc16=CRC16(txbuf1,23);
										txbuf1[23]=crc16;
										txbuf1[24]=crc16>>8;
										
										USART1_Send(txbuf1,25);
									}
								
									
								}
								else if(tempnum==10)
								{
									if(tempregaddr==15)
									{
										txbuf1[0]=0x01;
										txbuf1[1]=0x03;
										txbuf1[2]=10;
										for(index=0;index<10;index++)
										{
											txbuf1[3+index]=Cross_Read(index);
										}
										crc16=CRC16(txbuf1,13);
										txbuf1[13]=crc16;
										txbuf1[14]=crc16>>8;
										
										USART1_Send(txbuf1,15);
									}
								}
								else
								{
									tempregdata = ReadRegData(tempregaddr);
									
									txbuf[0] = 0x01;
									txbuf[1] = 0x03;
									txbuf[2] = 2;
									txbuf[3] = tempregdata>>8;
									txbuf[4] = tempregdata;
									crc16    = CRC16(txbuf, 5);
									txbuf[5] = crc16;
									txbuf[6] = crc16>>8;
									
									USART1_Send(txbuf, 7);
								}
								
								break;
								case 0x06://д�����Ĵ���������
								tempregaddr = ((unsigned short)USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];
								tempregdata = ((unsigned short)USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];
								WriteRegData(tempregaddr, tempregdata);
								
								USART1_Send(USART1_RX_BUF, tempdata);
								break;
							case 0x16://д�����Ĵ���
								tempregaddr = ((unsigned short)USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];
								tempregdata = ((unsigned short)USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];
								Recive_num=tempdata;
								for(index=0;index<Recive_num;index++)
								{
									Recive_data[index]=USART1_RX_BUF[index];
								}
								if(tempregaddr==13&&tempregdata==20)
								{
									STATMOD_FLAG=1;
									
								}
								else if(tempregaddr==14&&tempregdata==20)
								{
									STATIM_FLAG=1;
								}
								else if(tempregaddr==15&&tempregdata==10)
								{
									CROSS_FLAG=1;
								}
								break;
							default:
								break;
							
						}
						
					}
//						switch( USART1_RX_BUF[1] )
//						{
//							/*	���Ĵ���	*/
//							case 0x04:
//								tempaddr	= (USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];//��ʼ�Ĵ�����ַ
//								tempnum		= (USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];//�Ĵ�������
//								templen		= 3+tempnum*2+2;						//����ָ����ֽ���
//								
//								USART1_TX_BUF[0] = USART1_RX_BUF[0];	//�ӻ���ַ
//								USART1_TX_BUF[1] = 0x04;							//������
//								USART1_TX_BUF[2] = tempnum*2;					//�Ĵ��������ֽڸ���
//								
//								tempdata	= 3;
//								if( tempaddr == 1){//�Ĵ���1-33 ������ʱ�� ģʽ �ٶ� ������ģʽ(0:�й�,1:�޹�) �޹����
//										for( data=1;data<=10;data++ )
//										{
//											tempreg = VoiceTimeRead( data );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//										}
//										for( data=1;data<=10;data++ )
//										{
//											tempreg = VoiceModRead( data );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//										}
//										for( data=1;data<=10;data++ )
//										{
//											tempreg = VoiceRunModRead( data );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//										}
//											//31���ٶ�
//											tempreg = SpeedModRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//											//32��������ģʽ(0:�й�,1:�޹�)
//											tempreg = RobotModTrackRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//											//33���޹����
//											tempreg = TracklessDistRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//								}else if( tempaddr == 202 ){//�Ĵ���202 ����ָ��
//										USART1_TX_BUF[tempdata++] = 0x00;
//										USART1_TX_BUF[tempdata++] = Task.command;
//								}
//								
//								tempdata = CRC16(USART1_TX_BUF,templen-2);	//CRC16У��
//								USART1_TX_BUF[templen-2] = tempdata;				//CRC16У����ֽ�
//								USART1_TX_BUF[templen-1] = tempdata>>8;			//CRC16У����ֽ�
//								break;
//							/*	д�Ĵ���	*/
//							case 0x06:
//								tempaddr	= (USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];	//�Ĵ�����ַ
//								tempreg		= (USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];	//�Ĵ�������
//								templen		= tempdata;																	//����ָ����ֽ���
//								
//								if( tempaddr <= 10 ){//�Ĵ���1-10 ����ʱ��
//									VoiceTimeSet( tempaddr, tempreg );	
//								}else if( tempaddr <= 20 ){//�Ĵ���11-20 ����ģʽ
//									if(tempreg<=VOICEMOD_MULLOOP)
//										VoiceModSet( tempaddr-10, tempreg );
//								}else if( tempaddr <= 30 ){//�Ĵ���21-30 ����ģʽ
//									if(tempreg<=VOICERUNMOD_6)
//										VoiceRunModSet( tempaddr-20, tempreg );
//								}else if( tempaddr == 31 ){//�Ĵ���31 �ٶ�ģʽ
//									if( tempreg<=2 )
//									{	SpeedModSet(tempreg); }
//								}else if( tempaddr == 32 ){//�Ĵ���32 ������ģʽ(0:�й�,1:�޹�)
//									if( tempreg<=1 )
//									{	RobotModTrackSet(tempreg); }
//								}else if( tempaddr == 33 ){//�Ĵ���33 �޹����
//									if( tempreg<=9 )
//									{	TracklessDistSet(tempreg); }
//								}else if( tempaddr == 202 ){//�Ĵ���202 �ն˿���ָ��
//										// ------------------------------------------------------
//										/*	����ָ����������	*/
//										switch( tempreg )
//										{
//											case Com_Send:
//// 												if( Task.tasktable == 0 ) 			break;//����Ϊ0,����Ч���˳�
//												if( Task.taskstatus != TaskNo ) break;//�Ѿ����ù�����
//												CommandReceiveStatus = OK;
//												/* ================================
//												��������״̬(�ṹ�����Task)����ȡ����·����Ϣ(�ṹ�����RouteMsg)
//												��������ɺ�, ���������������.��
//												================================ */
//												WriteContrCommand(Com_Send);
//											
//// 												RouteMsg.robotid = RobotIDRead( );
//// 												RouteMsg.tabledir= TableDIRRead( Task.tasktable );
//// 												RouteMsg.route_l = RouteLeftRead( Task.tasktable ) - 100;
//// 												RouteMsg.route_r = RouteRightRead( Task.tasktable );
//											break;
//											case Com_Stop:
//												CommandReceiveStatus = OK;
//												/* ================================
//												��������״̬(�ṹ�����Task)
//												================================ */
//												WriteContrCommand(Com_Stop);
//												
//											break;
//											case Com_Back:
//												CommandReceiveStatus = OK;
//												/* ================================
//												��������״̬(�ṹ�����Task)
//												================================ */
//												WriteContrCommand(Com_Back);
//												
//											break;
//											default :
//												break;
//										}
//								}else if( tempaddr == 205 ){//�Ĵ���205
//									if( tempreg<=2 )
//									{	SpeedModSet(tempreg); }
//								}
//								
//								USART1_TX_BUF[0] = USART1_RX_BUF[0];	//�ӻ���ַ
//								USART1_TX_BUF[1] = USART1_RX_BUF[1];	//������
//								USART1_TX_BUF[2] = USART1_RX_BUF[2];	//�Ĵ�����ַ���ֽ�
//								USART1_TX_BUF[3] = USART1_RX_BUF[3];	//�Ĵ�����ַ���ֽ�
//								USART1_TX_BUF[4] = USART1_RX_BUF[4];	//�Ĵ������ݸ��ֽ�
//								USART1_TX_BUF[5] = USART1_RX_BUF[5];	//�Ĵ������ݵ��ֽ�
//								USART1_TX_BUF[templen-2] = USART1_RX_BUF[templen-2];	//CRC16У����ֽ�
//								USART1_TX_BUF[templen-1] = USART1_RX_BUF[templen-1];	//CRC16У����ֽ�
//								break;
//						}
//						
//						USART1_Send(USART1_TX_BUF,templen);
//					}
//				break;
//			default :
//				break;
		}
		USART1_RX_STA=0;
		TIM2_IRQSource=NULL;				//��ձ�־
	}
}
//===========================================================================================================================//
//===========================================================================================================================//
/*********************************************
	ͨ�ö�ʱ��6�жϳ�ʼ��
	����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36MHz
	arr���Զ���װֵ��
	psc��ʱ��Ԥ��Ƶ��
*********************************************/
void TIM6_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<4;	//TIM6ʱ��ʹ�� 
 	TIM6->ARR=arr;  		//�趨�������Զ���װֵ
	TIM6->PSC=psc;  		//Ԥ��Ƶ��72,�õ�1MHz�ļ���ʱ��
	
	TIM6_En();
	TIM6->EGR|=1<<0;		//����һ�������¼�
	TIM6_Dis();
	
	TIM6->DIER|=1<<0;   //��������ж�
  MY_NVIC_Init(0,3,TIM6_IRQn,2);//��ռ0,�����ȼ�3,��2
	
	TIM6_Dis();
}
/*********************************************
	�嶨ʱ��6�ļ�����
*********************************************/
void TIM6_ClrCont( void )
{
	TIM6->CNT=0;    //�嶨ʱ��6�ļ�����
}
/*********************************************
	ʹ��ͨ�ö�ʱ��6
*********************************************/
void TIM6_En( void )
{
	TIM6->CR1|=0x01;    //ʹ�ܶ�ʱ��6
}
/*********************************************
	��ֹͨ�ö�ʱ��6
*********************************************/
void TIM6_Dis( void )
{
	TIM6->CR1&=~0x01;    //��ֹ��ʱ��6
}
/*********************************************
	�������ƣ�void TIM6_IRQHandler(void),��ʱ��6�жϷ������,5ms
	��		�ܣ�ID����(����4)���ճ�ʱ�ж�
	��		�Σ���
	��		�أ���
*********************************************/
void TIM6_IRQHandler(void)
{
	if(TIM6->SR&0X0001)//����ж�
	{
		unsigned short  tempdata=0;
		TIM6->SR&=~(1<<0);//����жϱ�־λ
		TIM6_Dis();//�ض�ʱ��

		/*	ID�����Ŷ�ȡ�ж�	*/
		/*	ID���ű����ʽ��13�ֽ�, 0x02 x x x x x x x x x x X 0x03	*/
		/*	XΪʮ�����ƿ���, XǰʮλΪʮ���ƿ���	*/
		/*	CK-G06������(����ǩ��ʱ���ڸ�Ӧ����,������������)	*/
		if( (0x24==USART4_RX_BUF[0]) && (0x23==USART4_RX_BUF[19]) )//������ȷ 
		{
				ID = 0;
				tempdata = 5;
				while( USART4_RX_BUF[tempdata] == '0' ){	tempdata++;	}
				while( tempdata <= 16 ){	ID = ID*10+USART4_RX_BUF[tempdata]-'0'; tempdata++;	}
				IDNumReceiveStatus=OK;		//���ݽ�����ɱ�־��1
		}else
		/*	EMϵ�ж�����	*/
		if((0x02==USART4_RX_BUF[0])&&(0x03==USART4_RX_BUF[12]))//������ȷ
		{
				ID = USART4_RX_BUF[11];
				IDNumReceiveStatus=OK;		//������ɱ�־��1
		}else{//���մ���
				ID = 0;	//����
				IDNumReceiveStatus=ERROR;	//���ݽ��ܴ���
		}
		USART4_RX_STA=0;
	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
