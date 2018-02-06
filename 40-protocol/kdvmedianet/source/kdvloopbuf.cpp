/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvLoopBuf.cpp
相关文件    : KdvLoopBuf.h
文件实现功能: CKdvLoopBuf Implementation
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
******************************************************************************/


#include "kdvloopbuf.h"

extern BOOL32 g_bUseMemPool;

//初始化类成员
CKdvLoopBuf::CKdvLoopBuf()
{
    m_pBuf=NULL;
    m_nBufLen=0;
    m_nUintBufLen=0;
    m_nReadPos=0;
    m_nWritePos=0;
    m_nSubLen=0;
}
//析构类成员
CKdvLoopBuf::~CKdvLoopBuf()
{
    FreeBuf();
}

/*=============================================================================
    函数名        Create
    功能        ：对类成员进行初始化，分配一些空间
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  nUintBufLen  环状缓冲单元长度
                  nUnitBufNum  环状缓冲单元个数
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvLoopBuf::Create (s32 nUintBufLen, s32 nUnitBufNum)
{
    FreeBuf();//复位

    m_nBufLen=(nUintBufLen+sizeof(s32))*nUnitBufNum;//加上有效长度单元
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
    函数名        Read
    功能        ：读取数据缓冲中的单元数据
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  pBuf [out]         缓冲
                  nBufSize [out]     缓冲长度
    返回值说明： 参见错误码定义
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
    //s32 nRealLen = *((s32 *)(m_pBuf+m_nReadPos));//取出数据长度
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

    //修改标识位
    m_nReadPos    += m_nUintBufLen;
    m_nSubLen    -= m_nUintBufLen;
    if(m_nReadPos >= m_nBufLen)
    {
        m_nReadPos = 0;
    }
    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    函数名        Write
    功能        ：读取循环缓冲中的单元数据
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  pBuf         写入的缓冲
                  nBufSize     写入的缓冲长度
    返回值说明： 参见错误码定义
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

    //*((s32 *)(m_pBuf+m_nWritePos)) = nBufSize;//写入数据长度
    memcpy((m_pBuf+m_nWritePos), &nBufSize, sizeof(s32));
    memcpy(m_pBuf + m_nWritePos + sizeof(s32), pBuf, nBufSize);

    //修改标识位
    m_nWritePos  += m_nUintBufLen;
    m_nSubLen    += m_nUintBufLen;
    if(m_nWritePos >= m_nBufLen)
    {
        m_nWritePos = 0;
    }
    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    函数名        FreeBuf
    功能        ：复位类成员
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无

    返回值说明： 参见错误码定义
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
