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

		// 从上一个位置向后写,如果本次满了，要跳转至从头
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

		pCurNode->mtVideoParam.mbKeyFrame  = node.mtVideoParam.mbKeyFrame;
		pCurNode->mtVideoParam.mnHeight    = node.mtVideoParam.mnHeight;
		pCurNode->mtVideoParam.mnWidth     = node.mtVideoParam.mnWidth;
	}
	else
	{
		// 队列已满，不能写，就要报错了哦
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
	return TRUE;
}

BOOL CMainBuf::Read(CFrameNode* pNode)
{
	DWORD dwBegin = GetTickCount();
	EnterCriticalSection(&mcsData);
	CFrameNode* pCurNode = &mQueueFrame[mNodeReadPos];
	if(pCurNode->mbUsed)
	{
		memcpy(mpPackBuf,pCurNode->mpDataBuf,pCurNode->mnDataSize);

		if(pCurNode->mbNeedFlip)
		{
			mnDataReadPos = 0;
		}
		mnDataReadPos += pCurNode->mnDataSize;
		//++++++++++++++++++++++++++++++++++++++++++++++  打印空闲的BUF  +++++++++++++++++++++++++++++++++++++++++++++++++
		//uint32_t nL = 0;
		//if(mNodeWritePos >= mNodeReadPos)
		//{
		//    nL = mNodeWritePos - mNodeReadPos;
		//}
		//else
		//{
		//    nL = mNodeWritePos + 10 - mNodeReadPos;
		//}
		//printf("Node Left:%d.\n",nL);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		pNode->mbyMediaType     = pCurNode->mbyMediaType;
		pNode->mbyStreamID      = pCurNode->mbyStreamID;
		pNode->mpDataBuf        = mpPackBuf;
		pNode->mnDataSize       = pCurNode->mnDataSize;
		pNode->mnFrameID        = pCurNode->mnFrameID;
		pNode->mnSSRC           = pCurNode->mnSSRC;

		pNode->mtVideoParam.mbKeyFrame  = pCurNode->mtVideoParam.mbKeyFrame;
		pNode->mtVideoParam.mnHeight    = pCurNode->mtVideoParam.mnHeight;
		pNode->mtVideoParam.mnWidth     = pCurNode->mtVideoParam.mnWidth;

		pCurNode->mbUsed = pNode->mbUsed = false;
		memset(pCurNode,0,sizeof(CFrameNode));
		mNodeReadPos ++;
		if(mNodeReadPos >= 10)
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
	DWORD dwEnd = GetTickCount();
	mnReadCon = dwEnd - dwBegin;
	if(mnReadCon > 0)
		printf("mnReadCon:%d\n",mnReadCon);
	return TRUE;
}