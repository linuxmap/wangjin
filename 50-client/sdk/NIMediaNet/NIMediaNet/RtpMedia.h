#pragma once

#include "H264Dec.h"
#include <kdvdef.h>
#include <kdvtype.h>
#include <osp.h>
#include <kdvmedianet.h>

#define DEFAULT_SECOND_FRAME_NUM		25

struct CRtpMediaParam
{
	uint16_t	unPort;		// �����˿�
	HWND		hWndPlay;	// ������ʾ���
};

struct CFpsCount
{
	DWORD			dwLastTime;		// �ϴμ���ʱ��
	DWORD			dwStatistTime;	// ͳ��ʱ������λ��s���룩
	uint8_t			unCurFpsCount;	// ��ǰ֡����������һ��֡����������ܹ����ĵ�ʱ�䣬����֡����
	uint16_t		unAveFpsSec;	// ��ǰÿ��ƽ��֡�ʣ��ᶯ̬����
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
	void StatisticsFrameRate();		// ֡��ͳ�ƺ���
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

