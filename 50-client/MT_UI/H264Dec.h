#pragma once
#include "MainBuf.h"

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
}

typedef void(* RGB24CallBack)(uint8_t* pData,uint32_t width,uint32_t height,uint32_t pContext);

class CH264Dec
{
public:
	CH264Dec(void);
	~CH264Dec(void);
public:
	void DecodeProc();
	void SetRGB24DataCallBack(RGB24CallBack fnCB,uint32_t pContext);
	void InputData(CFrameNode& node);
	void SetPlayHwnd(HWND hWnd){mhWndPlay = hWnd;}
private:
	uint16_t Init();
	void	Deinit();
private:
	HANDLE							mThrDecode;         // 解码线程
	CMainBuf                        mMainBuf;           // 主缓存区
	AVCodecContext*                 mpCodecCtx;         // 解码器上下文
	uint8_t                         mnConsuming;       
	RGB24CallBack					mfnRGBHander;
	uint32_t						mfnpContext;
	HWND							mhWndPlay;
	bool							mbThreadStop;
};

