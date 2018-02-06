/******************************************************************************
ģ����  �� OSP
�ļ���  �� ospTestAgent.cpp
����ļ���
�ļ�ʵ�ֹ��ܣ�ʵ����CppUnit���Դ�������Ҫ��TestAgent��App
����    �����Ľ�
�汾    ��1.0.02.7.5
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
��������OspAgentStart
���ܣ�����Osp���Դ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����port: ��������Ƿ����㣬�ڶ˿�port�ϴ��������׽��֡�

����ֵ˵����ʵ��ʹ�õ������˿ں�.
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

	// ����������
	g_cOspTestSvr.CreateApp("OspTestServer", OSP_TEST_SERVER_APP_ID, OSP_SERVER_APP_PRI, MAX_MSG_WAITING );
	g_cOspTestSvr.SetInstAlias(CInstance::DAEMON, daemName_svr, (u8)(strlen(daemName_svr)+1));
	for(wInsNo=1; wInsNo<=MAX_INS_NUM; wInsNo++)
	{
		instLen = sprintf(instName, "svrInst%d", wInsNo);
		g_cOspTestSvr.SetInstAlias(wInsNo, instName, (u8)(instLen+1));
	}

	OspSetLogLevel(OSP_TEST_SERVER_APP_ID, 0, 0);
	OspSetTrcFlag(OSP_TEST_SERVER_APP_ID, 0, 0);

	// �����ͻ���
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
��������COspAgtIns::InstanceEntry
���ܣ���ͨʵ�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pMsg: �����Ϣָ�룬

����ֵ˵������.
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
		// ����Ӧ��
		tOspTestReqAck.iid = htonl(MAKEIID(GetAppID(), GetInsID()));
		tOspTestReqAck.aliasLen = GetAliasLen();
		GetAlias(tOspTestReqAck.achAlias, MAX_ALIAS_LENGTH, NULL);
		SetSyncAck(&tOspTestReqAck, sizeof(tOspTestReqAck));

        OspNodeDiscCBReg(pMsg->srcnode, GetAppID(), GetInsID());

		// ���ݲ�ͬ����������ת�벻ͬ��״̬
		reqType = ntohl(*(u32 *)pMsg->content);
		switch(reqType)
		{
		case OSP_TEST_TYPE_LOG:
			// ��״̬
			memset(&m_tOspLogStat, 0, sizeof(m_tOspLogStat));

			// ת����־����״̬
            NextState(LOG_TEST_STATE);
			break;

		case OSP_TEST_TYPE_COMM:
			// ��״̬
			memset(&m_tOspCommStat, 0, sizeof(m_tOspCommStat));

			// ת��ͨ�Ų���״̬
			NextState(COMM_TEST_STATE);
			break;

		case OSP_TEST_TYPE_TIMER:
			// ��״̬
			memset(&m_tOspTimerStat, 0, sizeof(m_tOspTimerStat));

			// ת�붨ʱ������״̬
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
			// ��������ת��Ϊ�����ֽ�˳��
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
			// �籾ʵ������������, �ȴ��ͻ��˵���Ϣ
			if((pMsg == NULL) || (pMsg->content == NULL) || (pMsg->length <= sizeof(u32)))
			{
				break;
			}

			ptOspCommTestCmdEx = (TOspCommTestCmdEx *)(pMsg->content + sizeof(u32)); // ��һ��u32Ϊ��������
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

			// �籾ʵ�������ͻ���, �������������
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

			// ����ԭʼ��������
			pWORD16RawData = (u16 *)m_tOspCommTestPara.rawData;

			srand( (unsigned)time(NULL) );
			for(i=0; i<m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.rawDataLen/2; i++)
			{
				pWORD16RawData[i] = (u16)rand();
			}

			// �������ת���������ԭʼ��������
			if(OSP_OK != post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				              OSP_COMM_TEST_RAWDATA,
				              m_tOspCommTestPara.rawData,
				              m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.rawDataLen,
				              g_uSvrNode))
			{
				break;
			}

			// �������
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
			// ȡ��һ���ɾ��Ļ���
			OspSetLogLevel(0,0,0);
			OspSetTrcFlag(0,0,0);
			OspSetLogLevel(GetAppID(), 0, 0);
			OspSetTrcFlag(GetAppID(), 0, 0);

			// ȡ�Ĳ��Բ���
			if(pMsg != NULL && pMsg->content != NULL)
			{
				ptOspLogTestCmd = (TOspLogTestCmd *)(pMsg->content + sizeof(u32)); // ��һ��u32Ϊ��������
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
			// ȡ�Ĳ��Բ���
			memset(&m_tOspTimerStat, 0 , sizeof(TOspTimerStat));
			if(pMsg != NULL && pMsg->content != NULL)
			{
				ptOspTimerTestCmd = (TOspTimerTestCmd *)(pMsg->content + sizeof(u32)); // ��һ��u32Ϊ��������
				if(ptOspTimerTestCmd != NULL)
				{
					// �ֽ���ת��
					ptOspTimerTestCmd->times = ntohl(ptOspTimerTestCmd->times);
					ptOspTimerTestCmd->intval = ntohl(ptOspTimerTestCmd->intval);

					if(ptOspTimerTestCmd->type == TIMER_TEST_TYPE_ABS)
					{   //���Զ�ʱ
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
��������COspAgtIns::OspTxTest
���ܣ����Ͳ��ԣ�������Ϣ�����б�Ҫ��ͳ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵������.
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
		case NoAliasGPost:  // ʹ��ʵ��ID��ȫ��post����
			ret = ::OspPost(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case AliasGPost:    // ʹ�ñ�����ȫ��post����
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

		case NoAliasGSend:  // ʹ��ʵ��ID��ȫ��send����
			ret = ::OspSend(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode,
				MAKEIID(GetAppID(), GetInsID())
				);
			break;

		case AliasGSend:    // ʹ�ñ�����ȫ��send����
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

		case NoAliasPost:   // ʹ��ʵ��ID��ʵ��post����
			ret = post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case AliasPost:     // ʹ�ñ�����ʵ��post����
			ret = post(m_tOspCommTestPara.tOspCommTestCmdEx.achAlias,
				m_tOspCommTestPara.tOspCommTestCmdEx.aliasLen,
				GETAPP(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid),
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case NoAliasSend:   // ʹ��ʵ��ID��ʵ��send����
			ret = send(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid,
				OSP_COMM_TEST,
				m_tOspCommTestPara.rawData,
				m_wCurLen,
				g_uSvrNode
				);
			break;

		case AliasSend:      // ʹ�ñ�����ʵ��send����
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

		// �����ͳɹ�, ʹ�õ�ǰ���ͳ��ȼ�1, �����ط�����
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

	// �ж��Ƿ������
	if(++m_tOspCommStat.tOspSendStat.sendTimes >= m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.times)
	{
		post(m_tOspCommTestPara.tOspCommTestCmdEx.peeriid, OSP_COMM_TEST_END, NULL, 0, g_uSvrNode);
//		NextState(IDLE_STATE);
		return;
	}

	// �����η��ͺ�ʱ������������ʱ����, ����һ���ʵ��ĳ�ʱֵ, �Ա��´η���
	// ����, ����һ���̵Ķ�ʱ�Կ��ٴ�����һ�η���(����, ��ʵ������һ����ʱ��Ϣ������������һ�η���)
	if(m_dwSendMs < m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.period)
	{
		SetTimer(OSP_COMM_TEST_TIMER, m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.period-m_dwSendMs);
	}
	else
	{
#if 0
	// �����⣺��Ϊ����Ϣ�ڱ�APP������������¿��ܻᶪʧ
		post(MAKEIID(GetAppID(), GetInsID()), OSP_COMM_TEST_TIMEOUT, NULL, 0);
#endif
		SetTimer(OSP_COMM_TEST_TIMER, 10);
	}
}

/*====================================================================
��������COspAgtIns::OspRxTest
���ܣ����ղ��ԣ�������Ϣ�����б�Ҫ��ͳ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pMsg: ��ǰ��Ϣָ�롣

����ֵ˵������.
====================================================================*/
void COspAgtIns::OspRxTest(const CMessage *pMsg)
{
	u32 dwChkLen;

	// �յ���һ��ʱ��ʼ��ʱ
	if(m_tOspCommStat.tOspRecvStat.recvPackets == 0)
	{
		m_dwStartTicks = OspTickGet();
	}

	// �����հ�ͳ��
	m_tOspCommStat.tOspRecvStat.recvPackets++;
    m_tOspCommStat.tOspRecvStat.recvBytes += pMsg->length;

	// ���ȴ�
	if(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkLenErr && pMsg->length != m_wExpectLen)
	{
		m_tOspCommStat.tOspRecvStat.recvLenError++;
	}

	// ���ݴ���С���ȼ��
    dwChkLen = (pMsg->length > m_wExpectLen) ? m_wExpectLen : pMsg->length;
	if(m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.bChkConErr && memcmp(pMsg->content, m_tOspCommTestPara.rawData, dwChkLen) != 0 )
	{
		m_tOspCommStat.tOspRecvStat.recvContError++;
	}

	// ��һ���ڴ��������Ǳ�������1
	m_wExpectLen = (u16)(pMsg->length+1);
	if(m_wExpectLen > m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.maxLen)
	{
		m_wExpectLen = m_tOspCommTestPara.tOspCommTestCmdEx.tOspCommTestCmd.minLen;
	}

	// �ۼ��հ�ʱ��
	m_tOspCommStat.tOspRecvStat.totalMS = tickToMs(OspTickGet() - m_dwStartTicks);
}

/*====================================================================
��������COspAgtIns::OspLogTest
���ܣ���־����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵������.
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
��������COspAgtIns::OspTimerTest
���ܣ���ʱ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵������.
====================================================================*/
void COspAgtIns::OspTimerTest(void)
{
	m_dwTimerTestStartTicks = OspTickGet();
	// ��Զ�ʱ����
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_REL)
	{
		SetTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.intval);
	}

	// ��ʱ��ɱ������
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_KILL)
	{
		for(u32 i=0; i<m_tOspTimerTestPara.times; i++)
		{
			SetTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.intval);
			OspDelay(m_tOspTimerTestPara.intval/2);
			KillTimer(OSP_TIMER_TEST_TIMER);
		}
	}

	//�����ʱ������
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_MULTI)
	{
		for(u32 i=m_tOspTimerTestPara.times; i>0; i--)
		{
			SetTimer(OSP_TIMER_TEST_MULTI_TIMER+i, m_tOspTimerTestPara.intval+i*5);
	        OspTaskDelay(1); // _WINDOWS_   1����
		}
	}

	/*���Զ�ʱ�����ԣ���λΪ��*/
	if(m_tOspTimerTestPara.type == TIMER_TEST_TYPE_ABS)
	{
		SetAbsTimer(OSP_TIMER_TEST_TIMER, m_tOspTimerTestPara.year, (u8)m_tOspTimerTestPara.month,
			        (u8)m_tOspTimerTestPara.day, (u8)m_tOspTimerTestPara.hour, (u8)m_tOspTimerTestPara.min,
					(u8)m_tOspTimerTestPara.sec);
	}
}