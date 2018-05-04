#include "crc16.h"

/*********************************************
	puchMsg:		���ڼ���CRC�ı���
	usDataLen:	�����е��ֽ���
*********************************************/
unsigned short CRC16(unsigned char *puchMsg,  /* ���ڼ���CRC�ı��� */
										 unsigned short usDataLen)/* �����е��ֽ��� */
																							/* ������unsigned short ���ͷ���CRC */
{
	unsigned char uchCRCHi = 0xFF; /* CRC�ĸ��ֽڳ�ʼ�� */
	unsigned char uchCRCLo = 0xFF; /* CRC�ĵ��ֽڳ�ʼ�� */
	unsigned uIndex ; 						 /* CRC��ѯ������ */
	while (usDataLen--) 					 /* ����������Ļ����� */
	{
		uIndex = uchCRCLo ^ *puchMsg++; /* ����CRC */
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
}

