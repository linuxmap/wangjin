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

//日志文件总的大小: 5M
#define MAX_LOG_SIZE_KB          (u32)(5<<10)

//runlog
#define DEFAULT_RUN_LOG_DIR_NAME      "./runlog/"
#define DEFAULT_RUN_LOG_FILE_SIZE_kb  (1*1024)     //每个运行日志文件最大为1M
#define DEFAULT_RUN_LOG_FILE_NUM      (10)         //10文件循环覆盖

//errlog
#define DEFAULT_ERR_LOG_DIR_NAME      "./errlog/"
#define DEFAULT_ERR_LOG_FILE_SIZE_kb  (1*1024)    //每个错误日志文件最大为1M
#define DEFAULT_ERR_LOG_FILE_NUM      (5)         //5文件循环覆盖

#define DEFAULT_LOG_DIR_NAME       ("./osplog/")
#define DEFAULT_LOG_FILE_SIZE_kb   (1024)   //1024*1kb = 1M
#define DEFAULT_LOG_FILE_NUM       (10)
#define DEFAULT_LOG_SUFFIX         (".log")

//日志文件
class COspXLogFile
{
public:
    COspXLogFile();
    ~COspXLogFile();

private:
    COspXLogFile(const COspXLogFile&);
    void operator=(const COspXLogFile&);
public:
    //以日志创建时间为日志文件命名
    void WriteLogFile(const char*);

    //设置日志文件参数
    BOOL32 SetLogFileParam(const char* szFileName, const char* pchDir, u32 nLogSizeMax_kb, u32 nLogNumMax, u32 dwLogType);

    //获取进程名和当前时间组成的文件戳
    static u32 GetFileStamp(u32 dwBuffenLen, char* pchBuffer);

    //从文件名获取后缀名，可以没有
	static u32 GetSuffixFromFile(const char* pchFileName,
								 u32 dwBuffenLen, char* pchBuffer);

    //获得当前日志文件目录
    const char * GetLogDir()
    {
        return m_achLogDir;
    };

    //获得当前日志文件名
    const char * GetLogFileName()
    {
        return m_achLogNamePrefix;
    };

    //获得日志文件的最大字节数
    u32 GetLogFileSizeMax()
    {
        return m_nLogFileSizeMax;
    }
    //获得日志文件数目
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
    u32 m_dwLogType;     //日志类型
    char m_achLogDir[MAX_FILE_DIR_NAME_LEN];//string m_strLogDir;  //日志目录
    //日志文件名前缀
    char m_achLogNamePrefix[MAX_FILE_DIR_NAME_LEN];
    u32 m_nLogFileSizeMax;   //日志文件最大值：单位：kb
    u32 m_nLogFileNumMax;    //日志文件数目
    char m_achCurFile[MAX_FILE_DIR_NAME_LEN];//string m_strCurFile; //当前可写文件
    FILE* m_pFile;       //当前打开的文件句柄，进程运行中文件一直打开，并不每次写日志都开文件和关文件
    COspSemLock m_cLock;   //可能多线程写文件，加锁保护
};

//=>2015/06/30 dcg added begin
//OSP_CLASSLOG GLBLOG接口的头结构
#pragma pack(1)
typedef struct
{
	u32 dwLogType;//默认值为OSP_LOG_FILE_NONE;
}TOspUniformLogHead;

#define OSP_LOG_INTERFACE_SIMPLE   1 //原来的记录方式 记录接口OspPrint OspLog等
#define OSP_LOG_INTERFACE_UNIFORM 2 //增强型的log记录方式 记录接口为OSP_CLASSLOG OSP_LOG
typedef struct
{
    u8 chOspInterfaceType;
    s8 achData[1];
}TOspLogCommonHead;
//2015/06/30 dcg added end<=
#pragma pack()

//管理日志缓冲队列 输出日志到屏幕或者文件
class COspLog
{
private:
	//任务号
	u32 m_dwTaskID;

	//任务句柄 目前m_hTask和m_dwTaskID相等
	TASKHANDLE m_hTask;

	//邮箱读句柄
    MAILBOXID m_dwReadQueHandle;
	//邮箱写句柄
    MAILBOXID m_dwWriteQueHandle;

	//是否允许写日志文件 默认不允许
	BOOL32 m_bLogFileEnable;

	//logtask退出消息是否发出
	BOOL32 m_bIsLogTaskMsgSnd;

	//邮箱最大消息容量
	u32 m_dwMaxMsgWaiting;

    //错误日志保存的文件对象
    COspXLogFile m_cErrLogFile;

    //运行时日志保存的文件对象
    COspXLogFile m_cRunLogFile;

    //此日志对象为了兼容旧的机制(OspLog, OspPrintf等) 这些接口的日志保存到该对象
    //建议今后统一使用OSP_CLASSLOG GLOG接口
    //CXLogFile m_cSimpleLogFile;

public:
	//全局屏幕打印级别  osplog()中使用 级别值小于等于该值的日志在屏幕上显示
	u8 m_byLogScreenLevel;

	//全局日志打印级别 osplog()中使用 级别值小于等于该值的日志记录日志文件
    u8 m_byLogFileLevel;

	//屏幕打印开关 控制日志是否在屏幕上输出
	BOOL32 m_bScrnLogEnbl;

	//长消息打印开关
	BOOL32 m_bLMsgDumpEnbl;

	//接收的日志消息总数
	u32 m_dwMsgIncome;

	//已处理的日志消息总数
	u32 m_dwMsgProcessed;

	//因邮箱满丢弃的日志消息总数
	u32 m_dwMsgDropped;

	//屏幕日志累积总数
	u32 m_dwScreenLogNum;

	//运行日志总次数
	u32 m_dwRunFileLogNum;

	//错误日志总次数
	u32 m_dwErrorFileLogNum;

	//日志文件读写信号量
	//SEMHANDLE m_tLogSem;

public:
	COspLog(void);

	//日志任务初始化
	BOOL32 Initialize(void);

	//日志任务退出
	void Quit(void);

	//发送日志到日志队列
    void LogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen);

    //发送日志到日志队列 供_OspWriteUniformLogFile()和LogQueWrite()调用
    void LogQueWriteFinal(TOspLogCommonHead *ptOspLogMsg);

	//从日志队列取出日志，打印到屏幕或者保存到文件
    void LogQueOut(void);

    //设置日志文件的目录，文件大小及最大的日志文件个数
    BOOL32 OspSetLogFileParam(const char* szFileName, const char* pchDir, u32 dwLogSizeMax_kb,
                            u32 dwLogNumMax, u32 dwLogType);

	//启用日志文件，允许写日志文件
	void LogFileOpen();

	//停用日志文件，不允许写日志文件
	void LogFileClose();
private:

    //从日志队列中取出，打印，对应OspPrintf系列接口
    u32 LogQueOutSimpleFinal(TOspLogHead *ptOspLogHead);

    //从日志队列中取出，打印，对应OSP_CLASSLOG GLOG接口的最终输出
    void LogQueOutUniformFinal(TOspUniformLogHead *ptOspLogHead);

public:
	//关闭屏幕打印
	void StopScrnLog(void)
	{
		m_bScrnLogEnbl = FALSE;
	}
	//恢复屏幕打印
	void ResumeScrnLog(void)
	{
		m_bScrnLogEnbl = TRUE;
	}
	//打印日志系统信息
	void Show(void);
};

//日志模块描述的最大长度
#define MAX_LOG_MODULE_NAME_LENGTH 10

//日志级别描述的最大长度
#define MAX_LOG_LEVEL_NAME_LENGTH  20

//模块的日志级别设置
struct TLogModLevelDesc
{
	u8 dwLogLevel;//日志级别
	char achModuleName[MAX_LOG_MODULE_NAME_LENGTH + 1];
};

//日志级别字符描述
struct TLogLevelDesc
{
	char achLogLevelName[MAX_LOG_LEVEL_NAME_LENGTH + 1];
};

//以模块为关键字 保存模块的日志级别和模块名称
typedef CXMap<u32, TLogModLevelDesc> CModuleLogInfo;

//以日志级别为关键字 保存日志级别的描述
typedef CXMap<u32, TLogLevelDesc> COspLogLevDesc;

static void replace_str(char* pchBody, char chSrc, char chDst);

static BOOL32 DelFile(const char * szFileName);

static u32 _OspGetTidStr(u32 dwBufferLen, s8* pchBuffer);


u32 GetProcessName(u32 dwBufferLen, char* pchBuffer);

API void OspWriteUniformLogFile(TOspUniformLogHead tLogData,
                                 const char * szContent, u32 dwLen);

#endif //__OSP_LOG_PRIVATE_H__
