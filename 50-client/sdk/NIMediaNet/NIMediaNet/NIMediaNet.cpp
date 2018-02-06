// NIMediaNet.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "NIMediaNet.h"
#include "GBSession.h"

NI_MEDIA_NET_API LONG __stdcall Media_Net_Init()
{
	return CGBSession::Init();
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_Cleanup()
{
	return CGBSession::Cleanup();
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_Login(LPMEDIA_NET_LOGIN_INFO pLoginInfo, int& nUserID)
{
	return CGBSession::LoginInObj(pLoginInfo, nUserID);
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_Logout(int nUserID)
{
	return CGBSession::LoginOutObj(nUserID);
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_RealPlay(int nUserID,LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int& nRealHandle)
{
	return CGBSession::RealPlayObj(nUserID,lpPreviewInfo, nRealHandle);
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_StopRealPlay(int nRealHandle)
{
	return CGBSession::StopRealPlayObj(nRealHandle);
}

NI_MEDIA_NET_API LONG __stdcall Media_Net_QueryVideoRecord(int nUserID, LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond)
{
	return CGBSession::QueryVideoRecordObj(nUserID,lpVideoFileCond);
}
