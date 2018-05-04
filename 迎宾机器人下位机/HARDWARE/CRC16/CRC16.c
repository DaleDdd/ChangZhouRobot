#include "crc16.h"

/*********************************************
	puchMsg:		用于计算CRC的报文
	usDataLen:	报文中的字节数
*********************************************/
unsigned short CRC16(unsigned char *puchMsg,  /* 用于计算CRC的报文 */
										 unsigned short usDataLen)/* 报文中的字节数 */
																							/* 函数以unsigned short 类型返回CRC */
{
	unsigned char uchCRCHi = 0xFF; /* CRC的高字节初始化 */
	unsigned char uchCRCLo = 0xFF; /* CRC的低字节初始化 */
	unsigned uIndex ; 						 /* CRC查询表索引 */
	while (usDataLen--) 					 /* 完成整个报文缓冲区 */
	{
		uIndex = uchCRCLo ^ *puchMsg++; /* 计算CRC */
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
}

