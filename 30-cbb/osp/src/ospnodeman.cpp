/******************************************************************************
ģ����  �� OSP
�ļ���  �� ospNodeMan.cpp
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP�������ܵ���Ҫʵ���ļ�
����    �����
�汾    ��1.0.02.7.5
-------------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
06/11/2003  2.0         ���          ����
******************************************************************************/

#include "../include/ospNodeMan.h"
#include "../include/ospSch.h"
#include <time.h>
#ifndef OSP_COMPRESS_LOG_LEVEL
#define OSP_COMPRESS_LOG_LEVEL 21
#endif

extern COsp g_Osp;

// �����ͷ�Timer�������ڴ�й¶ [1/26/2011 yhq]
//ʵ���˳�
void CNodeManInstance::InstanceExit(void)
{
	KillTimer(NODE_SCAN_TIMER);
}
/*====================================================================
��������CNodeManInstance::InstanceEntry
���ܣ������ʵ������Ϣ�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pMsg: ��Ϣͷ��ָ��

  ����ֵ˵������
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
			/* ��ʼ��һЩͳ������ */
			m_dwStatPrtCount = 0;

			g_Osp.m_cNodePool.m_wNodeDisconnTimes = 0;
			g_Osp.m_cNodePool.m_wNodeHBDiscnTimes = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnByApp = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnBySendFailed = 0;
			g_Osp.m_cNodePool.m_wNodeDiscnByRecvFailed = 0;

			//����ɨ�趨ʱ��
			SetTimer(NODE_SCAN_TIMER, NODE_SCAN_INTERVAL);

			/* ת������̬ */
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
			//�����
			NodeScan();
			//����ɨ�趨ʱ��
			SetTimer(NODE_SCAN_TIMER, NODE_SCAN_INTERVAL);
			break;

		case OSP_NETBRAECHO:
			/* ����ǶԷ����������Ӽ����Ϣ������һ��Ӧ�� */
			if( g_Osp.m_cNodePool.IsNodeCheckEnable(dwSrcNode) )
			{
				post(MAKEIID(NODE_MAN_APPID, 1), OSP_NETBRAECHOACK, NULL, 0, pMsg->srcnode);
			}		
			break;

		case OSP_NETBRAECHOACK:
			/* ����ǶԷ����������Ӽ���Ӧ��Ϣ�������ø�node�Ļ�Ӧ����flag */
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
			/* �յ��Է�������ping��Ϣ������һ��Ӧ�� */
			post(MAKEIID(NODE_MAN_APPID, 1), OSP_NETSTATESTACK, NULL, 0, pMsg->srcnode);
			break;

		case OSP_NETSTATESTACK:
            {            
			    /* �յ�pingӦ�𣬴�ӡ���� */
                u32 dwip=OspNodeLastIpGet(dwSrcNode);
			    OspPrintf(TRUE, FALSE, "Osp: received ping ack message from node %d (%u.%u.%u.%u)\n", dwSrcNode, OspQuadAddr(dwip));
            }
			break;
        case OSP_COMPRESS_SUPPORT: //by wubin 2011-4-13
            /*�յ��Է�������ѹ��֧��֪ͨ*/
            if(dwSrcNode > 0 && dwSrcNode <= MAX_NODE_NUM)
            {
                OspTaskSafe();
                OspSemTake(g_Osp.m_cNodePool.m_tSemaNodePool);
                if(FALSE == g_Osp.m_cNodePool.m_acNodeRegTable[dwSrcNode-1].m_bCMessageCompressSupport)
                {
                    // �Է�֧��ѹ����Ϣ����ӦOSP_COMPRESS_SUPPORT��ʾ����֧��ѹ����Ϣ
                    g_Osp.m_cNodePool.m_acNodeRegTable[dwSrcNode-1].m_bCMessageCompressSupport = TRUE;
                    OspSemGive(g_Osp.m_cNodePool.m_tSemaNodePool);
                    OspTaskUnsafe();
                    post(MAKEIID(NODE_MAN_APPID, 1), OSP_COMPRESS_SUPPORT, NULL, 0, dwSrcNode);
                    OspLog(OSP_COMPRESS_LOG_LEVEL, "OSP_COMPRESS_SUPPORT set to node %d\n", dwSrcNode);
                }
                else
                {
                    // ��ǰ��ȷ�϶Է�֧��ѹ����Ϣ�����ٻ�ӦOSP_COMPRESS_SUPPORT��˫��ȷ�Ͼ�֧��ѹ����Ϣ
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
��������CNodeManInstance::NodeScan
���ܣ�ÿ1���ӽ���һ�����Ӽ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������
====================================================================*/
void CNodeManInstance::NodeScan()
{
    time_t tCurTime;	
	
	/*����Ƿ����ڴ���澯*/
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

	/* ÿ1Сʱ��ӡһ��ϵͳ״̬ */
	if( ++m_dwStatPrtCount > DEFAULT_STATUS_MSGOUT_TIME ) 
	{
		m_dwStatPrtCount = 0;
		
//		OspPrintf(TRUE, TRUE, "\n*********************************************************************\n");
		OspLog(1, "\n*********************************************************************\n");
		
		time(&tCurTime);
		//OspPrintf(TRUE, TRUE, "Osp: current time and date:\t\t%s", ctime(&tCurTime));
		OspLog(1, "Osp: current time and date:\t\t%s", ctime(&tCurTime));
		OspVerPrintf();
		
		/* ���״̬��ӡ�������ã���ӡϵͳ״̬ */
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
		
		/* ʹ��һЩ�򵥺��������������Ż������Ա�����Telnet����Ϊ������� */
		if(g_Osp.m_bCmdFuncEnable)
		{
			OspCmdFuncEnable();
		}	
	}
	
	/* ɨ����������н�������״�� */
	g_Osp.m_cNodePool.Scan();
}
