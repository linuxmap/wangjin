#pragma once
#include <stdint.h>
#include <d2d1.h>
#pragma comment(lib,"d2d1.lib")
class CDirect2D
{
public:
	CDirect2D();
	~CDirect2D();
	BOOL Create(HWND hWnd, uint32_t nDstWidth, uint32_t nDstHeight, uint32_t nSrcWidth, uint32_t nSrcHeight);
	void Render(uint8_t* pData);
	void Free();
private:
	HRESULT CreateDevicesResource();
	void FreeDevicesResource();
private:
	uint32_t				mnDstWidth;
	uint32_t				mnDstHeight;
	uint32_t				mnSrcWidth;
	uint32_t				mnSrcHeight;
	HWND					mhPlayWnd;
	D2D1_RECT_U				mrcImage;
	ID2D1Factory*			mpD2dFactory;
	ID2D1HwndRenderTarget*	mpRenderTarget;
	ID2D1Bitmap*			mpBitmap;
	RECT					mrcWnd;
	uint8_t*				mpImgData;
};

