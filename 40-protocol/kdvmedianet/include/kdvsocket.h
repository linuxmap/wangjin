/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvSocket.h
相关文件    : KdvSocket.cpp
文件实现功能: CKdvSocket 类定义,不支持多线程
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
******************************************************************************/

#if !defined(_KDVSOCKET_0605_H_)
#define _KDVSOCKET_0605_H_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

#include "common.h"
#ifdef WIN32
#include <ws2tcpip.h>
#else

#endif


#define MAX_NET_RCV_BUF_LEN        128*1024*8*4
#define MAX_NET_SND_BUF_LEN        128*1024*4*4

#define MIN_NET_BUF_LEN            64000

#define MAX_RCV_NUM    512
#define MAX_SND_NUM    1024

//记录加入组播组时 列举到的当前设置的所有有效ip
#define MAX_LOCAL_IP_NUM    16

#define SOCKET_TYPE_NULL      0//
#define SOCKET_TYPE_AUDIO     1//音频
#define SOCKET_TYPE_VIDEO     2//
#define SOCKET_TYPE_DATA      3//

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#endif
typedef void (*PCALLBACK)(u8 *pBuf, s32 nBufSize, void* pContext);//, TAddr &tRemoteAddr);

typedef struct tagCallBack
{
    PCALLBACK m_pCallBack;
    void*     m_pContext;
}TCallBack;

typedef struct tagCreateSock
{
    s32        m_nSocketType;
    u16        m_wSocketPort;
    u32        m_dwLocalAddr;
    u32        m_dwMultiAddr;
    BOOL32  m_bRcv;
    void*    m_pRegFunc;
    void*    m_pUnregFunc;
}TCreateSock;


//全局KdvMediaRcv列表结构
typedef struct tagMediaRcvList
{
    s32         m_nMediaRcvCount;               //接收对象总数
    CKdvMediaRcv   *m_tMediaRcvUnit[FD_SETSIZE];
}TMediaRcvList;

//全局KdvNetSnd列表结构
typedef struct tagMediaSndList
{
    s32         m_nMediaSndCount;               //发送对象总数
    CKdvMediaSnd   *m_tMediaSndUnit[MAX_SND_NUM];
}TMediaSndList;


typedef struct tagMedianetNetAddr
{
    u32 m_dwIp;//网络序
    u16 m_wPort;
}TMedianetNetAddr;

typedef struct tagMsgData
{
    u64  m_lluContext;
    u32  m_dwMode;
    u32  m_dwIndex;
}TMsgData;

BOOL32 IsMultiCastAddr(u32 dwIP);
BOOL32 IsBroadCastAddr(u32 dwIP);

class CKdvSocket
{
public:
    CKdvSocket();
    virtual ~CKdvSocket();

public:
    BOOL32 Create(s32 nSocketType, u16 wSocketPort,
                  u32 dwLocalAddr, u32 dwMultiAddr, BOOL32 bRcv = FALSE,
                  u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL);
    void   Close(BOOL32 bNotifyThread = FALSE);
    void   SetCallBack(PCALLBACK pCallBack, void* pContext);

    BOOL32 SetLocalUserData(u32 dwUserDataLen, u8* pbyUserData);

    u16    SendTo(u8 *pBuf,s32 nSize,u32 dwRemoteIp,u16 wRemotePort);

    void   SetSrcAddr(u32 dwSrcIP, u16 wSrcPort);

    void   GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo);

    u16 SendUserDefinedBuff(u8 *pBuf, s32 nSize, u32 dwRemoteIp, u16 wRemotePort);

public:
    friend  void* SocketRcvTaskProc(void * pParam);
    friend  void* RtcpSndTaskProc(void * pParam);
    friend    void* MedianetRpctrlCallBack(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);
    friend    void* MedianetU2ACallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);
    friend    void* MedianetDataSwitchCallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);
//    friend    void* MedianetTestCallback(u8* pPackBuf, u16 wPackLen, u32 dwDstAddr, u32 dwSrcAddr, u64 qwTimeStamp, u32 dwContext);

    BOOL32  Create(BOOL32 bSend = FALSE);
    void    CallBack(u8 *pBuf, s32 nBufSize);

    BOOL32  SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv = FALSE);
    BOOL32  SetBCastOpt();

    BOOL32  SetSockMediaType( u8 bySockMediaType );
    u8      GetSockMediaType();

public:
    void    PrintErrMsg(s8 *szErrMsg, BOOL32 bShowSockArg = TRUE);
private:
    BOOL32  IsMultiCastAddr(u32 dwIP);
    BOOL32  IsBroadCastAddr(u32 dwIP);
    BOOL32  ResetLocalIP();

public:
    SOCKHANDLE     m_hSocket;
    BOOL32         m_bMultiCast;
    SOCKADDR_IN  m_tAddrIn;
    u32          m_tdwIPList[MAX_LOCAL_IP_NUM];  //记录加入组播组时 列举到的当前设置的所有有效ip
    u16          m_dwIPNum;                      //记录加入组播组时 列举到ip 数目

    SEMHANDLE    m_hSynSem;           //用于套结子删除时的同步
    SEMHANDLE    m_hCreateSynSem;  //用于套结子创建时的同步
    BOOL32         m_bSuccess;
    BOOL32         m_bSendUsage;     //这个socket用于数据发送还是用于接收。

    TCreateSock  m_tCreateSock;
    TCallBack    m_tCallBack;

    s32             m_nSndBufSize;
    s32             m_nRcvBufSize;

private:
    BOOL32       m_bUseRawSend;       //是否使用 raw-socket 进行数据包投递
    TIPHdr       m_tIPHdr;
    TUDPHdr      m_tUDPHdr;
    s8           m_szRawPackBuf[MAX_SND_PACK_SIZE_BY_RAW_IP+12];
    u8           m_bySockMediaType; //媒体类型, 用于发送音频还是视频，类型不同TOS设置不一样

    u32          m_dwSrcIP;         //用于IP伪装时修改原地址  hual 2006-06-08
    u16          m_wSrcPort;

    u32          m_dwLocalUserDataLen;           //过代理时本地头长度
    u8           m_abyLocalUserData[MAX_USERDATA_LEN]; //过代理时的本地头数据 hual 2006-08-25
    u32                m_dwFlag;//标志位，指明从dataswtich,网络，rpctrl，下载等接收数据
    TMedianetNetAddr    m_tMedianetNetAddr;
};

#endif // !defined(_KDVSOCKET_0605_H_)
