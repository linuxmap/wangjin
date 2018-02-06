/******************************************************************************
ģ����  �� OSP
�ļ���  �� ospPost.cpp
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��Ϣ���͹��ܵ���Ҫʵ���ļ�
����    �����Ľ�
�汾    ��1.0.02.7.5
--------------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
05/24/2003  2.1         ���        �淶��; �ڲ���Ϣ�㿽��
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
�� �� ����OspEnableBroadcastAck
��    �ܣ����������£��û����͹㲥��Ϣ������Ҫ�Է�����ȷ����Ϣ��ȱʡ����£�
          PIS������Ϣ�ķ����߷���OSP_BROADCASTACK��Ϣ�����ǿ���ͨ���ú����޸���
          ��ѡ��Ƿ���OSP_BROADCASTACK���ɽ��ܽڵ�����ġ�
ע    �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��    ����u16 aid : [in] ��ʾ�㲥��Ϣ�Ľ�����AID��
          BOOL32 bEnable : [in] enable or not
�� �� ֵ��BOOL32 - true Succeeded, false Failed
=============================================================================*/
API BOOL32 OspEnableBroadcastAck(u16, BOOL32)
{
    return FALSE;
}

/*====================================================================
�� �� ����CreateTcpNodeNoRegist
���ܣ��ڵ�ַuAddr�Ͷ˿�uPort�ϴ���һ�������׽��֣����׽��ֲ�ע�ᵽ���
      ���С�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����uAddr: ����IP��ַ��uPort: �˿ں�

  ����ֵ˵�����ɹ����ؿ��õ��׽���, ʧ�ܷ���INVALID_SOCKET.
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
�� �� ����OspConnectToSock5Proxy
��	  �ܣ���sock5�������������TCP��·������TCP��UDP��Խsock5���������Ƚ�����TCP��·��
ע	  �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��	  ����ptOspSock5Proxy : [in] �����������Ϣ�ṹ
		  dwTimeoutMs : [in] ������ʱʱ��
�� �� ֵ��ʧ�ܷ���INVALID_SOCKET��
		  �ɹ����������ͨ�ŵ�TCP Socket���ɽ�һ������OspConnectTcpNodeThroughSock5Proxy����TCP�����
		  ��OspUdpAssociateThroughSock5Proxy����UDP Associate��
		  �������TCP����˳ɹ����������������OspDisconnectTcpNode��
		  �������TCP�����ʧ�ܣ�osp�ڲ�����OspDisconnectFromSock5Proxy�ͷ�OspConnectToSock5Proxy�õ���socket��
		  ����ǽ���UDP Associate����Ҫ�ϲ�ά����TCP���ӣ��������������OspDisconnectFromSock5Proxy��
-------------------------------------------------------------------------------
 �޸ļ�¼��
 �� 	 ��  �汾  �޸���  �޸�����
 2006/08/21  4.0   ��С��
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
		//���֧���û��������Ȩ���ж���Ч��
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

    //����һ�� socket
    tSock = socket( AF_INET, SOCK_STREAM, 0 );
    if( tSock == INVALID_SOCKET )
    {
        OspLog(1, "Osp: OspConnectToSock5Proxy() create socket failed!\n");
        return INVALID_SOCKET;
    }

    OspLog( 21, "Osp: OspConnectToSock5Proxy %s@%d, please wait...\n", inet_ntoa( tSvrINAddr.sin_addr ), ptOspSock5Proxy->m_wProxyPort );

	//���ӷ�����
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

	/* �����׽���ѡ��: ��������(�������κα��⾺�����㷨) */
    optVal = 1;
    setsockopt(tSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optVal, sizeof(optVal));

	/* �����׽���ѡ��: �����׽ӿڷ��ͽ��ջ���Ĵ�С */
	optVal = SOCKET_SEND_BUF;
    setsockopt(tSock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, sizeof(optVal));
	optVal = SOCKET_RECV_BUF;
    setsockopt(tSock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));

	/* �����׽���ѡ��: ����TCP���Ӽ�⹦�� */
	optVal = 1;
    setsockopt(tSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optVal, sizeof(optVal));


	OspLog(21, "Osp: OspConnectToSock5Proxy %s@%d OK!\n", inet_ntoa( tSvrINAddr.sin_addr ), ptOspSock5Proxy->m_wProxyPort );

	//���ͼ�Ȩ����
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

	//�ȴ���Ȩ�ظ�
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

	//����sock5����֧�ֵļ�Ȩ����
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
	//�����Ҫ�û��������Ȩ����������֤����
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

			//�ȴ���Ȩ�ظ�
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

			//�û�����Ȩ�Ƿ�ɹ�
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
�� �� ����OspConnectTcpNodeThroughSock5Proxy
��	  �ܣ�TCP��Խsock5�������ӷ���ˣ���OspConnectTcpNode��ͬ��
ע	  �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��	  ����ptOspSock5Proxy : [in] �����������Ϣ�ṹ;
		  dwServerIP : [in] ������IP
		  wServerPort : [in] �������˿�
		  wHb: [in] �����������(��)
		  byHbNum: [in] byHbNum��δ�����Ӽ��Ӧ�����Ϊ��·�ѶϿ�
		  dwTimeoutMs : [in] ������ʱʱ��
		  pdwLocalIP: [in,out] ��TCP����ʹ�õı���IP
�� �� ֵ��ʧ�ܷ���INVALID_NODE��
		  �ɹ�����һ����̬����Ľ���, �˺��û��ɽ���������ͨ��
		  �ϲ��������������OspDisconnectTcpNode���������޴���ʱ������
-------------------------------------------------------------------------------
 �޸ļ�¼��
 �� 	 ��  �汾  �޸���  �޸�����
 2006/08/21  4.0   ��С��
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

	//���ʹ�Խsock5������������
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

	//�ȴ���������ظ�
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

	/* Ϊ���ӳɹ����׽����ڽ����з���һ���Ľ��� */
    if( !g_Osp.m_cNodePool.Regist( 0 , hSocket, &nodeId, wHb, byHbNum ) )
	{
		OspLog(1, "Osp: OspConnectTcpNodeThroughSock5Proxy() regist socket failed, close it.\n");
		OspDisConnectFromSock5Proxy(hSocket);
		return INVALID_NODE;
	}


    // ��ȡ��ǰ�������õ�IP��ַ
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

    OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, nodeId); // ����node��֪ͨ�Է�����֧��ѹ����Ϣ by wubin 2011-02-22
	SockChangeNotify();
    return nodeId;
}


/*=============================================================================
�� �� ����OspUdpAssociateThroughSock5Proxy
��	  �ܣ�UDP��Խsock5����
ע	  �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��	  ����hSocket : [in] OspConnectToSock5Proxy���ص�socket(�ɸ���);
		  dwLocalIP : [in] �����շ�socket IP���Ա����������������ݴ�Խ��������
		  wLocaPort : [in] �����շ�socket �˿ڣ��Ա����������������ݴ�Խ��������
		  pdwProxyMapIP : [out] ���������ӳ��IP��������
		  pwProxyMapPort : [out] ���������ӳ��˿ڣ�������
		  dwTimeoutMs : [in] ������ʱʱ��
�� �� ֵ��ʧ�ܷ���FALSE��
		  �ɹ�����TRUE
-------------------------------------------------------------------------------
 �޸ļ�¼��
 �� 	 ��  �汾  �޸���  �޸�����
 2006/08/21  4.0   ��С��
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

	//����UDP Associate����
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

	//�ȴ���������ظ�
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
�� �� ����OspDisConnectFromSock5Proxy
��	  �ܣ���Խsock5�������ӷ����
ע	  �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��	  ����hSocket : [in] OspConnectToSock5Proxy���ص�socket;
�� �� ֵ��ʧ�ܷ���FALSE��
		  �ɹ�����TRUE
-------------------------------------------------------------------------------
 �޸ļ�¼��
 �� 	 ��  �汾  �޸���  �޸�����
 2006/08/21  4.0   ��С��
=============================================================================*/
API BOOL32 OspDisConnectFromSock5Proxy( SOCKHANDLE hSocket )
{
	if( INVALID_SOCKET == hSocket )
		return FALSE;
	SockClose( hSocket );
	return TRUE;
}

/*====================================================================
��������OspCreateTcpNode
���ܣ��ڵ�ַuAddr�Ͷ˿�uPort�ϴ���һ�������׽��ֲ������׽���ע�ᵽ���
      ���С�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAddr: ����IP��ַ��uPort: �˿ں�

����ֵ˵�����ɹ����ؿ��õ��׽���, ʧ�ܷ���INVALID_SOCKET.
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
�� �� ����OspConnectTcpNode
��    �ܣ��ڵ�ַuIpv4Addr�Ͷ˿�uTcpPort�����ӷ�����, �����ö���������.
ע    �⣺
�㷨ʵ�֣�
ȫ�ֱ�����g_Osp
��    ����uIpv4Addr : [in] �������IP��ַ,
          uTcpPort : [in] ������������˿ں�,
		  uHb: [in] �����������(��),
          uHbNum: [in] uHbNum��δ�����Ӽ��Ӧ�����Ϊ��·�ѶϿ�.

�� �� ֵ���ɹ�����һ����̬����Ľ���, �˺��û��ɽ���������ͨ��.
          ʧ�ܷ���INVALID_NODE.
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

	/* ׼��SERVER��ַ */
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wTcpPort);
    tSvrINAddr.sin_addr.s_addr = dwIpv4Addr;

    /* ����һ�� socket */
    tSock = socket(AF_INET, SOCK_STREAM, 0);
    if(tSock == INVALID_SOCKET)
    {
//        OspPrintf(TRUE, TRUE, "Osp: OspConnectTcpNode() create socket failed!\n");
        OspLog(1, "Osp: OspConnectTcpNode() create socket failed!\n");
        return INVALID_NODE;
    }

//    OspPrintf(TRUE, TRUE, "Osp: connecting to %s@%d, please wait...\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort);
    OspLog(1, "Osp: connecting to %s@%d, please wait...\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort);


	/* ���ӷ����� */

	int ret;
	//���÷�������ʽ����
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


    // ��ȡ��ǰ�������õ�IP��ַ
    if (pdwLocalIP != NULL)
    {
        struct sockaddr_in tClientAddr;
        int length = sizeof(tClientAddr);
        if (getsockname(tSock, (struct sockaddr *)&tClientAddr, &length) == 0)
        {
            *pdwLocalIP = (u32) tClientAddr.sin_addr.s_addr;
        }
    }

    /* �����׽���ѡ��: ��������(�������κα��⾺�����㷨) */
    optVal = 1;
    set_result = setsockopt(tSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR)
	{
		win_error = WSAGetLastError();
//		OspPrintf(TRUE, TRUE, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail  %d\n",win_error);
	}

	/* �����׽���ѡ��: �����׽ӿڷ��ͻ���Ĵ�С */
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

	/* �����׽���ѡ��: ����TCP���Ӽ�⹦�� */
	optVal = 1;
    set_result = setsockopt(tSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optVal, sizeof(optVal));
    if(set_result == SOCKET_ERROR )
	{
		win_error = WSAGetLastError();
		OspLog(1, "\nOsp: OspConnectTcpNode() set sock option fail, errno %d\n", win_error);
	}

	/* Ϊ���ӳɹ����׽����ڽ����з���һ���Ľ��� */
    if( !g_Osp.m_cNodePool.Regist(dwIpv4Addr, tSock, &nodeId, wHb, byHbNum) )
	{
		SockClose(tSock);
		OspLog(1, "\nOsp: OspConnectTcpNode() regist socket failed, close it.\n");
		return INVALID_NODE;
	}

    OspLog(1, "Osp: connect to %s@%d OK, the nodeid = %d!\n", inet_ntoa(tSvrINAddr.sin_addr), wTcpPort, nodeId);

    OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, nodeId); // ����node��֪ͨ�Է�����֧��ѹ����Ϣ by wubin 2011-02-22
	SockChangeNotify();
    return nodeId;
}

/*====================================================================
��������OspSetHBParam
���ܣ����ý�����·������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNodeID: ����,
              wHb: wHb���Ӽ��һ��,
			  byHbNum: byHbNum����Ӧ���, �����Ͽ�.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSetHBParam(u32 dwNodeID, u16 wHb, u8 byHbNum)
{
	return g_Osp.m_cNodePool.SetHBParam(dwNodeID, wHb, byHbNum);
}

/*====================================================================
��������OspDisconnectTcpNode
���ܣ��Ͽ���һ��node�ϵ����ӡ�����app����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����dwNodeId: ���š�

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������SockChangeNotify
���ܣ� ֪ͨ�ػ�����, ���µ�Sock����,��������Sock,�����µ�����,�µ����ӽ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������OspIsValidTcpNode
���ܣ��ж�һ��TCP�ڵ��Ƿ���Ч.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uNodeId: ����.

  ����ֵ˵����TRUE ��Ч��FALSE ��Ч
====================================================================*/
API BOOL32 OspIsValidTcpNode(u32 dwNodeId)
{
	if(dwNodeId == 0 || dwNodeId > MAX_NODE_NUM)
		return FALSE;

    return g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bValid;
}

/*====================================================================
��������OspIsNodeCheckEnable
���ܣ��ж�ָ��������·��⹦���Ƿ����á�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uNodeId: ���š�

  ����ֵ˵�������÷���TRUE, ���÷���FALSE.
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
��������OspEnableNodeCheck
���ܣ�����ָ��������·��⹦�ܡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uNodeId: ���š�

  ����ֵ˵������
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
��������OspDisableNodeCheck
���ܣ�����ָ��������·��⹦�ܡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����dwNodeId: ���š�

  ����ֵ˵������
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
��������OspPost
���ܣ�ʹ��ʵ��ID�����첽��Ϣ��C����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����dwDstId: Ŀ��ʵ����ʵ��ID(app, ins),
              wEvent: ��Ϣ��,
              pvContent: ��Ϣ��ָ��,
			  wLength: ��Ϣ����,
			  dwDstNode: Ŀ��ʵ���Ľ���,
              dwSrcIId: Դʵ��ID(app, ins),
              dwSrcNode: Դ����.

  ����ֵ˵�����ɹ�����OSP_OK, ʧ�ܷ���OSP_ERROR.
====================================================================*/
API int OspPost(u32 dwDstId, u16 wEvent, const void* pvContent, u16 wLength, u32 dwDstNode, u32 dwSrcIId, u32 dwSrcNode, int nTimeout)
{
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

	/* ��Ϣ���� */
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

	/* ����OspPostMsg()����ʵ�ʵķ��� */
    return OspPostMsg(dwDstId, wEvent, pvContent, wLength, dwDstNode,
		              dwSrcIId, dwSrcNode, TRUE, MSG_TYPE_ASYNC, nTimeout);
}

/*====================================================================
��������OspPost
���ܣ�ʹ��ʵ����������һ���첽��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����dstalias: Ŀ��ʵ���ı���,
              aliaslen: ��������,
              dstapp: Ŀ��ʵ������App��,
              event: ��Ϣ��,
              content: ��Ϣ��ָ��,
			  length: ��Ϣ����,
			  dstnode: Ŀ��ʵ���Ľ���,
              srciid: Դʵ��ID(app, ins),
              srcnode: Դ����.

  ����ֵ˵�����ɹ�����OSP_OK, ʧ�ܷ���OSP_ERROR.
====================================================================*/
int OspPost(const char* dstalias, u8 aliaslen, u16 dstapp, u16 event,
		    const void* content, u16 length, u32 dstnode, u32 srciid, u32 srcnode, int nTimeout)
{
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

	/* ��Ϣ���� */
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

	/* ����OspPostMsg()����ʵ�ʵķ��� */
    return OspPostMsg(dstalias, aliaslen, dstapp, dstnode, event, content, length, srciid, srcnode, TRUE);
}

/*====================================================================
��������SendQueIdFind
���ܣ�ȡ��ָ��app�����䷢�˾��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uAppId: app��

  ����ֵ˵�����ɹ�������Ч������ID, ʧ�ܷ���0.
====================================================================*/
API MAILBOXID SendQueIdFind(u16 wAppId)
{
    return g_Osp.m_cAppPool.SendQueIdFind(wAppId);
}

/*====================================================================
��������RcvQueIdFind
���ܣ�ȡ��ָ��app�������ն˾��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����wAppId: app��

  ����ֵ˵�����ɹ�������Ч������ID, ʧ�ܷ���0.
====================================================================*/
API MAILBOXID RcvQueIdFind(u16 wAppId)
{
    return g_Osp.m_cAppPool.RcvQueIdFind(wAppId);
}

/*====================================================================
��������NodeMsgRcv
���ܣ���Node��Ӧ��socket�Ͻ�����Ϣ, ��ת�����ڲ�App
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����dwNodeId: ����,
              tNodeSock: Ҫ������Ϣ���׽���.

  ����ֵ˵������
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

	/* ��������Ӧ�Ľ�� */
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

	/* �����Ϣ���� */
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

	/* ��Ϣͷδ���գ�����δ������ϣ��������Ϣͷ */
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
		/* ����Ϣͷδ�������������˳� */
		if( dwRcvdLen < dwHeadLen )
		{
			pNode->SetRcvdLen(dwRcvdLen);
			return;
		}
		/* ��Ϣͷ����������������Ϣͷ */
		/* ����Ϣͷ��ת��Ϊ�����ֽ��� */
		dwRcvdLen = sizeof(CMessageForSocket);
		MsgNtoh(pcMsg4S);
		/* ת����Ϣ��Դ, Ŀ�Ľڵ�� */
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
		/* ��Ϣͷ�������������ٽ�����Ϣ�� */
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

	/* ������Ϣ���ݣ�
	 * ע�⣺���һ�ν��ղ��꣬������һ�ε��ô˺���ʱ��������
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
		/* ��Ϣ����δ�����������򷵻� */
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
		/* ��Ϣ������ϣ��������������ʱָ�� */
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
	/* ��Ϣ������ϣ��������������ʱָ�� */
	pNode->msgRcvedInc();
	pNode->SetRcvdData(NULL);
	pNode->SetRcvdLen(0);



	/* ��������Ϣ���� */
	if(g_Osp.m_cNodePool.m_dwRcvTrcFlag > 0)
    {
		len = sprintf(achBuf, "\nOsp: message received from node %d\n", dwNodeId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pcMsg);
		OspMsgTrace(TRUE, TRUE, achBuf, len);
    }

    /* �ػ�ȫ��ͬ��Ӧ�� */
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

    /* ������Ϣ��Ŀ��App�������� */
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

    if(pApp->GetMsgWaitingNum() >= pApp->maxMsgWaiting)  // �����������, ��������Ϣ
    {
		OspFreeMem(pcMsg);
        u32 dwip=OspNodeLastIpGet(dwNodeId);
        OspLog(1, "Osp: node%d (%u.%u.%u.%u) recv msg to app%d dropped\n", dwNodeId, OspQuadAddr(dwip), dstAppId);
		pApp->msgdropped++;
		return;
	}

	/* ����Ϣ�ŵ�Ŀ��App�������� */
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
��������NodeMsgDispatch
���ܣ����ڲ�socket�Ͻ�����Ϣ���ֽ����ڴ�ӡ���״̬�ı���Ϣ.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����tLocalSock: �ڲ��׽���

  ����ֵ˵�����ɹ�����OSP_OK, ʧ�ܷ���OSP_ERROR.
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

	/* ���ؽڵ�? */
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
��������SvrFdSet
���ܣ�����������socket���ã�����select.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����ptSet: ָ���׽ӿڼ���ָ��

  ����ֵ˵����
====================================================================*/
API void SvrFdSet(fd_set *ptSet)
{
    FD_ZERO(ptSet);

    if(g_Osp.m_cNodePool.m_tListenSock != INVALID_SOCKET)
	{
		FD_SET(g_Osp.m_cNodePool.m_tListenSock ,ptSet); //�������������Sock
	}

    if(g_Osp.m_cNodePool.m_tLocalOutSock != INVALID_SOCKET)
	{
		FD_SET(g_Osp.m_cNodePool.m_tLocalOutSock ,ptSet); //�������ڲ�����Sock
	}

    for(u32 i=0; i<MAX_NODE_NUM; i++) //��������Sock
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
��������OspNodeShow
���ܣ���ʾ����Node��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspNodeShow()
{
    g_Osp.m_cNodePool.Show();
}

/*====================================================================
��������PostDaemon
���ܣ��ػ��߳�, �ȴ��ͻ��˵Ľ���, �ȴ������ڵ����Ϣ���벢ת��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����

  ����ֵ˵����
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

        //����Ƿ����µ����ӽ���
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

        //����Ƿ��������ڵ����Ϣ���뵽���ڵ�,���ַ�������Ӧ��
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

        //����Ƿ����ڲ���Ϣ�跢�͵������ڵ� will be deleted
        if( FD_ISSET(g_Osp.m_cNodePool.m_tLocalOutSock, &tWaitFd)  )
        {
            NodeMsgDispatch(g_Osp.m_cNodePool.m_tLocalOutSock);
        }
    }
}

/*====================================================================
��������OspNodeConnTest
���ܣ�������Ӳ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����NodeId: Ŀ�����ʶ(������MAX_NODE_NUM��
                      ���ʾ�����еĽ�㶼���Ͳ�����Ϣ)

  ����ֵ˵����
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
				OspPost(MAKEIID(NODE_MAN_APPID,1), OSP_NETSTATEST, NULL, 0, i);//�������������Ϣ
				OspTaskDelay(50);
			}
		}
		return;
	}

	if(g_Osp.m_cNodePool.m_acNodeRegTable[NodeId-1].m_bValid)
	{
		OspPost(MAKEIID(NODE_MAN_APPID,1), OSP_NETSTATEST, 0, 0, NodeId);//�������������Ϣ
	}
	else
	{
		OspPrintf(TRUE, FALSE, "Osp: this node is not an actived node!\n");
	}
}

/*====================================================================
��������OspStatusMsgOutSet
���ܣ�����״̬��Ϣ�Ƿ񱻲������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����OutMsgEnable: �Ƿ�������Ϣ���

  ����ֵ˵����
====================================================================*/
API void OspStatusMsgOutSet(BOOL32 OutMsgEnable) //set status message can or not be out put every timer
{
	g_Osp.m_bStatusPrtEnable = OutMsgEnable;
}

/*====================================================================
��������MsgHton
���ܣ������ֽ�ת��: �����ֽ���-->�����ֽ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptMsg: ��Ϣͷָ��

  ����ֵ˵����
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
��������MsgNtoh
���ܣ������ֽ�ת��: �����ֽ���-->�����ֽ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptMsg: ��Ϣͷָ��

  ����ֵ˵����
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
��������MsgDump2Buf
���ܣ���ӡ����Ϣ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����buf:       ������ָ��
			  buflen:    ��������С
			  CMessage:  ����ӡ��Ϣ

  ����ֵ˵������ӡ���ݵĳ���
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

	/* ����ǰ�ʵ���������͵�, ��ʾʵ������, ������ʾʵ���� */
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

	/* �����Ϣ�� */
    pPtr = (char*)ptMsg->content;
    for(i=0; i<ptMsg->length; i++, pPtr++)
	{
		/* �ڽ�ֹ����Ϣ���������£�ֻ���10������ */
		if(!IsOspLogLongMsgPrintEnbl())
		{
			if( i >= (DEF_MSGTRC_LINES * MSGTRCBYTES_PER_LINE) )
			{
				break;
			}
		}
		else
		{
			/* �������������Ϣ�������, Ҳ���������MAX_MSGTRC_LINES������ */
			if( i >= (MAX_MSGTRC_LINES * MSGTRCBYTES_PER_LINE) )
			{
				break;
			}
		}

		/* ÿ16��u8Ϊһ�� */
		if( (i&0x0F) == 0 )
		{
			actLen += sprintf(buf+actLen, "%4xh: ", i);
		}

		actLen += sprintf(buf+actLen, "%.2X ", (u8)*pPtr);
		if( ((i+1)&0x0F) == 0 )
		{
			actLen += sprintf(buf+actLen, ";   "); // ÿ���16��u8����ӡ���е��ַ���ʾ
			for(iChar=15; iChar>=0; iChar--)
			{
				if( ((*(pPtr-iChar))>=0x21) && ((*(pPtr-iChar)) <= 0x7E) ) // �ɴ�ӡ�ַ�
				{
					actLen += sprintf(buf+actLen, "%1c", *(pPtr-iChar));
				}
				else  // ���ɴ�ӡ�ַ�
				{
					actLen += sprintf(buf+actLen, ".");
				}
			}
			actLen += sprintf(buf+actLen, "\n");
		}

		if( (i == (ptMsg->length-1)) && (((i+1)&0x0F) != 0) ) // ���һ��
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
��������OspMsgDumpSet
���ܣ����ó���ʮ�е���Ϣ�����Ƿ������ȱʡ�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bLongMsgDumpEnbl: ����/��ֹ����Ϣ���

  ����ֵ˵����
====================================================================*/
}

/*====================================================================
��������MsgIdChange
���ܣ�������Ϣ��DstNodeID��SrcNodeID
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptMsg: ��Ϣָ��
			  uNode: ָ����Ϣ�µĽ���

  ����ֵ˵����
====================================================================*/
API void MsgIdChange(CMessageForSocket *ptMsg,u32  uNode) // ������Ϣ,ת��DstNodeID��SrcNodeID;
{
    ptMsg->srcnode = uNode;
    ptMsg->dstnode = 0;
}

/*====================================================================
��������OspNodeDiscCBReg
���ܣ�������node�����ж�ʱ��֪ͨ��appid��InstId
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNodeId: Ŀ����
			  wAppId:   Ŀ��APP��
			  wInsId:   Ŀ��Instance

  ����ֵ˵�����ɹ�����OSP_OK��ʧ�ܲμ�������
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

	/* ע��Ψһ��֪ͨʵ�� */
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum      = 1;
    g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformAppId[0] = wAppId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformInsId[0] = wInsId;

	OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
	OspTaskUnsafe();

	return OSP_OK;
}

/*====================================================================
��������OspNodeDiscCBRegQ
���ܣ�������node�����ж�ʱ��֪ͨ��appid��InstId
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNodeId: Ŀ����
			  wAppId:   Ŀ��APP��
			  wInsId:   Ŀ��Instance

  ����ֵ˵�����ɹ����ص�ǰ��ע�������ʧ�ܲμ�������
====================================================================*/
API int OspNodeDiscCBRegQ(u32 dwNodeId, u16 wAppId, u16 wInsId)
{
	if(dwNodeId<=0 || dwNodeId>MAX_NODE_NUM)
		return OSPERR_ILLEGAL_PARAM;

	if(wAppId<=0 || wAppId>MAX_APP_NUM)
		return OSPERR_ILLEGAL_PARAM;

	OspTaskSafe();
	OspSemTake( g_Osp.m_cNodePool.m_tSemaNodePool );
	/* �����ע������ */
	int curr_idx = g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum;
	if ((curr_idx + 1) >= NODE_MAX_CBNUM)
	{
		OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
		OspTaskUnsafe();

		OspPrintf(TRUE, FALSE, "Osp: Callback instance number has reached maximum.\n");
		return OSP_ERROR;
	}
	/* ��鵱ǰʵ���Ƿ��Ѿ�ע��� */
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

	/* ע��Ψһ��֪ͨʵ�� */

    g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformAppId[curr_idx] = wAppId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_wDiscInformInsId[curr_idx] = wInsId;
	g_Osp.m_cNodePool.m_acNodeRegTable[dwNodeId-1].m_bDiscInformNum++;

	OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
	OspTaskUnsafe();

	return OSP_OK;
}

/*=============================================================================
  �� �� ����OspNodeLastIpGet
  ��    �ܣ��ڵ���Чʱ���ָ��������һ�ζ�Ӧ��Ip��ַ����Чʱ��ýڵ��Ӧ��IP��ַ(�����ڴ�ӡ��Ϣ)
  ע    �⣺
  �㷨ʵ�֣�
  ȫ�ֱ�����
  ��    ����dwNodeId : [in] node id

  �� �� ֵ���ɹ����ؽ��IP, ʧ�ܷ���0.
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
�� �� ����OspNodeIpGet
��    �ܣ����ָ������Ip��ַ
ע    �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��    ����dwNodeId : [in] node id

  �� �� ֵ���ɹ����ؽ��IP, ʧ�ܷ���0.
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
  �� �� ����OspNodeLocalIpGet
  ��    �ܣ���ȡ�ڵ�ı��ص�ַ
  ע    �⣺�����Ѿ����ӵ�tcp�ڵ㣬�ڱ��ش��ڶ����ַ������£���Ҫ֪���Է�ʵ�����ӵı���ip��ַ��
  �㷨ʵ�֣�
  ȫ�ֱ�����
  ��    ����dwNodeId : [in] node id

  �� �� ֵ���ɹ����ؽ��IP, ʧ�ܷ���0.
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
��������NodeDiscCallBack(
���ܣ�node�����ж�ʱ����OSP_DISCONNECT��Ϣ��NodeDiscCBReg�����õ�appid��InstId
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNodeId: Ŀ����
			  wAppId:   Ŀ��APP��
			  wInsId:   Ŀ��Instance

  ����ֵ˵����
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
��������DispatchTaskEntry
���ܣ�Dispatch Task�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pcDispTask: ��������Ϣ��CDispatchTaskָ��

  ����ֵ˵����
====================================================================*/
API void DispatchTaskEntry(CDispatchTask *pcDispTask)
{
    pcDispTask->NodeMsgSendToSock();
}

/*====================================================================
��������postMsg
���ܣ������첽��Ϣ�����ڵ�������ڵ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwDstId:		Ŀ��ʵ����ʵ��ID(app, ins),
              wEvent:		��Ϣ��,
              pvContent:	��Ϣ��ָ��,
			  wLength:		��Ϣ����,
			  dwDstNode:	Ŀ��ʵ���Ľ���,
              dwSrcIId:		Դʵ��ID(app, ins),
              dwSrcNode:	Դ����
			  bDroppable:   ����Ϣ�Ƿ�ɶ���
			  byType:		��Ϣ����
			  nTimeout:		��ʱʱ��

  ����ֵ˵�����ɹ�����OSP_OK, ʧ�ܷ���OSP_ERROR
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

	/* ȷ��������һ���� */
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
		//��ʾAPP��Ӧ
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

    /* ������Ϣ����������ϵͳ���� */
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
        //��Ϣѹ��by wubin 2011-02-22
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

	/* ����ڲ���Ϣ������Ϣͷָ�뷢�͵�Ŀ��App�������� */
    u16 appId = GETAPP(dwDstId);
	MAILBOXID dstSendQueId = SendQueIdFind(GETAPP(dwDstId) ); // Ŀ��Ӧ�õ���Ϣ����

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

	/* ���Ŀ��App�����������ұ���Ϣ���Զ��� */
	if( bDroppable && (pApp->GetMsgWaitingNum() > pApp->maxMsgWaiting) )
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		pApp->msgdropped++;
		OspPrintf(TRUE, FALSE, "Osp: postmsg, app %d's message dropped\n", appId);
        pApp->MsgIncomeNumDec();
		return OSP_ERROR;
	}

	/* ����Ϣ��ַ���͵�Ŀ��App������ */
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
��������OspPostMsg
���ܣ���ʵ�����������첽��Ϣ�����ڵ�������ڵ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����
			  pchDstAlias:  Ŀ��ʵ���ı���
			  byDstAliasLen:��������
			  wDstAppId:	Ŀ��APP
			  dwDstNode:	Ŀ��NODE
              wEvent:		��Ϣ��,
              pvContent:	��Ϣ��ָ��,
			  wLength:		��Ϣ����,
			  dwDstNode:	Ŀ��ʵ���Ľ���,
              dwSrcIId:		Դʵ��ID(app, ins),
              dwSrcNode:	Դ����
			  bDroppable:   ����Ϣ�Ƿ�ɶ���
			  Type:			��Ϣ����

  ����ֵ˵�����ɹ�OSP_OK, ʧ��OSP_ERROR
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

	/* ��֤������һ���� */
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

    /* ������Ϣ����������ϵͳ���� */
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
        //��Ϣѹ��by wubin 2011-02-22
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

	/* ����ڲ���Ϣ������Ϣͷָ�뷢�͵�Ŀ��App�������� */
 	MAILBOXID dstSendQueId = SendQueIdFind(wDstAppID); // Ŀ��Ӧ�õ���Ϣ����
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
	/* ���Ŀ��App�����������ұ���Ϣ���Զ��� */
	if( bDroppable && ( pApp->GetMsgWaitingNum() > pApp->maxMsgWaiting) )
	{
		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
		pApp->msgdropped++;
		OspPrintf(TRUE, FALSE, "Osp: postmsg, App%d's message dropped\n", wDstAppID);
        pApp->MsgIncomeNumDec();
		return OSP_ERROR;
	}

	/* ����Ϣ��ַ���͵�Ŀ��App������ */
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
��������OspSend
���ܣ�ȫ��ͬ����Ϣ���ͺ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwDstId:		Ŀ��ʵ����ʵ��ID(app, ins),
              wEvent:		��Ϣ��,
              pvContent:	��Ϣ��ָ��,
			  wLength:		��Ϣ����,
			  dwDstNode:	Ŀ��ʵ���Ľ���,
              dwSrcIId:		Դʵ��ID(app, ins),
              dwSrcNode:	Դ����
			  ackbuf:		��Ӧ��Ϣ������
			  ackbuflen:	��������С
			  realacklen:	ʵ�ʻ�Ӧ��Ϣ�ĳ���
			  timeout:		��ʱʱ��

  ����ֵ˵�����ɹ�OSP_OK�� ʧ��OSP_ERROR
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

	/* ͬʱֻ����һ���̷߳���ȫ��ͬ����Ϣ */
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
		/* ���Ӧ��buffer����, ����Ӧ�𳬳����� */
		if(	ackbuf==NULL || ackbuflen<g_Osp.m_wSyncAckLen )
		{
			OspSemGive(g_Osp.m_tMutexSema);
			OspTaskUnsafe();
			return OSPERR_SYNCACK_EXCEED;
		}

		/* ��Ӧ�𿽱���Ӧ��buffer�� */
		memcpy(ackbuf, g_Osp.m_achSyncAck, g_Osp.m_wSyncAckLen);
		memset(g_Osp.m_achSyncAck, 0, sizeof(g_Osp.m_achSyncAck));
		g_Osp.m_wSyncAckLen = 0;
	}

	OspSemGive(g_Osp.m_tMutexSema);
	OspTaskUnsafe();
	return OSP_OK;
}

/*====================================================================
��������OspSend
���ܣ�ȫ��ͬ����Ϣ���ͺ�����ʹ�ñ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����DstAlias:     Ŀ��ʵ���ı���
			  AliasLen:     ��������
			  dstapp:	    Ŀ��APP
              event:		��Ϣ��,
              content:		��Ϣ��ָ��,
			  length:		��Ϣ����,
			  dstnode:		Ŀ��ʵ���Ľ���,
              srcIId:		Դʵ��ID(app, ins),
              SrcNode:		Դ����
			  ackbuf:		��Ӧ��Ϣ������
			  ackbuflen:	��������С
			  realacklen:	ʵ�ʻ�Ӧ��Ϣ�ĳ���
			  timeout:		��ʱʱ��

  ����ֵ˵�����ɹ�OSP_OK�� ʧ��OSP_ERROR

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

	/* ͬʱֻ����һ���̷߳���ȫ��ͬ����Ϣ */
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
��������CAppPool
���ܣ����캯��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������CNodePool::Initialize
���ܣ����س�ʼ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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

	/* ���������App */
	int ret = m_cNodeManApp.CreateApp("OspNodeMan", NODE_MAN_APPID, NODE_MAN_APPPRI, (u16)MAX_NODEMANMSG_WAITING);
	if(ret != OSP_OK)
	{
		OspLog(1, "Osp: create app OspNodeMan fail\n");
		return FALSE;
	}

	//���������
	ret = OspPost(MAKEIID(NODE_MAN_APPID, 1), START_UP_EVENT);
	if(ret != OSP_OK)
	{
		OspLog(1, "Osp: invoke OspNodeMan fail\n");
		return FALSE;
	}

	return TRUE;
}

/*====================================================================
��������CNodePool::NodeGet
���ܣ����ݽ��ţ��ӽ����еõ���ָ��ý���ָ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����nodeId: �ڵ�ID

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
�� �� ����OspGetNodeAddr
��	  �ܣ�����Osp����ַ��������Զ��IP+Port����
ע	  �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��	  ����u32 dwNodeId : [in] ���ID
TOspNodeAddr* ptOspNodeAddr : [out] ����ַ
�� �� ֵ��FALSE - ��ѯ�ɹ�
TRUE - ��ѯʧ��
-------------------------------------------------------------------------------
�޸ļ�¼��
��		��	�汾  �޸���  �޸�����
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
��������CNodePool::Show()
���ܣ���ʾ����Node��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
        //����Ƿ�֧��ѹ���Ĵ�ӡ
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
��������Regist
���ܣ���IP��ַ��socket����Ӧ�����ݸ���һ����node��������nodeId
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwIpAddr:		Ҫע���IP��ַ
			  dwSock:		��Ӧ���׽���
			  puNodeId:		[out]�ڵ��ʶ
			  wHb:			HeartBeat���Ƶ��
			  byHbNum:		��������ʱ��������δ��ӦHeartbeat����

  ����ֵ˵����
====================================================================*/
BOOL32 CNodePool::Regist(u32 dwIpAddr, SOCKHANDLE dwSock, u32 *puNodeId,u16 wHb,u8 byHbNum) // ����·��ID,��pNOdeId
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
��������NodeDisRcv
���ܣ�Node still registed , but post deamon stop listening from that node
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNode: Ŀ��ڵ�ID

  ����ֵ˵����
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
��������CNodePool::SetHBParam
���ܣ����ý�����·������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uNodeID: ����,
			  uHb: uHb���Ӽ��һ��,
			  uHbNum: uHbNum����Ӧ���, �����Ͽ�.

  ����ֵ˵������.
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
��������NodeSockClose
���ܣ��ر�һ��ָ��Node��SOCKET
		����ֻ�ر�SOCKET���������Ͷ�����Ϣ��ֻ���ڷ��ͳ���ʱ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNode:		Ŀ��ڵ�ID

����ֵ˵����
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
��������CNodePool::NodeUnRegistNoSema
���ܣ��ر�һ��ָ����Node
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNode:		Ŀ��ڵ�ID
			  byReason:		ָ���ر�ԭ��

  ����ֵ˵����
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

	/* ֪ͨPostDaemon����״̬�ı��Ա�PostDaemon�ı��׽ӿڶ��� */
	SockChangeNotify();
	return TRUE;
}

/*====================================================================
��������GetSock
���ܣ�����һ��ָ��Node�ϵ�socket��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����dwNodeId: Ŀ��ڵ�ΪID
			  puSock:   [out]���ص��׽���

  ����ֵ˵����
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
��������CNodePool::IsNodeCheckEnable
���ܣ������Ƿ�������������⹦��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����node: Ŀ��NODE

  ����ֵ˵����
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
��������CNodePool::NodeCheckEnable
���ܣ�������������⹦��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������CNodePool::NodeCheckEnable
���ܣ���ֹ��������⹦��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����node: Ŀ��NODE

  ����ֵ˵����
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
��������CNodePool::NodePoolScan
���ܣ�ά��������ÿ����������״�����ڽ��ɨ��ʵ���е���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
void CNodePool::Scan()
{
	u32 iNode;
	CNodeItem *pcNode = NULL;

	OspTaskSafe();
	OspSemTake(m_tSemaNodePool);

	/* ������еĽڵ��Ƿ������ӣ����ͼ����Ϣ */
	for(iNode=1; iNode<=MAX_NODE_NUM; iNode++)
	{
		pcNode = &m_acNodeRegTable[iNode-1];

		/* ����δ���ӵĽ��Ͳ�Ҫ����Ľ�� */
		if( !pcNode->m_bValid || ( pcNode->m_wDisTime == UNDETECT_TCP_HEARTBEAT ) )
		{
			continue;
		}

        // ���ͳ�����ֱ�ӹر�SOCKET������SOCKET��ΪINVALID��
        // ��������֪ͨAPP�Ĺ�����������
        // ��NodeMsgSendToSock�в���ֱ��NodeUnRegist�����������Ϣ����
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

		/* �������ã�ÿwDisTime�����һ�μ�� */
		if(++pcNode->m_wDisTimeUsed >= pcNode->m_wDisTime)
		{
			pcNode->m_wDisTimeUsed = 0;

			if(pcNode->m_bMsgEchoAck) // ����յ����Ӽ���Ӧ����λ���״̬, �������
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
			else // ��Ӧδ����
			{
				if(++pcNode->m_byDisconnHBsused >= pcNode->m_byDisconnHBs)  // ��������������ù�, ����
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
				else  // ��������δ�ù�
				{
					OspSemGive(m_tSemaNodePool);
					OspTaskUnsafe();

					OspPost(MAKEIID(NODE_MAN_APPID, 1), OSP_NETBRAECHO, 0, 0, iNode, MAKEIID(NODE_MAN_APPID,1));
                                               // INVALID_NODE, NODEMAN_MSG_TIMEOUT); // �ٴη���

					OspSemTake(m_tSemaNodePool);
					OspTaskSafe();
				}
			} // ��Ӧδ����
		} // ������˸ü���ʱ���
	} // end for

	OspSemGive(m_tSemaNodePool);
	OspTaskUnsafe();
}

/*====================================================================
��������OspDispatchShow
���ܣ���ʾDispatch task����Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspDispatchShow()
{
    g_Osp.m_cDispatchPool.Show();
}

/*====================================================================
�������� OspAppShow
���ܣ���ʾApp��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspAppShow()
{
    OspPrintf(TRUE,FALSE,"dwGloFailDispNode: %d ,dwGloFailPostNode: %d,dwGloFailPostApp: %d\n",g_Osp.m_cNodePool.m_dwGloFailDispNode,g_Osp.m_cNodePool.m_dwGloFailPostNode,g_Osp.m_cNodePool.m_dwGloFailPostApp);
    g_Osp.m_cAppPool.Show();
}

/*====================================================================
�������� OspInstShow
���ܣ���ʾĳ��App�е�Instance��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����aid: Ҫ��ʾ��APP��ID

  ����ֵ˵����
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
�������� OspInstShowAll
���ܣ���ʾ����Instance��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspInstShowAll()
{
	g_Osp.m_cAppPool.InstanceShowAll();
}

/*====================================================================
�������� OspInstDump
���ܣ���ʾָ��Instance����Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����aid:		Ŀ��APP��
			  InstId:	Ŀ��Instance
			  param:	�������

  ����ֵ˵����
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
��������DispatchSysInit
���ܣ�������������������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API BOOL32 DispatchSysInit(void)
{
	return g_Osp.m_cDispatchPool.Initialize();
}

/*====================================================================
��������DispatchSysQuit
���ܣ��˳�����ϵͳ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void DispatchSysExit(void)
{
	g_Osp.m_cDispatchPool.Quit();
}

/*====================================================================
��������CDispatchTask::CDispatchTask
���ܣ�CDispatchTask��Ĺ��캯��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
CDispatchTask::CDispatchTask()
{

}

/*====================================================================
��������CDispatchTask::Initialize
���ܣ����������ʼ������ʼ����Ա����, �����������������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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

    m_hTask = OspTaskCreate(DispatchTaskEntry,        // Dispatch�������
		                    "OspDispatchTask",        // Dispatch������
							OSP_DISPATCH_TASKPRI,     // �������ȼ�
							OSP_DISPATCH_STACKSIZE,   // �����ջ��С
							(KD_PTR)this,                // ����
							0,                        // ���񴴽���־
							&m_dwTaskID                // ���ص�����ID
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
��������CDispatchTask::Destroy
���ܣ��ͷ�����, ɱ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
void CDispatchTask::Quit()
{
	/* �ͷ����� */
	OspCloseMailbox(m_dwReadQue, m_dwWriteQue);
	m_dwReadQue = 0;
	m_dwWriteQue = 0;

	/* ɾ���ź��� */
	OspSemDelete(m_tSemMutex);
	m_tSemMutex = NULL;

	/* �˳������� */
    m_hTask = 0;
	m_dwTaskID = 0;
}

/*====================================================================
��������CDispatchTask::MsgIncomeInc
���ܣ�Dispatch��������ĵȴ���Ϣ����1��������Ҫ�ڶ����д��ֵ���������
      �ź���������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������CDispatchTask::MsgIncomeNum
���ܣ�ȡ��Dispatch��������ĵȴ���Ϣ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������CDispatchTask::MsgWaitingNum
���ܣ�ȡ��Dispatch��������ĵȴ���Ϣ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������CDispatchTask::NodeMsgSendToSock
���ܣ��������н�����Ϣ��ͨ���׽ӿں���ת�����ⲿ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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

		/* ���ռ��� */
		m_dwMsgProcessed++;

		/* �������Ϊ��, ���ѵȴ�����Dispatcher������ */
        if(MsgWaitingNum() == 0)
		{
			OspSemGive(pDispatchPool->m_tSemTaskFull);
		}

		pMsg = (CMessage *)osMsg.address;

        /* �յ��˳����� */
		if(pMsg->event == OSP_QUIT)
		{
			/* �ͷŷ�����Ϊ����Ϣ������ڴ� */
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10

			/* �ͷ������, ɾ���ź��� */
			Quit();

			/* ����˳������� */
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

		/* ������Ϣ����, �����������ֽ���ת��ǰ�� */
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
		/* ��Ϣͷ��ת��Ϊ�����ֽ��� */

		u32 before, after, temp;

		before = OspTickGet();

		/* ���׽ӿ��Ϸ��ͱ���Ϣ */
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
��������CDispatchPool::Initialize
���ܣ��������и���������������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������CDispatchPool::Quit
���ܣ��˳����з�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������.
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
			// ��OSP�˳�ʱ��LOG�����Ѿ�������ʹ�ã����Ҫֱ�Ӵ�ӡ
			printf("Osp: send message to mailbox failed in OspQuit().\n");
		}
	}

	OspSemDelete(m_tSemTaskFull);
    m_tSemTaskFull = NULL;
}

/*====================================================================
��������CDispatchPool::Show
���ܣ��˳����з�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������CDispatchPool::NodeMsgPost
���ܣ������͸��ⲿ������Ϣת����Dispatch����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����dstNode: Ŀ����,
              content: ָ�������Ϣ��ָ��,
              length: ��Ϣ����.

  ����ֵ˵����
====================================================================*/
s32 CDispatchPool::NodeMsgPost(u32 dstNode, const char * content, u32)
{
	BOOL32 bSuccess;
	TOsMsgStruc osMsg;
    CNodeItem *pNode = NULL;

	osMsg.address = (void*)content;

	/* ��㷢����Ϣ���� */
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
//<--��Ϣѹ��by wubin 2011-02-22
/*====================================================================
  ��������OspCompressMessagePack
  ���ܣ����Ѿ�������֡��CMessage����Ϣ�����ѹ��
  �㷨ʵ�֣�
            ��Ϣ��(content)��ǰ2���ֽڱ���ԭ��Ϣ�ţ�֮��2���ֽڱ���ԭ��Ϣ�峤�ȣ�֮��Ϊѹ������Ϣ�������
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵����
  ====================================================================*/

BOOL32 OspCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen)
{
    u32 dwAllocMsgSize = 0;             //Ԥ����ѹ�������ݻ��峤��
    u16 *pwUncompressEvent = NULL;      //���ԭ��Ϣ�Ż����ָ��
    u16 *pwUncompressContentLength = NULL; //���ԭ��Ϣ���Ȼ����ָ��
    u32 dwCompressContentLength = 0;    //δѹ�����峤��
    u8 *pbyCompressContentBuf = NULL;   //δѹ������ָ��
    CMessage *pCUncompressedMessage = NULL; //δѹ��CMessageָ��
    CMessage *pCCompressedMessage = NULL; //ѹ����CMessageָ��

    if(ppMsg && pdwMsgLen &&*ppMsg)
    {
        pCUncompressedMessage = *ppMsg;
        //��ȡ��Ϣ��ѹ������ܴ�С�����ֵ
        dwCompressContentLength = OspGetCompressLengthBound(*(pdwMsgLen) - sizeof(CMessage) - pCUncompressedMessage->dstAliasLen);
        //ʹ��OspAllocMem�����µ�CMessage
        dwAllocMsgSize = sizeof(CMessage) + sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength + pCUncompressedMessage->dstAliasLen;
        pCCompressedMessage = (CMessage *)OspAllocMem(dwAllocMsgSize);
        if (NULL == pCCompressedMessage)
        {
            return FALSE;
        }
        memset(pCCompressedMessage, 0, dwAllocMsgSize);

        //ָ��ѹ����ԭ��Ϣ�š�ԭ��Ϣ�峤���Լ�ѹ�������ݵĻ���ָ��
        pwUncompressEvent = (u16 *)((u8 *)pCCompressedMessage + sizeof(CMessage));
        pwUncompressContentLength = (u16 *)((u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16));
        pbyCompressContentBuf = (u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16) + sizeof(u16);
        //��������ѹ��
        if (TRUE == OspCompressData(pbyCompressContentBuf, &dwCompressContentLength, pCUncompressedMessage->content, (u32)pCUncompressedMessage->length))
        {
            //ȷ��ѹ�������ݳ����ܹ�����
            if (sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength < pCUncompressedMessage->length)
            {
                //������Ϣ����
                memcpy(pCCompressedMessage, pCUncompressedMessage, sizeof(CMessage));
                pCCompressedMessage->event = OSP_COMPRESS_MSG;
                pCCompressedMessage->length = sizeof(u16) + sizeof(u16) + (u16)dwCompressContentLength;
                pCCompressedMessage->content = pbyCompressContentBuf - sizeof(u16) - sizeof(u16);
                pCCompressedMessage->dstAlias = (s8 *)pbyCompressContentBuf + (u16)dwCompressContentLength;
                *pwUncompressEvent = pCUncompressedMessage->event;
                *pwUncompressContentLength = pCUncompressedMessage->length;
	            memcpy(pCCompressedMessage->dstAlias, pCUncompressedMessage->dstAlias, pCUncompressedMessage->dstAliasLen);
                //ʹ���µ�CMessage����ʹ��OspFreeMem����ԭCMessage
                *ppMsg = pCCompressedMessage;
                *pdwMsgLen = sizeof(CMessage) + pCCompressedMessage->length + pCCompressedMessage->dstAliasLen;
                OspFreeMem(pCUncompressedMessage);
                pCUncompressedMessage = NULL;
                return TRUE;
            }
        }
        //���ѹ��ʧ�ܻ���û��ѹ��Ч�����������½�CMessage��ʹ��ԭ����
        OspFreeMem(pCCompressedMessage);
        pCCompressedMessage = NULL;
    }
    return FALSE;
}

/*====================================================================
  ��������OspUnCompressMessagePack
  ���ܣ����Ѿ�������֡��CMessage����Ϣ����н�ѹ
  �㷨ʵ�֣�
            ��Ϣ��(content)��ǰ2���ֽڱ���ԭ��Ϣ�ţ�֮��2���ֽڱ���ԭ��Ϣ�峤�ȣ�֮��Ϊѹ������Ϣ�������
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵����
  ====================================================================*/

BOOL32 OspUnCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen)
{
    u32 dwAllocMsgSize = 0;             //Ԥ�����ѹ�����ݻ��峤��
    u16 *pwUncompressEvent = NULL;      //���ԭ��Ϣ�Ż����ָ��
    u16 *pwUncompressContentLength = NULL;//���ԭ��Ϣ���Ȼ����ָ��
    u8 *pbyUncompressContentBuf = NULL;//��ѹ�󻺳�ָ��
    u32 dwUncompressContentLength = 0;//��ѹ�󻺳峤��
    CMessage *pCUncompressedMessage = NULL; //��ѹ��CMessageָ��
    CMessage *pCCompressedMessage = NULL; //δ��ѹCMessageָ��


    if((NULL != ppMsg) && (NULL != pdwMsgLen) && (NULL != *ppMsg))
    {
        pCCompressedMessage = *ppMsg;
        if (OSP_COMPRESS_MSG != pCCompressedMessage->event)
        {
            return TRUE; //�������ѹ����Ϣ��ֱ�ӷ��سɹ�
        }
        //��ȡԭ��Ϣ�峤��
        pwUncompressContentLength = (u16*)((u8 *)pCCompressedMessage + sizeof(CMessage) + sizeof(u16));
        dwAllocMsgSize = sizeof(CMessage) + *pwUncompressContentLength + pCCompressedMessage->dstAliasLen;
        //ʹ��OspAllocMem�����µ�CMessage
        pCUncompressedMessage = (CMessage *)OspAllocMem(dwAllocMsgSize);
        if(NULL == pCUncompressedMessage)
        {
            return FALSE;
        }
        memset(pCUncompressedMessage, 0, dwAllocMsgSize);

        //�趨ԭ��Ϣ�š�ԭ��Ϣ�峤���Լ�ѹ�������ݵĻ���ָ��
        pwUncompressEvent = (u16*)((u8 *)pCCompressedMessage + sizeof(CMessage));
        pbyUncompressContentBuf = (u8 *)pCUncompressedMessage + sizeof(CMessage);
        dwUncompressContentLength = (u32)*pwUncompressContentLength;
        //��ѹ����
        if (TRUE == OspUnCompressData(pbyUncompressContentBuf, &dwUncompressContentLength, (u8 *)pCCompressedMessage->content + sizeof(u16) + sizeof(u16), (u16)(pCCompressedMessage->length - sizeof(u16) - sizeof(u16))))
        {
            //�����ѹ���ݳ����Ƿ���ȷ
            if(dwUncompressContentLength == (u32)*pwUncompressContentLength)
            {
                //������Ϣ����
                memcpy(pCUncompressedMessage, pCCompressedMessage, sizeof(CMessage));
                pCUncompressedMessage->event = *pwUncompressEvent;
                pCUncompressedMessage->length = (u16)dwUncompressContentLength;
                pCUncompressedMessage->content = pbyUncompressContentBuf;
                pCUncompressedMessage->dstAlias = (s8 *)pbyUncompressContentBuf + dwUncompressContentLength;
	            memcpy(pCUncompressedMessage->dstAlias, pCCompressedMessage->dstAlias, pCCompressedMessage->dstAliasLen);
                //ʹ���µ�CMessage����ʹ��OspFreeMem����ԭCMessage
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
  ��������OspGetCompressLengthBound
  ���ܣ���ȡѹ�������ݳ��ȵ����ֵ
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵����
  ====================================================================*/
u32 OspGetCompressLengthBound(u32 dwUncompressedLength, u8 byAlgorithm)
{
    return compressBound(dwUncompressedLength); //
}

/*====================================================================
  ��������OspCompressData
  ���ܣ�ʹ��ѹ���㷨ѹ������
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵����
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
  ��������OspUnCompressData
  ���ܣ�ʹ�ý�ѹ�㷨��ѹ����
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵����
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

