/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSPLOG.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP ��־,���ٹ��ܵ���Ҫ����ͷ�ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98	 1.0        ĳĳ        ------------
2015/06/30   2.0         �˲���      ����OSP_CLASSLOG GLBLOG��־��¼�ӿ�
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

//ȫ·������󳤶�
#define MAX_FILE_DIR_NAME_LEN    (u32)200

//��־�ļ�������󳤶�
#define MAX_FILENAME_LEN         (u32)200

//ÿ����־����󳤶�
#define MAX_LOG_MSG_LEN          (u32)6000

//����¼���
#define MAX_EVENT_COUNT          (u16)0xffff

//��־���ͣ���������־
#define LOG_TYPE_MASKABLE        0

//��־���ͣ�����������־
#define LOG_TYPE_UNMASKABLE      1

#ifdef _LINUX_
#define SNPRINTF snprintf
#else
#define SNPRINTF _snprintf
#endif


//��־�ṹ 2015/06/30
//�ܹ��������ָ��ģ���ָ���ֶ�
//��־�Ƿ񱣴浽��־��ȡ����g_byLogFileLev�����ã���־levelֵС�ڵ���g_byLogFileLev�ģ����ᱣ�浽�ļ�
//��־�Ƿ�����Ļ����ʾ
//    ����ָ����ģ�����־��ȡ����g_cModuleLogLev[byModule].dwLogLev[], ��־levelС�ڵ��ڵģ��������Ļ
//    ����δָ��ģ�����־��ȡ����g_byGlbLogPrintLev, ��־levelС�ڵ��ڵģ��������Ļ
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

    s8 m_achBodyField[MAX_BODY_FIELD_LEN];       //��־����
    s8 m_achModLev[MAX_MOD_LEVEL_STR_LEN];          //��־ģ�鼶�� ͨ���ò�������ģ�����־�Ƿ�����Ļ���
    s8 m_achOspPrifixField[MAX_PRIFIXED_FIELD_LEN];  //ospǰ׺(�߳�����-app-inst-task-state)
    s8 m_achCompileField[MAX_COMPILE_FIELD_LEN];    //������Ϣ
    u8     m_byLogLev;           //��־����
    BOOL32 m_bIsPrintScreen;     //�Ƿ���Ļ���
};

#pragma pack(1)
//��־��ӡ��Ϣͷ
typedef struct
{
	//����(������Ļ��ʾ�Ƿ������) LOG_TYPE_UNMASKABLE/LOG_TYPE_MASKABLE
	u8 type;
	//��ӡ����Ļ
    BOOL32 bToScreen;
	//��ӡ���ļ�
    BOOL32 bToFile;
	//��ӡ��Ϣ����
    u32 dwLength;
}TOspLogHead;
#pragma pack()


//�¼�����
class COspEventDesc
{
public:
    //�¼�����
    char * EventDesc[MAX_EVENT_COUNT];
    //�����¼�����
    void DescAdd(const char * szDesc, u16 wEvent);
    //��ѯ�¼�����
    char * DescGet(u16 wEvent);
    //����¼�����
    void Destroy();
    //��ʼ���¼�����
    void COspEventInit();
    COspEventDesc();
    ~COspEventDesc();
};

//Ӧ������
class COspAppDesc
{
public:
	//Ӧ������
    char * AppDesc[MAX_APP_NUM];
	//����Ӧ������
    void DescAdd(const char * szDesc, u16 wAppID);
	//���Ӧ������
	void Destroy(void);
    COspAppDesc();
	~COspAppDesc();
};

//��־ϵͳ��ʼ��
API BOOL32 LogSysInit(void);

//��־ϵͳ�̺߳���
API void LogTask();

//��־���
API void OspLogQueWrite(TOspLogHead tOspLogHead, const char * szContent, u32 dwLen);

//��־��ӡ
API void OspLog (u8 byLevel, const char * szFormat, ...);
API void OspDumpPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...);
API void OspTrcPrintf(BOOL32 bScreen, BOOL32 bFile, const char * szFormat, ...);
API void OspMsgTrace(BOOL32 bScreen, BOOL32 bFile, const char * szContent, u32 dwLen);
API void OspUniformPrintf(TOspLogContent& cLogContent);

//��Telnet�ͻ�����Ļ�����û�����.
API void OspLogTeleCmdEcho(const char *pchCmdStr, u32 wLen);

//����ʮ�е���Ϣ�����Ƿ���� TRUE ���  FALSE �����
API inline BOOL32 IsOspLogLongMsgPrintEnbl(void);

//��ѯ�ļ���־��Ϣ�ۻ�����
API u32 OspFileLogNum(void);

//��ѯ��Ļ��־��Ϣ�ۻ�����
API u32 OspScrnLogNum(void);

//��ѯ��ǰ��־�ļ���
API u32 OspLogFileNo(void);

#endif // OSP_LOG_H
