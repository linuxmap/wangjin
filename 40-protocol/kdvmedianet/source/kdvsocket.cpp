/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvSocket.cpp
相关文件    : KdvSocket.h
文件实现功能: CKdvSocket 类实现
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
2004/05/12  2.0         万春雷      完善组播组的增删处理功能
2004/07/07  2.0         万春雷      完善全局线程安全退出
2004/08/04  2.0         万春雷      KdvSocketStartup KdvSocketCleanup 调用正确返回 MEDIANET_NO_ERROR
2004/09/29  2.0         万春雷      增加linux版本编译支持
2004/11/18  2.0         万春雷      调整组播组的增删策略，对于win32平台，同一个socket不应进行多地址加入组播组，而应设置为ADDR_ANY
2004/12/28  2.0         万春雷      增加SendTo的返回值错误码的具体区分
2005/01/13  3.5         万春雷      用于发送的SOCKET 采用RawSocket，从而设置其 IP header-TOS Field
2005/01/18  3.5         万春雷      增加模拟发送测试调用
2005/01/18  3.5         万春雷      增加对嵌入式操作系统调用以太网平滑发送接口设置平滑（暂时禁用）
2005/01/19  3.5         万春雷      调整 用于发送的SOCKET 采用RawSocket 的时机选择（默认关闭）
                                    若读取到配置文件，则采用RawSocket，否则仍使用UDPSocket
2005/01/26  3.5         万春雷      调整 是否平滑发送的时机选择（默认关闭）
                                    若读取到配置文件，且当前vx平台的os支持平滑发送, 则开启
                                    OS目前尚未支持，暂时禁用
2005/01/31  3.5         万春雷      对 发送的SOCKET 采用RawSocket，源ip应为本端有效ip而非组播ip
2005/03/16  3.6         万春雷      开启对嵌入式操作系统调用以太网平滑发送接口设置平滑
2005/03/20  3.6         万春雷      细化ip优先级的各类取值 音频、视频等
2005/03/22  3.6         万春雷      以太网平滑发送新增:设置制定网口的最大和最小发送速率
******************************************************************************/
#ifdef _LINUX_
#else

#include "kdvsocket.h"
#include "kdvnetsnd.h"
#include "kdvnetrcv.h"

extern int   g_nShowDebugInfo;        //是否显示隐藏的一些调试状态信息
extern BOOL32 g_bUseMemPool;

//Socket 状态消息
typedef enum
{
    SOCK_CREATE = 0,
    SOCK_RCV    = 1,
    SOCK_STOP   = 2,
    SOCK_DEL    = 3,
    SOCK_EXIT   = 4
}TSockState;

TMediaSndList   g_tMediaSndList;     //接收对象列表全局变量
TMediaRcvList   g_tMediaRcvList;     //发送对象列表全局变量
TMediaSndList   g_tMediaSndListTmp;
TMediaRcvList   g_tMediaRcvListTmp;

SEMHANDLE   g_hMediaSndSem = NULL;    //发送对象列表的访问维护的信号量
SEMHANDLE   g_hMediaRcvSem = NULL;    //接收对象列表的访问维护的信号量

s32  g_nMaxRcvSockNum = MAX_RCV_NUM;
s32  g_nMaxSndSockNum = MAX_SND_NUM;

//全局接收socket列表结构
typedef struct tagSOCKLIST
{
    s32         m_nSockCount;               //接收套结子总数 <= FD_SETSIZE
    CKdvSocket**   m_pptSockUnit;
}TSockList;

//全局发送socket列表结构
typedef struct tagSNDSOCKLIST
{
    s32         m_nSockCount;               //发送套结子总数 <= MAX_SND_NUM
    CKdvSocket**   m_pptSockUnit;
}TSndSockList;

static     s32           g_nRefCount     = 0;        //线程引用计数
static     TASKHANDLE    g_hRcvTask      = NULL;    //socket接收线程handle
static     TASKHANDLE    g_hSndTask      = NULL;    //rtcp定时rtcp包上报线程handle
static     SEMHANDLE     g_hSem          = NULL;    //判断线程退出的信号量
static   TSockList     g_tRcvSockList;                 //接收socket列表全局变量

static   TSndSockList  g_tSndSockList;             //发送socket列表全局变量

static   SEMHANDLE       g_hSndSockSem = NULL;    //发送套接字列表的访问维护的信号量

//win32是否初始化了sock库的判断
static   BOOL32        g_bSockInit     = FALSE;
static   SOCKHANDLE    g_hWatchSocket  = INVALID_SOCKET;  //控制线程的socket
static   BOOL32        g_bExitTask     = FALSE;           //判断任务退出

//用于序列化 接收套结子 的创建删除请求操作的同步信号量
static     SEMHANDLE     g_hCreateDelSem  = NULL;

#define  MAXWATCHSOCKPORT  (u16)20599//max watchsock port
#define  MINWATCHSOCKPORT  (u16)20400//min watchsock port

//watch socket port,deafult is MINWATCHSOCKPORT;
static   u16           g_wWatchSockPort = MINWATCHSOCKPORT;


//全局的时间戳tick数，用于统一提供收发对象的SSRC，保证唯一性
static   u32           g_dwTimeStamp = 0;
//用于统一提供收发对象的SSRC的同步信号量
static     SEMHANDLE     g_hGetTimeStampSem  = NULL;


BOOL32 g_bUseSmoothSnd = TRUE;          // 查询是否使用定时发送以太网数据以达到平滑发送
BOOL32 g_bBrdEthSndUseTimer = FALSE;  // 当前OS是否支持网络平滑发送

u32 g_dwMinSendBitrate = 2048;        //该网口的最小发送速率（以Kbps为单位,默认2M）
u32 g_dwMaxSendBitrate = 10240;       //该网口的最大发送速率（以Kbps为单位,默认10M）


static SOCKHANDLE g_hRawSocket = INVALID_SOCKET;
static SEMHANDLE g_hRawSocketSem = NULL;
static s8 m_szRawSocketPackBuf[MAX_SND_PACK_SIZE_BY_RAW_IP+12] = {0};
u16 CalcChecksum(u16 *wBuffer, s32 nSize);

void* SocketRcvTaskProc(void * pParam);
void* RtcpSndTaskProc(void * pParam);
void* MedianetRpctrlCallBack(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);
void* MedianetU2ACallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);
void* MedianetDataSwitchCallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext);


static u16 RawSocketSendto(u8 *pBuf, s32 nSize, u32 dwSrcIp, u16 wSrcPort, u32 dwRemoteIp, u16 wRemotePort)
{
    u16 wRet = MEDIANET_NO_ERROR;
    TIPHdr m_tIPHdr;
    TUDPHdr  m_tUDPHdr;
    s32 nIPVersion = 4;    //IPV4
    s32 nIPSize    = sizeof(m_tIPHdr) / sizeof(u32);
    s32 nUDPSize   = sizeof(m_tUDPHdr) + nSize;
    s32 nTotalSize = sizeof(m_tIPHdr) + sizeof(m_tUDPHdr) + nSize;
    BOOL32 bNeedCheckSum = TRUE;

    // 初始化IP头
    m_tIPHdr.byIPVerLen   = (nIPVersion << 4) | nIPSize;
    m_tIPHdr.byIPTos  = 0;          // IP type of service
    m_tIPHdr.wIPTotalLen  = htons(nTotalSize);    // Total packet len
    m_tIPHdr.wIPID        = 0;                    // Unique identifier: set to 0
    m_tIPHdr.wIPOffset    = 0;                    // Fragment offset field
    m_tIPHdr.byIPTtl      = 128;              // Time to live
    m_tIPHdr.byIPProtocol = 0x11;                 // Protocol(UDP) TCP-6;UDP-17
    m_tIPHdr.wIPCheckSum  = 0 ;                   // IP checksum
    m_tIPHdr.dwSrcAddr    = dwSrcIp;          // Source address
    if (0 == m_tIPHdr.dwSrcAddr)
    {
        bNeedCheckSum = FALSE;
    }
    m_tIPHdr.dwDstAddr    = dwRemoteIp;           // Destination address
    m_tUDPHdr.wSrcPort    = htons(wSrcPort) ;
    m_tUDPHdr.wDstPort    = htons(wRemotePort) ;
    m_tUDPHdr.wUDPLen     = htons(nUDPSize) ;
    m_tUDPHdr.wUDPCheckSum = 0 ;

    // 计算UDP校验和(称为网际校验和算法，把被校验的数据１６位进行累加，然后取反码，
    // 若数据字节长度为奇数，则数据尾部补一个字节的０以凑成偶数。
    // 此算法适用于IPv4、ICMPv4、IGMPV4、ICMPv6、UDP和TCP校验和，更详细的信息请参考RFC1071)
    //
    // UDP校验和的数据为 UDP伪包头(UDP pseudo-header)＋UDP包头＋数据(data)
    // 注意Data长度为奇数时尾部补一个0凑成偶数
    //

    // 为减少一次拷贝次数，这里借用了一部分空间 （sizeof(m_tIPHdr) > sizeof(tUDPPsdHdr)）
    // 校验和运算分为两步：先分别计算CRC（即累积和值），再取反码。
    //

    // hualiang 2006-08-31
    // 如果源地址为0，则源地址由下层填写，这时checksum将由下面计算（vxworks）
    // 对于linux，下层不会重新计算checksum，如果源地址按0计算，则checksum会出错，因此直接添0，表示不校验。

    MEDIANET_SEM_TAKE( g_hRawSocketSem );

    s8 *pszRawBuf = (s8 *)&m_szRawSocketPackBuf;
    if ( bNeedCheckSum )
    {
        TUDPPsdHdr   tUDPPsdHdr;
        memset(&tUDPPsdHdr, 0 , sizeof(tUDPPsdHdr));
        tUDPPsdHdr.dwSrcAddr    = m_tIPHdr.dwSrcAddr;
        tUDPPsdHdr.dwDstAddr    = m_tIPHdr.dwDstAddr;
        tUDPPsdHdr.byIPProtocol = m_tIPHdr.byIPProtocol;
        tUDPPsdHdr.wUDPLen      = m_tUDPHdr.wUDPLen;

        pszRawBuf = (s8 *)&m_szRawSocketPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
        s32 nUdpChecksumSize = 0;

        memcpy(pszRawBuf, &tUDPPsdHdr, sizeof(tUDPPsdHdr));
        pszRawBuf += sizeof(tUDPPsdHdr);
        nUdpChecksumSize += sizeof(tUDPPsdHdr);

        memcpy(pszRawBuf, &m_tUDPHdr, sizeof(m_tUDPHdr));
        pszRawBuf += sizeof(m_tUDPHdr);
        nUdpChecksumSize += sizeof(m_tUDPHdr);

        memcpy(pszRawBuf, pBuf, nSize);
        pszRawBuf += nSize;
        nUdpChecksumSize += nSize;

        if( 0 != (nUdpChecksumSize%2) )
        {
            *pszRawBuf = 0;
            pszRawBuf += 1;
            nUdpChecksumSize += 1;
        }

        pszRawBuf = (s8 *)&m_szRawSocketPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
        u16 wCkSum = CalcChecksum((u16 *)pszRawBuf, nUdpChecksumSize);

        m_tUDPHdr.wUDPCheckSum = wCkSum;
    }
    else
    {
        memcpy(pszRawBuf+sizeof(m_tIPHdr)+sizeof(m_tUDPHdr), pBuf, nSize);
    }

    //构造完整的 Raw Socket 数据包并进行发送

    pszRawBuf = (s8 *)&m_szRawSocketPackBuf;
    memcpy(pszRawBuf, &m_tIPHdr, sizeof(m_tIPHdr));
    pszRawBuf += sizeof(m_tIPHdr);
    memcpy(pszRawBuf, &m_tUDPHdr, sizeof(m_tUDPHdr));

    SOCKADDR_IN m_tAddrIn;
    memset( &m_tAddrIn, 0, sizeof(m_tAddrIn));
    m_tAddrIn.sin_family      = AF_INET;
    m_tAddrIn.sin_addr.s_addr = dwRemoteIp;
    m_tAddrIn.sin_port          = htons(wRemotePort);

    s32 nSndBytes = sendto( g_hRawSocket, (s8*)&m_szRawSocketPackBuf, nTotalSize, 0,
        (sockaddr *)&m_tAddrIn, sizeof(SOCKADDR_IN) );
    if( nSndBytes <= 0 )
    {
        wRet = ERROR_SND_SEND_UDP;
    }

    MEDIANET_SEM_GIVE( g_hRawSocketSem );
    return wRet;
}


//2006-05-31 hual
BOOL32 g_bRtcpTaskStart = FALSE;
u32 g_dwTimer = 5000; //rtcp timer inter


//2006-06-08 hual
s32 g_nRateRadioLess1M = 300;
s32 g_nRateRadioMore1M = 200;

API void showsmoothsnd()
{
    OspPrintf( TRUE, FALSE, "[showsmoothsnd] UseFlag.%d, BrdEnable.%d MinSendBitrate.%d MaxSendBitrate.%d \n",
               g_bUseSmoothSnd, g_bBrdEthSndUseTimer, g_dwMinSendBitrate, g_dwMaxSendBitrate );
}

API void disablesmoothsnd()
{
    g_bUseSmoothSnd = FALSE;
    OspPrintf( TRUE, FALSE, "[disablesmoothsnd] UseFlag=%d, BrdEnable=%d \n",
               g_bUseSmoothSnd, g_bBrdEthSndUseTimer );
}

BOOL32 g_bUseRawSend = FALSE;    // 是否使用 raw-socket 进行数据包投递

API void setrawsend(s32 nRawSend)
{
    if( 0 == nRawSend )
    {
        g_bUseRawSend = FALSE;
    }
    else
    {
        g_bUseRawSend = TRUE;
    }
    OspPrintf(TRUE, FALSE, "\n Set Use Raw Socket Send Flag=%d \n", g_bUseRawSend);
}

u8 g_byTosType = 2;     // TOS服务类型:Type Of Service 服务类型划分：0-不使用TOS优化设置 1-IP优先级 2-Difference Service 区分服务优先
u8 g_byAudTOS  = 0xB8;    // 设置发送socket的数据包的TOS值，默认设置为 EF （Expedited Forwarding）策略
u8 g_byVidTOS  = 0xB8;    // 设置发送socket的数据包的TOS值，默认设置为 EF （Expedited Forwarding）策略
u8 g_byDataTOS = 0xB8;    // 设置发送socket的数据包的TOS值，默认设置为 EF （Expedited Forwarding）策略


u32 g_dwAdditionMulticastIf = 0;
API int kdvSetMediaTOS(s32 nTOS, s32 nType)
{
    switch( nType )
    {
    case 0:
        g_byAudTOS  = nTOS;
        g_byVidTOS  = nTOS;
        g_byDataTOS = nTOS;
        break;
    case 1:
        g_byAudTOS  = nTOS;
        break;
    case 2:
        g_byVidTOS  = nTOS;
        break;
    case 3:
        g_byDataTOS = nTOS;
        break;

    default:
        return -1;
    }

    if ((0 == g_byAudTOS) &&
        (0 == g_byVidTOS) &&
        (0 == g_byDataTOS))
    {
        g_bUseRawSend = FALSE;
    }
    else
    {
        g_bUseRawSend = TRUE;
    }


    OspPrintf(TRUE, FALSE, "\n Set New IP DSCP Field One Byte nTOS.%d nType.%d \n", nTOS, nType);
    return nTOS;
}


API s32 kdvGetMediaTOS(s32 nType)
{
    s32 nTOS;

    switch( nType )
    {
    case 0:
        if (g_byAudTOS == g_byVidTOS && g_byAudTOS == g_byDataTOS)
        {
            nTOS = g_byAudTOS;
        }
        else
        {
            nTOS = -1;
        }
        break;

    case 1:
        nTOS = g_byAudTOS;
        break;

    case 2:
        nTOS = g_byVidTOS;
        break;

    case 3:
        nTOS = g_byDataTOS;
        break;

    default:
        nTOS = -1;
    }

    return nTOS;
}


u8 g_byTTL = 0x40;    // 设置发送socket的数据包的ttl值

API void setttl(s32 nTTL)
{
    g_byTTL = nTTL;
    OspPrintf(TRUE, FALSE, "\n Set New IP TTL Field One Byte ttl=%d \n", g_byTTL);
}

u8 g_byNeedChecksum = 1;

API void setchecksum(s32 nchecksum)
{
    g_byNeedChecksum = nchecksum;
    OspPrintf(TRUE, FALSE, "\n Set New checksum Value=%d \n", g_byNeedChecksum);
}

API void setUseRawSend(BOOL32 bUsed)
{
    g_bUseRawSend = bUsed?TRUE:FALSE;
    OspPrintf(TRUE, FALSE, "Set UseRawSend Value=%d \n", g_bUseRawSend);
}

//设置RTCPinfo
u16 SetRtcpMode(BOOL32 bRtcpTaskStart, u32 dwTimer/*ms*/)
{
    if(dwTimer < 1000 || dwTimer >10000)
    {
        OspPrintf(TRUE, FALSE, "\nsettine time %d not in range(1000-10000). Error\n", dwTimer);
        return ERROR_SET_USERDATA;
    }
    else
    {
        g_bRtcpTaskStart = bRtcpTaskStart;
        g_dwTimer = dwTimer;
        OspPrintf(TRUE, FALSE, "\nset rtcp mode rtcptaskstart= %d, timer= %d. Complete\n",g_bRtcpTaskStart, g_dwTimer);
    }

    return MEDIANET_NO_ERROR;
}

API void setrtcpmode(BOOL32 bRtcpTaskStart, u32 dwTimer)
{
    if(dwTimer < 1000 || dwTimer >10000)
    {
        OspPrintf(TRUE, FALSE, "\nsettine time %d not in range(1000-10000). Error\n", dwTimer);
    }
    else
    {
        g_bRtcpTaskStart = bRtcpTaskStart;
        g_dwTimer = dwTimer;
        OspPrintf(TRUE, FALSE, "\nset rtcp mode rtcptaskstart= %d, timer= %d. Complete\n",g_bRtcpTaskStart, g_dwTimer);
    }
}

API void printrtcpinfo()
{
    OspPrintf(TRUE, FALSE, "\n rtcp mode rtcptaskstart = %d, timer = %d ms", g_bRtcpTaskStart, g_dwTimer);
}


// add by hual 2005-07-26
extern s32 g_nFrameRelyMode;
API s32 SetMarkRelyMode(s32 nMode)
{
    if (nMode > 2)
    {
        return -1;
    }

    OspPrintf(TRUE, FALSE, "set g_nFrameRelyMode from %d to %d\n",
        g_nFrameRelyMode,
        nMode);

    g_nFrameRelyMode = nMode;

    return nMode;
}

// add by hual 2005-9-22
extern BOOL32 g_bForceH263Plus;
API s32 SetForceH263Plus(s32 nForceH263Plus)
{
    BOOL32 bValue;

    if (nForceH263Plus != 0)
    {
        bValue = TRUE;
    }
    else
    {
        bValue = FALSE;
    }

    OspPrintf(TRUE, FALSE, "set g_bForceH263Plus from %d to %d\n",
              g_bForceH263Plus, bValue);

    g_bForceH263Plus = bValue;

    return bValue;
}


/*=============================================================================
    函数名        ：GetExclusiveSSRC
    功能        ：统一提供收发对象的SSRC的操作
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：


    返回值说明： 无
=============================================================================*/
u32 GetExclusiveSSRC()
{
    u32 dwRet = 0;

    if(NULL == g_hGetTimeStampSem)
    {
        return dwRet;
    }

    MEDIANET_SEM_TAKE(g_hGetTimeStampSem);

    if(0 == g_dwTimeStamp)
    {
        g_dwTimeStamp = OspTickGet();
    }
    else
    {
        g_dwTimeStamp++;
    }
    // 引入随机数，使时间戳和SSRC在Reset时差异变大  liuhf 2004.9.8
    srand(g_dwTimeStamp);
    dwRet = rand();

    if (0 == dwRet)
    {
        dwRet = rand();
    }

    MEDIANET_SEM_GIVE(g_hGetTimeStampSem);

    return dwRet;
}

/*=============================================================================
    函数名        ：SendCtlMsg
    功能        ：发送控制消息
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  s32 nMode        TSockState
                  u32 dwContext

    返回值说明： 无
=============================================================================*/
BOOL32 SendCtlMsg(u32 dwMode, void* pContext)
{
    static u32 dwIndex=0;
    BOOL32 bRet = FALSE;

    //保护一下
    if(g_hWatchSocket  == INVALID_SOCKET)
    {
        return bRet;
    }

    MEDIANET_SEM_TAKE(g_hCreateDelSem);

    s32 nSndNum  = 0;
    dwIndex++;

    SOCKADDR_IN  AddrIn;
    memset( &AddrIn, 0, sizeof(AddrIn));
    AddrIn.sin_family       = AF_INET;
    AddrIn.sin_addr.s_addr = inet_addr("127.0.0.1");
    AddrIn.sin_port           = htons(g_wWatchSockPort);

    TMsgData tMsgData;
    tMsgData.m_lluContext = (u64)pContext;
    tMsgData.m_dwMode = dwMode;
    tMsgData.m_dwIndex = dwIndex;

    if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo )
    {
        OspPrintf( TRUE, FALSE, "sendctlmsg nmode:[%d], pcontext:[%x], index:[%d]\n",
                    dwMode, pContext, dwIndex );
    }

    nSndNum = sendto( g_hWatchSocket, (s8*)&tMsgData, sizeof(tMsgData), 0,
                            (sockaddr *)&AddrIn, sizeof(SOCKADDR_IN) );

    if(nSndNum == sizeof(tMsgData))
    {
        bRet = TRUE;
    }

    MEDIANET_SEM_GIVE(g_hCreateDelSem);

    return bRet;
}

CKdvSocket g_cTestSock;
API void sendaddsock()
{
    SendCtlMsg(SOCK_CREATE, (void*)(&g_cTestSock));
}

API void senddelsock()
{
    SendCtlMsg(SOCK_DEL, (void*)(&g_cTestSock));
}

API void printsock();

/*=============================================================================
    函数名        ：KdvSocketStartUp
    功能        ：根据状态，判断是否创建全局线程对象。
    算法实现    ：（可选项）
    引用全局变量：
                g_nRefCount          线程引用计数
                g_hRcvTask            接收线程handle
                g_hSndTask            rtcp定时rtcp包上报线程handle
                g_hSem                 判断线程退出的信号量
                g_tRcvSockList             socket列表全局变量
                g_tRcvSockListTmp         socket列表增减所用的中转结构
                g_bSockInit            win32是否初始化了sock库的判断
                g_pcWatchSock       控制线程的socket
                g_bExitTask         判断任务退出
    输入参数说明：无


    返回值说明：成功MEDIANET_NO_ERROR 否则参见错误码定义
=============================================================================*/
u16 KdvSocketStartup()
{
    if(0 == g_nRefCount)
    {

#ifdef _VXWORKS_
        GetSSendInfoFromIni();

        if( TRUE == g_bUseSmoothSnd )
        {
            g_bBrdEthSndUseTimer = (BOOL32)BrdEthSndUseTimer();
            BrdSetEthSpeedLimit( 0, g_dwMinSendBitrate, g_dwMaxSendBitrate);
            BrdSetEthSpeedLimit( 1, g_dwMinSendBitrate, g_dwMaxSendBitrate);
            OspPrintf(TRUE, FALSE, "BrdEthSndUseTimer Ret=%d ... \n", g_bBrdEthSndUseTimer);
        }
#endif

        CKdvSocket** m_pptRcvSockUnit = (CKdvSocket**)malloc(g_nMaxRcvSockNum*sizeof(CKdvSocket*));
        if (NULL == m_pptRcvSockUnit)
        {
            return ERROR_SND_MEMORY;
        }
        memset(m_pptRcvSockUnit, 0, g_nMaxRcvSockNum*sizeof(CKdvSocket*));
        g_tRcvSockList.m_nSockCount = 0;
        g_tRcvSockList.m_pptSockUnit = m_pptRcvSockUnit;

        CKdvSocket** m_pptSndSockUnit = (CKdvSocket**)malloc(g_nMaxSndSockNum*sizeof(CKdvSocket*));
        if (NULL == m_pptSndSockUnit)
        {
            return ERROR_SND_MEMORY;
        }
        memset(m_pptSndSockUnit, 0, g_nMaxSndSockNum*sizeof(CKdvSocket*));
        g_tSndSockList.m_nSockCount = 0;
        g_tSndSockList.m_pptSockUnit = m_pptSndSockUnit;

        memset(&g_tMediaRcvList,0,sizeof(g_tMediaRcvList));
        memset(&g_tMediaSndList,0,sizeof(g_tMediaSndList));

        /*终止任务*/
        if(g_hRcvTask != NULL)
        {
            OspTaskTerminate(g_hRcvTask);
            g_hRcvTask = NULL;
        }
        if(g_hSndTask != NULL)
        {
            OspTaskTerminate(g_hSndTask);
            g_hSndTask = NULL;
        }

        /*清除ws2_32.dll的使用*/
        if(g_bSockInit)
        {
            SockCleanup();
            g_bSockInit=FALSE;
        }
        /*初始化winsock库*/
        g_bSockInit=SockInit();
        if(!g_bSockInit)
        {
            OspPrintf(1,0,"\n KdvSocketStartup SockInit Error \n");
            return ERROR_WSA_STARTUP;
        }

        /*创建一个同步二元信号量*/
        if(g_hSem!=NULL)
        {
            OspSemDelete(g_hSem);
            g_hSem=NULL;
        }
        if(!OspSemBCreate( &g_hSem))
        {
            g_hSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }

        if(g_hCreateDelSem!=NULL)
        {
            OspSemDelete(g_hCreateDelSem);
            g_hCreateDelSem=NULL;
        }
        //g_hCreateDelSem 开始有信号
        if(!OspSemBCreate( &g_hCreateDelSem))
        {
            g_hCreateDelSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hCreateDelSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }

        if(g_hMediaRcvSem!=NULL)
        {
            OspSemDelete(g_hMediaRcvSem);
            g_hMediaRcvSem=NULL;
        }
        //g_hMediaRcvSem 开始有信号
        if(!OspSemBCreate( &g_hMediaRcvSem))
        {
            g_hMediaRcvSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hMediaRcvSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }

        if(g_hMediaSndSem!=NULL)
        {
            OspSemDelete(g_hMediaSndSem);
            g_hMediaSndSem=NULL;
        }
        //g_hMediaSndSem 开始有信号
        if(!OspSemBCreate( &g_hMediaSndSem))
        {
            g_hMediaSndSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hMediaSndSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }

        if(g_hSndSockSem!=NULL)
        {
            OspSemDelete(g_hSndSockSem);
            g_hSndSockSem=NULL;
        }
        //g_hSndSockSem 开始有信号
        if(!OspSemBCreate( &g_hSndSockSem))
        {
            g_hSndSockSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hSndSockSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }

        if(g_hGetTimeStampSem != NULL)
        {
            OspSemDelete(g_hGetTimeStampSem);
            g_hGetTimeStampSem=NULL;
        }
        //g_hGetTimeStampSem 开始有信号
        if(!OspSemBCreate( &g_hGetTimeStampSem))
        {
            g_hGetTimeStampSem=NULL;
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hGetTimeStampSem Error  \n");
            return ERROR_CREATE_SEMAPORE;
        }
        //获取 SSRC 的基值
        g_dwTimeStamp = OspTickGet();
        srand(g_dwTimeStamp);


        /*创建控制套接字*/
        if( g_hRawSocket != INVALID_SOCKET )
        {
            SockClose( g_hRawSocket );
            g_hRawSocket = INVALID_SOCKET;
        }
        g_hRawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if( g_hRawSocket == INVALID_SOCKET )
        {
            OspPrintf(1,0,"\n KdvSocketStartup raw socket() Error  \n");
        //    return ERROR_SOCKET_CALL;
        }

        BOOL32 bOpt = TRUE;
        if (SOCKET_ERROR == setsockopt(g_hRawSocket, IPPROTO_IP, IP_HDRINCL, (s8 *)&bOpt, sizeof(bOpt)) )
        {
            OspPrintf(1,0,"\n KdvSocketStartup set raw socket() IP_HDRINCL  Error  \n");
        //    return ERROR_SOCKET_CALL;
        }

        if( g_hRawSocketSem != NULL )
        {
            OspSemDelete( g_hRawSocketSem );
            g_hRawSocketSem = NULL;
        }
        if( !OspSemBCreate( &g_hRawSocketSem ))
        {
            OspPrintf(1,0,"\n KdvSocketStartup OspSemBCreate g_hRawSocketSem Error  \n");
        //    return ERROR_CREATE_SEMAPORE;
        }

        if(g_hWatchSocket != INVALID_SOCKET)
        {
            SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
        }
        g_hWatchSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if( INVALID_SOCKET == g_hWatchSocket )
        {
            OspPrintf(1,0,"\n KdvSocketStartup WatchSock socket() Error  \n");
            return ERROR_SOCKET_CALL;
        }
        SOCKADDR_IN addr;
        memset(&addr, 0, sizeof(SOCKADDR_IN));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = 0;
        addr.sin_port        = htons(g_wWatchSockPort);

        while( (SOCKET_ERROR == bind(g_hWatchSocket, (sockaddr *)&addr, sizeof(SOCKADDR_IN))) &&
               (g_wWatchSockPort <= MAXWATCHSOCKPORT) )
        {
            g_wWatchSockPort++;
            addr.sin_port    = htons(g_wWatchSockPort);
        }

        if(g_wWatchSockPort>MAXWATCHSOCKPORT)
        {
            SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
            OspPrintf(1,0,"\n KdvSocketStartup WatchSock bind Error  \n");
            return ERROR_BIND_SOCKET;
        }

        g_bExitTask=FALSE;

        /* 创建分发套接字线程（任务） */
        g_hRcvTask = OspTaskCreate( SocketRcvTaskProc, "tRcvDataTask", 60, 512*1024, 0, 0 );
        if(g_hRcvTask == NULL)
        {
            SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
            OspPrintf(1,0,"\n KdvSocketStartup OspTaskCreate tRcvDataTask Error  \n");
            return ERROR_CREATE_THREAD;
        }

#ifdef MEDIANET_FOR_LINUX_NRU
        OspTaskSetPriority( g_hRcvTask , 40 );
#endif

         /* 创建rtcp定时rtcp包上报线程（任务） 这个任务级别较低 */
        g_hSndTask = OspTaskCreate( RtcpSndTaskProc, "tSndRtcpTask", 120, 512*1024, 0, 0 );
        if(g_hSndTask == NULL)
        {
            SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
            if(g_hRcvTask != NULL)
            {
                OspTaskTerminate(g_hRcvTask);
                g_hRcvTask = NULL;
            }
            OspPrintf(1,0,"\n KdvSocketStartup OspTaskCreate tSndRtcpTask Error  \n");
            return ERROR_RTCP_SET_TIMER;//ERROR_CREATE_THREAD;
        }


#ifdef _LINUX_
        OspRegCommand("kdvmedianethelp", (void*)kdvmedianethelp, "kdvmedianethelp()");
        OspRegCommand("pdinfo", (void*)pdinfo, "pdinfo(s32 nShowDebugInfo)");
        OspRegCommand("pbinfo", (void*)pbinfo, "pbinfo(BOOL32 bShowRcv, BOOL32 bShowSnd)");
        OspRegCommand("setdiscardspan", (void*)setdiscardspan, "setdiscardspan(s32 nDiscardSpan)");
        OspRegCommand("setrcvdiscardspan", (void*)setrcvdiscardspan, "setrcvdiscardspan(s32 nRcvDiscardSpan)");
        OspRegCommand("setrepeatsend", (void*)setrepeatsend, "setrepeatsend(s32 nRepeatSnd)");
        OspRegCommand("seth263dsend", (void*)seth263dsend, "seth263dsend(s32 nRtpDSend)");
        OspRegCommand("setconfuedspan", (void*)setconfuedspan, "setconfuedspan(s32 nConfuedSpan)");
        OspRegCommand("mediarecvframerate", (void*)mediarecvframerate, "mediarecvframerate(s32 nRecvId, s32 nDelaySecond)");
        OspRegCommand("mediasndframerate", (void*)mediasndframerate, "mediasndframerate(s32 nSndId, s32 nDelaySecond)");
        OspRegCommand("mediarelaystart", (void*)mediarelaystart, "mediarelaystart(s32 nRecvId, s8* pchIpStr, u16 wPort)");
        OspRegCommand("mediarelaystop", (void*)mediarelaystop, "mediarelaystop()");
        OspRegCommand("setsendrate", (void*)setsendrate, "setsendrate(s32 nSendId, u32 dwRate)");
        OspRegCommand("pssinfo", (void*)pssinfo, "pssinfo(s32 nSendId)");
        OspRegCommand("sendaddsock", (void*)sendaddsock, "sendaddsock");
        OspRegCommand("senddelsock", (void*)senddelsock, "senddelsock");
        OspRegCommand("printsock", (void*)printsock, "printsock");
        OspRegCommand("mediasendadd", (void*)mediasendadd, "mediasendadd(s32 nSendId, s8* pchIpStr, u16 wPort)");
        OspRegCommand("mediasenddel", (void*)mediasenddel, "mediasenddel(s32 nSendId, s8* pchIpStr, u16 wPort)");
        OspRegCommand("mediasocketinfo", (void*)mediasocketinfo, "mediasocketinfo()");
      OspRegCommand("setrtcpmode", (void*)setrtcpmode, "setrtcpmode()");
       OspRegCommand("printrtcpinfo", (void*)printrtcpinfo, "printrtcpinfo()");
        OspRegCommand("pbsend", (void*)pbsend, "pbsend()");
        OspRegCommand("pbrecv", (void*)pbrecv, "pbrecv()");
#endif
    }

    g_nRefCount++;

    return MEDIANET_NO_ERROR;
}

u16 KdvSocketStartupUnuseOspMemPool()
{
    g_bUseMemPool = FALSE;

    return KdvSocketStartup();
}
/*=============================================================================
    函数名        ：KdvSocketCleanUp
    功能        ：根据状态。判断是否退出全局线程对象
    算法实现    ：（可选项）
    引用全局变量：
                g_nRefCount          线程引用计数
                g_hRcvTask            接收线程handle
                g_hSndTask            rtcp定时rtcp包上报线程handle
                g_hSem                 判断线程退出的信号量
                g_tRcvSockList             socket列表全局变量
                g_tRcvSockListTmp         socket列表增减所用的中转结构
                g_bSockInit            win32是否初始化了sock库的判断
                g_pcWatchSock       控制线程的socket
                g_bExitTask         判断任务退出
    输入参数说明：无


    返回值说明：成功MEDIANET_NO_ERROR 否则参见错误码定义
=============================================================================*/
u16 KdvSocketCleanup()
{
    s32 nExit = g_nRefCount>0 ? g_nRefCount-- : 0;//判断是否清除一些全局成员。

    if(nExit==1)
    {
        g_bExitTask = TRUE;       //控制线程退出的变量置为TRUE

        if (g_hSndTask != NULL)
        {
            OspTaskTerminate(g_hSndTask);
            //windows下OspTaskCreate使用的是createThread，需要CloseHandle关闭线程句柄,否则会造成句柄泄露
            //OspTaskTerminate之后也需要CloseHandle
    #ifdef WIN32
            CloseHandle(g_hSndTask);
    #endif
            g_hSndTask = NULL;
        }

        if(g_hRcvTask != NULL)
        {
            if(g_hSem != NULL)
            {
                SendCtlMsg(SOCK_EXIT, (void*)0); //向线程发消息退出
                if( FALSE == OspSemTakeByTime( g_hSem,5000 ) )//等待线程退出
                {
                    OspTaskTerminate(g_hRcvTask);
                }
            }
            else
            {
                OspTaskTerminate(g_hRcvTask);
            }

            //windows下OspTaskCreate使用的是createThread，需要CloseHandle关闭线程句柄,否则会造成句柄泄露
            //OspTaskTerminate之后也需要CloseHandle
    #ifdef WIN32
            CloseHandle(g_hRcvTask);
    #endif
            g_hRcvTask = NULL;
        }

        if(g_hSem!=NULL)
        {
            OspSemDelete(g_hSem);
            g_hSem = NULL;
        }

        if(g_hCreateDelSem!=NULL)
        {
            OspSemDelete(g_hCreateDelSem);
            g_hCreateDelSem = NULL;
        }
        if(g_hMediaRcvSem!=NULL)
        {
            OspSemDelete(g_hMediaRcvSem);
            g_hMediaRcvSem = NULL;
        }
        if(g_hMediaSndSem!=NULL)
        {
            OspSemDelete(g_hMediaSndSem);
            g_hMediaSndSem = NULL;
        }
        if(g_hSndSockSem!=NULL)
        {
            OspSemDelete(g_hSndSockSem);
            g_hSndSockSem = NULL;
        }
        if(g_hGetTimeStampSem!=NULL)
        {
            OspSemDelete(g_hGetTimeStampSem);
            g_hGetTimeStampSem = NULL;
        }

        if( g_hRawSocketSem != NULL )
        {
            OspSemDelete( g_hRawSocketSem );
            g_hRawSocketSem = NULL;
        }

        //关闭watchsocket
        if(g_hWatchSocket != INVALID_SOCKET)
        {
            SockClose(g_hWatchSocket);
            g_hWatchSocket = INVALID_SOCKET;
        }

        if( g_hRawSocket != INVALID_SOCKET )
        {
            SockClose( g_hRawSocket );
            g_hRawSocket = INVALID_SOCKET;
        }

        if(g_bSockInit)
        {
            SockCleanup();
            g_bSockInit = FALSE;
        }

        if (g_tRcvSockList.m_pptSockUnit)
        {
            free(g_tRcvSockList.m_pptSockUnit);
            g_tRcvSockList.m_pptSockUnit = NULL;
        }
        g_tRcvSockList.m_nSockCount = 0;

        if (g_tSndSockList.m_pptSockUnit)
        {
            free(g_tSndSockList.m_pptSockUnit);
            g_tSndSockList.m_pptSockUnit = NULL;
        }
        g_tSndSockList.m_nSockCount = 0;

        memset(&g_tMediaRcvList, 0, sizeof(g_tMediaRcvList));
        memset(&g_tMediaSndList, 0, sizeof(g_tMediaSndList));
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函 数 名： CalcChecksum
    功    能： 校验和计算, 在外壳调用前保证为nSize偶数
    算法实现：
    全局变量：
    参    数： u16 *wBuffer
               s32 nSize    偶数
    返 回 值： u16
-----------------------------------------------------------------------------
    修改记录：
    日  期        版本        修改人        走读人    修改内容
    2005/1/13   3.5            万春雷                  创建
=============================================================================*/
u16 CalcChecksum(u16 *wBuffer, s32 nSize)
{
    unsigned long dwCkSum = 0;

    while (nSize > 1)
    {
        dwCkSum += *wBuffer++;
        nSize  -= sizeof(u16);
    }
    if(nSize)
    {
        dwCkSum += *(u8*)wBuffer;
    }

    dwCkSum  = (dwCkSum >> 16) + (dwCkSum & 0xffff);
    dwCkSum += (dwCkSum >> 16);

    return (u16)(~dwCkSum);
}

#define CHECK_THREAD_INIT { if(g_nRefCount == 0) return FALSE;}//

//类成员初始化
CKdvSocket::CKdvSocket()
{
    m_hSocket     = INVALID_SOCKET;
    m_hSynSem    = NULL;
    m_hCreateSynSem = NULL;
    memset(&m_tAddrIn, 0, sizeof(m_tAddrIn));
    memset(&m_tCallBack, 0, sizeof(m_tCallBack));
    memset(&m_tCreateSock, 0, sizeof(m_tCreateSock));

    m_bUseRawSend= FALSE;

    m_bMultiCast = FALSE;
    m_dwIPNum    = 0;
    memset(&m_tdwIPList, 0, sizeof(m_tdwIPList));

    m_nSndBufSize = 0;
    m_nRcvBufSize = 0;

    if(!OspSemCCreate(&m_hSynSem,0,1))
    {
        m_hSynSem = NULL;
        return;
    }
    if(!OspSemCCreate(&m_hCreateSynSem,0,1))
    {
        m_hCreateSynSem = NULL;
        return;
    }

    m_bySockMediaType = SOCKET_TYPE_AUDIO;

    m_dwSrcIP  = 0;
    m_wSrcPort = 0;
    m_dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM;

//    InitThread();
}

//销毁类
CKdvSocket::~CKdvSocket()
{
    Close(TRUE);
    if(m_hSynSem!=NULL)
    {
        OspSemDelete(m_hSynSem);
        m_hSynSem = NULL;
    }
    if(m_hCreateSynSem != NULL)
    {
        OspSemDelete(m_hCreateSynSem);
        m_hCreateSynSem = NULL;
    }
//    CloseThread();
}

/*=============================================================================
    函数名        ：Create
    功能        ：申请创建Socket
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   nSocketType     socket type (SOCK_DGRAM,SOCK_STREAM)
                   nSocketPort     socket port;
                   dwLocalAddr     local IP;
                   dwMultiAddr     Multicast IP;

    返回值说明：成功TRUE,失败FALSE;
=============================================================================*/
BOOL32 CKdvSocket::Create( s32 nSocketType, u16 wSocketPort,
                           u32 dwLocalAddr, u32 dwMultiAddr, BOOL32 bRcv, u32 dwFlag, void* pRegFunc, void* pUnregFunc)
{
    CHECK_THREAD_INIT

    BOOL32 bRet = FALSE;

    if(m_hCreateSynSem == NULL || m_hSynSem == NULL)
    {
        return bRet;
    }

    Close(TRUE);
    memset(&m_tCreateSock, 0, sizeof(m_tCreateSock));
    m_tCreateSock.m_dwLocalAddr = dwLocalAddr;
    m_tCreateSock.m_dwMultiAddr = dwMultiAddr;
    m_tCreateSock.m_nSocketType = nSocketType;
    m_tCreateSock.m_wSocketPort = wSocketPort;
    m_tCreateSock.m_bRcv        = bRcv;
    m_tCreateSock.m_pRegFunc    = pRegFunc;
    m_tCreateSock.m_pUnregFunc = pUnregFunc;

    m_dwFlag = dwFlag;

    if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo )
    {
        OspPrintf(TRUE, FALSE, "CKdvSocket::Create m_dwLocalAddr:[%x], m_dwMultiAddr:[%x],m_nSocketType:[%d], m_wSocketPort:[%d], m_bRcv:[%d], m_dwRegFunc:[%x], m_dwUnregFunc:[%x], m_dwFlag:[%x]\n",
                                m_tCreateSock.m_dwLocalAddr, m_tCreateSock.m_dwMultiAddr,
                                m_tCreateSock.m_nSocketType, m_tCreateSock.m_wSocketPort,
                                m_tCreateSock.m_bRcv, m_tCreateSock.m_pRegFunc, m_tCreateSock.m_pUnregFunc,
                                m_dwFlag);
    }

    //对于接收套结子，由辅助线程统一注册记录，并进行创建,并回馈消息，同步创建结果
    if(TRUE == bRcv)
    {
        if( m_dwFlag & MEDIANETRCV_FLAG_FROM_RECVFROM )
        {
            m_bSuccess = FALSE;

            if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo )
            {
                OspPrintf(TRUE, FALSE, "CKdvSocket::Create msg:[%d], context:[%d]\n", SOCK_CREATE, (void*)this);
            }
            bRet = SendCtlMsg(SOCK_CREATE, (void*)this);
            if(bRet == FALSE)
            {
                PrintErrMsg("Socket Create SendCtlMsg Error, Print Argument", FALSE);
                OspPrintf(1,0,"g_hWatchSocket:%d  \n", g_hWatchSocket);
            }

            if(bRet)
            {
                if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo )
                {
                    //printf( "In CKdvSocket::Create %d\n" , OspTickGet() );
                    OspPrintf( TRUE , FALSE , "In CKdvSocket::Create %d\n" , OspTickGet() );
                }
                 if(OspSemTakeByTime( m_hCreateSynSem, 2000 ) == FALSE)
                {
                    PrintErrMsg("Socket Create OspSemTakeByTime Error\n");
                }
                if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo )
                {
                    //printf( "Out CKdvSocket::Create %d Success %d\n\n" , OspTickGet() , m_bSuccess );
                    OspPrintf( TRUE , FALSE , "Out CKdvSocket::Create %d Success %d\n\n" , OspTickGet() , m_bSuccess );
                }
                bRet = m_bSuccess;
            }
        }
        else
        {
            bRet = Create( FALSE );
        }
    }
    else//对于发送用套结子，直接由调用线程创建
    {
        bRet = Create(TRUE);

        if(bRet)
        {
            MEDIANET_SEM_TAKE(g_hSndSockSem);

            //加入发送套结子链表中记录对象指针
            BOOL32 bFind = FALSE;
            if(g_tSndSockList.m_nSockCount < MAX_SND_NUM)
            {
                for(s32 nPos=0; nPos<g_tSndSockList.m_nSockCount; nPos++)
                {
                    if(g_tSndSockList.m_pptSockUnit[nPos] == this)
                    {
                        bFind = TRUE;
                        break;
                    }
                }
                if(FALSE == bFind)
                {
                    g_tSndSockList.m_nSockCount++;
                    g_tSndSockList.m_pptSockUnit[g_tSndSockList.m_nSockCount-1] = this;
                }
            }

            MEDIANET_SEM_GIVE(g_hSndSockSem);
        }
    }

    if(bRet == FALSE)
    {
        PrintErrMsg("Socket Create Error");
    }

    return bRet;
}

/*=============================================================================
    函数名        ：Create
    功能        ：创建socket对象 绑定socket，设置组播、广播选项
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：bSend - TRUE- 套接字用于发送


    返回值说明：成功TRUE,失败FALSE;
=============================================================================*/
BOOL32 CKdvSocket::Create(BOOL32 bSend /*=FALSE*/ )
{
    BOOL32 bRet = FALSE;
    u16 wRet = 0;
    u32 dwRet = 0;

    // 用于发送的SOCKET 采用RawSocket，从而设置其 IP header-TOS Field
    if( TRUE == bSend && TRUE == g_bUseRawSend )
    {
        m_hSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);//, NULL, 0, 0);
        if( INVALID_SOCKET == m_hSocket )
        {
            PrintErrMsg("Socket socket() Error");
            return FALSE;
        }

        // Enable the IP header include option
        //
        BOOL32 bOpt = TRUE;
        if (SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_IP, IP_HDRINCL, (s8 *)&bOpt, sizeof(bOpt)) )
        {
            PrintErrMsg("setsockopt(IP_HDRINCL) Error");
            return FALSE;
        }

#ifdef WIN32
        unsigned long dwOn;
        s32 nRet;

        //设置非阻塞
        dwOn = TRUE;
        nRet = ioctlsocket(m_hSocket, FIONBIO, &dwOn);

        //设置ICMP不处理
        dwOn = FALSE;
        nRet = ioctlsocket(m_hSocket, SIO_UDP_CONNRESET, &dwOn);
#else
        u32 dwOn;
        s32 nRet;

        //设置非阻塞
        dwOn = TRUE;
        nRet = ioctl(m_hSocket, FIONBIO,  &dwOn);
#endif

        // 如果用户没有对 LocalIP 进行有效设置，则列举当前的一个有效ip
        if( ( 0 == m_tCreateSock.m_dwLocalAddr ) ||
            ( inet_addr("127.0.0.1") == m_tCreateSock.m_dwLocalAddr) ||
            ( TRUE == IsBroadCastAddr(m_tCreateSock.m_dwLocalAddr) ) ||
            ( TRUE == IsMultiCastAddr(m_tCreateSock.m_dwLocalAddr) ) )
        {
            u32 dwOldIP = m_tCreateSock.m_dwLocalAddr;
            if( FALSE == ResetLocalIP() )
            {
                if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf( TRUE, FALSE, "[Create] ResetLocalIP Err. LocalAddr=%d \n", m_tCreateSock.m_dwLocalAddr );
                }
                else
                {
                    OspPrintf( TRUE, FALSE, "[Create] ResetLocalIP OK. OldLocalAddr=%d, NewLocalAddr=%d \n",
                        dwOldIP, m_tCreateSock.m_dwLocalAddr );
                }
            }
        }
    }
    else
    {
        //不是从网络接收接收
        if( bSend == FALSE && !(m_dwFlag&MEDIANETRCV_FLAG_FROM_RECVFROM))
        {
            m_tMedianetNetAddr.m_dwIp = m_tCreateSock.m_dwLocalAddr;
            m_tMedianetNetAddr.m_wPort = m_tCreateSock.m_wSocketPort;
            //如果是组播或广播地址，则替换掉
            if( IsMultiCastAddr(m_tCreateSock.m_dwMultiAddr) || IsBroadCastAddr(m_tCreateSock.m_dwMultiAddr))
            {
                m_tMedianetNetAddr.m_dwIp = m_tCreateSock.m_dwMultiAddr;
            }

/*            if( m_dwFlag & MEDIANETRCV_FLAG_FROM_DATASWITCH )
            {
                dsRegRcvChannel( (TDSNetAddr*)&m_tMedianetNetAddr, MedianetDataSwitchCallback, (u32)this );
            }
            else if( m_dwFlag & MEDIANETRCV_FLAG_FROM_RPCTRL )
            {
                RPRegSndChannel( (TNetAddr*)&m_tMedianetNetAddr, MedianetRpctrlCallBack, (u32)this );
            }
            else if( m_dwFlag & MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD )
            {
                U2ARegSndChannel( (TU2ANetAddr*)&m_tMedianetNetAddr, MedianetU2ACallback, (u32)this );
            }
*/            if( (m_dwFlag & MEDIANETRCV_FLAG_FROM_DATASWITCH ))
            {
                    void* pRegFunc = m_tCreateSock.m_pRegFunc;
                    if( pRegFunc == 0 )
                    {
                        OspPrintf(TRUE, FALSE, "dwregfunc is 0, error param\n");
                        //printf( "dwregfunc is 0, error param\n");
                        return FALSE;
                    }

                    dwRet = ((u32(*)(void*,void*,void*))pRegFunc)((void*)&m_tMedianetNetAddr, (void*)MedianetDataSwitchCallback, (void*)this);
                    if( dwRet > 0 )
                    {
                        bRet = TRUE;
                    }
            }
            else if(    (m_dwFlag & MEDIANETRCV_FLAG_FROM_RPCTRL        ) ||
                        (m_dwFlag & MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD    ) )
            {
                void* pRegFunc = m_tCreateSock.m_pRegFunc;
                if( pRegFunc == NULL )
                {
                    OspPrintf(TRUE, FALSE, "dwregfunc is 0, error param\n");
                    //printf( "dwregfunc is 0, error param\n");
                    return FALSE;
                }

                wRet = ((u16(*)(void*,void*,void*))pRegFunc)((void*)&m_tMedianetNetAddr, (void*)MedianetRpctrlCallBack, (void*)this);
                if( wRet == 0 )
                {
                    bRet = TRUE;
                }
            }
            else if( m_dwFlag & MEDIANETRCV_FLAG_TEST )
            {
//                OspTaskCreate( Medianetrcvtest, "Medianetrcvtest", 100, 128<<10, (u32)this, 0);
            }
            else
            {

            }

            return bRet;
        }

        //生成socket对象
        m_hSocket = socket( AF_INET, m_tCreateSock.m_nSocketType, 0 );
        if( INVALID_SOCKET == m_hSocket )
        {
            PrintErrMsg("Socket socket() Error");
            return FALSE;
        }

#ifdef WIN32
        BOOL bset = ::SetHandleInformation((HANDLE)m_hSocket, HANDLE_FLAG_INHERIT, 0);
#endif

        // set reuse option
        s32 nReuseAddr = 1;
        if( SOCKET_ERROR  == setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR,
            (s8 *)&nReuseAddr, sizeof(nReuseAddr)) )
        {
            PrintErrMsg("Socket setsockopt SO_REUSEADDR Error");
            Close();
            return FALSE;
        }

        //绑定socket
        SOCKADDR_IN addr;
        memset(&addr, 0, sizeof(SOCKADDR_IN));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = m_tCreateSock.m_dwLocalAddr; //ADDR_ANY
        addr.sin_port        = htons(m_tCreateSock.m_wSocketPort);
        if( SOCKET_ERROR == bind(m_hSocket, (sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
        {
            PrintErrMsg("Socket bind Error");
            Close();
            return FALSE;
        }

#ifdef WIN32
        unsigned long dwOn;
        s32 nRet;

        //设置非阻塞
        dwOn = TRUE;
        nRet = ioctlsocket(m_hSocket, FIONBIO, &dwOn);

        //设置ICMP不处理
        dwOn = FALSE;
        nRet = ioctlsocket(m_hSocket, SIO_UDP_CONNRESET, &dwOn);
#else
        u32 dwOn;
        s32 nRet;

        //设置非阻塞
        dwOn = TRUE;
        nRet = ioctl(m_hSocket, FIONBIO,  &dwOn);
#endif
    }

    if( FALSE == bSend )
    {
        //Set multicast option
        if( IsMultiCastAddr(m_tCreateSock.m_dwMultiAddr) )
        {
            //列举当前设置的所有有效地ip
            m_dwIPNum = 0;
            memset(&m_tdwIPList, 0, sizeof(m_tdwIPList));

#ifdef _LINUX_
            //2004-09-29 需要osplinux版本提供OspAddrListGet函数
            m_dwIPNum      = 1;
            m_tdwIPList[0] = m_tCreateSock.m_dwLocalAddr;
#else
            m_dwIPNum = OspAddrListGet(m_tdwIPList, MAX_LOCAL_IP_NUM);
            if( 0 == m_dwIPNum )
            {
                PrintErrMsg("Socket setsockopt IP_ADD_MEMBERSHIP: OspAddrListGet Error");
                Close();
                return FALSE;
            }
#endif

#ifdef WIN32
            //对于win32平台，不应进行多地址加入组播组，而应设置为ADDR_ANY
            m_dwIPNum      = 1;
            m_tdwIPList[0] = ADDR_ANY;
#endif

            for(u16 dwAdapter=0; dwAdapter<m_dwIPNum; dwAdapter++)
            {
                if((inet_addr( "127.0.0.1" ) != m_tdwIPList[dwAdapter]))
                {
                    if( FALSE == SetMCastOpt(m_tdwIPList[dwAdapter], m_tCreateSock.m_dwMultiAddr, TRUE) )
                    {
                        OspPrintf(1, 0, "Socket setsockopt IP_ADD_MEMBERSHIP: SetMCastOpt Error,ip:%x, dwAdapter:%d  \n",
                                  m_tdwIPList[dwAdapter], dwAdapter);
                        Close();
                        return FALSE;
                    }
                    else
                    {
                        if(3 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                        {
                            OspPrintf(1, 0, "Socket setsockopt IP_ADD_MEMBERSHIP: SetMCastOpt OK,ip:%x, dwAdapter:%d  \n",
                                m_tdwIPList[dwAdapter], dwAdapter);
                        }
                    }
                }
            }

            m_bMultiCast = TRUE;
        }

        //Set Broadcast option
        if( IsBroadCastAddr(m_tCreateSock.m_dwMultiAddr) )
        {
            if( FALSE == SetBCastOpt() )
            {
                PrintErrMsg("Socket setsockopt SO_BROADCAST Error");
                Close();
                return FALSE;
            }
        }
    }

#ifdef MEDIANET_FOR_LINUX_NRU
    s32 nBufSize = 2*1024*1024;
    setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (s8 *)&nBufSize, sizeof(s32));
    m_nSndBufSize = nBufSize;

    setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF,(s8 *)&nBufSize, sizeof(s32));
    m_nRcvBufSize = nBufSize;
#else //MEDIANET_FOR_LINUX_NRU

#ifdef _LINUX_
#ifndef _IOS_   //for ios complie
    s32 optVal1 = 1;
    setsockopt (m_hSocket, SOL_SOCKET, SO_BSDCOMPAT, (char*)&optVal1, sizeof (optVal1));
#endif
#endif

    s32 nBufSize = MAX_NET_SND_BUF_LEN;
    s32 nTotalBufSize = MAX_NET_SND_BUF_LEN;
    s32 nRealBufSize = 0;
    s32 nSize = sizeof(nRealBufSize);

    /*
    调整套结子的接收、发送缓冲大小 ;
    1.最大接收缓冲大小为 128000*8*4 bytes, 最大发送缓冲大小为 128000*4 bytes
      缺省及最小收发缓冲大小为 128000*2 bytes ;
    2.用于发送的套接字，其发送缓冲大小将由发送最大值开始尝试,接收缓冲大小为缺省值
      用于接收的套接字，其接收缓冲大小将由接收最大值开始尝试,发送缓冲大小为缺省值 .
    */

    if(FALSE == bSend)
    {
        nBufSize = MIN_NET_BUF_LEN;
        nTotalBufSize = MIN_NET_BUF_LEN;
    }

    //Set Send buffer length
    while(nBufSize >= MIN_NET_BUF_LEN)
    {
        if( SOCKET_ERROR == setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8 *)&nTotalBufSize, sizeof(s32)) )
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }

#ifdef _LINUX_
        if( SOCKET_ERROR == getsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8*)&nRealBufSize, (socklen_t*)&nSize) )
#else
        if( SOCKET_ERROR == getsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8*)&nRealBufSize, &nSize) )
#endif
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else if(nRealBufSize < nBufSize)
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else
        {
            break;
        }
    }

    if(nBufSize < MIN_NET_BUF_LEN)
    {
#ifndef _LINUX_
        PrintErrMsg("Socket setsockopt SO_SNDBUF Error");
#endif
    }

    m_nSndBufSize = nBufSize;

    nRealBufSize = 0;
    if(TRUE == bSend)
    {
        nBufSize = MIN_NET_BUF_LEN;
        nTotalBufSize = MIN_NET_BUF_LEN;
    }
    else
    {
        nBufSize = MAX_NET_RCV_BUF_LEN;
        nTotalBufSize = MAX_NET_RCV_BUF_LEN;
    }

    //Set recieve buffer length
    while(nBufSize >= MIN_NET_BUF_LEN)
    {
        if( SOCKET_ERROR == setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8 *)&nTotalBufSize, sizeof(s32)) )
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }

#ifdef _LINUX_
        if( SOCKET_ERROR == getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8*)&nRealBufSize, (socklen_t*)&nSize) )
#else
        if( SOCKET_ERROR == getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8*)&nRealBufSize, &nSize) )
#endif
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else if(nRealBufSize < nBufSize)
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else
        {
            break;
        }
    }

    if(nBufSize < MIN_NET_BUF_LEN)
    {
#ifndef _LINUX_
        PrintErrMsg("Socket setsockopt SO_RCVBUF Error");
        //modify by hual 2006-02-2, cannot correct set buf under linux
        //Close();
        //return FALSE;
#endif
    }

    m_nRcvBufSize = nBufSize;

#endif //MEDIANET_FOR_LINUX_NRU

    if( TRUE == bSend && TRUE == g_bUseRawSend )
    {
        m_bUseRawSend = TRUE;
    }
    else
    {
        m_bUseRawSend = FALSE;
    }

    //当套介子发送的数据包未被接收时，禁止 回馈的ICMP包 的通知
    // disable  new behavior using IOCTL: SIO_UDP_CONNRESET

    return TRUE;
}

/*=============================================================================
    函数名        ：Close
    功能        ：关闭socket
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：bNotifyThread
                  是否通知线程去除该对象在全局数组中的记录

    返回值说明：成功TRUE,失败FALSE;
=============================================================================*/
void CKdvSocket::Close(BOOL32 bNotifyThread /* = FALSE */)
{
    if(m_hCreateSynSem == NULL || m_hSynSem == NULL)
    {
        return;
    }

    //从网络接收才会有socket
    if(INVALID_SOCKET == m_hSocket && (m_dwFlag & MEDIANETRCV_FLAG_FROM_RECVFROM) )
    {
        return;
    }

/*    if( m_dwFlag & MEDIANETRCV_FLAG_FROM_DATASWITCH )
    {
        dsUnRegRcvChannel((TDSNetAddr*)&m_tMedianetNetAddr, MedianetDataSwitchCallback);
    }
    else if( m_dwFlag & MEDIANETRCV_FLAG_FROM_RPCTRL )
    {
        RPUnRegSndChannel( (TNetAddr*)&m_tMedianetNetAddr );
    }
    else if( m_dwFlag & MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD )
    {
        U2AUnRegSndChannel( (TU2ANetAddr*)&m_tMedianetNetAddr );
    }
*/
    if( (m_dwFlag & MEDIANETRCV_FLAG_FROM_DATASWITCH ) )
    {
        void* pUnregFunc = m_tCreateSock.m_pUnregFunc;
        if(NULL == pUnregFunc)
        {
            OspPrintf(TRUE, FALSE, "dwUnregFunc is 0, error param\n");
            //printf( "dwUnregFunc is 0, error param\n");
            return;
        }
//        OspPrintf(TRUE, FALSE, "close MedianetDataSwitchCallback:[%x]\n", MedianetDataSwitchCallback);
        ((u32(*)(void*,void*,void*))pUnregFunc)((void*)&m_tMedianetNetAddr, (void*)MedianetDataSwitchCallback, (void*)this);
    }
    else if(    (m_dwFlag & MEDIANETRCV_FLAG_FROM_RPCTRL ) ||
                (m_dwFlag & MEDIANETRCV_FLAG_FROM_RAWDOWNLOAD ) )
    {
        void* pUnregFunc = m_tCreateSock.m_pUnregFunc;
        if( pUnregFunc == 0 )
        {
            OspPrintf(TRUE, FALSE, "dwUnregFunc is 0, error param\n");
            //printf( "dwUnregFunc is 0, error param\n");
            return;
        }
        ((u16(*)(void*))pUnregFunc)((void*)&m_tMedianetNetAddr);
    }
    else if( m_dwFlag & MEDIANETRCV_FLAG_TEST )
    {
    }
    else
    {
    }

    if(bNotifyThread && m_tCreateSock.m_bRcv && ( m_dwFlag&MEDIANETRCV_FLAG_FROM_RECVFROM ) )
    {
        //对于接收套结子，由辅助线程统一注册记录，并进行删除,并回馈消息，同步删除结果

        BOOL32 bRet = SendCtlMsg(SOCK_DEL,(void*)this);//退出全局socket列表

        if(FALSE == bRet)
        {
            PrintErrMsg("Socket Close SendCtlMsg Error, Print Argument", FALSE);
            OspPrintf(1,0,"g_hWatchSocket:%d  \n", g_hWatchSocket);
        }
        else
        {
            if(m_hSynSem != NULL)
            {
                if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    //printf( "In CKdvSocket::Close %d\n" , OspTickGet() );
                    OspPrintf( TRUE , FALSE , "In CKdvSocket::Close %d\n" , OspTickGet() );
                }
                if (OspSemTakeByTime( m_hSynSem, 2000 ) == FALSE) //等待socket退出
                {
                    PrintErrMsg("Socket Close OspSemTakeByTime Error\n");
                }
                if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    //printf( "Out CKdvSocket::Close %d\n\n" , OspTickGet() );
                    OspPrintf( TRUE , FALSE , "Out CKdvSocket::Close %d\n\n" , OspTickGet() );
                }
            }
        }
    }

    if(bNotifyThread && !m_tCreateSock.m_bRcv)
    {
        MEDIANET_SEM_TAKE(g_hSndSockSem);

        //从发送套结子链表中删除对象指针
        s32 nTemp = 0;
        for(s32 i=0; i<g_tSndSockList.m_nSockCount; i++)
        {
            if(g_tSndSockList.m_pptSockUnit[i] == this)
            {
                continue;
            }

            g_tSndSockList.m_pptSockUnit[nTemp] = g_tSndSockList.m_pptSockUnit[i];
            if (nTemp != i)
            {
                g_tSndSockList.m_pptSockUnit[i] = NULL;
            }
            nTemp++;
        }
        g_tSndSockList.m_nSockCount = nTemp;
        MEDIANET_SEM_GIVE(g_hSndSockSem);
    }

    if( TRUE == m_bMultiCast )
    {
        //close multicast option
        for(u16 dwAdapter=0; dwAdapter<m_dwIPNum; dwAdapter++)
        {
            if((inet_addr( "127.0.0.1" ) != m_tdwIPList[dwAdapter]))
            {
                ip_mreq    tMreq;
                memset( &tMreq, 0, sizeof(ip_mreq));
                tMreq.imr_multiaddr.s_addr = m_tCreateSock.m_dwMultiAddr;
                tMreq.imr_interface.s_addr = m_tdwIPList[dwAdapter];
                setsockopt( m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (s8*)&tMreq, sizeof(ip_mreq) );
            }
        }
        m_bMultiCast = FALSE;
        m_dwIPNum    = 0;
        memset(&m_tdwIPList, 0, sizeof(m_tdwIPList));
    }
    if(bNotifyThread && m_tCreateSock.m_bRcv && ( m_dwFlag&MEDIANETRCV_FLAG_FROM_RECVFROM ) )
    {
    }
    else
    {
        //close socket
        if( INVALID_SOCKET != m_hSocket )
        {
            BOOL32 bRet = SockClose(m_hSocket);
            if (FALSE == bRet)
            {
                OspPrintf(TRUE, FALSE, "SockClose error!(%d) \n", m_hSocket);
            }
            m_hSocket = INVALID_SOCKET;
        }
    }
    memset(&m_tAddrIn, 0, sizeof(m_tAddrIn));

    m_bUseRawSend = FALSE;

    m_dwSrcIP  = 0;
    m_wSrcPort = 0;

    m_dwFlag = MEDIANETRCV_FLAG_FROM_RECVFROM;

    return;
}

/*=============================================================================
    函数名        ：IsMultiCastAddr
    功能        ：判断是否是组播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 CKdvSocket::IsMultiCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    u8 byLOByte = *pch;

    if( (byLOByte >= 224) && (byLOByte <= 239))
        return TRUE;
    else
        return FALSE;
}

/*=============================================================================
    函数名        IsBroadCastAddr
    功能        ：判断是否是广播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 CKdvSocket::IsBroadCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    pch += 3;

    u8 byHIByte = *pch;

    if( 255 == byHIByte )
        return TRUE;
    else
        return FALSE;
}

static u32 getLocalIp()
{

    SOCKHANDLE hSocket;
    hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//, NULL, 0, 0);
    if( INVALID_SOCKET == hSocket)
    {
        return 0;
    }

    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));

#ifdef _LINUX_
    socklen_t nLen;
#else
    s32 nLen;
#endif

    nLen = sizeof(sockaddr);

    s32 nRet;

    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr("202.0.0.0");
    addr.sin_port        = htons(80);

    nRet = connect(hSocket, (sockaddr *)&addr,sizeof(SOCKADDR_IN));
    if (nRet != 0)
    {
        OspPrintf(1 , 0 , "getLocalIp connect ret:%d, You Need config default route.\n", nRet);
        //printf("getLocalIp connect ret:%d, You Need config default route.\n", nRet);
        SockClose(hSocket);
        return 0;
    }

    nLen = sizeof(sockaddr);
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    nRet = getsockname(hSocket,(sockaddr *)&addr, &nLen);
    if (nRet != 0)
    {
        OspPrintf(1, 0 , "getLocalIp getsockname ret:%d\n", nRet);
        //printf("getLocalIp getsockname ret:%d\n", nRet);
        SockClose(hSocket);
        return 0;
    }

    SockClose(hSocket);

    return addr.sin_addr.s_addr;
}
/*=============================================================================
    函 数 名： ResetLocalIP
    功    能： 当本端源ip为无效ip或者环回、组播ip时，以一个本端有效ip替换
    算法实现：
    全局变量：
    参    数：
    返 回 值： TRUE - 操作成功
-----------------------------------------------------------------------------
    修改记录：
    日  期        版本        修改人        走读人    修改内容
    2005/01/31  3.5            万春雷                  创建
=============================================================================*/
BOOL32 CKdvSocket::ResetLocalIP()
{
    BOOL32 bRet = FALSE;

    if( ( 0 == m_tCreateSock.m_dwLocalAddr ) ||
        ( inet_addr("127.0.0.1") == m_tCreateSock.m_dwLocalAddr) ||
        ( TRUE == IsBroadCastAddr(m_tCreateSock.m_dwLocalAddr) ) ||
        ( TRUE == IsMultiCastAddr(m_tCreateSock.m_dwLocalAddr) ) )
    {
        u32  tdwIPList[MAX_LOCAL_IP_NUM];  //列举到的当前设置的所有有效ip
        u16  dwIPNum;                      //列举到ip 数目

    #ifdef _LINUX_
        // 需要osplinux版本提供OspAddrListGet函数
        tdwIPList[0] = getLocalIp();
        if (0 == tdwIPList[0])
        {
            dwIPNum = 0;
        }
        else
        {
            dwIPNum = 1;
        }
    #else
        dwIPNum = OspAddrListGet(tdwIPList, MAX_LOCAL_IP_NUM);
        if( 0 == dwIPNum )
        {
            PrintErrMsg("OspAddrListGet Error");
            return bRet;
        }
    #endif

        for(u16 dwAdapter=0; dwAdapter<dwIPNum; dwAdapter++)
        {
            if((inet_addr( "127.0.0.1" ) != tdwIPList[dwAdapter]))
            {
                m_tCreateSock.m_dwLocalAddr = tdwIPList[dwAdapter];
                bRet = TRUE;
                break;
            }
        }
    }
    else
    {
        bRet = TRUE;
    }

    return bRet;
}

/*=============================================================================
    函数名        SetMCastOpt
    功能        ：加入组播组
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  dwLocalIP－ 接口IP(net order)
                  dwMCastIP－ 组播组IP地址(net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 CKdvSocket::SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv /*=FALSE*/)
{
    BOOL32 bRet = FALSE;

    if(m_hCreateSynSem == NULL || m_hSynSem == NULL)
    {
        return bRet;
    }
    if(INVALID_SOCKET == m_hSocket)
    {
        return bRet;
    }

    m_tCreateSock.m_dwMultiAddr = dwMCastIP;

    s8 byttl = g_byTTL;
    if(SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL,
        (s8*)&byttl, sizeof(byttl)))
    {
        return bRet;
    }

    if(FALSE == bRcv)
    {
        // 如果用户没有对 LocalIP 进行有效设置，则列举当前的一个有效ip
        if( ( 0 == dwLocalIP ) ||
            ( inet_addr("127.0.0.1") == dwLocalIP) ||
            ( TRUE == IsBroadCastAddr(dwLocalIP) ) ||
            ( TRUE == IsMultiCastAddr(dwLocalIP) ) )
        {
            u32 dwOldIP = m_tCreateSock.m_dwLocalAddr;
            if( FALSE == ResetLocalIP() )
            {
                if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf( TRUE, FALSE, "[SetMCastOpt] ResetLocalIP Err. LocalAddr=%d \n", m_tCreateSock.m_dwLocalAddr );
                }
            }
            else
            {
                if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf( TRUE, FALSE, "[SetMCastOpt] ResetLocalIP OK. OldLocalAddr=%d, NewLocalAddr=%d \n",
                                            dwOldIP, m_tCreateSock.m_dwLocalAddr );
                }
            }
        }
        else
        {
            m_tCreateSock.m_dwLocalAddr = dwLocalIP;
        }

        in_addr  ifaddr;
        ifaddr.s_addr = m_tCreateSock.m_dwLocalAddr ;
        if( SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_IF,
            (s8*)&ifaddr, sizeof(ifaddr)) )
        {
            return bRet;
        }

        s8 byLoop = 0;
        if( SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
            (s8*)&byLoop, sizeof(byLoop)) )
        {
            return bRet;
        }

        if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "Socket setsockopt IP_MULTICAST_IF: SetMCastOpt OK,ip:%x \n", dwLocalIP);
        }
    }
    else
    {
        m_tCreateSock.m_dwLocalAddr = dwLocalIP;

        ip_mreq    tMreq;
        memset( &tMreq, 0, sizeof(ip_mreq));
        tMreq.imr_multiaddr.s_addr = m_tCreateSock.m_dwMultiAddr;
        tMreq.imr_interface.s_addr = m_tCreateSock.m_dwLocalAddr;
        if( SOCKET_ERROR == setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (s8*)&tMreq, sizeof(ip_mreq)) )
        {
            return bRet;
        }

        if(3 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "Socket setsockopt IP_MULTICAST_IF: SetMCastOpt OK,ip:%x \n", dwLocalIP);
        }
    }

    return TRUE;
}

/*=============================================================================
    函数名        SetBCastOpt
    功能        ：加入广播组
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 CKdvSocket::SetBCastOpt( )
{
    BOOL32 bRet = FALSE;

    if(m_hCreateSynSem == NULL || m_hSynSem == NULL)
    {
        return bRet;
    }
    if(INVALID_SOCKET == m_hSocket)
    {
        return bRet;
    }

    s32 optval = 1;
    if( SOCKET_ERROR == setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST,
        (s8*)&optval, sizeof(optval)) )
    {
        return bRet;
    }

    return TRUE;
}

/*=============================================================================
    函 数 名： SetSockMediaType
    功    能： 设置媒体类型, 用于发送音频还是视频，类型不同TOS设置不一样
    算法实现：
    全局变量：
    参    数： u8 bySockMediaType
    返 回 值： BOOL32
-----------------------------------------------------------------------------
    修改记录：
    日  期        版本        修改人        走读人    修改内容
    2005/03/20  3.6            万春雷                  创建
=============================================================================*/
BOOL32 CKdvSocket::SetSockMediaType( u8 bySockMediaType )
{
    m_bySockMediaType = bySockMediaType;

    return TRUE;
}

/*=============================================================================
    函 数 名： GetSockMediaType
    功    能： 获取媒体类型, 用于发送音频还是视频，类型不同TOS设置不一样
    算法实现：
    全局变量：
    参    数：
    返 回 值： u8
-----------------------------------------------------------------------------
    修改记录：
    日  期        版本        修改人        走读人    修改内容
    2005/03/20  3.6            万春雷                  创建
=============================================================================*/
u8 CKdvSocket::GetSockMediaType()
{
    return m_bySockMediaType;
}

u16 CKdvSocket::SendUserDefinedBuff(u8 *pBuf, s32 nSize, u32 dwRemoteIp, u16 wRemotePort)
{
    u16 wRet = MEDIANET_NO_ERROR;

    if(INVALID_SOCKET == m_hSocket)
    {
        return ERROR_SND_INVALID_SOCK;
    }

    memset( &m_tAddrIn, 0, sizeof(m_tAddrIn));
    m_tAddrIn.sin_family      = AF_INET;
    m_tAddrIn.sin_addr.s_addr = dwRemoteIp;
    m_tAddrIn.sin_port          = htons(wRemotePort);

    s32 nSndBytes = sendto( m_hSocket, (s8*)pBuf, nSize, 0,
                            (sockaddr *)&m_tAddrIn, sizeof(SOCKADDR_IN) );
    if( nSndBytes <= 0 )
    {
        wRet = ERROR_SND_SEND_UDP;
    }

    return wRet;
}

/*=============================================================================
    函数名        : SendTo
    功能        ：判断是否是广播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明： pBuf         Buffer to Send;
                   nSize        Buffer size to Send;
                   dwRemoteIp   remote ip(net order);
                   wRemotePort  remote port(machine order)

    返回值说明：成功返回MEDIANET_NO_ERROR，失败参见错误码定义;
=============================================================================*/
u16 CKdvSocket::SendTo(u8 *pBuf, s32 nSize, u32 dwRemoteIp, u16 wRemotePort)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //不是从网络接收时，
    if( !(m_dwFlag & MEDIANETRCV_FLAG_FROM_RECVFROM ) )
    {
        //这里发送主要是rtcp包的发送
        if( m_dwFlag & MEDIANETRCV_FLAG_FROM_DATASWITCH)
        {
/*            TDSNetAddr tSrcAddr,tDstAddr;
            memset(&tSrcAddr, 0, sizeof(tSrcAddr));
            memset(&tDstAddr, 0, sizeof(tDstAddr));
            tSrcAddr.dwIP = m_tMedianetNetAddr.m_dwIp;
            tSrcAddr.wPort = m_tMedianetNetAddr.m_wPort;
            tDstAddr.dwIP = dwRemoteIp;
            tDstAddr.wPort = wRemotePort;
            wRet = dsUDPPackSend( pBuf, nSize, &tSrcAddr, &tDstAddr );
*/
            wRet = RawSocketSendto(pBuf, nSize, m_tMedianetNetAddr.m_dwIp, m_tMedianetNetAddr.m_wPort, dwRemoteIp, wRemotePort);
        }
        return wRet;
    }

    //到了这点，则是从网络接收
    if(INVALID_SOCKET == m_hSocket)
    {
        return ERROR_SND_INVALID_SOCK;
    }

    // 用于发送的SOCKET 采用RawSocket，从而设置其 IP header-TOS Field
    if( TRUE == m_bUseRawSend )
    {
        s32 nIPVersion = 4;    //IPV4
        s32 nIPSize    = sizeof(m_tIPHdr) / sizeof(u32);
        s32 nUDPSize   = sizeof(m_tUDPHdr) + nSize;
        s32 nTotalSize = sizeof(m_tIPHdr) + sizeof(m_tUDPHdr) + nSize;
        BOOL32 bNeedCheckSum = TRUE;

        // 初始化IP头
        m_tIPHdr.byIPVerLen   = (nIPVersion << 4) | nIPSize;
        if( SOCKET_TYPE_AUDIO == m_bySockMediaType )
        {
            m_tIPHdr.byIPTos  = g_byAudTOS;           // IP type of service
        }
        else if( SOCKET_TYPE_VIDEO == m_bySockMediaType )
        {
            m_tIPHdr.byIPTos  = g_byVidTOS;           // IP type of service
        }
        else if( SOCKET_TYPE_DATA == m_bySockMediaType )
        {
            m_tIPHdr.byIPTos  = g_byDataTOS;          // IP type of service
        }
        m_tIPHdr.wIPTotalLen  = htons(nTotalSize);    // Total packet len
        m_tIPHdr.wIPID        = 0;                    // Unique identifier: set to 0
        m_tIPHdr.wIPOffset    = 0;                    // Fragment offset field
        m_tIPHdr.byIPTtl      = g_byTTL;              // Time to live
        m_tIPHdr.byIPProtocol = 0x11;                 // Protocol(UDP) TCP-6;UDP-17
        m_tIPHdr.wIPCheckSum  = 0 ;                   // IP checksum

        if (m_dwSrcIP != 0)
        {
            m_tIPHdr.dwSrcAddr    = m_dwSrcIP;          // Source address
        }
        else
        {
            m_tIPHdr.dwSrcAddr    = m_tCreateSock.m_dwLocalAddr;  // Source address
        }

        if (0 == m_tIPHdr.dwSrcAddr)
        {
            bNeedCheckSum = FALSE;
        }

        m_tIPHdr.dwDstAddr    = dwRemoteIp;           // Destination address

        // 初始化UDP头
        if (m_wSrcPort != 0)
        {
            m_tUDPHdr.wSrcPort    = htons(m_wSrcPort) ;
        }
        else
        {
            m_tUDPHdr.wSrcPort    = htons(m_tCreateSock.m_wSocketPort) ;
        }

        m_tUDPHdr.wDstPort    = htons(wRemotePort) ;
        m_tUDPHdr.wUDPLen     = htons(nUDPSize) ;
        m_tUDPHdr.wUDPCheckSum = 0 ;

        // 计算UDP校验和(称为网际校验和算法，把被校验的数据１６位进行累加，然后取反码，
        // 若数据字节长度为奇数，则数据尾部补一个字节的０以凑成偶数。
        // 此算法适用于IPv4、ICMPv4、IGMPV4、ICMPv6、UDP和TCP校验和，更详细的信息请参考RFC1071)
        //
        // UDP校验和的数据为 UDP伪包头(UDP pseudo-header)＋UDP包头＋数据(data)
        // 注意Data长度为奇数时尾部补一个0凑成偶数
        //

        // 为减少一次拷贝次数，这里借用了一部分空间 （sizeof(m_tIPHdr) > sizeof(tUDPPsdHdr)）
        // 校验和运算分为两步：先分别计算CRC（即累积和值），再取反码。
        //

        // hualiang 2006-08-31
        // 如果源地址为0，则源地址由下层填写，这时checksum将由下面计算（vxworks）
        // 对于linux，下层不会重新计算checksum，如果源地址按0计算，则checksum会出错，因此直接添0，表示不校验。


        s8 *pszRawBuf = (s8 *)&m_szRawPackBuf;
        if (bNeedCheckSum && g_byNeedChecksum)
        {
            TUDPPsdHdr   tUDPPsdHdr;
            memset(&tUDPPsdHdr, 0 , sizeof(tUDPPsdHdr));
            tUDPPsdHdr.dwSrcAddr    = m_tIPHdr.dwSrcAddr;
            tUDPPsdHdr.dwDstAddr    = m_tIPHdr.dwDstAddr;
            tUDPPsdHdr.byIPProtocol = m_tIPHdr.byIPProtocol;
            tUDPPsdHdr.wUDPLen      = m_tUDPHdr.wUDPLen;

            pszRawBuf = (s8 *)&m_szRawPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
            s32 nUdpChecksumSize = 0;

            memcpy(pszRawBuf, &tUDPPsdHdr, sizeof(tUDPPsdHdr));
            pszRawBuf += sizeof(tUDPPsdHdr);
            nUdpChecksumSize += sizeof(tUDPPsdHdr);

            memcpy(pszRawBuf, &m_tUDPHdr, sizeof(m_tUDPHdr));
            pszRawBuf += sizeof(m_tUDPHdr);
            nUdpChecksumSize += sizeof(m_tUDPHdr);

            memcpy(pszRawBuf, pBuf, nSize);
            pszRawBuf += nSize;
            nUdpChecksumSize += nSize;

            if( 0 != (nUdpChecksumSize%2) )
            {
                *pszRawBuf = 0;
                pszRawBuf += 1;
                nUdpChecksumSize += 1;
            }

            pszRawBuf = (s8 *)&m_szRawPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
            u16 wCkSum = CalcChecksum((u16 *)pszRawBuf, nUdpChecksumSize);

            m_tUDPHdr.wUDPCheckSum = wCkSum;
        }
        else
        {
            memcpy(pszRawBuf+sizeof(m_tIPHdr)+sizeof(m_tUDPHdr), pBuf, nSize);
        }

        //构造完整的 Raw Socket 数据包并进行发送

        pszRawBuf = (s8 *)&m_szRawPackBuf;
        memcpy(pszRawBuf, &m_tIPHdr, sizeof(m_tIPHdr));
        pszRawBuf += sizeof(m_tIPHdr);
        memcpy(pszRawBuf, &m_tUDPHdr, sizeof(m_tUDPHdr));

        if( 0 == g_byNeedChecksum )
        {
            pszRawBuf += sizeof(m_tUDPHdr);
            memcpy(pszRawBuf, pBuf, nSize);
        }
        else
        {
            // 这里可减少一次拷贝
            /*
            pszRawBuf += sizeof(m_tUDPHdr);
            memcpy(pszRawBuf, pBuf, nSize);
            */
        }

        memset( &m_tAddrIn, 0, sizeof(m_tAddrIn));
        m_tAddrIn.sin_family      = AF_INET;
        m_tAddrIn.sin_addr.s_addr = dwRemoteIp;
        m_tAddrIn.sin_port          = htons(wRemotePort);

        s32 nSndBytes = sendto( m_hSocket, (s8*)&m_szRawPackBuf, nTotalSize, 0,
                                (sockaddr *)&m_tAddrIn, sizeof(SOCKADDR_IN) );
        if( nSndBytes <= 0 )
        {
            wRet = ERROR_SND_SEND_UDP;
        }
    }
    else
    {
        memset( &m_tAddrIn, 0, sizeof(m_tAddrIn));
        m_tAddrIn.sin_family      = AF_INET;
        m_tAddrIn.sin_addr.s_addr = dwRemoteIp;
        m_tAddrIn.sin_port          = htons(wRemotePort);

        s32 nSndBytes = sendto( m_hSocket, (s8*)pBuf, nSize, 0,
                                (sockaddr *)&m_tAddrIn, sizeof(SOCKADDR_IN) );
        if( nSndBytes <= 0 )
        {
            wRet = ERROR_SND_SEND_UDP;
        }
    }

    return wRet;
}


/*=============================================================================
    函数名        : SetSrcAddr
    功能        ：设置源地址，用于IP伪装，只有在使用raw socket时才有效
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无

    返回值说明：无
=============================================================================*/
void CKdvSocket::SetSrcAddr(u32 dwSrcIP, u16 wSrcPort)
{
    m_dwSrcIP  = dwSrcIP;
    m_wSrcPort = wSrcPort;
}

void CKdvSocket::GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo)
{
    tSocketInfo.m_bUseRawSocket = m_bUseRawSend;
    tSocketInfo.m_dwSrcIP = m_dwSrcIP;
    tSocketInfo.m_wPort = m_wSrcPort;
}


/*=============================================================================
    函数名        : SetCallBack
    功能        ：设置回调函数
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无

    返回值说明：无
=============================================================================*/
void CKdvSocket::SetCallBack(PCALLBACK pCallBack,void* pContext)
{
   m_tCallBack.m_pCallBack = pCallBack;
   m_tCallBack.m_pContext = pContext;
}


BOOL32 CKdvSocket::SetLocalUserData(u32 dwUserDataLen, u8* pbyUserData)
{
    if (dwUserDataLen >= 0 && dwUserDataLen <= MAX_USERDATA_LEN && NULL != pbyUserData)
    {
        m_dwLocalUserDataLen = dwUserDataLen;
        memcpy(m_abyLocalUserData, pbyUserData, m_dwLocalUserDataLen);
        return TRUE;
    }

    return FALSE;
}



/*=============================================================================
    函数名        : CallBack
    功能        ：有数据来的时候，向上层回调
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pBuf           buffer
                    nBufSize       buffer size
    返回值说明：无
=============================================================================*/
void CKdvSocket::CallBack(u8 *pBuf, s32 nBufSize)
{
    if(m_tCallBack.m_pCallBack != NULL)
    {
        m_tCallBack.m_pCallBack(pBuf, nBufSize, m_tCallBack.m_pContext);
    }
    else
    {
        if(1 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvSocket::CallBack] m_tCallBack.m_pCallBack == NULL   \n");
        }
    }
}

/*=============================================================================
    函数名        : PrintErrMsg
    功能        ：打印错误信息
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：szErrMsg     错误信息字串指针
                  bShowSockArg 是否业打印当前套结子相应参数
    返回值说明：无
=============================================================================*/
void CKdvSocket::PrintErrMsg(s8 *szErrMsg, BOOL32 bShowSockArg /*= TRUE*/)
{
    OspPrintf(1, 0, "\n CKdvSocket::szErrMsg: %s \n", szErrMsg);
    if(TRUE == bShowSockArg)
    {
        OspPrintf(1, 0, "Print Argument List   \n");
        OspPrintf(1, 0, "LocalAddr     MultiAddr    Port    bRcv\n");
        OspPrintf(1, 0, "%x     %x     %d     %d \n",
                    m_tCreateSock.m_dwLocalAddr, m_tCreateSock.m_dwMultiAddr,
                    m_tCreateSock.m_wSocketPort, m_tCreateSock.m_bRcv);
    }
}


/////////////////////////////////////////////////////////////////////////////////
#define            MAX_MEDIANET_RCV_PACK_LEN            8192
void* MedianetRpctrlCallBack(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext)
{
    TMedianetNetAddr* ptDstAddr = (TMedianetNetAddr*)dwDstAddr;
    TMedianetNetAddr* ptSrcAddr = (TMedianetNetAddr*)dwSrcAddr;

    CKdvSocket* pMain = (CKdvSocket*)pContext;
    if( pMain == NULL || pPackBuf == NULL || wPackLen == 0 )
    {
        return NULL;
    }

    if(ptDstAddr != NULL && ptDstAddr->m_wPort != pMain->m_tCreateSock.m_wSocketPort )
    {
        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[medianet]dst port:[%d] != wsocketport:[%d]\n",
                ptDstAddr->m_wPort, pMain->m_tCreateSock.m_wSocketPort);
        }
        return NULL;
    }

    pMain->CallBack( pPackBuf, wPackLen );
    return NULL;
}

//从裸录
void* MedianetU2ACallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext)
{
    TMedianetNetAddr* ptDstAddr = (TMedianetNetAddr*)dwDstAddr;
    TMedianetNetAddr* ptSrcAddr = (TMedianetNetAddr*)dwSrcAddr;

    CKdvSocket* pMain = (CKdvSocket*)pContext;
    if( pMain == NULL || pPackBuf == NULL || wPackLen == 0 )
    {
        return NULL;
    }

    if(ptDstAddr != NULL && ptDstAddr->m_wPort != pMain->m_tCreateSock.m_wSocketPort )
    {
        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[medianet]dst port:[%d] != wsocketport:[%d]\n",
                ptDstAddr->m_wPort, pMain->m_tCreateSock.m_wSocketPort);
        }
        return NULL;
    }

    pMain->CallBack( pPackBuf, wPackLen );
    return NULL;
}

//从dataswitch
void* MedianetDataSwitchCallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext)
{
    TMedianetNetAddr* ptDstAddr = (TMedianetNetAddr*)dwDstAddr;
    TMedianetNetAddr* ptSrcAddr = (TMedianetNetAddr*)dwSrcAddr;

    CKdvSocket* pMain = (CKdvSocket*)pContext;
    if( pMain == NULL || pPackBuf == NULL || wPackLen == 0 || wPackLen > MAX_MEDIANET_RCV_PACK_LEN)
    {
        return NULL;
    }

    if(ptDstAddr != NULL && ptDstAddr->m_wPort != pMain->m_tCreateSock.m_wSocketPort )
    {
        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[medianet]dst port:[%d] != wsocketport:[%d]\n",
                ptDstAddr->m_wPort, pMain->m_tCreateSock.m_wSocketPort);
        }
        return NULL;
    }

    pMain->CallBack( pPackBuf, wPackLen );
    return NULL;
}

//测试用
void* MedianetTestCallback(u8* pPackBuf, u16 wPackLen, void* dwDstAddr, void* dwSrcAddr, u64 qwTimeStamp, void* pContext)
{
    CKdvSocket* pMain = (CKdvSocket*)pContext;
    if( pMain == NULL || pPackBuf == NULL || wPackLen == 0 || wPackLen > MAX_MEDIANET_RCV_PACK_LEN)
    {
        return NULL;
    }

    pMain->CallBack( pPackBuf, wPackLen );
    return NULL;
}


/*=============================================================================
    函数名        : SocketRcvTaskProc
    功能        ：接收线程入口函数
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pParam     user data
    返回值说明：无
=============================================================================*/
void* SocketRcvTaskProc(void * pParam)
{
    if(g_hWatchSocket == INVALID_SOCKET)
    {
        return 0;
    }
    if(g_hSem == NULL)
    {
        return 0;
    }

#ifdef _LINUX_
    OspRegTaskInfo(getpid(), "MediaRecvTask");
#endif

#ifdef _LINUX_
    if(5 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        OspPrintf(1 , 0 , "[medianet]run in socketrcvtaskproc, pid:%d\n", getpid());
    }
#endif

    BOOL32 bExit = FALSE;
    u32  dwBufLen = RTP_FIXEDHEADER_SIZE + MAX_RCV_PACK_SIZE;
    u8  *pBuf;
    MEDIANET_MALLOC(pBuf, dwBufLen+1);
    if( pBuf == NULL )
    {
        OspPrintf(TRUE, FALSE, "[medianet]SocketRcvTaskProc new pBuf #####1 fail\n");
        return 0;
    }
    s32     nRcvNum  = 0;

    FRAMEHDR    tFrmHdr;
    memset( &tFrmHdr,0, sizeof(tFrmHdr) );

    //g_hSem开始有信号，进入线程后置为无信号
    MEDIANET_SEM_TAKE(g_hSem);// be used to wait for  Exiting thread;

    fd_set  rcvSocketfd,rcvSocketfdTemp;
    FD_ZERO(&rcvSocketfdTemp);
    FD_SET( g_hWatchSocket, &rcvSocketfdTemp);
    rcvSocketfd = rcvSocketfdTemp;

      while (TRUE)
    {
        //线程安全退出
        if(g_bExitTask == TRUE)
        {
            OspPrintf(TRUE, FALSE, "[medianet]SocketRcvTaskProc g_bExitTask == TRUE\n");
            break;
        }

        if(5 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[SocketRcvTaskProc] select is running   \n");
        }

        if (SOCKET_ERROR == select(FD_SETSIZE, &rcvSocketfd, NULL, NULL, NULL))
        {
#ifdef _MSC_VER
            if (25 == g_nShowDebugInfo)
            {
                OspPrintf(1, 0, "[SocketRcvTaskProc] select error %d.\n", WSAGetLastError());
            }
#else
            if (25 == g_nShowDebugInfo)
            {
                OspPrintf(1, 0, "[SocketRcvTaskProc] select error %d.\n", errno);
            }
#endif
            OspTaskDelay(100);
			rcvSocketfd = rcvSocketfdTemp;
            continue;
        }

        if( FD_ISSET( g_hWatchSocket, &rcvSocketfd ) )
        {
            if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                //printf( "In SocketRcvTaskProc %d\n", OspTickGet() );
                OspPrintf( TRUE , FALSE , "In SocketRcvTaskProc %d\n", OspTickGet() );
            }
            while(TRUE)
            {
                TMsgData tMsgData;

                memset(&tMsgData, 0, sizeof(TMsgData));
                //解析控制包
                u32 dwRcvNum = recvfrom(g_hWatchSocket, (s8 *)&tMsgData, sizeof(TMsgData), 0, NULL, NULL);
                if (dwRcvNum <= 0)
                {
                    if(5 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                    {
                        OspPrintf(TRUE, FALSE, "[SocketRcvTaskProc] recvfrom g_hWatchSocket Exception: dwRcvNum = %d \n", dwRcvNum);
                    }
                    break;
                }
                u32 dwMode = tMsgData.m_dwMode;                 //控制命令
                CKdvSocket *pMain = (CKdvSocket *)tMsgData.m_lluContext;//对象
                u32 dwIndex =tMsgData.m_dwIndex;

                if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(TRUE, FALSE, "[createsocket] recv msg mode:[%d], pmain:[%x]=u32[%lld], index:[%d]\n",
                        tMsgData.m_dwMode, pMain, tMsgData.m_lluContext, tMsgData.m_dwIndex);
                }

                if(NULL == pMain && dwMode != SOCK_EXIT)
                {
                    break;
                }

                //解析控制命令
                switch(dwMode)
                {
                case SOCK_CREATE:   //创建socket
                    {
                        if(g_tRcvSockList.m_nSockCount >= g_nMaxRcvSockNum)
                        {
                            pMain->PrintErrMsg("g_tRcvSockList.m_nSockCount >= g_nMaxRcvSockNum");
                            g_nMaxRcvSockNum = g_nMaxRcvSockNum*2;
                            CKdvSocket** ppTempSocket = (CKdvSocket**)malloc(g_nMaxRcvSockNum*sizeof(CKdvSocket*));
                            if (NULL == ppTempSocket)
                            {
                                //完成线程同步
                                if(pMain->m_hCreateSynSem != NULL)
                                {
                                    pMain->m_bSuccess = FALSE;
                                    MEDIANET_SEM_GIVE(pMain->m_hCreateSynSem);
                                }
                                break;
                            }
                            memset(ppTempSocket, 0, g_nMaxRcvSockNum*sizeof(CKdvSocket*));
                            memcpy(ppTempSocket, g_tRcvSockList.m_pptSockUnit, g_tRcvSockList.m_nSockCount*sizeof(CKdvSocket*));
                            free(g_tRcvSockList.m_pptSockUnit);
                            g_tRcvSockList.m_pptSockUnit = ppTempSocket;
                        }
                        if(FALSE == pMain->Create())
                        {
                            //完成线程同步
                            if(pMain->m_hCreateSynSem != NULL)
                            {
                                pMain->m_bSuccess = FALSE;
                                MEDIANET_SEM_GIVE(pMain->m_hCreateSynSem);
                            }
                            break;
                        }
                        //加入socket list
                        BOOL32 bFind = FALSE;
                        for(s32 nPos=0; nPos<g_tRcvSockList.m_nSockCount; nPos++)
                        {
                            if(g_tRcvSockList.m_pptSockUnit[nPos] == pMain)
                            {
                                bFind=TRUE;
                                break;
                            }
                        }
                        if(!bFind)
                        {
                            g_tRcvSockList.m_nSockCount++;
                            g_tRcvSockList.m_pptSockUnit[g_tRcvSockList.m_nSockCount-1] = pMain;
                            if( g_tRcvSockList.m_pptSockUnit[g_tRcvSockList.m_nSockCount-1]->m_hSocket != INVALID_SOCKET )
                            {
                                FD_SET( g_tRcvSockList.m_pptSockUnit[g_tRcvSockList.m_nSockCount-1]->m_hSocket , &rcvSocketfdTemp );
                            }
                        }

                        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                        {
                            //printf( "Out SocketRcvTaskProc SOCK_CREATE %d %d\n" , g_tRcvSockList.m_nSockCount , OspTickGet() );
                            OspPrintf( TRUE , FALSE , "Out SocketRcvTaskProc SOCK_CREATE %d %d\n" , g_tRcvSockList.m_nSockCount , OspTickGet() );
                        }

                        //完成线程同步
                        if(pMain->m_hCreateSynSem != NULL)
                        {
                            pMain->m_bSuccess = TRUE;
                            MEDIANET_SEM_GIVE(pMain->m_hCreateSynSem);
                        }

                        break;
                    }
                case SOCK_DEL:
                    {
                        //从数组中删除对象指针
                        s32 nTemp = 0;
                        for(s32 i=0; i<g_tRcvSockList.m_nSockCount; i++)
                        {
                            if(g_tRcvSockList.m_pptSockUnit[i] == pMain)
                            {
                                if( g_tRcvSockList.m_pptSockUnit[i]->m_hSocket != INVALID_SOCKET )
                                {
                                    FD_CLR( g_tRcvSockList.m_pptSockUnit[i]->m_hSocket , &rcvSocketfdTemp );
                                }
                                continue;
                            }
                            g_tRcvSockList.m_pptSockUnit[nTemp] = g_tRcvSockList.m_pptSockUnit[i];
                            if (nTemp != i)
                            {
                                g_tRcvSockList.m_pptSockUnit[i] = NULL;
                            }
                            nTemp++;
                        }
                        g_tRcvSockList.m_nSockCount = nTemp;

                        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                        {
                            //printf( "Out SocketRcvTaskProc SOCK_DEL %d\n" , OspTickGet() );
                            OspPrintf( TRUE , FALSE , "Out SocketRcvTaskProc SOCK_DEL %d\n" , OspTickGet() );
                        }
                        //close socket
                        if( INVALID_SOCKET != pMain->m_hSocket )
                        {
                            SockClose(pMain->m_hSocket);
                            pMain->m_hSocket = INVALID_SOCKET;
                        }
                        if( 25 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                        {
                            //printf( "Out SocketRcvTaskProc SOCK_DEL %d\n" , OspTickGet() );
                            OspPrintf( TRUE , FALSE , "Out SocketRcvTaskProc SockClose %d\n" , OspTickGet() );
                        }
                        //完成线程同步
                        MEDIANET_SEM_GIVE(pMain->m_hSynSem);
                        break;
                    }
                case SOCK_EXIT:
                    {
                        bExit = TRUE;
                        break;
                    }
                default : break;
                }

                unsigned long dwLength;

#ifdef _LINUX_
                if (ioctl(g_hWatchSocket, FIONREAD, &dwLength) == 0 )
#else
                if (ioctlsocket(g_hWatchSocket, FIONREAD, &dwLength) == 0 )
#endif
                {
                    if (dwLength > 0)
                    {
                        OspPrintf(TRUE, FALSE, "ioctlsocket FIONREAD dwLength : %d\n", dwLength);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        if (TRUE == bExit)
        {
            continue;
        }
        //循环查找激活socket
        for(s32 j=0; j<g_tRcvSockList.m_nSockCount; j++)
        {
            if(g_tRcvSockList.m_pptSockUnit[j]->m_hSocket == INVALID_SOCKET)
            {
                continue;
            }
            if(FD_ISSET(g_tRcvSockList.m_pptSockUnit[j]->m_hSocket, &rcvSocketfd))
            {
                nRcvNum = recvfrom(g_tRcvSockList.m_pptSockUnit[j]->m_hSocket,
                    (s8 *)pBuf,
                    dwBufLen,
                    0, NULL, NULL);
                if( nRcvNum <= 0)
                {
                    if(5 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                    {
                        OspPrintf(1, 0, "[SocketRcvTaskProc] recvfrom Exception: nRcvNum = %d   \n", nRcvNum);
                    }
                    continue;
                }
                u32 dwTick1 = OspTickGet();
                g_tRcvSockList.m_pptSockUnit[j]->CallBack(pBuf, nRcvNum);
                u32 dwTick2 = OspTickGet();
                u32 dwTicks = (dwTick2 - dwTick1)*(1000/OspClkRateGet());
                if (dwTicks > 40)
                {
                    OspPrintf(TRUE, FALSE, "[SocketRcvTaskProc] CallBack take %u ticks.\n", dwTicks);
                }
            }

        }

        if(g_hWatchSocket == INVALID_SOCKET)
        {
            //报警，g_hWatchSocket == INVALID_SOCKET
            OspPrintf(1,0,"\n Warning: g_hWatchSocket == INVALID_SOCKET, Ctl Thread Exception..... \n");
        }

        rcvSocketfd = rcvSocketfdTemp;
    }

    MEDIANET_SAFE_FREE(pBuf)

    MEDIANET_SEM_GIVE(g_hSem);// be used to wait for  Exiting thread;

    OspPrintf(1, 0, "[SocketRcvTaskProc]  is running over.....   \n");

    return 0;
}

/*=============================================================================
    函数名        : RtcpSndTaskProc
    功能        ：rtcp定时rtcp包上报线程（任务）入口函数 这个任务级别较低
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pParam     user data
    返回值说明：无
=============================================================================*/
void* RtcpSndTaskProc(void * pParam)
{
    u32 dwRtcpTaskLastTs = 0;
//    u32 dwNatProbeTaskLastTs = 0 ;
    u32 dwTaskNowTs = 0;
    u32 dwSpanTs = 0;

    while(TRUE)
    {
        //线程安全退出
        if(g_bExitTask == TRUE)
        {
            break;
        }

        if(6 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[RtcpSndTaskProc] is running   \n");
        }
        //NatProbe Task, 考虑cpu情况，nat探测包，参考rtcp 只获取一次当前时间。
        dwTaskNowTs = OspTickGet();
        {
            if( NULL != g_hMediaRcvSem )
            {
                MEDIANET_SEM_TAKE(g_hMediaRcvSem);

                CKdvMediaRcv  *pcKdvMediaRcv = NULL;
                for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
                {
                    pcKdvMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
                    if(NULL != pcKdvMediaRcv)
                    {
                        pcKdvMediaRcv->DealNatProbePack(dwTaskNowTs);
                    }
                }

                MEDIANET_SEM_GIVE(g_hMediaRcvSem);
            }
        }
        //rtcp task
        dwTaskNowTs = OspTickGet();
        dwSpanTs = (dwTaskNowTs-dwRtcpTaskLastTs)*1000 / OspClkRateGet();
        if(g_bRtcpTaskStart && dwSpanTs >= g_dwTimer)
        {
        if(NULL != g_hMediaSndSem)
        {
            MEDIANET_SEM_TAKE(g_hMediaSndSem);

            CKdvMediaSnd  *pcKdvMediaSnd = NULL;

            for(s32 nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
            {
                pcKdvMediaSnd = g_tMediaSndList.m_tMediaSndUnit[nPos];
                if(NULL != pcKdvMediaSnd)
                {
                    pcKdvMediaSnd->DealRtcpTimer();
                }
            }

            if(NULL != g_hMediaSndSem)
            {
                MEDIANET_SEM_GIVE(g_hMediaSndSem);
            }
        }

        if( NULL != g_hMediaRcvSem )
        {
            MEDIANET_SEM_TAKE(g_hMediaRcvSem);

            CKdvMediaRcv  *pcKdvMediaRcv = NULL;

            for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
            {
                pcKdvMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
                if(NULL != pcKdvMediaRcv)
                {
                    pcKdvMediaRcv->DealRtcpTimer();
                }
            }

            if(NULL != g_hMediaRcvSem)
            {
                MEDIANET_SEM_GIVE(g_hMediaRcvSem);
            }
        }

            dwRtcpTaskLastTs = OspTickGet();
        }

        OspTaskDelay(1000); //定时间隔为1s
    }

    OspPrintf(1, 0, "RtcpSndTaskProc is over ...... \n");

    return 0;
}


API void printsock()
{
    OspPrintf(TRUE, FALSE, "FD_SETSIZE:%d\n", FD_SETSIZE);
    OspPrintf(1,0,"tRecvDataTask status is g_nRefCount=%d \n", g_nRefCount);
    OspPrintf(1,0,"Prinf kdvMediaNet Lib Valid Socket \n");
    OspPrintf(1,0,"Max Rcv Socket BufSize:%d Bytes  \n", MAX_NET_RCV_BUF_LEN);
    OspPrintf(1,0,"Max Snd Socket BufSize:%d Bytes  \n", MAX_NET_SND_BUF_LEN);
    OspPrintf(1,0,"Default Rcv&Snd Socket BufSize:%d Bytes  \n", MIN_NET_BUF_LEN);
    OspPrintf(1,0,"Valid Rcv Socket Total Num:%d   \n", g_tRcvSockList.m_nSockCount);
    OspPrintf(1,0,"Valid Rcv Socket List   \n");
    OspPrintf(1,0,"HANDLE   LocalIP     MultiIP    PROT    SndBufSize    RcvBufSize\n");
    for(s32 k=0; k<g_tRcvSockList.m_nSockCount; k++)
    {
        CKdvSocket* m_tSockUnit = g_tRcvSockList.m_pptSockUnit[k];

        OspPrintf(1,0,"%d     %x        %x        %d       %d       %d\n",
            m_tSockUnit->m_hSocket,
            m_tSockUnit->m_tCreateSock.m_dwLocalAddr,
            m_tSockUnit->m_tCreateSock.m_dwMultiAddr,
            m_tSockUnit->m_tCreateSock.m_wSocketPort,
            m_tSockUnit->m_nSndBufSize,
            m_tSockUnit->m_nRcvBufSize);
    }
    OspPrintf(1,0,"Valid Snd Socket Total Num:%d   \n", g_tSndSockList.m_nSockCount);
    OspPrintf(1,0,"Valid Snd Socket List   \n");
    OspPrintf(1,0,"HANDLE   LocalIP     MultiIP    PROT    SndBufSize    RcvBufSize\n");
    for(s32 l=0; l<g_tSndSockList.m_nSockCount; l++)
    {
        CKdvSocket* m_tSockUnit = g_tSndSockList.m_pptSockUnit[l];

        OspPrintf(1,0,"%d     %x        %x        %d       %d       %d\n",
            m_tSockUnit->m_hSocket,
            m_tSockUnit->m_tCreateSock.m_dwLocalAddr,
            m_tSockUnit->m_tCreateSock.m_dwMultiAddr,
            m_tSockUnit->m_tCreateSock.m_wSocketPort,
            m_tSockUnit->m_nSndBufSize,
            m_tSockUnit->m_nRcvBufSize);
    }
}

static TASKHANDLE g_hNewRcvTask = NULL;    //模拟数据的socket接收线程handle
static u16 g_wCurRcvPort = 0;           //模拟数据的 接收端口（主机序）
static u32 g_dwCurRcvPackNum = 0;       //模拟数据的 接收到的小包数

static TASKHANDLE g_hNewSndTask = NULL;    //模拟数据的socket发送线程handle
static u16 g_nRawSndSock   = 0;         //使用RawSocket模拟数据的发送
static u16 g_wLocalSndPort = MAXWATCHSOCKPORT;      //模拟数据的 本端发送端口（主机序）
static u32 g_dwLocalSndIP = inet_addr("127.0.0.1"); //模拟数据的 本端发送地址（网络序）
static u16 g_wCurSndPort = 0;                       //模拟数据的 发送目标端口（主机序）
static u32 g_dwCurSndIP  = inet_addr("127.0.0.1");  //模拟数据的 发送目标地址（网络序）
static u32 g_dwCurSndPackNum = 0;                   //模拟数据的 已经发送的小包数
static s32 g_nLeftPackNum = 0;                      //模拟数据的 待发送的小包数
static u32 g_dwPackLen = 4;                         //模拟数据的 待发送的UDP包长度

SOCKHANDLE CreateSocket( BOOL32 bRaw, BOOL32 bSend, u16 wLocalPort )
{
    SOCKHANDLE hSocket = INVALID_SOCKET;

    // 用于发送的SOCKET 采用RawSocket，从而设置其 IP header-TOS Field
    if(TRUE == bSend && TRUE == bRaw)
    {
        hSocket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if( INVALID_SOCKET == hSocket )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] Socket socket() Error \n" );
            return hSocket;
        }
        BOOL32 bOpt = TRUE;
        if (SOCKET_ERROR == setsockopt(hSocket, IPPROTO_IP, IP_HDRINCL, (s8 *)&bOpt, sizeof(bOpt)) )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] setsockopt(IP_HDRINCL) Error \n");
            return hSocket;
        }

        //如果用户没有对 LocalIP 进行设置，则列举当前的一个有效ip
        if( ( 0 == g_dwLocalSndIP ) ||
            ( inet_addr("127.0.0.1") == g_dwLocalSndIP ) )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] Set New LocalIP Start LocalSndIP=%d\n", g_dwLocalSndIP);

            u32  tdwIPList[MAX_LOCAL_IP_NUM];  //列举到的当前设置的所有有效ip
            u16  dwIPNum;                      //列举到ip 数目

        #ifdef _LINUX_
            // 需要osplinux版本提供OspAddrListGet函数
            dwIPNum = 1;
        #else
            dwIPNum = OspAddrListGet(tdwIPList, MAX_LOCAL_IP_NUM);
            if( 0 == dwIPNum )
            {
                OspPrintf( TRUE, FALSE, "[CreateSocket] OspAddrListGet Error \n" );
            }
        #endif
            for(u16 dwAdapter=0; dwAdapter<dwIPNum; dwAdapter++)
            {
                if((inet_addr( "127.0.0.1" ) != tdwIPList[dwAdapter]))
                {
                    g_dwLocalSndIP = tdwIPList[dwAdapter];
                    break;
                }
            }

            OspPrintf( TRUE, FALSE, "[CreateSocket] Set New LocalIP End -- LocalSndIP=%d\n", g_dwLocalSndIP);
        }
    }
    else
    {
        //生成socket对象
        hSocket = socket( AF_INET, SOCK_DGRAM, 0 );
        if( INVALID_SOCKET == hSocket )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] Socket socket() Error \n");
            SockClose(hSocket);
            hSocket = INVALID_SOCKET;
            return hSocket;
        }

        // set reuse option
        s32 nReuseAddr = 1;
        if( SOCKET_ERROR  == setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR,
                                        (s8 *)&nReuseAddr, sizeof(nReuseAddr)) )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] Socket setsockopt SO_REUSEADDR Error \n");
            SockClose(hSocket);
            hSocket = INVALID_SOCKET;
            return hSocket;
        }

        //绑定socket
        SOCKADDR_IN addr;
        memset(&addr, 0, sizeof(SOCKADDR_IN));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = ADDR_ANY;
        addr.sin_port        = htons(wLocalPort);
        if( SOCKET_ERROR == bind(hSocket, (sockaddr *)&addr,sizeof(SOCKADDR_IN)) )
        {
            OspPrintf( TRUE, FALSE, "[CreateSocket] Socket bind Error \n");
            SockClose(hSocket);
            hSocket = INVALID_SOCKET;
            return hSocket;
        }
    }

    s32 nBufSize = MAX_NET_SND_BUF_LEN;
    s32 nTotalBufSize = MAX_NET_SND_BUF_LEN;
    s32 nRealBufSize = 0;
    s32 nSize = sizeof(nRealBufSize);

    if(FALSE == bSend)
    {
        nBufSize = MIN_NET_BUF_LEN;
        nTotalBufSize = MIN_NET_BUF_LEN;
    }

    //Set Send buffer length
    while(nBufSize >= MIN_NET_BUF_LEN)
    {
        if( SOCKET_ERROR == setsockopt( hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8 *)&nTotalBufSize, sizeof(s32)) )
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }

#ifdef _LINUX_
        if( SOCKET_ERROR == getsockopt( hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8*)&nRealBufSize, (socklen_t*)&nSize) )
#else
        if( SOCKET_ERROR == getsockopt( hSocket, SOL_SOCKET, SO_SNDBUF,
                                        (s8*)&nRealBufSize, &nSize) )
#endif
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else if(nRealBufSize < nBufSize)
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else
        {
            break;
        }
    }

    if(nBufSize < MIN_NET_BUF_LEN)
    {
        OspPrintf( TRUE, FALSE, "Socket setsockopt SO_SNDBUF Error \n");
        SockClose(hSocket);
        hSocket = INVALID_SOCKET;
        return hSocket;
    }

    nRealBufSize = 0;
    if(TRUE == bSend)
    {
        nBufSize = MIN_NET_BUF_LEN;
        nTotalBufSize = MIN_NET_BUF_LEN;
    }
    else
    {
        nBufSize = MAX_NET_RCV_BUF_LEN;
        nTotalBufSize = MAX_NET_RCV_BUF_LEN;
    }

    //Set recieve buffer length
    while(nBufSize >= MIN_NET_BUF_LEN)
    {
        if( SOCKET_ERROR == setsockopt( hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8 *)&nTotalBufSize, sizeof(s32)) )
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }

#ifdef _LINUX_
        if( SOCKET_ERROR == getsockopt( hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8*)&nRealBufSize, (socklen_t*)&nSize) )
#else
        if( SOCKET_ERROR == getsockopt( hSocket, SOL_SOCKET, SO_RCVBUF,
                                        (s8*)&nRealBufSize, &nSize) )
#endif
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else if(nRealBufSize < nBufSize)
        {
            nBufSize = (nBufSize / 2);
            nTotalBufSize = nBufSize;
            continue;
        }
        else
        {
            break;
        }
    }

    if(nBufSize < MIN_NET_BUF_LEN)
    {
        OspPrintf( TRUE, FALSE, "Socket setsockopt SO_RCVBUF Error \n");
        SockClose(hSocket);
        hSocket = INVALID_SOCKET;
        return hSocket;
    }

    //当套介子发送的数据包未被接收时，禁止 回馈的ICMP包 的通知
    // disable  new behavior using IOCTL: SIO_UDP_CONNRESET

    return hSocket;
}

s32 SendData(SOCKHANDLE hSocket, BOOL32 bRaw, s8 *pszRawPackBuf, u8 *pBuf, s32 nSize, u32 dwRemoteIp, u16 wRemotePort)
{
    s32 nRet = 0;

    if(INVALID_SOCKET == hSocket)
    {
        return nRet;
    }

    // 用于发送的SOCKET 采用RawSocket，从而设置其 IP header-TOS Field
    if( TRUE == bRaw )
    {
        TIPHdr  tIPHdr;
        TUDPHdr tUDPHdr;

        s32 nIPVersion = 4;    //IPV4
        s32 nIPSize    = sizeof(tIPHdr) / sizeof(u32);
        s32 nUDPSize   = sizeof(tUDPHdr) + nSize;
        s32 nTotalSize = sizeof(tIPHdr) + sizeof(tUDPHdr) + nSize;

        // 初始化IP头
        tIPHdr.byIPVerLen   = (nIPVersion << 4) | nIPSize;
        tIPHdr.byIPTos      = g_byAudTOS;           // IP type of service //默认按照音频
        tIPHdr.wIPTotalLen  = htons(nTotalSize);    // Total packet len
        tIPHdr.wIPID        = 0;                    // Unique identifier: set to 0
        tIPHdr.wIPOffset    = 0;                    // Fragment offset field
        tIPHdr.byIPTtl      = g_byTTL;              // Time to live
        tIPHdr.byIPProtocol = 0x11;                 // Protocol(UDP) TCP-6;UDP-17
        tIPHdr.wIPCheckSum  = 0 ;                   // IP checksum
        tIPHdr.dwSrcAddr    = g_dwLocalSndIP;       // Source address
        tIPHdr.dwDstAddr    = dwRemoteIp;           // Destination address

        // 初始化UDP头
        tUDPHdr.wSrcPort    = htons(g_wLocalSndPort) ;
        tUDPHdr.wDstPort    = htons(wRemotePort) ;
        tUDPHdr.wUDPLen     = htons(nUDPSize) ;
        tUDPHdr.wUDPCheckSum = 0 ;

        // 计算UDP校验和
        s8 *pszRawBuf = pszRawPackBuf;
        if( 0 != g_byNeedChecksum )
        {
            TUDPPsdHdr   tUDPPsdHdr;
            memset(&tUDPPsdHdr, 0 , sizeof(tUDPPsdHdr));
            tUDPPsdHdr.dwSrcAddr    = tIPHdr.dwSrcAddr;
            tUDPPsdHdr.dwDstAddr    = tIPHdr.dwDstAddr;
            tUDPPsdHdr.byIPProtocol = tIPHdr.byIPProtocol;
            tUDPPsdHdr.wUDPLen      = tUDPHdr.wUDPLen;

            pszRawBuf = pszRawPackBuf + sizeof(tIPHdr) - sizeof(tUDPPsdHdr);
            s32 nUdpChecksumSize = 0;

            memcpy(pszRawBuf, &tUDPPsdHdr, sizeof(tUDPPsdHdr));
            pszRawBuf += sizeof(tUDPPsdHdr);
            nUdpChecksumSize += sizeof(tUDPPsdHdr);

            memcpy(pszRawBuf, &tUDPHdr, sizeof(tUDPHdr));
            pszRawBuf += sizeof(tUDPHdr);
            nUdpChecksumSize += sizeof(tUDPHdr);

            memcpy(pszRawBuf, pBuf, nSize);
            pszRawBuf += nSize;
            nUdpChecksumSize += nSize;

            if( 0 != (nUdpChecksumSize%2) )
            {
                *pszRawBuf = 0;
                pszRawBuf += 1;
                nUdpChecksumSize += 1;
            }

            pszRawBuf = pszRawPackBuf + sizeof(tIPHdr) - sizeof(tUDPPsdHdr);
            u16 wCkSum = CalcChecksum((u16 *)pszRawBuf, nUdpChecksumSize);

            tUDPHdr.wUDPCheckSum = wCkSum;
        }

        //构造完整的 Raw Socket 数据包并进行发送

        pszRawBuf = pszRawPackBuf;
        memcpy(pszRawBuf, &tIPHdr, sizeof(tIPHdr));
        pszRawBuf += sizeof(tIPHdr);
        memcpy(pszRawBuf, &tUDPHdr, sizeof(tUDPHdr));

        if( 0 == g_byNeedChecksum )
        {
            pszRawBuf += sizeof(tUDPHdr);
            memcpy(pszRawBuf, pBuf, nSize);
        }
        else
        {
            // 这里可减少一次拷贝
            /*
            pszRawBuf += sizeof(tUDPHdr);
            memcpy(pszRawBuf, pBuf, nSize);
            */
        }

        SOCKADDR_IN tAddrIn;
        memset( &tAddrIn, 0, sizeof(tAddrIn));
        tAddrIn.sin_family        = AF_INET;
        tAddrIn.sin_addr.s_addr = dwRemoteIp;
        tAddrIn.sin_port        = htons(wRemotePort);

        nRet = sendto( hSocket, pszRawPackBuf, nTotalSize, 0,
                       (sockaddr *)&tAddrIn, sizeof(SOCKADDR_IN) );
    }
    else
    {
        SOCKADDR_IN tAddrIn;
        memset( &tAddrIn, 0, sizeof(tAddrIn));
        tAddrIn.sin_family        = AF_INET;
        tAddrIn.sin_addr.s_addr = dwRemoteIp;
        tAddrIn.sin_port        = htons(wRemotePort);

        nRet = sendto( hSocket, (s8*)pBuf, nSize, 0,
                       (sockaddr *)&tAddrIn, sizeof(SOCKADDR_IN) );
    }

    return nRet;
}

void* NewRcvTaskProc(void * pParam)
{
    //a. 创建指定端口的接收套节子
    SOCKHANDLE hSocket = CreateSocket( FALSE, FALSE, g_wCurRcvPort );
    if( INVALID_SOCKET == hSocket )
    {
        OspPrintf(1, 0, "[NewRcvTaskProc] CreateSocket Error  \n");
        return 0;
    }

    //b. 进入线程循环 select 接收数据
    u32  dwBufLen = RTP_FIXEDHEADER_SIZE + MAX_RCV_PACK_SIZE;
    u8  *pBuf;
    MEDIANET_MALLOC(pBuf, dwBufLen+1);
    if( pBuf == NULL )
    {
        OspPrintf(TRUE, FALSE, "[medianet]NewRcvTaskProc new pbuf fail ####2 \n");
        return 0;
    }
    s32     nRcvNum  = 0;

    fd_set  rcvSocketfd;
    FD_ZERO(&rcvSocketfd);
    FD_SET( hSocket, &rcvSocketfd);

      while( SOCKET_ERROR != select(FD_SETSIZE, &rcvSocketfd, NULL, NULL, NULL ) )
    {
        if( FD_ISSET( hSocket, &rcvSocketfd ) )
        {
            nRcvNum = recvfrom( hSocket, (s8 *)pBuf, dwBufLen,
                                0, NULL, NULL);
            if( nRcvNum <= 0)
            {
                if(10 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(1, 0, "[NewRcvTaskProc] recvfrom Exception: nRcvNum = %d   \n", nRcvNum);
                }

                FD_ZERO(&rcvSocketfd);
                FD_SET( hSocket, &rcvSocketfd);

                continue;
            }

            if(10 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf(1, 0, "[NewRcvTaskProc] recvfrom data packet: nRcvNum = %d   \n", nRcvNum);
            }

            g_dwCurRcvPackNum++;
        }

        FD_ZERO(&rcvSocketfd);
        FD_SET( hSocket, &rcvSocketfd);
    }

    MEDIANET_SAFE_FREE(pBuf)
    SockClose(hSocket);

    OspPrintf(1, 0, "[NewRcvTaskProc]  is running over.....   \n");

    return 0;
}

void* NewSndTaskProc(void * pParam)
{
    //a. 创建指定端口的发送套节子
    SOCKHANDLE hUDPSocket = CreateSocket( FALSE, TRUE, g_wLocalSndPort );
    if( INVALID_SOCKET == hUDPSocket )
    {
        OspPrintf(1, 0, "[NewSndTaskProc] CreateSocket udp sock Error  \n");
        return 0;
    }

    SOCKHANDLE hRawSocket = CreateSocket( TRUE, TRUE, (g_wLocalSndPort+1) );
    if( INVALID_SOCKET == hRawSocket )
    {
        OspPrintf(1, 0, "[NewSndTaskProc] CreateSocket raw sock Error  \n");
        SockClose(hUDPSocket);
        return 0;
    }

    //b. 进入线程循环 select 接收数据
    u32  dwBufLen   = RTP_FIXEDHEADER_SIZE + MAX_RCV_PACK_SIZE;
    s8  *pRawBuf    = (s8 *)OspAllocMem(dwBufLen+sizeof(TIPHdr)+sizeof(TUDPHdr)+4);
    if( pRawBuf == NULL )
    {
        OspPrintf(TRUE, FALSE, "[medianet]NewSndTaskProc new prawbuf fail #####3\n");
        return 0;
    }
    u8  *pDataBuf   = (u8 *)OspAllocMem(dwBufLen+4);
    if( pDataBuf == NULL )
    {
        OspPrintf(TRUE, FALSE, "[medianet]NewSndTaskProc new pdatabuf fail ####4 \n");
        return 0;
    }
    u32  dwDataLen  = g_dwPackLen;
    *(u32*)pDataBuf = g_dwCurSndPackNum;

      while( TRUE )
    {
        s32 nRet = 0;
        if( g_nLeftPackNum > 0 )
        {
            g_nLeftPackNum--;
            if( 0 == g_nRawSndSock )
            {
                nRet = SendData(hUDPSocket, FALSE, pRawBuf, pDataBuf, dwDataLen, g_dwCurSndIP, g_wCurSndPort);
            }
            else
            {
                nRet = SendData(hRawSocket, TRUE, pRawBuf, pDataBuf, dwDataLen, g_dwCurSndIP, g_wCurSndPort);
            }
            if( nRet <= 0 )
            {
                OspPrintf(1, 0, "[NewSndTaskProc] SendData Err-- Raw=%d, dwDataLen=%d, DstIP=%d, DstPort=%d \n",
                                 g_nRawSndSock, dwDataLen, g_dwCurSndIP, g_wCurSndPort );
            }
            else
            {
                g_dwCurSndPackNum++;
            }
            OspTaskDelay(1);
        }
        else
        {
            OspTaskDelay(1000);
        }
    }

    OspFreeMem(pRawBuf);
    pRawBuf = NULL;
    OspFreeMem(pDataBuf);
    pDataBuf = NULL;

    SockClose(hUDPSocket);
    SockClose(hRawSocket);

    OspPrintf(1, 0, "[NewSndTaskProc]  is running over.....   \n");

    return 0;
}

/*=============================================================================
    函数名        ：IsMultiCastAddr
    功能        ：判断是否是组播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 IsMultiCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    u8 byLOByte = *pch;

    if( (byLOByte >= 224) && (byLOByte <= 239))
        return TRUE;
    else
        return FALSE;
}

/*=============================================================================
    函数名        IsBroadCastAddr
    功能        ：判断是否是广播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 IsBroadCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    pch += 3;
    u8 byHIByte = *pch;

    if( 255 == byHIByte )
        return TRUE;
    else
        return FALSE;
}


//模拟数据的接收线程
API void startrcvtask(int nCurRcvPort)
{
    if(g_hNewRcvTask == NULL)
    {
        g_wCurRcvPort = nCurRcvPort;

        g_hNewRcvTask = OspTaskCreate( NewRcvTaskProc, "tNewRcvTask", 60, 512*1024, 0, 0 );
        if(g_hNewRcvTask == NULL)
        {
            g_wCurRcvPort = 0;
            OspPrintf(1,0," [startrcvtask] OspTaskCreate tNewRcvTask Error...  \n");
        }
    }
    else
    {
        OspPrintf(1,0," [startrcvtask]  tNewRcvTask is running...  \n");
    }

    return;
}

//模拟数据的接收线程
API void startsndtask(int nPackNum, int nPackLen, int nRaw, int nCurSndPort, int nAddrIP=0)
{
    g_nRawSndSock  = (s32)nRaw;
    g_nLeftPackNum = (s32)nPackNum;
    g_wCurSndPort  = (u16)nCurSndPort;
    if( 0 == nAddrIP )
    {
        g_dwCurSndIP  = inet_addr("127.0.0.1");
    }
    else
    {
        g_dwCurSndIP   = (u32)nAddrIP;
    }
    if( nPackLen <= 0 )
    {
        g_dwPackLen  = 4;
    }
    else if( nPackLen > MAX_RCV_PACK_SIZE )
    {
        g_dwPackLen  = (u32)MAX_RCV_PACK_SIZE;
    }
    else
    {
        g_dwPackLen  = (u32)nPackLen;
    }

    if(NULL == g_hNewSndTask)
    {
        g_hNewSndTask = OspTaskCreate( NewSndTaskProc, "tNewSndTask", 60, 512*1024, 0, 0 );
        if(NULL == g_hNewSndTask)
        {
            g_wCurSndPort = 0;
            OspPrintf(1,0," [startsndtask] OspTaskCreate tNewSndTask Error...  \n");
        }
    }
    else
    {
        OspPrintf(1,0," [startsndtask]  tNewSndTask is running...  \n");
    }

    return;
}


//显示模拟数据的接收结果
API void showrcvresult()
{
    OspPrintf( 1,0,"[showrcvresult] Current Port:%d, Recived Packet Num:%d   \n",
                    g_wCurRcvPort, g_dwCurRcvPackNum );
}

//显示模拟数据的收发结果
API void showsimresult()
{
    OspPrintf( 1,0,"[showsimresult] Current RcvPort:%d, Current SndPort:%d, Received PackNum:%d, Sended PackNum:%d   \n",
                    g_wCurRcvPort, g_wCurSndPort, g_dwCurRcvPackNum, g_dwCurSndPackNum );
}

API void mediasocketinfo()
{
    OspPrintf(TRUE, FALSE, "FD_SETSIZE:%d\n", FD_SETSIZE);
    OspPrintf(TRUE, FALSE, "Cur Rcv object count:%d Total:%d\n",
        g_tMediaRcvList.m_nMediaRcvCount, sizeof(g_tMediaRcvList.m_tMediaRcvUnit)/sizeof(g_tMediaRcvList.m_tMediaRcvUnit[0]));
    OspPrintf(TRUE, FALSE, "Cur Snd object count:%d Total:%d\n",
        g_tMediaSndList.m_nMediaSndCount, sizeof(g_tMediaSndList.m_tMediaSndUnit)/sizeof(g_tMediaSndList.m_tMediaSndUnit[0]));
    OspPrintf(TRUE, FALSE, "Cur Rcv socket count:%d Total:%d\n",
        g_tRcvSockList.m_nSockCount, sizeof(g_tRcvSockList.m_pptSockUnit)/sizeof(g_tRcvSockList.m_pptSockUnit[0]));
    OspPrintf(TRUE, FALSE, "Cur Snd socket count:%d Total:%d\n",
        g_tSndSockList.m_nSockCount, sizeof(g_tSndSockList.m_pptSockUnit)/sizeof(g_tSndSockList.m_pptSockUnit[0]));
}

#endif