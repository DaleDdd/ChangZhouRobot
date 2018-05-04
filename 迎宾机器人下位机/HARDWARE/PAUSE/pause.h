#ifndef __PAUSE_H
#define __PAUSE_H
#include "sys.h"

#define Key_Pause	PCin(2)				//暂停按钮,自锁,常闭
extern unsigned char PauseFlag;	//暂停按钮按下标志

void PauseKey_Init( void );
#endif

