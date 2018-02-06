/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvRtcp.cpp
相关文件    : KdvRtcp.h
文件实现功能: CKdvRtcp define
作者        : Jason
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         Jason      Create
2003/06/03  2.0         Jason      Add RTP Option
******************************************************************************/

#if !defined(_KDVRTCP_0603_H_)
#define _KDVRTCP_0603_H_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

#include "common.h"
#include "kdvsocket.h"
#include "kdvmedianet.h"

#ifdef WIN32
typedef  void * HANDLE;
#endif

#define FROM1900TILL1970          (u32)0x83AA7E80//
#define MAX_RTCP_PACK             (s32)1460//

#define MAX_DROPOUT               3000//
#define MAX_MISORDER              100//
#define MIN_SEQUENTIAL            2//
#define RTP_SEQ_MOD               0x10000//seq num cycle unit 65535

#define reduceNNTP(a) (((a).msdw<<16)+((a).lsdw>>16))

#define W32Len(l)  ((l + 3) / 4)  /* length in 32-bit words */

/* initial bit field value for RTCP headers: V=2,P=0,RC=0,PT=0,len=0 */
#define RTCP_HEADER_INIT          0x80000000

/* RTCP header bit locations - see the standard */
#define HEADER_V                  30      /* version                       */
#define HEADER_P                  29      /* padding                       */
#define HEADER_RC                 24      /* reception report count        */
#define HEADER_PT                 16      /* packet type                   */
#define HEADER_len                0       /* packet length in 32-bit words */

/* RTCP header bit field lengths - see the standard */
#define HDR_LEN_V                 2       /* version                       */
#define HDR_LEN_P                 1       /* padding                       */
#define HDR_LEN_RC                5       /* reception report count        */
#define HDR_LEN_PT                8       /* packet type                   */
#define HDR_LEN_len               16      /* packet length in 32-bit words */


/* used to overcome byte-allignment issues */
#define SIZEOF_RTCPHEADER         (sizeof(u32) * 2)// RTCP报文头 2字节
#define SIZEOF_SR                 (sizeof(u32) * 5)//  发送信息 5字节
#define SIZEOF_RR                 (sizeof(u32) * 6)//  接收报告块，每块为6字节

#define SIZEOF_SDES(sdes)         (((sdes).m_byLength + 6) & 0xfc)
                                   //32-bit boundary,6=1+1+1+3  ???

//参见RFC3550
typedef enum {
        RTCP_SDES_END   = 0,  // 0表示结束
        RTCP_SDES_CNAME = 1,
        RTCP_SDES_NAME  = 2,
        RTCP_SDES_EMAIL = 3,
        RTCP_SDES_PHONE = 4,
        RTCP_SDES_LOC   = 5,
        RTCP_SDES_TOOL  = 6,
        RTCP_SDES_NOTE  = 7,
        RTCP_SDES_PRIV  = 8
} TRtcpSDesType;

//重发请求类型：以SN+TIMESTAMP 或者只以TIMESTAMP请求
#define SN_RSQ_TYPE          (u8)0  //目前使用的方式
#define TIMESTAMP_RSQ_TYPE   (u8)1  //未使用
/*我们的重传包结构
++++++++++++++++++++++++++++++++++++++++++
             m_dwRtpIp
------------------------------------------
m_wRtpPort       |m_byRSQType|m_byPackNum    目前m_byRSQType=SN_RSQ_TYPE m_byPackNum=1
------------------------------------------
            m_wStartSeqNum
------------------------------------------
            m_dwTimeStamp                   = 0
------------------------------------------
             m_byMaskBit                   目前只用这个
------------------------------------------
             m_byMaskBit
------------------------------------------
             m_byMaskBit
++++++++++++++++++++++++++++++++++++++++++
                                   */
//NOTE RSQ SDES
//medianet是4字节对齐的，注意强转时的对齐问题
typedef struct tagRtpRSQ
{
    u8    m_byType;  //我们公司固定为NOTE-7
    u8    m_byLength; //only text length
  //u16   m_wMagic;                   //掩码位填充位（0xFEFE）
    u32   m_dwRtpIP;                  //与rtcp相对应的rtp接收ip   (网络序)
    u16   m_wRtpPort;                 //与rtcp相对应的rtp接收port (网络序)
    u8    m_byRSQType;                  //重发请求类型：以SN+TIMESTAMP 或者只以TIMESTAMP请求
    u8    m_byPackNum;                //重发请求帧中的总小包数（以SN+TIMESTAMP请求时有效）
    u16   m_wStartSeqNum;             //重发请求帧中的起始包序列
    u32   m_dwTimeStamp;              //重发请求帧的时间戳
    u8    m_byMaskBit[MAX_PACK_NUM/8];//重发请求帧中各包的重发掩码位，00010101 （1－需要重发, 0－不重发）
}
TRtcpSDESRSQ;

typedef struct tagMyInfo
{
    BOOL32    m_bActive;
    s32       m_nCollision;//和标准类型一致
    u32       m_dwSSRC;
    u32       m_dwTimestamp;
    TRtcpSR   m_tSR;
    TRtcpSDES m_tCName;
}TMyInfo;


typedef struct tagRtcpHeader
{
    u32   m_dwBits;
    u32   m_dwSSRC;
}TRtcpHeader;

typedef struct tagRtcpInfoList
{
    s32       m_nSessionNum;
    TRtcpInfo    m_tRtcpInfo[MAX_SESSION_NUM];
}TRtcpInfoList;

typedef struct tagBuffer
{
    u32  m_dwLen;
    u8  *m_pBuf;
}TBuffer;

class CKdvRtp;

class CKdvRtcp
{
public:
    CKdvRtcp();
    virtual ~CKdvRtcp();
public:
    u16  Create(u32 dwSSRC);
    u16  SetRtp(CKdvRtp *pRtp);

    u16  SetLocalAddr(u32 dwIp, u16 wPort, u32 dwUserDataLen=0, u8* pbyUserData=NULL, u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL);
    u16  GetLocalAddr(u32* pdwIp, u16* pwPort);

    u16  RemoveLocalAddr();
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext);

    u16  SetRemoteAddr(TRemoteAddr &tRemoteAddr);
    u16  GetRemoteAddr(TRemoteAddr &tRemoteAddr);

    u16  UpdateSend(s32 nSendDataSize, u32 dwTimeStamp);
    u16  UpdateRcv(u32 dwSSRC, u32 LocalTimestamp,
                     u32 timestamp, u16 wSequence);
    void ResetSSRC(u32 dwSSRC);

    u16 SendUserDefinedBuff(u8 *pbyBuf, u16 wBufLen, u32 dwPeerAddr, u16 wPeerPort);

    u16  SendRSQ(TRtcpSDESRSQ &tRSQ);

    u16  SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv = FALSE);
    u16  SetBCastOpt();

    u16 SetSrcAddr(u32 dwSrcIP, u16 wSrcPort);

    u16 GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo);

public:
    friend void RtcpDataCallBack(u8 *pBuf, s32 nBufSize, void* pContext);
    void DealData(u8 *pBuf, s32 nBufSize);

    u16  DealTimer();

private:
    void    FreeBuf();

    void    CreateRTCPPacket(TBuffer   *ptBuf);
    void    InitSeq(TRtpSource *pRtpSource, u16 seq);
    BOOL32  UpdateSeq(TRtpSource *pRtpSource, u16 seq,
                      u32 dwArrivalTS, u32 dwTimeStamp);

    void    ProcessRTCPPacket(u8 *pData, s32 nDataLen,
                              TRtcpType type, s32  nRCount,
                              TUint64 myTime);
    TUint64            GetNNTPTime();
    TRtcpInfo      *GetRtcpInfo(u32 dwSSRC);
    TRtcpInfo      *AddRtcpInfo(TRtcpInfo &Info);
    TRtcpHeader        MakeHeader(u32 dwSSRC, u8 count,
                               TRtcpType type, u16 dataLen);

    BOOL32   BufAddToBuffer(TBuffer *pTo, TBuffer *pFrom, u32 Offset);
    BOOL32   BufValid(TBuffer *pBuf, u32 dwSize);
    TBuffer  BufCreate(void* pData, u32 dwSize);

    u32      GetLost(TRtpSource *pRtpSource);
    u32      GetJitter(TRtpSource *pRtpSource);
    u32      GetSequence(TRtpSource *pRtpSource);

    void     CreateCustomRTCPPacket(TBuffer *ptBuf, TRtcpSDESRSQ &tRSQ);
    void     ParseRSQ(TRtcpSDES* pSdes, u32 dwLen);

private:
    CKdvSocket   *m_pSocket;
    u8          *m_pbyBuf;

    TAddr         m_tLocalAddr;
    TRemoteAddr   m_tRemoteAddr;
    TMyInfo       m_tMyInfo;
    TRtcpInfoList m_tRtcpInfoList;


    PRTCPINFOCALLBACK  m_pRtcpInfoCallbackHandler;
    void *m_pContext;
    u32              m_nStartTime;
    u32           m_nStartMilliTime;

    CKdvRtp      *m_pRtp;
    u8          *m_pbyCustomBuf;
//    TRtcpSDESRSQ  m_tBackRSQ;     //重发请求包

    SEMHANDLE     m_hSem;
};

#endif // !defined(_KDVRTCP_0603_H_)
