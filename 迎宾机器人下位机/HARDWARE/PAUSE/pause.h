#ifndef __PAUSE_H
#define __PAUSE_H
#include "sys.h"

#define Key_Pause	PCin(2)				//��ͣ��ť,����,����
extern unsigned char PauseFlag;	//��ͣ��ť���±�־

void PauseKey_Init( void );
#endif

