#ifndef _OspTest_h
#define _OspTest_h

#include "osp.h"
//#include "kdvsys.h"

/**************************************************************
 ��׼���Խӿڣ�������OSP�������������͸��ֲ�������
***************************************************************/
// ���Դ��������/�ͻ��˶˿ں�
#define OSP_AGENT_SERVER_PORT                      20000
#define OSP_AGENT_CLIENT_PORT                      20001

// OSP���Դ�������
#define OSP_AGENT_TYPE_SERVER                      0
#define OSP_AGENT_TYPE_CLIENT                      1

// OSP��������
#define OSP_REQ_TYPE_SERVER                        0
#define OSP_REQ_TYPE_CLIENT                        1

// �������Ͷ���
#define OSP_TEST_TYPE_BASE                         0 
#define OSP_TEST_TYPE_SERIAL                       (OSP_TEST_TYPE_BASE + 0)     // ���ڲ���
#define OSP_TEST_TYPE_NET                          (OSP_TEST_TYPE_BASE + 1)     // ���ڲ���
#define OSP_TEST_TYPE_TASK                         (OSP_TEST_TYPE_BASE + 2)
#define OSP_TEST_TYPE_SEMA                         (OSP_TEST_TYPE_BASE + 3)
#define OSP_TEST_TYPE_MSGQ                         (OSP_TEST_TYPE_BASE + 4)
#define OSP_TEST_TYPE_NODE                         (OSP_TEST_TYPE_BASE + 5)
#define OSP_TEST_TYPE_APP                          (OSP_TEST_TYPE_BASE + 6)
#define OSP_TEST_TYPE_TIMER                        (OSP_TEST_TYPE_BASE + 7)
#define OSP_TEST_TYPE_COMM                         (OSP_TEST_TYPE_BASE + 8)
#define OSP_TEST_TYPE_LOG                          (OSP_TEST_TYPE_BASE + 9)
#define OSP_TEST_TYPE_END                          (OSP_TEST_TYPE_BASE + 255)

// OSP������Ϣ, �ڲ�ʹ��
OSPEVENT(OSP_TEST_REQ,      0x10);		// ��������
OSPEVENT(OSP_TEST_REQ_ACK,  0x11);      // ����Ӧ��
OSPEVENT(OSP_TEST_CMD,      0x12);		// ��������
OSPEVENT(OSP_TEST_QRY,      0x13);      // ״̬��ѯ
OSPEVENT(OSP_COMM_TEST,     0x14);      // ͨ�Ų�����Ϣ
OSPEVENT(OSP_COMM_TEST_ACK, 0x15);      // ����Ӧ��
OSPEVENT(OSP_COMM_TEST_END, 0x16);      // ���Խ���
OSPEVENT(OSP_COMM_TEST_RAWDATA, 0x17);
// OSP��������ṹ
#define MAX_TEST_REQ_PARA_LEN     100
typedef struct
{
	u32 type; // ��������
	u8 para[MAX_TEST_REQ_PARA_LEN]; // ����
}TOspTestReq;

// OSP��������ṹ
#define MAX_TEST_CMD_PARA_LEN     5500
typedef struct
{
	u32 type; // ��������
	u8 para[MAX_TEST_CMD_PARA_LEN]; // ����
}TOspTestCmd;

/* ͨ�Ų��Բ��� */
const u16 OSP_TEST_SERVER_APP_ID  =  120;
const u16 OSP_TEST_CLIENT_APP_ID  =  121;

// ͨ�Ų�����������ṹ
typedef struct
{
	u32 agtType;    // Ŀ��APP����: ������APP���ǿͻ���APP
	u32 reqType;    // ������Ƿ�����ʵ�����ǿͻ���ʵ��
}TOspCommTestReq;

// ��������Ӧ��: server -> client
#define MAX_ALIAS_LENGTH 20
typedef struct
{
	u32 iid;    // �Զ�ʵ��ID
	u8 aliasLen; // �Զ˱�������
	char achAlias[MAX_ALIAS_LENGTH]; // �Զ˱���	
}TOspTestReqAck;

/* ͨ�Ų����������: ���Ͳ��� */

// ���ͺ������Ͷ���
enum OspCommFunc
{
	NoAliasGPost,  // ʹ��ʵ��ID��ȫ��post����
	AliasGPost,    // ʹ�ñ�����ȫ��post����
	NoAliasGSend,  // ʹ��ʵ��ID��ȫ��send����
	AliasGSend,    // ʹ�ñ�����ȫ��send����
	NoAliasPost,   // ʹ��ʵ��ID��ʵ��post����
	AliasPost,     // ʹ�ñ�����ʵ��post����
	NoAliasSend,   // ʹ��ʵ��ID��ʵ��send����
	AliasSend      // ʹ�ñ�����ʵ��send����
};

#define COMM_TEST_TYPE_SINGLEINS        // ins��ͨ��
#define COMM_TEST_TYPE_SINGLEAPP        // app��ͨ��
#define COMM_TEST_TYPE_SINGLENODE       // node��ͨ��
#define COMM_TEST_TYPE_MULTINODE        // node��ͨ��

// �û�ʹ��OspTestCmd����ʱ��Ҫ����Ĳ���
typedef struct
{
	u32 reqType;   // ����--�Ƿ����ͻ�ʵ��, ���Ƿ���ʵ��������

/* ���Ͳ��� */
	u32 ip;         // �Զ�ip��ַ
	u16 port;       // �Զ˶˿ں�
	
	u8  funcType;   // ���ͺ������
	u32 times;      // ���ʹ���
	u16 period;     // ��������(ms)
	u16 packets;    // ÿ������������
    u16 minLen;     // ��С����
	u16 maxLen;     // ������

/* ���ղ��� */
	BOOL bChkLenErr; // ��������?
	BOOL bChkConErr; // ������ݴ�?

/* ԭʼ�������� */
	u16 rawDataLen; // ԭʼ�������ݳ���
}TOspCommTestCmd;

// OspTestCmdʵ��ʱʵ��ʹ�õĽṹ
typedef struct
{
/* ���Ͳ��� */
	u32 peeriid;    // �Զ�ʵ��ID
	u8 aliasLen; // �Զ˱�������
	char achAlias[MAX_ALIAS_LENGTH]; // �Զ˱���
    TOspCommTestCmd tOspCommTestCmd;
}TOspCommTestCmdEx;

/* ͨ��״̬ */
typedef struct
{
	u32 sendTimes;
	u32 sendPackets;   // ������
	u32 succSendPackets; // �ɹ�������
	u32 sendTimeouts; // ���ͳ�ʱ��
	u32 sendBytes; // ����Byte��
	u32 totalMS; // �����ܺ�ʱ(ms)
}TOspSendStat;    

typedef struct
{
	u32 recvPackets;    // �հ���
	u32 recvLenError;   // ���ȴ�
	u32 recvContError;  // ���ݴ�
	u32 recvBytes; // ��ȷ����Byte��, �������ȴ�
	u32 totalMS; // �����ܺ�ʱ(ms)
}TOspRecvStat;

typedef struct
{
	TOspSendStat tOspSendStat;  // ����ͳ��
    TOspRecvStat tOspRecvStat;  // ����ͳ��
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
// ���ڲ�����������ṹ
typedef struct
{
	u8 type;
	u8 port;
	u16 baudRate;
	u32 bytes2rw;
	char buf2rw[MAX_SERIAL_DATA];
}TSerTestCmd;

// ����״̬���ݽṹ, ��Ϊ��ѯ����Ľ����������
typedef struct
{
	u8 port;     // ��ǰ�����Ķ˿�
	BOOL opened;  // �Ƿ�ɹ���
	BOOL readSuc; // ��ǰ�������Ƿ�ɹ�
	BOOL writeSuc; // ��ǰд�����Ƿ�ɹ�
	u16 baudRate; // ������	
	u32 bytesWrite; // д����u8��
    u32 bytesRead; // �����u8��
	char readBuf[MAX_SERIAL_DATA]; // ����������    
}TSerialVar;

// ��ʱ�������йؽṹ
const u8 TIMER_TEST_TYPE_ABS  = 0;
const u8 TIMER_TEST_TYPE_REL  = 1;
const u8 TIMER_TEST_TYPE_KILL = 2;
const u8 TIMER_TEST_TYPE_MULTI = 3;
typedef struct
{
	u32 times;              // ���Դ���
	u8 type;                // 0--���Զ�ʱ���ԣ� 1--��Զ�ʱ����,  2--��ʱɱ������,  3--�ඨʱ������
	u32 intval;             // ʱ����(ms)
	/*���Զ�ʱ������*/
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

// ��־�����йؽṹ
const u8 LOG_FUNC_TYPE_GLOB = 0;
const u8 LOG_FUNC_TYPE_INS = 1;

const u8 LOG_TYPE_FILE = 0;
const u8 LOG_TYPE_SCRN = 1;

#define MAX_LOGMSG_LEN    2000
typedef struct
{
	u8 logType;             // 0 -- �ļ�; 1 -- ��Ļ
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
��������OspTestBuild
���ܣ�Ϊָ���Ĵ��⹦�ܽ������Ի���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����(in) node -- ����Ŀ��(Target)�Ľ���, 
              (in) type -- �������Ļ������
			  (in) param -- ���⹦���������

����ֵ˵�����ɹ�����0��ʧ�ܷ���OSP_ERROR
====================================================================*/
API int OspTestBuild(u32 node, u32 type, void *param);

/*====================================================================
��������OspTestCmd
���ܣ����Ͳ��������Լ���һ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����(in) node -- ����Ŀ��(Target)�Ľ���, 
              (in) type -- �������
			  (in) param -- �������

����ֵ˵�����ɹ�����OSP_OK��ʧ�ܷ���OSP_ERROR
====================================================================*/
API int OspTestCmd(u32 node, u32 type, void *param);

/*====================================================================
��������OspTestQuery
���ܣ���ѯ���Խ��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����(in) node -- ����Ŀ��(Target)�Ľ���, 
              (in) type -- ����ѯ�������
			  (in, out) param -- ��Ų�ѯ�����buf


����ֵ˵������
====================================================================*/
API int OspTestQuery(u32 node, u32 type, void *param);

#endif
