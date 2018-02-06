#include "dscommon.h"
#ifdef _MSC_VER
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#ifdef _LINUX_
#include <sys/un.h>
#include <linux/unistd.h>
#endif
/*
#ifdef USE_EPOLL
#warning "use epoll"
#else
#warning "use select"
#endif
*/

#define OspPrintf(n, m, fmt, ...)     DEBUG::dbg_printf("[DS]"fmt, ##__VA_ARGS__)

extern char g_szSendIf[];

int g_socksndbuff = 128*1024*1024; 
int g_sockrcvbuff = 128*1024*1024; 

static uint32_t  g_debug = 0;
static uint32_t g_dwLogLevel = DS_LOG_ERROR;

DS_API void dsDbg()
{
	g_debug = !g_debug;
	OspPrintf( TRUE,FALSE ,"Dataswitch debug info \"%s\"\n",g_debug ? "OPEN":"CLOSE");
}


DS_API void dsdbg()
{
	dsDbg();
}

DS_API void ddlevel(uint32_t level)
{
	if (DS_LOG_NONE == level)
    {
		g_dwLogLevel = DS_LOG_NONE;
        OspPrintf(TRUE, FALSE, "[DS] log level : none\n");
    }
    else if (DS_LOG_ALL == level)
    {
        g_dwLogLevel = DS_LOG_ALL;
        OspPrintf(TRUE, FALSE, "[DS] log level : all\n");
    }
	else
    {
		g_dwLogLevel |= (1 << level);
        OspPrintf(TRUE, FALSE, "[DS] log level : %X\n", g_dwLogLevel);
    }
}

void dslog(uint32_t level, const char * fmt, ...)
{
	if (g_dwLogLevel & (1 << level) || DS_LOG_ALL == level)
	{
		va_list argptr;
		va_start(argptr, fmt);    
	    char PrintBuf[255];
		int BufLen = vsprintf(PrintBuf, fmt, argptr);
		va_end(argptr);
		OspPrintf(TRUE, FALSE, "[DS] %s", PrintBuf);
	}
}


//////////////////////////////////////////////////
static DSID g_dsId[DS_MAXNUM_HANLE]={INVALID_DSID};

void dsInitIDPool()
{
	for( int i=0 ;i< DS_MAXNUM_HANLE ;i++ )
		g_dsId[i]=INVALID_DSID;

}


DSID dsAllocDSID()
{
	
	for(int i=0 ;i< DS_MAXNUM_HANLE ;i++)
	{
		if( INVALID_DSID == g_dsId[i] )
		{
			g_dsId[i] = (DSID)(i+1);
			return g_dsId[i];
		}
	}

	return INVALID_DSID;
}


void dsFreeDSID(DSID dsId )
{
	if( INVALID_DSID == dsId )
		return ;

	for(int i = 0; i < DS_MAXNUM_HANLE; i++)
	{
		if( dsId == g_dsId[i] )
		{
			g_dsId[i] = INVALID_DSID;
			return;
		}
	}
}


int  GetDSID( DSID dsId[] ,int nArraySize)
{
	int num=0;
	for( int i=0 ;i< DS_MAXNUM_HANLE ;i++)
	{
		if( num >= nArraySize )
		{
			return num;
		}
		if( INVALID_DSID != g_dsId[i] )
		{
			dsId[i] = g_dsId[i];
			num++;
		}
	}

	return num;
}


bool IsExistDSID(DSID dsId)
{
	for( int i=0 ;i< DS_MAXNUM_HANLE ;i++)
	{
		if( INVALID_DSID != dsId && g_dsId[i]== dsId )
		{
			return true;
		}
	}
	return false;
}


///////////////////////////////////////////

/*
xyp:
该函数仍然无法解决多线程下的可重入性问题!!!
byIdx没有办法保证线程安全
*/
char* ipString(uint32_t dwIP )
{
	static char achIPStr[16][32]={0};
	static char byIdx = 0;
	struct   in_addr in;

	byIdx++;
	byIdx %=sizeof(achIPStr)/sizeof(achIPStr[0]);
	in.s_addr   = dwIP;
	memset( achIPStr[byIdx] ,0,sizeof(achIPStr[0]) );
	
#ifndef _VXWORKS_
	strncpy( achIPStr[byIdx] ,inet_ntoa(in),sizeof(achIPStr[0]) );
#else
	inet_ntoa_b( in ,achIPStr[byIdx] );
#endif

	achIPStr[byIdx][31]= 0;
	return achIPStr[byIdx];
}


int getSockErrno()
{
#ifdef _MSC_VER
	return ::WSAGetLastError();
#else
	return errno;
#endif
}

#ifdef _LINUX_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#endif

#ifdef WIN32
#ifndef SIO_UDP_CONNRESET 
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12) 
#endif
#endif


/*===================================================
	函数名  ：createSocket建
	功能    ：创建一个Socket 并绑定在dwIP@wPort上
	输入    ：dwIP  - 绑定IP
	          wPort - 绑定端口号
	输出    ：无
	返回值  ：socket描述符
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
SOCKHANDLE createUDPSocket( uint32_t dwIP ,uint16_t wPort , uint32_t dwBufSize)
{
	SOCKHANDLE sock  = INVALID_SOCKET;
	SOCKADDR_IN tSockAddr;
	
	SETSOCKADDR( tSockAddr ,dwIP,wPort);

	//Allocate a socket
	sock = socket ( AF_INET, SOCK_DGRAM , 0);
	if( sock == INVALID_SOCKET )
	{
		dslog(DS_LOG_ERROR, "Create UDP Socket Error. (%d)\n",getSockErrno());
		return INVALID_SOCKET;
	}
#ifdef _LINUX_
    if (strlen(g_szSendIf) > 0)
    {
		uint8_t byTry = 0;
		const uint8_t TRY_MAX = 3;

		do {
			if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, g_szSendIf, strlen(g_szSendIf) + 1) < 0)
			{
				dslog(DS_LOG_ERROR, "createUDPSocket setsockopt SO_BINDTODEVICE (%s) error : (%d)%s!\n", g_szSendIf, errno, strerror(errno));
			}
			else
			{
				dslog(DS_LOG_DEBUG, "createUDPSocket setsockopt SO_BINDTODEVICE (%s) OK!\n", g_szSendIf);
				break;
			}

			byTry++;
		} while(byTry < TRY_MAX);

		if (byTry >= TRY_MAX)
		{
			/// SockClose(sock);
            CBB::cbb_sock_close(sock);
			sock = INVALID_SOCKET;
			dslog(DS_LOG_ERROR, "createUDPSocket setsockopt SO_BINDTODEVICE (%s) failed %u times!\n", g_szSendIf, byTry);
			return INVALID_SOCKET;
		}
    }
#endif

	if( bind(sock, (SOCKADDR *)&tSockAddr, sizeof(tSockAddr)) == SOCKET_ERROR )
	{
		/// SockClose(sock);
        CBB::cbb_sock_close(sock);
		dslog(DS_LOG_ERROR, "Bind socket %s@%d Error.(%d)\n",ipString(dwIP),wPort ,getSockErrno() );
		return INVALID_SOCKET;
	}
	int optVal,sndBufSize,rcvBufSize;
	sndBufSize = g_socksndbuff;
	rcvBufSize = g_sockrcvbuff;
	optVal = sndBufSize;
	setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, sizeof (optVal));

	optVal = rcvBufSize;
	setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, sizeof (optVal));

	/*
	nSize = sizeof ( sndBufSize );
#if defined _LINUX_
	if( SOCKET_ERROR != getsockopt( sock, SOL_SOCKET, SO_SNDBUF, ( char * ) &optVal, (socklen_t*)&nSize ) )
#else
	if( SOCKET_ERROR != getsockopt( sock, SOL_SOCKET, SO_SNDBUF, ( char * ) &optVal, &nSize ) )
#endif
	{
		//printf( "getsockopt SO_SNDBUF = %d!\n" , optVal );
	}
	
	nSize = sizeof ( rcvBufSize );
#if defined _LINUX_
	if( SOCKET_ERROR != getsockopt( sock, SOL_SOCKET, SO_RCVBUF, ( char * ) &optVal, (socklen_t*)&nSize ) )
#else
	if( SOCKET_ERROR != getsockopt( sock, SOL_SOCKET, SO_RCVBUF, ( char * ) &optVal, &nSize ) )
#endif
	{
		//printf( "getsockopt SO_RCVBUF = %d!\n" , optVal );
	}
	*/

	u_long on = 1;
#ifdef WIN32
	ioctlsocket(sock, FIONBIO, &on);

	//set not recv icmp
	on = 0;
	int nRet;
	nRet = ioctlsocket(sock, SIO_UDP_CONNRESET, &on);

#else
	ioctl(sock, FIONBIO, &on);
#endif

	return sock;
}


/*===================================================
	函数名  ：createRawSocket建
	功能    ：创建一个原始Socket
	输入    ：无
	输出    ：无
	返回值  ：socket描述符
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
SOCKHANDLE  createRawSocket()
{
    SOCKHANDLE sock = INVALID_SOCKET;
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
    if (sock != -1)
    {
        int optVal,sndBufSize;
        sndBufSize = 2*1024*1024;
        optVal = sndBufSize;
        setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, sizeof (optVal));

        u_long on = 1;
#ifdef WIN32
        ioctlsocket(sock, FIONBIO, &on);

        //set not recv icmp
        on = 0;
        int nRet;
        nRet = ioctlsocket(sock, SIO_UDP_CONNRESET, &on);
#else
        ioctl(sock, FIONBIO, &on);
#endif
    }

    return sock;
}


/*===================================================
	函数名  ：JoinMulticast建
	功能    ：将一个接口加入组播组
	输入    ：sock     -  socket
	          dwMcIP   － 组播组IP地址
	          dwIfIP   － 接口IP
	输出    ：无
	返回值  ：成功返回DSOK，否则返回DSERROR
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容
=====================================================*/
uint32_t joinMulticast( SOCKHANDLE sock ,uint32_t dwMcIP ,uint32_t dwIfIP )
{
   	struct ip_mreq mreq;
    
	if( SOCKET_ERROR == sock )
	{
		dslog(DS_LOG_ERROR, "\"joinMulticast\" use invalid socket when join mc.%s with if.%s.\n",ipString(dwMcIP),ipString(dwIfIP));
		return DSERROR;
	}

	mreq.imr_multiaddr.s_addr = dwMcIP;
    mreq.imr_interface.s_addr = dwIfIP;

    if (setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char *)&mreq, sizeof(mreq)) < 0)
    {
		dslog(DS_LOG_ERROR, "sock.%d \"joinMulticast\" join multicast.%s with if.%s failed(%d).\n",sock,ipString(dwMcIP),ipString(dwIfIP) ,getSockErrno());
        return DSERROR;
    }

    struct	in_addr  addr;
	addr.s_addr = dwIfIP ;

	if( setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof(addr)) != 0 )
	{
        dslog(DS_LOG_ERROR, "Select interface(%s) for outgoing multicast(%s) failed.(%d)\n",ipString(dwIfIP) ,ipString(dwMcIP) ,getSockErrno() );
		return 0;
	}
	dslog(DS_LOG_DEBUG,  "sock.%d join multicas.%s with if.%s success\n",sock,ipString(dwMcIP),ipString(dwIfIP));
	return DSOK;
}


/*===================================================
	函数名  ：dropMulticast
	功能    ：将一个接口撤离组播组
	输入    ：sock     -  socket
	          dwMcIP   － 组播组IP地址
	          dwIfIP   － 接口IP
	输出    ：无
	返回值  ：成功返回DSOK，否则返回DSERROR
-----------------------------------------------------
	修改记录    ：
	日  期      版本        修改人        修改内容]
=====================================================*/
uint32_t dropMulticast( SOCKHANDLE sock ,uint32_t dwMcIP ,uint32_t dwIfIP )
{
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = dwMcIP;
	mreq.imr_interface.s_addr = dwIfIP;

	if( INVALID_SOCKET == sock )
	{
        dslog(DS_LOG_ERROR, "\"dropMulticast\" use invalid socket when join mc.%s with if.%s.\n",ipString(dwMcIP),ipString(dwIfIP));
		return DSERROR;
	}

	if(setsockopt( sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) != 0)
	{
        dslog(DS_LOG_ERROR, "\"dropMulticast\" join multicas.%s with if.%s failed.\n",ipString(dwMcIP),ipString(dwIfIP));
		return DSERROR;
	}
	dslog(DS_LOG_DEBUG,  "Drop multicas.%s with if.%s success\n",ipString(dwMcIP),ipString(dwIfIP));
	return DSOK;
}


////////////////////////////////////

CDSSocket::TMcMmb      CDSSocket::m_atMc[] ={0}; /* 该接口加入组播组状况  */
CDSSocket::TMcMmb*     CDSSocket::m_ptMcFree =NULL;
bool        CDSSocket::m_bMcPoolInit = false;


bool CDSSocket::Init()
{
	if( m_bMcPoolInit ) return false;
	int i;
	for( i =1 ;i< (sizeof(m_atMc)/sizeof(m_atMc[0]) );i++ )
	{
		m_atMc[i-1].ptNext = &m_atMc[i];
		m_atMc[i-1].dwIfIP = 0L;
		m_atMc[i-1].dwMcIP = 0L;
		m_atMc[i-1].dwRef  = 0L;
	}
	m_atMc[i-1].ptNext = NULL;
	m_ptMcFree = &m_atMc[0];
	m_bMcPoolInit = true;
	return true;
}


CDSSocket::TMcMmb* CDSSocket::AllocMcMmb()
{
	TMcMmb* ptMmb;
	if( m_ptMcFree == NULL )return NULL;
	ptMmb = m_ptMcFree;
	m_ptMcFree = m_ptMcFree->ptNext;
	ptMmb->ptNext = NULL;
	return ptMmb;
}


void CDSSocket::FreeMcMmb( CDSSocket::TMcMmb* ptMmb )
{
	ptMmb->dwIfIP = 0L;
	ptMmb->dwMcIP = 0L;
	ptMmb->dwRef  = 0L;
	ptMmb->ptNext = m_ptMcFree;
	m_ptMcFree    = ptMmb;
}


void CDSSocket::InitInterVal()
{
	m_sock     = INVALID_SOCKET;
	m_bBC      = false;
	m_dwIP     = 0;  
	m_wPort    = 0;        
	m_dwMapIP  = 0;  
	m_wMapPort = 0;
	m_dwIfIP   = 0;         
	m_dwRcvPkt = 0;      
	m_dwRcvBytes = 0;     
	m_ptMcMmb    = NULL;
}


CDSSocket::CDSSocket()
{
	Init();
	InitInterVal();
}


bool CDSSocket::Create()
{
	m_sock   = createRawSocket();
	if( INVALID_SOCKET == m_sock)
		return false;
	m_dwIP   = 0;
	m_wPort  = 0;
	m_dwMapIP   = 0;
	m_wMapPort  = 0;
	m_dwIfIP = 0;
	return true;
}
bool CDSSocket::Create( uint32_t dwIP ,uint16_t wPort ,uint32_t dwIfIP , uint32_t dwBufSize)
{
	if( m_sock != INVALID_SOCKET )
	{
		dslog(DS_LOG_DEBUG, "Grp already exist!");
		return false;
	}
	
	m_sock = createUDPSocket( dwIP ,wPort , dwBufSize);
	if( INVALID_SOCKET == m_sock )
		return false;
	if( IS_MCIP(dwIP) )
	{
		if( !JoinMc(dwIP ,dwIfIP) )
		{
			return false;
		}
	}

	m_dwIP   = dwIP;
	m_wPort  = wPort;
	m_dwMapIP   = 0;
	m_wMapPort  = 0;
	m_dwIfIP = dwIfIP;
	return true;
}


void CDSSocket::Destroy()
{
	TMcMmb *ptMmb ,*ptFreeMmb;
	ptMmb = m_ptMcMmb;
	while( ptMmb)
	{
		ptFreeMmb = ptMmb;
		dropMulticast( m_sock ,ptMmb->dwMcIP ,ptMmb->dwIfIP );

		ptMmb = ptMmb->ptNext;		
		FreeMcMmb( ptFreeMmb );
	}

	/// SockClose( m_sock );
    CBB::cbb_sock_close(m_sock);
	InitInterVal();
}


bool CDSSocket::JoinMc( uint32_t dwMcIP ,uint32_t dwIfIP )
{
	if( m_sock == INVALID_SOCKET )
	{
		dslog(DS_LOG_ERROR, "CDSSocket join MC.%s with invalid socket!\n",ipString(dwMcIP) );
		return false;
	}

	TMcMmb *ptMmb ;
	ptMmb     = m_ptMcMmb;
	while( ptMmb)
	{
		if( dwMcIP == ptMmb->dwMcIP &&
			dwIfIP == ptMmb->dwIfIP )
		{
			ptMmb->dwRef++;
			return true;
		}
		ptMmb     = ptMmb->ptNext;
	}
	ptMmb = AllocMcMmb();
	if( NULL == ptMmb )
	{
		dslog(DS_LOG_ERROR, "Alloc Mc Mmb failed !\n");
		return false;
	}

	if( ! joinMulticast(m_sock ,dwMcIP,dwIfIP) )
	{
		dslog(DS_LOG_ERROR, "sock %d join multicast %s with if ip %s failed!\n",m_sock ,ipString(dwMcIP),ipString(dwIfIP));
		FreeMcMmb(ptMmb);
		return false;
	}
	ptMmb->dwIfIP = dwIfIP;
	ptMmb->dwMcIP = dwMcIP;
	ptMmb->dwRef  = 1;
	ptMmb->ptNext = m_ptMcMmb;
	m_ptMcMmb     = ptMmb;
	return true;
}


bool CDSSocket::DropMc( uint32_t dwMcIP ,uint32_t dwIfIP )
{
	TMcMmb *ptMmb ,**pptPreMmb;
	ptMmb      = m_ptMcMmb;
    pptPreMmb  = &m_ptMcMmb;
	while( ptMmb)
	{
		if( dwMcIP == ptMmb->dwMcIP &&
			dwIfIP == ptMmb->dwIfIP &&
			ptMmb->dwRef > 0 )
		{
			ptMmb->dwRef--;
			if( ptMmb->dwRef==0 )
			{
				dropMulticast( m_sock ,dwMcIP ,dwIfIP );
				*pptPreMmb = ptMmb->ptNext;
				FreeMcMmb( ptMmb );
			}
			return true;
		}
		pptPreMmb = &ptMmb->ptNext;
		ptMmb     = ptMmb->ptNext;		
	}
	return false;
}


bool CDSSocket::SetBC()
{
	const int optval = 1;
	if( m_bBC )
	{
		return true;
	}
	
	m_bBC = ( 0 == setsockopt( m_sock,SOL_SOCKET,SO_BROADCAST,(char*)&optval, sizeof(optval)));
	return m_bBC;
}


void CDSSocket::Show()
{
	TMcMmb *ptMmb ;
	ptMmb = m_ptMcMmb;
	
	OspPrintf( TRUE ,FALSE ,"Socket (%d) %s:%d %s\n",m_sock ,
								ipString(m_dwIfIP),
								m_wPort,
								m_bBC ? "Support broadcast":"");
	while( ptMmb)
	{
		OspPrintf( TRUE ,FALSE ,"\t mc %s refer %d times\n",ipString(ptMmb->dwMcIP),ptMmb->dwRef);
		ptMmb     = ptMmb->ptNext;		
	}
}


void CDSSocket::ShowMc( uint32_t dwIfIP )
{
	TMcMmb *ptMmb ;
	ptMmb      = m_ptMcMmb;
 	while( ptMmb)
	{
		if( dwIfIP == ptMmb->dwIfIP )
		{
			OspPrintf( TRUE ,FALSE ,"\t mc %s refer %d times\n",
									ipString(ptMmb->dwMcIP),
									ptMmb->dwRef);
		}
		ptMmb     = ptMmb->ptNext;		
	}
}



/////////////////////////////////////////
/**
 * 登记指定交换模块中的指定IP。
 */
bool CDSInterface::Add( uint32_t dwIfIP ,DSID dsId )
{
	uint32_t dwIfIdx;
	//遍历整个数组，若IP已经存在，则将对应的DSID值置为TRUE，并增加引用计数
	//若对应的DSID已为TRUE，则不做任何操作，返回TRUE
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( m_atIPIf[dwIfIdx].dwIP == dwIfIP )
		{
			if( !m_atIPIf[dwIfIdx].dsId[dsId] )
			{
				m_atIPIf[dwIfIdx].dsId[dsId] = true;
				m_atIPIf[dwIfIdx].dwRef++;
			}
			dslog(DS_LOG_DEBUG, "The IfIP(%s) exist !\n",ipString( dwIfIP ) );
			return true;
		}
	}

//	//检测该IP是否为本机IP
//	if(!OspIsLocalHost( dwIfIP ) )
//	{
//		dsLog("The IfIP(%s) is not local host IP addr!\n",ipString( dwIfIP ) );
//		return false;
//	}
//
	//指定IP未登记。找到一个空闲位置，登记该IP
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( m_atIPIf[dwIfIdx].dwRef == 0 )
		{
		#ifdef _VXWORKS_
			m_atIPIf[dwIfIdx].lIfIdx = if2_ip2index( dwIfIP );
            dslog(DS_LOG_DEBUG, "Vx: The IfIP(%s) index is %d!\n",ipString( dwIfIP ),m_atIPIf[dwIfIdx].lIfIdx );
		#else
			m_atIPIf[dwIfIdx].lIfIdx  = (dwIfIdx+1);
            dslog(DS_LOG_DEBUG, "Win: The IfIP(%s) index is %d!\n",ipString( dwIfIP ),m_atIPIf[dwIfIdx].lIfIdx );
		#endif

			if( INVALID_IFIDX == m_atIPIf[dwIfIdx].lIfIdx )
			{
				dslog(DS_LOG_DEBUG, "Get if idex failed!\n");
				return false;
			}
            m_atIPIf[dwIfIdx].dwIP = dwIfIP;
            m_atIPIf[dwIfIdx].dsId[dsId] = true;
			m_atIPIf[dwIfIdx].dwRef++;
			return true;
		}
	}
    dslog(DS_LOG_DEBUG, "CDSInterface::Add can't find free space save if ip index !\n" );
	return false;
}


/**
 * 删除指定交换模块中所有IP
 */
void CDSInterface::Remove( DSID dsId )
{
	uint32_t dwIfIdx;
	
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( m_atIPIf[dwIfIdx].dsId[dsId] )
		{
			m_atIPIf[dwIfIdx].dsId[dsId] = false;
			m_atIPIf[dwIfIdx].dwRef--;
		}
	}
}


/**
 * 根据IP，获取其索引值
 */
long CDSInterface::GetIfIdx( uint32_t dwIfIP )
{
	uint32_t dwIfIdx;
	if((dwIfIP == 0) || (dwIfIP == 0xffffffff))
		return INVALID_IFIDX;
		
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( m_atIPIf[dwIfIdx].dwIP == dwIfIP )
		{
			return m_atIPIf[dwIfIdx].lIfIdx;
		}
	}
	dslog(DS_LOG_DEBUG, "Can't find %s 's interface index!\n",ipString(dwIfIP) );
	return INVALID_IFIDX;
}


/**
 * 获取指定交换模块中的所有IP
 */
int  CDSInterface::GetIP( DSID dsId ,uint32_t adwIP[] ,uint32_t dwArrySize)
{
	uint32_t dwIfIdx ,num;
	num =0;
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( num >= dwArrySize )
		{
			return num;
		}
		
		if( m_atIPIf[dwIfIdx].dsId[dsId] )
		{
			adwIP[num] = m_atIPIf[dwIfIdx].dwIP;
			num++;
		}
	}
	return num;
}


/**
 * 获取所有已登记IP的列表
 */
int  CDSInterface::GetIP( uint32_t adwIP[] ,uint32_t dwArrySize)
{
	uint32_t dwIfIdx ,num;
	num =0;
	for( dwIfIdx=0 ;dwIfIdx < MAXNUM_DSINTERFACE ;dwIfIdx++ )
	{
		if( num >= dwArrySize )
		{
			return num;
		}
		
		if( m_atIPIf[dwIfIdx].dwRef > 0 )
		{
			adwIP[num] = m_atIPIf[dwIfIdx].dwIP;
			num++;
		}
	}
	return num;
}


////////////////////////////////////
DS_API void dsver(void)
{
#ifdef _USE_WHILE_
    static const char* DS_TRANS_MODE = "while";
#else
#ifdef USE_EPOLL
    static const char* DS_TRANS_MODE = "epoll";
#else
    static const char* DS_TRANS_MODE = "select";
#endif
#endif
	OspPrintf(TRUE,FALSE,"DataSwitch Socket (%s) Version: %s\t, Compile at: %s %s\n", DS_TRANS_MODE, DSVERSION, __TIME__, __DATE__);
}


DS_API void dshelp(void)
{
	OspPrintf(TRUE,FALSE,"dsver : print current dataswitch version\n");
	OspPrintf(TRUE,FALSE,"dsinfo : print current dataswitch rules\n");
	OspPrintf(TRUE, FALSE, "ddlevel(uint32_t level) : set log level (0 : none; 1 : error; 2 : debug; 3 : recv; 4 : send; -1 : all)\n");
	OspPrintf(TRUE, FALSE, "dsrcv(const char* szRcvIp, uint16_t wRcvPort) : trace recv\n");
	OspPrintf(TRUE, FALSE, "dssnd(const char* szSndIp, uint16_t wSndPort) : trace send\n");
	OspPrintf(TRUE, FALSE, "dstranslimit(int nTransLimit) : set trans data count limit (0 : print current limit; -1 : unlimited; 1+ : limit)\n");
#ifdef USE_EPOLL
    OspPrintf(TRUE, FALSE, "dscmdopt(bool bOpt) : enable/disable cmd opt\n");
#endif
}


