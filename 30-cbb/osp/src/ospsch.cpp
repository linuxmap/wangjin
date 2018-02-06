/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP 系统调度功能的主要实现文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98		1.0      某某        ------------
******************************************************************************/

#include "stdio.h"
#include "ospsch.h"
#include "ospteleserver.h"


COsp g_Osp;

u32    g_dwMallocTimes;
u32    g_dwFreeTimes;

extern s8 g_TelnetUsername[AUTHORIZATION_NAME_SIZE];
extern s8 g_TelnetPasswd[AUTHORIZATION_NAME_SIZE];

//char g_achModuleName[MAX_MODULE_NAME_LENGTH+1] = "osp";
char g_achModuleName[MAX_MODULE_NAME_LENGTH+1] = "";

extern SEMHANDLE g_tCpuInfoSem;
extern SEMHANDLE g_tTickGet64Sem;
int g_max_msg_waiting = 0;
u32 max_inst_entry_interval = 0;

extern TASKID g_dwTeletTaskID;
extern TASKID g_dwPostDaemonTaskId;

/*====================================================================
函数名：OspGetLocalID
功能：得到本地结点号
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：本地结点号
====================================================================*/
API int OspGetLocalID()
{
    return 0;
}

//======================================================================================================//
BOOL32 COspStack::StackCreate(u32 dwStackInitBlkNum)
{
	u32 dwCount;
	_TOspStackNode *ptNewNode = NULL;
	void *pNewMallocMem = NULL;
	OspSemTake(m_hStackSem);
	for(dwCount = 0;dwCount < dwStackInitBlkNum;++dwCount)
	{
		pNewMallocMem = malloc(sizeof(_TOspStackNode)+m_dwStackMemBlkSize);
		if(NULL != pNewMallocMem)
		{
			memset(pNewMallocMem,0,sizeof(_TOspStackNode)+m_dwStackMemBlkSize);
			ptNewNode = (_TOspStackNode *)pNewMallocMem;
			ptNewNode->tHeader.hMemStackAddr = (OSP_MEMSTACKHANDLE)this;
			ptNewNode->tHeader.dwFlag = m_dwStackMark;
			ptNewNode->tHeader.bReturn = TRUE;
			ptNewNode->tHeader.pvNextNode = m_ptTopNode;
			m_ptTopNode = ptNewNode;
			ptNewNode->pBuffStart = (void*)((u8*)pNewMallocMem + sizeof(_TOspStackNode));
			++m_dwStackAvailbleBlkNum;
			++m_dwdStackTotalBlkNum;
		}
		else
		{
			//内存分配失败，释放申请的内存
			OspSemGive(m_hStackSem);
			StackDestroy();
			return FALSE;
		}
	}
	OspSemGive(m_hStackSem);
	return TRUE;
}

BOOL32 COspStack::StackDestroy()
{
	OspSemTake(m_hStackSem);
	_TOspStackNode *pDelNode = NULL;
	while (NULL != m_ptTopNode)
	{
		if (m_ptTopNode->tHeader.dwFlag == m_dwStackMark)
		{
			pDelNode = m_ptTopNode;
			m_ptTopNode = (_TOspStackNode *)((m_ptTopNode->tHeader).pvNextNode);
			free(pDelNode);
			--m_dwdStackTotalBlkNum;
			--m_dwStackAvailbleBlkNum;
			pDelNode = NULL;
		}
		else
		{
			printf("error Mark in %d size buff\n",m_dwStackMemBlkSize);
			OspSemGive(m_hStackSem);
			return FALSE;
		}
	}
	//当销毁内存池时，已分配数应该为0，否则视为由游离内存块没有还回链表，此时不应该销毁内存池
	if (0 != m_dwdStackTotalBlkNum)
	{
		printf("maybe this still have %d blk size :%x not return\n",m_dwdStackTotalBlkNum,m_dwStackMemBlkSize);
		OspSemGive(m_hStackSem);
		return FALSE;
	}
	OspSemGive(m_hStackSem);
	return TRUE;
}

void* COspStack::StackAlloc()
{
	void* pAllocNode = NULL;
	OspSemTake(m_hStackSem);
	if (NULL != m_ptTopNode)
	{
		pAllocNode = (void*)m_ptTopNode;
		assert(m_dwStackMark == m_ptTopNode->tHeader.dwFlag);
		m_ptTopNode = (_TOspStackNode*)(((_TOspStackNode*)pAllocNode)->tHeader.pvNextNode);
		((_TOspStackNode*)pAllocNode)->tHeader.pvNextNode = NULL;
		((_TOspStackNode*)pAllocNode)->tHeader.bReturn = FALSE;
		memset(((_TOspStackNode*)pAllocNode)->pBuffStart,0,m_dwStackMemBlkSize);
		--m_dwStackAvailbleBlkNum;
	}
	else
	{
		pAllocNode = malloc(sizeof(_TOspStackNode)+m_dwStackMemBlkSize);
		if(NULL == pAllocNode)
		{
			printf("malloc size:%u failed,errno:%d\n",m_dwStackMemBlkSize,errno);
			OspSemGive(m_hStackSem);
			return pAllocNode;
		}
		memset(pAllocNode,0,sizeof(_TOspStackNode)+m_dwStackMemBlkSize);
		((_TOspStackNode *)pAllocNode)->tHeader.hMemStackAddr = (OSP_MEMSTACKHANDLE)this;
		((_TOspStackNode *)pAllocNode)->tHeader.dwFlag = m_dwStackMark;
		((_TOspStackNode *)pAllocNode)->tHeader.bReturn = FALSE;
		((_TOspStackNode *)pAllocNode)->tHeader.pvNextNode = NULL;
		((_TOspStackNode *)pAllocNode)->pBuffStart = (void *)((u8*)pAllocNode + sizeof(_TOspStackNode));
		m_dwdStackTotalBlkNum++;
	}
	OspSemGive(m_hStackSem);
	return ((_TOspStackNode *)pAllocNode)->pBuffStart;
}

void COspStack::StackStateShow()
{
	OspSemTake(m_hStackSem);
	printf("\nm_dwStackMemBlkSize :%x\t,m_dwStackAvailbleBlkNum :%u\t,m_dwdStackTotalBlkNum :%u\t\n",m_dwStackMemBlkSize,m_dwStackAvailbleBlkNum,m_dwdStackTotalBlkNum);
	OspSemGive(m_hStackSem);
}

void COspStack::StackReturn(void* pMsg)
{
	void *pRetNode = NULL;
	if (NULL != pMsg)
	{
		OspSemTake(m_hStackSem);
		pRetNode = (void *)((u8*)pMsg - sizeof(_TOspStackNode));//存在传入的指针已经被free的情况，此时无法检测
		if(m_dwStackMark == ((_TOspStackNode*)pRetNode)->tHeader.dwFlag)
		{
			//此处可以加比例判断
			//
			//
			if (TRUE == ((_TOspStackNode*)pRetNode)->tHeader.bReturn)
			{
				//已回收，避免重复回收
				OspSemGive(m_hStackSem);
				return;
			}
			if( (0 == m_dwdStackTotalBlkNum) ||
				(((m_dwStackAvailbleBlkNum+1)*100/m_dwdStackTotalBlkNum)<m_dwStackAvailbleBlkPercentUpperLimit)
				)//感觉每次计算除法比较耗时，最好用定时器定时检测
			{
				((_TOspStackNode*)pRetNode)->tHeader.bReturn = TRUE;
				((_TOspStackNode*)pRetNode)->tHeader.pvNextNode = (void *)m_ptTopNode;
				m_ptTopNode = (_TOspStackNode*)pRetNode;
				++m_dwStackAvailbleBlkNum;
			}
			else
			{
				//			    memset(((_TOspStackNode*)pRetNode)->pBuffStart,0,m_dwStackMemBlkSize);
				free(pRetNode);
				--m_dwdStackTotalBlkNum;
			}
			//当不存在使用块时，存在峰值的百分比个空闲，需处理
			//////////////////////////////////////////////////////////////////////////
			if (m_dwStackAvailbleBlkNum == m_dwdStackTotalBlkNum)
			{
				while(NULL !=m_ptTopNode)
				{
					if (1 == m_dwdStackTotalBlkNum)
					{
						break; //既然已经使用过这类类型的内存，则在不使用的时候保存一个内存节点。
					}
					pRetNode = m_ptTopNode;
					m_ptTopNode = (_TOspStackNode *)(m_ptTopNode->tHeader.pvNextNode);
					free(pRetNode);
					--m_dwdStackTotalBlkNum;
					--m_dwStackAvailbleBlkNum;
				}
			}
		}
		else
		{
			/* 传入的内存块不正确，或者已经被损坏，不做任何处理 */
			s32 ospstack_marker_lost = 0;
			assert(ospstack_marker_lost);
			//失败处理
			printf("StackReturn flag %x is not %x\n",((_TOspStackNode*)pRetNode)->tHeader.dwFlag,m_dwStackMark);
		}
		OspSemGive(m_hStackSem);
	}
	return;
}

BOOL32 COspMemPool::OspMemPoolInit(TOspMemPoolPara *ptMemPoolPara)
{
	if (NULL != ptMemPoolPara)
	{
		m_tOspMemPoolPara.dwLargeBlockLevelGrowSize = ptMemPoolPara->dwLargeBlockLevelGrowSize;
		m_tOspMemPoolPara.dwMaxFreePercent = ptMemPoolPara->dwMaxFreePercent;
	}
	u32 dwCurrStackNums = 0;
	u32 dwBlockSize = 64;
	OspSemTake(m_hMemPoolSem);
	if (TRUE == m_bInitFlag)
	{
		OspSemGive(m_hMemPoolSem);
		return TRUE;
	}
	//不大于2M的部分以2的倍数递增
	for (;dwCurrStackNums<OSP_MEMPOOL_GENERAL;dwCurrStackNums++)
	{
		m_apCOspStack[dwCurrStackNums] = new COspStack(dwBlockSize,(u32)this);
		if(NULL != m_apCOspStack[dwCurrStackNums])
		{
			m_apCOspStack[dwCurrStackNums]->m_dwStackAvailbleBlkPercentUpperLimit = m_tOspMemPoolPara.dwMaxFreePercent;
			m_apCOspStack[dwCurrStackNums]->StackCreate(0);
		}
		else
		{
			for (dwCurrStackNums = dwCurrStackNums -1;dwCurrStackNums >= 0;--dwCurrStackNums)
			{
				if(NULL != m_apCOspStack[dwCurrStackNums])
				{
					m_apCOspStack[dwCurrStackNums]->StackDestroy();
					delete m_apCOspStack[dwCurrStackNums];
					m_apCOspStack[dwCurrStackNums] = NULL;
				}
			}
			OspSemGive(m_hMemPoolSem);
			return FALSE;
		}
		//保证从此循环退出时dwBlockSize为2M
		if (dwCurrStackNums < STACK_2M)
		{
			dwBlockSize *=2;
		}
	}
	//大于2M的部分以m_tOspMemPoolPara.dwLargeBlockLevelGrowSize M递增
	if(m_tOspMemPoolPara.dwLargeBlockLevelGrowSize > 0)
	{
		for(;dwCurrStackNums < OSP_MEMPOOL_GENERAL + OSP_MEMPOOL_EXTERN;dwCurrStackNums++)
		{
			dwBlockSize +=1024*1024*m_tOspMemPoolPara.dwLargeBlockLevelGrowSize;
			m_apCOspStack[dwCurrStackNums] = new COspStack(dwBlockSize,(u32)this);
			if (NULL != m_apCOspStack[dwCurrStackNums])
			{
				m_apCOspStack[dwCurrStackNums]->m_dwStackAvailbleBlkPercentUpperLimit = m_tOspMemPoolPara.dwMaxFreePercent;
				m_apCOspStack[dwCurrStackNums]->StackCreate(0);
			}
			else
			{
				for (dwCurrStackNums = dwCurrStackNums -1;dwCurrStackNums >= 0;--dwCurrStackNums)
				{
					if(NULL != m_apCOspStack[dwCurrStackNums])
					{
						m_apCOspStack[dwCurrStackNums]->StackDestroy();
						delete m_apCOspStack[dwCurrStackNums];
						m_apCOspStack[dwCurrStackNums] = NULL;
					}
				}
				OspSemGive(m_hMemPoolSem);
				return FALSE;
			}
		}
	}
	m_bInitFlag = TRUE;
	OspSemGive(m_hMemPoolSem);
	return TRUE;
}

BOOL32 COspMemPool::IsOspMemPoolInit()
{
	return m_bInitFlag;
}
BOOL32 COspMemPool::OspMemPoolDestory()
{
	u32 dwCurrStackNums = 0;
	u32 dwUnDestroySuccess = 0;
	BOOL32 bRet = FALSE;
	OspSemTake(m_hMemPoolSem);
	if(TRUE == m_bDestoryFlag)
	{
		OspSemGive(m_hMemPoolSem);
		return TRUE;
	}
	for (;dwCurrStackNums < OSP_MEMPOOL_GENERAL + OSP_MEMPOOL_EXTERN;++dwCurrStackNums)
	{
		if(NULL != m_apCOspStack[dwCurrStackNums])
		{
			bRet = m_apCOspStack[dwCurrStackNums]->StackDestroy();
			if (bRet)
			{
				delete m_apCOspStack[dwCurrStackNums];
				m_apCOspStack[dwCurrStackNums] = NULL;
			}
			else
			{
				++dwUnDestroySuccess;
				printf("the stack %d destroy fail\n",dwCurrStackNums);
			}
		}
	}
	if (0 != dwUnDestroySuccess)
	{
		printf("there is still %d stack undestroy\n",dwUnDestroySuccess);
		OspSemGive(m_hMemPoolSem);
		return FALSE;
	}
	m_bInitFlag = FALSE;
	m_bDestoryFlag = TRUE;
	OspSemGive(m_hMemPoolSem);
	return TRUE;

}

void* COspMemPool::OspMemPoolAlloc(u32 dwSize)
{
	void* pAlloc = NULL;
	u32 n = 0;
	u32 dwCalacSize = 0;
	if (0 == dwSize)
	{
		printf("OspAlloc--error arg\n");
		return pAlloc;
	}
	dwCalacSize = dwSize -1;
	if (dwCalacSize < 2*1024*1024)
	{
		if (dwCalacSize < 512)
		{
			if (dwCalacSize <128)
			{
				if (dwCalacSize < 64)
				{
					pAlloc = m_apCOspStack[STACK_64B]->StackAlloc();
				}else{
					pAlloc = m_apCOspStack[STACK_128B]->StackAlloc();
				}
			}else{
				if (dwCalacSize < 256)
				{
					pAlloc = m_apCOspStack[STACK_256B]->StackAlloc();
				}else{
					pAlloc = m_apCOspStack[STACK_512B]->StackAlloc();
				}
			}
		}else{
			if (dwCalacSize < 128*1024)
			{
				if (dwCalacSize < 16*1024)
				{
					if (dwCalacSize < 4*1024)
					{
						if (dwCalacSize <2*1024)
						{
							if(dwCalacSize < 1*1024)
							{
								pAlloc = m_apCOspStack[STACK_1K]->StackAlloc();
							}else{
								pAlloc = m_apCOspStack[STACK_2K]->StackAlloc();
							}
						}else{
							pAlloc = m_apCOspStack[STACK_4K]->StackAlloc();
						}
					}else{
						if(dwCalacSize < 8*1024)
						{
							pAlloc = m_apCOspStack[STACK_8K]->StackAlloc();
						}else{
							pAlloc = m_apCOspStack[STACK_16K]->StackAlloc();
						}
					}
				}else{
					if (dwCalacSize < 64*1024)
					{
						if (dwCalacSize < 32*1024)
						{
							pAlloc = m_apCOspStack[STACK_32K]->StackAlloc();
						}else{
							pAlloc = m_apCOspStack[STACK_64K]->StackAlloc();
						}
					}else{
						pAlloc = m_apCOspStack[STACK_128K]->StackAlloc();
					}
				}
			}else{
				if (dwCalacSize < 512*1024)
				{
					if (dwCalacSize <256*1024)
					{
						pAlloc = m_apCOspStack[STACK_256K]->StackAlloc();
					}else{
						pAlloc = m_apCOspStack[STACK_512K]->StackAlloc();
					}
				}else{
					if (dwCalacSize < 1024*1024)
					{
						pAlloc = m_apCOspStack[STACK_1M]->StackAlloc();
					}else{
						pAlloc = m_apCOspStack[STACK_2M]->StackAlloc();
					}
				}
			}
		}
	}else{
		if(m_tOspMemPoolPara.dwLargeBlockLevelGrowSize > 0)
		{
			n = (dwCalacSize - 2*1024*1024)/(m_tOspMemPoolPara.dwLargeBlockLevelGrowSize * 1024*1024);
			if (n < OSP_MEMPOOL_EXTERN)
			{
				pAlloc = m_apCOspStack[STACK_2M+n+1]->StackAlloc();
			}else{
			    if(IsOspInitd())
			    {
			        OspPrintf(1,0,"OspAllocMem -- arg %u,is too large ,out of mem\n",dwSize);
			    }else{
				    printf("OspAllocMem -- arg %u is too large ,out of mem\n",dwSize);
				    }
			}
		}
	}
	return pAlloc;
}
void COspMemPool::OspMemPoolFree()
{
}
void COspMemPool::OspMemPooLState()
{
	u32 dwNum = 0;
	printf("\n=======================================================\n");
	for (;dwNum<OSP_MEMPOOL_GENERAL + OSP_MEMPOOL_EXTERN;++dwNum)
	{
		if (NULL !=m_apCOspStack[dwNum])
		{
			m_apCOspStack[dwNum]->StackStateShow();
		}
	}
	printf("\n=======================================================\n");
}
API BOOL32 OspMemPoolCreate(TOspMemPoolPara* ptMemPoolPara,INOUT OSP_MEMPOOLHANDLE *pOspMemPoolHandle)
{
	COspMemPool* pcMemPoolHanlde = NULL;
	BOOL32 bRet = FALSE;
	if (NULL == pOspMemPoolHandle)
	{
		printf("the arg is NULL\n");
		return FALSE;
	}
	pcMemPoolHanlde = new COspMemPool();
	if (NULL != pcMemPoolHanlde)
	{
		bRet = pcMemPoolHanlde->OspMemPoolInit(ptMemPoolPara);
		if(FALSE == bRet)
		{
			delete pcMemPoolHanlde;
			pcMemPoolHanlde = NULL;
		}
	}
	*pOspMemPoolHandle = pcMemPoolHanlde;
	return bRet;
}


API void *OspAllocMemEx(size_t size,OSP_MEMPOOLHANDLE OspMemPoolHanle)
{
	COspMemPool* pcSourcePool = NULL;
	if (NULL == OspMemPoolHanle)
	{
		return OspAllocMem(size);
	}
	pcSourcePool = (COspMemPool*)OspMemPoolHanle;
	return pcSourcePool->OspMemPoolAlloc(size);
}

API BOOL32 OspMemPoolDestroy(OSP_MEMPOOLHANDLE pOspMemPoolHandle)
{
	BOOL32 bRet = FALSE;
	if (NULL == pOspMemPoolHandle)
	{
		return FALSE;
	}
	bRet = ((COspMemPool*)pOspMemPoolHandle)->OspMemPoolDestory();
	if (TRUE == bRet)
	{
		delete (COspMemPool*)pOspMemPoolHandle;
		pOspMemPoolHandle = NULL;
	}
	return bRet;
}

API void OspMemPoolStateShow(OSP_MEMPOOLHANDLE pOspMemPoolHandle)
{
	if (NULL == pOspMemPoolHandle)
	{
		return;
	}
	((COspMemPool*)pOspMemPoolHandle)->OspMemPooLState();
	return;
}

/*====================================================================
函数名：IsOspInitd
功能：判断Osp是否已初始化
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：

返回值说明：Osp已经初始化返回TRUE；Osp还未初始化返回FALSE.
====================================================================*/
API BOOL32 IsOspInitd(void)
{
	BOOL32 bInitd = FALSE;

	OspSemTake(g_Osp.m_tMutexSema);
	bInitd = g_Osp.m_bInitd;
	OspSemGive(g_Osp.m_tMutexSema);

	return bInitd;
}

/*====================================================================
函数名：OspInit
功能：Osp模块的初始化函数，启动所有服务任务。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：bTelnetEnable: 是否启动作为Osp外壳的Telnet终端，
              uTelnetPort: Osp的Telnet服务器的侦听端口号。

返回值说明：成功返回TRUE；失败返回FALSE。
====================================================================*/
API BOOL32 OspInit(BOOL32 bTelnetEnable, u16 wTelnetPort, \
			const char* pchModuleName , u32 dwMaxNodeNum , u32 dwMaxDispatchMsg)
{
    u16 wPort;
	u32 dwTaskID = 0;
    TASKHANDLE hTask;
    SOCKHANDLE hLocalListenSock;
    SOCKHANDLE hLocalDispatchSock;
    SOCKHANDLE hLocalInSock;
	sockaddr_in tINAddr;
	BOOL32 bResult = FALSE;

    int uAddrLenIn = sizeof(tINAddr);

	OspSemTake(g_Osp.m_tMutexSema);
	if(g_Osp.m_bInitd)
	{
//		OspPrintf(TRUE, TRUE, "Osp: Osp had been initiated before.\n");
		OspLog(1, "Osp: Osp had been initiated before.\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	if( !g_Osp.m_cNodePool.Alloc(dwMaxNodeNum, dwMaxDispatchMsg) )
	{
		OspSemGive(g_Osp.m_tMutexSema);
		OspLog(1, "node mem alloc fail, node num[%d]\n", dwMaxNodeNum);
		printf("node mem alloc fail, node num[%d]\n", dwMaxNodeNum);
		return FALSE;
	}


    if( NULL == pchModuleName || 0 == *pchModuleName)
    {
//		strcpy( g_achModuleName , "osp" );
		g_achModuleName[0] = 0;
    }
    else if( strlen( pchModuleName ) <= MAX_MODULE_NAME_LENGTH )
    {
		strcpy( g_achModuleName , pchModuleName );
    }
    else
    {
		OspLog( 1, "Osp: osp module name too long . The Max length is %d\n" , MAX_MODULE_NAME_LENGTH );
		OspSemGive( g_Osp.m_tMutexSema );
		return FALSE;
    }

	g_Osp.m_bKillOsp = FALSE;

	/*初始化系统内存区和用户内存区*/
	/*分配原则：系统常用的内存堆栈区分配100块内存块，
	            用户内存堆栈区常用的（尺寸较小）分配100块内存块，
	            不常用的尺寸越大预分配内存块越少（减少系统占用内存）*/
	/*定长内存区*/
#if 0
	g_Osp.m_cTimerStack.CreateStack(0);
//	g_Osp.m_cMsgStack.CreateStack(100, OSP_MSG_STACK_MARKER);
//	g_Osp.m_cLogStack.CreateStack(100, OSP_LOG_STACK_MARKER);
	g_Osp.m_cInstTimeStack.CreateStack(0);

	//最小匹配内存区//
	g_Osp.m_c64BStack.CreateStack(0);
	g_Osp.m_c128BStack.CreateStack(0);
	g_Osp.m_c256BStack.CreateStack(0);
	g_Osp.m_c512BStack.CreateStack(0);
	g_Osp.m_c1KBStack.CreateStack(0);
	g_Osp.m_c2KBStack.CreateStack(0);
	g_Osp.m_c4KBStack.CreateStack(0);
	g_Osp.m_c8KBStack.CreateStack(0);
	g_Osp.m_c16KBStack.CreateStack(0);
	g_Osp.m_c32KBStack.CreateStack(0);
	g_Osp.m_c64KBStack.CreateStack(0);
	g_Osp.m_c128KBStack.CreateStack(0);
	g_Osp.m_c256KBStack.CreateStack(0);
	g_Osp.m_c512KBStack.CreateStack(0);
	g_Osp.m_c1MBStack.CreateStack(0);
	g_Osp.m_c2MBStack.CreateStack(0);
	g_Osp.m_c4MBStack.CreateStack(0);
#endif //if 0

	//创建定时器内存池//
	g_Osp.m_pcTimerStack = new COspStack(OSP_TIMER_STACK_BLK_SIZE,OSP_TIMER_STACK_MARKER);
	if(NULL == g_Osp.m_pcTimerStack)
	{
		printf("OspInit create timer pool failed\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}
	g_Osp.m_pcTimerStack->StackCreate(0);

	//创建instance定时器内存池//
	g_Osp.m_pcInstTimeStack = new COspStack(OSP_INST_TIME_STACK_BLK_SIZE,OSP_INST_TIME_STACK_MARKER);
	if(NULL == g_Osp.m_pcInstTimeStack)
	{
		g_Osp.m_pcTimerStack->StackDestroy();
		delete g_Osp.m_pcTimerStack;
		g_Osp.m_pcTimerStack = NULL;
	}
	//osp内部内存池初始化//
	bResult = g_Osp.m_cOspInerMemPool.OspMemPoolInit(NULL);
	if(!bResult)
	{
		g_Osp.m_pcInstTimeStack->StackDestroy();
		delete g_Osp.m_pcInstTimeStack;
		g_Osp.m_pcInstTimeStack = NULL;
		g_Osp.m_pcTimerStack->StackDestroy();
		delete g_Osp.m_pcTimerStack;
		g_Osp.m_pcTimerStack = NULL;
		printf("OspInit create memery pool faile\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}
	printf("[OspInit]init mem module success\n");
	OspSemBCreate(&g_tCpuInfoSem);
	OspSemBCreate(&g_tTickGet64Sem);

    /* 日志系统初始化 */
	if(!LogSysInit())
	{
		OspLog(1, "Osp: log system init failed.\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	printf("[OspInit]init log module success\n");
	OSP_LOG(MOD_OSP, OSP_CRITICAL_LEV, "init log module success\n");
    /* 结点间消息发送任务 */
	if(!DispatchSysInit())
	{
		OspLog(1, "Osp: log system init failed.\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	printf("[OspInit]init DispatchPool success\n");
	OSP_LOG(MOD_OSP, OSP_CRITICAL_LEV, "init DispatchPool success\n");
	/* 创建互连的两个内部套接字 */
	if( !SockInit() )
	{
		OspLog(1, "Osp: socket init failed\n");
        printf("Osp: socket init failed\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	hLocalInSock = socket(AF_INET, SOCK_STREAM,0);

	hLocalListenSock = INVALID_SOCKET;
    for(wPort=OSP_INER_PORT_BEGIN; wPort<OSP_INER_PORT_END; wPort++) // creat inter server sock
    {
        hLocalListenSock = CreateTcpNodeNoRegist(0, wPort); // server's port
        if(hLocalListenSock != INVALID_SOCKET)
        {
            OspLog(1, "internal listen sock %d\n", hLocalListenSock);
            printf("internal listen sock %d\n", hLocalListenSock);
            break;
        }
	}

	if(hLocalListenSock == INVALID_SOCKET)
	{
		OspLog(1, "Osp: create internal listen sock fail\n" );
        printf( "Osp: create internal listen sock fail\n");
		OspLog(1, "Osp: initial fail\n");
        printf("Osp: initial fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

    tINAddr.sin_family = AF_INET;
    tINAddr.sin_port = htons(wPort);
    tINAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if( SOCKET_ERROR == connect(hLocalInSock, (sockaddr *)&tINAddr, sizeof(tINAddr) ) )
	{
		OspLog(1, "Osp: connect to internal error\n");
		OspLog(1, "Osp: initial fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	hLocalDispatchSock = accept(hLocalListenSock, (sockaddr *)&tINAddr, &uAddrLenIn);

	g_Osp.m_cNodePool.m_tLocalInSock = hLocalInSock;
	g_Osp.m_cNodePool.m_tLocalOutSock = hLocalDispatchSock;

	SockClose(hLocalListenSock);

	/* 定时模块初始化 */
	if(!TimerSysInit())
	{
		OspLog(1, "Osp: create Timer task fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	printf("[OspInit]init timer success\n");
	OSP_LOG(MOD_OSP, OSP_CRITICAL_LEV, "init timer success\n");
	/* OSP终端初始化 */
	if(!OspTelInit(wTelnetPort))
	{
		printf("OspTelInit fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	hTask = OspTaskCreate(OspTeleDaemon, "OspTeleDaemon", OSP_TELEDAEMON_TASKPRI,
		                                OSP_TELEDAEMON_STACKSIZE, (KD_PTR)wTelnetPort, 0, &dwTaskID);
	if(hTask == NULL)
	{
		OspLog(1, "Osp: create task OspTeleDeamon fail\n");
		OspLog(1, "Osp: initial fail\n");
        printf ("Osp: create task OspTeleDeamon fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}
	g_Osp.AddTask(hTask, dwTaskID, "OspTeleDeamon");
	g_dwTeletTaskID = dwTaskID;

	if(bTelnetEnable)
	{
		OspTaskDelay(1000);
		OspShellStart();
	}

	/* 通信模块初始化 */
	hTask = OspTaskCreate(PostDaemon, "OspPostDaemon", OSP_POSTDAEMON_TASKPRI,
		                              OSP_POSTDAEMON_STACKSIZE, 0, 0, &dwTaskID);
	if(NULL==hTask)
	{
		OspLog(1, "Osp: create  PostDaemon task fail\n");
		OspLog(1, "Osp: initial fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}
	g_Osp.AddTask(hTask, dwTaskID, "OspPostDaemon");
	g_dwPostDaemonTaskId = dwTaskID;

	/* windows下增加必要的延时以使得随后的打印能输出到telnet上　*/
	if(bTelnetEnable) OspTaskDelay(1000);

	if( !g_Osp.m_cNodePool.Initialize() )
	{
		OspLog(1, "Osp: node pool initialize fail\n");
		OspLog(1, "Osp: initial fail\n");
		OspSemGive(g_Osp.m_tMutexSema);
		return FALSE;
	}

	/*初始化OSP事件描述*/
	g_Osp.m_cOspEventDesc.COspEventInit();

	/*初始化内存检查内容*/
	g_Osp.m_pdwCheckMemAddr = NULL;    //要检查的内存地址
	g_Osp.m_dwCheckValue = 0;      //要检查的值
	g_Osp.m_bEqual = FALSE;
	g_Osp.m_bErrArised = FALSE;
	memset(g_Osp.m_chBeforErrThread, 0, sizeof(g_Osp.m_chBeforErrThread));
	memset(g_Osp.m_chErrCurrentThread, 0, sizeof(g_Osp.m_chErrCurrentThread));

	printf("[OspInit]init finished\n");
	OSP_LOG(MOD_OSP, OSP_CRITICAL_LEV, "init finished\n");
	OspLog(1, "Osp: initial success!\n");
    g_Osp.m_bInitd = TRUE;
	OspSemGive(g_Osp.m_tMutexSema);
	return TRUE;
}

/*====================================================================
函数名：OspQuit
功能：退出Osp，杀死所有Osp服务任务和所有App，释放Osp用到的所有资源
算法实现：（可选项）
引用全局变量：sockClient: Telnet数据套接字;
              sockTelSer: Telnet侦听套接字;
			  g_Osp: 代表Osp的全局对象。
输入参数说明：无

返回值说明：无
====================================================================*/
extern SOCKHANDLE sockClient;
extern SOCKHANDLE sockTelSer;
extern u16 g_wportListtening;

API void OspQuit(void)
{
   u16 wAppID;
   CApp *pcApp;
   BOOL32 bResult = FALSE;

   OspSemTake(g_Osp.m_tMutexSema);

   /* 如果用户没有初始化，直接返回 */
   if( !g_Osp.m_bInitd )
   {
	   OspSemGive(g_Osp.m_tMutexSema);
	   return;
   }

   /* 输出退出信息 */
   printf("Osp:  User quit Osp, please wait...\n");

   /* 先让所有非调度任务自杀: Timer任务和ConnEcho任务是带
      超时的死循环任务，不需要特别通知；其余任务均有阻塞点，
      需要去阻塞，对于套接字有关任务，先关闭套接字，对于使用
      邮箱的任务，发送一条OSP_QUIT消息 */

   /* 设置OSP退出标志，使得PostDamon、TimerTask等任务退出、Telnet管理任务 */
   g_Osp.m_bKillOsp = TRUE;

   // 关闭内部socket，以使得PostDaemon任务的select返回，从而有机会自杀
   if(g_Osp.m_cNodePool.m_tLocalInSock != INVALID_SOCKET)
   {
	   SockClose(g_Osp.m_cNodePool.m_tLocalInSock);
	   g_Osp.m_cNodePool.m_tLocalInSock = INVALID_SOCKET;
   }

   /* 再让所有App自杀，方法是向所有调度任务发送一条OSP_QUIT消息 */
   for(wAppID=1; wAppID<=MAX_APP_NUM; wAppID++)
   {
	   pcApp = g_Osp.m_cAppPool.AppGet(wAppID);
	   if(pcApp != NULL)
	   {
		   ::OspPost(MAKEIID(wAppID), OSP_QUIT);
	   }
   }

   /* 再让所有分派任务自杀 */
   DispatchSysExit();

   /* 向LogTask发送一条长度为0的日志，请求它退出 */
   TOspLogHead tOspLogHead;
   memset(&tOspLogHead, 0, sizeof(TOspLogHead));
   tOspLogHead.type = LOG_TYPE_UNMASKABLE;
   tOspLogHead.bToScreen = TRUE;
   OspLogQueWrite(tOspLogHead, NULL, 0);

   /* OspShell 退出 */
   if(sockClient != INVALID_SOCKET)
   {
	   SockClose(sockClient);
	   sockClient = INVALID_SOCKET;
   }

   SockClose(sockTelSer);
   sockTelSer = INVALID_SOCKET;

   OspShellExit();

   /* 等待全部OSP任务退出，如超时强行杀死未自杀的任务 */

   // 如果在APP上下文中调用，先退出该APP中所有实例
   u32 selfTaskID = OspTaskSelfID();

   /* 从任务表中删除调用任务, 因为一个任务不能等待自己退出 */
   g_Osp.DelTask(selfTaskID);

   // MFC下使用有效的等待机制，对无法自杀的任务强行杀死
   TTaskInfo* ptCurTask = g_Osp.GetFirstTask();
   TTaskInfo* ptNextTask = NULL;

   while( ptCurTask != NULL )
   {
	   ptNextTask = g_Osp.GetNextTask(ptCurTask->id);
//	   OspTaskSetPriority(ptCurTask->handle, 60);
	   if(WAIT_OBJECT_0 != WaitForSingleObject(ptCurTask->handle, 2000))
	   {
		   OspLog(1, "Osp: Wait task %s exit failed, terminate it.\n", ptCurTask->name);
		   OspTaskTerminate(ptCurTask->handle);
	   }
	   CloseHandle(ptCurTask->handle);
	   g_Osp.DelTask(ptCurTask->id);
	   ptCurTask = ptNextTask;
   }

   SockCleanup();
	OspSemDelete(g_tTickGet64Sem);
	OspSemDelete(g_tCpuInfoSem);
   g_Osp.m_cOspAppDesc.Destroy();
   g_Osp.m_cOspEventDesc.Destroy();

   /*释放内存区*/
   bResult = g_Osp.m_pcTimerStack->StackDestroy();
   if(!bResult)
   {
	   printf("osp timerpool destroy failed ,still continue\n");
   }
   delete g_Osp.m_pcTimerStack;
   g_Osp.m_pcTimerStack = NULL;
   bResult = g_Osp.m_pcInstTimeStack->StackDestroy();
   if (!bResult)
   {
	   printf("osp InstTimerpool destroy failed ,still continue\n");
   }
   bResult = g_Osp.m_cOspInerMemPool.OspMemPoolDestory();
   if(!bResult)
   {
	   printf("osp memery pool destroy failed ,still continue\n");
   }
#if 0
   g_Osp.m_cTimerStack.DestroyStack();
//   g_Osp.m_cMsgStack.DestroyStack();
//   g_Osp.m_cLogStack.DestroyStack();
   g_Osp.m_cInstTimeStack.DestroyStack();

   g_Osp.m_c64BStack.DestroyStack();
   g_Osp.m_c128BStack.DestroyStack();
   g_Osp.m_c256BStack.DestroyStack();
   g_Osp.m_c512BStack.DestroyStack();
   g_Osp.m_c1KBStack.DestroyStack();
   g_Osp.m_c2KBStack.DestroyStack();
   g_Osp.m_c4KBStack.DestroyStack();
   g_Osp.m_c8KBStack.DestroyStack();
   g_Osp.m_c16KBStack.DestroyStack();
   g_Osp.m_c32KBStack.DestroyStack();
   g_Osp.m_c64KBStack.DestroyStack();
   g_Osp.m_c128KBStack.DestroyStack();
   g_Osp.m_c256KBStack.DestroyStack();
   g_Osp.m_c512KBStack.DestroyStack();
   g_Osp.m_c1MBStack.DestroyStack();
   g_Osp.m_c2MBStack.DestroyStack();
   g_Osp.m_c4MBStack.DestroyStack();
#endif //if 0
   printf("Osp: Osp quit success!\n");
   g_Osp.m_bInitd = FALSE;
   OspSemGive(g_Osp.m_tMutexSema);
}

/*====================================================================
函数名：OspTelAuthor
功能：设置Telnet的授权用户名和密码
算法实现：（可选项）
引用全局变量：
输入参数说明：
返回值说明：
====================================================================*/
API void OspTelAuthor(const char * szUsername ,const char * szPassword)
{
    if(szUsername != NULL)
    {
        u32 dwUsernameLen = strlen(szUsername);
        if(dwUsernameLen >= AUTHORIZATION_NAME_SIZE)
        {
            OspPrintf(TRUE,FALSE,"Osp: telnet username is too long!\n");
            return;
        }
    }
    if(szPassword != NULL)
    {
        u32 dwPasswordLen = strlen(szPassword);
        if(dwPasswordLen >= AUTHORIZATION_NAME_SIZE)
        {
            OspPrintf(TRUE,FALSE,"Osp: telnet password is too long!\n");
            return;
        }
    }

    if(szUsername == NULL)
    {
        strcpy(g_TelnetUsername,"");
    }
    else
    {
        strcpy(g_TelnetUsername,szUsername);
    }
    if(szPassword == NULL)
    {
        strcpy(g_TelnetPasswd,"");
    }
    else
    {
        strcpy(g_TelnetPasswd,szPassword);
    }

    return;
}

/*=============================================================================
函数名      :OspAppInstanceEntry
功能        :调用业务的函数 并记录执行前后的时刻和状态信息
             如果dwEndTick为0, 说明函数阻塞了
             注意该函数只在AppEntry()中使用，没有锁保护
算法实现    :
参数说明    :
	[I] CApp *pcApp
	[I] CInstance *pIns
	[I] CMessage *pMsg
返回值说明  :无
-------------------------------------------------------------------------------
修改记录    :
日  期      版本        修改人        修改内容
2015/05/20  1.0         邓昌葛        新建
===============================================================================*/
void OspAppInstanceEntry(CApp *pcApp, CInstance *pcIns,
								CMessage *pMsg, u8 chMethod)
{
	if ((NULL == pcApp) || (NULL == pcIns) || (NULL == pMsg))
	{
		return;
	}

	u32 dwState = pcIns->CurState();

	//记录开关为FALSE，则不需要记录
	if(FALSE==pcApp->GetCallBackInfoRecordFlag())
	{
		pcApp->InstInfoAdd(pcIns->GetInsID(), dwState,
		                   pMsg->event, pcApp->curEvtSnd);

		if (CALL_DAEMON_INSTANCE_ENTRY == chMethod)
		{
			pcIns->DaemonInstanceEntry(pMsg, pcApp);
		}
		else
		{
			pcIns->InstanceEntry(pMsg);
		}
	}
	else
	{//记录开关打开，记录回调函数执行前后的时刻

		pcApp->InstInfoAdd(pcIns->GetInsID(), dwState,
						   pMsg->event, pcApp->curEvtSnd);

		if (CALL_DAEMON_INSTANCE_ENTRY == chMethod)
		{
			pcApp->CallBackInfoAddStart(OspTickGet(), pcIns->m_instId, pMsg->event,
									pMsg->type, "DaemonInstanceEntry");
			pcIns->DaemonInstanceEntry(pMsg, pcApp);
		}
		else
		{
			pcApp->CallBackInfoAddStart(OspTickGet(), pcIns->m_instId, pMsg->event,
									pMsg->type, "InstanceEntry");
			pcIns->InstanceEntry(pMsg);
		}

		pcApp->CallBackInfoAddEnd(OspTickGet());
	}
}

/*====================================================================
函数名：OspAppEntry
功能：App的任务入口，接收消息并发给各个Instance
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：pcApp: 获得CPU的当前App指针.

返回值说明：
====================================================================*/
API void OspAppEntry(CApp *pcApp)
{
    u16 instId;
	u32 state;
    CInstance *pIns;
    u16 instCount;
    u32 lenrcv;
    u32 instMax = pcApp->GetInstanceNumber();
	char achBuf[MAX_LOG_MSG_LEN];
	CMessage *pMsg = NULL;
	TOsMsgStruc osMsg;
	int len;
    u32 dwUncompressedMsgLen = 0;

	while(TRUE)
	{
        osMsg.address = 0;//初始化以防OspRcvMsg出错，便于后续访问下校验
  		/* 增加同步消息机制后，先从备份队列取消息处理直到空为止 */
		if(pcApp->GetBakMsgNum() > 0)
		{
			OspRcvMsg(pcApp->bakQueRcvId, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &lenrcv);
    		pcApp->BakMsgNumDec();
		}
		else
		{
			if ( !OspRcvMsg(pcApp->queRcvId, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &lenrcv) )
			{
				OspPrintf(1, 0, "osprcvmsg fail, quit app[%d]\n", pcApp->appId);
				return ;
			}

			pcApp->MsgProcessedNumInc();
		}

		if(NULL == osMsg.address)
		{
			OspLog(1, "Osp: OspAppEntry OspRcvMsg fail, addr=%d.\n", osMsg.address );
			continue;
		}

		pMsg = (CMessage *)osMsg.address;

        //<--消息压缩by wubin 2011-02-22
        if (OSP_COMPRESS_MSG == pMsg->event)
        {
            dwUncompressedMsgLen = 0;
            if (FALSE == OspUnCompressMessagePack(&pMsg, &dwUncompressedMsgLen))
            {
                OspLog(1, "Osp: Message uncompress failed.\n \tsrcnode(%lu) dstnode(%lu) dstid(%lu) srcid(%lu) type(%lu) event(%lu) length(%lu)\n" ,
                    pMsg->srcnode , pMsg->dstnode , pMsg->dstid , pMsg->srcid , pMsg->type , pMsg->event , pMsg->length );
                continue; //如果解压出现错误， 丢弃该包。
            }
        }
        //by wubin 2011-02-22-->
		pcApp->curMsgRcv = pMsg;

		if(pMsg->length <= 0)
		{
			if(pMsg->content != NULL)
			{
				continue;
			}
		}

		/* 收到来自OspQuit()的退出消息 */
        if(pMsg->event == OSP_QUIT)
		{
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			pcApp->QuitApp(); // 退出本App: 退出所有实例，归还占用的系统资源
			OspTaskExit(); // 退出任务
		}

		/* 找到合适的Instance，调用InstanceEntry */
		BOOL32 bScrnTrc= (pcApp->scrnTraceFlag & TRCEVENT) ? TRUE:FALSE;
        BOOL32 bFileTrc= (pcApp->fileTraceFlag & TRCEVENT) ? TRUE:FALSE;

		instId = GETINS(pMsg->dstid);
		if(instId == CInstance::INVALID)
		{
			pIns = pcApp->FindInstByAlias(pMsg->dstAlias, pMsg->dstAliasLen);
            if(pIns != NULL)  // 找到了同名实例
			{
				pMsg->dstid = MAKEIID(pIns->GetAppID(), pIns->GetInsID());  /* 更正实例ID */
				instId = GETINS(pMsg->dstid);  // 校正实例ID
			}
			else
			{
				if(bScrnTrc || bFileTrc)
				{
					len = sprintf(achBuf, "alias message recved but instance not found, app: %s = %d\n", pcApp->pAppName, pcApp->appId);
					len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pMsg);
					OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
				}
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				continue;
			}
		}

		/*超时消息，清除实例定时器信息块*/
	/*	if(pMsg->type == MSG_TYPE_TIMEOUT)
		{
			u16  timerId = pMsg->event;
			CInstance *pIns = pcApp->GetInstance(instId);
			if(pIns != NULL)
			{
				pIns->DelInstTimerInfo(timerId);
			}
		}
*/ //在timer模块中清除，保持一致

		/* 在正常处理中，同步应答是非法消息，应丢弃 */
		if(pMsg->type == MSG_TYPE_SYNCACK || pMsg->type == MSG_TYPE_GSYNCACK)
		{
			if(bScrnTrc || bFileTrc)
			{
				len = sprintf(achBuf, "Osp: unexpected syncack message recved in app: %s = %d\n", pcApp->pAppName, pcApp->appId);
				len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pMsg);
				OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
			}
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			continue;
		}

		if(bScrnTrc || bFileTrc)
		{
			len = sprintf(achBuf, "message recved app: %s = %d\n", pcApp->pAppName, pcApp->appId);
			len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, pMsg);
			OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
		}

		u32 before, after, tmp;
		before = OspTickGet();

        switch(instId)
        {
        case CInstance::DAEMON:   // send to daemon instance.
 			pIns = pcApp->GetInstance(CInstance::DAEMON);
			if(pIns != NULL)
			{
				OspAppInstanceEntry(pcApp, pIns,
				                    pMsg, CALL_DAEMON_INSTANCE_ENTRY);
			}
            break;

        case CInstance::PENDING: // look for idle instance, if no idle instance can found, than return a overflow message to sender
            pcApp->wLastIdleInstID %= pcApp->GetInstanceNumber();//因为m_uLastIdleInstID没有被初始化，我们不能保证他的有效性。
            instCount = pcApp->wLastIdleInstID;
            do {
                instCount++;//0是无效ID
                pIns = pcApp->GetInstance(instCount);
                if( pIns->CurState() == 0 )
				{
					OspAppInstanceEntry(pcApp, pIns,
					                    pMsg, CALL_INSTANCE_ENTRY);
                    break;
                }
                instCount %= pcApp->GetInstanceNumber();
            }while( instCount != pcApp->wLastIdleInstID );
            if( instCount == pcApp->wLastIdleInstID ) //找了一圈，没有IDLE实例？
                OspPost(pMsg->srcid, OSP_OVERFLOW/*overflow event*/ , pMsg->content,pMsg->length,pMsg->srcnode,0,0);
            pcApp->wLastIdleInstID = instCount;
            break;

        case CInstance::EACH:   // send to every instance, don't send to idle instance.
            for(instCount=1; instCount<=instMax; instCount++)
            {
                pIns = pcApp->GetInstance(instCount);
                if(pIns != NULL && pIns->CurState() != 0)
                {
					OspAppInstanceEntry(pcApp, pIns,
										pMsg, CALL_INSTANCE_ENTRY);
                }
            }
            break;

        case CInstance::EACH_ACK:    // send to every instance, don't send to idle instance. and send an ack
            for(instCount=1; instCount<=instMax; instCount++)
            {
                pIns = pcApp->GetInstance(instCount);
                if(pIns != NULL && pIns->CurState() != 0)
                {
					OspAppInstanceEntry(pcApp, pIns,
										pMsg, CALL_INSTANCE_ENTRY);
                }
            }
            OspPost(pMsg->srcid, OSP_BROADCASTACK, pMsg->content, pMsg->length, pMsg->srcnode, pMsg->dstid, pMsg->dstnode);
            break;

		case CInstance::CONN_ACK:
			OspPost(MAKEIID(pcApp->appId,0,0), OSP_APPCONN_ACK);
			break;

		default:  // send to the spcified instance and run
            if(instId > instMax)
            {
                OspLog(1, "Osp: illegal instance appid %d instid %d instmax %d\n",
					pcApp->appId, instId, instMax );
				OspLog(1, "pMsg srcnode(%d) dstnode(%d) dstid(%d) srcid(%d) type(%d) event(%d) length(%d)\n" ,
					pMsg->srcnode , pMsg->dstnode , pMsg->dstid , pMsg->srcid , pMsg->type , pMsg->event , pMsg->length );
                break;
            }
            pIns = pcApp->GetInstance(instId);
			if(pIns != NULL)
			{
				OspAppInstanceEntry(pcApp, pIns,
									pMsg, CALL_INSTANCE_ENTRY);
			}
        }

		/* 对实例同步消息返回应答 */
		if(pMsg->type == MSG_TYPE_SYNC)
		{
			OspPostMsg(pMsg->srcid, OSPEVENT_SYNCACK_MSG, pMsg->output, pMsg->outlen,
				pMsg->srcnode, pMsg->dstid, pMsg->dstnode, FALSE, MSG_TYPE_SYNCACK);


			if( (pMsg->output != NULL) && (pMsg->outlen > 0))
			{
				OspFreeMem(pMsg->output);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg->output) ;   //11-10
			}
		}

		/* 对全局同步消息, 如果是本地的立即give信号量, 否则, 返回一条全局同步应答消息 */
		if(pMsg->type == MSG_TYPE_GSYNC)
		{
			if(pMsg->srcnode == 0 /* && !g_Osp.m_bSyncAckExpired */)
			{
				memcpy(g_Osp.m_achSyncAck, pMsg->output, pMsg->outlen);
				g_Osp.m_wSyncAckLen = pMsg->outlen;
				OspSemGive(g_Osp.m_tSyncSema);
			}
			else
			{
				OspPostMsg(pMsg->srcid, OSPEVENT_SYNCACK_MSG, pMsg->output, pMsg->outlen,
					pMsg->srcnode, pMsg->dstid, pMsg->dstnode, FALSE, MSG_TYPE_GSYNCACK);
			}

			if( (pMsg->output != NULL) && (pMsg->outlen > 0))
			{
				OspFreeMem(pMsg->output);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg->output) ;   //11-10
			}
		}
		after = OspTickGet();
		tmp = after - before;
		if (tmp > max_inst_entry_interval)
		{
			max_inst_entry_interval= tmp;
		}

		OspFreeMem(pMsg);
//		g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
    } // while(TRUE)
}


API void OspAppEchoTest(u16 AppId)
{
	if(AppId==0 || AppId>MAX_APP_NUM)
		return;

	OspPost(MAKEIID(AppId, CInstance::CONN_ACK, 0), 0, 0, 0, 0);
}
// 类CApp的实现
#if defined(_LINUX_) && defined(PWLIB_SUPPORT)
CApp::CApp() : PThread(200<<10)
#else
CApp::CApp()
#endif
{
	msgIncome = 0;
	msgProcessed = 0;
	timerProcessed = 0;
	maxMsgWaiting = 0;
	msgdropped = 0;
	msgWaitingTop = 0;
	pAppName = NULL;
	queSendId = 0;
	queRcvId = 0;
	bakQueMsgNum = 0;
	bakQueSendId = 0;
	bakQueRcvId = 0;
	taskID = 0;
	scrnTraceFlag = 0;
	fileTraceFlag = 0;
	scrnLogFlag = 0;
	fileLogFlag = 0;
	appId = INVALID_APP;
	byAppPri = 100;
	wLastIdleInstID = 0;
	hAppTask = 0;
#ifndef _LINUX_
	tSemMutex = NULL;
#endif
	byInstInfoHead = 0;
	byInstInfoTail = 0;
    m_byCallBackIndex = 0;
    m_dwCallBackCount = 0;
    m_bCallBackInfoRecFlag = TRUE;
    memset(m_atCallBackInfo, 0, sizeof(TCallBackInfo)*APP_MAX_CALLBACK_RECORD);
    for(u32 i=0; i< APP_MAX_CALLBACK_RECORD; i++)
    {
        memset(m_atCallBackInfo[i].achFunctionName, 0, sizeof(char)*APP_MAX_FUNCTION_NAME);
    }
    curMsgRcv = NULL;
	curEvtSnd = 0;
    }

CApp::~CApp()
{
	scrnTraceFlag = 0;
	fileTraceFlag = 0;
	scrnLogFlag = 0;
	fileLogFlag = 0;
}
// 类CApp的实现
/*====================================================================
函数名：CApp::BakMsgNumInc
功能：App备用邮箱消息数加1
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
BOOL32 CApp::BakMsgNumInc(void)
{
	return ( bakQueMsgNum++ < maxMsgWaiting );
}

/*====================================================================
函数名：CApp::BakMsgNumDec
功能：App备用邮箱消息数减1
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
BOOL32 CApp::BakMsgNumDec(void)
{
	if(bakQueMsgNum > 0)
	{
		bakQueMsgNum--;
		return TRUE;
	}
	return FALSE;
}

/*====================================================================
函数名：CApp::GetBakMsgNum
功能：得到App备用邮箱中的消息数
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
u32 CApp::GetBakMsgNum(void)
{
	return bakQueMsgNum;
}

/*====================================================================
函数名：CApp::GetMsgWaitingNum
功能：得到App邮箱中待处理的消息数
算法实现：
引用全局变量：无
输入参数说明：无

返回值说明：App邮箱中待处理的消息数
====================================================================*/
u32 CApp::GetMsgWaitingNum()
{

	u32 dwMsgWaiting = 0;

	OspSemTake(tSemMutex);
        if (msgIncome > msgProcessed)
        {
            dwMsgWaiting = msgIncome-msgProcessed;
        }
	OspSemGive(tSemMutex);

	return dwMsgWaiting;
}

/*====================================================================
函数名：CApp::GetMsgIncomeNum
功能：得到外部往App邮箱中发送消息的计数
算法实现：
引用全局变量：无
输入参数说明：无

返回值说明：无
====================================================================*/
u32 CApp::GetMsgIncomeNum()
{
	u32 dwTemp = 0;

	OspSemTake(tSemMutex);
    dwTemp = msgIncome;
	OspSemGive(tSemMutex);

	return dwTemp;
}

/*====================================================================
函数名：CApp::MsgIncomeNumInc
功能：消息等待计数.
算法实现：
引用全局变量：无
输入参数说明：无

返回值说明：无
====================================================================*/
void CApp::MsgIncomeNumInc()
{
	OspSemTake(tSemMutex);
    msgIncome++;
    if ((msgIncome > msgProcessed) &&
            ((msgIncome - msgProcessed) > msgWaitingTop))
    {
        msgWaitingTop = msgIncome - msgProcessed;
		if (msgWaitingTop > (u32)g_max_msg_waiting)
		{
			g_max_msg_waiting = msgWaitingTop;
		}
    }
	OspSemGive(tSemMutex);
}

void CApp::MsgIncomeNumDec()
{
	OspSemTake(tSemMutex);
        if (msgIncome > 0) msgIncome--;
	OspSemGive(tSemMutex);
}

void CApp::MsgProcessedNumInc()
{
	OspSemTake(tSemMutex);
	msgProcessed++;
	OspSemGive(tSemMutex);
}

/*====================================================================
函数名：CApp::SetName
功能：设置App名字
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
void CApp::SetName( const char * pName )
{
	if( pName == NULL )
	{
		return;
	}

	if( pAppName != NULL )
	{
		OspFreeMem( pAppName );
	}

	pAppName = (char*)OspAllocMem( strlen(pName)+1 );
	strcpy( pAppName, pName );
}

/*====================================================================
函数名：CApp::NameGet
功能：取得App名字
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
char *CApp::NameGet(void)
{
	return pAppName;
}

/*====================================================================
函数名：CApp::TimerProcessedIncrease
功能：App定时器处理计数加1
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
void CApp::TimerProcessedIncrease(void)
{
	timerProcessed++;
}

/*====================================================================
函数名：CApp::LogLevelSet
功能：设置App日志控制级别
算法实现：（可选项）
引用全局变量：
输入参数说明：file_level: App文件日志级别, 输出级别低于该级别的实例日志
              将不会输出到日志文件中,
              screen_level: App屏幕日志级别, 输出级别低于该级别的实例日志
              将不会输出到屏幕上.

返回值说明：
====================================================================*/
void CApp::LogLevelSet(u8 file_level, u8 screen_level)
{
	scrnLogFlag = screen_level;// trace flag
	fileLogFlag = file_level;// file trace flag
}

/*====================================================================
函数名：CApp::LogLevelSet
功能：设置App跟踪控制标志:
      TRCEVENT -- 跟踪消息的发送和接收,
	  TRCTIMER -- 跟踪定时器的设置和杀死,
	  TRCSTATE -- 跟踪状态的跃迁.
算法实现：（可选项）
引用全局变量：
输入参数说明：file_flag: 文件跟踪标志,
              screen_flag: 屏幕跟踪标志.

返回值说明：
====================================================================*/
void CApp::TrcFlagSet(u16 file_flag, u16 screen_flag)
{
	scrnTraceFlag = screen_flag;// trace flag
	fileTraceFlag = file_flag;// file trace flag
}

/*====================================================================
函数名：CApp::InstInfoAdd
功能：记录实例信息, 状态和事件
算法实现：（可选项）
引用全局变量：
输入参数说明：ins: 实例号,
              state: 状态,
			  evtrcv: 接收的消息号,
			  evtsnd: 发送的消息号.

返回值说明：
====================================================================*/
void CApp::InstInfoAdd(u16 ins, u32 state, u16 evtrcv, u16 evtsnd)
{
	OspSemTake(tSemMutex);

	/* 新的实例信息放到队尾 */
	tInstInfo[byInstInfoTail].insid = ins;
	tInstInfo[byInstInfoTail].state = state;
    tInstInfo[byInstInfoTail].evtrcv = evtrcv;
	tInstInfo[byInstInfoTail].evtsnd = evtsnd;

	/* 队尾指针加1 */
	if(++byInstInfoTail >= MAX_INSTINFO_NUM)
	{
		byInstInfoTail = 0;
	}

	/* 遇队满, 删除队头 */
	if(byInstInfoTail == byInstInfoHead)
	{
		if(++byInstInfoHead >= MAX_INSTINFO_NUM)
		{
			byInstInfoHead = 0;
		}
	}

	OspSemGive(tSemMutex);
}
BOOL32 CApp::GetCallBackInfoRecordFlag(void)
{
	return m_bCallBackInfoRecFlag;
}
void CApp::SetCallBackInfoRecordFlag(BOOL32 bFlag)
{
	m_bCallBackInfoRecFlag = bFlag;
}
/*=============================================================================
函数名      :CallBackInfoAddStart
功能        :统计回调函数的信息，记录起始时间
             只在AppEntry()中使用，是串行的，不需要锁
             在执行回调函数之前调用
算法实现    : 不加锁的原因是，AppEntry()内部时序上可以保证串行
              Telnet显示线程和AppEntry()可能有冲突，但可以忽略
参数说明    :
	[I] dwTick 回调函数执行前的时刻
	[I] wInstId 执行回调的实例id
	[I] dwEvent 事件id
	[I] wMsgType 消息类型 MSG_TYPE_ASYNC 0 MSG_TYPE_SYNC 1
	    MSG_TYPE_SYNCACK 2  MSG_TYPE_GSYNC 3
	[I] pchFuncName 函数名指针
返回值说明  :无
-------------------------------------------------------------------------------
修改记录    :
日  期      版本        修改人        修改内容
2015/05/20  1.0         邓昌葛        新建
===============================================================================*/
void CApp::CallBackInfoAddStart(u32 dwTick, u16 wInstId, u16 wEvent,
 							    u16 wMsgType, char *pchFuncName)
{
	m_dwCallBackCount ++;

	m_atCallBackInfo[m_byCallBackIndex].dwIndex = m_dwCallBackCount;
	m_atCallBackInfo[m_byCallBackIndex].wInstId = wInstId;
	m_atCallBackInfo[m_byCallBackIndex].wEvent 	= wEvent;
	m_atCallBackInfo[m_byCallBackIndex].wMsgType 	= wMsgType;
	m_atCallBackInfo[m_byCallBackIndex].dwStartTick = dwTick;
	m_atCallBackInfo[m_byCallBackIndex].dwEndTick 	= 0;

    sprintf(m_atCallBackInfo[m_byCallBackIndex].achFunctionName,
    		"%s", pchFuncName);
    m_atCallBackInfo[m_byCallBackIndex].achFunctionName[APP_MAX_FUNCTION_NAME - 1] = '\0';
}
/*=============================================================================
函数名      :CallBackInfoAddEnd
功能        :统计回调函数的信息，记录截止时间
             只在AppEntry()中使用，注意时序上保证和CallBackInfoAddStart一一对应
             在执行回调函数之后调用
算法实现    :
参数说明    :
	[I] dwTick 回调函数执行完成的时刻
返回值说明  :无
-------------------------------------------------------------------------------
修改记录    :
日  期      版本        修改人        修改内容
2015/05/20  1.0         邓昌葛        新建
===============================================================================*/
void CApp::CallBackInfoAddEnd(u32 dwTick)
{
	m_atCallBackInfo[m_byCallBackIndex].dwEndTick = dwTick;

	if(++m_byCallBackIndex >= APP_MAX_CALLBACK_RECORD)
	{
		m_byCallBackIndex = 0;
	}
}
//显示App中涉及的回调函数执行历史记录
void CApp::CallBackInfoShow(void)
{
	u8 chIndex;

	//开关未开启，不打印任何信息
	if(FALSE == GetCallBackInfoRecordFlag())
	{
		return;
	}

	for (chIndex = 0; chIndex < APP_MAX_CALLBACK_RECORD; chIndex++)
	{
		if(chIndex == 0)
		{
			OspPrintf(TRUE, FALSE, "\nprint callback history of this app:\n");
			OspPrintf(TRUE, FALSE, "------------------------------------------\n");
		}

		OspPrintf(TRUE, FALSE, "index=%d,instid=%d, "\
				  			   "start=%d,end=%d,event=%d,msgtype=%d,function=%s\n",
			m_atCallBackInfo[chIndex].dwIndex, m_atCallBackInfo[chIndex].wInstId,
			m_atCallBackInfo[chIndex].dwStartTick, m_atCallBackInfo[chIndex].dwEndTick,
			m_atCallBackInfo[chIndex].wEvent, m_atCallBackInfo[chIndex].wMsgType,
			m_atCallBackInfo[chIndex].achFunctionName);
	}
}
/*====================================================================
函数名：CApp::InstInfoShow
功能：显示最近MAX_INSTINFO_NUM条实例信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
void CApp::InstInfoShow(void)
{
	u8 cur;
	u32 dwLineCount = 0;

	OspSemTake(tSemMutex);

	cur = byInstInfoHead;
    while(cur != byInstInfoTail)
	{
		if(cur == byInstInfoHead)
		{
			OspPrintf(TRUE, FALSE, "\nNow print the latest history of this app:\n");
			OspPrintf(TRUE, FALSE, "-------------------------------------------\n");
		}

		OspPrintf( TRUE , FALSE , "ins = %d , state = %d , event recv = %d event send = %d\n" ,
			tInstInfo[cur].insid , tInstInfo[cur].state , tInstInfo[cur].evtrcv , tInstInfo[cur].evtsnd );
		dwLineCount++;
		if( dwLineCount > 256 )
		{
			OspTaskDelay( 256 );
			dwLineCount = 0;
		}

		if(++cur >= MAX_INSTINFO_NUM)
		{
			cur = 0;
		}
	}

	OspSemGive(tSemMutex);
}

/*====================================================================
函数名：CApp::CreateApp
功能：激活一个App, 此后该App中的实例才能接收消息
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：szName: App名,
              wAid: 用户指定的App号,
			  uPri: App承载任务的优先级,
			  uQueueSize: App邮箱的消息条数,
              uStackSize: App承载任务的堆栈大小(未用)

返回值说明：  成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int CApp::CreateApp(const char * szName, u16 wAid, u8 byPri, u16 wQueueSize, u32 wStackSize)
{
	u16 i;
    CInstance *pIns;
	int insNum = GetInstanceNumber();

	/* 入口参数检查 */
	if(wAid==0 || wAid>MAX_APP_NUM)
	{
		OspLog(1, "Osp: illegal appId=%d.\n", wAid);
		return OSP_ERROR;
	}

	if(g_Osp.m_cAppPool.AppGet(wAid) != NULL)
    {
		OspLog(1, "Osp: app%d create twice, maybe cause to unexpected issue.\n", wAid);
//		return OSP_ERROR;
    }

	if(byPri < APP_TASKPRI_LIMIT) // 限制APP的最高优先级为APP_TASKPRI_LIMIT
	{
		byPri = APP_TASKPRI_LIMIT;
	}

	byAppPri = byPri;

	/* 设置App名 */
	if(szName != NULL)
	{
		pAppName = (char*)OspAllocMem(strlen(szName)+1);
		strcpy(pAppName, szName);
	}

    /* 初始化App有关的一些统计数据 */
	/*初始化实例别名记录信息*/
	InitAliasArray();

	msgProcessed = 0;
	msgIncome = 0;
    maxMsgWaiting = wQueueSize;
    msgdropped = 0;
    timerProcessed = 0; // 已经处理的定时数
    appId = wAid;   // 记录本App的App号
	bakQueMsgNum = 0; // 备用邮箱中等待处理的消息数
	scrnTraceFlag = 0; // 屏幕跟踪标志
    fileTraceFlag = 0; // 文件跟踪标志
    scrnLogFlag = 0; // 屏幕日志控制标志
    fileLogFlag = 0; // 文件日志控制标志
	byInstInfoHead = 0; // app历史表头
	byInstInfoTail = 0; // app历史表尾

	/* Daemon实例初始化 */
	pIns = GetInstance(CInstance::DAEMON);
	if(pIns != NULL)
	{
		pIns->m_instId = CInstance::DAEMON;
		pIns->m_appId = wAid;
		pIns->m_curState = 0;
		pIns->m_maxAliasLen = GetMaxAliasLen();
		pIns->m_aliasLen = 0;
		if(pIns->m_maxAliasLen > 0)
		{
			memset(pIns->m_alias, 0, MAX_ALIAS_LEN);
		}
		OspSemBCreate(&pIns->m_tSemTimerList);
		pIns->m_timerInfoListHead.timerId = 0 ;
		pIns->m_timerInfoListHead.timerBlkAdrr = NULL ;
		pIns->m_timerInfoListHead.next = NULL ;
		pIns->m_chInstStateIndex = 0;
		pIns->m_dwInstStateTimes = 0;
        memset(&(pIns->m_atInstStateInfo), 0, sizeof(TInstStateChangeInfo)*MAX_INST_STATE_RECORD_COUNT);

	}
	else
	{
		OspLog(1, "Osp: CInstance::DAEMON is NULL\n");
	}


	/* 普通实例初始化 */
    for(i=1; i<=insNum; i++) //设置 APPID,InstanceID
    {
        pIns=GetInstance(i);
		if(pIns != NULL)
		{
			pIns->m_instId = i;
			pIns->m_appId = wAid;
			pIns->m_curState = 0;
			pIns->m_maxAliasLen = GetMaxAliasLen();
			pIns->m_aliasLen = 0;
			if(pIns->m_maxAliasLen > 0)
			{
				memset(pIns->m_alias, 0, MAX_ALIAS_LEN);
			}
			OspSemBCreate(&pIns->m_tSemTimerList);
			pIns->m_timerInfoListHead.timerId = 0 ;
			pIns->m_timerInfoListHead.timerBlkAdrr = NULL ;
			pIns->m_timerInfoListHead.next = NULL ;
			pIns->m_chInstStateIndex = 0;
			pIns->m_dwInstStateTimes = 0;
        	memset(&(pIns->m_atInstStateInfo), 0, sizeof(TInstStateChangeInfo)*MAX_INST_STATE_RECORD_COUNT);
		}
		else
		{
			OspLog(1, "Osp: Instance%d is NULL\n", i);
		}
    }

	/* 创建App主邮箱 */
    if( !OspCreateMailbox(
		"OspAppMailbox",            // 邮箱名
		 maxMsgWaiting,           // 队列buffer数，消息头部和内容是分开的，要乘2
		 sizeof(TOsMsgStruc),       // 采用零拷贝后，邮箱中只存放消息头指针
		 &queRcvId,                 // 读端
		 &queSendId) )              // 写端
	{
		queRcvId = 0;
        queSendId = 0;
		OspLog(1, "Osp: create %s task mailbox fail\n", szName);
		return OSP_ERROR;
	}

	/* 创建App备用邮箱 */
    if( !OspCreateMailbox(
		"OspAppBakMailbox",          // 邮箱名
		 maxMsgWaiting,            // 邮箱buffer数
		 sizeof(TOsMsgStruc),        // 采用零拷贝后，邮箱中只存放消息头指针
		 &bakQueRcvId,               // 读端
		 &bakQueSendId) )            // 写端
	{
		bakQueRcvId = 0;
        bakQueSendId = 0;
		OspCloseMailbox(queRcvId, queSendId);
        queRcvId = 0;
        queSendId = 0;
		OspLog(1, "Osp: create %s task's backup mailbox fail\n",szName);
		return OSP_ERROR;
	}

	/* 创建一个任务来运行本App */
	if(wStackSize < OSP_APP_STACKSIZE)
		wStackSize = (u32)OSP_APP_STACKSIZE;

    hAppTask = OspTaskCreate(OspAppEntry, szName, byPri, wStackSize, (KD_PTR)this, 0x0, &taskID);
    if(hAppTask == 0)
	{
		taskID = 0;

		OspCloseMailbox(queRcvId, queSendId);
		queRcvId = 0;
        queSendId = 0;

		OspCloseMailbox(bakQueRcvId, bakQueSendId);
		bakQueRcvId = 0;
        bakQueSendId = 0;

		OspLog(1, "Osp: create %s task fail\n", szName);
		return OSP_ERROR;
	}

	/* 注册到AppPooL中 */
	g_Osp.m_cAppPool.m_apcAppRegTable[wAid-1] = this;

	/* 添加到Osp任务链表中 */
	g_Osp.AddTask(hAppTask, taskID, szName);

	/* 创建信号量保护本APP中所有可能被多个任务访问的成员变量 */
	OspSemBCreate(&tSemMutex);
    return OSP_OK;
}

/*====================================================================
函数名：CApp::QuitApp
功能：App退出, 退出所有实例，释放系统资源。
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
void CApp::QuitApp()
{
	CInstance *pIns = NULL;
    u16 instCount;
	u32 instMax = GetInstanceNumber();

	// 退出本App中所有实例，释放实例申请的内存
	pIns = GetInstance(CInstance::DAEMON);
	if(pIns != NULL)
	{
		pIns->InstanceExit();
		pIns->m_aliasLen = 0;
		//-- add in 03-10-27  ---//
		pIns->DelAllInstTimerInfo();
		OspSemDelete(pIns->m_tSemTimerList);
		//-----------------------//
	}

	for(instCount=1; instCount<=instMax; instCount++)
	{
		pIns = GetInstance(instCount);
		if(pIns != NULL)
		{
			pIns->InstanceExit();
			pIns->m_aliasLen = 0;
			//-- add in 03-10-27  ---//
			pIns->DelAllInstTimerInfo();
			OspSemDelete(pIns->m_tSemTimerList);
			//-----------------------//
		}
	}

	// 退出App：释放动态内存
	if(pAppName != NULL)
	{
		OspFreeMem(pAppName);
		pAppName = NULL;
	}

	// 释放App邮箱
	OspCloseMailbox(queRcvId, queSendId);
	queRcvId = 0;
	queSendId = 0;
	OspCloseMailbox(bakQueRcvId, bakQueSendId);
	bakQueRcvId = 0;
	bakQueSendId = 0;

	// 删除信号量
	if(tSemMutex != NULL)
	{
		::OspSemDelete(tSemMutex);
		tSemMutex = NULL;
	}

	if(appId>0 && appId<=MAX_APP_NUM)
	{
		g_Osp.m_cAppPool.m_apcAppRegTable[appId-1] = NULL;
	}
}

/*====================================================================
函数名：CApp::SetPriority
功能：设置/更改本App的优先级。
算法实现：（可选项）
引用全局变量：
输入参数说明：byPrior -- 优先级.

返回值说明：成功, 返回TRUE; 失败, 返回FALSE.
====================================================================*/
BOOL32 CApp::SetPriority(u8 byPrior)
{
	if( !OspTaskSetPriority(hAppTask, byPrior) )
	{
		OspPrintf(TRUE, FALSE, "CApp::SetPriority() for app%d failed, byPrior=%d.\n", appId, byPrior);
	    return FALSE;
	}

	byAppPri = byPrior;
	return TRUE;
}

/*====================================================================
函数名：OspSetAppPriority
功能：设置App的调度优先级。
算法实现：（可选项）
引用全局变量：
输入参数说明：wAppId -- 目标App号,
              byPrior -- 优先级.

  返回值说明：成功, 返回TRUE; 失败, 返回FALSE.
====================================================================*/
API BOOL32 OspSetAppPriority(u16 wAppId, u8 byPrior)
{
	CApp *pcApp = g_Osp.m_cAppPool.AppGet(wAppId);

	if(pcApp == NULL)
	{
		OspPrintf(TRUE, FALSE, "Osp: OspSetAppPriority() but app%d NOT exist.\n", wAppId);
		return FALSE;
	}

	return pcApp->SetPriority(byPrior);
}

/*====================================================================
函数名：OspGetAppPriority
功能：获得指定App的调度优先级。
算法实现：（可选项）
引用全局变量：
输入参数说明：wAppId -- (in)目标App号,
              pbyPrior -- (out)优先级.

  返回值说明：成功, 返回TRUE; 失败, 返回FALSE.
====================================================================*/
API BOOL32 OspGetAppPriority(u16 wAppId, u8* pbyPrior)
{
	CApp *pcApp = g_Osp.m_cAppPool.AppGet(wAppId);

	if(pcApp == NULL)
	{
		OspPrintf(TRUE, FALSE, "Osp: OspSetAppPriority() but app%d NOT exist.\n", wAppId);
		return FALSE;
	}

	if(pbyPrior != NULL)
	{
		*pbyPrior = pcApp->GetPriority();
	}

    return TRUE;
}

/*====================================================================
函数名：CInstance::SetAlias
功能：设置实例别名
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：pchAlias: 要设置的实例别名,
              len: 别名长度.

返回值说明：成功返回OSP_OK, 别名已被占用返回OSPERR_ALIAS_REPEAT
            其他返回OSP_ERROR.
====================================================================*/
int CInstance::SetAlias(const char * pchAlias, u8 len)
{
	CApp *pcApp = NULL;
	CInstance *pIns = NULL;
	BOOL32  ret;

	/* 入口参数检验 */
	if( (pchAlias==NULL) || (len<=0) || (len>m_maxAliasLen) )
	{
		return OSP_ERROR;
	}

	/* 看别名是否被其他实例占用 */
	pcApp = g_Osp.m_cAppPool.AppGet(m_appId);
	if(pcApp == NULL)
	{
		return OSP_ERROR;
	}

	pIns = pcApp->FindInstByAlias(pchAlias, len);
//	if( (pIns!=NULL) && (pIns->GetInsID()!=m_instId) )
	if(pIns!=NULL)
	{
		return OSPERR_ALIAS_REPEAT;
	}

	ret = pcApp->SetInstAlias(m_instId, pchAlias, len);
	if(ret == FALSE)
	{
		return OSP_ERROR;
	}

	/* create new */
	memcpy(m_alias, pchAlias, len);
	m_aliasLen = len;
	return OSP_OK;
}

/*====================================================================
函数名：CInstance::DeleteAlias
功能：取消别名
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
void CInstance::DeleteAlias()
{
	CApp *pcApp = NULL;

	if( (m_alias!=NULL) && (m_aliasLen>0) )
	{

		pcApp = g_Osp.m_cAppPool.AppGet(m_appId);
		if(pcApp == NULL)
		{
			return;
		}
        pcApp->ClearInstAlias(m_instId, m_alias, m_aliasLen);

		memset(m_alias, 0, m_aliasLen);
	}
	m_aliasLen = 0;
}

/*====================================================================
函数名：CInstance::GetAlias
功能：获得实例别名
算法实现：（可选项）
引用全局变量：
输入参数说明：aliasBuf: 用来存放实例别名的buf指针,
              bufLen: buf长度,
              aliasLen: 别名长度.

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
BOOL32 CInstance::GetAlias(char *aliasBuf, u8 bufLen, u8 *aliasLen)
{
	if(aliasBuf == NULL || bufLen < m_aliasLen)
	{
	    printf("GetAlias return false,buflen:%d,m_aliaslen:%d\n",bufLen,m_aliasLen);
		return FALSE;
	}

	if(m_alias==NULL || m_aliasLen==0)
	{
	    printf("GetAlias return false,m_alias:%s,m_aliaslen:%d\n",m_alias,bufLen,m_aliasLen);
		return FALSE;
	}

	memcpy(aliasBuf, m_alias, m_aliasLen);
	if(aliasLen != NULL)
	{
		*aliasLen = m_aliasLen;
	}
	return TRUE;
}

/*====================================================================
函数名：CInstance::GetAliasLen
功能：获得实例别名长度
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
u8 CInstance::GetAliasLen(void)
{
	return m_aliasLen;
}

/*====================================================================
函数名：CInstance::log
功能：实例日志功能实现
算法实现：（可选项）
引用全局变量：
输入参数说明：uLevel: 日志输出级别, 只有不高于App文件/屏幕日志控制级别的日志才能
                      输出到日志文件/屏幕,
              pchFormat: 格式化字符串,
			  ...: 可变参数表.

返回值说明：
====================================================================*/
void CInstance::log(u8 byLevel, const char *szFormat, ...)
{
    va_list pvList;
	TOspLogHead tOspLogHead;
	char msg[MAX_LOG_MSG_LEN];
	u32 actLen = 0;
	BOOL32 bToScreen = FALSE;
	BOOL32 bToFile = FALSE;

	if(szFormat == NULL)
	{
		return;
	}

	bToScreen = (byLevel <= g_Osp.m_cAppPool.AppGet(GetAppID())->scrnLogFlag) ? TRUE : FALSE;
	bToFile = (byLevel <= g_Osp.m_cAppPool.AppGet(GetAppID())->fileLogFlag) ? TRUE : FALSE;
    if(!bToFile && !bToScreen)
	{
		return;
	}

	tOspLogHead.type = LOG_TYPE_MASKABLE; // 可以通过开关屏蔽实例日志输出
	tOspLogHead.bToScreen = bToScreen;
    tOspLogHead.bToFile = bToFile;

	va_start(pvList, szFormat);
  	actLen = _vsnprintf(msg, MAX_LOG_MSG_LEN, szFormat, pvList);
    va_end(pvList);
    if(actLen <= 0)
    {
        printf("Osp: _vsnprintf() failed in CInstance::log().\n");
        return;
    }
    if (actLen >= MAX_LOG_MSG_LEN)
	{
        printf("Osp: msg's length is over MAX_LOG_MSG_LEN in CInstance::log().\n");
		return;
	}
    OspLogQueWrite(tOspLogHead, msg, actLen);
}

/*====================================================================
函数名：CInstance::NextState
功能：Instance的状态转换
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uState: 新状态,
              szState: 新状态的字符串描述.

返回值说明：
====================================================================*/
void CInstance::NextState(u32 dwState, char *szState)
{
    u16 appId = GetAppID();
    u16 insId = GetInsID();
	BOOL32 bSrcTrc = FALSE;
	BOOL32 bFileTrc = FALSE;

    m_lastState = m_curState;
    m_curState = dwState;

	//begin==>维测信息 状态迁移记录
	m_dwInstStateTimes++;
    if(m_chInstStateIndex >= MAX_INST_STATE_RECORD_COUNT)
    {
    	m_chInstStateIndex = 0;
    }
    m_atInstStateInfo[m_chInstStateIndex].dwIndex = m_dwInstStateTimes;
    m_atInstStateInfo[m_chInstStateIndex].dwState = dwState;
    m_chInstStateIndex++;
	//维测信息 状态迁移记录end<==
	bSrcTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCSTATE) ? TRUE:FALSE;
	bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCSTATE) ? TRUE:FALSE;
    if(!bSrcTrc && !bFileTrc)
	{
		return;
	}

	if(szState == NULL)
	{
		OspTrcPrintf(bSrcTrc, bFileTrc, "Osp: app %d, ins %d, goto state %d\n", appId, insId, dwState);
	}
	else
	{
		OspTrcPrintf(bSrcTrc, bFileTrc, "Osp: app %d, ins %d, goto state '%s'\n", appId, insId, szState);
	}
}

/*====================================================================
函数名：CInstance::post
功能：实例向结点uDstNode上实例ID为uDstIId的实例发送异步消息
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uDstIId: 目标实例的ID,
              uEvent: 待发消息的消息号,
              pvContent: 消息体,
			  uLength: 消息长度,
			  uDstNode: 目标结点.

返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int CInstance::post(u32 dwDstIId, u16 wEvent, const void *pvContent, u16 wLength, u32 dwDstNode)
{
	int len;
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];

    CApp *pApp = g_Osp.m_cAppPool.AppGet(m_appId);

	if(m_appId < 1 || m_appId > MAX_APP_NUM || pApp==NULL)
	{
		OspLog(1, "Osp: instance's appId=%d error.\n", m_appId);
		return OSP_ERROR;
	}

    bScrnTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[m_appId-1]->scrnTraceFlag & TRCEVENT) ? TRUE : FALSE;
    bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[m_appId-1]->fileTraceFlag & TRCEVENT) ? TRUE : FALSE;
    if(bFileTrc || bScrnTrc)
    {
        CMessage msg;
        msg.srcnode= 0;
        msg.dstnode= dwDstNode;
        msg.dstid = dwDstIId;     // app + gate + ins(2)
        msg.srcid = MAKEIID(GetAppID(), GetInsID(), 0);
		msg.type = MSG_TYPE_ASYNC;
        msg.event = wEvent;      // the name of the message
        msg.length = wLength;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)pvContent;
		msg.dstAlias = NULL;
		msg.dstAliasLen = 0;

		len = sprintf(achBuf, "message post app: %s = %d\n", pApp->pAppName, m_appId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace
    //add for debug

    return OspPostMsg(dwDstIId, wEvent, pvContent, wLength, dwDstNode,
		           MAKEIID(GetAppID(),GetInsID(),0), OspGetLocalID(),
		           TRUE);
}

/*====================================================================
函数名：CInstance::post
功能：实例向结点uDstNode上uDstApp下别名为pchDstAlias, 别名长度为uAliasLen,
      的实例发送异步消息
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：pchDstAlias: 目标实例的别名指针,
              uAliasLen: 别名长度,
              uDstApp: 目标实例所在App,
              uEvent: 待发消息的消息号,
              pvContent: 消息体,
			  uLength: 消息长度,
			  uDstNode: 目标结点.

返回值说明：
====================================================================*/
int CInstance::post(const char* pchDstAlias, u8 byAliasLen, u16 wDstApp, u16 wEvent,
		 const void* pvContent, u16 wLength, u32 dwDstNode)
{
    u16 appId = GetAppID();
    CApp *pApp = g_Osp.m_cAppPool.AppGet(appId);

    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	char achBuf[MAX_LOG_MSG_LEN];
	int len;

	if(appId< 1 || appId>MAX_APP_NUM || pApp==NULL)
	{
		OspLog(1, "Osp: instance's appId=%d error.\n", appId);
		return OSP_ERROR;
	}

    bScrnTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCEVENT) ? TRUE:FALSE;
    bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCEVENT) ? TRUE:FALSE;

	if ( bFileTrc || bScrnTrc )
    {
        CMessage msg;
        msg.srcnode = 0;
        msg.dstnode = dwDstNode;
        msg.dstid = MAKEIID(wDstApp,CInstance::INVALID,0);     // app + gate + ins(2)
        msg.srcid = MAKEIID(GetAppID(), GetInsID(), 0 );
		msg.type = MSG_TYPE_ASYNC;
        msg.event = wEvent;      // the name of the message
        msg.length= wLength;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)pvContent;
		msg.dstAlias = (char *)pchDstAlias;
		msg.dstAliasLen = byAliasLen;

		len = sprintf(achBuf, "message post app: %s = %d\n", pApp->pAppName, appId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace

    return OspPostMsg(pchDstAlias, byAliasLen, wDstApp, dwDstNode, wEvent,
		pvContent, wLength, MAKEIID(appId,GetInsID(),0) , OspGetLocalID() ,TRUE, MSG_TYPE_ASYNC);
}

/*====================================================================
函数名：CInstance::send
功能：实例向结点uDstNode上实例ID为uDstIId的实例发送同步消息
算法实现：（可选项）
引用全局变量：
输入参数说明：dstiid: 目标实例的ID,
              event: 待发消息的消息号,
              content: 消息体,
			  length: 消息长度,
			  dstnode: 目标结点,
              ackbuf: 用户指定的用来存放同步应答的buffer,
			  ackbuflen: 应答buffer长度,
			  realacklen: 实际收到的应答的长度,
			  timeout: 超时设置(ms).

返回值说明：发送成功返回OSP_OK, 发送超时返回OSPERR_SEND_TIMEOUT,
            应答超长返回OSPERR_SYNCACK_EXCEED, 其他返回OSP_ERROR.
====================================================================*/
int CInstance::send(u32 dstiid, u16 event, const void *content, u16 length, u32 dstnode,
		     void* ackbuf, u16 ackbuflen, u16 *realacklen,
			 u16 timeout)
{
	u32 lenrcv;
	TOsMsgStruc osMsg;
    CMessage *pMsg = NULL;
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	int len;
	char achBuf[MAX_LOG_MSG_LEN];

	u16 appId = GetAppID();
    CApp *pcApp = g_Osp.m_cAppPool.AppGet(appId);

	if(pcApp == NULL)
	{
		OspPrintf(TRUE, FALSE, "Osp: fatal error in CInstance::send(): pcApp is NULL.\n");
		return OSP_ERROR;
	}

    bScrnTrc = (pcApp->scrnTraceFlag & TRCEVENT) ? TRUE:FALSE;
    bFileTrc = (pcApp->fileTraceFlag & TRCEVENT) ? TRUE:FALSE;

	if(bFileTrc || bScrnTrc)
    {
        CMessage msg;
        msg.srcnode= 0;
        msg.dstnode= dstnode;
        msg.dstid = dstiid;     // app + gate + ins(2)
        msg.srcid = MAKEIID(GetAppID(), GetInsID(), 0 );
		msg.type = MSG_TYPE_SYNC;
        msg.event = event;      // the name of the message
        msg.length= length;      // the length (max 64K) of content of the message
        msg.content = (unsigned char*)content;
		msg.dstAlias = NULL;
		msg.dstAliasLen = 0;

		len = sprintf(achBuf, "message send app %s = %d\n", pcApp->pAppName, appId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    } // message trace

    int ret = OspPostMsg(dstiid, event, content, length, dstnode,
		           MAKEIID(appId,GetInsID(),0), OspGetLocalID(),
		           TRUE, MSG_TYPE_SYNC);
	if(ret != 0)
	{
		return OSP_ERROR;
	}

	/* 设置定时 */
	if(timeout != 0)
	{
		SetTimer(OSP_SYNC_TIMER, timeout, 0);
	}

    while(TRUE)
	{
		/* 从消息队列里接收消息 */
		OspRcvMsg(pcApp->queRcvId, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &lenrcv);
        if(NULL == osMsg.address)
		{
			OspLog(1, "Osp: CInstance::send OspRcvMsg fail, addr=%d.\n", osMsg.address );
			continue;
		}

		pMsg = (CMessage *)osMsg.address;
		if(pMsg == NULL)
		{
			OspLog(1, "Osp: fatal error in CInstance::send(): pMsg is NULL.\n");
			continue;
		}

		pcApp->MsgProcessedNumInc();

		/* 收到来自OspQuit()的退出消息 */
        if(pMsg->event == OSP_QUIT)
		{
			// 释放发送端为本条消息申请的内存
            OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10

			// 退出本App中所有实例，释放实例申请的内存
			pcApp->QuitApp();

			// 退出任务
			OspTaskExit();
		}

    	switch(pMsg->type)
		{
        /* 如果是同步应答，杀死定时器，将应答放到用户Buf中，再返回 */
		case MSG_TYPE_SYNCACK:
			KillTimer(OSP_SYNC_TIMER);
			if(realacklen != NULL)
			{
				*realacklen = pMsg->length;
			}

            if(pMsg->length > ackbuflen)  //应答buf不够用
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				return OSPERR_SYNCACK_EXCEED;
			}

			if(ackbuf != NULL)
			{
				memcpy(ackbuf, pMsg->content, pMsg->length);
			}

			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			return OSP_OK;
			break;

		/* 其他消息 */
        case MSG_TYPE_SYNC:
		case MSG_TYPE_GSYNC:
		case MSG_TYPE_ASYNC:
        case MSG_TYPE_TIMEOUT:
			/* 如果是同步发送超时消息，返回超时错 */
            if(pMsg->type==MSG_TYPE_TIMEOUT &&
				pMsg->event==OSP_SYNC_TIMER)
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				return OSPERR_SEND_TIMEOUT;
			}

			/* 如果是其他消息，放到备用邮箱里 */
			BOOL32 bSuccess;

            bSuccess = OspSndMsg(pcApp->bakQueSendId, (char *)&osMsg, sizeof(TOsMsgStruc));
			if( !bSuccess )
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				OspLog(1,"Osp: send message to mailbox failed in CInstance::send().\n");
				return OSP_ERROR;
			}

			/* 备用邮箱消息计数 */
			pcApp->BakMsgNumInc();
			break;

		default:
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			KillTimer(OSP_SYNC_TIMER);
			OspLog(1, "Osp: unknown message type encountered in CInstance::send.\n");
            return OSP_ERROR;
			break;
		}
	}
}

/*====================================================================
函数名：CInstance::send
功能：实例向结点uDstNode上dstapp下名为dstalias的实例发送同步消息
算法实现：（可选项）
引用全局变量：
输入参数说明：dstalias: 目标实例别名指针,
              aliaslen: 别名长度,
              dstapp: 目标App,
              event: 待发消息的消息号,
              content: 消息体,
			  length: 消息长度,
			  dstnode: 目标结点,
              ackbuf: 用户指定的用来存放同步应答的buffer,
			  ackbuflen: 应答buffer长度,
			  realacklen: 实际收到的应答的长度,
			  timeout: 超时设置(ms).

返回值说明：
====================================================================*/
int CInstance::send(const char* dstalias, u8 aliaslen, u16 dstapp, u16 event,
		     const void* content, u16 length, u32 dstnode,
			 void* ackbuf, u16 ackbuflen, u16 *realacklen,
			 u16 timeout)
{
	u32 lenrcv;
	TOsMsgStruc osMsg;
    CMessage *pMsg = NULL;
    BOOL32 bFileTrc;
    BOOL32 bScrnTrc;
	int len;
	char achBuf[MAX_LOG_MSG_LEN];

	u16 appId = GetAppID();
    CApp *pcApp = g_Osp.m_cAppPool.AppGet(appId);

	if(pcApp == NULL)
	{
		OspPrintf(TRUE, FALSE, "Osp: fatal error in CInstance::send(): pcApp is NULL.\n");
		return OSP_ERROR;
	}

    bScrnTrc = (pcApp->scrnTraceFlag & TRCEVENT) ? TRUE:FALSE;
    bFileTrc = (pcApp->fileTraceFlag & TRCEVENT) ? TRUE:FALSE;

	if(bFileTrc || bScrnTrc)
    {
        CMessage msg;

        msg.srcnode= 0;
        msg.dstnode= dstnode;
        msg.dstid = MAKEIID(dstapp, CInstance::INVALID, 0 );     // app + gate + ins(2)
        msg.srcid = MAKEIID(GetAppID(), GetInsID(), 0 );
		msg.type = MSG_TYPE_SYNC;
        msg.event = event;      // the name of the message
        msg.length= length;      // the length (max 64K) of content of the message
        msg.content = (u8 *)content;
		msg.dstAlias = (char *)dstalias;
		msg.dstAliasLen = aliaslen;

		len = sprintf(achBuf, "\nmessage send app %s =%d\n", pcApp->pAppName, appId);
		len += MsgDump2Buf(achBuf+len, MAX_LOG_MSG_LEN-len, &msg);
		OspMsgTrace(bScrnTrc, bFileTrc, achBuf, len);
    }

    int ret = OspPostMsg(dstalias, aliaslen, dstapp, dstnode, event,
		          content, length, MAKEIID(appId,GetInsID(),0) , OspGetLocalID() ,
				  TRUE, MSG_TYPE_SYNC);
	if(ret != 0)
	{
		return OSP_ERROR;
	}

	/* 设置定时 */
	if(timeout != 0)
	{
		SetTimer(OSP_SYNC_TIMER, timeout, 0);
	}

    while(TRUE)
	{
		/* 从消息队列里接收消息 */
		OspRcvMsg(pcApp->queRcvId, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &lenrcv);
        if(NULL == osMsg.address)
		{
			OspLog(1, "Osp: CInstance::send OspRcvMsg fail, addr=%d.\n", osMsg.address );
			continue;
		}

		pMsg = (CMessage *)osMsg.address;
		if(pMsg == NULL)
		{
			OspLog(1, "Osp: fatal error in CInstance::send(): pMsg is NULL.\n");
			continue;
		}

		pcApp->MsgProcessedNumInc();

		/* 收到来自OspQuit()的退出消息 */
        if(pMsg->event == OSP_QUIT)
		{
			// 释放发送端为本条消息申请的内存
            OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10

			// 退出本App中所有实例，释放实例申请的内存
			pcApp->QuitApp();

			// 退出任务
			OspTaskExit();
		}

    	switch(pMsg->type)
		{
        /* 如果是同步应答，杀死定时器，将应答放到用户Buf中，再返回 */
		case MSG_TYPE_SYNCACK:
			KillTimer(OSP_SYNC_TIMER);
			if(realacklen != NULL)
			{
				*realacklen = pMsg->length;
			}

            if(pMsg->length > ackbuflen)  //应答buf不够用
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				return OSPERR_SYNCACK_EXCEED;
			}

			if(ackbuf != NULL)
			{
				memcpy(ackbuf, pMsg->content, pMsg->length);
			}

			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			return OSP_OK;
			break;

		/* 其他消息 */
        case MSG_TYPE_SYNC:
		case MSG_TYPE_GSYNC:
		case MSG_TYPE_ASYNC:
        case MSG_TYPE_TIMEOUT:
			/* 如果是同步发送超时消息，返回超时错 */
            if(pMsg->type==MSG_TYPE_TIMEOUT &&
				pMsg->event==OSP_SYNC_TIMER)
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				return OSPERR_SEND_TIMEOUT;
			}

			/* 如果是其他消息，放到备用邮箱里 */
			BOOL32 bSuccess;

            bSuccess = OspSndMsg(pcApp->bakQueSendId, (char *)&osMsg, sizeof(TOsMsgStruc));
			if( !bSuccess )
			{
				OspFreeMem(pMsg);
//				g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
				OspLog(1,"Osp: send message to mailbox failed in CInstance::send().\n");
				return OSP_ERROR;
			}

			/* 备用邮箱消息计数 */
			pcApp->BakMsgNumInc();
			break;

		default:
			OspFreeMem(pMsg);
//			g_Osp.m_cMsgStack.ReturnStack(pMsg) ;   //11-10
			KillTimer(OSP_SYNC_TIMER);
			OspLog(1, "Osp: unknown message type encountered in CInstance::send.\n");
            return OSP_ERROR;
			break;
		}
	}
}

/*====================================================================
函数名：CInstance::reply
功能：返回同步应答
算法实现：（可选项）
引用全局变量：
输入参数说明：ack: 存放同步应答的内存指针,
              ackLen: 应答长度.

返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int CInstance::reply(const void* ack, u16 ackLen)
{
	return SetSyncAck(ack, ackLen);
}

/*====================================================================
函数名：CInstance::SetSyncAck
功能：返回同步应答
算法实现：（可选项）
引用全局变量：
输入参数说明：ack: 存放同步应答的内存指针,
              ackLen: 应答长度.

返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int CInstance::SetSyncAck(const void* ack, u16 ackLen)
{
	if(ack==NULL || ackLen<=0 || ackLen>MAX_SYNCACK_LEN)
		return OSP_ERROR;

	CApp *pcApp = g_Osp.m_cAppPool.AppGet(m_appId);
	if(pcApp == NULL)
	{
		return OSP_ERROR;
	}

	if(pcApp->curMsgRcv != NULL)
	{
	    pcApp->curMsgRcv->output = (u8 *)OspAllocMem(ackLen);
//		pcApp->curMsgRcv->output = (u8 *)pcApp->curMsgRcv+sizeof(CMessage)+pcApp->curMsgRcv->length+pcApp->curMsgRcv->dstAliasLen;
//		pcApp->curMsgRcv->output = (u8 *)g_Osp.m_cMsgStack.GetStack();
		memcpy(pcApp->curMsgRcv->output, ack, ackLen);
		pcApp->curMsgRcv->outlen = ackLen;
		return OSP_OK;
	}
	return OSP_ERROR;
}

/*====================================================================
函数名：CInstance::SetTimer
功能：设置定时器
算法实现：（可选项）
引用全局变量：
输入参数说明：uTimer: 定时器号,
              uMilliSeconds: 时间间隔(ms),
              uPara: 参数, 暂时不用.

返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
====================================================================*/
int CInstance::SetTimer(u32 dwTimer, long uMilliSeconds, u32 dwPara)
{
    u16 appId = GetAppID();
    u16 insId = GetInsID();
	BOOL32 bSrcTrc;
	BOOL32 bFileTrc;

	if(appId==0 || appId>MAX_APP_NUM || insId==INVALID_INS)
		return OSP_ERROR;

	void * pTimerBlkAddr = FindInstTimerInfo( dwTimer ) ;
    if( pTimerBlkAddr != NULL )
	{
		g_Osp.m_cTmListQue.KillQueTimer(appId, insId, (u16)dwTimer, pTimerBlkAddr);
		DelInstTimerInfo( dwTimer ) ;
	}

	bSrcTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCTIMER) ? TRUE:FALSE;
	bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCTIMER) ? TRUE:FALSE;
	if(bFileTrc || bSrcTrc)
	{
		OspTrcPrintf(bSrcTrc, bFileTrc, "app %d, ins %d set timer %d\n",appId,insId,dwTimer);
	}

	void * pTmBlkAddr = g_Osp.m_cTmListQue.SetQueTimer(appId, insId, (u16)dwTimer, uMilliSeconds, dwPara);
	if( pTmBlkAddr == NULL )
	{
		return OSP_ERROR ;
	}
	AddInstTimerInfo( dwTimer, pTmBlkAddr );

    return OSP_OK;
}

/*====================================================================
函数名：CInstance::SetAbsTimer
功能：设置绝对定时器
算法实现：（可选项）
引用全局变量：
输入参数说明：uTimer: 定时器号,
              nYear/nMon/nDay/nHour/nMin/nSec: 年/月/日/时/分/秒格式的设置到时时间,
              uPara: 参数, 暂时不用.

返回值说明：成功返回OSP_OK, 失败返回OSP_ERROR.
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/24/2003		3.0      李雪峰        增加功能函数
====================================================================*/
int CInstance::SetAbsTimer(u32 nTimer, u16 nYear, u8 nMon, u8 nDay, u8 nHour, u8 nMin,
						   u8 nSec, u32 dwPara)
{
    if((nMon > 12) || (nDay > 31) || (nHour > 23) || (nMin > 59) || (nSec > 59))
		return OSP_ERROR;

	tm      tSetTime;
	time_t  setTime, curTime;
	double  retMisec;
	tSetTime.tm_year = nYear - 1900;
	tSetTime.tm_mon = nMon - 1;
	tSetTime.tm_mday = nDay;
	tSetTime.tm_hour = nHour;
	tSetTime.tm_min = nMin;
	tSetTime.tm_sec = nSec;

	if( (setTime = mktime( &tSetTime )) == (time_t)-1 ){
		return OSP_ERROR;
	}
	time(&curTime);
    retMisec = difftime(setTime, curTime);
	if(retMisec <= 0){
		return OSP_ERROR;
	}
    u16 appId = GetAppID();
    u16 insId = GetInsID();
	BOOL32 bSrcTrc;
	BOOL32 bFileTrc;

	if(appId==0 || appId>MAX_APP_NUM || insId==INVALID_INS){
		return OSP_ERROR;
	}

	void * pTimerBlkAddr = FindInstTimerInfo(nTimer) ;
    if( pTimerBlkAddr != NULL )
	{
		g_Osp.m_cTmListQue.KillAbsTimer(appId, insId, (u16)nTimer);
		DelInstTimerInfo(nTimer) ;
	}

	bSrcTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCTIMER) ? TRUE:FALSE;
	bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCTIMER) ? TRUE:FALSE;
	if(bFileTrc || bSrcTrc)
	{
		OspTrcPrintf(bSrcTrc, bFileTrc, "app %d, ins %d set AbsTimer %d\n",appId,insId,nTimer);
	}

	void * pTmBlkAddr = g_Osp.m_cTmListQue.SetAbsTimer(appId, insId, (u16)nTimer, setTime, dwPara);
	if( pTmBlkAddr == NULL )
	{
		return OSP_ERROR ;
	}
	AddInstTimerInfo( nTimer, pTmBlkAddr ) ;

    return OSP_OK;
}
/*add 11-19*/
/*====================================================================
  函数名：CInstance::KillAbsTimer
  功能：杀死一个绝对定时器
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：uTimerNo: 待杀死的定时器号

  返回值说明：
  ====================================================================*/
    int CInstance::KillAbsTimer( u32 nTimer)
	{

	    u16 appId = GetAppID();
	    u16 insId = GetInsID();
	    BOOL32 bSrcTrc;
	    BOOL32 bFileTrc;

	    if(appId==0 || appId>MAX_APP_NUM || insId==INVALID_INS)
	    {
			return OSP_ERROR;
	    }

	    bSrcTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCTIMER) ? TRUE:FALSE;
	    bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCTIMER) ? TRUE:FALSE;
	    if(bFileTrc || bSrcTrc)
	    {
			OspTrcPrintf(bSrcTrc, bFileTrc, "app %d, ins %d del AbsTimer %d\n",appId,insId,nTimer);
	    }
	    BOOL ret = g_Osp.m_cTmListQue.KillAbsTimer(appId, insId, (u16)nTimer);
		if (ret == TRUE)
		{
			return OSP_OK;
		}

	    return OSP_ERROR;
	}

/*====================================================================
函数名：CInstance::KillTimer
功能：杀死一个定时器
算法实现：（可选项）
引用全局变量：
输入参数说明：uTimerNo: 待杀死的定时器号

返回值说明：
====================================================================*/
int CInstance::KillTimer(u32 dwTimerNo)
{
	BOOL32 bSrcTrc;
	BOOL32 bFileTrc;
    u16 appId = GetAppID();
    u16 insId = GetInsID();

	if(appId==0 || appId>MAX_APP_NUM)
	{
		return OSP_ERROR;
	}

	void * pTimerBlkAddr = FindInstTimerInfo( dwTimerNo ) ;
    if( pTimerBlkAddr == NULL )
	{
		return OSP_ERROR ;
	}

	bSrcTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->scrnTraceFlag & TRCTIMER) ? TRUE:FALSE;
	bFileTrc = (g_Osp.m_cAppPool.m_apcAppRegTable[appId-1]->fileTraceFlag & TRCTIMER) ? TRUE:FALSE;
    if(bFileTrc || bSrcTrc)
	{
		OspTrcPrintf(bSrcTrc, bFileTrc, "app %d, ins %d kill timer %d\n",appId,insId,dwTimerNo);
	}

	g_Osp.m_cTmListQue.KillQueTimer(appId, insId, (u16)dwTimerNo, pTimerBlkAddr);
	DelInstTimerInfo( dwTimerNo ) ;

    return OSP_OK;
}

/*====================================================================
函数名：CInstance::GetAppID
功能：取得当前实例的App号.
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：当前实例的App号.
====================================================================*/
u16 CInstance::GetAppID(void)
{
    return m_appId;
}

/*====================================================================
函数名：CInstance::GetInsID
功能：取得当前实例的实例号
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：当前实例的实例号
====================================================================*/
u16 CInstance::GetInsID()
{
    return m_instId;
}

/*====================================================================
函数名：CInstance::LastState
功能：获取实例的上一状态
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：实例的上一状态
====================================================================*/
u32 CInstance::LastState()
{
    return m_lastState;
}

/*====================================================================
函数名：CInstance::CurState
功能：获取实例的当前状态
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：实例的当前状态
====================================================================*/
u32 CInstance::CurState()
{
    return m_curState;
}
//打印实例状态改变的历史信息
void CInstance::InstStateInfoShow(u16 wInstId)
{
	u32 dwLineCount = 0;

	for(u32 dwIndex = 0; dwIndex < MAX_INST_STATE_RECORD_COUNT; dwIndex++)
	{
		//如果是0，说明该inst没有使用过，不需要打印
		if (0 == m_atInstStateInfo[dwIndex].dwIndex)
		{
			break;
		}

		if(0 == dwIndex)
		{
			OspPrintf(TRUE, FALSE, "print instance[%d] state history:\n", wInstId);
			OspPrintf(TRUE, FALSE, "--------------------------------------\n", wInstId);
        }

		OspPrintf( TRUE , FALSE , "index=%d, state=%d\n", \
			m_atInstStateInfo[dwIndex].dwIndex, m_atInstStateInfo[dwIndex].dwState);
		if( ++dwLineCount > 50 )
		{
			OspTaskDelay( 50 );
			dwLineCount = 0;
		}
	}
}
/*====================================================================
函数名：CInstance::AddInstTimerInfo
功能：增加实例定时器信息块
算法实现：（可选项）
引用全局变量：
输入参数说明：nTimerNo : 定时器Id
              pTimerAddr : 定时器控制块在timer模块中定时器列表中的地址

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void   CInstance::AddInstTimerInfo(u32 nTimerNo , void * pTimerAddr)
{
//	TInstTimerInfo * newTimerInfo = (TInstTimerInfo *)OspAllocMem(sizeof(TInstTimerInfo)) ;
	OspSemTake(m_tSemTimerList);

	TInstTimerInfo * newTimerInfo = (TInstTimerInfo *)g_Osp.m_pcInstTimeStack->StackAlloc();   //11-10
	if( newTimerInfo == NULL )
	{
		OspSemGive(m_tSemTimerList);
		return;
	}

	newTimerInfo->next = NULL ;
	newTimerInfo->timerId = nTimerNo ;
	newTimerInfo->timerBlkAdrr = pTimerAddr ;

	/*直接插入头部*/
	newTimerInfo->next = m_timerInfoListHead.next;
	m_timerInfoListHead.next = newTimerInfo;

	OspSemGive(m_tSemTimerList);
}

/*====================================================================
函数名：CInstance::DelInstTimerInfo
功能：删除实例定时器信息块
算法实现：（可选项）
引用全局变量：
输入参数说明：nTimerNo : 定时器Id

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void   CInstance::DelInstTimerInfo(u32 nTimerNo)
{
	OspSemTake(m_tSemTimerList);

	TInstTimerInfo * tempPreInfo = NULL ;
	TInstTimerInfo * tempNextInfo = NULL ;

	tempPreInfo = &m_timerInfoListHead ;
	TInstTimerInfo * tempInfo = m_timerInfoListHead.next ;
	while( tempInfo != NULL )
	{
		tempNextInfo = tempInfo->next ;
		if( tempInfo->timerId == nTimerNo )
		{
			tempPreInfo->next = tempNextInfo ;
			g_Osp.m_pcInstTimeStack->StackReturn(tempInfo) ;
//			tempInfo = NULL ;
			OspSemGive(m_tSemTimerList);
			return ;
		}
		tempPreInfo = tempInfo ;
		tempInfo = tempInfo->next ;
	}

	OspSemGive(m_tSemTimerList);
}

/*====================================================================
函数名：CInstance::FindInstTimerInfo
功能：查找实例定时器信息块
算法实现：（可选项）
引用全局变量：
输入参数说明：nTimerNo : 定时器Id

  返回值说明：查找成功，返回定时器信息块的首地址，
              查找失败，返回NULL
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void* CInstance::FindInstTimerInfo(u32 nTimerNo)
{
	OspSemTake(m_tSemTimerList);

	TInstTimerInfo * tempInfo = m_timerInfoListHead.next ;
	while( tempInfo != NULL )
	{
		if( tempInfo->timerId == nTimerNo )
		{
			OspSemGive(m_tSemTimerList);
			return tempInfo->timerBlkAdrr ;
		}
		tempInfo = tempInfo->next ;
	}

	OspSemGive(m_tSemTimerList);
	return NULL ;
}

/*====================================================================
函数名：CInstance::DelAllInstTimerInfo
功能：  删除所有实例定时器信息块
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
12/29/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void   CInstance::DelAllInstTimerInfo()
{
	OspSemTake(m_tSemTimerList);

    TInstTimerInfo * tempNextInfo = NULL;
	TInstTimerInfo * tempInfo = m_timerInfoListHead.next;
	while( tempInfo != NULL )
	{
		tempNextInfo = tempInfo->next;
		g_Osp.m_pcInstTimeStack->StackReturn(tempInfo);

		tempInfo = tempNextInfo;
	}

	OspSemGive(m_tSemTimerList);
}

/*====================================================================
函数名：OspAllocMem
功能：从内存池中分配内存块
算法实现：（可选项）
引用全局变量：
输入参数说明：size: 待分配的内存块大小，

返回值说明：成功返回指向分配到的内存块的指针，失败返回NULL.
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/10/2003		3.0      李雪峰        内存管理功能
====================================================================*/
API void *OspAllocMem(size_t size)
{
#if 0
	void *pRet=NULL;

	if(size > 4*1024*1024)
		return NULL;

//	pRet = malloc(size);

	if(size <= 64)
		pRet = g_Osp.m_c64BStack.GetStack();
	else if(size <= 128)
		pRet = g_Osp.m_c128BStack.GetStack();
	else if(size <= 256)
		pRet = g_Osp.m_c256BStack.GetStack();
	else if(size <= 512)
		pRet = g_Osp.m_c512BStack.GetStack();
	else if(size <= 1024)
		pRet = g_Osp.m_c1KBStack.GetStack();
	else if(size <= 2*1024)
		pRet = g_Osp.m_c2KBStack.GetStack();
	else if(size <= 4*1024)
		pRet = g_Osp.m_c4KBStack.GetStack();
	else if(size <= 8*1024)
		pRet = g_Osp.m_c8KBStack.GetStack();
	else if(size <= 16*1024)
		pRet = g_Osp.m_c16KBStack.GetStack();
	else if(size <= 32*1024)
		pRet = g_Osp.m_c32KBStack.GetStack();
	else if(size <= 64*1024)
		pRet = g_Osp.m_c64KBStack.GetStack();
	else if(size < 128*1024)
		pRet = g_Osp.m_c128KBStack.GetStack();
	else if(size <= 256*1024)
		pRet = g_Osp.m_c256KBStack.GetStack();
	else if(size <= 512*1024)
		pRet = g_Osp.m_c512KBStack.GetStack();
	else if(size <= 1024*1024)
		pRet = g_Osp.m_c1MBStack.GetStack();
	else if(size <= 2*1024*1024)
		pRet = g_Osp.m_c2MBStack.GetStack();
    else if(size <= 4*1024*1024)
		pRet = g_Osp.m_c4MBStack.GetStack();

	if(pRet != NULL)
	{
		g_dwMallocTimes++ ;

		return pRet;
	}
	else
	{
		return  NULL;
	}
#endif //if 0
	void* pRet =NULL;
	if(g_Osp.m_cOspInerMemPool.IsOspMemPoolInit())
	{
		pRet = g_Osp.m_cOspInerMemPool.OspMemPoolAlloc(size);
	}
	return pRet;
}

/*====================================================================
函数名：OspFreeMem
功能：释放事先分配的内存块
算法实现：（可选项）
引用全局变量：
输入参数说明：token: 指向待释放的内存块的指针。

返回值说明：无.
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/10/2003		3.0      李雪峰        内存管理功能
====================================================================*/
API void OspFreeMem(void *token)
{
//	free(token);
#if 0
	if(token != NULL)
	{
		gTOspStackNode  * retNode = (gTOspStackNode *)((PTR)token - sizeof(TOspStackHeader));
		if(retNode != NULL)
		{
            g_dwFreeTimes++;
			switch(retNode->header.flag)
			{
			case OSP_64_BYTE_STACK_MARKER:
				g_Osp.m_c64BStack.ReturnStack(token);
				break;
			case OSP_128_BYTE_STACK_MARKER:
				g_Osp.m_c128BStack.ReturnStack(token);
				break;
			case OSP_256_BYTE_STACK_MARKER:
				g_Osp.m_c256BStack.ReturnStack(token);
				break;
			case OSP_512_BYTE_STACK_MARKER:
				g_Osp.m_c512BStack.ReturnStack(token);
				break;
			case OSP_1K_BYTE_STACK_MARKER:
				g_Osp.m_c1KBStack.ReturnStack(token);
				break;
			case OSP_2K_BYTE_STACK_MARKER:
				g_Osp.m_c2KBStack.ReturnStack(token);
				break;
			case OSP_4K_BYTE_STACK_MARKER:
				g_Osp.m_c4KBStack.ReturnStack(token);
				break;
			case OSP_8K_BYTE_STACK_MARKER:
				g_Osp.m_c8KBStack.ReturnStack(token);
				break;
			case OSP_16K_BYTE_STACK_MARKER:
				g_Osp.m_c16KBStack.ReturnStack(token);
				break;
			case OSP_32K_BYTE_STACK_MARKER:
				g_Osp.m_c32KBStack.ReturnStack(token);
				break;
			case OSP_64K_BYTE_STACK_MARKER:
				g_Osp.m_c64KBStack.ReturnStack(token);
				break;
			case OSP_128K_BYTE_STACK_MARKER:
				g_Osp.m_c128KBStack.ReturnStack(token);
				break;
			case OSP_256K_BYTE_STACK_MARKER:
				g_Osp.m_c256KBStack.ReturnStack(token);
				break;
			case OSP_512K_BYTE_STACK_MARKER:
				g_Osp.m_c512KBStack.ReturnStack(token);
				break;
			case OSP_1M_BYTE_STACK_MARKER:
				g_Osp.m_c1MBStack.ReturnStack(token);
				break;
			case OSP_2M_BYTE_STACK_MARKER:
				g_Osp.m_c2MBStack.ReturnStack(token);
				break;
			case OSP_4M_BYTE_STACK_MARKER:
				g_Osp.m_c4MBStack.ReturnStack(token);
				break;
			default:
                g_dwFreeTimes--;
				OspTrcPrintf(TRUE, TRUE, "MemError : OspFreeMem Find no stack !");
				break;
			}
		}
	}
#endif //if 0
	COspStack* pcReturnStack = NULL;
	if (NULL == token)
	{
		printf("this arg is NULL\n");
		return;
	}
	pcReturnStack = (COspStack*)(((_TOspStackNode*)((u8*)token - sizeof(_TOspStackNode)))->tHeader.hMemStackAddr);
	pcReturnStack->StackReturn(token);
	return;
}

/*====================================================================
函数名：OspMemShow
功能：  显示内存管理堆栈中消息块数目
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspMemShow()
{
#if 0
	OspPrintf(TRUE, FALSE, "Osp System Stack Info\n");
/*	OspPrintf(TRUE, FALSE, "  MsgStackNum Free %d, Total %d\n",
		      g_Osp.m_cMsgStack.m_wdStackAvailableBlkNum, g_Osp.m_cMsgStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  LogStackNum Free %d, Total %d\n",
		      g_Osp.m_cLogStack.m_wdStackAvailableBlkNum, g_Osp.m_cLogStack.m_wdStackTotalBlkNum);
*/
    OspPrintf(TRUE, FALSE, "  TimerStackNum Free %d, Total %d\n",
		      g_Osp.m_cTimerStack.m_wdStackAvailableBlkNum, g_Osp.m_cTimerStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  InstTimeStackNum Free %d, Total %d\n",
		      g_Osp.m_cInstTimeStack.m_wdStackAvailableBlkNum, g_Osp.m_cInstTimeStack.m_wdStackTotalBlkNum);

    OspPrintf(TRUE, FALSE, "Osp: call <OspAllocMem> times %d <OspFreeMem> times %d\n", g_dwMallocTimes, g_dwFreeTimes);

    OspPrintf(TRUE, FALSE, "  64BYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c64BStack.m_wdStackAvailableBlkNum, g_Osp.m_c64BStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  128BYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c128BStack.m_wdStackAvailableBlkNum, g_Osp.m_c128BStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  256BYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c256BStack.m_wdStackAvailableBlkNum, g_Osp.m_c256BStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  512BYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c512BStack.m_wdStackAvailableBlkNum, g_Osp.m_c512BStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  1KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c1KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c1KBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  2KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c2KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c2KBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  4KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c4KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c4KBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  8KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c8KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c8KBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  16KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c16KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c16KBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  32KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c32KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c32KBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  64KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c64KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c64KBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  128KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c128KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c128KBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  256KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c256KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c256KBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  512KBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c512KBStack.m_wdStackAvailableBlkNum, g_Osp.m_c512KBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  1MBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c1MBStack.m_wdStackAvailableBlkNum, g_Osp.m_c1MBStack.m_wdStackTotalBlkNum);
    OspPrintf(TRUE, FALSE, "  2MBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c2MBStack.m_wdStackAvailableBlkNum, g_Osp.m_c2MBStack.m_wdStackTotalBlkNum);
	OspPrintf(TRUE, FALSE, "  4MBYTEStackNum Free %d, Total %d\n",
		      g_Osp.m_c4MBStack.m_wdStackAvailableBlkNum, g_Osp.m_c4MBStack.m_wdStackTotalBlkNum);
#endif
	g_Osp.m_cOspInerMemPool.OspMemPooLState();
}

/*====================================================================
函数名：OspSetPriRealTime
功能：把定时器、消息发送等任务提高到比系统net任务还高的优先级
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：无.
====================================================================*/
API void OspSetPriRealTime()
{
	TTaskInfo* ptCurTask = g_Osp.GetFirstTask();
    TTaskInfo* ptNextTask = NULL;

    while( ptCurTask != NULL )
    {
		ptNextTask = g_Osp.GetNextTask(ptCurTask->id);

	    if(OspTaskHandleVerify(ptCurTask->handle))
		{
			if(memcmp(ptCurTask->name, "OspLogTask", strlen("OspLogTask")) == 0)
				OspTaskSetPriority(ptCurTask->handle, 200);
			else if(memcmp(ptCurTask->name, "TimerTask", strlen("TimerTask")) == 0)
				OspTaskSetPriority(ptCurTask->handle, 40);
			else
				OspTaskSetPriority(ptCurTask->handle, 45);
		}

	    ptCurTask = ptNextTask;
    }
}


/*=============================================================================
函 数 名：OspSetMemCheck
功    能：获取内存地址内容
注    意：
算法实现：
全局变量：
参    数： u32 * pdwCheckMem :  要检查的内存地址
           u32 dwCheckValue ：  期望检查的值
		   BOOL32  bEqual ：当本参数为真时，表示要检查的内存地址内容不等于期望值时需要告警
		                    当本参数为假时，表示要检查的内存地址内容等于期望值时需要告警
返 回 值： 成功返回TRUE, 失败返回FALSE.
-------------------------------------------------------------------------------
修改记录：
日      期  版本  修改人  修改内容
2004/03/10  3.0   李雪峰  增加功能接口
=============================================================================*/
API	BOOL32  OspSetMemCheck(u32 * pdwCheckMem, u32 dwCheckValue, BOOL32 bEqual)
{
	if(pdwCheckMem == NULL)
		return  FALSE;

	/* Just for removing compile warning */
	bEqual = FALSE;
	dwCheckValue = 0;


	return  TRUE;
}

