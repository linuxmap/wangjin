/******************************************************************************
模块名  ： OSP
文件名  ： ospPost.cpp
相关文件：
文件实现功能：OSP 消息传送功能的主要实现文件
作者    ：张文江
版本    ：1.0.02.7.5
--------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
05/24/2003  2.1         向飞        规范化; 内部消息零拷贝
******************************************************************************/
#include "../include/ospsch.h"
#include "../include/zlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

class CDispatchTask;

#define OSP_SOCKS_AUTH_PROTO	1
#define OSP_COMPRESS_LOG_LEVEL 21
u32 g_maxsndpipeint = 0;

extern int g_max_msg_waiting;
extern u32 max_inst_entry_interval;

u32 g_maxsend_interval = 0;
u32 g_maxrecv_interval = 0;

TASKID g_dwPostDaemonTaskId;

API void DispatchTaskEntry(CDispatchTask *pDispTask);

/*=============================================================================
函 数 名：OspEnableBroadcastAck
功    能：大多数情况下，用户发送广播消息并不需要对方返回确认消息。缺省情况下，
          PIS不向消息的发送者发送OSP_BROADCASTACK消息，但是可以通过该函数修改这
          个选项。是否发送OSP_BROADCASTACK是由接受节点决定的。
注    意：
算法实现：
全局变量：
参    数：u16 aid : [in] 表示广播消息的接受者AID。
          BOOL32 bEnable : [in] enable or not
返 回 值：BOOL32 - true Succeeded, false Failed
=============================================================================*/
API BOOL32 OspEnableBroadcastAck(u16, BOOL32)
{
    return FALSE;
}

/*====================================================================
函 数 名：CreateTcpNodeNoRegist
功能：在地址uAddr和端口uPort上创建一个侦听套接字，该套接字不注册到结点
      池中。
算法实现：（可选项）
引用全局变量：无
输入参数说明：uAddr: 本地IP地址，uPort: 端口号

  返回值说明：成功返回可用的套接字, 失败返回INVALID_SOCKET.
====================================================================*/
API SOCKHANDLE CreateTcpNodeNoRegist(u32, u16 wPort,  BOOL32 bTcpNodeReuse)
{
    SOCKHANDLE tSock = INVALID_SOCKET;
    SOCKADDR_IN tSvrINAddr;
    u32 optVal = 0;

    memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );

    // set up the local address
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wPort);
    tSvrINAddr.sin_addr.s_addr = INADDR_ANY;

    //Allocate a socket
    tSock = socket(AF_INET, SOCK_STREAM, 0);
    if(tSock == INVALID_SOCKET)
    {
//        OspPrintf(TRUE, TRUE, "\nOsp: Tcp server can't create socket!\n");
        OspLog(1, "\nOsp: Tcp server can't create socket!\n");
        return INVALID_SOCKET;
    }

    /*set the sock can reuserd immediated*/
    if(bTcpNodeReuse)
    {
        optVal = 1;
        setsockopt(tSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));
    }

	if(bind(tSock, (SOCKADDR *)&tSvrINAddr, sizeof(tSvrINAddr)) == SOCKET_ERROR)
	{
//		OspPrintf(TRUE, TRUE, "\nOsp: PassiveTcp: bind error!\n");
		OspLog(1, "\nOsp: PassiveTcp: bind error!\n");
		SockClose(tSock);
		return INVALID_SOCKET;
	}

	if(listen(tSock, 15) == SOCKET_ERROR) // max 15 waiting connection
	{
//		OspPrintf(TRUE, TRUE, "\nOsp: PassiveTcp can't listen on port = %d!\n", wPort);
		OspLog(1, "\nOsp: PassiveTcp can't listen on port = %d!\n", wPort);
		SockClose(tSock);
		return INVALID_SOCKET;
	}

	return tSock;
}

/*=============================================================================
函 数 名：OspConnectToSock5Proxy
功	  能：与sock5代理服务器建立TCP链路（无论TCP或UDP穿越sock5代理都必须先建立该TCP链路）
注	  意：
算法实现：
全局变量：
参	  数：ptOspSock5Proxy : [in] 代理服务器信息结构
		  dwTimeoutMs : [in] 操作超时时间
返 回 值：失败返回INVALID_SOCKET；
		  成功返回与代理通信的TCP Socket，可进一步调用OspConnectTcpNodeThroughSock5Proxy连接TCP服务端
		  或OspUdpAssociateThroughSock5Proxy建立UDP Associate；
		  如果连接TCP服务端成功，主动断链请调用OspDisconnectTcpNode；
		  如果连接TCP服务端失败，osp内部调用OspDisconnectFromSock5Proxy释放OspConnectToSock5Proxy得到的socket；
		  如果是建立UDP Associate，需要上层维护本TCP连接，主动断链请调用OspDisconnectFromSock5Proxy；
-------------------------------------------------------------------------------
 修改纪录：
 日 	 期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
API SOCKHANDLE OspConnectToSock5Proxy( TOspSock5Proxy* ptOspSock5Proxy , u32 dwTimeoutMs, u8 bySocksAuthVerion)
{
	u32 optVal = 0;
    SOCKHANDLE tSock = INVALID_SOCKET;
    SOCKADDR_IN tSvrINAddr;
	struct timeval tTimeVal;
	struct timeval *ptTimeVal = NULL;
	u8 byIndex = 0;
	u8 abyMsgBuffer[0xff] = {0};
	fd_set tWaitFd;
    s32 nRet = 0;
	u8 version;

	version = bySocksAuthVerion; // just for avoiding compile warning

	if( NULL == ptOspSock5Proxy )
	{
		return INVALID_SOCKET;
	}

	if( ptOspSock5Proxy->m_byAuthenNum == 0 )
	{
		OspLog(1, "Osp: OspConnectToSock5Proxy() authenticate method Num incorrect!\n");
		return INVALID_SOCKET;
	}

	for( byIndex = 0 ; byIndex < ptOspSock5Proxy->m_byAuthenNum ; byIndex++ )
	{
		//如果支持用户名密码鉴权，判断有效性
		if( SOCK5_PROXY_AUTHEN_USERNAME_PASSWORD == ptOspSock5Proxy->m_abyAuthenMethod[byIndex] )
		{
			if(( MAX_SOCK5PROXY_NAME_PASS_LENGTH < strlen(ptOspSock5Proxy->m_achUseName) ) ||
				( MAX_SOCK5PROXY_NAME_PASS_LENGTH < strlen(ptOspSock5Proxy->m_achPassword) ) )
			{
				OspLog(1, "Osp: OspConnectToSock5Proxy() UserName or Password length incorrect!\n");
				return INVALID_SOCKET;
			}
		}
	}

    memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(ptOspSock5Proxy->m_wProxyPort);
    tSvrINAddr.sin_addr.s_addr = ptOspSock5Proxy->m_dwProxyIP;

    //创建一个 socket
    tSock = socket( AF_INET, SOCK_STREAM, 0 );
    if( tSock == INVALID_SOCKET )
    {
        OspLog(1, "Osp: OspConnectToSock5Proxy() create socket failed!\n");
        return INVALID_SOCKET;
    }

    OspLog( 21, "Osp: OspConnectToSock5Proxy %s@%d, please wait...\n", inet_ntoa( tSvrINAddr.sin_addr ), ptOspSock5Proxy->m_wProxyPort );

	//连接服务结点
	u32 on= 1;
	if (ioctl(tSock, FIONBIO, (ioctlOnPtrTypeCast)&on) < 0)
    {
		perror("Osp: fcntl regist nonblock failed.\n");
	}

    if( connect(tSock, (SOCKADDR*)&tSvrINAddr, sizeof(tSvrINAddr)) == SOCKET_ERROR )
    {
		struct timeval tm;
		fd_set set;
		BOOL32 bRet = FALSE;
		s32 optVal;
		s32 len =sizeof(optVal);
		if(dwTimeoutMs<1000)
		{
			dwTimeoutMs = 1000;
		}
        tm.tv_sec  = dwTimeoutMs/1000;;
        tm.tv_usec = (dwTimeoutMs%1000)*1000;
        FD_ZERO(&set);
        FD_SET(tSock, &set);
		if( select(tSock+1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(tSock, SOL_SOCKET, SO_ERROR, (char*)&optVal, &len);
			if(optVal == 0)
				bRet = TRUE;
			else
				bRet = FALSE;
		}
		else
			bRet = FALSE;

		if(bRet == FALSE)
		{
			OspLog(1, "Osp: OspConnectToSock5Proxy() connecting failed!\n");
			SockClose(tSock);
			return INVALID_SOCKET;
		}
    }

	on= 1;
	if (ioctl(tSock, FIONBIO, (ioctlOnPtrTypeCast)&on) < 0)
    {
		perror("Osp: fcntl unregist nonblock failed.\n");
	}

	/* 设置套接字选项: 立即发送(不采用任何避免竞争的算法) */
    optVal = 1;
    setsockopt(tSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optVal, sizeof(optVal));

	/* 设置套接字选项: 设置套接口发送接收缓冲的大小 */
	optVal = SOCKET_SEND_BUF;
    setsockopt(tSock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, sizeof(optVal));
	optVal = SOCKET_RECV_BUF;
    setsockopt(tSock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));

	/* 设置套接字选项: 启用TCP连接检测功能 */
	optVal = 1;
    setsockopt(tSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optVal, sizeof(optVal));


	OspLog(21, "Osp: OspConnectToSock5Proxy %s@%d OK!\n", inet_ntoa( tSvrINAddr.sin_addr ), ptOspSock5Proxy->m_wProxyPort );

	//发送鉴权请求
	abyMsgBuffer[0] = SOCK5_PROXY_VERSION;
	abyMsgBuffer[1] = ptOspSock5Proxy->m_byAuthenNum;
	for( byIndex = 0 ; byIndex < ptOspSock5Proxy->m_byAuthenNum ; byIndex++ )
	{
		abyMsgBuffer[2+byIndex] = ptOspSock5Proxy->m_abyAuthenMethod[byIndex];
	}
	if( SOCKET_ERROR == send( tSock , (char*)abyMsgBuffer , 2+ptOspSock5Proxy->m_byAuthenNum , 0 ) )
	{
		OspLog(1, "Osp: OspConnectToSock5Proxy() send authenticate request failed!\n");
		SockClose(tSock);
        return INVALID_SOCKET;
	}

	//等待鉴权回复
	if( dwTimeoutMs > 0 )
	{
		memset(&tTimeVal, 0, sizeof(tTimeVal));
		tTimeVal.tv_sec = dwTimeoutMs/1000;
		tTimeVal.tv_usec = (dwTimeoutMs%1000)*1000;

		ptTimeVal = &tTimeVal;
	}
	FD_ZERO(&tWaitFd);
	FD_SET( tSock , &tWaitFd );
    nRet = select(FD_SETSIZE, &tWaitFd, NULL, NULL, ptTimeVal);
	if( 0 >= nRet )
	{
		OspLog(1, "Osp: OspConnectToSock5Proxy() recv authenticate reply failed! nRet =%d\n",nRet);
        if(nRet < 0)
        {
            OspLog(1,"Osp: OspConnectToSock5Proxy select failed! WSAGetLastError = %d\n", WSAGetLastError);
        }

		SockClose(tSock);
        return INVALID_SOCKET;
	}

	//接收sock5代理支持的鉴权方法
	if( ( 2 != recv( tSock, (char *)abyMsgBuffer, 0xff, 0 ) ) ||
		( SOCK5_PROXY_VERSION != abyMsgBuffer[0] ) )
	{
		OspLog(1, "Osp: OspConnectToSock5Proxy() authenticate reply message unknown!\n");
		SockClose(tSock);
        return INVALID_SOCKET;
	}
	switch( abyMsgBuffer[1] )
	{
	case SOCK5_PROXY_AUTHEN_NO_REQUIERD:
		{
			break;
		}
	//如果需要用户名密码鉴权，发送子认证过程
	case SOCK5_PROXY_AUTHEN_USERNAME_PASSWORD:
		{
			/*abyMsgBuffer[0] = SOCK5_PROXY_VERSION;*/
            abyMsgBuffer[0] = OSP_SOCKS_AUTH_PROTO;
			abyMsgBuffer[1] = (u8)strlen(ptOspSock5Proxy->m_achUseName);
			memcpy( abyMsgBuffer+2 , ptOspSock5Proxy->m_achUseName , strlen(ptOspSock5Proxy->m_achUseName) );
			abyMsgBuffer[2+strlen(ptOspSock5Proxy->m_achUseName)] = (u8)strlen(ptOspSock5Proxy->m_achPassword);
			memcpy( abyMsgBuffer+3+strlen(ptOspSock5Proxy->m_achUseName) , ptOspSock5Proxy->m_achPassword ,
				strlen(ptOspSock5Proxy->m_achPassword) );
			if( SOCKET_ERROR == send( tSock , (char*)abyMsgBuffer ,
				3+strlen(ptOspSock5Proxy->m_achUseName)+strlen(ptOspSock5Proxy->m_achPassword) , 0 ) )
			{
				OspLog(1, "Osp: OspConnectToSock5Proxy() send detailed authenticate request failed!\n");
				SockClose(tSock);
				return INVALID_SOCKET;
			}

			//等待鉴权回复
            nRet = select(FD_SETSIZE, &tWaitFd, NULL, NULL, ptTimeVal);
			if( 0 >= nRet )
			{
				OspLog(1, "Osp: OspConnectToSock5Proxy() recv detailed authenticate reply failed! nRet =%d\n",nRet);

                if(nRet < 0)
                {
                    OspLog(1,"Osp: OspConnectToSock5Proxy select again failed! WSAGetLastError = %d\n", WSAGetLastError);
                }

				SockClose(tSock);
				return INVALID_SOCKET;
			}

			//用户名鉴权是否成功
			if( ( 2 != recv( tSock, (char *)abyMsgBuffer, 0xff, 0 ) ) ||
				/*( SOCK5_PROXY_VERSION != abyMsgBuffer[0] ) ||*/
                ( OSP_SOCKS_AUTH_PROTO != abyMsgBuffer[0] ) ||
				( SOCK5_PROXY_SUCCESS != abyMsgBuffer[1] ) )
			{
				OspLog(1, "Osp: OspConnectToSock5Proxy() detailed authenticate UserName or Password incorrect!\n");
				SockClose(tSock);
				return INVALID_SOCKET;
			}
			break;
		}
	default:
		{
			OspLog(1, "Osp: OspConnectToSock5Proxy() authenticate be denied!\n");
			SockClose(tSock);
			return INVALID_SOCKET;
		}
	}

	OspLog(21, "Osp: OspConnectToSock5Proxy() authenticate success!\n\n");

	return tSock;
}

/*=============================================================================
函 数 名：OspConnectTcpNodeThroughSock5Proxy
功	  能：TCP穿越sock5代理连接服务端（与OspConnectTcpNode相同）
注	  意：
算法实现：
全局变量：
参	  数：ptOspSock5Proxy : [in] 代理服务器信息结构;
		  dwServerIP : [in] 服务器IP
		  wServerPort : [in] 服务器端口
		  wHb: [in] 断链检测周期(秒)
		  byHbNum: [in] byHbNum次未到连接检测应答后认为链路已断开
		  dwTimeoutMs : [in] 操作超时时间
		  pdwLocalIP: [in,out] 本TCP链接使用的本地IP
返 回 值：失败返回INVALID_NODE；
		  成功返回一个动态分配的结点号, 此后用户可借此与服务结点通信
		  上层主动断链需调用OspDisconnectTcpNode，其他与无代理时无区别
-------------------------------------------------------------------------------
 修改纪录：
 日 	 期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
API int OspConnectTcpNodeThroughSock5Proxy( TOspSock5Proxy* ptOspSock5Proxy , u32 dwServerIP, u16 wServerPort,
									u16 wHb, u8 byHbNum , u32 dwTimeoutMs , u32 *pdwLocalIP , u8 bySocksAuthVerion )
{
	u32 nodeId = 0;
	SOCKHANDLE hSocket = INVALID_SOCKET;
	SOCKADDR_IN tSvrINAddr;
	struct timeval tTimeVal;
	struct timeval *ptTimeVal = NULL;
	u8 abyMsgBuffer[0xff] = {0};
	fd_set tWaitFd;
    s32 nRet = 0;

	if( NULL == ptOspSock5Proxy )
	{
		return INVALID_NODE;
	}

	hSocket = OspConnectToSock5Proxy( ptOspSock5Proxy , dwTimeoutMs ,bySocksAuthVerion );
	if( INVALID_SOCKET == hSocket )
	{
		return INVALID_NODE;
	}

	memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wServerPort);
    tSvrINAddr.sin_addr.s_addr = dwServerIP;


    OspLog( 1, "Osp: OspConnectTcpNodeThroughSock5Proxy to %s@%d, please wait...\n", inet_ntoa( tSvrINAddr.sin_addr ), wServerPort );

	//发送穿越sock5代理连接请求
	abyMsgBuffer[0] = SOCK5_PROXY_VERSION;
	abyMsgBuffer[1] = SOCK5_PROXY_CMD_TCP_CONNECT;
	abyMsgBuffer[2] = SOCK5_PROXY_RESERVED_DATA;
	abyMsgBuffer[3] = SOCK5_PROXY_IPV4_ADDR;
	memcpy( abyMsgBuffer+4 , &dwServerIP , 4 );
	wServerPort = htons(wServerPort);
	memcpy( abyMsgBuffer+8 , &wServerPort , 2 );
	if( SOCKET_ERROR == send( hSocket , (char*)abyMsgBuffer , 10 , 0 ) )
	{
		OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy() send connect request failed!\n");
		OspDisConnectFromSock5Proxy(hSocket);
        return INVALID_NODE;
	}

	//等待连接请求回复
	if( dwTimeoutMs > 0 )
	{
		memset(&tTimeVal, 0, sizeof(tTimeVal));
		tTimeVal.tv_sec = dwTimeoutMs/1000;
		tTimeVal.tv_usec = (dwTimeoutMs%1000)*1000;

		ptTimeVal = &tTimeVal;
	}
	FD_ZERO(&tWaitFd);
	FD_SET( hSocket , &tWaitFd );
    nRet = select(FD_SETSIZE, &tWaitFd, NULL, NULL, ptTimeVal);
	if( 0 >= nRet )
	{
		OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy() recv connect reply failed! nRet =%d\n",nRet);

        if(nRet < 0)
        {
            OspLog(1,"Osp: OspConnectTcpNodeThroughSock5Proxy select failed! WSAGetLastError = %d\n", WSAGetLastError);
        }

		OspDisConnectFromSock5Proxy(hSocket);
        return INVALID_NODE;
	}
	if( ( 10 != recv( hSocket, (char *)abyMsgBuffer, 0xff, 0 ) ) ||
		( SOCK5_PROXY_VERSION != abyMsgBuffer[0] ) ||
		( SOCK5_PROXY_SUCCESS != abyMsgBuffer[1] ) ||
		( SOCK5_PROXY_RESERVED_DATA != abyMsgBuffer[2] ) ||
		( SOCK5_PROXY_IPV4_ADDR != abyMsgBuffer[3] ) )
	{
		OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy() connect failed!\n");
		OspDisConnectFromSock5Proxy(hSocket);
        return INVALID_NODE;
	}

	/* 为连接成功的套接字在结点池中分配一个的结点号 */
    if( !g_Osp.m_cNodePool.Regist( 0 , hSocket, &nodeId, wHb, byHbNum ) )
	{
		OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy() regist socket failed, close it.\n");
		OspDisConnectFromSock5Proxy(hSocket);
		return INVALID_NODE;
	}


    // 获取当前连接所用的IP地址
    if( pdwLocalIP != NULL )
    {
        struct sockaddr_in tClientAddr;
        int length = sizeof(tClientAddr);
        if( getsockname(hSocket, (struct sockaddr *)&tClientAddr, &length ) == 0 )
        {
            *pdwLocalIP = (u32) tClientAddr.sin_addr.s_addr;
        }
    }

	OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy to %s@%d OK, the nodeid = %d!\n\n",
		inet_ntoa( tSvrINAddr.sin_addr ), ntohs(wServerPort) , nodeId );

    OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, nodeId); // 创建node后通知对方本方支持压缩消息 by wubin 2011-02-22
	SockChangeNotify();
    return nodeId;
}


/*=============================================================================
函 数 名：OspUdpAssociateThroughSock5Proxy
功	  能：UDP穿越sock5代理
注	  意：
算法实现：
全局变量：
参	  数：hSocket : [in] OspConnectToSock5Proxy返回的socket(可复用);
		  dwLocalIP : [in] 本地收发socket IP，以便代理服务器限制数据穿越（网络序）
		  wLocaPort : [in] 本地收发socket 端口，以便代理服务器限制数据穿越（主机序）
		  pdwProxyMapIP : [out] 代理服务器映射IP（网络序）
		  pwProxyMapPort : [out] 代理服务器映射端口（主机序）
		  dwTimeoutMs : [in] 操作超时时间
返 回 值：失败返回FALSE；
		  成功返回TRUE
-------------------------------------------------------------------------------
 修改纪录：
 日 	 期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
API BOOL32 OspUdpAssociateThroughSock5Proxy( SOCKHANDLE hSocket , u32 dwLocalIP, u16 wLocalPort ,
									u32* pdwProxyMapIP, u16* pwProxyMapPort , u32 dwTimeoutMs )
{
	SOCKADDR_IN tSvrINAddr;
	struct timeval tTimeVal;
	struct timeval *ptTimeVal = NULL;
	u8 abyMsgBuffer[0xff] = {0};
	fd_set tWaitFd;
	u32 dwProxyMapIP = 0;
	u16 wProxyMapPort = 0;
    s32 nRet = 0;

	memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wLocalPort);
    tSvrINAddr.sin_addr.s_addr = dwLocalIP;


    OspLog( 1, "Osp: OspUdpAssociateThroughSock5Proxy from local %s@%d, please wait...\n",
		inet_ntoa( tSvrINAddr.sin_addr ), wLocalPort );

	//发送UDP Associate请求
	abyMsgBuffer[0] = SOCK5_PROXY_VERSION;
	abyMsgBuffer[1] = SOCK5_PROXY_CMD_UDP_ASSOCIATE;
	abyMsgBuffer[2] = SOCK5_PROXY_RESERVED_DATA;
	abyMsgBuffer[3] = SOCK5_PROXY_IPV4_ADDR;
	memcpy( abyMsgBuffer+4 , &dwLocalIP , 4 );
	wLocalPort = htons(wLocalPort);
	memcpy( abyMsgBuffer+8 , &wLocalPort , 2 );
	if( SOCKET_ERROR == send( hSocket , (char*)abyMsgBuffer , 10 , 0 ) )
	{
		OspLog(1, "Osp: OspUdpAssociateThroughSock5Proxy() send udp associate request failed!\n");
        return FALSE;
	}

	//等待连接请求回复
	if( dwTimeoutMs > 0 )
	{
		memset(&tTimeVal, 0, sizeof(tTimeVal));
		tTimeVal.tv_sec = dwTimeoutMs/1000;
		tTimeVal.tv_usec = (dwTimeoutMs%1000)*1000;

		ptTimeVal = &tTimeVal;
	}
	FD_ZERO(&tWaitFd);
	FD_SET( hSocket , &tWaitFd );
    nRet = select(FD_SETSIZE, &tWaitFd, NULL, NULL, ptTimeVal);
	if( 0 >= nRet )
	{
		OspLog(1, "Osp: OspUdpAssociateThroughSock5Proxy() recv udp associate reply failed! nRet =%d\n",nRet);

        if(nRet < 0)
        {
            OspLog(1,"Osp: OspUdpAssociateThroughSock5Proxy select failed! WSAGetLastError = %d\n", WSAGetLastError);
        }

        return FALSE;
	}
	if( ( 10 != recv( hSocket, (char *)abyMsgBuffer, 0xff, 0 ) ) ||
		( SOCK5_PROXY_VERSION != abyMsgBuffer[0] ) ||
		( SOCK5_PROXY_SUCCESS != abyMsgBuffer[1] ) ||
		( SOCK5_PROXY_RESERVED_DATA != abyMsgBuffer[2] ) ||
		( SOCK5_PROXY_IPV4_ADDR != abyMsgBuffer[3] ) )
	{
		OspLog(1, "Osp: OspUdpAssociateThroughSock5Proxy() udp associate failed!\n");
        return FALSE;
	}

	memcpy( &dwProxyMapIP , abyMsgBuffer+4 , 4 );
	memcpy( &wProxyMapPort , abyMsgBuffer+8 , 2 );
	wProxyMapPort = ntohs(wProxyMapPort);

	memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = wProxyMapPort;
    tSvrINAddr.sin_addr.s_addr = dwProxyMapIP;

	OspLog(1, "Osp: OspUdpAssociateThroughSock5Proxy to %s@%d OK!\n\n",
		inet_ntoa( tSvrINAddr.sin_addr ), wProxyMapPort );

	if( NULL != pdwProxyMapIP )
	{
		*pdwProxyMapIP = dwProxyMapIP;
	}
	if( NULL != pwProxyMapPort )
	{
		*pwProxyMapPort = wProxyMapPort;
	}

	return TRUE;
}

/*=============================================================================
函 数 名：OspDisConnectFromSock5Proxy
功	  能：穿越sock5代理连接服务端
注	  意：
算法实现：
全局变量：
参	  数：hSocket : [in] OspConnectToSock5Proxy返回的socket;
返 回 值：失败返回FALSE；
		  成功返回TRUE
-------------------------------------------------------------------------------
 修改纪录：
 日 	 期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
API BOOL32 OspDisConnectFromSock5Proxy( SOCKHANDLE hSocket )
{
	if( INVALID_SOCKET == hSocket )
		return FALSE;
	SockClose( hSocket );
	return TRUE;
}

/*====================================================================
函数名：OspCreateTcpNode
功能：在地址uAddr和端口uPort上创建一个侦听套接字并将该套接字注册到结点
      池中。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAddr: 本地IP地址，uPort: 端口号

返回值说明：成功返回可用的套接字, 失败返回INVALID_SOCKET.
====================================================================*/
API SOCKHANDLE OspCreateTcpNode(u32 dwAddr, u16 wPort,  BOOL32 bTcpNodeReuse)
{
	if(g_Osp.m_cNodePool.m_tListenSock != INVALID_SOCKET)
	{
//		OspPrintf(TRUE, TRUE, "Osp: OspCreateTcpNode() failed: create server node twice NOT allowed.\n");
		OspLog(1, "Osp: OspCreateTcpNode() failed: create server node twice NOT allowed.\n");
		return INVALID_SOCKET;
	}

    SOCKHANDLE sock = CreateTcpNodeNoRegist(dwAddr, wPort,  bTcpNodeReuse);
	if(sock == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	g_Osp.m_cNodePool.m_tListenSock = sock;
	g_Osp.m_cNodePool.m_wListenPort = wPort;

	SockChangeNotify();
	return sock;
}

/*=============================================================================
函 数 名：OspConnectTcpNode
功    能：在地址uIpv4Addr和端口uTcpPort上连接服务结点, 并设置断链检测参数.
注    意：
算法实现：
全局变量：g_Osp
参    数：uIpv4Addr : [in] 服务结点的IP地址,
          uTcpPort : [in] 服务结点的侦听端口号,
		  uHb: [in] 断链检测周期(秒),
          uHbNum: [in] uHbNum次未后到连接检测应答后认为链路已断开.

返 回 值：成功返回一个动态分配的结点号, 此后用户可借此与服务结点通信.
          失败返回INVALID_NODE.
=============================================================================*/
API int OspConnectTcpNode(u32 dwIpv4Addr, u16 wTcpPort, u16 wHb, u8 byHbNum, u32 dwTimeOutMs, u32 *pdwLocalIP)
{
	u32 optVal;
	int set_result;
    u32 nodeId;
    int win_error;
    SOCKHANDLE tSock = INVALID_SOCKET;
    SOCKADDR_IN tSvrINAddr;
    SOCKADDR_IN tSelfINAddr;

    memset(&tSvrINAddr, 0, sizeof(tSvrINAddr));
    memset(&tSelfINAddr, 0, sizeof(tSelfINAddr));

	/* 准备SERVER地址 */
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wTcpPort);
    tSvrINAddr.sin_addr.s_addr = dwIpv4Addr;

    /* 创建一个 socket */
    tSock = socket(AF_INET, SOCK_STREAM, 0);
    if(tSock == INVALID_SOCKET)
    {
//        OspPrintf(TRUE, TRUE, "Osp: OspConnectTcpNode() create socket failed!\n");
        OspLog(1, "Osp: OspConnectTcpNode() create socket failed!\n");
        return INVALID_NODE;
    }

//    OspPrintf(TRUE, TRUE, "Osp: connecting to %s@%d, please wait...\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort);
    OspLog(1, "Osp: connecting to %s@%d, please wait...\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort);


	/* 连接服务结点 */

	int ret;
	//设置非阻塞方式连接
	unsigned long unblock = 1;

	ret = ioctlsocket(tSock, FIONBIO, (unsigned long*)&unblock);
	if(ret==SOCKET_ERROR)
	{
		OspLog(1, "\nOsp: OspConnectTcpNode() Set Socket NonBlock Failed!\n");
		SockClose(tSock);
		return INVALID_NODE;
	}

	ret = connect(tSock, (SOCKADDR*)&tSvrINAddr, sizeof(tSvrINAddr)) ;

	if(ret == SOCKET_ERROR)
	{	/* if connect error */
		if (WSAGetLastError()!= WSAEWOULDBLOCK)
		{
			OspLog(1, "\nOsp: OspConnectTcpNode() Connecting Failed!\n");
			SockClose(tSock);
			return INVALID_NODE;
		}
		else
		{
			struct timeval tTimeVal;
			struct timeval *pTimeVal;
			int bWait;
			fd_set rset, wset, eset;
			int error = 0;
			int len = sizeof(error);

			FD_ZERO(&rset);
			FD_SET(tSock, &rset);

			wset = rset;
			eset = rset;
			if (dwTimeOutMs == 0){
				pTimeVal = NULL;
			}
			else
			{
				tTimeVal.tv_sec = dwTimeOutMs/1000;
				tTimeVal.tv_usec = (dwTimeOutMs%1000)*1000;
				pTimeVal = &tTimeVal;
			}

			bWait = select(tSock + 1, &rset, &wset, &eset, pTimeVal);
			if (bWait <= 0)
			{
				OspLog(1, "\nOsp: OspConnectTcpNode() Connect TimeOut!\n");
				SockClose(tSock);
				return INVALID_NODE;
			}

			if (getsockopt(tSock, SOL_SOCKET,SO_ERROR, (char *)&error, &len) < 0)
			{
				OspLog(1, "\nOsp: OspConnectTcpNode() Get Socket Error !\n");
				SockClose(tSock);
				return INVALID_NODE;
			}

			if (error > 0)
			{
				OspLog(1, "\nOsp: OspConnectTcpNode() Socket Error %d !\n",error );
				SockClose(tSock);
				return INVALID_NODE;
			}
		}
	}


	unblock = 0;

	ret = ioctlsocket(tSock, FIONBIO, (unsigned long*)&unblock);
	if(ret==SOCKET_ERROR)
	{
		OspLog(1, "\nOsp: OspConnectTcpNode() Set Socket NonBlock Failed!\n");
		SockClose(tSock);
		return INVALID_NODE;
	}


    // 获取当前连接所用的IP地址
    if (pdwLocalIP != NULL)
    {
        struct sockaddr_in tClientAddr;
        int length = sizeof(tClientAddr);
        if (getsockname(tSock, (struct sockaddr *)&tClientAddr, &length) == 0)
        {
            *pdwLocalIP = (u32) tClientAddr.sin_addr.s_addr;
        }
    }

    /* 设置套接字选项: 立即发送(不采用任何避免竞争的算法) */
    optVal = 1;
    set_result = setsockopt(tSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR)
	{
		win_error = WSAGetLastError();
//		OspPrintf(TRUE, TRUE, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
	}

	/* 设置套接字选项: 设置套接口发送缓冲的大小 */
	optVal = SOCKET_SEND_BUF;
    set_result = setsockopt(tSock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR)
	{
		win_error = WSAGetLastError();
//		OspPrintf(TRUE, TRUE, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
	}
	optVal = SOCKET_RECV_BUF;
    set_result = setsockopt(tSock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR)
	{
		win_error = WSAGetLastError();
//		OspPrintf(TRUE, TRUE, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
	}

	/* 设置套接字选项: 启用TCP连接检测功能 */
	optVal = 1;
    set_result = setsockopt(tSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR )
	{
		win_error = WSAGetLastError();
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail, errno %d\n", win_error);
	}

	/* 为连接成功的套接字在结点池中分配一个的结点号 */
    if( !g_Osp.m_cNodePool.Regist(dwIpv4Addr, tSock, &nodeId, wHb, byHbNum) )
	{
		SockClose(tSock);
		OspLog(1, "\nOsp: OspConnectTcpNode() regist socket failed, close it.\n");
		return INVALID_NODE;
	}

    OspLog(1, "Osp: connect to %s@%d OK, the nodeid = %d!\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort, nodeId);

    OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, nodeId); // 创建node后通知对方本方支持压缩消息 by wubin 2011-02-22
	SockChangeNotify();
    return nodeId;
}

/*====================================================================
函数名：OspSetHBParam
功能：设置结点的链路检测参数
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNodeID: 结点号,
              wHb: wHb秒钟检测一次,
			  byHbNum: byHbNum次无应答后, 主动断开.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSetHBParam(u32 dwNodeID, u16 wHb, u8 byHbNum)
{
	return g_Osp.m_cNodePool.SetHBParam(dwNodeID, wHb, byHbNum);
}

/*====================================================================
函数名：OspDisconnectTcpNode
功能：断开在一个node上的连接。仅供app调用
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：dwNodeId: 结点号。

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspDisconnectTcpNode(u32 dwNodeId)
{
    if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
	{
		return FALSE;
	}

	return g_Osp.m_cNodePool.NodeUnRegist(dwNodeId, NODE_DISC_REASON_BYAPP);
}

/*====================================================================
函数名：SockChangeNotify
功能： 通知守护任务, 有新的Sock建立,包含服务Sock,建立新的连接,新的连接进入
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void SockChangeNotify()
{
    CMessageForSocket tMsg;

    tMsg.srcnode = 0;
    tMsg.dstnode = 0;
    tMsg.dstid =0;
    tMsg.srcid = 0;
    tMsg.event =0;
    tMsg.length =0;

    SockSend(g_Osp.m_cNodePool.m_tLocalInSock, (char *)&tMsg, sizeof(CMessageForSocket));
}

/*====================================================================
函数名：OspIsValidTcpNode
功能：判断一个TCP节点是否有效.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uNodeId: 结点号.

  返回值说明：TRUE 有效，FALSE 无效
====================================================================*/
API BOOL32 OspIsValidTcpNode(u32 dwNodeId)
{
	if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
		return FALSE;

    return g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid;
}

/*====================================================================
函数名：OspIsNodeCheckEnable
功能：判断指定结点的链路检测功能是否起用。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uNodeId: 结点号。

  返回值说明：启用返回TRUE, 禁用返回FALSE.
====================================================================*/
API BOOL32 OspIsNodeCheckEnable(u32 dwNodeId)
{
	if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
	{
		OspPrintf(TRUE, FALSE, "Osp: Node ID %d invalid.\n", dwNodeId);
		return FALSE;
	}

	return g_Osp.m_cNodePool.IsNodeCheckEnable(dwNodeId);
}

/*====================================================================
函数名：OspEnableNodeCheck
功能：起用指定结点的链路检测功能。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uNodeId: 结点号。

  返回值说明：无
====================================================================*/
API void OspEnableNodeCheck(u32 dwNodeId)
{
	if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
	{
		OspPrintf(TRUE, FALSE, "Osp: Node ID %d invalid.\n", dwNodeId);
		return;
	}

	g_Osp.m_cNodePool.NodeCheckEnable(dwNodeId);
}

/*====================================================================
函数名：OspDisableNodeCheck
功能：禁用指定结点的链路检测功能。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：dwNodeId: 结点号。

  返回值说明：无
====================================================================*/
API void OspDisableNodeCheck(u32 dwNodeId)
{
	if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
	{
		OspPrintf(TRUE, FALSE, "Osp: Node ID %d invalid.\n", dwNodeId);
		return;
	}

	g_Osp.m_cNodePool.NodeCheckDisable(dwNodeId);
}

/*====================================================================
函数名：OspPost
功能：使用实例ID发送异步消息的C函数
算法实现：（可选项）
引用全局变量：无
输入参数说明：dwDstId: 目标实例的实例ID(app, ins),
              wEvent: 消息号,
              pvContent: 消息体指针,
			  wLength: 消息长度,
			  dwDstNode: 目标实例的结点号,
              dwSrcIId: 源实例ID(app, ins),
              dwSrcNode: 源结点号.

  返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
API int OspPost(u32 dwDstId, u16 wEvent, const void* pvContent, u16 wLength, u32 dwDstNode, u32 dwSrcIId, u32 dwSrcNode, int nTimeout)
{
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

	/* 消息跟踪 */
    bFileTrc = (g_Osp.m_cAppPool.m_wGloFileTrc & TRCEVENT) ? TRUE:FALSE;
    bScrnTrc = (g_Osp.m_cAppPool.m_wGloScrTrc & TRCEVENT) ? TRUE:FALSE;
    if(bFileTrc || bScrnTrc)
    {
        CMessage msg;
        msg.srcnode = dwSrcNode;
        msg.dstnode = dwDstNode;
        msg.dstid = dwDstId;     // app + gate + ins(2)
		msg.srcid = dwSrcIId;
        msg.event = wEvent;      // the name of the message
        msg.length = wLength;      // the length (max 64K) of content of the message
        msg.content = (u8* )pvContent;
		msg.dstAlias = NULL;
		msg.dstAliasLen = 0;
		len = sprintf(achBuf, "message post app: %s = %d\n", NULL, 0);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    }

	/* 调用OspPostMsg()进行实际的发送 */
    return OspPostMsg(dwDstId, wEvent, pvContent, wLength, dwDstNode,
		              dwSrcIId, dwSrcNode, TRUE, MSG_TYPE_ASYNC, nTimeout);
}

/*====================================================================
函数名：OspPost
功能：使用实例别名发送一条异步消息
算法实现：（可选项）
引用全局变量：无
输入参数说明：dstalias: 目标实例的别名,
              aliaslen: 别名长度,
              dstapp: 目标实例所在App号,
              event: 消息号,
              content: 消息体指针,
			  length: 消息长度,
			  dstnode: 目标实例的结点号,
              srciid: 源实例ID(app, ins),
              srcnode: 源结点号.

  返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int OspPost(const char* dstalias, u8 aliaslen, u16 dstapp, u16 event,
		    const void* content, u16 length, u32 dstnode, u32 srciid, u32 srcnode, int nTimeout)
{
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

	/* 消息跟踪 */
    bFileTrc = (g_Osp.m_cAppPool.m_wGloFileTrc & TRCEVENT) ? TRUE:FALSE;
    bScrnTrc = (g_Osp.m_cAppPool.m_wGloScrTrc & TRCEVENT) ? TRUE:FALSE;
    if ( bFileTrc || bScrnTrc )
    {
        CMessage msg;
        msg.srcnode = srcnode;
        msg.dstnode = dstnode;
        msg.dstid = MAKEIID(dstapp,CInstance::INVALID);     // app + gate + ins(2)
		msg.srcid = srciid;
        msg.event = event;      // the name of the message
        msg.length= length;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)content;
		msg.dstAlias = (char *)dstalias;
		msg.dstAliasLen = aliaslen;

		len = sprintf(achBuf, "message post app: %s = %d\n", NULL ,0);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace

	/* 调用OspPostMsg()进行实际的发送 */
    return OspPostMsg(dstalias, aliaslen, dstapp, dstnode, event, content, length, srciid, srcnode, TRUE);
}

/*====================================================================
函数名：SendQueIdFind
功能：取得指定app的邮箱发端句柄
算法实现：（可选项）
引用全局变量：
输入参数说明：uAppId: app号

  返回值说明：成功返回有效的邮箱ID, 失败返回0.
====================================================================*/
API MAILBOXID SendQueIdFind(u16 wAppId)
{
    return g_Osp.m_cAppPool.SendQueIdFind(wAppId);
}

/*====================================================================
函数名：RcvQueIdFind
功能：取得指定app的邮箱收端句柄
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：wAppId: app号

  返回值说明：成功返回有效的邮箱ID, 失败返回0.
====================================================================*/
API MAILBOXID RcvQueIdFind(u16 wAppId)
{
    return g_Osp.m_cAppPool.RcvQueIdFind(wAppId);
}

/*====================================================================
函数名：NodeMsgRcv
功能：从Node对应的socket上接收信息, 并转发到内部App
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：dwNodeId: 结点号,
              tNodeSock: 要接收消息的套接字.

  返回值说明：无
====================================================================*/
API void NodeMsgRcv(SOCKHANDLE tNodeSock, u32 dwNodeId)
{
	int len;
    u16 dstAppId;
	CApp *pApp;
	MAILBOXID dstSendQueId;
	char achBuf[MAX_LOG_MSG_LEN];
	CMessage *pcMsg;
	CMessageForSocket *pcMsg4S = NULL;
	u32 before, after, tmp;
	u32 dwHeadLen = CMessageForSocket::GetPackedSize();

	/* 检索出相应的结点 */
	CNodeItem *pNode;
	pNode = g_Osp.m_cNodePool.NodeGet(dwNodeId);
	if(pNode == NULL)
	{
/*		OspPrintf(TRUE, FALSE, "Osp: Node %d not exist but message received.\n", dwNodeId);*/
        u32 dwip=OspNodeLastIpGet(dwNodeId);
        OspLog(255, "Osp: Node %d (%u.%u.%u.%u) not exist but message received.\n", dwNodeId, OspQuadAddr(dwip));
        g_Osp.m_cNodePool.m_dwGloFailPostApp++;
		return;
	}

	/* 检查消息缓冲 */
	u32 dwRcvdLen = pNode->GetRcvdLen();
	void *pvRcvdData = pNode->GetRcvdData();
	u32 dwRcvLen = 0;

	if(dwRcvdLen == 0)
	{
		if (pvRcvdData != NULL)
		{
            u32 dwip=OspNodeLastIpGet(dwNodeId);
			OspPrintf(TRUE, FALSE, "Osp: Node %d (%u.%u.%u.%u) message buffer is not empty when a new message arrives.\n", dwNodeId, OspQuadAddr(dwip));
			return;
		}
		pvRcvdData = OspAllocMem(sizeof(CMessageForSocket));
		if (pvRcvdData == NULL)
		{
            u32 dwip=OspNodeLastIpGet(dwNodeId);
			OspPrintf(TRUE, FALSE, "Osp: Node %d (%u.%u.%u.%u) alloc memory failed.\n", dwNodeId, OspQuadAddr(dwip));
			return;
		}
		memset((void *)pvRcvdData, 0, sizeof(CMessageForSocket));
    	pNode->SetRcvdData(pvRcvdData);
	}
	pcMsg4S = (CMessageForSocket *)pvRcvdData;

	/* 消息头未接收，或者未接收完毕，则接收消息头 */
	if(dwRcvdLen < dwHeadLen)
	{

		before = OspTickGet();
		if( !SockRecv(tNodeSock, (char *)pvRcvdData+dwRcvdLen, dwHeadLen-dwRcvdLen, &dwRcvLen) )
		{

			after = OspTickGet();
			tmp = after - before;
			if (tmp > g_maxrecv_interval){
				g_maxrecv_interval = tmp;
			}

			OspFreeMem(pvRcvdData);
			pNode->SetRcvdData(NULL);
			pNode->SetRcvdLen(0);
            u32 dwip=OspNodeLastIpGet(dwNodeId);
			OspPrintf(TRUE, TRUE, "Osp: node %d (%u.%u.%u.%u) recv msg fail, will be deleted. SockErr=%d\n", dwNodeId, OspQuadAddr(dwip), errno);
			g_Osp.m_cNodePool.NodeUnRegist(dwNodeId, NODE_DISC_REASON_RECVERR); // add by xiang
			return;
		}

		after = OspTickGet();
		tmp = after - before;
		if (tmp > g_maxrecv_interval){
			g_maxrecv_interval = tmp;
		}

		dwRcvdLen += dwRcvLen;
		/* 若消息头未按收完整，则退出 */
		if( dwRcvdLen < dwHeadLen )
		{
			pNode->SetRcvdLen(dwRcvdLen);
			return;
		}
		/* 消息头接收完整，处理消息头 */
		/* 将消息头部转换为主机字节序 */
		dwRcvdLen = sizeof(CMessageForSocket);
		MsgNtoh(pcMsg4S);
		/* 转换消息的源, 目的节点号 */
		MsgIdChange(pcMsg4S, dwNodeId);
		if( (pcMsg4S->length > MAX_MSG_LEN) || (pcMsg4S->dstAliasLen > MAX_ALIAS_LEN) )
		{
			OspFreeMem(pvRcvdData);
			pNode->SetRcvdData(NULL);
			pNode->SetRcvdLen(0);
            u32 dwip=OspNodeLastIpGet(dwNodeId);
			OspPrintf(TRUE, TRUE, "Osp: NodeMsgRcv() encountered length error, message from node%d (%u.%u.%u.%u), length=%d, alias length=%d.\n", dwNodeId, OspQuadAddr(dwip), pcMsg4S->length, pcMsg4S->dstAliasLen);
			return;
		}
		/* 消息头接收完整，则不再接收消息体 */
		if( (pcMsg4S->length + pcMsg4S->dstAliasLen) > 0)
		{
			pcMsg4S = (CMessageForSocket *)OspAllocMem(sizeof(CMessageForSocket)+pcMsg4S->length+pcMsg4S->dstAliasLen);
			memcpy(pcMsg4S, pvRcvdData, sizeof(CMessageForSocket));
			OspFreeMem(pvRcvdData);
			pNode->SetRcvdData(pcMsg4S);
			pNode->SetRcvdLen(sizeof(CMessageForSocket));
			return;
		}
	}

	/* 接收消息内容；
	 * 注意：如果一次接收不完，则在下一次调用此函数时继续接收
	 */
	if( dwRcvdLen < (sizeof(CMessageForSocket) +pcMsg4S->length+pcMsg4S->dstAliasLen) )
	{
		dwRcvLen = 0;
		before = OspTickGet();
		if( !SockRecv(tNodeSock, (char *)pcMsg4S+dwRcvdLen, sizeof(CMessageForSocket)+pcMsg4S->length+pcMsg4S->dstAliasLen-dwRcvdLen, &dwRcvLen) )
		{
			after = OspTickGet();
			tmp = after - before;
			if (tmp > g_maxrecv_interval){
				g_maxrecv_interval = tmp;
			}

			OspFreeMem(pcMsg4S);
			pNode->SetRcvdData(NULL);
			pNode->SetRcvdLen(0);
            u32 dwip=OspNodeLastIpGet(dwNodeId);
            OspPrintf(TRUE, TRUE, "Osp: node %d (%u.%u.%u.%u) recv msg failed, will be deleted. SockErr=%d\n", dwNodeId, OspQuadAddr(dwip), errno);
            g_Osp.m_cNodePool.NodeUnRegist(dwNodeId, NODE_DISC_REASON_RECVERR); // add by xiang
			return;
		}

		after = OspTickGet();
		tmp = after - before;
		if (tmp > g_maxrecv_interval){
			g_maxrecv_interval = tmp;
		}
		dwRcvdLen += dwRcvLen;
		/* 消息内容未接收完整，则返回 */
		if( dwRcvdLen < (sizeof(CMessageForSocket)+pcMsg4S->length+pcMsg4S->dstAliasLen) )
		{
			pNode->SetRcvdLen(dwRcvdLen);
			return;
		}
	}

	if (sizeof(CMessage) != sizeof(CMessageForSocket))
	{
		pcMsg = (CMessage*)OspAllocMem(sizeof(CMessage) + pcMsg4S->length + pcMsg4S->dstAliasLen);
		pcMsg->srcnode = pcMsg4S->srcnode;
		pcMsg->dstnode = pcMsg4S->dstnode;
		pcMsg->dstid = pcMsg4S->dstid;
		pcMsg->srcid = pcMsg4S->srcid;
		pcMsg->type= pcMsg4S->type;
		pcMsg->event = pcMsg4S->event;
		pcMsg->length= pcMsg4S->length;
#ifdef SYNCMSG
		pcMsg->output = NULL;
		pcMsg->outlen = pcMsg4S->outlen;
		pcMsg->expire = pcMsg4S->expire;
#endif
		pcMsg->dstAliasLen = pcMsg4S->dstAliasLen;
		if (pcMsg->length > 0)
		{
			pcMsg->content = (u8 *)pcMsg + sizeof(CMessage);
			memcpy(pcMsg->content, (u8 *)pcMsg4S + sizeof(CMessageForSocket), pcMsg->length);
		}
		else
		{
			pcMsg->content = NULL;
		}
		if (pcMsg->dstAliasLen > 0)
		{
			pcMsg->dstAlias = (char *)pcMsg + sizeof(CMessage) + pcMsg->length;
			memcpy(pcMsg->dstAlias, (u8 *)pcMsg4S + sizeof(CMessageForSocket) + pcMsg4S->length, (u32)pcMsg->dstAliasLen);
		}
		else
		{
			pcMsg->dstAlias = NULL;
		}
		/* 消息解析完毕，计数，并清除临时指针 */
		OspFreeMem(pcMsg4S);
		pcMsg4S = NULL;
	}
	else
	{
		pcMsg = (CMessage*)pcMsg4S;
		pcMsg4S = NULL;
	if(pcMsg->length > 0)
	{
		pcMsg->content = (u8 *)pcMsg+sizeof(CMessage);
	}
	else
	{
		pcMsg->content = NULL;
	}

	if(pcMsg->dstAliasLen > 0)
	{
		pcMsg->dstAlias = (char *)pcMsg+sizeof(CMessage)+pcMsg->length;
	}
	else
	{
		pcMsg->dstAlias = NULL;
	}
#ifdef SYNCMSG
		pcMsg->output = NULL;
#endif
	}
	/* 消息解析完毕，计数，并清除临时指针 */
	pNode->msgRcvedInc();
	pNode->SetRcvdData(NULL);
	pNode->SetRcvdLen(0);



	/* 结点接收消息跟踪 */
	if(g_Osp.m_cNodePool.m_dwRcvTrcFlag > 0)
    {
		len = sprintf(achBuf, "\nOsp: message received from node %d\n", dwNodeId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pcMsg);
		OspMsgTrace(TRUE, TRUE, achBuf, len);
    }

    /* 截获全局同步应答 */
    if( pcMsg->type == MSG_TYPE_GSYNCACK )
	{
		if( (pcMsg->length > 0) && (pcMsg->content != NULL) )
		{
			memcpy(g_Osp.m_achSyncAck, pcMsg->content, pcMsg->length);
			g_Osp.m_wSyncAckLen = pcMsg->length;
		}

		OspSemGive(g_Osp.m_tSyncSema);
		OspFreeMem(pcMsg);
		return;
	}

    /* 发送消息到目标App的邮箱中 */
    dstAppId = GETAPP(pcMsg->dstid);
    dstSendQueId = SendQueIdFind(dstAppId);
    if (dstSendQueId == 0)
    {
        u32 dwip=OspNodeLastIpGet(dwNodeId);
		int len = sprintf(achBuf, "\nOsp: node %d (%u.%u.%u.%u) recv msg to unexist app%d\n", dwNodeId, OspQuadAddr(dwip), dstAppId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pcMsg);
		OspMsgTrace(TRUE, TRUE, achBuf, len);
		OspFreeMem(pcMsg);
        return;
    }

    pApp = g_Osp.m_cAppPool.AppGet(dstAppId);
	if(pApp == NULL)
	{
		OspFreeMem(pcMsg);
		OspPrintf(TRUE, FALSE, "Osp: App corresponding to AppID %d not exist.\n", dstAppId);
		return;
	}

    if(pApp->GetMsgWaitingNum() >= pApp->maxMsgWaiting)  // 如果邮箱已满, 丢掉本消息
    {
		OspFreeMem(pcMsg);
        u32 dwip=OspNodeLastIpGet(dwNodeId);
        OspLog(1, "Osp: node%d (%u.%u.%u.%u) recv msg to app%d dropped\n", dwNodeId, OspQuadAddr(dwip), dstAppId);
		pApp->msgdropped++;
		return;
	}

	/* 将消息放到目标App的邮箱中 */
	BOOL32 bSuccess;
	TOsMsgStruc osMsg;
	osMsg.address = (void*)pcMsg;
	pApp->MsgIncomeNumInc();

	before = OspTickGet();
    bSuccess = OspSndMsg(dstSendQueId, (char *)&osMsg, sizeof(TOsMsgStruc));
	after = OspTickGet();

	tmp = after - before;
	if (tmp > g_maxsndpipeint){
		g_maxsndpipeint = tmp;
	}

	if( !bSuccess )
	{
		pApp->msgdropped++;
		OspFreeMem(pcMsg);
        u32 dwip=OspNodeLastIpGet(dwNodeId);
		OspPrintf(TRUE, FALSE, "Osp: NodeMsgRcv, message from node%d (%u.%u.%u.%u) send to app%d failed, discard it.\n", dwNodeId, OspQuadAddr(dwip), dstAppId);
                pApp->MsgIncomeNumDec();
		return;
	}
}

/*====================================================================
函数名：NodeMsgDispatch
功能：从内部socket上接收消息，现仅用于打印结点状态改变信息.
算法实现：（可选项）
引用全局变量：
输入参数说明：tLocalSock: 内部套接字

  返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
API int NodeMsgDispatch(SOCKHANDLE tLocalOutSock)
{
    char achMsgBuf[sizeof(CMessageForSocket) + MAX_MSG_LEN];
    CMessageForSocket *pcMsg = (CMessageForSocket *)achMsgBuf;
    u32 totalLen;
    SOCKHANDLE dstSock;
    u32 rcvExpect;
    u32 rcvLen;

    rcvExpect = sizeof(CMessageForSocket);
    while(rcvExpect > 0)
    {
        if( !SockRecv(tLocalOutSock, achMsgBuf+sizeof(CMessageForSocket)-rcvExpect, rcvExpect, &rcvLen) )
        {
			SockClose(tLocalOutSock);
			g_Osp.m_cNodePool.m_tLocalOutSock = INVALID_SOCKET;
			return OSP_ERROR;
		}
        rcvExpect -= rcvLen;
    }

    rcvExpect = pcMsg->length;
	if(rcvExpect > MAX_MSG_LEN)
	{
		return OSP_ERROR;
	}

    while(rcvExpect > 0)
    {
        if( !SockRecv(tLocalOutSock, achMsgBuf+sizeof(CMessageForSocket)+pcMsg->length-rcvExpect, rcvExpect, &rcvLen) )
        {
			SockClose(tLocalOutSock);
			g_Osp.m_cNodePool.m_tLocalOutSock = INVALID_SOCKET;
			return OSP_ERROR;
		}
        rcvExpect -= rcvLen;
    }

	/* 本地节点? */
    if(pcMsg->dstnode == 0)
    {
	   //OspPrintf(TRUE, FALSE, "Osp: sock connection status changed\n"  ); remarked by xiang, 5/19/2003
        return OSP_OK;
    }

    totalLen = sizeof(CMessageForSocket) + pcMsg->length;
    if( !g_Osp.m_cNodePool.GetSock(pcMsg->dstnode, &dstSock) )
    {
/*        OspPrintf(TRUE, FALSE, "Osp: dstNode %d not exists.\n", pcMsg->dstnode);*/
        u32 dwip=OspNodeLastIpGet(pcMsg->dstnode);
        OspLog(255, "Osp: dstNode %d (%u.%u.%u.%u) not exists.\n", pcMsg->dstnode, OspQuadAddr(dwip));
        g_Osp.m_cNodePool.m_dwGloFailDispNode++;
        return OSP_ERROR;
    }

    MsgHton(pcMsg);

    SockSend(dstSock, achMsgBuf, totalLen);
    return OSP_OK;
}

/*====================================================================
函数名：SvrFdSet
功能：服务器监听socket设置，用于select.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：ptSet: 指向套接口集的指针

  返回值说明：
====================================================================*/
API void SvrFdSet(fd_set *ptSet)
{
    FD_ZERO(ptSet);

    if(g_Osp.m_cNodePool.m_tListenSock != INVALID_SOCKET)
	{
		FD_SET(g_Osp.m_cNodePool.m_tListenSock ,ptSet); //服务器对外服务Sock
	}

    if(g_Osp.m_cNodePool.m_tLocalOutSock != INVALID_SOCKET)
	{
		FD_SET(g_Osp.m_cNodePool.m_tLocalOutSock ,ptSet); //服务器内部服务Sock
	}

    for(u32 i=0; i<MAX_NODE_NUM; i++) //所有其他Sock
    {
        if( ( g_Osp.m_cNodePool.m_acNodeRegTable[i].m_bValid ) &&
			( g_Osp.m_cNodePool.m_acNodeRegTable[i].m_bListenning ) &&
            ( g_Osp.m_cNodePool.m_acNodeRegTable[i].m_tSockHandle != INVALID_SOCKET ) )
        {
            FD_SET(g_Osp.m_cNodePool.m_acNodeRegTable[i].m_tSockHandle, ptSet);
        }
    }
}

/*====================================================================
函数名：OspNodeShow
功能：显示所有Node的状态信息
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：

  返回值说明：
====================================================================*/
API void OspNodeShow()
{
    g_Osp.m_cNodePool.Show();
}

/*====================================================================
函数名：PostDaemon
功能：守护线程, 等待客户端的接入, 等待其他节点的消息输入并转发
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：

  返回值说明：
====================================================================*/
API int PostDaemon()
{
    SOCKHANDLE tSockClient;
    sockaddr_in tAddrClient;
    int addrLenIn=sizeof(tAddrClient);
    u32 dwClientNode;
    fd_set tWaitFd;
    int win_error;

    while(TRUE)
    {
		if(g_Osp.m_bKillOsp)
		{
			if(g_Osp.m_cNodePool.m_tListenSock != INVALID_SOCKET)
			{
				SockClose(g_Osp.m_cNodePool.m_tListenSock);
				g_Osp.m_cNodePool.m_tListenSock = INVALID_SOCKET;
			}

			for(u32 dwNodeId=1; dwNodeId<=MAX_NODE_NUM; dwNodeId++)
			{
				if(g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_tSockHandle != INVALID_SOCKET)
					SockClose(g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_tSockHandle);
			}
			g_Osp.DelTask(g_dwPostDaemonTaskId);
			printf("[PostDaemon] del task[%x]\n", g_dwPostDaemonTaskId);
			OspTaskExit();
		}

        SvrFdSet(&tWaitFd);

		int ret = select(FD_SETSIZE, &tWaitFd, NULL, NULL, NULL);
        if(ret == SOCKET_ERROR || ret == 0)
        {
           OspTaskDelay(100);
           continue;
        }

        //检查是否有新的连接进入
        if(g_Osp.m_cNodePool.m_tListenSock != INVALID_SOCKET) // this node is a server
        {
            if(FD_ISSET( g_Osp.m_cNodePool.m_tListenSock, &tWaitFd))// new connect coming
            {
                addrLenIn = sizeof(tAddrClient);

                tSockClient = accept( g_Osp.m_cNodePool.m_tListenSock, (sockaddr *)&tAddrClient, &addrLenIn);
				if(tSockClient == INVALID_SOCKET)
				{
					 OspLog(1, "Osp : PostDaemon accept sock connect fail\n");
				}
				else
                {
					// set the socket's property, as heartbeat,
					u32 optVal;
					int set_result;

					optVal = 1;
					set_result = setsockopt(tSockClient,IPPROTO_TCP ,TCP_NODELAY ,(char*) &optVal, sizeof(optVal) );
					if(set_result == SOCKET_ERROR )
					{
						win_error = WSAGetLastError();
						OspLog(1, "\nOsp: ConnectTcp: set sock option fail  %d\n",win_error);
					}

					optVal = SOCKET_SEND_BUF;
					set_result = setsockopt(tSockClient,SOL_SOCKET ,SO_SNDBUF,(char *)&optVal, sizeof(optVal) );
					if(set_result == SOCKET_ERROR )
					{
						win_error = WSAGetLastError();
						OspLog(1, "\nOsp: ConnectTcp: set sock option fail  %d\n",win_error);
					}

					optVal = SOCKET_RECV_BUF;
					set_result = setsockopt(tSockClient,SOL_SOCKET ,SO_RCVBUF,(char *)&optVal, sizeof(optVal) );
					if(set_result == SOCKET_ERROR )
					{
						win_error = WSAGetLastError();
						OspLog(1, "\nOsp: ConnectTcp: set sock option fail  %d\n",win_error);
					}

					optVal = 1;
					set_result = setsockopt(tSockClient,SOL_SOCKET ,SO_KEEPALIVE,(char *)&optVal, sizeof(optVal));
					if(set_result == SOCKET_ERROR )
					{
						win_error = WSAGetLastError();
						OspLog(1, "\nOsp: ConnectTcp: set sock option fail  %d\n",win_error);
					}

					BOOL32 bRet;

					dwClientNode = 0;
					bRet = g_Osp.m_cNodePool.Regist( (u32)tAddrClient.sin_addr.s_addr ,
						(u32)tSockClient, (u32 *)&dwClientNode, DEFAULT_TCP_HEARTBEAT_TIME , DEFAULT_TCP_HEARTBEAT_NUM );

					if(bRet)
					{
						OspLog(1, "Osp: client %s accepted on port %d, the nodeid = %d\n", inet_ntoa(tAddrClient.sin_addr), g_Osp.m_cNodePool.m_wListenPort, dwClientNode);
					}
					else
					{
						OspLog(1, "Osp: client %s accepted but regist to node pool failed, will be closed.\n", inet_ntoa(tAddrClient.sin_addr));
						SockClose(tSockClient);
					}
				}
			}
        }

        //检查是否有其他节点的消息输入到本节点,并分发到各个应用
        u32 iNode;

        for (iNode = 1; iNode <= MAX_NODE_NUM; iNode ++)
        {
            if((g_Osp.m_cNodePool.m_acNodeRegTable[iNode-1].m_bValid) &&
                    (g_Osp.m_cNodePool.m_acNodeRegTable[iNode-1].m_tSockHandle != INVALID_SOCKET))
            {
                if(FD_ISSET(g_Osp.m_cNodePool.m_acNodeRegTable[iNode-1].m_tSockHandle, &tWaitFd))
                {
                    NodeMsgRcv(g_Osp.m_cNodePool.m_acNodeRegTable[iNode-1].m_tSockHandle, iNode);
                }
            }
        }

        //检查是否有内部消息需发送到其他节点 will be deleted
        if( FD_ISSET(g_Osp.m_cNodePool.m_tLocalOutSock, &tWaitFd)  )
        {
            NodeMsgDispatch(g_Osp.m_cNodePool.m_tLocalOutSock);
        }
    }
}

/*====================================================================
函数名：OspNodeConnTest
功能：结点连接测试
算法实现：（可选项）
引用全局变量：
输入参数说明：NodeId: 目标结点标识(若大于MAX_NODE_NUM，
                      则表示给所有的结点都发送测试消息)

  返回值说明：
====================================================================*/
API void OspNodeConnTest(u32 NodeId)
{
	u32 i;

	if(NodeId <= 0)
	{
		return;
	}

	if(NodeId > MAX_NODE_NUM)
	{
		OspPrintf(TRUE, FALSE, "Osp: this node is not exist, test all!\n");

		for(i=1; i<=MAX_NODE_NUM; i++)
		{
			if( g_Osp.m_cNodePool.m_acNodeRegTable[i-1].m_bValid )
			{
				OspPost(MAKEIID(NODE_MAN_APPID,1), OSP_NETSTATEST, NULL, 0, i);//发送网络测试消息
				OspTaskDelay(50);
			}
		}
		return;
	}

	if(g_Osp.m_cNodePool.m_acNodeRegTable[NodeId-1].m_bValid)
	{
		OspPost(MAKEIID(NODE_MAN_APPID,1), OSP_NETSTATEST, 0, 0, NodeId);//发送网络测试消息
	}
	else
	{
		OspPrintf(TRUE, FALSE, "Osp: this node is not an actived node!\n");
	}
}

/*====================================================================
函数名：OspStatusMsgOutSet
功能：设置状态信息是否被不断输出
算法实现：（可选项）
引用全局变量：
输入参数说明：OutMsgEnable: 是否允许消息输出

  返回值说明：
====================================================================*/
API void OspStatusMsgOutSet(BOOL32 OutMsgEnable) //set status message can or not be out put every timer
{
	g_Osp.m_bStatusPrtEnable = OutMsgEnable;
}

/*====================================================================
函数名：MsgHton
功能：网络字节转换: 主机字节序-->网络字节序
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMsg: 消息头指针

  返回值说明：
====================================================================*/
API void MsgHton(CMessageForSocket *ptMsg)
{
    ptMsg->srcnode = htonl(ptMsg->srcnode);
    ptMsg->dstnode = htonl(ptMsg->dstnode);   //
    ptMsg->dstid = htonl(ptMsg->dstid);       // app + gate + ins(2)
    ptMsg->srcid = htonl(ptMsg->srcid);
    ptMsg->event = htons(ptMsg->event);       // the name of the message
    ptMsg->length = htons(ptMsg->length);     // the length (max 64K) of content of the message
	ptMsg->type = htons(ptMsg->type);
  	ptMsg->outlen = htons(ptMsg->outlen);
}

/*====================================================================
函数名：MsgNtoh
功能：网络字节转换: 网络字节序-->主机字节序
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMsg: 消息头指针

  返回值说明：
====================================================================*/
API void MsgNtoh( CMessageForSocket *ptMsg)
{
    ptMsg->srcnode = ntohl(ptMsg->srcnode);
    ptMsg->dstnode = ntohl(ptMsg->dstnode);   //
    ptMsg->dstid = ntohl(ptMsg->dstid);       // app + gate + ins(2)
    ptMsg->srcid = ntohl(ptMsg->srcid);
	ptMsg->type = ntohs(ptMsg->type);
    ptMsg->event = ntohs(ptMsg->event);       // the name of the message
    ptMsg->length = ntohs(ptMsg->length);     // the length (max 64K) of content of the message
	ptMsg->outlen = ntohs(ptMsg->outlen);
}

/*====================================================================
函数名：MsgDump2Buf
功能：打印出消息的内容
算法实现：（可选项）
引用全局变量：
输入参数说明：buf:       缓冲区指针
			  buflen:    缓冲区大小
			  CMessage:  待打印消息

  返回值说明：打印内容的长度
====================================================================*/
API u32 MsgDump2Buf(char *buf, int bufLen, CMessage *ptMsg)
{
	int i;
	int iChar; // used for char print
	char *pPtr;
    char *pEventDesc;
    static u32 dwMsgDumpSeq = 0;
	u32 actLen = 0;

	if(bufLen <= 0)
	{
		return 0;
	}

	if(bufLen > MAX_LOG_MSG_LEN)
	{
		OspLog(1, "Osp: MsgDump2Buf bufLen too long.\n");
		return 0;
	}

    dwMsgDumpSeq++;

	/* 如果是按实例别名发送的, 显示实例别名, 否则显示实例号 */
	if(ptMsg->dstAlias != NULL && ptMsg->dstAliasLen > 0)
	{
		actLen += sprintf(buf+actLen, "Message Seq=%d dst: Node=%d App=%d InstAlias=0x",
			dwMsgDumpSeq, ptMsg->dstnode, GETAPP(ptMsg->dstid));
		for(u8 i=0; i<ptMsg->dstAliasLen; i++)
		{
			actLen += sprintf(buf+actLen, "%02x", ptMsg->dstAlias[i]);
		}
		actLen += sprintf(buf+actLen, "\n");
	}
    else
	{
		actLen += sprintf(buf+actLen, "Message Seq=%d dst: Node=%d App=%d Ins=%d\n",
			dwMsgDumpSeq, ptMsg->dstnode, GETAPP(ptMsg->dstid), GETINS(ptMsg->dstid) );
	}

    pEventDesc = g_Osp.m_cOspEventDesc.DescGet(ptMsg->event);
	actLen += sprintf(buf+actLen, "event: %s eventid=%d length=%d\n",
		             pEventDesc, ptMsg->event, ptMsg->length);

	actLen += sprintf(buf+actLen, "source: Node=%d App=%d Ins=%d\n",
		             ptMsg->srcnode, GETAPP(ptMsg->srcid), GETINS(ptMsg->srcid));

	/* 输出消息体 */
    pPtr = (char*)ptMsg->content;
    for(i=0; i<ptMsg->length; i++, pPtr++)
	{
		/* 在禁止长消息输出的情况下，只输出10行内容 */
		if(!IsOspLogLongMsgPrintEnbl())
		{
			if( i >= (DEF_MSGTRC_LINES * MSGTRCBYTES_PER_LINE) )
			{
				break;
			}
		}
		else
		{
			/* 在允许输出长消息的情况下, 也仅允许输出MAX_MSGTRC_LINES行内容 */
			if( i >= (MAX_MSGTRC_LINES * MSGTRCBYTES_PER_LINE) )
			{
				break;
			}
		}

		/* 每16个u8为一行 */
		if( (i&0x0F) == 0 )
		{
			actLen += sprintf(buf+actLen, "%4xh: ", i);
		}

		actLen += sprintf(buf+actLen, "%.2X ", (u8)*pPtr);
		if( ((i+1)&0x0F) == 0 )
		{
			actLen += sprintf(buf+actLen, ";   "); // 每输出16个u8，打印该行的字符提示
			for(iChar=15; iChar>=0; iChar--)
			{
				if( ((*(pPtr-iChar))>=0x21) && ((*(pPtr-iChar)) <= 0x7E) ) // 可打印字符
				{
					actLen += sprintf(buf+actLen, "%1c", *(pPtr-iChar));
				}
				else  // 不可打印字符
				{
					actLen += sprintf(buf+actLen, ".");
				}
			}
			actLen += sprintf(buf+actLen, "\n");
		}

		if( (i == (ptMsg->length-1)) && (((i+1)&0x0F) != 0) ) // 最后一行
		{
			u32 iBlank = 16 - ((i+1)&0x0F);
			u32 iBCount;

			for(iBCount=0; iBCount<iBlank; iBCount++) // print black space
			{
				actLen += sprintf(buf+actLen, "   ");
			}
			actLen += sprintf(buf+actLen, ";   ");

			for(iChar=(i&0x0F); iChar>=0; iChar--) // print the char
			{
				if(((*(pPtr-iChar)) >= 0x21)&&((*(pPtr-iChar)) <= 0x7e))
				{
					actLen += sprintf(buf+actLen, "%1c", *(pPtr-iChar));
				}
				else
				{
					actLen += sprintf(buf+actLen, ".");
				}
			}  //end for
		} //end if
	} // end for

	actLen += sprintf(buf+actLen, (i>0) ? "\n\n":"\n");
	return actLen;

/*====================================================================
函数名：OspMsgDumpSet
功能：设置超过十行的消息内容是否输出，缺省不输出
算法实现：（可选项）
引用全局变量：
输入参数说明：bLongMsgDumpEnbl: 允许/禁止长消息输出

  返回值说明：
====================================================================*/
}

/*====================================================================
函数名：MsgIdChange
功能：更改消息的DstNodeID和SrcNodeID
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMsg: 消息指针
			  uNode: 指定消息新的结点号

  返回值说明：
====================================================================*/
API void MsgIdChange(CMessageForSocket *ptMsg,u32  uNode) // 接受消息,转换DstNodeID和SrcNodeID;
{
    ptMsg->srcnode = uNode;
    ptMsg->dstnode = 0;
}

/*====================================================================
函数名：OspNodeDiscCBReg
功能：设置在node连接中断时需通知的appid和InstId
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNodeId: 目标结点
			  wAppId:   目标APP号
			  wInsId:   目标Instance

  返回值说明：成功返回OSP_OK，失败参见错误码
====================================================================*/
API int OspNodeDiscCBReg(u32 dwNodeId, u16 wAppId, u16 wInsId)
{
	if(dwNodeId<=0 || dwNodeId>MAX_NODE_NUM)
		return OSPERR_ILLEGAL_PARAM;

	if(wAppId<=0 || wAppId>MAX_APP_NUM)
		return OSPERR_ILLEGAL_PARAM;

	OspTaskSafe();
	OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );

	if( !g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid )
	{
		OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
		OspTaskUnsafe();

		OspPost(MAKEIID(wAppId,wInsId),OSP_DISCONNECT,&dwNodeId,sizeof(dwNodeId));

		return OSPERR_NODE_INVALID;
	}

	/* 注册唯一的通知实例 */
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum      = 1;
    g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformAppId[0] = wAppId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformInsId[0] = wInsId;

	OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
	OspTaskUnsafe();

	return OSP_OK;
}

/*====================================================================
函数名：OspNodeDiscCBRegQ
功能：增加在node连接中断时需通知的appid和InstId
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNodeId: 目标结点
			  wAppId:   目标APP号
			  wInsId:   目标Instance

  返回值说明：成功返回当前已注册个数，失败参见错误码
====================================================================*/
API int OspNodeDiscCBRegQ(u32 dwNodeId, u16 wAppId, u16 wInsId)
{
	if(dwNodeId<=0 || dwNodeId>MAX_NODE_NUM)
		return OSPERR_ILLEGAL_PARAM;

	if(wAppId<=0 || wAppId>MAX_APP_NUM)
		return OSPERR_ILLEGAL_PARAM;

	OspTaskSafe();
	OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );
	/* 检查已注册数量 */
	int curr_idx = g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum;
	if ((curr_idx + 1) >= NODE_MAX_CBNUM)
	{
		OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
		OspTaskUnsafe();

		OspPrintf(TRUE, FALSE, "Osp: Callback instance number has reached maximum.\n");
		return OSP_ERROR;
	}
	/* 检查当前实例是否已经注册过 */
	for (int InformNum = 0; InformNum < curr_idx; InformNum++)
	{
		if ((g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformAppId[InformNum] == wAppId) &&
			(g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformInsId[InformNum] == wInsId))
		{
			OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
			OspTaskUnsafe();

			OspPrintf(TRUE, FALSE, "Osp: curr inst(%d:%d) has already been registered.\n", wAppId, wInsId);
			return OSP_ERROR;
		}
	}

	if( !g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid )
	{
		OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
		OspTaskUnsafe();

		OspPost(MAKEIID(wAppId,wInsId),OSP_DISCONNECT,&dwNodeId,sizeof(dwNodeId));

		return OSPERR_NODE_INVALID;
	}

	/* 注册唯一的通知实例 */

    g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformAppId[curr_idx] = wAppId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformInsId[curr_idx] = wInsId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum++;

	OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
	OspTaskUnsafe();

	return OSP_OK;
}

/*=============================================================================
  函 数 名：OspNodeLastIpGet
  功    能：节点无效时获得指定结点的上一次对应的Ip地址，有效时获得节点对应的IP地址(仅用于打印信息)
  注    意：
  算法实现：
  全局变量：
  参    数：dwNodeId : [in] node id

  返 回 值：成功返回结点IP, 失败返回0.
  =============================================================================*/
API u32 OspNodeLastIpGet(u32 dwNodeId)
{
    u32 ret = 0;

    OspTaskSafe();
    OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );

    if(dwNodeId > 0 && dwNodeId <= MAX_NODE_NUM )
    {
	    ret = g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_dwIpAddr;
    }

    OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
    OspTaskUnsafe();

    return ret;
}


/*=============================================================================
函 数 名：OspNodeIpGet
功    能：获得指定结点的Ip地址
注    意：
算法实现：
全局变量：
参    数：dwNodeId : [in] node id

  返 回 值：成功返回结点IP, 失败返回0.
=============================================================================*/
API u32 OspNodeIpGet(u32 dwNodeId)
{
	u32 ret = 0;

	OspTaskSafe();
	OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );

	if(dwNodeId > 0 &&
		dwNodeId <= MAX_NODE_NUM &&
		g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid)
	{
		ret = g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_dwIpAddr;
	}

	OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
	OspTaskUnsafe();

	return ret;
}

/*=============================================================================
  函 数 名：OspNodeLocalIpGet
  功    能：获取节点的本地地址
  注    意：对于已经连接的tcp节点，在本地存在多个地址的情况下，需要知道对方实际连接的本地ip地址。
  算法实现：
  全局变量：
  参    数：dwNodeId : [in] node id

  返 回 值：成功返回结点IP, 失败返回0.
  =============================================================================*/
API u32 OspNodeLocalIpGet(u32 dwNodeId)
{
    u32 ret = 0;

    OspTaskSafe();
    OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );

    if(dwNodeId > 0 &&
        dwNodeId <= MAX_NODE_NUM &&
        g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid)
    {
        SOCKHANDLE tSockHandle = g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_tSockHandle;

        if (INVALID_SOCKET != tSockHandle)
        {
            SOCKADDR_IN tAddr;
            s32 nAddrLen;

            nAddrLen = sizeof(tAddr);
            if (getsockname(tSockHandle, (SOCKADDR*)&tAddr, &nAddrLen) != 0)
            {
                ret = (u32)-1;
            }
            else
            {
                ret = tAddr.sin_addr.s_addr;
            }
        }
    }

    OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
    OspTaskUnsafe();

    return ret;
}

/*====================================================================
函数名：NodeDiscCallBack(
功能：node连接中断时发送OSP_DISCONNECT消息给NodeDiscCBReg中设置的appid和InstId
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNodeId: 目标结点
			  wAppId:   目标APP号
			  wInsId:   目标Instance

  返回值说明：
====================================================================*/
API void NodeDiscCallBack(u32 dwNodeId, u16 wAppId, u16 wInsId)
{
    if(dwNodeId<=0 || dwNodeId>MAX_NODE_NUM)
		return;

    if(wAppId<=0 || wAppId>MAX_APP_NUM)
		return;

    OspPrintf(TRUE, FALSE, "Osp: post OSP_DISCONNECT message to apps: %d,%d for node: %d\n", wAppId, wInsId, dwNodeId);
    OspPost(MAKEIID(wAppId,wInsId,0), OSP_DISCONNECT, &dwNodeId, sizeof(dwNodeId), 0, 0, 0);
}

/*====================================================================
函数名：DispatchTaskEntry
功能：Dispatch Task的入口
算法实现：（可选项）
引用全局变量：
输入参数说明：pcDispTask: 待发送消息的CDispatchTask指针

  返回值说明：
====================================================================*/
API void DispatchTaskEntry(CDispatchTask *pcDispTask)
{
    pcDispTask->NodeMsgSendToSock();
}

/*====================================================================
函数名：postMsg
功能：发送异步消息到本节点或其他节点
算法实现：（可选项）
引用全局变量：
输入参数说明：dwDstId:		目标实例的实例ID(app, ins),
              wEvent:		消息号,
              pvContent:	消息体指针,
			  wLength:		消息长度,
			  dwDstNode:	目标实例的结点号,
              dwSrcIId:		源实例ID(app, ins),
              dwSrcNode:	源结点号
			  bDroppable:   该消息是否可丢弃
			  byType:		消息类型
			  nTimeout:		超时时限

  返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR
====================================================================*/
API int OspPostMsg(u32 dwDstId, u16 wEvent, const void* pvContent, u16 wLength, u32 dwDstNode,
				   u32 dwSrcIId, u32 dwSrcNode, BOOL32 bDroppable, u8 byType, int nTimeout)
{
    CMessage *pMsg = NULL;
    u32 dwMsgLen = 0;

    if(wLength > MAX_MSG_LEN)
	{
		return OSP_ERROR;
	}

	/* 确保参数的一致性 */
	if( (pvContent == NULL) || (wLength == 0) )
	{
		pvContent = NULL;
		wLength = 0;
	}

	CApp *pcSrcApp = g_Osp.m_cAppPool.AppGet(GETAPP(dwSrcIId));
	if(pcSrcApp != NULL)
	{
		pcSrcApp->curEvtSnd = wEvent;
	}

    if(wEvent == OSP_APPCONN_ACK)
	{
		//显示APP回应
		u16 appId = GETAPP(dwDstId);
		OspPrintf(TRUE, FALSE, "Osp: AppId %d received APP_CONNECT_ACK messaeg\n", appId);
		return OSP_OK;
	}

	pMsg = (CMessage *)OspAllocMem(sizeof(CMessage)+wLength);
//	pMsg = (CMessage *)g_Osp.m_cMsgStack.GetStack();   //11-10

    memset(pMsg, 0, sizeof(CMessage));

	pMsg->srcnode = dwSrcNode;
    pMsg->dstnode = dwDstNode;
    pMsg->dstid   = dwDstId;
    pMsg->srcid   = dwSrcIId;
	pMsg->type    = byType;
    pMsg->event   = wEvent;
    pMsg->length  = wLength;

	if(wLength > 0)
	{
		pMsg->content = (u8 *)pMsg+sizeof(CMessage);
		memcpy(pMsg->content, pvContent, wLength);
	}

    /* 结点间消息，交给分派系统处理 */
	if(dwDstNode > 0)
	{
		if( (dwDstNode > MAX_NODE_NUM) || !g_Osp.m_cNodePool.m_acNodeRegTable[dwDstNode-1].m_bValid )
		{
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
            u32 dwip=OspNodeLastIpGet(dwDstNode);
			OspLog(255, "Osp: postmsg failed due to dstNode %d (%u.%u.%u.%u) not exist.\n", dwDstNode, OspQuadAddr(dwip));
            g_Osp.m_cNodePool.m_dwGloFailPostNode++;
			return OSP_ERROR;
		}
        //消息压缩by wubin 2011-02-22
        dwMsgLen = sizeof(CMessage) + wLength;
        if (OSP_NETBRAECHO != wEvent && OSP_NETBRAECHOACK != wEvent && wLength > 0)
        {
            if (TRUE == g_Osp.m_cNodePool.m_acNodeRegTable[dwDstNode - 1].m_bCMessageCompressSupport)
            {

                if (FALSE == OspCompressMessagePack(&pMsg, &dwMsgLen))
                {
                    OspLog(OSP_COMPRESS_LOG_LEVEL, "Osp: compress msg fail or the compressed msg size is even large. use origin msg\n");
                }
            }
            else
            {
                OspLog(OSP_COMPRESS_LOG_LEVEL, "Node %u compress not supported\n", dwDstNode);
            }
        }
        // wubin 2011-02-22 -->
		return g_Osp.m_cDispatchPool.NodeMsgPost(dwDstNode, (char *)pMsg, dwMsgLen);
		//return OSP_OK;
	}

	/* 结点内部消息，将消息头指针发送到目标App的邮箱中 */
    u16 appId = GETAPP(dwDstId);
	MAILBOXID dstSendQueId = SendQueIdFind(GETAPP(dwDstId) ); // 目标应用的消息队列

	if(dstSendQueId == NULL)
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		OspLog(255, "Osp: postmsg, app%d not exists\n", appId);
        g_Osp.m_cNodePool.m_dwGloFailPostApp++;
		return OSP_ERROR;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(appId);
	if(pApp == NULL)
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		return OSP_ERROR;
	}
    pApp->MsgIncomeNumInc();

	/* 如果目标App的邮箱满而且本消息可以丢弃 */
	if( bDroppable && (pApp->GetMsgWaitingNum() > pApp->maxMsgWaiting) )
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		pApp->msgdropped++;
		OspPrintf(TRUE, FALSE, "Osp: postmsg, app %d's message dropped\n", appId);
        pApp->MsgIncomeNumDec();
		return OSP_ERROR;
	}

	/* 将消息地址发送到目标App邮箱中 */
	BOOL32 bSuccess;
	TOsMsgStruc osMsg;
	osMsg.address = (void*)pMsg;
	bSuccess = OspSndMsg(dstSendQueId, (char *)&osMsg, sizeof(TOsMsgStruc), nTimeout);
	if( !bSuccess )
	{
		pApp->msgdropped++;
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		OspPrintf(TRUE, FALSE, "Osp: postmsg, send message to app%d failed, discard it.\n", appId);
                pApp->MsgIncomeNumDec();
		return OSP_ERROR;
	}
    return OSP_OK;
}

/*====================================================================
函数名：OspPostMsg
功能：用实例别名发送异步消息到本节点或其他节点
算法实现：（可选项）
引用全局变量：
输入参数说明：
			  pchDstAlias:  目标实例的别名
			  byDstAliasLen:别名长度
			  wDstAppId:	目标APP
			  dwDstNode:	目标NODE
              wEvent:		消息号,
              pvContent:	消息体指针,
			  wLength:		消息长度,
			  dwDstNode:	目标实例的结点号,
              dwSrcIId:		源实例ID(app, ins),
              dwSrcNode:	源结点号
			  bDroppable:   该消息是否可丢弃
			  Type:			消息类型

  返回值说明：成功OSP_OK, 失败OSP_ERROR
====================================================================*/
int OspPostMsg(const char* pchDstAlias, u8 byDstAliasLen, u16 wDstAppID, u32 dwDstNode,
				u16 wEvent, const void* pvContent, u16 wLength,
				u32 dwSrcIId, u32 dwSrcNode,
				BOOL32 bDroppable, u8 type)
{
    CMessage *pMsg = NULL;
    u32 dwMsgLen = 0;

    if( (wLength > MAX_MSG_LEN) || (byDstAliasLen > MAX_ALIAS_LEN) )
	{
		return OSP_ERROR;
    }

	if( (pchDstAlias == NULL) || (byDstAliasLen <= 0) )
	{
		return OSP_ERROR;
	}

	/* 保证参数的一致性 */
	if( (pvContent == NULL) || (wLength == 0) )
	{
		pvContent = NULL;
		wLength = 0;
	}

	CApp *pcSrcApp = g_Osp.m_cAppPool.AppGet(GETAPP(dwSrcIId));
	if(pcSrcApp != NULL)
	{
		pcSrcApp->curEvtSnd = wEvent;
	}

    if(wEvent == OSP_APPCONN_ACK)
	{
		OspPrintf(TRUE, FALSE, "Osp: AppId %d received APP_CONNECT_ACK messaeg\n", wDstAppID);
		return OSP_OK;
	}

	pMsg = (CMessage *)OspAllocMem(sizeof(CMessage)+wLength+byDstAliasLen);
//	pMsg = (CMessage *)g_Osp.m_cMsgStack.GetStack();   //11-10

    memset(pMsg, 0, sizeof(CMessage));

    pMsg->srcnode = dwSrcNode;
    pMsg->dstnode = dwDstNode;
    pMsg->dstid   = MAKEIID(wDstAppID, CInstance::INVALID, 0);
    pMsg->srcid   = dwSrcIId;
	pMsg->type    = type;
    pMsg->event   = wEvent;
    pMsg->length  = wLength;
    pMsg->dstAliasLen = byDstAliasLen;

	if(wLength > 0)
	{
		pMsg->content = (u8 *)pMsg + sizeof(CMessage);
		memcpy(pMsg->content, pvContent, wLength);
	}

	pMsg->dstAlias = (char *)pMsg + sizeof(CMessage) + wLength;
	memcpy(pMsg->dstAlias, pchDstAlias, byDstAliasLen);

    /* 结点间消息，交给分派系统处理 */
	if(dwDstNode > 0)
	{
		if( (dwDstNode > MAX_NODE_NUM) || (!g_Osp.m_cNodePool.m_acNodeRegTable[dwDstNode-1].m_bValid) )
		{
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
            u32 dwip=OspNodeLastIpGet(dwDstNode);
            OspLog(255, "Osp: postmsg failed due to dstNode %d (%u.%u.%u.%u) not exist.\n", dwDstNode, OspQuadAddr(dwip));
            g_Osp.m_cNodePool.m_dwGloFailPostNode++;
			return OSP_ERROR;
		}
        //消息压缩by wubin 2011-02-22
        dwMsgLen = sizeof(CMessage) + wLength + byDstAliasLen;
        if (OSP_NETBRAECHO != wEvent && OSP_NETBRAECHOACK != wEvent && wLength > 0)
        {
            if (TRUE == g_Osp.m_cNodePool.m_acNodeRegTable[dwDstNode - 1].m_bCMessageCompressSupport)
            {

                if (FALSE == OspCompressMessagePack(&pMsg, &dwMsgLen))
                {
                    OspLog(OSP_COMPRESS_LOG_LEVEL, "Osp: compress msg fail or the compressed msg size is even large. use origin msg\n");
                }
            }
            else
            {
                OspLog(OSP_COMPRESS_LOG_LEVEL, "Node %u compress not supported\n", dwDstNode);
            }
        }
        // wubin 2011-02-22 -->
		return g_Osp.m_cDispatchPool.NodeMsgPost(dwDstNode, (char *)pMsg, dwMsgLen);
		//return OSP_OK;
	}

	/* 结点内部消息，将消息头指针发送到目标App的邮箱中 */
 	MAILBOXID dstSendQueId = SendQueIdFind(wDstAppID); // 目标应用的消息队列
	if(dstSendQueId == 0)
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		OspLog(255, "Osp: postmsg, app%d not exists\n", wDstAppID);
        g_Osp.m_cNodePool.m_dwGloFailPostApp++;
		return OSP_ERROR;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(wDstAppID);
	if(pApp == NULL)
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		return OSP_ERROR;
	}
    pApp->MsgIncomeNumInc();
	/* 如果目标App的邮箱满而且本消息可以丢弃 */
	if( bDroppable && ( pApp->GetMsgWaitingNum() > pApp->maxMsgWaiting) )
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		pApp->msgdropped++;
		OspPrintf(TRUE, FALSE, "Osp: postmsg, App%d's message dropped\n", wDstAppID);
        pApp->MsgIncomeNumDec();
		return OSP_ERROR;
	}

	/* 将消息地址发送到目标App邮箱中 */
	BOOL32 bSuccess;
	TOsMsgStruc osMsg;
	osMsg.address = (void*)pMsg;

	bSuccess = OspSndMsg(dstSendQueId, (char *)&osMsg, sizeof(TOsMsgStruc));
	if( !bSuccess )
	{
		pApp->msgdropped++;
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		OspPrintf(TRUE, FALSE, "Osp: postmsg, send message to app%d failed, discard it.\n", wDstAppID);
                pApp->MsgIncomeNumDec();
        return OSP_ERROR;
	}
    return OSP_OK;
}

/*====================================================================
函数名：OspSend
功能：全局同步消息发送函数
算法实现：（可选项）
引用全局变量：
输入参数说明：dwDstId:		目标实例的实例ID(app, ins),
              wEvent:		消息号,
              pvContent:	消息体指针,
			  wLength:		消息长度,
			  dwDstNode:	目标实例的结点号,
              dwSrcIId:		源实例ID(app, ins),
              dwSrcNode:	源结点号
			  ackbuf:		回应消息缓冲区
			  ackbuflen:	缓冲区大小
			  realacklen:	实际回应消息的长度
			  timeout:		超时时限

  返回值说明：成功OSP_OK， 失败OSP_ERROR
====================================================================*/
API int OspSend(u32 dwDstId, u16 wEvent, const void *pvContent, u16 wLength, u32 dwDstNode,
		        u32 srciid, u32 srcnode,
				void* ackbuf, u16 ackbuflen, u16 *realacklen,
			    u16 timeout)
{
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

    bFileTrc = (g_Osp.m_cAppPool.m_wGloFileTrc & TRCEVENT) ? TRUE:FALSE;
    bScrnTrc = (g_Osp.m_cAppPool.m_wGloScrTrc & TRCEVENT) ? TRUE:FALSE;
    if ( bFileTrc || bScrnTrc )
    {
        CMessage msg;
        msg.srcnode= 0;
        msg.dstnode= dwDstNode;
        msg.dstid = dwDstId;     // app + gate + ins(2)
		msg.srcid = srciid;
        msg.event = wEvent;      // the name of the message
        msg.length = wLength;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)pvContent;
		msg.dstAlias = NULL;
		msg.dstAliasLen = 0;

		len = sprintf(achBuf, "\nmessage send app %s = %d\n", NULL, 0);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace

	/* 同时只允许一个线程发送全局同步消息 */
	OspTaskSafe();
	OspSemTake(g_Osp.m_tMutexSema);

    int ret = OspPostMsg(dwDstId, wEvent, pvContent, wLength, dwDstNode,
		              srciid, srcnode, TRUE, MSG_TYPE_GSYNC);
	if(ret != OSP_OK)
	{
		OspSemGive(g_Osp.m_tMutexSema);
		OspTaskUnsafe();
		return ret;
	}

//	g_Osp.m_bSyncAckExpired = FALSE;
	if( ! OspSemTakeByTime(g_Osp.m_tSyncSema, timeout) )
	{
//		g_Osp.m_bSyncAckExpired = TRUE;
		OspSemGive(g_Osp.m_tMutexSema);
		OspTaskUnsafe();
		return OSPERR_SEND_TIMEOUT;
	}

	if(realacklen != NULL)
	{
		*realacklen = g_Osp.m_wSyncAckLen;
	}

	if(g_Osp.m_wSyncAckLen > 0)
	{
		/* 如果应答buffer不够, 返回应答超长错误 */
		if(	ackbuf==NULL || ackbuflen<g_Osp.m_wSyncAckLen )
		{
			OspSemGive(g_Osp.m_tMutexSema);
			OspTaskUnsafe();
			return OSPERR_SYNCACK_EXCEED;
		}

		/* 将应答拷贝到应答buffer中 */
		memcpy(ackbuf, g_Osp.m_achSyncAck, g_Osp.m_wSyncAckLen);
		memset(g_Osp.m_achSyncAck, 0, sizeof(g_Osp.m_achSyncAck));
		g_Osp.m_wSyncAckLen = 0;
	}

	OspSemGive(g_Osp.m_tMutexSema);
	OspTaskUnsafe();
	return OSP_OK;
}

/*====================================================================
函数名：OspSend
功能：全局同步消息发送函数，使用别名
算法实现：（可选项）
引用全局变量：
输入参数说明：DstAlias:     目标实例的别名
			  AliasLen:     别名长度
			  dstapp:	    目标APP
              event:		消息号,
              content:		消息体指针,
			  length:		消息长度,
			  dstnode:		目标实例的结点号,
              srcIId:		源实例ID(app, ins),
              SrcNode:		源结点号
			  ackbuf:		回应消息缓冲区
			  ackbuflen:	缓冲区大小
			  realacklen:	实际回应消息的长度
			  timeout:		超时时限

  返回值说明：成功OSP_OK， 失败OSP_ERROR

====================================================================*/
int OspSend(const char* dstalias, u8 aliaslen, u16 dstapp, u16 event,
		    const void* content, u16 length, u32 dstnode,
			u32 srciid, u32 srcnode,
			void* ackbuf, u16 ackbuflen, u16 *realacklen,
			u16 timeout)
{
	char achBuf[MAX_LOG_MSG_LEN];
	int len;
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;

    bFileTrc = (g_Osp.m_cAppPool.m_wGloFileTrc & TRCEVENT) ? TRUE:FALSE;
    bScrnTrc = (g_Osp.m_cAppPool.m_wGloScrTrc & TRCEVENT) ? TRUE:FALSE;
    if ( bFileTrc || bScrnTrc )
    {
        CMessage msg;
        msg.srcnode= 0;
        msg.dstnode= dstnode;
        msg.dstid = MAKEIID(dstapp,CInstance::INVALID);     // app + gate + ins(2)
		msg.srcid = srciid;
        msg.event = event;      // the name of the message
        msg.length= length;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)content;
		msg.dstAlias = (char *)dstalias;
		msg.dstAliasLen = aliaslen;

		len = sprintf(achBuf, "\nmessage send app %s = %d\n", NULL, 0);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace

	/* 同时只允许一个线程发送全局同步消息 */
	OspTaskSafe();
	OspSemTake(g_Osp.m_tMutexSema);

    int ret = OspPostMsg(dstalias, aliaslen, dstapp, dstnode, event, content, length, srciid, srcnode, TRUE, MSG_TYPE_GSYNC);
	if(ret != OSP_OK)
	{
		OspSemGive(g_Osp.m_tMutexSema);
		OspTaskUnsafe();
		return ret;
	}

	if( ! OspSemTakeByTime(g_Osp.m_tSyncSema, timeout) )
	{
		OspSemGive(g_Osp.m_tMutexSema);
		OspTaskUnsafe();
		return OSPERR_SEND_TIMEOUT;
	}

	if( g_Osp.m_wSyncAckLen!=0 && ackbuf!=NULL && ackbuflen>=g_Osp.m_wSyncAckLen )
	{
		memcpy(ackbuf, g_Osp.m_achSyncAck, g_Osp.m_wSyncAckLen);
		if(realacklen != NULL)
		{
			*realacklen = g_Osp.m_wSyncAckLen;
		}
	}

	OspSemGive(g_Osp.m_tMutexSema);
	OspTaskUnsafe();
	return OSP_OK;
}

/*====================================================================
函数名：CAppPool
功能：构造函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
CAppPool::CAppPool()
{
    u32 i;

    for(i=0; i<MAX_APP_NUM; i++)
    {
        m_apcAppRegTable[i] = NULL;
    }
}

BOOL32 CNodePool::Alloc(u32 dwMaxNodeNum, u32 dwMaxDispatchMsg)
{
	if ( dwMaxNodeNum < 1 || dwMaxNodeNum > OSP_NODE_MAX_CAPABLE )
		return FALSE ;
	if ( dwMaxDispatchMsg < 1 || dwMaxDispatchMsg > OSP_MSG_MAX_CAPABLE )
		return FALSE ;

	if ( NULL == m_acNodeRegTable )
	{
		MAX_DISPATCHMSG_WAITING = dwMaxDispatchMsg ;
		MAX_NODE_NUM = dwMaxNodeNum;
		m_acNodeRegTable = new CNodeItem[MAX_NODE_NUM];
		if ( NULL == m_acNodeRegTable )
			return FALSE ;

		for ( u32 i = 0; i < MAX_NODE_NUM ;i ++)
			m_acNodeRegTable[i].Initialize();
	}

	return TRUE ;
}

/*====================================================================
函数名：CNodePool::Initialize
功能：结点池初始化
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
BOOL32 CNodePool::Initialize()
{
    u32 i;

	m_wListenPort = 0;
    m_dwSendTrcFlag=0;
    m_dwRcvTrcFlag=0;

    for(i=0; i<MAX_NODE_NUM; i++)
    {
		m_acNodeRegTable[i].Initialize();
    }

    OspSemBCreate(&m_tSemaNodePool);

	/* 激活结点管理App */
	int ret = m_cNodeManApp.CreateApp("OspNodeMan", NODE_MAN_APPID, NODE_MAN_APPPRI, (u16)MAX_NODEMANMSG_WAITING);
	if(ret != OSP_OK)
	{
		OspLog(1, "Osp: create app OspNodeMan fail\n");
		return FALSE;
	}

	//启动结点检测
	ret = OspPost(MAKEIID(NODE_MAN_APPID, 1), START_UP_EVENT);
	if(ret != OSP_OK)
	{
		OspLog(1, "Osp: invoke OspNodeMan fail\n");
		return FALSE;
	}

	return TRUE;
}

/*====================================================================
函数名：CNodePool::NodeGet
功能：根据结点号，从结点池中得到该指向该结点的指针
算法实现：（可选项）
引用全局变量：
输入参数说明：nodeId: 节点ID

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
CNodeItem *CNodePool::NodeGet(u32 nodeId)
{
	if(nodeId <= 0 || nodeId > MAX_NODE_NUM)
	{
		return NULL;
	}

	if(m_acNodeRegTable[nodeId-1].m_bValid)
	{
		return &m_acNodeRegTable[nodeId-1];
	}
	return NULL;
}

/*=============================================================================
函 数 名：OspGetNodeAddr
功	  能：查找Osp结点地址（本端与远端IP+Port）。
注	  意：
算法实现：
全局变量：
参	  数：u32 dwNodeId : [in] 结点ID
TOspNodeAddr* ptOspNodeAddr : [out] 结点地址
返 回 值：FALSE - 查询成功
TRUE - 查询失败
-------------------------------------------------------------------------------
修改纪录：
日		期	版本  修改人  修改内容
2006/08/10	4.0
=============================================================================*/
API BOOL32 OspGetNodeAddr( u32 dwNodeId , TOspNodeAddr* ptOspNodeAddr )
{
	if( NULL == ptOspNodeAddr )
	{
		return FALSE;
	}
	if( ( 0 == dwNodeId ) || (dwNodeId > MAX_NODE_NUM) ||
		!g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid )
	{
		return FALSE;
	}
	struct sockaddr_in tLocalAddr , tPeerAddr;
	s32 nLocalLength = sizeof(tLocalAddr);
	s32 nPeerLength = sizeof(tPeerAddr);

	if( ( 0 != getsockname( g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_tSockHandle,
		(SOCKADDR*)&tLocalAddr, &nLocalLength ) ) ||
		( 0 != getpeername( g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_tSockHandle,
		(SOCKADDR*)&tPeerAddr, &nPeerLength ) ) )
	{
		return FALSE;
	}

	ptOspNodeAddr->m_dwLocalIP = tLocalAddr.sin_addr.s_addr;
	ptOspNodeAddr->m_wLocalPort = ntohs(tLocalAddr.sin_port);
	ptOspNodeAddr->m_dwPeerIP = tPeerAddr.sin_addr.s_addr;
	ptOspNodeAddr->m_wPeerPort = ntohs(tPeerAddr.sin_port);

	return TRUE;
}


extern SOCKHANDLE sockClient;
extern SOCKHANDLE sockTelSer;
extern u16 g_wportListtening;

/*====================================================================
函数名：CNodePool::Show()
功能：显示所有Node的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CNodePool::Show()
{
    u32 i, j;
	u32 dwLineCount = 0;
	char *StringLocalIPAddr;
	char *StringPeerIPAddr;
    u32 dwcount =0;
	struct sockaddr_in tLocalAddr , tPeerAddr;
	TOspNodeAddr tOspNodeAddr;
	OspPrintf(TRUE , FALSE , "print CNodePool info:\n");
	OspPrintf(TRUE , FALSE , "-----------------------\n");
    OspPrintf( TRUE, FALSE, "Osp MSG compression supported\n" );
	OspPrintf(TRUE, FALSE, "current max message waiting %d\n", g_max_msg_waiting);
	OspPrintf(TRUE, FALSE, "current max send interval %d\n", g_maxsend_interval);
	OspPrintf(TRUE, FALSE ,"current max recv interval %d\n", g_maxrecv_interval);
	OspPrintf(TRUE, FALSE ,"current max instance interval %d\n", max_inst_entry_interval);

	OspPrintf( TRUE, FALSE, "current max node num = %d\n", MAX_NODE_NUM );
	OspPrintf( TRUE, FALSE, "current max fdset size = %d\n", FD_SETSIZE );
    OspPrintf( TRUE, FALSE, "node server listen Sock = %d\n", m_tListenSock );
	OspPrintf( TRUE, FALSE, "node server listen Port = %d\n", m_wListenPort );
    OspPrintf( TRUE, FALSE, "node server localInSock = %d\n", m_tLocalInSock );
    OspPrintf( TRUE, FALSE, "node server localOutSock = %d\n", m_tLocalOutSock );
	OspPrintf( TRUE, FALSE, "telnetserver sock = %d\n", sockTelSer );
	OspPrintf( TRUE, FALSE, "telnetserver Port = %d\n", g_wportListtening );
	OspPrintf( TRUE, FALSE, "telnetserver client sock = %d\n", sockClient );

    for (i=0; i<MAX_NODE_NUM; i++ )
    {
        if(m_acNodeRegTable[i].m_bValid)
        {
            dwcount++;
			OspPrintf( TRUE, FALSE, "node=%d,sock=%d," , i+1 , m_acNodeRegTable[i].m_tSockHandle );
        //添加是否支持压缩的打印
            OspPrintf(TRUE, FALSE, "msg compression=%s,", g_Osp.m_cNodePool.m_acNodeRegTable[i].m_bCMessageCompressSupport == TRUE?"TRUE": "FALSE");

			if( TRUE == OspGetNodeAddr( i+1 , &tOspNodeAddr ) )
			{
				tLocalAddr.sin_addr.s_addr = tOspNodeAddr.m_dwLocalIP;
				StringLocalIPAddr= inet_ntoa( tLocalAddr.sin_addr );
				OspPrintf( TRUE , FALSE , "localAddr(%s@%d)," ,
					StringLocalIPAddr , tOspNodeAddr.m_wLocalPort );

				tPeerAddr.sin_addr.s_addr = tOspNodeAddr.m_dwPeerIP;
				StringPeerIPAddr= inet_ntoa( tPeerAddr.sin_addr );
				OspPrintf( TRUE , FALSE , "peerAddr(%s@%d)\n" ,
					StringPeerIPAddr , tOspNodeAddr.m_wPeerPort );

				dwLineCount +=2;
			}

            OspPrintf( TRUE, FALSE, "\tmsgSend=%d,msgRecv=%d,discCBNum=%d,discTime=%d,discHBs=%d\n",
				m_acNodeRegTable[i].m_dwMsgSended, m_acNodeRegTable[i].m_dwMsgRcved,
				m_acNodeRegTable[i].m_bDiscInformNum,m_acNodeRegTable[i].m_wDisTime,m_acNodeRegTable[i].m_byDisconnHBs);
			dwLineCount += 2;
            OspPrintf(TRUE, FALSE, "\tMaxSendTime=%dms(%luticks), MaxSendEvent=%d(%s), MaxSendMsgLength=%d\n",
                tickToMs(m_acNodeRegTable[i].m_dwMaxSendTicks),m_acNodeRegTable[i].m_dwMaxSendTicks,
                m_acNodeRegTable[i].m_wMaxSendEvent, g_Osp.m_cOspEventDesc.DescGet(m_acNodeRegTable[i].m_wMaxSendEvent),
                m_acNodeRegTable[i].m_wMaxSendMsgLen);
            if (m_acNodeRegTable[i].m_bDiscInformNum > 0)
            {
                OspPrintf(TRUE, FALSE, "Disconnect inform apps: ");
                for (j = 0; j < m_acNodeRegTable[i].m_bDiscInformNum; j++)
				{
                    OspPrintf(TRUE, FALSE, "%d,%d ", m_acNodeRegTable[i].m_wDiscInformAppId[j],
                            m_acNodeRegTable[i].m_wDiscInformInsId[j]);
				}
                OspPrintf(TRUE, FALSE, "\n");
				dwLineCount += 2+m_acNodeRegTable[i].m_bDiscInformNum;
            }
        }
		if( dwLineCount > 20 )
		{
			OspTaskDelay( 20 );
			dwLineCount = 0;
		}
    }


	OspPrintf(TRUE, FALSE, "Total node num listed is %d\n"
           "Total node disconnect times is %d\n"
		   "node disconnect by ConnEcho is %d\n"
		   "node disconnect by app is %d\n"
		   "node disconnect by send fail is %d\n"
		   "node disconnect by recv fail is %d\n",
           dwcount,
		   m_wNodeDisconnTimes,
		   m_wNodeHBDiscnTimes,
		   m_wNodeDiscnByApp,
		   m_wNodeDiscnBySendFailed,
		   m_wNodeDiscnByRecvFailed);
}



/*====================================================================
函数名：Regist
功能：将IP地址，socket和相应的数据赋与一个新node，并返回nodeId
算法实现：（可选项）
引用全局变量：
输入参数说明：dwIpAddr:		要注册的IP地址
			  dwSock:		相应的套接字
			  puNodeId:		[out]节点标识
			  wHb:			HeartBeat检测频率
			  byHbNum:		保持连接时允许的最大未响应Heartbeat次数

  返回值说明：
====================================================================*/
BOOL32 CNodePool::Regist(u32 dwIpAddr, SOCKHANDLE dwSock, u32 *puNodeId,u16 wHb,u8 byHbNum) // 返回路由ID,即pNOdeId
{
    u32 i;

	OspTaskSafe();
    OspSemTake(m_tSemaNodePool );

    static u32 nodeId = 0;

    for (i=0; i<MAX_NODE_NUM; i++ )
    {
        nodeId ++;
        nodeId = nodeId %  MAX_NODE_NUM;
        if(nodeId==0)  nodeId = 1;

        if( ! m_acNodeRegTable[nodeId-1].m_bValid )
        {
            m_acNodeRegTable[nodeId-1].m_bValid = TRUE;
            m_acNodeRegTable[nodeId-1].m_bListenning = TRUE;
            m_acNodeRegTable[nodeId-1].m_dwIpAddr = dwIpAddr ;
            m_acNodeRegTable[nodeId-1].m_tSockHandle = dwSock;

            m_acNodeRegTable[nodeId-1].m_dwMsgSended=0;
            m_acNodeRegTable[nodeId-1].m_dwMsgRcved=0;
            m_acNodeRegTable[nodeId-1].m_wDisTime = wHb;

			m_acNodeRegTable[nodeId-1].m_wDisTimeUsed = 0;
			m_acNodeRegTable[nodeId-1].m_byDisconnHBs = byHbNum;
			m_acNodeRegTable[nodeId-1].m_byDisconnHBsused = 0;
			m_acNodeRegTable[nodeId-1].m_bDiscCheckEnable = TRUE;
			m_acNodeRegTable[nodeId-1].m_bMsgEchoAck = TRUE;
            m_acNodeRegTable[nodeId - 1].m_bCMessageCompressSupport = FALSE; // by wubin 2011-02-22
            //<--
            m_acNodeRegTable[nodeId-1].m_dwMaxSendTicks = 0;
            m_acNodeRegTable[nodeId-1].m_wMaxSendEvent = 0;
            m_acNodeRegTable[nodeId-1].m_wMaxSendMsgLen = 0;
            //-->by wubin 2013-3-6
            *puNodeId = nodeId;
            if ((m_acNodeRegTable[nodeId - 1].m_pvRcvdData != NULL) ||
                (m_acNodeRegTable[nodeId - 1].m_dwRcvdLen != 0))
            {
                OspLog(1, "Osp: Found an corrupted node!\n");
                if (m_acNodeRegTable[nodeId - 1].m_pvRcvdData != NULL)
                {
                    OspFreeMem(m_acNodeRegTable[nodeId - 1].m_pvRcvdData);
                }
                m_acNodeRegTable[nodeId - 1].m_pvRcvdData = NULL;
                m_acNodeRegTable[nodeId - 1].m_dwRcvdLen = 0;
            }
            OspSemGive(m_tSemaNodePool);
			OspTaskUnsafe();
            return TRUE;
        }
    }
    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
    return FALSE;
}

// Node still registed , but post deamon stop listening from that node
/*====================================================================
函数名：NodeDisRcv
功能：Node still registed , but post deamon stop listening from that node
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNode: 目标节点ID

  返回值说明：
====================================================================*/
BOOL32 CNodePool::NodeDisRcv(u32 dwNode)
{
	if(dwNode==0 || dwNode>MAX_NODE_NUM)
		return FALSE;

	OspTaskSafe();
    OspSemTake(m_tSemaNodePool);

    if ( m_acNodeRegTable[dwNode-1].m_bValid )
    {
        m_acNodeRegTable[dwNode-1].m_bListenning = FALSE;
    }

    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
    return TRUE;
}

/*====================================================================
函数名：CNodePool::SetHBParam
功能：设置结点的链路检测参数
算法实现：（可选项）
引用全局变量：
输入参数说明：uNodeID: 结点号,
			  uHb: uHb秒钟检测一次,
			  uHbNum: uHbNum次无应答后, 主动断开.

  返回值说明：无.
====================================================================*/
BOOL32 CNodePool::SetHBParam(u32 nodeId, u16 wHb, u8 byHbNum)
{
	if(nodeId<=0 || nodeId>MAX_NODE_NUM)
	{
		return FALSE;
	}

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool);
	if(!m_acNodeRegTable[nodeId-1].m_bValid)
	{
		OspSemGive(m_tSemaNodePool);
		OspTaskUnsafe();
		return FALSE;
	}


	m_acNodeRegTable[nodeId-1].m_wDisTime = wHb;
	m_acNodeRegTable[nodeId-1].m_wDisTimeUsed = 0;
	m_acNodeRegTable[nodeId-1].m_byDisconnHBs = byHbNum;
	m_acNodeRegTable[nodeId-1].m_byDisconnHBsused = 0;

	OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();

	return TRUE;
}

/*====================================================================
函数名：NodeSockClose
功能：关闭一个指定Node的SOCKET
		这里只关闭SOCKET，但不发送断链消息。只用在发送出错时。
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNode:		目标节点ID

返回值说明：
====================================================================*/
void CNodePool::NodeSockClose(u32 dwNode)
{
    if(dwNode<=0 || dwNode>MAX_NODE_NUM)
    {
        return;
    }

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool);

    SockClose(m_acNodeRegTable[dwNode - 1].m_tSockHandle);
    m_acNodeRegTable[dwNode - 1].m_tSockHandle = INVALID_SOCKET;

    OspSemGive(m_tSemaNodePool);
    OspTaskUnsafe();
}

/*====================================================================
函数名：CNodePool::NodeUnRegistNoSema
功能：关闭一个指定的Node
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNode:		目标节点ID
			  byReason:		指明关闭原因

  返回值说明：
====================================================================*/
BOOL32 CNodePool::NodeUnRegist(u32 dwNode, u8 byReason)
{
	if(dwNode<=0 || dwNode>MAX_NODE_NUM)
	{
		return FALSE;
	}

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool);

	if( !m_acNodeRegTable[dwNode-1].m_bValid )
	{
		OspSemGive(m_tSemaNodePool);
		OspTaskUnsafe();

		return FALSE;
	}

  	if(m_acNodeRegTable[dwNode-1].m_bDiscInformNum != 0)
	{
		OspSemGive(m_tSemaNodePool);
		OspTaskUnsafe();

		for(int InformNum = 0; InformNum < m_acNodeRegTable[dwNode-1].m_bDiscInformNum; InformNum++)
		{
			NodeDiscCallBack(dwNode, m_acNodeRegTable[dwNode-1].m_wDiscInformAppId[InformNum],
				                     m_acNodeRegTable[dwNode-1].m_wDiscInformInsId[InformNum]);
		}

		OspTaskSafe();
		OspSemTake(m_tSemaNodePool);
	}

    if (m_acNodeRegTable[dwNode-1].m_tSockHandle != INVALID_SOCKET)
    {
    	SockClose(m_acNodeRegTable[dwNode-1].m_tSockHandle);
    }

    m_wNodeDisconnTimes++;

	switch(byReason)
	{
	case NODE_DISC_REASON_HBFAIL:
		m_wNodeHBDiscnTimes++;
		break;

	case NODE_DISC_REASON_SENDERR:
		m_wNodeDiscnBySendFailed++;
		break;

	case NODE_DISC_REASON_RECVERR:
		m_wNodeDiscnByRecvFailed++;
		break;

	case NODE_DISC_REASON_BYAPP:
		m_wNodeDiscnByApp++;
		break;

	default:
		break;
	}

	OspPrintf(1,0,"[osp]: node(%u) discon.reason is %u.\n",dwNode,byReason);
	m_acNodeRegTable[dwNode-1].m_tSockHandle=INVALID_SOCKET;
	m_acNodeRegTable[dwNode-1].m_bValid = FALSE;
	m_acNodeRegTable[dwNode-1].m_bListenning = FALSE;
	m_acNodeRegTable[dwNode-1].m_bDiscInformNum = 0;
	for (int InformNum = 0; InformNum < NODE_MAX_CBNUM; InformNum++)
    {
		m_acNodeRegTable[dwNode-1].m_wDiscInformAppId[InformNum] = INVALID_APP;
		m_acNodeRegTable[dwNode-1].m_wDiscInformInsId[InformNum] = INVALID_INS;
	}
	m_acNodeRegTable[dwNode-1].m_wDisTime = 1;
	m_acNodeRegTable[dwNode-1].m_wDisTimeUsed = 0;
	m_acNodeRegTable[dwNode-1].m_byDisconnHBs = DEFAULT_TCP_HEARTBEAT_NUM;
	m_acNodeRegTable[dwNode-1].m_byDisconnHBsused = 0;
	m_acNodeRegTable[dwNode-1].m_bMsgEchoAck = FALSE;
    m_acNodeRegTable[dwNode - 1].m_bCMessageCompressSupport = FALSE; //by wubin 2011-02-22

    //<--
    m_acNodeRegTable[dwNode-1].m_dwMaxSendTicks = 0;
    m_acNodeRegTable[dwNode-1].m_wMaxSendEvent = 0;
    m_acNodeRegTable[dwNode-1].m_wMaxSendMsgLen = 0;
            //-->by wubin 2013-3-6
    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();

	/* 通知PostDaemon连接状态改变以便PostDaemon改变套接口读集 */
	SockChangeNotify();
	return TRUE;
}

/*====================================================================
函数名：GetSock
功能：返回一个指定Node上的socket号
算法实现：（可选项）
引用全局变量：
输入参数说明：dwNodeId: 目标节点为ID
			  puSock:   [out]返回的套接字

  返回值说明：
====================================================================*/
BOOL32 CNodePool::GetSock(u32 dwNodeId, SOCKHANDLE *phSock)
{
	if(dwNodeId == 0 || dwNodeId>MAX_NODE_NUM)
		return FALSE;

	if(phSock == NULL)
		return FALSE;

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool);
    if(m_acNodeRegTable[dwNodeId-1].m_bValid)
    {
        *phSock = m_acNodeRegTable[dwNodeId-1].m_tSockHandle;
        OspSemGive(m_tSemaNodePool);
		OspTaskUnsafe();
        return TRUE;
    }
    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
    return FALSE;
}

/*====================================================================
函数名：CNodePool::IsNodeCheckEnable
功能：测试是否启动结点断链检测功能
算法实现：（可选项）
引用全局变量：
输入参数说明：node: 目标NODE

  返回值说明：
====================================================================*/
BOOL32 CNodePool::IsNodeCheckEnable(u32 node)
{
    if ( node >0 &&
		 node <= MAX_NODE_NUM &&
		 m_acNodeRegTable[node-1].m_bValid == TRUE )
    {
        return m_acNodeRegTable[node-1].m_bDiscCheckEnable;
    }

    return FALSE;
}

/*====================================================================
函数名：CNodePool::NodeCheckEnable
功能：启动结点断链检测功能
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CNodePool::NodeCheckEnable(u32 node)
{
	if(node == 0 || node > MAX_NODE_NUM)
		return;

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool );

    if(m_acNodeRegTable[node-1].m_bValid)
    {
		m_acNodeRegTable[node-1].m_bDiscCheckEnable = TRUE;
    }
    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
}

/*====================================================================
函数名：CNodePool::NodeCheckEnable
功能：禁止结点断链检测功能
算法实现：（可选项）
引用全局变量：
输入参数说明：node: 目标NODE

  返回值说明：
====================================================================*/
void CNodePool::NodeCheckDisable(u32 node)
{
	if(node == 0 || node > MAX_NODE_NUM)
		return;

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool );

    if(	m_acNodeRegTable[node-1].m_bValid )
    {
		m_acNodeRegTable[node-1].m_bDiscCheckEnable = FALSE;
    }
    OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
}
void CNodePool::UpdateMaxSend(u32 dwNode, u32 dwMaxSendTicks, u16 wMaxSendEvent, u16 wMaxSendLen) // add by wubin 2013-03-06
{
    if(dwNode == 0 || dwNode > MAX_NODE_NUM)
        return;

    OspTaskSafe();
    OspSemTake(m_tSemaNodePool );
    if(	m_acNodeRegTable[dwNode-1].m_bValid && dwMaxSendTicks >= m_acNodeRegTable[dwNode-1].m_dwMaxSendTicks )
    {
        m_acNodeRegTable[dwNode-1].m_dwMaxSendTicks = dwMaxSendTicks;
        m_acNodeRegTable[dwNode-1].m_wMaxSendEvent = wMaxSendEvent;
        m_acNodeRegTable[dwNode-1].m_wMaxSendMsgLen = wMaxSendLen;
    }
    OspSemGive(m_tSemaNodePool);
    OspTaskUnsafe();
}
/*====================================================================
函数名：CNodePool::NodePoolScan
功能：维护结点池中每个结点的连接状况，在结点扫描实例中调用
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CNodePool::Scan()
{
	u32 iNode;
	CNodeItem *pcNode = NULL;

	OspTaskSafe();
	OspSemTake(m_tSemaNodePool);

	/* 检查所有的节点是否有连接，发送检测消息 */
	for(iNode=1; iNode<=MAX_NODE_NUM; iNode++)
	{
		pcNode = &m_acNodeRegTable[iNode-1];

		/* 跳过未连接的结点和不要求检测的结点 */
		if( !pcNode->m_bValid || ( pcNode->m_wDisTime == UNDETECT_TCP_HEARTBEAT ) )
		{
			continue;
		}

        // 发送出错后会直接关闭SOCKET，并把SOCKET置为INVALID；
        // 但出错处理及通知APP的工作在这里做
        // 在NodeMsgSendToSock中不能直接NodeUnRegist，否则会有消息死锁
        if ((pcNode->m_bValid) && (pcNode->m_tSockHandle == INVALID_SOCKET))
        {
            OspSemGive(m_tSemaNodePool);
            OspTaskUnsafe();

            u32 dwip=OspNodeLastIpGet(iNode);
            OspLog(1, "Osp: connection with node %d (%u.%u.%u.%u) had errors, delete it.\n", iNode, OspQuadAddr(dwip));
            NodeUnRegist(iNode, NODE_DISC_REASON_SENDERR);

            OspSemTake(m_tSemaNodePool);
            OspTaskSafe();

            continue;
        }

		/* 根据设置，每wDisTime秒进行一次检测 */
		if(++pcNode->m_wDisTimeUsed >= pcNode->m_wDisTime)
		{
			pcNode->m_wDisTimeUsed = 0;

			if(pcNode->m_bMsgEchoAck) // 如果收到连接检测回应，复位结点状态, 继续检测
			{
				OspSemGive(m_tSemaNodePool);
				OspTaskUnsafe();

				OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_NETBRAECHO, 0, 0, iNode, MAKEIID(NODE_MAN_APPID,1));
                                        //INVALID_NODE, NODEMAN_MSG_TIMEOUT);

				OspSemTake(m_tSemaNodePool);
				OspTaskSafe();

				pcNode->m_bMsgEchoAck = FALSE;
				pcNode->m_byDisconnHBsused = 0;
			}
			else // 回应未到达
			{
				if(++pcNode->m_byDisconnHBsused >= pcNode->m_byDisconnHBs)  // 如果断链次数已用光, 拆链
				{
					pcNode->m_byDisconnHBsused = 0;

					OspSemGive(m_tSemaNodePool);
					OspTaskUnsafe();

                    u32 dwip=OspNodeLastIpGet(iNode);
					OspLog(1, "Osp: connection daemon checked node %d (%u.%u.%u.%u) disconnect, delete it.\n", iNode, OspQuadAddr(dwip));

					NodeUnRegist(iNode, NODE_DISC_REASON_HBFAIL); // add by xiang

					OspSemTake(m_tSemaNodePool);
					OspTaskSafe();
				}
				else  // 断链次数未用光
				{
					OspSemGive(m_tSemaNodePool);
					OspTaskUnsafe();

					OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_NETBRAECHO, 0, 0, iNode, MAKEIID(NODE_MAN_APPID,1));
                                               // INVALID_NODE, NODEMAN_MSG_TIMEOUT); // 再次发送

					OspSemTake(m_tSemaNodePool);
					OspTaskSafe();
				}
			} // 回应未到达
		} // 如果到了该检测的时间点
	} // end for

	OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
}

/*====================================================================
函数名：OspDispatchShow
功能：显示Dispatch task的信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspDispatchShow()
{
    g_Osp.m_cDispatchPool.Show();
}

/*====================================================================
函数名： OspAppShow
功能：显示App的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspAppShow()
{
    OspPrintf(TRUE,FALSE,"dwGloFailDispNode: %d ,dwGloFailPostNode: %d,dwGloFailPostApp: %d\n",g_Osp.m_cNodePool.m_dwGloFailDispNode,g_Osp.m_cNodePool.m_dwGloFailPostNode,g_Osp.m_cNodePool.m_dwGloFailPostApp);
    g_Osp.m_cAppPool.Show();
}

/*====================================================================
函数名： OspInstShow
功能：显示某个App中的Instance的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：aid: 要显示的APP的ID

  返回值说明：
====================================================================*/
API void OspInstShow(u16 aid)
{
	if(aid <= 0 || aid > MAX_APP_NUM)
	{
		return;
	}

	g_Osp.m_cAppPool.InstanceShow(aid);
}

/*====================================================================
函数名： OspInstShowAll
功能：显示所有Instance的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspInstShowAll()
{
	g_Osp.m_cAppPool.InstanceShowAll();
}

/*====================================================================
函数名： OspInstDump
功能：显示指定Instance的信息
算法实现：（可选项）
引用全局变量：
输入参数说明：aid:		目标APP号
			  InstId:	目标Instance
			  param:	传入参数

  返回值说明：
====================================================================*/
API void OspInstDump(u16 aid, u16 InstId, u32 param)
{
	if(aid <= 0 || aid > MAX_APP_NUM)
	{
		return;
	}

	g_Osp.m_cAppPool.InstanceDump(aid, InstId, param);
}

/*====================================================================
函数名：DispatchSysInit
功能：创建分派任务及其邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API BOOL32 DispatchSysInit(void)
{
	return g_Osp.m_cDispatchPool.Initialize();
}

/*====================================================================
函数名：DispatchSysQuit
功能：退出分派系统
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void DispatchSysExit(void)
{
	g_Osp.m_cDispatchPool.Quit();
}

/*====================================================================
函数名：CDispatchTask::CDispatchTask
功能：CDispatchTask类的构造函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
CDispatchTask::CDispatchTask()
{

}

/*====================================================================
函数名：CDispatchTask::Initialize
功能：分派任务初始化，初始化成员变量, 创建分派任务和邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
BOOL32 CDispatchTask::Initialize()
{
	m_dwNodeId = 0;
	m_dwMsgIncome = 0;
	m_dwMsgProcessed = 0;
	m_dwMsgWaitingTop = 0;
	m_dwMaxMsgWaiting = MAX_DISPATCHMSG_WAITING;
	pDispatchPool = &g_Osp.m_cDispatchPool;

    OspSemBCreate( &m_tSemMutex );

	if( !OspCreateMailbox("OspDispatchMailBox", MAX_DISPATCHMSG_WAITING, sizeof(TOsMsgStruc), &m_dwReadQue, &m_dwWriteQue) )
	{
		return FALSE;
	}

    m_hTask = OspTaskCreate(DispatchTaskEntry,        // Dispatch任务入口
		                    "OspDispatchTask",        // Dispatch任务名
							OSP_DISPATCH_TASKPRI,     // 任务优先级
							OSP_DISPATCH_STACKSIZE,   // 任务堆栈大小
							(KD_PTR)this,                // 参数
							0,                        // 任务创建标志
							&m_dwTaskID                // 返回的任务ID
							);
	if(m_hTask == 0)
	{
		OspCloseMailbox(m_dwReadQue, m_dwWriteQue);
		m_dwReadQue = 0;
        m_dwWriteQue = 0;
		return FALSE;
	}

	g_Osp.AddTask(m_hTask, m_dwTaskID, "OspDispatchTask");
	return TRUE;
}

/*====================================================================
函数名：CDispatchTask::Destroy
功能：释放邮箱, 杀死任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CDispatchTask::Quit()
{
	/* 释放邮箱 */
	OspCloseMailbox(m_dwReadQue, m_dwWriteQue);
	m_dwReadQue = 0;
	m_dwWriteQue = 0;

	/* 删除信号量 */
	OspSemDelete(m_tSemMutex);
	m_tSemMutex = NULL;

	/* 退出本任务 */
    m_hTask = 0;
	m_dwTaskID = 0;
}

/*====================================================================
函数名：CDispatchTask::MsgIncomeInc
功能：Dispatch任务邮箱的等待消息数加1，由于需要在多个读写该值，必须加上
      信号量保护。
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CDispatchTask::MsgIncomeInc()
{
	OspSemTake(m_tSemMutex);
    m_dwMsgIncome++;
    if ((m_dwMsgIncome > m_dwMsgProcessed) &&
            ((m_dwMsgIncome - m_dwMsgProcessed) > m_dwMsgWaitingTop))
    {
        m_dwMsgWaitingTop = m_dwMsgIncome - m_dwMsgProcessed;
    }
	OspSemGive(m_tSemMutex);
}

void CDispatchTask::MsgIncomeDec()
{
	OspSemTake(m_tSemMutex);
        if (m_dwMsgIncome > 0) m_dwMsgIncome--;
	OspSemGive(m_tSemMutex);
}

/*====================================================================
函数名：CDispatchTask::MsgIncomeNum
功能：取得Dispatch任务邮箱的等待消息数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
u32 CDispatchTask::MsgIncomeNum()
{
	u32 dwTemp;

	OspSemTake(m_tSemMutex);
    dwTemp = m_dwMsgIncome;
	OspSemGive(m_tSemMutex);

	return dwTemp;
}

/*====================================================================
函数名：CDispatchTask::MsgWaitingNum
功能：取得Dispatch任务邮箱的等待消息数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
u32 CDispatchTask::MsgWaitingNum()
{
	u32 dwTemp = 0;

	OspSemTake(m_tSemMutex);
    if( m_dwMsgIncome > m_dwMsgProcessed )
    {
        dwTemp = m_dwMsgIncome - m_dwMsgProcessed;
    }
	OspSemGive(m_tSemMutex);

	return dwTemp;
}

API BOOL32 OspSockSend(SOCKHANDLE tSock, const char * pchBuf, u32 dwLen)
{
    int ret = SOCKET_ERROR;
    u32 dwTotalSendLen = 0;

    if(tSock == INVALID_SOCKET)
    {
		return FALSE;
    }

    if(pchBuf == NULL)
    {
		return FALSE;
    }

    while( dwTotalSendLen < dwLen )
    {
		ret = send(tSock, (char*)(pchBuf+dwTotalSendLen), (dwLen-dwTotalSendLen), SOCK_SEND_FLAGS);
		if(SOCKET_ERROR == ret || ret != dwLen)
		{
			OspLog(1, "Osp: sock send error, errno %d\n", errno);
			return FALSE;
		}

		dwTotalSendLen += ret;
    }

    return TRUE;
}

/*====================================================================
函数名：CDispatchTask::NodeMsgSendToSock
功能：从邮箱中接收消息并通过套接口函数转发给外部结点
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void CDispatchTask::NodeMsgSendToSock()
{
    CMessage *pMsg = NULL;
	CMessageForSocket tMsg4S;
	char achBuf[MAX_LOG_MSG_LEN];
	TOsMsgStruc osMsg;
	u32 dstNode = 0;
    u16 wEvent;
	u32 lenrcv = 0;
	BOOL32 bRet = FALSE;
	int len;

	while(TRUE)
    {
        bRet = OspRcvMsg(m_dwReadQue, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &lenrcv);
        if((bRet == FALSE) || (lenrcv <= 0) )
		{
			OspLog(1, "Osp: dispatch task OspRevMsg fail, ret=%d, lenrcv=%d, errno:%d.\n",\
			       bRet, lenrcv, errno);
			printf("Osp: dispatch task OspRevMsg fail, ret=%d, lenrcv=%u, errno:%d.\n", \
				   bRet, lenrcv,errno);
			continue;
		}

		/* 接收计数 */
		m_dwMsgProcessed++;

		/* 如果邮箱为空, 唤醒等待空闲Dispatcher的任务 */
        if(MsgWaitingNum() == 0)
		{
			OspSemGive(pDispatchPool->m_tSemTaskFull);
		}

		pMsg = (CMessage *)osMsg.address;

        /* 收到退出命令 */
		if(pMsg->event == OSP_QUIT)
		{
			/* 释放发送者为本消息申请的内存 */
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10

			/* 释放邮箱等, 删除信号量 */
			Quit();

			/* 最后退出本任务 */
			OspTaskExit();
		}

		if( (pMsg->length > MAX_MSG_LEN) || (pMsg->dstAliasLen > MAX_ALIAS_LEN) )
		{
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
            u32 dwip=OspNodeLastIpGet(dstNode);
            OspLog(1, "Osp: dispatch task to node %d (%u.%u.%u.%u) message length error, message length=%d, alias length=%d.\n", dstNode, OspQuadAddr(dwip), pMsg->length, pMsg->dstAliasLen);
            continue;
		}

		dstNode = pMsg->dstnode;
         wEvent = OspGetOriginEvent(pMsg);
        // see if dstnode is error
        if( dstNode <= 0 || dstNode > MAX_NODE_NUM )
        {
            OspLog(1, "Osp: system error: internal message to out dispatch queue\n");
			OspLog(1, "pMsg srcnode(%d) dstnode(%d) dstid(%d) srcid(%d) type(%d) event(%d) length(%d)\n" ,
				pMsg->srcnode , pMsg->dstnode , pMsg->dstid , pMsg->srcid , pMsg->type , pMsg->event , pMsg->length );
			OspFreeMem(pMsg);
            continue;
        }

        // find the destination socket
        SOCKHANDLE dstSock;
        if( ! g_Osp.m_cNodePool.GetSock(dstNode, &dstSock) )
        {
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
            u32 dwip=OspNodeLastIpGet(dstNode);
            OspLog(255, "Osp: dispatch task to node %d (%u.%u.%u.%u) not exists.\n", dstNode, OspQuadAddr(dwip));
            g_Osp.m_cNodePool.m_dwGloFailDispNode++;
            continue;
        }

        if(g_Osp.m_cNodePool.m_dwSendTrcFlag > 0)
        {
        	OspPrintf(TRUE,FALSE, "Osp : Start Send a massage to node %d\r\n",pMsg->dstnode);

			len = sprintf(achBuf, "\nOsp: message send to node %d\n", pMsg->dstnode);
			len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pMsg);
			OspMsgTrace(TRUE, TRUE, achBuf, len);
        }

		/* 计算消息长度, 必须在网络字节序转换前做 */
		u16 wPackSize = tMsg4S.GetPackedSize();
        u32 dwTotalMsgLen = wPackSize + pMsg->length + pMsg->dstAliasLen;

		tMsg4S.srcnode = pMsg->srcnode;
		tMsg4S.dstnode = pMsg->dstnode;
		tMsg4S.dstid = pMsg->dstid;
		tMsg4S.srcid = pMsg->srcid;
		tMsg4S.event = pMsg->event;
		tMsg4S.type = pMsg->type;
		tMsg4S.length = pMsg->length;
		tMsg4S.content = NULL;
#ifdef SYNCMSG
		tMsg4S.expire = pMsg->expire;
		tMsg4S.outlen = pMsg->outlen;
		tMsg4S.output = NULL;
#endif
		tMsg4S.dstAliasLen = pMsg->dstAliasLen;
		tMsg4S.dstAlias = NULL;
		if (pMsg->length > 0)
		{
			memcpy(&achBuf[wPackSize], pMsg->content, pMsg->length);
		}
		if (pMsg->dstAliasLen > 0)
		{
			memcpy(&achBuf[wPackSize + pMsg->length], pMsg->dstAlias, pMsg->dstAliasLen);
		}
		MsgHton(&tMsg4S);
		memcpy(achBuf, &tMsg4S, wPackSize);
		/* 消息头部转换为网络字节序 */

		u32 before, after, temp;

		before = OspTickGet();

		/* 在套接口上发送本消息 */
        if( ! OspSockSend(dstSock, achBuf, dwTotalMsgLen) )
        {
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
            u32 dwip=OspNodeLastIpGet(dstNode);
            OspLog(1, "Osp:  Dispatchtask send message to node %d (%u.%u.%u.%u) fail.\n", dstNode, OspQuadAddr(dwip));
			int win_error = WSAGetLastError();
			OspLog(1, "\nOsp: send errno = %d\n", win_error);

	   	   after = OspTickGet();
		   temp = after - before;
		   if ( temp > g_maxsend_interval )
		   {
			   g_maxsend_interval = temp;
		   }

			if(g_Osp.m_cNodePool.m_dwSendTrcFlag > 0)
			{
				OspPrintf(TRUE,FALSE, "Osp : End Send a massage to node %d error\r\n",pMsg->dstnode);
			}
            g_Osp.m_cNodePool.NodeUnRegist(dstNode, NODE_DISC_REASON_SENDERR);
            g_Osp.m_cNodePool.NodeSockClose(dstNode);
			continue;
        }
		if(g_Osp.m_cNodePool.m_dwSendTrcFlag > 0)
		{
			OspPrintf(TRUE,FALSE, "Osp : End Send a massage to node %d\r\n",pMsg->dstnode);
		}

		after = OspTickGet();
		temp = after - before;
		if ( temp > g_maxsend_interval )
		{
			g_maxsend_interval = temp;
		}
        g_Osp.m_cNodePool.UpdateMaxSend(dstNode, temp, wEvent, dwTotalMsgLen);
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
    } // while(TRUE)
}

/*====================================================================
函数名：CDispatchPool::Initialize
功能：创建池中各分派任务及其邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
BOOL32 CDispatchPool::Initialize()
{
	OspSemBCreate( &m_tSemTaskFull);
//	OspSemBCreate( &m_tSemDispatchPool);

	for(int i=0; i<THREADNUM; i++)
	{
		m_acDispTasks[i].Initialize();
	}

	return TRUE;
}

/*====================================================================
函数名：CDispatchPool::Quit
功能：退出所有分派任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：无.
====================================================================*/
void CDispatchPool::Quit()
{
	BOOL32 bSuccess;
	CMessage *pcQuitMsg = NULL;
	TOsMsgStruc osMsg;

	for(int taskSeq=0; taskSeq<THREADNUM; taskSeq++)
	{
		pcQuitMsg = (CMessage *)OspAllocMem(sizeof(CMessage));
//		pcQuitMsg = (CMessage *)g_Osp.m_cMsgStack.GetStack();   //11-10

		pcQuitMsg->event = OSP_QUIT;
		osMsg.address = (void*)pcQuitMsg;
		bSuccess = OspSndMsg(g_Osp.m_cDispatchPool.m_acDispTasks[taskSeq].m_dwWriteQue, (char *)&osMsg, sizeof(TOsMsgStruc));
		if( !bSuccess )
		{
			OspFreeMem(pcQuitMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pcQuitMsg) ;   //11-10
			// 在OSP退出时，LOG任务已经不能再使用，因此要直接打印
			printf("Osp: send message to mailbox failed in OspQuit().\n");
		}
	}

	OspSemDelete(m_tSemTaskFull);
    m_tSemTaskFull = NULL;
}

/*====================================================================
函数名：CDispatchPool::Show
功能：退出所有分派任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
void CDispatchPool::Show(void)
{
	u32 taskSeq;
	OspPrintf( TRUE , FALSE , "print CDispatchPool state:\n");
	OspPrintf( TRUE , FALSE , "---------------------------------\n");

	for( taskSeq=0; taskSeq < THREADNUM; taskSeq++ )
	{
		OspPrintf( TRUE , FALSE , "current dipatchtask id %d\n" , taskSeq );
		OspPrintf( TRUE , FALSE , "current send message to node %d\n" , m_acDispTasks[taskSeq].m_dwNodeId );
		OspPrintf( TRUE , FALSE , "maxMsgWaiting %d\n" , m_acDispTasks[taskSeq].m_dwMaxMsgWaiting );
		OspPrintf( TRUE , FALSE , "msgIncome %d\n" , m_acDispTasks[taskSeq].MsgIncomeNum() );
		OspPrintf( TRUE , FALSE , "msgProcessed %d\n" , m_acDispTasks[taskSeq].m_dwMsgProcessed );
		OspPrintf( TRUE , FALSE , "msgWaitingTop %d\n" , m_acDispTasks[taskSeq].m_dwMsgWaitingTop );
	}
}

/*====================================================================
函数名：CDispatchPool::NodeMsgPost
功能：将发送给外部结点的消息转发给Dispatch任务
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：dstNode: 目标结点,
              content: 指向待发消息的指针,
              length: 消息长度.

  返回值说明：
====================================================================*/
s32 CDispatchPool::NodeMsgPost(u32 dstNode, const char * content, u32)
{
	BOOL32 bSuccess;
	TOsMsgStruc osMsg;
    CNodeItem *pNode = NULL;

	osMsg.address = (void*)content;

	/* 结点发送消息计数 */
	pNode = g_Osp.m_cNodePool.NodeGet(dstNode);
	if(pNode != NULL)
	{
		pNode->msgSendedInc();
	}

    m_acDispTasks[0].MsgIncomeInc();
	m_acDispTasks[0].m_dwNodeId = dstNode;
    bSuccess = OspSndMsg( m_acDispTasks[0].m_dwWriteQue, (char *)&osMsg, sizeof(TOsMsgStruc));
    if( !bSuccess )
    {
        OspFreeMem((void *)osMsg.address);
        OspLog(1,"Osp: send message failed in CDispatchPool::NodeMsgPost().\n");
        m_acDispTasks[0].MsgIncomeDec();
        return OSP_ERROR;
    }
    return OSP_OK;
}
//<--消息压缩by wubin 2011-02-22
/*====================================================================
  函数名：OspCompressMessagePack
  功能：将已经成数据帧的CMessage的消息体进行压缩
  算法实现：
            消息体(content)的前2个字节保存原消息号，之后2个字节保存原消息体长度，之后为压缩后消息体的数据
  引用全局变量：
  输入参数说明：
  返回值说明：
  ====================================================================*/

BOOL32 OspCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen)
{
    u32 dwAllocMsgSize = 0;             //预分配压缩后数据缓冲长度
    u16 *pwUncompressEvent = NULL;      //存放原消息号缓冲的指针
    u16 *pwUncompressContentLength = NULL; //存放原消息长度缓冲的指针
    u32 dwCompressContentLength = 0;    //未压缩缓冲长度
    u8 *pbyCompressContentBuf = NULL;   //未压缩缓冲指针
    CMessage *pCUncompressedMessage = NULL; //未压缩CMessage指针
    CMessage *pCCompressedMessage = NULL; //压缩后CMessage指针

    if(ppMsg && pdwMsgLen &&*ppMsg)
    {
        pCUncompressedMessage = *ppMsg;
        //获取消息体压缩后可能大小的最大值
        dwCompressContentLength = OspGetCompressLengthBound(*(pdwMsgLen) - sizeof(CMessage) - pCUncompressedMessage->dstAliasLen);
        //使用OspAllocMem分配新的CMessage
        dwAllocMsgSize = sizeof(CMessage) + sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength + pCUncompressedMessage->dstAliasLen;
        pCCompressedMessage = (CMessage *)OspAllocMem(dwAllocMsgSize);
        if (NULL == pCCompressedMessage)
        {
            return FALSE;
        }
        memset(pCCompressedMessage, 0, dwAllocMsgSize);

        //指定压缩后原消息号、原消息体长度以及压缩后数据的缓冲指针
        pwUncompressEvent = (u16 *)((u8 *)pCCompressedMessage + sizeof(CMessage));
        pwUncompressContentLength = (u16 *)((u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16));
        pbyCompressContentBuf = (u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16) + sizeof(u16);
        //进行数据压缩
        if (TRUE == OspCompressData(pbyCompressContentBuf, &dwCompressContentLength, pCUncompressedMessage->content, (u32)pCUncompressedMessage->length))
        {
            //确保压缩后数据长度能够减少
            if (sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength < pCUncompressedMessage->length)
            {
                //复制消息参数
                memcpy(pCCompressedMessage, pCUncompressedMessage, sizeof(CMessage));
                pCCompressedMessage->event = OSP_COMPRESS_MSG;
                pCCompressedMessage->length = sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength;
                pCCompressedMessage->content = pbyCompressContentBuf - sizeof(u16) - sizeof(u16);
                pCCompressedMessage->dstAlias = (s8 *)pbyCompressContentBuf + (u16)dwCompressContentLength;
                *pwUncompressEvent = pCUncompressedMessage->event;
                *pwUncompressContentLength = pCUncompressedMessage->length;
	            memcpy(pCCompressedMessage->dstAlias, pCUncompressedMessage->dstAlias, pCUncompressedMessage->dstAliasLen);
                //使用新的CMessage，并使用OspFreeMem销毁原CMessage
                *ppMsg = pCCompressedMessage;
                *pdwMsgLen = sizeof(CMessage) + pCCompressedMessage->length + pCCompressedMessage->dstAliasLen;
                OspFreeMem(pCUncompressedMessage);
                pCUncompressedMessage = NULL;
                return TRUE;
            }
        }
        //如果压缩失败或者没有压缩效果，则销毁新建CMessage，使用原来的
        OspFreeMem(pCCompressedMessage);
        pCCompressedMessage = NULL;
    }
    return FALSE;
}

/*====================================================================
  函数名：OspUnCompressMessagePack
  功能：将已经成数据帧的CMessage的消息体进行解压
  算法实现：
            消息体(content)的前2个字节保存原消息号，之后2个字节保存原消息体长度，之后为压缩后消息体的数据
  引用全局变量：
  输入参数说明：
  返回值说明：
  ====================================================================*/

BOOL32 OspUnCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen)
{
    u32 dwAllocMsgSize = 0;             //预分配解压后数据缓冲长度
    u16 *pwUncompressEvent = NULL;      //存放原消息号缓冲的指针
    u16 *pwUncompressContentLength = NULL;//存放原消息长度缓冲的指针
    u8 *pbyUncompressContentBuf = NULL;//解压后缓冲指针
    u32 dwUncompressContentLength = 0;//解压后缓冲长度
    CMessage *pCUncompressedMessage = NULL; //解压后CMessage指针
    CMessage *pCCompressedMessage = NULL; //未解压CMessage指针


    if((NULL != ppMsg) && (NULL != pdwMsgLen) && (NULL != *ppMsg))
    {
        pCCompressedMessage = *ppMsg;
        if (OSP_COMPRESS_MSG != pCCompressedMessage->event)
        {
            return TRUE; //如果不是压缩消息，直接返回成功
        }
        //获取原消息体长度
        pwUncompressContentLength = (u16*)((u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16));
        dwAllocMsgSize = sizeof(CMessage) + *pwUncompressContentLength + pCCompressedMessage->dstAliasLen;
        //使用OspAllocMem分配新的CMessage
        pCUncompressedMessage = (CMessage *)OspAllocMem(dwAllocMsgSize);
        if(NULL == pCUncompressedMessage)
        {
            return FALSE;
        }
        memset(pCUncompressedMessage, 0, dwAllocMsgSize);

        //设定原消息号、原消息体长度以及压缩后数据的缓冲指针
        pwUncompressEvent = (u16*)((u8 *)pCCompressedMessage + sizeof(CMessage));
        pbyUncompressContentBuf = (u8 *)pCUncompressedMessage + sizeof(CMessage);
        dwUncompressContentLength = (u32)*pwUncompressContentLength;
        //解压数据
        if (TRUE == OspUnCompressData(pbyUncompressContentBuf, &dwUncompressContentLength, (u8 *)pCCompressedMessage->content + sizeof(u16) + sizeof(u16), (u16)(pCCompressedMessage->length - sizeof(u16) - sizeof(u16))))
        {
            //检验解压数据长度是否正确
            if(dwUncompressContentLength == (u32)*pwUncompressContentLength)
            {
                //复制消息参数
                memcpy(pCUncompressedMessage, pCCompressedMessage, sizeof(CMessage));
                pCUncompressedMessage->event = *pwUncompressEvent;
                pCUncompressedMessage->length = (u16)dwUncompressContentLength;
                pCUncompressedMessage->content = pbyUncompressContentBuf;
                pCUncompressedMessage->dstAlias = (s8 *)pbyUncompressContentBuf + dwUncompressContentLength;
	            memcpy(pCUncompressedMessage->dstAlias, pCCompressedMessage->dstAlias, pCCompressedMessage->dstAliasLen);
                //使用新的CMessage，并使用OspFreeMem销毁原CMessage
                *ppMsg = pCUncompressedMessage;
                *pdwMsgLen = sizeof(CMessage) + pCCompressedMessage->length + pCCompressedMessage->dstAliasLen;
                OspFreeMem(pCCompressedMessage);
                pCCompressedMessage = NULL;
                return TRUE;
            }
        }
        OspFreeMem(pCUncompressedMessage);
        pCUncompressedMessage = NULL;
    }
    return FALSE;
}

/*====================================================================
  函数名：OspGetCompressLengthBound
  功能：获取压缩后数据长度的最大值
  算法实现：
  引用全局变量：
  输入参数说明：
  返回值说明：
  ====================================================================*/
u32 OspGetCompressLengthBound(u32 dwUncompressedLength, u8 byAlgorithm)
{
    return compressBound(dwUncompressedLength); //
}

/*====================================================================
  函数名：OspCompressData
  功能：使用压缩算法压缩数据
  算法实现：
  引用全局变量：
  输入参数说明：
  返回值说明：
  ====================================================================*/
BOOL32 OspCompressData(u8* pbyCompressedBuf, u32* pdwCompressedBufLen, u8* pbyUncompressedBuf, u32 dwUncompressedBufLen, u8 byAlgorithm)
{
	uLong CompressedBufLen, UnCompressedBufLen;
	CompressedBufLen = (uLong)*pdwCompressedBufLen;
	UnCompressedBufLen = (uLong)dwUncompressedBufLen;
    if(Z_OK == compress(pbyCompressedBuf, &CompressedBufLen, pbyUncompressedBuf, UnCompressedBufLen))
    {
		*pdwCompressedBufLen = (u32)CompressedBufLen;
        return TRUE;
    }
    return FALSE;

}

/*====================================================================
  函数名：OspUnCompressData
  功能：使用解压算法解压数据
  算法实现：
  引用全局变量：
  输入参数说明：
  返回值说明：
  ====================================================================*/
BOOL32 OspUnCompressData(u8* pbyUncompressedBuf, u32* pdwUncompressedBufLen, u8* pbyCompressedBuf, u32 dwCompressedBufLen, u8 byAlgorithm)
{
uLong CompressedBufLen, UnCompressedBufLen;
    CompressedBufLen = (uLong)dwCompressedBufLen;
    UnCompressedBufLen = (uLong)*pdwUncompressedBufLen;
    s32 nZlibRet = uncompress(pbyUncompressedBuf, &UnCompressedBufLen, pbyCompressedBuf, CompressedBufLen);
    if(Z_OK == nZlibRet)
    {
        *pdwUncompressedBufLen = (u32)UnCompressedBufLen;
        return TRUE;
    }
    return FALSE;
}

u16 OspGetOriginEvent(CMessage * pcMsg)
{
    u16 * pwEvent = NULL;
    if (pcMsg->event != OSP_COMPRESS_MSG)
    {
        return pcMsg->event;
    }
    pwEvent = (u16 *)((u8 *)pcMsg + sizeof(CMessage));
    if (NULL == pwEvent)
    {
        return OSP_COMPRESS_MSG;
    }
    return *pwEvent;
}
//by wubin 2011-4-13-->

