/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP 定时功能的主要实现文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98		1.0      某某        ------------
******************************************************************************/
#include "../include/ospsch.h"
#include "stdio.h"

#include   "malloc.h"

static TASKID g_dwTimerTaskID;

/*====================================================================
函数名：msToTick
功能：将毫秒数转换为tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：uMsCount: 毫秒数

  返回值说明：tick数
====================================================================*/
u64 msToTick(u32 dwMsCount)
{
    return (u64)dwMsCount;
}

/*====================================================================
函数名：tickToMs
功能：将tick数转换为毫秒数
算法实现：（可选项）
引用全局变量：
输入参数说明：uTick: tick数

  返回值说明：毫秒数
====================================================================*/
u32 tickToMs(u64 dwTick)
{
    return (u32)dwTick;
}

/*====================================================================
函数名：TmListQue::TmListQue
功能： 定时器对列的构造函数, 对成员变量进行初始化.
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
TmListQue::TmListQue()
{
	int i;
    OspSemBCreate(&semaphor);

	m_nActiveTimerCount = 0;
	m_nBaseTick = 0 ;
	for (i = 0; i < TVN_SIZE; i++)
	{
		INIT_LIST_HEAD(tv5.vec + i);
		INIT_LIST_HEAD(tv4.vec + i);
		INIT_LIST_HEAD(tv3.vec + i);
		INIT_LIST_HEAD(tv2.vec + i);
	}
	for (i = 0; i < TVR_SIZE; i++)
		INIT_LIST_HEAD(tv1.vec + i);

	m_nReorderTimerCount = 0;
	m_nDropTimerCount = 0;
	m_nKilledTimerCount = 0;

	m_tOppTimerQue.dwCount = 0;
	memset( m_tOppTimerQue.pTmBlk , NULL , sizeof(m_tOppTimerQue.pTmBlk) );

	m_tAbsTimerQue.count = 0;
	m_tAbsTimerQue.suc = (TmBlk *)&m_tAbsTimerQue;
	m_tAbsTimerQue.pre = (TmBlk *)&m_tAbsTimerQue;

	m_qwTickBase = 0;
	m_qwTickLast = 0;
}

/*====================================================================
函数名：TmListQue::~TmListQue
功能： 定时器队列的析构函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
TmListQue::~TmListQue()
{
   OspSemDelete(semaphor);
}

/*====================================================================
函数名：TmListQue::ReturnTmBlk
功能：归还一个Item到空闲定时器队列中
算法实现：归还到头部, 使该Item成为新的头部
引用全局变量：
输入参数说明：ptTmBlk: 指向待归还Item的指针.

  返回值说明：
====================================================================*/
void TmListQue::ReturnTmBlk(TmBlk* ptTmBlk)
{
	ptTmBlk->appId = 0;
	ptTmBlk->instId = 0;
	ptTmBlk->timerId = 0;
	ptTmBlk->tick = 0;
	ptTmBlk->settedTick = 0;
	ptTmBlk->timeToLeft = 0;
	ptTmBlk->param = 0;
	ptTmBlk->absTime = 0;

	g_Osp.m_pcTimerStack->StackReturn(ptTmBlk);

    m_nActiveTimerCount-- ;
}

/*====================================================================
函数名：TmListQue::AllocTmBlk()
功能：从空闲定时器队列中分配一个Item.
算法实现：若空闲定时器队列为空, 则直接从内存中分配.
引用全局变量：
输入参数说明：

  返回值说明：分配到的空闲Item指针
====================================================================*/
TmBlk* TmListQue::AllocTmBlk()
{
//        pTmBlkTmp = (TmBlk*)OspAllocMem(sizeof(TmBlk));        
	TmBlk * pTmBlkTmp = (TmBlk*)g_Osp.m_pcTimerStack->StackAlloc(); 

	if(pTmBlkTmp != NULL)
		m_nActiveTimerCount++ ;

    return pTmBlkTmp;
}

/*====================================================================
函数名：TmListQue::InternalAddTimer
功能：向定时器队列里插入一个item
算法实现：
引用全局变量：
输入参数说明：timer: 指向待插入Item的指针

  返回值说明：成功返回TRUE, 失败返回FALSE.
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
======================================================================*/
void TmListQue::InternalAddTimer(TmBlk *timer, u32 tvwhat, s32 index /* = -1 */)
{
	s64 nExpires = timer->tick -1;     //u64
	nExpires /= OSP_TIMER_PRICISION;
	nExpires += 1;
	u64 uIdx = nExpires - m_nBaseTick;
	TmBlk * ptVec;
	u16 uLoc;

	if ((s64)uIdx < 0)
	{
        ptVec = tv1.vec + (m_nBaseTick & TVR_MASK);
	}
	else if (uIdx < TVR_SIZE)
	{
		uLoc = (u16)(nExpires & TVR_MASK);
		ptVec = (tv1.vec + uLoc) ;
	}
	else if (uIdx < (1 << (TVR_BITS + TVN_BITS)))
	{
		uLoc = (u16)((nExpires >> TVR_BITS) & TVN_MASK);
		if ( tvwhat == 2 && uLoc == index )
			uLoc = (uLoc +1 ) & TVN_MASK;
		ptVec = (tv2.vec + uLoc);
	}
	else if (uIdx < (1 << (TVR_BITS + 2 * TVN_BITS)))
	{
		uLoc = (u16)((nExpires >> (TVR_BITS + TVN_BITS)) & TVN_MASK);
		if ( tvwhat == 3 && uLoc == index )
			uLoc = (uLoc +1 ) & TVN_MASK;
		ptVec =  (tv3.vec + uLoc);
	}
	else if (uIdx < (1 << (TVR_BITS + 3 * TVN_BITS)))
	{
		uLoc = (u16)((nExpires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK);
		if ( tvwhat == 4 && uLoc == index )
			uLoc = (uLoc +1 ) & TVN_MASK;
		ptVec = (tv4.vec + uLoc);
	}
	else if (uIdx <= 0xffffffffUL)
	{
		uLoc = (u16)((nExpires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK);
		if ( tvwhat == 5 && uLoc == index )
			uLoc = (uLoc +1 ) & TVN_MASK;
		ptVec = (tv5.vec + uLoc);
	}
	else
	{
		INIT_LIST_HEAD(timer);
		return;
	}
	/*  Timers are FIFO! */
	ListAdd(timer, ptVec->pre, ptVec->pre->suc);
}

/*====================================================================
函数名：TmListQue::ListAdd
功能：向定时器控制块插入队列中
算法实现：
引用全局变量：
输入参数说明：newTimer: 指向待插入定时器控制块的指针
              prev : 插入位置的前一个节点
			  next : 插入位置的后一个节点

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
======================================================================*/
void TmListQue::ListAdd(TmBlk *newTimer, TmBlk *prev, TmBlk * next)
{
	//mod by gzj. 修改此处的条件判断。原代码会造成到达 255 个timer后越界访问，造成死循环。
	if( m_tOppTimerQue.dwCount < TEMP_OPP_TIMER_NUM )
	{
		m_tOppTimerQue.pTmBlk[m_tOppTimerQue.dwCount] = newTimer;
		m_tOppTimerQue.dwCount++;
	}
	next->pre = newTimer;
	newTimer->suc = next;
	newTimer->pre = prev;
	prev->suc = newTimer;
}

/*====================================================================
函数名：TmListQue::SetQueTimer
功能：设置一个定时器
算法实现：从空闲队列里分配一个空闲定时块, 填上适当的信息后插入到有效队
          列中.
引用全局变量：
输入参数说明：uAppId: 该定时器属于那个app,
              uInstId: 该定时器属于那个ins,
			  uTimer: 定时器号,
			  uMilliSeconds: 定时间隔(ms),
			  uPara: 定时器参数.

  返回值说明：增加返回值 ，插入成功则返回定时器块的地址，否则返回NULL
====================================================================*/
void * TmListQue::SetQueTimer(u16 wAppId, u16 wInstId, u16 wTimer, u32 dwMilliSeconds, u32 dwPara)
{
	OspTaskSafe();
    OspSemTake(semaphor);

	u64 tickCurrent = GetCurrentTickNoSema();

	TmBlk *pTmBlk = AllocTmBlk();
    if(pTmBlk == NULL)
	{
		OspSemGive(semaphor);
		return NULL ;
	}

    pTmBlk->appId = wAppId;
    pTmBlk->instId = wInstId;
    pTmBlk->timerId = wTimer;
    pTmBlk->param = dwPara;
	pTmBlk->settedTick = msToTick(dwMilliSeconds);
	pTmBlk->timeToLeft = 0;
    pTmBlk->tick = msToTick(dwMilliSeconds) + tickCurrent;
	pTmBlk->absTime = 0;

    InternalAddTimer(pTmBlk);

    OspSemGive(semaphor);
	OspTaskUnsafe();

	return (void *)pTmBlk ;
}

/*====================================================================
函数名：TmListQue::InsertAbsTimer
功能：向绝对定时器队列里插入一个item
算法实现：如绝对定时器队列为空则新的item成为队头, 否则, 按tick值从小到大的顺序
         插入绝对定时器队列中.
引用全局变量：
输入参数说明：ptTmBlk: 指向待插入Item的指针

  返回值说明：成功返回TRUE, 失败返回FALSE.
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/24/2003		3.0      李雪峰        增加功能函数
====================================================================*/
BOOL32 TmListQue::InsertAbsTimer(TmBlk* newTimer)
{
    double retMisec;
    TmBlk *pTmTemp = m_tAbsTimerQue.suc;

	while(pTmTemp != (TmBlk *)&m_tAbsTimerQue)
	{
		retMisec = difftime(newTimer->absTime, pTmTemp->absTime);
		if (retMisec < 0)
		{
			break;
		}
		pTmTemp = pTmTemp->suc;
	}

	ListAdd(newTimer, pTmTemp->pre, pTmTemp);

	return TRUE;
}

/*====================================================================
函数名：TmListQue::SetAbsTimer
功能：设置一个绝对定时器
算法实现：从空闲队列里分配一个空闲定时块, 填上适当的信息后插入到绝对定
          时器队列中.
引用全局变量：
输入参数说明：uAppId: 该定时器属于那个app,
              uInstId: 该定时器属于那个ins,
			  uTimer: 定时器号,
			  tAbsTime:绝对定时到时时间,
			  uPara: 定时器参数.

  返回值说明：增加返回值 ，插入成功则返回定时器块的地址，否则返回NULL
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/24/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void *  TmListQue::SetAbsTimer(u16 wAppId, u16 wInstId, u16 wTimer, time_t tAbsTime, u32 dwPara)
{
	OspTaskSafe();
    OspSemTake(semaphor);

	TmBlk *pTmBlk = AllocTmBlk();
    if(pTmBlk == NULL)
	{
		OspSemGive(semaphor);
		OspTaskUnsafe();
		return NULL ;
	}

    pTmBlk->appId = wAppId;
    pTmBlk->instId = wInstId;
    pTmBlk->timerId = wTimer;
    pTmBlk->param = dwPara;
	pTmBlk->settedTick = 0;
	pTmBlk->timeToLeft = 0;
    pTmBlk->tick = 0;
	pTmBlk->absTime = tAbsTime;

	InsertAbsTimer(pTmBlk);

    OspSemGive(semaphor);
	OspTaskUnsafe();

	return (void *)pTmBlk ;
}

/*====================================================================
函数名：SetTimerEx
功能：设置一个定时器
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAppId: 该定时器属于那个app,
              uInstId: 该定时器属于那个ins,
			  uTimer: 定时器号,
			  uMilliSeconds: 定时间隔(ms),
			  uPara: 定时器参数(暂时不用).

  返回值说明：
====================================================================*/
API void SetTimerEx(u16 wAppId, u16 wInstId, u16 wTimer, u32 dwMilliSeconds, u32 dwPara)
{
    g_Osp.m_cTmListQue.SetQueTimer(wAppId, wInstId, wTimer, dwMilliSeconds, dwPara);
}


BOOL TmListQue::KillAbsTimer(u16 wAppId, u16 wInstId, u16 wTimer)
{
	BOOL ret = FALSE;

    OspTaskSafe();
    OspSemTake(semaphor);

    TmBlk *pTmTemp = m_tAbsTimerQue.suc;

	while(pTmTemp != (TmBlk *)&m_tAbsTimerQue)
	{
		if ( (pTmTemp->appId == wAppId )&& (pTmTemp->instId == wInstId) && (pTmTemp->timerId == wTimer))
		{
			ListDel(pTmTemp);
			ReturnTmBlk(pTmTemp);
			ret = TRUE;
			break;
		}

		pTmTemp = pTmTemp->suc;
	}

    OspSemGive(semaphor);
    OspTaskUnsafe();

    return ret;
}
/*====================================================================
函数名：TmListQue::KillQueTimer
功能：删除一个定时器
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAppId: 该定时器属于那个app,
              uInstId: 该定时器属于那个ins,
			  uTimer: 定时器号.
			  pTmBlkAddr: 要删除的定时器控制块的地址

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加输入参数（pTmBlkAddr）
====================================================================*/
void TmListQue::KillQueTimer(u16 wAppId, u16 wInstId, u16 wTimer , void * pTmBlkAddr)
{
	if( pTmBlkAddr == NULL )
		return;

	OspTaskSafe();
    OspSemTake(semaphor);

	TmBlk * pTmBlk = (TmBlk *)pTmBlkAddr;
	if( (pTmBlk != NULL) && (pTmBlk->pre != NULL) && (pTmBlk->suc != NULL) )
	{
		if( (pTmBlk->appId == wAppId) && (pTmBlk->instId == wInstId) &&
			(pTmBlk->timerId == wTimer) )
		{
			ListDel( pTmBlk );
			ReturnTmBlk( pTmBlk );

			m_nKilledTimerCount++;
		}
	}

    OspSemGive(semaphor);
    OspTaskUnsafe();
}

/*====================================================================
函数名：KillTimerEx
功能：删除一个定时器
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAppId: 该定时器属于那个app,
              uInstId: 该定时器属于那个ins,
			  uTimer: 定时器号.

  返回值说明：
====================================================================*/
void KillTimerEx(u8 byAppId, u16 wInstId, u16 wTimer)
{
    g_Osp.m_cTmListQue.KillQueTimer(byAppId, wInstId, wTimer);
}

/*====================================================================
函数名：TmListQue::GetCurrentTick
功能：获取当前系统启动后经历的tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：前系统启动后经历的tick数
====================================================================*/
u64  TmListQue::GetCurrentTick()
{
	u64 result;

	OspTaskSafe();
	OspSemTake(semaphor);

	result = GetCurrentTickNoSema();

	OspSemGive(semaphor);
	OspTaskUnsafe();
	return result;
}

/*====================================================================
函数名：TmListQue::GetCurrentTickNoSema
功能：获取当前系统启动后经历的tick数,没有信号量保护
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：前系统启动后经历的tick数
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
11/24/2003		3.0      李雪峰        增加功能函数
====================================================================*/
u64  TmListQue::GetCurrentTickNoSema()
{
	u64 tickCurr;
	u64 result;

	tickCurr = OspTickGet64();
	if(tickCurr < m_qwTickLast)
	{
		OspPrintf(1,0,"[GetCurrentTickNoSema] time roll back,curTick:%llu, lastTick:%llu, baseTick:%llu\n",
			tickCurr, m_qwTickLast, m_qwTickBase);
		m_qwTickBase += (u64)0xffffffff;//the 32 bit tick may return to 0 after some time,so add
		m_qwTickBase += (u64)0x1;
	}
	m_qwTickLast = tickCurr;

	result = /*m_qwTickBase* + */tickCurr;

	return result;
}


int g_OspListDelError = 0;

/*====================================================================
函数名：TmListQue::ListDel
功能：把定时器控制块从链表中删除
算法实现：（可选项）
引用全局变量：
输入参数说明：entry:要删除的定时器控制块地址

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void TmListQue::ListDel(TmBlk *entry)
{
	if( m_tOppTimerQue.dwCount >= 1 )
	{
		for( u32 dwIndex = 0 ; dwIndex < m_tOppTimerQue.dwCount ; dwIndex++ )
		{
			if( m_tOppTimerQue.pTmBlk[dwIndex] == entry )
			{
				m_tOppTimerQue.pTmBlk[dwIndex] = NULL;
				m_tOppTimerQue.pTmBlk[dwIndex] = m_tOppTimerQue.pTmBlk[m_tOppTimerQue.dwCount-1];
				m_tOppTimerQue.dwCount--;
				break;
			}
		}
	}
	entry->suc->pre = entry->pre;
	entry->pre->suc = entry->suc;

	entry->suc = NULL;
	entry->pre = NULL;
}

/*====================================================================
函数名：TmListQue::CascadeTimers
功能：把上一个级别定时器表盘中的定时器重新分散转移到下一个级别的定时器表盘中
算法实现：（可选项）
引用全局变量：
输入参数说明：tv:要往下转移的定时器表盘

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
// void TmListQue::CascadeTimers(timerVec *tv, u32 tvwhat)
// {
//     TmBlk *head, *curr, *next;
//
//     /*确认不越界*/
//     if(tv->index > TVN_MASK)
// 		tv->index = 0;
//
//     head = tv->vec + tv->index;
//     curr = head->suc;
//     while (curr != head)
//     {
// 		next = curr->suc;
// 		ListDel(curr);
// 		InternalAddTimer(curr, tvwhat, tv->index);
// 		curr = next;
//     }
//     INIT_LIST_HEAD(head);
//     tv->index = ((tv->index + 1) & TVN_MASK);
//
//     /*确认不越界*/
//     if(tv->index > TVN_MASK)
// 		tv->index = 0;
// }

s32  TmListQue::CascadeTimers( timerVec *tv, u32 tvwhat, s32 index)
{
    TmBlk *head, *curr, *next;

    head = tv->vec + index;
    curr = head->suc;
    while (curr != head)
    {
        next = curr->suc;
        ListDel(curr);
        InternalAddTimer(curr, tvwhat, index);
        curr = next;
    }
    INIT_LIST_HEAD(head);

    return index;
}

/*====================================================================
函数名：TmListQue::ReviseBaseTick
功能：系统启动初期校正基准tick数和各个表盘的index
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void TmListQue::ReviseBaseTick()
{
	OspTaskSafe();
	OspSemTake(semaphor);

    s64  curTick = GetCurrentTickNoSema();
	curTick /= OSP_TIMER_PRICISION;
//    curTick++;

//    u16  index1 = (u16)(curTick & TVR_MASK);
//	u16  index2 = (u16)((curTick >> TVR_BITS) & TVN_MASK);
//	u16  index3 = (u16)((curTick >> (TVR_BITS + TVN_BITS)) & TVN_MASK);
//	u16  index4 = (u16)((curTick >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK);
//	u16  index5 = (u16)((curTick >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK);

//	tv1.index = index1;
//	tv2.index = index2+1;
//	tv3.index = index3+1;
//	tv4.index = index4+1;
//	tv5.index = index5+1;
	m_nBaseTick = curTick;

	OspSemGive(semaphor);
	OspTaskUnsafe();
}

/*====================================================================
函数名：TmListQue::FreeTvn
功能：释放表盘中所有的定时器控制块
算法实现：（可选项）
引用全局变量：
输入参数说明：TvnHead : 要释放的定时器表盘首地址

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void TmListQue::FreeTvn(TmBlk *TvnHead)
{
	TmBlk  *head, *curr;
	BOOL32  runFlag = TRUE;
	while ( runFlag )
	{
		head = TvnHead;
		curr = TvnHead->suc;
		if (curr != head)
		{
			ListDel(curr);
			g_Osp.m_pcTimerStack->StackReturn(curr); 
//			curr = NULL;
		}
		else
		{
			runFlag = FALSE;
		}
	}
}

/*====================================================================
函数名：TmListQue::FreeTv
功能：释放所有表盘中的所有定时器控制块
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void TmListQue::FreeTv()
{
	int i;
	for (i = 0; i < TVN_SIZE; i++)
	{
		FreeTvn(tv5.vec + i);
		FreeTvn(tv4.vec + i);
		FreeTvn(tv3.vec + i);
		FreeTvn(tv2.vec + i);
	}
	for (i = 0; i < TVR_SIZE; i++)
		FreeTvn(tv1.vec + i);
}

/*====================================================================
函数名：TmListQue::GetOspBaseTick
功能：获取基准tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：基准tick数
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
u64  TmListQue::GetOspBaseTick()
{
	return m_nBaseTick;
}

/*====================================================================
函数名：TmListQue::RunTimerList
功能：扫描各个表盘和到时的定时器队列，下移表盘定时器队列，发出定时器超时消息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/22/2003		3.0      李雪峰        增加功能函数
====================================================================*/
#define INDEX(N) ((m_nBaseTick >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)
void TmListQue::RunTimerList()
{
	u16 appId;
    u16 insId;
    u16 timerId;
	int    ret;
	u32 param;   //定时器返回参数
    u64    interTick;
	TmBlk *head, *curr;
//	int n;
	BOOL32  runFlag;

	OspTaskSafe();
	OspSemTake(semaphor);

	interTick = GetCurrentTickNoSema();
	interTick /= OSP_TIMER_PRICISION;
	u32 dwCount = 0;
	while( interTick >= m_nBaseTick )
	{
		if ( dwCount++ > 512 )
			break;
		s32 index = m_nBaseTick & TVR_MASK;

        if (!index &&
            (!CascadeTimers(tvecs[1], 2, INDEX(0))) &&
            (!CascadeTimers(tvecs[2], 3, INDEX(1))) &&
            !CascadeTimers(tvecs[3], 4, INDEX(2)))
			CascadeTimers(tvecs[4], 5, INDEX(3));

        ++m_nBaseTick;

		runFlag = TRUE;
		while(runFlag)
		{
			head = tv1.vec + index;
			curr = head->suc;
			if (curr != head)
			{
				appId = curr->appId;
				insId = curr->instId;
				timerId = curr->timerId;
				param = curr->param;

				ListDel(curr);

				CApp *pApp = g_Osp.m_cAppPool.AppGet(appId);
				if(pApp != NULL)
				{
					pApp->TimerProcessedIncrease();
				}

				// 超时消息可以丢弃，这个定时器将被放入下一个Index中,消息中增加返回定时器设置的参数
				ret = OspPostMsg(MAKEIID(appId,insId,0), timerId, &param, sizeof(param), 0, 0, 0, TRUE, MSG_TYPE_TIMEOUT,0);
				if(ret == OSP_ERROR)
				{
					curr->timeToLeft++;

					if(curr->timeToLeft > OSP_TIMER_TTL)
					{
						CApp *pApp = g_Osp.m_cAppPool.AppGet(appId);
						if(pApp != NULL)
						{
							CInstance *pIns = pApp->GetInstance(insId);
							if(pIns != NULL)
							{
								pIns->DelInstTimerInfo(timerId);
							}
						}
						ReturnTmBlk(curr);

						m_nDropTimerCount++;
					}
					else
					{
						curr->tick += curr->settedTick / OSP_TIMER_PRICISION + 1;
						InternalAddTimer(curr);

						m_nReorderTimerCount++;
					}
				}
				else
				{
					/* 删除实例中的定时器信息，保持和timer模块中的同步 */
					CApp *pApp = g_Osp.m_cAppPool.AppGet(appId);
					if(pApp != NULL)
					{
						CInstance *pIns = pApp->GetInstance(insId);
						if(pIns != NULL)
						{
							pIns->DelInstTimerInfo(timerId);
						}
					}

					ReturnTmBlk(curr);
				}
			}
			else
			{
				runFlag = FALSE;
			}
		}

		interTick = GetCurrentTickNoSema();
		interTick /= OSP_TIMER_PRICISION;
	}

	OspSemGive(semaphor);
	OspTaskUnsafe();
}

/*====================================================================
函数名：TmListQue::RunAbsTimerList
功能：扫描绝对定时器队列，发出定时器超时消息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
--------------------------------------------------------------------------------------------------
增加、修改记录:
日  期		    版本	 修改人		   修改内容
10/24/2003		3.0      李雪峰        增加功能函数
====================================================================*/
void TmListQue::RunAbsTimerList()
{
	time_t curTime;
	double retMisec;
	TmBlk * pTmTemp = NULL;
	u16 appId;
	u16 insId;
	u16 timerId;
	int ret;
	u32 param;

	OspTaskSafe();
	OspSemTake(semaphor);

	time(&curTime);

	pTmTemp = m_tAbsTimerQue.suc;
	while(pTmTemp != (TmBlk *)&m_tAbsTimerQue)
	{
		TmBlk * pTmDel = pTmTemp;
		pTmTemp = pTmDel->suc;	  // 到下个Item

		appId = pTmDel->appId;
		insId = pTmDel->instId;
		timerId = pTmDel->timerId;
		param = pTmDel->param;
		retMisec = difftime(pTmDel->absTime, curTime);

		if(retMisec <= 0)
		{
			ret = OspPostMsg(MAKEIID(appId,insId,0), timerId, &param, sizeof(param), 0, 0, 0, TRUE, MSG_TYPE_TIMEOUT,0);
			if(ret == OSP_OK)
			{
				ListDel(pTmDel);
				ReturnTmBlk(pTmDel);
			}
		}
		else
		{
			break;
		}
	}

	OspSemGive(semaphor);
	OspTaskUnsafe();
}

/*====================================================================
函数名：TimerTask
功能：timer task的入口, 扫描定时器队列，发送超时消息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void TimerTask()
{
	int      absContl;

	absContl = 0;
	while(TRUE)
	{
		/* 如果用户调用OspQuit, 释放定时器队列占用的内存, 退出本任务 */
		if(g_Osp.m_bKillOsp)
		{
			g_Osp.m_cTmListQue.FreeAll();
			g_Osp.DelTask(g_dwTimerTaskID);
			printf("[TimerTask] del task[%x]\n", g_dwTimerTaskID);
			OspTaskExit();
        }

		g_Osp.m_cTmListQue.RunTimerList();

		//接着扫描绝对定时器队列//
		absContl++;
		if(absContl >= 300)    //5秒钟扫描一次
		{
			absContl = 0;
		    g_Osp.m_cTmListQue.RunAbsTimerList();
		}

        OspTaskDelay(3); // _WINDOWS_ 每15毫秒扫描一次
    }
}

/*====================================================================
函数名：TimerSysInit
功能：创建定时队列扫描任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API BOOL32 TimerSysInit()
{
	TASKHANDLE hTask;
	u32 dwTaskID = 0;

	// 在使用之前进行修正
	g_Osp.m_cTmListQue.ReviseBaseTick();

    hTask = OspTaskCreate( TimerTask, "OspTimerTask", OSP_TIMER_TASKPRI, OSP_TIMER_STACKSIZE, 0x0 , 0x0, &dwTaskID);
	if(hTask == 0)
	{
		return FALSE;
	}

	g_Osp.AddTask(hTask, dwTaskID, "TimerTask");

	return TRUE;
}

/*====================================================================
函数名：OspTimerShow
功能：显示timer状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspTimerShow(void)
{
    g_Osp.m_cTmListQue.Show();
}

/*====================================================================
函数名：TimerShowAll
功能：显示timer状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void TimerShowAll()
{
    g_Osp.m_cTmListQue.ShowAll();
}

/*====================================================================
函数名：TmListQue::Show
功能：显示timer的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void TmListQue::Show()
{
	OspPrintf( TRUE, FALSE, "print timer info:\n");
	OspPrintf( TRUE, FALSE, "---------------------------\n");
    OspPrintf(TRUE, FALSE, "total active timer %d \n", m_nActiveTimerCount);
    OspPrintf(TRUE, FALSE, "total free timer %d \n", g_Osp.m_pcTimerStack->m_dwStackAvailbleBlkNum);
	OspPrintf(TRUE, FALSE, "total killed timer %d \n", m_nKilledTimerCount);
	OspPrintf(TRUE, FALSE, "total %d times reorder timer dueto post <timeout> failed \n", m_nReorderTimerCount);
	OspPrintf(TRUE, FALSE, "total %d timer dropped dueto exceed TTL\n", m_nDropTimerCount);
}

/*====================================================================
函数名：TmListQue::Show
功能：显示所有timer的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void TmListQue::ShowAll() // show all the timer details
{
    Show();

    u64 currentTick = g_Osp.m_cTmListQue.GetCurrentTick();
	OspTaskSafe();
    OspSemTake(semaphor);

	if( m_tOppTimerQue.dwCount == 0 )
	{
		OspSemGive(semaphor);
		OspTaskUnsafe();
		return;
	}

	TmBlk *curr = NULL;
	CApp *pApp = NULL;
	u32 number = 0;
	for( u32 dwIndex = 0 ; dwIndex < m_tOppTimerQue.dwCount ; dwIndex++ )
	{
		number++;
		curr = m_tOppTimerQue.pTmBlk[dwIndex];
		pApp = g_Osp.m_cAppPool.AppGet( curr->appId );

		OspPrintf( TRUE, FALSE, "Timer Item%d: target appId=%d, instId=%d, timerid=%d, timeleft=%d ticks\n",
			number, curr->appId, curr->instId, curr->timerId, (u32)(curr->tick-currentTick) );
	}

    OspSemGive(semaphor);
	OspTaskUnsafe();
}


