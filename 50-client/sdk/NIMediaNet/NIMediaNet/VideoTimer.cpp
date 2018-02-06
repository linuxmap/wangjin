#include "stdafx.h"
#include "VideoTimer.h"


CVideoTimer::CVideoTimer()
{
	mWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


CVideoTimer::~CVideoTimer()
{
	CloseHandle(mWaitEvent);
}

void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	CVideoTimer* pThis = (CVideoTimer*)dwUser;
	pThis->Timeup();
}

void CVideoTimer::Timeup()
{
	SetEvent(mWaitEvent);
}

void CVideoTimer::Sleep(UINT dwMilliseconds)
{
	if (timeBeginPeriod(1) == TIMERR_NOERROR)
	{
		muTimerID = timeSetEvent(dwMilliseconds, 1, TimeProc, (DWORD_PTR)this, TIME_PERIODIC);
	}

	DWORD dwResult = WaitForSingleObject(mWaitEvent, INFINITE);
	if (dwResult == WAIT_OBJECT_0)
	{

	}
	else
	{

	}
	timeKillEvent(muTimerID);
	timeEndPeriod(1);
}
