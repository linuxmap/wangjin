#include "StdAfx.h"
#include "RtpMedia.h"

CRtpMedia::CRtpMedia()
{
	mhPlayWnd = NULL;
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

	mh264Dec.InputData(node);
}

BOOL CRtpMedia::Start()
{
	uint16_t nRet = mMediaRecver.Create(512 << 10,(PFRAMEPROC)FrameCallBack,(uint32_t)this);
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	TLocalNetParam netParam;
	memset(&netParam,NULL,sizeof(TLocalNetParam));
	netParam.m_tLocalNet.m_dwRTPAddr = INADDR_ANY;//inet_addr("10.10.10.102");
	netParam.m_tLocalNet.m_wRTPPort = mnRtpPort;
	nRet = mMediaRecver.SetNetRcvLocalParam(netParam);
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	nRet = mMediaRecver.StartRcv();
	if(nRet != MEDIANET_NO_ERROR) return FALSE;

	mh264Dec.SetPlayHwnd(mhPlayWnd);
	return TRUE;
}

BOOL CRtpMedia::Stop()
{
	return FALSE;
}
