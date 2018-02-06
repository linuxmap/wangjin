#pragma once
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

typedef BOOL(*VIDEOTIMERCALLBACK)(DWORD dwUser);

class CVideoTimer
{
public:
	CVideoTimer();
	~CVideoTimer();
public:
	void Sleep(UINT dwMilliseconds);
	void Timeup();
private:
	HANDLE			mWaitEvent;
	MMRESULT		muTimerID;
};
