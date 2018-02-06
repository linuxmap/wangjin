/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvRtp.h
����ļ�    : KdvRtp.cpp
�ļ�ʵ�ֹ���: CKdvRtp �ඨ��,��֧�ֶ��̵߳���.
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
******************************************************************************/

#if !defined(_KDVRTP_0603_H_)
#define _KDVRTP_0603_H_

#ifndef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

#include "common.h"
#include "kdvrtcp.h"
#include "kdvsocket.h"
#include "kdvloopbuf.h"

typedef void (*PRCVPROC)(TRtpPack *ptRtpPack, void* pContext);
typedef void (*RAwRTPTRANSPROC)(u8 *pBuf, u32 dwSize);


class CKdvRtp
{
public:
    CKdvRtp();
    virtual ~CKdvRtp();
public:
    u16  Create(u32 dwSSRC, BOOL32 bAllocLPBuf = FALSE, BOOL32 bVidPayload = FALSE, u8 byBuffMultipleTimes = 1);
    u16  SetRtcp(CKdvRtcp *pRtcp);
    u16  SetCallBack(PRCVPROC pCallBackHandle, void* pContext);

    u16  SetLocalAddr(u32 dwIp, u16 wPort, BOOL32 bRcv=FALSE, u32 dwUserDataLen=0, u8* pbyUserData=NULL, u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL);
    u16  GetLocalAddr(u32* pdwIp, u16* pwPort);

    u16  RemoveLocalAddr();
    u16 InputRtpPack(u8 * pRtpBuf, u32 dwRtpBuf);
    u16  SetRemoteAddr(TRemoteAddr &tRemoteAddr);
    u16  GetRemoteAddr(TRemoteAddr &tRemoteAddr);

    u16  Write(TRtpPack &tRtpPack,BOOL32 bSend = FALSE, BOOL32 bTrans = FALSE, BOOL32 bSendRtp = TRUE);
    u16  SendByPackNum(s32 nPackNum);
    u16  SendBySize(s32 nTotalPackSize);
    void ResetSeq();
    void ResetSSRC(u32 dwSSRC);
    u16  ResetRSFlag(u16 wRLBUnitNum, BOOL32 bRepeatSnd);

    u16 SendUserDefinedBuff(u8 *pbyBuf, u16 wBufLen, u32 dwPeerAddr, u16 wPeerPort);

    u16  SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv = FALSE);
    u16  SetBCastOpt();

    void SetRawTransHandle(RAwRTPTRANSPROC pRawRtpTrans);

    u16 SetSrcAddr(u32 dwSrcIP, u16 wSrcPort);
    u16 GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo);

public:
    friend void RtpCallBackProc(u8 *pBuf, s32 nSize, void* pContext);
    void DealData(u8 *pBuf, s32 nSize);
    u16  DealRSQBackQuest(TRtcpSDESRSQ *ptRSQ);

private:
    u16  CheckPackAvailabe(const TRtpPack *ptRtpPack, s32 *pnHeadLen);
    u16  DirectSend(u8 *pPackBuf, s32 nPackLen, u32 dwTimeStamp);
    u16  SaveIntoLPBuf(u8 *pPackBuf, s32 nPackLen, BOOL32 bTrans);
    void FreeBuf();

    u16  CreateRLB(s32 nUnitBufNum);
    u16  WriteRLB(u16 wSequence, u32 dwTimestamp, u8 *pPacketBuf, s32 nPacketSize);
    u16  ReadRLBBySN(u8 *pBuf, s32 &nBufSize, u32 dwTimeStamp, u16 wSeqNum);
    s32  FindRLBByTS(u32 dwTimeStamp);


    typedef struct tagLRSPacketInfo
    {
        u32             m_dwTS;
        u16             m_wSN;
        u8*             m_pbyBuff;
        s32             n_nDataSize;
    } TLRSPacketInfo;

public:
    u16         m_wSeqNum;
    void*       m_pRlbPackStart;
    void*       m_pRlbPackEnd;
    void*       m_pRlbBufStart;
    void*       m_pRlbBufEnd;

private:
    CKdvRtcp    *m_pRtcp;
    CKdvSocket  *m_pSocket;
    CKdvSocket  *m_pAdditionMSocket;    //������鲥socket
    CKdvLoopBuf *m_pLoopBuf;

    TAddr        m_tLocalAddr;
    TRemoteAddr  m_tRemoteAddr;

    u32          m_dwSSRC;

    //RTP FIXED HEADER;
    u32          m_dwRtpHeader[RTP_FIXEDHEADER_SIZE/sizeof(u32)];
    u8          *m_pPackBuf;

    PRCVPROC     m_pCallBack;
    void*        m_pContext;

    BOOL32         m_bRepeatSend;  //�Ƿ��ط�
    SEMHANDLE    m_hSynSem;         //�����ط����λ���ķ��ʻ���
    u8          *m_pRSPackBuf;   //�ط������壬


    s32             m_nRLBUnitNum;  //�ط����λ�����С���ռ�����
    TLRSPacketInfo* m_atRLBPackets; //�ط����λ����а���Ϣ����
    u8*             m_pRLBBuf;    //�ط����λ����а�����
    u16             m_wRLBLastSN;   //�ط����λ�������һ�α������SN


    u32          m_dwTotalPackNum;  //���͵��ܵ�С����



    RAwRTPTRANSPROC     m_pRawRtpTrans;
};

#endif //!defined(_KDVRTP_0603_H_)
