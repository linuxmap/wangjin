/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP 操作系统封装功能的主要包含头文件 
作者	：张文江
版本	：1.0.02.7.5
-------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98	1.0         某某        ------------
02/10/2003  1.1         向飞        将有关信号量操作的函数移到osp.h中。
******************************************************************************/
#ifndef INC_OSPVOS_H
#define INC_OSPVOS_H

#include "osp.h"

//本地最大外部结点数

extern u32 MAX_NODE_NUM;

#define  THREADAFFMASK	            1

	#define SOCK_SEND_FLAGS             (int)0

//发送消息任务邮箱最大消息容量
extern u32 MAX_DISPATCHMSG_WAITING;

//延时操作
API void OspTaskDelay(u32 dwMseconds);
//取得邮箱中待处理的消息数
API u32 OspMBAvailMsgs(MAILBOXID tReadID, u32 dwMsgLen);
//TCP发送
API BOOL32 SockSend(SOCKHANDLE sock,const char * buf, u32 len);
//TCP接收
API BOOL32 SockRecv(SOCKHANDLE sock, char * buf, u32 len, u32 * pRcvLen);


#define OSP_LB_MAGCI_NUM			0x88AB1435

#define		BUFF_START				1	
#define		BUFF_IN_WRITE			2
#define		BUFF_IN_READ			3
#define		BUFF_IN_CLOSE			4
#define		BUFF_IN_RESET			5

typedef struct osplb_data_real
{
	u32				m_dwMagic;
	
	s32				m_nBuffSize ;
	s32				m_nMaxUnitSize;
	
	u8 *			m_pbyBuf;
	
	volatile s32	m_nReadPos;
    volatile s32	m_nWritePos;
	
	s32				m_nUnitNum;		// useless
	
	volatile	s32	m_start;
	volatile	s32 m_write;
	volatile	s32 m_read ;
	volatile	s32	m_close;
	volatile	s32	m_reset;
	
	u8*				m_lock;		// useless
	u8				m_byRaceLvl;
	
} osplb_data_t;		/* MODIFY LB_DATA_SIZE WHEN YOU MODIFY THIS STRUCTURE !!!!!*/


#define  charcpy(dst, src, count)	\
	do								\
{								\
	s16 i;						\
	for (i = 0; i < (count); i ++)	\
	*((dst)+i) = *((src)+i);	\
	}while(0)



#define SAFE_DEL(x)	\
	if ( NULL != (x) )	\
{						\
	delete (x);		\
	(x) = NULL ;		\
}


#define SAFE_DELA(x)	\
	if ( NULL != (x) )	\
{						\
	delete [](x);		\
	(x) = NULL ;		\
}

#define SAFE_FREE(x)	\
	if ( NULL != (x) )	\
{							\
	free((x));			\
	(x) = NULL ;		\
}

#endif	//INC_OSPVOS_H
