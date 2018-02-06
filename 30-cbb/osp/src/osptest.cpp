/******************************************************************************
模块名  ： OSP
文件名  ： ospTest.cpp
相关文件：
文件实现功能：搭建测试环境
作者    ：张文江
版本    ：1.0.02.7.5
******************************************************************************/

#include "../include/ospTest.h"

#define MAX_PARAM_LEN    6000
typedef struct
{
	u32 type;
	char para[MAX_PARAM_LEN];
}TTestStru;

static TOspTestReqAck g_tCltTestReqAck;
static TOspTestReqAck g_tSvrTestReqAck;

/*====================================================================
函数名：OspTestBuild
功能：为指定的待测功能建立测试环境
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的结点号, 
              (in) type -- 待建立的环境类型
			  (in) param -- 待测功能所需参数

返回值说明：成功返回OSP_OK，失败返回OSP_ERROR
====================================================================*/
API int OspTestBuild(u32 node, u32 type, void *param)
{
	int ret;
	TTestStru tTestStru;
	TOspCommTestReq *ptOspCommTestReq = NULL;
	u16 appID;
	TOspTestReqAck *ptOspTestReqAck = NULL;

	memset(&tTestStru, 0, sizeof(TTestStru));

	tTestStru.type = type;

	switch(type)
	{
	case OSP_TEST_TYPE_SERIAL:
		break;

	case OSP_TEST_TYPE_COMM:		
		if(sizeof(TOspCommTestReq) > MAX_PARAM_LEN)
			return OSP_ERROR;

		ptOspCommTestReq = (TOspCommTestReq *)param;
		if(ptOspCommTestReq == NULL) 
			return OSP_ERROR;
        
		if(ptOspCommTestReq->agtType == OSP_AGENT_TYPE_CLIENT)
		{
			appID = OSP_TEST_CLIENT_APP_ID;
		}
		else if(ptOspCommTestReq->agtType == OSP_AGENT_TYPE_SERVER)
		{
			appID = OSP_TEST_SERVER_APP_ID;
		}
		else
		{
			return OSP_ERROR;
		}

		if(ptOspCommTestReq->reqType == OSP_REQ_TYPE_SERVER)
		{
			ptOspTestReqAck = (TOspTestReqAck *)&g_tSvrTestReqAck;
		}
		else if(ptOspCommTestReq->reqType == OSP_REQ_TYPE_CLIENT)
		{
			ptOspTestReqAck = (TOspTestReqAck *)&g_tCltTestReqAck;
		}
		else
		{
			return OSP_ERROR;
		}
		
		type = htonl(type);
		ret =  ::OspSend(MAKEIID(appID, 0),		
				             OSP_TEST_REQ, 
							 &type, 
							 sizeof(u32), 
							 node, 
							 MAKEIID(INVALID_APP, INVALID_INS), 
							 INVALID_NODE, 
							 ptOspTestReqAck, 
							 sizeof(TOspTestReqAck),
							 NULL,
							 10000
							);
		if(ret != OSP_OK)
			return OSP_ERROR;

        ptOspTestReqAck->iid = ntohl(ptOspTestReqAck->iid);
		return OSP_OK;
		break;

    case OSP_TEST_TYPE_LOG:
		type = htonl(type);
		ret = ::OspSend(MAKEIID(OSP_TEST_SERVER_APP_ID, 0),		
				             OSP_TEST_REQ, 
							 &type, 
							 sizeof(u32), 
							 node, 
							 MAKEIID(INVALID_APP, INVALID_INS), 
							 INVALID_NODE, 
							 &g_tSvrTestReqAck, 
							 sizeof(TOspTestReqAck)
							);
		if(ret != OSP_OK)
			return OSP_ERROR;

        g_tSvrTestReqAck.iid = ntohl(g_tSvrTestReqAck.iid);
		return OSP_OK;
		break;

    case OSP_TEST_TYPE_TIMER:
		type = htonl(type);
		ret = ::OspSend(MAKEIID(OSP_TEST_SERVER_APP_ID, 0),		
				             OSP_TEST_REQ, 
							 &type, 
							 sizeof(u32), 
							 node, 
							 MAKEIID(INVALID_APP, INVALID_INS), 
							 INVALID_NODE, 
							 &g_tSvrTestReqAck, 
							 sizeof(TOspTestReqAck)
							);
		if(ret != OSP_OK)
			return OSP_ERROR;

        g_tSvrTestReqAck.iid = ntohl(g_tSvrTestReqAck.iid);
		return OSP_OK;
		break;

	case OSP_TEST_TYPE_NET:
		break;

	default:
		OspPrintf(TRUE, FALSE, "Undefined test type comes in OspTestBuild().\n");
		break;
	}
	return OSP_ERROR;
}

/*====================================================================
函数名：OspTestCmd
功能：发送测试命令以激发一个测试
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的全局标识, 
              (in) type -- 命令类别
			  (in) param -- 待测功能所需参数

返回值说明：成功返回OSP_OK，失败返回OSP_ERROR
====================================================================*/
API int OspTestCmd(u32 node, u32 type, void *param)
{
	u32 dstIID=MAKEIID(0, CInstance::INVALID);
	u32 testLen = 0;
	TTestStru tTestStru;
	TOspCommTestCmd *ptOspCommTestCmd = NULL;
	TOspCommTestCmdEx *ptOspCommTestCmdEx = NULL;
	TOspLogTestCmd *ptOspLogTestCmd = NULL;
	TOspTimerTestCmd *ptOspTimerTestCmd = NULL;

	memset(&tTestStru, 0, sizeof(tTestStru));

	tTestStru.type = htonl(type);

	switch(type)
	{
	case OSP_TEST_TYPE_SERIAL:
		break;

	case OSP_TEST_TYPE_NET:
		break;

	case OSP_TEST_TYPE_LOG:
		if(param == NULL)
		{
			return OSP_ERROR;
		}	

		ptOspLogTestCmd = (TOspLogTestCmd *)param;

		// 命令参数长度
		testLen = sizeof(TOspLogTestCmd) - MAX_LOGMSG_LEN + ptOspLogTestCmd->rawDataLen;
		if(testLen > MAX_PARAM_LEN)
			return OSP_ERROR;
	
		memcpy(tTestStru.para, param, testLen);
		
        // 转换为网络字节序发送
		ptOspLogTestCmd = (TOspLogTestCmd *)tTestStru.para;
		ptOspLogTestCmd->logFileSize = htonl(ptOspLogTestCmd->logFileSize);
        ptOspLogTestCmd->logNum = htonl(ptOspLogTestCmd->logNum);
        ptOspLogTestCmd->rawDataLen = htonl(ptOspLogTestCmd->rawDataLen);		

		// 要发送给测试代理的实际长度为命令参数加上一个u32
		dstIID = g_tSvrTestReqAck.iid;
		testLen += sizeof(u32);		
		break;

	case OSP_TEST_TYPE_TIMER:
		if(param == NULL)
		{
			return OSP_ERROR;
		}	

		// 命令参数长度
		testLen = sizeof(TOspTimerTestCmd);
		if(testLen > MAX_PARAM_LEN)
			return OSP_ERROR;		

		memcpy(tTestStru.para, param, testLen);
        ptOspTimerTestCmd = (TOspTimerTestCmd *)tTestStru.para;

		// 字节序转换
        ptOspTimerTestCmd->times = htonl(ptOspTimerTestCmd->times);
		ptOspTimerTestCmd->intval = htonl(ptOspTimerTestCmd->intval);
		
		if(ptOspTimerTestCmd->type == TIMER_TEST_TYPE_ABS)
		{   //绝对定时
			ptOspTimerTestCmd->year = htons(ptOspTimerTestCmd->year);
			ptOspTimerTestCmd->month = htons(ptOspTimerTestCmd->month);
			ptOspTimerTestCmd->day = htons(ptOspTimerTestCmd->day);
			ptOspTimerTestCmd->hour = htons(ptOspTimerTestCmd->hour);
			ptOspTimerTestCmd->min = htons(ptOspTimerTestCmd->min);
			ptOspTimerTestCmd->sec = htons(ptOspTimerTestCmd->sec);
		}

		// 实际发送给测试代理的数据长度
		dstIID = g_tSvrTestReqAck.iid;
		testLen += sizeof(u32);
		break;

	case OSP_TEST_TYPE_COMM:
		if(param == NULL) 
		{
			return OSP_ERROR;
		}

		if(sizeof(TOspCommTestCmdEx) > MAX_PARAM_LEN)
		{
			return OSP_ERROR;
		}

		ptOspCommTestCmd = (TOspCommTestCmd *)param;
		ptOspCommTestCmdEx = (TOspCommTestCmdEx *)&tTestStru.para;

		memcpy(&ptOspCommTestCmdEx->tOspCommTestCmd, ptOspCommTestCmd, sizeof(TOspCommTestCmd));

		ptOspCommTestCmdEx->tOspCommTestCmd.reqType = htonl(ptOspCommTestCmdEx->tOspCommTestCmd.reqType);
//      ptOspCommTestCmdEx->tOspCommTestCmd.ip = ptOspCommTestCmdEx->tOspCommTestCmd.ip; 已经是网络字节顺序?
        ptOspCommTestCmdEx->tOspCommTestCmd.port = htons(ptOspCommTestCmdEx->tOspCommTestCmd.port);
        ptOspCommTestCmdEx->tOspCommTestCmd.times = htonl(ptOspCommTestCmdEx->tOspCommTestCmd.times);
        ptOspCommTestCmdEx->tOspCommTestCmd.period = htons(ptOspCommTestCmdEx->tOspCommTestCmd.period);
        ptOspCommTestCmdEx->tOspCommTestCmd.packets = htons(ptOspCommTestCmdEx->tOspCommTestCmd.packets);
        ptOspCommTestCmdEx->tOspCommTestCmd.minLen = htons(ptOspCommTestCmdEx->tOspCommTestCmd.minLen);
        ptOspCommTestCmdEx->tOspCommTestCmd.maxLen = htons(ptOspCommTestCmdEx->tOspCommTestCmd.maxLen);
        ptOspCommTestCmdEx->tOspCommTestCmd.bChkLenErr = htonl(ptOspCommTestCmdEx->tOspCommTestCmd.bChkLenErr);
        ptOspCommTestCmdEx->tOspCommTestCmd.bChkConErr = htonl(ptOspCommTestCmdEx->tOspCommTestCmd.bChkConErr);
        ptOspCommTestCmdEx->tOspCommTestCmd.rawDataLen = htons(ptOspCommTestCmdEx->tOspCommTestCmd.rawDataLen);
  
		// 先向客户实例发送测试命令
		if(ptOspCommTestCmd->reqType == OSP_AGENT_TYPE_CLIENT)
		{
			ptOspCommTestCmdEx->peeriid = htonl(g_tSvrTestReqAck.iid);
			ptOspCommTestCmdEx->aliasLen = g_tSvrTestReqAck.aliasLen;
			memcpy(ptOspCommTestCmdEx->achAlias, g_tSvrTestReqAck.achAlias, g_tSvrTestReqAck.aliasLen);
			return OspPost(g_tCltTestReqAck.iid, OSP_TEST_CMD, &tTestStru, sizeof(u32) + sizeof(TOspCommTestCmdEx), node);
		}

		if(ptOspCommTestCmd->reqType == OSP_AGENT_TYPE_SERVER)
		{
			ptOspCommTestCmdEx->peeriid = htonl(g_tCltTestReqAck.iid);
			ptOspCommTestCmdEx->aliasLen = g_tCltTestReqAck.aliasLen;
			memcpy(ptOspCommTestCmdEx->achAlias, g_tCltTestReqAck.achAlias, g_tCltTestReqAck.aliasLen);
			return OspPost(g_tSvrTestReqAck.iid, OSP_TEST_CMD, &tTestStru, sizeof(u32) + sizeof(TOspCommTestCmdEx), node);
		}
		return OSP_ERROR;
		break;

	default:
		OspPrintf(TRUE, FALSE, "Undefined test type comes in OspTestBuild().\n");
		return OSP_ERROR;
		break;
	}

	return OspPost(dstIID, OSP_TEST_CMD, &tTestStru, (u16)testLen, node);
}

/*====================================================================
函数名：OspTestQuery
功能：查询测试结果
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的全局标识, 
              (in) type -- 待查询功能类别
			  (out) param -- 存放查询结果的buf

返回值说明：无
====================================================================*/
API int OspTestQuery(u32 node, u32 type, void *param)
{
	int ret;
	u16 realLen = 0;
	TTestStru tTestStru;
	TOspCommTestResult *ptOspCommTestResult = NULL;
	TOspLogTestResult *ptOspLogTestResult = NULL;
	TOspTimerTestResult *ptOspTimerTestResult = NULL;
		
	if(param == NULL)
	{
		return OSP_ERROR;
	}

	memset(&tTestStru, 0, sizeof(tTestStru));

	tTestStru.type = htonl(type);

	switch(type)
	{
	case OSP_TEST_TYPE_SERIAL:
		break;

	case OSP_TEST_TYPE_NET:
		break;

	case OSP_TEST_TYPE_LOG:
		if(param == NULL)
			return OSP_ERROR;

		type = htonl(type);
        ptOspLogTestResult = (TOspLogTestResult *)param;
		ret = ::OspSend(g_tSvrTestReqAck.iid, 
			            OSP_TEST_QRY, 
						&type, 
						sizeof(u32), 
						node, 
						MAKEIID(INVALID_APP, INVALID_INS), 
						INVALID_NODE, 
						ptOspLogTestResult, 
						sizeof(TOspLogTestResult), 
						&realLen
					   );
		if(ret != OSP_OK) 
			return OSP_ERROR;

        ptOspLogTestResult->bLogNumIncreased = ntohl(ptOspLogTestResult->bLogNumIncreased);
		ptOspLogTestResult->bLogOutputInFile = ntohl(ptOspLogTestResult->bLogOutputInFile);
		break;

	case OSP_TEST_TYPE_TIMER:
		if(param == NULL)
			return OSP_ERROR;

		type = htonl(type);
        ptOspTimerTestResult = (TOspTimerTestResult *)param;
		ret = ::OspSend(g_tSvrTestReqAck.iid, 
			            OSP_TEST_QRY, 
						&type, 
						sizeof(u32), 
						node, 
						MAKEIID(INVALID_APP, INVALID_INS), 
						INVALID_NODE, 
						ptOspTimerTestResult, 
						sizeof(TOspTimerTestResult), 
						&realLen
					   );
		if(ret != OSP_OK) 
			return OSP_ERROR;

	    ptOspTimerTestResult->timeouts = ntohl(ptOspTimerTestResult->timeouts);
        ptOspTimerTestResult->totalMs = ntohl(ptOspTimerTestResult->totalMs);
		break;

	case OSP_TEST_TYPE_COMM:
		if(param == NULL)
			return OSP_ERROR;

		ptOspCommTestResult = (TOspCommTestResult *)param;

		type = htonl(type);
		if(ptOspCommTestResult->agtType == OSP_AGENT_TYPE_SERVER)
		{
			ret = ::OspSend(g_tSvrTestReqAck.iid, 
				            OSP_TEST_QRY, 
							&type, 
							sizeof(u32), 
							node, 
							MAKEIID(INVALID_APP, INVALID_INS), 
							INVALID_NODE, 
							&ptOspCommTestResult->tServerStat, 
							sizeof(TOspCommStat), 
							&realLen,
							10000
						   );
			if(ret != OSP_OK) 
				return OSP_ERROR;
			
			ptOspCommTestResult->tServerStat.tOspRecvStat.recvPackets = ntohl(ptOspCommTestResult->tServerStat.tOspRecvStat.recvPackets);
			ptOspCommTestResult->tServerStat.tOspRecvStat.recvLenError = ntohl(ptOspCommTestResult->tServerStat.tOspRecvStat.recvLenError);
			ptOspCommTestResult->tServerStat.tOspRecvStat.recvContError = ntohl(ptOspCommTestResult->tServerStat.tOspRecvStat.recvContError);
			ptOspCommTestResult->tServerStat.tOspRecvStat.recvBytes = ntohl(ptOspCommTestResult->tServerStat.tOspRecvStat.recvBytes);
			ptOspCommTestResult->tServerStat.tOspRecvStat.totalMS = ntohl(ptOspCommTestResult->tServerStat.tOspRecvStat.totalMS);
			ptOspCommTestResult->tServerStat.tOspSendStat.sendTimes = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.sendTimes);
			ptOspCommTestResult->tServerStat.tOspSendStat.sendPackets = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.sendPackets);
			ptOspCommTestResult->tServerStat.tOspSendStat.sendTimeouts = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.sendTimeouts);
			ptOspCommTestResult->tServerStat.tOspSendStat.totalMS = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.totalMS);
			ptOspCommTestResult->tServerStat.tOspSendStat.sendBytes = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.sendBytes);
			ptOspCommTestResult->tServerStat.tOspSendStat.succSendPackets = ntohl(ptOspCommTestResult->tServerStat.tOspSendStat.succSendPackets);
        }
		else if(ptOspCommTestResult->agtType == OSP_AGENT_TYPE_CLIENT)
		{
			ret = ::OspSend(g_tCltTestReqAck.iid, 
				            OSP_TEST_QRY, 
							&type, 
							sizeof(u32), 
							node, 
							MAKEIID(INVALID_APP, INVALID_INS), 
							INVALID_NODE, 
							&ptOspCommTestResult->tClientStat, 
							sizeof(TOspCommStat), 
							&realLen,
							10000
						   );
			if(ret != OSP_OK) 
				return OSP_ERROR;
			
			ptOspCommTestResult->tClientStat.tOspRecvStat.recvPackets = ntohl(ptOspCommTestResult->tClientStat.tOspRecvStat.recvPackets);
			ptOspCommTestResult->tClientStat.tOspRecvStat.recvLenError = ntohl(ptOspCommTestResult->tClientStat.tOspRecvStat.recvLenError);
			ptOspCommTestResult->tClientStat.tOspRecvStat.recvContError = ntohl(ptOspCommTestResult->tClientStat.tOspRecvStat.recvContError);
			ptOspCommTestResult->tClientStat.tOspRecvStat.recvBytes = ntohl(ptOspCommTestResult->tClientStat.tOspRecvStat.recvBytes);
			ptOspCommTestResult->tClientStat.tOspRecvStat.totalMS = ntohl(ptOspCommTestResult->tClientStat.tOspRecvStat.totalMS);
			ptOspCommTestResult->tClientStat.tOspSendStat.sendTimes = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.sendTimes);
			ptOspCommTestResult->tClientStat.tOspSendStat.sendPackets = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.sendPackets);
			ptOspCommTestResult->tClientStat.tOspSendStat.sendTimeouts = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.sendTimeouts);
			ptOspCommTestResult->tClientStat.tOspSendStat.totalMS = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.totalMS);
			ptOspCommTestResult->tClientStat.tOspSendStat.sendBytes = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.sendBytes);
			ptOspCommTestResult->tClientStat.tOspSendStat.succSendPackets = ntohl(ptOspCommTestResult->tClientStat.tOspSendStat.succSendPackets);
		}
		else
		{
			return OSP_ERROR;
		}
		break;

	default:
		OspPrintf(TRUE, FALSE, "Undefined test type comes in OspTestBuild().\n");
		return OSP_ERROR;
	}

	return OSP_OK;
}
