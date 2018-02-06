/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��Ϣ���͵���Ҫ����ͷ�ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
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
		//Դ���
		u32 srcnode;
		//Ŀ�Ľ��
		u32 dstnode;
		//Ŀ��Ӧ��ʵ��
		u32 dstid;
		//ԴĿ��ʵ��
		u32 srcid;
		//��Ϣ����
		u16 type;
		//��Ϣ��
		u16 event;
		//��Ϣ�峤��
		u16 length;
		//��Ϣ��
		u32 content;

		//���֧��ͬ����Ϣ
#ifdef SYNCMSG
		//ͬ����ϢӦ��
		u32 output;
		//ͬ����ϢӦ�𳤶�
		u16 outlen;
		//δʹ��
		u16 expire;
#endif

		//ʵ������
		u32 dstAlias;
		//ʵ����������
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
	//������
#if defined(_VXWORKS_) || (defined(_LINUX_) && !defined(_EQUATOR_))
	__attribute__ ((packed))
#endif
		CMessageForSocket;

//ͬ����Ϣ
#define  OSPEVENT_BASE					60001
//ͬ����ϢӦ��
#define  OSPEVENT_SYNCACK_MSG			(OSPEVENT_BASE + 0)

//������ԭ����
// Heartbeat��
#define NODE_DISC_REASON_HBFAIL			(u8)1
// ����ʧ�ܶ�
#define NODE_DISC_REASON_SENDERR		(u8)2
// ����ʧ�ܶ�
#define NODE_DISC_REASON_RECVERR		(u8)3
// App������
#define NODE_DISC_REASON_BYAPP			(u8)4

// ÿ�������Ϣ���ݵ�16���ֽ�
#define MSGTRCBYTES_PER_LINE			16
// ����������������: ��ÿ��80�ַ�����, ��ȥΪ��Ϣͷ�������4��
#define MAX_MSGTRC_LINES				( (MAX_LOG_MSG_LEN/80) - 4 )
// ȱʡ���������
#define DEF_MSGTRC_LINES				10

#define MAX_MSGBLK_NUM					400
#define MSGBLK_FLAG						0xabcdef

//������ʱ��֪ͨʵ����������
#define NODE_MAX_CBNUM					(int)32
// 100ms; ���Ͷ��������Ϣʱ��timeoutֵ
#define NODEMAN_MSG_TIMEOUT				(int)100

// 256K; SOCKET�ķ��ͻ����С
#define SOCKET_SEND_BUF					1024 * 256
// 256K; SOCKET�Ľ��ջ����С
#define SOCKET_RECV_BUF					1024 * 256


//�Ե��������ʽ��ӡip��ַ�������ڶ��̻߳�����ʹ��inet_ntoa()
//����: sprintf(calledAddr, "TA:%u.%u.%u.%u:%u", OspQuadAddr(ip), port)
#if !defined OspQuadAddr
#define OspQuadAddr(ip) ((u8 *)&(ip))[0], \
                     ((u8 *)&(ip))[1], \
                     ((u8 *)&(ip))[2], \
                     ((u8 *)&(ip))[3]
#endif


//ǰ������
class CDispatchTask;
class CDispatchPool;
class COsp;
extern COsp g_Osp;


//Ӧ�ó�(Ӧ�ú���Ӧ�ö����ӳ��)
class CAppPool
{
public:
    CAppPool();

	//����Ӧ�úŲ�ѯӦ�ö���
    CApp* AppGet(u16 appId)
    {
		if( ( appId == 0 ) ||
			( appId > MAX_APP_NUM ) )
		{
			return NULL;
		}

		return m_apcAppRegTable[appId-1];
    }

	//����Ӧ����������ѯӦ�ö���
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

	//����Ӧ�úŲ�ѯӦ������������
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

	//����Ӧ�úŲ�ѯӦ��������д���
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

	//����Ӧ�ñ���
    void AppNameSet(u16 appId, const char *pName)
    {
        CApp* pApp;
        pApp = AppGet(appId);
        if ( pApp != NULL )
            pApp->SetName( pName );
    }

	//��ӡӦ�õ�ͳ����Ϣ
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
				//��ӡAPP�ż�����
				if( pcApp->pAppName != NULL )
				{
					OspPrintf( TRUE, FALSE, "app %d : \"%s\"", i+1, pcApp->pAppName );
				}
				else
				{
					OspPrintf( TRUE, FALSE, "app %d :", i+1 );
				}

				//Ӧ�����������Ϣ����
				OspPrintf( TRUE , FALSE , "\n\tmaxWaiting = %d\n" , pcApp->maxMsgWaiting );
				//Ӧ�ý�����Ϣ����
				OspPrintf( TRUE , FALSE , "\tmsgInCome = %d\n" , pcApp->GetMsgIncomeNum() );
				//Ӧ���Ѵ�����Ϣ����
				OspPrintf( TRUE , FALSE , "\tmsgProcessed = %d\n" , pcApp->msgProcessed );
				//Ӧ�������д��������Ϣ������ֵ
				OspPrintf( TRUE , FALSE , "\tmsgWaitingTop = %d\n" , pcApp->msgWaitingTop );
				//����������������Ϣ����
				OspPrintf( TRUE , FALSE , "\tmsgdropped = %d\n" , pcApp->msgdropped );
				OspPrintf( TRUE , FALSE , "\tbakmsg = %d\n" , pcApp->GetBakMsgNum() );
				//��Ļlog����
				OspPrintf( TRUE , FALSE , "\tscrnLogLevel = 0x%x\n" , pcApp->scrnLogFlag );
				//��Ļtrc��־
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

	//��ӡָ��Ӧ�õ�ʵ����ͳ����Ϣ
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
			//���ʵ�������ڿ���״̬
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
            //ʵ��״̬Ǩ����Ϣ��ӡ
            pcInstance->InstStateInfoShow(wCurrInsNo);
       	}
        //�ص�����ִ����Ϣ��ӡ
        pcApp->CallBackInfoShow();
		//ʵ����Ҫ��Ϣ��ӡ
		pcApp->InstInfoShow();
	}

	//��ӡӦ�ó�������Ӧ�õ�ʵ����ͳ����Ϣ
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

	//��ӡָ��Ӧ��ָ��ʵ���Զ�����Ϣ
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
		//��ʾ����ʵ�����Զ�����Ϣ
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
		//��ʾ�ǿ���ʵ�����Զ�����Ϣ
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
		//��ʾָ��ʵ�����Զ�����Ϣ
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
	//Ӧ�ó�
    CApp *m_apcAppRegTable[MAX_APP_NUM];

	//ȫ���ļ�trc��־
	u16 m_wGloFileTrc;

	//ȫ����Ļtrc��־
	u16 m_wGloScrTrc;
};

//���(��Ӧ��Զ�˵�һ��TCP��·)
class CNodeItem
{
public:
	//����ʼ��
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

	//��������Ϣ��������
    void msgRcvedInc()
    {
        m_dwMsgRcved++;
    }

	//��㷢����Ϣ��������
    void msgSendedInc()
    {
        m_dwMsgSended++;
    }

	//��ѯ������Ϣ����
	inline void *GetRcvdData()
	{
		return m_pvRcvdData;
	}

	//���ý�����Ϣ����
	inline void SetRcvdData(void *pvRcvdData)
	{
		m_pvRcvdData = pvRcvdData;
	}

	//��ѯ������Ϣ����
	inline u32 GetRcvdLen()
	{
		return m_dwRcvdLen;
	}

	//���ý�����Ϣ����
	inline void SetRcvdLen(u32 dwRcvdLen)
	{
		m_dwRcvdLen = dwRcvdLen;
	}

public:
	//����Ƿ���Ч
    BOOL32 m_bValid;
	//���ս�����Ϣ�����Ƿ���ձ�������Ϣ
    BOOL32 m_bListenning;
	//���������·�ĶԶ�IP��ַ
    u32 m_dwIpAddr;
	//���������·��Socket
    SOCKHANDLE m_tSockHandle;
	//��������Ҫ֪ͨ��Ӧ��ʵ����
    u16 m_wDiscInformAppId[NODE_MAX_CBNUM];
    u16 m_wDiscInformInsId[NODE_MAX_CBNUM];
	//��������Ҫ֪ͨ��ʵ��������
	u8 m_bDiscInformNum;
	//��������Ϣ����
    u32 m_dwMsgRcved;
	//��㷢����Ϣ����
    u32 m_dwMsgSended;
	//�������ʱ����(��)
	u16 m_wDisTime;
	//���ϴζ�������ʱ���(��)
	u16 m_wDisTimeUsed;
	//��·��⹦���Ƿ�ʹ��
    BOOL32 m_bDiscCheckEnable;
	//��������Ƿ��յ���Ӧ
	BOOL32 m_bMsgEchoAck;
	//��������ʱ��������δ��ӦHeartbeat����
	u8 m_byDisconnHBs;
	//δ��ӦHeartbeat����
	u8 m_byDisconnHBsused;
	//������Ϣ����
	void *m_pvRcvdData;
	//������Ϣ����(����ospͷ)
	u32 m_dwRcvdLen;
    BOOL32 m_bCMessageCompressSupport;//by wubin 2011-02-22
    // ��㷢�ͺ�ʱ���ֵ(tick)
    u32 m_dwMaxSendTicks; //by wubin 2013-3-6
    // ��㷢�ͺ�ʱ������Ϣ��
    u16 m_wMaxSendEvent; //by wubin 2013-3-6
    // ��㷢�ͺ�ʱ������Ϣ����
    u16 m_wMaxSendMsgLen; //by wubin 2013-3-6
};

//����
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

	//��ӡ����ͳ����Ϣ
	void Show();
	//�����ڴ�
	BOOL32 Alloc(u32 dwMaxNodeNum, u32 dwMaxDispatchMsg);
	//���س�ʼ��
	BOOL32 Initialize(void);
	//���ݽ��Ų�ѯ������
    CNodeItem *NodeGet(u32 nodeId);
	//���ע��
    BOOL32 Regist(u32 dwIpAddr, SOCKHANDLE dwSock, u32 *pdwNodeId, u16 uHb,u8 byHbNum);
	//���ע��
    BOOL32 NodeUnRegist(u32 node, u8 reason);
	//�رս��socket
    void NodeSockClose(u32 dwNode);
	//ֹͣ��ָ����������Ϣ
    BOOL32 NodeDisRcv(u32 node);
	//���ݽ��Ų�ѯ���socket
    BOOL32 GetSock(u32 nodeId, SOCKHANDLE *phSock);
	//����������Ƿ�ʹ��
	BOOL32 IsNodeCheckEnable(u32 nodeId);
	//ʹ��ָ�����������
	void NodeCheckEnable(u32 nodeId);
	//ָֹͣ�����������
	void NodeCheckDisable(u32 nodeId);
	//����ָ��������������
	BOOL32 SetHBParam(u32 nodeId, u16 uHb, u8 byHbNum);
	//�������
	void Scan(void);
    void UpdateMaxSend(u32 dwNode, u32 dwMaxSendTicks, u16 wMaxSendEvent, u16 wMaxSendLen); // add by wubin 2013-03-06
public:
	//����
	CNodeItem * m_acNodeRegTable;
	//���������Ӧ��
	CNodeManApp m_cNodeManApp;
	//����������socket
    SOCKHANDLE m_tListenSock;
	//�ڲ�socket,����ֻ���ڷ��ͻ���PostDeamon����Ϣ
    SOCKHANDLE m_tLocalInSock;
	//�ڲ�socket,����ֻ���ڽ��ջ���PostDeamon����Ϣ
    SOCKHANDLE m_tLocalOutSock;
	//�����ź�������
    SEMHANDLE m_tSemaNodePool;
	//�����������˿�
	u16 m_wListenPort;
	//�ܶ�������
	u16 m_wNodeDisconnTimes;
	//�����������Ӧ���µĶ�������
	u16 m_wNodeHBDiscnTimes;
	//Ӧ�������������µĶ�������
	u16 m_wNodeDiscnByApp;
	//socket����ʧ�ܵ��µĶ�������
	u16 m_wNodeDiscnByRecvFailed;
	//socket����ʧ�ܵ��µĶ�������
	u16 m_wNodeDiscnBySendFailed;
	//DispatchTask������Ϣ����trc��־
    u32 m_dwSendTrcFlag;
	//PostDaemon������Ϣ����trc��־
    u32 m_dwRcvTrcFlag;
    u32 m_dwGloFailDispNode;
    u32 m_dwGloFailPostNode;
    u32 m_dwGloFailPostApp;
};

//������Ϣ����
class CDispatchTask
{
public:
	//������Ϣ������
	TASKHANDLE m_hTask;
	//������Ϣ�����
	u32 m_dwTaskID;
	//������Ϣ������������
    MAILBOXID m_dwReadQue;
	//������Ϣ��������д���
    MAILBOXID m_dwWriteQue;
	//������Ϣ����
	u32 m_dwMaxMsgWaiting;
	//���͵���Ϣ����
    u32 m_dwMsgIncome;
	//�ѷ��ͳɹ�����Ϣ����
    u32 m_dwMsgProcessed;
	//�����л�����Ϣ�����ķ�ֵ
    u32 m_dwMsgWaitingTop;

	//ռ�ñ�����ĵ�ǰ���
    u32 m_dwNodeId;
	//�������������п��ܱ����������ʵĳ�Ա����
    SEMHANDLE m_tSemMutex;
	//������Ϣ�����ָ��
    CDispatchPool *pDispatchPool;

public:
    CDispatchTask(void);
	//������Ϣ�����ʼ��
	BOOL32 Initialize(void);
	//������Ϣ�����˳�
	void Quit(void);
	//������Ϣ
    void NodeMsgSendToSock(void);
	//���͵���Ϣ��������
	void MsgIncomeInc(void);
	//���͵���Ϣ�����ݼ�
	void MsgIncomeDec(void);
	//��ѯ�����д����͵���Ϣ��
	u32 MsgWaitingNum(void);
	//��ѯ���͵���Ϣ����
	u32 MsgIncomeNum(void);
};

//��Ϣ�����������
const u8 THREADNUM = 1;
//������Ϣ�����
class CDispatchPool
{
public:
	//������Ϣ����
	CDispatchTask m_acDispTasks[THREADNUM];
	//������Ϣ������ź�������
    SEMHANDLE m_tSemTaskFull;

public:
	//������Ϣ����س�ʼ��
    BOOL32 Initialize(void);
	//������Ϣ������˳�
	void Quit(void);
	//��ӡ������Ϣ�����ͳ����Ϣ
    void Show(void);
	//������Ϣ
    s32 NodeMsgPost(u32 dstid, const char *content, u32);
};

//������Ϣ����س�ʼ��
API BOOL32 DispatchSysInit(void);
//������Ϣ������˳�
API void DispatchSysExit(void);
//������Ϣ����������
API void DispatchTaskEntry(CDispatchTask *pDispTask);
//����Ӧ�úŲ�ѯӦ������������
API MAILBOXID RcvQueIdFind(u16 appId);

//������Ϣ����������
API int PostDaemon();
//�������socket
API void SvrFdSet(fd_set * set);

//��������ת��ospͷ����������
API void MsgHton(CMessageForSocket *pcMsg);
//��������ת��ospͷ����������
API void MsgNtoh(CMessageForSocket *pcMsg);

//������Ϣʱת��Ŀ�Ľ��ź�Դ����
API void MsgIdChange(CMessageForSocket *pMsg, u32 node);
//��������socket
API SOCKHANDLE CreateTcpNodeNoRegist(u32,u16 uPort,BOOL32 bTcpNodeReuse=FALSE);
//ת����Ϣ���ݣ�׼����ӡ
API u32 MsgDump2Buf(char *buf, int bufLen, CMessage *ptMsg);
//�����ص�
API void NodeDiscCallBack(u32 nodeId, u16 appId, u16 insId);
//����socket���仯������PostDaemon
API void SockChangeNotify(void);
//��Ϊ��ʹ��һЩ���ܱ��������Ż����ĺ���������
API void OspCmdFuncEnable(void);
//�첽��Ϣ����
API int OspPostMsg( u32 dstid, u16  event, const void* content=0, u16  length=0,
             u32 dstnode=0, u32 srciid=0, u32 srcnode=0,
			 BOOL32 bDroppable=TRUE, u8 type=MSG_TYPE_ASYNC, int nTimeout = 2000 );
//�첽��Ϣ����
int OspPostMsg(const char* pchDstAlias, u8 byDstAliasLen, u16 wDstAppID, u32 dwDstNode,
				u16  uEvent, const void* pvContent=0, u16  uLength=0,
				u32 dwSrcIId=0, u32 dwSrcNode=0,
				BOOL32 bDroppable=TRUE, u8 type=MSG_TYPE_ASYNC);

API u32 OspNodeLastIpGet(u32 dwNodeId);

//<--��Ϣѹ��by wubin 2011-02-22
//ѹ����Ϊ�ѹ���Ϊ���ݰ�(content ����ͷ��)��CMessage
API BOOL32 OspCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen);
//��ѹCMessage
API BOOL32 OspUnCompressMessagePack(CMessage ** ppMsg, u32 *pdwMsgLen);
//��ȡѹ�������ݳ������ֵ
API u32 OspGetCompressLengthBound(u32 wUncompressedLength, u8 byAlgorithm = 0);
//ѹ������
API BOOL32 OspCompressData(u8* pbyCompressedBuf, u32* pdwCompressedBufLen, u8* pbyUncompressedBuf, u32 dwUncompressedBufLen, u8 byAlgorithm = 0);
//��ѹ����
API BOOL32 OspUnCompressData(u8* pbyUncompressedBuf, u32* pdwUncompressedBufLen, u8* pbyCompressedBuf, u32 dwCompressedBufLen, u8 byAlgorithm = 0);
API u16 OspGetOriginEvent(CMessage * pcMsg);
//by wubin 2011-02-22-->
#endif // OSP_POST_H