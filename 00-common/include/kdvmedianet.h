/**
* @file         kdvmedianet.h
* @brief        ý�巢�ͺͽ���
* @details      rtp���Ĳ�����������ݰ����շ�
* @author       ������
* @date         2016/06/20
* @version      2016/06/20
* @par Copyright (c):
*    kedacom
* @par History:
*   version:V2.0  Copyright(C) 2003-2016 KDC, All rights reserved.
*/

#ifndef _KDVMEDIANET_0603_H_    ///<medianetͷ�ļ���,��ֹ�ذ���
#define _KDVMEDIANET_0603_H_    ///<medianetͷ�ļ���,��ֹ�ذ���


/** �汾��Ϣ�ִ�����*/
#define VER_KDVMEDIANET  ( const char * )"kdvmedianet for kdm common 00.01.01.01.150422.16:40"


/**
 *@brief  ��ʾkdvmedianet�İ汾��Ϣ
 *@return
 *@ref
 *@see
 *@note
 */
API void kdvmedianetver();


/**
 *@brief        ��ȡkdvmedianet�İ汾��Ϣ
 *@param[out]   ppVerionString      ����汾���ַ�����ַ
 *@return
 *@ref
 *@see
 *@note
 */
API void KdvGetMedianetVer(char** ppVerionString);


/**
 *@brief  ��ʾkdvmedianet��ģ�������Ϣ
 *@return
 *@ref
 *@see
 *@note
 */
API void kdvmedianethelp();


#define  MEDIANET_NO_ERROR                  (u16)0                      ///<�ɹ�����ֵ
#define  ERROR_MEDIA_NET_BASE               (u16)16000                  ///<medianet������λ�ֵ
#define  ERROR_SND_PARAM                    (ERROR_MEDIA_NET_BASE+1)    ///<���÷���ģ���������
#define  ERROR_SND_NOCREATE                 (ERROR_MEDIA_NET_BASE+2)    ///<����ģ��û�д���
#define  ERROR_SND_MEMORY                   (ERROR_MEDIA_NET_BASE+3)    ///<����ģ���ڴ��������
#define  ERROR_SND_CREATE_SOCK              (ERROR_MEDIA_NET_BASE+4)    ///<����ģ�鴴��socket
#define  ERROR_RTP_SSRC                     (ERROR_MEDIA_NET_BASE+5)    ///<RTPͬ��Դ����.
#define  ERROR_LOOPBUF_CREATE               (ERROR_MEDIA_NET_BASE+6)    ///<��״���崴������
#define  ERROR_RTP_NO_INIT                  (ERROR_MEDIA_NET_BASE+7)    ///<RTP����Щ����û����
#define  ERROR_RTCP_NO_INIT                 (ERROR_MEDIA_NET_BASE+8)    ///<RTP����Щ����û����
#define  ERROR_RTCP_SET_TIMER               (ERROR_MEDIA_NET_BASE+9)    ///<RTCP���ö�ʱ������
#define  ERROR_RTP_SSRC_COLLISION           (ERROR_MEDIA_NET_BASE+10)   ///<RTP ͬ��Դ����
#define  ERROR_SOCK_INIT                    (ERROR_MEDIA_NET_BASE+11)   ///<socket û�г�ʼ��
#define  ERROR_H261_PACK_NUM                (ERROR_MEDIA_NET_BASE+12)   ///<H261�İ���������
#define  ERROR_PACK_TOO_LEN                 (ERROR_MEDIA_NET_BASE+13)   ///<G.711�����ٰ�̫��
#define  ERROR_H263_PACK_NUM                (ERROR_MEDIA_NET_BASE+14)   ///<H263�İ���������
#define  ERROR_H263_PACK_TOOMUCH            (ERROR_MEDIA_NET_BASE+15)   ///<H263�����ݰ�̫��

#define  ERROR_SND_INVALID_SOCK             (ERROR_MEDIA_NET_BASE+16)   ///<����ģ����Чsocket
#define  ERROR_SND_SEND_UDP                 (ERROR_MEDIA_NET_BASE+17)   ///<����ģ�����ݰ�Ͷ��ʧ�ܣ�Ŀ����ܲ��ɴ
#define  ERROR_SND_FRAME                    (ERROR_MEDIA_NET_BASE+18)   ///<����ģ��֡�������
#define  ERROR_SND_PSOPEN                   (ERROR_MEDIA_NET_BASE+19)   ///<ps��ʼ��ʧ��
#define  ERROR_SND_PSSETPROGRAM             (ERROR_MEDIA_NET_BASE+20)   ///<ps��������ʧ��
#define  ERROR_SND_PSSEND                   (ERROR_MEDIA_NET_BASE+21)   ///<ps����ʧ��

#define  ERROR_NET_RCV_PARAM                (ERROR_MEDIA_NET_BASE+100)  ///<���ý���ģ���������
#define  ERROR_NET_RCV_NOCREATE             (ERROR_MEDIA_NET_BASE+101)  ///<����ģ��û�д���
#define  ERROR_NET_RCV_MEMORY               (ERROR_MEDIA_NET_BASE+102)  ///<����ģ���ڴ��������
#define  ERROR_RCV_RTP_CREATE               (ERROR_MEDIA_NET_BASE+103)  ///<����ģ��RTP����ʧ��
#define  ERROR_RCV_RTP_CALLBACK             (ERROR_MEDIA_NET_BASE+104)  ///<����ģ������RTP�ص�����ʧ��
#define  ERROR_RCV_RTP_SETREMOTEADDR        (ERROR_MEDIA_NET_BASE+105)  ///<����ģ������RTPԶ�˵�ַʧ��
#define  ERROR_CREATE_LOOP_BUF              (ERROR_MEDIA_NET_BASE+106)  ///<������״����ʧ��
#define  ERROR_RCV_NO_CREATE                (ERROR_MEDIA_NET_BASE+107)  ///<����ģ����ն���û�д���

#define  ERROR_WSA_STARTUP                  (ERROR_MEDIA_NET_BASE+200)  ///<wsastartup error
#define  ERROR_CREATE_SEMAPORE              (ERROR_MEDIA_NET_BASE+201)  ///<create semapore error
#define  ERROR_SOCKET_CALL                  (ERROR_MEDIA_NET_BASE+202)  ///<����socket() ��������
#define  ERROR_BIND_SOCKET                  (ERROR_MEDIA_NET_BASE+203)  ///<socket �󶨳���
#define  ERROR_CREATE_THREAD                (ERROR_MEDIA_NET_BASE+204)  ///<�����̳߳���

#define  ERROR_LOOPBUF_FULL                 (ERROR_MEDIA_NET_BASE+205)  ///<ѭ��������


#define  ERROR_SET_DECRYPTKEY               (ERROR_MEDIA_NET_BASE+210)  ///<���ý���keyʧ��
#define  ERROR_DECRYPT_FRAME                (ERROR_MEDIA_NET_BASE+212)  ///<����һ֡ʧ��
#define  ERROR_SET_ENCRYPTKEY               (ERROR_MEDIA_NET_BASE+213)  ///<���ü���keyʧ��
#define  ERROR_ENCRYPT_FRAME                (ERROR_MEDIA_NET_BASE+215)  ///<����һ֡ʧ��
#define  ERROR_SET_USERDATA                 (ERROR_MEDIA_NET_BASE+216)  ///<�����û�����ʧ��
#define  ERROR_NOT_SUPPORT                  (ERROR_MEDIA_NET_BASE+217)  ///<��֧��
#define  ERROR_NODATATOSEND                 (ERROR_MEDIA_NET_BASE+218)  ///<���巢��ʱ��ȡ��������
#define  ERROR_PARSE_SPSPPS                 (ERROR_MEDIA_NET_BASE+219)  ///<����sps��ppsʧ��
#define  ERROR_SND_H265FRAME                (ERROR_MEDIA_NET_BASE+220)  ///<����H264ʧ��


#define     MIN_PRE_BUF_SIZE                    (s32)28     ///<G.711���ټ�һ�ֽ�

#define MAX_H261_HEADER_LEN      (s32)388       ///<H261ͷ������
#define MAX_H263_HEADER_LEN      (s32)3076      ///<H263ͷ������
#define MAX_H263PLUS_HEADER_LEN  (s32)3076      ///<H263 plusͷ������
/** ���������Ϣ*/
#ifndef DES_ENCRYPT_MODE
#define DES_ENCRYPT_MODE         (u8)0      ///<DES����ģʽ
#define AES_ENCRYPT_MODE         (u8)1      ///<AES����ģʽ
#define ENCRYPT_KEY_SIZE         (u8)32     ///<��Կ���� ȡ���еĽϴ�ֵ
#define DES_ENCRYPT_KEY_SIZE     (u8)8      ///<DES��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_A (u8)16  ///<AES Mode-A ��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_B (u8)24  ///<AES Mode-B ��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_C (u8)32  ///<AES Mode-C ��Կ����
#endif
#define MAXSDES                   255       ///<SDES�ε������Ŀ


/** rtp���ص�������*/
typedef enum
{
    NORMAL_RTP_CALL_BACK = 1,           ///<��������µĻص�
    UNKOWN_PT_RTP_CALL_BACK = 2         ///<����ʶ�Ķ�̬�غ�ʱ�Ļص�
}TRtpCallbackType;


/** ���������*/
#define            MEDIANETRCV_FLAG_FROM_RECVFROM            (0x1)

/** ��dataswitch����*/
#define            MEDIANETRCV_FLAG_FROM_DATASWITCH        (0x1<<1)

/** ��rpctrl����*/
#define            MEDIANETRCV_FLAG_FROM_RPCTRL            (0x1<<2)

/** ����udp���ؽ���*/
#define            MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD        (0x1<<3)

/** ����*/
#define            MEDIANETRCV_FLAG_TEST                    (0x1<<31)


/** Frame Header Structure.*/
typedef struct tagFrameHdr
{
    u8     m_byMediaType;   ///<ý������
    u8    *m_pData;         ///<���ݻ���
    u32    m_dwPreBufSize;  ///<m_pData����ǰԤ���˶��ٿռ䣬���ڼ�
                            ///< RTP option��ʱ��ƫ��ָ��һ��Ϊ12+4+12
                            ///< (FIXED HEADER + Extence option + Extence bit)
    u32    m_dwDataSize;    ///<m_pDataָ���ʵ�ʻ����С�����С
    u8     m_byFrameRate;   ///<����֡��,���ڽ��ն�
    u32    m_dwFrameID;     ///<֡��ʶ�����ڽ��ն�
    u32    m_dwTimeStamp;   ///<ʱ���, ���ڽ��ն�
    u32    m_dwSSRC;        ///<ͬ��Դ, ���ڽ��ն�
    union
    {
        struct{
                   BOOL32    m_bKeyFrame;       ///<Ƶ֡���ͣ�I or P��
                   u16       m_wVideoWidth;     ///<��Ƶ֡��
                   u16       m_wVideoHeight;    ///<��Ƶ֡��
              }m_tVideoParam;                   ///<��Ƶ����
        u8    m_byAudioMode;                    ///<��Ƶģʽ
    };
	u8   m_byStreamID;                          ///<֡ID
}FRAMEHDR,*PFRAMEHDR;


/** ������緢��Ŀ����*/
#ifndef  MAX_NETSND_DEST_NUM
#define  MAX_NETSND_DEST_NUM   5    ///<������緢��Ŀ����
#endif

/** ��ֹ��ַ�ṹ�ض���*/
#ifndef TNETSTRUCT
#define TNETSTRUCT

#define MAX_USERDATA_LEN    16      ///<����û����ݳ���



/** ������ */
typedef enum
{
	EStreamType_Null = 0,	///< NULL
	EstreamType_PS = 1,		///< PS
}ENetStreamType;



/** �������*/
typedef struct tagNetSession
{
    tagNetSession(){m_dwRTPAddr=0; m_wRTPPort=0; m_dwRTCPAddr=0; m_wRTCPPort=0; \
                    m_dwRtpUserDataLen = 0; m_dwRtcpUserDataLen = 0;}

    u32   m_dwRTPAddr;          ///<RTP��ַ(������)
    u16   m_wRTPPort;           ///<RTP�˿�(������)
    u32   m_dwRTCPAddr;         ///<RTCP��ַ(������)
    u16   m_wRTCPPort;          ///<RTCP�˿�(������)
    u32   m_dwRtpUserDataLen;   ///<Rtp�û����ݳ���,�����ڴ��������,��Ҫ��ÿ��udpͷǰ��ӹ̶�ͷ���ݵ����
    u8    m_abyRtpUserData[MAX_USERDATA_LEN];   ///<�û�����ָ��
    u32   m_dwRtcpUserDataLen;   ///<Rtcp�û����ݳ���,�����ڴ��������,��Ҫ��ÿ��udpͷǰ��ӹ̶�ͷ���ݵ����
    u8    m_abyRtcpUserData[MAX_USERDATA_LEN];  ///<�û�����ָ��
}TNetSession;


/** �����������,��������pxy����, ���ص�ַ�е�m_wUserLen��ʾ����ʱ��Ҫȥ����ͷ����*/
typedef struct tagLocalNetParam
{
    TNetSession m_tLocalNet;        ///<���ص�ַ,������RTP��RTCP��ip��port
    u32         m_dwRtcpBackAddr;   ///<RTCP�ط���ַ(������)
    u16         m_wRtcpBackPort;    ///<RTCP�ط��˿ڣ�������
}TLocalNetParam;

/** ���緢�Ͳ���*/
typedef struct tagNetSndParam
{
    u8          m_byNum;        ///<ʵ�ʵ�ַ����
    TNetSession m_tLocalNet;    ///<���ص�ַ��
    TNetSession m_tRemoteNet[MAX_NETSND_DEST_NUM];  ///<Զ�˵�ַ��
}TNetSndParam;

#endif //end TNETSTRUCT

/** ����ģ��״̬��Ϣ*/
typedef struct tagKdvSndStatus
{
    u8           m_byMediaType;     ///<ý������
    u32          m_dwMaxFrameSize;  ///<����֡��С
    u32           m_dwNetBand;      ///<���ʹ���
    u32          m_dwFrameID;       ///<����֡��ʶ
    u8           m_byFrameRate;     ///<����Ƶ��
    TNetSndParam m_tSendAddr;       ///<���͵�ַ
}TKdvSndStatus;

/** ����ģ��ͳ����Ϣ*/
typedef struct tagKdvSndStatistics
{
    u32    m_dwPackSendNum;     ///<�ѷ��͵İ���
    u32    m_dwFrameNum;        ///<�ѷ��͵�֡��
    u32    m_dwFrameLoseNum;    ///<���ڻ�������ԭ����ɵķ��͵Ķ�֡��
}TKdvSndStatistics;

/** ����scoket��Ϣ*/
typedef struct tagKdvSndSocketInfo
{
    BOOL32 m_bUseRawSocket;     ///<�Ƿ�����raw�׽���
    u32    m_dwSrcIP;           ///<raw�׽���ָ����Դip
    u32    m_wPort;             ///<raw�׽���Դ�˿�
}TKdvSndSocketInfo;

/** ����ģ��״̬��Ϣ*/
typedef struct tagKdvRcvStatus
{
    BOOL32          m_bRcvStart;    ///<�Ƿ�ʼ����
    u32             m_dwFrameID;    ///<����֡ID
    TLocalNetParam  m_tRcvAddr;     ///<���յ��ص�ַ
    u32             m_dwRcvFlag;    ///<���տ�����־
    void*           m_pRegFunc;     ///<���յ�ַע�ắ��
    void*           m_pUnregFunc;   ///<���յ�ַж�غ���
}TKdvRcvStatus;

/** ������ͳ����Ϣ*/
typedef struct tagKdvRcvStatistics
{
    u32    m_dwPackNum;         ///<�ѽ��յİ���
    u32    m_dwPackLose;        ///<�G����
    u32    m_dwPackIndexError;  ///<��������
    u32    m_dwFrameNum;        ///<�ѽ��յ�֡��
}TKdvRcvStatistics;


/** rtcp info ��ؽṹ*/
typedef struct tagUint64
{
    u32  msdw;  ///<most significant word;
    u32  lsdw;  ///<least significant word;
}TUint64;

/** SR��,��ϸ�μ�RFC3550*/
typedef struct tagRtcpSR
{
    TUint64 m_tNNTP;        ///<NTPʱ���
    u32     m_dwRTP;        ///<timestamp RTPʱ���
    u32     m_dwPackets;    ///<send packets
    u32     m_dwBytes;      ///<send bytes
} TRtcpSR;

/** RR��,�μ�RFC3550*/
typedef struct tagRtcpRR
{
    u32  m_dwSSRC;      ///<RR����SSRC
    u32  m_dwFLost;     ///<8Bit fraction lost and 24 bit cumulative lost  ���������ۼư���ʧ��
    u32  m_dwExtMaxSeq; ///<���յ�����չ��������к�
    u32  m_dwJitter;    ///<����������
    u32  m_dwLSR;       ///<time  ��һSR���ĵ�ʱ���
    u32  m_dwDLSR;      ///<time  ����һSR����ʱ
}TRtcpRR;

/** CNAME SDES,�μ�RFC3550*/
typedef struct tagRtcpSDES
{
    u8  m_byType;               ///<CNAME-1 NAME-2 EMAIL-3 PHONE-4 LOC-5 TOOL-6 NOTE-7 PRIV-8 END-0
    u8  m_byLength;             ///<only text length
    s8  m_szValue[MAXSDES + 1]; ///<leave a place for an asciiz
} TRtcpSDES;

/** RTPԴ��Ϣ,�μ�RFC3550*/
typedef struct tagRtpSouce
{
    u16  m_wMaxSeq;             ///<highest seq. number seen
    u32  m_dwCycles;            ///<shifted count of seq. number cycles
    u32  m_dwBaseSeq;           ///<base seq number
    u32  m_dwBadSeq;            ///<last 'bad' seq number + 1
    u32  m_dwProbation;         ///<sequ. packets till source is valid
    u32  m_dwReceived;          ///<packets received
    u32  m_dwExpectedPrior;     ///<packet expected at last interval
    u32  m_dwReceivedPrior;     ///<packet received at last interval
    u32  m_dwTransit;           ///<relative trans time for prev pkt
    u32  m_dwJitter;            ///<estimated jitter
}TRtpSource;

/** RTCP��������,�μ�RFC3550*/
typedef enum
{
        RTCP_SR   = 200,            ///<sender report
        RTCP_RR   = 201,            ///<receiver report
        RTCP_SDES = 202,            ///<source description items
        RTCP_BYE  = 203,            ///<end of participation
        RTCP_APP  = 204             ///<application specific
}TRtcpType;

/** RTCP��*/
typedef struct tagRtcpInfo
{
    TRtcpType   m_emRtcpInfoType;   ///<RTCP������
    s32         m_nInvalid;         ///<�Ƿ���Ч
    BOOL32       m_bActive;         ///<RTP/RTCP����
    TRtpSource  m_tSrc;             ///<rtpԴ��Ϣ
    u32         m_dwSSRC;           ///<SSRC
    u32         m_dwLSRMyTime;      ///<��һ��RTCP����ʱ��
    TRtcpSR     m_tSR;              ///<SR��
    TRtcpRR     m_tToRR;            ///<RR��,����������
    TRtcpRR     m_tFromRR;          ///<RR��,�ӷ����߽��յ���
    TRtcpSDES   m_tCName;           ///<SDES��
}TRtcpInfo;


/**
 *@brief    ģ���ʼ��,��ʹ���շ���֮ǰ����ô˺���?
 *@return   �μ������붨��
 *@ref
 *@see
 *@note
 */
u16 KdvSocketStartup();


/**
 *@brief    �ýӿ��ṩ��G100ʹ�ã��ڲ���ʹ��osp�ڴ��
 *@return   �μ������붨��
 *@ref
 *@see
 *@note
 */
u16 KdvSocketStartupUnuseOspMemPool();


/**
 *@brief    ��KdvSocketStartup���Ӧ���������
 *@return   �μ������붨��
 *@ref
 *@see
 *@note
 */
u16 KdvSocketCleanup();


/**
 *@brief        �����Ƿ�ʱ����RTCP����Ĭ�ϲ�������������ʱ��Ƶ��(Ĭ��5000ms)
 *@param[in]    bRtcpStart       ʹ��rtcp����
 *@param[in]    dwTimer          rtcp���ļ��,ʱ�䷶Χ������1s-10s
 *@return       �μ������붨��
 *@ref
 *@see
 *@note
 */
u16 SetRtcpMode(BOOL32 bRtcpStart, u32 dwTimer);

/**
 *@brief        �����Ƿ�ʹ��raw�׽��ַ���
 *@param[in]    bUsed       ʹ��raw�׽��ַ���
 *@return
 *@ref
 *@see
 *@note
 */
API void setUseRawSend(BOOL32 bUsed);

/** RTP���Ľṹ��*/
typedef struct tagRtpPack
{
    u8   m_byMark;          ///<�Ƿ�֡�߽�1��ʾ���һ��
    u8   m_byExtence;       ///<�Ƿ�����չ��Ϣ
    u8   m_byPadNum;        ///<���ܵ�padding����
    u8   m_byPayload;       ///<�غ�
    u32  m_dwSSRC;          ///<ͬ��Դ
    u16  m_wSequence;       ///<���к�
    u32  m_dwTimeStamp;     ///<ʱ���
    u8  *m_pExData;         ///<��չ����
    s32  m_nExSize;         ///<��չ��С��sizeof(u32)�ı�����
    u8  *m_pRealData;       ///<ý������
    s32  m_nRealSize;       ///<���ݴ�С
    s32  m_nPreBufSize;     ///<m_pRealDataǰԤ����Ŀռ�;
    u32  m_dwRcvTimeStamps; ///<�յ�������֡��ʱ��
}TRtpPack;

/** �ش�����*/
typedef struct tagRSParam
{
    u16  m_wFirstTimeSpan;      ///<��һ���ش�����
    u16  m_wSecondTimeSpan;     ///<�ڶ����ش�����
    u16  m_wThirdTimeSpan;      ///<�������ش�����
    u16  m_wRejectTimeSpan;     ///<���ڶ�����ʱ����
} TRSParam;

/** ���Ͷ˸߼����ò���*/
typedef struct tagAdvancedSndInfo
{
    s32      m_nMaxSendNum;                     ///<���ݴ�����������ʹ���;
    BOOL32   m_bRepeatSend;                     ///<���� (mp4) �Ƿ��ط�
    u16      m_wBufTimeSpan;                    ///<bufferʱ�䳤��
    BOOL32   m_bEncryption;                     ///<���� (mp4/H.26X/Audio) �Ƿ����ü���
    u8       m_byEncryptMode;                   ///<����ģʽ Aes ���� Des
    u16      m_wKeySize;                        ///<������Կ����
    s8       m_szKeyBuf[ENCRYPT_KEY_SIZE+1];    ///<������Կ����
    u8       m_byLocalActivePT;                 ///<��̬�غ�PTֵ
} TAdvancedSndInfo;

/** ���ն˸߼����ò���*/
typedef struct tagAdvancedRcvInfo
{
    BOOL32    m_bConfuedAdjust;                 ///<���� (mp3) �Ƿ����������
    BOOL32    m_bRepeatSend;                    ///<���� (mp4) �Ƿ��ط�
    TRSParam  m_tRSParam;                       ///<�ش�����
    BOOL32    m_bDecryption;                    ///<���� (mp4/H.26X/Audio) �Ƿ����ý���
    u8        m_byDecryptMode;                  ///<����ģʽ Aes ���� Des
    u16       m_wKeySize;                       ///<������Կ����
    s8        m_szKeyBuf[ENCRYPT_KEY_SIZE+1];   ///<������Կ����
    u8        m_byRmtActivePT;                  ///<���յ��Ķ�̬�غɵ�Playloadֵ
    u8        m_byRealPT;                       ///<�ö�̬�غ�ʵ�ʴ���ĵ�Playload���ͣ�ͬ�ڷ���ʱ��PTԼ��
	ENetStreamType m_eStreamType;
} TAdvancedRcvInfo;

/** PS����Ϣ*/
typedef struct tagPSInfo
{
    u8        m_byVideoType;    ///<��Ƶ����
    u8        m_byAudioType;    ///<��Ƶ����
}TPSInfo;

/** Struct about nat probe*/
typedef struct tagNatProbePack
{
    u8 *pbyBuf;         ///<NAT ̽���,buf
    u16 wBufLen;        ///<NAT ̽���,buf�ĳ���
    u32 m_dwPeerAddr;   ///<nat ̽����Զ˵�ַRTCP�ط���ַ(������)
    u16 m_wPeerPort;    ///<nat ̽����Զ˵�ַRTCP�ط��˿ڣ�������
}TNatProbePack;

/** nat̽�����*/
typedef struct tagNatProbeProp
{
    TNatProbePack tRtpNatProbePack;     ///<rtp natpack struct
    TNatProbePack tRtcpNatProbePack;    ///<rtcp natpack struct
    u32 dwInterval;                     ///<���ͼ������λs����Ϊ0�����ʾ�����͡�
}TNatProbeProp;

/** AACLC����ģʽ*/
typedef enum
{
    AACLC_TRANSMODE_MPEG4_GENERIC,  ///<mpeg4ͨ�ø�ʽ
    AACLC_TRANSMODE_MP4A_LATM,      ///<mp4a LATM��ʽ
}EAacTransMode;

/**
 *@brief  ֡�ص���������
 *@param[in]  pFrmHdr       ֡��Ϣ,��֡���ݡ�ʱ�����ssrc����Ϣ
 *@param[in]  pContext      �ص�������,�������ն���ʱ�����������
 *@return
 *@ref
 *@see
 *@note
 */
typedef   void (*PFRAMEPROC )(PFRAMEHDR pFrmHdr, KD_PTR pContext);

/**
 *@brief  RTP���ص���������
 *@param[in]  pRtpPack      RTP����Ϣ,���غ����͡����кš�ʱ�����ssrc����Ϣ
 *@param[in]  pContext      �ص�������
 *@return
 *@ref
 *@see
 *@note
 */
typedef   void (*PRTPCALLBACK)(TRtpPack *pRtpPack, KD_PTR pContext);

/**
 *@brief  RTCP���ص���������
 *@param[in]  pRtpPack      RTCP����Ϣ,��RTCP���͡�SSRC����Ϣ
 *@param[in]  pContext      �ص�������
 *@return
 *@ref
 *@see
 *@note
 */
typedef void (*PRTCPINFOCALLBACK)(TRtcpInfo *pRtcpInfo, KD_PTR pContext);

/** ���Ͷ���CKdvNetSnd����*/
class CKdvNetSnd;

/** ���Ͷ���CKdvNetSnd����*/
class CKdvMediaSnd
{
public:
    /**
     *@brief CKdvNetSnd���캯��
     *@return
     *@ref
     *@see
     *@note
     */
    CKdvMediaSnd();

    /**
     *@brief CKdvNetSnd��������
     *@return
     *@ref
     *@see
     *@note
     */
    ~CKdvMediaSnd();
public:
    /**
     *@brief ��������ģ��
     *@param[in]    dwMaxFrameSize  ���֡��С
     *@param[in]    dwNetBand       һ��ָ����
     *@param[in]    ucFrameRate     ���͵�����֡��
     *@param[in]    ucMediaType     ���͵�ý������
     *@param[in]    dwSSRC          ���Ͷ˵�SSRC
     *@param[in]    ptPSInfo        PS��Ϣ,�������PS��,���ֶβ���Ҫ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize, u32 dwNetBand,
                u8 ucFrameRate, u8 ucMediaType, u32 dwSSRC = 0, TPSInfo* ptPSInfo = NULL);

    /**
     *@brief �������緢�Ͳ���(���еײ��׽��ӵĴ������󶨶˿�,�Լ�����Ŀ���ַ���趨�ȶ���)
     *@param[in]    tNetSndParam  ���緢�͵�ַ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetNetSndParam( TNetSndParam tNetSndParam );

    /**
     *@brief �Ƴ����緢�ͱ��ص�ַ����(���еײ��׽��ӵ�ɾ�����ͷŶ˿ڵȶ���)
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 RemoveNetSndLocalParam();

    /**
     *@brief ���� ��̬�غɵ� Playloadֵ, byLocalActivePT����Ϊ0-��ʾ��� ���˶�̬�غɱ��
     *@param[in]    byLocalActivePT ָ����̬�غ�����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetActivePT(const u8 byLocalActivePT );

	     //����NAT ̽��� ����, dwInterval=0 ʱ��ʾ�����͡�
    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp); 

     //�ڲ��ӿڣ� ��ʱ����nat ̽��� �� dwNowTs ��ǰtick��
    u16 DealNatProbePack(u32 dwNowTs = 0);
    /**
     *@brief ���ü���key�ִ������������Ķ�̬�غ�PTֵ, pszKeyBuf����ΪNULL-��ʾ������
     *@param[in]    pszKeyBuf       ����KEY����
     *@param[in]    wKeySize        ����KEY���ݵĴ�С
     *@param[in]    byEncryptMode   ����ģʽ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetEncryptKey( s8 *pszKeyBuf, u16 wKeySize, u8 byEncryptMode=DES_ENCRYPT_MODE );

    /**
     *@brief ����RTCP�ص�ɼ��
     *@param[in]    pRtcpInfoCallback   RTCP�ص�����ָ��
     *@param[in]    pContext            RTCP�ص�������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext);

    /**
     *@brief ����֡ID
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetFrameId();

    /**
     *@brief ����ͬ��ԴSSRC
     *@param[in]    dwSSRC      ���õ�SSRC
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetSSRC(u32 dwSSRC = 0);

    /**
     *@brief ���÷��Ͷ��ش�����Ŀ���,�رպ󣬽������Ѿ����͵����ݰ����л���
     *@param[in]    wBufTimeSpan    �����ش�ָ����ʱ�䳤��
     *@param[in]    bRepeatSnd      ʹ���ش�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd = FALSE);

    /**
     *@brief ���÷�����������
     *@param[in]    dwNetBand       һ��ָ����
     *@param[in]    ucFrameRate     ���͵�����֡��
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetSndInfo(u32 dwNetBand, u8 ucFrameRate);

    /**
     *@brief ��������֡������֡���ȳ���MAX_FRAME_SIZE��ʧ�ܣ�
     *@param[in]    pFrmHdr     ��Ҫ���͵�֡��Ϣ
     *@param[in]    bAvalid     �Ƿ�ִ����Ч����
     *@param[in]    bSendRtp    ʹ��ֱ�ӷ���(io����)
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 Send ( PFRAMEHDR pFrmHdr, BOOL32 bAvalid=TRUE, BOOL32 bSendRtp = TRUE);

    /**
     *@brief �������ݰ�
     *@param[in]    pRtpPack    ��Ҫ���͵�RTP������Ϣ
     *@param[in]    bTrans      �Ƿ�ֱ��ת��
     *@param[in]    bAvalid     �Ƿ�ִ����Ч����
     *@param[in]    bSendRtp    ʹ��ֱ�ӷ���(io����)
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 Send (TRtpPack *pRtpPack, BOOL32 bTrans = TRUE, BOOL32 bAvalid = TRUE, BOOL32 bSendRtp = TRUE);

    /**
     *@brief ��ȡ����ģ��״̬
     *@param[in]    TKdvSndStatus   ����ģ��״̬
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetStatus ( TKdvSndStatus &tKdvSndStatus );

    /**
     *@brief ��ȡ����ģ��ͳ����
     *@param[in]    tKdvSndStatistics  ����ģ��ͳ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetStatistics ( TKdvSndStatistics &tKdvSndStatistics );

    /**
     *@brief ��ȡ���Ͷ˸߼����ò������ش��ȣ�
     *@param[in]    tAdvancedSndInfo  ���Ͷ���߼�������Ϣ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetAdvancedSndInfo(TAdvancedSndInfo &tAdvancedSndInfo);

    /**
     *@brief �����Բ���
     *@param[in]    nFrameLen       ����֡��
     *@param[in]    nSndNum         ������Ŀ
     *@param[in]    nSpan           ���ͼ��
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SelfTestSend (s32 nFrameLen, s32 nSndNum, s32 nSpan);

    /**
     *@brief rtcp��ʱrtcp���ϱ�, �ڲ�ʹ�ã��ⲿ�������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 DealRtcpTimer();

    /**
     *@brief ����Դ��ַ������IPαװ��ֻ����ʹ��raw socket��ʱ�����Ч
     *@param[in]    tSrcNet       �����ַ����,����RTP��RTCP��ip��port
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetSrcAddr(TNetSession tSrcNet);

    /**
     *@brief �õ�socket��ص���Ϣ
     *@param[in]    tRtpSockInfo    RTP���͵�socket��Ϣ
     *@param[in]    tRtcpSockInfo   RTCP���͵�socket��Ϣ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetSndSocketInfo(TKdvSndSocketInfo &tRtpSockInfo, TKdvSndSocketInfo &tRtcpSockInfo);

    /**
     *@brief �õ����緢�Ͳ���
     *@param[in]    ptNetSndParam   ���Ͷ�������緢�Ͳ���,�����Զ�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetNetSndParam(TNetSndParam* ptNetSndParam);

    /**
     *@brief ��������͵İ���
     *@param[in]    dwMaxSendPackSize  �����֧�ֵİ���,������RTPͷ��
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetMaxSendPackSize(u32 dwMaxSendPackSize);

    /**
     *@brief ��ͨ����������aaclc��Ƶ���Ͳ���4�ֽ�ͷ���Ǳ�׼��
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetAaclcSend(BOOL32 bNoHead);

private:
    CKdvNetSnd *m_pcNetSnd;     ///<���緢�Ͷ���
    void*       m_hSndSynSem;   ///<���ڶ���ĵ��̲߳�����ͬ����
};

/**���ն���CKdvNetRcv����*/
class CKdvNetRcv;

/**���ն���CKdvNetRcv����*/
class CKdvMediaRcv
{
public:
    /**
     *@brief ~CKdvMediaRcv���캯��
     *@return
     *@ref
     *@see
     *@note
     */
    CKdvMediaRcv();

    /**
     *@brief ~CKdvMediaRcv��������
     *@return
     *@ref
     *@see
     *@note
     */
    ~CKdvMediaRcv();
public:
    /**
     *@brief ��������ģ��(dwMaxFrameSize���ֵΪMAX_FRAME_SIZE�����ճ��ȳ���dwMaxFrameSize��֡�ᱻ����)
     *@param[in]    dwMaxFrameSize      ���֡��С
     *@param[in]    pFrameCallBackProc  ֡���ݻص�����
     *@param[in]    pContext            ֡�ص�������
     *@param[in]    dwSSRC              ���ն˵�SSRC
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize,
                PFRAMEPROC pFrameCallBackProc,
                KD_PTR pContext,
                u32 dwSSRC = 0);

    /**
     *@brief ��������ģ��
     *@param[in]    dwMaxFrameSize      ���֡��С
     *@param[in]    PRtpCallBackProc    RTP���ص�����
     *@param[in]    pContext            RTP���ص�������
     *@param[in]    dwSSRC              ���ն˵�SSRC
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize,
                PRTPCALLBACK PRtpCallBackProc,
                KD_PTR pContext,
                u32 dwSSRC = 0);

    /**
     *@brief ���ý��յ�ַ����(���еײ��׽��ӵĴ������󶨶˿ڵȶ���)
     *@param[in]    tLocalNetParam  ������յ�ַ����
     *@param[in]    dwFlag          ������ձ��
     *@param[in]    pRegFunc        ������յ�ַע�ắ��
     *@param[in]    pUnregFunc      ������յ�ַж�غ���
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetNetRcvLocalParam (TLocalNetParam tLocalNetParam, u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL );

    /**
     *@brief �Ƴ����յ�ַ����(���еײ��׽��ӵ�ɾ�����ͷŶ˿ڵȶ���)
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 RemoveNetRcvLocalParam();

    /**
     *@brief ����NAT ̽��� ����, dwInterval=0 ʱ��ʾ�����͡�
     *@param[in]    ptNatProbeProp  nat̽�ⷢ�Ͳ���
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp);

    /**
     *@brief �ڲ��ӿڣ� ��ʱ����nat ̽���
     *@param[in]    dwNowTs  ��ǰtick��
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 DealNatProbePack(u32 dwNowTs = 0);

    /**
     *@brief ����RTP���ĽM֡�ӿ�
     *@param[in]    pRtpBuf   ����RTP����
     *@param[in]    dwRtpBuf  ����RTP���ĳ���
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 InputRtpPack(u8 *pRtpBuf, u32 dwRtpBuf);

    /**
     *@brief ���ö�̬�غɵ� Playloadֵ
     *@param[in]    byRmtActivePT   Ϊ0��ʾ��ձ��˶�̬�غɱ��
     *@param[in]    byRealPT        ʵ�ʵ�RTP�غ�ֵ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetActivePT( u8 byRmtActivePT, u8 byRealPT );

    /**
     *@brief ���ý��ն˶���mpeg4����H.264���õ��ش�����Ŀ���,�رպ󣬽��������ش�����
     *@param[in]    tRSParam        �����ش�����
     *@param[in]    bRepeatSnd      ʹ�ܽ����ش�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetRSFlag(const TRSParam tRSParam, BOOL32 bRepeatSnd = FALSE);

    /**
     *@brief ���ý��ն˶��� (mp3) �Ƿ���������������Ŀ���, �رպ󣬽�������
     *@param[in]    bConfuedAdjust       ʹ���������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetCAFlag(BOOL32 bConfuedAdjust = TRUE);

    /**
     *@brief ����RTP�ص��ӿ���Ϣ
     *@param[in]    pRtpCallBackProc    RTP�ص�����
     *@param[in]    pContext            RTP�ص�������
     *@param[in]    emCallbackType      RTP�ص�����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note �����������ºͲ���ʶ�Ķ�̬�غɶ���Ҫ�ص��Ļ�,�ϲ���Ҫ����������ͬ�Ļص�������(��Ҫ���Ǽ�����ǰ�ı���)
     */
    u16 ResetRtpCalllback(PRTPCALLBACK pRtpCallBackProc, KD_PTR pContext, TRtpCallbackType emCallbackType = NORMAL_RTP_CALL_BACK);

    /**
     *@brief ����RTCP�ص�����
     *@param[in]    pRtcpInfoCallback   RTCP�ص�����
     *@param[in]    pContext            RTCP�ص�������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, KD_PTR pContext);

    /**
     *@brief ���ý��ս���key�ִ�
     *@param[in]    pszKeyBuf       ����key��,ΪNULL��ʾ������
     *@param[in]    wKeySize        ����key������
     *@param[in]    byDecryptMode   ��������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetDecryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byDecryptMode=DES_ENCRYPT_MODE);

    /**
     *@brief ��ʼ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
     u16 StartRcv();

    /**
     *@brief ֹͣ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
     u16 StopRcv();

    /**
     *@brief ��ȡ���ն����״̬
     *@param[in]    tKdvRcvStatus       ���ն����״̬
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
     u16 GetStatus ( TKdvRcvStatus &tKdvRcvStatus );

    /**
     *@brief ��ȡ���ն����ͳ����Ϣ
     *@param[in]    tKdvRcvStatistics       ���ն����ͳ����Ϣ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetStatistics ( TKdvRcvStatistics &tKdvRcvStatistics );

    /**
     *@brief ��ȡ���ն���ĸ߼�״̬��Ϣ
     *@param[in]    tAdvancedRcvInfo       ���ն���߼����ò���(�ش����������ŵ�)
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo);

    /**
     *@brief �õ���ǰ���յ������֡�ߴ�
     *@param[in]    dwMaxFrameSize       ��ǰ���յ������֡�ߴ�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetMaxFrameSize(u32 &dwMaxFrameSize);

    /**
     *@brief �õ���ǰ���յ���ý������,ʵʱ��
     *@param[in]    byMediaType       ��ǰ���յ���ý������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 GetLocalMediaType(u8 &byMediaType);

    /**
     *@brief rtcp��ʱrtcp���ϱ�, �ڲ�ʹ�ã��ⲿ�������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 DealRtcpTimer();

    /**
     *@brief ����ת������,������
     *@param[in]    dwIP       ת��IP
     *@param[in]    wPort      ת��PORT
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 RelayRtpStart(u32 dwIP, u16 wPort);

    /**
     *@brief ֹͣ����ת��,������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 RelayRtpStop();

    /**
     *@brief �ش���natʱ�����ñ�����rtp���ն˿ڶ�Ӧ�Ĺ�����ַ,Ŀ��Ϊʹ�ش�ʱ���ù㲥
     *@param[in]    dwRtpPublicIp       ��Ӧ�Ĺ���IP
     *@param[in]    wRtpPublicPort      ��Ӧ�Ĺ���PORT
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort);

    /**
     *@brief ������PPS��SPS��Ȼ�ص�,Ĭ��ΪFALSE�����ص�
     *@param[in]    bNoPPSSPSStillCallback   ʹ����PPS��SPS��Ȼ�ص�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback);

    /**
     *@brief ��λ����buffer
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ResetAllPackets();

    /**
     *@brief ���ò�����
     *@param[in]    dwSample   ������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetTimestampSample(u32 dwSample);

    /**
     *@brief ������δ���ؼ�֡��Ϣ���ϲ�����spsbuf��ppsbuf��������ߵ���Ϣ
     *@param[in]    pbySpsBuf       SPS����
     *@param[in]    nSpsBufLen      SPS���ݳ���
     *@param[in]    pbyPpsBuf       PPS����
     *@param[in]    nPpsBufLen      PPS���ݳ���
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen);

    /**
     *@brief ���ps֡�ص��ӿڣ��������Ƿ�ص�ȥpsͷ��֡ ��־λ��
     *@param[in]    pFrameCallBackProc          ps֡�ص�����
     *@param[in]    bCallBackFrameWithOutPs     ʹ�ܻص�ȥpsͷ��֡
     *@param[in]    pContext                    ps֡�ص�������
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext);

     /**
     *@brief �����Ƿ����4k����,Ĭ��Ϊfalse��
     *@param[in]    bis4k          ʹ�ܽ���4K����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetIs4k(BOOL32 bis4k = false);

    /**
     *@brief ���ý�����ʱ����������ж�֡�߽磬��������������ͨʹ��
     *@param[in]    bCompFrameByTimeStamp   ʹ�ܸ���ʱ����������ж�֡�߽�
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp = FALSE);

    /**
     *@brief ����AACLC����ģʽ,Ĭ��ΪAACLC_TRANSMODE_MPEG4_GENERIC
     *@param[in]    eTransMode   AACLC����ģʽ
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
    u16 SetAaclcTransMode(EAacTransMode eTransMode);
	
	 /**
     *@brief ����������
     *@param[in]    ENetStreamType eStreamType ����
     *@return       �μ������붨��
     *@ref
     *@see
     *@note
     */
	u16 SetStreamType(ENetStreamType eStreamType);

private:
    CKdvNetRcv *m_pcNetRcv;     ///<������ն���
    void*       m_hRcvSynSem;   ///<���ڶ���ĵ��̲߳�����ͬ����
};

/**
 *@brief ����ý��TOSֵ
 *@param[in]    nTOS    TOSֵ
 *@param[in]    nType   0:ȫ�� 1:��Ƶ 2:��Ƶ 3: ����
 *@return       �μ������붨��
 *@ref
 *@see
 *@note
 */
API s32 kdvSetMediaTOS(s32 nTOS, s32 nType);

/**
 *@brief ��ȡý��TOSֵ
 *@param[in]    nType   0:ȫ�� 1:��Ƶ 2:��Ƶ 3: ����
 *@return       �μ������붨��
 *@ref
 *@see
 *@note
 */
API s32 kdvGetMediaTOS(s32 nType);

#endif

