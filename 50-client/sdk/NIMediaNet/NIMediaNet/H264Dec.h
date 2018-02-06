#pragma once
#include "MainBuf.h"
#include "Direct2D.h"
#include <windows.h>
#include <Mmsystem.h>
#include <tchar.h>
#include "VideoTimer.h"

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
}

typedef void(* RGB24CallBack)(uint8_t* pData,uint32_t width,uint32_t height,uint32_t pContext);
typedef enum EMRenderMode
{
	RENDER_GDI,
	RENDER_DIRECT2D
};
class CH264Dec
{
public:
	CH264Dec(void);
	~CH264Dec(void);
public:
	void DecodeProc();
	void SetRGB24DataCallBack(RGB24CallBack fnCB,uint32_t pContext);
	void InputData(CFrameNode& node);
	void Create(HWND hPlayWnd, EMRenderMode RenderMode = RENDER_GDI);
private:
	uint16_t Init();
	void	Deinit();
	BOOL WindowIsChanged(HWND hWnd, uint32_t& nlastWidth, uint32_t&nlastHeight);
	void Render(HDC hdc,uint8_t* pData,uint32_t nSrcWidth,uint32_t nSrcHeight,uint32_t nDstWidth,uint32_t nDstHeight);

private:
	HANDLE							mThrDecode;         // 解码线程
	CMainBuf                        mMainBuf;           // 主缓存区
	AVCodecContext*                 mpCodecCtx;         // 解码器上下文
	uint8_t                         mnConsuming;       
	RGB24CallBack					mfnRGBHander;
	uint32_t						mfnpContext;
	HWND							mhWndPlay;
	bool							mbThreadStop;
	EMRenderMode					mRenderMode;
	CDirect2D						mD2Draw;
	DWORD							mdwFramePTS;		// 这一帧的绘图时间
	CVideoTimer						mVideoTimer;		// windows多媒体定时器封装
};

