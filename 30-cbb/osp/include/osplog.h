/******************************************************************************
模块名	： OSP
文件名	： OSPLOG.h
相关文件：
文件实现功能：OSP 日志,跟踪功能的主要包含头文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98	 1.0        某某        ------------
2015/06/30   2.0         邓昌葛      增加OSP_CLASSLOG GLBLOG日志记录接口
******************************************************************************/
#ifndef OSP_LOG_H
#define OSP_LOG_H

#include <string>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "osp.h"
#include "ospteleserver.h"

//全路径名最大长度
#define MAX_FILE_DIR_NAME_LEN    (u32)200

//日志文件名的最大长度
#define MAX_FILENAME_LEN         (u32)200

//每条日志的最大长度
#define MAX_LOG_MSG_LEN          (u32)6000

//最大事件数
#define MAX_EVENT_COUNT          (u16)0xffff

//日志类型：可屏蔽日志
#define LOG_TYPE_MASKABLE        0

//日志类型：不可屏蔽日志
#define LOG_TYPE_UNMASKABLE      1

#ifdef _LINUX_
#define SNPRINTF snprintf
#else
#define SNPRINTF _snprintf
#endif


//日志结构 2015/06/30
//能够控制输出指定模块的指定字段
//日志是否保存到日志，取决于g_byLogFileLev的设置，日志level值小于等于g_byLogFileLev的，都会保存到文件
//日志是否在屏幕上显示
//    对于指定了模块的日志，取决于g_cModuleLogLev[byModule].dwLogLev[], 日志level小于等于的，输出到屏幕
//    对于未指定模块的日志，取决于g_byGlbLogPrintLev, 日志level小于等于的，输出到屏幕
struct TOspLogContent
{
public:
    enum
    {
        MAX_BODY_FIELD_LEN = MAX_LOG_MSG_LEN,
        MAX_MOD_LEVEL_STR_LEN = 25,
        MAX_PRIFIXED_FIELD_LEN = 100,
        MAX_COMPILE_FIELD_LEN  = 100,
    };
public:
    TOspLogContent();
    ~TOspLogContent();

    s8 m_achBodyField[MAX_BODY_FIELD_LEN];       //日志正文
    s8 m_achModLev[MAX_MOD_LEVEL_STR_LEN];          //日志模块级别 通过该参数控制模块的日志是否在屏幕输出
    s8 m_achOspPrifixField[MAX_PRIFIXED_FIELD_LEN];  //osp前缀(线程名称-app-inst-task-state)
    s8 m_achCompileField[MAX_COMPILE_FIELD_LEN];    //编译信息
    u8     m_byLogLev;           //日志级别
    BOOL32 m_bIsPrintScreen;     //是否屏幕输出
};

#pragma pack(1)
//日志打印信息头
typedef struct
{
	//类型(控制屏幕显示是否可屏蔽) LOG_TYPE_UNMASKABLE/LOG_TYPE_MASKABLE
	u8 type;
	//打印到屏幕
    BOOL32 bToScreen;
	//打印到文件
    BOOL32 bToFile;
	//打印信息长度
    u32 dwLength;
}TOspLogHead;
#pragma pack()


//事件描述
class COspEventDesc
{
public:
    //事件描述
    char * EventDesc[MAX_EVENT_COUNT];
    //加入事件描述
    void DescAdd(const char * szDesc, u16 wEvent);
    //查询事件描述
    char * DescGet(u16 wEvent);
    //清除事件描述
    void Destroy();
    //初始化事件描述
    void COspEventInit();
    COspEventDesc();
    ~COspEventDesc();
};

//应用描述
class COspAppDesc
{
public:
	//应用描述
    char * AppDesc[MAX_APP_NUM];
	//加入应用描述
    void DescAdd(const char * szDesc, u16 wAppID);
	//清除应用描述
	void Destroy(void);
    COspAppDesc();
	~COspAppDesc();
};

//日志系统初始化
API BOOL32 LogSysInit(void);

//日志系统线程函数
API void LogTask();

//日志输出
API void OspLogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen);

//日志打印
API void OspLog (u8 byLevel, const char * szFormat, ...);
API void OspDumpPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...);
API void OspTrcPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...);
API void OspMsgTrace(BOOL32 bScreen, BOOL32 bFile, const char * szContent, u32 dwLen);
API void OspUniformPrintf(TOspLogContent& cLogContent);

//向Telnet客户端屏幕回显用户命令.
API void OspLogTeleCmdEcho(const char *pchCmdStr, u32 wLen);

//超过十行的消息内容是否输出 TRUE 输出  FALSE 不输出
API inline BOOL32 IsOspLogLongMsgPrintEnbl(void);

//查询文件日志信息累积总数
API u32 OspFileLogNum(void);

//查询屏幕日志信息累积总数
API u32 OspScrnLogNum(void);

//查询当前日志文件号
API u32 OspLogFileNo(void);

#endif // OSP_LOG_H
