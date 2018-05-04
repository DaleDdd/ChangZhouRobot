/*---------------------------------------------------------------------------------------
	STM32F103ZET6大容量主存储块(FLASH程序存储器)地址:0x08000000-0x0807FFFF,页(sector)大小为2KByte.
	程序存储器分配:
			0x08000000-0x0807D7FF:502K,用户程序存储区
			0x0807D800-0x0807DFFF:	2K,用户控制变量存储区
			0x0807E000-0x0807FFFF:	8K,ID卡卡号、路径等信息存储区
	说明:卡号相关信息只可进行设置或修改, 但设置后不可删除, 若要进行删除必须全部檫除进行重新设置.
																					<完>
---------------------------------------------------------------------------------------*/
#include "include.h"



