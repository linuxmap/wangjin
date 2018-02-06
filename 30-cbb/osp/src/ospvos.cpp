/******************************************************************************
ģ����  �� OSP
�ļ���  �� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP  ����ϵͳ��װ����Ҫʵ���ļ�
����    �����Ľ�
�汾    ��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
06/03/2003  2.1         ��  ��      �淶��
******************************************************************************/
#include "ospsch.h"
#include "ospvos.h"
#include "stdio.h"

u32 g_maxrecvinterval = 0;

u32 MAX_NODE_NUM;
u32 MAX_DISPATCHMSG_WAITING;
/*====================================================================
��������OspTaskCreate
���ܣ�����������һ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pvTaskEntry: ������ڣ�
              szName: ����������NULL�������ַ�����
              uPriority: �������ȼ���
              uStacksize: �����ջ��С��
              uParam: ���������
			  uFlag: ������־��
			  puTaskID: ���ز���������ID.

  ����ֵ˵�����ɹ���������ľ����ʧ�ܷ���NULL.
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

	if(szName == NULL){} /* ���ڱ���澯 */
	wFlag = 0; /* ���ڱ���澯 */

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
		//ȡ��CPU�󶨣���ֹ��˻���������task��������һ��CPU��
		//SetThreadAffinityMask(hTask, THREADAFFMASK); // �����̵߳Ĵ�������������

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
  ��������OspTaskSetAffinity
  ���ܣ�
  ��װ
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hTaskHandle:windows�µ��߳�/���̵ľ����hTaskId:�󶨵��߳�/����/�����ID��
                dwCpuId:��󶨵�CPU(��0��ʼ����,��εݼ�)
                byTaskType���󶨵��߳�/����/��������ͣ�0--�̣߳�1-���̣�����-���񣩣�linux��Ĭ��0����
                pdwPreviousBind�����֮ǰ�ý���/�߳�/���񱻰󶨵�ĳCPU����᷵�ظ�CPU�ţ���Solaris����Ч��
  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspTaskExit
���ܣ��˳��������������˳�֮ǰ�û�Ҫע���ͷű�����������ڴ桢�ź�������Դ��
      ��װWindows��ExitThread(0)��vxWorks��taskDelete(0)��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspTaskExit(void)
{
	ExitThread(0);
	return TRUE;
}

/*====================================================================
��������OspTaskTerminate
���ܣ��˳��������������˳�֮ǰ�û�Ҫע���ͷű�����������ڴ桢�ź�������Դ��
      ��װWindows��TerminateThread()��vxWorks��taskDelete()��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hTask: Ҫɱ������ľ��

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspTaskTerminate(TASKHANDLE hTask)
{
	return TerminateThread(hTask, 0);
}

/*====================================================================
��������OspTaskSetPriority
���ܣ��ı���������ȼ���

�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hTask: Ŀ������ľ��,
              uPriority: Ҫ���õ����ȼ�.

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspTaskSetPriority(TASKHANDLE hTask, u8 byPriority , int newSchedPolicy)
{

	int nPri;   // ʵ�����õ����ȼ�

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
��������OspTaskGetPriority
���ܣ������������ȼ���

�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hTask: Ŀ������ľ��,
              puPri: ���ز���, �ɹ�������������ȼ�.

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspTaskSuspend
���ܣ�����ָ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hTask: Ŀ������ľ��, ΪNULLʱ����ǰ����.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspTaskSuspend(TASKHANDLE hTask)
{

	if(hTask == NULL)  // ����������ΪNULL, ��������.
	{
		hTask = GetCurrentThread();
	}

	return (SuspendThread(hTask) != -1);

}

/*====================================================================
��������OspTaskResume
���ܣ�ʹ����ǰ������������ִ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hTask: Ŀ������ľ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspTaskResume(TASKHANDLE hTask)
{
	return (ResumeThread(hTask) != -1);
}

/*====================================================================
��������OspTaskSelfHandle
���ܣ���õ�������ľ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ���������ľ��, ʧ�ܷ���NULL.
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
��������OspTaskSelfID
���ܣ���õ��������ID
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�������������ID.
====================================================================*/
API u32 OspTaskSelfID(void)
{
	return GetCurrentThreadId();
}

/*====================================================================
��������OspTaskSafe
���ܣ������������񲻱��Ƿ�ɾ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspTaskSafe(void)
{
}

/*====================================================================
��������OspTaskUnsafe
���ܣ��ͷŵ��������ɾ��������ʹ�õ���������Ա���ȫɾ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspTaskUnsafe(void)
{
}

/*====================================================================
��������OspTaskHandleVerify
���ܣ��ж�ָ�������Ƿ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������OspCreateMailbox
���ܣ�����һ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szName: ������, ������NULL�������ַ���,
              uMsgNumber: �����пɻ������Ϣ����,
              uMsgLength: ÿ����Ϣ����󳤶�,
			  puReadID: ��������Ķ���,
              puWriteID: ���������д��.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
#define PIPE_CTRLHEAD_LEN        24   // Windows�Ĺܵ���ÿ�η���ʱ������һ��24u8��ͷ��

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
��������OspCloseMailbox
���ܣ��ر�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uReadID: ����Ķ���,
              uWriteID--����д��.

  ����ֵ˵������
====================================================================*/
API void OspCloseMailbox(MAILBOXID tReadID, MAILBOXID tWriteID)
{
	CloseHandle(tReadID);
	CloseHandle(tWriteID);
}

/*====================================================================
��������OspSndMsg
���ܣ������䷢��Ϣ
�㷨ʵ�֣�
����ȫ�ֱ�������
�������˵����dwWriteID: �����д��,
              pchMsgBuf: ��д��Ϣ��ָ��,
			  dwLen: ��Ϣ����,
			  timeout: ��ʱֵ(ms).

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSndMsg(MAILBOXID tWriteID, const char * pchMsgBuf, u32 dwLen, int nTimeout)
{
	if(tWriteID <= 0 || pchMsgBuf == NULL)
	{
		return FALSE;
	}

    BOOL32 bReturn = TRUE;
    u32 dwLenSended = 0;

	nTimeout = 2000; /* ���ڱ���澯 */

    bReturn = WriteFile(tWriteID, pchMsgBuf, dwLen, (unsigned long *)&dwLenSended, NULL);
	if(!bReturn || (dwLenSended != dwLen))
	{
		bReturn = FALSE;
	}
    return bReturn;

}

/*====================================================================
��������OspRcvMsg
���ܣ�����������Ϣ
�㷨ʵ�֣�
����ȫ�ֱ�������
�������˵����uReadID: ����Ķ���,
              uTimeout: ��ʱ����(ms),
			  pchMsgBuf: ��Ž�����Ϣ��bufferָ��,
              uLen: buffer����,
			  puLenRcved: ʵ�ʽ��յ�����Ϣ����.
����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspRcvMsg(MAILBOXID tReadID, u32 dwTimeout, char *pchMsgBuf, u32 dwLen, u32 *pdwLenRcved)
{
	u32 dwRecvLen;

	BOOL32 bRet;

	dwTimeout = 2000;   /* ���ڱ���澯 */

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
��������OspMBAvailMsgs
���ܣ�ȡ�������д��������Ϣ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uReadID: ����Ķ���,
              dwMsgLen: ÿ����Ϣ�ĳ��ȡ�

  ����ֵ˵����ʣ����Ϣ����
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
��������OspSemBCreate
���ܣ�����һ����Ԫ�ź���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����phSema: ���ص��ź������

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
��������OspSemCCreate
���ܣ����������ź���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����phSema: �ź���������ز�����
              uInitCount: ��ʼ������
			  uMaxCount: ������

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE.
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
��������OspSemTake
���ܣ��ź���p����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hSema: �ź������

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32  OspSemTake(SEMHANDLE hSema)
{
    return ( WAIT_FAILED != WaitForSingleObject(hSema, INFINITE) );
}

/*====================================================================
��������OspSemTakeByTime
���ܣ�����ʱ���ź���p����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hSema: �ź������,
              uTimeout: ��ʱ���(��msΪ��λ)

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSemTakeByTime(SEMHANDLE hSema, u32 dwTimeout)
{
    return ( WAIT_OBJECT_0 == WaitForSingleObject(hSema, dwTimeout) );
}

/*====================================================================
��������OspSemDelete
���ܣ�ɾ���ź���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hSema: ��ɾ���ź����ľ��

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSemDelete(SEMHANDLE hSema)
{
    return CloseHandle(hSema);
}

/*====================================================================
��������OspSemGive
���ܣ��ź���v����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hSema: �ź������

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSemGive(SEMHANDLE hSema)
{
    u32 previous;

    return ReleaseSemaphore(hSema, 1, (LPLONG)&previous);
}

/*====================================================================
��������OspTaskDelay
���ܣ�������ʱ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uMs: ʱ����(ms)

  ����ֵ˵����
====================================================================*/
API void OspTaskDelay(u32 dwMs)
{
    Sleep(dwMs);
}

/*====================================================================
��������SockInit
���ܣ��׽ӿڿ��ʼ������װwindows��WSAStartup��vxWorks�·���TRUE.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
====================================================================*/
API BOOL32 SockInit(void)
{
    WSADATA wsaData;
    u32 err;
	static int flag = 0;

	if (flag == 1)				/* ֻ��ʼ��һ��*/
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
��������SockSend
���ܣ���socket������Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
		//����ʧ��ԭ��Ϊ�ײ�û��Bufʱ��Ҫ���³��Է���3��
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
��������SockRecv
���ܣ���socket������Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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

        // �������ݴ�С��Ϊ�㣬���������أ�
        // ���򣬻ᵼ��recvҲ�����㣬���������ͻ
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
��������SockCleanup
���ܣ��׽ӿڿ�������װwindows��WSACleanup��vxWorks�·���TRUE
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
====================================================================*/
API BOOL32 SockCleanup(void)
{
    u32 err;

	return TRUE;
	/* ��clean up����ֹ�е�ʱ��ҵ�������clean���ֵ���socket����*/

    err = WSACleanup();
	if(err != 0)
		return FALSE;

	return TRUE;
}

/*====================================================================
��������SockClose
���ܣ��ر��׽��֣�windows�·�װclosesocket��vxWorks�·�װclose
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
��������OspTickGet
���ܣ�ȡ�õ�ǰ��ϵͳtick��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������ǰ��ϵͳtick��
====================================================================*/
API u32 OspTickGet()
{
    return GetTickCount();
}

//��ȡ64λtick����
SEMHANDLE g_tTickGet64Sem;
/*====================================================================
��������OspTickGet
���ܣ�ȡ�õ�ǰ��ϵͳtick��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������ǰ��ϵͳtick��
====================================================================*/
API u64 _OspTickGet64()
{
	static int dwIntialized = 0;		/* �Ƿ��ʼ�� */
	static u64 dwCurrentTick = 0;		/* ��ǰ��u64��tick*/
	static u32 dwLastTick = 0;		/* �ϴλ�ȡ��ϵͳtick*/
	u32 dwNowTick;			/* ��ǰϵͳtick*/

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
��������OspClkRateGet
���ܣ��õ�tick����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵����tick����
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

//��ȡCPU_INFO����
SEMHANDLE g_tCpuInfoSem;
//��С��ѯ���3��
#define MIN_INTERVAL_OSP_GET_CPU	3

/*====================================================================
��������_OspGetCpuInfo
���ܣ�ȡ�õ�ǰ��CPU��Ϣ,��������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptCpuInfo���û�CPU��Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
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

	// ȡϵͳ�д�������Ŀ
	status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
	if (status != NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// ȡϵͳʱ��
	status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
	if (status!=NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// ȡCpu����ʱ��
	status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
	if (status != NO_ERROR)
	{
		g_dwOspGetCpuFailedNum++;
		return FALSE;
	}

	// ��һ�ε���ʱ�������Ժ�ÿ�μ���һ��ռ����
	if (liOldIdleTime.QuadPart != 0)
	{
		// ��ǰ��ֵ = ��ǰֵ - ԭ��ֵ
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
		// ����Cpu����ʱ��ռϵͳʱ��İٷֱ� = IdleTime / SystemTime
		dbIdleTime = dbIdleTime / dbSystemTime;
		// ����Cpuռ����% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
	}

	// ��¼�µĿ�����ֵ
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
��������OspGetCpuInfo
���ܣ�ȡ�õ�ǰ��CPU��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptCpuInfo���û�CPU��Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
====================================================================*/
API BOOL32 OspGetCpuInfo( TOspCpuInfo* ptCpuInfo )
{
	BOOL32 bRet ;

	OspSemTake(g_tCpuInfoSem);
	bRet = _OspGetCpuInfo(ptCpuInfo);
	OspSemGive(g_tCpuInfoSem);

	return bRet;
}

//vc6.0 ��ʶ��LPMEMORYSTATUSEXϵ�нӿ� ��������
#ifdef LPMEMORYSTATUSEX
typedef   void(WINAPI*   FunctionGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);//��������ԭ��ָ��
#endif
/*====================================================================
��������OspGetMemInfo
���ܣ�ȡ�õ�ǰ��Mem��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptMemInfo���û�MEM��Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
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
	hModule = LoadLibrary("kernel32.dll");//���붯̬���ӿ�kernel32.dll���������ľ��
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
��������OspGetTimeInfo
���ܣ�ȡ�õ�ǰ��Time��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptTimeInfo��ϵͳtime��Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
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
��������OspSetTimeInfo
���ܣ����õ�ǰ��Time��Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����ptTimeInfo��ϵͳtime��Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
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
��������OspGetDiskInfo
���ܣ�ȡ�õ�ǰ�Ĵ��̷�����Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����s8 *pchPartionName�� ������
			  ptDiskInfo��ϵͳ���̷�����Ϣ�ṹ
����ֵ˵����ΪTRUEʱ��ʾ��ȡ��Ϣ�ɹ��������ʾʧ��
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

// ���ڷ�װ����Windows���ڲ�������Ϊ׼����VxWorks���ṩ��Ӧ����
/*====================================================================
��������OspSerialOpen
���ܣ���ָ���Ĵ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uPort: ���򿪵Ķ˿ں�

����ֵ˵�����ɹ����ش򿪴��ڵľ����ʧ�ܷ���INVALID_SERIALHANDLE
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
��������OspSerialClose
���ܣ��ر�ָ���Ĵ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspSerialRead
���ܣ���ָ���Ĵ��ڶ�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              pchBuf: ��Ŷ������ݵ�buffer,
              uBytesToRead: Ҫ������u8��,
              puBytesRead: ����ʵ�ʶ�����u8��.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspSerialWrite
���ܣ���ָ���Ĵ���д����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              pchBuf: ��д���ݵ�buffer,
              uBytesToWrite: Ҫд��u8��,
              puBytesWrite: ����ʵ��д���u8��.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSerialWrite(SERIALHANDLE hCom, const char *pchBuf, u32 dwBytesToWrite, u32 *pdwBytesWrite)
{
	return WriteFile(hCom, pchBuf, dwBytesToWrite, (unsigned long *)pdwBytesWrite, NULL);
}

/*====================================================================
��������OspGetCommTimeouts
���ܣ�ȡ�ô򿪴��ڵĳ�ʱ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              ptCommTimeouts: ָ��һ����ʱ�ṹ.

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspSetCommTimeouts
���ܣ����ô򿪴��ڵĳ�ʱ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              ptCommTimeouts: ָ��һ����ʱ�ṹ.

  ����ֵ˵����
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
��������OspGetCommState
���ܣ�ȡ�ô���״̬
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              ptDCB: ָ��һ��TOspDCB�ṹ��ָ��.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspSetCommState
���ܣ����ô���״̬��ע�⣺����ֵ����ʹ��Ԥ����ĺꡣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              ptDCB: ָ��һ��TOspDCB�ṹ��ָ��.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspSerialEscapeFunction
���ܣ����ô�����չ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              uFunc: Ҫʵ�ֵ���չ����.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspEscapeCommFunction(SERIALHANDLE hCom, u32 dwFunc)
{
	if(hCom == 0) return FALSE;

	return EscapeCommFunction(hCom, dwFunc);
}

/*====================================================================
��������OspPurgeComm
���ܣ��崮��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              uFlags: ��־.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������OspClearCommError
���ܣ�������ڴ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����hCom: ���ھ��,
              puErrors: ������,
              ptStat: ͨ��״̬.

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
�� �� ����OspAddrListGet
��    �ܣ���ȡ������ַ������
ע    �⣺�������Windows��ʹ�ñ������������ȵ���OspInit������WSAStartup��
������Ч�ʽϵͣ���Ҫ�ظ����á�
�㷨ʵ�֣�
ȫ�ֱ�����
��    ���� u32   adwIP[] : [in/out]�û���������ڵ�ַ�����ݵ�������׵�ַ
           u16   wNum : [in]�û����������Ĵ�С
�� �� ֵ�� ���ر�����ַ���е�ַ����������ʱ����0
-------------------------------------------------------------------------------
�޸ļ�¼��
��      ��  �汾  �޸���  �޸�����
2004/03/10  3.0   ��ѩ��  ���ӹ��ܽӿ�
=============================================================================*/
API	u16  OspAddrListGet( u32 adwIPList[], u16 wNum )
{
	u16  wIpNum = 0;
	if((adwIPList == NULL) || (wNum <= 0))
	{
		return 0;
	}

	char szName[64];

    /* ȡ�ñ����� */
	if(SOCKET_ERROR == gethostname(szName, 64))
	{
		return 0;
	}

	/* ͨ���������õ���ַ�� */
	struct hostent *pHost = gethostbyname(szName);
	if(pHost == NULL)
	{
		return 0;
	}

	/* ��������ַ���в������IP��ַ */
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
������      : OspGetEthernetAdapterInfoAll
����        ����ȡ��ϵͳ������̫��������Ϣ
�㷨ʵ��    ������ѡ�
����ȫ�ֱ�������
�������˵����TEthernetAdapterInfoAll ��������Ϣ�ṹ���ο����ݽṹ�Ķ���
����ֵ˵��  ��ETHERNET_ADAPTER_ERROR/ETHERNET_ADAPTER_OK
              �ɹ�/ʧ��
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
������      : OspGetEthernetAdapterInfo
����        �����������Ż�ȡ������Ϣ
�㷨ʵ��    ������ѡ�
����ȫ�ֱ�������
�������˵���� nEthAdapterId �������0-ETHERNET_ADAPTER_MAX_NUM�����֧��16������
             TEthernetAdapterInfo ��������Ϣ�ṹ
����ֵ˵��  ��ETHERNET_ADAPTER_ERROR/ETHERNET_ADAPTER_OK
              �ɹ�/ʧ��
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
