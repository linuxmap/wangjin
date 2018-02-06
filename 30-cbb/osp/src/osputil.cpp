/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP 常用功能的主要实现文件
作者	：张文江
版本	：1.0.02.7.5
-------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
05/27/2003	2.1         向飞        规范化
******************************************************************************/

#include <sys/timeb.h>
#include "osp.h"
#include <assert.h>
#include "../include/ospSch.h"
#include "../include/ospTimer.h"

#include "../include/OspTeleServer.h"
#include "osptool.h"

extern HMODULE ahRegModule[MAX_MOD_NUM];   // 模块登记表, 在OspTeleServer.cpp中定义

/*====================================================================
函数名：OspIsLocalHost
功能：判断是否是本机IP，如OspIsLocalHost( inet_addr( "127.0.0.1" ) );
      如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
算法实现：（可选项）
引用全局变量：
输入参数说明：uIP: 需要判断的IP

  返回值说明：如果uIP是本机IP, 返回TRUE; 否则, 返回FALSE.
====================================================================*/
API BOOL32 OspIsLocalHost(u32 dwIP)
{
 	if(dwIP == htonl(INADDR_LOOPBACK))
	{
		return TRUE;
	}

	char szName[64];

    /* 取得本机名 */
	if(SOCKET_ERROR == gethostname(szName, 64))
	{
		return FALSE;
	}

	/* 通过主机名得到地址表 */
	struct hostent *pHost = gethostbyname(szName);
	if(pHost == NULL)
	{
		return FALSE;
	}

	/* 在主机地址表中查找这个IP地址 */
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
函数名：osphelp
功能：显示本模块的帮助信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
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
函数名：OspRegistModule
功能：向OSP登记一个模块, Windows下适用.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchModuleName - 模块（.exe文件）名，如"kdvmt.exe"

  返回值说明：成功返回TRUE, 失败返回FALSE.
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
		//module个数超过最大规格
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
函 数 名：OspUnRegistModule
功      能：从OSP注销一个已登记的模块(仅windows下有效，在OspRegistModule之后且处于同一次加载中才生效)
注      意：
算法实现：
全局变量：
参      数：(in)pchModuleName - 模块（.exe文件）名，以NULL结尾的字符串，
如"kdvmt.dll"等
返 回 值：成功返回TRUE，失败返回FALSE
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
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
		//module个数超过最大规格
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
函数名：OspSetPrompt
功能：设置Telnet提示符, Windows下适用; Osp取模块名为缺省提示符.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：prompt: 新的提示符

  返回值说明：成功返回TRUE, 失败返回FALSE.
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
函数名：OspCmdFuncEnable
功能：人为地使得一些可能被编译器优化掉的函数被调用
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：无.
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
/*初始化用户名和密码为空，避免在vxworks下被优化*/
    OspTelAuthor();
	    
}

//时间类实现
/*
    之所以不用GetTick，原因如下：
    1，避免频繁系统调用
    2，Linux下Tick以10ms为单位，不精确
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
    //如果两个时间间隔超过1193个小时，u32将无法保存，该函数会溢出
    //业务程序应该使用该函数获取短时间内的毫秒差
    u32 dwDiffMs = 1000*(u32(tEndTime.tSecond - tStartTime.tSecond)) + (tEndTime.tMilliSecond - tStartTime.tMilliSecond);
    return dwDiffMs;
}

//根据系统ticks来获取秒级时间，避免依赖时间差的操作因为修改了系统时间而出现异常
u64 OspGetSecondByTicks()
{
    //获取ticks，该接口不存在溢出问题
    u64 lwTicks = OspTickGet64();

    //ticks/sec
    u32 dwClkRate = OspClkRateGet();

    //计算出秒数
    return (lwTicks / dwClkRate);
}

//线程安全的获取localtime
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

    //必须将该变量初始化为-1，否则在有夏令时的地方，时间有可能会出现1个小时的时差
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
	//注意：当打印的长度>=dwBuffLen的时候, pchBuff将没有'\0'结尾
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
//返回值为字符串的有效长度，不包括\0
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
//返回值为字符串的有效长度，不包括\0
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
	//_snprintf 返回值不包括终止符
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
//锁类实现
COspSemLock::COspSemLock()
{
    if (OspSemBCreate(&m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemBCreate() 调用失败\n");
    }
}

COspSemLock::~COspSemLock()
{
    if (m_hSemaphore != NULL)
    {
        if (OspSemDelete(m_hSemaphore) == FALSE)
        {
            OspPrintf(FALSE, TRUE, "OspSemDelete() 调用失败\n");
        }

        m_hSemaphore = NULL;
    }
}

BOOL32 COspSemLock::Lock(u32 dwTimeout)
{
    if (OspSemTakeByTime(m_hSemaphore, dwTimeout) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemTakeByTime() 调用失败\n");
        return FALSE;
    }

    return TRUE;
}

BOOL32 COspSemLock::UnLock()
{
    if (OspSemGive(m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemGive() 调用失败\n");
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

//信号量
COspSemaphore::COspSemaphore(u32 dwInitCnt, u32 dwMaxCnt)
{
    if (OspSemCCreate(&m_hSemaphore, dwInitCnt, dwMaxCnt) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemCCreate() 调用失败\n");
    }
}

COspSemaphore::~COspSemaphore()
{
    if (m_hSemaphore != NULL)
    {
        if (OspSemDelete(m_hSemaphore) == FALSE)
        {
            OspPrintf(FALSE, TRUE, "OspSemDelete() 调用失败\n");
        }

        m_hSemaphore = NULL;
    }
}

BOOL32 COspSemaphore::Take(u32 dwTimeout)
{
    if (OspSemTakeByTime(m_hSemaphore, dwTimeout) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemTakeByTime() 调用失败\n");
        return FALSE;
    }

    return TRUE;
}

BOOL32 COspSemaphore::Give()
{
    if (OspSemGive(m_hSemaphore) == FALSE)
    {
        OspPrintf(FALSE, TRUE, "OspSemGive() 调用失败\n");
        return FALSE;
    }

    return TRUE;
}

//自动锁实现
COspAutoLock::COspAutoLock(COspSemLock& rLock):m_Lock(rLock)
{
    m_Lock.Lock();
}

COspAutoLock::~COspAutoLock()
{
    m_Lock.UnLock();
}

//获取进程路径
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