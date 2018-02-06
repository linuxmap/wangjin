/**
* @file         kdvmedianet.h
* @brief        媒体发送和接收
* @details      rtp报文拆组和网络数据包的收发
* @author       顾晓康
* @date         2016/06/20
* @version      2016/06/20
* @par Copyright (c):
*    kedacom
* @par History:
*   version:V2.0  Copyright(C) 2003-2016 KDC, All rights reserved.
*/

#ifndef _KDVMEDIANET_0603_H_    ///<medianet头文件宏,防止重包含
#define _KDVMEDIANET_0603_H_    ///<medianet头文件宏,防止重包含


/** 版本信息字串定义*/
#define VER_KDVMEDIANET  ( const char * )"kdvmedianet for kdm common 00.01.01.01.150422.16:40"


/**
 *@brief  显示kdvmedianet的版本信息
 *@return
 *@ref
 *@see
 *@note
 */
API void kdvmedianetver();


/**
 *@brief        获取kdvmedianet的版本信息
 *@param[out]   ppVerionString      输出版本号字符串地址
 *@return
 *@ref
 *@see
 *@note
 */
API void KdvGetMedianetVer(char** ppVerionString);


/**
 *@brief  显示kdvmedianet的模块帮助信息
 *@return
 *@ref
 *@see
 *@note
 */
API void kdvmedianethelp();


#define  MEDIANET_NO_ERROR                  (u16)0                      ///<成功返回值
#define  ERROR_MEDIA_NET_BASE               (u16)16000                  ///<medianet错误码段基值
#define  ERROR_SND_PARAM                    (ERROR_MEDIA_NET_BASE+1)    ///<设置发送模块参数出错
#define  ERROR_SND_NOCREATE                 (ERROR_MEDIA_NET_BASE+2)    ///<发送模块没有创建
#define  ERROR_SND_MEMORY                   (ERROR_MEDIA_NET_BASE+3)    ///<发送模块内存操作出错
#define  ERROR_SND_CREATE_SOCK              (ERROR_MEDIA_NET_BASE+4)    ///<发送模块创建socket
#define  ERROR_RTP_SSRC                     (ERROR_MEDIA_NET_BASE+5)    ///<RTP同步源错误.
#define  ERROR_LOOPBUF_CREATE               (ERROR_MEDIA_NET_BASE+6)    ///<环状缓冲创建错误
#define  ERROR_RTP_NO_INIT                  (ERROR_MEDIA_NET_BASE+7)    ///<RTP类有些对象没创建
#define  ERROR_RTCP_NO_INIT                 (ERROR_MEDIA_NET_BASE+8)    ///<RTP类有些对象没创建
#define  ERROR_RTCP_SET_TIMER               (ERROR_MEDIA_NET_BASE+9)    ///<RTCP设置定时器出错
#define  ERROR_RTP_SSRC_COLLISION           (ERROR_MEDIA_NET_BASE+10)   ///<RTP 同步源出错
#define  ERROR_SOCK_INIT                    (ERROR_MEDIA_NET_BASE+11)   ///<socket 没有初始化
#define  ERROR_H261_PACK_NUM                (ERROR_MEDIA_NET_BASE+12)   ///<H261的包数不合理
#define  ERROR_PACK_TOO_LEN                 (ERROR_MEDIA_NET_BASE+13)   ///<G.711的数举包太长
#define  ERROR_H263_PACK_NUM                (ERROR_MEDIA_NET_BASE+14)   ///<H263的包数不合理
#define  ERROR_H263_PACK_TOOMUCH            (ERROR_MEDIA_NET_BASE+15)   ///<H263的数据包太长

#define  ERROR_SND_INVALID_SOCK             (ERROR_MEDIA_NET_BASE+16)   ///<发送模块无效socket
#define  ERROR_SND_SEND_UDP                 (ERROR_MEDIA_NET_BASE+17)   ///<发送模块数据包投递失败（目标可能不可达）
#define  ERROR_SND_FRAME                    (ERROR_MEDIA_NET_BASE+18)   ///<发送模块帧拆包错误
#define  ERROR_SND_PSOPEN                   (ERROR_MEDIA_NET_BASE+19)   ///<ps初始化失败
#define  ERROR_SND_PSSETPROGRAM             (ERROR_MEDIA_NET_BASE+20)   ///<ps设置类型失败
#define  ERROR_SND_PSSEND                   (ERROR_MEDIA_NET_BASE+21)   ///<ps发送失败

#define  ERROR_NET_RCV_PARAM                (ERROR_MEDIA_NET_BASE+100)  ///<设置接收模块参数出错
#define  ERROR_NET_RCV_NOCREATE             (ERROR_MEDIA_NET_BASE+101)  ///<接收模块没有创建
#define  ERROR_NET_RCV_MEMORY               (ERROR_MEDIA_NET_BASE+102)  ///<接收模块内存操作出错
#define  ERROR_RCV_RTP_CREATE               (ERROR_MEDIA_NET_BASE+103)  ///<接收模块RTP创建失败
#define  ERROR_RCV_RTP_CALLBACK             (ERROR_MEDIA_NET_BASE+104)  ///<接收模块设置RTP回调函数失败
#define  ERROR_RCV_RTP_SETREMOTEADDR        (ERROR_MEDIA_NET_BASE+105)  ///<接收模块设置RTP远端地址失败
#define  ERROR_CREATE_LOOP_BUF              (ERROR_MEDIA_NET_BASE+106)  ///<创建环状缓冲失败
#define  ERROR_RCV_NO_CREATE                (ERROR_MEDIA_NET_BASE+107)  ///<接收模块接收对象没有创建

#define  ERROR_WSA_STARTUP                  (ERROR_MEDIA_NET_BASE+200)  ///<wsastartup error
#define  ERROR_CREATE_SEMAPORE              (ERROR_MEDIA_NET_BASE+201)  ///<create semapore error
#define  ERROR_SOCKET_CALL                  (ERROR_MEDIA_NET_BASE+202)  ///<调用socket() 函数出错
#define  ERROR_BIND_SOCKET                  (ERROR_MEDIA_NET_BASE+203)  ///<socket 绑定出错
#define  ERROR_CREATE_THREAD                (ERROR_MEDIA_NET_BASE+204)  ///<创建线程出错

#define  ERROR_LOOPBUF_FULL                 (ERROR_MEDIA_NET_BASE+205)  ///<循环缓冲满


#define  ERROR_SET_DECRYPTKEY               (ERROR_MEDIA_NET_BASE+210)  ///<设置解密key失败
#define  ERROR_DECRYPT_FRAME                (ERROR_MEDIA_NET_BASE+212)  ///<解密一帧失败
#define  ERROR_SET_ENCRYPTKEY               (ERROR_MEDIA_NET_BASE+213)  ///<设置加密key失败
#define  ERROR_ENCRYPT_FRAME                (ERROR_MEDIA_NET_BASE+215)  ///<加密一帧失败
#define  ERROR_SET_USERDATA                 (ERROR_MEDIA_NET_BASE+216)  ///<设置用户数据失败
#define  ERROR_NOT_SUPPORT                  (ERROR_MEDIA_NET_BASE+217)  ///<不支持
#define  ERROR_NODATATOSEND                 (ERROR_MEDIA_NET_BASE+218)  ///<缓冲发送时，取不到数据
#define  ERROR_PARSE_SPSPPS                 (ERROR_MEDIA_NET_BASE+219)  ///<解析sps和pps失败
#define  ERROR_SND_H265FRAME                (ERROR_MEDIA_NET_BASE+220)  ///<解析H264失败


#define     MIN_PRE_BUF_SIZE                    (s32)28     ///<G.711需再加一字节

#define MAX_H261_HEADER_LEN      (s32)388       ///<H261头部长度
#define MAX_H263_HEADER_LEN      (s32)3076      ///<H263头部长度
#define MAX_H263PLUS_HEADER_LEN  (s32)3076      ///<H263 plus头部长度
/** 定义加密信息*/
#ifndef DES_ENCRYPT_MODE
#define DES_ENCRYPT_MODE         (u8)0      ///<DES加密模式
#define AES_ENCRYPT_MODE         (u8)1      ///<AES加密模式
#define ENCRYPT_KEY_SIZE         (u8)32     ///<密钥长度 取其中的较大值
#define DES_ENCRYPT_KEY_SIZE     (u8)8      ///<DES密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_A (u8)16  ///<AES Mode-A 密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_B (u8)24  ///<AES Mode-B 密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_C (u8)32  ///<AES Mode-C 密钥长度
#endif
#define MAXSDES                   255       ///<SDES段的最大数目


/** rtp包回调的类型*/
typedef enum
{
    NORMAL_RTP_CALL_BACK = 1,           ///<正常情况下的回调
    UNKOWN_PT_RTP_CALL_BACK = 2         ///<不认识的动态载荷时的回调
}TRtpCallbackType;


/** 从网络接收*/
#define            MEDIANETRCV_FLAG_FROM_RECVFROM            (0x1)

/** 从dataswitch接收*/
#define            MEDIANETRCV_FLAG_FROM_DATASWITCH        (0x1<<1)

/** 从rpctrl接收*/
#define            MEDIANETRCV_FLAG_FROM_RPCTRL            (0x1<<2)

/** 从裸udp下载接收*/
#define            MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD        (0x1<<3)

/** 测试*/
#define            MEDIANETRCV_FLAG_TEST                    (0x1<<31)


/** Frame Header Structure.*/
typedef struct tagFrameHdr
{
    u8     m_byMediaType;   ///<媒体类型
    u8    *m_pData;         ///<数据缓冲
    u32    m_dwPreBufSize;  ///<m_pData缓冲前预留了多少空间，用于加
                            ///< RTP option的时候偏移指针一般为12+4+12
                            ///< (FIXED HEADER + Extence option + Extence bit)
    u32    m_dwDataSize;    ///<m_pData指向的实际缓冲大小缓冲大小
    u8     m_byFrameRate;   ///<发送帧率,用于接收端
    u32    m_dwFrameID;     ///<帧标识，用于接收端
    u32    m_dwTimeStamp;   ///<时间戳, 用于接收端
    u32    m_dwSSRC;        ///<同步源, 用于接收端
    union
    {
        struct{
                   BOOL32    m_bKeyFrame;       ///<频帧类型（I or P）
                   u16       m_wVideoWidth;     ///<视频帧宽
                   u16       m_wVideoHeight;    ///<视频帧宽
              }m_tVideoParam;                   ///<视频参数
        u8    m_byAudioMode;                    ///<音频模式
    };
	u8   m_byStreamID;                          ///<帧ID
}FRAMEHDR,*PFRAMEHDR;


/** 最大网络发送目的数*/
#ifndef  MAX_NETSND_DEST_NUM
#define  MAX_NETSND_DEST_NUM   5    ///<最大网络发送目的数
#endif

/** 防止地址结构重定义*/
#ifndef TNETSTRUCT
#define TNETSTRUCT

#define MAX_USERDATA_LEN    16      ///<最大用户数据长度



/** 流类型 */
typedef enum
{
	EStreamType_Null = 0,	///< NULL
	EstreamType_PS = 1,		///< PS
}ENetStreamType;



/** 网络参数*/
typedef struct tagNetSession
{
    tagNetSession(){m_dwRTPAddr=0; m_wRTPPort=0; m_dwRTCPAddr=0; m_wRTCPPort=0; \
                    m_dwRtpUserDataLen = 0; m_dwRtcpUserDataLen = 0;}

    u32   m_dwRTPAddr;          ///<RTP地址(网络序)
    u16   m_wRTPPort;           ///<RTP端口(本机序)
    u32   m_dwRTCPAddr;         ///<RTCP地址(网络序)
    u16   m_wRTCPPort;          ///<RTCP端口(本机序)
    u32   m_dwRtpUserDataLen;   ///<Rtp用户数据长度,用于在代理情况下,需要在每个udp头前添加固定头数据的情况
    u8    m_abyRtpUserData[MAX_USERDATA_LEN];   ///<用户数据指针
    u32   m_dwRtcpUserDataLen;   ///<Rtcp用户数据长度,用于在代理情况下,需要在每个udp头前添加固定头数据的情况
    u8    m_abyRtcpUserData[MAX_USERDATA_LEN];  ///<用户数据指针
}TNetSession;


/** 本地网络参数,当本地在pxy后面, 本地地址中的m_wUserLen表示接收时需要去除的头长度*/
typedef struct tagLocalNetParam
{
    TNetSession m_tLocalNet;        ///<本地地址,即本地RTP、RTCP的ip和port
    u32         m_dwRtcpBackAddr;   ///<RTCP回发地址(网络序)
    u16         m_wRtcpBackPort;    ///<RTCP回发端口（本机序）
}TLocalNetParam;

/** 网络发送参数*/
typedef struct tagNetSndParam
{
    u8          m_byNum;        ///<实际地址对数
    TNetSession m_tLocalNet;    ///<当地地址对
    TNetSession m_tRemoteNet[MAX_NETSND_DEST_NUM];  ///<远端地址对
}TNetSndParam;

#endif //end TNETSTRUCT

/** 发送模块状态信息*/
typedef struct tagKdvSndStatus
{
    u8           m_byMediaType;     ///<媒体类型
    u32          m_dwMaxFrameSize;  ///<最大的帧大小
    u32           m_dwNetBand;      ///<发送带宽
    u32          m_dwFrameID;       ///<数据帧标识
    u8           m_byFrameRate;     ///<发送频率
    TNetSndParam m_tSendAddr;       ///<发送地址
}TKdvSndStatus;

/** 发送模块统计信息*/
typedef struct tagKdvSndStatistics
{
    u32    m_dwPackSendNum;     ///<已发送的包数
    u32    m_dwFrameNum;        ///<已发送的帧数
    u32    m_dwFrameLoseNum;    ///<由于缓冲满等原因造成的发送的丢帧数
}TKdvSndStatistics;

/** 发送scoket信息*/
typedef struct tagKdvSndSocketInfo
{
    BOOL32 m_bUseRawSocket;     ///<是否启用raw套接字
    u32    m_dwSrcIP;           ///<raw套接字指定的源ip
    u32    m_wPort;             ///<raw套接字源端口
}TKdvSndSocketInfo;

/** 接收模块状态信息*/
typedef struct tagKdvRcvStatus
{
    BOOL32          m_bRcvStart;    ///<是否开始接收
    u32             m_dwFrameID;    ///<数据帧ID
    TLocalNetParam  m_tRcvAddr;     ///<接收当地地址
    u32             m_dwRcvFlag;    ///<接收开启标志
    void*           m_pRegFunc;     ///<接收地址注册函数
    void*           m_pUnregFunc;   ///<接收地址卸载函数
}TKdvRcvStatus;

/** 解码器统计信息*/
typedef struct tagKdvRcvStatistics
{
    u32    m_dwPackNum;         ///<已接收的包数
    u32    m_dwPackLose;        ///<丟包数
    u32    m_dwPackIndexError;  ///<包乱序数
    u32    m_dwFrameNum;        ///<已接收的帧数
}TKdvRcvStatistics;


/** rtcp info 相关结构*/
typedef struct tagUint64
{
    u32  msdw;  ///<most significant word;
    u32  lsdw;  ///<least significant word;
}TUint64;

/** SR包,详细参见RFC3550*/
typedef struct tagRtcpSR
{
    TUint64 m_tNNTP;        ///<NTP时间戳
    u32     m_dwRTP;        ///<timestamp RTP时间戳
    u32     m_dwPackets;    ///<send packets
    u32     m_dwBytes;      ///<send bytes
} TRtcpSR;

/** RR包,参见RFC3550*/
typedef struct tagRtcpRR
{
    u32  m_dwSSRC;      ///<RR包的SSRC
    u32  m_dwFLost;     ///<8Bit fraction lost and 24 bit cumulative lost  丢包率与累计包丢失数
    u32  m_dwExtMaxSeq; ///<接收到的扩展的最高序列号
    u32  m_dwJitter;    ///<到达间隔抖动
    u32  m_dwLSR;       ///<time  上一SR报文的时间戳
    u32  m_dwDLSR;      ///<time  自上一SR的延时
}TRtcpRR;

/** CNAME SDES,参见RFC3550*/
typedef struct tagRtcpSDES
{
    u8  m_byType;               ///<CNAME-1 NAME-2 EMAIL-3 PHONE-4 LOC-5 TOOL-6 NOTE-7 PRIV-8 END-0
    u8  m_byLength;             ///<only text length
    s8  m_szValue[MAXSDES + 1]; ///<leave a place for an asciiz
} TRtcpSDES;

/** RTP源信息,参见RFC3550*/
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

/** RTCP包的类型,参见RFC3550*/
typedef enum
{
        RTCP_SR   = 200,            ///<sender report
        RTCP_RR   = 201,            ///<receiver report
        RTCP_SDES = 202,            ///<source description items
        RTCP_BYE  = 203,            ///<end of participation
        RTCP_APP  = 204             ///<application specific
}TRtcpType;

/** RTCP包*/
typedef struct tagRtcpInfo
{
    TRtcpType   m_emRtcpInfoType;   ///<RTCP包类型
    s32         m_nInvalid;         ///<是否有效
    BOOL32       m_bActive;         ///<RTP/RTCP保活
    TRtpSource  m_tSrc;             ///<rtp源信息
    u32         m_dwSSRC;           ///<SSRC
    u32         m_dwLSRMyTime;      ///<上一个RTCP包的时间
    TRtcpSR     m_tSR;              ///<SR包
    TRtcpRR     m_tToRR;            ///<RR包,发给发送者
    TRtcpRR     m_tFromRR;          ///<RR包,从发送者接收到的
    TRtcpSDES   m_tCName;           ///<SDES包
}TRtcpInfo;


/**
 *@brief    模块初始化,在使用收发类之前需调用此函数?
 *@return   参见错误码定义
 *@ref
 *@see
 *@note
 */
u16 KdvSocketStartup();


/**
 *@brief    该接口提供给G100使用，内部不使用osp内存池
 *@return   参见错误码定义
 *@ref
 *@see
 *@note
 */
u16 KdvSocketStartupUnuseOspMemPool();


/**
 *@brief    和KdvSocketStartup相对应的清除函数
 *@return   参见错误码定义
 *@ref
 *@see
 *@note
 */
u16 KdvSocketCleanup();


/**
 *@brief        设置是否定时发送RTCP包（默认不发），并设置时间频率(默认5000ms)
 *@param[in]    bRtcpStart       使能rtcp报文
 *@param[in]    dwTimer          rtcp报文间隔,时间范围限制在1s-10s
 *@return       参见错误码定义
 *@ref
 *@see
 *@note
 */
u16 SetRtcpMode(BOOL32 bRtcpStart, u32 dwTimer);

/**
 *@brief        设置是否使用raw套接字发送
 *@param[in]    bUsed       使能raw套接字发送
 *@return
 *@ref
 *@see
 *@note
 */
API void setUseRawSend(BOOL32 bUsed);

/** RTP报文结构体*/
typedef struct tagRtpPack
{
    u8   m_byMark;          ///<是否帧边界1表示最后一包
    u8   m_byExtence;       ///<是否有扩展信息
    u8   m_byPadNum;        ///<可能的padding长度
    u8   m_byPayload;       ///<载荷
    u32  m_dwSSRC;          ///<同步源
    u16  m_wSequence;       ///<序列号
    u32  m_dwTimeStamp;     ///<时间戳
    u8  *m_pExData;         ///<扩展数据
    s32  m_nExSize;         ///<扩展大小：sizeof(u32)的倍数；
    u8  *m_pRealData;       ///<媒体数据
    s32  m_nRealSize;       ///<数据大小
    s32  m_nPreBufSize;     ///<m_pRealData前预分配的空间;
    u32  m_dwRcvTimeStamps; ///<收到包所在帧的时间
}TRtpPack;

/** 重传参数*/
typedef struct tagRSParam
{
    u16  m_wFirstTimeSpan;      ///<第一个重传检测点
    u16  m_wSecondTimeSpan;     ///<第二个重传检测点
    u16  m_wThirdTimeSpan;      ///<第三个重传检测点
    u16  m_wRejectTimeSpan;     ///<过期丢弃的时间跨度
} TRSParam;

/** 发送端高级设置参数*/
typedef struct tagAdvancedSndInfo
{
    s32      m_nMaxSendNum;                     ///<根据带块计算的最大发送次数;
    BOOL32   m_bRepeatSend;                     ///<对于 (mp4) 是否重发
    u16      m_wBufTimeSpan;                    ///<buffer时间长度
    BOOL32   m_bEncryption;                     ///<对于 (mp4/H.26X/Audio) 是否设置加密
    u8       m_byEncryptMode;                   ///<加密模式 Aes 或者 Des
    u16      m_wKeySize;                        ///<加密密钥长度
    s8       m_szKeyBuf[ENCRYPT_KEY_SIZE+1];    ///<加密密钥缓冲
    u8       m_byLocalActivePT;                 ///<动态载荷PT值
} TAdvancedSndInfo;

/** 接收端高级设置参数*/
typedef struct tagAdvancedRcvInfo
{
    BOOL32    m_bConfuedAdjust;                 ///<对于 (mp3) 是否做乱序调整
    BOOL32    m_bRepeatSend;                    ///<对于 (mp4) 是否重发
    TRSParam  m_tRSParam;                       ///<重传参数
    BOOL32    m_bDecryption;                    ///<对于 (mp4/H.26X/Audio) 是否设置解密
    u8        m_byDecryptMode;                  ///<解密模式 Aes 或者 Des
    u16       m_wKeySize;                       ///<解密密钥长度
    s8        m_szKeyBuf[ENCRYPT_KEY_SIZE+1];   ///<解密密钥缓冲
    u8        m_byRmtActivePT;                  ///<接收到的动态载荷的Playload值
    u8        m_byRealPT;                       ///<该动态载荷实际代表的的Playload类型，同于发送时的PT约定
	ENetStreamType m_eStreamType;
} TAdvancedRcvInfo;

/** PS流信息*/
typedef struct tagPSInfo
{
    u8        m_byVideoType;    ///<视频类型
    u8        m_byAudioType;    ///<音频类型
}TPSInfo;

/** Struct about nat probe*/
typedef struct tagNatProbePack
{
    u8 *pbyBuf;         ///<NAT 探测包,buf
    u16 wBufLen;        ///<NAT 探测包,buf的长度
    u32 m_dwPeerAddr;   ///<nat 探测包对端地址RTCP回发地址(网络序)
    u16 m_wPeerPort;    ///<nat 探测包对端地址RTCP回发端口（本机序）
}TNatProbePack;

/** nat探测参数*/
typedef struct tagNatProbeProp
{
    TNatProbePack tRtpNatProbePack;     ///<rtp natpack struct
    TNatProbePack tRtcpNatProbePack;    ///<rtcp natpack struct
    u32 dwInterval;                     ///<发送间隔，单位s。若为0，则表示不发送。
}TNatProbeProp;

/** AACLC发送模式*/
typedef enum
{
    AACLC_TRANSMODE_MPEG4_GENERIC,  ///<mpeg4通用格式
    AACLC_TRANSMODE_MP4A_LATM,      ///<mp4a LATM格式
}EAacTransMode;

/**
 *@brief  帧回调函数类型
 *@param[in]  pFrmHdr       帧信息,含帧数据、时间戳、ssrc等信息
 *@param[in]  pContext      回调上下文,创建接收对象时传入的上下文
 *@return
 *@ref
 *@see
 *@note
 */
typedef   void (*PFRAMEPROC )(PFRAMEHDR pFrmHdr, KD_PTR pContext);

/**
 *@brief  RTP包回调函数类型
 *@param[in]  pRtpPack      RTP包信息,含载荷类型、序列号、时间戳、ssrc等信息
 *@param[in]  pContext      回调上下文
 *@return
 *@ref
 *@see
 *@note
 */
typedef   void (*PRTPCALLBACK)(TRtpPack *pRtpPack, KD_PTR pContext);

/**
 *@brief  RTCP包回调函数类型
 *@param[in]  pRtpPack      RTCP包信息,含RTCP类型、SSRC等信息
 *@param[in]  pContext      回调上下文
 *@return
 *@ref
 *@see
 *@note
 */
typedef void (*PRTCPINFOCALLBACK)(TRtcpInfo *pRtcpInfo, KD_PTR pContext);

/** 发送对象CKdvNetSnd声明*/
class CKdvNetSnd;

/** 发送对象CKdvNetSnd定义*/
class CKdvMediaSnd
{
public:
    /**
     *@brief CKdvNetSnd构造函数
     *@return
     *@ref
     *@see
     *@note
     */
    CKdvMediaSnd();

    /**
     *@brief CKdvNetSnd析构函数
     *@return
     *@ref
     *@see
     *@note
     */
    ~CKdvMediaSnd();
public:
    /**
     *@brief 创建发送模块
     *@param[in]    dwMaxFrameSize  最大帧大小
     *@param[in]    dwNetBand       一般指码率
     *@param[in]    ucFrameRate     发送的码流帧率
     *@param[in]    ucMediaType     发送的媒体类型
     *@param[in]    dwSSRC          发送端的SSRC
     *@param[in]    ptPSInfo        PS信息,如果不是PS流,此字段不需要关心
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize, u32 dwNetBand,
                u8 ucFrameRate, u8 ucMediaType, u32 dwSSRC = 0, TPSInfo* ptPSInfo = NULL);

    /**
     *@brief 设置网络发送参数(进行底层套结子的创建，绑定端口,以及发送目标地址的设定等动作)
     *@param[in]    tNetSndParam  网络发送地址参数
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetNetSndParam( TNetSndParam tNetSndParam );

    /**
     *@brief 移除网络发送本地地址参数(进行底层套结子的删除，释放端口等动作)
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 RemoveNetSndLocalParam();

    /**
     *@brief 设置 动态载荷的 Playload值, byLocalActivePT设置为0-表示清空 本端动态载荷标记
     *@param[in]    byLocalActivePT 指定动态载荷类型
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetActivePT(const u8 byLocalActivePT );

	     //设置NAT 探测包 功能, dwInterval=0 时表示不发送。
    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp); 

     //内部接口， 定时处理nat 探测包 ， dwNowTs 当前tick数
    u16 DealNatProbePack(u32 dwNowTs = 0);
    /**
     *@brief 设置加密key字串及加密码流的动态载荷PT值, pszKeyBuf设置为NULL-表示不加密
     *@param[in]    pszKeyBuf       加密KEY数据
     *@param[in]    wKeySize        加密KEY数据的大小
     *@param[in]    byEncryptMode   加密模式
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetEncryptKey( s8 *pszKeyBuf, u16 wKeySize, u8 byEncryptMode=DES_ENCRYPT_MODE );

    /**
     *@brief 设置RTCP回调杉树
     *@param[in]    pRtcpInfoCallback   RTCP回调函数指针
     *@param[in]    pContext            RTCP回调上下文
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext);

    /**
     *@brief 重置帧ID
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetFrameId();

    /**
     *@brief 重置同步源SSRC
     *@param[in]    dwSSRC      重置的SSRC
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetSSRC(u32 dwSSRC = 0);

    /**
     *@brief 重置发送端重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
     *@param[in]    wBufTimeSpan    发送重传指定的时间长度
     *@param[in]    bRepeatSnd      使能重传
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd = FALSE);

    /**
     *@brief 设置发送码流参数
     *@param[in]    dwNetBand       一般指码率
     *@param[in]    ucFrameRate     发送的码流帧率
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetSndInfo(u32 dwNetBand, u8 ucFrameRate);

    /**
     *@brief 发送数据帧（发送帧长度超过MAX_FRAME_SIZE会失败）
     *@param[in]    pFrmHdr     需要发送的帧信息
     *@param[in]    bAvalid     是否执行有效发送
     *@param[in]    bSendRtp    使能直接发送(io操作)
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 Send ( PFRAMEHDR pFrmHdr, BOOL32 bAvalid=TRUE, BOOL32 bSendRtp = TRUE);

    /**
     *@brief 发送数据包
     *@param[in]    pRtpPack    需要发送的RTP报文信息
     *@param[in]    bTrans      是否直接转发
     *@param[in]    bAvalid     是否执行有效发送
     *@param[in]    bSendRtp    使能直接发送(io操作)
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 Send (TRtpPack *pRtpPack, BOOL32 bTrans = TRUE, BOOL32 bAvalid = TRUE, BOOL32 bSendRtp = TRUE);

    /**
     *@brief 获取发送模块状态
     *@param[in]    TKdvSndStatus   发送模块状态
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetStatus ( TKdvSndStatus &tKdvSndStatus );

    /**
     *@brief 获取发送模块统计量
     *@param[in]    tKdvSndStatistics  发送模块统计量
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetStatistics ( TKdvSndStatistics &tKdvSndStatistics );

    /**
     *@brief 获取发送端高级设置参数（重传等）
     *@param[in]    tAdvancedSndInfo  发送对象高级参数信息
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetAdvancedSndInfo(TAdvancedSndInfo &tAdvancedSndInfo);

    /**
     *@brief 发送自测试
     *@param[in]    nFrameLen       发送帧长
     *@param[in]    nSndNum         发送数目
     *@param[in]    nSpan           发送间隔
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SelfTestSend (s32 nFrameLen, s32 nSndNum, s32 nSpan);

    /**
     *@brief rtcp定时rtcp包上报, 内部使用，外部请勿调用
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 DealRtcpTimer();

    /**
     *@brief 设置源地址，用于IP伪装，只能在使用raw socket的时候才有效
     *@param[in]    tSrcNet       网络地址参数,包括RTP、RTCP的ip和port
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetSrcAddr(TNetSession tSrcNet);

    /**
     *@brief 得到socket相关的信息
     *@param[in]    tRtpSockInfo    RTP发送的socket信息
     *@param[in]    tRtcpSockInfo   RTCP发送的socket信息
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetSndSocketInfo(TKdvSndSocketInfo &tRtpSockInfo, TKdvSndSocketInfo &tRtcpSockInfo);

    /**
     *@brief 得到网络发送参数
     *@param[in]    ptNetSndParam   发送对象的网络发送参数,包括对端
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetNetSndParam(TNetSndParam* ptNetSndParam);

    /**
     *@brief 设置最大发送的包长
     *@param[in]    dwMaxSendPackSize  最大能支持的包长,不包括RTP头部
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetMaxSendPackSize(u32 dwMaxSendPackSize);

    /**
     *@brief 对通会议码流，aaclc音频发送不带4字节头（非标准）
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetAaclcSend(BOOL32 bNoHead);

private:
    CKdvNetSnd *m_pcNetSnd;     ///<网络发送对象
    void*       m_hSndSynSem;   ///<用于对象的单线程操作的同步量
};

/**接收对象CKdvNetRcv声明*/
class CKdvNetRcv;

/**接收对象CKdvNetRcv定义*/
class CKdvMediaRcv
{
public:
    /**
     *@brief ~CKdvMediaRcv构造函数
     *@return
     *@ref
     *@see
     *@note
     */
    CKdvMediaRcv();

    /**
     *@brief ~CKdvMediaRcv析构函数
     *@return
     *@ref
     *@see
     *@note
     */
    ~CKdvMediaRcv();
public:
    /**
     *@brief 创建接收模块(dwMaxFrameSize最大值为MAX_FRAME_SIZE，接收长度超过dwMaxFrameSize的帧会被丢弃)
     *@param[in]    dwMaxFrameSize      最大帧大小
     *@param[in]    pFrameCallBackProc  帧数据回调函数
     *@param[in]    pContext            帧回调上下文
     *@param[in]    dwSSRC              接收端的SSRC
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize,
                PFRAMEPROC pFrameCallBackProc,
                KD_PTR pContext,
                u32 dwSSRC = 0);

    /**
     *@brief 创建接收模块
     *@param[in]    dwMaxFrameSize      最大帧大小
     *@param[in]    PRtpCallBackProc    RTP包回调函数
     *@param[in]    pContext            RTP包回调上下文
     *@param[in]    dwSSRC              接收端的SSRC
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 Create( u32 dwMaxFrameSize,
                PRTPCALLBACK PRtpCallBackProc,
                KD_PTR pContext,
                u32 dwSSRC = 0);

    /**
     *@brief 设置接收地址参数(进行底层套结子的创建，绑定端口等动作)
     *@param[in]    tLocalNetParam  网络接收地址参数
     *@param[in]    dwFlag          网络接收标记
     *@param[in]    pRegFunc        网络接收地址注册函数
     *@param[in]    pUnregFunc      网络接收地址卸载函数
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetNetRcvLocalParam (TLocalNetParam tLocalNetParam, u32 dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM, void* pRegFunc=NULL, void* pUnregFunc=NULL );

    /**
     *@brief 移除接收地址参数(进行底层套结子的删除，释放端口等动作)
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 RemoveNetRcvLocalParam();

    /**
     *@brief 设置NAT 探测包 功能, dwInterval=0 时表示不发送。
     *@param[in]    ptNatProbeProp  nat探测发送参数
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetNatProbeProp(TNatProbeProp *ptNatProbeProp);

    /**
     *@brief 内部接口， 定时处理nat 探测包
     *@param[in]    dwNowTs  当前tick数
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 DealNatProbePack(u32 dwNowTs = 0);

    /**
     *@brief 本地RTP报文組帧接口
     *@param[in]    pRtpBuf   本地RTP报文
     *@param[in]    dwRtpBuf  本地RTP报文长度
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 InputRtpPack(u8 *pRtpBuf, u32 dwRtpBuf);

    /**
     *@brief 设置动态载荷的 Playload值
     *@param[in]    byRmtActivePT   为0表示清空本端动态载荷标记
     *@param[in]    byRealPT        实际的RTP载荷值
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetActivePT( u8 byRmtActivePT, u8 byRealPT );

    /**
     *@brief 重置接收端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不发送重传请求
     *@param[in]    tRSParam        接收重传参数
     *@param[in]    bRepeatSnd      使能接收重传
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetRSFlag(const TRSParam tRSParam, BOOL32 bRepeatSnd = FALSE);

    /**
     *@brief 重置接收端对于 (mp3) 是否采用乱序调整处理的开关, 关闭后，将不调整
     *@param[in]    bConfuedAdjust       使能乱序调整
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetCAFlag(BOOL32 bConfuedAdjust = TRUE);

    /**
     *@brief 重置RTP回调接口信息
     *@param[in]    pRtpCallBackProc    RTP回调函数
     *@param[in]    pContext            RTP回调上下文
     *@param[in]    emCallbackType      RTP回调类型
     *@return       参见错误码定义
     *@ref
     *@see
     *@note 如果正常情况下和不认识的动态载荷都需要回调的话,上层需要定义两个不同的回调函数，(主要考虑兼容以前的编译)
     */
    u16 ResetRtpCalllback(PRTPCALLBACK pRtpCallBackProc, KD_PTR pContext, TRtpCallbackType emCallbackType = NORMAL_RTP_CALL_BACK);

    /**
     *@brief 设置RTCP回调函数
     *@param[in]    pRtcpInfoCallback   RTCP回调函数
     *@param[in]    pContext            RTCP回调上下文
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, KD_PTR pContext);

    /**
     *@brief 设置接收解密key字串
     *@param[in]    pszKeyBuf       加密key串,为NULL表示不加密
     *@param[in]    wKeySize        加密key串长度
     *@param[in]    byDecryptMode   加密类型
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetDecryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byDecryptMode=DES_ENCRYPT_MODE);

    /**
     *@brief 开始接收
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
     u16 StartRcv();

    /**
     *@brief 停止接收
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
     u16 StopRcv();

    /**
     *@brief 获取接收对象的状态
     *@param[in]    tKdvRcvStatus       接收对象的状态
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
     u16 GetStatus ( TKdvRcvStatus &tKdvRcvStatus );

    /**
     *@brief 获取接收对象的统计信息
     *@param[in]    tKdvRcvStatistics       接收对象的统计信息
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetStatistics ( TKdvRcvStatistics &tKdvRcvStatistics );

    /**
     *@brief 获取接收对象的高级状态信息
     *@param[in]    tAdvancedRcvInfo       接收对象高级设置参数(重传、乱序重排等)
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo);

    /**
     *@brief 得到当前接收到的最大帧尺寸
     *@param[in]    dwMaxFrameSize       当前接收到的最大帧尺寸
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetMaxFrameSize(u32 &dwMaxFrameSize);

    /**
     *@brief 得到当前接收到的媒体类型,实时性
     *@param[in]    byMediaType       当前接收到的媒体类型
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 GetLocalMediaType(u8 &byMediaType);

    /**
     *@brief rtcp定时rtcp包上报, 内部使用，外部请勿调用
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 DealRtcpTimer();

    /**
     *@brief 接收转发功能,调试用
     *@param[in]    dwIP       转发IP
     *@param[in]    wPort      转发PORT
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 RelayRtpStart(u32 dwIP, u16 wPort);

    /**
     *@brief 停止接收转发,调试用
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 RelayRtpStop();

    /**
     *@brief 重传过nat时，设置本机的rtp接收端口对应的公网地址,目的为使重传时不用广播
     *@param[in]    dwRtpPublicIp       对应的公网IP
     *@param[in]    wRtpPublicPort      对应的公网PORT
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort);

    /**
     *@brief 设置无PPS、SPS仍然回调,默认为FALSE，不回调
     *@param[in]    bNoPPSSPSStillCallback   使能无PPS、SPS仍然回调
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback);

    /**
     *@brief 复位接收buffer
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ResetAllPackets();

    /**
     *@brief 设置采样率
     *@param[in]    dwSample   采样率
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetTimestampSample(u32 dwSample);

    /**
     *@brief 码流中未带关键帧信息，上层输入spsbuf和ppsbuf。解析宽高等信息
     *@param[in]    pbySpsBuf       SPS数据
     *@param[in]    nSpsBufLen      SPS数据长度
     *@param[in]    pbyPpsBuf       PPS数据
     *@param[in]    nPpsBufLen      PPS数据长度
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen);

    /**
     *@brief 添加ps帧回调接口，并设置是否回调去ps头的帧 标志位。
     *@param[in]    pFrameCallBackProc          ps帧回调函数
     *@param[in]    bCallBackFrameWithOutPs     使能回调去ps头的帧
     *@param[in]    pContext                    ps帧回调上下文
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext);

     /**
     *@brief 设置是否接收4k码流,默认为false。
     *@param[in]    bis4k          使能接收4K码流
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetIs4k(BOOL32 bis4k = false);

    /**
     *@brief 设置仅根据时间戳跳变来判断帧边界，仅供网关组对外对通使用
     *@param[in]    bCompFrameByTimeStamp   使能根据时间戳跳变来判断帧边界
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp = FALSE);

    /**
     *@brief 设置AACLC发送模式,默认为AACLC_TRANSMODE_MPEG4_GENERIC
     *@param[in]    eTransMode   AACLC发送模式
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
    u16 SetAaclcTransMode(EAacTransMode eTransMode);
	
	 /**
     *@brief 设置流类型
     *@param[in]    ENetStreamType eStreamType 类型
     *@return       参见错误码定义
     *@ref
     *@see
     *@note
     */
	u16 SetStreamType(ENetStreamType eStreamType);

private:
    CKdvNetRcv *m_pcNetRcv;     ///<网络接收对象
    void*       m_hRcvSynSem;   ///<用于对象的单线程操作的同步量
};

/**
 *@brief 设置媒体TOS值
 *@param[in]    nTOS    TOS值
 *@param[in]    nType   0:全部 1:音频 2:视频 3: 数据
 *@return       参见错误码定义
 *@ref
 *@see
 *@note
 */
API s32 kdvSetMediaTOS(s32 nTOS, s32 nType);

/**
 *@brief 获取媒体TOS值
 *@param[in]    nType   0:全部 1:音频 2:视频 3: 数据
 *@return       参见错误码定义
 *@ref
 *@see
 *@note
 */
API s32 kdvGetMediaTOS(s32 nType);

#endif

