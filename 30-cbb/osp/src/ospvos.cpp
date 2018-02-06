/******************************************************************************
模块名  ： OSP
文件名  ： OSP.h
相关文件：
文件实现功能：OSP  操作系统封装的主要实现文件
作者    ：张文江
版本    ：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
06/03/2003  2.1         向  飞      规范化
******************************************************************************/
#include "ospsch.h"
#include "ospvos.h"
#include "stdio.h"

u32 g_maxrecvinterval = 0;

u32 MAX_NODE_NUM;
u32 MAX_DISPATCHMSG_WAITING;
/*====================================================================
函数名：OspTaskCreate
功能：创建并激活一个任务
算法实现：（可选项）
引用全局变量：
输入参数说明：pvTaskEntry: 任务入口，
              szName: 任务名，以NULL结束的字符串，
              uPriority: 任务优先级别，
              uStacksize: 任务堆栈大小，
              uParam: 任务参数，
			  uFlag: 创建标志，
			  puTaskID: 返回参数，任务ID.

  返回值说明：成功返回任务的句柄，失败返回NULL.
====================================================================*/
API TASKHANDLE OspTaskCreate(void *pvTaskEntry,
							 const char * szName,
							 u8 byPriority,
							 u32 dwStacksize,
							 KD_PTR pvParam,
							 u16 wFlag,
							 u32 *pdwTaskID
							 )
{
	TASKHANDLE  hTask;
	u32 dwTaskID;

	int Priority;

	if(szName == NULL){} /* 用于避免告警 */
	wFlag = 0; /* 用于避免告警 */

	if(byPriority < 50)
	{
		Priority = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else if(byPriority < 100)
	{
		Priority = THREAD_PRIORITY_HIGHEST;
	}
	else if(byPriority < 120)
	{
		Priority = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else if(byPriority < 150)
	{
		Priority = THREAD_PRIORITY_NORMAL;
	}
	else if(byPriority < 200)
	{
		Priority = THREAD_PRIORITY_BELOW_NORMAL;
	}
	else
	{
		Priority = THREAD_PRIORITY_LOWEST;
	}

    hTask = CreateThread(NULL, dwStacksize, (LPTHREAD_START_ROUTINE)pvTaskEntry, (LPVOID)pvParam, 0, (unsigned long *)&dwTaskID);
	if(hTask != NULL)
    {
		//取消CPU绑定，防止多核环境下所有task均运行在一个CPU上
		//SetThreadAffinityMask(hTask, THREADAFFMASK); // 设置线程的处理器姻亲掩码

		if(SetThreadPriority(hTask, Priority) != 0)
		{
			if(pdwTaskID != NULL)
			{
				*pdwTaskID = dwTaskID;
			}
			return hTask;
		}
    }

    return 0;
}


/*====================================================================
  函数名：OspTaskSetAffinity
  功能：
  封装
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：hTaskHandle:windows下的线程/进程的句柄；hTaskId:绑定的线程/进程/任务的ID；
                dwCpuId:需绑定的CPU(从0开始计数,逐次递加)
                byTaskType：绑定的线程/进程/任务的类型（0--线程，1-进程，其他-任务），linux下默认0即可
                pdwPreviousBind：如果之前该进程/线程/任务被绑定到某CPU，则会返回该CPU号（仅Solaris下有效）
  返回值说明：成功返回TRUE, 失败返回FALSE.
  ====================================================================*/
API BOOL32 OspTaskSetAffinity( TASKHANDLE hTaskHandle, u32 dwCpuId, u8 byTaskType, u32* pdwPreviousBind)
{
    BOOL32 bRet = FALSE;
    u32 dwTaskMask = 0;
    if(dwCpuId>32)
    {
        OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set dwCpuId is too big! dwCpuId=%d\n",dwCpuId);
        return FALSE;
    }
    if(dwCpuId == 0)
    {
        dwTaskMask = 0x01;
    }
    else
    {
        dwTaskMask = (0x01)<<dwCpuId;
    }

//    if(byTaskType == 0)
//    {
//        TASKHANDLE hTask;
//        hTask = OpenProcess(0,FALSE,hTaskId);
//        if(hTask == NULL)
//        {
//            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity failed! ProcessId %d is not exist!WSAGetLastError=%d\n",hTaskId,WSAGetLastError);
//            return FALSE;
//        }
//        bRet = SetProcessAffinityMask(hTask , dwTaskMask);
//        if(bRet == FALSE)
//        {
//            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set ProcessAffinityMask Failed! WSAGetLastError=%d\n",WSAGetLastError);
//            return FALSE;
//        }
//        return TRUE;
//    }
//    else if(byTaskType == 1)
//    {
//        TASKHANDLE hTask;
//
//        hTask = OpenThread(0,FALSE,hTaskId);
//
//        if(hTask == NULL)
//        {
//            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity failed! ProcessId %d is not exist!WSAGetLastError=%d\n",hTaskId,WSAGetLastError);
//            return FALSE;
//        }
//        DWORD nRet =0;
//        nRet = SetThreadAffinityMask (hTask , dwTaskMask);
//        if(0 == nRet)
//        {
//            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set ThreadAffinityMask Failed! WSAGetLastError=%d\n",WSAGetLastError);
//            return FALSE;
//        }
//        return TRUE;
//    }
//    else
//    {
//        OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set tasktype error! byTaskType=%d\n",byTaskType);
//        return FALSE;
//    }

    if(byTaskType == 0)
    {
        bRet = SetProcessAffinityMask(hTaskHandle , dwTaskMask);
        if(bRet == FALSE)
        {
            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set ProcessAffinityMask Failed! WSAGetLastError=%d\n",WSAGetLastError);
            return FALSE;
        }
        return TRUE;
    }
    else if(byTaskType == 1)
    {
        DWORD nRet =0;
        nRet = SetThreadAffinityMask (hTaskHandle , dwTaskMask);
        if(0 == nRet)
        {
            OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set ThreadAffinityMask Failed! WSAGetLastError=%d\n",WSAGetLastError);
            return FALSE;
        }
        return TRUE;
    }
    else
    {
        OspPrintf(TRUE,FALSE,"osp:OspTaskSetAffinity set tasktype error! byTaskType=%d\n",byTaskType);
        return FALSE;
    }
}

/*====================================================================
函数名：OspTaskExit
功能：退出调用任务，任务退出之前用户要注意释放本任务申请的内存、信号量等资源。
      封装Windows的ExitThread(0)和vxWorks的taskDelete(0)。
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskExit(void)
{
	ExitThread(0);
	return TRUE;
}

/*====================================================================
函数名：OspTaskTerminate
功能：退出调用任务。任务退出之前用户要注意释放本任务申请的内存、信号量等资源。
      封装Windows的TerminateThread()和vxWorks的taskDelete()。
算法实现：（可选项）
引用全局变量：
输入参数说明：hTask: 要杀死任务的句柄

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskTerminate(TASKHANDLE hTask)
{
	return TerminateThread(hTask, 0);
}

/*====================================================================
函数名：OspTaskSetPriority
功能：改变任务的优先级。

算法实现：（可选项）
引用全局变量：
输入参数说明：hTask: 目标任务的句柄,
              uPriority: 要设置的优先级.

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskSetPriority(TASKHANDLE hTask, u8 byPriority , int newSchedPolicy)
{

	int nPri;   // 实际设置的优先级

	if(byPriority < 50)
	{
		nPri = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else if(byPriority < 100)
	{
		nPri = THREAD_PRIORITY_HIGHEST;
	}
	else if(byPriority < 120)
	{
		nPri = THREAD_PRIORITY_ABOVE_NORMAL;
	}
	else if(byPriority < 150)
	{
		nPri = THREAD_PRIORITY_NORMAL;
	}
	else if(byPriority < 200)
	{
		nPri = THREAD_PRIORITY_BELOW_NORMAL;
	}
	else
	{
		nPri = THREAD_PRIORITY_LOWEST;
	}

	return SetThreadPriority(hTask, nPri);
}

/*====================================================================
函数名：OspTaskGetPriority
功能：获得任务的优先级。

算法实现：（可选项）
引用全局变量：
输入参数说明：hTask: 目标任务的句柄,
              puPri: 返回参数, 成功返回任务的优先级.

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskGetPriority(TASKHANDLE hTask, u8* puPri, int *SchedPolicy)
{
	if(puPri == NULL)
	{
		return FALSE;
	}

	int nRetPrior;

	nRetPrior = GetThreadPriority(hTask);
	if(nRetPrior == THREAD_PRIORITY_ERROR_RETURN)
	{
		return FALSE;
	}

	switch(nRetPrior)
	{
	case THREAD_PRIORITY_TIME_CRITICAL:
		*puPri = 0;
		break;

	case THREAD_PRIORITY_HIGHEST:
		*puPri = 50;
		break;

	case THREAD_PRIORITY_ABOVE_NORMAL:
		*puPri = 100;
		break;

	case THREAD_PRIORITY_NORMAL:
		*puPri = 120;
		break;

	case THREAD_PRIORITY_BELOW_NORMAL:
        *puPri = 150;
		break;

	case THREAD_PRIORITY_LOWEST:
		*puPri = 200;
		break;
	default:
		*puPri = 120;
		break;
	}
    return TRUE;
}

/*====================================================================
函数名：OspTaskSuspend
功能：挂起指定任务
算法实现：（可选项）
引用全局变量：
输入参数说明：hTask: 目标任务的句柄, 为NULL时挂起当前任务.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskSuspend(TASKHANDLE hTask)
{

	if(hTask == NULL)  // 如果传入参数为NULL, 挂起本任务.
	{
		hTask = GetCurrentThread();
	}

	return (SuspendThread(hTask) != -1);

}

/*====================================================================
函数名：OspTaskResume
功能：使得先前挂起的任务继续执行
算法实现：（可选项）
引用全局变量：
输入参数说明：hTask: 目标任务的句柄

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspTaskResume(TASKHANDLE hTask)
{
	return (ResumeThread(hTask) != -1);
}

/*====================================================================
函数名：OspTaskSelfHandle
功能：获得调用任务的句柄
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功调用任务的句柄, 失败返回NULL.
====================================================================*/
API TASKHANDLE OspTaskSelfHandle(void)
{
	TASKHANDLE hTask = NULL;
	BOOL32 bSuccess;

    bSuccess = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hTask, 0, FALSE, DUPLICATE_SAME_ACCESS);
	if( !bSuccess )
	{
		hTask = NULL;
	}
	return hTask;
}

/*====================================================================
函数名：OspTaskSelfID
功能：获得调用任务的ID
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：调用任务的ID.
====================================================================*/
API u32 OspTaskSelfID(void)
{
	return GetCurrentThreadId();
}

/*====================================================================
函数名：OspTaskSafe
功能：保护调用任务不被非法删除
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspTaskSafe(void)
{
}

/*====================================================================
函数名：OspTaskUnsafe
功能：释放调用任务的删除保护，使得调用任务可以被安全删除
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspTaskUnsafe(void)
{
}

/*====================================================================
函数名：OspTaskHandleVerify
功能：判断指定任务是否存在
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API BOOL32 OspTaskHandleVerify(TASKHANDLE hTask)
{
	u32  uExitCode = 0;

	if(GetExitCodeThread(hTask, (unsigned long *)&uExitCode) == 0)
	{
		return FALSE;
	}

	return (uExitCode == STILL_ACTIVE);
}

/*====================================================================
函数名：OspCreateMailbox
功能：创建一个邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：szName: 邮箱名, 必须是NULL结束的字符串,
              uMsgNumber: 邮箱中可缓存的消息条数,
              uMsgLength: 每条消息的最大长度,
			  puReadID: 返回邮箱的读端,
              puWriteID: 返回邮箱的写端.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
#define PIPE_CTRLHEAD_LEN        24   // Windows的管道在每次发送时都加上一个24u8的头部

API BOOL32 OspCreateMailbox(const char* szName, u32 dwMsgNumber, u32 dwMsgLength, MAILBOXID *ptReadID, MAILBOXID *ptWriteID)
{
	if(ptReadID == NULL || ptWriteID == NULL)
	{
		return FALSE;
	}

	szName = NULL;
    return CreatePipe( (PHANDLE)ptReadID, (PHANDLE)ptWriteID, NULL, dwMsgNumber*(dwMsgLength+PIPE_CTRLHEAD_LEN) );
}

/*====================================================================
函数名：OspCloseMailbox
功能：关闭邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：uReadID: 邮箱的读端,
              uWriteID--邮箱写端.

  返回值说明：无
====================================================================*/
API void OspCloseMailbox(MAILBOXID tReadID, MAILBOXID tWriteID)
{
	CloseHandle(tReadID);
	CloseHandle(tWriteID);
}

/*====================================================================
函数名：OspSndMsg
功能：向邮箱发消息
算法实现：
引用全局变量：无
输入参数说明：dwWriteID: 邮箱的写端,
              pchMsgBuf: 待写消息的指针,
			  dwLen: 消息长度,
			  timeout: 超时值(ms).

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSndMsg(MAILBOXID tWriteID, const char * pchMsgBuf, u32 dwLen, int nTimeout)
{
	if(tWriteID <= 0 || pchMsgBuf == NULL)
	{
		return FALSE;
	}

    BOOL32 bReturn = TRUE;
    u32 dwLenSended = 0;

	nTimeout = 2000; /* 用于避免告警 */

    bReturn = WriteFile(tWriteID, pchMsgBuf, dwLen, (unsigned long *)&dwLenSended, NULL);
	if(!bReturn || (dwLenSended != dwLen))
	{
		bReturn = FALSE;
	}
    return bReturn;

}

/*====================================================================
函数名：OspRcvMsg
功能：从邮箱收消息
算法实现：
引用全局变量：无
输入参数说明：uReadID: 邮箱的读端,
              uTimeout: 超时设置(ms),
			  pchMsgBuf: 存放接收消息的buffer指针,
              uLen: buffer长度,
			  puLenRcved: 实际接收到的消息长度.
返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspRcvMsg(MAILBOXID tReadID, u32 dwTimeout, char *pchMsgBuf, u32 dwLen, u32 *pdwLenRcved)
{
	u32 dwRecvLen;

	BOOL32 bRet;

	dwTimeout = 2000;   /* 用于避免告警 */

    bRet = ReadFile(tReadID, pchMsgBuf, dwLen, (unsigned long *)&dwRecvLen, NULL);
	if( !bRet )
	{
		return FALSE;
	}
	if(pdwLenRcved != NULL)
	{
		*pdwLenRcved = (u32)dwRecvLen;
	}
	return TRUE;
}

/*====================================================================
函数名：OspMBAvailMsgs
功能：取得邮箱中待处理的消息数
算法实现：（可选项）
引用全局变量：
输入参数说明：uReadID: 邮箱的读端,
              dwMsgLen: 每条消息的长度。

  返回值说明：剩余消息数。
====================================================================*/
API u32 OspMBAvailMsgs(MAILBOXID tReadID, u32 dwMsgLen)
{
	if(tReadID <= 0 || dwMsgLen <= 0)
	{
		return 0;
	}

	u32 dwBytesAvail = 0;
	BOOL32 bReturn;

	bReturn = PeekNamedPipe(tReadID, NULL, 0, NULL, (unsigned long *)&dwBytesAvail, NULL);
	if( !bReturn )
	{
		return 0;
	}
    return dwBytesAvail/(dwMsgLen+PIPE_CTRLHEAD_LEN);
}

/*====================================================================
函数名：OspSemBCreate
功能：创建一个二元信号量
算法实现：（可选项）
引用全局变量：
输入参数说明：phSema: 返回的信号量句柄

  返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 OspSemBCreate(SEMHANDLE *phSema)
{
	if(phSema == NULL)
    {
		return FALSE;
	}

    *phSema = CreateSemaphore(NULL, 1, 1, NULL);
    return (*phSema != NULL);
}

/*====================================================================
函数名：OspSemCCreate
功能：创建计数信号量
算法实现：（可选项）
引用全局变量：
输入参数说明：phSema: 信号量句柄返回参数，
              uInitCount: 初始计数，
			  uMaxCount: 最大计数

  返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemCCreate(SEMHANDLE *phSema, u32 dwInitCount, u32 dwMaxCount)
{
	if(phSema == NULL)
	{
		return FALSE;
	}

    *phSema = CreateSemaphore(NULL, dwInitCount, dwMaxCount, NULL);
    return (*phSema != NULL);
}

/*====================================================================
函数名：OspSemTake
功能：信号量p操作
算法实现：（可选项）
引用全局变量：
输入参数说明：hSema: 信号量句柄

  返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32  OspSemTake(SEMHANDLE hSema)
{
    return ( WAIT_FAILED != WaitForSingleObject(hSema, INFINITE) );
}

/*====================================================================
函数名：OspSemTakeByTime
功能：带超时的信号量p操作
算法实现：（可选项）
引用全局变量：
输入参数说明：hSema: 信号量句柄,
              uTimeout: 超时间隔(以ms为单位)

返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemTakeByTime(SEMHANDLE hSema, u32 dwTimeout)
{
    return ( WAIT_OBJECT_0 == WaitForSingleObject(hSema, dwTimeout) );
}

/*====================================================================
函数名：OspSemDelete
功能：删除信号量
算法实现：（可选项）
引用全局变量：
输入参数说明：hSema: 待删除信号量的句柄

返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemDelete(SEMHANDLE hSema)
{
    return CloseHandle(hSema);
}

/*====================================================================
函数名：OspSemGive
功能：信号量v操作
算法实现：（可选项）
引用全局变量：
输入参数说明：hSema: 信号量句柄

返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemGive(SEMHANDLE hSema)
{
    u32 previous;

    return ReleaseSemaphore(hSema, 1, (LPLONG)&previous);
}

/*====================================================================
函数名：OspTaskDelay
功能：任务延时
算法实现：（可选项）
引用全局变量：
输入参数说明：uMs: 时间间隔(ms)

  返回值说明：
====================================================================*/
API void OspTaskDelay(u32 dwMs)
{
    Sleep(dwMs);
}

/*====================================================================
函数名：SockInit
功能：套接口库初始化。封装windows的WSAStartup，vxWorks下返回TRUE.
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 SockInit(void)
{
    WSADATA wsaData;
    u32 err;
	static int flag = 0;

	if (flag == 1)				/* 只初始化一次*/
		return TRUE;

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(err != 0)
	{
		return FALSE;
	}

	flag = 1;

	return TRUE;
}

/*====================================================================
函数名：SockSend
功能：向socket发送消息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API BOOL32 SockSend(SOCKHANDLE tSock, const char * pchBuf, u32 dwLen)
{
    int ret = SOCKET_ERROR;
	u32 nTotalSendLen;
	int nTryNum, nErrNo;

	if((tSock == INVALID_SOCKET) || (pchBuf == NULL))
	{
		return FALSE;
	}

	nTotalSendLen = 0;
	while (nTotalSendLen < dwLen)
	{
		//发送失败原因为底层没有Buf时，要重新尝试发送3次
		for(nTryNum = 0; nTryNum < 3; nTryNum++)
		{
			ret = send(tSock, (char*)(pchBuf + nTotalSendLen), dwLen - nTotalSendLen, SOCK_SEND_FLAGS);
			if(ret == SOCKET_ERROR)
			{
				nErrNo = WSAGetLastError();
				if(nErrNo != WSAENOBUFS && nErrNo != WSAEWOULDBLOCK)
				{
					OspPrintf(TRUE, FALSE, "Osp: SockSend error : %d\n", nErrNo);
					return FALSE;
				}
				OspTaskDelay(50);
			}
			else
			{
				break;
			}
		}
		nTotalSendLen += ret;
	}
    return TRUE;
}

/*====================================================================
函数名：SockRecv
功能：从socket接收消息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API BOOL32 SockRecv(SOCKHANDLE tSock, char *pchBuf, u32 dwLen, u32 *puRcvLen)
{
	int win_error;
	int ret = SOCKET_ERROR;

	if(tSock == INVALID_SOCKET)
	{
		return FALSE;
	}

	if(pchBuf == NULL)
	{
		return FALSE;
	}

        // 接收数据大小若为零，则立即返回；
        // 否则，会导致recv也返回零，导致语义冲突
        if (dwLen == 0)
        {
        	if(puRcvLen != NULL) *puRcvLen = 0;
         	return TRUE;
        }

	u32 before ,after, tmp;

	before = OspTickGet();
    ret = recv(tSock, pchBuf, dwLen, 0);
	after = OspTickGet();
	tmp = after - before;

	if (tmp > g_maxrecvinterval){
		g_maxrecvinterval = tmp;
	}

    if(SOCKET_ERROR == ret || 0 == ret)
	{
			win_error = WSAGetLastError();
			OspPrintf(TRUE, FALSE, "Osp: sock receive error, errno %d\n", win_error);
		return FALSE;
	}

	if(puRcvLen != NULL) *puRcvLen = (u32)ret;
	return TRUE;
}

/*====================================================================
函数名：SockCleanup
功能：套接口库清理，封装windows的WSACleanup，vxWorks下返回TRUE
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 SockCleanup(void)
{
    u32 err;

	return TRUE;
	/* 不clean up，防止有的时候业务代码先clean后又调用socket操作*/

    err = WSACleanup();
	if(err != 0)
		return FALSE;

	return TRUE;
}

/*====================================================================
函数名：SockClose
功能：关闭套接字，windows下封装closesocket，vxWorks下封装close
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 SockClose(SOCKHANDLE hSock)
{
	if(hSock == INVALID_SOCKET)
	{
		return FALSE;
	}

    return ( 0 == closesocket(hSock) );
}

/*====================================================================
函数名：OspTickGet
功能：取得当前的系统tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：当前的系统tick数
====================================================================*/
API u32 OspTickGet()
{
    return GetTickCount();
}

//获取64位tick的锁
SEMHANDLE g_tTickGet64Sem;
/*====================================================================
函数名：OspTickGet
功能：取得当前的系统tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：当前的系统tick数
====================================================================*/
API u64 _OspTickGet64()
{
	static int dwIntialized = 0;		/* 是否初始化 */
	static u64 dwCurrentTick = 0;		/* 当前的u64的tick*/
	static u32 dwLastTick = 0;		/* 上次获取的系统tick*/
	u32 dwNowTick;			/* 当前系统tick*/

	dwNowTick = GetTickCount();

	if (dwIntialized == 0)
	{
		dwCurrentTick = (u32)dwNowTick;
		dwLastTick = dwNowTick;
		dwIntialized = 1;
		return dwCurrentTick;
	}

	if (dwNowTick >= dwLastTick)
	{
		dwCurrentTick += (dwNowTick - dwLastTick);
	}
	else
	{
		dwCurrentTick += (dwNowTick + 0xffffffff - dwLastTick);
	}

	dwLastTick = dwNowTick;
	return dwCurrentTick;
}

API u64 OspTickGet64()
{
	u64 dwRet;
	OspSemTake(g_tTickGet64Sem);
//#if (_WIN32_WINNT >= 0x0600)
//	dwRet = GetTickCount64();
//#else
	dwRet = _OspTickGet64();
//#endif
	OspSemGive(g_tTickGet64Sem);
	return dwRet;
}


/*====================================================================
函数名：OspClkRateGet
功能：得到tick精度
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：tick精度
====================================================================*/
API u32 OspClkRateGet()
{
    return 1000;
}

	#define SystemBasicInformation       0
	#define SystemPerformanceInformation 2
	#define SystemTimeInformation        3

	#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

	typedef struct
	{
		DWORD   dwUnknown1;
		ULONG   uKeMaximumIncrement;
		ULONG   uPageSize;
		ULONG   uMmNumberOfPhysicalPages;
		ULONG   uMmLowestPhysicalPage;
		ULONG   uMmHighestPhysicalPage;
		ULONG   uAllocationGranularity;
		PVOID   pLowestUserAddress;
		PVOID   pMmHighestUserAddress;
		ULONG   uKeActiveProcessors;
		BYTE    bKeNumberProcessors;
		BYTE    bUnknown2;
		WORD    wUnknown3;
	} SYSTEM_BASIC_INFORMATION;

	typedef struct
	{
		LARGE_INTEGER   liIdleTime;
		DWORD           dwSpare[76];
	} SYSTEM_PERFORMANCE_INFORMATION;

	typedef struct
	{
		LARGE_INTEGER liKeBootTime;
		LARGE_INTEGER liKeSystemTime;
		LARGE_INTEGER liExpTimeZoneBias;
		ULONG         uCurrentTimeZoneId;
		DWORD         dwReserved;
	} SYSTEM_TIME_INFORMATION;

	typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

u32 g_dwOspGetCpuNum = 0;
u32 g_dwOspGetCpuSuccessNum = 0;
u32 g_dwOspGetCpuFailedNum = 0;
u32 g_dwOspGetCpuReturnLittleInterval = 0;
u32 g_dwMinIdleCpu = 100;
u32 g_dwMaxIdleCpu = 0;
API void OspCpuShow()
{
	TOspCpuInfo tOspCpuInfo;
	if( OspGetCpuInfo(&tOspCpuInfo) )
	{
		OspPrintf( TRUE , FALSE , "Last Idle Cpu percent : %d\n" , tOspCpuInfo.m_byIdlePercent );
	}
	OspPrintf( TRUE , FALSE , "g_dwOspGetCpuNum : %d\n" , g_dwOspGetCpuNum );
	OspPrintf( TRUE , FALSE , "g_dwOspGetCpuSuccessNum : %d\n" , g_dwOspGetCpuSuccessNum );
	OspPrintf( TRUE , FALSE , "g_dwOspGetCpuFailedNum : %d\n" , g_dwOspGetCpuFailedNum );
	OspPrintf( TRUE , FALSE , "g_dwOspGetCpuReturnLittleInterval : %d\n" , g_dwOspGetCpuReturnLittleInterval );
	OspPrintf( TRUE , FALSE , "g_dwMinIdleCpu : %d\n" , g_dwMinIdleCpu );
	OspPrintf( TRUE , FALSE , "g_dwMaxIdleCpu : %d\n" , g_dwMaxIdleCpu );
}

//获取CPU_INFO的锁
SEMHANDLE g_tCpuInfoSem;
//最小查询间隔3秒
#define MIN_INTERVAL_OSP_GET_CPU	3

/*====================================================================
函数名：_OspGetCpuInfo
功能：取得当前的CPU信息,加锁保护
算法实现：（可选项）
引用全局变量：
输入参数说明：ptCpuInfo，用户CPU信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/

static BOOL32 _OspGetCpuInfo(TOspCpuInfo* ptCpuInfo)
{
	if( NULL == ptCpuInfo )
	{
		return FALSE;
	}

	g_dwOspGetCpuNum++;
	static BOOL32 bFirstOperate = TRUE;
	static u8 byIdleTimePrev = 100;
	static u32 dwLastTick = 0;
	static u32 dwCurrentTick = 0;

	dwCurrentTick = OspTickGet();
	if( !bFirstOperate &&
		( ( dwCurrentTick - dwLastTick )/OspClkRateGet() < MIN_INTERVAL_OSP_GET_CPU ) )
	{
		::OspLog( 255 , "OspGetCpuInfo in short time interval , reserved IdleCpu percent : %d!\n" , byIdleTimePrev );
		ptCpuInfo->m_byIdlePercent = byIdleTimePrev;
		g_dwOspGetCpuReturnLittleInterval++;
		return TRUE;
	}

	PROCNTQSI NtQuerySystemInformation;

	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	SYSTEM_TIME_INFORMATION        SysTimeInfo;
	SYSTEM_BASIC_INFORMATION       SysBaseInfo;
	double                         dbIdleTime = 0;
	double                         dbSystemTime;
	LONG                           status;
	static LARGE_INTEGER           liOldIdleTime = {0,0};
	static LARGE_INTEGER           liOldSystemTime = {0,0};

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle("ntdll"), "NtQuerySystemInformation");

	if( !NtQuerySystemInformation )
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// 取系统中处理器数目
	status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
	if (status != NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// 取系统时间
	status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
	if (status!=NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// 取Cpu空闲时间
	status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
	if (status != NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// 第一次调用时跳过，以后每次计算一下占用率
	if (liOldIdleTime.QuadPart != 0)
	{
		// 当前差值 = 当前值 - 原有值
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
		// 所有Cpu空闲时间占系统时间的百分比 = IdleTime / SystemTime
		dbIdleTime = dbIdleTime / dbSystemTime;
		// 单个Cpu占用率% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
	}

	// 记录新的空闲数值
	liOldIdleTime = SysPerfInfo.liIdleTime;
	liOldSystemTime = SysTimeInfo.liKeSystemTime;

	byIdleTimePrev = (u8)100 - (u8)dbIdleTime;
	ptCpuInfo->m_byIdlePercent = byIdleTimePrev;
	g_dwOspGetCpuSuccessNum++;
	if( byIdleTimePrev < g_dwMinIdleCpu )
	{
		g_dwMinIdleCpu = byIdleTimePrev;
	}
	if( byIdleTimePrev > g_dwMaxIdleCpu )
	{
		g_dwMaxIdleCpu = byIdleTimePrev;
	}
	::OspLog( 255 , "OspGetCpuInfo Idle Cpu percent : %d(%d)!\n" , byIdleTimePrev , (u8)100 - (u8)dbIdleTime );

	dwLastTick = dwCurrentTick;
	if( bFirstOperate )
	{
		bFirstOperate = FALSE;
	}
	return TRUE;
}

/*====================================================================
函数名：OspGetCpuInfo
功能：取得当前的CPU信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptCpuInfo，用户CPU信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetCpuInfo( TOspCpuInfo* ptCpuInfo )
{
	BOOL32 bRet ;

	OspSemTake(g_tCpuInfoSem);
	bRet = _OspGetCpuInfo(ptCpuInfo);
	OspSemGive(g_tCpuInfoSem);

	return bRet;
}

//vc6.0 不识别LPMEMORYSTATUSEX系列接口 予以屏蔽
#ifdef LPMEMORYSTATUSEX
typedef   void(WINAPI*   FunctionGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);//声明函数原型指针
#endif
/*====================================================================
函数名：OspGetMemInfo
功能：取得当前的Mem信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMemInfo，用户MEM信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetMemInfo( TOspMemInfo* ptMemInfo )
{
	#ifdef LPMEMORYSTATUSEX
	HMODULE hModule;
	FunctionGlobalMemoryStatusEx GlobalMemoryStatusEx;
	MEMORYSTATUSEX tMemStatus;
	if( NULL == ptMemInfo )
	{
		return FALSE;
	}
	tMemStatus.dwLength = sizeof(tMemStatus);
	hModule = LoadLibrary("kernel32.dll");//载入动态链接库kernel32.dll，返回它的句柄
	if(NULL==hModule)
	{
		return FALSE;
	}
	GlobalMemoryStatusEx   =(FunctionGlobalMemoryStatusEx)GetProcAddress(hModule,"GlobalMemoryStatusEx");
	if(NULL==GlobalMemoryStatusEx)
	{
		return FALSE;
	}
	GlobalMemoryStatusEx(&tMemStatus);
	if (tMemStatus.dwLength != sizeof(MEMORYSTATUSEX))
	{
		return FALSE;
	}
	ptMemInfo->m_dwPhysicsSize = tMemStatus.ullTotalPhys/1024;
	ptMemInfo->m_dwAllocSize = (tMemStatus.ullTotalPhys - tMemStatus.ullAvailPhys)/1024;
	ptMemInfo->m_dwFreeSize = tMemStatus.ullAvailPhys/1024;
	FreeLibrary(hModule);
    return TRUE;
    #endif
    return FALSE;
}
API BOOL32 OspGetMemInfoEx( TOspMemInfoEx* ptMemInfo )
{
    return FALSE;
}

/*====================================================================
函数名：OspGetTimeInfo
功能：取得当前的Time信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptTimeInfo，系统time信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetTimeInfo( TOspTimeInfo* ptTimeInfo )
{
	if( NULL == ptTimeInfo )
	{
		return FALSE;
	}

	SYSTEMTIME tSysTime;
	GetLocalTime( &tSysTime );
	ptTimeInfo->m_wYear = tSysTime.wYear;
	ptTimeInfo->m_wMonth = tSysTime.wMonth;
	ptTimeInfo->m_wDay = tSysTime.wDay;
	ptTimeInfo->m_wHour = tSysTime.wHour;
	ptTimeInfo->m_wMinute = tSysTime.wMinute;
	ptTimeInfo->m_wSecond = tSysTime.wSecond;
    return TRUE;
}

/*====================================================================
函数名：OspSetTimeInfo
功能：设置当前的Time信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptTimeInfo，系统time信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspSetTimeInfo( TOspTimeInfo* ptTimeInfo )
{
	if( NULL == ptTimeInfo )
	{
		return FALSE;
	}

	SYSTEMTIME tSysTime;
	memset( &tSysTime , 0 , sizeof(SYSTEMTIME) );
	tSysTime.wYear = ptTimeInfo->m_wYear;
	tSysTime.wMonth = ptTimeInfo->m_wMonth;
	tSysTime.wDay = ptTimeInfo->m_wDay;
	tSysTime.wHour = ptTimeInfo->m_wHour;
	tSysTime.wMinute = ptTimeInfo->m_wMinute;
	tSysTime.wSecond = ptTimeInfo->m_wSecond;
    return SetLocalTime( &tSysTime );
}

/*====================================================================
函数名：OspGetDiskInfo
功能：取得当前的磁盘分区信息
算法实现：（可选项）
引用全局变量：
输入参数说明：s8 *pchPartionName， 分区名
			  ptDiskInfo，系统磁盘分区信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetDiskInfo(const s8 *pchPartionName , TOspDiskInfo* ptDiskInfo )
{
	if( ( NULL == pchPartionName ) ||
		( NULL == ptDiskInfo ) )
	{
		return FALSE;
	}

	u32 dwNameLength = strlen(pchPartionName);
	if( ( dwNameLength > MAX_PARTION_NAME_LENGTH ) ||
		( dwNameLength == 0 ) )
	{
		return FALSE;
	}

	ULARGE_INTEGER tUlargeUserSpace ;
	ULARGE_INTEGER tUlargeFullSpace ;
	ULARGE_INTEGER tUlargeFreeSpace ;
	memset(&tUlargeUserSpace, 0, sizeof(ULARGE_INTEGER));
	memset(&tUlargeFullSpace, 0, sizeof(ULARGE_INTEGER));
	memset(&tUlargeFreeSpace, 0, sizeof(ULARGE_INTEGER));

	if( FALSE == GetDiskFreeSpaceEx( pchPartionName, &tUlargeUserSpace, &tUlargeFullSpace, &tUlargeFreeSpace) )
	{
		return FALSE;
	}

	ptDiskInfo->m_dwPhysicsSize = (u32)(tUlargeFullSpace.QuadPart/(1<<20));
	ptDiskInfo->m_dwFreeSize = (u32)(tUlargeFreeSpace.QuadPart/(1<<20));
	ptDiskInfo->m_dwUsedSize = ptDiskInfo->m_dwPhysicsSize - ptDiskInfo->m_dwFreeSize;
	return TRUE;
}

// 串口封装：以Windows串口操作函数为准，在VxWorks下提供相应函数
/*====================================================================
函数名：OspSerialOpen
功能：打开指定的串口
算法实现：（可选项）
引用全局变量：
输入参数说明：uPort: 待打开的端口号

返回值说明：成功返回打开串口的句柄，失败返回INVALID_SERIALHANDLE
====================================================================*/
API SERIALHANDLE OspSerialOpen(u8 byPort)
{
	char achComName[10];

	if( sprintf(achComName, "COM%d", byPort) <= 3 )
	{
		return INVALID_SERIALHANDLE;
	}

	return CreateFile(achComName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

/*====================================================================
函数名：OspSerialClose
功能：关闭指定的串口
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSerialClose(SERIALHANDLE hCom)
{
	if(hCom == 0)
	{
		return FALSE;
	}

	return CloseHandle(hCom);
}

/*====================================================================
函数名：OspSerialRead
功能：从指定的串口读数据
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              pchBuf: 存放读出数据的buffer,
              uBytesToRead: 要读出的u8数,
              puBytesRead: 返回实际读出的u8数.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSerialRead(SERIALHANDLE hCom, char *pchBuf, u32 dwBytesToRead, u32 *pdwBytesRead)
{
	if(pchBuf == NULL || dwBytesToRead == 0)
	{
		return FALSE;
	}

	return ReadFile(hCom, pchBuf, (unsigned long)dwBytesToRead, (unsigned long *)pdwBytesRead, NULL);
}

/*====================================================================
函数名：OspSerialWrite
功能：向指定的串口写数据
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              pchBuf: 待写数据的buffer,
              uBytesToWrite: 要写的u8数,
              puBytesWrite: 返回实际写入的u8数.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSerialWrite(SERIALHANDLE hCom, const char *pchBuf, u32 dwBytesToWrite, u32 *pdwBytesWrite)
{
	return WriteFile(hCom, pchBuf, dwBytesToWrite, (unsigned long *)pdwBytesWrite, NULL);
}

/*====================================================================
函数名：OspGetCommTimeouts
功能：取得打开串口的超时设置
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              ptCommTimeouts: 指向一个超时结构.

返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspGetCommTimeouts(SERIALHANDLE hCom, TOspCommTimeouts *ptCommTimeouts)
{
	if(hCom == 0 || ptCommTimeouts == NULL)
	{
		return FALSE;
	}

	return GetCommTimeouts(hCom, ptCommTimeouts);
}

/*====================================================================
函数名：OspSetCommTimeouts
功能：设置打开串口的超时数据
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              ptCommTimeouts: 指向一个超时结构.

  返回值说明：
====================================================================*/
API BOOL32 OspSetCommTimeouts(SERIALHANDLE hCom, TOspCommTimeouts *ptCommTimeouts)
{
	if(hCom == 0 || ptCommTimeouts == NULL)
	{
		return FALSE;
	}

	return SetCommTimeouts(hCom, ptCommTimeouts);
}

/*====================================================================
函数名：OspGetCommState
功能：取得串口状态
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              ptDCB: 指向一个TOspDCB结构的指针.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspGetCommState(SERIALHANDLE hCom, TOspDCB *ptDCB)
{
	if(hCom == 0 || ptDCB == NULL)
	{
		return FALSE;
	}

	return GetCommState(hCom, ptDCB);
}

/*====================================================================
函数名：OspSetCommState
功能：设置串口状态。注意：设置值必须使用预定义的宏。
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              ptDCB: 指向一个TOspDCB结构的指针.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspSetCommState(SERIALHANDLE hCom, TOspDCB *ptDCB)
{
	if(hCom == 0 || ptDCB == NULL)
	{
		return FALSE;
	}

	return SetCommState(hCom, ptDCB);
}

/*====================================================================
函数名：OspSerialEscapeFunction
功能：设置串口扩展功能
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              uFunc: 要实现的扩展功能.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspEscapeCommFunction(SERIALHANDLE hCom, u32 dwFunc)
{
	if(hCom == 0) return FALSE;

	return EscapeCommFunction(hCom, dwFunc);
}

/*====================================================================
函数名：OspPurgeComm
功能：清串口
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              uFlags: 标志.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspPurgeComm(SERIALHANDLE hCom, u32 dwFlags)
{
	if(hCom == 0)
	{
		return FALSE;
	}

	return PurgeComm(hCom, dwFlags);
}

/*====================================================================
函数名：OspClearCommError
功能：清除串口错误
算法实现：（可选项）
引用全局变量：
输入参数说明：hCom: 串口句柄,
              puErrors: 错误码,
              ptStat: 通信状态.

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 OspClearCommError(SERIALHANDLE hCom, u32 *puErrors, TOspComStat *ptStat)
{
	if(hCom == 0 || puErrors == NULL || ptStat == NULL)
	{
		return FALSE;
	}

	return ClearCommError(hCom, (unsigned long *)puErrors, ptStat);
}

/*=============================================================================
函 数 名：OspAddrListGet
功    能：获取本机地址表内容
注    意：如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
本函数效率较低，不要重复调用。
算法实现：
全局变量：
参    数： u32   adwIP[] : [in/out]用户申请的用于地址表内容的数组的首地址
           u16   wNum : [in]用户申请的数组的大小
返 回 值： 返回本机地址表中地址个数，错误时返回0
-------------------------------------------------------------------------------
修改记录：
日      期  版本  修改人  修改内容
2004/03/10  3.0   李雪峰  增加功能接口
=============================================================================*/
API	u16  OspAddrListGet( u32 adwIPList[], u16 wNum )
{
	u16  wIpNum = 0;
	if((adwIPList == NULL) || (wNum <= 0))
	{
		return 0;
	}

	char szName[64];

    /* 取得本机名 */
	if(SOCKET_ERROR == gethostname(szName, 64))
	{
		return 0;
	}

	/* 通过主机名得到地址表 */
	struct hostent *pHost = gethostbyname(szName);
	if(pHost == NULL)
	{
		return 0;
	}

	/* 在主机地址表中查找这个IP地址 */
    for(int i=0; pHost->h_addr_list[i] != NULL; i++)
	{
		u32 *pAddr = (u32*)pHost->h_addr_list[i];
        if(wIpNum >= wNum)
			return wIpNum;

		adwIPList[wIpNum] = *pAddr;
		wIpNum++;
    }
    return wIpNum;
}



static BOOL _OspGetEthernetAdapterInfoAll(TOSPEthernetAdapterInfoAll * ptEthernetAdapterInfoAll)
{
	ptEthernetAdapterInfoAll->nNum = 0;
	return FALSE;
}

/*====================================================================
函数名      : OspGetEthernetAdapterInfoAll
功能        ：获取本系统所有以太网网卡信息
算法实现    ：（可选项）
引用全局变量：无
输入参数说明：TEthernetAdapterInfoAll 多网卡信息结构，参考数据结构的定义
返回值说明  ：ETHERNET_ADAPTER_ERROR/ETHERNET_ADAPTER_OK
              成功/失败
====================================================================*/
API BOOL OspGetEthernetAdapterInfoAll(TOSPEthernetAdapterInfoAll * ptEthernetAdapterInfoAll)
{
    if (ptEthernetAdapterInfoAll == NULL)
    {
        return FALSE;
    }

    return _OspGetEthernetAdapterInfoAll(ptEthernetAdapterInfoAll);
}


static BOOL _OspGetEthernetAdapterInfoData(s32 nEthAdapterId, TOSPEthernetAdapterInfo * ptEthernetAdapterInfo)
{
    s32 nLoop = 0;
    s32 nNum = 0;
    TOSPEthernetAdapterInfoAll tEthernetAdapterInfoAll;

    memset(&tEthernetAdapterInfoAll, 0, sizeof(tEthernetAdapterInfoAll));

    if (_OspGetEthernetAdapterInfoAll(&tEthernetAdapterInfoAll) == FALSE)
    {
        return FALSE;
    }

    nNum = tEthernetAdapterInfoAll.nNum;

    for (nLoop = 0; nLoop < nNum; nLoop ++)
    {
        if (tEthernetAdapterInfoAll.atEthernetAdapter[nLoop].nId == nEthAdapterId)
        {
            memcpy(ptEthernetAdapterInfo, &(tEthernetAdapterInfoAll.atEthernetAdapter[nLoop]), sizeof(TOSPEthernetAdapterInfo));
            return TRUE;
        }
    }

    return TRUE;
}


/*====================================================================
函数名      : OspGetEthernetAdapterInfo
功能        ：根据网卡号获取网卡信息
算法实现    ：（可选项）
引用全局变量：无
输入参数说明： nEthAdapterId 网卡编号0-ETHERNET_ADAPTER_MAX_NUM，最多支持16个网卡
             TEthernetAdapterInfo 单网卡信息结构
返回值说明  ：ETHERNET_ADAPTER_ERROR/ETHERNET_ADAPTER_OK
              成功/失败
====================================================================*/
API BOOL OspGetEthernetAdapterInfo(u32 nEthAdapterId, TOSPEthernetAdapterInfo * ptEthernetAdapterInfo)
{
    if(ptEthernetAdapterInfo == NULL)
    {
        return FALSE;
    }

    return _OspGetEthernetAdapterInfoData(nEthAdapterId, ptEthernetAdapterInfo);
}



s16 osplb_create(osplb_handle* handle, osplb_create_param * ptParam)
{
	s16 wRet = OSP_LB_ERR_OK;
	osplb_data_t * lb;

	if ( NULL == handle
		|| NULL == ptParam
		|| ptParam->m_nBufSize == 0
		|| ptParam->m_nMaxUnitSize == 0
		|| ptParam->m_nMaxUnitSize > ptParam->m_nBufSize
		)
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t *)handle;

	if ( OSP_LB_MAGCI_NUM == lb->m_dwMagic )
		return OSP_LB_ERR_CREATED ;

	if ( lb->m_start)
		return OSP_LB_ERR_CREATED;

	lb->m_byRaceLvl		= ptParam->m_byRaceLvl ;
	lb->m_pbyBuf		= NULL ;


	lb->m_pbyBuf = (u8*)malloc(ptParam->m_nBufSize + ptParam->m_nMaxUnitSize + sizeof(s32));
	if ( NULL == lb->m_pbyBuf )
	{
		OspPrintf(1, 0, "lb malloc size[%d[ fail\n", ptParam->m_nBufSize);
		wRet = OSP_LB_ERR_NOMEM;
		goto err;
	}

	lb->m_nBuffSize		= ptParam->m_nBufSize;
	lb->m_nMaxUnitSize	= ptParam->m_nMaxUnitSize;
	lb->m_nReadPos		= 0;
	lb->m_nWritePos		= 0;
	lb->m_dwMagic = OSP_LB_MAGCI_NUM;
	lb->m_start = 1;


	return wRet ;

err:

	SAFE_FREE(lb->m_pbyBuf);

	return wRet ;

}

s16	osplb_close(osplb_handle* handle)
{
	osplb_data_t * lb;
	if ( NULL == handle )
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start)
		return OSP_LB_ERR_NOT_CREATE;

	lb->m_nWritePos		= 0;
	lb->m_nReadPos		= 0;

	SAFE_FREE(lb->m_pbyBuf);
	lb->m_nBuffSize		= 0;
	lb->m_nMaxUnitSize	= 0;
	lb->m_byRaceLvl		= 0;
	lb->m_dwMagic		= 0;


	return OSP_LB_ERR_OK ;
}

s16 osplb_reset(osplb_handle *handle)
{
	osplb_data_t * lb;
	if ( NULL == handle )
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;
	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start)
		return OSP_LB_ERR_NOT_CREATE;

	lb->m_nReadPos		= 0;
	lb->m_nWritePos		= 0;
	lb->m_start			= 1;


	return OSP_LB_ERR_OK ;
}

s16 osplb_write(osplb_handle* handle, u8 * pBuf, s32 nBufSize)
{
	osplb_data_t * lb;
	s16 wRet = OSP_LB_ERR_OK;
	s32	nBufLeft;
	s32 tmp;

	if ( NULL == handle || NULL == pBuf)
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start )
	{
		wRet = OSP_LB_ERR_NOT_CREATE;
		goto err;
	}

    if( nBufSize <= 0 || nBufSize > lb->m_nMaxUnitSize )
    {
		wRet = OSP_LB_ERR_PARAM;
		goto err;
    }

	nBufLeft = lb->m_nWritePos - lb->m_nReadPos ;
	if ( nBufLeft < 0 && -nBufLeft -1 < nBufSize + (s32)sizeof(s32))
	{
		wRet = OSP_LB_ERR_FULL;
		goto err;
	}

	if ( lb->m_nWritePos > lb->m_nBuffSize )
	{
		if ( lb->m_nReadPos == 0 )
		{
			wRet = OSP_LB_ERR_FULL;
			goto err;
		}
		else
		{
			lb->m_nWritePos = 0;
		}
	}

	nBufLeft = lb->m_nWritePos - lb->m_nReadPos ;
	if ( nBufLeft < 0 && -nBufLeft -1 < nBufSize + (s32)sizeof(s32))
	{
		wRet = OSP_LB_ERR_FULL;
		goto err;
	}

	charcpy(lb->m_pbyBuf+lb->m_nWritePos, (u8*)&nBufSize, sizeof(s32));
	if ( nBufSize <= 8 )
		charcpy(lb->m_pbyBuf + lb->m_nWritePos + sizeof(s32), pBuf, (s16)nBufSize);
	else
		memcpy(lb->m_pbyBuf + lb->m_nWritePos + sizeof(s32), pBuf, nBufSize);

	tmp = lb->m_nWritePos + nBufSize + sizeof(s32);
	lb->m_nWritePos = tmp ;

err:

	lb->m_write = 0;


	return wRet;

}

s16 osplb_read(osplb_handle* handle, u8 * pBuf, s32 *nBufSize)
{
	osplb_data_t * lb;
	s16 wRet = OSP_LB_ERR_OK ;
	s32 nReadLen , tmp;

	nReadLen = 0;
	if ( NULL == handle || NULL == pBuf || NULL == nBufSize)
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start )
	{
		wRet = OSP_LB_ERR_NOT_CREATE;
		goto err;
	}

	if ( lb->m_nWritePos == lb->m_nReadPos )
	{
		wRet = OSP_LB_ERR_EMPTY;
		goto err;
	}

	if ( lb->m_nReadPos > lb->m_nBuffSize )
	{
		if ( lb->m_nWritePos != lb->m_nReadPos )
			lb->m_nReadPos = 0;
		else
		{
			wRet = OSP_LB_ERR_EMPTY;
			goto err;
		}
	}

	if ( lb->m_nWritePos == lb->m_nReadPos )
	{
		wRet = OSP_LB_ERR_EMPTY;
		goto err;
	}

	charcpy((u8*)&nReadLen, lb->m_pbyBuf + lb->m_nReadPos, sizeof(s32));
	if ( nReadLen < 0 || nReadLen > lb->m_nMaxUnitSize )
	{
		wRet = OSP_LB_ERR_INVALID_MEM;
		goto err;
	}
	if ( nReadLen > *nBufSize )
	{
		wRet = OSP_LB_ERR_PARAM;
		goto err;
	}

	*nBufSize = nReadLen ;
	if ( nReadLen <= 8 )
		charcpy(pBuf, lb->m_pbyBuf + lb->m_nReadPos + sizeof(s32), (s16)nReadLen);
	else
		memcpy(pBuf, lb->m_pbyBuf + lb->m_nReadPos + sizeof(s32), nReadLen);

	tmp = lb->m_nReadPos + nReadLen + sizeof(s32);
	lb->m_nReadPos = tmp;


err:

	lb->m_read = 0;

	return wRet;
}

s16 osplb_total_size(osplb_handle* handle, s32 * nSize)
{
	osplb_data_t * lb;

	if ( NULL == handle )
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start )
		return OSP_LB_ERR_NOT_CREATE;

	*nSize = lb->m_nBuffSize;

	return OSP_LB_ERR_OK;
}

s16 osplb_available_size(osplb_handle* handle, s32 * nSize)
{
	osplb_data_t * lb;
	u32 t1, t2;

	if ( NULL == handle )
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start )
		return OSP_LB_ERR_NOT_CREATE;

	t1 = lb->m_nReadPos;
	t2 = lb->m_nWritePos ;

	if ( t1 > t2 )
		*nSize = t1 - t2 ;
	else
		*nSize = lb->m_nBuffSize - t2 + t1 ;

	return OSP_LB_ERR_OK;
}

s16 osplb_is_empty(osplb_handle * handle, u8 * empty)
{
	osplb_data_t * lb;

	if ( NULL == handle || NULL == empty)
		return OSP_LB_ERR_PARAM ;

	lb = (osplb_data_t * )handle;

	if ( OSP_LB_MAGCI_NUM != lb->m_dwMagic )
		return OSP_LB_ERR_INVALID_HANDLE ;

	if ( !lb->m_start )
		return OSP_LB_ERR_NOT_CREATE;

	if ( lb->m_nWritePos == lb->m_nReadPos )
		*empty = 1;
	else
		*empty = 0;

	return OSP_LB_ERR_OK;

}
