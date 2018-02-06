#ifndef _OspTest_h
#define _OspTest_h

#include "osp.h"
//#include "kdvsys.h"

/**************************************************************
 标准测试接口：用于向OSP各测试用例发送各种测试命令
***************************************************************/
// 测试代理服务器/客户端端口号
#define OSP_AGENT_SERVER_PORT                      20000
#define OSP_AGENT_CLIENT_PORT                      20001

// OSP测试代理类型
#define OSP_AGENT_TYPE_SERVER                      0
#define OSP_AGENT_TYPE_CLIENT                      1

// OSP请求类型
#define OSP_REQ_TYPE_SERVER                        0
#define OSP_REQ_TYPE_CLIENT                        1

// 测试类型定义
#define OSP_TEST_TYPE_BASE                         0 
#define OSP_TEST_TYPE_SERIAL                       (OSP_TEST_TYPE_BASE + 0)     // 串口测试
#define OSP_TEST_TYPE_NET                          (OSP_TEST_TYPE_BASE + 1)     // 网口测试
#define OSP_TEST_TYPE_TASK                         (OSP_TEST_TYPE_BASE + 2)
#define OSP_TEST_TYPE_SEMA                         (OSP_TEST_TYPE_BASE + 3)
#define OSP_TEST_TYPE_MSGQ                         (OSP_TEST_TYPE_BASE + 4)
#define OSP_TEST_TYPE_NODE                         (OSP_TEST_TYPE_BASE + 5)
#define OSP_TEST_TYPE_APP                          (OSP_TEST_TYPE_BASE + 6)
#define OSP_TEST_TYPE_TIMER                        (OSP_TEST_TYPE_BASE + 7)
#define OSP_TEST_TYPE_COMM                         (OSP_TEST_TYPE_BASE + 8)
#define OSP_TEST_TYPE_LOG                          (OSP_TEST_TYPE_BASE + 9)
#define OSP_TEST_TYPE_END                          (OSP_TEST_TYPE_BASE + 255)

// OSP测试消息, 内部使用
OSPEVENT(OSP_TEST_REQ,      0x10);		// 测试请求
OSPEVENT(OSP_TEST_REQ_ACK,  0x11);      // 测试应答
OSPEVENT(OSP_TEST_CMD,      0x12);		// 测试命令
OSPEVENT(OSP_TEST_QRY,      0x13);      // 状态查询
OSPEVENT(OSP_COMM_TEST,     0x14);      // 通信测试消息
OSPEVENT(OSP_COMM_TEST_ACK, 0x15);      // 测试应答
OSPEVENT(OSP_COMM_TEST_END, 0x16);      // 测试结束
OSPEVENT(OSP_COMM_TEST_RAWDATA, 0x17);
// OSP测试请求结构
#define MAX_TEST_REQ_PARA_LEN     100
typedef struct
{
	u32 type; // 请求类型
	u8 para[MAX_TEST_REQ_PARA_LEN]; // 参数
}TOspTestReq;

// OSP测试命令结构
#define MAX_TEST_CMD_PARA_LEN     5500
typedef struct
{
	u32 type; // 命令类型
	u8 para[MAX_TEST_CMD_PARA_LEN]; // 参数
}TOspTestCmd;

/* 通信测试参数 */
const u16 OSP_TEST_SERVER_APP_ID  =  120;
const u16 OSP_TEST_CLIENT_APP_ID  =  121;

// 通信测试请求参数结构
typedef struct
{
	u32 agtType;    // 目标APP类型: 服务器APP还是客户端APP
	u32 reqType;    // 请求的是服务器实例还是客户端实例
}TOspCommTestReq;

// 测试请求应答: server -> client
#define MAX_ALIAS_LENGTH 20
typedef struct
{
	u32 iid;    // 对端实例ID
	u8 aliasLen; // 对端别名长度
	char achAlias[MAX_ALIAS_LENGTH]; // 对端别名	
}TOspTestReqAck;

/* 通信测试命令参数: 发送参数 */

// 发送函数类型定义
enum OspCommFunc
{
	NoAliasGPost,  // 使用实例ID的全局post函数
	AliasGPost,    // 使用别名的全局post函数
	NoAliasGSend,  // 使用实例ID的全局send函数
	AliasGSend,    // 使用别名的全局send函数
	NoAliasPost,   // 使用实例ID的实例post函数
	AliasPost,     // 使用别名的实例post函数
	NoAliasSend,   // 使用实例ID的实例send函数
	AliasSend      // 使用别名的实例send函数
};

#define COMM_TEST_TYPE_SINGLEINS        // ins内通信
#define COMM_TEST_TYPE_SINGLEAPP        // app内通信
#define COMM_TEST_TYPE_SINGLENODE       // node内通信
#define COMM_TEST_TYPE_MULTINODE        // node间通信

// 用户使用OspTestCmd函数时需要传入的参数
typedef struct
{
	u32 reqType;   // 类型--是发给客户实例, 还是服务实例的命令

/* 发送参数 */
	u32 ip;         // 对端ip地址
	u16 port;       // 对端端口号
	
	u8  funcType;   // 发送函数类别
	u32 times;      // 发送次数
	u16 period;     // 发送周期(ms)
	u16 packets;    // 每次连续发包数
    u16 minLen;     // 最小包长
	u16 maxLen;     // 最大包长

/* 接收参数 */
	BOOL bChkLenErr; // 检测包长错?
	BOOL bChkConErr; // 检测内容错?

/* 原始测试数据 */
	u16 rawDataLen; // 原始测试数据长度
}TOspCommTestCmd;

// OspTestCmd实现时实际使用的结构
typedef struct
{
/* 发送参数 */
	u32 peeriid;    // 对端实例ID
	u8 aliasLen; // 对端别名长度
	char achAlias[MAX_ALIAS_LENGTH]; // 对端别名
    TOspCommTestCmd tOspCommTestCmd;
}TOspCommTestCmdEx;

/* 通信状态 */
typedef struct
{
	u32 sendTimes;
	u32 sendPackets;   // 发包数
	u32 succSendPackets; // 成功发包数
	u32 sendTimeouts; // 发送超时数
	u32 sendBytes; // 发送Byte数
	u32 totalMS; // 发包总耗时(ms)
}TOspSendStat;    

typedef struct
{
	u32 recvPackets;    // 收包数
	u32 recvLenError;   // 长度错
	u32 recvContError;  // 内容错
	u32 recvBytes; // 正确接收Byte数, 包括长度错
	u32 totalMS; // 接收总耗时(ms)
}TOspRecvStat;

typedef struct
{
	TOspSendStat tOspSendStat;  // 发送统计
    TOspRecvStat tOspRecvStat;  // 接收统计
}TOspCommStat;

typedef struct
{
	u32 agtType;
	TOspCommStat tServerStat;
    TOspCommStat tClientStat;
}TOspCommTestResult;

typedef struct
{
}TLogSta;

typedef struct
{
}TSerSta;

#define SERIAL_TEST_SUCCESS          1
#define SERIAL_TEST_FAIL             0

enum SerCmdType
{
	SERIAL_INIT_TEST,
	SERIAL_CLOSE_TEST,
	SERIAL_READ_TEST,
	SERIAL_WRITE_TEST,
	SERIAL_ALL_TEST
};

#define MAX_SERIAL_DATA           100
// 串口测试命令参数结构
typedef struct
{
	u8 type;
	u8 port;
	u16 baudRate;
	u32 bytes2rw;
	char buf2rw[MAX_SERIAL_DATA];
}TSerTestCmd;

// 串口状态数据结构, 作为查询命令的结果参数返回
typedef struct
{
	u8 port;     // 当前操作的端口
	BOOL opened;  // 是否成功打开
	BOOL readSuc; // 当前读操作是否成功
	BOOL writeSuc; // 当前写操作是否成功
	u16 baudRate; // 波特率	
	u32 bytesWrite; // 写出的u8数
    u32 bytesRead; // 读入的u8数
	char readBuf[MAX_SERIAL_DATA]; // 读到的数据    
}TSerialVar;

// 定时器测试有关结构
const u8 TIMER_TEST_TYPE_ABS  = 0;
const u8 TIMER_TEST_TYPE_REL  = 1;
const u8 TIMER_TEST_TYPE_KILL = 2;
const u8 TIMER_TEST_TYPE_MULTI = 3;
typedef struct
{
	u32 times;              // 测试次数
	u8 type;                // 0--绝对定时测试， 1--相对定时测试,  2--定时杀死测试,  3--多定时器测试
	u32 intval;             // 时间间隔(ms)
	/*绝对定时测试用*/
	u16 year; 
	u16 month;
	u16 day;
	u16 hour;
	u16 min;
	u16 sec;
}TOspTimerTestCmd;

typedef struct
{
	u32 timeouts;
	u32 totalMs;
}TOspTimerStat;

typedef TOspTimerStat TOspTimerTestResult;

// 日志测试有关结构
const u8 LOG_FUNC_TYPE_GLOB = 0;
const u8 LOG_FUNC_TYPE_INS = 1;

const u8 LOG_TYPE_FILE = 0;
const u8 LOG_TYPE_SCRN = 1;

#define MAX_LOGMSG_LEN    2000
typedef struct
{
	u8 logType;             // 0 -- 文件; 1 -- 屏幕
	u8 logFileNum;
	u32 logFileSize;
	u8 funcType;
    u32 logNum;
	u8 logCtrlLevl;
	u8 logOutLevl;
	u32 rawDataLen;
	char rawData[MAX_LOGMSG_LEN];
}TOspLogTestCmd;

typedef struct
{
	BOOL bLogNumIncreased;
	BOOL bLogOutputInFile;
}TOspLogTestResult;

typedef struct
{
	u32 fileLogs;
	u32 fileLogSucs;
	u8 logFiles;
	u32 curLogFileNo;

	u32 scrnLogs;
	u32 scrnLogSucs;
}TOspLogStat;

/*====================================================================
函数名：OspTestBuild
功能：为指定的待测功能建立测试环境
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的结点号, 
              (in) type -- 待建立的环境类别
			  (in) param -- 待测功能所需参数

返回值说明：成功返回0，失败返回OSP_ERROR
====================================================================*/
API int OspTestBuild(u32 node, u32 type, void *param);

/*====================================================================
函数名：OspTestCmd
功能：发送测试命令以激发一个测试
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的结点号, 
              (in) type -- 命令类别
			  (in) param -- 命令参数

返回值说明：成功返回OSP_OK，失败返回OSP_ERROR
====================================================================*/
API int OspTestCmd(u32 node, u32 type, void *param);

/*====================================================================
函数名：OspTestQuery
功能：查询测试结果
算法实现：（可选项）
引用全局变量：
输入参数说明：(in) node -- 测试目标(Target)的结点号, 
              (in) type -- 待查询功能类别
			  (in, out) param -- 存放查询结果的buf


返回值说明：无
====================================================================*/
API int OspTestQuery(u32 node, u32 type, void *param);

#endif
