#include "sockswitch.h"

#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <sys/ioctl.h>
#include <sys/prctl.h>
#endif


#define OspPrintf(n, m, fmt, ...)     DEBUG::dbg_printf("[DS]"fmt, ##__VA_ARGS__)

/*
#ifdef USE_EPOLL
#warning "use epoll"
#else
#warning "use select"
#endif
*/
#define DSINTERPROCID 0x3c5af8f7
#ifdef WIN32 //替换掉winsock2.h中的FDSET的宏定义，去除循环查找FD是否已经在FDSET中的处理
typedef int socklen_t;

#define FD_SET2(fd, set) do { \
    u_int __i= ((fd_set FAR *)(set))->fd_count; \
    if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
    ((fd_set FAR *)(set))->fd_array[__i] = (fd); \
    ((fd_set FAR *)(set))->fd_count++; \
    } \
} while(0)
#endif

#ifdef _LINUX_
#include <sys/un.h>
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif
#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof (*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif
#endif


#ifdef USE_EPOLL
	//接收任务epoll文件描述符
	static int g_nDSTaskEpollFd = -1;
#endif




//默认最大接收结点数
#define DEFAULT_MAXNUM_RCVGRP		(uint32_t)1024 //(运营为4096)
//同一接收结点下最大转发目标数(不可修改，可配合增大接收结点数而减小)
#define DEFAULT_MAXNUM_TARGET_PER_RCVGRP	(uint32_t)256
//最大交换总数限制
#define MAXNUM_SWITCH               (uint32_t)1024*1024

//当前最大接收结点数(可调用dsSetCapacity修改，但转发目标总容量限制在MAXNUM_SWITCH)
uint32_t g_dwMaxRcvGrp = DEFAULT_MAXNUM_RCVGRP;
//当前同一接收结点下最大转发目标数(可调用dsSetCapacity修改，但转发目标总容量限制在MAXNUM_SWITCH)
uint32_t g_dwMaxSndMmbPerRcvGrp = DEFAULT_MAXNUM_TARGET_PER_RCVGRP;
//发送的hash大小
uint32_t     g_dwSndHashSize = DEFAULT_MAXNUM_RCVGRP*4;  //【TODO】20130124 huangzhichun：这个hash大小应该是跟g_dwMaxRcvGrp保持一致。

//接收总字节数
int64_t g_swRcvBytesCount = 0;
//发送总字节数
int64_t g_swSndBytesCount = 0;

uint32_t g_dwCmdSn = 0;


extern int32_t g_socksndbuff;
extern int32_t g_sockrcvbuff;


#define API_TIME_OUT		10000

static	int32_t g_nDsApiTimeOut = API_TIME_OUT ;

TRcvGrp*	g_ptRcvGrpPool = NULL;
TSndMmb*	g_ptSndMmbPool = NULL;
TSndMmb*	g_ptFreeSndMmb = NULL;

TSndMmb**   g_aptSndHash = NULL;   //发送hash，用于按源交换时查找发送目标
TRcvGrp**   g_aptRcvHash = NULL;   //接收Hash，用于windows select中，根据socket查找recv对象

TNetSndMember*	g_ptNetSndMembers = NULL;

static CSockSwitch  g_cSockSwitch;
static bool         g_bDataSwitchInit = false;
char g_szSendIf[16];
static SEMHANDLE    g_smDSToken;
static uint32_t g_localipnum = inet_addr("127.0.0.1");

static SOCKHANDLE g_rawSocket  = INVALID_SOCKET;

static void RemovefromSndHash(TSndMmb* ptSnd, uint32_t dwLocalIP, uint16_t wLocalPort);
static void AddtoSndHash(TSndMmb* ptSnd, uint32_t dwLocalIP, uint16_t wLocalPort);
static TSndMmb* GetHashList(uint32_t dwLocalIP, uint16_t wLocalPort, uint32_t dwSrcIP, uint16_t wSrcPort);
DS_API void sndcheckhash();
DS_API void dstrace(DSID, char*, uint16_t, uint32_t);

static void RemovefromRcvHash(TRcvGrp* ptRcvGrp);
static void AddtoRcvHash(TRcvGrp* ptRcvGrp);
static TRcvGrp* GetRcvNode(uint32_t hSock);


uint32_t g_dwDsStep = 0;
uint32_t g_dwDsIp = 0;;
uint32_t g_dwDsPort = 0;
bool g_bDsJustRecv;


typedef struct
{
	uint32_t m_RtpSeq;
	uint32_t m_RtpSrc;
} TRtpInfo;

static uint32_t GetBitField(uint32_t dwValue, int32_t nStartBit, int32_t nBits)
{
    int32_t  nMask = (1 << nBits) - 1;
    return (dwValue >> nStartBit) & nMask;
}

static void GetRtpInfo(void* pBuf, int32_t nSize, TRtpInfo* pInfo)
{
	//copy head to local buffer
	uint32_t Tmp = ntohl(((uint32_t*)pBuf)[0]);

	//get seq
	pInfo->m_RtpSeq = GetBitField(Tmp, 0, 16);

	Tmp = ntohl(((uint32_t*)pBuf)[2]);

	//get ssrc
	pInfo->m_RtpSrc = Tmp;
}


#ifdef _DS_CHECK_LOST_

#include <map>
using namespace std;

typedef map<uint64_t, TRtpInfo> TIpPortRtpMap;

static bool CheckPackLost(uint32_t dwIp, uint16_t wPort, TRtpInfo tRtpInfo)
{
	static TIpPortRtpMap s_mapIpPortSn;

	uint64_t wwIpPort = ((uint64_t)wPort << 32) | dwIp;
	TIpPortRtpMap::iterator iter = s_mapIpPortSn.find(wwIpPort);
	if (iter == s_mapIpPortSn.end())
	{
		s_mapIpPortSn[wwIpPort] = tRtpInfo;
		return false; // not lost
	}

	TRtpInfo tLastRtpInfo = (*iter).second;
	(*iter).second = tRtpInfo;
	if (tLastRtpInfo.m_RtpSrc == tRtpInfo.m_RtpSrc)
	{
		return (tLastRtpInfo.m_RtpSeq + 1 != tRtpInfo.m_RtpSeq);
	}
	else
	{
		return false; // not lost
	}
}

#endif // _DS_CHECK_LOST_

#ifdef USE_EPOLL
bool g_bCmdOpt = true;

DS_API void dscmdopt(bool bOpt)
{
    g_bCmdOpt = bOpt;
}
#endif

static uint32_t recv_tcp_data(SOCKHANDLE s, uint8_t * buf, uint32_t size, uint32_t timeout)
{
	uint32_t len = 0;
	int32_t ret = 0;
	uint32_t count = 0;

#ifdef USE_EPOLL
    bool bCmdOpt = g_bCmdOpt;
    int nInterEpollFd = -1;
    epoll_event tInterEpollEvent;

    if (bCmdOpt)
    {
        nInterEpollFd = epoll_create(1);
        if (-1 == nInterEpollFd)
        {
            dslog(DS_LOG_ERROR, "epoll_create failed errno(%d)(%s)\n", errno, strerror(errno));
            return 0;
        }

        memset(&tInterEpollEvent, 0, sizeof(epoll_event));
		uint64_t dwFdIdx = DSINTERPROCID;
		tInterEpollEvent.data.u64 = (dwFdIdx << 32) + s;
	    tInterEpollEvent.events = EPOLLIN | EPOLLPRI;

	    if (0 != epoll_ctl(nInterEpollFd, EPOLL_CTL_ADD, s, &tInterEpollEvent))
	    {
		    dslog(DS_LOG_ERROR, "epoll_ctl add failed! errno(%d)(%s)\n", errno, strerror(errno));
		    return 0;
	    }
    }
#endif

    while ( count++ < timeout / 10 && len != size)
    {
#ifdef USE_EPOLL
        if (bCmdOpt)
        {
	        memset(&tInterEpollEvent, 0, sizeof(epoll_event));
		    int32_t msEventCount = epoll_wait(nInterEpollFd, &tInterEpollEvent, 1, 10);
		    if (msEventCount < 0)
		    {
			    dslog(DS_LOG_ERROR, "recv_tcp_data epoll_wait failed, err(%d)\n", getSockErrno());
                CBB::cbb_task_delay(10);
			    continue;
		    }

            if (!(tInterEpollEvent.events & EPOLLIN) || (DSINTERPROCID != (uint32_t)(tInterEpollEvent.data.u64 >> 32)))
            {
                continue;
            }
        }
#else
		fd_set fdSet;
		FD_ZERO( &fdSet );

		FD_SET2(s, &fdSet);

		struct timeval nTimeVal;
		nTimeVal.tv_sec = timeout;
		nTimeVal.tv_usec = 0;
		int32_t ret_num = select(FD_SETSIZE, &fdSet, NULL, NULL, &nTimeVal);
		if( ret_num == 0 )
		{
			dslog(DS_LOG_ERROR, "recv_tcp_data select TimeOut.ErrorNum (%d)\n" , getSockErrno() );
            CBB::cbb_task_delay(10);
			continue;
		}
		else if( ret_num == SOCKET_ERROR )
		{
			dslog(DS_LOG_ERROR, "recv_tcp_data select Failed.ErrorNum (%d)\n" , getSockErrno() );
            CBB::cbb_task_delay(10);
			continue;
		}

		if (!FD_ISSET(s, &fdSet))
		{
			continue;
		}
#endif
        ret = recv(s, (char *)buf + len, size - len, 0);
		if (ret == -1)
		{
			dslog(DS_LOG_DEBUG, "recv tcp data fail:%s\n", strerror(errno));
			if (count >= timeout / 10)
            {
				break;
            }
			else
			{
				dslog(DS_LOG_DEBUG, "recv inter socket fail, err:%d %s\n", errno, strerror(errno));
#ifdef USE_EPOLL
                if (bCmdOpt)
                {
                    continue;
                }
#endif
                CBB::cbb_task_delay(10);
				continue;
			}
		}
		else
		{
			len += ret;
		}

		if (len != size && count < timeout / 10)
		{
#ifdef USE_EPOLL
            if (bCmdOpt)
            {
                continue;
            }
#endif
            CBB::cbb_task_delay(10);
		}
	}

#ifdef USE_EPOLL
    if (bCmdOpt)
    {
	    if (0 != epoll_ctl(nInterEpollFd, EPOLL_CTL_DEL, s, NULL))
	    {
		    dslog(DS_LOG_ERROR, "epoll_ctl del failed! errno(%d)(%s)\n", errno, strerror(errno));
	    }

        if (-1 != nInterEpollFd)
        {
            close(nInterEpollFd);
            nInterEpollFd = -1;
        }
    }
#endif

    return len;
}

static uint32_t send_tcp_data(SOCKHANDLE s, uint8_t * buf, uint32_t size, uint32_t timeout)
{
	uint32_t len = 0;
	int32_t ret ;
	uint32_t count = 0;

	while ( count ++ < timeout / 10 && len != size )
	{
		ret = send(s, (char *)buf + len, size -len , 0);
		if ( ret == -1 )
		{
			dslog(DS_LOG_DEBUG, "send tcp data fail:%s\n", strerror(errno));
			if ( count >= timeout / 10 )
            {
				break;
            }
			else
			{
				dslog(DS_LOG_DEBUG, "send inter socket fail, err:%d %s\n", errno, strerror(errno));
                CBB::cbb_task_delay(10);
				continue;
			}
		}
		else
		{
			len += ret;
		}

		if ( len != size && count < timeout / 10)
		{
			dslog(DS_LOG_DEBUG, "send tcp data sleep 10ms, len=%u, size=%u, count=%u, timeout=%u\n", len, size, count, timeout);
			CBB::cbb_task_delay(10);
		}
	}

	return len ;
}

static int GetLoopSock(int fds[])
{
    SOCKHANDLE s1,s2,s3;

    struct sockaddr_in ad;
    struct sockaddr_in realad;
    socklen_t adlen;
    short port;
    int32_t nSockPort = DATASWITCH_PORT_START;

    s1 = socket(AF_INET, SOCK_STREAM, 0);
    if (s1 < 0)
    {
        dslog(DS_LOG_ERROR, "%s %d : create socket error\n",__FILE__,__LINE__);
        return -1;
    }

    s2 = socket(AF_INET, SOCK_STREAM, 0);
    if (s2 < 0)
    {
        dslog(DS_LOG_ERROR, "%s %d : create socket error\n",__FILE__,__LINE__);
        return -1;
    }

    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = inet_addr("127.0.0.1");
    ad.sin_port = htons(nSockPort);

    while( (SOCKET_ERROR == bind(s1, (struct sockaddr *)&ad, sizeof(struct sockaddr_in))) &&
                (nSockPort <= DATASWITCH_PORT_END) )
    {
        nSockPort++;
        ad.sin_port    = htons(nSockPort);
    }
    if(nSockPort > DATASWITCH_PORT_END)
    {
        /// SockClose(s1);
        /// SockClose(s2);
        CBB::cbb_sock_close(s1);
        CBB::cbb_sock_close(s2);
        nSockPort = INVALID_SOCKET;
        dslog(DS_LOG_ERROR, "%s %d : bind socket error %d\n",__FILE__,__LINE__,errno);
        return -1;
    }

    nSockPort++;
    while( (SOCKET_ERROR == bind(s2, (struct sockaddr *)&ad, sizeof(struct sockaddr_in))) &&
                (nSockPort <= DATASWITCH_PORT_END) )
    {
        nSockPort++;
        ad.sin_port    = htons(nSockPort);
    }
    if(nSockPort > DATASWITCH_PORT_END)
    {
        CBB::cbb_sock_close(s1);
        CBB::cbb_sock_close(s2);
        nSockPort = INVALID_SOCKET;
        dslog(DS_LOG_ERROR, "%s %d : bind socket error %d\n",__FILE__,__LINE__,errno);
        return -1;
    }

    memset(&realad, 0, sizeof(struct sockaddr_in) );
    adlen = sizeof(struct sockaddr_in);

    getsockname(s1, (struct sockaddr *)&realad, &adlen);
    port = realad.sin_port;

    if (listen(s1, 15)<0)
    {
        CBB::cbb_sock_close(s1);
        CBB::cbb_sock_close(s2);
        nSockPort = INVALID_SOCKET;

        dslog(DS_LOG_ERROR, "%s %d : listen socket error %d\n",__FILE__,__LINE__,errno);
        return -1;

    }

    realad.sin_addr.s_addr = inet_addr("127.0.0.1");
    realad.sin_family = AF_INET;

    if (connect(s2, (struct sockaddr *)&realad, adlen) < 0)
    {
        CBB::cbb_sock_close(s1);
        CBB::cbb_sock_close(s2);
        nSockPort = INVALID_SOCKET;

        dslog(DS_LOG_ERROR, "%s %d : connect socket error %d\n",__FILE__,__LINE__,errno);
        return -1;
    }

    s3 = accept(s1, NULL, NULL);
    if (s3 < 0)
    {
        CBB::cbb_sock_close(s1);
        CBB::cbb_sock_close(s2);
        nSockPort = INVALID_SOCKET;

        dslog(DS_LOG_ERROR, "%s %d : accept socket error %d\n",__FILE__,__LINE__,errno);
        return -1;
    }

    CBB::cbb_sock_close(s1);

    fds[0] = s2;
    fds[1] = s3;

    return 0;
}

DS_API void dsSetApiTimeOut(int32_t timeout)
{
    if ( timeout <= 0 )
      g_nDsApiTimeOut = API_TIME_OUT ;
    else
      g_nDsApiTimeOut = timeout ;
}

DS_API void dsstep()
{
	OspPrintf(1, 0, "ds step:%d ds ip:%x ds port:%d\n", g_dwDsStep,g_dwDsIp, g_dwDsPort);
}

CSockSwitch::CSockSwitch()
{
	m_dsSpyId = INVALID_DSID;
	m_dwSpyInstId = 0;
	m_nSpyUDPDataLen = 0;
	m_wSpyEvent  = 0;
#ifdef USE_EPOLL
	m_ptEpollEvent = NULL;
#endif

    m_aptMmbSameSrc = NULL;
    m_aptMmbDefaultSrc = NULL;
}

CSockSwitch::~CSockSwitch()
{
#ifdef USE_EPOLL
	if (NULL != m_ptEpollEvent)
	{
		delete[] m_ptEpollEvent;
		m_ptEpollEvent = NULL;
	}

    if (-1 != g_nDSTaskEpollFd)
	{
		close(g_nDSTaskEpollFd);
		g_nDSTaskEpollFd = -1;
	}
#endif

    if (m_aptMmbSameSrc)
    {
        delete[] m_aptMmbSameSrc;
    }
    if (m_aptMmbDefaultSrc)
    {
        delete[] m_aptMmbDefaultSrc;
    }
}

DSID CSockSwitch::Create( uint32_t num ,uint32_t adwIP[] )
{
	DSID dsId ;
	uint32_t Idx;

	if( num > MAXNUM_DSINTERFACE )
	{
		dslog(DS_LOG_DEBUG,  "reg %d if large than dataswitch support max if ip is %d\n",num,MAXNUM_DSINTERFACE);
		return INVALID_DSID;
	}

	dsId = dsAllocDSID();
	if( INVALID_DSID == dsId )
	{
		return INVALID_DSID;
	}
	if( INVALID_DSID != dsId)
	{

		for( Idx =0 ;Idx < num ;Idx++)
		{
			if( !m_cDSIf.Add( adwIP[Idx] ,dsId ) )
			{
				dsFreeDSID( dsId );
				m_cDSIf.Remove( dsId );
				dsId = INVALID_DSID;
				break;
			}
		}
	}

	return dsId;
}


/*===================================================
	函数  : GetRcvGrp
	功能  : 获取指定接收组的指针
	输入  : dwIP  - 该组接收IP
	        wPort - 该组接收端口号
	输出  : 无
	返回  : 若存在返回接收组指针,否则返回NULL
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
TRcvGrp* CSockSwitch::GetRcvGrp( uint32_t dwIP ,uint16_t wPort )
{
	for( uint32_t dwIdx= 0 ;dwIdx < g_dwMaxRcvGrp ;dwIdx++ )
	{
		if( !(g_ptRcvGrpPool+dwIdx)->bUsing )
			continue;

		if( (g_ptRcvGrpPool+dwIdx)->cSock.IsEqual( dwIP ,wPort) )
		{
			return (g_ptRcvGrpPool+dwIdx);
		}
	}

	return NULL;
}


/*===================================================
	函数  : GetSndMmb
	功能  : 获取指定接收组的转发成员指针
	输入  : dwIP  - 该成员IP
	        	  wPort - 该成员端口号
	输出  : 无
	返回  : 若存在返回成员指针,若不存在且
			  所属接收规则节点的发送目标数
			  已达最大限制则返回-1的相应指针
			  形式,否则返回NULL
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
	2007/5/16              高祺          增加容量限制
=====================================================*/
TSndMmb*  CSockSwitch::GetSndMmb( TRcvGrp* ptGrp ,uint32_t dwIP ,uint16_t wPort )
{
	TSndMmb* ptSndMmb;
	uint32_t dwTgtsOfGrp = 0;

	if( NULL == ptGrp ) return NULL;

	ptSndMmb = ptGrp->ptSndMmb;
	while( ptSndMmb )
	{
		if( ptSndMmb->dwDstIP  == dwIP &&
			ptSndMmb->wDstPort == wPort)
		{
			return ptSndMmb;
		}
		dwTgtsOfGrp++;
		ptSndMmb = ptSndMmb->ptNextMmb;
	}

	return NULL;
}



/*===================================================
	函数  : GetSndMmb
	功能  : 获取指定接收组的转发成员指针
	输入  :
			dwSrcIP 源地址
			wSrcPort 源端口
			dwDstIP 目的地址
	        wDstPort 目的端口
	输出  : 无
	返回  : 若存在返回成员指针,若不存在且
			  所属接收规则节点的发送目标数
			  已达最大限制则返回-1的相应指针
			  形式,否则返回NULL
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
	2007/5/16              高祺          增加容量限制
=====================================================*/
TSndMmb*  CSockSwitch::GetSndMmb( TRcvGrp* ptGrp ,uint32_t dwSrcIP, uint16_t dwSrcPort, uint32_t dwDstIP ,uint16_t wDstPort )
{
	TSndMmb* ptSndMmb;
	uint32_t dwTgtsOfGrp = 0;

	if( NULL == ptGrp ) return NULL;

	ptSndMmb = ptGrp->ptSndMmb;
	while( ptSndMmb )
	{
		if (ptSndMmb->dwSrcIP == dwSrcIP &&
				ptSndMmb->wSrcPort == dwSrcPort)
		{
			dwTgtsOfGrp ++;
		}

		if( ptSndMmb->dwDstIP == dwDstIP &&
			ptSndMmb->wDstPort == wDstPort &&
			ptSndMmb->dwSrcIP == dwSrcIP &&
			ptSndMmb->wSrcPort== dwSrcPort)
		{
			return ptSndMmb;
		}

		ptSndMmb = ptSndMmb->ptNextMmb;
	}

	if (dwTgtsOfGrp < g_dwMaxSndMmbPerRcvGrp)
	{
		return NULL;
	}
	else
	{
		return (TSndMmb*)(-1);
	}
}


/*===================================================
	函数  : Add
	功能  : 添加转发成员
	输入  : DSID dsId ,
		    uint32_t  dwLocalIP,
		    uint16_t  wLocalPort,
		    uint32_t  dwLoalIfIP,
		    uint32_t  dwSrcIP ,
		    uint16_t  wSrcPort,
		    uint32_t  dwDstIP,
		    uint16_t  wDstPort,
		    uint32_t  dwDstOutIfIP
	输出  : 无
	返回  : 若存在返回成员指针,否则返回NULL
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
uint32_t CSockSwitch::Add(DSID dsId ,
		uint32_t  dwLocalIP,
		uint16_t  wLocalPort,
		uint32_t  dwLoalIfIP,
		uint32_t  dwSrcIP ,
		uint16_t  wSrcPort,
		uint32_t  dwDstIP,
		uint16_t  wDstPort,
		uint32_t  dwDstOutIfIP,
		uint32_t   dwBufSize)
{
	TRcvGrp* ptRcvGrp = NULL;
	TSndMmb* ptSndMmb = NULL;

	//检测ID合法性
	if( !IsExistDSID(dsId)) return DSERROR;

	/*
	uint32_t dwIFIP[2];
	uint32_t dwIFNum = m_cDSIf.GetIP( dsId, dwIFIP ,2);
	if( 0 == dwIFNum )
	{
		dslog(DS_LOG_DEBUG,  "DSID.%d get if ip failed!\n",dsId);
		return DSERROR;
	}
	*/
	if(dwLoalIfIP == 0) dwLoalIfIP = dwLocalIP;

	//检测接收组是否已经存在，若不存在则创建
	ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
	if( NULL == ptRcvGrp )
	{
		ptRcvGrp = AllocGrp( dwLocalIP ,wLocalPort ,dwLoalIfIP, dwBufSize );
		if( NULL == ptRcvGrp )
		{
			dslog(DS_LOG_DEBUG, "Alloc recv group failed!\n");
			return DSERROR;
		}
	}

	if( IS_MCIP( dwLocalIP))
	{
		if( ! ptRcvGrp->cSock.JoinMc( dwLocalIP ,dwLoalIfIP  ) )
		{
			FreeMmb( ptSndMmb );
			if( NULL == ptRcvGrp->ptSndMmb &&
				0L   == ptRcvGrp->dwDump   &&
				0L   == ptRcvGrp->dwSpying)
			{
				FreeGrp( ptRcvGrp );
			}
			dslog(DS_LOG_DEBUG,  "grp join mc %s.%d.%s failed!\n",ipString(dwDstIP),wDstPort,ipString(dwDstOutIfIP));
			return DSERROR;
		}
	}

	//垃圾接收组
	if( 0 == dwDstIP && 0==wDstPort && 0==dwDstOutIfIP &&
		0L == dwSrcIP && 0==wSrcPort )
	{
		SET_HANDLE( dsId ,ptRcvGrp->dwDump );
		return DSOK;
	}

	//嗅听
	if(    IsSetIP(dwDstIP) && IsSetPort(wDstPort) && IsSetIP(dwDstOutIfIP) &&
		   IsSetIP(dwSrcIP) && IsSetPort(wSrcPort ) )
	{
		SET_HANDLE( dsId ,ptRcvGrp->dwSpying );
		ptRcvGrp->dwInstId = m_dwSpyInstId;
		return DSOK;
	}

	//检测目的转发成员是否存在，若不存在添加
	ptSndMmb = GetSndMmb( ptRcvGrp, dwSrcIP, wSrcPort, dwDstIP, wDstPort );
	if( NULL != ptSndMmb )
	{
		if (ptSndMmb == (TSndMmb*)(-1))
		{
			dslog(DS_LOG_DEBUG, "Add sending item failed (reach \"MaxSndMmbPerRcvGrp\" limit)!\n");
			return DSERROR;
		}

		dslog(DS_LOG_DEBUG, "Add an exist member!\n");
		return DSOK;
	}

	if(dwDstOutIfIP == 0) dwDstOutIfIP = dwLocalIP;

	ptSndMmb = AllocMmb( dwDstIP, wDstPort, dwDstOutIfIP);
	if( NULL == ptSndMmb )
	{
		if( NULL == ptRcvGrp->ptSndMmb &&
			0L   == ptRcvGrp->dwDump   &&
			0L   == ptRcvGrp->dwSpying)
		{
			FreeGrp( ptRcvGrp );
		}
		dslog(DS_LOG_DEBUG,  "Alloc mmb %s.%d.%s failed!\n",ipString(dwDstIP),wDstPort,ipString(dwDstOutIfIP));
		return DSERROR;
	}

	if( IS_MCIP( dwDstIP ) )
	{
		if( ! ptRcvGrp->cSock.JoinMc( dwDstIP ,dwDstOutIfIP ) )
		{
			FreeMmb( ptSndMmb );
			if( NULL == ptRcvGrp->ptSndMmb &&
				0L   == ptRcvGrp->dwDump   &&
				0L   == ptRcvGrp->dwSpying)
			{
				FreeGrp( ptRcvGrp );
			}
			dslog(DS_LOG_DEBUG,  "mmb join mc %s.%d.%s failed!\n",ipString(dwDstIP),wDstPort,ipString(dwDstOutIfIP));
			return DSERROR;
		}
	}

	ptSndMmb->ptNextMmb = ptRcvGrp->ptSndMmb;
	ptRcvGrp->ptSndMmb  = ptSndMmb;
	ptSndMmb->dwSrcIP  = dwSrcIP;
	ptSndMmb->wSrcPort = wSrcPort;
	ptSndMmb->dsId     = dsId;
    ptSndMmb->ptRcv    = ptRcvGrp;

    //add to hash
    AddtoSndHash(ptSndMmb, dwLocalIP, wLocalPort);


	return DSOK;
}



uint32_t CSockSwitch::SetUserData(DSID dsId ,
		uint32_t  dwLocalIP,
		uint16_t  wLocalPort,
		uint32_t  dwLoalIfIP,
		uint32_t  dwSrcIP ,
		uint16_t  wSrcPort,
		uint32_t  dwDstIP,
		uint16_t  wDstPort,
		uint32_t  dwDstOutIfIP,
        bool bSend,
        uint8_t *pchUserData,
        int32_t nUserDataLen)
{
	TRcvGrp* ptRcvGrp = NULL;
	TSndMmb* ptSndMmb = NULL;

	//检测ID合法性
	if( !IsExistDSID(dsId)) return DSERROR;

	/*
	uint32_t dwIFIP[2];
	uint32_t dwIFNum = m_cDSIf.GetIP( dsId, dwIFIP ,2);
	if( 0 == dwIFNum )
	{
		dslog(DS_LOG_DEBUG,  "DSID.%d get if ip failed!\n",dsId);
		return DSERROR;
	}
	*/
	if(dwLoalIfIP == 0) dwLoalIfIP   = dwLocalIP;

    //检测接收组是否已经存在，若不存在返回
	ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
	if( NULL == ptRcvGrp )
	{
		dslog(DS_LOG_DEBUG, "Grp.%s@%d not exist\n",ipString(dwLocalIP),wLocalPort);
		return DSERROR;
	}

    if (!bSend)
    {
        ptRcvGrp->nUserDataLen = nUserDataLen;
        memcpy(ptRcvGrp->achUserData, pchUserData, nUserDataLen);
    }
    else
    {
	    //查找并设置用户数据
        ptSndMmb     = ptRcvGrp->ptSndMmb;
        while( ptSndMmb )
        {
            if( dsId == ptSndMmb->dsId &&
                dwSrcIP == ptSndMmb->dwSrcIP &&
                wSrcPort == ptSndMmb->wSrcPort &&
                dwDstIP == ptSndMmb->dwDstIP &&
                wDstPort == ptSndMmb->wDstPort)
            {
                ptSndMmb->nUserDataLen = nUserDataLen;
                memcpy(ptSndMmb->achUserData, pchUserData, nUserDataLen);
                break;
            }

            ptSndMmb = ptSndMmb->ptNextMmb;
        }

        if (NULL == ptSndMmb)
        {
            //没有找到
            return DSERROR;
        }
    }

	return DSOK;
}


uint32_t CSockSwitch::Remove(DSID dsId,
						uint32_t dwLocalIP,
						uint16_t wLocalPort,
						uint32_t dwSrcIP,
						uint16_t wSrcPort,
						uint32_t dwDstIP,
						uint16_t wDstPort)
{
	bool     bRemove;
	TRcvGrp* ptRcvGrp = NULL;
	TSndMmb* ptSndMmb = NULL;
	TSndMmb** pptPreSndMmb;

	//检测ID合法性
	if( !IsExistDSID(dsId)) return DSERROR;

	//检测接收组是否已经存在，若不存在返回
	ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
	if( NULL == ptRcvGrp )
	{
		dslog(DS_LOG_DEBUG, "Grp.%s@%d not exist\n",ipString(dwLocalIP),wLocalPort);
		return DSERROR;
	}

	//删除嗅听组
	if( IsSetIP(dwSrcIP) && IsSetIP(dwDstIP) && IsSetPort(wSrcPort) && IsSetPort(wDstPort) )
	{
		if( IS_HANLDE( dsId ,ptRcvGrp->dwSpying ) )
		{
			CLR_HANDLE( dsId ,ptRcvGrp->dwSpying );
			if( NULL == ptRcvGrp->ptSndMmb &&
				0L   == ptRcvGrp->dwDump   &&
				0L   == ptRcvGrp->dwSpying)
			{
				FreeGrp( ptRcvGrp );
			}
			return DSOK;
		}
		return DSERROR;
	}

	ptSndMmb     = ptRcvGrp->ptSndMmb;
	pptPreSndMmb = &ptRcvGrp->ptSndMmb;

	while( ptSndMmb )
	{
		if( dsId != ptSndMmb->dsId )
		{//ID不匹配
			pptPreSndMmb = &ptSndMmb->ptNextMmb;
			ptSndMmb     = ptSndMmb->ptNextMmb;
			continue;
		}

		bRemove = false;
		if( IsValidIP( dwSrcIP) && IsValidPort( wSrcPort) )
		{
			if( ptSndMmb->dwSrcIP == dwSrcIP && ptSndMmb->wSrcPort == wSrcPort )
			{//匹配的源IP端口号
				if( IsSetIP( dwDstIP) && IsSetPort( wDstPort ) )
				{//删除所有根据 源 dwSrcIP@wSrcPort 转发的目的成员
					bRemove = true;
				}
				else if( dwDstIP == ptSndMmb->dwDstIP && wDstPort == ptSndMmb->wDstPort)
				{//删除根据源  dwSrcIP@wSrcPort 转发 到dwDstIP@wDstPort的成员
					bRemove = true;
				}
			}
		}
		else if( IsZeroIP( dwSrcIP) && IsZeroPort( wSrcPort) )
		{
			if( dwDstIP == ptSndMmb->dwDstIP && wDstPort == ptSndMmb->wDstPort)
				bRemove = true;
		}

		if( bRemove )
		{
			//remove from hash
			RemovefromSndHash(ptSndMmb, dwLocalIP, wLocalPort);
			ptSndMmb->ptRcv = NULL;

			*pptPreSndMmb = ptSndMmb->ptNextMmb;
			if( IS_MCIP( ptSndMmb->dwDstIP ) )
				ptRcvGrp->cSock.DropMc( ptSndMmb->dwDstIP ,ptSndMmb->dwDstOutIfIP );
			FreeMmb( ptSndMmb );
			ptSndMmb = *pptPreSndMmb;
			dslog(DS_LOG_DEBUG, "Remove DS(%d) L(%s@%d) ",dsId,ipString(dwLocalIP),wLocalPort);
			dslog(DS_LOG_DEBUG, "S(%s@%d) ",ipString(dwSrcIP),wSrcPort);
			dslog(DS_LOG_DEBUG, "D(%s@%d) .\n",ipString(dwDstIP),wDstPort);
		}
		else
		{
			pptPreSndMmb = &ptSndMmb->ptNextMmb;
			ptSndMmb = ptSndMmb->ptNextMmb;
		}

	}

	//要删除一个垃圾端口
	if( 0 == dwSrcIP && 0 == wSrcPort&&
		0 == dwDstIP && 0 == wDstPort )
	{
		CLR_HANDLE( dsId ,ptRcvGrp->dwDump);
	}

	//要删除一个嗅听组
	if( NULL == ptRcvGrp->ptSndMmb &&
		0L   == ptRcvGrp->dwDump   &&
		0L   == ptRcvGrp->dwSpying)
	{
		FreeGrp( ptRcvGrp );
	}

 	return DSOK;
}


uint32_t CSockSwitch::RemoveAllMmb(DSID dsId,
							  uint32_t dwSrcIP,
							  uint16_t wSrcPort,
							  uint32_t dwDstIP,
							  uint16_t wDstPort)
{
	for( uint32_t dwIdx= 0 ;dwIdx < g_dwMaxRcvGrp ;dwIdx++ )
	{
		if( !(g_ptRcvGrpPool+dwIdx)->bUsing )continue;

		Remove( dsId,
				(g_ptRcvGrpPool+dwIdx)->cSock.GetIP(),
				(g_ptRcvGrpPool+dwIdx)->cSock.GetPort(),
				dwSrcIP,
				wSrcPort,
				dwDstIP,
				wDstPort );
	}

	return DSOK;
}


/*===================================================
	函数  : RemoveAll
	功能  : 清除所有交换成员
	输入  : dsId - 交换句柄
	输出  : 无
	返回  : 无
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
uint32_t CSockSwitch::RemoveAll( DSID dsId, bool bRemoveDump /*= TRUE*/)
{
	TSndMmb *ptMmb, **pptPreMmb;
	for( uint32_t dwGrpIdx = 0; dwGrpIdx < g_dwMaxRcvGrp; dwGrpIdx++ )
	{
		if( (g_ptRcvGrpPool+dwGrpIdx)->bUsing )
		{
			pptPreMmb = &(g_ptRcvGrpPool+dwGrpIdx)->ptSndMmb;
			ptMmb = (g_ptRcvGrpPool+dwGrpIdx)->ptSndMmb;
			while( ptMmb )
			{
				if( ptMmb->dsId == dsId )
				{
					if( IS_MCIP(ptMmb->dwDstIP) )
					{
						(g_ptRcvGrpPool+dwGrpIdx)->cSock.DropMc( ptMmb->dwDstIP ,ptMmb->dwDstOutIfIP);
					}
					//remove from hash
					RemovefromSndHash(ptMmb, \
						(g_ptRcvGrpPool+dwGrpIdx)->cSock.GetIP(), \
						(g_ptRcvGrpPool+dwGrpIdx)->cSock.GetPort());
					ptMmb->ptRcv = NULL;

					*pptPreMmb = ptMmb->ptNextMmb;
					FreeMmb(ptMmb);
					ptMmb = *pptPreMmb;
				}
				else
				{
					pptPreMmb = &ptMmb->ptNextMmb;
					ptMmb = ptMmb->ptNextMmb;
				}
			}

			if (bRemoveDump)
			{
				CLR_HANDLE( dsId, (g_ptRcvGrpPool+dwGrpIdx)->dwDump );
			}

			CLR_HANDLE( dsId, (g_ptRcvGrpPool+dwGrpIdx)->dwSpying );
			if( NULL == (g_ptRcvGrpPool+dwGrpIdx)->ptSndMmb &&
				0L   == (g_ptRcvGrpPool+dwGrpIdx)->dwDump   &&
				0L   == (g_ptRcvGrpPool+dwGrpIdx)->dwSpying )
			{
				FreeGrp( (g_ptRcvGrpPool+dwGrpIdx) );
			}
		}
	}
	return DSOK;
}


bool CSockSwitch::AllocMemory()
{
	g_ptRcvGrpPool = new TRcvGrp[g_dwMaxRcvGrp];
	if( NULL == g_ptRcvGrpPool )
	{
		return false;
	}

	g_ptSndMmbPool = new TSndMmb[g_dwMaxRcvGrp*g_dwMaxSndMmbPerRcvGrp];
	if( NULL == g_ptSndMmbPool )
	{
		FreeMemory();
		return false;
	}

    g_aptSndHash = new TSndMmb* [g_dwSndHashSize];
    memset(g_aptSndHash, 0, g_dwSndHashSize*sizeof(TSndMmb*));

	g_ptNetSndMembers = new TNetSndMember[g_dwMaxSndMmbPerRcvGrp];
	if( NULL == g_ptNetSndMembers )
	{
		FreeMemory();
		return false;
	}

    m_aptMmbSameSrc = new TSndMmb*[g_dwMaxSndMmbPerRcvGrp];
    if( NULL == m_aptMmbSameSrc )
    {
        FreeMemory();
        return false;
    }

    m_aptMmbDefaultSrc = new TSndMmb*[g_dwMaxSndMmbPerRcvGrp];
    if( NULL == m_aptMmbDefaultSrc )
    {
        FreeMemory();
        return false;
    }

	return true;
}


void CSockSwitch::FreeMemory()
{
	if( NULL != g_ptRcvGrpPool )
	{
		delete [] g_ptRcvGrpPool;
		g_ptRcvGrpPool = NULL;
	}

	if( NULL != g_ptSndMmbPool )
	{
		delete [] g_ptSndMmbPool;
		g_ptSndMmbPool = NULL;
	}

    if (NULL != g_aptSndHash)
    {
        delete [] g_aptSndHash;
        g_aptSndHash = NULL;
    }

    if (NULL != g_aptRcvHash)
    {
        delete [] g_aptRcvHash;
        g_aptRcvHash = NULL;
    }

	if( NULL != g_ptNetSndMembers )
	{
		delete[] g_ptNetSndMembers;
		g_ptNetSndMembers = NULL;
	}

    if (m_aptMmbSameSrc)
    {
        delete[] m_aptMmbSameSrc;
        m_aptMmbSameSrc = NULL;
    }

    if (m_aptMmbDefaultSrc)
    {
        delete[] m_aptMmbDefaultSrc;
        m_aptMmbDefaultSrc = NULL;
    }
}

/*===================================================
	函数  : InitGrpMmbPool
	功能  : 初始化接收组和转发成员池
	输入  : 无
	输出  : 无
	返回  : 无
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
void CSockSwitch::InitGrpMmbPool()
{
	//将个组设置为未使用状态
	for( uint32_t dwGrpIdx = 0 ; dwGrpIdx < g_dwMaxRcvGrp ; dwGrpIdx++ )
	{
		(g_ptRcvGrpPool+dwGrpIdx)->bUsing = false;
		(g_ptRcvGrpPool+dwGrpIdx)->dwDump = 0;
		(g_ptRcvGrpPool+dwGrpIdx)->dwSpying = 0;
		(g_ptRcvGrpPool+dwGrpIdx)->ptSndMmb = NULL;
		(g_ptRcvGrpPool+dwGrpIdx)->dwInstId = 0;
		(g_ptRcvGrpPool+dwGrpIdx)->pfFilter = NULL;
		(g_ptRcvGrpPool+dwGrpIdx)->pfCallback = NULL;
		(g_ptRcvGrpPool+dwGrpIdx)->dwTraceNum = 0;
	}

	//建立成员池
	memset( g_ptSndMmbPool ,0 , g_dwMaxRcvGrp*g_dwMaxSndMmbPerRcvGrp*sizeof(TSndMmb) );
	uint32_t dwMmbIdx;
	for( dwMmbIdx=0 ;dwMmbIdx < g_dwMaxRcvGrp*g_dwMaxSndMmbPerRcvGrp-1; dwMmbIdx++)
		//"-1" for avoiding the error of "offset by 1"
	{
		(g_ptSndMmbPool+dwMmbIdx)->ptNextMmb = (g_ptSndMmbPool+dwMmbIdx+1);
	}

	(g_ptSndMmbPool+dwMmbIdx)->ptNextMmb = NULL;
	g_ptFreeSndMmb = g_ptSndMmbPool;
}

/*===================================================
	函数  : AllocMmb
	功能  : 分配成员
	输入  : 无
	输出  : 无
	返回  : 成功返回成员指针,否则返回NULL
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
TSndMmb* CSockSwitch::AllocMmb(uint32_t  dwDstIP,uint16_t  wDstPort, uint32_t  dwDstOutIfIP)
{
	TSndMmb* ptSndMmb;
	if( NULL == g_ptFreeSndMmb )
	{
		dslog(DS_LOG_ERROR,  "Member Pool is empty!\n");
		return NULL;
	}

	ptSndMmb = g_ptFreeSndMmb;
	g_ptFreeSndMmb = ptSndMmb->ptNextMmb;

	memset( ptSndMmb , 0 , sizeof(TSndMmb) );
	ptSndMmb->dwDstIP      = dwDstIP;
	ptSndMmb->wDstPort     = wDstPort;
	ptSndMmb->dwDstOutIfIP = dwDstOutIfIP;
    ptSndMmb->nUserDataLen = 0;
	ptSndMmb->dwRawIp = 0;
	ptSndMmb->wRawPort = 0;

	return ptSndMmb;
}

/*===================================================
	函数  : FreeMmb
	功能  : 释放成员到成员池中
	输入  : 无
	输出  : 无
	返回  : 无
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
void CSockSwitch::FreeMmb( TSndMmb* ptSndMmb )
{
	if( NULL == ptSndMmb ) 	return;

	memset( ptSndMmb ,0,sizeof(TSndMmb) );

	ptSndMmb->ptNextMmb = g_ptFreeSndMmb;
	g_ptFreeSndMmb = ptSndMmb;

}

TRcvGrp*  CSockSwitch::AllocGrp( uint32_t dwLocalIP ,uint16_t wLocalPort ,uint32_t dwLoalIfIP, uint32_t dwBufSize )
{
	TRcvGrp* ptRcvGrp = NULL;
	uint64_t qwIndex = 0;
	for( uint32_t dwIdx =0 ;dwIdx < g_dwMaxRcvGrp ;dwIdx++ )
	{
		if( !(g_ptRcvGrpPool+dwIdx)->bUsing )
		{
			ptRcvGrp = (g_ptRcvGrpPool+dwIdx);
			if( !ptRcvGrp->cSock.Create( dwLocalIP ,wLocalPort , dwLoalIfIP, dwBufSize ) )
			{
				dslog(DS_LOG_ERROR, "create socket failed when AllocGrp!\n");
				return NULL;
			}
#ifdef USE_EPOLL
			//connection socket join in epoll set
			epoll_event tEpollEvent;
			memset( &tEpollEvent , 0 , sizeof(epoll_event) );
			qwIndex = dwIdx;
			tEpollEvent.data.u64 = (qwIndex << 32) + ptRcvGrp->cSock.GetSock();
			tEpollEvent.events = EPOLLIN ;

			if( 0 != epoll_ctl( g_nDSTaskEpollFd , EPOLL_CTL_ADD , ptRcvGrp->cSock.GetSock() , &tEpollEvent ) )
			{
				ptRcvGrp->cSock.Destroy();
				dslog(DS_LOG_ERROR, "epoll_ctl add connecting socket failed! errno(%d)(%s)" , errno , strerror(errno) );
				return NULL;
			}
#endif
			ptRcvGrp->bUsing = true;
			ptRcvGrp->dwDump = 0;
			ptRcvGrp->dwSpying = 0;
			ptRcvGrp->ptSndMmb = NULL;
			ptRcvGrp->dwInstId = 0;
			ptRcvGrp->pfFilter = NULL;
			ptRcvGrp->pfCallback = NULL;
            ptRcvGrp->nUserDataLen = 0;

            AddtoRcvHash(ptRcvGrp);

			return ptRcvGrp;
		}
	}
	return NULL;
}

void CSockSwitch::FreeGrp(TRcvGrp* ptRcvGrp )
{
	if( NULL == ptRcvGrp )
		return;

	//必须在套接字被关闭之前，将接收组从hash表中移除该接受组
	//否则将造成hash表的混乱 chg by linlifen 2010.11.5
	RemovefromRcvHash(ptRcvGrp);

	ptRcvGrp->cSock.Destroy();
	ptRcvGrp->bUsing = false;
	ptRcvGrp->dwDump = 0;
	ptRcvGrp->dwSpying = 0;
	ptRcvGrp->ptSndMmb = NULL;
	ptRcvGrp->dwInstId = 0;
	ptRcvGrp->pfFilter = NULL;
	ptRcvGrp->pfCallback = NULL;
	ptRcvGrp->nUserDataLen = 0;
}

uint32_t CSockSwitch::RegSpy( DSID dsId , uint32_t dwInstId , uint16_t wEvent,uint16_t wUDPDataLen)
{
	if( INVALID_DSID != m_dsSpyId ||
		INVALID_DSID == dsId      ||
		0 == dwInstId ||
		0 ==  wEvent ||
		0 ==  wUDPDataLen)
		return DSERROR;
	m_dsSpyId = dsId;
	m_dwSpyInstId   = dwInstId;
	m_wSpyEvent     = wEvent;
	m_nSpyUDPDataLen = (int)wUDPDataLen;
	return DSOK;

}

uint32_t CSockSwitch::UnRegSpy(DSID dsId )
{
	if( INVALID_DSID != m_dsSpyId ||INVALID_DSID == dsId )return DSERROR;
	//
	//将所嗅听组去掉
	//
	return DSOK;

}

///////////////////////////////////////////////////////
//
//    <<               >>
//
//
///////////////////////////////////////////////////////
/*===================================================
	函数  : StartupInterServer
	功能  : 启动内部服务并等待连接
	输入  : 无
	输出  : 无
	返回  : 无
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
bool CSockSwitch::StartupInterServer()
{
#ifdef USE_EPOLL
	//内部socket加入epoll读集
	epoll_event tEpollEvent;
	memset( &tEpollEvent , 0 , sizeof(epoll_event) );

	uint64_t qwFdIndex = DSINTERPROCID;
	tEpollEvent.data.u64 = (qwFdIndex << 32) + m_sktInterServ;
	tEpollEvent.events = EPOLLIN | EPOLLPRI;

	if( 0 != epoll_ctl( g_nDSTaskEpollFd, EPOLL_CTL_ADD, m_sktInterServ, &tEpollEvent ) )
	{
		dslog(DS_LOG_ERROR,  "epoll_ctl add m_sktInterServ failed! errno(%d)(%s)\n" , errno , strerror(errno) );
		return false;
	}
#endif

    u_long on = 1;
#ifdef WIN32
	on = 1;
    ioctlsocket(m_sktInterServ, FIONBIO, &on);
	on = 1;
	ioctlsocket(m_sktInterClient, FIONBIO, &on);
#else
	on = 1;
    ioctl(m_sktInterServ, FIONBIO, &on);
	on = 1;
	ioctl(m_sktInterClient, FIONBIO, &on);
#endif

	return true;
}

/*===================================================
	函数  : CreateInterSock
	功能  : 创建内部服务Socket
	输入  : 无
	输出  : 无
	返回  : 无
	注    :
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
bool CSockSwitch::CreateInterSock()
{
	int fds[2];
	int ret;

	ret = GetLoopSock(fds);
	if (ret < 0)
	{
		dslog(DS_LOG_ERROR, "Bind failed !! Server listen port.\n");
		return false;
	}

	m_sktInterServ = fds[0];
	m_sktInterClient = fds[1];

	return true;
}

DS_API void dsjustrecv()
{
	if (g_bDsJustRecv	== true)
	{
		OspPrintf(1, 0, "ds just recv off\n");
		g_bDsJustRecv = false ;
	}
	else
	{
		g_bDsJustRecv = true;
		OspPrintf(1, 0, "ds just recv on \n");
	}

}

uint32_t g_dwTraceRcvIp = 0;
uint16_t g_wTraceRcvPort = 0;
uint32_t g_dwTraceSndIp = 0;
uint16_t g_wTraceSndPort = 0;

DS_API void dsrcv(const char* szRcvIp, uint16_t wRcvPort)
{
    if (NULL == szRcvIp)
    {
        g_dwTraceRcvIp = 0;
    }
    else
    {
        g_dwTraceRcvIp = inet_addr(szRcvIp);
    }
    g_wTraceRcvPort = wRcvPort;
}

DS_API void dssnd(const char* szSndIp, uint16_t wSndPort)
{
    if (NULL == szSndIp)
    {
        g_dwTraceSndIp = 0;
    }
    else
    {
        g_dwTraceSndIp = inet_addr(szSndIp);
    }
    g_wTraceSndPort = wSndPort;
}

int32_t g_nTransLimit = -1;
uint32_t g_dwMaxTransTick = 0;

DS_API void dstranslimit(int32_t nTransLimit)
{
    if (nTransLimit < -1)
    {
        nTransLimit = -1;
    }

    if (0 == nTransLimit || g_nTransLimit == nTransLimit)
    {
        OspPrintf(TRUE, FALSE, "[DS] current trans limit is %d, max trans tick is %u\n", g_nTransLimit, g_dwMaxTransTick);
        return;
    }

    OspPrintf(TRUE, FALSE, "[DS] old trans limit is %d\n", g_nTransLimit);
    g_nTransLimit = nTransLimit;
    OspPrintf(TRUE, FALSE, "[DS] new trans limit is %d\n", g_nTransLimit);
}

bool CSockSwitch::Init()
{
	if( m_bInit )
		return true; //已经初始化过

    /////////////////////////////////////////////////////////////////////////
    //RawSocket的创建 与 参数设置
	m_sktRawSend = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if( INVALID_SOCKET == m_sktRawSend )
	{
		OspPrintf( TRUE , FALSE , "CSockSwitch::Init RAW_SOCK failed!\n" );
		return false;
	}

#ifdef _LINUX_
    if (strlen(g_szSendIf) > 0)
    {
		uint8_t byTry = 0;
		const uint8_t TRY_MAX = 3;

		do {
			if (setsockopt(m_sktRawSend, SOL_SOCKET, SO_BINDTODEVICE, g_szSendIf, strlen(g_szSendIf) + 1) < 0)
			{
				dslog(DS_LOG_ERROR, "CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) error : (%d)%s!\n", g_szSendIf, errno, strerror(errno));
				printf("CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) error : (%d)%s!\n", g_szSendIf, errno, strerror(errno));
			}
			else
			{
				dslog(DS_LOG_DEBUG, "CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) OK!\n", g_szSendIf);
				printf("CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) OK!\n", g_szSendIf);
				break;
			}

			byTry++;
		} while(byTry < TRY_MAX);

		if (byTry >= TRY_MAX)
		{
			/// SockClose(m_sktRawSend);
            CBB::cbb_sock_close(m_sktRawSend);
			m_sktRawSend = INVALID_SOCKET;
			dslog(DS_LOG_ERROR, "CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) failed %u times!\n", g_szSendIf, byTry);
			printf("CSockSwitch::Init setsockopt SO_BINDTODEVICE (%s) failed %u times!\n", g_szSendIf, byTry);
			return false;
		}
    }
#endif

    int opt = 1;
	if( SOCKET_ERROR == setsockopt(m_sktRawSend, IPPROTO_IP, IP_HDRINCL, (char *)&opt, sizeof(opt)) )
	{
		OspPrintf( TRUE , FALSE , "CSockSwitch::Init set RAW_SOCK sockopt failed!\n" );
		return false;
	}

    //+by lxx @20090515 : 对于仅用于发送的Raw Socket，将其接收缓冲设置为0 (Bug00017128)。
    int nRawRcvBufSize = 0;
	setsockopt(m_sktRawSend, SOL_SOCKET, SO_RCVBUF, (char*)&nRawRcvBufSize, sizeof(nRawRcvBufSize));

    /////////////////////////////////////////////////////////////////////////
    //DomainSocket的创建 与 参数设置
#ifdef _LINUX_
	m_hDomainSocket = socket( AF_LOCAL, SOCK_DGRAM, 0 );
	if( m_hDomainSocket == INVALID_SOCKET )
	{
		OspPrintf(TRUE, FALSE, "[dataswitch] create domain socket fail\n");
		return false;
	}
	int32_t nReuseAddr = 1;
	if( SOCKET_ERROR  == setsockopt(m_hDomainSocket	, SOL_SOCKET, SO_REUSEADDR,
		(char *)&nReuseAddr, sizeof(nReuseAddr)) )
	{
		OspPrintf(TRUE, FALSE, "[dataswitch]domain Socket setsockopt SO_REUSEADDR Error");
		/// SockClose(m_hDomainSocket );
		/// SockClose(m_sktRawSend);
        CBB::cbb_sock_close(m_hDomainSocket);
        CBB::cbb_sock_close(m_sktRawSend);
		return false;
	}

	int optVal,sndBufSize,rcvBufSize;
	sndBufSize = g_socksndbuff;
	rcvBufSize = g_sockrcvbuff;
	optVal = sndBufSize;
	setsockopt (m_hDomainSocket, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, sizeof (optVal));

	optVal = rcvBufSize;
	setsockopt (m_hDomainSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, sizeof (optVal));

	uint32_t on = 1;
	ioctl(m_hDomainSocket, FIONBIO, &on);

#endif

	//初始化变量
	if (false == AllocMemory())
	{
		return false;
	}

	//清零接收发送总字节数
	g_swRcvBytesCount = 0;
	g_swSndBytesCount = 0;

	//初始化转发成员池
	InitGrpMmbPool();

	//创建内部通信Socket
	if( !CreateInterSock() )
		return false;

    /////////////////////////////////////////////////////////////////////////
    //创建码流接收epoll文件描述符

#ifdef USE_EPOLL
    bool bOK = true;

    do {
	    //创建epoll文件描述符
	    if (-1 != g_nDSTaskEpollFd)
	    {
		    close(g_nDSTaskEpollFd);
		    g_nDSTaskEpollFd = -1;
	    }
	    g_nDSTaskEpollFd = epoll_create(g_dwMaxRcvGrp + 1);
	    if (-1 == g_nDSTaskEpollFd)
	    {
		    dslog(DS_LOG_ERROR, "epoll_create failed errno(%d)(%s)\n", errno, strerror(errno));
		    bOK = false;
            break;
	    }

        if (NULL != m_ptEpollEvent)
	    {
		    delete[] m_ptEpollEvent;
		    m_ptEpollEvent = NULL;
	    }
	    m_ptEpollEvent = new epoll_event[g_dwMaxRcvGrp + 1];
	    if (NULL == m_ptEpollEvent)
	    {
		    dslog(DS_LOG_ERROR, "new epoll_event array failed errno(%d)(%s)\n", errno, strerror(errno));
		    bOK = false;
            break;
	    }
	    memset(m_ptEpollEvent, 0, sizeof(epoll_event)*(g_dwMaxRcvGrp + 1));
    } while(0);

    if (!bOK)
    {
	    if (-1 != g_nDSTaskEpollFd)
	    {
		    close(g_nDSTaskEpollFd);
		    g_nDSTaskEpollFd = -1;
	    }

        if (NULL != m_ptEpollEvent)
	    {
		    delete[] m_ptEpollEvent;
		    m_ptEpollEvent = NULL;
	    }

        return false;
    }
#endif

	//启动内部线程
    int nRet = CBB::cbb_task_create(m_hThread, DataSwitchTaskProc, this);
    if (0 != nRet)
    {
        dslog(DS_LOG_ERROR, "task create failed errno(%d)\n", errno);
    }
	// m_hThread = OspTaskCreate( DataSwitchTaskProc , "DataSwitchTaskProc" , 60 , 1<<20 , (KD_PTR)this , 0 );
	m_bInit = true;

    /////////////////////////////////////////////////////////////////////////
    //初始化Telnet调试命令
#ifdef _LINUX_
	DEBUG::debug_reg_cmd("dsstep", (void*)dsstep, "step info");
	DEBUG::debug_reg_cmd("dshelp", (void*)dshelp , "print help command list");
    DEBUG::debug_reg_cmd("dsver", (void*)dsver, "print current dataswitch version");
	DEBUG::debug_reg_cmd("dsinfo", (void*)dsinfo, "print current dataswitch rules");
	DEBUG::debug_reg_cmd("ddlevel", (void*)ddlevel, "set log level");
	DEBUG::debug_reg_cmd("sndcheckhash", (void*)sndcheckhash, "");
	DEBUG::debug_reg_cmd("dstrace", (void*)dstrace, "");
	DEBUG::debug_reg_cmd("dsjustrecv", (void*)dsjustrecv, "dsjustrecv");
	DEBUG::debug_reg_cmd("dsrcv", (void*)dsrcv, "trace recv");
	DEBUG::debug_reg_cmd("dssnd", (void*)dssnd, "trace send");
	DEBUG::debug_reg_cmd("dstranslimit", (void*)dstranslimit, "set trans data count limit");
#ifdef USE_EPOLL
    DEBUG::debug_reg_cmd("dscmdopt", (void*)dscmdopt, "enable/disable cmd opt");
#endif
#endif
	return m_bInit;
}

void* CSockSwitch::DataSwitchTaskProc( void* pParam )
{
	CSockSwitch* lpThis = (CSockSwitch*)pParam;
#ifdef _LINUX_
    int ret;
    ret = prctl(PR_SET_NAME, "DataswitchTask");
    if (ret < 0)
    {
        dslog(DS_LOG_ERROR, "data switch set task name failed,errno:%d\n", errno);
    }
#endif
	return lpThis->DataSwitchTaskProck();
}

static int32_t sockBytesAvailable(SOCKHANDLE hSocket)
{
    u_long nByte;
    int32_t nRet;
#ifdef WIN32
    nRet = ioctlsocket(hSocket, FIONREAD, &nByte);
#else
	nRet = ioctl(hSocket, FIONREAD, &nByte);
#endif

    if( nRet < 0 )
    {
        return nRet;
    }

    return (int32_t)nByte;
}


void* CSockSwitch::DataSwitchTaskProck()
{
	char buf[64*1024];
	uint16_t wGrpIdx = 0;

	//等待客户端连接
	dslog(DS_LOG_ALL, "Starting inter server ....!\n");
	while(!StartupInterServer());
	dslog(DS_LOG_ALL, "Inter server recv connect!\n");

	bool bRun = true;

#ifdef _LINUX_

	cpu_set_t mask;
	CPU_ZERO( &mask );

	int NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF);

	//* CPU_SET sets only the bit corresponding to cpu.
	if (NUM_PROCS > 0)
	{
		int nProcNO = 0;
		if (NUM_PROCS > 2)
		{
			nProcNO = NUM_PROCS - 2;
		}
		CPU_SET( nProcNO, &mask );

		printf("cpu num:%d\n", NUM_PROCS);

		//* sched_setaffinity returns 0 in success
		if( sched_setaffinity( 0, sizeof(mask), &mask ) == -1 )
		{
			printf("WARNING: Could not set CPU Affinity, continuing...errno:%d\n", errno);
		}
		else
		{
			printf("bind ds recviever to cpu %d\n", nProcNO);
		}
	}

#endif

	bool bDelay = false;

	while( bRun )
	{
#ifndef _USE_WHILE_

#ifdef USE_EPOLL
		int msEventCount = epoll_wait( g_nDSTaskEpollFd , m_ptEpollEvent , g_dwMaxRcvGrp+1 , -1 );
		if( msEventCount < 0 )
		{
			dslog(DS_LOG_ERROR, "CSockSwitch::DataSwitchTaskProc epoll_wait Failed.ErrorNum (%d)\n" , getSockErrno() );
            CBB::cbb_task_delay(10);
			continue;
		}

		char* ptr = (char *)m_ptEpollEvent;
        int32_t nTransCount = 0;
        uint32_t dwTransTickStart = CBB::cbb_tick_get();
		for(int msIndex = 0; msIndex < msEventCount ; msIndex++ )
		{
			epoll_event* ptRecvEpollEvent = (epoll_event *)(ptr + msIndex*12);
			if( ptRecvEpollEvent->events & EPOLLIN)
			{
				//消息来自内部接口
				if( ( INVALID_SOCKET != m_sktInterServ ) &&
					( DSINTERPROCID == (uint32_t)(ptRecvEpollEvent->data.u64 >> 32)) )
				{
					bRun = ProssInterMsg();
				}
				else
				{
					int32_t cSockIndex = (uint32_t)(ptRecvEpollEvent->data.u64 >> 32);
					if ( ( cSockIndex >= 0 ) && ( cSockIndex < g_dwMaxRcvGrp ) &&
						 ( ( g_ptRcvGrpPool+cSockIndex)->bUsing ) &&
						 ( ( g_ptRcvGrpPool+cSockIndex)->cSock.GetSock() == (uint32_t)ptRecvEpollEvent->data.u64))
					{
						TransData( (g_ptRcvGrpPool+cSockIndex),(char *) buf,sizeof(buf));
                        nTransCount++;
					}
				}
			}

            if (g_nTransLimit > 0 && nTransCount >= g_nTransLimit)
            {
                break; // 暂不转发，以便及时处理内部消息。
            }
		}

        uint32_t dwTransTick = CBB::cbb_tick_get() - dwTransTickStart; // 忽略处理内部消息所耗时间
        if (dwTransTick > g_dwMaxTransTick)
        {
            g_dwMaxTransTick = dwTransTick;
        }
#else // USE_EPOLL
		//设置
		FD_ZERO( &m_fdSockSet );

#ifdef WIN32
		FD_SET2(m_sktInterServ, &m_fdSockSet);
#else // WIN32
		FD_SET(m_sktInterServ, &m_fdSockSet);
#endif // WIN32

		for( wGrpIdx=0; wGrpIdx< g_dwMaxRcvGrp ; wGrpIdx++)
		{
			if( (g_ptRcvGrpPool+wGrpIdx)->bUsing )
			{
#ifdef WIN32
				FD_SET2( (unsigned)(g_ptRcvGrpPool+wGrpIdx)->cSock.GetSock(), &m_fdSockSet);
#else // WIN32
				FD_SET( (unsigned)(g_ptRcvGrpPool+wGrpIdx)->cSock.GetSock(), &m_fdSockSet);
#endif // WIN32
			}
		}
		int32_t ret_num = select(FD_SETSIZE, &m_fdSockSet, NULL, NULL, NULL);
		if( ret_num == 0 )
		{
			dslog(DS_LOG_ERROR, "CSockSwitch::DataSwitchTaskProc select TimeOut.ErrorNum (%d)\n" , getSockErrno() );
            CBB::cbb_task_delay(10);
			continue;
		}
		else if( ret_num == SOCKET_ERROR )
		{
			dslog(DS_LOG_ERROR, "CSockSwitch::DataSwitchTaskProc select Failed.ErrorNum (%d)\n" , getSockErrno() );
            CBB::cbb_task_delay(10);
			continue;
		}

		//消息来自内部接口
		if(FD_ISSET( m_sktInterServ, &m_fdSockSet))
		{
			bRun = ProssInterMsg();
		}

#ifdef WIN32 //在windows下直接使用fdset中的socket，而不使用遍历方式
		for (int32_t i = 0; i < ret_num; i++)
		{
			if (m_fdSockSet.fd_array[i] == m_sktInterServ)
			{
				continue;
			}

			TRcvGrp* ptRcv = GetRcvNode(m_fdSockSet.fd_array[i]);

			if (NULL != ptRcv)
			{
				//while( sockBytesAvailable( ptRcv->cSock.GetSock() ) > 0 )
                TransData(ptRcv,(char*) buf,sizeof(buf));
			}
		}
#else // WIN32
		for( wGrpIdx=0; wGrpIdx< g_dwMaxRcvGrp ; wGrpIdx++)
		{
			if( (g_ptRcvGrpPool+wGrpIdx)->bUsing )
			{
				if(FD_ISSET( (g_ptRcvGrpPool+wGrpIdx)->cSock.GetSock(),&m_fdSockSet))
				{
					//while( sockBytesAvailable( (g_ptRcvGrpPool+wGrpIdx)->cSock.GetSock() ) > 0 )
					{
						TransData( (g_ptRcvGrpPool+wGrpIdx),(char*) buf,sizeof(buf));
					}
				}
			}
		}
#endif // WIN32

#endif // USE_EPOLL

#else // _USE_WHILE_
		if ( bDelay )
        {
            dslog(DS_LOG_DEBUG, "DataSwitchTask sleep 10 ms\n");
            CBB::cbb_task_delay(10);
        }

		bDelay = TRUE ;
		g_dwDsStep = 50;
		for( wGrpIdx=0; wGrpIdx< g_dwMaxRcvGrp ; wGrpIdx++)
		{
			if( (g_ptRcvGrpPool+wGrpIdx)->bUsing == FALSE )
			{
				continue;
			}

			int32_t count = 0;
			while(1)
			{

				g_dwDsStep = 51;
				if (TransData( (g_ptRcvGrpPool+wGrpIdx),(char*) buf,sizeof(buf)) == FALSE)
				{
                    dslog(DS_LOG_DEBUG, "TransData fail at count %d\n", count);
					break;
				}
				g_dwDsStep = 52;
				count++;
				if (count > 20)
				{
					break;
				}
			}

			if ( count > 20 )
            {
				bDelay = FALSE;
                dslog(DS_LOG_DEBUG, "DataSwitchTask busy count %d\n", count);
            }

			g_dwDsStep = 53;
		}

		g_dwDsStep = 0;
		bRun = ProssInterMsg();

		g_dwDsStep = 5;
#endif // _USE_WHILE_
	}

	dslog(DS_LOG_ALL,  "\nDataswitch thread exit!\n");
	return NULL;
}

bool CSockSwitch::TransData( TRcvGrp* ptRcvGrp, char* bufOrg, int buflen )
{
	int SndLen;
	int nRcvDataLen;
	static int errcount =0;
	unsigned long dwSrcIP;
	unsigned short wSrcPort;
	uint32_t dwRecvIP;
	uint16_t wRecvPort;
	uint32_t dwMappedIP;
	uint16_t wMappedPort;
	bool IsMapped = false;
	TSndMmb* ptMmb = NULL;

    //20130124 huangzhichun:用于TransData中过滤出需要分发的Mnb列表,这两个数组有点冗余，后续可以考虑合并。
    //m_aptMmbSameSrc与dwMmbNumDefaultSrc是临时数组，每次都要初始化
	memset(m_aptMmbSameSrc, 0, g_dwMaxSndMmbPerRcvGrp* sizeof(TSndMmb*));
	uint32_t dwMmbNumSameSrc = 0;
	memset(m_aptMmbDefaultSrc, 0, g_dwMaxSndMmbPerRcvGrp*sizeof(TSndMmb*));
	uint32_t dwMmbNumDefaultSrc = 0;

	uint32_t dwIndex = 0;

	char* buf = bufOrg + DS_MAX_USER_LEN;        //prebuff len 32 byte

	TRtpInfo tRtpInfo;

	if( ptRcvGrp == NULL)
	{

		return false;
	}
	ptMmb = ptRcvGrp->ptSndMmb;

	g_dwDsStep = 6;
	nRcvDataLen = ptRcvGrp->cSock.RecvFrom( buf ,buflen ,dwSrcIP ,wSrcPort );
	if (nRcvDataLen < 0)
	{

		return false;
	}
	g_dwDsStep = 7;
	if ( g_bDsJustRecv )
	{
		return false;
	}

	if( (nRcvDataLen>0) && (nRcvDataLen <= buflen) )
	{
        bool bTrace;

        if (ptRcvGrp->dwTraceNum > 0)
        {
            ptRcvGrp->dwTraceNum--;
            bTrace = true;
        }
        else
        {
            bTrace = false;
        }

		g_swRcvBytesCount += nRcvDataLen;
		dwRecvIP = ptRcvGrp->cSock.GetIP();
		wRecvPort = ptRcvGrp->cSock.GetPort();


        if (g_wTraceRcvPort != 0 || g_wTraceSndPort != 0)
        {
		    GetRtpInfo(buf, nRcvDataLen, &tRtpInfo);
        }

        if (g_dwTraceRcvIp == dwSrcIP && g_wTraceRcvPort == wSrcPort)
        {
			dslog(DS_LOG_ALL, "recvfrom, seq:%d, srcIp:%X, srcPort:%u, recvLen:%d\n", tRtpInfo.m_RtpSeq, dwSrcIP, wSrcPort, nRcvDataLen);
        }

#ifdef _DS_CHECK_LOST_
		if (CheckPackLost(dwSrcIP, wSrcPort, tRtpInfo))
		{
			dslog(DS_LOG_ALL, "recv lost, current seq:%d, srcIp:%X, srcPort:%u\n", tRtpInfo.m_RtpSeq, dwSrcIP, wSrcPort);
		}
#endif // _DS_CHECK_LOST_

        if (bTrace)
        {
            OspPrintf(TRUE, FALSE, "[DsTrace][1] Recv:%x:%d Src:%x:%d Len:%d\n",
                     dwRecvIP, wRecvPort, dwSrcIP, wSrcPort, nRcvDataLen);
        }

        // 执行过滤函数
        if (ptRcvGrp->pfFilter != NULL)
        {
			// 若过滤函数返回非零，则按DUMP返回
			if( 0 != ( *(ptRcvGrp->pfFilter) )( ptRcvGrp->cSock.GetIP(),
                ptRcvGrp->cSock.GetPort(), dwSrcIP, wSrcPort,
                (uint8_t *)buf, (uint32_t)nRcvDataLen) )
			{
                if (bTrace)
                {
                    OspPrintf(TRUE, FALSE, "[DsTrace][2] Filter drop it\n");
                }
                return true;
			}
        }

		//查找源匹配的规则和默认规则
		dwMmbNumSameSrc = 0;
		dwMmbNumDefaultSrc = 0;

        TSndMmb* ptHashList = GetHashList(dwRecvIP, wRecvPort, dwSrcIP, wSrcPort);

        TSndMmb* ptLoop = ptHashList;

        while(NULL != ptLoop)
        {
            if (dwSrcIP == ptLoop->dwSrcIP &&
                wSrcPort == ptLoop->wSrcPort &&
                ptRcvGrp == ptLoop->ptRcv)
            {
                m_aptMmbSameSrc[dwMmbNumSameSrc] = ptLoop;
                dwMmbNumSameSrc++;
                if( dwMmbNumSameSrc >= g_dwMaxSndMmbPerRcvGrp )
                {
                    break;
                }
            }

            ptLoop = ptLoop->ptHashNext;
        }

        if (0 == dwMmbNumSameSrc)
        {
            ptHashList = GetHashList(dwRecvIP, wRecvPort, 0, 0);
            ptLoop = ptHashList;
            while(NULL != ptLoop)
            {
                if (ptRcvGrp == ptLoop->ptRcv)
                {
                    m_aptMmbDefaultSrc[dwMmbNumDefaultSrc] = ptLoop;
                    dwMmbNumDefaultSrc++;
                    if( dwMmbNumDefaultSrc >= g_dwMaxSndMmbPerRcvGrp )
                    {
                        break;
                    }
                }

                ptLoop = ptLoop->ptHashNext;
            }
        }

		if( ( 0 == dwMmbNumSameSrc ) &&
			( 0 == dwMmbNumDefaultSrc ) )
		{
            if (bTrace)
            {
                OspPrintf(TRUE, FALSE, "[DsTrace][3] no send found\n");
            }
            return true;
		}
		g_dwDsStep = 8;

		if( NULL == ptRcvGrp->pfCallback )
		{
			g_dwDsStep = 9;
			//优先源匹配的转发规则
			if( 0 != dwMmbNumSameSrc )
			{
				for( dwIndex = 0 ; dwIndex < dwMmbNumSameSrc ; dwIndex++ )
				{
                    //Add User Data
                    char* pSendBuff = buf;
					int32_t nSendLen = nRcvDataLen;

                    if (m_aptMmbSameSrc[dwIndex]->nUserDataLen > 0 &&
                        m_aptMmbSameSrc[dwIndex]->nUserDataLen <= DS_MAX_USER_LEN)
                    {
                        memcpy(buf-m_aptMmbSameSrc[dwIndex]->nUserDataLen,
                               m_aptMmbSameSrc[dwIndex]->achUserData,
                               m_aptMmbSameSrc[dwIndex]->nUserDataLen);

                        nSendLen += m_aptMmbSameSrc[dwIndex]->nUserDataLen;
                        pSendBuff -= m_aptMmbSameSrc[dwIndex]->nUserDataLen;
                    }

					IsMapped = false;

					if ( m_aptMmbSameSrc[dwIndex]->dwRawIp )
					{
						dwMappedIP = m_aptMmbSameSrc[dwIndex]->dwRawIp;
						wMappedPort = m_aptMmbSameSrc[dwIndex]->wRawPort;
						IsMapped = true;
					}
					else
					{
						dwMappedIP = ptRcvGrp->cSock.GetMapIP();
						wMappedPort = ptRcvGrp->cSock.GetMapPort();
						// 如果当前接收地址被映射到另一个地址，则用发送时，不再使用接收地址，
						// 而是使用映射后的地址
						if( ( dwMappedIP > 0 ) && ( wMappedPort > 0 ) )
						{
							IsMapped = true;
						}
					}

                    if (!IsMapped)
					{
						#ifdef _LINUX_
							if (m_aptMmbSameSrc[dwIndex]->dwDstIP == g_localipnum)
							{
                                if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
                                {
    								dslog(DS_LOG_ALL, "Linux DomainSocketSend 1, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
                                }
								SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
							}
							else
							{
                                if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
                                {
    								dslog(DS_LOG_ALL, "Linux UdpSendTo 1, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
                                }
								SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
							}
						#else
                            if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
                            {
    							dslog(DS_LOG_ALL, "non Linux UdpSendTo 1, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
                            }
							SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
						#endif
					}
					else
					{
						#ifdef _LINUX_
							if (m_aptMmbSameSrc[dwIndex]->dwDstIP == g_localipnum)
							{
                                if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
                                {
    								dslog(DS_LOG_ALL, "Linux DomainSocketSend 2, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
                                }
								SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
							}
							else
							{
								if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
								{
									dslog(DS_LOG_ALL, "Linux RawSockSend 2, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
								}
								SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
							}
						#else
                            if (g_dwTraceSndIp == m_aptMmbSameSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbSameSrc[dwIndex]->wDstPort)
                            {
    							dslog(DS_LOG_ALL, "non Linux RawSockSend 2, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort, nSendLen);
                            }
							SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , m_aptMmbSameSrc[dwIndex]->dwDstIP, m_aptMmbSameSrc[dwIndex]->wDstPort);
						#endif
					}

					if( SndLen >= 0 )
					{
						m_aptMmbSameSrc[dwIndex]->dwSndPkt++;
						g_swSndBytesCount += nSendLen;
					}
				}

                if (bTrace)
                {
                    OspPrintf(TRUE, FALSE, "[DsTrace][4] send src trans %d\n", dwMmbNumSameSrc);
                }
			}
			//无源匹配的转发规则，按默认规则转发
			else if( 0 != dwMmbNumDefaultSrc )
			{
				for( dwIndex = 0 ; dwIndex < dwMmbNumDefaultSrc ; dwIndex++ )
				{
                    //Add User Data
                    char *pSendBuff = buf;
                    int32_t nSendLen = nRcvDataLen;
                    if (m_aptMmbDefaultSrc[dwIndex]->nUserDataLen > 0 &&
                        m_aptMmbDefaultSrc[dwIndex]->nUserDataLen <= DS_MAX_USER_LEN)
                    {
                        memcpy(buf-m_aptMmbDefaultSrc[dwIndex]->nUserDataLen,
                            m_aptMmbDefaultSrc[dwIndex]->achUserData,
                            m_aptMmbDefaultSrc[dwIndex]->nUserDataLen);

                        nSendLen += m_aptMmbDefaultSrc[dwIndex]->nUserDataLen;
                        pSendBuff -= m_aptMmbDefaultSrc[dwIndex]->nUserDataLen;
                    }

					IsMapped = false;
					if ( m_aptMmbDefaultSrc[dwIndex]->dwRawIp )
					{
						dwMappedIP = m_aptMmbDefaultSrc[dwIndex]->dwRawIp;
						wMappedPort = m_aptMmbDefaultSrc[dwIndex]->wRawPort;
						IsMapped = true;
					}
					else
					{
						dwMappedIP = ptRcvGrp->cSock.GetMapIP();
						wMappedPort = ptRcvGrp->cSock.GetMapPort();
						// 如果当前接收地址被映射到另一个地址，则用发送时，不再使用接收地址，
						// 而是使用映射后的地址
						if( ( dwMappedIP > 0 ) && ( wMappedPort > 0 ) )
						{
							IsMapped = true;
						}
					}

					if (!IsMapped)
					{
						#ifdef _LINUX_
							if( m_aptMmbDefaultSrc[dwIndex]->dwDstIP == g_localipnum)
							{
                                if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
								{
    								dslog(DS_LOG_ALL, "Linux DomainSocketSend 3, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
								}
								SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort);
							}
							else
							{
                                if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
								{
    								dslog(DS_LOG_ALL, "Linux UdpSendTo 3, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
								}
								SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort );
							}
						#else
                            if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
							{
    							dslog(DS_LOG_ALL, "non Linux UdpSendTo 3, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
							}
							SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort );
						#endif
					}
					else
					{
						#ifdef _LINUX_
							if( m_aptMmbDefaultSrc[dwIndex]->dwDstIP == g_localipnum)
							{
                                if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
								{
    								dslog(DS_LOG_ALL, "Linux DomainSocketSend 4, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
								}
								SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort);
							}
							else
							{
                                if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
								{
									dslog(DS_LOG_ALL, "Linux RawSockSend 4, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
								}
								SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort);
							}
						#else
                            if (g_dwTraceSndIp == m_aptMmbDefaultSrc[dwIndex]->dwDstIP && g_wTraceSndPort == m_aptMmbDefaultSrc[dwIndex]->wDstPort)
							{
    							dslog(DS_LOG_ALL, "non Linux RawSockSend 4, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort, nSendLen);
							}
							SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , m_aptMmbDefaultSrc[dwIndex]->dwDstIP, m_aptMmbDefaultSrc[dwIndex]->wDstPort);
						#endif
					}

					if( SndLen >= 0 )
					{
						m_aptMmbDefaultSrc[dwIndex]->dwSndPkt++;
						g_swSndBytesCount += nSendLen;
					}
				}

                if (bTrace)
                {
                    OspPrintf(TRUE, FALSE, "[DsTrace][5] send port trans %d\n", dwMmbNumDefaultSrc);
                }
			}

			g_dwDsStep = 10;
		}
		else
		{
			g_dwDsStep = 11;
			//如果有发送回调，上层可能修改数据
			uint16_t wSndMemberNum = 0;
			char achReserveBuf[SENDMEM_MAX_MODLEN];

			//优先源匹配的转发规则
			if( 0 != dwMmbNumSameSrc )
			{
				for( dwIndex = 0 ; dwIndex < dwMmbNumSameSrc ; dwIndex++ )
				{
                    memset( (g_ptNetSndMembers+wSndMemberNum) , 0 , sizeof(TNetSndMember) );
					(g_ptNetSndMembers+wSndMemberNum)->tDstAddr.dwIP = m_aptMmbSameSrc[dwIndex]->dwDstIP;
					(g_ptNetSndMembers+wSndMemberNum)->tDstAddr.wPort = m_aptMmbSameSrc[dwIndex]->wDstPort;
					(g_ptNetSndMembers+wSndMemberNum)->lpuser = (void*)(m_aptMmbSameSrc[dwIndex]);
					(g_ptNetSndMembers+wSndMemberNum)->pAppData = m_aptMmbSameSrc[dwIndex]->pAppData;
					wSndMemberNum++;
				}

                if (bTrace)
                {
                    OspPrintf(TRUE, FALSE, "[DsTrace][6] send src trans %d\n", dwMmbNumSameSrc);
                }
			}
			//无源匹配的转发规则，按默认规则转发
			else if( 0 != dwMmbNumDefaultSrc )
			{
				for( dwIndex = 0 ; dwIndex < dwMmbNumDefaultSrc ; dwIndex++ )
				{
					memset( (g_ptNetSndMembers+wSndMemberNum) , 0 , sizeof(TNetSndMember) );
					(g_ptNetSndMembers+wSndMemberNum)->tDstAddr.dwIP = m_aptMmbDefaultSrc[dwIndex]->dwDstIP;
					(g_ptNetSndMembers+wSndMemberNum)->tDstAddr.wPort = m_aptMmbDefaultSrc[dwIndex]->wDstPort;
					(g_ptNetSndMembers+wSndMemberNum)->lpuser = (void*)(m_aptMmbDefaultSrc[dwIndex]);
					(g_ptNetSndMembers+wSndMemberNum)->pAppData = m_aptMmbDefaultSrc[dwIndex]->pAppData;
                    wSndMemberNum++;
				}

                if (bTrace)
                {
                    OspPrintf(TRUE, FALSE, "[DsTrace][7] send port trans %d\n", dwMmbNumDefaultSrc);
                }
			}
			g_dwDsStep = 12;

			(*(ptRcvGrp->pfCallback))( ptRcvGrp->cSock.GetIP() , ptRcvGrp->cSock.GetPort() ,
				dwSrcIP , ntohs(wSrcPort) , g_ptNetSndMembers , &wSndMemberNum , (uint8_t *)buf , (uint32_t)nRcvDataLen );

			//保留原始数据（最多SENDMEM_MAX_MODLEN字节长）
			uint16_t wReserveLen = ( nRcvDataLen < SENDMEM_MAX_MODLEN ) ? nRcvDataLen : SENDMEM_MAX_MODLEN;
			memcpy( achReserveBuf , buf , wReserveLen );

			for( uint16_t wIndex = 0 ; wIndex < wSndMemberNum ; wIndex++ )
			{
                if( (g_ptNetSndMembers+wIndex)->wLen > wReserveLen )
				{
					continue;
				}
				memcpy( buf , achReserveBuf , wReserveLen );
				memcpy( buf , (g_ptNetSndMembers+wIndex)->pNewData , (g_ptNetSndMembers+wIndex)->wLen );

				ptMmb = ( TSndMmb* )((g_ptNetSndMembers+wIndex)->lpuser);

                //Add User Data
                char *pSendBuff = buf;
                int32_t nSendLen = nRcvDataLen;

                if (ptMmb->nUserDataLen > 0 &&
                    ptMmb->nUserDataLen <= DS_MAX_USER_LEN)
                {
                    memcpy(buf - ptMmb->nUserDataLen, ptMmb->achUserData, ptMmb->nUserDataLen);

                    nSendLen += ptMmb->nUserDataLen;
                    pSendBuff -= ptMmb->nUserDataLen;
                }
				IsMapped = false;
				if ( ptMmb->dwRawIp )
				{
					dwMappedIP = ptMmb->dwRawIp;
					wMappedPort = ptMmb->wRawPort;
					IsMapped = true;
				}
				else
				{
					dwMappedIP = ptRcvGrp->cSock.GetMapIP();
					wMappedPort = ptRcvGrp->cSock.GetMapPort();
					// 如果当前接收地址被映射到另一个地址，则用发送时，不再使用接收地址，
					// 而是使用映射后的地址
					if( ( dwMappedIP > 0 ) && ( wMappedPort > 0 ) )
					{
						IsMapped = true;
					}
				}

				if (!IsMapped)
				{
					#ifdef _LINUX_
						if( ptMmb->dwDstIP == g_localipnum)
						{
                            if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
							{
								dslog(DS_LOG_ALL, "Linux DomainSocketSend 5, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
							}
							SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, ptMmb->dwDstIP, ptMmb->wDstPort);
						}
						else
						{
                            if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
							{
   								dslog(DS_LOG_ALL, "Linux UdpSendTo 5, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
							}
							SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, ptMmb->dwDstIP, ptMmb->wDstPort );
						}
					#else
                        if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
						{
   							dslog(DS_LOG_ALL, "non Linux UdpSendTo 5, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
						}
						SndLen = ptRcvGrp->cSock.SendTo(pSendBuff, nSendLen, ptMmb->dwDstIP, ptMmb->wDstPort);
					#endif
				}
				else
				{
					#ifdef _LINUX_
						if( ptMmb->dwDstIP == g_localipnum)
						{
                            if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
							{
								dslog(DS_LOG_ALL, "Linux DomainSocketSend 6, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
							}
							SndLen = DomainSocketSend(pSendBuff, nSendLen, 0, 0, ptMmb->dwDstIP, ptMmb->wDstPort);
						}
						else
						{
                            if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
							{
								dslog(DS_LOG_ALL, "Linux RawSockSend 6, seq:%d, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
							}
							SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , ptMmb->dwDstIP, ptMmb->wDstPort);
						}
					#else
                        if (g_dwTraceSndIp == ptMmb->dwDstIP && g_wTraceSndPort == ptMmb->wDstPort)
						{
							dslog(DS_LOG_ALL, "non Linux RawSockSend 6, dstIp:%X, dstPort:%u, sendLen:%d\n", tRtpInfo.m_RtpSeq, ptMmb->dwDstIP, ptMmb->wDstPort, nSendLen);
						}
						SndLen = RawSockSend(pSendBuff, nSendLen, dwMappedIP , wMappedPort , ptMmb->dwDstIP, ptMmb->wDstPort);
					#endif
				}

				if( SndLen < 0 )
				{
					errcount ++;
					if(errcount == 100)
					{
						dslog(DS_LOG_DEBUG, " err = %d In send to %s@%d failed!\n",getSockErrno(), ipString(ptMmb->dwDstIP), ptMmb->wDstPort );
						errcount = 0;
					}
				}
				else
				{
					ptMmb->dwSndPkt++;
					g_swSndBytesCount += nSendLen;
				}
			}

			g_dwDsStep = 13;
            if (bTrace)
            {
                OspPrintf(TRUE, FALSE, "[DsTrace][8] real send trans %d\n", wSndMemberNum);
            }
		}
		return true;
	}

	return true;
}

/*=============================================================================
函 数 名： CalcChecksum
功    能： 校验和计算, 在外壳调用前保证为nSize偶数
算法实现：
全局变量：
参    数： uint16_t *wBuffer
int32_t nSize    偶数
返 回 值： uint16_t
-----------------------------------------------------------------------------
修改记录：
日  期		版本		修改人		走读人    修改内容
2005/1/13   3.5			万春雷                  创建
=============================================================================*/
uint16_t CSockSwitch::CalcChecksum(uint16_t *wBuffer, int32_t nSize)
{
	unsigned long dwCkSum = 0;

	while (nSize > 1)
	{
		dwCkSum += *wBuffer++;
		nSize  -= sizeof(uint16_t);
	}
	if(nSize)
	{
		dwCkSum += *(uint8_t *)wBuffer;
	}

	dwCkSum  = (dwCkSum >> 16) + (dwCkSum & 0xffff);
	dwCkSum += (dwCkSum >> 16);

	return (uint16_t)(~dwCkSum);
}

#ifdef _LINUX_
int	CSockSwitch::DomainSocketSend(char* buf,int buflen ,
		uint32_t dwSrcIP ,uint16_t wSrcPort , uint32_t dwDstIP , uint16_t wDstPort)
{
	if( buf == NULL || buflen <= 0 || dwDstIP != g_localipnum)
	{
		return 0;
	}
	char achSocketPath[UNIX_PATH_MAX] = {0};
	memset(achSocketPath, 0, UNIX_PATH_MAX);
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sun_family = AF_LOCAL;
	sprintf(addr.sun_path, "/opt/kdm/data/unixsocket/%u-%d", dwDstIP, wDstPort);
	int nSndLen = sendto( m_hDomainSocket, (char *)buf, buflen, 0, (struct sockaddr *)&addr, sizeof(addr) );
	if ( nSndLen < 0 )
	{
		dslog(DS_LOG_ERROR, "domain socket send fail : %d(%s)\n", errno, strerror(errno));
	}
	else if (nSndLen < buflen)
	{
		dslog(DS_LOG_SEND, "domain socket send less : %d/%d\n", nSndLen, buflen);
	}

	return nSndLen;
}
#endif


int CSockSwitch::RawSockSend( char* pBuf,int nSize ,
				uint32_t dwSrcIP ,uint16_t wSrcPort , uint32_t dwDstIP , uint16_t wDstPort )
{
	int32_t nIPVersion = 4;	//IPV4
	int32_t nIPSize    = sizeof(m_tIPHdr) / sizeof(uint32_t);
	int32_t nUDPSize   = sizeof(m_tUDPHdr) + nSize;
	int32_t nTotalSize = sizeof(m_tIPHdr) + sizeof(m_tUDPHdr) + nSize;

	// 初始化IP头
	m_tIPHdr.byIPVerLen = (nIPVersion << 4) | nIPSize;
	m_tIPHdr.byIPTos  = 0;
	m_tIPHdr.wIPTotalLen  = htons(nTotalSize);		// Total packet len
	m_tIPHdr.wIPID        = 0;						// Unique identifier: set to 0
	m_tIPHdr.wIPOffset    = 0;						// Fragment offset field
	m_tIPHdr.byIPTtl      = 128;					// Time to live
	m_tIPHdr.byIPProtocol = 0x11;					// Protocol(UDP) TCP-6;UDP-17
	m_tIPHdr.wIPCheckSum  = 0;						// IP checksum
	m_tIPHdr.dwSrcAddr = dwSrcIP;					// Source address
	m_tIPHdr.dwDstAddr    = dwDstIP;				// Destination address

	// 初始化UDP头
	m_tUDPHdr.wSrcPort    = htons(wSrcPort) ;
	m_tUDPHdr.wDstPort    = htons(wDstPort) ;
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

	char *pszRawBuf = (char *)&m_szRawPackBuf;
	TDSUDPPsdHdr   tUDPPsdHdr;
	memset(&tUDPPsdHdr, 0 , sizeof(tUDPPsdHdr));
	tUDPPsdHdr.dwSrcAddr    = m_tIPHdr.dwSrcAddr;
	tUDPPsdHdr.dwDstAddr    = m_tIPHdr.dwDstAddr;
	tUDPPsdHdr.byIPProtocol = m_tIPHdr.byIPProtocol;
	tUDPPsdHdr.wUDPLen      = m_tUDPHdr.wUDPLen;

	pszRawBuf = (char *)&m_szRawPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
	int32_t nUdpChecksumSize = 0;

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

	pszRawBuf = (char *)&m_szRawPackBuf + sizeof(m_tIPHdr) - sizeof(tUDPPsdHdr);
	uint16_t wCkSum = CalcChecksum((uint16_t *)pszRawBuf, nUdpChecksumSize);

	m_tUDPHdr.wUDPCheckSum = wCkSum;

	//构造完整的 Raw Socket 数据包并进行发送
	pszRawBuf = (char *)&m_szRawPackBuf;
	memcpy(pszRawBuf, &m_tIPHdr, sizeof(m_tIPHdr));
	pszRawBuf += sizeof(m_tIPHdr);
	memcpy(pszRawBuf, &m_tUDPHdr, sizeof(m_tUDPHdr));

	memset( &m_tAddrIn, 0, sizeof(m_tAddrIn));
	m_tAddrIn.sin_family	  = AF_INET;
	m_tAddrIn.sin_addr.s_addr = dwDstIP;
	m_tAddrIn.sin_port		  = htons(wDstPort);

	int nSndLen = sendto( m_sktRawSend, (char *)&m_szRawPackBuf, nTotalSize, 0,
		(SOCKADDR*)&m_tAddrIn, sizeof(SOCKADDR_IN) );
	if ( nSndLen < 0 )
	{
		dslog(DS_LOG_ERROR, "raw socket send fail : %d(%s)\n", errno, strerror(errno));
	}
	else if (nSndLen < nTotalSize)
	{
		dslog(DS_LOG_SEND, "raw socket send less : %d/%d\n", nSndLen, nTotalSize);
	}

    return nSndLen;
}

void CSockSwitch::Destroy(DSID dsId)
{
	RemoveAll( dsId );
	dsFreeDSID(dsId);
}


bool CSockSwitch::GetCmdData(uint8_t * pBuf)
{
	uint8_t buf[1024];
	uint32_t len = 0;
	uint32_t bytes = 0;
	bool bGetHead = false;
	TCmdHead * ptCmdHead;

	while ( bytes < sizeof(TDSCommand) )
	{
		len = recv_tcp_data(m_sktInterServ, buf, 1, 10 );
		if ( 1 == len )
		{
			bytes ++ ;

			if ( buf[0] != DS_CMD_MAGIC1 )
			{
				dslog(DS_LOG_ERROR, "in getcmd, buf[0] != ds magic1\n");
				continue ;
			}
			else
			{
				len = recv_tcp_data(m_sktInterServ, buf, 3, g_nDsApiTimeOut );
				if ( 3 == len )
				{
					bytes += 3 ;

					if ( DS_CMD_MAGIC2 != buf[0]
						|| DS_CMD_MAGIC3 != buf[1]
						|| DS_CMD_MAGIC4 != buf[2] )

					{
						dslog(DS_LOG_ERROR, "in getcmd, next magic error\n");
						continue;
					}
					else
					{
						bGetHead = true;
						break;
					}
				}
				else
				{
					dslog(DS_LOG_ERROR, "in getcmd, recv next 3 magic timeout:%d\n", g_nDsApiTimeOut);
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	if ( !bGetHead )
		return false;

	pBuf[0] = DS_CMD_MAGIC1;
	pBuf[1] = DS_CMD_MAGIC2;
	pBuf[2] = DS_CMD_MAGIC3;
	pBuf[3] = DS_CMD_MAGIC4;

	len = recv_tcp_data(m_sktInterServ, pBuf +4, sizeof(TCmdHead) -4, g_nDsApiTimeOut );
	if ( len != sizeof(TCmdHead) -4 )
	{
		dslog(DS_LOG_ERROR, "in getcmd, recv cmd head timeout:%d\n", g_nDsApiTimeOut);
		return false;
	}

	ptCmdHead = (TCmdHead * )pBuf;

	len = recv_tcp_data(m_sktInterServ, pBuf + sizeof(TCmdHead), ptCmdHead->m_dwMsgLen - sizeof(TCmdHead), g_nDsApiTimeOut);
	if ( len != ptCmdHead->m_dwMsgLen - sizeof(TCmdHead))
	{
		dslog(DS_LOG_ERROR, "in getcmd, recv cmd len[%d] timeout:%d\n", ptCmdHead->m_dwMsgLen - sizeof(TCmdHead),g_nDsApiTimeOut);
		return false;
	}

	/// uint32_t dwNow = OspTickGet() * ( 1000 / OspClkRateGet());
	uint32_t dwNow = CBB::cbb_tick_get() * ( 1000 / CBB::cbb_clk_rate_get());

	if ( dwNow - ptCmdHead->m_time > (uint32_t)g_nDsApiTimeOut )
	{
		dslog(DS_LOG_ERROR, "in getcmd, cmd timeout: %d - %d > %d\n", dwNow, ptCmdHead->m_time, g_nDsApiTimeOut);
		return false;
	}
	return true;
}

bool CSockSwitch::ProssInterMsg()
{
	uint32_t dwRet = DSERROR;
	TRcvGrp *ptRcvGrp = NULL;
	uint8_t buf[1024] = {0};
	TCmdHead * ptHead = NULL;
	TDSCommand *ptCmd = NULL;
	TDSCommand_AttchedData *ptAttacheData = NULL;
	bool ifAttachedData;

	g_dwDsStep = 1;

	if ( !GetCmdData(buf))
		return true ;

	ptHead = (TCmdHead * )buf;
	ptCmd = (TDSCommand*)(buf + sizeof(TCmdHead));
	if ( ptHead->m_dwMsgLen == sizeof(TCmdHead) + sizeof(TDSCommand))
	{
		ifAttachedData = false ;
		ptAttacheData = NULL ;
	}
	else
	{
		ptAttacheData = (TDSCommand_AttchedData*)(buf + sizeof(TCmdHead) + sizeof(TDSCommand));
		ifAttachedData = true ;
	}

	g_dwDsStep = 2;

	if (buf[0] == 1)
	{
		ifAttachedData = true;
	}

	switch( ptCmd->dwCmd )
	{
	case DSCMD_ADD:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_ADD\n" );
			RemoveAllMmb( ptCmd->dsId,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort);

			dwRet = Add(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					 ptCmd->dwLocalIfIP,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort,
					 ptCmd->dwDstIfIP);
		}
		break;

	case DSCMD_REMOVE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_REMOVE\n" );
			dwRet = RemoveAllMmb( ptCmd->dsId,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort);
		}
		break;

	case DSCMD_ADD_M2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_ADD_M2ONE\n" );
			dwRet = Add(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					 ptCmd->dwLocalIfIP,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort,
					 ptCmd->dwDstIfIP);
		}
		break;

	case DSCMD_RM_M2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_M2ONE\n" );
			dwRet = Remove( ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort);
		}
		break;

	case DSCMD_RM_ALLM2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_ALLM2ONE\n" );
			dwRet = RemoveAllMmb( ptCmd->dsId,
					0,
					0,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort);
		}
		break;

	case DSCMD_ADD_SRC_M2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_ADD_SRC_M2ONE\n" );
			dwRet = Add(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					 ptCmd->dwLocalIfIP,
					 ptCmd->dwSrcIP,
					 ptCmd->wSrcPort,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort,
					 ptCmd->dwDstIfIP);
		}
		break;

	case DSCMD_RM_SRC_M2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_SRC_M2ONE\n" );
			dwRet = Remove(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					 ptCmd->dwSrcIP,
					 ptCmd->wSrcPort,
					 ptCmd->dwDstIP,
					 ptCmd->wDstPort);
		}
		break;

	case DSCMD_RM_SRC_ALLM2ONE:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_SRC_ALLM2ONE\n" );
			dwRet = RemoveAllMmb( ptCmd->dsId,
					 ptCmd->dwSrcIP,
					 ptCmd->wSrcPort,
					~0,
					~0);
		}
		break;

	case DSCMD_ADD_DUMP:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_ADD_DUMP\n" );
			dwRet = Add(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					 ptCmd->dwLocalIfIP,
					0,
					0,
					0,
					0,
					0);
		}
		break;

	case DSCMD_RM_DUMP:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_DUMP\n" );
			dwRet = Remove(  ptCmd->dsId,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					0,
					0,
					0,
					0);
		}
		break;

	case DSCMD_DESTORY:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_DESTORY\n" );
			Destroy(  ptCmd->dsId );
		}
		break;

	case DSCMD_ADD_SPY:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_ADD_SPY\n" );

			// 增加DUMP规则
			dwRet=Add( ptCmd->dsId ,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					  ptCmd->dwLocalIfIP,
					 0,
					 0,
					 0,
					 0,
					 0);

			// 设置函数指针
			ptRcvGrp = GetRcvGrp( ptCmd->dwLocalIP,  ptCmd->wLocalPort);
			if (ptRcvGrp != NULL)
			{
				// 只要能设置上函数指针就算成功
				ptRcvGrp->pfFilter = (FilterFunc)( ptCmd->llContext);
				dwRet = DSOK;
			}
			else
			{
				dwRet = DSERROR;
			}
		}
		break;

	case DSCMD_RM_SPY:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_SPY\n" );
			ptRcvGrp = GetRcvGrp( ptCmd->dwLocalIP,  ptCmd->wLocalPort);
			if (ptRcvGrp != NULL)
			{
				// 只要能清除函数指针就算成功
				ptRcvGrp->pfFilter = NULL;
				dwRet = DSOK;
			}
			else
			{
				dwRet = DSERROR;
			}
			Remove(  ptCmd->dsId ,
					 ptCmd->dwLocalIP,
					 ptCmd->wLocalPort,
					0,
					0,
					0,
					0);
		}
		break;

	case DSCMD_RM_ALL:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_ALL\n" );
			dwRet = RemoveAll(ptCmd->dsId );
		}
		break;

	case DSCMD_RM_ALL_EXCEPT_DUMP:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_RM_ALL_EXCEPT_DUMP\n");

			dwRet = RemoveAll(ptCmd->dsId, false);
		}
		break;

	case DSCMD_SET_CALLBACK:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_SET_CALLBACK\n" );
			ptRcvGrp = GetRcvGrp(ptCmd->dwLocalIP , ptCmd->wLocalPort );
			if( ptRcvGrp != NULL )
			{
				ptRcvGrp->pfCallback = (SendCallback)(ptCmd->llContext);
				dwRet = DSOK;
			}
			else
			{
				dwRet = DSERROR;
			}
		}
		break;

	case DSCMD_SET_APPDATA:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_SET_APPDATA\n" );
			ptRcvGrp = GetRcvGrp( ptCmd->dwLocalIP, ptCmd->wLocalPort );
			if( ptRcvGrp != NULL )
			{
				TSndMmb* ptSndMmb = GetSndMmb( ptRcvGrp , ptCmd->dwDstIP , ptCmd->wDstPort );
				if( ptSndMmb == NULL || ptSndMmb == (TSndMmb*)(-1))
				{
					dwRet = DSERROR;
					break;
				}
				ptSndMmb->pAppData = (void*)( ptCmd->llContext);
				dwRet = DSOK;
			}
			else
			{
				dwRet = DSERROR;
			}
		}
		break;

	case DSCMD_SET_MAP_ADDR:
		{
			dslog(DS_LOG_DEBUG,  "CSockSwitch::ProssInterMsg : DSCMD_SET_MAP_ADDR\n" );
			ptRcvGrp = GetRcvGrp( ptCmd->dwLocalIP , ptCmd->wLocalPort );
			if( ptRcvGrp != NULL )
			{
				ptRcvGrp->cSock.SetMapIP(ptCmd->dwSrcIP );
				ptRcvGrp->cSock.SetMapPort(ptCmd->wSrcPort );
				dwRet = DSOK;
			}
			else
			{
				dwRet = DSERROR;
			}
		}
		break;

	case DSCMD_SET_SRC_ADDR_BY_DST:
		{
			dwRet = DSOK;
			dslog(DS_LOG_DEBUG, "ds set src addr by dst\n");
			ptRcvGrp = GetRcvGrp(ptCmd->dwLocalIP, ptCmd->wLocalPort);
			if ( NULL != ptRcvGrp )
			{
				TSndMmb * ptSndMmb = GetSndMmb(ptRcvGrp, ptCmd->dwDstIP, ptCmd->wDstPort);
				if ( NULL != ptSndMmb )
				{
					ptSndMmb->dwRawIp = ptCmd->dwSrcIP;
					ptSndMmb->wRawPort = ptCmd->wSrcPort;
				}
				else
				{
					OspPrintf(1, 0, "ds can't find snm groud:%x:%d\n", ptCmd->dwDstIP, ptCmd->wDstPort);
					dwRet = DSERROR;
				}

			}
			else
			{
				OspPrintf(1, 0, "ds can't find rcv group %x:%d\n", ptCmd->dwLocalIP, ptCmd->wLocalPort);
				dwRet = DSERROR ;
			}
		}

		break;

	case DSCMD_SET_USERDATA:
		{
			if (ifAttachedData == false)
			{
				dslog(DS_LOG_DEBUG, "DSCMD_SET_USERDATA Error msg\n");
				break;
			}

			dwRet = SetUserData(ptCmd->dsId,
				ptCmd->dwLocalIP,
				ptCmd->wLocalPort,
				ptCmd->dwLocalIfIP,
				ptCmd->dwSrcIP,
				ptCmd->wSrcPort,
				ptCmd->dwDstIP,
				ptCmd->wDstPort,
				ptCmd->dwDstIfIP,
				ptAttacheData->uAttchedData.tUserData.bSend,
				(uint8_t *)ptAttacheData->uAttchedData.tUserData.achUserData,
				ptAttacheData->uAttchedData.tUserData.nUserDataLen);
		}
		break;

	default:
		break;
	}

	g_dwDsStep = 3;
	uint8_t ack[12] = {0};

	ack[0] = DS_CMD_MAGIC1;
	ack[1] = DS_CMD_MAGIC2;
	ack[2] = DS_CMD_MAGIC3;
	ack[3] = DS_CMD_MAGIC4;

	memcpy(ack+ 4, (uint8_t *)&ptHead->m_sn, sizeof(uint32_t));
	memcpy(ack+ 8, (uint8_t *)&dwRet, sizeof(uint32_t));
	uint32_t len = send_tcp_data(m_sktInterServ, ack, sizeof(ack), g_nDsApiTimeOut);
	if ( sizeof(ack) != len )
	{
		dslog(DS_LOG_DEBUG, "Send InterMessage Ack failed. Flag is: %d.\n",dwRet);
	}
	g_dwDsStep = 4;

	return ptCmd->dwCmd == DSCMD_DESTORY ? false :true;
}


uint32_t CSockSwitch::SetCmd(TDSCommand * ptCmd, TDSCommand_AttchedData * ptAttached)
{
    uint32_t dwCmdTickStart = CBB::cbb_tick_get();/// OspTickGet();

	uint8_t buf[1024] = {0};
	TCmdHead * ptHead = NULL;
	uint32_t sn;
	uint32_t len ;
	uint32_t ackSn;
	uint32_t ackRet = DSERROR;

	sn = g_dwCmdSn ;
	ptHead = (TCmdHead * )buf ;

	ptHead->m_Magic1 = DS_CMD_MAGIC1;
	ptHead->m_Magic2 = DS_CMD_MAGIC2;
	ptHead->m_Magic3 = DS_CMD_MAGIC3;
	ptHead->m_Magic4 = DS_CMD_MAGIC4;

	memcpy(buf + sizeof(TCmdHead), ptCmd, sizeof(TDSCommand));
	if ( ptAttached )
	{
		ptHead->m_dwMsgLen = sizeof(TCmdHead) + sizeof(TDSCommand) + sizeof(TDSCommand_AttchedData);
		memcpy(buf + sizeof(TCmdHead) + sizeof (TDSCommand), ptAttached, sizeof(TDSCommand_AttchedData));
	}
	else
	{
		ptHead->m_dwMsgLen = sizeof(TCmdHead) + sizeof(TDSCommand) ;
	}

	ptHead->m_sn = sn ;
	/// ptHead->m_time = OspTickGet() * (1000 / OspClkRateGet());
	ptHead->m_time = CBB::cbb_tick_get() * (1000 / CBB::cbb_clk_rate_get());

	len = send_tcp_data(m_sktInterClient, buf, ptHead->m_dwMsgLen, g_nDsApiTimeOut );
	if ( ptHead->m_dwMsgLen != len )
	{
		dslog(DS_LOG_ERROR, "in SetCmd, send tcp data timeout :%d\n", g_nDsApiTimeOut);
		return DSERROR ;
	}

	while ( 1)
	{
		uint32_t bytes = 0;
		bool bGetHead = false;

		while ( bytes < 8 )
		{
			len = recv_tcp_data(m_sktInterClient, buf, 1, g_nDsApiTimeOut );
			if ( 1 == len )
			{
				bytes ++ ;

				if ( buf[0] != DS_CMD_MAGIC1 )
				{
					dslog(DS_LOG_ERROR, "in setcmd , buf[0] != DS_CMD_MAGIC1\n");
					continue ;
				}
				else
				{
					len = recv_tcp_data(m_sktInterClient, buf, 3, g_nDsApiTimeOut );
					if ( 3 == len )
					{
						bytes += 3 ;

						if ( DS_CMD_MAGIC2 != buf[0]
							|| DS_CMD_MAGIC3 != buf[1]
							|| DS_CMD_MAGIC4 != buf[2] )
						{
							dslog(DS_LOG_ERROR, "in setcmd, next 3 magic error\n");
							continue ;
						}
						else
						{
							bGetHead = true;
							break;
						}
					}
					else
					{
						dslog(DS_LOG_ERROR, "in setcmd, recv last 3 magic timeout:%d\n", g_nDsApiTimeOut);
						goto ret;
					}
				}
			}
			else
			{
				dslog(DS_LOG_ERROR, "in setcmd, recv ds magic timeout:%d\n", g_nDsApiTimeOut);
				goto ret;
			}
		}

		if ( !bGetHead )
			goto ret;

		len = recv_tcp_data(m_sktInterClient, buf, 8, g_nDsApiTimeOut );
		if ( 8 != len )
		{
			dslog(DS_LOG_ERROR, "in setcmd, recv cmd ack timeout:%d\n", g_nDsApiTimeOut);
			goto ret;
		}

		memcpy((uint8_t *)&ackSn, buf, 4);
		memcpy((uint8_t *)&ackRet, buf+4, 4);

		if ( sn != ackSn )
		{
			dslog(DS_LOG_ERROR, "in setcmd, sn[%d] != acksn[%d]\n", sn, ackSn);
			ackRet = DSERROR ;
			continue ;
		}
		else
		{
			//如果sn一致，则需要将真实的返回结果告诉上层，否则会造成dataswitch认为交换建立失败，
			//但是因为这里将ackRet修改为成功，而使得上层认为交换建立成功。
			dslog(DS_LOG_DEBUG, "in setcmd, sn[%d] == acksn[%d], return %d\n", sn, ackSn, ackRet);
// 			ackRet = DSOK;
			break;
		}
	}

ret:

	if (ptCmd->dwCmd == DSCMD_DESTORY)
	{
		FreeMemory();
        CBB::cbb_sock_close(m_sktRawSend);
		m_sktRawSend = INVALID_SOCKET;
        CBB::cbb_sock_close(m_sktInterClient);
		m_sktInterClient = INVALID_SOCKET;
        CBB::cbb_sock_close(m_sktInterServ);
		m_sktInterServ = INVALID_SOCKET;
		m_bInit = false;
	}

    uint32_t dwCmdTick = CBB::cbb_tick_get() - dwCmdTickStart;
    if (dwCmdTick >= 1)
    {
        dslog(DS_LOG_DEBUG, "CSockSwitch::SetCmd cost %u tick(s).\n", dwCmdTick);
    }

    return ackRet;
}


uint32_t CSockSwitch::GetRecvPktCount( DSID dsId , uint32_t  dwLocalIP , uint16_t wLocalPort ,
		uint32_t dwSrcIP , uint16_t wSrcPort , uint32_t &dwRecvPktCount )
{
	//检测ID合法性
	if( !IsExistDSID(dsId) )
		return DSERROR;

	//检测接收组是否已经存在
	TRcvGrp* ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
	if( NULL == ptRcvGrp )
	{
		return DSERROR;
	}

	dwRecvPktCount = ptRcvGrp->cSock.GetRcvPkt();

	return DSOK;
}


uint32_t CSockSwitch::GetSendPktCount( DSID dsId , uint32_t  dwLocalIP , uint16_t wLocalPort ,
		uint32_t dwSrcIP , uint16_t wSrcPort ,
		uint32_t dwDstIP, uint16_t  wDstPort, uint32_t& dwSendPktCount )
{
	//检测ID合法性
	if( !IsExistDSID(dsId) )
		return DSERROR;

	//检测接收组是否已经存在
	TRcvGrp* ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
	if( NULL == ptRcvGrp )
	{
		return DSERROR;
	}

	//检测目的转发成员是否存在
	TSndMmb* ptSndMmb = GetSndMmb( ptRcvGrp ,dwDstIP ,wDstPort );
	if( NULL == ptSndMmb || ptSndMmb == (TSndMmb*)(-1))
	{
		return DSERROR;
	}

	dwSendPktCount = ptSndMmb->dwSndPkt;

	return DSOK;
}


void CSockSwitch::ShowInfo( DSID dsId )
{
	TSndMmb* ptMmb = NULL;
	bool     bGrpPrint = false;
	char     achSrc[100];
	uint32_t     dwRcvGrpNum = 0, dwItemsInMatrix = 0;

	OspPrintf(TRUE,FALSE,"\t==================================================\n");
	OspPrintf(TRUE,FALSE,"\t=                Data Switch %-4d                =\n",dsId);
	OspPrintf(TRUE,FALSE,"\t==================================================\n");

	for( uint32_t dwIdx =0 ;dwIdx < g_dwMaxRcvGrp ;dwIdx++ )
	{
		bGrpPrint = false;
		if( (g_ptRcvGrpPool+dwIdx)->bUsing )
		{
			if( IS_HANLDE(dsId ,(g_ptRcvGrpPool+dwIdx)->dwDump) || IS_HANLDE(dsId,(g_ptRcvGrpPool+dwIdx)->dwSpying) )
			{
				OspPrintf( TRUE ,FALSE ,"\n%s@%d(%s@%d) filter:%x %s%s (%d Packets)",
					ipString((g_ptRcvGrpPool+dwIdx)->cSock.GetIP()),
					(g_ptRcvGrpPool+dwIdx)->cSock.GetPort(),
					ipString((g_ptRcvGrpPool+dwIdx)->cSock.GetMapIP()),
					(g_ptRcvGrpPool+dwIdx)->cSock.GetMapPort(),
                    (g_ptRcvGrpPool+dwIdx)->pfFilter,
					IS_HANLDE(dsId,(g_ptRcvGrpPool+dwIdx)->dwDump) ? "[DUMP]":"",
					IS_HANLDE(dsId,(g_ptRcvGrpPool+dwIdx)->dwSpying) ? "[SPYING]":"",
					(g_ptRcvGrpPool+dwIdx)->cSock.GetRcvPkt());

                if ((g_ptRcvGrpPool+dwIdx)->nUserDataLen > 0)
                {
                    uint8_t *pbyData = (g_ptRcvGrpPool+dwIdx)->achUserData;
                    int32_t nDataLen = (g_ptRcvGrpPool+dwIdx)->nUserDataLen;

                    OspPrintf(TRUE, FALSE, "[Len:%d Data:", (g_ptRcvGrpPool+dwIdx)->nUserDataLen);

                    int32_t nDataIndex;
                    for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                    {
                        OspPrintf(1, 0, "%x ", *pbyData++);
                    }
                    OspPrintf(1, 0, "]");
                }

                OspPrintf(TRUE, FALSE, "\n");

				dwRcvGrpNum++;
				bGrpPrint = true;
						}

			ptMmb = (g_ptRcvGrpPool+dwIdx)->ptSndMmb;
			while ( ptMmb )
			{
				if( ptMmb->dsId == dsId )
				{
					if( !bGrpPrint )
					{
				        OspPrintf( TRUE ,FALSE ,"\n%s@%d(%s@%d) filter:%x %s%s (%d Packets)",
					        ipString((g_ptRcvGrpPool+dwIdx)->cSock.GetIP()),
					        (g_ptRcvGrpPool+dwIdx)->cSock.GetPort(),
					        ipString((g_ptRcvGrpPool+dwIdx)->cSock.GetMapIP()),
					        (g_ptRcvGrpPool+dwIdx)->cSock.GetMapPort(),
                            (g_ptRcvGrpPool+dwIdx)->pfFilter,
					        IS_HANLDE(dsId,(g_ptRcvGrpPool+dwIdx)->dwDump) ? "[DUMP]":"",
                            IS_HANLDE(dsId,(g_ptRcvGrpPool+dwIdx)->dwSpying) ? "[SPYING]":"",
					        (g_ptRcvGrpPool+dwIdx)->cSock.GetRcvPkt());
						dwRcvGrpNum++;

                        if ((g_ptRcvGrpPool+dwIdx)->nUserDataLen > 0)
                        {
                            uint8_t *pbyData = (g_ptRcvGrpPool+dwIdx)->achUserData;
                            int32_t nDataLen = (g_ptRcvGrpPool+dwIdx)->nUserDataLen;

                            OspPrintf(TRUE, FALSE, "[Len:%d Data:", (g_ptRcvGrpPool+dwIdx)->nUserDataLen);

                            int32_t nDataIndex;
                            for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                            {
                                OspPrintf(1, 0, "%x ", *pbyData++);
                            }
                            OspPrintf(1, 0, "]");
                        }

                        OspPrintf(TRUE, FALSE, "\n");

						bGrpPrint = true;
					}
					memset( achSrc ,0 ,sizeof(achSrc) );
					if( ptMmb->dwSrcIP && ptMmb->wSrcPort )
					{
						sprintf(achSrc ,"<%s@%d>",ipString(ptMmb->dwSrcIP),ptMmb->wSrcPort);
					}
					OspPrintf( TRUE ,FALSE ,"\t|->%s@%d %s (%d Packets )",
						ipString(ptMmb->dwDstIP),
						ptMmb->wDstPort,
						achSrc,
						ptMmb->dwSndPkt);

                    if (ptMmb->nUserDataLen > 0)
                    {
                        uint8_t *pbyData = ptMmb->achUserData;
                        int32_t nDataLen = ptMmb->nUserDataLen;

                        OspPrintf(TRUE, FALSE, "[Len:%d Data:", ptMmb->nUserDataLen);

                        int32_t nDataIndex;
                        for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                        {
                            OspPrintf(1, 0, "%x ", *pbyData++);
                        }
                        OspPrintf(1, 0, "]");
                    }

                    OspPrintf(TRUE, FALSE, "\n");
				}
				dwItemsInMatrix++;
				ptMmb = ptMmb->ptNextMmb;
			}
		}
	}
	OspPrintf(TRUE,FALSE,"\nRcvRules: %lu, ItemsInMatrix: %lu.\n", dwRcvGrpNum, dwItemsInMatrix);
}


void CSockSwitch::ShowInfo()
{
	if (false == g_cSockSwitch.m_bInit)
	{
		OspPrintf( TRUE , FALSE , "DataSwitch is not Created!\n" );
		return;
	}
	DSID adsId[DS_MAXNUM_HANLE];
	int num;
	num = GetDSID( adsId ,DS_MAXNUM_HANLE);
	for( int i=0 ;i<num;i++)
		ShowInfo(adsId[i]);
	OspPrintf(TRUE,FALSE,"______________________________________________________________\n");
}


void CSockSwitch::ShowIFInfo()
{
	uint32_t adwIP[10] ,num;
	long lIdx;
	num = m_cDSIf.GetIP( adwIP ,sizeof(adwIP) );
	for( uint32_t Idx=0 ;Idx< num ;Idx++ )
	{
		lIdx = m_cDSIf.GetIfIdx(adwIP[Idx]);
		OspPrintf(TRUE,FALSE,"IP Interface %d .IP Addr is %s.\n",lIdx,ipString(adwIP[Idx]));
		for( uint32_t dwGrpIdx = 0; dwGrpIdx < g_dwMaxRcvGrp; dwGrpIdx++ )
		{
			if( (g_ptRcvGrpPool+dwGrpIdx)->bUsing )
			{
				(g_ptRcvGrpPool+dwGrpIdx)->cSock.ShowMc( adwIP[Idx]);
			}
		}
	}
}


void CSockSwitch::ShowSocket()
{

	for( uint32_t dwGrpIdx = 0; dwGrpIdx < g_dwMaxRcvGrp; dwGrpIdx++ )
	{
		if( (g_ptRcvGrpPool+dwGrpIdx)->bUsing )
		{
			(g_ptRcvGrpPool+dwGrpIdx)->cSock.Show();
		}
	}

}


uint32_t CSockSwitch::SetTrace(DSID dsId, uint32_t dwLocalIP, uint16_t wLocalPort, uint32_t dwTraceNum)
{
    //检测ID合法性
    if( !IsExistDSID(dsId) )
        return DSERROR;

    //检测接收组是否已经存在
    TRcvGrp* ptRcvGrp = GetRcvGrp( dwLocalIP ,wLocalPort );
    if( NULL == ptRcvGrp )
    {
        return DSERROR;
    }

    OspPrintf(TRUE, FALSE, "SetTrace, last:%d, now %d\n", ptRcvGrp->dwTraceNum, dwTraceNum);
    ptRcvGrp->dwTraceNum = dwTraceNum;

    return DSOK;
}

///////////////////////////////////////////////////////////////////////
//
//          DATA SWITCH DS_API 接口
//
///////////////////////////////////////////////////////////////////////

/**********************************************************
 * 函数: dsSetCapacity                                    *
 * 功能: 容量设置(只能在创建交换模块之前调用)             *
 * 输入: dwMaxRcvGrp        - 最大接收组数                *
 *       dwMaxSndMmbPerRcvGrp  - 每一接收组最大发送目标数   *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsSetCapacity( uint32_t dwMaxRcvGrp , uint32_t dwMaxSndMmbPerRcvGrp )
{
	if( g_bDataSwitchInit )
	{
		dslog(DS_LOG_DEBUG,  "Can't set capacity after dataswitch module has been initialized!\n" );
		return DSERROR;
	}

    //【Comment】20130124 huangzhichun:该限制目前是为了限制而限制，就算去除也不会影响业务（只要不是因为误配--传入很大的值，超过系统的性能即可）
	//if( dwMaxRcvGrp*dwMaxSndMmbPerRcvGrp > MAXNUM_SWITCH )
	//{
	//	dslog(DS_LOG_DEBUG,  "It's limited that max num of switch target is %d!\n" , MAXNUM_SWITCH );
	//	return DSERROR;
	//}

#ifdef USE_EPOLL
	if( dwMaxRcvGrp >= 1024 )
	{
		rlimit tRLimit;
		tRLimit.rlim_cur = 10240;
		tRLimit.rlim_max = 10240;
		if( 0 != setrlimit(RLIMIT_NOFILE , &tRLimit) )
		{
			dslog(DS_LOG_DEBUG,  "extend max file description num per process failed!\n" );
			return DSERROR;
		}
	}
#endif

	g_dwMaxRcvGrp = dwMaxRcvGrp;
	g_dwMaxSndMmbPerRcvGrp = dwMaxSndMmbPerRcvGrp;
	return DSOK;
}

/**********************************************************
 * 函数: dsCreate                                         *
 * 功能: 创建交换模块                                     *
 * 输入: num     - 接口IP数组个数                         *
 *       dwIP[]  - 接口IP数组                             *
 * 输出: 无                                               *
 * 返回: 成功返回不等于INVALID_DSID的值                   *
 *       否则返回 INVALID_DSID                            *
 **********************************************************/
DS_API DSID dsCreate( uint32_t num, uint32_t adwIP[], const char* pszSendIf )
{
    memset(g_szSendIf, 0, sizeof(g_szSendIf));
    if (NULL != pszSendIf)
    {
        strncpy(g_szSendIf, pszSendIf, sizeof(g_szSendIf) - 1);
    }

    if( !g_bDataSwitchInit )
	{
		/// ::OspSemBCreate(&g_smDSToken);
        CBB::cbb_semb_create(g_smDSToken);

		dsInitIDPool();
		g_bDataSwitchInit = true;
	}

	if( !g_cSockSwitch.Init() )
	{
		return INVALID_DSID;
	}

	return g_cSockSwitch.Create( num, adwIP );
}


/**********************************************************
 * 函数: dsDestroy                                        *
 * 功能: 销毁交换模块                                     *
 * 输入: dsID    - 创建交换模块时的句柄                   *
 * 输出: 无                                               *
 * 返回: 无                                               *
 **********************************************************/
DS_API void dsDestroy( DSID dsId )
{
	if( INVALID_DSID == dsId ) return ;

	TDSCommand tCmd;
	/// CBB::cbb_sem_take(g_smDSToken);
    CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId  = dsId;
	tCmd.dwCmd    = DSCMD_DESTORY;

	tCmd.dwLocalIP  = 0;
	tCmd.wLocalPort  = 0;
	tCmd.dwLocalIfIP = 0;

	tCmd.dwSrcIP = 0L;
	tCmd.wSrcPort = 0;

	tCmd.dwDstIP  = 0;
	tCmd.wDstPort  = 0;
	tCmd.dwDstIfIP = 0;

	g_dwCmdSn++;
	g_cSockSwitch.SetCmd(&tCmd, NULL);

	/// CBB::cbb_sem_give(g_smDSToken);
    CBB::cbb_sem_give(g_smDSToken);

	return;
}


/**********************************************************
 * 函数: dsGetRecvPktCount                                *
 * 功能: 查询接收总包数                                   *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIP   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwSrcIP     - 源IP                               *
 *       wSrcPort    - 源端口号                           *
 *       dwRecvPktCount  - 接收总包数                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsGetRecvPktCount( DSID dsId , TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, uint32_t &dwRecvPktCount)
{
	return g_cSockSwitch.GetRecvPktCount( dsId , tRcvAddr.dwIP , tRcvAddr.wPort , tSrcAddr.dwIP , tSrcAddr.wPort , dwRecvPktCount );
}


/**********************************************************
 * 函数: dsGetSendPktCount                                *
 * 功能: 查询发送总包数                                   *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIP   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwSrcIP     - 源IP                               *
 *       wSrcPort    - 源端口号                           *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 *       dwSendPktCount  - 发送总包数                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsGetSendPktCount( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, TDSNetAddr tSendToAddr, uint32_t &dwSendPktCount )
{
	return g_cSockSwitch.GetSendPktCount( dsId , tRcvAddr.dwIP , tRcvAddr.wPort , tSrcAddr.dwIP , tSrcAddr.wPort , tSendToAddr.dwIP , tSendToAddr.wPort , dwSendPktCount );
}


/**********************************************************
 * 函数: dsGetRecvBytesCount	                          *
 * 功能: 查询接收总字节数                                 *
 * 输入:												  *
 * 输出: 无                                               *
 * 返回: 返回即时接收字节数								  *
 **********************************************************/
DS_API int64_t dsGetRecvBytesCount( )
{
	return g_swRcvBytesCount;
}


/**********************************************************
 * 函数: dsGetSendBytesCount	                          *
 * 功能: 查询发送总字节数                                 *
 * 输入:												  *
 * 输出: 无                                               *
 * 返回: 返回即时字节数									  *
 **********************************************************/
DS_API int64_t dsGetSendBytesCount( )
{
	return g_swSndBytesCount;
}


/**********************************************************
 * 函数: dsAdd                                            *
 * 功能: 增加交换节点                                     *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIP   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwLocalIfIP - 本地接收接口IP                     *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 *   	 dwDstIfIP   - 转发目的接口IP                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsAdd(DSID dsId, TDSNetAddr tRecvAddr, uint32_t dwInLocalIP, TDSNetAddr tSendToAddr, uint32_t dwOutLocalIP)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
	{
		return DSERROR;
	}

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId  = dsId;
	tCmd.dwCmd          = DSCMD_ADD;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = dwInLocalIP;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = dwOutLocalIP;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);
	return dwRet;
}


/**********************************************************
 * 函数: dsRemove                                         *
 * 功能: 删除交换节点                                     *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemove(DSID dsId, TDSNetAddr tSendToAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_REMOVE;

	tCmd.dwLocalIP      = 0;
	tCmd.wLocalPort     = 0;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsAddDump                                        *
 * 功能: 增加垃圾节点                                     *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIp   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwLocalIfIP - 本地接收接口IP                     *
 *       dwDstIpAddr - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 *   	 dwDstOutIfIP- 转发目的接口IP                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsAddDump(DSID dsId, TDSNetAddr tRecvAddr, uint32_t dwInLocalIP)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_ADD_DUMP;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = dwInLocalIP;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsRemoveDump                                     *
 * 功能: 删除垃圾节点                                     *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIP   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemoveDump( DSID dsId, TDSNetAddr tRecvAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_RM_DUMP;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsAddManyToOne                                   *
 * 功能: 增加交支持多点交换到一点的交换                   *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalP    - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwLocalIfIP - 本地接收接口IP                     *
 *       dwDstIIP    - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 *   	 dwDstOutIfIP- 转发目的接口IP                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsAddManyToOne(DSID dsId , TDSNetAddr tRecvAddr, uint32_t  dwInLocalIP, TDSNetAddr tSendToAddr, uint32_t  dwOutLocalIP)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
	{
		return DSERROR;
	}

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_ADD_M2ONE;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = dwInLocalIP;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = dwOutLocalIP;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsRemoveAllManyToOne                             *
 * 功能: 删除所有交换到指定目的节点的交换                 *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemoveAllManyToOne(DSID dsId , TDSNetAddr tSendToAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_RM_ALLM2ONE;

	tCmd.dwLocalIP      = 0;
	tCmd.wLocalPort     = 0;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsRemoveManyToOne                                *
 * 功能: 删除指定的交换节点                               *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIP   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemoveManyToOne(DSID dsId , TDSNetAddr tRecvAddr, TDSNetAddr tSendToAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_RM_M2ONE;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0L;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsAddSrcSwitch                                   *
 * 功能: 增加根据UDP包的源IP、Port交换的点                *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIp   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwLocalIfIP - 本地接收接口IP                     *
 *       dwSrcIP     - 源IP                               *
 *       wSrcPort    - 源端口号                           *
 *       dwDstIP     - 转发目的IP地址                     *
 *       wDstPort    - 转发目的端口号                     *
 *   	 dwDstOutIfIP- 转发目的接口IP                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 *                                                        *
 * 注: 该方式为支持多点到一点交换                         *
 *     该函数将dwLocalIP@wLocalPort 收到的源IP/PORT为     *
 * dwSrcIP@wSrcPort的UDP数据包转发到dwDstIP@wDstPort      *
 **********************************************************/
DS_API uint32_t dsAddSrcSwitch(DSID dsId , TDSNetAddr tRecvAddr, uint32_t  dwInLocalIP, TDSNetAddr tSrcAddr, TDSNetAddr tSendToAddr, uint32_t  dwOutLocalIP)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_ADD_SRC_M2ONE;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = dwInLocalIP;

	tCmd.dwSrcIP        = tSrcAddr.dwIP;
	tCmd.wSrcPort       = tSrcAddr.wPort;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = dwOutLocalIP;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);
	 return  dwRet;
 }


DS_API uint32_t dsSetUserData(DSID dsId,
					TDSNetAddr tLocalAddr,
					uint32_t        dwLocalIfIp,
					TDSNetAddr tSrcAddr,
					TDSNetAddr tDstAddr,
					uint32_t        dwDstOutIfIP,
					bool      bSend,
					uint8_t *pbyUserData,
					int32_t nUserLen)
 {
	 uint32_t dwRet = DSERROR;

     if( INVALID_DSID == dsId ) return DSERROR;
     if (nUserLen < 0 || nUserLen > DS_MAX_USER_LEN) return DSERROR;
     if (nUserLen > 0 && NULL == pbyUserData) return DSERROR;

	 TDSCommand tCmd;
	 TDSCommand_AttchedData tAttacheData;

	 CBB::cbb_sem_take(g_smDSToken);
     tCmd.dsId           = dsId;
     tCmd.dwCmd          = DSCMD_SET_USERDATA;

     tCmd.dwLocalIP      = tLocalAddr.dwIP;
     tCmd.wLocalPort     = tLocalAddr.wPort;
     tCmd.dwLocalIfIP    = dwLocalIfIp;

     tCmd.dwSrcIP        = tSrcAddr.dwIP;
     tCmd.wSrcPort       = tSrcAddr.wPort;

     tCmd.dwDstIP        = tDstAddr.dwIP;
     tCmd.wDstPort       = tDstAddr.wPort;
     tCmd.dwDstIfIP      = dwDstOutIfIP;

     tAttacheData.nAttchedDataLen = sizeof(tAttacheData);

     tAttacheData.uAttchedData.tUserData.bSend = bSend;
     tAttacheData.uAttchedData.tUserData.nUserDataLen = nUserLen;
     memcpy((void *)tAttacheData.uAttchedData.tUserData.achUserData, pbyUserData, nUserLen);

	 g_dwCmdSn++;
     dwRet = g_cSockSwitch.SetCmd(&tCmd, &tAttacheData);

     CBB::cbb_sem_give(g_smDSToken);
	return dwRet;
}


/**********************************************************
 * 函数: dsRemoveAllSrcSwitch                             *
 * 功能: 删除所有根据源IP、Port交换到目的节点的交换       *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwLocalIp   - 本地接收IP                         *
 *       wLocalPort  - 本地接收端口号                     *
 *       dwDstIP - 转发目的IP地址                         *
 *       wDstPort    - 转发目的端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemoveAllSrcSwitch(DSID dsId, TDSNetAddr tRecvAddr, TDSNetAddr tSrcAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_RM_SRC_ALLM2ONE;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = tSrcAddr.dwIP;
	tCmd.wSrcPort       = tSrcAddr.wPort;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsRemoveSrcSwitch                                *
 * 功能: 删除指定的根据源IP、Port交换到目的节点的交换     *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwSrcIP     - 源IP                               *
 *       wSrcPort    - 源端口号                           *
 *       dwDstIP - 转发目的IP地址                         *
 *       wDstPort    - 转发目的端口号                     *
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsRemoveSrcSwitch(DSID dsId, TDSNetAddr tRecvAddr,  TDSNetAddr tSrcAddr,  TDSNetAddr tSendToAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_RM_SRC_M2ONE;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = tSrcAddr.dwIP;
	tCmd.wSrcPort       = tSrcAddr.wPort;

	tCmd.dwDstIP        = tSendToAddr.dwIP;
	tCmd.wDstPort       = tSendToAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsSpecifyFwdSrc                                  *
 * 功能: 为指定接收地址设置转发数据包所填充的源地址       *
 * 输入:
 * @param dsId          - DSID
 * @param dwOrigIP        - 原始IP
 * @param wOrigPort      - 原始Port
 * @param dwMappedIP      - 映射IP
 * @param wMappedPort    - 映射Port
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsSpecifyFwdSrc(DSID dsId, TDSNetAddr tOrigAddr, TDSNetAddr tMappedAddr)
{
    if ((tOrigAddr.dwIP == tMappedAddr.dwIP) && (tOrigAddr.wPort == tMappedAddr.wPort))
		return DSOK;    //Modified by Gaoqi on 2007-6-13,
                                                                                //in request of liuhuiping.
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;

	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_MAP_ADDR;

	tCmd.dwLocalIP      = tOrigAddr.dwIP;
	tCmd.wLocalPort     = tOrigAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = tMappedAddr.dwIP;
	tCmd.wSrcPort       = tMappedAddr.wPort;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsResetFwdSrc                                    *
 * 功能: 恢复指定地址转发数据包的源地址
 * 输入:
 * @param dsId          - DSID
 * @param dwOrigIP        - 原始IP
 * @param wOrigPort      - 原始Port
 * 输出: 无                                               *
 * 返回: 成功返回DSOK 否则DSERROR                         *
 **********************************************************/
DS_API uint32_t dsResetFwdSrc(DSID dsId, TDSNetAddr tOrigAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_MAP_ADDR;

	tCmd.dwLocalIP      = tOrigAddr.dwIP;
	tCmd.wLocalPort     = tOrigAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


DS_API uint32_t dsSetSrcAddrByDst(DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tDstAddr, TDSNetAddr tRawAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
    CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_SRC_ADDR_BY_DST;

	tCmd.dwLocalIP      = tRcvAddr.dwIP;
	tCmd.wLocalPort     = tRcvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = tRawAddr.dwIP;
	tCmd.wSrcPort       = tRawAddr.wPort;

	tCmd.dwDstIP        = tDstAddr.dwIP;
	tCmd.wDstPort       = tDstAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);
    CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}

DS_API uint32_t dsResetSrcAddrByDst(DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tDstAddr)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_SRC_ADDR_BY_DST;

	tCmd.dwLocalIP      = tRcvAddr.dwIP;
	tCmd.wLocalPort     = tRcvAddr.wPort;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = tDstAddr.dwIP;
	tCmd.wDstPort       = tDstAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);
	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}
/**********************************************************
 * 函数: dsSetSSRCChange                                  *
 * 功能: 设置对码流中SSRC作改动                           *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwIP     - IP                                    *
 *       wPort    - 端口号                                *
 * 输出: 无                                               *
 * 返回: 无                                               *
 **********************************************************/
DS_API uint32_t dsSetSSRCChange(DSID dsId,
								uint32_t  dwIP,
								uint16_t  wPort)
{
	/* Win32上暂时不支持此功能 */
	return DSERROR;
}


/**********************************************************
 * 函数: dsCancelSSRCChange                                  *
 * 功能: 取消对码流中SSRC作改动                           *
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwIP     - IP                                    *
 *       wPort    - 端口号                                *
 * 输出: 无                                               *
 * 返回: 无                                               *
 **********************************************************/
DS_API uint32_t dsCancelSSRCChange(DSID dsId,
									uint32_t  dwIP,
									uint16_t  wPort)
{
	/* Win32上暂时不支持此功能 */
	return DSERROR;
}


/**********************************************************
 * 函数: dsSetFilterFunc                                  *
 * 功能: 设置过滤函数                                     *
 *       取消时，设置函数指针为NULL即可
 * 输入: dsID        - 创建交换模块时的句柄               *
 *       dwIP     - IP                                    *
 *       wPort    - 端口号                                *
 *       pfFilter - 函数指针
 * 输出: 无                                               *
 * 返回: 无                                               *
 **********************************************************/
DS_API uint32_t dsSetFilterFunc(DSID dsId, TDSNetAddr tRecvAddr, FilterFunc ptFunc)
{
	// 虽然可以直接通过全局变量设置回调函数，不过，实现时
	// 还是遵循正常的逻辑，以免在特殊情况下两个线程不同步
	// 产生错误和混乱
	// 在设置函数指针前，也会像NIP上的那样先增加一个DUMP
	// 规则，不过，不在这里显式地指定
	uint32_t dwRet = DSERROR;

	TDSCommand tCmd;
	if( INVALID_DSID == dsId )
		return DSERROR;

	dslog(DS_LOG_DEBUG, "dsSetFilterFunc: local(%s@%d)\n", ipString(tRecvAddr.dwIP), tRecvAddr.wPort);
	CBB::cbb_sem_take(g_smDSToken);

	tCmd.dsId              = dsId;

	tCmd.dwLocalIP      = tRecvAddr.dwIP;
	tCmd.wLocalPort     = tRecvAddr.wPort;
	tCmd.dwLocalIfIP    = tRecvAddr.dwIP;

	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	// 命令号暂时还用SPY
	if (ptFunc != NULL)
	{
		tCmd.dwCmd      = DSCMD_ADD_SPY;
		tCmd.llContext    = (uint64_t)ptFunc;
	}
	else
	{
		tCmd.dwCmd      = DSCMD_RM_SPY;
		tCmd.llContext    = 0;
	}

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsSetSendCallback                                *
 * 功能: 设置发送回调函数                                 *
 *       取消时，设置函数指针为NULL即可
 * 输入: dsID           - 创建交换模块时的句柄            *
 *       dwLocalIp      - 本地接收IP                      *
 *       wLocalPort     - 本地接收端口号                  *
 *       dwSrcIP        - 转发目的IP地址                  *
 *       wSrcPort       - 转发目的端口号                  *
 *       pfCallback     - 回调函数
 * 输出: 无                                               *
 * 返回: 无                                               *
 **********************************************************/
DS_API uint32_t dsSetSendCallback( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, SendCallback pfCallback)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;

	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_CALLBACK;

	tCmd.dwLocalIP      = tRcvAddr.dwIP;
	tCmd.wLocalPort     = tRcvAddr.wPort;
	tCmd.llContext      = (uint64_t)pfCallback;

	tCmd.dwSrcIP        = tSrcAddr.dwIP;
	tCmd.wSrcPort       = tSrcAddr.wPort;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


/**********************************************************
 * 函数: dsSetAppDataForSend                              *
 * 功能: 为发送目标设置一个自定义的指针                   *
 *       取消时，设置函数指针为NULL即可
 * 输入: dsID           - 创建交换模块时的句柄            *
 *       dwLocalIp      - 本地接收IP                      *
 *       wLocalPort     - 本地接收端口号                  *
 *       dwSrcIP        - 源IP地址                  *
 *       wSrcPort       - 源端口号                  *
 *       dwDstIP        - 转发目的IP地址                  *
 *       wDstPort       - 转发目的端口号                  *
 *       pAppData       - 自定义指针
 * 输出: 无                                               *
 * 返回:
 *     DSOK:成功 DSERROR:失败                             *
 **********************************************************/
DS_API uint32_t dsSetAppDataForSend( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, TDSNetAddr tDstAddr, void * pAppData)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;

	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	tCmd.dwCmd          = DSCMD_SET_APPDATA;

	tCmd.dwLocalIP      = tRcvAddr.dwIP;
	tCmd.wLocalPort     = tRcvAddr.wPort;
	tCmd.llContext      = (uint64_t)pAppData;

	tCmd.dwSrcIP        = tSrcAddr.dwIP;
	tCmd.wSrcPort       = tSrcAddr.wPort;

	tCmd.dwDstIP        = tDstAddr.dwIP;
	tCmd.wDstPort       = tDstAddr.wPort;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}


static DSID		g_dsIdSpyId = INVALID_DSID;
static uint32_t g_dwSpyInsId = 0;
static uint16_t g_wSpyEvent = 0;
static uint16_t g_wSpyUdpDataLen = 0;

DS_API uint32_t dsRegSpy(DSID dsId,
					uint32_t dwInstId,
					uint16_t wEvent,
					uint16_t wUDPDataLen)
{
	// 这里的SPY实现，是作为验证手段，而不是功能
	// 提供的，所以与原来实现的语义并不相同
	if ( g_dsIdSpyId  != INVALID_DSID ||
		 g_dwSpyInsId != 0            ||
		 g_wSpyEvent  != 0            ||
		 g_wSpyUdpDataLen != 0 )
	{
		dslog(DS_LOG_DEBUG, "DS.%d already reg spy for inst.%d with envent(%d) for %dbyte data!\n",
				g_dsIdSpyId,g_dwSpyInsId,g_wSpyEvent,g_wSpyUdpDataLen);
		return DSERROR;
	}

	g_dsIdSpyId       = dsId;
	g_dwSpyInsId      = dwInstId;
	g_wSpyEvent       = wEvent;
	g_wSpyUdpDataLen  = wUDPDataLen;

	return DSOK;
}


DS_API uint32_t dsUnRegSpy(DSID dsId)
{
	if ( dsId != g_dsIdSpyId || INVALID_DSID == g_dsIdSpyId )
	{
		dslog(DS_LOG_DEBUG, "DS.%d not reg spy!the current spy is DS.%d\n",dsId,g_dsIdSpyId );
		return DSERROR;
	}

	g_dsIdSpyId  = INVALID_DSID;
	g_dwSpyInsId = 0;
	g_wSpyEvent  = 0;
	g_wSpyUdpDataLen = 0 ;

	return DSOK;
}


/*********************************************************
* 函数: FuncForSpy                                       *
* 功能: 用回调函数实现SPY功能                            *
		    应用程序应该直接使用回调函数，不应再使用SPY接口。
		    此处实现只是为了兼容性，及验证回调函数。
		    另外，OspPost在失败时会有固定的2秒延迟，无法修改。
* 输入:
*       dwRecvIP   - 本地接收IP                          *
*       wRecvPort  - 本地接收端口号                      *
*       dwSrcIP    - 源IP                          			 *
*       wSrcPort   - 源端口号                      			 *
*			  pData			 - UDP数据包
* 		  nLen			 - 数据包长度
* 输出: 无                                               *
* 返回: 成功返回DSOK 否则DSERROR                         *
**********************************************************/
uint32_t FuncForSpy(uint32_t dwRecvIP,
					uint16_t wRecvPort,
					uint32_t dwSrcIP,
					uint16_t wSrcPort,
					uint8_t *pData,
					uint32_t nLen)
{
	return DSOK;
}


DS_API uint32_t dsAddSpy(DSID dsId, TDSNetAddr tLocalAddr,
					uint32_t dwIfIP)
{
	return dsSetFilterFunc(dsId, tLocalAddr, FuncForSpy);
}


DS_API uint32_t dsRemvoeSpy(DSID dsId, TDSNetAddr tLocalAddr)
{
	return dsSetFilterFunc(dsId, tLocalAddr, NULL);
}


DS_API uint32_t dsRemoveAll(DSID dsId, bool bKeepDump /*= FALSE*/)
{
	uint32_t dwRet = DSERROR;

	if( INVALID_DSID == dsId )
		return DSERROR;

	TDSCommand tCmd;
	CBB::cbb_sem_take(g_smDSToken);
	tCmd.dsId              = dsId;
	if (!bKeepDump)
	{
		tCmd.dwCmd          = DSCMD_RM_ALL;
	}
	else
	{
		tCmd.dwCmd      = DSCMD_RM_ALL_EXCEPT_DUMP;
		// 注意：这里要填0
	}


	tCmd.dwLocalIP      = 0;
	tCmd.wLocalPort     = 0;
	tCmd.dwLocalIfIP    = 0;

	tCmd.dwSrcIP        = 0;
	tCmd.wSrcPort       = 0;

	tCmd.dwDstIP        = 0;
	tCmd.wDstPort       = 0;
	tCmd.dwDstIfIP      = 0;

	g_dwCmdSn++;
	dwRet = g_cSockSwitch.SetCmd(&tCmd, NULL);

	CBB::cbb_sem_give(g_smDSToken);

	return dwRet;
}

DS_API void dsinfo()
{
	g_cSockSwitch.ShowInfo();
}


DS_API void dsif()
{
	g_cSockSwitch.ShowIFInfo();
}


DS_API void dssock()
{
	g_cSockSwitch.ShowSocket();
}


 DS_API void dstrace(DSID dsId, char *pszLocalIP, uint16_t wLocalPort, uint32_t dwTraceNum)
 {
     uint32_t dwRet;
     dwRet = g_cSockSwitch.SetTrace(dsId, inet_addr(pszLocalIP), wLocalPort, dwTraceNum);
     OspPrintf(TRUE, FALSE, "SetTrace return: %d\n", dwRet);
 }

//发送hash，用于按源交换时查找发送目标
static void RemovefromSndHash(TSndMmb* ptSnd, uint32_t dwLocalIP, uint16_t wLocalPort)
{
    if (NULL == ptSnd)
    {
        return;
    }

    uint32_t dwHash = dwLocalIP + wLocalPort + ptSnd->dwSrcIP + ptSnd->wSrcPort;
    dwHash = dwHash % g_dwSndHashSize;

    TSndMmb* ptLoop = g_aptSndHash[dwHash];
    TSndMmb* ptPre = NULL;

    while(NULL != ptLoop)
    {
        if (ptLoop == ptSnd)
        {
            break;
        }

        ptPre = ptLoop;
        ptLoop = ptLoop->ptHashNext;
    }

    if (NULL == ptLoop)
    {
        return;
    }

    if (NULL == ptPre)
    {
        g_aptSndHash[dwHash] = ptLoop->ptHashNext;
    }
    else
    {
        ptPre->ptHashNext = ptLoop->ptHashNext;
    }

    ptLoop->ptHashNext = NULL;

    return;
}

static void AddtoSndHash(TSndMmb* ptSnd, uint32_t dwLocalIP, uint16_t wLocalPort)
{
    if (NULL == ptSnd)
    {
        return;
    }

    ptSnd->ptHashNext = NULL;

    uint32_t dwHash = dwLocalIP + wLocalPort + ptSnd->dwSrcIP + ptSnd->wSrcPort;
    dwHash = dwHash % g_dwSndHashSize;

    //for safe, check if it is already in hash list
    RemovefromSndHash(ptSnd, dwLocalIP, wLocalPort);

    ptSnd->ptHashNext = g_aptSndHash[dwHash];
    g_aptSndHash[dwHash] = ptSnd;

    return;
}


static TSndMmb* GetHashList(uint32_t dwLocalIP, uint16_t wLocalPort, uint32_t dwSrcIP, uint16_t wSrcPort)
{
    uint32_t dwHash = dwLocalIP + wLocalPort + dwSrcIP + wSrcPort;
    dwHash = dwHash % g_dwSndHashSize;

    return g_aptSndHash[dwHash];
}


DS_API void sndcheckhash()
{
   uint32_t i;

    if (NULL != g_aptSndHash)
    {
        uint32_t nTotalCount = 0;
        uint32_t nPerMaxCount = 0;

        for(i = 0 ; i < g_dwSndHashSize; i++)
        {
            TSndMmb* ptLoop;
            ptLoop = g_aptSndHash[i];

            uint32_t nPerCount = 0;

            while(NULL != ptLoop && nPerCount < g_dwSndHashSize)
            {
                nPerCount++;
                ptLoop = ptLoop->ptHashNext;
            }

            if (nPerCount >= g_dwSndHashSize)
            {
                OspPrintf(TRUE, FALSE, "Per Count error.\n");
                break;
            }

            if (nPerCount > nPerMaxCount)
            {
                nPerMaxCount = nPerCount;
            }

            nTotalCount += nPerCount;
        }

        OspPrintf(TRUE, FALSE, "Snd hash total count:%d max per:%d\n", nTotalCount, nPerMaxCount);
    }
}


//接收Hash，用于windows select中，根据socket查找recv对象
static void RemovefromRcvHash(TRcvGrp* ptRcvGrp)
{
    if (NULL == ptRcvGrp || NULL == g_aptRcvHash)	//To avoid the chance of refer to NULL!
    {
        return;
    }

    uint32_t dwHash = ptRcvGrp->cSock.GetSock();
    dwHash = dwHash % g_dwMaxRcvGrp;

    TRcvGrp* ptLoop = g_aptRcvHash[dwHash];
    TRcvGrp* ptPre = NULL;

    while(NULL != ptLoop)
    {
        if (ptLoop == ptRcvGrp)
        {
            break;
        }

        ptPre = ptLoop;
        ptLoop = ptLoop->ptHashNext;
    }

    if (NULL == ptLoop)
    {
        return;
    }

    if (NULL == ptPre)
    {
        g_aptRcvHash[dwHash] = ptLoop->ptHashNext;
    }
    else
    {
        ptPre->ptHashNext = ptLoop->ptHashNext;
    }

    ptLoop->ptHashNext = NULL;

    return;
}

static void AddtoRcvHash(TRcvGrp* ptRcvGrp)
{
    if (NULL == ptRcvGrp)
    {
        return;
    }

    if (NULL == g_aptRcvHash)
    {
        g_aptRcvHash = new TRcvGrp* [g_dwMaxRcvGrp];
        memset(g_aptRcvHash, 0, g_dwMaxRcvGrp*sizeof(TRcvGrp*));
    }

    ptRcvGrp->ptHashNext = NULL;

    uint32_t dwHash = ptRcvGrp->cSock.GetSock();

    dwHash = dwHash % g_dwMaxRcvGrp;

    //for safe, check if it is already in hash list
    RemovefromRcvHash(ptRcvGrp);

    ptRcvGrp->ptHashNext = g_aptRcvHash[dwHash];
    g_aptRcvHash[dwHash] = ptRcvGrp;

    return;
}


static TRcvGrp* GetRcvNode(uint32_t hSock)
{
	if (g_aptRcvHash == NULL)		//To avoid the chance of refer to NULL!
		return NULL;

	uint32_t dwHash = hSock % g_dwMaxRcvGrp;

	TRcvGrp* ptRcv = g_aptRcvHash[dwHash];

	while(NULL != ptRcv)
	{
		if (ptRcv->cSock.GetSock() == hSock)
			break;

		ptRcv = ptRcv->ptHashNext;
	}

	return ptRcv;
}
