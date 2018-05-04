#ifndef __WDG_H
#define __WDG_H
#include "sys.h"
#include "led.h"
void IWDG_Init(u8 prer,u16 rlr);
void IWDT_En( void );
void IWDG_Feed(void);
void WWDG_Init(u8 tr,u8 wr,u8 fprer);
void WWDG_Set_Counter(u8 cnt);
void WWDG_IRQHandler(void);
#endif

























