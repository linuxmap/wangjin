/******************************************************************************
模块名  ： OSP
文件名  ： ospNodeMan.cpp
相关文件：
文件实现功能：OSP结点管理功能的主要实现文件
作者    ：向飞
版本    ：1.0.02.7.5
-------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
06/11/2003  2.0         向飞          创建
******************************************************************************/

#include "../include/ospNodeMan.h"
#include "../include/ospSch.h"
#include <time.h>
#ifndef OSP_COMPRESS_LOG_LEVEL
#define OSP_COMPRESS_LOG_LEVEL 21
#endif

extern COsp g_Osp;

// 主动释放Timer，消除内存泄露 [1/26/2011 yhq]
//实例退出
void CNodeManInstance::InstanceExit(void)
{
	KillTimer(NODE_SCAN_TIMER);
}
/*====================================================================
函数名：CNodeManInstance::InstanceEntry
功能：结点检测实例的消息处理入口
算法实现：（可选项）
引用全局变量：
输入参数说明：pMsg: 消息头部指针

  返回值说明：无
====================================================================*/
void CNodeManInstance::InstanceEntry(CMessage *const pMsg)
{
	u32 dwCurState = CurState();
	u16 wCurEvent = pMsg->event;
	u32 dwSrcNode = pMsg->srcnode;

	switch(dwCurState)
	{
	case IDLE_STATE:
		switch(wCurEvent)
		{
		case START_UP_EVENT:
			/* 初始化一些统计数据 */
			m_dwStatPrtCount = 0;

			g_Osp.m_cNodePool.m_wNodeDisconnTimes = 0;
			g_Osp.m_cNodePool.m_wNodeHBDiscnTimes = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnByApp = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnBySendFailed = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnByRecvFailed = 0;

			//设置扫描定时器
			SetTimer(NODE_SCAN_TIMER, NODE_SCAN_INTERVAL);

			/* 转入运行态 */
			NextState(RUNNING_STATE);
			break;

		default:
			break;
		}
		break;

	case RUNNING_STATE:
		switch(wCurEvent)			
		{
		case NODE_SCAN_TIMEOUT:
			//结点检测
			NodeScan();
			//设置扫描定时器
			SetTimer(NODE_SCAN_TIMER, NODE_SCAN_INTERVAL);
			break;

		case OSP_NETBRAECHO:
			/* 如果是对方发来的连接检测消息，返回一个应答 */
			if( g_Osp.m_cNodePool.IsNodeCheckEnable(dwSrcNode) )
			{
				post(MAKEIID(NODE_MAN_APPID, 1), OSP_NETBRAECHOACK, NULL, 0, pMsg->srcnode);
			}		
			break;

		case OSP_NETBRAECHOACK:
			/* 如果是对方发来的连接检测回应消息，则设置该node的回应到达flag */
			if(dwSrcNode > 0 && dwSrcNode <= MAX_NODE_NUM)
			{
				OspTaskSafe();
				OspSemTake(g_Osp.m_cNodePool.m_tSemaNodePool);
				g_Osp.m_cNodePool.m_acNodeRegTable[dwSrcNode-1].m_bMsgEchoAck = TRUE;
				OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
				OspTaskUnsafe();
			}
			break;

		case OSP_NETSTATEST:
			/* 收到对方发来的ping消息，返回一个应答 */
			post(MAKEIID(NODE_MAN_APPID, 1), OSP_NETSTATESTACK, NULL, 0, pMsg->srcnode);
			break;

		case OSP_NETSTATESTACK:
            {            
			    /* 收到ping应答，打印出来 */
                u32 dwip=OspNodeLastIpGet(dwSrcNode);
			    OspPrintf(TRUE, FALSE, "Osp: received ping ack message from node %d (%u.%u.%u.%u)\n", dwSrcNode, OspQuadAddr(dwip));
            }
			break;
        case OSP_COMPRESS_SUPPORT: //by wubin 2011-4-13
            /*收到对方发来的压缩支持通知*/
            if(dwSrcNode > 0 && dwSrcNode <= MAX_NODE_NUM)
            {
                OspTaskSafe();
                OspSemTake(g_Osp.m_cNodePool.m_tSemaNodePool);
                if(FALSE == g_Osp.m_cNodePool.m_acNodeRegTable[dwSrcNode-1].m_bCMessageCompressSupport)
                {
                    // 对方支持压缩消息，回应OSP_COMPRESS_SUPPORT表示本方支持压缩消息
                    g_Osp.m_cNodePool.m_acNodeRegTable[dwSrcNode-1].m_bCMessageCompressSupport = TRUE;
                    OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
                    OspTaskUnsafe();
                    post(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, dwSrcNode);
                    OspLog(OSP_COMPRESS_LOG_LEVEL, "OSP_COMPRESS_SUPPORT set to node %d\n", dwSrcNode);
                }
                else
                {
                    // 先前已确认对方支持压缩消息，不再回应OSP_COMPRESS_SUPPORT，双方确认均支持压缩消息
                    OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
                    OspTaskUnsafe();
                }
            }
            break;
        default:
            break;
        
        }
		break;

	default:
		break;
	}
}

/*====================================================================
函数名：CNodeManInstance::NodeScan
功能：每1秒钟进行一次连接检测
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：无
====================================================================*/
void CNodeManInstance::NodeScan()
{
    time_t tCurTime;	
	
	/*检查是否有内存检查告警*/
	if(g_Osp.m_bErrArised == TRUE)
	{
		TTaskInfo* ptCurTask = g_Osp.GetFirstTask();
		TTaskInfo* ptNextTask = NULL;
		while( ptCurTask != NULL )
		{
			ptNextTask = g_Osp.GetNextTask(ptCurTask->id);
			if(OspTaskHandleVerify(ptCurTask->handle))
			{
				if(memcmp(ptCurTask->name, g_Osp.m_chBeforErrThread, strlen(g_Osp.m_chBeforErrThread)) == 0)
					OspTaskSuspend(ptCurTask->handle);		
				else if(memcmp(ptCurTask->name, g_Osp.m_chErrCurrentThread, strlen(g_Osp.m_chErrCurrentThread)) == 0)
					OspTaskSuspend(ptCurTask->handle);
			}   
			ptCurTask = ptNextTask;
		}

		g_Osp.m_bErrArised = FALSE;
	}

	/* 每1小时打印一次系统状态 */
	if( ++m_dwStatPrtCount > DEFAULT_STATUS_MSGOUT_TIME ) 
	{
		m_dwStatPrtCount = 0;
		
//		OspPrintf(TRUE, TRUE, "\n*********************************************************************\n");
		OspLog(1, "\n*********************************************************************\n");
		
		time(&tCurTime);
		//OspPrintf(TRUE, TRUE, "Osp: current time and date:\t\t%s", ctime(&tCurTime));
		OspLog(1, "Osp: current time and date:\t\t%s", ctime(&tCurTime));
		OspVerPrintf();
		
		/* 如果状态打印功能启用，打印系统状态 */
		if(g_Osp.m_bStatusPrtEnable)
		{
			OspInstShowAll();
			OspTimerShow();
			OspNodeShow();
			OspAppShow();
			OspDispatchShow();			   
		}	
//		OspPrintf(TRUE, TRUE, "\n*********************************************************************\n");
		OspLog(1, "\n*********************************************************************\n");
		
		/* 使得一些简单函数不被编译器优化掉，以便能在Telnet上作为命令输出 */
		if(g_Osp.m_bCmdFuncEnable)
		{
			OspCmdFuncEnable();
		}	
	}
	
	/* 扫描结点池中所有结点的连接状况 */
	g_Osp.m_cNodePool.Scan();
}
