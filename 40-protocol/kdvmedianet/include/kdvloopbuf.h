/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvLoopBuf.h
����ļ�    : KdvLoopBuf.cpp
�ļ�ʵ�ֹ���: CKdvLoopBuf �ඨ��
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
******************************************************************************/


#ifndef _KDVLOOPBUF_0605_H_
#define _KDVLOOPBUF_0605_H_

#include "common.h"

#define    LOOPBUF_NO_ERROR                   (u16)0
#define    ERROR_LOOP_BUF_BASE                (u16)15000
#define    ERROR_LOOP_BUF_PARAM               (ERROR_LOOP_BUF_BASE+1)//���û�״�����������
#define    ERROR_LOOP_BUF_NULL                (ERROR_LOOP_BUF_BASE+2)//��״�������Ч���ݿ�
#define    ERROR_LOOP_BUF_FULL                (ERROR_LOOP_BUF_BASE+3)//��״�������Ч������ 
#define    ERROR_LOOP_BUF_NOCREATE            (ERROR_LOOP_BUF_BASE+4)//��״�������û�д���
#define    ERROR_LOOP_BUF_SIZE                (ERROR_LOOP_BUF_BASE+5)//��״�����е����ݵ�Ԫ��Ч���ȳ���
#define    ERROR_LOOP_BUF_MEMORY              (ERROR_LOOP_BUF_BASE+6)//��״�����е��ڴ��������

class CKdvLoopBuf
{
public:
    CKdvLoopBuf();
    ~CKdvLoopBuf();
public:
    u16 Create (s32 nUnitBufLen, s32 nUnitBufNum);
    u16 Read (u8 *pBuf, s32 &nBufSize);
    u16 Write (u8 *pBuf, s32 nBufSize);
    u16 FreeBuf();
private:
    u8  *m_pBuf;        //����洢���ݵĻ���
    s32  m_nBufLen;     //�������ݻ��峤��
    s32  m_nUintBufLen;   //���ݻ��巢��Ԫ����
    s32  m_nReadPos;     //���ݻ���Ķ�λ��
    s32  m_nWritePos;    //���ݻ����дλ��
    s32  m_nSubLen;     //��Ч���ݳ���
};



#endif // _KDVLOOPBUF_0605_H_
