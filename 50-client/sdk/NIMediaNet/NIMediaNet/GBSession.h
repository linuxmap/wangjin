#pragma once

#include <stdint.h>
#include <vector>
#include <Windows.h>
#include <WinSock2.h>

#include "NIMediaNet.h"
#include <gb.h>
#include "RtpMedia.h"


#define BEGIN_RTP_PORT							50000
// 本地会话信令状态机
enum emGBState
{
	GB_STATE_INVALID,
	GB_STATE_INIT,
	GB_STATE_REGISTING,
	GB_STATE_REGISTED,
	GB_STATE_QUERYING_CATALOG,
	GB_STATE_QUERYED_CATALOG,
};

extern class CGBSession;
struct InviteMeidaParam
{
	CGBSession*				mSession;
	CRtpMedia*				mpRtpMedia;
	int32_t					mSSRC;
	int16_t					mRtpPort;
	char					msResourceID[21];
	HWND					mhWnd;
};

class CGBSession
{
public:
	CGBSession();
	~CGBSession();
public:
	static LONG Init();
	static LONG Cleanup();
	static LONG LoginInObj(LPMEDIA_NET_LOGIN_INFO pLoginInfo,int32_t& nUserID);
	static LONG LoginOutObj(int32_t nUserID);
	static LONG RealPlayObj(int32_t nUserID,LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo,int32_t& nRealHandle);
	static LONG StopRealPlayObj(int32_t nRealHandle);
	static LONG QueryVideoRecordObj(int32_t nUserID, LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond);
	int OnGBMessageCallBack(int type, void *var, int var_len);
public:
	void SetUserId(LONG id) { mUserId = id; }
	LONG LoginIn(LPMEDIA_NET_LOGIN_INFO pLoginInfo);
	LONG LoginOut();
	LONG RealPlay(LPMEDIA_NET_PREVIEW_INFO lpPreviewInfo, int32_t& nRealHandle);
	LONG StopRealPlay(int32_t lRealHandle);
	LONG QueryVideoRecord(LPMEDIA_NET_VIDEOFILE_COND lpVideoFileCond);
private:
	gb_handle_t							mGBhandle;
	emGBState							mGBstate;
	fExcuteResultCallBack				mUserCallBack;
	int32_t								mUserId;
	uint8_t*							mUserObj;
};

