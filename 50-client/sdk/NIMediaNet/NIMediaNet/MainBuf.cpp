#include "StdAfx.h"
#include "MainBuf.h"


BOOL CMainBuf::Init()
{
	mpDataBuf = NULL;
	mnDataWirtePos = mnDataReadPos = 0;
	mNodeWritePos = mNodeReadPos = 0;
	mnDataLen = 1024 << 10;
	mpPackBuf = new uint8_t[mnDataLen];
	mWriteAfterReadCount = 0;
	mnCacheNumber = FORCE_FRAME_NUMBER;

	try
	{
		mpDataBuf = new uint8_t[mnDataLen];
		for(int i = 0;i < 10;i ++)
		{
			memset(&mQueueFrame[i],0,sizeof(CFrameNode));
		}
	}
	catch(...)
	{
		return FALSE;
	}

	// ��ʼ״̬Ϊ������
	mhEventDataComming = CreateEvent(NULL, FALSE, FALSE,NULL);

	InitializeCriticalSection(&mcsData);
	memset(mpDataBuf,NULL,mnDataLen);
	return TRUE;
}

void CMainBuf::Uninit()
{
	if(mnDataLen > 0)
	{
		delete mpDataBuf;
	}

	delete mpPackBuf;
	DeleteCriticalSection(&mcsData);
}

BOOL CMainBuf::Write(CFrameNode& node)
{
	EnterCriticalSection(&mcsData);
	CFrameNode* pCurNode = &mQueueFrame[mNodeWritePos];
	if(!pCurNode->mbUsed)
	{
		uint8_t* pLastPos = NULL;

		// ����һ��λ�����д,����������ˣ�Ҫ��ת����ͷ
		if((mnDataWirtePos + node.mnDataSize) < mnDataLen)
		{
			memcpy(&mpDataBuf[mnDataWirtePos],node.mpDataBuf,node.mnDataSize);
			pLastPos = &mpDataBuf[mnDataWirtePos];
			mnDataWirtePos += node.mnDataSize;
		}
		else
		{
			memcpy(&mpDataBuf[0],node.mpDataBuf,node.mnDataSize);
			pLastPos = &mpDataBuf[0];
			mnDataWirtePos = 0;
			mnDataWirtePos = node.mnDataSize;
			pCurNode->mbNeedFlip = true;
		}

		pCurNode->mbUsed = true;
		pCurNode->mbyMediaType     = node.mbyMediaType;
		pCurNode->mbyStreamID      = node.mbyStreamID;
		pCurNode->mpDataBuf        = pLastPos;
		pCurNode->mnDataSize       = node.mnDataSize;
		pCurNode->mnFrameID        = node.mnFrameID;
		pCurNode->mnSSRC           = node.mnSSRC;
		pCurNode->mnInterval	   = node.mnInterval;

		pCurNode->mtVideoParam.mbKeyFrame  = node.mtVideoParam.mbKeyFrame;
		pCurNode->mtVideoParam.mnHeight    = node.mtVideoParam.mnHeight;
		pCurNode->mtVideoParam.mnWidth     = node.mtVideoParam.mnWidth;
	}
	else
	{
		// ��������������д����Ҫ������Ŷ
		printf("mWriteAfterReadCount:%d\n",++ mWriteAfterReadCount);
		LeaveCriticalSection(&mcsData);
		return FALSE;
	}

	mNodeWritePos += 1;
	if(mNodeWritePos >= 10)
	{
		mNodeWritePos = 0;
	}

	LeaveCriticalSection(&mcsData);

	SetEvent(mhEventDataComming);

	return TRUE;
}

void CMainBuf::CacheFrameNumber(uint16_t num)
{
	mnCacheNumber = num;
}

BOOL CMainBuf::Read(CFrameNode* pNode)
{
	EnterCriticalSection(&mcsData);

	//// ǿ�ƻ������
	uint16_t nGap = 0;
	if (mNodeWritePos > mNodeReadPos)
	{
		nGap = mNodeWritePos - mNodeReadPos;
	}
	else
	{
		nGap = mNodeWritePos + 10 - mNodeReadPos;
	}
	if (nGap < mnCacheNumber)
	{
		LeaveCriticalSection(&mcsData);
		return FALSE;
	}

	CFrameNode* pCurNode = &mQueueFrame[mNodeReadPos];
	if (pCurNode->mbUsed)
	{
		memcpy(mpPackBuf, pCurNode->mpDataBuf, pCurNode->mnDataSize);

		if (pCurNode->mbNeedFlip)
		{
			mnDataReadPos = 0;
		}
		mnDataReadPos += pCurNode->mnDataSize;
		pNode->mbyMediaType				= pCurNode->mbyMediaType;
		pNode->mbyStreamID				= pCurNode->mbyStreamID;
		pNode->mpDataBuf				= mpPackBuf;
		pNode->mnDataSize				= pCurNode->mnDataSize;
		pNode->mnFrameID				= pCurNode->mnFrameID;
		pNode->mnSSRC					= pCurNode->mnSSRC;
		pNode->mnInterval				= pCurNode->mnInterval;

		pNode->mtVideoParam.mbKeyFrame	= pCurNode->mtVideoParam.mbKeyFrame;
		pNode->mtVideoParam.mnHeight	= pCurNode->mtVideoParam.mnHeight;
		pNode->mtVideoParam.mnWidth		= pCurNode->mtVideoParam.mnWidth;

		pCurNode->mbUsed = pNode->mbUsed = false;
		memset(pCurNode, 0, sizeof(CFrameNode));
		mNodeReadPos++;
		if (mNodeReadPos >= 10)
		{
			mNodeReadPos = 0;
		}
	}
	else
	{
		LeaveCriticalSection(&mcsData);
		return FALSE;
	}
	LeaveCriticalSection(&mcsData);
	return TRUE;
}

void CMainBuf::Wait()
{
	DWORD dwRet = WaitForSingleObject(mhEventDataComming,5); // INFINITE

	if (dwRet != WAIT_OBJECT_0)
	{

	}
}

uint32_t CMainBuf::GetFrameLen()
{
	uint32_t nLen = 0;
	EnterCriticalSection(&mcsData);
	if (mNodeWritePos > mNodeReadPos)
	{
		nLen = mNodeWritePos - mNodeReadPos;
	}
	else
	{
		nLen = mNodeWritePos + 10 - mNodeReadPos;
	}
	LeaveCriticalSection(&mcsData);
	return nLen;
}