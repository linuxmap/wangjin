/******************************************************************************
模块名	： OspSch
文件名	： OspSch.h
相关文件：
文件实现功能：OSP 调度功能的主要包含头文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98		1.0      某某        ------------
******************************************************************************/

#ifndef OSP_SCH_H
#define OSP_SCH_H
#include "osp.h"
#include "../include/ospvos.h"
#include "../include/osplog.h"
#include "../include/osppost.h"
#include "../include/osptimer.h"

#if defined(_WIN64) || defined(WIN64) ||defined (__LP64__) || defined (__64BIT__) || defined (_LP64) || (__WORDSIZE == 64)
    typedef u64      OSPPTR;
#else
    typedef u32      OSPPTR;
#endif

//断言
#define OSP_ASSERT      assert

//OSP内部socket起始端口号
const u16 OSP_INER_PORT_BEGIN  =    20010;
const u16 OSP_INER_PORT_END    =    20200;

//osp服务任务优先级
//日志任务优先级
#define OSP_LOG_TASKPRI				(u8)250
//定时器任务优先级
#define OSP_TIMER_TASKPRI			(u8)40
//发送消息任务优先级
#define OSP_DISPATCH_TASKPRI		(u8)70
//接收消息任务优先级
#define OSP_POSTDAEMON_TASKPRI		(u8)70
//应用最高优先级
#define APP_TASKPRI_LIMIT			(u8)80
//断链检测任务优先级
#define NODE_MAN_APPPRI				(u8)75
//telnet监听任务优先级
#define OSP_TELEDAEMON_TASKPRI		(u8)70
//telnet服务任务优先级
#define OSP_TELEECHO_TASKPRI		(u8)70

//日志任务邮箱最大消息容量
#define MAX_LOGMSG_WAITING            2048//1024→2048 by gxk 接NVR64打印不全

//断链检测任务邮箱最大消息容量
#define MAX_NODEMANMSG_WAITING        10 + 2*MAX_NODE_NUM

//osp服务任务堆栈大小
//日志任务堆栈大小
#define OSP_LOG_STACKSIZE			(u32)(40<<10)
//定时器任务堆栈大小
#define OSP_TIMER_STACKSIZE			(u32)(20<<10)
//发送消息任务堆栈大小
#define OSP_DISPATCH_STACKSIZE		(u32)(40<<10)
//接收消息任务堆栈大小
#define OSP_POSTDAEMON_STACKSIZE	(u32)(1<<18)
//应用堆栈大小
#define OSP_APP_STACKSIZE			(u32)(200<<10)
//telnet服务任务堆栈大小
#define OSP_TELEECHO_STACKSIZE		(u32)(40<<10)
//telnet监听任务堆栈大小
#define OSP_TELEDAEMON_STACKSIZE	(u32)(20<<10)

#define CALL_INSTANCE_ENTRY 1
#define CALL_DAEMON_INSTANCE_ENTRY 2

//邮箱消息结构
typedef struct tagOSMsg
{
	//邮箱消息地址
	void*	address;
}TOsMsgStruc;

//osp任务别名最大长度
#define MAX_TASKNAME_LEN			(u32)40

//OSP任务简要信息
typedef struct
{
	//任务号
	u32 id;
	//任务句柄
	TASKHANDLE handle;
	//任务别名
	char name[MAX_TASKNAME_LEN];
}TTaskInfo;

//OSP任务简要信息结点
typedef struct _TTaskNode
{
	//任务简要信息
	TTaskInfo tTaskInfo;
	//下一个任务简要信息结点
	struct _TTaskNode* next;
}TTaskNode;

//OSP任务链表
class CTaskList
{
private:
	//osp任务链表头结点
	TTaskNode *ptHead;

public:
	CTaskList(void)
	{
		ptHead = NULL;
	}

	~CTaskList(void)
	{
		TTaskNode *curNode=ptHead;
		TTaskNode *nextNode=NULL;

		while( curNode != NULL )
		{
			nextNode = curNode->next;
			OspFreeMem(curNode);
			curNode = nextNode;
		}
		ptHead = NULL;
	}

	//任务链表是否为空
	BOOL32 IsEmpty(void)
	{
		return ptHead == NULL;
	}

	//查询首任务
	TTaskInfo* GetFirstTask(void)
	{
		if( ptHead == NULL )
			return NULL;

		return &ptHead->tTaskInfo;
	}

	//根据任务号查询下一个任务
	TTaskInfo* GetNextTask( u32 curTaskID )
	{
		TTaskNode *curNode = ptHead;

		while( curNode != NULL )
		{
			if( ( curNode->tTaskInfo.id == curTaskID ) &&
				( curNode->next != NULL ) )
				return &curNode->next->tTaskInfo;

			curNode = curNode->next;
		}

		return NULL;
	}

	//加入任务
	BOOL32 AddTask(TASKHANDLE hTask, u32 taskID, const char* nameStr)
	{
		if( ( nameStr != NULL ) &&
			( strlen(nameStr) >= MAX_TASKNAME_LEN ) )
		{
			return FALSE;
		}
		TTaskNode *ptTaskNode = (TTaskNode *)OspAllocMem(sizeof(TTaskNode));
		ptTaskNode->tTaskInfo.handle = hTask;
		ptTaskNode->tTaskInfo.id = taskID;
		strcpy( ptTaskNode->tTaskInfo.name, nameStr );
		ptTaskNode->next = ptHead;
		ptHead = ptTaskNode;
		return TRUE;
	}

	//根据任务号删除任务
	void DelTask( u32 taskID )
	{
		TTaskNode *preNode = NULL;
		TTaskNode *curNode = ptHead;

		while( curNode != NULL )
		{
			if( curNode->tTaskInfo.id == taskID )
			{
				if( curNode == ptHead )
				{
					ptHead = ptHead->next;
				}
				else
				{
					preNode->next = curNode->next;
				}
				OspFreeMem(curNode);
				break;
			}
			preNode = curNode;
			curNode = curNode->next;
		}
	}
};

/*内存管理模块*/
/*定时器堆栈的幻数*/
#define  OSP_TIMER_STACK_MARKER		(u32)0xaeefaeef
/*消息堆栈的幻数*/
//#define  OSP_MSG_STACK_MARKER		(u32)0xbeefbeef
/*日志堆栈的幻数*/
//#define  OSP_LOG_STACK_MARKER		(u32)0xceefceef
/*实例定时器堆栈的幻数*/
#define  OSP_INST_TIME_STACK_MARKER	(u32)0xdeefdeef

/*64字节堆栈的幻数*/
#define  OSP_64_BYTE_STACK_MARKER   (u32)0x1ffd1ffd
/*128字节堆栈的幻数*/
#define  OSP_128_BYTE_STACK_MARKER  (u32)0x2ffd2ffd
/*256字节堆栈的幻数*/
#define  OSP_256_BYTE_STACK_MARKER  (u32)0x3ffd3ffd
/*512字节堆栈的幻数*/
#define  OSP_512_BYTE_STACK_MARKER  (u32)0x4ffd4ffd
/*1k字节堆栈的幻数*/
#define  OSP_1K_BYTE_STACK_MARKER   (u32)0x5ffd5ffd
/*2k字节堆栈的幻数*/
#define  OSP_2K_BYTE_STACK_MARKER   (u32)0x6ffd6ffd
/*4k字节堆栈的幻数*/
#define  OSP_4K_BYTE_STACK_MARKER   (u32)0x7ffd7ffd
/*8k字节堆栈的幻数*/
#define  OSP_8K_BYTE_STACK_MARKER   (u32)0x8ffd8ffd
/*16k字节堆栈的幻数*/
#define  OSP_16K_BYTE_STACK_MARKER  (u32)0x9ffd9ffd
/*32k字节堆栈的幻数*/
#define  OSP_32K_BYTE_STACK_MARKER  (u32)0xaffdaffd
/*64k字节堆栈的幻数*/
#define  OSP_64K_BYTE_STACK_MARKER  (u32)0xbffdbffd
/*128k字节堆栈的幻数*/
#define  OSP_128K_BYTE_STACK_MARKER (u32)0xcffdcffd
/*256k字节堆栈的幻数*/
#define  OSP_256K_BYTE_STACK_MARKER (u32)0xdffddffd
/*512k字节堆栈的幻数*/
#define  OSP_512K_BYTE_STACK_MARKER (u32)0xeffdeffd
/*1M字节堆栈的幻数*/
#define  OSP_1M_BYTE_STACK_MARKER   (u32)0xfffdfffd
/*2M字节堆栈的幻数*/
#define  OSP_2M_BYTE_STACK_MARKER   (u32)0x1ddf1ddf
/*4M字节堆栈的幻数*/
#define  OSP_4M_BYTE_STACK_MARKER   (u32)0x2ddf2ddf

#define  OSP_TIMER_STACK_BLK_SIZE    sizeof(TmBlk)     /*44Byte*/
//#define  OSP_MSG_STACK_BLK_SIZE   sizeof(CMessage)+ MAX_MSG_LEN+MAX_ALIAS_LEN    /*5294Byte(5K)*/
//#define  OSP_LOG_STACK_BLK_SIZE  sizeof(TOspLogHead)+MAX_LOG_MSG_LEN+1   /*6017Byte(6K)*/
#define  OSP_INST_TIME_STACK_BLK_SIZE sizeof(TInstTimerInfo)     /*12Byte*/

#if 0
typedef  struct  TOspStackHeader
{
	u32     flag;      /*每个堆栈内存块的幻数*/
	u32     bReturn;   /*是否已经被回收，避免对一块内存重复释放*/
	void    *  preNode;   /* TospStackNode* */
	void    *  nextNode;  /* TospStackNode* */
}TOspStackHeader;

typedef struct gTOspStackNode
{
	TOspStackHeader  header;    /*每个堆栈内存块头*/
	u8      *       msg;       /*每个堆栈内存块的内容*/
}gTOspStackNode;


template< u32 stkBlkSize, u32 stkMarker >
class COspStack
{
public:
	typedef struct TOspStackNode
	{
		TOspStackHeader  header;    /*每个堆栈内存块头*/
		u8             msg[stkBlkSize];       /*每个堆栈内存块的内容*/
	}TOspStackNode;

	typedef struct  TOspStackHandle
	{
		u32           size;       /*本堆栈中每个内存块内容的大小*/
		TOspStackNode *  topNode;    /*堆栈的头节点*/
		TOspStackNode *  botNode;    /*堆栈的尾节点*/
	}TOspStackHandle;

	COspStack()
	{
		m_wdStackAvailableBlkNum = 0;
		m_wdStackTotalBlkNum = 0;
		m_tStackHandle.size = 0;
		m_tStackHandle.topNode = NULL;
		m_tStackHandle.botNode = NULL;
	}

	BOOL32    CreateStack(u32  stkBlkNum);     /*堆栈中内存块的个数*/
	void    DestroyStack(void);
	void    ReturnStack(void * pMsg);     /*把内存块放回堆栈中*/
	void *  GetStack(void);                 /*向堆栈申请内存块*/

	u32            m_wdStackAvailableBlkNum;   /*堆栈中可用的内存块个数*/
	u32            m_wdStackTotalBlkNum;    /*堆栈中总的内存块个数*/

private:
	TOspStackHandle   m_tStackHandle;     /*堆栈句柄*/
	SEMHANDLE         m_tStackSem;        /* 堆栈信号量*/
};


/**===================    OSP内存管理模块   =========================
 * 将实现代码放在CPP中是合适的，不过，由于VC中的BUG－ Q239436，只能将
 * 定义与实现放在一起。
 * 在VxWorks下不存在此问题。但是，它不能调试头文件中的代码。如果确定需
 * 要，只好临时移动一下。
 */

template <u32  stkBlkSize, u32  stkMarker>
void  COspStack<stkBlkSize, stkMarker>::DestroyStack(void)
{
	OspSemTake(m_tStackSem);

	/*释放堆栈中所有空闲内存块*/
	while((m_tStackHandle.topNode != NULL) &&
		(m_tStackHandle.topNode->header.flag == stkMarker))
	{
		TOspStackNode * delNode = m_tStackHandle.topNode;
		if(delNode != NULL)
		{
			m_tStackHandle.topNode = (TOspStackNode *)delNode->header.nextNode;
		}
		free(delNode);
		delNode = NULL;
	}
	if (m_tStackHandle.topNode != NULL)
	{
		int ospstack_marker_lost = 0;
		OSP_ASSERT( ospstack_marker_lost );
	}
	m_tStackHandle.topNode = NULL;
	m_tStackHandle.botNode = NULL;
	m_wdStackTotalBlkNum = 0;
	m_wdStackAvailableBlkNum = 0;
	OspSemDelete(m_tStackSem);
}

template <u32  stkBlkSize, u32  stkMarker>
BOOL32  COspStack<stkBlkSize, stkMarker>::CreateStack(u32  stkBlkNum)
{
	OspSemBCreate(&m_tStackSem);

	m_wdStackAvailableBlkNum = 0;
	m_wdStackTotalBlkNum = 0;
	m_tStackHandle.size = 0;
	m_tStackHandle.topNode = NULL;
	m_tStackHandle.botNode = NULL;

	/*初始化堆栈内存块队列，事先申请用户指定尺寸大小、个数的内存区*/
	for( u32 i = 0; i < stkBlkNum; i++)
	{
		TOspStackNode * newNode = (TOspStackNode *)malloc(sizeof(TOspStackNode));
		if(newNode != NULL)
		{
			/*新建节点成功，初始化新建节点柄放入堆栈中 */
			newNode->header.preNode = NULL;
			newNode->header.nextNode = NULL;
			newNode->header.flag = stkMarker;  /*填充好堆栈幻数*/
			newNode->header.bReturn = 0;     /*复位释放标志*/
			memset(newNode->msg, 0, stkBlkSize);

			ReturnStack((void *)newNode->msg);
			m_wdStackTotalBlkNum++;
		} else {
		/**
		 * 内存分配失败，说明系统内存已用完，或者其他严重错误。
		 * 停止进一步操作，并打印出告警信息。
		 */
			int createstack_malloc_err = 0;	// 变量名作为出错的告警信息
			OSP_ASSERT( createstack_malloc_err );
			OspLog(1, "COspStack::CreateStack malloc error!\n");

			/* 释放已经申请的内存 */
			DestroyStack();
			return FALSE;
		}
	}
	return TRUE;
}

template <u32  stkBlkSize, u32  stkMarker>
void COspStack<stkBlkSize, stkMarker>::ReturnStack(void * pMsg)
{
	if(pMsg != NULL)
	{
		OspSemTake(m_tStackSem);    /*信号量保护*/

		/*取回放回节点内存块结构的首地址*/
		TOspStackNode  * retNode = (TOspStackNode  *)((PTR)pMsg - sizeof(TOspStackHeader));
		if(retNode != NULL)
		{
			/*校验返回堆栈内存块的幻数*/
			if(retNode->header.flag != stkMarker)
			{
				/* 传入的内存块不正确，或者已经被损坏，不做任何处理 */
				int ospstack_marker_lost = 0;
				OSP_ASSERT( ospstack_marker_lost );
				OspSemGive(m_tStackSem);
				OspLog(1, "ReturnStack() header.flag is illegal\n");
				return;
			}
			if(retNode->header.bReturn == 1)     /*内存块已经被释放过*/
			{
				OspSemGive(m_tStackSem);
				return;
			}
			retNode->header.bReturn = 1;     /*置上释放标志*/
			if(m_tStackHandle.topNode == NULL)  /*堆栈为空*/
			{
				//头节点、尾节点指向新插入的内存块
				m_tStackHandle.botNode = retNode;
				m_tStackHandle.topNode = m_tStackHandle.botNode;
				m_wdStackAvailableBlkNum++;
			}
			else     /*堆栈中已有空闲内存块*/
			{
				/*插到队列头部，头节点指向新插入的内存块*/
				m_tStackHandle.topNode->header.preNode = retNode;
				retNode->header.nextNode = m_tStackHandle.topNode;
				m_tStackHandle.topNode = retNode;
				m_tStackHandle.topNode->header.preNode = NULL;
				m_wdStackAvailableBlkNum++;
			}
		}
		OspSemGive(m_tStackSem);
	}
}

//template <u32  stkBlkSize, u32  stkMarker>
//void* COspStack<stkBlkSize, stkMarker>::GetStack(void)
template <u32  stkBlkSize, u32  stkMarker>
void* COspStack<stkBlkSize, stkMarker>::GetStack(void)
{
	OspSemTake(m_tStackSem);    /*信号量保护*/

	if(m_tStackHandle.topNode != NULL)    /*堆栈中有空闲内存块*/
	{
		TOspStackNode *retNode = m_tStackHandle.topNode;
		m_tStackHandle.topNode = (TOspStackNode *)m_tStackHandle.topNode->header.nextNode;
		if (m_tStackHandle.topNode == NULL)
		{
			/*最后一块空闲内存块*/
			m_tStackHandle.botNode = NULL;
		} else {
			m_tStackHandle.topNode->header.preNode = NULL;
		}
		/*校验幻数*/
		if(retNode->header.flag == stkMarker)
		{
			retNode->header.preNode = NULL;
			retNode->header.nextNode = NULL;
			retNode->header.bReturn = 0;     /*复位释放标志*/
			m_wdStackAvailableBlkNum--;
			OspSemGive(m_tStackSem);
			return  (void *)retNode->msg;
		}
		else
		{
			int ospstack_marker_lost = 0;
			OSP_ASSERT( ospstack_marker_lost );

			/**
			*  试图找回未被破坏的内存块：从队列尾部查起，找出可用的内存块
			*/
			m_wdStackAvailableBlkNum = 0;
			m_tStackHandle.topNode = m_tStackHandle.botNode;

			/* 先检查最后一块 */
			if ((m_tStackHandle.topNode != NULL) &&
				(m_tStackHandle.topNode->header.flag == stkMarker))
			{
				m_wdStackAvailableBlkNum++;
				/* 倒着检查所有的块 */
				while ( (m_tStackHandle.topNode->header.preNode != NULL) &&
					(((TOspStackNode *)(m_tStackHandle.topNode->header.preNode))->header.flag == stkMarker))
				{
					m_wdStackAvailableBlkNum++;
					m_tStackHandle.topNode = (TOspStackNode *)(m_tStackHandle.topNode->header.preNode);
				}

			} else {
				m_tStackHandle.topNode = NULL;
				m_tStackHandle.botNode = NULL;
			}
			OspSemGive(m_tStackSem);
			OspLog(1, "GetStack() TopNode's header.flag is illegal\n");
			return NULL;
		}
	}
	else      /*堆栈中没有空闲内存块，分配出去的还没有回收*/
	{   /*新建一个节点*/
		TOspStackNode * newNode = (TOspStackNode *)malloc(sizeof(TOspStackNode));
		if(newNode != NULL)
		{
			/*新建节点成功，初始化新建节点，返回内容地址*/
			newNode->header.preNode = NULL;
			newNode->header.nextNode = NULL;
			newNode->header.flag = stkMarker;    /*返回时好放回堆栈中*/
			newNode->header.bReturn = 0;        /*复位释放标志*/
			memset(newNode->msg, 0, stkBlkSize);

			m_wdStackTotalBlkNum++;
			OspSemGive(m_tStackSem);
			return  (void *)newNode->msg;
		}
		else
		{
			int ospstack_malloc_err = 0;
			OSP_ASSERT( ospstack_malloc_err );
			OspSemGive(m_tStackSem);
			//			OspPrintf(TRUE, TRUE, "GetStack() malloc failure\n");
			OspLog(1, "GetStack() malloc failure\n");
			return NULL;
		}
	}
}


typedef COspStack<OSP_TIMER_STACK_BLK_SIZE, OSP_TIMER_STACK_MARKER>         COspTimerStack;
//typedef COspStack<OSP_MSG_STACK_BLK_SIZE>       COspMsgStack;
//typedef COspStack<OSP_LOG_STACK_BLK_SIZE>       COspLogStack;
typedef COspStack<OSP_INST_TIME_STACK_BLK_SIZE, OSP_INST_TIME_STACK_MARKER> COspInstTimeStack;

typedef COspStack<64, OSP_64_BYTE_STACK_MARKER>            COsp64ByteStack;
typedef COspStack<128, OSP_128_BYTE_STACK_MARKER>          COsp128ByteStack;
typedef COspStack<256, OSP_256_BYTE_STACK_MARKER>          COsp256ByteStack;
typedef COspStack<512, OSP_512_BYTE_STACK_MARKER>          COsp512ByteStack;
typedef COspStack<1024, OSP_1K_BYTE_STACK_MARKER>          COsp1KByteStack;
typedef COspStack<2*1024, OSP_2K_BYTE_STACK_MARKER>        COsp2KByteStack;
typedef COspStack<4*1024, OSP_4K_BYTE_STACK_MARKER>        COsp4KByteStack;
typedef COspStack<8*1024, OSP_8K_BYTE_STACK_MARKER>        COsp8KByteStack;
typedef COspStack<16*1024, OSP_16K_BYTE_STACK_MARKER>      COsp16KByteStack;
typedef COspStack<32*1024, OSP_32K_BYTE_STACK_MARKER>      COsp32KByteStack;
typedef COspStack<64*1024, OSP_64K_BYTE_STACK_MARKER>      COsp64KByteStack;
typedef COspStack<128*1024, OSP_128K_BYTE_STACK_MARKER>    COsp128KByteStack;
typedef COspStack<256*1024, OSP_256K_BYTE_STACK_MARKER>    COsp256KByteStack;
typedef COspStack<512*1024, OSP_512K_BYTE_STACK_MARKER>    COsp512KByteStack;
typedef COspStack<1024*1024, OSP_1M_BYTE_STACK_MARKER>     COsp1MByteStack;
typedef COspStack<2*1024*1024, OSP_2M_BYTE_STACK_MARKER>   COsp2MByteStack;
typedef COspStack<4*1024*1024, OSP_4M_BYTE_STACK_MARKER>   COsp4MByteStack;

#endif //if 0

//===================================================================================================================//
#define OSP_MEMSTACKHANDLE     void*
#define   OSP_MEMPOOL_GENERAL 16
#define   OSP_MEMPOOL_EXTERN 16
#define OSP_MEM_POOL_CONTAINER 32


enum EMEMSTACKSIZE{
	STACK_64B       = 0,
	STACK_128B      = 1,
	STACK_256B,
	STACK_512B,
	STACK_1K,
	STACK_2K,
	STACK_4K,
	STACK_8K,
	STACK_16K,
	STACK_32K,
	STACK_64K,
	STACK_128K,
	STACK_256K,
	STACK_512K,
	STACK_1M,
	STACK_2M,
};

typedef struct tagOspStackNodeHeader
{
	u32 dwFlag;                      //校验幻数
	OSP_MEMSTACKHANDLE hMemStackAddr;//节点所属堆栈地址
	BOOL32 bReturn;                  //是否已经被回收，避免对一块内存重复释放
	void* pvNextNode;                //指向下一个_TOspStackNode结构节点
}TOspStackNodeHeadr;

#pragma pack(push,1)
typedef struct
{
	TOspStackNodeHeadr tHeader;
	void* pBuffStart;
}_TOspStackNode;
#pragma pack(pop)

#define OSP_MEMPOOLHANDLE       void*
class COspStack
{
public:
	COspStack(u32 dwStackBlkSize,u32 dwStackMark)
	{
		m_dwStackAvailbleBlkNum = 0;
		m_dwdStackTotalBlkNum = 0;
		m_hStackSem = NULL;
		m_dwStackMemBlkSize = dwStackBlkSize;
		m_dwStackMark = dwStackMark;
		m_dwStackAvailbleBlkPercentUpperLimit = 1;
		m_ptTopNode = NULL;
		OspSemBCreate(&m_hStackSem);
	}
	~COspStack()
	{
		m_dwStackAvailbleBlkNum = 0;
		m_dwdStackTotalBlkNum = 0;
		m_dwStackAvailbleBlkPercentUpperLimit = 1;
		OspSemDelete(m_hStackSem);
		m_hStackSem = NULL;
	}
	BOOL32 StackCreate(u32 dwStackInitBlkNum);
	BOOL32 StackDestroy();
	void*  StackAlloc();
	void   StackReturn(void* pMsg);
	void   StackStateShow();
	u32 m_dwStackAvailbleBlkNum;
	u32 m_dwdStackTotalBlkNum;
	u32 m_dwStackAvailbleBlkPercentUpperLimit;
private:
	SEMHANDLE m_hStackSem;      //堆栈信号量
	u32 m_dwStackMemBlkSize;    //本堆栈中每个内存块内容大小
	u32 m_dwStackMark;
	_TOspStackNode* m_ptTopNode;

};

class COspMemPool
{
public:
	COspMemPool()
	{
		m_bInitFlag = FALSE;
		m_bDestoryFlag = FALSE;
		memset(&m_tOspMemPoolPara,0,sizeof(TOspMemPoolPara));
		m_tOspMemPoolPara.dwLargeBlockLevelGrowSize = 0;
		m_tOspMemPoolPara.dwMaxFreePercent = 100;
		memset(m_apCOspStack,0,sizeof(m_apCOspStack));
		OspSemBCreate(&m_hMemPoolSem);
	}
	~COspMemPool()
	{
		OspSemDelete(m_hMemPoolSem);
		memset(m_apCOspStack,0,sizeof(m_apCOspStack));
		memset(&m_tOspMemPoolPara,0,sizeof(TOspMemPoolPara));
		m_bDestoryFlag = TRUE;
		m_bInitFlag = FALSE;
	}
	BOOL32 OspMemPoolInit(TOspMemPoolPara *ptMemPoolPara);
	BOOL32 IsOspMemPoolInit();
	BOOL32 OspMemPoolDestory();
	void*  OspMemPoolAlloc(u32 dwSize);
	void   OspMemPoolFree();
	void   OspMemPooLState();
	TOspMemPoolPara m_tOspMemPoolPara;
private:
	SEMHANDLE m_hMemPoolSem;
	BOOL32 m_bInitFlag;
	BOOL32 m_bDestoryFlag;
	COspStack* m_apCOspStack[OSP_MEMPOOL_GENERAL + OSP_MEMPOOL_EXTERN];
};
#define INOUT


//===================================================================================================================//


class COsp
{
public:
    CAppPool m_cAppPool; // application pool
    CDispatchPool m_cDispatchPool; // dispatch task pool
	CNodePool m_cNodePool;   // node Pool
	//COspLog m_cOspLog; // osplog Task
	TmListQue m_cTmListQue; // timer queue
	CTaskList m_cTaskList; // service tasks list

    BOOL32 m_bBlock; // whether the system is a blocking system
	BOOL32 m_bKillOsp; // whether user called OspQuit
	BOOL32 m_bInitd; // whether user called OspInit

    COspAppDesc m_cOspAppDesc; // application's descriptiong
    COspEventDesc m_cOspEventDesc; // event description pool
	BOOL32 m_bStatusPrtEnable;  // enable status message output every times
	BOOL32 m_bCmdFuncEnable; // enable Osp functions be called on command line

	SEMHANDLE m_tSyncSema; // semaphore used to sync communication
	SEMHANDLE m_tMutexSema; // semaphore used to protect global sync ack
	BOOL32 m_bSyncAckExpired; // flag for global sync timeout
	u16 m_wSyncAckLen; // global syncack length
	u8 m_achSyncAck[MAX_MSG_LEN]; // global syncack content
    char m_achShellPrompt[MAX_PROMPT_LEN]; // shell prompt

	/*定长内存区*/
	COspStack* m_pcTimerStack;    //定时器模块内存区
	COspStack* m_pcInstTimeStack;  //实例定时器模块内存区
	COspMemPool m_cOspInerMemPool;
#if 0
	COspTimerStack      m_cTimerStack;    //定时器模块内存区
//	COspMsgStack        m_cMsgStack;      //消息模块内存区
//	COspLogStack        m_cLogStack;      //日志模块内存区
	COspInstTimeStack   m_cInstTimeStack; //实例定时器模块内存区

	/*最小匹配内存区*/
	COsp64ByteStack    m_c64BStack;    //64字节用户内存区
	COsp128ByteStack   m_c128BStack;   //128字节用户内存区
	COsp256ByteStack   m_c256BStack;   //256字节用户内存区
	COsp512ByteStack   m_c512BStack;   //512字节用户内存区
	COsp1KByteStack    m_c1KBStack;    //1K字节用户内存区
	COsp2KByteStack    m_c2KBStack;    //2K字节用户内存区
	COsp4KByteStack    m_c4KBStack;    //4K字节用户内存区
	COsp8KByteStack    m_c8KBStack;    //8K字节用户内存区
	COsp16KByteStack   m_c16KBStack;    //16K字节用户内存区
	COsp32KByteStack   m_c32KBStack;    //32K字节用户内存区
	COsp64KByteStack   m_c64KBStack;    //64K字节用户内存区
	COsp128KByteStack  m_c128KBStack;   //128K字节用户内存区
	COsp256KByteStack  m_c256KBStack;   //256K字节用户内存区
	COsp512KByteStack  m_c512KBStack;   //512K字节用户内存区
	COsp1MByteStack    m_c1MBStack;    //1M字节用户内存区
	COsp2MByteStack    m_c2MBStack;    //2M字节用户内存区
	COsp4MByteStack    m_c4MBStack;    //4M字节用户内存区
#endif //if 0
	/*内存地址检查*/
	u32 *  m_pdwCheckMemAddr;    //要检查的内存地址
	u32    m_dwCheckValue;      //要检查的值
	BOOL32  m_bEqual;          //要检查相等否不等
	char  m_chBeforErrThread[50];    //出错前的线程明
	char  m_chErrCurrentThread[50];  //出错时的当前线程名
	BOOL32  m_bErrArised;      //出错告警标志

public:
    COsp()
    {
		OspSemBCreate(&m_tMutexSema);
		OspSemCCreate(&m_tSyncSema, 0, 1);

        m_bBlock = TRUE;
		m_bInitd = FALSE;
		m_bKillOsp = FALSE;
		m_bStatusPrtEnable = FALSE;
		m_bCmdFuncEnable = FALSE;
		m_pcTimerStack = NULL;
		m_pcInstTimeStack = NULL;

		memset(m_achSyncAck, 0, sizeof(m_achSyncAck));
		memset(&m_cTaskList, 0, sizeof(m_cTaskList));

		m_bSyncAckExpired = FALSE;
		m_wSyncAckLen = 0;

		memset(m_achShellPrompt, 0, MAX_PROMPT_LEN);
   }

	~COsp()
	{
		OspSemDelete(m_tMutexSema);
		OspSemDelete(m_tSyncSema);
	}

	BOOL32 IsTaskListEmpty(void)
	{
		return m_cTaskList.IsEmpty();
	}

	TTaskInfo* GetFirstTask(void)
	{
		return m_cTaskList.GetFirstTask();
	}

	TTaskInfo* GetNextTask(u32 curTaskID)
	{
		return m_cTaskList.GetNextTask(curTaskID);
	}

	void AddTask(TASKHANDLE taskHandle, u32 taskID, const char* nameStr)
	{
		m_cTaskList.AddTask(taskHandle, taskID, nameStr);
	}

	void DelTask(u32 taskID)
	{
		m_cTaskList.DelTask(taskID);
	}
};

extern COsp g_Osp;

API void OspShellStart();
API void OspShellExit();

#endif	//OSP_SCH_H
