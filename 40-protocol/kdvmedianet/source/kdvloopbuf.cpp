/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvLoopBuf.cpp
����ļ�    : KdvLoopBuf.h
�ļ�ʵ�ֹ���: CKdvLoopBuf Implementation
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
******************************************************************************/


#include "kdvloopbuf.h"

extern BOOL32 g_bUseMemPool;

//��ʼ�����Ա
CKdvLoopBuf::CKdvLoopBuf()
{
    m_pBuf=NULL;
    m_nBufLen=0;
    m_nUintBufLen=0;
    m_nReadPos=0;
    m_nWritePos=0;
    m_nSubLen=0;
}
//�������Ա
CKdvLoopBuf::~CKdvLoopBuf()
{
    FreeBuf();
}

/*=============================================================================
    ������        Create
    ����        �������Ա���г�ʼ��������һЩ�ռ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  nUintBufLen  ��״���嵥Ԫ����
                  nUnitBufNum  ��״���嵥Ԫ����
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvLoopBuf::Create (s32 nUintBufLen, s32 nUnitBufNum)
{
    FreeBuf();//��λ

    m_nBufLen=(nUintBufLen+sizeof(s32))*nUnitBufNum;//������Ч���ȵ�Ԫ
    if(m_nBufLen<=0)
    {
        return ERROR_LOOP_BUF_PARAM;
    }
    MEDIANET_MALLOC(m_pBuf, m_nBufLen);
    if(m_pBuf == NULL)
    {
        return ERROR_LOOP_BUF_MEMORY;
    }
    memset(m_pBuf, 0, m_nBufLen);
    m_nUintBufLen=nUintBufLen+sizeof(s32);

    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    ������        Read
    ����        ����ȡ���ݻ����еĵ�Ԫ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  pBuf [out]         ����
                  nBufSize [out]     ���峤��
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvLoopBuf::Read (u8 *pBuf, s32 &nBufSize)
{

    if(m_pBuf == NULL)
    {
        return ERROR_LOOP_BUF_NOCREATE;
    }
    if(pBuf == NULL)
    {
        return ERROR_LOOP_BUF_PARAM;
    }
    if(m_nSubLen < m_nUintBufLen)
    {
        return ERROR_LOOP_BUF_NULL;
    }
    //s32 nRealLen = *((s32 *)(m_pBuf+m_nReadPos));//ȡ�����ݳ���
    s32 nRealLen;
    memcpy(&nRealLen, m_pBuf+m_nReadPos, sizeof(s32));
    if(nRealLen < 0|| nRealLen > (m_nUintBufLen - (s32)sizeof(s32)))
    {
        //hual debug
        //printf("[debug0623]readlen=%d, sublen=%d, readpos=%d writepos=%d\n",
        //        nRealLen, m_nSubLen, m_nReadPos, m_nWritePos);

        OspPrintf(1 , 0 , "[debug0623]readlen=%d, sublen=%d, readpos=%d writepos=%d\n",
                nRealLen, m_nSubLen, m_nReadPos, m_nWritePos);

        return ERROR_LOOP_BUF_SIZE;
    }

    memcpy(pBuf, m_pBuf + m_nReadPos + sizeof(s32), nRealLen);
    nBufSize = nRealLen;

    //�޸ı�ʶλ
    m_nReadPos    += m_nUintBufLen;
    m_nSubLen    -= m_nUintBufLen;
    if(m_nReadPos >= m_nBufLen)
    {
        m_nReadPos = 0;
    }
    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    ������        Write
    ����        ����ȡѭ�������еĵ�Ԫ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  pBuf         д��Ļ���
                  nBufSize     д��Ļ��峤��
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvLoopBuf::Write (u8 *pBuf, s32 nBufSize)
{

    if(m_pBuf == NULL)
    {
        return ERROR_LOOP_BUF_NOCREATE;
    }
    if(pBuf == NULL|| nBufSize > (m_nUintBufLen -(s32)sizeof(s32)))
    {
        return ERROR_LOOP_BUF_PARAM;
    }
    if(m_nSubLen >= m_nBufLen)
    {
        return ERROR_LOOP_BUF_FULL;
    }

    //*((s32 *)(m_pBuf+m_nWritePos)) = nBufSize;//д�����ݳ���
    memcpy((m_pBuf+m_nWritePos), &nBufSize, sizeof(s32));
    memcpy(m_pBuf + m_nWritePos + sizeof(s32), pBuf, nBufSize);

    //�޸ı�ʶλ
    m_nWritePos  += m_nUintBufLen;
    m_nSubLen    += m_nUintBufLen;
    if(m_nWritePos >= m_nBufLen)
    {
        m_nWritePos = 0;
    }
    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    ������        FreeBuf
    ����        ����λ���Ա
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvLoopBuf::FreeBuf()
{
    MEDIANET_SAFE_FREE(m_pBuf)
    m_nBufLen        =0;
    m_nUintBufLen    =0;
    m_nReadPos        =0;
    m_nWritePos        =0;
    m_nSubLen        =0;
    return LOOPBUF_NO_ERROR;
}
