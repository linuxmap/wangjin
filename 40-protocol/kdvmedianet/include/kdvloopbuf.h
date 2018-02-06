/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvLoopBuf.h
相关文件    : KdvLoopBuf.cpp
文件实现功能: CKdvLoopBuf 类定义
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
******************************************************************************/


#ifndef _KDVLOOPBUF_0605_H_
#define _KDVLOOPBUF_0605_H_

#include "common.h"

#define    LOOPBUF_NO_ERROR                   (u16)0
#define    ERROR_LOOP_BUF_BASE                (u16)15000
#define    ERROR_LOOP_BUF_PARAM               (ERROR_LOOP_BUF_BASE+1)//设置环状缓冲参数出错
#define    ERROR_LOOP_BUF_NULL                (ERROR_LOOP_BUF_BASE+2)//环状缓冲的有效数据空
#define    ERROR_LOOP_BUF_FULL                (ERROR_LOOP_BUF_BASE+3)//环状缓冲的有效数据满 
#define    ERROR_LOOP_BUF_NOCREATE            (ERROR_LOOP_BUF_BASE+4)//环状缓冲对象没有创建
#define    ERROR_LOOP_BUF_SIZE                (ERROR_LOOP_BUF_BASE+5)//环状缓冲中的数据单元有效长度出错
#define    ERROR_LOOP_BUF_MEMORY              (ERROR_LOOP_BUF_BASE+6)//环状缓冲中的内存操作出错

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
    u8  *m_pBuf;        //用语存储数据的缓冲
    s32  m_nBufLen;     //整个数据缓冲长度
    s32  m_nUintBufLen;   //数据缓冲发单元长度
    s32  m_nReadPos;     //数据缓冲的读位置
    s32  m_nWritePos;    //数据缓冲的写位置
    s32  m_nSubLen;     //有效数据长度
};



#endif // _KDVLOOPBUF_0605_H_
