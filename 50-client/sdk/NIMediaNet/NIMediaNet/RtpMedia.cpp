#include "StdAfx.h"
#include "RtpMedia.h"

CRtpMedia::CRtpMedia()
{
	mhPlayWnd = NULL;
	memset(&mFpsCount, NULL, sizeof(CFpsCount));
	mFpsCount.unAveFpsSec = DEFAULT_SECOND_FRAME_NUM;
	mFpsCount.dwStatistTime = 5;
}
CRtpMedia::~CRtpMedia()
{

}

BOOL CRtpMedia::Init()
{
	u16 nRet = KdvSocketStartup();
	if(nRet != MEDIANET_NO_ERROR)
	{
		printf("Startup Error.Code:%d\n",nRet);
		return FALSE;
	}

	printf("Startup Sucess!\n");
	return TRUE;
}

BOOL CRtpMedia::UnInit()
{
	u16 nRet = KdvSocketCleanup();
	if(nRet != MEDIANET_NO_ERROR)
	{
		printf("Cleanup Error.Code:%d\n",nRet);
		return FALSE;
	}

	printf("Cleanup Sucess!\n");
	return TRUE;
}

void CRtpMedia::SetPlayParam(CRtpMediaParam* Param)
{
	mnRtpPort = Param->unPort;
	mhPlayWnd = Param->hWndPlay;

	mh264Dec.Create(mhPlayWnd,RENDER_DIRECT2D);
	//mh264Dec.Create(mhPlayWnd);
}

void FrameCallBack(PFRAMEHDR pFrmHdr, KD_PTR pContext)
{
	if(pContext)
	{
		CRtpMedia* pThis = (CRtpMedia*)pContext;
		pThis->FrameCBHander(pFrmHdr);
	}
}

void CRtpMedia::FrameCBHander(PFRAMEHDR pFrmHdr)
{
	CFrameNode node;
	node.mbyMediaType   = pFrmHdr->m_byMediaType;
	node.mbyStreamID    = pFrmHdr->m_byStreamID;
	node.mpDataBuf      = pFrmHdr->m_pData;
	node.mnDataSize     = pFrmHdr->m_dwDataSize;
	node.mnFrameID      = pFrmHdr->m_dwFrameID;
	node.mnSSRC         = pFrmHdr->m_dwSSRC;

	node.mtVideoParam.mbKeyFrame    = pFrmHdr->m_tVideoParam.m_bKeyFrame;
	node.mtVideoParam.mnHeight      = pFrmHdr->m_tVideoParam.m_wVideoHeight;
	node.mtVideoParam.mnWidth       = pFrmHdr->m_tVideoParam.m_wVideoWidth;

	StatisticsFrameRate();

	// 保护，防止mFpsCount.unAveFpsSec为0，无法除
	if (mFpsCount.unAveFpsSec > 0)
	{
		node.mnInterval = 1000 / mFpsCount.unAveFpsSec;
	}
	else
	{
		node.mnInterval = 1000 / DEFAULT_SECOND_FRAME_NUM;
	}
	
	mh264Dec.InputData(node);
}

BOOL CRtpMedia::Start()
{
	uint16_t nRet = mMediaRecver.Create(512 << 10,(PFRAMEPROC)FrameCallBack,(uint32_t)this);
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	TLocalNetParam netParam;
	memset(&netParam,NULL,sizeof(TLocalNetParam));
	netParam.m_tLocalNet.m_dwRTPAddr = INADDR_ANY;
	netParam.m_tLocalNet.m_wRTPPort = mnRtpPort;
	nRet = mMediaRecver.SetNetRcvLocalParam(netParam);
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	nRet = mMediaRecver.StartRcv();
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	return TRUE;
}

BOOL CRtpMedia::Stop()
{
	mMediaRecver.StopRcv();
	return TRUE;
}

void trace(const TCHAR * format, ...)
{
	static const int BufferLen = 1024;
	static int count;
	va_list pArg;
	TCHAR szMessageBuffer[BufferLen] = { 0 };
	va_start(pArg, format);
	_vsntprintf(szMessageBuffer, BufferLen - 1, format, pArg);
	va_end(pArg);
	OutputDebugString(szMessageBuffer);
}

// 不断去统计帧率
void CRtpMedia::StatisticsFrameRate()
{
	mFpsCount.unCurFpsCount++;
	if (mFpsCount.unCurFpsCount == 1)
	{
		mFpsCount.dwLastTime = timeGetTime();
	}
	else
	{
		DWORD dwCurTime = timeGetTime();
		DWORD dwInterv = dwCurTime - mFpsCount.dwLastTime;
		if (dwInterv >= mFpsCount.dwStatistTime * 1000)
		{
			mFpsCount.unAveFpsSec = mFpsCount.unCurFpsCount / mFpsCount.dwStatistTime;
			mFpsCount.dwLastTime = dwCurTime;
			mFpsCount.unCurFpsCount = 1;

			trace(_TEXT("自行统计的帧率:%d.\n"), mFpsCount.unAveFpsSec);
		}
	}
}
