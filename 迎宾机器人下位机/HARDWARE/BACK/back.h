#ifndef _BACK_H
#define _BACK_H
#include "sys.h"

#define BackFlag_NULL			((unsigned char)0x00)
#define BackFlag_WAIT			((unsigned char)0x01)
#define BackFlag_BACK			((unsigned char)0x02)

#define BackButton1	PCin(2)		//���ذ�ť��������, ����(�ߵ�ƽ), ���º�Ϊ�͵�ƽ
#define BackButton2	PCin(3)		//���ذ�ť��������, ����(�ߵ�ƽ), ���º�Ϊ�͵�ƽ
extern unsigned char BackFlag;//�Ͳͷ��ر�־, 1:���·��ذ�ť; 0:���ذ�ť�ɿ�
void BackButton_Init( void );	//�˿ڳ�ʼ��
#endif
