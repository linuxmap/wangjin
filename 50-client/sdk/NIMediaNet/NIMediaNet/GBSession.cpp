#include "StdAfx.h"
#include "GBSession.h"

static std::vector<CGBSession*>		gSessionList;
static CRITICAL_SECTION				gSessionCritial;
std::vector<InviteMeidaParam*>		gvctMedia;
CRITICAL_SECTION					gvctMediaCritial;

static uint32_t gnCurrentPort = BEGIN_RTP_PORT;

LONG CGBSession::Init()
{
	InitializeCriticalSection(&gSessionCritial);
	InitializeCriticalSection(&gvctMediaCritial);

	WSADATA wsaData;
	int err;
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::Cleanup()
{
	DeleteCriticalSection(&gvctMediaCritial);
	DeleteCriticalSection(&gSessionCritial);
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::LoginInObj(LPMEDIA_NET_LOGIN_INFO pLoginInfo,int32_t& nUserID)
{
	CGBSession* pThis = NULL;
	try
	{
		pThis = new CGBSession;
	}
	catch (...)
	{
		return MEDIA_NET_MEMORY_NOT_ENOUGH;
	}

	LONG lRet = pThis->LoginIn(pLoginInfo);
	if (MEDIA_NET_NOERROR != lRet)
	{
		delete pThis;
		pThis = NULL;
		return lRet;
	}
	
	EnterCriticalSection(&gSessionCritial);
	gSessionList.push_back(pThis);
	nUserID = (int32_t)pThis;
	LeaveCriticalSection(&gSessionCritial);

	pThis->SetUserId(nUserID);
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::LoginOutObj(int32_t nUserID)
{
	CGBSession* pThis = NULL;
	try
	{
		EnterCriticalSection(&gSessionCritial);

		for (std::vector<CGBSession*>::iterator iter = gSessionList.begin();iter != gSessionList.end();iter ++)
		{
			pThis = *iter;
			if (pThis == (CGBSession*)nUserID)
			{
				gSessionList.erase(iter);
				break;
			}
		}
		LeaveCriticalSection(&gSessionCritial);
	}
	catch (...)
	{
		LeaveCriticalSection(&gSessionCritial);
		return MEDIA_NET_NOINIT;
	}
	if (pThis)
	{
		pThis->LoginOut();
		delete pThis;
		pThis = NULL;
	}

	return MEDIA_NET_NOERROR;
}

LONG CGBSession::RealPlayObj(int32_t nUserID,LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int32_t& nRealHandle)
{
	CGBSession* pThis = NULL;
	EnterCriticalSection(&gSessionCritial);
	for (std::vector<CGBSession*>::iterator iter = gSessionList.begin(); iter != gSessionList.end(); iter++)
	{
		pThis = *iter;
		if (pThis == (CGBSession*)nUserID)
		{
			break;
		}
	}
	LeaveCriticalSection(&gSessionCritial);

	if (pThis)
	{
		return pThis->RealPlay(lpPreviewInfo, nRealHandle);
	}
	return MEDIA_NET_NOINIT;
}

LONG CGBSession::StopRealPlayObj(int32_t nRealHandle)
{
	InviteMeidaParam* pThis = NULL;
	EnterCriticalSection(&gSessionCritial);
	for (std::vector<InviteMeidaParam*>::iterator iter = gvctMedia.begin(); iter != gvctMedia.end(); iter++)
	{
		pThis = *iter;
		if (pThis == (InviteMeidaParam*)nRealHandle)
		{
			break;
		}
	}
	LeaveCriticalSection(&gSessionCritial);

	if (pThis)
	{
		return pThis->mSession->StopRealPlay(nRealHandle);

	}

	return MEDIA_NET_NOINIT;
}

LONG CGBSession::QueryVideoRecordObj(int32_t nUserID, LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond)
{
	CGBSession* pThis = NULL;
	EnterCriticalSection(&gSessionCritial);
	for (std::vector<CGBSession*>::iterator iter = gSessionList.begin(); iter != gSessionList.end(); iter++)
	{
		pThis = *iter;
		if (pThis == (CGBSession*)nUserID)
		{
			break;
		}
	}
	LeaveCriticalSection(&gSessionCritial);

	if (pThis)
	{
		return pThis->QueryVideoRecord(lpVideoFileCond);
	}
	return MEDIA_NET_NOINIT;
}

CGBSession::CGBSession():mUserId(-1)
{

}

CGBSession::~CGBSession()
{

}

int GBMessageHandler(void *ctx, int type, void *var, int var_len)
{
	CGBSession* pThis = (CGBSession*)ctx;
	return pThis->OnGBMessageCallBack(type, var, var_len);
}

int CGBSession::OnGBMessageCallBack(int type, void *var, int var_len)
{
	switch (type)
	{
	case GB::ECLIENT_EVT_REGISTERED:
		if(GB_STATE_REGISTING == mGBstate)
		{
			mGBstate = GB_STATE_REGISTED;
			mUserCallBack(mUserId, MEDIA_NET_GET_LOGIN_RESULT, (uint32_t)var, mUserObj);

			// 第一版本先不提供QueryCatalog的接口，登录成功了，直接拉取Catalog
			int nRet = GB::gb_client_query_catalog(mGBhandle);
			if (GB_SUCCESS != nRet)
			{
				printf("检索资源失败！\n");
			}
			else
			{
				mGBstate = GB_STATE_QUERYING_CATALOG;
			}
		}		
		break;
	case GB::ECLIENT_EVT_RES_CATALOG:
		{
			mUserCallBack(mUserId, MEDIA_NET_GET_RESOURCE_RESULT, (uint32_t)var, mUserObj);
			GB::CGbCatalogItem* item = (GB::CGbCatalogItem*)var;
			printf("%s\n",item->name);
		}
		break;
	case GB::ECLIENT_EVT_RES_INVITE:
		{
			
		}
		break;
	case GB::ECLIENT_EVT_RES_QUERY_RECORD_INFO:
		{
			mUserCallBack(mUserId, MEDIA_NET_GET_VIDEORECORD_RESULT, (uint32_t)var, mUserObj);
			//GB::CGbRecordItem *item = (GB::CGbRecordItem *)var;
			//printf("录像节点的总个数是:%d", item->sum_num);

			//while (item)
			//{
			//	item = item->next;
			//}
		}
		break;
	default:
		break;
	}
	return 0;
}

LONG CGBSession::LoginIn(LPMEDIA_NET_LOGIN_INFO pLoginInfo)
{
	if (!pLoginInfo->cbExcuteResult) return MEDIA_NET_LOCAL_INIT_ERROR;
	this->mUserCallBack = pLoginInfo->cbExcuteResult;

	GB::CGbClientParam LocalParam;
	ZeroMemory(&LocalParam,sizeof(GB::CGbClientParam));

	strcpy_s(LocalParam.ip,pLoginInfo->sClientAddress);
	LocalParam.port = pLoginInfo->wClientPort;
	strcpy_s(LocalParam.sip_id,pLoginInfo->sClientSipID);
	strcpy_s(LocalParam.domain_id,pLoginInfo->sClientDomainID);
	strcpy_s(LocalParam.user_agent,"NETINTECH/IMOS");
	LocalParam.session_linger_time = pLoginInfo->uSessionLingerTime;

	int nRet = GB::gb_client_create(&mGBhandle, LocalParam);
	if(nRet < 0)
	{
		return MEDIA_NET_LOCAL_INIT_ERROR;
	}

	GB::CGbRegParam RegParam;
	ZeroMemory(&RegParam,sizeof(GB::CGbRegParam));

	nRet = GB::gb_client_set_cb(mGBhandle, this, GBMessageHandler);

	RegParam.server_ip = ntohl(inet_addr(pLoginInfo->sServerAddress));
	strcpy_s(RegParam.server_sip_id,pLoginInfo->sServerSipID);
	strcpy_s(RegParam.server_domain_id, pLoginInfo->sClientDomainID);
	strcpy_s(RegParam.passwd, pLoginInfo->sServerPassword);
	RegParam.server_port = pLoginInfo->wServerPort;
	RegParam.keep_interval = pLoginInfo->uServerKeepInterval;
	RegParam.keep_max_cnt = pLoginInfo->uServerKeepMaxCnt;

	mGBstate = GB_STATE_REGISTING;
	nRet = GB::gb_client_reg_to_server(mGBhandle, RegParam);

	return MEDIA_NET_NOERROR;
}

LONG CGBSession::LoginOut()
{
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::RealPlay(LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int32_t& nRealHandle)
{
	InviteMeidaParam* InvParam = new InviteMeidaParam;

	InvParam->mSSRC = rand();
	strcpy_s(InvParam->msResourceID, lpPreviewInfo->sResourceID);

	CRtpMediaParam RtpParam;
	RtpParam.unPort = gnCurrentPort;
	RtpParam.hWndPlay = lpPreviewInfo->hPlayWnd;
	gnCurrentPort++;

	InvParam->mSession = this;
	InvParam->mpRtpMedia = new CRtpMedia;
	InvParam->mpRtpMedia->Init();
	InvParam->mpRtpMedia->SetPlayParam(&RtpParam);
	InvParam->mRtpPort = RtpParam.unPort;
	InvParam->mhWnd = lpPreviewInfo->hPlayWnd;

	BOOL bSucess = InvParam->mpRtpMedia->Start();
	if (bSucess)
	{
		int nRet = GB::gb_client_invite_media(mGBhandle, InvParam->msResourceID, InvParam->mSSRC, RtpParam.unPort, 96);
		if (nRet != GB_SUCCESS)
		{
			InvParam->mpRtpMedia->Stop();
			return MEDIA_NET_LOCAL_INIT_ERROR;
		}
		gnCurrentPort++;
		nRealHandle = (int32_t)InvParam;

		EnterCriticalSection(&gvctMediaCritial);
		gvctMedia.push_back(InvParam);
		LeaveCriticalSection(&gvctMediaCritial);
	}
	else
	{
		delete InvParam->mpRtpMedia;
		InvParam->mpRtpMedia = NULL;
		delete InvParam;
		InvParam = NULL;
	}
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::StopRealPlay(int32_t nRealHandle)
{
	InviteMeidaParam* pIMParam = NULL;
	EnterCriticalSection(&gvctMediaCritial);
	for (std::vector<InviteMeidaParam*>::iterator it = gvctMedia.begin();it != gvctMedia.end();it ++)
	{
		if (*it == (InviteMeidaParam*)nRealHandle)
		{
			pIMParam = *it;
			gvctMedia.erase(it);
			break;
		}
	}
	LeaveCriticalSection(&gvctMediaCritial);

	if (pIMParam)
	{
		int nRet = GB::gb_client_bye_media(mGBhandle, pIMParam->msResourceID, pIMParam->mSSRC, pIMParam->mRtpPort, 96);
		if (nRet != GB_SUCCESS)
		{
			return MEDIA_NET_LOCAL_INIT_ERROR;
		}
		
		pIMParam->mpRtpMedia->Stop();

		delete pIMParam->mpRtpMedia;
		pIMParam->mpRtpMedia = NULL;

		delete pIMParam;
		pIMParam = NULL;
	}
	else
	{
		return MEDIA_NET_NOINIT;
	}
	return MEDIA_NET_NOERROR;
}

LONG CGBSession::QueryVideoRecord(LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond)
{
	GB::CGbRecordQueryParam param = {0};
	strcpy(param.dev_id,lpVideoFileCond->sResourceID);

	tm tmStart = {0};
	tmStart.tm_year			= lpVideoFileCond->struStartTime.uYear - 1900;
	tmStart.tm_mon			= lpVideoFileCond->struStartTime.uMonth - 1;
	tmStart.tm_mday			= lpVideoFileCond->struStartTime.uDay;
	tmStart.tm_hour			= lpVideoFileCond->struStartTime.uHour;
	tmStart.tm_min			= lpVideoFileCond->struStartTime.uMinute;
	tmStart.tm_sec			= lpVideoFileCond->struStartTime.uSecond;

	time_t ttStart = mktime(&tmStart);

	tm tmStop = {0};
	tmStop.tm_year = lpVideoFileCond->struStopTime.uYear - 1900;
	tmStop.tm_mon = lpVideoFileCond->struStopTime.uMonth - 1;
	tmStop.tm_mday = lpVideoFileCond->struStopTime.uDay;
	tmStop.tm_hour = lpVideoFileCond->struStopTime.uHour;
	tmStop.tm_min = lpVideoFileCond->struStopTime.uMinute;
	tmStop.tm_sec = lpVideoFileCond->struStopTime.uSecond;

	time_t ttStop = mktime(&tmStop);
	param.start_time = ttStart;
	param.end_time = ttStop;

	int nRet = GB::gb_client_query_record_info(mGBhandle, param);
	if (nRet != GB_SUCCESS)
	{
		return MEDIA_NET_LOCAL_INIT_ERROR;
	}

	return MEDIA_NET_NOERROR;
}