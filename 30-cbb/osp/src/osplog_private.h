#ifndef __OSP_LOG_PRIVATE_H__
#define __OSP_LOG_PRIVATE_H__

#include <string>
#include <stdio.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "osp.h"
#include "xmap.h"
#include "osptool.h"
#include "ospsch.h"
#include "ospteleserver.h"
#include "osplog.h"

//��־�ļ��ܵĴ�С: 5M
#define MAX_LOG_SIZE_KB          (u32)(5<<10)

//runlog
#define DEFAULT_RUN_LOG_DIR_NAME      "./runlog/"
#define DEFAULT_RUN_LOG_FILE_SIZE_kb  (1*1024)     //ÿ��������־�ļ����Ϊ1M
#define DEFAULT_RUN_LOG_FILE_NUM      (10)         //10�ļ�ѭ������

//errlog
#define DEFAULT_ERR_LOG_DIR_NAME      "./errlog/"
#define DEFAULT_ERR_LOG_FILE_SIZE_kb  (1*1024)    //ÿ��������־�ļ����Ϊ1M
#define DEFAULT_ERR_LOG_FILE_NUM      (5)         //5�ļ�ѭ������

#define DEFAULT_LOG_DIR_NAME       ("./osplog/")
#define DEFAULT_LOG_FILE_SIZE_kb   (1024)   //1024*1kb = 1M
#define DEFAULT_LOG_FILE_NUM       (10)
#define DEFAULT_LOG_SUFFIX         (".log")

//��־�ļ�
class COspXLogFile
{
public:
    COspXLogFile();
    ~COspXLogFile();

private:
    COspXLogFile(const COspXLogFile&);
    void operator=(const COspXLogFile&);
public:
    //����־����ʱ��Ϊ��־�ļ�����
    void WriteLogFile(const char*);

    //������־�ļ�����
    BOOL32 SetLogFileParam(const char* szFileName, const char* pchDir, u32 nLogSizeMax_kb, u32 nLogNumMax, u32 dwLogType);

    //��ȡ�������͵�ǰʱ����ɵ��ļ���
    static u32 GetFileStamp(u32 dwBuffenLen, char* pchBuffer);

    //���ļ�����ȡ��׺��������û��
	static u32 GetSuffixFromFile(const char* pchFileName,
								 u32 dwBuffenLen, char* pchBuffer);

    //��õ�ǰ��־�ļ�Ŀ¼
    const char * GetLogDir()
    {
        return m_achLogDir;
    };

    //��õ�ǰ��־�ļ���
    const char * GetLogFileName()
    {
        return m_achLogNamePrefix;
    };

    //�����־�ļ�������ֽ���
    u32 GetLogFileSizeMax()
    {
        return m_nLogFileSizeMax;
    }
    //�����־�ļ���Ŀ
    u32 GetLogFileNumMax()
    {
        return m_nLogFileNumMax;
    }

private:
    u32 GetCurFile(u32 dwBuffenLen, char* pchBuffer);
    u32 GetFirstFile(u32 dwBuffenLen, char* pchBuffer);
    u32 GetLastFile(u32 dwBuffenLen, char* pchBuffer);
    u32 GetFileNum();

    u32 GetNewFile(u32 dwBuffenLen, char* pchBuffer);
    const char* GetSuffixName();
    const char* GetPrefixName(){return m_achLogNamePrefix;}

private:
    u32 m_dwLogType;     //��־����
    char m_achLogDir[MAX_FILE_DIR_NAME_LEN];//string m_strLogDir;  //��־Ŀ¼
    //��־�ļ���ǰ׺
    char m_achLogNamePrefix[MAX_FILE_DIR_NAME_LEN];
    u32 m_nLogFileSizeMax;   //��־�ļ����ֵ����λ��kb
    u32 m_nLogFileNumMax;    //��־�ļ���Ŀ
    char m_achCurFile[MAX_FILE_DIR_NAME_LEN];//string m_strCurFile; //��ǰ��д�ļ�
    FILE* m_pFile;       //��ǰ�򿪵��ļ�����������������ļ�һֱ�򿪣�����ÿ��д��־�����ļ��͹��ļ�
    COspSemLock m_cLock;   //���ܶ��߳�д�ļ�����������
};

//=>2015/06/30 dcg added begin
//OSP_CLASSLOG GLBLOG�ӿڵ�ͷ�ṹ
#pragma pack(1)
typedef struct
{
	u32 dwLogType;//Ĭ��ֵΪOSP_LOG_FILE_NONE;
}TOspUniformLogHead;

#define OSP_LOG_INTERFACE_SIMPLE   1 //ԭ���ļ�¼��ʽ ��¼�ӿ�OspPrint OspLog��
#define OSP_LOG_INTERFACE_UNIFORM 2 //��ǿ�͵�log��¼��ʽ ��¼�ӿ�ΪOSP_CLASSLOG OSP_LOG
typedef struct
{
    u8 chOspInterfaceType;
    s8 achData[1];
}TOspLogCommonHead;
//2015/06/30 dcg added end<=
#pragma pack()

//������־������� �����־����Ļ�����ļ�
class COspLog
{
private:
	//�����
	u32 m_dwTaskID;

	//������ Ŀǰm_hTask��m_dwTaskID���
	TASKHANDLE m_hTask;

	//��������
    MAILBOXID m_dwReadQueHandle;
	//����д���
    MAILBOXID m_dwWriteQueHandle;

	//�Ƿ�����д��־�ļ� Ĭ�ϲ�����
	BOOL32 m_bLogFileEnable;

	//logtask�˳���Ϣ�Ƿ񷢳�
	BOOL32 m_bIsLogTaskMsgSnd;

	//���������Ϣ����
	u32 m_dwMaxMsgWaiting;

    //������־������ļ�����
    COspXLogFile m_cErrLogFile;

    //����ʱ��־������ļ�����
    COspXLogFile m_cRunLogFile;

    //����־����Ϊ�˼��ݾɵĻ���(OspLog, OspPrintf��) ��Щ�ӿڵ���־���浽�ö���
    //������ͳһʹ��OSP_CLASSLOG GLOG�ӿ�
    //CXLogFile m_cSimpleLogFile;

public:
	//ȫ����Ļ��ӡ����  osplog()��ʹ�� ����ֵС�ڵ��ڸ�ֵ����־����Ļ����ʾ
	u8 m_byLogScreenLevel;

	//ȫ����־��ӡ���� osplog()��ʹ�� ����ֵС�ڵ��ڸ�ֵ����־��¼��־�ļ�
    u8 m_byLogFileLevel;

	//��Ļ��ӡ���� ������־�Ƿ�����Ļ�����
	BOOL32 m_bScrnLogEnbl;

	//����Ϣ��ӡ����
	BOOL32 m_bLMsgDumpEnbl;

	//���յ���־��Ϣ����
	u32 m_dwMsgIncome;

	//�Ѵ������־��Ϣ����
	u32 m_dwMsgProcessed;

	//����������������־��Ϣ����
	u32 m_dwMsgDropped;

	//��Ļ��־�ۻ�����
	u32 m_dwScreenLogNum;

	//������־�ܴ���
	u32 m_dwRunFileLogNum;

	//������־�ܴ���
	u32 m_dwErrorFileLogNum;

	//��־�ļ���д�ź���
	//SEMHANDLE m_tLogSem;

public:
	COspLog(void);

	//��־�����ʼ��
	BOOL32 Initialize(void);

	//��־�����˳�
	void Quit(void);

	//������־����־����
    void LogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen);

    //������־����־���� ��_OspWriteUniformLogFile()��LogQueWrite()����
    void LogQueWriteFinal(TOspLogCommonHead *ptOspLogMsg);

	//����־����ȡ����־����ӡ����Ļ���߱��浽�ļ�
    void LogQueOut(void);

    //������־�ļ���Ŀ¼���ļ���С��������־�ļ�����
    BOOL32 OspSetLogFileParam(const char* szFileName, const char* pchDir, u32 dwLogSizeMax_kb,
                            u32 dwLogNumMax, u32 dwLogType);

	//������־�ļ�������д��־�ļ�
	void LogFileOpen();

	//ͣ����־�ļ���������д��־�ļ�
	void LogFileClose();
private:

    //����־������ȡ������ӡ����ӦOspPrintfϵ�нӿ�
    u32 LogQueOutSimpleFinal(TOspLogHead *ptOspLogHead);

    //����־������ȡ������ӡ����ӦOSP_CLASSLOG GLOG�ӿڵ��������
    void LogQueOutUniformFinal(TOspUniformLogHead *ptOspLogHead);

public:
	//�ر���Ļ��ӡ
	void StopScrnLog(void)
	{
		m_bScrnLogEnbl = FALSE;
	}
	//�ָ���Ļ��ӡ
	void ResumeScrnLog(void)
	{
		m_bScrnLogEnbl = TRUE;
	}
	//��ӡ��־ϵͳ��Ϣ
	void Show(void);
};

//��־ģ����������󳤶�
#define MAX_LOG_MODULE_NAME_LENGTH 10

//��־������������󳤶�
#define MAX_LOG_LEVEL_NAME_LENGTH  20

//ģ�����־��������
struct TLogModLevelDesc
{
	u8 dwLogLevel;//��־����
	char achModuleName[MAX_LOG_MODULE_NAME_LENGTH + 1];
};

//��־�����ַ�����
struct TLogLevelDesc
{
	char achLogLevelName[MAX_LOG_LEVEL_NAME_LENGTH + 1];
};

//��ģ��Ϊ�ؼ��� ����ģ�����־�����ģ������
typedef CXMap<u32, TLogModLevelDesc> CModuleLogInfo;

//����־����Ϊ�ؼ��� ������־���������
typedef CXMap<u32, TLogLevelDesc> COspLogLevDesc;

static void replace_str(char* pchBody, char chSrc, char chDst);

static BOOL32 DelFile(const char * szFileName);

static u32 _OspGetTidStr(u32 dwBufferLen, s8* pchBuffer);


u32 GetProcessName(u32 dwBufferLen, char* pchBuffer);

API void OspWriteUniformLogFile(TOspUniformLogHead tLogData,
                                 const char * szContent, u32 dwLen);

#endif //__OSP_LOG_PRIVATE_H__
