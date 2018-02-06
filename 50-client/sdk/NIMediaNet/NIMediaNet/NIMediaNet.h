#ifndef _NI_MEDIA_NET_H_
#define _NI_MEDIA_NET_H_

#define	NI_MEDIA_NET_API	extern "C" __declspec(dllexport)

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;

/************************************消息编号************************************/

#define MEDIA_NET_GET_LOGIN_RESULT					100
#define	MEDIA_NET_GET_RESOURCE_RESULT				101
#define	MEDIA_NET_GET_VIDEORECORD_RESULT			102

/*************************************错误码*************************************/

#define MEDIA_NET_NOERROR							0	/* 没有错误 */
#define MEDIA_NET_PASSWORD_ERROR					1	/* 用户名或密码错误 */
#define MEDIA_NET_NOUGHPRI							2	/* 权限不足 */
#define MEDIA_NET_NOINIT							3   /* 没有初始化 */
#define MEDIA_NET_NETWORK_FAIL_CONNECT				4	/* 连接服务器失败 */
#define MEDIA_NET_MEMORY_NOT_ENOUGH					5	/* 内存不足 */
#define MEDIA_NET_LOCAL_INIT_ERROR					6	/* 本地初始化错误 */

/************************************回调函数************************************/

typedef void(__stdcall *fExcuteResultCallBack)
	(LONG lUserID,uint32_t uMsgID,uint32_t uResult,uint8_t *pUser);
typedef void(__stdcall *fRealDataCallBack)
	(LONG lRealHandle,uint32_t uDataType,uint8_t *pBuffer,uint32_t dwBufSize,uint8_t *pUser);

/***********************************通用结构体***********************************/

typedef struct  
{
	char					sServerAddress[16];								/* 服务端IPv4地址 */
	uint16_t				wServerPort;									/* 服务端端口 */
	char					sServerPassword[32];							/* 服务端密码 */
	char					sServerSipID[21];								/* 服务端国标唯一编码 */
	char					sServerDomainID[21];							/* 服务端域编码 */
	uint32_t				uServerKeepInterval;							/* 服务端心跳间隔 */
	uint32_t				uServerKeepMaxCnt;								/* 服务端最大心跳次数,超过该次数,链路将断开 */
	char					sClientAddress[16];								/* 客户端IPv4地址 */
	uint16_t				wClientPort;									/* 客户端端口 */
	char					sClientSipID[21];								/* 客户端国标唯一编码 */
	char					sClientDomainID[21];							/* 客户端域编码 */
	uint32_t				uSessionLingerTime;								/* 会话保持时间（ms） */
	fExcuteResultCallBack	cbExcuteResult;									/* 执行结果消息回调 */
	uint8_t*				cbUser;											
}MEDIA_NET_LOGIN_INFO,*LPMEDIA_NET_LOGIN_INFO;

typedef struct 
{
	HWND					hPlayWnd;										/* 窗口句柄，如果为NULL则不显示，但仍然有码流 */
	uint32_t				uShowMode;										/* 画面显示方式：0：GDI绘图 1：DirectX绘图 */
	char					sResourceID[21];								/* 资源国标唯一编码 */
	fRealDataCallBack		cbRealData;										/* 实时画面数据回调 */
	uint32_t				uMediaType;										/* 实时画面数据回调媒体类型 0：默认码流（h.264或h.265等编码数据） 1: YUV数据 2：BRG数据 */
	uint8_t					*pUser;											/* 用户数据 */
}MEDIA_NET_PREVIEW_INFO,*LPMEDIA_NET_PREVIEW_INFO;

typedef struct
{
	char					sResourceID[21];								/* 资源国标唯一编码 */
	char					sParentDevId[32];								/* 	*/
	char					sName[64];										/* 资源名称 */
	char					sManufacturer[64];								/* 资源国标唯一编码 */
	char					sModel[64];										/* 型号 */
	char					sOwner[64];
	char					sCivilCode[64];									/* 组织机构代码 */
	char					sAddress[64];									/* 地址 */
	int						nParental;
	int						nSafeWay;
	int						nRegisterWay;
	int						nSecrecy;
	int						nStatus;										/* 状态 */
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
	char					sResourceID[21];								/* 资源国标唯一编码 */
	MEDIA_NET_TIME			struStartTime;									/* 开始时间 */
	MEDIA_NET_TIME			struStopTime;									/* 结束时间 */
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

/********************************SDK接口函数声明*********************************/

NI_MEDIA_NET_API LONG __stdcall Media_Net_Init();
NI_MEDIA_NET_API LONG __stdcall Media_Net_Cleanup();
NI_MEDIA_NET_API LONG __stdcall Media_Net_Login(LPMEDIA_NET_LOGIN_INFO pLoginInfo, int& nUserID);
NI_MEDIA_NET_API LONG __stdcall Media_Net_Logout(int nUserID);
NI_MEDIA_NET_API LONG __stdcall Media_Net_RealPlay(int nUserID, LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int& nRealHandle);
NI_MEDIA_NET_API LONG __stdcall Media_Net_StopRealPlay(int nRealHandle);
NI_MEDIA_NET_API LONG __stdcall Media_Net_QueryVideoRecord(int lUserID, LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond);

#endif