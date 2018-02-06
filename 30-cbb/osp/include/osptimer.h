/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��ʱ���ܵ���Ҫ����ͷ�ļ� 
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
******************************************************************************/

#ifndef  OSP_TIME_H_INCLUDE
#define  OSP_TIME_H_INCLUDE
#include "osp.h"
#include "../include/osplog.h"
#include "ospvos.h"

#include "time.h"     //���Զ�ʱ

#include "stdio.h"

#define OSP_TIMER_BASE         (MAX_TIMER_NUM + 1)
#define OSP_SYNC_TIMER         (OSP_TIMER_BASE + 4999) //ͬ����Ϣ��ʱ��



u64 msToTick(u32 msCount);
u32 tickToMs(u64 tick);

struct TmBlk // ��ʱ���ƿ�
{
    TmBlk * suc; // ��һ��ʱ����ƿ顣
    TmBlk * pre; // ��һ��ʱ����ƿ顣
    u64 tick;
    u16 appId;
    u16 instId;
    u16 timerId;
    u32 param;
	u64 settedTick;
	u16 timeToLeft;
	time_t absTime;      //���Զ�ʱ��ʱʱ��
};

struct TmAbsQueHead
{
    TmBlk *suc; // first
    TmBlk *pre;
    u32 count;
};

#define TEMP_OPP_TIMER_NUM (u32)255
struct TmOppQue
{
	u32 dwCount;
	TmBlk *pTmBlk[TEMP_OPP_TIMER_NUM];
};

API BOOL32 TimerSysInit();

/* Event timer code*/
#define OSP_TIMER_PRICISION         100     /*100ms*/

#define OSP_TIMER_TTL               100   /*��ʱ�������*/

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

struct timerVec {
	//int index;
	struct TmBlk vec[TVN_SIZE];
};

struct timerVecRoot {
	//int index;
	struct TmBlk vec[TVR_SIZE];
};

static struct timerVec tv5;
static struct timerVec tv4;
static struct timerVec tv3;
static struct timerVec tv2;
static struct timerVecRoot tv1;

static struct timerVec * const tvecs[] = {
	(struct timerVec *)&tv1, &tv2, &tv3, &tv4, &tv5
};

#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

#define INIT_LIST_HEAD(ptr) do{ (ptr)->suc = (ptr); (ptr)->pre = (ptr); }while(0)

class TmListQue
{
private:
    SEMHANDLE semaphor;
	TmOppQue m_tOppTimerQue;//��Զ�ʱ������
	TmAbsQueHead  m_tAbsTimerQue;     //���Զ�ʱ������

	u64 m_qwTickBase;
	u64 m_qwTickLast;

public:
	TmListQue(); // ��ʼ������
    ~TmListQue(); // ��������

    void Show();

    void ShowAll(); // show all the timer details

	void FreeAll()
	{
		OspSemTake(semaphor);
		FreeTv() ;
		OspSemGive(semaphor);
	}
    
    u32 GetTmBlkNum()
    {
		return m_nActiveTimerCount ;
    }

    u32 GetTmBlkFreeNum()
    {
        return 0;      //  freeItemQue.count;
    }
    
    void *  SetQueTimer(u16 wAppId, u16 wInstId, u16 wTimer, u32 dwMilliSeconds, u32 uPara=0x80ffffff);
    void KillQueTimer(u16 wAppId, u16 wInstId, u16 wTimer , void * pTmBlkAddr = NULL );   //add tmBlkAddr
    u64  GetCurrentTick();  
	u64  GetCurrentTickNoSema();
	//���Զ�ʱ
	void *  SetAbsTimer(u16 wAppId, u16 wInstId, u16 wTimer, time_t tAbsTime, u32 dwPara=0x80ffffff);
	BOOL  KillAbsTimer(u16 wAppId, u16 wInstId, u16 wTimer);
	void  RunAbsTimerList();
	BOOL32  InsertAbsTimer(TmBlk* newTimer);

	u64   m_nBaseTick ;
	void  RunTimerList() ;
	void  ReviseBaseTick() ;
	void  FreeTv() ;
	u64    GetOspBaseTick() ;

private: 
	u32 m_nActiveTimerCount ; 
	void  InternalAddTimer(TmBlk *timer, u32 tvwhat = (u32)-1, s32 index = -1) ;
	void  ListAdd(TmBlk *newTimer, TmBlk *prev, TmBlk * next) ;
	void  ListDel(TmBlk *entry) ;
	//void  CascadeTimers( timerVec *tv, u32 tvwhat);
    s32  CascadeTimers( timerVec *tv, u32 tvwhat, s32 index);
	void  FreeTvn(TmBlk *TvnHead) ;
	u32  m_nReorderTimerCount;
	u32  m_nDropTimerCount;
	u32  m_nKilledTimerCount;

    TmBlk* AllocTmBlk();    // �õ�һ��TimeBlock�����ж���
    void ReturnTmBlk(TmBlk* pTmBlk); //�黹һ��TimeBlock�����ж���
};



#endif
