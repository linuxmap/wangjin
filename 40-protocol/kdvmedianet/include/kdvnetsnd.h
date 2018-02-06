/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvNetSnd.h
相关文件    : KdvNetSnd.cpp
文件实现功能: CKdvNetSnd 类定义
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
******************************************************************************/

#if !defined(_KDVNETSND_0603_H_)
#define _KDVNETSND_0603_H_

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#endif

#include "common.h"
#include "kdvrtp.h"
#include "kdvrtcp.h"


class CKdvNetSnd
{
public:
    CKdvNetSnd();
    virtual ~CKdvNetSnd();

public:
    u16  Create ( u32 dwMaxFrameSize, u32 dwNetBand, u8 byFrameRate,
                  u8 byMediaType, u32 dwSSRC = 0 , TPSInfo *ptPSInfo = NULL);
    u16  SetNetSndParam (TNetSndParam tNetSndParam);
    u16  RemoveNetSndLocalParam();
    u16  SetSndInfo(u32 dwNetBand, u8 byFrameRate);
    u16  ResetFrameId ();
    u16  ResetSSRC(u32 dwSSRC = 0);
    u16  ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd);
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext);

    //设置 动态载荷的 Playload值
    u16  SetActivePT( u8 byLocalActivePT );
    //设置发送加密key字串
    u16  SetEncryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byEncryptMode);

    u16  Send(PFRAMEHDR pFrmHdr, BOOL32 bAvalid, BOOL32 bSendRtp);
    u16  Send(TRtpPack *pRtpPack, BOOL32 bTrans, BOOL32 bAvalid, BOOL32 bSendRtp);
    u16  GetStatus(TKdvSndStatus &tKdvSndStatus);
    u16  GetStatistics(TKdvSndStatistics &tKdvSndStatistics);

    u16  GetAdvancedSndInfo(TAdvancedSndInfo &tAdvancedSndInfo);

    u16  SelfTestSend (s32 nFrameLen, s32 nSndNum, s32 nSpan);
    u16  DealRtcpTimer();

    u16 SetSrcAddr(TNetSession tSrcNet);
    u16 GetSndSocketInfo(TKdvSndSocketInfo &tRtpSockInfo, TKdvSndSocketInfo &tRtcpSockInfo);
    u16 GetNetSndParam (TNetSndParam* ptNetSndParam);
	u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp);
	//发送 NAT 探测包
	u16 DealNatProbePack(u32 dwNowTs = 0);
	u16  SendPS(TspsFRAMEHDR *ptFrame);
    u16  SetAaclcSend(BOOL32 bNoHead);
protected:
    u16  SendExPack(PFRAMEHDR pFrmHdr, BOOL32 bAudio);
    u16  SendSdPack(PFRAMEHDR pFrmHdr);

    u16  SendH261Pack(PFRAMEHDR pFrmHdr);
    u16  SendH263Pack(PFRAMEHDR pFrmHdr);
    u16  SendH264Pack(PFRAMEHDR pFrmHdr);
    u16  SendH265Pack(PFRAMEHDR pFrmHdr);
    u16 SendH264Pack_Flag0001(PFRAMEHDR pFrmHdr);
    u16 SendH265Pack_Flag0001(PFRAMEHDR pFrmHdr);
    u16  SendDataPayloadPack(PFRAMEHDR pFrmHdr);

    u16  SendSdAudioPack(PFRAMEHDR pFrmHdr);
    u16  SendSdAACAudioPack(PFRAMEHDR pFrmHdr);

    u16  ParasH261Head(PFRAMEHDR pFrmHdr);
    u16  SendOneH261Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos);

    u16  ParasH263Head(PFRAMEHDR pFrmHdr);
    u16  SendOneH263Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos);

    u16  ParseH264Head(PFRAMEHDR pFrmHdr);
    u16  SendOneH264Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos);
    u16 SendOneH264Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf, u32 dwNaluLen, BOOL32 bMark);
    u16 SendOneH265Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf, u32 dwNaluLen, BOOL32 bMark);
    u16 SendOneH264Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pPackBuf, u32 dwNaluLen, BOOL32 bMark);
    u16 SendOneH265Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pbyNalu, u32 dwNaluLen, BOOL32 bMark);

    u16  SimulateConfuedAndRepeate(TRtpPack *pRtpPack);
    u16  IncreaseTimestamp(PFRAMEHDR pFrmHdr, BOOL32 bAvalid);

    void ParseH264RtpHead(u8 *pRtpPackBuf, s32 nRtpPackBufSize, TKdvH264Header *ptH264Header);
    BOOL32 DecodeH264SPS(TBitStream *s, TSeqParameterSetRBSP *sps, TKdvH264Header *ptH264Header);
    BOOL32 DecodeH264PPS(TBitStream *s, TPicParameterSetRBSP *pps, TKdvH264Header *ptH264Header);

    BOOL32 ParasH263PlusHead(u16 wH263PlusHeader, TH263PlusHeader *ptH263PlusHeader);

    u16  EncryptRtpData(TRtpPack *pRtpPack, BOOL32 bUseNewBuf=FALSE);

    void FreeBuf ();

    u16  SendPSFrame(PFRAMEHDR pFrmHdr);
    u16  SendPSPack(PFRAMEHDR pFrmHdr);

private:
    TH261HeaderList      *m_ptH261HeaderList;
    TH263HeaderList      *m_ptH263HeaderList;
    TH264HeaderList      *m_ptH264HeaderList;

    CKdvRtp     *m_pcRtp;
    CKdvRtcp *m_pcRtcp;
    TRtpPack  m_tOldRtp;

    u32      m_dwFrameId;
    u32      m_dwTimeStamp;
    u8       m_byFrameRate;

    u8       m_byMediaType;
    u32      m_dwMaxFrameSize;

    u8       m_byExBuf[MAX_PACK_EX_LEN];

    s32      m_nMaxSendNum; //根据 发送码流/帧率 计算每次最大发送包数;
    s32      m_nMaxSendSize;//根据 发送码流/帧率 计算每次最大发送字节数;

    TKdvSndStatus        m_tKdvSndStatus;
    TKdvSndStatistics   m_tKdvSndStatistics;

    FRAMEHDR m_tSelfFrmHdr;
    u8      *m_pSelfFrameBuf;

    BOOL32     m_bRepeatSend;     //对于 (mp4) 是否重发
    u16      m_wBufTimeSpan;

    u8       m_byLocalActivePT; //本端发送的动态载荷PT值

    s8      *m_pszMaterialBuf;  //用于生成key的原始字串
    u16      m_wMaterialBufLen; //用于生成key的原始字串长度
    u8       m_byEncryptMode;   //加密模式 Aes 或者 Des

    u8       m_byAesMode;       //AES加密子模式
    u8      *m_pbyEncryptInBuf; //待加密的码流缓冲
    u8      *m_pbyEncryptOutBuf;//完成加密的码流缓冲

    u8       m_byLastEBit;      //记录上一帧的ebit值，已被进行校验

    BOOL32   m_bVidPayload;     //当前媒体流是否为视频流
    u8       m_byRmtAddrNum;    //当前媒体码流的发送地址数（不包括环回地址）
    u32      m_dwCurBandWidth;  //当前媒体码流带宽（平均码率） （kbit/s）
    u8       m_byAACPacket[MAX_EXTEND_PACK_SIZE];
    u32      m_dwFirstTimeStamps;
    BOOL32   m_bFirstFrame;
    HTspsWrite  m_hTspsWrite;
    TPSInfo     m_tPSInfo;
    BOOL32      m_bSendRtp;

    //原有缓冲放大倍数
    u8 m_byBuffMultipleTimes;
    s32 m_nSendRate;
    BOOL32 m_bAaclcNoHead;
	
	// 存储NAT 探测包信息
   	TNatProbeProp  m_tNatProbeProp;
   	u32 m_dwNatLastTs; //记录上一次发送保活包时间
};

#endif // !defined(_KDVNETSND_0603_H_)
