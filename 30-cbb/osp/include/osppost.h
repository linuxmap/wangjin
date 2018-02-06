/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP 消息传送的主要包含头文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98		1.0      某某        ------------
******************************************************************************/

#ifndef OSP_POST_H
#define OSP_POST_H

#include "osp.h"
#include "osplog.h"
#include "ospvos.h"
#include "stdio.h"
#include "ospsch.h"
#include "../include/ospnodeman.h"

	#if !defined ioctl
	#define ioctl ioctlsocket
	#endif
	typedef unsigned long * ioctlOnPtrTypeCast;         /* AL - 11/25/98 */

	typedef struct CMessageForSocket
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
		u32 content;

		//如果支持同步消息
#ifdef SYNCMSG
		//同步消息应答
		u32 output;
		//同步消息应答长度
		u16 outlen;
		//未使用
		u16 expire;
#endif

		//实例别名
		u32 dstAlias;
		//实例别名长度
		u8 dstAliasLen;
		static u32 GetPackedSize()
		{
			return sizeof(u32) + sizeof(u32) + sizeof(u32) + sizeof(u32) + sizeof(u16) +
				sizeof(u16) + sizeof(u16) + sizeof(/*u8 **/u32) +
#ifdef SYNCMSG
				sizeof(/*u8 **/u32) + sizeof(u16) + sizeof(u16) +
#endif
				sizeof(/*char **/u32) + sizeof(u8);
		};
	}
	//紧排列
#if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
	__attribute__ ((packed))
#endif
		CMessageForSocket;

//同步消息
#define  OSPEVENT_BASE					60001
//同步消息应答
#define  OSPEVENT_SYNCACK_MSG			(OSPEVENT_BASE + 0)

//结点断链原因定义
// Heartbeat断
#define NODE_DISC_REASON_HBFAIL			(u8)1
// 发送失败断
#define NODE_DISC_REASON_SENDERR		(u8)2
// 接收失败断
#define NODE_DISC_REASON_RECVERR		(u8)3
// App主动断
#define NODE_DISC_REASON_BYAPP			(u8)4

// 每行输出消息内容的16个字节
#define MSGTRCBYTES_PER_LINE			16
// 允许输出的最大行数: 按每行80字符计算, 除去为消息头部输出的4行
#define MAX_MSGTRC_LINES				( (MAX_LOG_MSG_LEN/80) - 4 )
// 缺省输出的行数
#define DEF_MSGTRC_LINES				10

#define MAX_MSGBLK_NUM					400
#define MSGBLK_FLAG						0xabcdef

//结点断链时可通知实例的最大个数
#define NODE_MAX_CBNUM					(int)32
// 100ms; 发送断链检测消息时的timeout值
#define NODEMAN_MSG_TIMEOUT				(int)100

// 256K; SOCKET的发送缓冲大小
#define SOCKET_SEND_BUF					1024 * 256
// 256K; SOCKET的接收缓冲大小
#define SOCKET_RECV_BUF					1024 * 256


//以点分整数形式打印ip地址，避免在多线程环境中使用inet_ntoa()
//用例: sprintf(calledAddr, "TA:%u.%u.%u.%u:%u", OspQuadAddr(ip), port)
#if !defined OspQuadAddr
#define OspQuadAddr(ip) ((u8 *)&(ip))[0], \
                     ((u8 *)&(ip))[1], \
                     ((u8 *)&(ip))[2], \
                     ((u8 *)&(ip))[3]
#endif


//前置声明
class CDispatchTask;
class CDispatchPool;
class COsp;
extern COsp g_Osp;


//应用池(应用号与应用对象的映射)
class CAppPool
{
public:
    CAppPool();

	//根据应用号查询应用对象
    CApp* AppGet(u16 appId)
    {
		if( ( appId == 0 ) ||
			( appId > MAX_APP_NUM ) )
		{
			return NULL;
		}

		return m_apcAppRegTable[appId-1];
    }

	//根据应用任务句柄查询应用对象
    CApp* FindAppByTaskID( u32 taskID )
    {
		for( u16 wIndex = 0; wIndex < MAX_APP_NUM; wIndex++ )
		{
			if( ( m_apcAppRegTable[wIndex] != NULL ) &&
				( taskID == m_apcAppRegTable[wIndex]->taskID ) )
			{
				return m_apcAppRegTable[wIndex];
			}
		}
        return NULL;
    }

	//根据应用号查询应用主邮箱读句柄
    MAILBOXID RcvQueIdFind( u16 appId )
    {
		if( ( appId <= 0 ) ||
			( appId > MAX_APP_NUM ) ||
			( m_apcAppRegTable[appId-1] == NULL ) )
		{
			return 0;
		}

        return m_apcAppRegTable[appId-1]->queRcvId;
    }

	//根据应用号查询应用主邮箱写句柄
    MAILBOXID SendQueIdFind(u16 appId)
    {
		if( ( appId <= 0 ) ||
			( appId > MAX_APP_NUM ) ||
			( m_apcAppRegTable[appId-1] == NULL ) )
		{
			return 0;
		}
        return m_apcAppRegTable[appId-1]->queSendId;
    }

	//设置应用别名
    void AppNameSet(u16 appId, const char *pName)
    {
        CApp* pApp;
        pApp = AppGet(appId);
        if ( pApp != NULL )
            pApp->SetName( pName );
    }

	//打印应用的统计信息
    void Show()
    {
        u32 i;
        CApp *pcApp;
		u32 dwLineCount = 0;

        for( i = 0; i < MAX_APP_NUM; i++ )
        {
			pcApp = m_apcAppRegTable[i];
            if(pcApp != NULL)
            {
				//打印APP号及别名
				if( pcApp->pAppName != NULL )
				{
					OspPrintf( TRUE, FALSE, "app %d : \"%s\"", i+1, pcApp->pAppName );
				}
				else
				{
					OspPrintf( TRUE, FALSE, "app %d :", i+1 );
				}

				//应用邮箱最大消息容量
				OspPrintf( TRUE , FALSE , "\n\tmaxWaiting = %d\n" , pcApp->maxMsgWaiting );
				//应用接收消息总数
				OspPrintf( TRUE , FALSE , "\tmsgInCome = %d\n" , pcApp->GetMsgIncomeNum() );
				//应用已处理消息总数
				OspPrintf( TRUE , FALSE , "\tmsgProcessed = %d\n" , pcApp->msgProcessed );
				//应用邮箱中待处理的消息总数峰值
				OspPrintf( TRUE , FALSE , "\tmsgWaitingTop = %d\n" , pcApp->msgWaitingTop );
				//因邮箱满丢弃的消息总数
				OspPrintf( TRUE , FALSE , "\tmsgdropped = %d\n" , pcApp->msgdropped );
				OspPrintf( TRUE , FALSE , "\tbakmsg = %d\n" , pcApp->GetBakMsgNum() );
				//屏幕log级别
				OspPrintf( TRUE , FALSE , "\tscrnLogLevel = 0x%x\n" , pcApp->scrnLogFlag );
				//屏幕trc标志
				OspPrintf( TRUE , FALSE , "\tscrnTraceFlag = 0x%x\n\n" , pcApp->scrnTraceFlag );

				dwLineCount += 8;
            }
			if( dwLineCount > 256 )
			{
				OspTaskDelay( 256 );
				dwLineCount = 0;
			}
        }
    }

	//打印指定应用的实例的统计信息
	void InstanceShow( u16 aid )
	{
		CApp *pcApp;
		int InstNum;
		u16 wCurrInsNo;
		CInstance* pcInstance = NULL;
        char *pchAlias = NULL;
		u8 byAliasLen = 0;
		u32 dwCurState = 0;
		u32 dwLineCount = 0;

		pcApp = AppGet( aid );
		if( pcApp == NULL )
		{
			OspPrintf( TRUE, FALSE, "this application is not exist!\n" );
			return;
		}

		OspPrintf( TRUE, FALSE, "application: %s\n", pcApp->pAppName );
		InstNum = pcApp->GetInstanceNumber();
		OspPrintf( TRUE, FALSE, "Instance num in this application is : %d\n", InstNum );

		for( wCurrInsNo = 1; wCurrInsNo <= InstNum; wCurrInsNo++ )
		{
			pcInstance = pcApp->GetInstance( wCurrInsNo );
			if( pcInstance == NULL )
			{
				continue;
			}

			pchAlias = pcInstance->m_alias;
			byAliasLen = pcInstance->m_aliasLen;
			dwCurState = pcInstance->CurState();
			//如果实例不处于空闲状态
			if( dwCurState != 0 )   //if the instance is not ain IDLE state
			{
				if( ( pchAlias != NULL ) &&
					( byAliasLen > 0 ) )
				{
					OspPrintf(TRUE, FALSE, "App is: %d, instance: %d, alias: %s, instance state: %d\n" ,
						aid, wCurrInsNo, pchAlias, dwCurState );
				}
				else
				{
					OspPrintf(TRUE, FALSE, "App is: %d, instance: %d, instance state: %d\n" ,
						aid, wCurrInsNo, dwCurState );
				}
				dwLineCount++;
			}
			if( dwLineCount > 256 )
			{
				OspTaskDelay( 256 );
				dwLineCount = 0;
			}
            //实例状态迁移信息打印
            pcInstance->InstStateInfoShow(wCurrInsNo);
       	}
        //回调函数执行信息打印
        pcApp->CallBackInfoShow();
		//实例简要信息打印
		pcApp->InstInfoShow();
	}

	//打印应用池中所有应用的实例的统计信息
	void InstanceShowAll()
    {
        u16 i;

        CApp *pcApp;
		OspPrintf( TRUE, FALSE, "\n" );
        OspPrintf(TRUE , FALSE , "print instance info:\n");
	    OspPrintf(TRUE , FALSE , "--------------------------\n");
        for( i = 0; i < MAX_APP_NUM; i++ )
        {
			pcApp = AppGet( i );
			if( pcApp != NULL )
			{
				OspPrintf( TRUE, FALSE, "the app id is :  %d\n", i );
				InstanceShow( i );
				OspPrintf( TRUE, FALSE, "\n\n" );
			}
        }
		OspPrintf( TRUE, FALSE, "\n" );
    }

	//打印指定应用指定实例自定义信息
	void InstanceDump( u16 aid, u16 InstId, u32 param )
	{
		CApp *pcApp;
		int InstNum;
		u16  CurrInsNum;
		CInstance* pcInstance = NULL;

		pcApp = AppGet( aid );
		if( pcApp == NULL )
		{
			OspPrintf( TRUE, FALSE, "this application is not exist!\n" );
			return;
		}

		InstNum = pcApp->GetInstanceNumber();
		//显示所有实例的自定义信息
		if( InstId == CInstance::PENDING )
		{
			for( CurrInsNum = 1; CurrInsNum <= InstNum; CurrInsNum++ )
			{
				pcInstance = pcApp->GetInstance( CurrInsNum );
				if( pcInstance != NULL )
				{
					pcInstance->InstanceDump( param );
				}
			}
		}
		//显示非空闲实例的自定义信息
		else if( InstId == CInstance::EACH )
		{
			for( CurrInsNum = 1; CurrInsNum <= InstNum; CurrInsNum++ )
			{
				pcInstance = pcApp->GetInstance(CurrInsNum);
				if(pcInstance != NULL && pcInstance->CurState() != 0)
				{
					pcInstance->InstanceDump(param);
				}
			}
		}
		//显示指定实例的自定义信息
		else
		{
			pcInstance = pcApp->GetInstance(InstId);
			if(pcInstance != NULL)
			{
				if(InstId == CInstance::DAEMON)
				{
					pcInstance->DaemonInstanceDump(param);
				}
				else
				{
					pcInstance->InstanceDump(param);
				}
			}
		}
	}

public:
	//应用池
    CApp *m_apcAppRegTable[MAX_APP_NUM];

	//全局文件trc标志
	u16 m_wGloFileTrc;

	//全局屏幕trc标志
	u16 m_wGloScrTrc;
};

//结点(对应与远端的一条TCP链路)
class CNodeItem
{
public:
	//结点初始化
	void Initialize(void)
	{
		m_bValid = FALSE;
		m_bDiscInformNum = 0;
		for( int InformNum = 0; InformNum < NODE_MAX_CBNUM; InformNum++ )
        {
			m_wDiscInformAppId[InformNum] = INVALID_APP;
			m_wDiscInformInsId[InformNum] = INVALID_INS;
		}

		m_tSockHandle = INVALID_SOCKET;
		m_dwIpAddr = 0;

		m_dwMsgRcved = 0;
		m_dwMsgSended = 0;

		m_wDisTime = 0;
		m_wDisTimeUsed = 0;
		m_bDiscCheckEnable = TRUE;
		m_bMsgEchoAck = FALSE;
		m_byDisconnHBs = 0;
		m_byDisconnHBsused = 0;

		m_pvRcvdData = NULL;
		m_dwRcvdLen = 0;
        m_bCMessageCompressSupport = FALSE; //by wubin 2011-02-22
        //<--
        m_dwMaxSendTicks = 0;
        m_wMaxSendEvent = 0;
        m_wMaxSendMsgLen = 0;
        //-->by wubin 2013-3-6
	}

	//结点接收消息总数递增
    void msgRcvedInc()
    {
        m_dwMsgRcved++;
    }

	//结点发送消息总数递增
    void msgSendedInc()
    {
        m_dwMsgSended++;
    }

	//查询接收消息缓冲
	inline void *GetRcvdData()
	{
		return m_pvRcvdData;
	}

	//设置接收消息缓冲
	inline void SetRcvdData(void *pvRcvdData)
	{
		m_pvRcvdData = pvRcvdData;
	}

	//查询接收消息长度
	inline u32 GetRcvdLen()
	{
		return m_dwRcvdLen;
	}

	//设置接收消息长度
	inline void SetRcvdLen(u32 dwRcvdLen)
	{
		m_dwRcvdLen = dwRcvdLen;
	}

public:
	//结点是否有效
    BOOL32 m_bValid;
	//接收结点间消息任务是否接收本结点的消息
    BOOL32 m_bListenning;
	//结点代表的链路的对端IP地址
    u32 m_dwIpAddr;
	//结点代表的链路的Socket
    SOCKHANDLE m_tSockHandle;
	//结点断链需要通知的应用实例号
    u16 m_wDiscInformAppId[NODE_MAX_CBNUM];
    u16 m_wDiscInformInsId[NODE_MAX_CBNUM];
	//结点断链需要通知的实例的总数
	u8 m_bDiscInformNum;
	//结点接收消息总数
    u32 m_dwMsgRcved;
	//结点发送消息总数
    u32 m_dwMsgSended;
	//断链检测时间间隔(秒)
	u16 m_wDisTime;
	//与上次断链检测的时间差(秒)
	u16 m_wDisTimeUsed;
	//链路检测功能是否使能
    BOOL32 m_bDiscCheckEnable;
	//断链检测是否收到响应
	BOOL32 m_bMsgEchoAck;
	//保持连接时允许的最大未响应Heartbeat次数
	u8 m_byDisconnHBs;
	//未响应Heartbeat次数
	u8 m_byDisconnHBsused;
	//结点间消息缓冲
	void *m_pvRcvdData;
	//结点间消息长度(包括osp头)
	u32 m_dwRcvdLen;
    BOOL32 m_bCMessageCompressSupport;//by wubin 2011-02-22
    // 结点发送耗时最大值(tick)
    u32 m_dwMaxSendTicks; //by wubin 2013-3-6
    // 结点发送耗时最大的消息号
    u16 m_wMaxSendEvent; //by wubin 2013-3-6
    // 结点发送耗时最大的消息长度
    u16 m_wMaxSendMsgLen; //by wubin 2013-3-6
};

//结点池
class CNodePool
{
public:
	CNodePool()
	{
		m_tListenSock = INVALID_SOCKET;
		m_tLocalInSock = INVALID_SOCKET;
		m_tLocalOutSock = INVALID_SOCKET;
		m_acNodeRegTable = NULL ;
	}

	~CNodePool()
	{
		OspSemDelete(m_tSemaNodePool);
		if ( NULL != m_acNodeRegTable)
			delete [] m_acNodeRegTable;
	}

	//打印结点池统计信息
	void Show();
	//分配内存
	BOOL32 Alloc(u32 dwMaxNodeNum, u32 dwMaxDispatchMsg);
	//结点池初始化
	BOOL32 Initialize(void);
	//根据结点号查询结点对象
    CNodeItem *NodeGet(u32 nodeId);
	//结点注册
    BOOL32 Regist(u32 dwIpAddr, SOCKHANDLE dwSock, u32 *pdwNodeId, u16 uHb,u8 byHbNum);
	//结点注销
    BOOL32 NodeUnRegist(u32 node, u8 reason);
	//关闭结点socket
    void NodeSockClose(u32 dwNode);
	//停止从指定结点接收消息
    BOOL32 NodeDisRcv(u32 node);
	//根据结点号查询结点socket
    BOOL32 GetSock(u32 nodeId, SOCKHANDLE *phSock);
	//结点断链检测是否使能
	BOOL32 IsNodeCheckEnable(u32 nodeId);
	//使能指定结点断链检测
	void NodeCheckEnable(u32 nodeId);
	//停止指定结点锻炼检测
	void NodeCheckDisable(u32 nodeId);
	//设置指定结点断链检测参数
	BOOL32 SetHBParam(u32 nodeId, u16 uHb, u8 byHbNum);
	//断链检测
	void Scan(void);
    void UpdateMaxSend(u32 dwNode, u32 dwMaxSendTicks, u16 wMaxSendEvent, u16 wMaxSendLen); // add by wubin 2013-03-06
public:
	//结点池
	CNodeItem * m_acNodeRegTable;
	//结点断链检测应用
	CNodeManApp m_cNodeManApp;
	//对外服务监听socket
    SOCKHANDLE m_tListenSock;
	//内部socket,现在只用于发送唤醒PostDeamon的消息
    SOCKHANDLE m_tLocalInSock;
	//内部socket,现在只用于接收唤醒PostDeamon的消息
    SOCKHANDLE m_tLocalOutSock;
	//结点池信号量保护
    SEMHANDLE m_tSemaNodePool;
	//对外服务监听端口
	u16 m_wListenPort;
	//总断链次数
	u16 m_wNodeDisconnTimes;
	//断链检测无响应导致的断链次数
	u16 m_wNodeHBDiscnTimes;
	//应用主动断链导致的断链次数
	u16 m_wNodeDiscnByApp;
	//socket接收失败导致的断链次数
	u16 m_wNodeDiscnByRecvFailed;
	//socket发送失败导致的断链次数
	u16 m_wNodeDiscnBySendFailed;
	//DispatchTask发送消息任务trc标志
    u32 m_dwSendTrcFlag;
	//PostDaemon接收消息任务trc标志
    u32 m_dwRcvTrcFlag;
    u32 m_dwGloFailDispNode;
    u32 m_dwGloFailPostNode;
    u32 m_dwGloFailPostApp;
};

//发送消息任务
class CDispatchTask
{
public:
	//发送消息任务句柄
	TASKHANDLE m_hTask;
	//发送消息任务号
	u32 m_dwTaskID;
	//发送消息任务邮箱读句柄
    MAILBOXID m_dwReadQue;
	//发送消息任务邮箱写句柄
    MAILBOXID m_dwWriteQue;
	//邮箱消息容量
	u32 m_dwMaxMsgWaiting;
	//发送的消息总数
    u32 m_dwMsgIncome;
	//已发送成功的消息总数
    u32 m_dwMsgProcessed;
	//邮箱中缓存消息条数的峰值
    u32 m_dwMsgWaitingTop;

	//占用本任务的当前结点
    u32 m_dwNodeId;
	//用来保护本类中可能被多个任务访问的成员变量
    SEMHANDLE m_tSemMutex;
	//发送消息任务池指针
    CDispatchPool *pDispatchPool;

public:
    CDispatchTask(void);
	//发送消息任务初始化
	BOOL32 Initialize(void);
	//发送消息任务退出
	void Quit(void);
	//发送消息
    void NodeMsgSendToSock(void);
	//发送的消息总数递增
	void MsgIncomeInc(void);
	//发送的消息总数递减
	void MsgIncomeDec(void);
	//查询邮箱中待发送的消息数
	u32 MsgWaitingNum(void);
	//查询发送的消息总数
	u32 MsgIncomeNum(void);
};

//消息发送任务个数
const u8 THREADNUM = 1;
//发送消息任务池
class CDispatchPool
{
public:
	//发送消息任务
	CDispatchTask m_acDispTasks[THREADNUM];
	//发送消息任务池信号量保护
    SEMHANDLE m_tSemTaskFull;

public:
	//发送消息任务池初始化
    BOOL32 Initialize(void);
	//发送消息任务池退出
	void Quit(void);
	//打印发送消息任务池统计信息
    void Show(void);
	//发送消息
    s32 NodeMsgPost(u32 dstid, const char *content, u32);
};

//发送消息任务池初始化
API BOOL32 DispatchSysInit(void);
//发送消息任务池退出
API void DispatchSysExit(void);
//发送消息任务主函数
API void DispatchTaskEntry(CDispatchTask *pDispTask);
//根据应用号查询应用主邮箱读句柄
API MAILBOXID RcvQueIdFind(u16 appId);

//接收消息任务主函数
API int PostDaemon();
//加入接收socket
API void SvrFdSet(fd_set * set);

//从主机序转换osp头部至网络序
API void MsgHton(CMessageForSocket *pcMsg);
//从网络序转换osp头部至主机序
API void MsgNtoh(CMessageForSocket *pcMsg);

//接收消息时转换目的结点号和源结点号
API void MsgIdChange(CMessageForSocket *pMsg, u32 node);
//创建监听socket
API SOCKHANDLE CreateTcpNodeNoRegist(u32,u16 uPort,BOOL32 bTcpNodeReuse=FALSE);
//转换消息内容，准备打印
API u32 MsgDump2Buf(char *buf, int bufLen, CMessage *ptMsg);
//断链回调
API void NodeDiscCallBack(u32 nodeId, u16 appId, u16 insId);
//接收socket集变化，唤醒PostDaemon
API void SockChangeNotify(void);
//人为地使得一些可能被编译器优化掉的函数被调用
API void OspCmdFuncEnable(void);
//异步消息发送
API int OspPostMsg( u32 dstid, u16  event, const void* content=0, u16  length=0,
             u32 dstnode=0, u32 srciid=0, u32 srcnode=0,
			 BOOL32 bDroppable=TRUE, u8 type=MSG_TYPE_ASYNC, int nTimeout = 2000 );
//异步消息发送
int OspPostMsg(const char* pchDstAlias, u8 byDstAliasLen, u16 wDstAppID, u32 dwDstNode,
				u16  uEvent, const void* pvContent=0, u16  uLength=0,
				u32 dwSrcIId=0, u32 dwSrcNode=0,
				BOOL32 bDroppable=TRUE, u8 type=MSG_TYPE_ASYNC);

API u32 OspNodeLastIpGet(u32 dwNodeId);

//<--消息压缩by wubin 2011-02-22
//压缩成为已构造为数据包(content 紧接头部)的CMessage
API BOOL32 OspCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen);
//解压CMessage
API BOOL32 OspUnCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen);
//获取压缩后数据长度最大值
API u32 OspGetCompressLengthBound(u32 wUncompressedLength, u8 byAlgorithm = 0);
//压缩数据
API BOOL32 OspCompressData(u8* pbyCompressedBuf, u32* pdwCompressedBufLen, u8* pbyUncompressedBuf, u32 dwUncompressedBufLen, u8 byAlgorithm = 0);
//解压数据
API BOOL32 OspUnCompressData(u8* pbyUncompressedBuf, u32* pdwUncompressedBufLen, u8* pbyCompressedBuf, u32 dwCompressedBufLen, u8 byAlgorithm = 0);
API u16 OspGetOriginEvent(CMessage * pcMsg);
//by wubin 2011-02-22-->
#endif // OSP_POST_H