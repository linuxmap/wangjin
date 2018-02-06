/******************************************************************************
ģ����	�� OspSch
�ļ���	�� OspSch.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ���ȹ��ܵ���Ҫ����ͷ�ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
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

//����
#define OSP_ASSERT      assert

//OSP�ڲ�socket��ʼ�˿ں�
const u16 OSP_INER_PORT_BEGIN  =    20010;
const u16 OSP_INER_PORT_END    =    20200;

//osp�����������ȼ�
//��־�������ȼ�
#define OSP_LOG_TASKPRI				(u8)250
//��ʱ���������ȼ�
#define OSP_TIMER_TASKPRI			(u8)40
//������Ϣ�������ȼ�
#define OSP_DISPATCH_TASKPRI		(u8)70
//������Ϣ�������ȼ�
#define OSP_POSTDAEMON_TASKPRI		(u8)70
//Ӧ��������ȼ�
#define APP_TASKPRI_LIMIT			(u8)80
//��������������ȼ�
#define NODE_MAN_APPPRI				(u8)75
//telnet�����������ȼ�
#define OSP_TELEDAEMON_TASKPRI		(u8)70
//telnet�����������ȼ�
#define OSP_TELEECHO_TASKPRI		(u8)70

//��־�������������Ϣ����
#define MAX_LOGMSG_WAITING            2048//1024��2048 by gxk ��NVR64��ӡ��ȫ

//��������������������Ϣ����
#define MAX_NODEMANMSG_WAITING        10 + 2*MAX_NODE_NUM

//osp���������ջ��С
//��־�����ջ��С
#define OSP_LOG_STACKSIZE			(u32)(40<<10)
//��ʱ�������ջ��С
#define OSP_TIMER_STACKSIZE			(u32)(20<<10)
//������Ϣ�����ջ��С
#define OSP_DISPATCH_STACKSIZE		(u32)(40<<10)
//������Ϣ�����ջ��С
#define OSP_POSTDAEMON_STACKSIZE	(u32)(1<<18)
//Ӧ�ö�ջ��С
#define OSP_APP_STACKSIZE			(u32)(200<<10)
//telnet���������ջ��С
#define OSP_TELEECHO_STACKSIZE		(u32)(40<<10)
//telnet���������ջ��С
#define OSP_TELEDAEMON_STACKSIZE	(u32)(20<<10)

#define CALL_INSTANCE_ENTRY 1
#define CALL_DAEMON_INSTANCE_ENTRY 2

//������Ϣ�ṹ
typedef struct tagOSMsg
{
	//������Ϣ��ַ
	void*	address;
}TOsMsgStruc;

//osp���������󳤶�
#define MAX_TASKNAME_LEN			(u32)40

//OSP�����Ҫ��Ϣ
typedef struct
{
	//�����
	u32 id;
	//������
	TASKHANDLE handle;
	//�������
	char name[MAX_TASKNAME_LEN];
}TTaskInfo;

//OSP�����Ҫ��Ϣ���
typedef struct _TTaskNode
{
	//�����Ҫ��Ϣ
	TTaskInfo tTaskInfo;
	//��һ�������Ҫ��Ϣ���
	struct _TTaskNode* next;
}TTaskNode;

//OSP��������
class CTaskList
{
private:
	//osp��������ͷ���
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

	//���������Ƿ�Ϊ��
	BOOL32 IsEmpty(void)
	{
		return ptHead == NULL;
	}

	//��ѯ������
	TTaskInfo* GetFirstTask(void)
	{
		if( ptHead == NULL )
			return NULL;

		return &ptHead->tTaskInfo;
	}

	//��������Ų�ѯ��һ������
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

	//��������
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

	//���������ɾ������
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

/*�ڴ����ģ��*/
/*��ʱ����ջ�Ļ���*/
#define  OSP_TIMER_STACK_MARKER		(u32)0xaeefaeef
/*��Ϣ��ջ�Ļ���*/
//#define  OSP_MSG_STACK_MARKER		(u32)0xbeefbeef
/*��־��ջ�Ļ���*/
//#define  OSP_LOG_STACK_MARKER		(u32)0xceefceef
/*ʵ����ʱ����ջ�Ļ���*/
#define  OSP_INST_TIME_STACK_MARKER	(u32)0xdeefdeef

/*64�ֽڶ�ջ�Ļ���*/
#define  OSP_64_BYTE_STACK_MARKER   (u32)0x1ffd1ffd
/*128�ֽڶ�ջ�Ļ���*/
#define  OSP_128_BYTE_STACK_MARKER  (u32)0x2ffd2ffd
/*256�ֽڶ�ջ�Ļ���*/
#define  OSP_256_BYTE_STACK_MARKER  (u32)0x3ffd3ffd
/*512�ֽڶ�ջ�Ļ���*/
#define  OSP_512_BYTE_STACK_MARKER  (u32)0x4ffd4ffd
/*1k�ֽڶ�ջ�Ļ���*/
#define  OSP_1K_BYTE_STACK_MARKER   (u32)0x5ffd5ffd
/*2k�ֽڶ�ջ�Ļ���*/
#define  OSP_2K_BYTE_STACK_MARKER   (u32)0x6ffd6ffd
/*4k�ֽڶ�ջ�Ļ���*/
#define  OSP_4K_BYTE_STACK_MARKER   (u32)0x7ffd7ffd
/*8k�ֽڶ�ջ�Ļ���*/
#define  OSP_8K_BYTE_STACK_MARKER   (u32)0x8ffd8ffd
/*16k�ֽڶ�ջ�Ļ���*/
#define  OSP_16K_BYTE_STACK_MARKER  (u32)0x9ffd9ffd
/*32k�ֽڶ�ջ�Ļ���*/
#define  OSP_32K_BYTE_STACK_MARKER  (u32)0xaffdaffd
/*64k�ֽڶ�ջ�Ļ���*/
#define  OSP_64K_BYTE_STACK_MARKER  (u32)0xbffdbffd
/*128k�ֽڶ�ջ�Ļ���*/
#define  OSP_128K_BYTE_STACK_MARKER (u32)0xcffdcffd
/*256k�ֽڶ�ջ�Ļ���*/
#define  OSP_256K_BYTE_STACK_MARKER (u32)0xdffddffd
/*512k�ֽڶ�ջ�Ļ���*/
#define  OSP_512K_BYTE_STACK_MARKER (u32)0xeffdeffd
/*1M�ֽڶ�ջ�Ļ���*/
#define  OSP_1M_BYTE_STACK_MARKER   (u32)0xfffdfffd
/*2M�ֽڶ�ջ�Ļ���*/
#define  OSP_2M_BYTE_STACK_MARKER   (u32)0x1ddf1ddf
/*4M�ֽڶ�ջ�Ļ���*/
#define  OSP_4M_BYTE_STACK_MARKER   (u32)0x2ddf2ddf

#define  OSP_TIMER_STACK_BLK_SIZE    sizeof(TmBlk)     /*44Byte*/
//#define  OSP_MSG_STACK_BLK_SIZE   sizeof(CMessage)+ MAX_MSG_LEN+MAX_ALIAS_LEN    /*5294Byte(5K)*/
//#define  OSP_LOG_STACK_BLK_SIZE  sizeof(TOspLogHead)+MAX_LOG_MSG_LEN+1   /*6017Byte(6K)*/
#define  OSP_INST_TIME_STACK_BLK_SIZE sizeof(TInstTimerInfo)     /*12Byte*/

#if 0
typedef  struct  TOspStackHeader
{
	u32     flag;      /*ÿ����ջ�ڴ��Ļ���*/
	u32     bReturn;   /*�Ƿ��Ѿ������գ������һ���ڴ��ظ��ͷ�*/
	void    *  preNode;   /* TospStackNode* */
	void    *  nextNode;  /* TospStackNode* */
}TOspStackHeader;

typedef struct gTOspStackNode
{
	TOspStackHeader  header;    /*ÿ����ջ�ڴ��ͷ*/
	u8      *       msg;       /*ÿ����ջ�ڴ�������*/
}gTOspStackNode;


template< u32 stkBlkSize, u32 stkMarker >
class COspStack
{
public:
	typedef struct TOspStackNode
	{
		TOspStackHeader  header;    /*ÿ����ջ�ڴ��ͷ*/
		u8             msg[stkBlkSize];       /*ÿ����ջ�ڴ�������*/
	}TOspStackNode;

	typedef struct  TOspStackHandle
	{
		u32           size;       /*����ջ��ÿ���ڴ�����ݵĴ�С*/
		TOspStackNode *  topNode;    /*��ջ��ͷ�ڵ�*/
		TOspStackNode *  botNode;    /*��ջ��β�ڵ�*/
	}TOspStackHandle;

	COspStack()
	{
		m_wdStackAvailableBlkNum = 0;
		m_wdStackTotalBlkNum = 0;
		m_tStackHandle.size = 0;
		m_tStackHandle.topNode = NULL;
		m_tStackHandle.botNode = NULL;
	}

	BOOL32    CreateStack(u32  stkBlkNum);     /*��ջ���ڴ��ĸ���*/
	void    DestroyStack(void);
	void    ReturnStack(void * pMsg);     /*���ڴ��Żض�ջ��*/
	void *  GetStack(void);                 /*���ջ�����ڴ��*/

	u32            m_wdStackAvailableBlkNum;   /*��ջ�п��õ��ڴ�����*/
	u32            m_wdStackTotalBlkNum;    /*��ջ���ܵ��ڴ�����*/

private:
	TOspStackHandle   m_tStackHandle;     /*��ջ���*/
	SEMHANDLE         m_tStackSem;        /* ��ջ�ź���*/
};


/**===================    OSP�ڴ����ģ��   =========================
 * ��ʵ�ִ������CPP���Ǻ��ʵģ�����������VC�е�BUG�� Q239436��ֻ�ܽ�
 * ������ʵ�ַ���һ��
 * ��VxWorks�²����ڴ����⡣���ǣ������ܵ���ͷ�ļ��еĴ��롣���ȷ����
 * Ҫ��ֻ����ʱ�ƶ�һ�¡�
 */

template <u32  stkBlkSize, u32  stkMarker>
void  COspStack<stkBlkSize, stkMarker>::DestroyStack(void)
{
	OspSemTake(m_tStackSem);

	/*�ͷŶ�ջ�����п����ڴ��*/
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

	/*��ʼ����ջ�ڴ����У����������û�ָ���ߴ��С���������ڴ���*/
	for( u32 i = 0; i < stkBlkNum; i++)
	{
		TOspStackNode * newNode = (TOspStackNode *)malloc(sizeof(TOspStackNode));
		if(newNode != NULL)
		{
			/*�½��ڵ�ɹ�����ʼ���½��ڵ�������ջ�� */
			newNode->header.preNode = NULL;
			newNode->header.nextNode = NULL;
			newNode->header.flag = stkMarker;  /*���ö�ջ����*/
			newNode->header.bReturn = 0;     /*��λ�ͷű�־*/
			memset(newNode->msg, 0, stkBlkSize);

			ReturnStack((void *)newNode->msg);
			m_wdStackTotalBlkNum++;
		} else {
		/**
		 * �ڴ����ʧ�ܣ�˵��ϵͳ�ڴ������꣬�����������ش���
		 * ֹͣ��һ������������ӡ���澯��Ϣ��
		 */
			int createstack_malloc_err = 0;	// ��������Ϊ����ĸ澯��Ϣ
			OSP_ASSERT( createstack_malloc_err );
			OspLog(1, "COspStack::CreateStack malloc error!\n");

			/* �ͷ��Ѿ�������ڴ� */
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
		OspSemTake(m_tStackSem);    /*�ź�������*/

		/*ȡ�طŻؽڵ��ڴ��ṹ���׵�ַ*/
		TOspStackNode  * retNode = (TOspStackNode  *)((PTR)pMsg - sizeof(TOspStackHeader));
		if(retNode != NULL)
		{
			/*У�鷵�ض�ջ�ڴ��Ļ���*/
			if(retNode->header.flag != stkMarker)
			{
				/* ������ڴ�鲻��ȷ�������Ѿ����𻵣������κδ��� */
				int ospstack_marker_lost = 0;
				OSP_ASSERT( ospstack_marker_lost );
				OspSemGive(m_tStackSem);
				OspLog(1, "ReturnStack() header.flag is illegal\n");
				return;
			}
			if(retNode->header.bReturn == 1)     /*�ڴ���Ѿ����ͷŹ�*/
			{
				OspSemGive(m_tStackSem);
				return;
			}
			retNode->header.bReturn = 1;     /*�����ͷű�־*/
			if(m_tStackHandle.topNode == NULL)  /*��ջΪ��*/
			{
				//ͷ�ڵ㡢β�ڵ�ָ���²�����ڴ��
				m_tStackHandle.botNode = retNode;
				m_tStackHandle.topNode = m_tStackHandle.botNode;
				m_wdStackAvailableBlkNum++;
			}
			else     /*��ջ�����п����ڴ��*/
			{
				/*�嵽����ͷ����ͷ�ڵ�ָ���²�����ڴ��*/
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
	OspSemTake(m_tStackSem);    /*�ź�������*/

	if(m_tStackHandle.topNode != NULL)    /*��ջ���п����ڴ��*/
	{
		TOspStackNode *retNode = m_tStackHandle.topNode;
		m_tStackHandle.topNode = (TOspStackNode *)m_tStackHandle.topNode->header.nextNode;
		if (m_tStackHandle.topNode == NULL)
		{
			/*���һ������ڴ��*/
			m_tStackHandle.botNode = NULL;
		} else {
			m_tStackHandle.topNode->header.preNode = NULL;
		}
		/*У�����*/
		if(retNode->header.flag == stkMarker)
		{
			retNode->header.preNode = NULL;
			retNode->header.nextNode = NULL;
			retNode->header.bReturn = 0;     /*��λ�ͷű�־*/
			m_wdStackAvailableBlkNum--;
			OspSemGive(m_tStackSem);
			return  (void *)retNode->msg;
		}
		else
		{
			int ospstack_marker_lost = 0;
			OSP_ASSERT( ospstack_marker_lost );

			/**
			*  ��ͼ�һ�δ���ƻ����ڴ�飺�Ӷ���β�������ҳ����õ��ڴ��
			*/
			m_wdStackAvailableBlkNum = 0;
			m_tStackHandle.topNode = m_tStackHandle.botNode;

			/* �ȼ�����һ�� */
			if ((m_tStackHandle.topNode != NULL) &&
				(m_tStackHandle.topNode->header.flag == stkMarker))
			{
				m_wdStackAvailableBlkNum++;
				/* ���ż�����еĿ� */
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
	else      /*��ջ��û�п����ڴ�飬�����ȥ�Ļ�û�л���*/
	{   /*�½�һ���ڵ�*/
		TOspStackNode * newNode = (TOspStackNode *)malloc(sizeof(TOspStackNode));
		if(newNode != NULL)
		{
			/*�½��ڵ�ɹ�����ʼ���½��ڵ㣬�������ݵ�ַ*/
			newNode->header.preNode = NULL;
			newNode->header.nextNode = NULL;
			newNode->header.flag = stkMarker;    /*����ʱ�÷Żض�ջ��*/
			newNode->header.bReturn = 0;        /*��λ�ͷű�־*/
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
	u32 dwFlag;                      //У�����
	OSP_MEMSTACKHANDLE hMemStackAddr;//�ڵ�������ջ��ַ
	BOOL32 bReturn;                  //�Ƿ��Ѿ������գ������һ���ڴ��ظ��ͷ�
	void* pvNextNode;                //ָ����һ��_TOspStackNode�ṹ�ڵ�
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
	SEMHANDLE m_hStackSem;      //��ջ�ź���
	u32 m_dwStackMemBlkSize;    //����ջ��ÿ���ڴ�����ݴ�С
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

	/*�����ڴ���*/
	COspStack* m_pcTimerStack;    //��ʱ��ģ���ڴ���
	COspStack* m_pcInstTimeStack;  //ʵ����ʱ��ģ���ڴ���
	COspMemPool m_cOspInerMemPool;
#if 0
	COspTimerStack      m_cTimerStack;    //��ʱ��ģ���ڴ���
//	COspMsgStack        m_cMsgStack;      //��Ϣģ���ڴ���
//	COspLogStack        m_cLogStack;      //��־ģ���ڴ���
	COspInstTimeStack   m_cInstTimeStack; //ʵ����ʱ��ģ���ڴ���

	/*��Сƥ���ڴ���*/
	COsp64ByteStack    m_c64BStack;    //64�ֽ��û��ڴ���
	COsp128ByteStack   m_c128BStack;   //128�ֽ��û��ڴ���
	COsp256ByteStack   m_c256BStack;   //256�ֽ��û��ڴ���
	COsp512ByteStack   m_c512BStack;   //512�ֽ��û��ڴ���
	COsp1KByteStack    m_c1KBStack;    //1K�ֽ��û��ڴ���
	COsp2KByteStack    m_c2KBStack;    //2K�ֽ��û��ڴ���
	COsp4KByteStack    m_c4KBStack;    //4K�ֽ��û��ڴ���
	COsp8KByteStack    m_c8KBStack;    //8K�ֽ��û��ڴ���
	COsp16KByteStack   m_c16KBStack;    //16K�ֽ��û��ڴ���
	COsp32KByteStack   m_c32KBStack;    //32K�ֽ��û��ڴ���
	COsp64KByteStack   m_c64KBStack;    //64K�ֽ��û��ڴ���
	COsp128KByteStack  m_c128KBStack;   //128K�ֽ��û��ڴ���
	COsp256KByteStack  m_c256KBStack;   //256K�ֽ��û��ڴ���
	COsp512KByteStack  m_c512KBStack;   //512K�ֽ��û��ڴ���
	COsp1MByteStack    m_c1MBStack;    //1M�ֽ��û��ڴ���
	COsp2MByteStack    m_c2MBStack;    //2M�ֽ��û��ڴ���
	COsp4MByteStack    m_c4MBStack;    //4M�ֽ��û��ڴ���
#endif //if 0
	/*�ڴ��ַ���*/
	u32 *  m_pdwCheckMemAddr;    //Ҫ�����ڴ��ַ
	u32    m_dwCheckValue;      //Ҫ����ֵ
	BOOL32  m_bEqual;          //Ҫ�����ȷ񲻵�
	char  m_chBeforErrThread[50];    //����ǰ���߳���
	char  m_chErrCurrentThread[50];  //����ʱ�ĵ�ǰ�߳���
	BOOL32  m_bErrArised;      //����澯��־

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
