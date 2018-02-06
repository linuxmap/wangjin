#pragma once

#include "H264Dec.h"
#include <kdvdef.h>
#include <kdvtype.h>
#include <osp.h>
#include <kdvmedianet.h>

struct CRtpMediaParam
{
	uint16_t	unPort;		// ¼àÌý¶Ë¿Ú
	HWND		hWndPlay;	// »­ÃæÏÔÊ¾¾ä±ú
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
public:
	void SetPlayParam(CRtpMediaParam* Param);
	BOOL Start();
	BOOL Stop();
private:
	CKdvMediaRcv	mMediaRecver;
	CH264Dec		mh264Dec;
	uint16_t		mnRtpPort;
	HWND			mhPlayWnd;
};

