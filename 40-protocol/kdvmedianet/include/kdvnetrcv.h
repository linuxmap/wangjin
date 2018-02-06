/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvNetRcv.h
����ļ�    : KdvNetRcv.cpp
�ļ�ʵ�ֹ���: CKdvNetRcv �ඨ��
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
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
    u16  m_wSeq;         //��һ���յ��İ����
    u8   m_byMediaType;  //��һ���յ���ý������
    u8   m_byMark;       //��һ���յ���Mark��־
    u32  m_dwTimeStamp;  //��һ���յ���ʱ���
    u32  m_dwSSRC;         //��һ���յ���ͬ��Դ
    u32  m_dwRcvTimeSTamp; //��һ֡�յ�ʱ��
}TLastPackInfo;

typedef struct tagAudioHeader
{
    u8  m_byPackNum; //����С����
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


//����ģʽ
//mp3
#define     NET_AUDIO_MODE_WORST   (s32)0//���
#define     NET_AUDIO_MODE_BAD     (s32)1//��
#define     NET_AUDIO_MODE_NORMAL  (s32)2//һ��
#define     NET_AUDIO_MODE_FINE    (s32)3//��
#define     NET_AUDIO_MODE_BEST    (s32)4//���
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

//�����������ͳ�Ƶ� ֡���
#define  MAX_AUDIO_CONFUSE_STAT_SPAN (s32)200

#define FRAME_TIMESTAMP_SPAN        40*90       //��25֡90k�����ʼ���

//���ǰ����֡������ԣ������ϲ�������֡�ֱ�Ӳ�ֻ��Ƕ��� �Ĳ���ģʽ
typedef enum
{
    imode_none = 0,      // �Ƿ�֡������
    imode_imerge_divide, // �ϲ��������
    imode_direct_divide, // ֱ�Ӳ��

} emImergeMode;


//hual---------------------------------------------------------------------
#define RESEND_CHECK_DISTANCE_PERFRAME       5


typedef struct tagListBuffBlock
{
    struct tagListBuffBlock*    m_ptNext;    //ָ����һ��Block
    u32                         m_dwRealLen; //�ÿ��е�ʵ�����ݳ���
    s8                          m_BlockData;      //Block�е�����

    s8* GetBuffPtr(){ return &m_BlockData;};
}TListBuffBlock;

#define BLOCK_DATA_OFFSET     (sizeof(TListBuffBlock*) + sizeof(u32))


class CListBuffMgr
{
public:
    CListBuffMgr();
    ~CListBuffMgr();

    //��ʼ������
    BOOL32 Create(u32 dwBlockSize, u32 dwBlockNum);

    //�ع�
    BOOL32 ReBuild(u32 dwBlockSize);

    //���
    void Close();

    //���䲢��������
    TListBuffBlock* WriteBuff(u8 *pbyBuff, u32 dwBuffSize);

    TListBuffBlock* AllocOneBlock();

    //�������ݵ�ָ��λ�ã��������ͷ����ݿ���
    static u32 ReadBuff(TListBuffBlock* ptBuffBlock, u8* pbyBuf , s32 nSize = -1);
    static u32 ReadBuff2(TListBuffBlock* ptBuffBlock, u8* pbyBuf, u32 dwOffset);
    //�õ����ݵĴ�С
    static u32 GetBuffSize(TListBuffBlock* ptBuffBlock);

    //�õ��ܵķ�������ݴ�С
    u32 GetAllocSize();

    //�õ����С
    u32 GetBlockSize();

    //�ͷ����ݿ���
    u32 FreeBuff(TListBuffBlock* ptBuffBlock);

    u32 GetFreeBlocks();
    u32 GetMinFreeBlocks();
    BOOL32  m_bHad4kH264Stream;

private:
    u32     m_dwBlockSize;              //ÿ�����ݵĴ�С
    void*   m_pBuff;                    //����Buff��ָ��

    u32     m_dwFreeBlockNum;           //���п����Ŀ
    TListBuffBlock* m_ptFreeBuffList;   //���п�����
    u32     m_dwMinFreeBlocks;          //��С������
    u32     m_dwBuffSize;               //ʵ�ʷ��仺���С
    u32     m_dwTotalNum;               //ʵ��ʹ���ܿ���

};

#define MAX_RTP_HEAD_LEN   3*4    //���RTPͷ���� ��12�ֽڣ�

#define MAX_RPT_HEAD_BUFF_LEN      (MAX_RTP_HEAD_LEN + 4)

//һ�����ݽṹ
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
    u32             m_dwRcvTS; //�յ�����ʱ�䣬��֡Ϊ׼

} TPacketInfo;

#define       BUFF_BLOCK_LENGTH               256
//#define    MAX_MEDIA_BUFF_SIZE             5*MAX_FRAME_SIZE    //5*128k

#define    MAX_PACKET_BUFF_NUM             128

#define    INVALID_PACKET_POS              -1

#define    MAX_RESEND_QUEST_TIMES           3
#define    CURRENT_FRAME_DISCARD_DISTANCE   (m_nPacketBuffNum*3/4)


#define DEFAULT_BUFF_TIMELEN    200         //Ĭ�ϻ���ʱ����ms��
#define MAX_BUFF_TIMELEN        10000        //��󻺳�ʱ��(ms) modify by gxk 20100916 ֧������ǰ����Ҫ����2000����10000

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
    //���ý��ս���key�ִ�
    u16 SetDecryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byDecryptMode);

    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp);

    //���� NAT ̽���
    u16 DealNatProbePack(u32 dwNowTs = 0);

        //���ؽM֡�ӿ�
    u16 InputRtpPack(u8 *pRtpBuf, u32 dwRtpBuf);
    u16 GetStatus(TKdvRcvStatus &tKdvRcvStatus);
    u16 GetStatistics(TKdvRcvStatistics &tKdvRcvStatistics);

    u16 GetMaxFrameSize(u32 &dwMaxFrameSize);
    u16 GetLocalMediaType(u8 &byMediaType);

    u16 GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo);
    u16 DealRtcpTimer();
    u16 RelayRtpStart(u32 dwIP, u16 wPort);
    u16 RelayRtpStop();

    //�ش���natʱ�����ñ�����rtp���ն˿ڶ�Ӧ�Ĺ�����ַ,Ŀ��Ϊʹ�ش�ʱ���ù㲥
    u16 SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort);

    u16 SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback);
    u16 SetTimestampSample(u32 dwSample);

    void    ResetAllPackets();                  //�������а�

    //������δ���ؼ�֡��Ϣ���ϲ�����spsbuf��ppsbuf��������ߵ���Ϣ
    u16 ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen);
    //���ps֡�ص��ӿڣ��������Ƿ�ص�ȥpsͷ��֡ ��־λ��
    u16 AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext);

    u16 SetIs4k(BOOL32 bis4k);
    u16 SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp = FALSE);
    u16 SetAaclcTransMode(EAacTransMode eTransMode);
	BOOL32 SetStreamType(ENetStreamType eStreamType);
public:
    TLocalNetParam        m_tLocalNetParam;//�����������
    BOOL32              m_bRcvStart;   //�Ƿ�ʼ����
    PFRAMEPROC          m_pFrameCallBackHandler;//�ص�������ָ��
    void*                m_pContext;            //�û�����
    TKdvRcvStatistics   m_tRcvStatistics;
    u32                    m_dwTimeStampSample;
    TKdvSpsPpsInfo        m_tKdvSpsPpsInfo;
    //used to count video frame sample in ps stream
    u32                m_dwPsVideoLastFrameId;
    u32                m_dwPsVideoLastFrameTimeStamp;
   // ���ps ֡�ص��������Ա����
   PFRAMEPROC m_pPsFrameCallBackProc;
   BOOL32     m_bCallBackFrameWithOutPs;
   void*      m_pPsFrameCallBackContext;

      // �洢NAT ̽�����Ϣ
   TNatProbeProp  m_tNatProbeProp;
   u32 m_dwNatLastTs; //��¼��һ�η��ͱ����ʱ��

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


    void    DealRtpPacket(TRtpPack *pRtpPack);  //����Rtp���ݰ�

    BOOL32  CheckIfInsert(TRtpPack *pRtpPack);  //����Ƿ���Բ���ð�
    s32     InsertPacket(TRtpPack *pRtpPack);   //���������

    BOOL32  ProcessCurrentFrame(BOOL32 bForce); //����ǰ֡

    BOOL32  UpdateFrameNum(s32 nNewPackPos);    //���»���֡��


    //rtp �����ջص���Ӧ����
    friend void    RcvCallBack(TRtpPack *pRtpPack, void* pContext);


//�ش�����
    void   SendRSQSndQuest(u16 wSN);  //�����ش�����
    void   DealRSCheck(u16 wLastSN, u32 dwLastTS);  //�ش���SN���
    void   UpdateRSPos();             //�����ش���SN
    void   ResetRSPos();              //�����ش���SN

private:
//H26Xͷ����
    void ParasH261Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH261Header *ptH261Header);
    void ParasH263Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH263Header *ptH263Header);
    void ParasH263PlusHead(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH263PlusHeader *ptH263PlusHeader );
    BOOL32 ParseH264Head(u8 *pRtpPackBuf, s32 nRtpPackBufSize, TKdvH264Header *ptH264Header, BOOL32 bISBitStream = TRUE);
    BOOL32 ParseH265Head(u8 *pRtpPackBuf, s32 nRtpPackBufSize, TKdvH265Header *ptH265Header);

//������Ϣ����
    BOOL32 GetH263PicInfo( u8 *pbyBitstream, s32 nBitstreamLen, BOOL32 *pbIsKeyFrame,
        u16 *pwWidth, u16 *pwHeight );
    BOOL32 DecodeH264SPS(TBitStream *s, TSeqParameterSetRBSP *sps, TKdvH264Header *ptH264Header);
    BOOL32 DecodeH264PPS(TBitStream *s, TPicParameterSetRBSP *pps, TKdvH264Header *ptH264Header);
    BOOL32 DecodePTL(TBitStream *ptBs, u8 u8MaxSubLayersMinus1);
    BOOL32 DecodeH265SPS(TBitStream *s, TSPS *ptSps, TKdvH265Header *pStdH265Header);
    BOOL32 DecodeH265PPS(TBitStream *s, TPPS *ptPps, TKdvH265Header *pStdH265Header);
//�����
    //H26X
    BOOL32 CheckH26XPack(TRtpPack *pRtpPack);
    //MPEG4, MPEG2
    BOOL32 CheckMpg4Pack(TRtpPack *pRtpPack);
    //��Ƶ
    BOOL32 CheckMp3Pack(TRtpPack *pRtpPack);
    BOOL32 CheckG7xxPack(TRtpPack *pRtpPack);

    //H26X���ͷ����
    void   SaveH261HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit);
    void   SaveH263HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit, u32 dwHeaderLen);
    void   SaveH263PlusHeadList(u8* pPackDataBuf, u32 dwPackLen);

    //H261����ص�
    s32    CompagesH261AndCB(s32 nStartPos, s32 nEndPos);
    //H263����ص�
    s32    CompagesH263AndCB(s32 nStartPos, s32 nEndPos);
    //H263+����ص�
    s32    CompagesH263PlusAndCB(s32 nStartPos, s32 nEndPos);
    //H264����ص�
    s32    CompagesH264AndCB(s32 nStartPos, s32 nEndPos);
    //H265����ص�
    s32    CompagesH265AndCB(s32 nStartPos, s32 nEndPos);
    //MPEG4����ص�
    s32    CompagesMpeg4AndCB(s32 nStartPos, s32 nEndPos);
    //��Ƶ��֡�ص�
    s32    SendFrameAudio(s32 nStartPos, s32 nEndPos);
    s32    SendAMRFrameAudio(s32 nStartPos, s32 nEndPos);

    s32    CompagesPSAndCB(s32 nStartPos, s32 nEndPos);

//�������ܴ���
    BOOL32 DecryptRtpData(TRtpPack *pRtpPack);



private:
    CKdvRtp*      m_pcRtp;
    CKdvRtcp*     m_pcRtcp;

//���ò���
    u32              m_dwMaxFrameSize;       //���֡��С

    PRTPCALLBACK  m_pRtpCallBackHandler;  //RTP�ص�������

    void*              m_pRtpContext;         //RTP�û�����


    PRTPCALLBACK  m_pRtpUnkownPtHandler;  //RTP�ص�������
    void*          m_pRtpUnkownPtContext;            //�û�����


    s32           m_nBuffNumTH; //�����ż�ֵ

    BOOL32        m_bAudio;               //�Ƿ�����Ƶ


    //��ǰ֡��Ϣ
    u32       m_dwFrameId;      //֡ID
    u32       m_dwTimeStamp;    //֡ʱ���
    u8*       m_pFrameBuf;     //��֡�ص��Ļ���Buff


    //�й��ش�
    BOOL32      m_bRepeatSend;     //�Ƿ��ط�
    TRSParam  m_tRSParam;        //�ش�����

    u8 m_byRSFrameDistance[MAX_RESEND_QUEST_TIMES+1];   //���һ������RejectDistance

    //״̬ͳ��
    TKdvRcvStatus      m_tRcvStatus;


    SEMHANDLE m_hSem;

private:

    u16 CreateBuff(s32 nBuffTimeLen);

    TLastPackInfo    m_tLastInfo;           //��һ֡�ص���Ϣ
    FRAMEHDR         m_FrmHdr;              //�ص���֡�ṹָ��
    FRAMEHDR        m_FirstFrmHdr;   //   ��һ֡�����Ϣ��
    u32             m_dwRecvFrmId;

    TH261HeaderList  m_tH261HeaderList;
    TH263HeaderList  m_tH263HeaderList;

    TKdvH264Header   m_tH264Header;         //���ڱ���H264ͷ������Ϣ
    TKdvH265Header   m_tH265Header;
    TAudioHeader     m_tAudioHeader;        //���ڱ�����Ƶͷ������Ϣ

    u32 m_dwLastPackLen;

//��̬�غ�
    u8          m_byRmtActivePT;   // ���յ��Ķ�̬�غɵ�Playloadֵ
    u8          m_byRealPT;        // �ö�̬�غ�ʵ�ʴ���ĵ�Playload���ͣ�ͬ�ڷ���ʱ��PTԼ��

//�йؼ���
    s8*         m_pszMaterialBuf;  // ��������key��ԭʼ�ִ�
    u16         m_wMaterialBufLen; // ��������key��ԭʼ�ִ�����
    u8          m_byDecryptMode;   // ����ģʽ Aes ���� Des
    /// QFDES_mode  m_emDesMode; // DES������ģʽ
    u8          m_byAesMode;       // AES������ģʽ

    u8*         m_pbyDecryptOutBuf;//��ɽ��ܵ���������
    u32         m_dwLastFrameTimeStamp;
    u32         m_dwLastFrameID;
    u8          m_byOldFrameRate;
    BOOL32      m_bCompFrameByTimeStamp;
public:
   // 4k��Ŀ�����������
    u32 m_dwTempRcvBufLen;
    BOOL32 m_bHad4kH264Stream;
    BOOL32 m_bisRcv4kStream; //�Ƿ����4k�����ı�־λ�����ϲ㴫��������

//�й�Rtp������
    CListBuffMgr    m_cListBuffMgr;           //����Rtp���ݵĻ���

    TPacketInfo*    m_atPackets; //�����������Ϣ
    s32             m_nPacketBuffNum;           //�������
    s32             m_nAllocPacketBuffNum;      //����ռ����

    //TPacketInfo     m_atPackets[MAX_PACKET_BUFF_NUM]; //�����������Ϣ

    s32             m_nCurFrameStartPos;      //��ʼʱ����Ϊ(u32)-1;

    s32             m_nBuffFrameNum;          //��ǰ����֡��

    s32             m_nSSRCErrCount;          //����SSRC��Payload���������
                                              //�ﵽָ��ֵ��ʱSSRCתΪ�µ�ֵ

    s32             m_nSNErrCount;            //����SN�������

    typedef struct
    {
        u16 m_wSequence;
        u32 m_dwTimestamp;
    } TRsPoint;

    TRsPoint        m_wRSPoint[MAX_RESEND_QUEST_TIMES]; //�ش�SN
    s32             m_dwFrameTime;             //֡���ʱ��

    s32             m_nMP4PackLen;             //����MPEG2-MPEG4��ÿ�����ȡ�

    //��natʱ��rtp��Ӧ�Ĺ�����ַ
    u32 m_dwRtpPublicIp;
    u16 m_wRtpPublicPort;

    u32 m_dwLastUpdateTick;
    u32 m_dwIntervalFrameRecvd;
    u32 m_dwLastFrameRecvd;

    u32    m_dwBufPackNum;  //�����еİ���
    BOOL32 m_bNoPPSSPSStillCallback;
    u32   m_dwSample;
    HTspsRead m_hTspsRead;
    u32       m_dwRtpTime;

    SEMHANDLE    m_hSynSem;
    EAacTransMode m_eTransMode;
	ENetStreamType   m_eStreamType;
};


#endif // !defined(_KDVNETRCV_0603_H__)
