/******************************************************************************
模块名  ： OSP
文件名  ： ospTestAgent.cpp
相关文件：
文件实现功能：实现了CppUnit测试代码中需要的TestAgent　App
作者    ：张文江
版本    ：1.0.02.7.5
******************************************************************************/

#include <stdio.h>
#include "osp.h"
#include "../include/ospSch.h"
#include "../include/OspTestAgent.h"
#include <time.h>

static COspAgtApp g_cOspTestSvr;
static COspAgtApp g_cOspTestClt;
static u32 g_uSvrNode = INVALID_NODE;
//static BOOL32 g_bOspAgtStart = FALSE;
extern COsp g_Osp;

/*====================================================================
函数名：OspAgentStart
功能：启动Osp测试代理
算法实现：（可选项）
引用全局变量：
输入参数说明：port: 如果本结点非服务结点，在端口port上创建侦听套接字。

返回值说明：实际使用的侦听端口号.
====================================================================*/
API int OspAgentStart(u16 port)
{
	u16 wInsNo;
	u16 testPort = 0;
	char instName[MAX_ALIAS_LEN];
	int instLen;
	char *daemName_svr = "OspServerDaemon";
	char *daemName_clt = "OspClientDaemon";

	if( !IsOspInitd() ) OspInit(TRUE);

	if(g_Osp.m_cNodePool.m_tListenSock == INVALID_SOCKET)
	{
		if(OspCreateTcpNode(0, port) != OSP_ERROR)
		{
			testPort = port;
		}
	}
	else
	{
		testPort = g_Osp.m_cNodePool.m_wListenPort;
	}

	// 启动服务器
	g_cOspTestSvr.CreateApp("OspTestServer", OSP_TEST_SERVER_APP_ID, OSP_SERVER_APP_PRI, MAX_MSG_WAITING );
	g_cOspTestSvr.SetInstAlias(CInstance::DAEMON, daemName_svr, (u8)(strlen(daemName_svr)+1));
	for(wInsNo=1; wInsNo<=MAX_INS_NUM; wInsNo++)
	{
		instLen = sprintf(instName, "svrInst%d", wInsNo);
		g_cOspTestSvr.SetInstAlias(wInsNo, instName, (u8)(instLen+1));
	}

	OspSetLogLevel(OSP_TEST_SERVER_APP_ID, 0, 0);
	OspSetTrcFlag(OSP_TEST_SERVER_APP_ID, 0, 0);

	// 启动客户端
	g_cOspTestClt.CreateApp("OspTestClient", OSP_TEST_CLIENT_APP_ID, OSP_CLIENT_APP_PRI, MAX_MSG_WAITING );
	g_cOspTestClt.SetInstAlias(CInstance::DAEMON, daemName_clt, (u8)(strlen(daemName_clt)+1));
	for(wInsNo=1; wInsNo<=MAX_INS_NUM; wInsNo++)
	{
		instLen = sprintf(instName, "cltInst%d", wInsNo);
		g_cOspTestClt.SetInstAlias(wInsNo, instName, (u8)(instLen+1));
	}

	OspSetLogLevel(OSP_TEST_CLIENT_APP_ID, 0, 0);
	OspSetTrcFlag(OSP_TEST_CLIENT_APP_ID, 0, 0);

//	g_bOspAgtStart = TRUE;
	return testPort;
}

/*====================================================================
函数名：COspAgtIns::InstanceEntry
功能：普通实例入口
算法实现：（可选项）
引用全局变量：
输入参数说明：pMsg: 入口消息指针，

返回值说明：无.
====================================================================*/
void COspAgtIns::InstanceEntry(CMessage *const pMsg)
{
	int i;
	u32 qryType;
	u32 curState = CurState();
    u16 curEvent = pMsg->event;

	u32 reqType;
	u32 dwIpV4Addr;
	u16 wPort;
	TOspTestReqAck tOspTestReqAck;
	TOspCommTestCmdEx *ptOspCommTestCmdEx = NULL;
	TOspLogTestCmd *ptOspLogTestCmd = NULL;
	TOspLogTestResult tOspLogTestResult;
    TOspTimerTestCmd *ptOspTimerTestCmd = NULL;

	TOspCommStat tTempOspCommStat;
	TOspTimerStat tTempOspTimerStat;

	u16 *pWORD16RawData = NULL;

	if( curEvent == OSP_DISCONNECT)
	{
		NextState(IDLE_STATE);
		return;
	}

	if( curEvent == OSP_TEST_REQ )
	{
		// 返回应答
		tOspTestReqAck.iid = htonl(MAKEIID(GetAppID(), GetInsID()));
		tOspTestReqAck.aliasLen = GetAliasLen();
		GetAlias(tOspTestReqAck.achAlias, MAX_ALIAS_LENGTH, NULL);
		SetSyncAck(&tOspTestReqAck, sizeof(tOspTestReqAck));

        OspNodeDiscCBReg(pMsg->srcnode, GetAppID(), GetInsID());

		// 根据不同的请求类型转入不同的状态
		reqType = ntohl(*(u32 *)pMsg->content);
		switch(reqType)
		{
		case OSP_TEST_TYPE_LOG:
			// 清状态
			memset(&m_tOspLogStat, 0, sizeof(m_tOspLogStat));

			// 转入日志测试状态
            NextState(LOG_TEST_STATE);
			break;

		case OSP_TEST_TYPE_COMM:
			// 清状态
			memset(&m_tOspCommStat, 0, sizeof(m_tOspCommStat));

			// 转入通信测试状态
			NextState(COMM_TEST_STATE);
			break;

		case OSP_TEST_TYPE_TIMER:
			// 清状态
			memset(&m_tOspTimerStat, 0, sizeof(m_tOspTimerStat));

			// 转入定时器测试状态
			NextState(TIMER_TEST_STATE);
			break;

		default:
			break;
		}
		return;
	}

	if( curEvent == OSP_TEST_QRY )
	{
		qryType = ntohl(*(u32 *)pMsg->content);
        switch(qryType)
		{
		case OSP_TEST_TYPE_COMM:
			// 本地数据转换为网络字节顺序
			tTempOspCommStat.tOspRecvStat.recvPackets = htonl(m_tOspCommStat.tOspRecvStat.recvPackets);
            tTempOspCommStat.tOspRecvStat.recvLenError = htonl(m_tOspCommStat.tOspRecvStat.recvLenError);
			tTempOspCommStat.tOspRecvStat.recvContError = htonl(m_tOspCommStat.tOspRecvStat.recvContError);
			tTempOspCommStat.tOspRecvStat.recvBytes = htonl(m_tOspCommStat.tOspRecvStat.recvBytes);
			tTempOspCommStat.tOspRecvStat.totalMS = htonl(m_tOspCommStat.tOspRecvStat.totalMS);
			tTempOspCommStat.tOspSendStat.sendTimes = htonl(m_tOspCommStat.tOspSendStat.sendTimes);
			tTempOspCommStat.tOspSendStat.sendPackets = htonl(m_tOspCommStat.tOspSendStat.sendPackets);
			tTempOspCommStat.tOspSendStat.sendTimeouts = htonl(m_tOspCommStat.tOspSendStat.sendTimeouts);
			tTempOspCommStat.tOspSendStat.totalMS = htonl(m_tOspCommStat.tOspSendStat.totalMS);
			tTempOspCommStat.tOspSendStat.sendBytes = htonl(m_tOspCommStat.tOspSendStat.sendBytes);
			tTempOspCommStat.tOspSendStat.succSendPackets = htonl(m_tOspCommStat.tOspSendStat.succSendPackets);

			SetSyncAck(&tTempOspCommStat, sizeof(TOspCommStat));
			break;

		case OSP_TEST_TYPE_TIMER:
			tTempOspTimerStat.timeouts = htonl(m_tOspTimerStat.timeouts);
            tTempOspTimerStat.totalMs = htonl(m_tOspTimerStat.totalMs);

			SetSyncAck(&tTempOspTimerStat, sizeof(TOspTimerStat));
			break;

		case OSP_TEST_TYPE_LOG:
			m_dwCurFileLogs = ::OspFileLogNum();
			m_dwCurScrnLogs = ::OspScrnLogNum();
			m_tOspLogStat.fileLogs = (m_dwCurFileLogs-m_dwBeginFileLogs);
			m_tOspLogStat.scrnLogs = (m_dwCurScrnLogs-m_dwBeginScrnLogs);
			m_tOspLogStat.curLogFileNo = ::OspLogFileNo();
			m_tOspLogStat.fileLogSucs = m_tOspLogStat.fileLogs;
            m_tOspLogStat.scrnLogSucs = m_tOspLogStat.scrnLogSucs;

			tOspLogTestResult.bLogNumIncreased = htonl((m_tOspLogStat.fileLogs>0 || m_tOspLogStat.scrnLogs>0));
            tOspLogTestResult.bLogOutputInFile = htonl(TRUE);
			SetSyncAck(&tOspLogTestResult, sizeof(tOspLogTestResult));
			break;

		default:
			break;
		}
	}

	switch(curState)
	{
	case COMM_TEST_STATE:
		switch(curEvent)
		{
		case OSP_TEST_CMD:
			// 如本实例用作服务器, 等待客户端的消息
			if((pMsg == NULL) || (pMsg->content == NULL) || (pMsg->length <= sizeof(u32)))
			{
				break;
			}

			ptOspCommTestCmdEx = (TOspCommTestCmdEx *)(pMsg->content + sizeof(u32)); // 第一个u32为命令类型
			if(ptOspCommTestCmdEx == NULL) break;
			if( (sizeof(TOspCommTestCmdEx)+sizeof(u32)) != pMsg->length ) break;

			m_tOspCommTestPara.tOspCommTestCmdEx.peeriid = ntohl(ptOspCommTestCmdEx->peeriid);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.reqType = ntohl(ptOspCommTestCmdEx->tOspCommTestCmd.reqType);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.ip = ntohl(ptOspCommTestCmdEx->tOspCommTestCmd.ip);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.port = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.port);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.times = ntohl(ptOspCommTestCmdEx->tOspCommTestCmd.times);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.period = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.period);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.packets = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.packets);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.minLen);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.maxLen = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.maxLen);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkLenErr = ntohl(ptOspCommTestCmdEx->tOspCommTestCmd.bChkLenErr);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkConErr = ntohl(ptOspCommTestCmdEx->tOspCommTestCmd.bChkConErr);
			m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.rawDataLen = ntohs(ptOspCommTestCmdEx->tOspCommTestCmd.rawDataLen);

			m_wExpectLen = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen;
			if(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.reqType == OSP_REQ_TYPE_SERVER)
				break;

			// 如本实例用作客户端, 先与服务器建链
			if(g_uSvrNode != INVALID_NODE)
			{
				::OspDisconnectTcpNode(g_uSvrNode);
				g_uSvrNode = INVALID_NODE;
			}

			wPort = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.port;
			dwIpV4Addr = htonl(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.ip);
			if(wPort > 0)
			{
				g_uSvrNode = ::OspConnectTcpNode(dwIpV4Addr, wPort, 0, 0);
				if(g_uSvrNode == INVALID_NODE)
				{
					log(1, "OspAgent: connect to server failed.\n");
				}
			}

			// 生成原始测试数据
			pWORD16RawData = (u16 *)m_tOspCommTestPara.rawData;

			srand( (unsigned)time(NULL) );
			for(i=0; i<m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.rawDataLen/2; i++)
			{
				pWORD16RawData[i] = (u16)rand();
			}

			// 向服务器转发测试命令及原始测试数据
			if(OSP_OK != post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				              OSP_COMM_TEST_RAWDATA,
				              m_tOspCommTestPara.rawData,
				              m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.rawDataLen,
				              g_uSvrNode))
			{
				break;
			}

			// 发起测试
			m_dwSendStartTicks = OspTickGet();
			m_wCurLen = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen;
			OspTxTest();
			break;

        case OSP_COMM_TEST_RAWDATA:
			if(pMsg->length <= MAX_RAWDATA_LEN)
			{
				memcpy(m_tOspCommTestPara.rawData, pMsg->content, pMsg->length);
			}
			break;

		case OSP_COMM_TEST_TIMEOUT:
			OspTxTest();
			break;

		case OSP_COMM_TEST:
			post(pMsg->srcid, OSP_COMM_TEST_ACK, pMsg->content, pMsg->length, pMsg->srcnode);
			OspRxTest(pMsg);
			break;

		case OSP_COMM_TEST_ACK:
			OspRxTest(pMsg);
			break;

		case OSP_COMM_TEST_END:
			::OspDisconnectTcpNode(pMsg->srcnode);
			NextState(IDLE_STATE);
			break;

		default:
			break;
		}
		break;

	case LOG_TEST_STATE:
		switch(curEvent)
		{
		case OSP_TEST_CMD:
			// 取的一个干净的环境
			OspSetLogLevel(0,0,0);
			OspSetTrcFlag(0,0,0);
			OspSetLogLevel(GetAppID(), 0, 0);
			OspSetTrcFlag(GetAppID(), 0, 0);

			// 取的测试参数
			if(pMsg != NULL && pMsg->content != NULL)
			{
				ptOspLogTestCmd = (TOspLogTestCmd *)(pMsg->content + sizeof(u32)); // 第一个u32为命令类型
				if(ptOspLogTestCmd != NULL)
				{
					ptOspLogTestCmd->logFileSize = ntohl(ptOspLogTestCmd->logFileSize);
					ptOspLogTestCmd->logNum = ntohl(ptOspLogTestCmd->logNum);
					ptOspLogTestCmd->rawDataLen = ntohl(ptOspLogTestCmd->rawDataLen);
					memcpy(&m_tOspLogTestPara, ptOspLogTestCmd, sizeof(TOspLogTestCmd));
				}
			}

			OspLogTest();
			break;

		default:
			break;
		}
		break;

	case TIMER_TEST_STATE:
		switch(curEvent)
		{
		case OSP_TEST_CMD:
			// 取的测试参数
			memset(&m_tOspTimerStat, 0 , sizeof(TOspTimerStat));
			if(pMsg != NULL && pMsg->content != NULL)
			{
				ptOspTimerTestCmd = (TOspTimerTestCmd *)(pMsg->content + sizeof(u32)); // 第一个u32为命令类型
				if(ptOspTimerTestCmd != NULL)
				{
					// 字节序转换
					ptOspTimerTestCmd->times = ntohl(ptOspTimerTestCmd->times);
					ptOspTimerTestCmd->intval = ntohl(ptOspTimerTestCmd->intval);

					if(ptOspTimerTestCmd->type == TIMER_TEST_TYPE_ABS)
					{   //绝对定时
						ptOspTimerTestCmd->year = ntohs(ptOspTimerTestCmd->year);
						ptOspTimerTestCmd->month = ntohs(ptOspTimerTestCmd->month);
						ptOspTimerTestCmd->day = ntohs(ptOspTimerTestCmd->day);
						ptOspTimerTestCmd->hour = ntohs(ptOspTimerTestCmd->hour);
						ptOspTimerTestCmd->min = ntohs(ptOspTimerTestCmd->min);
						ptOspTimerTestCmd->sec = ntohs(ptOspTimerTestCmd->sec);
					}

					memcpy(&m_tOspTimerTestPara, ptOspTimerTestCmd, sizeof(TOspTimerTestCmd));
				}
			}
            OspSetTrcFlag(GetAppID(), 0, TRCTIMER);
			OspTimerTest();
			break;

		case OSP_TIMER_TEST_TIMEOUT:
			if(++m_tOspTimerStat.timeouts < m_tOspTimerTestPara.times)
            {
				SetTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.intval);
			}
			else
			{
				m_tOspTimerStat.totalMs = tickToMs(OspTickGet() - m_dwTimerTestStartTicks);
			}
			break;

		default:
			if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_MULTI)
			{
				if((curEvent >= OSP_TIMER_TEST_MULTI_TIMER) &&
			       (curEvent <= OSP_TIMER_TEST_MULTI_TIMER+m_tOspTimerTestPara.times))
				{
					if(++m_tOspTimerStat.timeouts >= m_tOspTimerTestPara.times)
					{
						m_tOspTimerStat.totalMs = tickToMs(OspTickGet() - m_dwTimerTestStartTicks);
					}
				}
			}
			break;
		}
		break;

	default:
		break;
	}
}

/*====================================================================
函数名：COspAgtIns::OspTxTest
功能：发送测试，发送消息并进行必要的统计
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：无.
====================================================================*/
void COspAgtIns::OspTxTest(void)
{
	int i;
	int ret;

	m_dwSendMs = tickToMs(OspTickGet());
	for(i=0; i<m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.packets; i++)
	{
		switch(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.funcType)
		{
		case NoAliasGPost:  // 使用实例ID的全局post函数
			ret = ::OspPost(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case AliasGPost:    // 使用别名的全局post函数
			ret = ::OspPost((const char * )m_tOspCommTestPara.tOspCommTestCmdEx.achAlias,
				m_tOspCommTestPara.tOspCommTestCmdEx.aliasLen,
				GETAPP(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid),
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case NoAliasGSend:  // 使用实例ID的全局send函数
			ret = ::OspSend(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case AliasGSend:    // 使用别名的全局send函数
			ret = ::OspSend((const char * )m_tOspCommTestPara.tOspCommTestCmdEx.achAlias,
				m_tOspCommTestPara.tOspCommTestCmdEx.aliasLen,
                GETAPP(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid),
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case NoAliasPost:   // 使用实例ID的实例post函数
			ret = post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case AliasPost:     // 使用别名的实例post函数
			ret = post(m_tOspCommTestPara.tOspCommTestCmdEx.achAlias,
				m_tOspCommTestPara.tOspCommTestCmdEx.aliasLen,
				GETAPP(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid),
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case NoAliasSend:   // 使用实例ID的实例send函数
			ret = send(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case AliasSend:      // 使用别名的实例send函数
			ret = send((const char * )m_tOspCommTestPara.tOspCommTestCmdEx.achAlias,
				m_tOspCommTestPara.tOspCommTestCmdEx.aliasLen,
				GETAPP(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid),
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		default:
			ret = OSP_ERROR;
			break;
		}

		m_tOspCommStat.tOspSendStat.sendPackets++;

		// 若发送成功, 使得当前发送长度加1, 否则重发本包
		if(ret == OSP_OK)
		{
			m_tOspCommStat.tOspSendStat.succSendPackets++;
			m_tOspCommStat.tOspSendStat.sendBytes += m_wCurLen;
			if(++m_wCurLen > m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.maxLen)
				m_wCurLen = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen;
		}
		else
		{
			if(ret == OSPERR_SEND_TIMEOUT)
               m_tOspCommStat.tOspSendStat.sendTimeouts++;
		}
	}
	m_dwSendMs = tickToMs(OspTickGet()) - m_dwSendMs;
	m_tOspCommStat.tOspSendStat.totalMS = tickToMs(OspTickGet()-m_dwSendStartTicks);

	// 判断是否发送完毕
	if(++m_tOspCommStat.tOspSendStat.sendTimes >= m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.times)
	{
		post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid, OSP_COMM_TEST_END, NULL, 0, g_uSvrNode);
//		NextState(IDLE_STATE);
		return;
	}

	// 若本次发送耗时不超过期望的时间间隔, 设置一个适当的超时值, 以便下次发送
	// 否则, 设置一个短的定时以快速触发下一次发送(否则, 向本实例发送一条超时消息以立即触发下一次发送)
	if(m_dwSendMs < m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.period)
	{
		SetTimer(OSP_COMM_TEST_TIMER, m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.period-m_dwSendMs);
	}
	else
	{
#if 0
	// 有问题：因为该消息在本APP邮箱满的情况下可能会丢失
		post(MAKEIID(GetAppID(), GetInsID()), OSP_COMM_TEST_TIMEOUT, NULL, 0);
#endif
		SetTimer(OSP_COMM_TEST_TIMER, 10);
	}
}

/*====================================================================
函数名：COspAgtIns::OspRxTest
功能：接收测试，接收消息并进行必要的统计
算法实现：（可选项）
引用全局变量：
输入参数说明：pMsg: 当前消息指针。

返回值说明：无.
====================================================================*/
void COspAgtIns::OspRxTest(const CMessage *pMsg)
{
	u32 dwChkLen;

	// 收到第一包时开始计时
	if(m_tOspCommStat.tOspRecvStat.recvPackets == 0)
	{
		m_dwStartTicks = OspTickGet();
	}

	// 进行收包统计
	m_tOspCommStat.tOspRecvStat.recvPackets++;
    m_tOspCommStat.tOspRecvStat.recvBytes += pMsg->length;

	// 长度错
	if(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkLenErr && pMsg->length != m_wExpectLen)
	{
		m_tOspCommStat.tOspRecvStat.recvLenError++;
	}

	// 内容错按较小长度检测
    dwChkLen = (pMsg->length > m_wExpectLen) ? m_wExpectLen : pMsg->length;
	if(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkConErr && memcmp(pMsg->content, m_tOspCommTestPara.rawData, dwChkLen) != 0 )
	{
		m_tOspCommStat.tOspRecvStat.recvContError++;
	}

	// 下一个期待包长总是本包长加1
	m_wExpectLen = (u16)(pMsg->length+1);
	if(m_wExpectLen > m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.maxLen)
	{
		m_wExpectLen = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen;
	}

	// 累计收包时间
	m_tOspCommStat.tOspRecvStat.totalMS = tickToMs(OspTickGet() - m_dwStartTicks);
}

/*====================================================================
函数名：COspAgtIns::OspLogTest
功能：日志测试
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：无.
====================================================================*/
void COspAgtIns::OspLogTest(void)
{
	if(m_tOspLogTestPara.funcType == LOG_FUNC_TYPE_GLOB)
	{
		if(m_tOspLogTestPara.logType == LOG_TYPE_FILE)
		{
			OspSetFileLogLevel(0, m_tOspLogTestPara.logCtrlLevl);
		}
		else
		{
			OspSetScrnLogLevel(0, m_tOspLogTestPara.logCtrlLevl);
		}
	}
	else if(m_tOspLogTestPara.funcType == LOG_FUNC_TYPE_INS)
	{
		if(m_tOspLogTestPara.logType == LOG_TYPE_FILE)
		{
			OspSetFileLogLevel(GetAppID(), m_tOspLogTestPara.logCtrlLevl);
		}
		else
		{
			OspSetScrnLogLevel(GetAppID(), m_tOspLogTestPara.logCtrlLevl);
		}
	}
	else
	{
		return;
	}

    if(m_tOspLogTestPara.logType == LOG_TYPE_FILE)
	{
		OspOpenLogFile(OSPTESTAGENT_LOGFILE_DIR, m_tOspLogTestPara.logFileNum, m_tOspLogTestPara.logFileSize);
	}

	m_dwBeginFileLogs = ::OspFileLogNum();
	m_dwBeginScrnLogs = ::OspScrnLogNum();
	for(u32 i=0; i<m_tOspLogTestPara.logNum; i++)
	{
		if(m_tOspLogTestPara.funcType == LOG_FUNC_TYPE_GLOB)
			::OspLog(m_tOspLogTestPara.logOutLevl, "Hello, world\n");
		else
			log(m_tOspLogTestPara.logOutLevl, "Hello, world\n");
	}
}

/*====================================================================
函数名：COspAgtIns::OspTimerTest
功能：定时器测试
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：无.
====================================================================*/
void COspAgtIns::OspTimerTest(void)
{
	m_dwTimerTestStartTicks = OspTickGet();
	// 相对定时测试
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_REL)
	{
		SetTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.intval);
	}

	// 定时器杀死测试
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_KILL)
	{
		for(u32 i=0; i<m_tOspTimerTestPara.times; i++)
		{
			SetTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.intval);
			OspDelay(m_tOspTimerTestPara.intval/2);
			KillTimer(OSP_TIMER_TEST_TIMER);
		}
	}

	//多个定时器测试
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_MULTI)
	{
		for(u32 i=m_tOspTimerTestPara.times; i>0; i--)
		{
			SetTimer(OSP_TIMER_TEST_MULTI_TIMER+i, m_tOspTimerTestPara.intval+i*5);
	        OspTaskDelay(1); // _WINDOWS_   1毫秒
		}
	}

	/*绝对定时器测试，单位为秒*/
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_ABS)
	{
		SetAbsTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.year, (u8)m_tOspTimerTestPara.month,
			        (u8)m_tOspTimerTestPara.day, (u8)m_tOspTimerTestPara.hour, (u8)m_tOspTimerTestPara.min,
					(u8)m_tOspTimerTestPara.sec);
	}
}