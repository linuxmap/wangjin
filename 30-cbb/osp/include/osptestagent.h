#ifndef _OspTestAgent_h
#define _OspTestAgent_h

#include "./ospTest.h"

const u8 MAX_INS_NUM=10;
const u16 MAX_MSG_LENGTH=5000;

#define OSP_COMM_TEST_TIMER           1
#define OSP_COMM_TEST_TIMEOUT         OSP_COMM_TEST_TIMER

#define OSP_TIMER_TEST_TIMER          2
#define OSP_TIMER_TEST_TIMEOUT        OSP_TIMER_TEST_TIMER
#define OSP_TIMER_TEST_MULTI_TIMER    1000

#define IDLE_STATE                    0
#define COMM_TEST_STATE               1
#define LOG_TEST_STATE                2
#define TIMER_TEST_STATE              3

#define SERVER_PORT                   20000

const u8 OSP_SERVER_APP_PRI =  100;
const u8 OSP_CLIENT_APP_PRI =  100;
const u32 MAX_MSG_WAITING = 100;

#define OSPTESTAGENT_LOGFILE_DIR      "./OspAgent_log/"

#define MAX_RAWDATA_LEN    MAX_MSG_LEN
typedef struct
{
	TOspCommTestCmdEx tOspCommTestCmdEx;
	u8 rawData[MAX_RAWDATA_LEN];
}TOspCommTestPara;

class COspAgtIns : public CInstance
{
private:
	void OspTimerTest(void);
	void OspLogTest(void);
	void OspTxTest(void);
	void OspRxTest(const CMessage *pMsg);
    void InstanceEntry(CMessage *const pMsg);
	void InstInit(void)
	{
		memset(&m_tOspCommTestPara, 0, sizeof(TOspCommTestCmd));
		memset(&m_tOspCommStat, 0, sizeof(TOspCommTestCmd));
        m_dwStartTicks = 0;
		m_wCurLen = 0;
	}

private:
	TOspCommTestPara m_tOspCommTestPara;
	TOspCommStat m_tOspCommStat; // Ospͨ��״̬��¼
	u32 m_dwStartTicks; // �յ���һ��ʱ��ticks
	u32 m_dwSendStartTicks; // ���͵�һ��ʱ��ticks
	u16 m_wCurLen; // ����Ҫ���͵İ���
	u16 m_wExpectLen; // �ڴ����յİ���
	u32 m_dwSendMs; // ÿ�η��͵�ʵ�ʺ�ʱ

	TOspLogTestCmd m_tOspLogTestPara;
	TOspLogStat m_tOspLogStat; // Ospͨ��״̬��¼
	u32 m_dwBeginFileLogs; // ��ʼ����ʱ�ļ���־��
	u32 m_dwCurFileLogs;  // ��ǰ�ļ���־��
	u32 m_dwBeginScrnLogs; // ��ʼ����ʱ��Ļ��־��
	u32 m_dwCurScrnLogs;  // ��ǰ��Ļ��־��

	TOspTimerTestCmd m_tOspTimerTestPara;
	TOspTimerStat m_tOspTimerStat;
	u32 m_dwTimerTestStartTicks; // ��ʼ��־����ʱ��ϵͳtick��
	u32 m_dwTimerTestTimes;
};

typedef zTemplate< COspAgtIns, MAX_INS_NUM, CAppNoData, MAX_ALIAS_LENGTH > COspAgtApp;

#endif /* _OspTestAgent_h */
