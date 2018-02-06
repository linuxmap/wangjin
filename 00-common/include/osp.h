#ifndef _OSP_H
#define _OSP_H

///////////////////////////////////////////////////////////////////
//    Linux 操作系统头文件
///////////////////////////////////////////////////////////////////
#ifdef _LINUX_

    #ifdef PWLIB_SUPPORT
        #include <ptlib.h>
    #endif

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>
    #include <stdarg.h>
    #ifndef _IOS_
    #include <malloc.h>
    #endif
    #include <errno.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <sys/types.h>
    #include "sys/wait.h"
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/times.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <termios.h>
    #include <signal.h>
    #ifdef __cplusplus
		#ifndef _OSP_NOT_INCLUDE_NEW_	//<new>在ubuntu下会引入stl的min/max定义（尤其是3个参数的形式），很容易跟其他地方的定义冲突
		#include <new>
		#endif
    #endif
    #include "dirent.h"
#include "syslog.h"

    #ifndef _EQUATOR_
    #if !defined (_ANDROID_) && !defined(_ARM_) && !defined (_IOS_)
        #include <mqueue.h>
    #endif
    #ifndef _IOS_
        #include <sys/epoll.h>
    #endif  //_IOS_
        #include <sys/resource.h>
    #endif  //_EQUATOR_

#endif //_LINUX_

///////////////////////////////////////////////////////////////////
//    Vxworks 操作系统头文件
///////////////////////////////////////////////////////////////////
#ifdef _VXWORKS_

    #include <vxworks.h>
    #include <tasklib.h>
    #include <msgqlib.h>
    #include <syslib.h>
    #include <memLib.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sockLib.h>
    #include <inetLib.h>
    #include <stdioLib.h>
    #include <strLib.h>
    #include <hostLib.h>
    #include <ioLib.h>
    #include <assert.h>
    #include <netdb.h>
    #include <errno.h>
    #include <sys/stat.h>
    #include <taskHookLib.h>
    #include <logLib.h>
    //串口支持
    #include "brddrvlib.h"

#endif // _VXWORKS_

///////////////////////////////////////////////////////////////////
//    Win32 操作系统头文件
///////////////////////////////////////////////////////////////////
#ifdef _MSC_VER

//     #ifdef _WIN32_WINNT
//         #undef _WIN32_WINNT
//     #endif
//     #define _WIN32_WINNT 0x0400

//     #ifdef WINVER
//         #undef WINVER
//     #endif
//     #define WINVER 0x0400

#include <malloc.h>
#ifdef __cplusplus
	#ifndef _OSP_NOT_INCLUDE_NEW_	//<new>在ubuntu下会引入stl的min/max定义（尤其是3个参数的形式），很容易跟其他地方的定义冲突
	#include <new>
	#endif
#endif

//     #ifndef _WINDOWS_
//         #include <afx.h>
//         #include <afxwin.h>
//         #include <winsock2.h>
//     #else
     #ifndef WIN32_LEAN_AND_MEAN
     #define WIN32_LEAN_AND_MEAN
     #endif
	 #pragma pack(8)//防止64位asf库select返回-1(10038)错误
        #include <winsock2.h>
	 #pragma pack()
        #include <windows.h>

//   #endif
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <limits.h>
    #include <time.h>
    #include <stddef.h>
    #include <assert.h>
    #include <stdarg.h>
    #ifndef _AFX_NO_DEBUG_CRT
    #ifndef _INC_CRTDBG
    #include <crtdbg.h>
    #endif
    #endif // _AFX_NO_DEBUG_CRT
    #pragma comment(lib,"Ws2_32.lib")

#endif    // _MSC_VER

///////////////////////////////////////////////////////////////////
//    公共系统头文件（类型定义）
///////////////////////////////////////////////////////////////////
#include "kdvtype.h"

///////////////////////////////////////////////////////////////////
//    操作系统相关数据类型定义
///////////////////////////////////////////////////////////////////
#ifdef _LINUX_

    typedef s32                   BOOL;
    #define TRUE                  1
    #define FALSE                 0

    typedef void *(*LINUXFUNC)(void *);

    #define SEMHANDLE             pthread_cond_t*
    #define TASKHANDLE            pthread_t
    #define TASKID                pthread_t     //由于IOS中pthread_t是一个指针，因此单独定义一个ttaskid,而非强转成u32
	#ifdef __cplusplus
    #define OSP_NEW               new(std::nothrow)
	#endif//OSP_NEW
#endif    // _LINUX_

#ifdef _VXWORKS_

    #define SEMHANDLE             SEM_ID
    #define TASKHANDLE            u32
    #define TASKID              u32
    #define  OSP_NEW             new
#endif    // _VXWORKS_

#ifndef _MSC_VER

//    typedef unsigned long long    u64;
    #define SOCKHANDLE            int
    #define SERIALHANDLE          int
	#define MAILBOXID             u32

    #ifndef SOCKADDR
        #define SOCKADDR          struct sockaddr
    #endif
    #define SOCKADDR_IN           struct sockaddr_in

	#ifndef OSP_NEW
		#define OSP_NEW                  new
	#endif
#endif    // _MSC_VER

#ifdef _MSC_VER

    #define SEMHANDLE             HANDLE
    #define TASKHANDLE            HANDLE
    #define MAILBOXID             HANDLE
    #define SOCKHANDLE            SOCKET
    #define SERIALHANDLE          HANDLE
    #define TASKID                u32
	#define  OSP_NEW               new(std::nothrow)
#endif    // _MSC_VER

///////////////////////////////////////////////////////////////////
//    操作系统相关宏定义
///////////////////////////////////////////////////////////////////
#ifdef _LINUX_

    #define SOCKET_ERROR          (-1)
    #define INVALID_SOCKET        (-1)
    #define INVALID_SERIALHANDLE  (-1)

    #define OSP_NO_WAIT           0
    #define OSP_WAIT_FOREVER      (-1)

    //SockShutdown参数
    #define STOP_RECV             0
    #define STOP_SEND             1
    #define STOP_BOTH             2

#endif // _LINUX_

#ifdef _VXWORKS_

    #define SOCKET_ERROR          ERROR
    #define INVALID_SOCKET        ERROR
    #define INVALID_SERIALHANDLE  ERROR

    #define OSP_NO_WAIT           NO_WAIT
    #define OSP_WAIT_FOREVER      WAIT_FOREVER

#endif    // _VXWORKS_

#ifdef _MSC_VER

    #define INVALID_SERIALHANDLE  INVALID_HANDLE_VALUE
    #define TOspCommTimeouts      COMMTIMEOUTS
    #define TOspDCB               DCB
    #define TOspComStat           COMSTAT

    #define OSP_NO_WAIT           ((int)0)
    #define OSP_WAIT_FOREVER      ((int)-1)

#endif    // _MSC_VER

///////////////////////////////////////////////////////////////////
//    操作系统无关宏定义
///////////////////////////////////////////////////////////////////
// for Microsoft c++
#ifdef _MSC_VER

    //紧排列
    #ifndef _EQUATOR_
        #pragma pack(push)
        #pragma pack(1)
    #endif

    #ifdef __cplusplus
    #define API  extern "C" __declspec(dllexport)
    #else
    #define API  __declspec(dllexport)
    #endif
    #define OSP_API    __declspec(dllexport)

// for gcc
#else
    #ifdef __cplusplus
    #define API extern "C"
    #else
    #define API
    #endif
    #define OSP_API

#endif    // _MSC_VER

//OSP版本信息
#define OSPVERSION               "OSP.MSG_COMPRESS.64Bit.20160308"

//OSP版本号
#define OSP_VERSION_ID           0x40

#ifdef __cplusplus
//未使用
const u8  OSPLOCAL               = 0;
//未使用
const u8  TCP_GATEWAY            = 255;
//未使用
const u16 SECONDS                = 1000;

//同一结点最大应用数
const u16 MAX_APP_NUM            = 512;
//同一应用最大实例数
const u16 MAX_INST_NUM           = 65000;
//最大定时器数
const u16 MAX_TIMER_NUM          = 5000;
//最大串口数
const u8  MAX_SERIALPORT_NUM     = 10;
#else
#define   OSPLOCAL               ((u8)0)
//未使用
#define   TCP_GATEWAY            (u8)255)
//未使用
#define   SECONDS                ((u16)1000)

//同一结点最大应用数
#define   MAX_APP_NUM            ((u16)512)
//同一应用最大实例数
#define   MAX_INST_NUM           ((u16)65000)
//最大定时器数
#define   MAX_TIMER_NUM          ((u16)5000)
//最大串口数
#define   MAX_SERIALPORT_NUM     ((u16)10)
#endif //__cplusplus
#define OSP_NODE_MAX_CAPABLE     10240
#define OSP_MSG_MAX_CAPABLE      10240

//无效的外部结点号
#define INVALID_NODE             0
//本地结点号
#define LOCAL_NODE               0

//无效的应用号
#define INVALID_APP              0
//本地全局应用号
#define LOCAL_APP                0

//无效的实例号
#define INVALID_INS              0xfffb
//本地实例号
#define LOCAL_INS                0

//默认TCP断链检测时间间隔(秒)
#define DEFAULT_TCP_HEARTBEAT_TIME        10
//默认TCP断链检测次数(多次检测均失败认为已经断链)
#define DEFAULT_TCP_HEARTBEAT_NUM         3
//默认OSP系统状态打印时间间隔(秒)
#define DEFAULT_STATUS_MSGOUT_TIME        3600
//不进行TCP断链检测
#define UNDETECT_TCP_HEARTBEAT            0

#ifndef MINCHAR
    #define MINCHAR              0x80
#endif

#ifndef MAXCHAR
    #define MAXCHAR              0x7f
#endif

#ifndef MINSHORT
    #define MINSHORT             0x8000
#endif

#ifndef MAXSHORT
    #define MAXSHORT             0x7fff
#endif

#ifndef MINLONG
    #define MINLONG              0x80000000
#endif

#ifndef MAXLONG
    #define MAXLONG              0x7fffffff
#endif

#ifndef MAXBYTE
    #define MAXBYTE              0xff
#endif

#ifndef MAXWORD
    #define MAXWORD              0xffff
#endif

#ifndef MAXDWORD
    #define MAXDWORD             0xffffffff
#endif

#ifndef INFINITE
    #define INFINITE            0xFFFFFFFF
#endif
#ifndef _MSC_VER

//
// Provider capabilities flags.
//

#define PCF_DTRDSR               ((u32)0x0001)
#define PCF_RTSCTS               ((u32)0x0002)
#define PCF_RLSD                 ((u32)0x0004)
#define PCF_PARITY_CHECK         ((u32)0x0008)
#define PCF_XONXOFF              ((u32)0x0010)
#define PCF_SETXCHAR             ((u32)0x0020)
#define PCF_TOTALTIMEOUTS        ((u32)0x0040)
#define PCF_INTTIMEOUTS          ((u32)0x0080)
#define PCF_SPECIALCHARS         ((u32)0x0100)
#define PCF_16BITMODE            ((u32)0x0200)

//
// Comm provider settable parameters.
//

#define SP_PARITY                ((u32)0x0001)
#define SP_BAUD                  ((u32)0x0002)
#define SP_DATABITS              ((u32)0x0004)
#define SP_STOPBITS              ((u32)0x0008)
#define SP_HANDSHAKING           ((u32)0x0010)
#define SP_PARITY_CHECK          ((u32)0x0020)
#define SP_RLSD                  ((u32)0x0040)

//
// Settable baud rates in the provider.
//
#ifdef _VXWORKS_
#define BAUD_110                 (u32)110
#define BAUD_300                 (u32)300
#define BAUD_1200                (u32)1200
#define BAUD_2400                (u32)2400
#define BAUD_4800                (u32)4800
#define BAUD_9600                (u32)9600
#define BAUD_19200               (u32)19200
#define BAUD_38400               (u32)38400
#endif

#if defined(_LINUX_) && !defined(_EQUATOR_)
#define BAUD_300                 (u32)300
#define BAUD_1200                (u32)1200
#define BAUD_2400                (u32)2400
#define BAUD_4800                (u32)4800
#define BAUD_9600                (u32)9600
#define BAUD_19200               (u32)19200
#define BAUD_38400               (u32)38400
#define BAUD_57600               (u32)57600
#define BAUD_115200              (u32)115200
#endif

// Equator上只能设置波特率，其他的都不能设置
#ifdef _EQUATOR_
// 注意：最高速率为9600
#define BAUD_110                 (u16)110
#define BAUD_300                 (u16)300
#define BAUD_600                 (u16)600
#define BAUD_1200                (u16)1200
#define BAUD_2400                (u16)2400
#define BAUD_4800                (u16)4800
#define BAUD_9600                (u16)9600
#endif

//
// Settable Data Bits
//
#if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
#define DATABITS_5               (u8)5
#define DATABITS_6               (u8)6
#define DATABITS_7               (u8)7
#define DATABITS_8               (u8)8
#else
#define DATABITS_5               ((u16)0x0001)
#define DATABITS_6               ((u16)0x0002)
#define DATABITS_7               ((u16)0x0004)
#define DATABITS_8               ((u16)0x0008)
#define DATABITS_16              ((u16)0x0010)
#define DATABITS_16X             ((u16)0x0020)
#endif

//
// Settable Stop and Parity bits.
//
#if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
#define STOPBITS_10              (u8)1
#define STOPBITS_20              (u8)2
#else
#define STOPBITS_10              ((u16)0x0001)
#define STOPBITS_15              ((u16)0x0002)
#define STOPBITS_20              ((u16)0x0004)
#endif

#if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
#define PARITY_NOUSE              (u8)0                            //改宏名，避免和ioctrl中的宏同名
#define PARITY_ODD               (u8)1
#define PARITY_EVEN              (u8)2
#define PARITY_MARK              (u8)3
#define PARITY_SPACE             (u8)4
#else
#define PARITY_NOUSE              ((u16)0x0100)
#define PARITY_ODD               ((u16)0x0200)
#define PARITY_EVEN              ((u16)0x0400)
#define PARITY_MARK              ((u16)0x0800)
#define PARITY_SPACE             ((u16)0x1000)
#endif
//
// DTR Control Flow Values.
//
#define DTR_CONTROL_DISABLE      0x00
#define DTR_CONTROL_ENABLE       0x01
#define DTR_CONTROL_HANDSHAKE    0x02

//
// RTS Control Flow Values
//
#define RTS_CONTROL_DISABLE      0x00
#define RTS_CONTROL_ENABLE       0x01
#define RTS_CONTROL_HANDSHAKE    0x02
#define RTS_CONTROL_TOGGLE       0x03

#define NOPARITY                 0
#define ODDPARITY                1
#define EVENPARITY               2
#define MARKPARITY               3
#define SPACEPARITY              4

#define ONESTOPBIT               0
#define ONE5STOPBITS             1
#define TWOSTOPBITS              2

//
// Escape Functions
//

#define SETXOFF                  1        // Simulate XOFF received
#define SETXON                   2        // Simulate XON received
#define SETRTS                   3        // Set RTS high
#define CLRRTS                   4        // Set RTS low
#define SETDTR                   5        // Set DTR high
#define CLRDTR                   6        // Set DTR low
#define RESETDEV                 7        // Reset device if possible
#define SETBREAK                 8        // Set the device break line.
#define CLRBREAK                 9        // Clear the device break line.

//
// PURGE function flags.
//
#define PURGE_TXABORT            0x0001    // Kill the pending/current writes to the comm port.
#define PURGE_RXABORT            0x0002    // Kill the pending/current reads to the comm port.
#define PURGE_TXCLEAR            0x0004    // Kill the transmit queue if there.
#define PURGE_RXCLEAR            0x0008    // Kill the typeahead buffer if there.

typedef struct
{
    u32 ReadIntervalTimeout;
    u32 ReadTotalTimeoutMultiplier;
    u32 ReadTotalTimeoutConstant;
    u32 WriteTotalTimeoutMultiplier;
    u32 WriteTotalTimeoutConstant;
} TOspCommTimeouts;

typedef struct
{
    u32 DCBlength;              // sizeof(DCB)
    u32 BaudRate;               // current baud rate
    u32 fBinary: 1;             // binary mode, no EOF check
    u32 fParity: 1;             // enable parity checking
    u32 fOutxCtsFlow:1;         // CTS output flow control
    u32 fOutxDsrFlow:1;         // DSR output flow control
    u32 fDtrControl:2;          // DTR flow control type
    u32 fDsrSensitivity:1;      // DSR sensitivity
    u32 fTXContinueOnXoff:1;    // XOFF continues Tx
    u32 fOutX: 1;               // XON/XOFF out flow control
    u32 fInX: 1;                // XON/XOFF in flow control
    u32 fErrorChar: 1;          // enable error replacement
    u32 fNull: 1;               // enable null stripping
    u32 fRtsControl:2;          // RTS flow control
    u32 fAbortOnError:1;        // abort on error
    u32 fDummy2:17;             // reserved
    u32 wReserved;              // not currently used
    u16 XonLim;                 // transmit XON threshold
    u16 XoffLim;                // transmit XOFF threshold
    u8 ByteSize;                // number of bits/Byte, 4-8
    u8 Parity;                  // 0-4=no, odd, even, mark, space
    u8 StopBits;                // 0, 1, 2 = 1, 1.5, 2
    s8 XonChar;                 // Tx and Rx XON character
    s8 XoffChar;                // Tx and Rx XOFF character
    s8 ErrorChar;               // error replacement character
    s8 EofChar;                 // end of input character
    s8 EvtChar;                 // received event character
    u16  wReserved1;            // reserved; do not use
}TOspDCB;

typedef struct
{
    u32 fCtsHold : 1;
    u32 fDsrHold : 1;
    u32 fRlsdHold : 1;
    u32 fXoffHold : 1;
    u32 fXoffSent : 1;
    u32 fEof : 1;
    u32 fTxim : 1;
    u32 fReserved : 25;
    u32 cbInQue;
    u32 cbOutQue;
} TOspComStat;

#endif //#ifndef _MSC_VER

#ifdef __cplusplus
// 根据结点号，APP号和本地实例号构造全局实例ID
inline u32 MAKEIID(u16 wAppID, u16 wInsID = 0, u8 = 0) // u8 byUnused = 0
{
    return (wAppID << 16) + wInsID;
}

// 从ID中获得应用号
inline u16 GETAPP(u32 dwGlobIID)
{
    return (u16)(dwGlobIID >> 16);
}

// 从ID中获得网关号
inline u8 GETGATE(u32 = 0)// u32 dwID = 0
{
    return 0;
}

// 从ID中获得实例号
inline u16 GETINS(u32 dwGlobIID)
{
    return (u16)(dwGlobIID);
}

// 从ID中获得实例号的低字节
inline u8 GETINSL(u32 dwGlobIID)
{
    return (u8)(dwGlobIID);
}

// 从ID中获得实例号的高字节
inline u8 GETINSH(u32 dwGlobIID)
{
    return (u8)(dwGlobIID >> 8);
}
#endif //__cplusplus
//telnet终端提示符最大长度
#define MAX_PROMPT_LEN                21

// OSP错误码
#define OSP_ERROR_BASE                0
//操作成功
#define OSP_OK                        (OSP_ERROR_BASE + 0)
//操作失败
#define OSP_ERROR                     (OSP_ERROR_BASE + 1)
//未使用
#define OSPERR_SEND_FAIL              (OSP_ERROR_BASE + 2)
//发送同步消息超时
#define OSPERR_SEND_TIMEOUT           (OSP_ERROR_BASE + 3)
//同步消息应答超长
#define OSPERR_SYNCACK_EXCEED         (OSP_ERROR_BASE + 4)
//同一应用下实例别名已经存在
#define OSPERR_ALIAS_REPEAT           (OSP_ERROR_BASE + 5)
//未使用
#define OSPERR_APP_REPEAT             (OSP_ERROR_BASE + 6)
//外部结点号无效
#define OSPERR_NODE_INVALID           (OSP_ERROR_BASE + 7)
//参数无效
#define OSPERR_ILLEGAL_PARAM          (OSP_ERROR_BASE + 8)
//同步消息对端无应答
#define OSPERR_PEER_NOT_RESPOND       (OSP_ERROR_BASE + 9)

//最大消息长度
#define MAX_MSG_LEN                   (u16)0xC000 //48K
//实例别名最大长度
#define MAX_ALIAS_LEN                 (u8)255
//最大同步消息应答长度
#define MAX_SYNCACK_LEN               MAX_MSG_LEN
//最大数据长度(未使用)
#define MAX_DATA_LEN                  (MAX_MSG_LEN+MAX_ALIAS_LEN+MAX_SYNCACK_LEN)

//OSP消息类型
// 异步消息
#define MSG_TYPE_ASYNC                (u16)0
// 同步消息
#define MSG_TYPE_SYNC                 (u16)1
// 同步应答消息
#define MSG_TYPE_SYNCACK              (u16)2
// 全局同步消息
#define MSG_TYPE_GSYNC                (u16)3
// 全局同步应答消息
#define MSG_TYPE_GSYNCACK             (u16)4
// 超时消息
#define MSG_TYPE_TIMEOUT              (u16)5

//切换实例状态机
#define NEXTSTATE(x)                  NextState(x,#x)

//OSP 用户自定义消息事件号基准(小于该值的消息为OSP保留消息，上层禁止发送该消息)
#define OSP_USEREVENT_BASE            (u16)0x0400
#ifdef __cplusplus
//定义消息事件号
#define OSPEVENT(x, y)                 const u16 x = y
#endif

//未使用
#define  OSP_CONFIRM         ((u16)0xff)
//启动断链检测
#define  OSP_POWERON         ((u16)0x100)
//未使用
#define  OSP_SWITCHMASTER    ((u16)0x101)
//未使用
#define  OSP_SWITCHSLAVE     ((u16)0x102)
//服务端无空闲实例
#define  OSP_OVERFLOW        ((u16)0x103)
//未使用
#define  OSP_EXCEPTION_IND   ((u16)0x104)
//未使用
#define  OSP_INSNOTEXIST      ((u16)0x105)
//与远端结点的TCP链路已断
#define  OSP_DISCONNECT        ((u16)0x106)
//广播消息确认应答
#define  OSP_BROADCASTACK     ((u16)0x107)
//未使用
#define  OSP_NODECLOSE        ((u16)0x108)
//断链检测探测消息
#define  OSP_NETBRAECHO       ((u16)0x109)
//断链检测探测消息确认应答
#define  OSP_NETBRAECHOACK    ((u16)0x110)
//OSP卸载消息
#define  OSP_QUIT             ((u16)0x120)
//结点间通信测试消息
#define  OSP_NETSTATEST       ((u16)0x121)
//结点间通信测试消息确认应答
#define  OSP_NETSTATESTACK    ((u16)0x122)
//未使用
#define  OSP_APPCONN_ACK      ((u16)0x123)
//支持消息压缩
#define  OSP_COMPRESS_SUPPORT ((u16)0x222) //by wubin 2011-4-13
//压缩消息号
#define  OSP_COMPRESS_MSG     ((u16)0x223) //by wubin 2011-4-13
//SIP类型的消息 消息结构COspSipMessage
#define  OSP_SIP_MSG           (OSP_COMPRESS_MSG + 1) // 0x224

// Trc跟踪标志
typedef enum TRCOPTION
{
    // 跟踪状态迁移
    TRCSTATE     = 1,
    // 跟踪事件
    TRCEVENT     = 2,
    // 跟踪定时器
    TRCTIMER     = 4,
}TRCOPTION;

//状态跃迁、事件、定时器均跟踪
#define TRCALL       (u16)(TRCSTATE|TRCEVENT|TRCTIMER)

// 1.1版以后支持同步消息
#if (defined OSP_VERSION_ID) && (OSP_VERSION_ID >= 0x11)
    #define SYNCMSG
#endif

#ifdef _LINUX_
    /**
     * 用于网络传输数据流的类
     * 任何一个结构要在网络上传输，都写入一个数据流对象中，
     * 然后把此数据流对象OSP，让OSP来传输
     * 不要再直接使用传输一个结构
     */
    #ifdef __cplusplus
    class COspStream
    {
    public:
        //当前数据长度
        u32 m_dwCurrSize;
        //当前数据
        u8 m_achData[MAX_MSG_LEN];

        COspStream()
        {
            m_dwCurrSize = 0;
            memset(m_achData, 0, sizeof(m_achData));
        }

        u32 PackElement(const u8* pbyData, u32 dwLen);
        u32 ExtractElement(u8* pbyData, u32 dwLen);
    };

    #define PackElement2(data)    PackElement((u8*)&data, sizeof(data))
    #define ExtractElement2(data)    ExtractElement((u8*)&data, sizeof(data))

    // OSP消息头部结构
    class CMessage
    {
    public:
        //源结点
        u32 srcnode;
        //目的结点
        u32 dstnode;
        //目的应用实例
        u32 dstid;
        //源目的实例
        u32 srcid;
        //消息类型
        u16 type;
        //消息号
        u16 event;
        //消息体长度
        u16 length;
        //消息体
        u8* content;

        //如果支持同步消息
    #ifdef SYNCMSG
        //同步消息应答
        u8* output;
        //同步消息应答长度
        u16 outlen;
        //未使用
        u16 expire;
    #endif

        //实例别名
        char * dstAlias;
        //实例别名长度
        u8 dstAliasLen;

        //得到OSP消息头部长度
        static u32 GetPackedSize()
        {
            return sizeof(u32) + sizeof(u32) + sizeof(u32) + sizeof(u32) + sizeof(u16) +
                    sizeof(u16) + sizeof(u16) + sizeof(u8 *) +
                    #ifdef SYNCMSG
                        sizeof(u8 *) + sizeof(u16) + sizeof(u16) +
                    #endif
                    sizeof(char *) + sizeof(u8);
        };

        //OSP消息头部打包
        void Pack(COspStream& tStream);
        //OSP消息头部解包
        void Extract(COspStream& tStream);
    };
	#endif // __cplusplus
#else
    #ifdef __cplusplus
    // OSP消息头部结构
    typedef struct CMessage
    {
        //源结点
        u32 srcnode;
        //目的结点
        u32 dstnode;
        //目的应用实例
        u32 dstid;
        //源目的实例
        u32 srcid;
        //消息类型
        u16 type;
        //消息号
        u16 event;
        //消息体长度
        u16 length;
        //消息体
        u8 *content;

        //如果支持同步消息
    #ifdef SYNCMSG
        //同步消息应答
        u8* output;
        //同步消息应答长度
        u16 outlen;
        //未使用
        u16 expire;
    #endif

        //实例别名
        char * dstAlias;
        //实例别名长度
        u8 dstAliasLen;
    }
    //紧排列
    #if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
        __attribute__ ((packed))
    #endif
    CMessage;
    #endif // __cplusplus
#endif // linux

//实例简要信息
typedef struct
{
    u16 insid;
    u32 state;
    u16 evtrcv;
    u16 evtsnd;
}TInstInfo;
//记录回调函数执行信息
#define APP_MAX_FUNCTION_NAME 30
#define APP_MAX_CALLBACK_RECORD 10 //记录最近10次的回调执行信息
typedef struct
{
    u32 dwIndex; //在累加调用次数中的序号
    u16 wInstId; //实例id
    u16 wEvent;  //事件id
    u16 wMsgType; //消息类型 异步消息 同步消息等
	u32 dwStartTick; //回调函数执行前的时间
    u32 dwEndTick;    //回调函数执行后的时间
	char achFunctionName[APP_MAX_FUNCTION_NAME]; //函数名
}TCallBackInfo;
//单个应用中可保存的最大实例简要信息个数
#define MAX_INSTINFO_NUM		   60

#ifdef __cplusplus
//应用抽象基类，主要用于定义用户App模板zTemplate
#if defined(PWLIB_SUPPORT) && defined(_LINUX_)
    class CApp: public PThread
#else
    struct OSP_API CApp
#endif
{
#if defined(PWLIB_SUPPORT) && defined(_LINUX_)
    PCLASSINFO(CApp, PThread);
#endif

public:
    //应用接收消息总数
    u32 msgIncome;
    //应用已处理消息总数
    u32 msgProcessed;
    //应用邮箱中待处理的消息总数(未使用)
    u32 msgWaiting;
    //应用邮箱中待处理的消息总数峰值
    u32 msgWaitingTop;
    //已激发的定时消息总数
    u32 timerProcessed;
    //应用邮箱最大消息容量
    u32 maxMsgWaiting;
    //因邮箱满丢弃的消息总数
    u32 msgdropped;
    //应用别名
    char * pAppName;

    //应用主邮箱写句柄
    MAILBOXID queSendId;
    //应用主邮箱读句柄
    MAILBOXID queRcvId;

    /* 为支持同步消息而增加的消息备份队列，存放同步消息发送后，应答超时消息
       或应答消息到达之前的所有消息 */
    //应用备邮箱中的消息总数
    u32 bakQueMsgNum;
    //应用备邮箱写句柄
    MAILBOXID bakQueSendId;
    //应用备邮箱读句柄
    MAILBOXID bakQueRcvId;

    //应用任务ID
    TASKID taskID;

    //屏幕trc标志 收发消息跟踪是否打印到屏幕标志
    u16 scrnTraceFlag;

    //文件trc标志 收发消息跟踪是否保存到日志文件标志
    u16 fileTraceFlag;

    //屏幕log级别 Instance::log()中进行控制
    u8 scrnLogFlag;

    //文件log级别 Instance::log()中进行控制
    u8 fileLogFlag;

    //应用ID
    u16 appId;
    //应用任务优先级
    u8 byAppPri;
    //应用前一个空闲实例号
    u16 wLastIdleInstID;

    //应用任务句柄
    TASKHANDLE hAppTask;
    //应用任务信号量句柄
    SEMHANDLE tSemMutex;

    // 当前收到的消息
    CMessage *curMsgRcv;
    // 当前发送的消息号
    u16 curEvtSnd;

private:
	// 用于记录实例最近状态和收到数据的环行缓冲
	u8 byInstInfoHead;
	u8 byInstInfoTail;
	TInstInfo tInstInfo[MAX_INSTINFO_NUM];

    //跟踪回调函数执行信息
    BOOL32 m_bCallBackInfoRecFlag;//记录回调执行信息开关 TRUE 记录 FALSE 不记录
	u8 m_byCallBackIndex; //环形缓冲的循环索引
    u32 m_dwCallBackCount;//回调函数执行累加的总次数
    //用于记录App最近回调函数调用信息的环行缓冲
	TCallBackInfo m_atCallBackInfo[APP_MAX_CALLBACK_RECORD];
public:

	CApp();

	virtual ~CApp();


    //根据实例号查询指定实例
    virtual class CInstance* GetInstance(u16 wInsid) = 0;
    //设定实例别名
    virtual BOOL32 SetInstAlias(u16 wInsID, const char* szAlias, u8 byLen) = 0;
    //根据别名查询指定实例
    virtual class CInstance* FindInstByAlias(const char* szAlias, u8 byLen) = 0;
    //得到应用最大实例数
    virtual s32 GetInstanceNumber() = 0;
    //得到实例别名最大长度
    virtual u8 GetMaxAliasLen() = 0;
    //初始化实例别名记录信息
    virtual BOOL32 InitAliasArray() = 0;
    //清除实例别名纪录信息
    virtual BOOL32 ClearInstAlias(u16 wInsID, const char* szAlias, u8 byLen) = 0;

    //启动应用
    // APP最高优先级为80，如果小于80，OSP将强制为80。此后, 用户可以调用OspSetAppPriority()或CApp::SetPriority()更改优先级.
    //成功返回OSP_OK      //操作成功    失败返回OSP_ERROR  //操作失败
    s32 CreateApp(const char* szName, u16 wAppId, u8 byPrior, u16 wQueueSize = 80, u32 dwStackSize = 200<<10);
    //设定应用任务优先级
    BOOL32 SetPriority(u8 byPrior);
    //查询任务优先级
    u8 GetPriority()
    {
        return byAppPri;
    }
    //应用退出
    //注意: 用户禁止调用QuitApp
    void QuitApp();

    //应用备邮箱消息总数增加减少查询
    BOOL32 BakMsgNumInc();
    BOOL32 BakMsgNumDec();
    u32 GetBakMsgNum();

    //应用主邮箱消息总数查询增加减少
    u32 GetMsgIncomeNum();
    void MsgIncomeNumInc();
    void MsgIncomeNumDec();

    //应用处理消息总数增加
    void MsgProcessedNumInc();

    //查询待处理的消息总数
    u32 GetMsgWaitingNum();

    //应用别名设定查询
    void SetName(const char* szName);
    char * NameGet();

    //定时激发次数增加
    void TimerProcessedIncrease();

    //设定文件屏幕log级别
    void LogLevelSet(u8 byFileLevel, u8 byScreenLevel);
    //设定文件屏幕trc标志
    void TrcFlagSet(u16 wFileFlag, u16 wScreenFlag);

    //实例简要信息打印
    void InstInfoShow();
    //实例简要信息增加
    void InstInfoAdd(u16 wInsID, u32 dwState, u16 wEvtRcv, u16 wEvtSnd);

	//获取本app的回调函数执行信息记录开关
    BOOL32 GetCallBackInfoRecordFlag(void);
	//设置本app的回调函数执行信息记录开关
    void   SetCallBackInfoRecordFlag(BOOL32 bFlag);

    //回调函数执行信息打印
	void CallBackInfoShow(void);

	//回调函数执行信息添加
	/*=============================================================================
	函数名      :CallBackInfoAddStart
	功能        :统计回调函数的信息，记录起始时间
	             只在AppEntry()中使用，是串行的，不需要锁
	             在执行回调函数之前调用
	算法实现    : 不加锁的原因是，AppEntry()内部时序上可以保证串行
	              Telnet显示线程和AppEntry()可能有冲突，但可以忽略
	参数说明    :
		[I] dwTick 回调函数执行前的时间
		[I] wInstId 执行回调的实例id
		[I] dwEvent 事件id
		[I] wMsgType 消息类型 MSG_TYPE_ASYNC 0 MSG_TYPE_SYNC 1
		    MSG_TYPE_SYNCACK 2  MSG_TYPE_GSYNC 3
		[I] pchFuncName 函数名指针
	返回值说明  :无
	-------------------------------------------------------------------------------
	修改记录    :
	日  期      版本        修改人        修改内容
	2015/05/20  1.0         邓昌葛        新建
	===============================================================================*/
    void CallBackInfoAddStart(u32 dwTick, u16 wInstId, u16 dwEvent,
                              u16 dwMsgType,char *pchFuncName);
	/*=============================================================================
	函数名      :CallBackInfoAddEnd
	功能        :统计回调函数的信息，记录截止时间
	             只在AppEntry()中使用，注意时序上保证和CallBackInfoAddStart一一对应
	             在执行回调函数之后调用
	算法实现    :
	参数说明    :
		[I] dwTick 回调函数执行完成的时刻
	返回值说明  :无
	-------------------------------------------------------------------------------
	修改记录    :
	日  期      版本        修改人        修改内容
	2015/05/20  1.0         邓昌葛        新建
	===============================================================================*/
    void CallBackInfoAddEnd(u32 dwTick);

#if defined(_LINUX_) && defined(PWLIB_SUPPORT)
    void Main();  // pwlib pure virtual function
#endif
};


//实例定时器信息
typedef  struct TInstTimerInfo
{
    TInstTimerInfo* next ;
    u32 timerId ;
    void* timerBlkAdrr ;
}TInstTimerInfo;


//实例状态改变历史记录
#define  MAX_INST_STATE_RECORD_COUNT 10
typedef  struct tagTInstStateChangeInfo
{
    u32 dwIndex;
    u32 dwState;
}TInstStateChangeInfo;

//CInstance抽象基类规范。
//基于Osp的应用必须从该CInstance基类派生一个具体的Instance类，
//并给出一个InstanceEntry()成员函数的具体实现
class OSP_API CInstance
{
public:
    enum
    {
        //空闲实例
        PENDING,
        //广播
        EACH = 0xffff,
        //广播且要求确认应答
        EACH_ACK = 0xfffe,
        //未使用
        CONN_ACK = 0xfffd,
        //守护实例
        DAEMON = 0xfffc,
        //目的实例实例号无效，别名有效
        INVALID = 0xfffb
    };

public:
    //应用号
    u16 m_appId;
    //实例号
    u16 m_instId;
    //实例别名
    s8 m_alias[MAX_ALIAS_LEN];
    //实例别名长度
    u8 m_aliasLen;
    //实例别名最大长度
    u8 m_maxAliasLen;
    //当前状态机
    u32 m_curState;
    //上一个状态机
    u32 m_lastState;

	//状态改变的历史记录 记录最近的10条
	u8 m_chInstStateIndex;//在环形缓冲区中的序号
    u32 m_dwInstStateTimes;//状态迁移次数
    //状态改变的历史记录
    TInstStateChangeInfo m_atInstStateInfo[MAX_INST_STATE_RECORD_COUNT];
public:
    //守护实例入口函数
    virtual void DaemonInstanceEntry(CMessage *const, CApp*) // CMessage *const pcMsg, CApp* pcApp
    {
        log(0, "Osp: message received in default daemon instance of app %d.\nCheck your daemon implementation, please!\n", GetAppID());
    }
    //实例入口函数
    virtual void InstanceEntry(CMessage *const pcMsg) = 0;
    //实例打印函数
    virtual void InstanceDump(u32 = 0) // u32 dwFlag = 0
    {
    }
    //守护实例打印函数
    virtual void DaemonInstanceDump(u32 = 0) // u32 dwReserved = 0
    {
    }
    //实例退出
    virtual void InstanceExit()
    {
    }

    //设定实例别名
    s32 SetAlias(const char* szAlias, u8 byLen);
    //删除实例别名
    void DeleteAlias();
    //查询实例别名
    BOOL32 GetAlias(char * szAlias, u8 byInBufLen, u8* pbyAliasLen);
    //查询实例别名长度
    u8 GetAliasLen();

    //查询上一个实例状态机
    u32 LastState();
    //查询当前实例状态机
    u32 CurState();

    //查询应用号
    u16 GetAppID();
    //查询实例号
    u16 GetInsID();

    //定时器信号量
    SEMHANDLE m_tSemTimerList;
    //定时器信息链表
    TInstTimerInfo m_timerInfoListHead;

    //加入定时器信息
    void AddInstTimerInfo(u32 dwTimerNo, void* pvTimerAddr);
    //删除定时器信息
    void DelInstTimerInfo(u32 dwTimerNo);
    //查询定时器信息
    void *FindInstTimerInfo(u32 dwTimerNo);
    //删除所有定时器信息
    void DelAllInstTimerInfo();

	//打印状态改变的最记录
    void InstStateInfoShow(u16 wInstId);
protected:
    //设置相对定时
    s32 SetTimer(u32 dwTimerNo, long dwMilliSeconds, u32 dwPara = 0);
    //设置绝对定时
    s32 SetAbsTimer(u32 dwTimerNo, u16 wYear, u8 byMon, u8 byDay, u8 byHour, u8 byMin, u8 bySec, u32 dwPara = 0);
    //取消定时
    s32 KillTimer(u32 nTimerNo);
    // 取消绝对定时器
    s32 KillAbsTimer(u32 nTimerNo);
    //实例打印,可调用OspSetLogLevel修改打印级别
    void log(u8 byLevel, const char * szFormat,...);
    //切换状态机
    void NextState(u32 dwState, char * szStateName = NULL);

    //发送异步消息(目的实例采用实例号标识)
    s32 post(u32 dwDstIID, u16 wEvent, const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0);
    //发送异步消息(目的实例采用实例别名标识)
    s32 post(const char* szDstAlias, u8 byAliasLen, u16 wDstAppID, u16 wEvent,
        const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0);

    //发送同步消息(目的实例采用实例号标识)
    s32 send(u32 dwDstIID, u16 wEvent, const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0,
        void* pvAckBuf = NULL, u16 wAckBufLen = 0, u16* pwRealAckLen = NULL,
        u16 wTimeout=1000);
    //发送同步消息(目的实例采用实例别名标识)
    s32 send(const char* szDstAlias, u8 byAliasLen, u16 wDstAppID, u16 wEvent,
        const void* pvContent=0, u16 wLength = 0, u32 dwDstNode = 0,
        void* pvAckBuf = NULL, u16 wAckBufLen = 0, u16* pwRealAckLen = NULL,
        u16 wTimeout=1000);

    //发送同步消息应答
    s32 SetSyncAck(const void* pvAckBuf = NULL, u16 wAckBufLen = 0);
    //发送同步消息应答(建议使用SetSyncAck)
    s32 reply(const void* pvAckBuf = NULL, u16 wAckBufLen = 0);
};

//用于派生用户App类的模板
class CAppNoData
{
};
#endif //#ifdef __cplusplus

//上层应用调用OSPSIP POST接口需要需要携带的身份标识
//该实体介于OSP及OSP协议层之上
#ifdef __cplusplus

#define OSP_INVALID_IID     (0)          //无效IID
#define OSP_INVALID_TASKNO  (0)          //无效事务号
#define OSP_INVALID_SEQ_NO  (0)          //无效序列号

struct TOspEntityId
{
public:
	u32 dwOspIID;    //appid,instid
	u32 dwTaskID;  //task id
	u32 dwSeqNO;  //seq no

	TOspEntityId()
	{
		dwOspIID = OSP_INVALID_IID;
		dwTaskID = OSP_INVALID_TASKNO;
		dwSeqNO = OSP_INVALID_SEQ_NO;
	}

	BOOL32 operator==(TOspEntityId& tOther) const
	{
        //注意，这里不比较SeqNO
		return (tOther.dwOspIID == dwOspIID && tOther.dwTaskID == dwTaskID);
	}
};
#endif//__cplusplus

//实例别名快速查找记录数据
typedef struct TInstAliasInfo
{
    //实例号
    u16  instId;
    //实例别名的设置值(也是存放的位置下标)
    u32  aliasRet;
    //相同设置值的下一个实例存放位置
    u32  nextLoc;
}TInstAliasInfo;

#ifdef __cplusplus
//应用模板(模板参数为实例的继承类以及最大实例实例数)
template <class I, s32 maxins = 1, class A = CAppNoData, u8 maxAliasLen = 0>
class zTemplate: public CApp, public A
{
public:
    //设置实例别名
    BOOL32 SetInstAlias(u16 wInsID, const char* szAlias, u8 byLen);
    //清除实例别名纪录信息
    BOOL32 ClearInstAlias(u16 wInsID, const char* szAlias, u8 byLen);

private:
     //由局部实例号查找实例
    CInstance* GetInstance(u16 wInsID);
    //由别名查找实例
    CInstance* FindInstByAlias(const char* szAlias, u8 byLen);
    //获得普通实例个数
    s32 GetInstanceNumber();
    //获得最大别名长度
    u8 GetMaxAliasLen();

private:
    //daemon实例
    I daemIns;
    //普通实例表
    I insarray[maxins];

    //实例别名快速查找
    //实例别名记录信息链头表
    TInstAliasInfo instAliasArray[maxins];
    //实例别名记录信息备用表，记录设置相同的实例
    TInstAliasInfo bakInstAliasArray[maxins];
    //当前备用表中记录的实例个数
    u32 curBakArrayNum;

    //初始化实例别名记录信息
    BOOL32 InitAliasArray();
    //实例别名转换成WORD32型整数
    u32 AliasToWord(char * szAlias, u8 byLen);
    //确定实例别名信息存放在主用表还是在备用表中
    BOOL32 IsInBakArray(u32 dwAtoW);
};
#endif //__cplusplus

//通用的LOG级别
enum EOspLogLev
{
    OSP_NOLOG_LEV = 0,    //无任何打印
    OSP_ERROR_LEV,        //错误级别
    OSP_WARNING_LEV,      //警告级别
    OSP_CRITICAL_LEV,     //关键级别
    OSP_EVENT_LEV,        //普通事件级别
    OSP_PROGRESS_LEV,     //进度级别
    OSP_FREQ_EVENT_LEV,   //频繁的事件级别 例如SIP事件 OSP事件等
    OSP_TIMER_LEV,        //定时器级别
    OSP_TRIVIAL_LEV,      //极其频繁的级别
    OSP_ALL_LEV,          //所有级别
    OSP_LOG_LEV_NUM,
};

//日志公共库模块，用于日志输出
enum EOspLogModule
{
    MOD_OSP,               //osp
    MOD_OSPEXT,            //ospext
    MOD_OSPPROT,           //ospprot
    MOD_OSPSIP,            //ospsip
    MOD_COMMON_NUM = 20,  //20个公共模块足够

    USER_MOD_START = 21, //上层模块由此开始

    MOD_MAX = 255,  //最大可设置255个模块日志输出
};

//日志文件的类型
enum EOspLogFileType
{
    OSP_LOG_FILE_NONE = 0,      //非法类型
    OSP_LOG_FILE_ERR = 1,       //错误日志
    OSP_LOG_FILE_RUN = 1<<1,    //运行日志
};

#ifdef __cplusplus
//对锁的封装
class COspSemLock
{
public:
    COspSemLock();
    ~COspSemLock();

private:
    COspSemLock(const COspSemLock&);
    void operator=(const COspSemLock&);

public:
    //上锁
    BOOL32 Lock(u32 dwTimeout = INFINITE);

    //解锁
    BOOL32 UnLock();

    //等同于Lock(), 看个人使用习惯
    BOOL32 Take(u32 dwTimeout = INFINITE);

    //等同于UnLock(), 看个人使用习惯
    BOOL32 Give();

private:
    SEMHANDLE m_hSemaphore;
};

//自动锁，简化加锁、解锁调用
class COspAutoLock
{
public:
    //构造函数上锁
    COspAutoLock(COspSemLock& rLock);

    //析构函数解锁
    ~COspAutoLock();
private:
	COspAutoLock& operator=(const COspAutoLock&);
private:
    COspSemLock& m_Lock;
};

//log是否支持输出编译信息
API BOOL32 IsOspPrintCI();
//对于不支持__FUNCTION__的编译器(vc6等) 置为空""
#if defined(_MSC_VER) && !defined(__FUNCTION__)
#define __FUNCTION__ ""
#endif

#define  OSP_FILE_NAME  (IsOspPrintCI() ? __FILE__ : (const s8 *)NULL)

#define  OSP_FILE_LINE  (IsOspPrintCI() ? __LINE__ : -1)

#define  OSP_TYPE_NAME  (IsOspPrintCI() ? (const s8 *)NULL : (const s8 *)NULL)

#define  OSP_FUNC_NAME  (IsOspPrintCI() ? __FUNCTION__ : (const s8 *)NULL)

//输出编译信息(file-line-class-function)的LOG
class COspXLog
{
public:
	//日志接口类 实现用户的日志生成 包括编译信息(file-line-class-function)等
    COspXLog(const s8* szFileName=NULL, s32 nFileLine=-1, const s8* szClassName=NULL, const s8* szFunName=NULL);
	~COspXLog();
    //进行模块控制的打印
    void operator()(u8 byLogLev, const s8* szFormat, ...) const;
    void operator()(u8 byLogMod, u8 byLogLev, const s8* szFormat, ...) const;

protected:
    u32 GetCompileInfo(u32 dwBufferLen, s8* pchBuffer) const;
    static u32 GetFileNameFromPath(const s8* szPath, u32 dwLength, s8* pchBuffer);
    static u32 GetTypeName(const s8* szTypeNameRaw, u32 dwLength,  s8* pchBuffer);

private:
    const s8* m_szFileName;
    u32 m_nFileLine;
    const s8* m_szClassName;
    const s8* m_szFunName;
};

/*=============================================================================
 函 数 名  : OSP_CLASSLOG
 功能描述  : 全局的日志记录接口，在类中使用
             有如下两种调用方法
             OSP_CLASSLOG(u8 byLogLev, const s8* szFormat, ...)
             OSP_CLASSLOG(u8 byLogMod, u8 byLogLev, const s8* szFormat, ...)
 算法实现  :
 参数说明  :
            [I]u8 byMod     日志模块枚举值
            [I]u8 byLevel   日志的设置级别
            [I]const s8* szFormat, ... 日志内容
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
#define OSP_CLASSLOG (COspXLog(OSP_FILE_NAME, OSP_FILE_LINE,\
                               OSP_TYPE_NAME, OSP_FUNC_NAME))

/*=============================================================================
 函 数 名  : OSP_LOG
 功能描述  : 全局的日志记录接口，全局使用或类的静态成员函数中使用，
             有如下两种调用方法
             OSP_LOG(u8 byLogLev, const s8* szFormat, ...)
             OSP_LOG(u8 byLogMod, u8 byLogLev, const s8* szFormat, ...)
 算法实现  :
 参数说明  :
            [I]u8 byMod 日志模块枚举值
            [I]u8 byLevel 日志的设置级别
            [I]const s8* szFormat, ... 日志内容
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
#define OSP_LOG (COspXLog(OSP_FILE_NAME, OSP_FILE_LINE, NULL, OSP_FUNC_NAME))

/*=============================================================================
 函 数 名  : OspSetModuleLogLev
 功能描述  : 设置模块的日志级别和模块名称
 算法实现  :
 参数说明  :
            [I]u8 byMod 日志模块枚举值  OSP已有的日志模块enum EOspLogModule,
               用户根据需要另行定义枚举值，注意不要与OSP已有的日志模块冲突
            [I]u8 byLevel 日志的设置级别 日志级别小于等于设置级别，才会写入日志文件
            [I]const char* szModName 模块描述

 返回值说明: TRUE 成功 FALSE 失败
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
API u32 OspSetModuleLogLev(u8 byMod, u8 byLevel, const char* szModName);

/*=============================================================================
 函 数 名  : OspGetModuleLogLevel
 功能描述  : 通过日志模块序号获取该模块的日志级别
 算法实现  :
 参数说明  :
            [I]u8 byMod     日志模块枚举值
            [I]const char* szModName 模块的描述

 返回值说明: 日志的设置级别 如果没有设置过 获取到的值将是TRIVIAL_LEV
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
API u32 OspGetModuleLogLevel(u8 byMod);

/*=============================================================================
 函 数 名  : OspGetLogStrModule
 功能描述  : 通过日志模块号获取该模块的名称描述
 算法实现  :
 参数说明  :
            [I]u8 byMod 日志模块枚举值
            [I]const char* szModName 模块描述

 返回值说明: 模块的名称描述 如果没有设置过 获取到的值将是"null"
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
API const char* OspGetLogStrModule(u8 byMod);

/*=============================================================================
 函 数 名  : OspGetStrLogLevel
 功能描述  : 通过日志级别获取字符描述
 算法实现  :
 参数说明  :
            [I]u8 byLogLev 日志的设置级别

 返回值说明: 日志级别字符描述 如果没有设置过 获取到的值将是"null"
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    新生成函数
=============================================================================*/
API const char* OspGetStrLogLevel(u8 byLogLev);

/*日志的全局打印级别 level值小于等于该值的日志允许在屏幕上输出
  默认级别为OSP_ERROR_LEV
  注意: 前提是OspResumeScrnLog()设置过，才能在屏幕上输出
*/
API void OspSetGlbLogPrintLevel(u8 byLogLev);

/*设置日志的时间信息打印 0:不打印时间; 1:打印秒级时间; 255:打印豪秒级时间
  默认为打印秒级时间
*/
API void OspSetLogTimeLevel(u8 byLogLev);

/*设置是否打印编译信息(file-line-class-function) 0:不打印编译信息; 1:打印编译信息
  默认不打印
*/
API void OspSetLogCompileInfo(u8 byPrint);

/*设置日志中是否打印前缀信息(app-inst-task-state) 0:不打印OSP前缀; 1:打印OSP前缀
  默认不打印
*/
API void OspSetLogPrefix(u8 byPrint);

//设置是否输出运行日志文件以及运行日志的级别
//0:不输出运行日志; 非0: 日志级别值小于等于该值的，输出为运行日志
API void OspSetWriteRunLog(u8 byIsWriteLog);

//设置是否输出错误日志文件：//0:不输出; 非0:输出
//包括以下两种级别的日志: ERROR_LEV //1错误级别  OSP_WARNING_LEV //2警告级别
API void OspSetWriteErrLog(u8 byIsWriteLog);


#endif //end __cplusplus

/*=============================================================================
函 数 名：OspLog
功      能：全局log函数。这个函数的开关是应用号0。调用OspSetLogLevel(0, xx)就可以
控制该函数的打开或关闭
注      意：
算法实现：
全局变量：
参      数：u8 byLevel : [in] log level
          char * szFormat : [in] message format
返 回 值：void
-------------------------------------------------------------------------------
修改记录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspLog(u8 byLevel, const char* szFormat, ...);

/*=============================================================================
函 数 名：OspMsgDumpSet
功      能：set long msg dump detail can/not be whole print, default is printf
10lines
注      意：
算法实现：
全局变量：
参      数：BOOL32 bMsgDumpEnbl : [in] enable or not
返 回 值：void
-------------------------------------------------------------------------------
修改记录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspMsgDumpSet(BOOL32 bLongMsgDumpEnbl);

/*=============================================================================
函 数 名：OspStatusMsgOutSet
功      能：set status message can or not be out put every timer
注      意：
算法实现：
全局变量：
参      数：BOOL32 bOutMsgEnable : [in] enable or not
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspStatusMsgOutSet(BOOL32 bOutMsgEnable);

/*====================================================================
函数名：OspPrintf
功能：把相应的内容显示到屏幕,存储到文件,不能屏蔽
算法实现：（可选项）
引用全局变量：
输入参数说明：
bScreen: 是否输出到屏幕,
bFile: 是否输出到文件,
szFormat: 格式,
返回值说明：
====================================================================*/
API void OspPrintf(BOOL32 bScreen, BOOL32 bFile, const char* szFormat,...);


/*====================================================================
函数名：OspTelAuthor
功能：设置Telnet的授权用户名和密码(vxworks下用户名和密码为空会设置失败)
算法实现：（可选项）
引用全局变量：
输入参数说明：
返回值说明：
====================================================================*/
#ifdef __cplusplus
API void OspTelAuthor(const char* szUsername = NULL, const char* szPassword = NULL);
#else
API void OspTelAuthor(const char* szUsername, const char* szPassword);
#endif
/*=============================================================================
函 数 名：OspVerPrintf, OspVer
功      能：output compile version info to screen
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
2010/03/10  V1R4B3 梁  黎  版本信息不再输出到日志文件
=============================================================================*/
#define  OspVerPrintf() \
OspPrintf(TRUE, FALSE, "Osp version: %s  compile time: %s %s\n", OSPVERSION, __DATE__, __TIME__)
API void ospver();

/*=============================================================================
函 数 名：OspCat
功      能：output file content to screen
注      意：
算法实现：
全局变量：
参      数：const char*  szFileName : [in] file to be opened
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspCat(const char* szFileName);

#ifdef _LINUX_
typedef void (*POspQuitFunc)(s32);
/*====================================================================
  函数名：OspRegQuitFunc
  功能：注册任务退出函数，仅linux下有效
  算法实现：
  引用全局变量：
  输入参数说明：
      pfQuitFunc    - 任务退出函数
  返回值说明：TRUE - 注册成功
  ====================================================================*/
API BOOL32 OspRegQuitFunc(POspQuitFunc pfQuitFunc);

/*====================================================================
  函数名：OspRegTaskInfo
  功能：注册任务信息，仅linux下有效
  算法实现：
  引用全局变量：
  输入参数说明：
      dwTaskId    - 任务号
        szName    - 任务别名
  返回值说明：无
  ====================================================================*/
API void OspRegTaskInfo(TASKID dwTaskId, const char* szName);

/*====================================================================
  函数名：OspUnRegTaskInfo
  功能：反注册任务信息
  算法实现：
  引用全局变量：
  输入参数说明：
      dwTaskId    - 任务号
  返回值说明：无
  ====================================================================*/
API BOOL32 OspUnRegTaskInfo(TASKID dwTaskId);

/*====================================================================
  函数名：OspTaskShow
  功能：打印任务信息，仅linux下有效
  算法实现：
  引用全局变量：
  输入参数说明：
  返回值说明：无
  ====================================================================*/
API void OspTaskShow();

/*====================================================================
  函数名：OspRegCommand
  功能：注册可以在Telnet下执行的函数(该函数仅在linux下有效，windows下不需注册)
  算法实现：
  引用全局变量：
  输入参数说明：
      szName    - Telnet下输入的命令
      pvFunc    - 对应的函数指针
      szUsage    - 命令的帮助信息
  返回值说明：无
  ====================================================================*/
API void OspRegCommand(const char* szName, void* pvFunc, const char* szUsage);

/*====================================================================
  函数名：OspRegCommandEx
  功能：注册可以在Telnet下执行的外部函数
  算法实现：
  引用全局变量：
  输入参数说明：
      szName    - Telnet下输入的命令
    pfFunc    - 对应的函数指针
    szUsage    - 命令的帮助信息
    szArgFormat - 注册函数参数类型(s:字符串 i:整型 c: 字符 其他telnet 运行的时候报错)
                  例如:func的三个参数依次为整型、字符、字符串,则szArgFormat为"ics"
  返回值说明：无
  ====================================================================*/
API void OspRegCommandEx(const char* szName, void* pvFunc, const char* szUsage,const char* szArgFormat);

#endif

/*=============================================================================
函 数 名：OspNodeConnTest
功      能：test a node's network connection
注      意：
算法实现：
全局变量：
参      数：u32 dwNodeID : [in] id of the node
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspNodeConnTest(u32 dwNodeID);

/*=============================================================================
函 数 名：OspNodeIpGet
功      能：获得指定结点的Ip地址
注      意：
算法实现：
全局变量：
参      数：dwNodeID : [in] node id

 返 回 值：成功返回结点IP, 失败返回0.
=============================================================================*/
API u32 OspNodeIpGet(u32 dwNodeID);

/*=============================================================================
  函 数 名：OspNodeLocalIpGet
  功    能：获取节点的本地地址
  注    意：对于已经连接的tcp节点，在本地存在多个地址的情况下，需要知道对方实际连接的本地ip地址。
  算法实现：
  全局变量：
  参    数：dwNodeID : [in] node id

  返 回 值：成功返回结点IP, 失败返回0.
  =============================================================================*/
API u32 OspNodeLocalIpGet(u32 dwNodeID);

/*====================================================================
函数名： OspInstShow
功能：显示某个App中的Instance的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspInstShow(u16 wAppID);

/*=============================================================================
函 数 名：OspInstShowAll
功      能：显示所有Instance的状态信息
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspInstShowAll();

/*=============================================================================
函 数 名：OspInstDump
功      能：显示某一instance的自定义信息，调用该instance的dump函数。
注      意：
算法实现：
全局变量：
参      数：wAppID : [in] app id
InstId : [in] instance id, 0:显示所有的，-1:显示非IDLE的
param : [in] reserved parameter
返 回 值：
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspInstDump(u16 wAppID, u16 wInsID, u32 dwReserved);

/*====================================================================
函数名：OspNodeShow
功能：显示所有Node的状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspNodeShow();

/*=============================================================================
函 数 名：OspAppShow
功      能：显示App的状态信息
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspAppShow();

/*=============================================================================
函 数 名：OspDispatchShow
功      能：显示Dispatch task的信息
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspDispatchShow();

/*====================================================================
函数名：OspTimerShow
功能：显示timer状态信息
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspTimerShow();

/*====================================================================
函数名：OspLogShow
功能：显示日志模块状态
算法实现：（可选项）
引用全局变量:
输入参数说明：
返回值说明: 无
====================================================================*/
API void OspLogShow();

/*====================================================================
函数名：OspMemShow
功能：    显示内存管理堆栈中消息块数目
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspMemShow();

/*=============================================================================
函 数 名：OspAppEchoTest
功      能：Application echo testing
注      意：
算法实现：
全局变量：
参      数：u16 AppId : [in] Application id
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspAppEchoTest(u16 wAppId);

/*=============================================================================
函 数 名：OspAddEventDesc
功      能：用于将事件的描述登记到消息跟踪系统
注      意：
算法实现：
全局变量：
参      数：const char* desc : [in]
u16 event : [in]
返 回 值：
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspAddEventDesc(const char* szDesc, u16 wEvent);

/*=============================================================================
函 数 名：OspSendTrcOn
功      能：打开OSP结点发送跟踪标识
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspSendTrcOn();

/*=============================================================================
函 数 名：OspSendTrcOff
功      能：关闭OSP结点发送跟踪标识
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspSendTrcOff();

/*=============================================================================
函 数 名：OspRcvTrcOn
功      能：打开OSP结点接收跟踪标识
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspRcvTrcOn();

/*=============================================================================
函 数 名：OspRcvTrcOff
功      能：关闭OSP结点接收跟踪标识
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspRcvTrcOff();

/*====================================================================
函数名：OspTrcAllOn
功能：打开OSP所有应用的跟踪标设
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspTrcAllOn();

/*====================================================================
函数名：OspTrcAllOn
功能：打开OSP所有应用的跟踪标设
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspTrcAllOff();

/*=============================================================================
 函 数 名  : OspOpenLogFile
 功能描述  : 启动日志文件 并设置日志文件的参数
             调用该接口后，会启动日志文件记录
 算法实现  :
 参数说明  :
            [I]const char* szDir 	   日志文件保存的目录
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
API BOOL32 OspOpenLogFile(const char* szFileName, u32 dwMaxSizeKB, u32 dwMaxFiles);

/*=============================================================================
 函 数 名  : OspOpenLogFileEx
 功能描述  : 启动日志文件 并设置日志文件的参数
             调用该接口后，会启动日志文件记录
 算法实现  :
 参数说明  :
 			[I]const char* szFileName  日志文件名字的前缀
            [I]const char* szDir 	   日志文件保存的目录
            [I]u32 dwLogSizeMax_kb 每个文件的最大字节数
            [I]dwLogNumMax         日志文件的最大个数
 返回值说明: TRUE 成功 FALSE 失败
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2015年7月6日    1.0    dengchange    修改
=============================================================================*/
API BOOL32 OspOpenLogFileEx(const char* szFileName, const char* szPath,
                            u32 dwMaxSizeKB, u32 dwMaxFiles);

/*=============================================================================
函 数 名：OspCloseLogFile
功      能：关闭日志文件。
注      意：如果调用OspQuit，则不需要调用该函数。
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspCloseLogFile();

/*=============================================================================
函 数 名：OspSetLogLevel
功      能：控制一个APP的CInstance::log函数的级别
注      意：必须在OspInit之后调用该函数才能生效。OspInit将所有应用的该标志置0
算法实现：
全局变量：
参      数：aid : [in] app id
file_level : [in] file level
screen_level : [in] screen level
if level=5, 那么0~5 打印,  6~255 不打印,设置为3,则只有log(0|1|2,"")
的调用能够输出。
返 回 值：
=============================================================================*/
API void OspSetLogLevel(u16 wAppID, u8 byFileLevel, u8 byScreenLevel);

/*====================================================================
函数名：OspSetFileLogLevel
功能：设置App的使用OspLog()/Instance::log()时的日志文件等级
      日志级别小于等于设置级别，才会记录到日志文件
算法实现：（可选项）
引用全局变量：
输入参数说明：wAid App的等级
byFileLevel: 文件Log的等级
返回值说明：
====================================================================*/
API void OspSetFileLogLevel(u16 wAppID, u8 byFileLevel);

/*====================================================================
函数名：OspSetScrnLogLevel
功能：设置App的使用OspLog()/Instance::log()时的日志文件等级
      日志级别小于等于设置级别，才会在屏幕输出
      注意: 需调用OspResumeScrnLog()后 才能输出日志
算法实现：（可选项）
引用全局变量：
输入参数说明：
uAid App的等级
uScrnLevel: 屏幕Log的等级
返回值说明：
====================================================================*/
API void OspSetScrnLogLevel(u16 wAppID, u8 byScrnLevel);

/*=============================================================================
 函 数 名  : OspSetTrcFlag
 功能描述  : 设置消息跟踪开关
 算法实现  : wFileFlag/wScrnFlag分为
                 1 跟踪状态机
                 2 收发消息
                 4 定时器
                 7 上述三者
 参数说明  :
            [I]u16 wAppID    AppId为0   设置全局(与App无关)的跟踪
                             AppId不为0 设置对应App的跟踪
            [I]u16 wFileFlag 跟踪结果保存到日志文件标志
            [I]u16 wScrnFlag 跟踪结果在屏幕上显示标志
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2002/05/23     1.0
 2. 2015年6月29日  2.0    dengchange    修改注释
=============================================================================*/
API void OspSetTrcFlag(u16 wAppID, u16 wFileFlag, u16 wScrnFlag);

/*=============================================================================
 函 数 名  : OspSetFileTrcFlag
 功能描述  : 设置消息跟踪开关
 算法实现  : wFileFlag/wScrnFlag分为
                 1 跟踪状态机
                 2 收发消息
                 4 定时器
                 7 上述三者
 参数说明  :
            [I]u16 wAppID    AppId为0   设置全局(与App无关)的跟踪
                             AppId不为0 设置对应App的跟踪
            [I]u16 wFileFlag 跟踪结果保存到日志文件标志
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2002/05/23     1.0
 2. 2015年6月29日  2.0    dengchange    修改注释
=============================================================================*/
API void OspSetFileTrcFlag(u16 wAppID, u16 wFileFlag);

/*=============================================================================
 函 数 名  : OspSetScrnTrcFlag
 功能描述  : 设置消息跟踪开关
 算法实现  : wFileFlag/wScrnFlag的值对应:
                 1 跟踪状态机
                 2 收发消息
                 4 定时器
                 7 上述三者
 参数说明  :
            [I]u16 wAppID    AppId为0   设置全局(与App无关)的跟踪
                             AppId不为0 设置对应App的跟踪
            [I]u16 wScrnFlag 跟踪结果在屏幕上显示标志
 返回值说明: 无
-------------------------------------------------------------------------------
 修改记录  :
 日  期            版本    修改人    修改内容
 1. 2002/05/23     1.0
 2. 2015年6月29日  2.0    dengchange    修改注释
=============================================================================*/
API void OspSetScrnTrcFlag(u16 wAppID, u16 wScreenFlag);

/*=============================================================================
函 数 名：OspRegistModule
功      能：向OSP登记一个模块(仅windows下有效)
注      意：
算法实现：
全局变量：
参      数：(in)pchModuleName - 模块（.exe文件）名，以NULL结尾的字符串，
如"kdvmt.dll"等
返 回 值：成功返回TRUE，失败返回FALSE
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspRegistModule(const char* szModuleName);

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
API BOOL32 OspUnRegistModule(const char* szModuleName);

/*=============================================================================
函 数 名：OspSetLogEventDataLength
功      能：设置全局的消息跟踪显示的长度。
注      意：
算法实现：
全局变量：
参      数：u16 wLength : [in] length of log event data
返 回 值：u16 previous length
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API u16 OspSetLogEventDataLength(u16 wLength);

/*=============================================================================
函 数 名：OspEnableBroadcastAck
功      能：大多数情况下，用户发送广播消息并不需要对方返回确认消息。缺省情况下，
PIS不向消息的发送者发送OSP_BROADCASTACK消息，但是可以通过该函数修改这
个选项。是否发送OSP_BROADCASTACK是由接受节点决定的。
注      意：
算法实现：
全局变量：
参      数：u16 aid : [in] 表示广播消息的接受者AID。
BOOL32 bEnable : [in] enable or not
返 回 值：BOOL32 - true Succeeded, false Failed
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspEnableBroadcastAck(u16 wAppID, BOOL32 bEnable);

/*=============================================================================
函 数 名：OspGetNodeAddr
功      能：查找Osp结点地址（本端与远端IP+Port）。
注      意：
算法实现：
全局变量：
参      数：u32 dwNodeID : [in] 结点ID
                    TOspNodeAddr* ptOspNodeAddr : [out] 结点地址
返 回 值：FALSE - 查询成功
                    TRUE - 查询失败
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2006/08/10    4.0
=============================================================================*/
typedef struct
{
    u32 m_dwLocalIP;//网络序
    u16 m_wLocalPort;//主机序
    u32 m_dwPeerIP;//网络序
    u16 m_wPeerPort;//主机序
}TOspNodeAddr;
API BOOL32 OspGetNodeAddr(u32 dwNodeID, TOspNodeAddr* ptOspNodeAddr);

#if defined(_MSC_VER) || defined(_LINUX_)

//sock5代理版本号
#define SOCK5_PROXY_VERSION                   (u8)0x05
//客户端支持鉴权方法
#define SOCK5_PROXY_AUTHEN_NO_REQUIERD        (u8)0x00
#define SOCK5_PROXY_AUTHEN_USERNAME_PASSWORD  (u8)0x02
//操作结果
#define SOCK5_PROXY_SUCCESS                   (u8)0x00
#define SOCK5_PROXY_AUTHEN_NO_ACCEPTABLE      (u8)0xff
//请求命令
#define SOCK5_PROXY_CMD_TCP_CONNECT           (u8)0x01
#define SOCK5_PROXY_CMD_TCP_BIND              (u8)0x02
#define SOCK5_PROXY_CMD_UDP_ASSOCIATE         (u8)0x03
//保留字段
#define SOCK5_PROXY_RESERVED_DATA             (u8)0x00
//地址类型
#define SOCK5_PROXY_IPV4_ADDR                 (u8)0x01
#define SOCK5_PROXY_DOMAIN_NAME               (u8)0x03
#define SOCK5_PROXY_IPV6_ADDR                 (u8)0x04


#define MAX_SOCK5PROXY_AUTHEN_NUM             (u8)0xff
#define MAX_SOCK5PROXY_NAME_PASS_LENGTH       (u8)64
typedef struct
{
    u32 m_dwProxyIP;
    u16 m_wProxyPort;
    u8 m_byAuthenNum;
    u8 m_abyAuthenMethod[MAX_SOCK5PROXY_AUTHEN_NUM];
    s8 m_achUseName[MAX_SOCK5PROXY_NAME_PASS_LENGTH+1];
    s8 m_achPassword[MAX_SOCK5PROXY_NAME_PASS_LENGTH+1];
}TOspSock5Proxy;

/*=============================================================================
函 数 名：OspConnectTcpNodeThroughSock5Proxy
功      能：TCP穿越sock5代理连接服务端（与OspConnectTcpNode功能相同，上层不需要调用OspConnectToSock5Proxy与代理建链）
注      意：
算法实现：
全局变量：
参      数：ptOspSock5Proxy : [in] 代理服务器信息结构;
          dwServerIP : [in] 服务器IP
          wServerPort : [in] 服务器端口
          wHb: [in] 断链检测周期(秒)
          byHbNum: [in] byHbNum次未到连接检测应答后认为链路已断开
          dwTimeoutMs : [in] 操作超时时间
          pdwLocalIP: [in, out] 本TCP链接使用的本地IP
返 回 值：失败返回INVALID_NODE；
          成功返回一个动态分配的结点号, 此后用户可借此与服务结点通信
          上层主动断链需调用OspDisconnectTcpNode，所有与无代理时均无区别
-------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
#ifdef __cplusplus
API s32 OspConnectTcpNodeThroughSock5Proxy(TOspSock5Proxy* ptOspSock5Proxy, u32 dwServerIP, u16 wServerPort,
                                           u16 wHb = 10, u8 byHbNum = 3, u32 dwTimeoutMs = 0,
                                           u32* pdwLocalIP = NULL, u8 bySocksAuthVerion = SOCK5_PROXY_VERSION);
#else
API s32 OspConnectTcpNodeThroughSock5Proxy(TOspSock5Proxy* ptOspSock5Proxy, u32 dwServerIP, u16 wServerPort,
                                           u16 wHb , u8 byHbNum , u32 dwTimeoutMs,
                                           u32* pdwLocalIP , u8 bySocksAuthVerion);
#endif

/*=============================================================================
函 数 名：OspConnectToSock5Proxy
功      能：与sock5代理服务器建立TCP链路（UDP穿越sock5代理必须先建立一条TCP链路）
注      意：
算法实现：
全局变量：
参      数：ptOspSock5Proxy : [in] 代理服务器信息结构
          dwTimeoutMs : [in] 操作超时时间
返 回 值：失败返回INVALID_SOCKET；
          成功返回与代理通信的TCP Socket，可进一步调用OspUdpAssociateThroughSock5Proxy建立UDP Associate；
          可复用，需要上层维护本TCP连接，链路段无效时，Udp Associate也应视为无效，
          主动断链请调用OspDisconnectFromSock5Proxy；
-------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
#ifdef __cplusplus
API SOCKHANDLE OspConnectToSock5Proxy(TOspSock5Proxy* ptOspSock5Proxy, u32 dwTimeoutMs = 0, u8 bySocksAuthVerion = SOCK5_PROXY_VERSION);
#else
API SOCKHANDLE OspConnectToSock5Proxy(TOspSock5Proxy* ptOspSock5Proxy, u32 dwTimeoutMs, u8 bySocksAuthVerion );
#endif
/*=============================================================================
函 数 名：OspUdpAssociateThroughSock5Proxy
功      能：UDP穿越sock5代理
注      意：
算法实现：
全局变量：
参      数：hSocket : [in] OspConnectToSock5Proxy返回的socket(可复用);
          dwLocalIP : [in] 本地收发socket IP，以便代理服务器限制数据穿越（网络序）
          wLocaPort : [in] 本地收发socket 端口，以便代理服务器限制数据穿越（主机序）
          pdwProxyMapIP : [out] 代理服务器映射IP（网络序）
          pwProxyMapPort : [out] 代理服务器映射端口（主机序）
          dwTimeoutMs : [in] 操作超时时间
返 回 值：失败返回FALSE；
          成功返回TRUE
-------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
#ifdef __cplusplus
API BOOL32 OspUdpAssociateThroughSock5Proxy(SOCKHANDLE hSocket, u32 dwLocalIP, u16 wLocalPort,
                                            u32* pdwProxyMapIP, u16* pwProxyMapPort, u32 dwTimeoutMs = 0);
#else
API BOOL32 OspUdpAssociateThroughSock5Proxy(SOCKHANDLE hSocket, u32 dwLocalIP, u16 wLocalPort,
                                            u32* pdwProxyMapIP, u16* pwProxyMapPort, u32 dwTimeoutMs);
#endif

/*=============================================================================
函 数 名：OspDisConnectFromSock5Proxy
功      能：断开与sock5代理建立的用于UDP Associate的TCP连接
注      意：
算法实现：
全局变量：
参      数：hSocket : [in] OspConnectToSock5Proxy返回的socket;
返 回 值：失败返回FALSE；
          成功返回TRUE
-------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2006/08/21  4.0   王小月
=============================================================================*/
API BOOL32 OspDisConnectFromSock5Proxy(SOCKHANDLE hSocket);

#endif

/*=============================================================================
函 数 名：OspCreateTcpNode
功      能：创建一个TCP节点。这以后，这个节点可以被连接。请使用6682作为port参数。
注      意：
算法实现：
全局变量：
参      数：u32 dwAddr : [in] reserved
u16 wPort : [in] which port to use
bTcpNodeReuse:[in]创建的node退出后端口是否能立即重用或被其他程序马上占用
返 回 值：s32 - If no error occurs, CreateTcpNode returns a descriptor
referencing the new socket. Otherwise, a value of
INVALID_SOCKET is returned.
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
#ifdef __cplusplus
API SOCKHANDLE OspCreateTcpNode(u32 dwAddr, u16 wPort, BOOL32 bTcpNodeReuse = FALSE);
#else
API SOCKHANDLE OspCreateTcpNode(u32 dwAddr, u16 wPort, BOOL32 bTcpNodeReuse );
#endif
/*=============================================================================
函 数 名：OspConnectTcpNode
功      能：在地址uIpv4Addr和端口uTcpPort上连接服务结点, 并设置断链检测参数.
注      意：
算法实现：
全局变量：g_cOsp
参      数：uIpv4Addr : [in] 服务结点的IP地址,
wTcpPort : [in] 服务结点的侦听端口号,
wHb: [in] 断链检测周期(秒),
byHbNum: [in] byHbNum次未到连接检测应答后认为链路已断开,
dwTimeOutMs: [in] 如果在dwTimeOutMs毫秒内连接不成功, 返回出错. 缺省值0,
表示取connect超时的缺省时间(75s).  该参数目前仅在VxWorks下有效.
pdwLocalIP: [out] 当前连接所用的IP地址

 返 回 值：成功返回一个动态分配的结点号, 此后用户可借此与服务结点通信.
 失败返回INVALID_NODE.
 -------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2003/03/12  2.0
=============================================================================*/
#ifdef __cplusplus
API s32 OspConnectTcpNode(u32 dwIpv4Addr, u16 wTcpPort, u16 wHb = 10, u8 byHbNum = 3, u32 dwTimeoutMs = 0, u32* pdwLocalIP = NULL);
#else
API s32 OspConnectTcpNode(u32 dwIpv4Addr, u16 wTcpPort, u16 wHb , u8 byHbNum , u32 dwTimeoutMs , u32* pdwLocalIP );
#endif
/*=============================================================================
函 数 名：OspSetHBParam
功      能：设置结点的断链检测参数
注      意：
算法实现：
全局变量：
参      数：dwNodeID: 结点号,
wHb: 链路检测周期(以秒为单位),
byHbNum: 如果连续ping了uHbNum次没有应答, 主动断开.

 返 回 值：成功返回TRUE, 失败返回FALSE.
 -------------------------------------------------------------------------------
 修改纪录：
 日      期  版本  修改人  修改内容
 2003/06/5     2.1
=============================================================================*/
API BOOL32 OspSetHBParam(u32 dwNodeID, u16 wHb, u8 byHbNum);

/*=============================================================================
函 数 名：OspNodeDiscCBReg
功      能：设置在node连接中断时需通知的appid和InstId
注      意：目前支持通知多个实例，每次调用该函数，则只通知指定实例，其他已注册待
　　　　　通知实例将被清除。
算法实现：
全局变量：
参      数：dwNodeID : [in] node id
wAppId : [in] application id
wInsId : [in] instance id
返 回 值：成功返回OSP_OK，失败参见错误码
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API s32 OspNodeDiscCBReg(u32 dwNodeID, u16 wAppID, u16 wInsID);

/*=============================================================================
函 数 名：OspNodeDiscCBRegQ
功      能：增加在node连接中断时需通知的appid和InstId
注      意：每次调用此函数，则增加一个要通知的实例。除非达到最大个数
算法实现：
全局变量：
参      数：dwNodeID : [in] node id
wAppId : [in] application id
wInsId : [in] instance id
返 回 值：成功返回OSP_OK，失败参见错误码
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API s32 OspNodeDiscCBRegQ(u32 dwNodeID, u16 wAppID, u16 wInsID);

/*=============================================================================
函 数 名：OspDisconnectTcpNode
功      能：断开在一个node上的连接。
注      意：
算法实现：
全局变量：
参      数：u8 node_id : [in] node id
返 回 值：BOOL32 - true Succeeded, false Failed
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspDisconnectTcpNode(u32 dwNodeID);

#ifdef _LINUX_
/*=============================================================================
函 数 名：OspDisconnectAllTcpNode
功      能：停止当前的监听,并断开其上创建全部连接(目前仅linux下有效)。
注      意：
算法实现：
全局变量：
参      数：
返 回 值：
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2006/12/06    1.0   罗健锋
=============================================================================*/
API BOOL32 OspDisconnectAllTcpNode();
#endif

/*=============================================================================
函 数 名：OspIsValidTcpNode
功      能：判断一个TCP节点是否有效。
注      意：
算法实现：
全局变量：
参      数：u32 node_id : [in] node id
返 回 值：BOOL32 - Valid or not
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspIsValidTcpNode(u32 dwNodeID);

/*====================================================================
函数名：OspIsNodeCheckEnable
功能：判断指定结点的链路检测功能是否起用。
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspIsNodeCheckEnable(u32 dwNodeID);

/*====================================================================
函数名：OspEnableNodeCheck
功能：起用指定结点的链路检测功能。
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspEnableNodeCheck(u32 dwNodeID);

/*====================================================================
函数名：OspDisableNodeCheck
功能：禁用指定结点的链路检测功能。
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspDisableNodeCheck(u32 dwNodeID);

/*====================================================================
函数名：OspSetAppPriority
功能：设置App的调度优先级。
算法实现：（可选项）
引用全局变量：
输入参数说明：wAppId -- 目标App号,
byPrior -- 优先级.

 返回值说明：成功, 返回TRUE; 失败, 返回FALSE.
====================================================================*/
API BOOL32 OspSetAppPriority(u16 wAppID, u8 byAppPrior);

/*====================================================================
函数名：OspGetAppPriority
功能：获得指定App的调度优先级。
算法实现：（可选项）
引用全局变量：
输入参数说明：wAppId -- (in)目标App号,
pbyPrior -- (out)优先级.

 返回值说明：成功, 返回TRUE; 失败, 返回FALSE.
====================================================================*/
API BOOL32 OspGetAppPriority(u16 wAppID, u8* pbyAppPrior);

/*=============================================================================
函 数 名：OspIsLocalHost
功      能：判断是否是本机IP，如OspIsLocalHost(inet_addr("127.0.0.1"));
注      意：如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
本函数效率较低，不要重复调用。
算法实现：
全局变量：
参      数：u32 dwIP : [in] 需要判断的IP
返 回 值：BOOL32    是否为本机IP
-------------------------------------------------------------------------------
修改记录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspIsLocalHost(u32 dwIP);

/*=============================================================================
函 数 名：OspPost
功      能：发送异步消息
注      意：
算法实现：
全局变量：
参      数：u32 dwDstIID : [in] destination info
u16 wEvent : [in] OSP_ event
const void* pvContent : [in] message data
u16 wLength : [in] sizeof content in u8s
u32 dwDstNode : [in] destionation socket node
u32 dwSrcIID : [in] source info
u32 dwSrcNode : [in] source node
s32 nTimeout : [in] drop the message when message queue full
返 回 值：s32 - 0 Succeeded, others Failed
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/

//因原来上层对post消息大部分没判断返回值，且认为发送必定成功，那是在阻塞的基础上
//现改为非阻塞后，可能会有发送超时导致失败，为保持以前的接口语意，所以将超时从2000改为-1 baihao 2008.6
#ifdef __cplusplus
API s32 OspPost(u32 dwDstIID, u16 wEvent, const void* pvContent = 0, u16 wLength = 0,
                u32 dwDstNode = 0, u32 dwSrcIID = MAKEIID(INVALID_APP, INVALID_INS), u32 dwSrcNode = INVALID_NODE, s32 nTimeout = 2000);

/* used only in C++ environment */
s32 OspPost(const char* szDstAlias, u8 byAliasLen, u16 wDstAppID, u16 wEvent,
            const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0, u32 dwSrcIID = MAKEIID(INVALID_APP, INVALID_INS), u32 dwSrcNode = INVALID_NODE, s32 nTimeout = 2000);
#else
API s32 OspPost(u32 dwDstIID, u16 wEvent, const void* pvContent, u16 wLength ,
                u32 dwDstNode , u32 dwSrcIID, u32 dwSrcNode, s32 nTimeout);
#endif

/*=============================================================================
函 数 名：OspSend
功      能：全局同步消息发送
注      意：
算法实现：
全局变量：
参      数：u32 dwDstIID : [in] destination info
u16 wEvent : [in] OSP_ event
const void* pvContent : [in] message data
u16 wLength : [in] sizeof content in u8s
u32 dwDstNode : [in] destionation socket node
u32 dwSrcIID : [in] source info
u32 dwSrcNode : [in] source node
返 回 值：s32 - 0 Succeeded, others Failed
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
#ifdef __cplusplus
API s32 OspSend(u32 dwDstIID, u16 wEvent, const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0,
                u32 dwSrcIID = MAKEIID(INVALID_APP, INVALID_INS), u32 dwSrcNode = INVALID_NODE,
                void* pvAckBuf = NULL, u16 wAckBufLen = 0, u16* pwRealAckLen = NULL,
                u16 wTimeout = 1000);

/* used only in C++ environment */
s32 OspSend(const char* szDstAlias, u8 byAliasLen, u16 wDstAppID, u16 wEvent,
            const void* pvContent = 0, u16 wLength = 0, u32 dwDstNode = 0,
            u32 dwSrcIID = MAKEIID(INVALID_APP, INVALID_INS), u32 dwSrcNode = INVALID_NODE,
            void* pvAckBuf = NULL, u16 wAckBufLen = 0, u16* pwRealAckLen = NULL,
            u16 wTimeout = 1000);
#else
API s32 OspSend(u32 dwDstIID, u16 wEvent, const void* pvContent , u16 wLength , u32 dwDstNode ,
                u32 dwSrcIID, u32 dwSrcNode,
                void* pvAckBuf, u16 wAckBufLen , u16* pwRealAckLen ,
                u16 wTimeout);
#endif

/*====================================================================
函数名：IsOspInitd
功能：判断OSP是否已初始化过了
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：OSP已初始化，返回TRUE；否则返回FALSE.
====================================================================*/
API BOOL32 IsOspInitd();


/*=============================================================================
函 数 名：OspInit
功      能：initialize OSP module
注      意：
算法实现：
全局变量：
参      数：TelNetEnable : [in] start telnet.exe
          TelNetPort : [in] osp telnet server port
          pchModuleName : [in] process module name，maxlength = 63
返 回 值：
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
#define MAX_MODULE_NAME_LENGTH 256
#ifdef __cplusplus
API BOOL32 OspInit(BOOL32 bTelNetEnable=FALSE, u16 wTelnetPort = 0, const char* szModuleName = NULL, u32 dwMaxNodeNum = 512, u32 dwMaxDispatchMsg = 1024);

// Osp初始化，扩展了是否指定telnet端口复用
API BOOL32 OspInitEx(BOOL32 bTelNetEnable=FALSE, u16 wTelNetPort=0, BOOL32 bTelnetREUSEADDR=TRUE, const char* pchModuleName=NULL, u32 dwMaxNodeNum=512, u32
dwMaxDispatchMsg=1024);
#else
API BOOL32 OspInit(BOOL32 bTelNetEnable, u16 wTelnetPort , const char* szModuleName , u32 dwMaxNodeNum , u32 dwMaxDispatchMsg);
API BOOL32 OspInitEx(BOOL32 bTelNetEnable, u16 wTelNetPort, BOOL32 bTelnetREUSEADDR, const char* pchModuleName, u32 dwMaxNodeNum, u32
dwMaxDispatchMsg);
#endif

/*====================================================================
  函数名：OspSetTeleMode
  功能：设置telnet的模式
  算法实现：
  引用全局变量：
  输入参数说明：
  	dwMode	- Telnet模式
  	          TELNET_PORT_OFF: 关闭telnet端口
  	          TELNET_LOCAL_ON: 仅本地端口可telnet
  	          TELNET_ALL_ON:   本地和远程都可以telnet
  返回值说明：成功返回TRUE,失败返回FALSE
  ====================================================================*/
enum ETelMode
{
    TELNET_PORT_OFF = 0,
    TELNET_LOCAL_ONLY = 1,
    TELNET_ALL_ON =2,
};
API BOOL32 OspSetTeleMode(u32 dwMode);
/*====================================================================
函数名：OspQuit
功能：退出OSP，调用本地所有APP的所有实例的InstanceExit方法，退出实例，
断开所有OSP结点，挂起所有OSP任务包括APP，然后返回。OSP退出之后，
所有OSP服务将不能使用。
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspQuit();

/*=============================================================================
函 数 名：OspSetPrompt
功      能：设置终端提示符
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API BOOL32 OspSetPrompt(const char* szPrompt);

/*====================================================================
函数名：OspStopScrnLog
功能：暂停屏幕日志输出.
算法实现：（可选项）
引用全局变量：
输入参数说明：无

 返回值说明：无.
====================================================================*/
API void OspStopScrnLog();

/*====================================================================
函数名：OspResumeScrnLog
功能：恢复屏幕日志输出.
      与OspSetScrnLogLevel()，OspSetGlbLogPrintLevel()等配合使用
算法实现：（可选项）
引用全局变量：
输入参数说明：无

 返回值说明：无.
====================================================================*/
API void OspResumeScrnLog();

/*=============================================================================
函 数 名：OspGetLocalID
功      能：本地全局编号
注      意：
算法实现：
全局变量：
参      数：
返 回 值：s32 - 本地全局编号
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API s32 OspGetLocalID(); // 本地全局编号

/*=============================================================================
函 数 名：OspEventDesc
功    能：返回消息的描述
注    意：
算法实现：
全局变量：
参    数：u16 wEventID : [in]消息的ID
返 回 值：const char*  消息的描述，如果消息ID无效或者该消息没有描述字符串，返回
值将为NULL。
-------------------------------------------------------------------------------
修改纪录：
日      期  版本  修改人  修改内容
2002/05/23  1.0
=============================================================================*/
API const char* OspEventDesc(u16 wEventID);

/*=============================================================================
函 数 名：OspEventDescShow
功      能：显示消息的描述
注      意：
算法实现：
全局变量：
参      数：u16 wEventID : [in]消息的ID
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void OspEventDescShow(u16 wEventID);

/*=============================================================================
函 数 名：osphelp
功      能：inline help function of OSP module
注      意：
算法实现：
全局变量：
参      数：
返 回 值：void
-------------------------------------------------------------------------------
修改纪录：
日        期    版本  修改人  修改内容
2002/05/23    1.0
=============================================================================*/
API void osphelp();

/*====================================================================
函数名：OspSemBCreate
功能：创建一个二元信号量
算法实现：（可选项）
引用全局变量：
输入参数说明：(out)phSemHandle 返回的信号量句柄

 返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 OspSemBCreate(SEMHANDLE* phSemHandle);

/*====================================================================
函数名：OspSemCCreate
功能：创建计数信号量
算法实现：（可选项）
引用全局变量：
输入参数说明：(out)phSemHandle 信号量句柄返回参数，
(in)initcount 初始计数，
(in)MaxSemCount 最大计数

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemCCreate(SEMHANDLE* phSemHandle, u32 dwInitCount, u32 dwMaxSemCount);

/*====================================================================
 函数名：OspSemDelete
 功能：删除信号量
 算法实现：（可选项）
 引用全局变量：
 输入参数说明：(in)hSemHandle 待删除信号量的句柄

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemDelete(SEMHANDLE hSemHandle);

/*====================================================================
函数名：OspSemTake
功能：信号量p操作，如该信号量不可用，则阻塞调用实例所在任务(APP)
算法实现：（可选项）
引用全局变量：
输入参数说明：(in)hSemHandle 信号量句柄

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemTake(SEMHANDLE hSemHandle);

/*====================================================================
函数名：OspSemTakeByTime
功能：带超时的信号量p操作，如信号量不可用，则时间消逝dwtime毫秒后，出错返回
算法实现：（可选项）
引用全局变量：
输入参数说明：(in)hSemHandle 信号量句柄, (in)dwtime 超时间隔(以ms为单位)

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemTakeByTime(SEMHANDLE hSemHandle, u32 dwTime);

/*====================================================================
函数名：OspSemGive
功能：信号量v操作
算法实现：（可选项）
引用全局变量：
输入参数说明：(in)hSemHandle 信号量句柄

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspSemGive(SEMHANDLE hSemHandle);

/*====================================================================
函数名：OspTaskCreate
功能：创建并执行一个任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
#ifdef _LINUX_
#define OSPTASK_PARAM_JOINABLE 0xff + PTHREAD_CREATE_JOINABLE  //OspTaskCreate默认情况下建立的是DETACHED, 若有需要可以设置wFlag，使得线程成为JOINABLE
    #ifdef __cplusplus
API TASKHANDLE OspTaskCreate(LINUXFUNC pvTaskEntry, const char* szName, u8 byPriority, u32 dwStacksize, KD_PTR pvParam, u16 wFlag, TASKID* pdwTaskID = NULL);
    #else
API TASKHANDLE OspTaskCreate(LINUXFUNC pvTaskEntry, const char* szName, u8 byPriority, u32 dwStacksize, KD_PTR pvParam, u16 wFlag, TASKID* pdwTaskID );
    #endif
#else
#ifdef __cplusplus
API TASKHANDLE OspTaskCreate(void* pvTaskEntry, const char* szName, u8 byPriority, u32 dwStacksize, KD_PTR pvParam, u16 wFlag, TASKID* pdwTaskID = NULL);
#else
API TASKHANDLE OspTaskCreate(void* pvTaskEntry, const char* szName, u8 byPriority, u32 dwStacksize, KD_PTR pvParam, u16 wFlag, TASKID* pdwTaskID );
#endif
#endif

#ifdef _LINUX_
API TASKHANDLE OspTaskCreate_FIFO(LINUXFUNC pvTaskEntry, const s8* szName, u8 byPriority, u32 dwStacksize, void* dwParam, u16 wFlag, u32 *puTaskID);
#else
API TASKHANDLE OspTaskCreate_FIFO(void* pvTaskEntry, const s8* szName, u8 byPriority, u32 dwStacksize, void* dwParam, u16 wFlag, u32 *puTaskID);
#endif


/*====================================================================
函数名：OspTaskWaitEnd
功能：等待指定线程结束
算法实现：windows下调用WaitForSingleObject，Linux下调用pthread_join
引用全局变量：
输入参数说明：hTask    --   需等待结束的的线程(Linux下需要创建非分离属性的线程)

  返回值说明：对应函数的返回值
====================================================================*/
API s32 OspTaskWaitEnd(TASKHANDLE hTask);


/*====================================================================
  函数名：OspTaskSetAffinity
  功能：
  封装
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：hTaskHandle:windows下的线程/进程的句柄；hTaskId:绑定的线程/进程/任务的ID；
                dwCpuId:需绑定的CPU(从0开始计数,逐次递加)
                byTaskType：绑定的线程/进程/任务的类型（0--进程，1-线程，其他-任务），linux下默认0即可
                pdwPreviousBind：如果之前该进程/线程/任务被绑定到某CPU，则会返回该CPU号（仅Solaris下有效）
  返回值说明：成功返回TRUE, 失败返回FALSE.
  ====================================================================*/
#ifdef _MSC_VER
#ifdef __cplusplus
API BOOL32 OspTaskSetAffinity(TASKHANDLE hTaskHandle, u32 dwCpuId, u8 byTaskType = 0, u32* pdwPreviousBind = NULL);
#else
API BOOL32 OspTaskSetAffinity(TASKHANDLE hTaskHandle, u32 dwCpuId, u8 byTaskType, u32* pdwPreviousBind);
#endif
#else
    #ifdef __cplusplus
API BOOL32 OspTaskSetAffinity(TASKHANDLE hTaskHandle, u32 dwCpuId, u8 byTaskType = 0, u32* pdwPreviousBind = NULL);
    #else
API BOOL32 OspTaskSetAffinity(TASKHANDLE hTaskHandle, u32 dwCpuId, u8 byTaskType , u32* pdwPreviousBind);
    #endif //__cplusplus
#endif

/*====================================================================
函数名：OspTaskExit
功能：退出调用任务，任务退出之前用户要注意释放本任务申请的内存、信号量等资源。
封装Windows的ExitThread(0)和vxWorks的taskDelete(0)。
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspTaskExit();

/*====================================================================
函数名：OspTaskTerminate
功能：强行终止指定任务。
封装Windows的ExitThread()和vxWorks的taskDelete()。
算法实现：（可选项）
引用全局变量：
输入参数说明：handle--待杀任务的handle

 返回值说明：
====================================================================*/
API BOOL32 OspTaskTerminate(TASKHANDLE hTaskHandle);

/*====================================================================
函数名：OspTaskSuspend
功能：挂起当前任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
#ifdef __cplusplus
API BOOL32 OspTaskSuspend(TASKHANDLE hTaskHandle = 0);
#else
API BOOL32 OspTaskSuspend(TASKHANDLE hTaskHandle);
#endif

/*====================================================================
函数名：OspTaskResume
功能：挂起当前任务
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspTaskResume(TASKHANDLE hTaskHandle);

/*====================================================================
函数名：OspTaskSetPriority
功能：改变任务的优先级。

 算法实现：（可选项）
 引用全局变量：
 输入参数说明：

  返回值说明：
====================================================================*/
#ifdef __cplusplus
API BOOL32 OspTaskSetPriority(TASKHANDLE hTaskHandle, u8 byNewPriority, s32 nNewSchedPolicy = 0);
#else
API BOOL32 OspTaskSetPriority(TASKHANDLE hTaskHandle, u8 byNewPriority, s32 nNewSchedPolicy);
#endif

/*====================================================================
函数名：OspTaskGetPriority
功能：获得任务的优先级。

 算法实现：（可选项）
 引用全局变量：
 输入参数说明：

  返回值说明：
====================================================================*/
#ifdef __cplusplus
API BOOL32 OspTaskGetPriority(TASKHANDLE hTaskHandle, u8* puPri, s32* pnSchedPolicy = NULL);
#else
API BOOL32 OspTaskGetPriority(TASKHANDLE hTaskHandle, u8* puPri, s32* pnSchedPolicy);
#endif

/*====================================================================
函数名：OspSetTaskPriorityByName
功能：通过任务名设置任务的优先级

 算法实现：（可选项）
 引用全局变量：
 输入参数说明：

  返回值说明：
====================================================================*/
#ifdef __cplusplus
API void OspSetTaskPriorityByName(const char* szName, u8 byPriority, s32 nNewSchedPolicy = 0);
#else
API void OspSetTaskPriorityByName(const char* szName, u8 byPriority, s32 nNewSchedPolicy);
#endif

/*====================================================================
函数名：OspSetTaskPriorityByName
功能：通过任务名设置任务的时间片大小，linux下有效
 算法实现：（可选项）
 引用全局变量：
 输入参数说明：

  返回值说明：
====================================================================*/
API void OspSetTaskNiceByName(const char* szName, s32 nNice);

/*====================================================================
函数名：OspSetTaskPriorityByName
功能：通过任务名获取任务的优先级

 算法实现：（可选项）
 引用全局变量：
 输入参数说明：

  返回值说明：
====================================================================*/
#ifdef __cplusplus
API void OspGetTaskPriorityByName(const char* szName, u8* pbyPriority, s32* pnNewSchedPolicy = NULL);
#else
API void OspGetTaskPriorityByName(const char* szName, u8* pbyPriority, s32* pnNewSchedPolicy);
#endif

/*====================================================================
函数名：OspTaskSelfHandle
功能：获得调用任务的句柄
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API TASKHANDLE OspTaskSelfHandle();

/*====================================================================
函数名：OspTaskSelfHandle
功能：获得调用任务的ID
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API TASKID OspTaskSelfID();

/*====================================================================
函数名：OspTaskHandleVerify
功能：判断指定任务是否存在
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspTaskHandleVerify(TASKHANDLE hTaskHandle);

/*====================================================================
函数名：OspTaskSafe
功能：保护调用任务不被非法删除
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspTaskSafe();

/*====================================================================
函数名：OspTaskUnsafe
功能：释放调用任务的删除保护，使得调用任务可以被安全删除
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API void OspTaskUnsafe();

/*====================================================================
函数名：OspTaskDelay
功能：任务延时
算法实现：（可选项）
引用全局变量：
输入参数说明：uMseconds--延时间隔(ms)

 返回值说明：
====================================================================*/
#define OspDelay     OspTaskDelay

API void OspTaskDelay(u32 dwMseconds);

/*====================================================================
函数名：OspCreateMailbox
功能：创建消息队列
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspCreateMailbox(const char* szName, u32 dwMsgNumber, u32 dwMsgLength, MAILBOXID* ptQueueIDRead, MAILBOXID* ptQueueIDWrite);

/*====================================================================
函数名：OspCloseMailbox
功能：关闭邮箱
算法实现：（可选项）
引用全局变量：
输入参数说明：uQueueIDRead, uQueueIDWrite--邮箱的读、写句柄

 返回值说明：
====================================================================*/
API void OspCloseMailbox(MAILBOXID tQueueIDRead, MAILBOXID tQueueIDWrite);

/*====================================================================
函数名：OspSetSocketTimeOut
功能：android下设置邮箱的发送和接收超时时间(注意同一个fd只能调用一次，多次对同一个fd操作会产生问题)
算法实现：该函数只在安卓下起作用
引用全局变量：
输入参数说明：
nSocketFd  :OspCreateMailbox返回的读写端句柄
dwTimeout  :设置的超时时间，单位ms
bIsWriteId :是否是对写端进行设置 0表示设置读端，1表示设置写端

 返回值说明：
====================================================================*/
#ifdef _ANDROID_
API BOOL32 OspSetSocketTimeOut(s32 nSocketFd,s32 dwTimeout,BOOL32 bIsWriteId);
#endif

/*====================================================================
函数名        ：OspSndMsg
功能        ：向信箱发消息
算法实现    ：
引用全局变量：无
输入参数说明：tQueueID: 信箱ID,
lpMsgBuf: 内容指针,
len: 长度,
timeout: 超时值(ms). Windows下该参数无效, 邮箱满将一直阻塞直到有空的buffer.
VxWorks下, 缺省为2秒;
OSP_NO_WAIT(0): 邮箱满立即返回;
OSP_WAIT_FOREVER(-1): 邮箱满阻塞直到有空的buffer.

 返回值说明  ：成功返回TRUE, 失败或超时返回FALSE.
====================================================================*/
#ifdef __cplusplus
API BOOL32 OspSndMsg(MAILBOXID tQueueID, const char* szMsgBuf, u32 dwMsgLen, s32 nTimeout = 2000);
#else
API BOOL32 OspSndMsg(MAILBOXID tQueueID, const char* szMsgBuf, u32 dwMsgLen, s32 nTimeout);
#endif
//for uniplay
API BOOL32 OspMessage(u32 dwWriteID, const char* pchMsgBuf, u32 dwLen, int nTimeout);

/*====================================================================
函数名        ：OspRcvMsg
功能        ：从信箱收消息
算法实现    ：
引用全局变量：无
输入参数说明：
OSTOKEN tQueueID    信箱ID
u32  dwTimeout     超时(毫秒)
LPVOID *lpMsgBuf    返回消息内容指针
返回值说明    ：
====================================================================*/
API BOOL32 OspRcvMsg(MAILBOXID tQueueID, u32 dwTimeout, char * szMsgBuf, u32 dwMsgLen, u32* pdwLenRcved);

/*====================================================================
函数名：OspClkRateGet
功能：得到tick精度：n ticks/sec
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API u32 OspClkRateGet();

/*====================================================================
函数名：OspTickGet
功能：取得当前的tick数
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API u32 OspTickGet();

/*====================================================================
函数名：OspTickGet64
功能：取得当前的tick数, 64长度，解决溢出问题
算法实现：（可选项）
引用全局变量：
输入参数说明：
返回值说明：64位tick
说明:该功能仅在Linux, Windows上实现, VXWorks上返回0
====================================================================*/
API u64 OspTickGet64();

API void OspCpuShow();

//CPU信息
typedef struct
{
    // 空闲百分比，0－100
    u8 m_byIdlePercent;
}TOspCpuInfo;

/*====================================================================
函数名：OspGetCpuInfo
功能：取得当前的CPU信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptCpuInfo，用户CPU信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetCpuInfo(TOspCpuInfo* ptCpuInfo);

//内存信息
typedef struct
{
    //物理内存大小(KByte)
    u32 m_dwPhysicsSize;
    //已使用内存(KBytes)
    u32 m_dwAllocSize;
    //可用内存(KBytes)
    u32 m_dwFreeSize;
}TOspMemInfo;

/*====================================================================
函数名：OspGetMemInfo
功能：取得当前的Mem信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMemInfo，用户MEM信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetMemInfo( TOspMemInfo* ptMemInfo );
//内存信息
typedef struct
{
    //物理内存大小(KByte)
    u32 m_dwPhysicsSize;
    //可用内存(KBytes);
    u32 m_dwFreeSize;
    //缓冲区(KBytes)
    u32 m_dwBuffers;
    //缓存(KBytes)
    u32 m_dwCached;
    //Swap缓存(KBytes)
    u32 m_dwSwapCached;
}TOspMemInfoEx;

/*====================================================================
函数名：OspGetMemInfoEx
功能：取得当前的Mem信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptMemInfo，用户MEM信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetMemInfoEx( TOspMemInfoEx* ptMemInfo );

//时间信息
typedef struct
{
    //年(2006)
    u16 m_wYear;
    //月(1--12)
    u16 m_wMonth;
    //日(1--31)
    u16 m_wDay;
    //时(0--23)
    u16 m_wHour;
    //分(0--59)
    u16 m_wMinute;
    //秒(0--59)
    u16 m_wSecond;
}TOspTimeInfo;

/*====================================================================
函数名：OspGetTimeInfo
功能：取得当前的Time信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptTimeInfo，系统time信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetTimeInfo(TOspTimeInfo* ptTimeInfo);

/*====================================================================
函数名：OspSetTimeInfo
功能：设置当前的Time信息
算法实现：（可选项）
引用全局变量：
输入参数说明：ptTimeInfo，系统time信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspSetTimeInfo(TOspTimeInfo* ptTimeInfo);

//磁盘分区信息
typedef struct
{
    //磁盘分区大小(MByte)
    u32 m_dwPhysicsSize;
    //已使用磁盘分区大小(MBytes)
    u32 m_dwUsedSize;
    //可用磁盘分区大小(MBytes)
    u32 m_dwFreeSize;
}TOspDiskInfo;
//磁盘分区最大长度
#define MAX_PARTION_NAME_LENGTH        255
/*====================================================================
函数名：OspGetDiskInfo
功能：取得当前的磁盘分区信息
算法实现：（可选项）
引用全局变量：
输入参数说明：const char* pchPartionName， 分区名
                    win32 : "c:"
                    Vxworks : "/ata0/"
                    Linux : "/"
              ptDiskInfo，系统磁盘分区信息结构
返回值说明：为TRUE时表示获取信息成功，否则表示失败
====================================================================*/
API BOOL32 OspGetDiskInfo(const char* szPartionName, TOspDiskInfo* ptDiskInfo);

//////////////////////////////////////////////////////////////////////
//    套接字封装：用户使用套接字前必须先调用SockInit()对套接字库进行初始化，
//    使用完之后则需要调用SockCleanup()释放对套接字库的使用。由于vxWorks和
//    Windows的关闭套接字的函数不同，而且在调用socket、connect等函数时，返回
//    值类型也不一样，OSP也进行了必要的封装，原则是保持与Windows下基本一致。
/////////////////////////////////////////////////////////////////////

/*====================================================================
函数名：SockInit
功能：套接字初始化，封装Windows下的WSAStartup，vxWorks下返回TRUE
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
#define OspSockInit     SockInit
API BOOL32 SockInit();

/*====================================================================
函数名：SockShutdown
功能：对各平台shutdown的简单封装
算法实现：（可选项）
引用全局变量：
输入参数说明：hSock--要操作的套接字，
              dwMode--操作模式, 可取值为STOP_SEND, STOP_RECV或STOP_BOTH

返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
#ifdef _LINUX_
#define OspSockShutdown SockShutdown
API BOOL32 SockShutdown(SOCKHANDLE hSock, u32 dwMode);
#endif

/*====================================================================
函数名：SockClose
功能：关闭套接字，封装windows下的closesocket和vxworks下的close.
算法实现：（可选项）
引用全局变量：
输入参数说明：tSock--待关闭套接字的句柄（调用socket时返回）。

 返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
#define OspSockClose SockClose
API BOOL32 SockClose(SOCKHANDLE hSock);

/*====================================================================
函数名：SockCleanup
功能：套接字销毁，封装windows的WSACleanup，vxWorks下返回TRUE
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
#define OspSockCleanup SockCleanup
API BOOL32 SockCleanup();

// 串口封装：以Windows串口操作函数为准，在VxWorks下提供相应函数
/*====================================================================
函数名：OspSerialOpen
功能：打开指定的串口
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：成功返回打开串口的句柄，失败返回INVALID_SERIALHANDLE
====================================================================*/
API SERIALHANDLE OspSerialOpen(u8 byPort);

/*====================================================================
函数名：OspSerialClose
功能：关闭指定的串口
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspSerialClose(SERIALHANDLE hCom);

/*====================================================================
函数名：OspSerialRead
功能：从指定的串口读数据
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspSerialRead(SERIALHANDLE hCom, char * szBuf, u32 dwToRead, u32* pdwRead);

/*====================================================================
函数名：OspSerialWrite
功能：向指定的串口写数据
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspSerialWrite(SERIALHANDLE hCom, const char* szBuf, u32 dwToWrite, u32* pdwWrite);

/*====================================================================
函数名：OspSetCommTimeouts
功能：设置串口超时
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspSetCommTimeouts(SERIALHANDLE hCom, TOspCommTimeouts* ptCommTimeouts);

/*====================================================================
函数名：OspGetCommState
功能：获得串口状态
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspGetCommState(SERIALHANDLE hCom, TOspDCB* ptDCB);

/*====================================================================
函数名：OspSetCommState
功能：设置串口状态
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspSetCommState(SERIALHANDLE hCom, TOspDCB* ptDCB);

/*====================================================================
函数名：OspEscapeCommFunction
功能：使用串口扩展功能
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspEscapeCommFunction(SERIALHANDLE hCom, u32 dwFunc);

/*====================================================================
函数名：OspPurgeComm
功能：丢弃所有未决数据，终止串口读写操作
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspPurgeComm(SERIALHANDLE hCom, u32 dwFlags);

/*====================================================================
函数名：OspClearCommError
功能：与Windows下ClearCommError相同
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspClearCommError(SERIALHANDLE hCom, u32* pdwErrors, TOspComStat* ptStat);

/*====================================================================
函数名：OspGetCommTimeouts
功能：取得打开串口的超时设置
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：
====================================================================*/
API BOOL32 OspGetCommTimeouts(SERIALHANDLE hCom, TOspCommTimeouts* ptCommTimeouts);

/*====================================================================
函数名：OspAgentStart
功能：启动一个Osp测试代理, 其中包含一个测试服务器APP和一个客户端APP, 它们
主要不同是APP号, 消息处理过程都是一样的.
算法实现：（可选项）
引用全局变量：
输入参数说明：(in)port -- 端口号, 如无其他APP创建服务节点则用该端口创建之

 返回值说明：
====================================================================*/
API s32 OspAgentStart(u16 wPort);

/*====================================================================
函数名：OspMemPoolCreate
功能：创建一个内存池
算法实现：（可选项）
引用全局变量：
输入参数说明：pOspMemPoolHandle   指向内存池句柄指针
              ptMemPoolPara        内存池参数结构体的指针

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
#define OSP_MEMPOOLHANDLE       void*

typedef struct tagOspMemPoolPara
{
    u32  dwMaxFreePercent;        //最大空闲百分比，范围(0-100)，用于控制内存池中每一类内存空闲内存块与该类总内存块的
                                  //比例不超过dwMaxFreePercent，超出部分还回系统
                                  //其中0表示禁用内存池，100表示内存池销毁前不归还内存给系统，
    u32  dwLargeBlockLevelGrowSize;    //超过2M内存的水平粒度增长大小，单位M，默认值1,
	                                   //且当前栈能分配的最大内存块为(2+15*dwLargeBlockLevelGrowSize)M,可分配的大块内存按照
									   //(即默认为1是可分配的最大单块内存为17M)
                                        //(不超过2M部分的内存块粒度始终按照2的幂增长)
}TOspMemPoolPara;
API BOOL32 OspMemPoolCreate(TOspMemPoolPara* ptMemPoolPara,OSP_MEMPOOLHANDLE *pOspMemPoolHandle);


/*====================================================================
函数名：OspAllocMem
功能：从osp内部的内存池中分配内存块,最大能分配2M空间
算法实现：（可选项）
引用全局变量：
输入参数说明：size: 待分配的内存块大小，

 返回值说明：成功返回指向分配到的内存块的指针，失败返回NULL.
====================================================================*/
API void *OspAllocMem(size_t size);

/*====================================================================
函数名：OspAllocMemEx
功能：从内存池中分配内存块
算法实现：（可选项）
引用全局变量：
输入参数说明：size: 待分配的内存块大小，
              OspMemPoolHanle: 内存块所在的内存池句柄
                               默认为空的时候是在osp内存池对象上分配

 返回值说明：成功返回指向分配到的内存块的指针，失败返回NULL.
====================================================================*/
API void *OspAllocMemEx(size_t size,OSP_MEMPOOLHANDLE OspMemPoolHanle);


/*====================================================================
函数名：OspMemPoolStateShow
功能：显示当前内存池内存分配使用的状况
算法实现：（可选项）
引用全局变量：
输入参数说明：
              OspMemPoolHanle: 内存块所在的内存池句柄

 返回值说明：成功返回指向分配到的内存块的指针，失败返回NULL.
====================================================================*/
API void OspMemPoolStateShow(OSP_MEMPOOLHANDLE pOspMemPoolHandle);



/*====================================================================
函数名：OspFreeMem
功能：释放事先分配的内存块
算法实现：（可选项）
引用全局变量：
输入参数说明：token: 指向待释放的内存块的指针。

 返回值说明：无.
====================================================================*/
API void OspFreeMem(void* pvMemToken);


/*====================================================================
函数名：OspMemPoolDestroy
功能：销毁指定的内存池
算法实现：（可选项）
引用全局变量：
输入参数说明： OspMemPoolHanle: 待销毁的内存池句柄

 返回值说明：成功返回TRUE，失败返回FALSE.
====================================================================*/
API BOOL32 OspMemPoolDestroy(OSP_MEMPOOLHANDLE OspMemPoolHanle);


#define OSP_SAFE_FREE(p) { if (NULL != (p)) { OspFreeMem((p)); (p) = NULL; } }

/*====================================================================
函数名：OspSetPriRealTime
功能：把定时器、消息发送等任务提高到比系统net任务还高的优先级
算法实现：（可选项）
引用全局变量：
输入参数说明：

 返回值说明：无.
====================================================================*/
API void OspSetPriRealTime();

/*=============================================================================
函 数 名：OspAddrListGet
功      能：获取本机地址表内容
注      意：如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
本函数效率较低，不要重复调用。
算法实现：
全局变量：
参      数： u32     adwIP[] : [in/out]用户申请的用于地址表内容的数组的首地址
u16   wNum : [in]用户申请的数组的大小
返 回 值： 返回本机地址表中地址个数，错误时返回0
=============================================================================*/
API u16 OspAddrListGet(u32 adwIPList[], u16 wNum);

//系统限制参数
#define OSP_ETHERNET_ADAPTER_MAX_IP          16           /*每个接口上最大地址个数*/
#define OSP_ETHERNET_ADAPTER_MAX_NUM         32           /*每个设备最大接口数*/
#define OSP_ETHERNET_ADAPTER_NAME_LENGTH     64           /*每个接口名的最大长度*/
#define OSP_ETHERNET_ADAPTER_MAC_LENGTH      6            /*接口的MAC地址长度*/

//网卡状态,对应于TOSPEthernetAdapterInfo 结构的nState字段
#define OSP_ETHERNET_ADAPTER_UNKNOWN         0
#define OSP_ETHERNET_ADAPTER_UP              1
#define OSP_ETHERNET_ADAPTER_DOWN            2

/* 单个网卡信息 */
typedef struct {
    u32 nId;                                         /*网卡号*/
    u32 nState;                                     /*网卡状态*/
    u8 achName[OSP_ETHERNET_ADAPTER_NAME_LENGTH];         /*网卡名*/
    u8 achDescription[OSP_ETHERNET_ADAPTER_NAME_LENGTH];    /*网卡信息描述*/
    u8 achMacAddr[OSP_ETHERNET_ADAPTER_MAC_LENGTH];         /*网卡物理地址*/
    u32 nIpNum;                                     /*地址数量*/
    u32 anIp[OSP_ETHERNET_ADAPTER_MAX_IP];                 /*地址*/
}TOSPEthernetAdapterInfo;

/* 多个网卡信息 */
typedef struct {
    u32 nNum;                                                         /*网卡数量*/
    TOSPEthernetAdapterInfo atEthernetAdapter[OSP_ETHERNET_ADAPTER_MAX_NUM];/*网卡信息*/
}TOSPEthernetAdapterInfoAll;


/*=============================================================================
函 数 名：OspGetEthernetAdapterInfoAll
功      能：获取本系统所有以太网网卡信息
注      意：如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
本函数效率较低，不要重复调用。
算法实现：
全局变量：
参      数：TEthernetAdapterInfoAll 多网卡信息结构，参考数据结构的定义
返 回 值： TRUE/ FALSE 成功/失败
=============================================================================*/
API BOOL32 OspGetEthernetAdapterInfoAll(TOSPEthernetAdapterInfoAll* ptEthernetAdapterInfoAll);

/*=============================================================================
函 数 名：OspGetEthernetAdapterInfo
功      能：根据网卡号获取网卡信息
注      意：如果是在Windows下使用本函数，必须先调用OspInit，或者WSAStartup。
本函数效率较低，不要重复调用。
算法实现：
全局变量：
参      数：nEthAdapterId 网卡编号0-ETHERNET_ADAPTER_MAX_NUM，最多支持16个网卡
            TEthernetAdapterInfo 单网卡信息结构
返 回 值：TRUE/FALSE  成功/失败
=============================================================================*/
API BOOL32 OspGetEthernetAdapterInfo(u32 dwEthAdapterID, TOSPEthernetAdapterInfo* ptEthernetAdapterInfo);

/*=============================================================================
函 数 名：OspSetMemCheck
功      能：获取内存地址内容
注      意：
算法实现：
全局变量：
参      数： u32 * pdwCheckMem :    要检查的内存地址
u32 dwCheckValue ：  期望检查的值
BOOL32    bEqual ：当本参数为真时，表示要检查的内存地址内容不等于期望值时需要告警
当本参数为假时，表示要检查的内存地址内容等于期望值时需要告警
返 回 值： 成功返回TRUE, 失败返回FALSE.
=============================================================================*/
API BOOL32 OspSetMemCheck(u32* pdwCheckMem, u32 dwCheckValue, BOOL32 bEqual);

#ifdef __cplusplus
/* 内联成员函数或模板类成员函数的实现 */
template <class I, s32 maxins, class A, u8 maxAliasLen>
BOOL32 zTemplate<I, maxins, A, maxAliasLen>::InitAliasArray()
{
    for (s32 i = 0; i < maxins; i++)
    {
        instAliasArray[i].instId = 0xFFFF;
        instAliasArray[i].aliasRet = 0;
        instAliasArray[i].nextLoc = 0xFFFF;

        bakInstAliasArray[i].instId = 0xFFFF;
        bakInstAliasArray[i].aliasRet = 0;
        bakInstAliasArray[i].nextLoc = 0xFFFF;
    }
    curBakArrayNum = 0;

    return TRUE;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
u32 zTemplate<I, maxins, A, maxAliasLen>::AliasToWord(char * szAlias, u8 byLen)
{
    u32 dwRet = 0;
    for (u8 i = 0; i < byLen; i++)
    {
        dwRet =  (dwRet << 5) + dwRet + (*(szAlias + i));
    }

    dwRet = dwRet & (maxins-1);
    return dwRet;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
BOOL32 zTemplate<I, maxins, A, maxAliasLen>::IsInBakArray(u32 dwAtoW)
{
    /*首先检查主数组*/
    if (0xFFFF == instAliasArray[dwAtoW].instId)    /*主数组中这个位置还没有被占用*/
    {
        return FALSE;      /*这个实例存放到主数组中*/
    }
    else         /*主数组中这个位置还已经被占用*/
    {
        return TRUE;   /*这个实例存放到bak数组中的空闲位置*/
    }
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
s32 zTemplate<I, maxins, A, maxAliasLen>::GetInstanceNumber()
{
    return maxins;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
u8 zTemplate<I, maxins, A, maxAliasLen>::GetMaxAliasLen()
{
    return maxAliasLen;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
CInstance* zTemplate<I, maxins, A, maxAliasLen>::GetInstance(u16 wInsID)
{
    if (CInstance::DAEMON == wInsID)
    {
        return &daemIns;
    }

    if ((wInsID > 0) && (wInsID <= maxins))
    {
        return &insarray[wInsID - 1];
    }

    return NULL;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
BOOL32 zTemplate<I, maxins, A, maxAliasLen>::SetInstAlias(u16 wInsID, const char* szAlias, u8 byLen)
{
    u32  dwCycTime = 0;

    /*先把记录信息放入快速查找数组中*/
    if (CInstance::DAEMON != wInsID)
    {
        u32 dwSetRet = AliasToWord((s8*)szAlias, byLen);    /*通过别名计算设置值*/
        if (dwSetRet >= maxins)
        {
            return FALSE;
        }

        BOOL32 bIsBak = IsInBakArray(dwSetRet);    /*确定存放位置*/
        if (FALSE == bIsBak)     /*存放在主数组中*/
        {
            instAliasArray[dwSetRet].aliasRet = dwSetRet;
            instAliasArray[dwSetRet].instId = wInsID;
            instAliasArray[dwSetRet].nextLoc = 0xFFFF;   /*初始化*/
        }
        else         /*存放在bak数组中*/
        {
            u32 dwLoc = curBakArrayNum;
            /*把数据记录到bak数组相应位置中*/
            for (dwLoc = 0; dwLoc < maxins; dwLoc++)
            {
                if (0xFFFF == bakInstAliasArray[dwLoc].instId)
                {
                    break;
                }
            }
            if (dwLoc >= maxins)
            {
                return FALSE;
            }

            bakInstAliasArray[dwLoc].aliasRet = dwSetRet;
            bakInstAliasArray[dwLoc].instId = wInsID;
            bakInstAliasArray[dwLoc].nextLoc = 0xFFFF;   /*初始化*/

            /*把相同设置值的数据块连接起来*/
            if (0xFFFF == instAliasArray[dwSetRet].nextLoc)   /*主数组中尚未连接*/
            {
                instAliasArray[dwSetRet].nextLoc = dwLoc;    /*指向bak数组中存放位置*/
            }
            else
            {
                TInstAliasInfo * ptTempAlias = &bakInstAliasArray[instAliasArray[dwSetRet].nextLoc];
                if (NULL == ptTempAlias)
                {
                    return FALSE;
                }
                dwCycTime = 0;
                while (0xFFFF != ptTempAlias->nextLoc)
                {
                    if (ptTempAlias->nextLoc >= maxins)
                    {
                        return FALSE;
                    }
                    ptTempAlias = &bakInstAliasArray[ptTempAlias->nextLoc];
                    if (NULL == ptTempAlias)
                    {
                        return FALSE;
                    }
                    dwCycTime++;
                    if (dwCycTime > maxins)
                    {
                        OspPrintf(TRUE, FALSE, "SetInstAlias() return Cycle as dwCycTime=%d\n", dwCycTime);
                        return FALSE;
                    }
                }
                ptTempAlias->nextLoc = dwLoc; /*找到bak数组中最后一个相同设置值的位置*/
            }
            curBakArrayNum++;      /*bak数组存放个数增加*/
        }
    }

    CInstance* pcIns = GetInstance(wInsID);
    if ((NULL != pcIns) && (byLen <= maxAliasLen))
    {
        CInstance* pcOtherIns = FindInstByAlias(szAlias, byLen);
        if ((NULL != pcOtherIns) && (pcOtherIns->GetInsID() != pcIns->GetInsID()))
        {
            return FALSE;
        }
        memcpy(pcIns->m_alias, szAlias, byLen);
        pcIns->m_aliasLen = byLen;
        //return pcIns->SetAlias(szAlias, byLen);
    }
    return TRUE;
}

template <class I, s32 maxins, class A, u8 maxAliasLen>
CInstance* zTemplate<I, maxins, A, maxAliasLen>::FindInstByAlias(const char* szAlias, u8 byLen)
{
    /* 先看是否是Daemon实例 */
    u8 byAliasLen=0;
    u32 dwCycTime = 0;

    if (maxAliasLen < byLen)
    {
        return NULL;
    }

    s8 achliasBuf[MAX_ALIAS_LEN];
    memset(achliasBuf, 0, MAX_ALIAS_LEN);

    CInstance *pcIns = GetInstance(CInstance::DAEMON);
    if ((NULL != pcIns) && (pcIns->GetAlias(achliasBuf, maxAliasLen, &byAliasLen)))
    {
        if ((byAliasLen == byLen) && (memcmp(achliasBuf, szAlias, byLen) == 0))
        {
            return pcIns;
        }
    }

    /* 再看是否普通实例 */
    TInstAliasInfo* ptTempAlias = NULL;
    u32 dwSetRet = AliasToWord((s8*)szAlias, byLen);
    /*先查找主数组*/
    if (dwSetRet >= maxins)
    {
        return NULL;
    }
    ptTempAlias = &instAliasArray[dwSetRet];
    if (NULL == ptTempAlias)
    {
        return NULL;
    }
    pcIns = GetInstance(ptTempAlias->instId);
    if ((NULL != pcIns) && (pcIns->GetAlias(achliasBuf, maxAliasLen, &byAliasLen)))
    {
        if ((byAliasLen == byLen) && (0 == memcmp(achliasBuf, szAlias, byLen)))
        {
            return pcIns;
        }
    }
    /*再查找bak数组*/
    do
    {
        if ((NULL == ptTempAlias) || (ptTempAlias->nextLoc >= maxins))
        {
            return NULL;
        }
        ptTempAlias = &bakInstAliasArray[ptTempAlias->nextLoc];
        if (NULL == ptTempAlias)
        {
            return NULL;
        }
        CInstance * pcIns = GetInstance(ptTempAlias->instId);
        if ((NULL != pcIns) && (pcIns->GetAlias(achliasBuf, maxAliasLen, &byAliasLen)))
        {
            if ((byAliasLen == byLen) && (memcmp(achliasBuf, szAlias, byLen) == 0))
            {
                return pcIns;
            }
        }

        dwCycTime++;
        if (dwCycTime > maxins)
        {
            OspPrintf(TRUE, FALSE, "FindInstByAlias() return Cycle as dwCycTime=%d\n", dwCycTime);
            return NULL;
        }
    }while (0xFFFF != ptTempAlias->nextLoc);

    return NULL;
}

/*清除实例别名纪录信息*/
template <class I, s32 maxins, class A, u8 maxAliasLen>
BOOL32 zTemplate<I, maxins, A, maxAliasLen>::ClearInstAlias(u16 wInsID, const char* szAlias, u8 byLen)
{
    u32 dwCycTime = 0;

    if ((maxAliasLen < byLen) || (wInsID == CInstance::DAEMON))
    {
        return FALSE;
    }

    u32 dwSetRet = AliasToWord((char *)szAlias, byLen);      /*通过别名计算设置值*/
    if (dwSetRet >= maxins)
    {
        return FALSE;
    }
    /*先查找主数组，再查找备用数组*/
    if (instAliasArray[dwSetRet].instId == wInsID)
    {
        if (0xFFFF == instAliasArray[dwSetRet].nextLoc)   /*主数组中尚未连接*/
        {
            instAliasArray[dwSetRet].aliasRet = 0;       /*初始化*/
            instAliasArray[dwSetRet].instId = 0xFFFF;
            instAliasArray[dwSetRet].nextLoc = 0xFFFF;
        }
        else
        {
            TInstAliasInfo* ptTempAlias = &instAliasArray[dwSetRet];
            if (NULL == ptTempAlias)
            {
                return FALSE;
            }
            if (ptTempAlias->nextLoc >= maxins)
            {
                return FALSE;
            }
            TInstAliasInfo * ptRelocAlias = &bakInstAliasArray[ptTempAlias->nextLoc];
            if (NULL == ptRelocAlias)
            {
                return FALSE;
            }

            dwCycTime = 0;
            while (0xFFFF != ptRelocAlias->nextLoc)
            {
                ptTempAlias->aliasRet = ptRelocAlias->aliasRet;    //用下一块记录信息填充本块记录信息
                ptTempAlias->instId = ptRelocAlias->instId;

                if (ptRelocAlias->nextLoc >= maxins)
                {
                    return FALSE;
                }
                ptTempAlias = ptRelocAlias;         //位置移到下一块
                ptRelocAlias = &bakInstAliasArray[ptRelocAlias->nextLoc];
                if (NULL == ptRelocAlias)
                {
                    return FALSE;
                }

                dwCycTime++;
                if (dwCycTime > maxins)
                {
                    OspPrintf(TRUE, FALSE, "ClearInstAlias(1) return Cycle as dwCycTime=%d\n", dwCycTime);
                    return FALSE;
                }
            }

            ptTempAlias->aliasRet = ptRelocAlias->aliasRet;    //已到最后一块记录信息块
            ptTempAlias->instId = ptRelocAlias->instId;
            ptTempAlias->nextLoc = 0xFFFF;

            ptRelocAlias->aliasRet = 0;       //清除最后一块记录块
            ptRelocAlias->instId = 0xFFFF;
            ptRelocAlias->nextLoc = 0xFFFF;

            curBakArrayNum--;      /*bak数组存放个数减少*/
        }
    }
    else
    {
        if (0xFFFF == instAliasArray[dwSetRet].nextLoc)   /*主数组中尚未连接*/
        {
            return FALSE;
        }
        else
        {
            TInstAliasInfo * ptTempAlias = &bakInstAliasArray[instAliasArray[dwSetRet].nextLoc];
            if (NULL == ptTempAlias)
            {
                return FALSE;
            }
            TInstAliasInfo * ptRelocAlias = &(instAliasArray[dwSetRet]);

            dwCycTime = 0;
            while (ptTempAlias->instId != wInsID)
            {
                ptRelocAlias = ptTempAlias;

                if (0xFFFF != ptTempAlias->nextLoc)
                {
                    if (ptTempAlias->nextLoc >= maxins)
                    {
                        return FALSE;
                    }

                    ptTempAlias = &bakInstAliasArray[ptTempAlias->nextLoc];
                    if (NULL == ptTempAlias)
                    {
                        return FALSE;
                    }
                    dwCycTime++;
                    if (dwCycTime > maxins)
                    {
                        OspPrintf(TRUE, FALSE, "ClearInstAlias(2) return Cycle as dwCycTime=%d\n", dwCycTime);
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }

            /*找到bak数组中对应实例纪录位置，清除该位置的记录值，把记录链表重新连接*/
            if (0xFFFF == ptTempAlias->nextLoc)
            {
                if (NULL != ptRelocAlias)
                {
                    ptRelocAlias->nextLoc = 0xFFFF;
                }

                ptTempAlias->aliasRet = 0;     //该位置为本设置值的最后一位置，直接清除
                ptTempAlias->instId = 0xFFFF;
            }
            else
            {
                ptRelocAlias = &bakInstAliasArray[ptTempAlias->nextLoc];
                if (NULL == ptRelocAlias)
                {
                    return FALSE;
                }
                dwCycTime = 0;
                while (0xFFFF != ptRelocAlias->nextLoc)
                {
                    ptTempAlias->aliasRet = ptRelocAlias->aliasRet;    //用下一块记录信息填充本块记录信息
                    ptTempAlias->instId = ptRelocAlias->instId;

                    if (ptRelocAlias->nextLoc >= maxins)
                    {
                        return FALSE;
                    }

                    ptTempAlias = ptRelocAlias;         //位置移到下一块
                    ptRelocAlias = &bakInstAliasArray[ptRelocAlias->nextLoc];
                    if (NULL == ptRelocAlias)
                    {
                        return FALSE;
                    }
                    dwCycTime++;
                    if (dwCycTime > maxins)
                    {
                        OspPrintf(TRUE, FALSE, "ClearInstAlias(3) return Cycle as dwCycTime=%d\n", dwCycTime);
                        return FALSE;
                    }
                }
                ptTempAlias->aliasRet = ptRelocAlias->aliasRet;    //已到最后一块记录信息块
                ptTempAlias->instId = ptRelocAlias->instId;
                ptTempAlias->nextLoc = 0xFFFF;

                ptRelocAlias->aliasRet = 0;       //清除最后一块记录块
                ptRelocAlias->instId = 0xFFFF;
                ptRelocAlias->nextLoc = 0xFFFF;
            }

            curBakArrayNum--;      /*bak数组存放个数减少*/
        }
    }
    return TRUE;
}
#endif

/*/////////////////////////////////////////////////////////
//    loop buffer light version from toolbox 2008.6 baihao
/////////////////////////////////////////////////////////*/

#define            OSP_LB_ERR_BASE                  7000
#define            OSP_LB_ERR_OK                    0
#define            OSP_LB_ERR_PARAM                 OSP_LB_ERR_BASE + 1
#define            OSP_LB_ERR_NOMEM                 OSP_LB_ERR_BASE + 2
#define            OSP_LB_ERR_FULL                  OSP_LB_ERR_BASE + 3
#define            OSP_LB_ERR_EMPTY                 OSP_LB_ERR_BASE + 4
#define            OSP_LB_ERR_NOT_CREATE            OSP_LB_ERR_BASE + 5
#define            OSP_LB_ERR_CREATED               OSP_LB_ERR_BASE + 6
#define            OSP_LB_ERR_SYSTEM                OSP_LB_ERR_BASE + 7
#define            OSP_LB_ERR_INVALID_HANDLE        OSP_LB_ERR_BASE + 8
#define            OSP_LB_ERR_INVALID_MEM           OSP_LB_ERR_BASE + 9

#define OSP_LB_DATA_SIZE        108        // assume cpu 64 bits

/* due to thread safety and efficiency,
i use structure instead of a pointer to represent a handle */
typedef struct lb_data
{
/* user MUST NOT touch this while operate on loop buffer!!
    but you need to memset this to 0 before create loop buffer*/
    u8 rawdata[OSP_LB_DATA_SIZE];
} osplb_handle;

typedef struct lb_create_param
{
    s32 m_nBufSize;            /* in bytes */
    s32 m_nMaxUnitSize;        /* in bytes */
    u8  m_byRaceLvl;        /* useless in light version */
} osplb_create_param;

s16 osplb_create(osplb_handle* phHandle, osplb_create_param* ptParam);

s16 osplb_close(osplb_handle* phHandle);

s16 osplb_reset(osplb_handle* phHandle);

s16 osplb_write(osplb_handle* phHandle, u8* pbyBuf, s32 nBufSize);

s16 osplb_read(osplb_handle* phHandle, u8* pbyBuf, s32* pnBufSize);

s16 osplb_total_size(osplb_handle* phHandle, s32* pnSize);

s16 osplb_available_size(osplb_handle* phHandle, s32* pnSize);

s16 osplb_is_empty(osplb_handle* phHandle, u8* pbyEmpty);

API s32 OspSystem(const char* szCmd);    // 嵌入式上不要使用，getpid()问题

#define MAX_LIST_FILE_NUM             4096
#define MAX_LIST_FILE_NAME_LEN        128

typedef struct ListFileInfoTag
{
    u32 m_dwNum;
    s8 m_astrFileName[MAX_LIST_FILE_NUM][MAX_LIST_FILE_NAME_LEN];
} TListFileInfo;

s32 OspListFile(char * szFilePath, TListFileInfo* ptFileInfo);

#if defined(_MSC_VER) && !defined(_EQUATOR_)
    #pragma pack(pop)
#endif

#endif // _OSP_H
