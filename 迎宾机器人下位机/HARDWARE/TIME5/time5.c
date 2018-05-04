/*==========================================================================
	定时器5初始化
	1.用定时器5作为串口接收一帧数据的5ms超时定时
	2.ID卡卡号接收的5ms超时定时
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
unsigned char TIM2_IRQSource	=NULL; //timer 2中断源标志
unsigned char TIM5_IRQSource	=NULL; //timer 5中断源标志
unsigned char TIM6_IRQSource	=NULL; //timer 6中断源标志
unsigned char STATMOD_FLAG=0;
unsigned char STATIM_FLAG=0;
unsigned char CROSS_FLAG=0;
unsigned char Recive_data[50];
unsigned char Recive_num=0;
																		 //ID_TRIG(1):LINE9上的外部中断触发的中断(维根码接收)
																		 //WIRELESS_TRIG(2):串口5触发的中断(接收上位机的数据帧)
																		 //HMI_TRIG(3):HMI人机接口触发中断(接收人机接口的Modbus命令,并返回命令响应)
const unsigned char ID_TRIG					=1;
const unsigned char WIRELESS_TRIG		=2;
const unsigned char HMI_TRIG				=3;
/*********************************************
	通用定时器5中断初始化
	这里时钟选择为APB1的2倍，而APB1为36MHz
	arr：自动重装值。
	psc：时钟预分频数
*********************************************/
void TIM5_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<3;	//TIM5时钟使能 
 	TIM5->ARR=arr;  		//设定计数器自动重装值
	TIM5->PSC=psc;  		//预分频器72,得到1MHz的计数时钟
	
	TIM5_En();
	TIM5->EGR|=1<<0;		//产生一个更新事件
	TIM5_Dis();
	
	TIM5->DIER|=1<<0;   //允许更新中断
	MY_NVIC_Init(2,3,TIM5_IRQn,2);//抢占2,子优先级3,组2
	
	TIM5_Dis();
}
/*********************************************
	清定时器5的计数器
*********************************************/
void TIM5_ClrCont( void )
{
	TIM5->CNT=0;    //清定时器4的计数器
}
/*********************************************
	使能通用定时器5
*********************************************/
void TIM5_En( void )
{
	TIM5->CR1|=0x01;    //使能定时器5
}
/*********************************************
	禁止通用定时器5
*********************************************/
void TIM5_Dis( void )
{
	TIM5->CR1&=~0x01;    //禁止定时器5
}
/*********************************************
	函数名称：void TIM5_IRQHandler(void),定时器5中断服务程序,5ms
	功		能：无线模块(串口5)数据接收超时中断,收发数据格式参见《上位机与机器人通讯协议》

	入		参：无
	返		回：无
*********************************************/
unsigned char CommandReceiveStatus=0;
unsigned char TaskTableBuf=0;//送餐桌号缓冲区
unsigned char GetCtrAreaStatusFlag=0;//得到管制区状态标志位
void TIM5_IRQHandler(void)
{
	if(TIM5->SR&0X0001)//溢出中断
	{
		unsigned short  tempdata=0;
		unsigned short  crccheck=0;
		unsigned char txbuf[5]={0xFE,0x01,0x01};
		unsigned short crc16;
		TIM5->SR&=~(1<<0);//清除中断标志位
		TIM5_Dis();//关定时器
		
		switch(TIM5_IRQSource)
		{
			/*	无线串口5触发的中断(接收上位机的数据帧)	*/
			case WIRELESS_TRIG :
					tempdata = USART5_RX_STA&0x7F;//计算有效数据字节数
					if(tempdata<2) break;
					crccheck = USART5_RX_BUF[tempdata-2]+(unsigned int)(USART5_RX_BUF[tempdata-1]<<8);//计算CRC16校验值
					if(crccheck == CRC16(USART5_RX_BUF,tempdata-2))//CRC校验成功
					{
						if(USART5_RX_BUF[0]==0xFE && USART5_RX_BUF[1]==0x01 && USART5_RX_BUF[2]==0x01)			//接收到另一台车已经启动的消息
						{
							crc16=CRC16(txbuf,3);
							txbuf[3]=crc16;
							txbuf[4]=crc16>>8;
							USART1_Send(txbuf,5);
						}
					}
					
		}
		USART5_RX_STA  = 0;
		TIM5_IRQSource=NULL;				//清空标志
	}
}
//===========================================================================================================================//
//===========================================================================================================================//
/*********************************************
	通用定时器2中断初始化
	这里时钟选择为APB1的2倍，而APB1为36MHz
	arr：自动重装值。
	psc：时钟预分频数
*********************************************/
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2时钟使能 
 	TIM2->ARR=arr;  		//设定计数器自动重装值
	TIM2->PSC=psc;  		//预分频器7200,得到10Khz的计数时钟	
	
	/*产生一个更新事件,使预转载寄存器的内容传送至影子寄存器,
		确保第一次RS485接收数据定时时间的准确性.*/
	TIM2_En();
	TIM2->EGR|=1<<0;		//产生一个更新事件
	TIM2_Dis();
	
	TIM2->DIER|=1<<0;   //允许更新中断
  MY_NVIC_Init(2,3,TIM2_IRQn,2);//抢占2,子优先级3,组2
	
	TIM2_Dis();
}
/*********************************************
	清定时器2的计数器
*********************************************/
void TIM2_ClrCont( void )
{
	TIM2->CNT=0;    //清定时器2的计数器
}
/*********************************************
	使能通用定时器2
*********************************************/
void TIM2_En( void )
{
	TIM2->CR1|=0x01;    //使能定时器2
}
/*********************************************
	禁止通用定时器2
*********************************************/
void TIM2_Dis( void )
{
	TIM2->CR1&=~0x01;   //禁止定时器2
}
/*********************************************
	函数名称：void TIM2_IRQHandler(void)定时器2中断服务程序,5ms中断.
	功		能：HMI主机设备串口1触发中断,接收到Modbus命令
	入		参：无
	返		回：无
*********************************************/
void TIM2_IRQHandler(void)
{
	if(TIM2->SR&0X0001)//溢出中断
	{
		unsigned char data=0;
		unsigned int  tempdata=0;			//字节个数
		unsigned int  tempreg	=0;
		unsigned int  tempaddr=0;
		unsigned int  tempnum	=0;
		unsigned int  templen =0;
		unsigned int  crccheck=0;			//校验码
		static unsigned char flag[100];
		unsigned short tempregaddr;
		unsigned short tempregdata;
		unsigned char  txbuf[8];
		unsigned char  txbuf1[25];
		unsigned short crc16;
		unsigned char index;
		unsigned char txerr_03[]={0x01, 0x83, 0x03, 0x01, 0x31};//0x03错误返回码
		unsigned char txerr_06[]={0x01, 0x86, 0x03, 0x02, 0x61};//0x06错误返回码
		TIM2->SR&=~(1<<0);//清除中断标志位
		TIM2_Dis();				//关定时器
		
		switch( TIM2_IRQSource )
		{
			
			
			/* HMI主机设备串口1触发中断,接收到Modbus命令 */
			//0x01 0x04 xx yy aa bb CRC16低8位 CRC16高8位
			//0xxxyy: 起始寄存器地址
			//oxaabb: 寄存器个数
			case HMI_TRIG :
					tempdata = USART1_RX_STA&0x7F;//计算有效数据字节数
					if(tempdata<3) break;
					crccheck = USART1_RX_BUF[tempdata-2]+(unsigned int)(USART1_RX_BUF[tempdata-1]<<8);//计算CRC16校验值
//					if(crccheck!=CRC16(USART1_RX_BUF,tempdata-2))
//					{	
//						if(USART1_RX_BUF[1]==0x03) USART1_Send(txerr_03, 5);
//						if(USART1_RX_BUF[1]==0x06) USART1_Send(txerr_06, 5);
//						return;				
//					}
					if(crccheck == CRC16(USART1_RX_BUF,tempdata-2))//CRC校验成功
					{
						switch(USART1_RX_BUF[1])
						{
							case 0x03:
								tempregaddr = ((unsigned short)USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];
								tempnum=((unsigned short)USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];			//寄存器个数
							    if(tempnum==20)
								{
									if(tempregaddr==13)									//获取站点停靠模式
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
									if(tempregaddr==14)									//获取站点停靠时间
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
								case 0x06://写单个寄存器功能码
								tempregaddr = ((unsigned short)USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];
								tempregdata = ((unsigned short)USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];
								WriteRegData(tempregaddr, tempregdata);
								
								USART1_Send(USART1_RX_BUF, tempdata);
								break;
							case 0x16://写连续寄存器
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
//							/*	读寄存器	*/
//							case 0x04:
//								tempaddr	= (USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];//起始寄存器地址
//								tempnum		= (USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];//寄存器个数
//								templen		= 3+tempnum*2+2;						//返回指令的字节数
//								
//								USART1_TX_BUF[0] = USART1_RX_BUF[0];	//从机地址
//								USART1_TX_BUF[1] = 0x04;							//功能码
//								USART1_TX_BUF[2] = tempnum*2;					//寄存器数据字节个数
//								
//								tempdata	= 3;
//								if( tempaddr == 1){//寄存器1-33 语音段时长 模式 速度 机器人模式(0:有轨,1:无轨) 无轨距离
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
//											//31：速度
//											tempreg = SpeedModRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//											//32：机器人模式(0:有轨,1:无轨)
//											tempreg = RobotModTrackRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//											//33：无轨距离
//											tempreg = TracklessDistRead( );
//											USART1_TX_BUF[tempdata++] = tempreg>>8;
//											USART1_TX_BUF[tempdata++] = tempreg;
//								}else if( tempaddr == 202 ){//寄存器202 控制指令
//										USART1_TX_BUF[tempdata++] = 0x00;
//										USART1_TX_BUF[tempdata++] = Task.command;
//								}
//								
//								tempdata = CRC16(USART1_TX_BUF,templen-2);	//CRC16校验
//								USART1_TX_BUF[templen-2] = tempdata;				//CRC16校验低字节
//								USART1_TX_BUF[templen-1] = tempdata>>8;			//CRC16校验高字节
//								break;
//							/*	写寄存器	*/
//							case 0x06:
//								tempaddr	= (USART1_RX_BUF[2]<<8) + USART1_RX_BUF[3];	//寄存器地址
//								tempreg		= (USART1_RX_BUF[4]<<8) + USART1_RX_BUF[5];	//寄存器数据
//								templen		= tempdata;																	//返回指令的字节数
//								
//								if( tempaddr <= 10 ){//寄存器1-10 语音时长
//									VoiceTimeSet( tempaddr, tempreg );	
//								}else if( tempaddr <= 20 ){//寄存器11-20 语音模式
//									if(tempreg<=VOICEMOD_MULLOOP)
//										VoiceModSet( tempaddr-10, tempreg );
//								}else if( tempaddr <= 30 ){//寄存器21-30 行走模式
//									if(tempreg<=VOICERUNMOD_6)
//										VoiceRunModSet( tempaddr-20, tempreg );
//								}else if( tempaddr == 31 ){//寄存器31 速度模式
//									if( tempreg<=2 )
//									{	SpeedModSet(tempreg); }
//								}else if( tempaddr == 32 ){//寄存器32 机器人模式(0:有轨,1:无轨)
//									if( tempreg<=1 )
//									{	RobotModTrackSet(tempreg); }
//								}else if( tempaddr == 33 ){//寄存器33 无轨距离
//									if( tempreg<=9 )
//									{	TracklessDistSet(tempreg); }
//								}else if( tempaddr == 202 ){//寄存器202 终端控制指令
//										// ------------------------------------------------------
//										/*	根据指令设置任务	*/
//										switch( tempreg )
//										{
//											case Com_Send:
//// 												if( Task.tasktable == 0 ) 			break;//桌号为0,则无效并退出
//												if( Task.taskstatus != TaskNo ) break;//已经设置过任务
//												CommandReceiveStatus = OK;
//												/* ================================
//												设置任务状态(结构体变量Task)、读取任务路径信息(结构体变量RouteMsg)
//												【任务完成后, 清空以下所有数据.】
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
//												设置任务状态(结构体变量Task)
//												================================ */
//												WriteContrCommand(Com_Stop);
//												
//											break;
//											case Com_Back:
//												CommandReceiveStatus = OK;
//												/* ================================
//												设置任务状态(结构体变量Task)
//												================================ */
//												WriteContrCommand(Com_Back);
//												
//											break;
//											default :
//												break;
//										}
//								}else if( tempaddr == 205 ){//寄存器205
//									if( tempreg<=2 )
//									{	SpeedModSet(tempreg); }
//								}
//								
//								USART1_TX_BUF[0] = USART1_RX_BUF[0];	//从机地址
//								USART1_TX_BUF[1] = USART1_RX_BUF[1];	//功能码
//								USART1_TX_BUF[2] = USART1_RX_BUF[2];	//寄存器地址高字节
//								USART1_TX_BUF[3] = USART1_RX_BUF[3];	//寄存器地址低字节
//								USART1_TX_BUF[4] = USART1_RX_BUF[4];	//寄存器数据高字节
//								USART1_TX_BUF[5] = USART1_RX_BUF[5];	//寄存器数据低字节
//								USART1_TX_BUF[templen-2] = USART1_RX_BUF[templen-2];	//CRC16校验低字节
//								USART1_TX_BUF[templen-1] = USART1_RX_BUF[templen-1];	//CRC16校验高字节
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
		TIM2_IRQSource=NULL;				//清空标志
	}
}
//===========================================================================================================================//
//===========================================================================================================================//
/*********************************************
	通用定时器6中断初始化
	这里时钟选择为APB1的2倍，而APB1为36MHz
	arr：自动重装值。
	psc：时钟预分频数
*********************************************/
void TIM6_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<4;	//TIM6时钟使能 
 	TIM6->ARR=arr;  		//设定计数器自动重装值
	TIM6->PSC=psc;  		//预分频器72,得到1MHz的计数时钟
	
	TIM6_En();
	TIM6->EGR|=1<<0;		//产生一个更新事件
	TIM6_Dis();
	
	TIM6->DIER|=1<<0;   //允许更新中断
  MY_NVIC_Init(0,3,TIM6_IRQn,2);//抢占0,子优先级3,组2
	
	TIM6_Dis();
}
/*********************************************
	清定时器6的计数器
*********************************************/
void TIM6_ClrCont( void )
{
	TIM6->CNT=0;    //清定时器6的计数器
}
/*********************************************
	使能通用定时器6
*********************************************/
void TIM6_En( void )
{
	TIM6->CR1|=0x01;    //使能定时器6
}
/*********************************************
	禁止通用定时器6
*********************************************/
void TIM6_Dis( void )
{
	TIM6->CR1&=~0x01;    //禁止定时器6
}
/*********************************************
	函数名称：void TIM6_IRQHandler(void),定时器6中断服务程序,5ms
	功		能：ID卡号(串口4)接收超时中断
	入		参：无
	返		回：无
*********************************************/
void TIM6_IRQHandler(void)
{
	if(TIM6->SR&0X0001)//溢出中断
	{
		unsigned short  tempdata=0;
		TIM6->SR&=~(1<<0);//清除中断标志位
		TIM6_Dis();//关定时器

		/*	ID卡卡号读取中断	*/
		/*	ID卡号编码格式：13字节, 0x02 x x x x x x x x x x X 0x03	*/
		/*	X为十六进制卡号, X前十位为十进制卡号	*/
		/*	CK-G06读卡器(若标签长时间在感应区内,则发送两次数据)	*/
		if( (0x24==USART4_RX_BUF[0]) && (0x23==USART4_RX_BUF[19]) )//接收正确 
		{
				ID = 0;
				tempdata = 5;
				while( USART4_RX_BUF[tempdata] == '0' ){	tempdata++;	}
				while( tempdata <= 16 ){	ID = ID*10+USART4_RX_BUF[tempdata]-'0'; tempdata++;	}
				IDNumReceiveStatus=OK;		//数据接受完成标志置1
		}else
		/*	EM系列读卡器	*/
		if((0x02==USART4_RX_BUF[0])&&(0x03==USART4_RX_BUF[12]))//接收正确
		{
				ID = USART4_RX_BUF[11];
				IDNumReceiveStatus=OK;		//接受完成标志置1
		}else{//接收错误
				ID = 0;	//清零
				IDNumReceiveStatus=ERROR;	//数据接受错误
		}
		USART4_RX_STA=0;
	}
}
//===========================================================================================================================//
//																					End Of File																																			 //
//===========================================================================================================================//
