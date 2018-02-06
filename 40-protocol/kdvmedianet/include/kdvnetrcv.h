/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvNetRcv.h
相关文件    : KdvNetRcv.cpp
文件实现功能: CKdvNetRcv 类定义
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
******************************************************************************/

#if !defined(_KDVNETRCV_0603_H__)
#define _KDVNETRCV_0603_H__

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#endif


#include  "common.h"

#include "kdvrtp.h"
#include "kdvrtcp.h"

typedef struct tagLastPackInfo
{
    u16  m_wSeq;         //上一次收到的包序号
    u8   m_byMediaType;  //上一次收到的媒体类型
    u8   m_byMark;       //上一次收到的Mark标志
    u32  m_dwTimeStamp;  //上一次收到的时间戳
    u32  m_dwSSRC;         //上一次收到的同步源
    u32  m_dwRcvTimeSTamp; //上一帧收到时间
}TLastPackInfo;

typedef struct tagAudioHeader
{
    u8  m_byPackNum; //所含小包数
    u8  m_byIndex;
    s32 m_nMode;
}TAudioHeader;


typedef struct tagAudioPack
{
    BOOL32 m_bInvalid;
    u16    m_wSeq;
    u8     m_byMode;
    u8     m_byMediaType;
    u32    m_dwTimeStamp;
    u32    m_dwSSRC;
    u8     m_pBuf[MAX_SND_PACK_SIZE+1];
    s32    m_nLen;
}TAudioPack;



#define MAX_OVERDUE_PACK_NUM (u16)0xFF
#define MAX_H26X_PACK_NUM    (u16)0xFF


//声音模式
//mp3
#define     NET_AUDIO_MODE_WORST   (s32)0//最差
#define     NET_AUDIO_MODE_BAD     (s32)1//差
#define     NET_AUDIO_MODE_NORMAL  (s32)2//一般
#define     NET_AUDIO_MODE_FINE    (s32)3//好
#define     NET_AUDIO_MODE_BEST    (s32)4//最好
//G.711
#define  NET_AUDIO_MODE_PCMA    (s32)5//a law
#define  NET_AUDIO_MODE_PCMU    (s32)6//u law
//G.723
#define  NET_AUDIO_MODE_G723_6  (s32)7
#define  NET_AUDIO_MODE_G723_5  (s32)8
//G.728
#define  NET_AUDIO_MODE_G728    (s32)9
//G.722
#define  NET_AUDIO_MODE_G722    (s32)10
//G.729
#define  NET_AUDIO_MODE_G729    (s32)11

//因批乱序幅度统计的 帧间隔
#define  MAX_AUDIO_CONFUSE_STAT_SPAN (s32)200

#define FRAME_TIMESTAMP_SPAN        40*90       //按25帧90k采样率计算

//检测前后两帧的相关性，决定合并处理后拆分、直接拆分还是丢弃 的操作模式
typedef enum
{
    imode_none = 0,      // 非法帧，丢弃
    imode_imerge_divide, // 合并处理后拆分
    imode_direct_divide, // 直接拆分

} emImergeMode;


//hual---------------------------------------------------------------------
#define RESEND_CHECK_DISTANCE_PERFRAME       5


typedef struct tagListBuffBlock
{
    struct tagListBuffBlock*    m_ptNext;    //指向下一个Block
    u32                         m_dwRealLen; //该块中的实际数据长度
    s8                          m_BlockData;      //Block中的数据

    s8* GetBuffPtr(){ return &m_BlockData;};
}TListBuffBlock;

#define BLOCK_DATA_OFFSET     (sizeof(TListBuffBlock*) + sizeof(u32))


class CListBuffMgr
{
public:
    CListBuffMgr();
    ~CListBuffMgr();

    //初始化创建
    BOOL32 Create(u32 dwBlockSize, u32 dwBlockNum);

    //重构
    BOOL32 ReBuild(u32 dwBlockSize);

    //清除
    void Close();

    //分配并拷贝数据
    TListBuffBlock* WriteBuff(u8 *pbyBuff, u32 dwBuffSize);

    TListBuffBlock* AllocOneBlock();

    //拷贝数据到指定位置，不进行释放数据块链
    static u32 ReadBuff(TListBuffBlock* ptBuffBlock, u8* pbyBuf , s32 nSize = -1);
    static u32 ReadBuff2(TListBuffBlock* ptBuffBlock, u8* pbyBuf, u32 dwOffset);
    //得到数据的大小
    static u32 GetBuffSize(TListBuffBlock* ptBuffBlock);

    //得到总的分配的数据大小
    u32 GetAllocSize();

    //得到块大小
    u32 GetBlockSize();

    //释放数据块链
    u32 FreeBuff(TListBuffBlock* ptBuffBlock);

    u32 GetFreeBlocks();
    u32 GetMinFreeBlocks();
    BOOL32  m_bHad4kH264Stream;

private:
    u32     m_dwBlockSize;              //每块数据的大小
    void*   m_pBuff;                    //整个Buff的指针

    u32     m_dwFreeBlockNum;           //空闲块的数目
    TListBuffBlock* m_ptFreeBuffList;   //空闲块链表
    u32     m_dwMinFreeBlocks;          //最小空闲数
    u32     m_dwBuffSize;               //实际分配缓冲大小
    u32     m_dwTotalNum;               //实际使用总块数

};

#define MAX_RTP_HEAD_LEN   3*4    //最大RTP头长度 （12字节）

#define MAX_RPT_HEAD_BUFF_LEN      (MAX_RTP_HEAD_LEN + 4)

//一包数据结构
typedef struct tagPacketInfo
{
    u32             m_dwTS;
    u16             m_wSN;
    BOOL32          m_bUsed;
    BOOL32          m_bMark;
    BOOL32          m_bKeyFrame;
    TListBuffBlock* m_ptDataBuff;
    u8              m_byFrameRate;
    u8              m_abyHeader[MAX_RPT_HEAD_BUFF_LEN];
    u32             m_dwRcvTS; //收到包的时间，以帧为准

} TPacketInfo;

#define       BUFF_BLOCK_LENGTH               256
//#define    MAX_MEDIA_BUFF_SIZE             5*MAX_FRAME_SIZE    //5*128k

#define    MAX_PACKET_BUFF_NUM             128

#define    INVALID_PACKET_POS              -1

#define    MAX_RESEND_QUEST_TIMES           3
#define    CURRENT_FRAME_DISCARD_DISTANCE   (m_nPacketBuffNum*3/4)


#define DEFAULT_BUFF_TIMELEN    200         //默认缓冲时长（ms）
#define MAX_BUFF_TIMELEN        10000        //最大缓冲时长(ms) modify by gxk 20100916 支持无线前端需要，由2000改至10000

//end hual---------------------------------------------------------------------



class CKdvNetRcv
{
public:
    CKdvNetRcv();
    virtual ~CKdvNetRcv();

public:
    u16 Create ( u32 dwMaxFrameSize,
                 PFRAMEPROC pFrameCallBackProc,
                 void* pContext,
                 u32 dwSSRC = 0);
    u16 Create ( u32 dwMaxFrameSize,
                 PRTPCALLBACK pRtpCallBackProc,
                 void* pContext,
                 u32 dwSSRC = 0);
    u16 SetNetRcvLocalParam (TLocalNetParam tLocalNetParam, u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL);
    u16 RemoveNetRcvLocalParam();
    u16 ResetRtpCalllback(PRTPCALLBACK pRtpCallBackProc, void* pContext, TRtpCallbackType emCallbackType = NORMAL_RTP_CALL_BACK);
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext);
    u16 SetActivePT( u8 byRmtActivePT, u8 byRealPT );
    u16 ResetRSFlag(TRSParam tRSParam, BOOL32 bRepeatSnd);
    u16 ResetCAFlag(BOOL32 bConfuedAdjust);
    u16 StartRcv();
    u16 StopRcv();
    u32 GetCurTime();
    //设置接收解密key字串
    u16 SetDecryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byDecryptMode);

    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp);

    //发送 NAT 探测包
    u16 DealNatProbePack(u32 dwNowTs = 0);

        //本地組帧接口
    u16 InputRtpPack(u8 *pRtpBuf, u32 dwRtpBuf);
    u16 GetStatus(TKdvRcvStatus &tKdvRcvStatus);
    u16 GetStatistics(TKdvRcvStatistics &tKdvRcvStatistics);

    u16 GetMaxFrameSize(u32 &dwMaxFrameSize);
    u16 GetLocalMediaType(u8 &byMediaType);

    u16 GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo);
    u16 DealRtcpTimer();
    u16 RelayRtpStart(u32 dwIP, u16 wPort);
    u16 RelayRtpStop();

    //重传过nat时，设置本机的rtp接收端口对应的公网地址,目的为使重传时不用广播
    u16 SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort);

    u16 SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback);
    u16 SetTimestampSample(u32 dwSample);

    void    ResetAllPackets();                  //重置所有包

    //码流中未带关键帧信息，上层输入spsbuf和ppsbuf。解析宽高等信息
    u16 ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen);
    //添加ps帧回调接口，并设置是否回调去ps头的帧 标志位。
    u16 AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext);

    u16 SetIs4k(BOOL32 bis4k);
    u16 SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp = FALSE);
    u16 SetAaclcTransMode(EAacTransMode eTransMode);
	BOOL32 SetStreamType(ENetStreamType eStreamType);
public:
    TLocalNetParam        m_tLocalNetParam;//本地网络参数
    BOOL32              m_bRcvStart;   //是否开始接收
    PFRAMEPROC          m_pFrameCallBackHandler;//回调处理函数指针
    void*                m_pContext;            //用户数据
    TKdvRcvStatistics   m_tRcvStatistics;
    u32                    m_dwTimeStampSample;
    TKdvSpsPpsInfo        m_tKdvSpsPpsInfo;
    //used to count video frame sample in ps stream
    u32                m_dwPsVideoLastFrameId;
    u32                m_dwPsVideoLastFrameTimeStamp;
   // 添加ps 帧回调，所需成员变量
   PFRAMEPROC m_pPsFrameCallBackProc;
   BOOL32     m_bCallBackFrameWithOutPs;
   void*      m_pPsFrameCallBackContext;

      // 存储NAT 探测包信息
   TNatProbeProp  m_tNatProbeProp;
   u32 m_dwNatLastTs; //记录上一次发送保活包时间

private:
    void FreeBuf();
    void DealNetData(TRtpPack *pRtpPack);
    void DealH261(TRtpPack *pRtpPack);
    void DealH263(TRtpPack *pRtpPack);
    void DealH264(TRtpPack *pRtpPack);
    void DealH263Plus(TRtpPack *pRtpPack);
    void DealH265(TRtpPack *pRtpPack);

    void DealMpg4(TRtpPack *pRtpPack);

    void DealMp3(TRtpPack  *pRtpPack);
    void DealG7xx(TRtpPack *pRtpPack);
    void DealPS(TRtpPack *pRtpPack);

    void DealDataPayload(TRtpPack *pRtpPack);

    BOOL32 DealStdPayload(TRtpPack *pRtpPack);
    BOOL32 DealExtPayload(TRtpPack *pRtpPack);


    void    DealRtpPacket(TRtpPack *pRtpPack);  //处理Rtp数据包

    BOOL32  CheckIfInsert(TRtpPack *pRtpPack);  //检查是否可以插入该包
    s32     InsertPacket(TRtpPack *pRtpPack);   //插入包缓冲

    BOOL32  ProcessCurrentFrame(BOOL32 bForce); //处理当前帧

    BOOL32  UpdateFrameNum(s32 nNewPackPos);    //更新缓冲帧数


    //rtp 包接收回调响应函数
    friend void    RcvCallBack(TRtpPack *pRtpPack, void* pContext);


//重传处理
    void   SendRSQSndQuest(u16 wSN);  //发送重传请求
    void   DealRSCheck(u16 wLastSN, u32 dwLastTS);  //重传点SN检查
    void   UpdateRSPos();             //更新重传点SN
    void   ResetRSPos();              //重置重传点SN

private:
//H26X头处理
    void ParasH261Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH261Header *ptH261Header);
    void ParasH263Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH263Header *ptH263Header);
    void ParasH263PlusHead(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH263PlusHeader *ptH263PlusHeader );
    BOOL32 ParseH264Head(u8 *pRtpPackBuf, s32 nRtpPackBufSize, TKdvH264Header *ptH264Header, BOOL32 bISBitStream = TRUE);
    BOOL32 ParseH265Head(u8 *pRtpPackBuf, s32 nRtpPackBufSize, TKdvH265Header *ptH265Header);

//码流信息处理
    BOOL32 GetH263PicInfo( u8 *pbyBitstream, s32 nBitstreamLen, BOOL32 *pbIsKeyFrame,
        u16 *pwWidth, u16 *pwHeight );
    BOOL32 DecodeH264SPS(TBitStream *s, TSeqParameterSetRBSP *sps, TKdvH264Header *ptH264Header);
    BOOL32 DecodeH264PPS(TBitStream *s, TPicParameterSetRBSP *pps, TKdvH264Header *ptH264Header);
    BOOL32 DecodePTL(TBitStream *ptBs, u8 u8MaxSubLayersMinus1);
    BOOL32 DecodeH265SPS(TBitStream *s, TSPS *ptSps, TKdvH265Header *pStdH265Header);
    BOOL32 DecodeH265PPS(TBitStream *s, TPPS *ptPps, TKdvH265Header *pStdH265Header);
//包检查
    //H26X
    BOOL32 CheckH26XPack(TRtpPack *pRtpPack);
    //MPEG4, MPEG2
    BOOL32 CheckMpg4Pack(TRtpPack *pRtpPack);
    //音频
    BOOL32 CheckMp3Pack(TRtpPack *pRtpPack);
    BOOL32 CheckG7xxPack(TRtpPack *pRtpPack);

    //H26X打包头处理
    void   SaveH261HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit);
    void   SaveH263HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit, u32 dwHeaderLen);
    void   SaveH263PlusHeadList(u8* pPackDataBuf, u32 dwPackLen);

    //H261组包回调
    s32    CompagesH261AndCB(s32 nStartPos, s32 nEndPos);
    //H263组包回调
    s32    CompagesH263AndCB(s32 nStartPos, s32 nEndPos);
    //H263+组包回调
    s32    CompagesH263PlusAndCB(s32 nStartPos, s32 nEndPos);
    //H264组包回调
    s32    CompagesH264AndCB(s32 nStartPos, s32 nEndPos);
    //H265组包回调
    s32    CompagesH265AndCB(s32 nStartPos, s32 nEndPos);
    //MPEG4组包回调
    s32    CompagesMpeg4AndCB(s32 nStartPos, s32 nEndPos);
    //音频组帧回调
    s32    SendFrameAudio(s32 nStartPos, s32 nEndPos);
    s32    SendAMRFrameAudio(s32 nStartPos, s32 nEndPos);

    s32    CompagesPSAndCB(s32 nStartPos, s32 nEndPos);

//码流解密处理
    BOOL32 DecryptRtpData(TRtpPack *pRtpPack);



private:
    CKdvRtp*      m_pcRtp;
    CKdvRtcp*     m_pcRtcp;

//设置参数
    u32              m_dwMaxFrameSize;       //最大帧大小

    PRTPCALLBACK  m_pRtpCallBackHandler;  //RTP回调处理含数

    void*              m_pRtpContext;         //RTP用户数据


    PRTPCALLBACK  m_pRtpUnkownPtHandler;  //RTP回调处理含数
    void*          m_pRtpUnkownPtContext;            //用户数据


    s32           m_nBuffNumTH; //缓冲门槛值

    BOOL32        m_bAudio;               //是否是音频


    //当前帧信息
    u32       m_dwFrameId;      //帧ID
    u32       m_dwTimeStamp;    //帧时间戳
    u8*       m_pFrameBuf;     //组帧回调的缓冲Buff


    //有关重传
    BOOL32      m_bRepeatSend;     //是否重发
    TRSParam  m_tRSParam;        //重传参数

    u8 m_byRSFrameDistance[MAX_RESEND_QUEST_TIMES+1];   //最后一个保存RejectDistance

    //状态统计
    TKdvRcvStatus      m_tRcvStatus;


    SEMHANDLE m_hSem;

private:

    u16 CreateBuff(s32 nBuffTimeLen);

    TLastPackInfo    m_tLastInfo;           //上一帧回调信息
    FRAMEHDR         m_FrmHdr;              //回调用帧结构指针
    FRAMEHDR        m_FirstFrmHdr;   //   第一帧存的信息。
    u32             m_dwRecvFrmId;

    TH261HeaderList  m_tH261HeaderList;
    TH263HeaderList  m_tH263HeaderList;

    TKdvH264Header   m_tH264Header;         //用于保存H264头解析信息
    TKdvH265Header   m_tH265Header;
    TAudioHeader     m_tAudioHeader;        //用于保存音频头解析信息

    u32 m_dwLastPackLen;

//动态载荷
    u8          m_byRmtActivePT;   // 接收到的动态载荷的Playload值
    u8          m_byRealPT;        // 该动态载荷实际代表的的Playload类型，同于发送时的PT约定

//有关加密
    s8*         m_pszMaterialBuf;  // 用于生成key的原始字串
    u16         m_wMaterialBufLen; // 用于生成key的原始字串长度
    u8          m_byDecryptMode;   // 解密模式 Aes 或者 Des
    /// QFDES_mode  m_emDesMode; // DES解密子模式
    u8          m_byAesMode;       // AES加密子模式

    u8*         m_pbyDecryptOutBuf;//完成解密的码流缓冲
    u32         m_dwLastFrameTimeStamp;
    u32         m_dwLastFrameID;
    u8          m_byOldFrameRate;
    BOOL32      m_bCompFrameByTimeStamp;
public:
   // 4k项目所需变量定义
    u32 m_dwTempRcvBufLen;
    BOOL32 m_bHad4kH264Stream;
    BOOL32 m_bisRcv4kStream; //是否接受4k码流的标志位，由上层传递下来。

//有关Rtp包缓冲
    CListBuffMgr    m_cListBuffMgr;           //保存Rtp数据的缓冲

    TPacketInfo*    m_atPackets; //保存各包的信息
    s32             m_nPacketBuffNum;           //缓冲包数
    s32             m_nAllocPacketBuffNum;      //分配空间块数

    //TPacketInfo     m_atPackets[MAX_PACKET_BUFF_NUM]; //保存各包的信息

    s32             m_nCurFrameStartPos;      //初始时设置为(u32)-1;

    s32             m_nBuffFrameNum;          //当前缓冲帧数

    s32             m_nSSRCErrCount;          //连续SSRC或Payload错误计数，
                                              //达到指定值后时SSRC转为新的值

    s32             m_nSNErrCount;            //连续SN错误计数

    typedef struct
    {
        u16 m_wSequence;
        u32 m_dwTimestamp;
    } TRsPoint;

    TRsPoint        m_wRSPoint[MAX_RESEND_QUEST_TIMES]; //重传SN
    s32             m_dwFrameTime;             //帧间隔时间

    s32             m_nMP4PackLen;             //对于MPEG2-MPEG4的每包长度。

    //过nat时，rtp对应的公网地址
    u32 m_dwRtpPublicIp;
    u16 m_wRtpPublicPort;

    u32 m_dwLastUpdateTick;
    u32 m_dwIntervalFrameRecvd;
    u32 m_dwLastFrameRecvd;

    u32    m_dwBufPackNum;  //缓冲中的包数
    BOOL32 m_bNoPPSSPSStillCallback;
    u32   m_dwSample;
    HTspsRead m_hTspsRead;
    u32       m_dwRtpTime;

    SEMHANDLE    m_hSynSem;
    EAacTransMode m_eTransMode;
	ENetStreamType   m_eStreamType;
};


#endif // !defined(_KDVNETRCV_0603_H__)
