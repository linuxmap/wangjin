/******************************************************************************
模块名  ： OSP
文件名  ： OspLog.cpp
相关文件：
文件实现功能：OSP 日志功能的主要实现文件
作者    ：张文江
版本    ：1.0.02.7.5
--------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
09/15/98        1.0      某某        ------------
2015/06/30             邓昌葛      增加支持OSP_CLASSLOG和GLBLOG接口记录日志
******************************************************************************/
#include "osplog_private.h"

static COspLog g_cOspLog; // osplog Task

#ifdef _DEBUG

static u8 g_byGlbLogPrintLev = OSP_FREQ_EVENT_LEV;     //全局打印级别 level值大于该值的日志不在屏幕显示

static u8 g_byPrintTimeLev = 1;       //0:不打印时间; 1:打印秒级时间; 255:打印豪秒级时间

static u8 g_byPrintCompileInfo = 1;   //0:不打印编译信息(file-line-class-function); 1:打印编译信息

static u8 g_byPrintOspPrefix = 1;     //0:不打印OSP前缀(app-inst-task-state); 1:打印OSP前缀

static BOOL32 g_bIsMonitorTimePerform = FALSE;  //FALSE:不开启性能监测，TRUE：开启

#else

static u8 g_byGlbLogPrintLev = OSP_ERROR_LEV;     //全局打印级别

static u8 g_byPrintTimeLev = 1;       //0:不打印时间; 1:打印秒级时间; 255:打印豪秒级时间

static u8 g_byPrintCompileInfo = 0;   //0:不打印编译信息(file-line-class-function); 1:打印编译信息

static u8 g_byPrintOspPrefix = 0;     //0:不打印OSP前缀(app-inst-task-state); 1:打印OSP前缀

static BOOL32 g_bIsMonitorTimePerform = FALSE;  //FALSE:不开启性能监测，TRUE：开启

#endif

static u32 g_dwTraceAppInst = 0;      //0:关闭trace; 非0:trace 指定APP的指定INST

static u32 g_dwTraceTaskNO = 0;        //0:关闭trace; 非0:trace 指定事务

static BOOL32 g_bIsWriteRunLog = TRUE; //是否输出运行日志文件，目前写入文件的内容和屏幕的打印内容一致

static BOOL32 g_bIsWriteErrLog = TRUE; //是否输出错误日志文件，目前写入文件的内容和屏幕的打印内容一致

#ifdef _LINUX_
static BOOL32 g_bIsParseMangleName = FALSE; //linux是否进行mangle name的解析，因为非常耗时，默认不进行解析
#endif

static u8 g_byRunLogFileLev = OSP_PROGRESS_LEV;  //运行日志文件级别

static CModuleLogInfo g_cModuleLogLev;

API BOOL32 IsOspPrintCI()
{
    return (g_byPrintCompileInfo != 0);
}
/*====================================================================
函数名：COspLog::COspLog()
功能：COspLog类的构造函数，对一些成员变量进行初始化
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
COspLog::COspLog()
{
	m_hTask 			= 0;
	m_dwTaskID 			= 0;

	m_dwScreenLogNum	= 0;
	m_dwRunFileLogNum   = 0;
	m_dwErrorFileLogNum	= 0;

	m_dwMsgIncome 		= 0;
	m_dwMsgProcessed 	= 0;
	m_dwMsgDropped 		= 0;

	m_byLogScreenLevel 	= 20;
	m_byLogFileLevel 	= 20;
	m_bScrnLogEnbl 		= TRUE;
	m_bLMsgDumpEnbl 	= FALSE;

	m_bLogFileEnable 	= FALSE;
}

/*====================================================================
函数名：COspLog::Initialize
功能：初始化日志系统，主要是创建日志任务及其邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
BOOL32 COspLog::Initialize()
{
	BOOL32 bOK;

    //m_dwCurrentSize = 0;
    //m_dwCurrentFileNo = 0;
	//m_dwFileLogNum = 0;
	//m_dwScreenLogNum = 0;
	m_dwMaxMsgWaiting = MAX_LOGMSG_WAITING;
	m_bIsLogTaskMsgSnd = FALSE;

    //OspSemBCreate(&m_tLogSem);

	/* 文件大小，文件名等由用户使用OspOpenLogFile时进行初始化 */

	/* 创建日志邮箱 */
	bOK = OspCreateMailbox("log",                   // 邮箱名
		                    m_dwMaxMsgWaiting,     // 消息条数
		                    sizeof(TOsMsgStruc),    // 零拷贝以后, 邮箱中只存放消息指针
							&m_dwReadQueHandle,     // 读端handle
							&m_dwWriteQueHandle     // 发端handle
						  );
	if( !bOK )
	{
		m_dwReadQueHandle = 0;
		m_dwWriteQueHandle = 0;
		return FALSE;
	}

	/* 创建日志任务 */
    m_hTask = OspTaskCreate( LogTask,              // 任务入口
		                     "OspLogTask",         // 任务名
							 OSP_LOG_TASKPRI,      // 优先级
							 OSP_LOG_STACKSIZE,    // 堆栈大小
							 0,                 // 参数
							 0,                 // 创建标志
							 &m_dwTaskID            // 任务ID
						   );

    // 如创建任务失败, 释放前面创建的日志邮箱
    if (0 == m_hTask)
    {
        OspCloseMailbox(m_dwReadQueHandle, m_dwWriteQueHandle);
        m_dwReadQueHandle = 0;
        m_dwWriteQueHandle = 0;
        m_dwTaskID = 0;
        return FALSE;
    }

	// 成功, 将日志任务添加到任务链表中以便以后删除
	g_Osp.AddTask(m_hTask, m_dwTaskID, "OspLogTask");


	return TRUE;
}

/*====================================================================
函数名：COspLog::Quit
功能：退出日志系统
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
void COspLog::Quit()
{
	/*if(m_ptLogFd != NULL)
	{
		fclose(m_ptLogFd);
        m_ptLogFd = NULL;
	}*/
	OspCloseMailbox(m_dwReadQueHandle, m_dwWriteQueHandle);
	m_dwReadQueHandle = 0;
	m_dwWriteQueHandle = 0;

	g_Osp.DelTask(m_dwTaskID);
    printf("[COspLog::Quit] del task[%x]\n", m_dwTaskID);

	/*OspSemDelete(m_tLogSem);
	m_tLogSem = NULL;*/
}

/*====================================================================
函数名：COspLog::LastStatusRestore
功能：将日志恢复到上一次系统关闭时的状态
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
/*BOOL32 COspLog::LastStatusRestore()
{
	time_t currtime;

    m_ptLogFd = fopen(m_achFullFileName, "a+t");
    if(m_ptLogFd == NULL)
	{
		return FALSE;
	}

    if(fscanf(m_ptLogFd,"%d", &m_dwCurrentFileNo) == 1) // previous file exists
    {
        if(m_dwCurrentFileNo >= m_dwMaxFiles)
		{
			m_dwCurrentFileNo = 0;
		}
    }
	else
    {
        m_dwCurrentFileNo=0;
    }

    fclose(m_ptLogFd);

    m_ptLogFd = fopen(m_achFullFileName, "a+t");

	time( &currtime );
	fprintf(m_ptLogFd,"%d\n", m_dwCurrentFileNo);
    fprintf(m_ptLogFd,"************* file log begin ************\n");
	fprintf(m_ptLogFd,"Current time: %s\n", ctime(&currtime));
    fflush(m_ptLogFd);

    m_dwCurrentSize = ftell(m_ptLogFd);
    return TRUE;
}*/

#define  DIR_SEP '/'

static BOOL32 MakeDir(const char * szDirName)
{
    if (szDirName == NULL)
    {
        return false;
    }

    if (TRUE == CreateDirectory(szDirName, NULL))
    {
        return true;
    }

    printf("make dir[%s] fail\n", szDirName);
    return false;
}


static BOOL32 ExistFileOrDir(const char * szFileName)
{
    if (szFileName == NULL)
    {
        return false;
    }

    return (_access(szFileName, 0) != -1);
}

static BOOL32 RecureMakeDir(char* szDirPath)
{
    if (NULL==szDirPath)
    {
        return FALSE;
    }

    BOOL32 bRet = FALSE;
	char* pchStart = szDirPath;
	char* pchEnd;
	pchEnd = strchr(pchStart, DIR_SEP);
    while(NULL!=pchEnd)
    {
    	u32 dwLen = pchEnd - pchStart + 1;
    	char subDir[MAX_FILE_DIR_NAME_LEN];
    	memcpy(subDir, pchStart, dwLen);
    	subDir[dwLen] = '\0';
        if (!ExistFileOrDir(subDir))
        {
            bRet = MakeDir(subDir);
            if(!bRet)
            {
                return FALSE;
            }
        }
        pchEnd = strchr(pchEnd + 1, DIR_SEP);
    }

    return TRUE;
}

//启动日志文件，允许写日志文件
void COspLog::LogFileOpen()
{
	m_bLogFileEnable = TRUE;
}

//停止日志文件，不允许写日志文件
void COspLog::LogFileClose()
{
	m_bLogFileEnable = FALSE;
}

/*====================================================================
函数名：COspLog::LogQueWriteFinal
功能：向日志邮箱中发消息
算法实现：（可选项）
引用全局变量：
输入参数说明：tOspLogHead: 日志头部,
              szContent: 要写的内容,
              uLen: 长度.
  返回值说明：
====================================================================*/
void COspLog::LogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen)
{
	//szContent==NULL dwLen=0 用于osp quit时退出日志线程
	/*if(NULL == szContent)
	{
		return;
	}*/

	if(dwLen > MAX_LOG_MSG_LEN || m_bIsLogTaskMsgSnd)
	{
		return;
	}

	if(NULL == szContent)
	{
		printf("szContent is NULL,will quit logtask\n");
		m_bIsLogTaskMsgSnd = TRUE;//没有考虑并发线程。
	}

	//分配内存长度为TOspLogCommonHead + TOspLogHead头 + MAX_LOG_MSG_LEN
	//为避免再次拷贝，该内存由LogQueOut()中使用完毕后进行释放
	TOspLogCommonHead* ptOspLogCommonHead =
			(TOspLogCommonHead*)OspAllocMem(sizeof(TOspLogCommonHead) - 1 + sizeof(TOspLogHead) + dwLen + 1);
	if(NULL == ptOspLogCommonHead)
	{
		printf("COspLog::LogQueWrite mem fail\n");
		return;
	}
	ptOspLogCommonHead->chOspInterfaceType = OSP_LOG_INTERFACE_SIMPLE;

	TOspLogHead *ptOspLogSimpleHead = (TOspLogHead*)(ptOspLogCommonHead->achData);
	s8 *pchLogStart = (s8*)(ptOspLogSimpleHead + 1);

	memcpy(ptOspLogSimpleHead, &tOspLogHead, sizeof(TOspLogHead));
	ptOspLogSimpleHead->dwLength = dwLen;

	if(szContent)
	{
		strncpy(pchLogStart, szContent, dwLen);
		pchLogStart[dwLen] = '\0';//以防万一字符串漏掉终止符\0
	}

	LogQueWriteFinal(ptOspLogCommonHead);
}

void COspLog::LogQueWriteFinal(TOspLogCommonHead *ptOspLogMsg)
{
	if(NULL == ptOspLogMsg)
	{
		return;
	}

	TOsMsgStruc osMsg;

	osMsg.address = (void*)ptOspLogMsg;

	// 如果邮箱已满或待处理消息数异常, 丢掉本消息
	if((m_dwMsgIncome - m_dwMsgProcessed) + 50 >= m_dwMaxMsgWaiting)

    {
		OspFreeMem(ptOspLogMsg);
		m_dwMsgDropped++;
		return;
	}

	BOOL32 bSuccess;
	bSuccess = OspSndMsg(m_dwWriteQueHandle, (char *)&osMsg, sizeof(TOsMsgStruc), 500);
	if( !bSuccess )
	{
		m_dwMsgDropped++;
		OspFreeMem(ptOspLogMsg);
		// 如果LOG任务已经出错，或者队列已满，可以通过错误计数来观察
		// 下面这条语句没有任何意义，将每一句打印都替换为一条错误提示
		// 这在发生错误时，不但没用，反而干扰调试
		//printf("Osp: send message to mailbox failed in COspLog::LogQueWrite().\n");
		return;
	}
	m_dwMsgIncome++;
}

/*====================================================================
函数名：COspLog::LogQueOut
功能：从日志邮箱中取出消息，输出到文件中/屏幕上
算法实现：（可选项）
引用全局变量：
输入参数说明：
返回值说明:
====================================================================*/
void COspLog::LogQueOut()
{
	BOOL32 bExitTask = FALSE;
	BOOL32 bRet = FALSE;
    TOsMsgStruc osMsg = {0};
    TOspLogCommonHead *ptOspLogHead = NULL;

    while(TRUE)
    {
		u32 dwLen = 0;
        bRet = OspRcvMsg(m_dwReadQueHandle, 0xffffffff, (char *)&osMsg, sizeof(TOsMsgStruc), &dwLen);
		if( NULL == osMsg.address)
		{
			printf("COspLog::LogQueOut receive null msg \n");
			continue;
		}

		m_dwMsgProcessed++;

		/* 邮箱存放的是指向日志消息头的指针 */
		ptOspLogHead = (TOspLogCommonHead *)osMsg.address;
        if(OSP_LOG_INTERFACE_SIMPLE == ptOspLogHead->chOspInterfaceType)
        {
        	//处理OspPrintf类的打印记录
        	bExitTask = LogQueOutSimpleFinal((TOspLogHead*)ptOspLogHead->achData);
        	if(TRUE == bExitTask)
        	{
        		printf("COspLog::LogQueOut exit log task\n");
        		OspFreeMem((void *)osMsg.address);
				break;
        	}
        }
        else if(OSP_LOG_INTERFACE_UNIFORM == ptOspLogHead->chOspInterfaceType)
        {
			LogQueOutUniformFinal((TOspUniformLogHead*)ptOspLogHead->achData);
        }
        else
        {
        	printf("COspLog::LogQueOut invalid interface type\n");
        }

		/* 释放日志消息占用的内存 */
		OspFreeMem((void *)osMsg.address);
    }//end while

	//线程退出
    Quit();
	OspTaskExit();
}
/*=============================================================================
 函 数 名  : COspLog::LogQueOutSimpleFinal
 功能描述  : 此函数对应原来的OspPrintf/OspLog/OspTrcPrintf/OspMsgTrace的日志输出
             此次增加OSP_CLASSLOG GLBLOG接口，用LogQueOutUniformFinal
 算法实现  :
 参数说明  :
            [I]ptOspLogHead
 返回值说明: TRUE 需要结束线程 FALSE 不需要结束线程
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
u32 COspLog::LogQueOutSimpleFinal(TOspLogHead *ptOspLogHead)
{
	s8 *pchOutputMsg = NULL;
	u32 dwOutputLen = 0;

	if((ptOspLogHead->type != LOG_TYPE_MASKABLE) &&
	   (ptOspLogHead->type != LOG_TYPE_UNMASKABLE))
	{
		printf("COspLog::LogQueOutSimpleFinal invalid log type\n");
		return FALSE;
	}
	if( ptOspLogHead->dwLength > MAX_LOG_MSG_LEN)
	{
		printf("COspLog::LogQueOutSimpleFinal log len too long\n");
		return FALSE;
	}

	/* 要输出的日志长度 */
	dwOutputLen = ptOspLogHead->dwLength;

	/* 真正要输出的消息紧跟在日志消息头部后 */
	pchOutputMsg = (s8*)(ptOspLogHead + 1);

	// 头部全为0的日志表示任务退出请求
    if(dwOutputLen <= 0)
	{
		return TRUE;
	}

	if(ptOspLogHead->bToScreen)
	{
		m_dwScreenLogNum++;

		// send to local debug telenet client the string
		if( m_bScrnLogEnbl || (ptOspLogHead->type == LOG_TYPE_UNMASKABLE) )
		{
			TelePrint(pchOutputMsg);
		}  // teleOutEnable
	}  // log to screen

	//日志记录到文件
	//此日志为了兼容旧的机制(OspLog, OspPrintf等) 这些接口的日志保存到该对象
	if((ptOspLogHead->bToFile) && (TRUE == m_bLogFileEnable))
	{
		++m_dwRunFileLogNum;

		m_cRunLogFile.WriteLogFile(pchOutputMsg);
	}

	return FALSE;
}
/*=============================================================================
 函 数 名  : COspLog::LogQueOutUniformFinal
 功能描述  : 对应此次增加OSP_CLASSLOG GLBLOG接口的日志输出
 算法实现  :
 参数说明  :
            [I]ptOspLogHead
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
void COspLog::LogQueOutUniformFinal(TOspUniformLogHead *ptOspLogHead)
{
	//如果未启动日志 不写日志文件
	if(TRUE != m_bLogFileEnable)
	{
		return;
	}

	if (ptOspLogHead->dwLogType & OSP_LOG_FILE_ERR)
    {
    	++m_dwRunFileLogNum;

        m_cErrLogFile.WriteLogFile((char*)(ptOspLogHead+1));
    }

    if (ptOspLogHead->dwLogType & OSP_LOG_FILE_RUN)
    {
    	++m_dwErrorFileLogNum;

        m_cRunLogFile.WriteLogFile( (char*)(ptOspLogHead+1) );
    }
}

//设置日志文件的目录，文件大小及最大的日志文件个数
BOOL32 COspLog::OspSetLogFileParam(const char* szFileName, const char* szDir, u32 dwLogSizeMax_kb,
                                 u32 dwLogNumMax, u32 dwLogType)
{
	if(OSP_LOG_FILE_ERR == dwLogType)
	{
		return m_cErrLogFile.SetLogFileParam(szFileName, szDir, dwLogSizeMax_kb,
									  dwLogNumMax, dwLogType);
	}
	else if(OSP_LOG_FILE_RUN == dwLogType)
	{
		return m_cRunLogFile.SetLogFileParam(szFileName, szDir, dwLogSizeMax_kb,
									  dwLogNumMax, dwLogType);
	}
}


void COspLog::Show(void)
{
	char achBuf[500]={0};
	int nLen = 0;

	nLen += sprintf(achBuf+nLen, "%s", "------------------------\n");

	nLen += sprintf(achBuf+nLen, "msgIncome=%d, msgProcessed=%d, msgDropped=%d, maxMsgWaiting=%d, ",
					m_dwMsgIncome, m_dwMsgProcessed, m_dwMsgDropped, m_dwMaxMsgWaiting);

	nLen += sprintf(achBuf+nLen, "logFileOpen=%d, ", m_bLogFileEnable ? 1:0);
	if(m_bLogFileEnable)
	{
		nLen += sprintf(achBuf+nLen, "errlog: logFileDir=\"%s\", maxFileSize=%d, maxFileNum=%d",
		             m_cErrLogFile.GetLogDir(), m_cErrLogFile.GetLogFileSizeMax(),
		             m_cErrLogFile.GetLogFileNumMax());
		nLen += sprintf(achBuf+nLen, "runlog: logFileDir=\"%s\", maxFileSize=%d, maxFileNum=%d",
		             m_cRunLogFile.GetLogDir(), m_cRunLogFile.GetLogFileSizeMax(),
		             m_cRunLogFile.GetLogFileNumMax());
	}

	nLen += sprintf(achBuf+nLen, "scrnLogEnable=%d, longMsgTrcEnable=%d, ",
					m_bScrnLogEnbl ? 1:0, m_bLMsgDumpEnbl ? 1:0);

	nLen += sprintf(achBuf+nLen, "globalScrnLogLevel=%d, globFileLogLevel=%d, ",
					m_byLogScreenLevel, m_byLogFileLevel);

	nLen += sprintf(achBuf+nLen, "scrnLogNum[%d], fileLogNum[%d], errLogNum[%d]\n",
					m_dwScreenLogNum, m_dwRunFileLogNum, m_dwErrorFileLogNum);

	TelePrint(achBuf);
}


static u32 _OspGetTid()
{
#if defined(WIN32)
    return (u32)GetCurrentThreadId();
#elif defined(_LINUX_)
    return (u32)syscall(224);
#endif
}
//返回值为字符串有效长度，不包括\0
static u32 _OspGetTidStr(u32 dwBufferLen, s8* pchBuffer)
{
	if(NULL == pchBuffer)
	{
		return 0;
	}
    u32 dwLen = SNPRINTF(pchBuffer, dwBufferLen, "ThreadId[%lu]: ", _OspGetTid());
	if(dwLen >= dwBufferLen)
	{
		pchBuffer[dwBufferLen - 1] = '\0';
		return dwBufferLen - 1;
	}

	return dwLen;
}


COspXLog::COspXLog(const s8* szFileName, s32 nFileLine, const s8* szClassName, const s8* szFunName)
            : m_szFileName(szFileName), m_nFileLine(nFileLine),
            m_szClassName(szClassName), m_szFunName(szFunName)
{

}

COspXLog::~COspXLog(){};


/*=============================================================================
 函 数 名  : COspXLog::operator()
 功能描述  : 打印输出日志
 算法实现  :
 参数说明  :
            [I]byLogLev 日志级别
 返回值说明: 文件名长度 包括'\0'
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
void COspXLog::operator()(u8 byLogLev, const s8* szFormat, ...) const
{
	//问题修改: osp退出后，若写日志，调用到OspAllocMem()会崩溃
	if ((FALSE == g_Osp.m_bInitd) || g_Osp.m_bKillOsp)
    {
        return;
    }
	TOspLogContent cLogContent;

    GETVALISTSTR(szFormat, cLogContent.m_achBodyField);

	GetCompileInfo(TOspLogContent::MAX_COMPILE_FIELD_LEN, cLogContent.m_achCompileField);

    cLogContent.m_byLogLev = byLogLev;

   	u32 dwPos = 0;
	u32 dwFreeSize = TOspLogContent::MAX_MOD_LEVEL_STR_LEN;

	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, '[');
	const char *pchLogLev = OspGetStrLogLevel(byLogLev);
	if(NULL != pchLogLev)
	{
		u32 dwLen = strlen(pchLogLev);
		INSERT_C_STRING(cLogContent.m_achModLev, dwPos, dwFreeSize, pchLogLev, dwLen);
	}
	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, ']');
	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, '\0');

    _OspGetTidStr(TOspLogContent::MAX_PRIFIXED_FIELD_LEN, cLogContent.m_achOspPrifixField);

    if (byLogLev>g_byGlbLogPrintLev )
    {
        cLogContent.m_bIsPrintScreen =  FALSE;
    }

    OspUniformPrintf(cLogContent);
}
/*=============================================================================
 函 数 名  : COspXLog::operator()
 功能描述  : 打印输出日志
 算法实现  :
 参数说明  :
 			[I]byLogMod 日志模块
            [I]byLogLev 日志级别
 返回值说明: 文件名长度 包括'\0'
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
void COspXLog::operator()(u8 byLogMod, u8 byLogLev, const s8* szFormat, ...) const
{
    //问题修改: osp退出后，若写日志，调用到OspAllocMem()会崩溃
	if ((FALSE == g_Osp.m_bInitd) || g_Osp.m_bKillOsp)
    {
        return;
    }

	TOspLogContent cLogContent;
    GETVALISTSTR(szFormat, cLogContent.m_achBodyField);

	GetCompileInfo(TOspLogContent::MAX_COMPILE_FIELD_LEN, cLogContent.m_achCompileField);

    cLogContent.m_byLogLev = byLogLev;


    u32 dwPos = 0;
	u32 dwFreeSize = TOspLogContent::MAX_MOD_LEVEL_STR_LEN;

	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, '[');

	const char *pchName = OspGetLogStrModule(byLogMod);
	u32 dwLen;
	if(NULL != pchName)
	{
		dwLen = strlen(pchName);
		INSERT_C_STRING(cLogContent.m_achModLev, dwPos, dwFreeSize, pchName, dwLen);
	}

	pchName = OspGetStrLogLevel(byLogLev);
	if(NULL != pchName)
	{
		INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, '-');
		dwLen = strlen(pchName);
		INSERT_C_STRING(cLogContent.m_achModLev, dwPos, dwFreeSize, pchName, dwLen);
	}

	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, ']');
	INSERT_CHAR(cLogContent.m_achModLev, dwPos, dwFreeSize, '\0');

    _OspGetTidStr(TOspLogContent::MAX_PRIFIXED_FIELD_LEN, cLogContent.m_achOspPrifixField);

    if (byLogLev>g_byGlbLogPrintLev )
    {
        cLogContent.m_bIsPrintScreen =  FALSE;
    }

    OspUniformPrintf(cLogContent);
}

/*=============================================================================
 函 数 名  : COspXLog::GetCompileInfo
 功能描述  : 提取编译信息(file-line-class-function)
 算法实现  :
 参数说明  :
            [I]szPath
            [I]dwBufferLen pchBuffer允许的最大长度 包括'\0'
            [O]pchBuffer 文件名存放空间起始地址
 返回值说明: 字符串长度 包括'\0'
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
u32 COspXLog::GetCompileInfo(u32 dwBufferLen, s8* pchBuffer) const
{
	if(NULL == pchBuffer)
	{
		return 0;
	}

	u32 dwPos = 0; //插入字符位置
	u32 dwFreeSize = dwBufferLen; //剩余的可存储字节数

	//插入文件名
    u32 dwLen = GetFileNameFromPath(m_szFileName, dwFreeSize, pchBuffer + dwPos);
    if(0 != dwLen)
    {
    	dwPos += (dwLen - 1); //扣除\0
		dwFreeSize -= (dwLen - 1);//扣除\0
    }

	//插入'('
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '(');
	//插入行号
	s8 szFileLine[10] = {0};
    sprintf(szFileLine, "%d", m_nFileLine);
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, szFileLine, strlen(szFileLine));
	//插入')'
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ')');

    //插入' '
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ' ');

	//插入类型名
	dwLen = GetTypeName(m_szClassName, dwFreeSize, pchBuffer + dwPos);
    if (0 != dwLen)
    {
    	dwPos += dwLen - 1; //扣除\0
		dwFreeSize -= dwLen + 1;//扣除\0
        //插入::
		INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ':');
        INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ':');
    }
	//插入函数名
    if (m_szFunName != NULL)
    {
    	INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, m_szFunName, strlen(m_szFunName));
        //插入()
		INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '(');
        INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ')');
    }

    INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '\0');

	return dwBufferLen - dwFreeSize;
}

/*=============================================================================
 函 数 名  : COspXLog::GetFileNameFromPath
 功能描述  : 提取文件名，从文件全名中取出不包括路径名的部分
 算法实现  :
 参数说明  :
            [I]szPath
            [I]dwBufferLen pchBuffer允许的最大长度 包括'\0'
            [O]pchBuffer 文件名存放空间起始地址
 返回值说明: 文件名长度 包括'\0'
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
u32 COspXLog::GetFileNameFromPath(const char* szPath, u32 dwBufferLen, char* pchBuffer)
{
    if ((NULL == szPath)||(NULL) == pchBuffer)
    {
        return 0;
    }

    u32 dwTotalLen = strlen(szPath);

    const char* pNameStartPos = szPath + dwTotalLen;

    while (pNameStartPos > szPath)
    {
        if (*pNameStartPos == '/' || *pNameStartPos == '\\')
        {
            pNameStartPos++;
            break;
        }

        pNameStartPos--;
    }

	//拷贝长度包括'\0'
    u32 dwActualLen = szPath + dwTotalLen - pNameStartPos + 1;
	dwActualLen = (dwActualLen <= dwBufferLen)? dwActualLen : dwBufferLen;

	memcpy(pchBuffer, pNameStartPos, dwActualLen);

    return dwActualLen;
}
/*=============================================================================
 函 数 名  : COspXLog::GetTypeName
 功能描述  : 提取类型名
 算法实现  :
 参数说明  :
            [I]szTypeNameRaw 原始的类名
            [I]dwBufferLen pchBuffer允许的最大长度 包括'\0'
            [O]pchBuffer 提取的实际类名存放空间起始地址
 返回值说明: 文件名长度 包括'\0'
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
u32 COspXLog::GetTypeName(const char* szTypeNameRaw, u32 dwBufferLen, char* pchBuffer)
{

    if ((NULL == szTypeNameRaw)||(NULL == pchBuffer))
    {
        return 0;
    }

    u32 dwActlen;

#ifdef WIN32
    //win32下直接就是真实的类型名
	dwActlen = strlen(szTypeNameRaw) + 1;
	dwActlen = (dwActlen < dwBufferLen)? dwActlen : dwBufferLen;
	memcpy(pchBuffer, szTypeNameRaw, dwActlen);
    return dwActlen;
#endif

#ifdef _LINUX_

	dwActlen = 0;
    //目前解析mangle name的方式非常耗时，默认不进行解析
    if (g_bIsParseMangleName == TRUE)
    {
        //使用shell命令c++filt得到真实的类型名
        char szTypeName[256] = {0};
        char achCmd[256] = {0};

        sprintf(achCmd, "c++filt %s", szTypeNameRaw);

        //使用管道执行命令，得到类型名
        FILE *fp = popen(achCmd, "r");
        if (fp == NULL)
        {
            return 0;
        }

        char* lpszRedStr = fgets(szTypeName, sizeof(szTypeName), fp);
        if(NULL != lpszRedStr)
        {
            u32 dwLen=strlen(szTypeName);

            dwActlen = dwLen + 1;
            if(szTypeName[dwLen-1] == '\n' || szTypeName[dwLen-1] == '\r')
            {
                szTypeName[dwLen-1] = '\0';
                dwActlen = dwLen;
            }
        }

        pclose(fp);

		dwActlen = (dwActlen < dwBufferLen)? dwActlen : dwBufferLen;
        memcpy(pchBuffer, szTypeName, dwActlen);
    }

    return dwActlen;

#endif
}

TOspLogContent::TOspLogContent()
{
	m_achBodyField 		[0] = '\0';
    m_achModLev			[0] = '\0';
	m_achOspPrifixField	[0] = '\0';
	m_achCompileField	[0] = '\0';

	m_byLogLev = OSP_NOLOG_LEV;
	m_bIsPrintScreen = TRUE;
}

TOspLogContent::~TOspLogContent()
{
}

#if 0
#endif
//日志文件管理及日志输出
COspXLogFile::COspXLogFile()
{
	m_achLogDir[0]='\0';
	m_achLogNamePrefix[0]='\0';
	m_achCurFile[0]='\0';
	m_pFile = NULL;
    SetLogFileParam("osplog_", DEFAULT_LOG_DIR_NAME, DEFAULT_LOG_FILE_SIZE_kb, DEFAULT_LOG_FILE_NUM, OSP_LOG_FILE_ERR);
}

COspXLogFile::~COspXLogFile()
{
    if (m_pFile != NULL)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

BOOL32 COspXLogFile::SetLogFileParam(const char* szFileName, const char* szDir, u32 nLogSizeMax_kb,
								   u32 nLogNumMax, u32 dwLogType)
{
    COspAutoLock cAutoLock(m_cLock);

    BOOL32 bRet = FALSE;

    m_dwLogType = dwLogType;

    strncpy(m_achLogDir, szDir, MAX_FILE_DIR_NAME_LEN);
    m_achLogDir[MAX_FILE_DIR_NAME_LEN - 1] = '\0';

#ifdef _LINUX_

#else
    //windows也可能使用"/"作为相对目录分隔符，首先替换掉
    replace_str(m_achLogDir, DIR_SEP, '\\');
#endif

    //补成完整目录路径
    u32 dwLen = strlen(m_achLogDir);
    if (m_achLogDir[dwLen - 1] != DIR_SEP)
    {
    	if(dwLen < MAX_FILE_DIR_NAME_LEN)
    	{
    		m_achLogDir[dwLen] = DIR_SEP;
        	m_achLogDir[dwLen + 1] = '\0';
    	}
    	else
    	{
    		m_achLogDir[MAX_FILE_DIR_NAME_LEN - 2] = DIR_SEP;
        	m_achLogDir[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
    	}
    }

    if (!ExistFileOrDir(m_achLogDir))
	{
		bRet = RecureMakeDir(m_achLogDir);
		if(!bRet)
		{
		    return FALSE;
		}
	}
	u32 dwPos = 0;
    u32 dwFreeSize = MAX_FILE_DIR_NAME_LEN;
	//文件名以用户指定的名字为前缀
    dwLen = SNPRINTF(m_achLogNamePrefix, dwFreeSize, "%s-", szFileName);
    dwFreeSize -= dwLen;
	dwPos += dwLen;

	//文件名附加上进程名字
	dwLen = GetProcessName(dwFreeSize, m_achLogNamePrefix + dwPos);
	dwFreeSize -= dwLen;
	dwPos += dwLen;

    m_nLogFileSizeMax = nLogSizeMax_kb;
    m_nLogFileNumMax = nLogNumMax;

    return TRUE;
}

#define IsStartWithStr(parent, sub) (strstr(parent, sub) == parent)

//日志文件
#define CAPACITY_1K    (1024)

void COspXLogFile::WriteLogFile(const char* pchLog)
{
	COspAutoLock cAutoLock(m_cLock);

	if ('\0' == m_achLogDir[0])
	{
		printf("OSP: WriteLogFile log dir is null\nd");
		return;
	}

	if ('\0' == m_achCurFile[0])
	{
		GetCurFile(MAX_FILE_DIR_NAME_LEN, m_achCurFile);
	}

	//寻找可写文件
	if (ExistFileOrDir(m_achCurFile)) //文件存在
	{
		struct stat fileStat;
		stat(m_achCurFile, &fileStat);
		long dwLogSize = fileStat.st_size;

		if (dwLogSize >= (long)(m_nLogFileSizeMax*CAPACITY_1K)) //超过最大log容量,更改文件
		{
			//先关闭当前文件
			if (m_pFile != NULL)
			{
				fclose(m_pFile);
				m_pFile = NULL;
			}

			//删除最老的文件
			if (GetFileNum() >= m_nLogFileNumMax)
			{
				char m_achFirstFile[MAX_FILE_DIR_NAME_LEN];
				GetFirstFile(MAX_FILE_DIR_NAME_LEN, m_achFirstFile);
				DelFile(m_achFirstFile);
			}

			//创建新文件
			GetNewFile(MAX_FILE_DIR_NAME_LEN, m_achCurFile);
		}
		else
		{
			//文件存在，并且未达到最大容量，是可写文件
		}
	}
	else //文件不存在，肯定可写，直接创建
	{
		//但文件句柄不为空，可能是用户手动删除的日志文件，是无效句柄，需要close
		if (m_pFile != NULL)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}

	if (m_pFile == NULL)
	{
		const char* szOpenMode = "a+";	 //默认打开方式,追加
		m_pFile = fopen(m_achCurFile, szOpenMode);
		if(m_pFile == NULL)
		{
			printf("osp-COspXLogFile::WriteLogFile-fopen[%s] fail\n", m_achCurFile);
		}
	}

	if(m_pFile != NULL)
	{
		if (fwrite(pchLog, strlen(pchLog), 1, m_pFile) != 1)
		{
#ifdef _LINUX_
			syslog(LOG_INFO, "osp-COspXLogFile::WriteLogFile-fwrite file[%s] fail, size[%u], buf[%s]\n",
				m_achCurFile,  strlen(pchLog), pchLog);
#endif
			return;
		}

		if (fflush(m_pFile) != 0)
		{
#ifdef _LINUX_
			syslog(LOG_INFO, "osp-COspXLogFile::WriteLogFile-fflush file[%s] fail, size[%u], buf[%s]\n",
				m_achCurFile,  strlen(pchLog), pchLog);
#endif
			return;
		}
	}
}


u32 COspXLogFile::GetCurFile(u32 dwBuffenLen, char* pchBuffer)
{
    if (GetFileNum() == 0)
    {
        //新建一个文件
        return GetNewFile(dwBuffenLen, pchBuffer);
    }
    else
    {
        //找一个最新的文件
        return GetLastFile(dwBuffenLen, pchBuffer);
    }
}
//构造新文件名
u32 COspXLogFile::GetNewFile(u32 dwBuffenLen, char* pchBuffer)
{
	if (NULL == pchBuffer)
	{
		return 0;
	}

	pchBuffer[0] = '\0';

    //构造新文件名
    u32 dwPos = 0;
	u32 dwFreeSize = dwBuffenLen;
	INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
	INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize,
		GetPrefixName(), strlen(GetPrefixName()));
    u32 dwLen = GetFileStamp(dwFreeSize, pchBuffer + dwPos);
	dwPos += dwLen;
	dwFreeSize -= dwLen;
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize,
    				GetSuffixName(), strlen(GetSuffixName()));

    return strlen(pchBuffer);
}

const char* COspXLogFile::GetSuffixName()
{
    if (m_dwLogType == OSP_LOG_FILE_ERR)
    {
        return ".errlog";
    }
    else if (m_dwLogType == OSP_LOG_FILE_RUN)
    {
        return ".runlog";
    }
    else
    {
        return DEFAULT_LOG_SUFFIX;
    }
}
//返回值为字符串有效长度，不包括\0
u32 COspXLogFile::GetFirstFile(u32 dwBuffenLen, char* pchBuffer)
{
    if(NULL == pchBuffer)
	{
		return 0;
	}
	pchBuffer[0] = '\0';

    //获取最old的文件
    u32 dwPos;
    u32 dwFreeSize;
    time_t tModifyFileTime = 0;
    char achFileName[MAX_FILE_DIR_NAME_LEN];

#ifdef _LINUX_

#ifdef USE_SCANDIR
    struct dirent** ppNamelist = NULL;
    s32 nEntryNum = 0;

    nEntryNum = scandir(m_achLogDir, &ppNamelist, 0, alphasort);
    if (nEntryNum < 0 || ppNamelist == NULL)
    {
        printf("scandir [%s] fail!!!\n", m_achLogDir);
        return 0;
    }

    s32 nSaveEntryNum = nEntryNum;
    struct stat statFile;
    while(nEntryNum--)
    {
        struct dirent* pFileEntry = ppNamelist[nEntryNum];

        if(pFileEntry->d_name[0] == '.')
            continue;

        char strFullPath[MAX_FILE_DIR_NAME_LEN];
		dwPos = 0;
    	dwFreeSize = MAX_FILE_DIR_NAME_LEN;
    	INSERT_C_STRING(strFullPath, dwPos, dwFreeSize,
    	                m_achLogDir, strlen(m_achLogDir));
    	INSERT_C_STRING(strFullPath, dwPos, dwFreeSize,
    	                pFileEntry->d_name, strlen(pFileEntry->d_name));

		memset(&statFile, 0, sizeof(struct stat));
		if(stat(strFullPath, &statFile) < 0)
		{
			printf("stat error = %s\n", strerror(errno));
		}
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }
        pFileEntry->d_name, statFile.st_atime, statFile.st_mtime, statFile.st_ctime);

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
            !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        if (tModifyFileTime == 0)
        {
            strncpy(achFileName, pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }

        if (statFile.st_mtime < tModifyFileTime)
        {
            strncpy(achFileName, pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }
    }

    FreeDirNameList(ppNamelist, nSaveEntryNum);
#else //end if USE_SCANDIR
    DIR* pDir = opendir(m_achLogDir);
    if(pDir == NULL)
    {
        return 0;
    }

    char achSaveWorkDir[MAX_FILE_DIR_NAME_LEN];
    OspGetProcessPath(MAX_FILE_DIR_NAME_LEN, achSaveWorkDir);

    struct dirent* pFileEntry = NULL;
    struct stat statFile;

    while((pFileEntry = readdir(pDir)) != NULL)
    {
        if(pFileEntry->d_name[0] == '.')
            continue;

        stat(pFileEntry->d_name, &statFile);
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
            !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        if (tModifyFileTime == 0)
        {
            strncpy(achFileName, tFindData.name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }

        if (statFile.st_mtime < tModifyFileTime)
        {
            strncpy(achFileName, tFindData.name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }
    }

    chdir(achSaveWorkDir);
    closedir(pDir);
#endif//end else USE_SCANDIR

#else //end if _LINUX_
    char achRoot[MAX_FILE_DIR_NAME_LEN];
    dwPos = 0;
    dwFreeSize = MAX_FILE_DIR_NAME_LEN;
    INSERT_C_STRING(achRoot, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
	INSERT_C_STRING(achRoot, dwPos, dwFreeSize, "*.*", strlen("*.*"));

	//vc6 没有intptr_t类型定义 实际上等价于long类型
	#ifndef intptr_t
	# define intptr_t long
	#endif

    intptr_t handle;
    struct _finddata_t tFindData;
    handle=_findfirst(achRoot, &tFindData);

    if(-1==handle)
        return 0;

    while(0 == _findnext(handle, &tFindData))
    {
        if(tFindData.name[0] == '.')
            continue;

        if (tFindData.attrib&_A_SUBDIR)
        {
            continue;
        }

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(tFindData.name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
            !IsStartWithStr(tFindData.name, GetPrefixName()))
        {
            continue;
        }

        if (tModifyFileTime == 0)
        {
            strncpy(achFileName, tFindData.name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';

            tModifyFileTime = tFindData.time_write;
        }

        if (tFindData.time_write < tModifyFileTime)
        {
            strncpy(achFileName, tFindData.name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';

            tModifyFileTime = tFindData.time_write;
        }
    }

    _findclose(handle);
#endif////end _LINUX_

    //添加目录
    dwPos = 0;
    dwFreeSize = dwBuffenLen;
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, achFileName, strlen(achFileName));

    return strlen(pchBuffer);
}

u32 COspXLogFile::GetLastFile(u32 dwBuffenLen, char* pchBuffer)
{
	if(NULL == pchBuffer)
	{
		return 0;
	}
	pchBuffer[0] = '\0';

    //获取最新的文件
    u32 dwPos;
    u32 dwFreeSize;
    time_t tModifyFileTime = 0;
    char achFileName[MAX_FILE_DIR_NAME_LEN];

#ifdef _LINUX_

#ifdef USE_SCANDIR
    struct dirent** ppNamelist = NULL;
    s32 nEntryNum = 0;

    nEntryNum = scandir(m_achLogDir, &ppNamelist, 0, alphasort);
    if (nEntryNum < 0 || ppNamelist == NULL)
    {
        printf("scandir [%s] fail!!!\n", m_achLogDir);
        return 0;
    }

    s32 nSaveEntryNum = nEntryNum;
    struct stat statFile;
    while(nEntryNum--)
    {
        struct dirent* pFileEntry = ppNamelist[nEntryNum];

        if(pFileEntry->d_name[0] == '.')
            continue;

		char strFullPath[MAX_FILE_DIR_NAME_LEN];
		dwPos = 0;
    	dwFreeSize = MAX_FILE_DIR_NAME_LEN;
    	INSERT_C_STRING(strFullPath, dwPos, dwFreeSize,
    	                m_achLogDir, strlen(m_achLogDir));
    	INSERT_C_STRING(strFullPath, dwPos, dwFreeSize,
    	                pFileEntry->d_name, strlen(pFileEntry->d_name));

        stat(strFullPath, &statFile);
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }

		//名字的前缀和后缀都相等 才能写入
		char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if ( strcmp(GetSuffixName(), achSuffixName)!=0 ||
             !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        if (statFile.st_mtime > tModifyFileTime)
        {
            strncpy(achFileName, pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }
    }

    FreeDirNameList(ppNamelist, nSaveEntryNum);
#else//end if USE_SCANDIR

    DIR* pDir = opendir(m_achLogDir);
    if(pDir == NULL)
    {
        return 0;
    }

    char achSaveWorkDir[MAX_FILE_DIR_NAME_LEN];
    OspGetProcessPath(MAX_FILE_DIR_NAME_LEN, achSaveWorkDir);

    chdir(m_achLogDir);

    struct dirent* pFileEntry = NULL;
    struct stat statFile;

    while((pFileEntry = readdir(pDir)) != NULL)
    {
        if(pFileEntry->d_name[0] == '.')
            continue;

        stat(pFileEntry->d_name, &statFile);
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }

		char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
            !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        if (statFile.st_mtime > tModifyFileTime)
        {
            strncpy(achFileName, pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = statFile.st_mtime;
        }
    }

    chdir(achSaveWorkDir);
    closedir(pDir);

#endif//end USE_SCANDIR

#else//end if _LINUX_

    char achRoot[MAX_FILE_DIR_NAME_LEN];
    dwPos = 0;
    dwFreeSize = MAX_FILE_DIR_NAME_LEN;
    INSERT_C_STRING(achRoot, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
	INSERT_C_STRING(achRoot, dwPos, dwFreeSize, "*.*", strlen("*.*"));

    intptr_t handle;
    struct _finddata_t tFindData;
    handle=_findfirst(achRoot, &tFindData);

    if(-1==handle)
        return 0;

    while(0 == _findnext(handle, &tFindData))
    {
        if(tFindData.name[0] == '.')
            continue;

        if (tFindData.attrib&_A_SUBDIR)
        {
            continue;
        }

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(tFindData.name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
        if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
            !IsStartWithStr(tFindData.name, GetPrefixName()))
        {
            continue;
        }

        if (tFindData.time_write > tModifyFileTime)
        {
            strncpy(achFileName, tFindData.name, MAX_FILE_DIR_NAME_LEN);
            achFileName[MAX_FILE_DIR_NAME_LEN - 1] = '\0';
            tModifyFileTime = tFindData.time_write;
        }
    }

    _findclose(handle);

#endif//end if _LINUX_

    //添加目录
    //strFileName = m_strLogDir + strModifyFile;

	dwPos = 0;
    dwFreeSize = dwBuffenLen;
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, achFileName, strlen(achFileName));

    return strlen(pchBuffer);
}



u32 COspXLogFile::GetFileNum()
{
    u32 nFileNum = 0;
	u32 dwPos;
	u32 dwFreeSize;

#ifdef _LINUX_

#ifdef USE_SCANDIR
    struct dirent** ppNamelist = NULL;
    s32 nEntryNum = 0;

    nEntryNum = scandir(m_achLogDir, &ppNamelist, 0, alphasort);
    if (nEntryNum < 0 || ppNamelist == NULL)
    {
        printf("scandir [%s] fail!!!\n", m_achLogDir);
        return 0;
    }

    s32 nSaveEntryNum = nEntryNum;
    struct stat statFile;
    while(nEntryNum--)
    {
        struct dirent* pFileEntry = ppNamelist[nEntryNum];

        if(pFileEntry->d_name[0] == '.')
            continue;
		if(strlen(m_achLogDir) + strlen(pFileEntry->d_name) > MAX_FILENAME_LEN)
			continue;
		char achFullPath[MAX_FILENAME_LEN];
	    strncpy(achFullPath, m_achLogDir, MAX_FILENAME_LEN);
	    strcat(achFullPath, pFileEntry->d_name);
        //string strFullPath = m_achLogDir + pFileEntry->d_name;

        stat(achFullPath, &statFile);
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
		if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
		    !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        nFileNum++;
    }

    FreeDirNameList(ppNamelist, nSaveEntryNum);

#else//end if USE_SCANDIR
    DIR* pDir = opendir(m_achLogDir);
    if(pDir == NULL)
    {
        return 0;
    }

    char achSaveWorkDir[MAX_FILE_DIR_NAME_LEN];
    OspGetProcessPath(MAX_FILE_DIR_NAME_LEN, achSaveWorkDir);
    chdir(m_achLogDir);

    struct dirent* pFileEntry = NULL;
    struct stat statFile;

    while((pFileEntry = readdir(pDir)) != NULL)
    {
        if(pFileEntry->d_name[0] == '.')
            continue;

        stat(pFileEntry->d_name, &statFile);
        if(S_ISDIR(statFile.st_mode))
        {
            continue;
        }

        char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(pFileEntry->d_name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
		if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
		    !IsStartWithStr(pFileEntry->d_name, GetPrefixName()))
        {
            continue;
        }

        nFileNum++;
    }

    chdir(achSaveWorkDir);
    closedir(pDir);
#endif//end USE_SCANDIR

#else//end if _LINUX_

	char achRoot[MAX_FILE_DIR_NAME_LEN];
	dwPos = 0;
	dwFreeSize = MAX_FILE_DIR_NAME_LEN;
	INSERT_C_STRING(achRoot, dwPos, dwFreeSize, m_achLogDir, strlen(m_achLogDir));
	INSERT_C_STRING(achRoot, dwPos, dwFreeSize, "*.*", strlen("*.*"));

    intptr_t handle;
    struct _finddata_t tFindData;
    handle=_findfirst(achRoot, &tFindData);

    if(-1==handle)
        return 0;

    while(0 == _findnext(handle, &tFindData))
    {
        if(tFindData.name[0] == '.')
            continue;

        if (tFindData.attrib&_A_SUBDIR)
        {
            continue;
        }

		char achSuffixName[MAX_FILE_DIR_NAME_LEN];
		GetSuffixFromFile(tFindData.name, MAX_FILE_DIR_NAME_LEN, achSuffixName);
		if (strcmp(GetSuffixName(), achSuffixName)!=0 ||
			!IsStartWithStr(tFindData.name, GetPrefixName()))
		{
			continue;
		}

        nFileNum++;
    }

    _findclose(handle);
#endif//end _LINUX_

    return nFileNum;
}

u32 COspXLogFile::GetFileStamp(u32 dwBuffenLen, char* pchBuffer)
{
    //文件名称的时间标识
    TOspTimeInfo tTimeInfo;
    memset(&tTimeInfo, 0, sizeof(tTimeInfo));
    OspGetTimeInfo(&tTimeInfo);

    u32 dwLen = SNPRINTF(pchBuffer, dwBuffenLen, "-%04d-%02d-%02d-%02d-%02d-%02d",
        tTimeInfo.m_wYear, tTimeInfo.m_wMonth, tTimeInfo.m_wDay,
        tTimeInfo.m_wHour, tTimeInfo.m_wMinute, tTimeInfo.m_wSecond);
    if (dwLen >= dwBuffenLen)
	{
		pchBuffer[dwBuffenLen - 1] = '\0';
		return dwBuffenLen - 1;
	}

	return dwLen;
}

u32 COspXLogFile::GetSuffixFromFile(const char* pchFileName,
							     u32 dwBuffenLen, char* pchBuffer)
{
	if((NULL == pchFileName)||(NULL == pchBuffer))
	{
		return 0;
	}

	pchBuffer[0] = '\0';
    const char* pchLastDot = strrchr(pchFileName, '.');
    if (pchLastDot != NULL)
    {
        strncpy(pchBuffer, pchLastDot, dwBuffenLen);
        pchBuffer[dwBuffenLen - 1] = '\0';
    }

    return strlen(pchBuffer);
}


/*====================================================================
函数名：LogSysInit
功能：日志系统初始化，由OspInit()调用。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：无

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 LogSysInit()
{
	if(TRUE != g_cOspLog.Initialize())
	{
		return FALSE;
	}

	//设置OSP模块的日志级别和模块名称
	#if _DEBUG
	OspSetModuleLogLev(MOD_OSP, OSP_FREQ_EVENT_LEV, "OSP");
	OspSetModuleLogLev(MOD_OSPEXT, OSP_FREQ_EVENT_LEV, "OSPEXT");
	OspSetModuleLogLev(MOD_OSPPROT, OSP_FREQ_EVENT_LEV, "OSPPROT");
	OspSetModuleLogLev(MOD_OSPSIP, OSP_FREQ_EVENT_LEV, "OSPSIP");
	#else
	OspSetModuleLogLev(MOD_OSP, OSP_CRITICAL_LEV, "OSP");
	OspSetModuleLogLev(MOD_OSPEXT, OSP_CRITICAL_LEV, "OSPEXT");
	OspSetModuleLogLev(MOD_OSPPROT, OSP_CRITICAL_LEV, "OSPPROT");
	OspSetModuleLogLev(MOD_OSPSIP, OSP_CRITICAL_LEV, "OSPSIP");
	#endif

	return TRUE;
}

/*====================================================================
函数名：ospver
功能：显示Osp的当前版本号
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：无
====================================================================*/
API void ospver()
{
	OspVerPrintf();
}
/*====================================================================
函数名：OspLog
功能：把相应的内容显示到屏幕,存储到文件
算法实现：（可选项）
引用全局变量：
输入参数说明：uLevel: log级别
              szFormat: Log 的格式
              ...: 变长参数表
返回值说明：无
====================================================================*/
API void OspLog(u8 byLevel, const char *szFormat, ...)
{
    va_list pvList;
    TOspLogHead tOspLogHead;
	char msg[MAX_LOG_MSG_LEN];
	u32 actLen = 0;
	BOOL32 bToScreen = FALSE;
	BOOL32 bToFile = FALSE;

	if(szFormat == NULL)
	{
		return;
	}

	bToScreen = (byLevel <= g_cOspLog.m_byLogScreenLevel) ? TRUE:FALSE;
	bToFile = (byLevel <= g_cOspLog.m_byLogFileLevel) ? TRUE:FALSE;

	if(!bToScreen && !bToFile)
	{
		return;
	}

	tOspLogHead.type = LOG_TYPE_MASKABLE;
	tOspLogHead.bToScreen = bToScreen;
	tOspLogHead.bToFile = bToFile;

	va_start(pvList, szFormat);
	actLen = _vsnprintf(msg, MAX_LOG_MSG_LEN, szFormat, pvList);
	va_end(pvList);
    if(actLen <= 0)
    {
        printf("Osp: _vsnprintf() failed in OspLog().\n");
        return;
    }
    if (actLen >= MAX_LOG_MSG_LEN)
    {
        printf("Osp: msg's length is over MAX_LOG_MSG_LEN in OspLog().\n");
        return;
    }
    OspLogQueWrite(tOspLogHead, msg, actLen);
}
/*=============================================================================
 函 数 名  : OspUniformPrintf
 功能描述  : 日志记录，可以控制特定模块的特定日志不输出
 			 对应此次增加OSP_CLASSLOG GLBLOG接口的日志输出
             此接口的日志最终会保存到文件
 算法实现  :
 参数说明  :
            [I]ptOspLogHead
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
API void OspUniformPrintf(TOspLogContent& cLogContent)
{
	//问题修改: osp退出后，若写日志，调用到OspAllocMem()会崩溃
	if ((FALSE == g_Osp.m_bInitd) || g_Osp.m_bKillOsp)
    {
        return;
    }

    s8 achFullMsg[MAX_LOG_MSG_LEN];

	u32 dwLen;
    u32 dwPos = 0;
    u32 dwFreeSize = MAX_LOG_MSG_LEN;

	//日志文件写日志的完整信息
	//嵌入时间
    if (g_byPrintTimeLev != 0)
    {
        INSERT_CHAR(achFullMsg, dwPos, dwFreeSize, '[');
	    if (g_byPrintTimeLev == 255)
	    {
	    	dwLen = COspTimeInfo::GetCurrStrTime_ms(dwFreeSize, achFullMsg + dwPos);
	    }
	    else
	    {
	        dwLen = COspTimeInfo::GetCurrStrTime(dwFreeSize, achFullMsg + dwPos);
	    }
	    dwPos += (dwLen);
	    dwFreeSize -= (dwLen);
	    INSERT_CHAR(achFullMsg, dwPos, dwFreeSize, ']');
    }

	//嵌入日志所属模块信息
	INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
					cLogContent.m_achModLev, strlen(cLogContent.m_achModLev));

	//前缀信息
    if (g_byPrintOspPrefix != 0)
    {
        INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  cLogContent.m_achOspPrifixField, strlen(cLogContent.m_achOspPrifixField));
    }

	//嵌入日志正文
	INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  			cLogContent.m_achBodyField, strlen(cLogContent.m_achBodyField));

	//嵌入编译信息
    if (g_byPrintCompileInfo != 0)
    {
		//替换之前的换行符
		if (achFullMsg[dwPos - 1] == '\n')
		{
			achFullMsg[dwPos - 1] = '\\';
		}
        INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  	  cLogContent.m_achCompileField, strlen(cLogContent.m_achCompileField));
		INSERT_CHAR(achFullMsg, dwPos, dwFreeSize, '\n');
    }

	//屏幕打印输出 受开关控制
    OspTrcPrintf(cLogContent.m_bIsPrintScreen, FALSE, "%s", achFullMsg);

    //日志文件输出
    if (OSP_TRIVIAL_LEV == cLogContent.m_byLogLev)
    {
        //级别OSP_TRIVIAL_LEV(255)是ospext内部极其频繁的打印级别(比如事务轮询)，不写日志文件
        return;
    }

    u32 dwLogFileTypeSet = OSP_LOG_FILE_NONE;

    //是否写运行日志
    if ((g_bIsWriteRunLog == TRUE) && (cLogContent.m_byLogLev <= g_byRunLogFileLev))
    {
        dwLogFileTypeSet += OSP_LOG_FILE_RUN;
    }

    //是否写错误日志
    if (g_bIsWriteErrLog == TRUE)
    {
        if (OSP_ERROR_LEV == cLogContent.m_byLogLev || OSP_WARNING_LEV == cLogContent.m_byLogLev)
        {
            dwLogFileTypeSet += OSP_LOG_FILE_ERR;
        }
    }

    if (dwLogFileTypeSet != OSP_LOG_FILE_NONE)
    {
        TOspUniformLogHead tLogData ;
        tLogData.dwLogType = dwLogFileTypeSet;

        OspWriteUniformLogFile(tLogData, achFullMsg, strlen(achFullMsg));
    }
}

/*====================================================================
函数名：OspPrintf
功能：把相应的内容显示到屏幕/存储到文件
      显示到屏幕功能，不受开关控制 必须在屏幕显示
算法实现：（可选项）
引用全局变量：
输入参数说明：bScreen: 是否输出到屏幕,
              bFile: 是否输出到文件,
			  szFormat: 格式,
返回值说明：
====================================================================*/
API void OspPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...)
{
	if(szFormat == NULL)
	{
		return;
	}

	if (FALSE == g_Osp.m_bInitd)
    {
        return;
    }

	if(g_Osp.m_bKillOsp)
	{
		return;
	}

	if(!bScreen && !bFile)
	{
		return;
	}

	TOspLogHead tOspLogHead = {0};
	s8 achLog[MAX_LOG_MSG_LEN];

	u32 dwActLen = 0;

	tOspLogHead.type = LOG_TYPE_UNMASKABLE;
    tOspLogHead.bToScreen = bScreen;
    tOspLogHead.bToFile = bFile;

	va_list pvList;
    va_start(pvList, szFormat);
 	dwActLen = _vsnprintf(achLog, MAX_LOG_MSG_LEN, szFormat, pvList);
    va_end(pvList);
    if((dwActLen <= 0)||(dwActLen >= MAX_LOG_MSG_LEN))
	{
        printf("OspPrintf _vsnprintf fail, len=%d\n", dwActLen);
		return;
	}
	tOspLogHead.dwLength = dwActLen;

    OspLogQueWrite(tOspLogHead, achLog, dwActLen);
}

/*====================================================================
函数名：OspTrcPrintf
功能：把相应的内容显示到屏幕, 存储到文件, OSP内部在消息跟踪时使用
算法实现：（可选项）
引用全局变量：
输入参数说明：bScreen: 是否输出到屏幕,
              bFile: 是否输出到文件,
			  szFormat: 格式
返回值说明：
====================================================================*/
API void OspTrcPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...)
{
    va_list pvList;
	TOspLogHead tOspLogHead;
	char msg[MAX_LOG_MSG_LEN];
	u32 actLen = 0;

	if( !bScreen && !bFile )
	{
		return;
	}

	if(szFormat == NULL)
	{
		return;
	}

	tOspLogHead.type = LOG_TYPE_MASKABLE;
    tOspLogHead.bToScreen = bScreen;
    tOspLogHead.bToFile = bFile;

    va_start(pvList, szFormat);
  	actLen = _vsnprintf(msg, MAX_LOG_MSG_LEN, szFormat, pvList);
    va_end(pvList);
    if(actLen <= 0)
	{
        printf("Osp: _vsnprintf() failed in OspTrcPrintf().\n");
        return;
    }
    if (actLen >= MAX_LOG_MSG_LEN)
    {
        printf("Osp: msg's length is over MAX_LOG_MSG_LEN in OspTrcPrintf().\n");
        return;
    }
    OspLogQueWrite(tOspLogHead, msg, actLen);
}

/*====================================================================
函数名：OspMsgTrace
功能：将消息作为一个整体进行屏幕／日志输出
算法实现：（可选项）
引用全局变量：
输入参数说明：bScreen: 是否输出到屏幕,
              bFile: 是否输出到文件,
			  content: 内容,
			  dwLen: 长度
返回值说明：
====================================================================*/
API void OspMsgTrace(BOOL32 bScreen, BOOL32 bFile, const char * szContent, u32 dwLen)
{
	TOspLogHead tOspLogHead;

	if( !bScreen && !bFile )
	{
		return;
	}

	tOspLogHead.type = LOG_TYPE_MASKABLE;
    tOspLogHead.bToScreen = bScreen;
    tOspLogHead.bToFile = bFile;

	OspLogQueWrite(tOspLogHead, szContent, dwLen);
}

/*====================================================================
函数名：OspDumpPrintf
功能：把相应的内容显示到屏幕,存储到文件
算法实现：（可选项）
引用全局变量：
输入参数说明：bScreen: 是否输出到屏幕,
              bFile: 是否输出到文件,
			  szFormat: 格式
返回值说明：
====================================================================*/
API void OspDumpPrintf(BOOL32 bScreen, BOOL32 bFile, const char *szFormat, ...)
{
	va_list pvList;
	TOspLogHead tOspLogHead;
	char msg[MAX_LOG_MSG_LEN];
	u32 actLen = 0;

	if(szFormat == NULL)
	{
		return;
	}

	if(!bScreen && !bFile)
	{
		return;
	}

	tOspLogHead.type = LOG_TYPE_MASKABLE;
    tOspLogHead.bToScreen = bScreen;
    tOspLogHead.bToFile = bFile;

    va_start(pvList, szFormat);
  	actLen = _vsnprintf(msg, MAX_LOG_MSG_LEN, szFormat, pvList);
    va_end(pvList);
	if(actLen <= 0)
	{
        printf("Osp: _vsnprintf() failed in OspDumpPrintf().\n");
        return;
    }
    if (actLen >= MAX_LOG_MSG_LEN)
    {
        printf("Osp: msg's length is over MAX_LOG_MSG_LEN in OspDumpPrintf().\n");
        return;
    }
    OspLogQueWrite(tOspLogHead, msg, actLen);
}

/*====================================================================
函数名：OspAddEventDesc
功能：把事件的描述登记到OSP
算法实现：（可选项）
引用全局变量：
输入参数说明：szDesc: 时间描述字符串
              uEvent: 事件号
返回值说明：
====================================================================*/
API void OspAddEventDesc(const char * desc, u16 event)
{
    if (!IsOspInitd())
	{
		return;
	}
    g_Osp.m_cOspEventDesc.DescAdd(desc,event);
}

/*====================================================================
函数名：OspAppDescAdd
功能：把其他节点的App的描述加入到OSP中
算法实现：（可选项）
引用全局变量：
输入参数说明：wAid: app编号
              szName: app名称, 必须是以'\0'结束的字符串
返回值说明：
====================================================================*/
API void OspAppDescAdd(u16 wAppID, const char * szName)
{
    if (!IsOspInitd())
    {
        return;
	}
	if(wAppID <= 0 || wAppID > MAX_APP_NUM)
	{
		return;
	}

	if(szName == NULL)
	{
		return;
	}

    g_Osp.m_cOspAppDesc.DescAdd(szName, wAppID);
}

/*=============================================================================
 函 数 名  : OspOpenLogFile
 功能描述  : 启动日志文件 并设置日志文件的参数
             调用该接口后，才会启动日志文件记录
 算法实现  :
 参数说明  :
            [I]const char * szDir 	   日志文件保存的目录
            [I]u32 dwMaxSizeKB     每个文件的最大字节数
            [I]dwLogNumMax         日志文件的最大个数
 返回值说明: TRUE 成功 FALSE 失败
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    修改
    原有的支持设置日志文件目录和文件名，改为仅仅支持设置文件目录
    日志文件名由osp自动生成，取文件生成的时间作为文件名
=============================================================================*/
API BOOL32 OspOpenLogFile(const char * szDir, u32 dwMaxSizeKB, u32 dwMaxFiles)
{
    BOOL32 bRet = FALSE;

    if((0==dwMaxSizeKB)||(0==dwMaxFiles))
    {
    	printf("dwMaxSizeKB or dwMaxFiles equal zero!\n");
    	return FALSE;
    }

    if (NULL == szDir)
    {
        szDir = "./log/";
    }

    //return g_cOspLog.LogFileOpen(szFileName, dwMaxSizeKB, dwMaxFiles, NULL);

	//目前设计为运行日志和错误日志都保存在同一目录
    bRet = g_cOspLog.OspSetLogFileParam("log_", szDir, dwMaxSizeKB, dwMaxFiles, OSP_LOG_FILE_ERR);
    if(!bRet)
    {
        return FALSE;
    }
    g_cOspLog.OspSetLogFileParam("log_", szDir, dwMaxSizeKB, dwMaxFiles, OSP_LOG_FILE_RUN);
    if(!bRet)
    {
        return FALSE;
    }

	g_cOspLog.LogFileOpen();

    return TRUE;
}

/*=============================================================================
 函 数 名  : OspOpenLogFileEx
 功能描述  : 启动日志文件 并设置日志文件的参数
 算法实现  :
 参数说明  :
 			[I]const char* szFileName   日志文件名字前缀
            [I]const char* szDir 	   日志文件保存的目录
            [I]u32 dwLogSizeMax_kb 每个文件的最大字节数
            [I]dwLogNumMax         日志文件的最大个数
 返回值说明: TRUE 成功 FALSE 失败
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    修改
=============================================================================*/
API BOOL32 OspOpenLogFileEx(const char * szFileName, const char * szDir,
							u32 dwMaxSizeKB, u32 dwMaxFiles)
{
    BOOL32 bRet = FALSE;

    if (NULL == szDir || 0 == szDir[0])
    {
    	printf("szDir is null, set log dir as ./log/\n");

        szDir = "./log/";
    }
    if (NULL == szFileName || 0 == szFileName[0])
    {
    	printf("szFileName is null, set name as log_xx\n");
        szFileName = "log_";
    }

	//目前设计为运行日志和错误日志都保存在同一目录
    bRet = g_cOspLog.OspSetLogFileParam(szFileName, szDir, dwMaxSizeKB, dwMaxFiles, OSP_LOG_FILE_ERR);
    if(!bRet)
    {
        return FALSE;
    }
    bRet = g_cOspLog.OspSetLogFileParam(szFileName, szDir, dwMaxSizeKB, dwMaxFiles, OSP_LOG_FILE_RUN);
    if(!bRet)
    {
        return FALSE;
    }
	g_cOspLog.LogFileOpen();

    return TRUE;
}

/*====================================================================
函数名：OspCloseLogFile
功能：关闭OSP的日志文件. 调用OspQuit()后，不需要再调用该函数.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明：
====================================================================*/
API void OspCloseLogFile()
{
	g_cOspLog.LogFileClose();
}

/*====================================================================
函数名：OspSetFileLogLevel
功能：设置文件日志等级。
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：wAid: LOCAL_APP -- 设置全局日志等级, 否则设置对应app的日志等级,
              byFileLevel: 文件日志等级.
返回值说明：
====================================================================*/
API void OspSetFileLogLevel(u16 wAid, u8 byFileLevel)
{
	if(wAid == LOCAL_APP)
	{
		g_cOspLog.m_byLogFileLevel = byFileLevel;
		return;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(wAid);
	if(pApp != NULL)
	{
		pApp->LogLevelSet(byFileLevel, pApp->scrnLogFlag);
	}
}

/*====================================================================
函数名：OspSetScrnLogLevel
功能：设置屏幕日志等级
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAid: LOCAL_APP -- 设置全局日志等级, 否则设置对应app的日志等级,
			  uScreenLevel: 屏幕日志等级.
返回值说明：
====================================================================*/
API void OspSetScrnLogLevel(u16 wAid, u8 byScrnLevel)
{
	if(wAid == LOCAL_APP)
	{
		g_cOspLog.m_byLogScreenLevel = byScrnLevel;
		return;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(wAid);
	if(pApp != NULL)
	{
		pApp->LogLevelSet(pApp->fileLogFlag, byScrnLevel);
	}
}

/*====================================================================
函数名：OspSetLogLevel
功能：设置App的日志等级
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAid: LOCAL_APP -- 设置全局日志等级, 否则设置对应app的日志等级,
              uFileLevel: 文件日志等级,
			  uScreenLevel: 屏幕日志等级.
返回值说明：
====================================================================*/
API void OspSetLogLevel(u16 wAppID, u8 byFileLevel, u8 byScreenLevel)
{
	OspSetFileLogLevel(wAppID, byFileLevel);
	OspSetScrnLogLevel(wAppID, byScreenLevel);
}

/*====================================================================
函数名：OspSetFileTrcFlag
功能：设置文件跟踪标志
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAid: LOCAL_APP -- 设置全局跟踪标志, 否则设置对应app的跟踪标志,
              uFileFlag: 文件跟踪标志,
返回值说明：
====================================================================*/
API void OspSetFileTrcFlag(u16 wAid, u16 wFileFlag)
{
	if(wAid == LOCAL_APP)
	{
		g_Osp.m_cAppPool.m_wGloFileTrc = wFileFlag;
		return;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(wAid);
	if (pApp != NULL)
	{
		pApp->TrcFlagSet(wFileFlag, pApp->scrnTraceFlag);
	}
}

/*====================================================================
函数名：OspSetScrnTrcFlag
功能：设置屏幕跟踪标志
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAid: LOCAL_APP -- 设置全局跟踪标志, 否则设置对应app的跟踪标志,
              uScreenFlag: 屏幕跟踪标志,
返回值说明：
====================================================================*/
API void OspSetScrnTrcFlag(u16 wAid, u16 wScreenFlag)
{
	if(wAid == LOCAL_APP)
	{
		g_Osp.m_cAppPool.m_wGloScrTrc = wScreenFlag;
		return;
	}

	CApp *pApp = g_Osp.m_cAppPool.AppGet(wAid);
	if (pApp != NULL)
	{
		pApp->TrcFlagSet(pApp->fileTraceFlag, wScreenFlag);
	}
}

/*====================================================================
函数名：OspSetTrcFlag
功能：设置文件和屏幕跟踪标志
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：uAid: LOCAL_APP -- 设置全局跟踪标志, 否则设置对应app的跟踪标志,
              uFileFlag: 文件跟踪标志，
              uScreenFlag: 屏幕跟踪标志.
====================================================================*/
API void OspSetTrcFlag(u16 wAid, u16 wFileFlag, u16 wScreenFlag)
{
	OspSetFileTrcFlag(wAid, wFileFlag);
    OspSetScrnTrcFlag(wAid, wScreenFlag);
}

/*====================================================================
函数名：OspTrcAllOn
功能：打开OSP所有的全部屏幕和文件跟踪标志
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
API void OspTrcAllOn(void)
{
	OspSetTrcFlag( LOCAL_APP, TRCALL, TRCALL );

	/* 变量uAppId必须用u16，否则可能导致uAppId溢出，从而死循环 */
    for(u16 wAppId=1; wAppId<=MAX_APP_NUM; wAppId++)
    {
        OspSetTrcFlag( wAppId, TRCALL, TRCALL );
    }
}

/*====================================================================
函数名：OspTrcAllOff
功能：关闭OSP所有的跟踪标志
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
API void OspTrcAllOff(void)
{
	OspSetTrcFlag( LOCAL_APP, 0, 0 );
    for(u16 wAppId=1; wAppId<=MAX_APP_NUM; wAppId++)
    {
        OspSetTrcFlag( wAppId, 0, 0 );
    }
}

/*====================================================================
函数名：OspSendTrcOn
功能：打开OSP外发消息跟踪标志
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：

  返回值说明：
====================================================================*/
API void OspSendTrcOn()
{
    g_Osp.m_cNodePool.m_dwSendTrcFlag = 1;
}

/*====================================================================
函数名：OspRcvTrcOn
功能：打开OSP接收外部消息功能
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
API void OspRcvTrcOn()
{
    g_Osp.m_cNodePool.m_dwRcvTrcFlag = 1;
}

/*====================================================================
函数名：OspSendTrcOff
功能：关闭OSP外发消息跟踪标设
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
API void OspSendTrcOff()
{
    g_Osp.m_cNodePool.m_dwSendTrcFlag = 0;
}

/*====================================================================
函数名：OspRcvTrcOff
功能：关闭OSP外部消息接收功能
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：
====================================================================*/
API void OspRcvTrcOff()
{
    g_Osp.m_cNodePool.m_dwRcvTrcFlag = 0;
}

/*====================================================================
函数名：OspSetLogEventDataLength
功能：设置全局的消息跟踪显示的长度, 未实现
算法实现：（可选项）
引用全局变量：
输入参数说明：uLength: 新长度

 返回值说明：
====================================================================*/
API u16 OspSetLogEventDataLength(u16)
{
    return 0;
}

//超过十行的消息内容是否输出 TRUE 输出  FALSE 不输出
API BOOL32 IsOspLogLongMsgPrintEnbl(void)
{
	return g_cOspLog.m_bLMsgDumpEnbl;
}

/*====================================================================
函数名：OspMsgDumpSet
功能：设置超过十行的消息内容是否输出，缺省不输出
算法实现：（可选项）
引用全局变量：
输入参数说明：bLongMsgDumpEnbl: 允许/禁止长消息输出

  返回值说明：
====================================================================*/
API void OspMsgDumpSet(BOOL32 bLongMsgDumpEnbl)
{
	g_cOspLog.m_bLMsgDumpEnbl = bLongMsgDumpEnbl;
}

/*====================================================================
函数名：LogTask, 日志任务入口函数
功能：从日志邮箱中读出消息,并输出到屏幕和文件中
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明：
====================================================================*/
API void LogTask()
{
    g_cOspLog.LogQueOut();
}

/*====================================================================
函数名：COspEventDesc::DescAdd
功能： 加入事件描述到OSP
算法实现：（可选项）
引用全局变量:
输入参数说明: szDesc: 事件的描述，必须是'\0'结束的字符串；uEvent: 事件
返回值说明：
====================================================================*/
void COspEventDesc::DescAdd(const char * szDesc, u16 wEvent)
{
	if( (szDesc == NULL) || (wEvent >= MAX_EVENT_COUNT) )
	{
		return;
	}

    if(EventDesc[wEvent] != NULL)
    {
        OspFreeMem(EventDesc[wEvent]);
        EventDesc[wEvent] = NULL;
    }
    EventDesc[wEvent] = (char *)OspAllocMem( strlen(szDesc)+1 );
	if(NULL != EventDesc[wEvent])
	{
		strcpy(EventDesc[wEvent], szDesc);
	}
}

/*====================================================================
函数名：COspEventDesc::DescGet
功能：获取事件对应的描述
算法实现：（可选项）
引用全局变量：uEvent: 事件
输入参数说明：

  返回值说明：返回事件描述
====================================================================*/
char * COspEventDesc::DescGet(u16 wEvent)
{
	if(wEvent >= MAX_EVENT_COUNT)
	{
		return "null";
	}
    return EventDesc[wEvent];
}

/*====================================================================
函数名：COspEventDesc::COspEventDesc
功能：事件描述类的构造函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：返回事件描述
====================================================================*/
COspEventDesc::COspEventDesc()
{
    memset( EventDesc, 0, sizeof(EventDesc) );
}

/*====================================================================
函数名：COspEventDesc::~COspEventDesc
功能：事件描述类的析构函数，释放事件描述所占用的内存
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：返回事件描述
====================================================================*/
COspEventDesc::~COspEventDesc()
{
	Destroy();
}

/*====================================================================
函数名：COspEventDesc::COspEventInit
功能：  初始化一些OSP事件的描述
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：返回事件描述
====================================================================*/
#define INIT_EVENT_DESC( x ) DescAdd( #x, x )
void COspEventDesc::COspEventInit()
{
    INIT_EVENT_DESC( OSP_CONFIRM        );
    INIT_EVENT_DESC( OSP_POWERON        );
    INIT_EVENT_DESC( OSP_SWITCHMASTER   );
    INIT_EVENT_DESC( OSP_SWITCHSLAVE    );
    INIT_EVENT_DESC( OSP_OVERFLOW       );
    INIT_EVENT_DESC( OSP_EXCEPTION_IND  );
    INIT_EVENT_DESC( OSP_INSNOTEXIST    );
    INIT_EVENT_DESC( OSP_DISCONNECT     );
    INIT_EVENT_DESC( OSP_BROADCASTACK   );
    INIT_EVENT_DESC( OSP_NODECLOSE      );
    INIT_EVENT_DESC( OSP_NETBRAECHO     );
    INIT_EVENT_DESC( OSP_NETBRAECHOACK  );
    INIT_EVENT_DESC( OSP_QUIT           );
    INIT_EVENT_DESC( OSP_NETSTATEST     );
    INIT_EVENT_DESC( OSP_NETSTATESTACK  );
    INIT_EVENT_DESC( OSP_APPCONN_ACK    );
    INIT_EVENT_DESC(OSP_COMPRESS_SUPPORT); //by wubin 2011-02-22
    INIT_EVENT_DESC(OSP_COMPRESS_MSG);//by wubin 2011-02-22
}
#undef  INIT_EVENT_DESC

/*====================================================================
函数名：COspEventDesc::Destroy
功能：事件描述类的析构函数，释放事件描述所占用的内存
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：返回事件描述
====================================================================*/
void COspEventDesc::Destroy(void)
{
    for(int i=0; i<MAX_EVENT_COUNT; i++)
	{
        if(EventDesc[i] != NULL)
		{
            OspFreeMem(EventDesc[i]);
			EventDesc[i] = NULL;
        }
    }
}

/*====================================================================
函数名：COspAppDesc::DescAdd
功能：加入对wAppId这个App的描述
算法实现：（可选项）
引用全局变量：
输入参数说明：szDesc: 描述，必须是'\0'结束的字符串,
              wAppId: 目标App的ID.
  返回值说明：
====================================================================*/
void COspAppDesc::DescAdd(const char * szDesc, u16 wAppId)
{
	if( (szDesc == NULL) || (wAppId <= 0) || (wAppId > MAX_APP_NUM) )
	{
		return;
	}

    if(AppDesc[wAppId-1] != NULL)
    {
        OspFreeMem(AppDesc[wAppId-1]);
		AppDesc[wAppId-1] = NULL;
    }

    AppDesc[wAppId-1] = (char *)OspAllocMem( strlen(szDesc)+1 );
	if (NULL != AppDesc[wAppId-1])
	{
		strcpy(AppDesc[wAppId-1], szDesc);
	}
}

/*====================================================================
函数名：COspAppDesc::COspAppDesc
功能：COspAppDesc类的构造函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
COspAppDesc::COspAppDesc()
{
    memset(this, 0, sizeof(COspAppDesc));
}

/*====================================================================
函数名：COspAppDesc::~COspAppDesc
功能：COspAppDesc类的析构函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
COspAppDesc::~COspAppDesc()
{
	Destroy();
}

/*====================================================================
函数名：COspAppDesc::Destroy
功能：COspAppDesc类的析构函数
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：
====================================================================*/
void COspAppDesc::Destroy()
{
    for(int i=0; i<MAX_APP_NUM; i++)
	{
        if(AppDesc[i] != NULL)
		{
            OspFreeMem(AppDesc[i]);
			AppDesc[i] = NULL;
        }
    }
}

/*====================================================================
函数名：OspCat
功能：显示指定文件
算法实现：（可选项）
引用全局变量：
输入参数说明：fname: 文件名

  返回值说明:
====================================================================*/
#define MAX_CMDLINE_LEN           80
API void OspCat(const char * fname)
{
	FILE * fhandle;
	char filenm[MAX_FILENAME_LEN];
	char line[MAX_CMDLINE_LEN];

	if(strlen(fname) >= MAX_FILENAME_LEN)
	{
		return;
	}

    strcpy(filenm, fname);
    fhandle = fopen(filenm, "r");
    if(fhandle == NULL)
	{
		OspPrintf(TRUE, FALSE, "Osp: open file %s failed.\n", fname);
		return ;
    }

	while( fgets(line, MAX_CMDLINE_LEN, fhandle) != NULL )
	{
		OspPrintf(TRUE, FALSE, "%s", line);
	}

    fclose(fhandle);
    return;
}

/*====================================================================
函数名：OspEventDescShow
功能：显示消息的描述
算法实现：（可选项）
引用全局变量：
输入参数说明：u16 wEventID : 消息的ID
返回值说明:
====================================================================*/
API void OspEventDescShow( u16 wEventID )
{
	if(wEventID > MAX_EVENT_COUNT)
	{
		OspPrintf(TRUE, FALSE, "Event %4d not exist\n", wEventID);
		return;
	}
    OspPrintf( TRUE, FALSE, "Event %4d: %s\n", wEventID, OspEventDesc(wEventID) );
}

/*====================================================================
函数名：OspEventDescShow
功能：返回消息的描述
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：u16 wEventID : 消息的ID
返回值说明: 消息的描述. 如果消息ID无效或者该消息没有描述字符串，
            返回值将为NULL.
====================================================================*/
API const char * OspEventDesc(u16 wEventID)
{
    return g_Osp.m_cOspEventDesc.DescGet(wEventID);
}

/*====================================================================
函数名：OspFileLogNum
功能：取得已成功输出的文件日志数.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明: 文件日志数.
====================================================================*/
API u32 OspFileLogNum(void)
{
	return g_cOspLog.m_dwRunFileLogNum + g_cOspLog.m_dwErrorFileLogNum;
}

/*====================================================================
函数名：OspScrnLogNum
功能：取得已成功输出的屏幕日志数.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明: 屏幕日志数.
====================================================================*/
API u32 OspScrnLogNum(void)
{
	return g_cOspLog.m_dwScreenLogNum;
}

/*====================================================================
函数名：OspLogFileNo
功能：取得当前日志文件号.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明: 日志文件号.
====================================================================*/
API u32 OspLogFileNo(void)
{
	return 0;
	//采用文件生成时刻为文件名 文件号无意义
	//return g_cOspLog.m_dwCurrentFileNo;
}

/*====================================================================
函数名：OspLogShow
功能：显示日志模块状态
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：
返回值说明: 无
====================================================================*/
API void OspLogShow(void)
{
	g_cOspLog.Show();
}


//设置App回调函数执行信息记录功能
API void OspSetCallBackRecFlag(u16 wAppId, BOOL32 bFlag)
{
	CApp *pApp = g_Osp.m_cAppPool.AppGet(wAppId);
	if (pApp != NULL)
	{
		pApp->SetCallBackInfoRecordFlag(bFlag);
	}
}

class COspLogLevString
{
public:
	#define ADD_LOG_LEV_DESC(x) AddLogLevDesc(x, #x)
    COspLogLevString()
    {
        ADD_LOG_LEV_DESC(OSP_NOLOG_LEV);
        ADD_LOG_LEV_DESC(OSP_ERROR_LEV);
        ADD_LOG_LEV_DESC(OSP_CRITICAL_LEV);
        ADD_LOG_LEV_DESC(OSP_WARNING_LEV);
        ADD_LOG_LEV_DESC(OSP_EVENT_LEV);
        ADD_LOG_LEV_DESC(OSP_PROGRESS_LEV);
        ADD_LOG_LEV_DESC(OSP_FREQ_EVENT_LEV);
        ADD_LOG_LEV_DESC(OSP_TIMER_LEV);
        ADD_LOG_LEV_DESC(OSP_ALL_LEV);
        ADD_LOG_LEV_DESC(OSP_TRIVIAL_LEV);
    }
public:
    void AddLogLevDesc(u8 chLevel, const char * szval)
    {
    	TLogLevelDesc tDescriptor;
		strncpy(tDescriptor.achLogLevelName, szval, MAX_LOG_LEVEL_NAME_LENGTH);
        m_cLogLevDesc[chLevel] = tDescriptor;
    }

	const char * GetLogLevDesc(u8 byLogLev)
	{
		if(m_cLogLevDesc.Exist(byLogLev))
		{
			return m_cLogLevDesc[byLogLev].achLogLevelName;
		}
		else
		{
			return "null";
		}
	}

private:
    COspLogLevDesc m_cLogLevDesc;
};

static COspLogLevString g_cLogLevString;


//根据日志级别的获取字符描述
API const char * OspGetStrLogLevel(u8 byLogLev)
{
    return g_cLogLevString.GetLogLevDesc(byLogLev);
}
//通过日志模块序号或者该模块的日志级别 如果没有设置过 获取到的值将是OSP_TRIVIAL_LEV
API u32 OspGetModuleLogLevel(u8 byMod)
{
	const TLogModLevelDesc *ptValue = NULL;

	if(g_cModuleLogLev.Exist(byMod))
	{
		ptValue = &(g_cModuleLogLev[byMod]);
		return ptValue->dwLogLevel;
	}
	else
	{
		return OSP_TRIVIAL_LEV;
	}
}
//通过日志模块序号获取该模块的名称
API const char * OspGetLogStrModule(u8 byMod)
{
	const TLogModLevelDesc *ptValue = NULL;

	if(g_cModuleLogLev.Exist(byMod))
	{
		ptValue = &(g_cModuleLogLev[byMod]);
		return ptValue->achModuleName;
	}
	else
	{
		return "null";
	}
}

/*=============================================================================
 函 数 名  : OspWriteUniformLogFile
 功能描述  : 将日志写入到日志队列
 			 对应此次增加OSP_CLASSLOG GLBLOG接口的日志输出
             此接口的日志最终会保存到文件
 算法实现  :
 参数说明  :
            [I]ptOspLogHead
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年6月29日    1.0    dengchange    新生成函数
=============================================================================*/
API void OspWriteUniformLogFile(TOspUniformLogHead tLogData, const char * szContent, u32 dwLen)
{
	//分配内存长度为TOspLogCommonHead + TOspLogHead头 + MAX_LOG_MSG_LEN
	//为避免再次拷贝，该内存由LogQueOut()中使用完毕后进行释放
	TOspLogCommonHead *ptOspLogCommonHead =
			(TOspLogCommonHead*)OspAllocMem(sizeof(TOspLogCommonHead) - 1 +
			sizeof(TOspUniformLogHead) + dwLen + 1);
	if(NULL == ptOspLogCommonHead)
	{
		printf("OspWriteUniformLogFile mem fail\n");
		return;
	}
	ptOspLogCommonHead->chOspInterfaceType = OSP_LOG_INTERFACE_UNIFORM;

	TOspUniformLogHead *ptOspLogHead = (TOspUniformLogHead*)(ptOspLogCommonHead->achData);
	s8 *pchLogStart = ((s8*)ptOspLogHead)+ sizeof(TOspUniformLogHead);

	memcpy(ptOspLogHead, &tLogData, sizeof(TOspUniformLogHead));
	memcpy(pchLogStart, szContent, dwLen);

	pchLogStart[dwLen] = '\0';//以防万一字符串漏掉终止符\0

	g_cOspLog.LogQueWriteFinal(ptOspLogCommonHead);
}

#ifdef _LINUX_

#else
static void replace_str(char* pchBody, char chSrc, char chDst)
{
    while(*pchBody != '\0')
    {
    	if(*pchBody == chDst)
    	{
    		*pchBody = chSrc;
    	}

		pchBody++;
    }
}
#endif

static BOOL32 DelFile(const char * szFileName)
{
    if (szFileName == NULL)
    {
        return false;
    }

#ifdef _LINUX_
    if (0 == remove(szFileName))
    {
        return true;
    }
#else
    if (TRUE == DeleteFile(szFileName))
    {
        return true;
    }
#endif

    printf("remove file[%s] fail\n", szFileName);
    return false;
}
//获取进程名称
u32 GetProcessName(u32 dwBufferLen, char* pchBuffer)
{
	if(NULL == pchBuffer)
	{
		return 0;
	}

	pchBuffer[0] = '\0';

    s8 szFullPath[1024] = {0};
    char* pchStart = NULL;

#ifdef _LINUX_
    readlink("/proc/self/exe", szFullPath, sizeof(szFullPath));
    pchStart = strrchr(szFullPath, '/') + 1;
#else
    GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
    pchStart = strrchr(szFullPath, '\\') + 1;
#endif
	if(NULL != pchStart)
	{
		strncpy(pchBuffer, pchStart, dwBufferLen);
		pchBuffer[dwBufferLen - 1] = '\0';
	}

	return strlen(pchBuffer);
}

/*====================================================================
函数名：TeleCmdEcho
功能：向Telnet客户端屏幕回显用户命令.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchCmdStr: 命令串,
              uLen: 命令长度.

返回值说明：无.
====================================================================*/
API void OspLogTeleCmdEcho(const char * pchCmdStr, u32 dwLen)
{
	TOspLogHead tOspLogHead;

	tOspLogHead.type = LOG_TYPE_UNMASKABLE;
	tOspLogHead.bToScreen = TRUE;
	tOspLogHead.bToFile = FALSE;
	tOspLogHead.dwLength = dwLen;
	OspLogQueWrite(tOspLogHead , pchCmdStr, dwLen);
}

//输出日志
API void OspLogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen)
{
	g_cOspLog.LogQueWrite(tOspLogHead, szContent, dwLen);
}

//分割符而已 便于代码阅读
#if 0
#endif
/*====================================================================
	以下命令常作为定位问题和调试之用，故集中于此
====================================================================*/

//暂停日志屏幕输出
API void OspStopScrnLog(void)
{
	g_cOspLog.StopScrnLog();
}

//恢复日志屏幕输出
API void OspResumeScrnLog(void)
{
	g_cOspLog.ResumeScrnLog();
}

//设置模块的日志级别和模块名称
API u32 OspSetModuleLogLev(u8 byMod, u8 byLevel, const char * szModName)
{
	if(NULL == szModName)
	{
		OSP_LOG(MOD_OSP, OSP_ERROR_LEV, "szModName\n");
		return FALSE;
	}

	if (g_cModuleLogLev.Exist(byMod))
	{
		OSP_LOG(MOD_OSP, OSP_ERROR_LEV, "Log Module[%d] already set\n", byMod);
		return FALSE;
	}

	TLogModLevelDesc tDescrptor;
	tDescrptor.dwLogLevel = byLevel;
	strncpy(tDescrptor.achModuleName, szModName, MAX_LOG_MODULE_NAME_LENGTH);

	g_cModuleLogLev[byMod] = tDescrptor;

	return TRUE;
}

//日志的全局打印级别 level值大于该值的日志不在屏幕显示
API void OspSetGlbLogPrintLevel(u8 byLogLev)
{
	g_byGlbLogPrintLev = byLogLev;
}

//设置日志的时间信息打印 0:不打印时间; 1:打印秒级时间; 255:打印豪秒级时间
API void OspSetLogTimeLevel(u8 byLogLev)
{
    g_byPrintTimeLev = byLogLev;
}

//设置是否打印编译信息(file-line-class-function) 0:不打印编译信息; 1:打印编译信息
API void OspSetLogCompileInfo(u8 byPrint)
{
    g_byPrintCompileInfo = byPrint;
}

#ifdef _LINUX_
//linux是否进行mangle name的解析
API void OspParseMangleNameSwitch(u8 byIsParse)
{
    //ADD_DBG_CMD();
    if (byIsParse == 0)
    {
        g_bIsParseMangleName = FALSE;
    }
    else
    {
        g_bIsParseMangleName = TRUE;
    }
}
#endif


//设置日志中是否打印前缀信息(app-inst-task-state) 0:不打印OSP前缀; 1:打印OSP前缀
API void OspSetLogPrefix(u8 byPrint)
{
    g_byPrintOspPrefix = byPrint;
}

//设置是否输出运行日志文件以及运行日志的级别
//0:不输出运行日志; 非0: 日志级别值小于等于该值的，输出为运行日志
//目前写入文件的内容和屏幕的打印内容一致
API void OspSetWriteRunLog(u8 byIsWriteLog)
{
    if (byIsWriteLog == 0)
    {
        g_bIsWriteRunLog = FALSE;
        g_byRunLogFileLev = 0;
    }
    else
    {
        g_bIsWriteRunLog = TRUE;
        g_byRunLogFileLev = byIsWriteLog;
    }
}

//设置是否输出错误日志文件：//0:不输出; 非0:输出
//以下两种级别的日志认为是错误日志，OSP_ERROR_LEV //1错误级别  OSP_WARNING_LEV //2警告级别
API void OspSetWriteErrLog(u8 byIsWriteLog)
{
    if (byIsWriteLog == 0)
    {
        g_bIsWriteErrLog = FALSE;
    }
    else
    {
        g_bIsWriteErrLog = TRUE;
    }
}

//显示日志控制参数
API void OspShowLogConfig()
{
    OspPrintf(TRUE, FALSE,"全局日志级别: %s\n", OspGetStrLogLevel(g_byGlbLogPrintLev));
    OspPrintf(TRUE, FALSE,"日志时间级别: %u  (0:不输出时间, 非0:秒级时间, 255: 毫秒级时间)\n", g_byPrintTimeLev);
    OspPrintf(TRUE, FALSE,"输出编译信息: %u  (0:不输出, 非0:输出)\n", g_byPrintCompileInfo);
    OspPrintf(TRUE, FALSE,"输出OSP前缀: %u  (0:不输出, 非0:输出)\n", g_byPrintOspPrefix);
    OspPrintf(TRUE, FALSE,"当前跟踪实例: [%u-%u] ([0-0]:不跟踪, 非[0-0]:跟踪的app-inst号)\n",
    GETAPP(g_dwTraceAppInst), GETINS(g_dwTraceAppInst));
    OspPrintf(TRUE, FALSE,"当前跟踪事务: %u  (0:不跟踪, 非0:跟踪事务号)\n", g_dwTraceTaskNO);
    OspPrintf(TRUE, FALSE,"是否写运行日志文件: %u  (0:不写, 非0:写)\n", g_bIsWriteRunLog);
    OspPrintf(TRUE, FALSE,"是否写错误日志文件: %u  (0:不写, 非0:写)\n", g_bIsWriteErrLog);
#ifdef _LINUX_
    OspPrintf(TRUE, FALSE,"是否解析Mangle Name: %u  (0:不解析, 非0:解析)\n", g_bIsParseMangleName);
#endif
    OspPrintf(TRUE, FALSE, "是否开启时间检测: %u  (0:不检测, 非0:检测)\n", g_bIsMonitorTimePerform);

    OspPrintf(TRUE, FALSE, "\n日志级别列表: \n");
    for(u8 byLogLev = OSP_NOLOG_LEV; byLogLev < OSP_LOG_LEV_NUM; byLogLev++)
    {
        OspPrintf(TRUE, FALSE,"  %s------%u\n", OspGetStrLogLevel(byLogLev), byLogLev);
    }
}


