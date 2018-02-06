/******************************************************************************
ģ����  �� OSP
�ļ���  �� OspLog.cpp
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��־���ܵ���Ҫʵ���ļ�
����    �����Ľ�
�汾    ��1.0.02.7.5
--------------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
09/15/98        1.0      ĳĳ        ------------
2015/06/30             �˲���      ����֧��OSP_CLASSLOG��GLBLOG�ӿڼ�¼��־
******************************************************************************/
#include "osplog_private.h"

static COspLog g_cOspLog; // osplog Task

#ifdef _DEBUG

static u8 g_byGlbLogPrintLev = OSP_FREQ_EVENT_LEV;     //ȫ�ִ�ӡ���� levelֵ���ڸ�ֵ����־������Ļ��ʾ

static u8 g_byPrintTimeLev = 1;       //0:����ӡʱ��; 1:��ӡ�뼶ʱ��; 255:��ӡ���뼶ʱ��

static u8 g_byPrintCompileInfo = 1;   //0:����ӡ������Ϣ(file-line-class-function); 1:��ӡ������Ϣ

static u8 g_byPrintOspPrefix = 1;     //0:����ӡOSPǰ׺(app-inst-task-state); 1:��ӡOSPǰ׺

static BOOL32 g_bIsMonitorTimePerform = FALSE;  //FALSE:���������ܼ�⣬TRUE������

#else

static u8 g_byGlbLogPrintLev = OSP_ERROR_LEV;     //ȫ�ִ�ӡ����

static u8 g_byPrintTimeLev = 1;       //0:����ӡʱ��; 1:��ӡ�뼶ʱ��; 255:��ӡ���뼶ʱ��

static u8 g_byPrintCompileInfo = 0;   //0:����ӡ������Ϣ(file-line-class-function); 1:��ӡ������Ϣ

static u8 g_byPrintOspPrefix = 0;     //0:����ӡOSPǰ׺(app-inst-task-state); 1:��ӡOSPǰ׺

static BOOL32 g_bIsMonitorTimePerform = FALSE;  //FALSE:���������ܼ�⣬TRUE������

#endif

static u32 g_dwTraceAppInst = 0;      //0:�ر�trace; ��0:trace ָ��APP��ָ��INST

static u32 g_dwTraceTaskNO = 0;        //0:�ر�trace; ��0:trace ָ������

static BOOL32 g_bIsWriteRunLog = TRUE; //�Ƿ����������־�ļ���Ŀǰд���ļ������ݺ���Ļ�Ĵ�ӡ����һ��

static BOOL32 g_bIsWriteErrLog = TRUE; //�Ƿ����������־�ļ���Ŀǰд���ļ������ݺ���Ļ�Ĵ�ӡ����һ��

#ifdef _LINUX_
static BOOL32 g_bIsParseMangleName = FALSE; //linux�Ƿ����mangle name�Ľ�������Ϊ�ǳ���ʱ��Ĭ�ϲ����н���
#endif

static u8 g_byRunLogFileLev = OSP_PROGRESS_LEV;  //������־�ļ�����

static CModuleLogInfo g_cModuleLogLev;

API BOOL32 IsOspPrintCI()
{
    return (g_byPrintCompileInfo != 0);
}
/*====================================================================
��������COspLog::COspLog()
���ܣ�COspLog��Ĺ��캯������һЩ��Ա�������г�ʼ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������COspLog::Initialize
���ܣ���ʼ����־ϵͳ����Ҫ�Ǵ�����־����������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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

	/* �ļ���С���ļ��������û�ʹ��OspOpenLogFileʱ���г�ʼ�� */

	/* ������־���� */
	bOK = OspCreateMailbox("log",                   // ������
		                    m_dwMaxMsgWaiting,     // ��Ϣ����
		                    sizeof(TOsMsgStruc),    // �㿽���Ժ�, ������ֻ�����Ϣָ��
							&m_dwReadQueHandle,     // ����handle
							&m_dwWriteQueHandle     // ����handle
						  );
	if( !bOK )
	{
		m_dwReadQueHandle = 0;
		m_dwWriteQueHandle = 0;
		return FALSE;
	}

	/* ������־���� */
    m_hTask = OspTaskCreate( LogTask,              // �������
		                     "OspLogTask",         // ������
							 OSP_LOG_TASKPRI,      // ���ȼ�
							 OSP_LOG_STACKSIZE,    // ��ջ��С
							 0,                 // ����
							 0,                 // ������־
							 &m_dwTaskID            // ����ID
						   );

    // �紴������ʧ��, �ͷ�ǰ�洴������־����
    if (0 == m_hTask)
    {
        OspCloseMailbox(m_dwReadQueHandle, m_dwWriteQueHandle);
        m_dwReadQueHandle = 0;
        m_dwWriteQueHandle = 0;
        m_dwTaskID = 0;
        return FALSE;
    }

	// �ɹ�, ����־������ӵ������������Ա��Ժ�ɾ��
	g_Osp.AddTask(m_hTask, m_dwTaskID, "OspLogTask");


	return TRUE;
}

/*====================================================================
��������COspLog::Quit
���ܣ��˳���־ϵͳ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������COspLog::LastStatusRestore
���ܣ�����־�ָ�����һ��ϵͳ�ر�ʱ��״̬
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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

//������־�ļ�������д��־�ļ�
void COspLog::LogFileOpen()
{
	m_bLogFileEnable = TRUE;
}

//ֹͣ��־�ļ���������д��־�ļ�
void COspLog::LogFileClose()
{
	m_bLogFileEnable = FALSE;
}

/*====================================================================
��������COspLog::LogQueWriteFinal
���ܣ�����־�����з���Ϣ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����tOspLogHead: ��־ͷ��,
              szContent: Ҫд������,
              uLen: ����.
  ����ֵ˵����
====================================================================*/
void COspLog::LogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen)
{
	//szContent==NULL dwLen=0 ����osp quitʱ�˳���־�߳�
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
		m_bIsLogTaskMsgSnd = TRUE;//û�п��ǲ����̡߳�
	}

	//�����ڴ泤��ΪTOspLogCommonHead + TOspLogHeadͷ + MAX_LOG_MSG_LEN
	//Ϊ�����ٴο��������ڴ���LogQueOut()��ʹ����Ϻ�����ͷ�
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
		pchLogStart[dwLen] = '\0';//�Է���һ�ַ���©����ֹ��\0
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

	// ��������������������Ϣ���쳣, ��������Ϣ
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
		// ���LOG�����Ѿ��������߶�������������ͨ������������۲�
		// �����������û���κ����壬��ÿһ���ӡ���滻Ϊһ��������ʾ
		// ���ڷ�������ʱ������û�ã��������ŵ���
		//printf("Osp: send message to mailbox failed in COspLog::LogQueWrite().\n");
		return;
	}
	m_dwMsgIncome++;
}

/*====================================================================
��������COspLog::LogQueOut
���ܣ�����־������ȡ����Ϣ��������ļ���/��Ļ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����
����ֵ˵��:
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

		/* �����ŵ���ָ����־��Ϣͷ��ָ�� */
		ptOspLogHead = (TOspLogCommonHead *)osMsg.address;
        if(OSP_LOG_INTERFACE_SIMPLE == ptOspLogHead->chOspInterfaceType)
        {
        	//����OspPrintf��Ĵ�ӡ��¼
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

		/* �ͷ���־��Ϣռ�õ��ڴ� */
		OspFreeMem((void *)osMsg.address);
    }//end while

	//�߳��˳�
    Quit();
	OspTaskExit();
}
/*=============================================================================
 �� �� ��  : COspLog::LogQueOutSimpleFinal
 ��������  : �˺�����Ӧԭ����OspPrintf/OspLog/OspTrcPrintf/OspMsgTrace����־���
             �˴�����OSP_CLASSLOG GLBLOG�ӿڣ���LogQueOutUniformFinal
 �㷨ʵ��  :
 ����˵��  :
            [I]ptOspLogHead
 ����ֵ˵��: TRUE ��Ҫ�����߳� FALSE ����Ҫ�����߳�
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
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

	/* Ҫ�������־���� */
	dwOutputLen = ptOspLogHead->dwLength;

	/* ����Ҫ�������Ϣ��������־��Ϣͷ���� */
	pchOutputMsg = (s8*)(ptOspLogHead + 1);

	// ͷ��ȫΪ0����־��ʾ�����˳�����
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

	//��־��¼���ļ�
	//����־Ϊ�˼��ݾɵĻ���(OspLog, OspPrintf��) ��Щ�ӿڵ���־���浽�ö���
	if((ptOspLogHead->bToFile) && (TRUE == m_bLogFileEnable))
	{
		++m_dwRunFileLogNum;

		m_cRunLogFile.WriteLogFile(pchOutputMsg);
	}

	return FALSE;
}
/*=============================================================================
 �� �� ��  : COspLog::LogQueOutUniformFinal
 ��������  : ��Ӧ�˴�����OSP_CLASSLOG GLBLOG�ӿڵ���־���
 �㷨ʵ��  :
 ����˵��  :
            [I]ptOspLogHead
 ����ֵ˵��: ��
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
void COspLog::LogQueOutUniformFinal(TOspUniformLogHead *ptOspLogHead)
{
	//���δ������־ ��д��־�ļ�
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

//������־�ļ���Ŀ¼���ļ���С��������־�ļ�����
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
//����ֵΪ�ַ�����Ч���ȣ�������\0
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
 �� �� ��  : COspXLog::operator()
 ��������  : ��ӡ�����־
 �㷨ʵ��  :
 ����˵��  :
            [I]byLogLev ��־����
 ����ֵ˵��: �ļ������� ����'\0'
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
void COspXLog::operator()(u8 byLogLev, const s8* szFormat, ...) const
{
	//�����޸�: osp�˳�����д��־�����õ�OspAllocMem()�����
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
 �� �� ��  : COspXLog::operator()
 ��������  : ��ӡ�����־
 �㷨ʵ��  :
 ����˵��  :
 			[I]byLogMod ��־ģ��
            [I]byLogLev ��־����
 ����ֵ˵��: �ļ������� ����'\0'
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
void COspXLog::operator()(u8 byLogMod, u8 byLogLev, const s8* szFormat, ...) const
{
    //�����޸�: osp�˳�����д��־�����õ�OspAllocMem()�����
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
 �� �� ��  : COspXLog::GetCompileInfo
 ��������  : ��ȡ������Ϣ(file-line-class-function)
 �㷨ʵ��  :
 ����˵��  :
            [I]szPath
            [I]dwBufferLen pchBuffer�������󳤶� ����'\0'
            [O]pchBuffer �ļ�����ſռ���ʼ��ַ
 ����ֵ˵��: �ַ������� ����'\0'
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
u32 COspXLog::GetCompileInfo(u32 dwBufferLen, s8* pchBuffer) const
{
	if(NULL == pchBuffer)
	{
		return 0;
	}

	u32 dwPos = 0; //�����ַ�λ��
	u32 dwFreeSize = dwBufferLen; //ʣ��Ŀɴ洢�ֽ���

	//�����ļ���
    u32 dwLen = GetFileNameFromPath(m_szFileName, dwFreeSize, pchBuffer + dwPos);
    if(0 != dwLen)
    {
    	dwPos += (dwLen - 1); //�۳�\0
		dwFreeSize -= (dwLen - 1);//�۳�\0
    }

	//����'('
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '(');
	//�����к�
	s8 szFileLine[10] = {0};
    sprintf(szFileLine, "%d", m_nFileLine);
    INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, szFileLine, strlen(szFileLine));
	//����')'
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ')');

    //����' '
	INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ' ');

	//����������
	dwLen = GetTypeName(m_szClassName, dwFreeSize, pchBuffer + dwPos);
    if (0 != dwLen)
    {
    	dwPos += dwLen - 1; //�۳�\0
		dwFreeSize -= dwLen + 1;//�۳�\0
        //����::
		INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ':');
        INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ':');
    }
	//���뺯����
    if (m_szFunName != NULL)
    {
    	INSERT_C_STRING(pchBuffer, dwPos, dwFreeSize, m_szFunName, strlen(m_szFunName));
        //����()
		INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '(');
        INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, ')');
    }

    INSERT_CHAR(pchBuffer, dwPos, dwFreeSize, '\0');

	return dwBufferLen - dwFreeSize;
}

/*=============================================================================
 �� �� ��  : COspXLog::GetFileNameFromPath
 ��������  : ��ȡ�ļ��������ļ�ȫ����ȡ��������·�����Ĳ���
 �㷨ʵ��  :
 ����˵��  :
            [I]szPath
            [I]dwBufferLen pchBuffer�������󳤶� ����'\0'
            [O]pchBuffer �ļ�����ſռ���ʼ��ַ
 ����ֵ˵��: �ļ������� ����'\0'
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
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

	//�������Ȱ���'\0'
    u32 dwActualLen = szPath + dwTotalLen - pNameStartPos + 1;
	dwActualLen = (dwActualLen <= dwBufferLen)? dwActualLen : dwBufferLen;

	memcpy(pchBuffer, pNameStartPos, dwActualLen);

    return dwActualLen;
}
/*=============================================================================
 �� �� ��  : COspXLog::GetTypeName
 ��������  : ��ȡ������
 �㷨ʵ��  :
 ����˵��  :
            [I]szTypeNameRaw ԭʼ������
            [I]dwBufferLen pchBuffer�������󳤶� ����'\0'
            [O]pchBuffer ��ȡ��ʵ��������ſռ���ʼ��ַ
 ����ֵ˵��: �ļ������� ����'\0'
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
u32 COspXLog::GetTypeName(const char* szTypeNameRaw, u32 dwBufferLen, char* pchBuffer)
{

    if ((NULL == szTypeNameRaw)||(NULL == pchBuffer))
    {
        return 0;
    }

    u32 dwActlen;

#ifdef WIN32
    //win32��ֱ�Ӿ�����ʵ��������
	dwActlen = strlen(szTypeNameRaw) + 1;
	dwActlen = (dwActlen < dwBufferLen)? dwActlen : dwBufferLen;
	memcpy(pchBuffer, szTypeNameRaw, dwActlen);
    return dwActlen;
#endif

#ifdef _LINUX_

	dwActlen = 0;
    //Ŀǰ����mangle name�ķ�ʽ�ǳ���ʱ��Ĭ�ϲ����н���
    if (g_bIsParseMangleName == TRUE)
    {
        //ʹ��shell����c++filt�õ���ʵ��������
        char szTypeName[256] = {0};
        char achCmd[256] = {0};

        sprintf(achCmd, "c++filt %s", szTypeNameRaw);

        //ʹ�ùܵ�ִ������õ�������
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
//��־�ļ�������־���
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
    //windowsҲ����ʹ��"/"��Ϊ���Ŀ¼�ָ����������滻��
    replace_str(m_achLogDir, DIR_SEP, '\\');
#endif

    //��������Ŀ¼·��
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
	//�ļ������û�ָ��������Ϊǰ׺
    dwLen = SNPRINTF(m_achLogNamePrefix, dwFreeSize, "%s-", szFileName);
    dwFreeSize -= dwLen;
	dwPos += dwLen;

	//�ļ��������Ͻ�������
	dwLen = GetProcessName(dwFreeSize, m_achLogNamePrefix + dwPos);
	dwFreeSize -= dwLen;
	dwPos += dwLen;

    m_nLogFileSizeMax = nLogSizeMax_kb;
    m_nLogFileNumMax = nLogNumMax;

    return TRUE;
}

#define IsStartWithStr(parent, sub) (strstr(parent, sub) == parent)

//��־�ļ�
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

	//Ѱ�ҿ�д�ļ�
	if (ExistFileOrDir(m_achCurFile)) //�ļ�����
	{
		struct stat fileStat;
		stat(m_achCurFile, &fileStat);
		long dwLogSize = fileStat.st_size;

		if (dwLogSize >= (long)(m_nLogFileSizeMax*CAPACITY_1K)) //�������log����,�����ļ�
		{
			//�ȹرյ�ǰ�ļ�
			if (m_pFile != NULL)
			{
				fclose(m_pFile);
				m_pFile = NULL;
			}

			//ɾ�����ϵ��ļ�
			if (GetFileNum() >= m_nLogFileNumMax)
			{
				char m_achFirstFile[MAX_FILE_DIR_NAME_LEN];
				GetFirstFile(MAX_FILE_DIR_NAME_LEN, m_achFirstFile);
				DelFile(m_achFirstFile);
			}

			//�������ļ�
			GetNewFile(MAX_FILE_DIR_NAME_LEN, m_achCurFile);
		}
		else
		{
			//�ļ����ڣ�����δ�ﵽ����������ǿ�д�ļ�
		}
	}
	else //�ļ������ڣ��϶���д��ֱ�Ӵ���
	{
		//���ļ������Ϊ�գ��������û��ֶ�ɾ������־�ļ�������Ч�������Ҫclose
		if (m_pFile != NULL)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}

	if (m_pFile == NULL)
	{
		const char* szOpenMode = "a+";	 //Ĭ�ϴ򿪷�ʽ,׷��
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
        //�½�һ���ļ�
        return GetNewFile(dwBuffenLen, pchBuffer);
    }
    else
    {
        //��һ�����µ��ļ�
        return GetLastFile(dwBuffenLen, pchBuffer);
    }
}
//�������ļ���
u32 COspXLogFile::GetNewFile(u32 dwBuffenLen, char* pchBuffer)
{
	if (NULL == pchBuffer)
	{
		return 0;
	}

	pchBuffer[0] = '\0';

    //�������ļ���
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
//����ֵΪ�ַ�����Ч���ȣ�������\0
u32 COspXLogFile::GetFirstFile(u32 dwBuffenLen, char* pchBuffer)
{
    if(NULL == pchBuffer)
	{
		return 0;
	}
	pchBuffer[0] = '\0';

    //��ȡ��old���ļ�
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

	//vc6 û��intptr_t���Ͷ��� ʵ���ϵȼ���long����
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

    //���Ŀ¼
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

    //��ȡ���µ��ļ�
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

		//���ֵ�ǰ׺�ͺ�׺����� ����д��
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

    //���Ŀ¼
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
    //�ļ����Ƶ�ʱ���ʶ
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
��������LogSysInit
���ܣ���־ϵͳ��ʼ������OspInit()���á�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵������

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 LogSysInit()
{
	if(TRUE != g_cOspLog.Initialize())
	{
		return FALSE;
	}

	//����OSPģ�����־�����ģ������
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
��������ospver
���ܣ���ʾOsp�ĵ�ǰ�汾��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵������
====================================================================*/
API void ospver()
{
	OspVerPrintf();
}
/*====================================================================
��������OspLog
���ܣ�����Ӧ��������ʾ����Ļ,�洢���ļ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uLevel: log����
              szFormat: Log �ĸ�ʽ
              ...: �䳤������
����ֵ˵������
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
 �� �� ��  : OspUniformPrintf
 ��������  : ��־��¼�����Կ����ض�ģ����ض���־�����
 			 ��Ӧ�˴�����OSP_CLASSLOG GLBLOG�ӿڵ���־���
             �˽ӿڵ���־���ջᱣ�浽�ļ�
 �㷨ʵ��  :
 ����˵��  :
            [I]ptOspLogHead
 ����ֵ˵��: ��
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
API void OspUniformPrintf(TOspLogContent& cLogContent)
{
	//�����޸�: osp�˳�����д��־�����õ�OspAllocMem()�����
	if ((FALSE == g_Osp.m_bInitd) || g_Osp.m_bKillOsp)
    {
        return;
    }

    s8 achFullMsg[MAX_LOG_MSG_LEN];

	u32 dwLen;
    u32 dwPos = 0;
    u32 dwFreeSize = MAX_LOG_MSG_LEN;

	//��־�ļ�д��־��������Ϣ
	//Ƕ��ʱ��
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

	//Ƕ����־����ģ����Ϣ
	INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
					cLogContent.m_achModLev, strlen(cLogContent.m_achModLev));

	//ǰ׺��Ϣ
    if (g_byPrintOspPrefix != 0)
    {
        INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  cLogContent.m_achOspPrifixField, strlen(cLogContent.m_achOspPrifixField));
    }

	//Ƕ����־����
	INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  			cLogContent.m_achBodyField, strlen(cLogContent.m_achBodyField));

	//Ƕ�������Ϣ
    if (g_byPrintCompileInfo != 0)
    {
		//�滻֮ǰ�Ļ��з�
		if (achFullMsg[dwPos - 1] == '\n')
		{
			achFullMsg[dwPos - 1] = '\\';
		}
        INSERT_C_STRING(achFullMsg, dwPos, dwFreeSize,
	  	  cLogContent.m_achCompileField, strlen(cLogContent.m_achCompileField));
		INSERT_CHAR(achFullMsg, dwPos, dwFreeSize, '\n');
    }

	//��Ļ��ӡ��� �ܿ��ؿ���
    OspTrcPrintf(cLogContent.m_bIsPrintScreen, FALSE, "%s", achFullMsg);

    //��־�ļ����
    if (OSP_TRIVIAL_LEV == cLogContent.m_byLogLev)
    {
        //����OSP_TRIVIAL_LEV(255)��ospext�ڲ�����Ƶ���Ĵ�ӡ����(����������ѯ)����д��־�ļ�
        return;
    }

    u32 dwLogFileTypeSet = OSP_LOG_FILE_NONE;

    //�Ƿ�д������־
    if ((g_bIsWriteRunLog == TRUE) && (cLogContent.m_byLogLev <= g_byRunLogFileLev))
    {
        dwLogFileTypeSet += OSP_LOG_FILE_RUN;
    }

    //�Ƿ�д������־
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
��������OspPrintf
���ܣ�����Ӧ��������ʾ����Ļ/�洢���ļ�
      ��ʾ����Ļ���ܣ����ܿ��ؿ��� ��������Ļ��ʾ
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bScreen: �Ƿ��������Ļ,
              bFile: �Ƿ�������ļ�,
			  szFormat: ��ʽ,
����ֵ˵����
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
��������OspTrcPrintf
���ܣ�����Ӧ��������ʾ����Ļ, �洢���ļ�, OSP�ڲ�����Ϣ����ʱʹ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bScreen: �Ƿ��������Ļ,
              bFile: �Ƿ�������ļ�,
			  szFormat: ��ʽ
����ֵ˵����
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
��������OspMsgTrace
���ܣ�����Ϣ��Ϊһ�����������Ļ����־���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bScreen: �Ƿ��������Ļ,
              bFile: �Ƿ�������ļ�,
			  content: ����,
			  dwLen: ����
����ֵ˵����
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
��������OspDumpPrintf
���ܣ�����Ӧ��������ʾ����Ļ,�洢���ļ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bScreen: �Ƿ��������Ļ,
              bFile: �Ƿ�������ļ�,
			  szFormat: ��ʽ
����ֵ˵����
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
��������OspAddEventDesc
���ܣ����¼��������Ǽǵ�OSP
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szDesc: ʱ�������ַ���
              uEvent: �¼���
����ֵ˵����
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
��������OspAppDescAdd
���ܣ��������ڵ��App���������뵽OSP��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����wAid: app���
              szName: app����, ��������'\0'�������ַ���
����ֵ˵����
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
 �� �� ��  : OspOpenLogFile
 ��������  : ������־�ļ� ��������־�ļ��Ĳ���
             ���øýӿں󣬲Ż�������־�ļ���¼
 �㷨ʵ��  :
 ����˵��  :
            [I]const char * szDir 	   ��־�ļ������Ŀ¼
            [I]u32 dwMaxSizeKB     ÿ���ļ�������ֽ���
            [I]dwLogNumMax         ��־�ļ���������
 ����ֵ˵��: TRUE �ɹ� FALSE ʧ��
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��7��6��    1.0    dengchange    �޸�
    ԭ�е�֧��������־�ļ�Ŀ¼���ļ�������Ϊ����֧�������ļ�Ŀ¼
    ��־�ļ�����osp�Զ����ɣ�ȡ�ļ����ɵ�ʱ����Ϊ�ļ���
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

	//Ŀǰ���Ϊ������־�ʹ�����־��������ͬһĿ¼
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
 �� �� ��  : OspOpenLogFileEx
 ��������  : ������־�ļ� ��������־�ļ��Ĳ���
 �㷨ʵ��  :
 ����˵��  :
 			[I]const char* szFileName   ��־�ļ�����ǰ׺
            [I]const char* szDir 	   ��־�ļ������Ŀ¼
            [I]u32 dwLogSizeMax_kb ÿ���ļ�������ֽ���
            [I]dwLogNumMax         ��־�ļ���������
 ����ֵ˵��: TRUE �ɹ� FALSE ʧ��
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��7��6��    1.0    dengchange    �޸�
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

	//Ŀǰ���Ϊ������־�ʹ�����־��������ͬһĿ¼
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
��������OspCloseLogFile
���ܣ��ر�OSP����־�ļ�. ����OspQuit()�󣬲���Ҫ�ٵ��øú���.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵����
====================================================================*/
API void OspCloseLogFile()
{
	g_cOspLog.LogFileClose();
}

/*====================================================================
��������OspSetFileLogLevel
���ܣ������ļ���־�ȼ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����wAid: LOCAL_APP -- ����ȫ����־�ȼ�, �������ö�Ӧapp����־�ȼ�,
              byFileLevel: �ļ���־�ȼ�.
����ֵ˵����
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
��������OspSetScrnLogLevel
���ܣ�������Ļ��־�ȼ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAid: LOCAL_APP -- ����ȫ����־�ȼ�, �������ö�Ӧapp����־�ȼ�,
			  uScreenLevel: ��Ļ��־�ȼ�.
����ֵ˵����
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
��������OspSetLogLevel
���ܣ�����App����־�ȼ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAid: LOCAL_APP -- ����ȫ����־�ȼ�, �������ö�Ӧapp����־�ȼ�,
              uFileLevel: �ļ���־�ȼ�,
			  uScreenLevel: ��Ļ��־�ȼ�.
����ֵ˵����
====================================================================*/
API void OspSetLogLevel(u16 wAppID, u8 byFileLevel, u8 byScreenLevel)
{
	OspSetFileLogLevel(wAppID, byFileLevel);
	OspSetScrnLogLevel(wAppID, byScreenLevel);
}

/*====================================================================
��������OspSetFileTrcFlag
���ܣ������ļ����ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAid: LOCAL_APP -- ����ȫ�ָ��ٱ�־, �������ö�Ӧapp�ĸ��ٱ�־,
              uFileFlag: �ļ����ٱ�־,
����ֵ˵����
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
��������OspSetScrnTrcFlag
���ܣ�������Ļ���ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAid: LOCAL_APP -- ����ȫ�ָ��ٱ�־, �������ö�Ӧapp�ĸ��ٱ�־,
              uScreenFlag: ��Ļ���ٱ�־,
����ֵ˵����
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
��������OspSetTrcFlag
���ܣ������ļ�����Ļ���ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����uAid: LOCAL_APP -- ����ȫ�ָ��ٱ�־, �������ö�Ӧapp�ĸ��ٱ�־,
              uFileFlag: �ļ����ٱ�־��
              uScreenFlag: ��Ļ���ٱ�־.
====================================================================*/
API void OspSetTrcFlag(u16 wAid, u16 wFileFlag, u16 wScreenFlag)
{
	OspSetFileTrcFlag(wAid, wFileFlag);
    OspSetScrnTrcFlag(wAid, wScreenFlag);
}

/*====================================================================
��������OspTrcAllOn
���ܣ���OSP���е�ȫ����Ļ���ļ����ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵����
====================================================================*/
API void OspTrcAllOn(void)
{
	OspSetTrcFlag( LOCAL_APP, TRCALL, TRCALL );

	/* ����uAppId������u16��������ܵ���uAppId������Ӷ���ѭ�� */
    for(u16 wAppId=1; wAppId<=MAX_APP_NUM; wAppId++)
    {
        OspSetTrcFlag( wAppId, TRCALL, TRCALL );
    }
}

/*====================================================================
��������OspTrcAllOff
���ܣ��ر�OSP���еĸ��ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵����
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
��������OspSendTrcOn
���ܣ���OSP�ⷢ��Ϣ���ٱ�־
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspSendTrcOn()
{
    g_Osp.m_cNodePool.m_dwSendTrcFlag = 1;
}

/*====================================================================
��������OspRcvTrcOn
���ܣ���OSP�����ⲿ��Ϣ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵����
====================================================================*/
API void OspRcvTrcOn()
{
    g_Osp.m_cNodePool.m_dwRcvTrcFlag = 1;
}

/*====================================================================
��������OspSendTrcOff
���ܣ��ر�OSP�ⷢ��Ϣ���ٱ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
API void OspSendTrcOff()
{
    g_Osp.m_cNodePool.m_dwSendTrcFlag = 0;
}

/*====================================================================
��������OspRcvTrcOff
���ܣ��ر�OSP�ⲿ��Ϣ���չ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵����
====================================================================*/
API void OspRcvTrcOff()
{
    g_Osp.m_cNodePool.m_dwRcvTrcFlag = 0;
}

/*====================================================================
��������OspSetLogEventDataLength
���ܣ�����ȫ�ֵ���Ϣ������ʾ�ĳ���, δʵ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����uLength: �³���

 ����ֵ˵����
====================================================================*/
API u16 OspSetLogEventDataLength(u16)
{
    return 0;
}

//����ʮ�е���Ϣ�����Ƿ���� TRUE ���  FALSE �����
API BOOL32 IsOspLogLongMsgPrintEnbl(void)
{
	return g_cOspLog.m_bLMsgDumpEnbl;
}

/*====================================================================
��������OspMsgDumpSet
���ܣ����ó���ʮ�е���Ϣ�����Ƿ������ȱʡ�����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����bLongMsgDumpEnbl: ����/��ֹ����Ϣ���

  ����ֵ˵����
====================================================================*/
API void OspMsgDumpSet(BOOL32 bLongMsgDumpEnbl)
{
	g_cOspLog.m_bLMsgDumpEnbl = bLongMsgDumpEnbl;
}

/*====================================================================
��������LogTask, ��־������ں���
���ܣ�����־�����ж�����Ϣ,���������Ļ���ļ���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵����
====================================================================*/
API void LogTask()
{
    g_cOspLog.LogQueOut();
}

/*====================================================================
��������COspEventDesc::DescAdd
���ܣ� �����¼�������OSP
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ���:
�������˵��: szDesc: �¼���������������'\0'�������ַ�����uEvent: �¼�
����ֵ˵����
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
��������COspEventDesc::DescGet
���ܣ���ȡ�¼���Ӧ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����uEvent: �¼�
�������˵����

  ����ֵ˵���������¼�����
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
��������COspEventDesc::COspEventDesc
���ܣ��¼�������Ĺ��캯��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵���������¼�����
====================================================================*/
COspEventDesc::COspEventDesc()
{
    memset( EventDesc, 0, sizeof(EventDesc) );
}

/*====================================================================
��������COspEventDesc::~COspEventDesc
���ܣ��¼�������������������ͷ��¼�������ռ�õ��ڴ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵���������¼�����
====================================================================*/
COspEventDesc::~COspEventDesc()
{
	Destroy();
}

/*====================================================================
��������COspEventDesc::COspEventInit
���ܣ�  ��ʼ��һЩOSP�¼�������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵���������¼�����
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
��������COspEventDesc::Destroy
���ܣ��¼�������������������ͷ��¼�������ռ�õ��ڴ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵���������¼�����
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
��������COspAppDesc::DescAdd
���ܣ������wAppId���App������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szDesc: ������������'\0'�������ַ���,
              wAppId: Ŀ��App��ID.
  ����ֵ˵����
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
��������COspAppDesc::COspAppDesc
���ܣ�COspAppDesc��Ĺ��캯��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
COspAppDesc::COspAppDesc()
{
    memset(this, 0, sizeof(COspAppDesc));
}

/*====================================================================
��������COspAppDesc::~COspAppDesc
���ܣ�COspAppDesc�����������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
====================================================================*/
COspAppDesc::~COspAppDesc()
{
	Destroy();
}

/*====================================================================
��������COspAppDesc::Destroy
���ܣ�COspAppDesc�����������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵����
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
��������OspCat
���ܣ���ʾָ���ļ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����fname: �ļ���

  ����ֵ˵��:
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
��������OspEventDescShow
���ܣ���ʾ��Ϣ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����u16 wEventID : ��Ϣ��ID
����ֵ˵��:
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
��������OspEventDescShow
���ܣ�������Ϣ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����u16 wEventID : ��Ϣ��ID
����ֵ˵��: ��Ϣ������. �����ϢID��Ч���߸���Ϣû�������ַ�����
            ����ֵ��ΪNULL.
====================================================================*/
API const char * OspEventDesc(u16 wEventID)
{
    return g_Osp.m_cOspEventDesc.DescGet(wEventID);
}

/*====================================================================
��������OspFileLogNum
���ܣ�ȡ���ѳɹ�������ļ���־��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵��: �ļ���־��.
====================================================================*/
API u32 OspFileLogNum(void)
{
	return g_cOspLog.m_dwRunFileLogNum + g_cOspLog.m_dwErrorFileLogNum;
}

/*====================================================================
��������OspScrnLogNum
���ܣ�ȡ���ѳɹ��������Ļ��־��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵��: ��Ļ��־��.
====================================================================*/
API u32 OspScrnLogNum(void)
{
	return g_cOspLog.m_dwScreenLogNum;
}

/*====================================================================
��������OspLogFileNo
���ܣ�ȡ�õ�ǰ��־�ļ���.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵��: ��־�ļ���.
====================================================================*/
API u32 OspLogFileNo(void)
{
	return 0;
	//�����ļ�����ʱ��Ϊ�ļ��� �ļ���������
	//return g_cOspLog.m_dwCurrentFileNo;
}

/*====================================================================
��������OspLogShow
���ܣ���ʾ��־ģ��״̬
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����
����ֵ˵��: ��
====================================================================*/
API void OspLogShow(void)
{
	g_cOspLog.Show();
}


//����App�ص�����ִ����Ϣ��¼����
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


//������־����Ļ�ȡ�ַ�����
API const char * OspGetStrLogLevel(u8 byLogLev)
{
    return g_cLogLevString.GetLogLevDesc(byLogLev);
}
//ͨ����־ģ����Ż��߸�ģ�����־���� ���û�����ù� ��ȡ����ֵ����OSP_TRIVIAL_LEV
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
//ͨ����־ģ����Ż�ȡ��ģ�������
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
 �� �� ��  : OspWriteUniformLogFile
 ��������  : ����־д�뵽��־����
 			 ��Ӧ�˴�����OSP_CLASSLOG GLBLOG�ӿڵ���־���
             �˽ӿڵ���־���ջᱣ�浽�ļ�
 �㷨ʵ��  :
 ����˵��  :
            [I]ptOspLogHead
 ����ֵ˵��: ��
-------------------------------------------------------------------------------
 �޸ļ�¼  :
 ��  ��            �汾    �޸���    �޸�����
 1. 2015��6��29��    1.0    dengchange    �����ɺ���
=============================================================================*/
API void OspWriteUniformLogFile(TOspUniformLogHead tLogData, const char * szContent, u32 dwLen)
{
	//�����ڴ泤��ΪTOspLogCommonHead + TOspLogHeadͷ + MAX_LOG_MSG_LEN
	//Ϊ�����ٴο��������ڴ���LogQueOut()��ʹ����Ϻ�����ͷ�
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

	pchLogStart[dwLen] = '\0';//�Է���һ�ַ���©����ֹ��\0

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
//��ȡ��������
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
��������TeleCmdEcho
���ܣ���Telnet�ͻ�����Ļ�����û�����.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchCmdStr: ���,
              uLen: �����.

����ֵ˵������.
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

//�����־
API void OspLogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen)
{
	g_cOspLog.LogQueWrite(tOspLogHead, szContent, dwLen);
}

//�ָ������ ���ڴ����Ķ�
#if 0
#endif
/*====================================================================
	���������Ϊ��λ����͵���֮�ã��ʼ����ڴ�
====================================================================*/

//��ͣ��־��Ļ���
API void OspStopScrnLog(void)
{
	g_cOspLog.StopScrnLog();
}

//�ָ���־��Ļ���
API void OspResumeScrnLog(void)
{
	g_cOspLog.ResumeScrnLog();
}

//����ģ�����־�����ģ������
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

//��־��ȫ�ִ�ӡ���� levelֵ���ڸ�ֵ����־������Ļ��ʾ
API void OspSetGlbLogPrintLevel(u8 byLogLev)
{
	g_byGlbLogPrintLev = byLogLev;
}

//������־��ʱ����Ϣ��ӡ 0:����ӡʱ��; 1:��ӡ�뼶ʱ��; 255:��ӡ���뼶ʱ��
API void OspSetLogTimeLevel(u8 byLogLev)
{
    g_byPrintTimeLev = byLogLev;
}

//�����Ƿ��ӡ������Ϣ(file-line-class-function) 0:����ӡ������Ϣ; 1:��ӡ������Ϣ
API void OspSetLogCompileInfo(u8 byPrint)
{
    g_byPrintCompileInfo = byPrint;
}

#ifdef _LINUX_
//linux�Ƿ����mangle name�Ľ���
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


//������־���Ƿ��ӡǰ׺��Ϣ(app-inst-task-state) 0:����ӡOSPǰ׺; 1:��ӡOSPǰ׺
API void OspSetLogPrefix(u8 byPrint)
{
    g_byPrintOspPrefix = byPrint;
}

//�����Ƿ����������־�ļ��Լ�������־�ļ���
//0:�����������־; ��0: ��־����ֵС�ڵ��ڸ�ֵ�ģ����Ϊ������־
//Ŀǰд���ļ������ݺ���Ļ�Ĵ�ӡ����һ��
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

//�����Ƿ����������־�ļ���//0:�����; ��0:���
//�������ּ������־��Ϊ�Ǵ�����־��OSP_ERROR_LEV //1���󼶱�  OSP_WARNING_LEV //2���漶��
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

//��ʾ��־���Ʋ���
API void OspShowLogConfig()
{
    OspPrintf(TRUE, FALSE,"ȫ����־����: %s\n", OspGetStrLogLevel(g_byGlbLogPrintLev));
    OspPrintf(TRUE, FALSE,"��־ʱ�伶��: %u  (0:�����ʱ��, ��0:�뼶ʱ��, 255: ���뼶ʱ��)\n", g_byPrintTimeLev);
    OspPrintf(TRUE, FALSE,"���������Ϣ: %u  (0:�����, ��0:���)\n", g_byPrintCompileInfo);
    OspPrintf(TRUE, FALSE,"���OSPǰ׺: %u  (0:�����, ��0:���)\n", g_byPrintOspPrefix);
    OspPrintf(TRUE, FALSE,"��ǰ����ʵ��: [%u-%u] ([0-0]:������, ��[0-0]:���ٵ�app-inst��)\n",
    GETAPP(g_dwTraceAppInst), GETINS(g_dwTraceAppInst));
    OspPrintf(TRUE, FALSE,"��ǰ��������: %u  (0:������, ��0:���������)\n", g_dwTraceTaskNO);
    OspPrintf(TRUE, FALSE,"�Ƿ�д������־�ļ�: %u  (0:��д, ��0:д)\n", g_bIsWriteRunLog);
    OspPrintf(TRUE, FALSE,"�Ƿ�д������־�ļ�: %u  (0:��д, ��0:д)\n", g_bIsWriteErrLog);
#ifdef _LINUX_
    OspPrintf(TRUE, FALSE,"�Ƿ����Mangle Name: %u  (0:������, ��0:����)\n", g_bIsParseMangleName);
#endif
    OspPrintf(TRUE, FALSE, "�Ƿ���ʱ����: %u  (0:�����, ��0:���)\n", g_bIsMonitorTimePerform);

    OspPrintf(TRUE, FALSE, "\n��־�����б�: \n");
    for(u8 byLogLev = OSP_NOLOG_LEV; byLogLev < OSP_LOG_LEV_NUM; byLogLev++)
    {
        OspPrintf(TRUE, FALSE,"  %s------%u\n", OspGetStrLogLevel(byLogLev), byLogLev);
    }
}


