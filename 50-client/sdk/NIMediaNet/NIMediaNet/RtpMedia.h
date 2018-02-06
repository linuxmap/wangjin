#pragma once

#include "H264Dec.h"
#include <kdvdef.h>
#include <kdvtype.h>
#include <osp.h>
#include <kdvmedianet.h>

#define DEFAULT_SECOND_FRAME_NUM		25

struct CRtpMediaParam
{
	uint16_t	unPort;		// 监听端口
	HWND		hWndPlay;	// 画面显示句柄
};

struct CFpsCount
{
	DWORD			dwLastTime;		// 上次计数时间
	DWORD			dwStatistTime;	// 统计时长，单位：s（秒）
	uint8_t			unCurFpsCount;	// 当前帧计数，当满一定帧数，会除以总共消耗的时间，就是帧率了
	uint16_t		unAveFpsSec;	// 当前每秒平均帧率，会动态调整
};

class CRtpMedia
{
public:
	CRtpMedia();
	~CRtpMedia();
public:
	static BOOL Init();
	static BOOL UnInit();
	void FrameCBHander(PFRAMEHDR pFrmHdr);
private:
	void StatisticsFrameRate();		// 帧率统计函数
public:
	void SetPlayParam(CRtpMediaParam* Param);
	BOOL Start();
	BOOL Stop();
private:
	CKdvMediaRcv	mMediaRecver;
	CH264Dec		mh264Dec;
	uint16_t		mnRtpPort;
	HWND			mhPlayWnd;
	CFpsCount		mFpsCount;
};

