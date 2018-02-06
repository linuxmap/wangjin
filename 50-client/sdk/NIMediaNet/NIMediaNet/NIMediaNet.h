#ifndef _NI_MEDIA_NET_H_
#define _NI_MEDIA_NET_H_

#define	NI_MEDIA_NET_API	extern "C" __declspec(dllexport)

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;

/************************************��Ϣ���************************************/

#define MEDIA_NET_GET_LOGIN_RESULT					100
#define	MEDIA_NET_GET_RESOURCE_RESULT				101
#define	MEDIA_NET_GET_VIDEORECORD_RESULT			102

/*************************************������*************************************/

#define MEDIA_NET_NOERROR							0	/* û�д��� */
#define MEDIA_NET_PASSWORD_ERROR					1	/* �û������������ */
#define MEDIA_NET_NOUGHPRI							2	/* Ȩ�޲��� */
#define MEDIA_NET_NOINIT							3   /* û�г�ʼ�� */
#define MEDIA_NET_NETWORK_FAIL_CONNECT				4	/* ���ӷ�����ʧ�� */
#define MEDIA_NET_MEMORY_NOT_ENOUGH					5	/* �ڴ治�� */
#define MEDIA_NET_LOCAL_INIT_ERROR					6	/* ���س�ʼ������ */

/************************************�ص�����************************************/

typedef void(__stdcall *fExcuteResultCallBack)
	(LONG lUserID,uint32_t uMsgID,uint32_t uResult,uint8_t *pUser);
typedef void(__stdcall *fRealDataCallBack)
	(LONG lRealHandle,uint32_t uDataType,uint8_t *pBuffer,uint32_t dwBufSize,uint8_t *pUser);

/***********************************ͨ�ýṹ��***********************************/

typedef struct  
{
	char					sServerAddress[16];								/* �����IPv4��ַ */
	uint16_t				wServerPort;									/* ����˶˿� */
	char					sServerPassword[32];							/* ��������� */
	char					sServerSipID[21];								/* ����˹���Ψһ���� */
	char					sServerDomainID[21];							/* ���������� */
	uint32_t				uServerKeepInterval;							/* ������������ */
	uint32_t				uServerKeepMaxCnt;								/* ����������������,�����ô���,��·���Ͽ� */
	char					sClientAddress[16];								/* �ͻ���IPv4��ַ */
	uint16_t				wClientPort;									/* �ͻ��˶˿� */
	char					sClientSipID[21];								/* �ͻ��˹���Ψһ���� */
	char					sClientDomainID[21];							/* �ͻ�������� */
	uint32_t				uSessionLingerTime;								/* �Ự����ʱ�䣨ms�� */
	fExcuteResultCallBack	cbExcuteResult;									/* ִ�н����Ϣ�ص� */
	uint8_t*				cbUser;											
}MEDIA_NET_LOGIN_INFO,*LPMEDIA_NET_LOGIN_INFO;

typedef struct 
{
	HWND					hPlayWnd;										/* ���ھ�������ΪNULL����ʾ������Ȼ������ */
	uint32_t				uShowMode;										/* ������ʾ��ʽ��0��GDI��ͼ 1��DirectX��ͼ */
	char					sResourceID[21];								/* ��Դ����Ψһ���� */
	fRealDataCallBack		cbRealData;										/* ʵʱ�������ݻص� */
	uint32_t				uMediaType;										/* ʵʱ�������ݻص�ý������ 0��Ĭ��������h.264��h.265�ȱ������ݣ� 1: YUV���� 2��BRG���� */
	uint8_t					*pUser;											/* �û����� */
}MEDIA_NET_PREVIEW_INFO,*LPMEDIA_NET_PREVIEW_INFO;

typedef struct
{
	char					sResourceID[21];								/* ��Դ����Ψһ���� */
	char					sParentDevId[32];								/* 	*/
	char					sName[64];										/* ��Դ���� */
	char					sManufacturer[64];								/* ��Դ����Ψһ���� */
	char					sModel[64];										/* �ͺ� */
	char					sOwner[64];
	char					sCivilCode[64];									/* ��֯�������� */
	char					sAddress[64];									/* ��ַ */
	int						nParental;
	int						nSafeWay;
	int						nRegisterWay;
	int						nSecrecy;
	int						nStatus;										/* ״̬ */
}MEDIA_NET_CATALOG_ITEM_INFO;

typedef struct
{
	uint32_t				uYear;
	uint32_t				uMonth;
	uint32_t				uDay;
	uint32_t				uHour;
	uint32_t				uMinute;
	uint32_t				uSecond;
}MEDIA_NET_TIME;

typedef struct
{
	char					sResourceID[21];								/* ��Դ����Ψһ���� */
	MEDIA_NET_TIME			struStartTime;									/* ��ʼʱ�� */
	MEDIA_NET_TIME			struStopTime;									/* ����ʱ�� */
}MEDIA_NET_VIDEOFILE_COND,*LPMEDIA_NET_VIDEOFILE_COND;

typedef struct MEDIA_NET_VIDEORECORD_ITEM
{
	char					sParentDevID[32];
	int						nSn;
	uint32_t				nSumNum;
	char					sParentName[32];
	void*					UserCtx;
	char					sResourceID[32];
	char					sName[32];
	char					sFilePath[32];
	char					sAddress[32];
	unsigned long long		uStartTime;
	unsigned long long		uEndTime;
	int						nSecrecy;
	int						nType;
	char					sRecordID[32];
	int						nIndistinct;
	MEDIA_NET_VIDEORECORD_ITEM *next;
};

/********************************SDK�ӿں�������*********************************/

NI_MEDIA_NET_API LONG __stdcall Media_Net_Init();
NI_MEDIA_NET_API LONG __stdcall Media_Net_Cleanup();
NI_MEDIA_NET_API LONG __stdcall Media_Net_Login(LPMEDIA_NET_LOGIN_INFO pLoginInfo, int& nUserID);
NI_MEDIA_NET_API LONG __stdcall Media_Net_Logout(int nUserID);
NI_MEDIA_NET_API LONG __stdcall Media_Net_RealPlay(int nUserID, LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int& nRealHandle);
NI_MEDIA_NET_API LONG __stdcall Media_Net_StopRealPlay(int nRealHandle);
NI_MEDIA_NET_API LONG __stdcall Media_Net_QueryVideoRecord(int lUserID, LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond);

#endif