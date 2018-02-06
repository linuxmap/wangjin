/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��ʱ���ܵ���Ҫʵ���ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
******************************************************************************/
#include "../include/ospsch.h"
#include "stdio.h"

#include   "malloc.h"

static TASKID g_dwTimerTaskID;

/*====================================================================
��������msToTick
���ܣ���������ת��Ϊtick��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uMsCount: ������

  ����ֵ˵����tick��
====================================================================*/
u64 msToTick(u32 dwMsCount)
{
    return (u64)dwMsCount;
}

/*====================================================================
��������tickToMs
���ܣ���tick��ת��Ϊ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uTick: tick��

  ����ֵ˵����������
====================================================================*/
u32 tickToMs(u64 dwTick)
{
    return (u32)dwTick;
}

/*====================================================================
��������TmListQue::TmListQue
���ܣ� ��ʱ�����еĹ��캯��, �Գ�Ա�������г�ʼ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������TmListQue::~TmListQue
���ܣ� ��ʱ�����е���������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
TmListQue::~TmListQue()
{
   OspSemDelete(semaphor);
}

/*====================================================================
��������TmListQue::ReturnTmBlk
���ܣ��黹һ��Item�����ж�ʱ��������
�㷨ʵ�֣��黹��ͷ��, ʹ��Item��Ϊ�µ�ͷ��
����ȫ�ֱ�����
�������˵����ptTmBlk: ָ����黹Item��ָ��.

  ����ֵ˵����
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
��������TmListQue::AllocTmBlk()
���ܣ��ӿ��ж�ʱ�������з���һ��Item.
�㷨ʵ�֣������ж�ʱ������Ϊ��, ��ֱ�Ӵ��ڴ��з���.
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�������䵽�Ŀ���Itemָ��
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
��������TmListQue::InternalAddTimer
���ܣ���ʱ�����������һ��item
�㷨ʵ�֣�
����ȫ�ֱ�����
�������˵����timer: ָ�������Item��ָ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::ListAdd
���ܣ���ʱ�����ƿ���������
�㷨ʵ�֣�
����ȫ�ֱ�����
�������˵����newTimer: ָ������붨ʱ�����ƿ��ָ��
              prev : ����λ�õ�ǰһ���ڵ�
			  next : ����λ�õĺ�һ���ڵ�

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
======================================================================*/
void TmListQue::ListAdd(TmBlk *newTimer, TmBlk *prev, TmBlk * next)
{
	//mod by gzj. �޸Ĵ˴��������жϡ�ԭ�������ɵ��� 255 ��timer��Խ����ʣ������ѭ����
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
��������TmListQue::SetQueTimer
���ܣ�����һ����ʱ��
�㷨ʵ�֣��ӿ��ж��������һ�����ж�ʱ��, �����ʵ�����Ϣ����뵽��Ч��
          ����.
����ȫ�ֱ�����
�������˵����uAppId: �ö�ʱ�������Ǹ�app,
              uInstId: �ö�ʱ�������Ǹ�ins,
			  uTimer: ��ʱ����,
			  uMilliSeconds: ��ʱ���(ms),
			  uPara: ��ʱ������.

  ����ֵ˵�������ӷ���ֵ ������ɹ��򷵻ض�ʱ����ĵ�ַ�����򷵻�NULL
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
��������TmListQue::InsertAbsTimer
���ܣ�����Զ�ʱ�����������һ��item
�㷨ʵ�֣�����Զ�ʱ������Ϊ�����µ�item��Ϊ��ͷ, ����, ��tickֵ��С�����˳��
         ������Զ�ʱ��������.
����ȫ�ֱ�����
�������˵����ptTmBlk: ָ�������Item��ָ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
11/24/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::SetAbsTimer
���ܣ�����һ�����Զ�ʱ��
�㷨ʵ�֣��ӿ��ж��������һ�����ж�ʱ��, �����ʵ�����Ϣ����뵽���Զ�
          ʱ��������.
����ȫ�ֱ�����
�������˵����uAppId: �ö�ʱ�������Ǹ�app,
              uInstId: �ö�ʱ�������Ǹ�ins,
			  uTimer: ��ʱ����,
			  tAbsTime:���Զ�ʱ��ʱʱ��,
			  uPara: ��ʱ������.

  ����ֵ˵�������ӷ���ֵ ������ɹ��򷵻ض�ʱ����ĵ�ַ�����򷵻�NULL
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
11/24/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������SetTimerEx
���ܣ�����һ����ʱ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAppId: �ö�ʱ�������Ǹ�app,
              uInstId: �ö�ʱ�������Ǹ�ins,
			  uTimer: ��ʱ����,
			  uMilliSeconds: ��ʱ���(ms),
			  uPara: ��ʱ������(��ʱ����).

  ����ֵ˵����
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
��������TmListQue::KillQueTimer
���ܣ�ɾ��һ����ʱ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAppId: �ö�ʱ�������Ǹ�app,
              uInstId: �ö�ʱ�������Ǹ�ins,
			  uTimer: ��ʱ����.
			  pTmBlkAddr: Ҫɾ���Ķ�ʱ�����ƿ�ĵ�ַ

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        �������������pTmBlkAddr��
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
��������KillTimerEx
���ܣ�ɾ��һ����ʱ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAppId: �ö�ʱ�������Ǹ�app,
              uInstId: �ö�ʱ�������Ǹ�ins,
			  uTimer: ��ʱ����.

  ����ֵ˵����
====================================================================*/
void KillTimerEx(u8 byAppId, u16 wInstId, u16 wTimer)
{
    g_Osp.m_cTmListQue.KillQueTimer(byAppId, wInstId, wTimer);
}

/*====================================================================
��������TmListQue::GetCurrentTick
���ܣ���ȡ��ǰϵͳ����������tick��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����ǰϵͳ����������tick��
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
��������TmListQue::GetCurrentTickNoSema
���ܣ���ȡ��ǰϵͳ����������tick��,û���ź�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����ǰϵͳ����������tick��
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
11/24/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::ListDel
���ܣ��Ѷ�ʱ�����ƿ��������ɾ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����entry:Ҫɾ���Ķ�ʱ�����ƿ��ַ

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::CascadeTimers
���ܣ�����һ������ʱ�������еĶ�ʱ�����·�ɢת�Ƶ���һ������Ķ�ʱ��������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����tv:Ҫ����ת�ƵĶ�ʱ������

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
====================================================================*/
// void TmListQue::CascadeTimers(timerVec *tv, u32 tvwhat)
// {
//     TmBlk *head, *curr, *next;
//
//     /*ȷ�ϲ�Խ��*/
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
//     /*ȷ�ϲ�Խ��*/
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
��������TmListQue::ReviseBaseTick
���ܣ�ϵͳ��������У����׼tick���͸������̵�index
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::FreeTvn
���ܣ��ͷű��������еĶ�ʱ�����ƿ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����TvnHead : Ҫ�ͷŵĶ�ʱ�������׵�ַ

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::FreeTv
���ܣ��ͷ����б����е����ж�ʱ�����ƿ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
��������TmListQue::GetOspBaseTick
���ܣ���ȡ��׼tick��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������׼tick��
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
====================================================================*/
u64  TmListQue::GetOspBaseTick()
{
	return m_nBaseTick;
}

/*====================================================================
��������TmListQue::RunTimerList
���ܣ�ɨ��������̺͵�ʱ�Ķ�ʱ�����У����Ʊ��̶�ʱ�����У�������ʱ����ʱ��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/22/2003		3.0      ��ѩ��        ���ӹ��ܺ���
====================================================================*/
#define INDEX(N) ((m_nBaseTick >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)
void TmListQue::RunTimerList()
{
	u16 appId;
    u16 insId;
    u16 timerId;
	int    ret;
	u32 param;   //��ʱ�����ز���
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

				// ��ʱ��Ϣ���Զ����������ʱ������������һ��Index��,��Ϣ�����ӷ��ض�ʱ�����õĲ���
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
					/* ɾ��ʵ���еĶ�ʱ����Ϣ�����ֺ�timerģ���е�ͬ�� */
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
��������TmListQue::RunAbsTimerList
���ܣ�ɨ����Զ�ʱ�����У�������ʱ����ʱ��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
--------------------------------------------------------------------------------------------------
���ӡ��޸ļ�¼:
��  ��		    �汾	 �޸���		   �޸�����
10/24/2003		3.0      ��ѩ��        ���ӹ��ܺ���
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
		pTmTemp = pTmDel->suc;	  // ���¸�Item

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
��������TimerTask
���ܣ�timer task�����, ɨ�趨ʱ�����У����ͳ�ʱ��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void TimerTask()
{
	int      absContl;

	absContl = 0;
	while(TRUE)
	{
		/* ����û�����OspQuit, �ͷŶ�ʱ������ռ�õ��ڴ�, �˳������� */
		if(g_Osp.m_bKillOsp)
		{
			g_Osp.m_cTmListQue.FreeAll();
			g_Osp.DelTask(g_dwTimerTaskID);
			printf("[TimerTask] del task[%x]\n", g_dwTimerTaskID);
			OspTaskExit();
        }

		g_Osp.m_cTmListQue.RunTimerList();

		//����ɨ����Զ�ʱ������//
		absContl++;
		if(absContl >= 300)    //5����ɨ��һ��
		{
			absContl = 0;
		    g_Osp.m_cTmListQue.RunAbsTimerList();
		}

        OspTaskDelay(3); // _WINDOWS_ ÿ15����ɨ��һ��
    }
}

/*====================================================================
��������TimerSysInit
���ܣ�������ʱ����ɨ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API BOOL32 TimerSysInit()
{
	TASKHANDLE hTask;
	u32 dwTaskID = 0;

	// ��ʹ��֮ǰ��������
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
��������OspTimerShow
���ܣ���ʾtimer״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspTimerShow(void)
{
    g_Osp.m_cTmListQue.Show();
}

/*====================================================================
��������TimerShowAll
���ܣ���ʾtimer״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void TimerShowAll()
{
    g_Osp.m_cTmListQue.ShowAll();
}

/*====================================================================
��������TmListQue::Show
���ܣ���ʾtimer��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������TmListQue::Show
���ܣ���ʾ����timer��״̬��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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


