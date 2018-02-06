/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ���ù��ܵ���Ҫʵ���ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
-------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
05/27/2003	2.1         ���        �淶��
******************************************************************************/

#include <sys/timeb.h>
#include "osp.h"
#include <assert.h>
#include "../include/ospSch.h"
#include "../include/ospTimer.h"

#include "../include/OspTeleServer.h"
#include "osptool.h"

extern HMODULE ahRegModule[MAX_MOD_NUM];   // ģ��ǼǱ�, ��OspTeleServer.cpp�ж���

/*====================================================================
��������OspIsLocalHost
���ܣ��ж��Ƿ��Ǳ���IP����OspIsLocalHost( inet_addr( "127.0.0.1" ) );
      �������Windows��ʹ�ñ������������ȵ���OspInit������WSAStartup��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uIP: ��Ҫ�жϵ�IP

  ����ֵ˵�������uIP�Ǳ���IP, ����TRUE; ����, ����FALSE.
====================================================================*/
API BOOL32 OspIsLocalHost(u32 dwIP)
{
 	if(dwIP == htonl(INADDR_LOOPBACK))
	{
		return TRUE;
	}

	char szName[64];

    /* ȡ�ñ����� */
	if(SOCKET_ERROR == gethostname(szName, 64))
	{
		return FALSE;
	}

	/* ͨ���������õ���ַ�� */
	struct hostent *pHost = gethostbyname(szName);
	if(pHost == NULL)
	{
		return FALSE;
	}

	/* ��������ַ���в������IP��ַ */
    for(int i=0; pHost->h_addr_list[i] != NULL; i++) 
	{
        u32 *pAddr = (u32*)pHost->h_addr_list[i];
        if(dwIP == *pAddr)
		{
            return TRUE;
        }
    }
    return FALSE;
}

/*====================================================================
��������osphelp
���ܣ���ʾ��ģ��İ�����Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void osphelp(void)
{
	OspPrintf(TRUE, FALSE, "Osp Version: %s. ", OSPVERSION);
	OspPrintf(TRUE, FALSE, "Compile Time: %s  %s\n", __TIME__, __DATE__);
	OspPrintf(TRUE, FALSE, "\nOspNodeShow:\n");
	OspPrintf(TRUE, FALSE, "Function: show information about all extern nodes\n");
	OspPrintf(TRUE, FALSE, "\nOspAppShow:\n");
	OspPrintf(TRUE, FALSE, "Function: show information about all apps on this node\n");
	OspPrintf(TRUE, FALSE, "\nOspInstShow:\n");
	OspPrintf(TRUE, FALSE, "Parameter: uAppId -- the app number to show\n");
	OspPrintf(TRUE, FALSE, "Function: show information about all instances in specified app\n");
	OspPrintf(TRUE, FALSE, "\nOspInstShowAll:\n");
	OspPrintf(TRUE, FALSE, "Function: show information about all instances on this node\n");
	OspPrintf(TRUE, FALSE, "\nOspTrcAllOn:\n");
	OspPrintf(TRUE, FALSE, "Function: enable all trace flags of all apps on this node\n");
	OspPrintf(TRUE, FALSE, "\nOspTrcAllOff:\n");
	OspPrintf(TRUE, FALSE, "Function: disable all trace flags of all apps on this node\n");
	OspPrintf(TRUE, FALSE, "\nOspSendTrcOn:\n");
	OspPrintf(TRUE, FALSE, "Function: enable trace for all outgoing messages of this node\n");
	OspPrintf(TRUE, FALSE, "\nOspSendTrcOff:\n");
	OspPrintf(TRUE, FALSE, "Function: disable trace for all outgoing messages of this node\n");
	OspPrintf(TRUE, FALSE, "\nOspRcvTrcOn:\n");
	OspPrintf(TRUE, FALSE, "Function: enable trace for all incoming messages of this node\n");
	OspPrintf(TRUE, FALSE, "\nOspRcvTrcOff:\n");
	OspPrintf(TRUE, FALSE, "Function: disable trace for all incoming messages of this node\n");
    OspPrintf(TRUE, FALSE, "\nOspSetLogLevel:\n");
    OspPrintf(TRUE, FALSE, "Function: set loglevel for App\n");    
}

/*====================================================================
��������OspRegistModule
���ܣ���OSP�Ǽ�һ��ģ��, Windows������.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchModuleName - ģ�飨.exe�ļ���������"kdvmt.exe"

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspRegistModule(const char *pchModuleName)
{
	HMODULE tmpHandle = GetModuleHandle(pchModuleName);
	u32 i;
	if(tmpHandle != NULL)
	{
		for(i=0; i<MAX_MOD_NUM; i++)
		{
			if(ahRegModule[i] == NULL)
			{
				ahRegModule[i] = tmpHandle;
				return TRUE;
			}
		}
		//module�������������
		OspPrintf(TRUE,FALSE,"ahRegModule attr is full ,i :%d\n",i);
		printf("ahRegModule attr is full ,i :%d\n",i);		
	}
	else
	{
		OspPrintf(TRUE,FALSE,"[GetModuleHandle] return NULL,error :%d\n",GetLastError());
		printf("[GetModuleHandle] return NULL,error :%d\n",GetLastError());		
	}
	
	return FALSE;
}

/*=============================================================================
�� �� ����OspUnRegistModule
��      �ܣ���OSPע��һ���ѵǼǵ�ģ��(��windows����Ч����OspRegistModule֮���Ҵ���ͬһ�μ����в���Ч)
ע      �⣺
�㷨ʵ�֣�
ȫ�ֱ�����
��      ����(in)pchModuleName - ģ�飨.exe�ļ���������NULL��β���ַ�����
��"kdvmt.dll"��
�� �� ֵ���ɹ�����TRUE��ʧ�ܷ���FALSE
-------------------------------------------------------------------------------
�޸ļ�¼��
��        ��    �汾  �޸���  �޸�����
2016/07/08    1.0
=============================================================================*/
API BOOL32 OspUnRegistModule(const char* pchModuleName)
{
	HMODULE tmpHandle = GetModuleHandle(pchModuleName);
	u32 i;
	if(tmpHandle != NULL)
	{
		for(i=0; i<MAX_MOD_NUM; i++)
		{
			if(tmpHandle == ahRegModule[i])
			{
				ahRegModule[i] = NULL;
				return TRUE;
			}
		}
		//module�������������
		OspPrintf(TRUE,FALSE,"[osp]cann't find module :%s,Maby you didn't regist this mode this time before\n",pchModuleName);
		printf("[osp]cann't find module :%s,Maby you didn't regist this mode this time before\n",pchModuleName);		
	}
	else
	{
		OspPrintf(TRUE,FALSE,"[GetModuleHandle] return NULL,error :%d\n",GetLastError());
		printf("[GetModuleHandle] return NULL,error :%d\n",GetLastError());		
	}

	return FALSE;
}

/*====================================================================
��������OspSetPrompt
���ܣ�����Telnet��ʾ��, Windows������; Ospȡģ����Ϊȱʡ��ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����prompt: �µ���ʾ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 OspSetPrompt(const char *prompt)
{
	if(prompt == NULL || strlen(prompt) > MAX_PROMPT_LEN)
	{
		return FALSE;
	}

	if( NULL == strcpy(g_Osp.m_achShellPrompt, prompt) )
	{
		return FALSE;
	}

	return TRUE;
}

/*====================================================================
��������OspCmdFuncEnable
���ܣ���Ϊ��ʹ��һЩ���ܱ��������Ż����ĺ���������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵������.
====================================================================*/
API void OspCmdFuncEnable(void)
{	
	OspStopScrnLog();
	OspResumeScrnLog();
	OspIsNodeCheckEnable(0);
	OspEnableNodeCheck(0);
	OspDisableNodeCheck(0);
	OspAgentStart(20000);
	OspSetHBParam(0,100,100);
	OspLogShow();
	IsOspInitd();
	OspMemShow();
	OspSetMemCheck(NULL, 0, 0);
	OspAddrListGet(NULL, 0);
/*��ʼ���û���������Ϊ�գ�������vxworks�±��Ż�*/
    OspTelAuthor();
	    
}

//ʱ����ʵ��
/*
    ֮���Բ���GetTick��ԭ�����£�
    1������Ƶ��ϵͳ����
    2��Linux��Tick��10msΪ��λ������ȷ
*/
TOspTimeMs OspGetTimeMs()
{
    struct timeb tp;
    ftime(&tp);
    TOspTimeMs tMsTime;
    tMsTime.tSecond = tp.time;
    tMsTime.tMilliSecond = tp.millitm;

    return tMsTime;
}
u32 OspGetTimeDiffMs(const TOspTimeMs& tEndTime, const TOspTimeMs& tStartTime)
{
    //�������ʱ��������1193��Сʱ��u32���޷����棬�ú��������
    //ҵ�����Ӧ��ʹ�øú�����ȡ��ʱ���ڵĺ����
    u32 dwDiffMs = 1000*(u32(tEndTime.tSecond - tStartTime.tSecond)) + (tEndTime.tMilliSecond - tStartTime.tMilliSecond);
    return dwDiffMs;
}

//����ϵͳticks����ȡ�뼶ʱ�䣬��������ʱ���Ĳ�����Ϊ�޸���ϵͳʱ��������쳣
u64 OspGetSecondByTicks()
{
    //��ȡticks���ýӿڲ������������
    u64 lwTicks = OspTickGet64();

    //ticks/sec
    u32 dwClkRate = OspClkRateGet();

    //���������
    return (lwTicks / dwClkRate);
}

//�̰߳�ȫ�Ļ�ȡlocaltime
tm OspGetLocalTime_r(const time_t* t)
{
    tm tTm = { 0 };
#ifdef _LINUX_
    localtime_r(t, &tTm);
    return tTm;
#else
    tm* ptTm = localtime(t);
    if (ptTm != NULL)
    {
        tTm = *ptTm;
    }
#endif

    return tTm;
}

COspTimeInfo::COspTimeInfo()
{
    Clear();
}

COspTimeInfo::COspTimeInfo(time_t tTime)
{
    SetTime(&tTime);
}

COspTimeInfo::COspTimeInfo(const COspTimeInfo& tObj)
{
    (*this) = tObj;
}

COspTimeInfo& COspTimeInfo::operator = (const COspTimeInfo& tObj)
{
    if (this != &tObj)
    {
        m_wYear    = tObj.m_wYear;
        m_byMonth  = tObj.m_byMonth;
        m_byMDay   = tObj.m_byMDay;
        m_byHour   = tObj.m_byHour;
        m_byMinute = tObj.m_byMinute;
        m_bySecond = tObj.m_bySecond;
        m_wMillSec = tObj.m_wMillSec;
    }

    return *this;
}

void COspTimeInfo::Clear()
{
    m_wYear    = 0;
    m_byMonth  = 0;
    m_byMDay   = 0;
    m_byHour   = 0;
    m_byMinute = 0;
    m_bySecond = 0;
    m_wMillSec = 0;
}

void COspTimeInfo::SetTime( const time_t *ptTime )
{
    if (NULL == ptTime)
    {
        return;
    }

    tm tLocalTime = OspGetLocalTime_r(ptTime);
    tm *ptLocalTime = &tLocalTime;
    if (NULL != ptLocalTime)
    {
        m_wYear    = (u16)ptLocalTime->tm_year + 1900;
        m_byMonth  = (u8)ptLocalTime->tm_mon + 1;
        m_byMDay   = (u8)ptLocalTime->tm_mday;
        m_byHour   = (u8)ptLocalTime->tm_hour;
        m_byMinute = (u8)ptLocalTime->tm_min;
        m_bySecond = (u8)ptLocalTime->tm_sec;
        m_wMillSec = 0;
    }
    else
    {
        Clear();
    }
}


void COspTimeInfo::GetTime( time_t &tTime ) const
{
    tTime = GetTime();
}

time_t COspTimeInfo::GetTime(void) const
{
    tm  tTmCurTime;

    //���뽫�ñ�����ʼ��Ϊ-1��������������ʱ�ĵط���ʱ���п��ܻ����1��Сʱ��ʱ��
    tTmCurTime.tm_isdst = -1;

    u16 wYear = m_wYear;
    if (wYear >= 1900)
    {
        tTmCurTime.tm_year = wYear - 1900;
    }
    else
    {
        tTmCurTime.tm_year = 106;
    }
    if (m_byMonth > 0 && m_byMonth <= 12)
    {
        tTmCurTime.tm_mon  = m_byMonth - 1;
    }
    else
    {
        tTmCurTime.tm_mon  = 0;
    }
    if (m_byMDay > 0 && m_byMDay <= 31)
    {
        tTmCurTime.tm_mday = m_byMDay;
    }
    else
    {
        tTmCurTime.tm_mday = 1;
    }
    if (m_byHour < 24)
    {
        tTmCurTime.tm_hour = m_byHour;
    }
    if (m_byMinute < 60)
    {
        tTmCurTime.tm_min  = m_byMinute;
    }
    if (m_bySecond < 60)
    {
        tTmCurTime.tm_sec  = m_bySecond;
    }

    return ::mktime( &tTmCurTime );
}

BOOL32 COspTimeInfo::operator == ( const COspTimeInfo &tObj ) const
{
    if (m_wYear    == tObj.m_wYear&&
        m_byMonth  == tObj.m_byMonth&&
        m_byMDay   == tObj.m_byMDay&&
        m_byHour   == tObj.m_byHour&&
        m_byMinute == tObj.m_byMinute&&
        m_bySecond == tObj.m_bySecond&&
        m_wMillSec == tObj.m_wMillSec)
    {
        return TRUE;
    }
    return FALSE;
}

u32 COspTimeInfo::GetString(u32 dwBuffLen, s8* pchBuff) const
{
    if (NULL == pchBuff)
    {
        return 0;
    }
	//ע�⣺����ӡ�ĳ���>=dwBuffLen��ʱ��, pchBuff��û��'\0'��β
	u32 dwLen = SNPRINTF(pchBuff,dwBuffLen, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
            m_wYear, m_byMonth, m_byMDay, m_byHour, m_byMinute, m_bySecond);
	if (dwLen >= dwBuffLen)
	{
		pchBuff[dwBuffLen - 1] = '\0';
		return dwBuffLen - 1;
	}

	return dwLen;
}

COspTimeInfo COspTimeInfo::GetCurrTime()
{
    COspTimeInfo tOspTime;

    struct timeb tp;
    ftime(&tp);

    tm tLocalTime = OspGetLocalTime_r(&tp.time);
    tm *ptLocalTime = &tLocalTime;

    if (NULL != ptLocalTime)
    {
        tOspTime.m_wYear    = (u16)ptLocalTime->tm_year + 1900;
        tOspTime.m_byMonth  = (u8)ptLocalTime->tm_mon + 1;
        tOspTime.m_byMDay   = (u8)ptLocalTime->tm_mday;
        tOspTime.m_byHour   = (u8)ptLocalTime->tm_hour;
        tOspTime.m_byMinute = (u8)ptLocalTime->tm_min;
        tOspTime.m_bySecond = (u8)ptLocalTime->tm_sec;
        tOspTime.m_wMillSec = tp.millitm;
    }

    return tOspTime;
}
//����ֵΪ�ַ�������Ч���ȣ�������\0
u32 COspTimeInfo::GetCurrStrTime(u32 dwBufLen, char *pchBuffer)
{
	if(NULL == pchBuffer)
	{
		return 0;
	}

    TOspTimeInfo tTimeInfo;
    memset(&tTimeInfo, 0, sizeof(tTimeInfo));
    OspGetTimeInfo(&tTimeInfo);

    u32 dwLen = SNPRINTF(pchBuffer, dwBufLen, "%04u-%02u-%02u %02u:%02u:%02u",
        tTimeInfo.m_wYear,
        tTimeInfo.m_wMonth,
        tTimeInfo.m_wDay,
        tTimeInfo.m_wHour,
        tTimeInfo.m_wMinute,
        tTimeInfo.m_wSecond);
	if(dwLen >= dwBufLen)
	{
		pchBuffer[dwBufLen - 1] = '\0';
		return dwBufLen - 1;
	}

	return dwLen;
}
//����ֵΪ�ַ�������Ч���ȣ�������\0
u32 COspTimeInfo::GetCurrStrTime_ms(u32 dwBufLen, char *pchBuffer)
{
	if(NULL == pchBuffer)
	{
		return 0;
	}

    struct timeb tp;
    ftime(&tp);

    TOspTimeInfo tTimeInfo;
    memset(&tTimeInfo, 0, sizeof(tTimeInfo));

    tm tLocalTime = OspGetLocalTime_r(&tp.time);
    tm *ptLocalTime = &tLocalTime;
    if (NULL != ptLocalTime)
    {
        tTimeInfo.m_wYear    = (u16)ptLocalTime->tm_year + 1900;
        tTimeInfo.m_wMonth  = (u16)ptLocalTime->tm_mon + 1;
        tTimeInfo.m_wDay   = (u16)ptLocalTime->tm_mday;
        tTimeInfo.m_wHour   = (u16)ptLocalTime->tm_hour;
        tTimeInfo.m_wMinute = (u16)ptLocalTime->tm_min;
        tTimeInfo.m_wSecond = (u16)ptLocalTime->tm_sec;
    }
	//_snprintf ����ֵ��������ֹ��
    u32 dwLen =  SNPRINTF(pchBuffer, dwBufLen, "%04u-%02u-%02u %02u:%02u:%02u-%03u",
        tTimeInfo.m_wYear,
        tTimeInfo.m_wMonth,
        tTimeInfo.m_wDay,
        tTimeInfo.m_wHour,
        tTimeInfo.m_wMinute,
        tTimeInfo.m_wSecond,
        u16(tp.millitm));
	if(dwLen >= dwBufLen)
	{
		pchBuffer[dwBufLen - 1] = '\0';
		return dwBufLen - 1;
	}

	return dwLen;
}
//����ʵ��
COspSemLock::COspSemLock()
{
    if (OspSemBCreate(&m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemBCreate() ����ʧ��\n");
    }
}

COspSemLock::~COspSemLock()
{
    if (m_hSemaphore != NULL)
    {
        if (OspSemDelete(m_hSemaphore) == FALSE)
        {
            OspPrintf(FALSE, TRUE, "OspSemDelete() ����ʧ��\n");
        }

        m_hSemaphore = NULL;
    }
}

BOOL32 COspSemLock::Lock(u32 dwTimeout)
{
    if (OspSemTakeByTime(m_hSemaphore, dwTimeout) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemTakeByTime() ����ʧ��\n");
        return FALSE;
    }

    return TRUE;
}

BOOL32 COspSemLock::UnLock()
{
    if (OspSemGive(m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemGive() ����ʧ��\n");
        return FALSE;
    }

    return TRUE;
}

BOOL32 COspSemLock::Take(u32 dwTimeout)
{
    return Lock(dwTimeout);
}

BOOL32 COspSemLock::Give()
{
    return UnLock();
}

//�ź���
COspSemaphore::COspSemaphore(u32 dwInitCnt, u32 dwMaxCnt)
{
    if (OspSemCCreate(&m_hSemaphore, dwInitCnt, dwMaxCnt) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemCCreate() ����ʧ��\n");
    }
}

COspSemaphore::~COspSemaphore()
{
    if (m_hSemaphore != NULL)
    {
        if (OspSemDelete(m_hSemaphore) == FALSE)
        {
            OspPrintf(FALSE, TRUE, "OspSemDelete() ����ʧ��\n");
        }

        m_hSemaphore = NULL;
    }
}

BOOL32 COspSemaphore::Take(u32 dwTimeout)
{
    if (OspSemTakeByTime(m_hSemaphore, dwTimeout) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemTakeByTime() ����ʧ��\n");
        return FALSE;
    }

    return TRUE;
}

BOOL32 COspSemaphore::Give()
{
    if (OspSemGive(m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemGive() ����ʧ��\n");
        return FALSE;
    }

    return TRUE;
}

//�Զ���ʵ��
COspAutoLock::COspAutoLock(COspSemLock& rLock):m_Lock(rLock)
{
    m_Lock.Lock();
}

COspAutoLock::~COspAutoLock()
{
    m_Lock.UnLock();
}

//��ȡ����·��
u32 OspGetProcessPath(u32 dwBufferLen, char* pchBuffer)
{
    if(NULL == pchBuffer)
    {
    	return 0;
    }
    pchBuffer[0] = '\0';

    s8 szFullPath[1024] = {0};

#ifdef _LINUX_
    readlink("/proc/self/exe", szFullPath, sizeof(szFullPath));
    *(strrchr(szFullPath, '/') + 1) = '\0';
#else
    GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
    *(strrchr(szFullPath, '\\') + 1) = '\0';
#endif

    strncpy(pchBuffer, szFullPath, dwBufferLen);
    pchBuffer[dwBufferLen - 1] = '\0';
    return strlen(pchBuffer);
}