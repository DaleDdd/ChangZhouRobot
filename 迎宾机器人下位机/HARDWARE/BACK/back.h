#ifndef _BACK_H
#define _BACK_H
#include "sys.h"

#define BackFlag_NULL			((unsigned char)0x00)
#define BackFlag_WAIT			((unsigned char)0x01)
#define BackFlag_BACK			((unsigned char)0x02)

#define BackButton1	PCin(2)		//返回按钮输入引脚, 常开(高电平), 按下后为低电平
#define BackButton2	PCin(3)		//返回按钮输入引脚, 常开(高电平), 按下后为低电平
extern unsigned char BackFlag;//送餐返回标志, 1:按下返回按钮; 0:返回按钮松开
void BackButton_Init( void );	//端口初始化
#endif
