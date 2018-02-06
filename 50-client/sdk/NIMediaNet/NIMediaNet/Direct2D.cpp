#include "stdafx.h"
#include "Direct2D.h"
#include <stdio.h>

CDirect2D::CDirect2D()
{
	mnDstHeight = 0;
	mnDstWidth = 0;
	mnSrcHeight = 0;
	mnSrcWidth = 0;
	mpRenderTarget = nullptr;
	mpBitmap = nullptr;
	mhPlayWnd = NULL;
	mpImgData = NULL;
}


CDirect2D::~CDirect2D()
{
}

BOOL CDirect2D::Create(HWND hWnd, uint32_t nDstWidth, uint32_t nDstHeight,uint32_t nSrcWidth,uint32_t nSrcHeight)
{
	mnDstHeight		= nDstHeight;
	mnDstWidth		= nDstWidth;
	mnSrcHeight		= nSrcHeight;
	mnSrcWidth		= nSrcWidth;
	mpRenderTarget	= nullptr;
	mpBitmap		= nullptr;
	mhPlayWnd		= hWnd;

	if (FAILED(CreateDevicesResource()))
	{
		return FALSE;
	}
	return TRUE;
}

void CDirect2D::Free()
{
	FreeDevicesResource();
}

HRESULT CDirect2D::CreateDevicesResource()
{
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &mpD2dFactory)))
	{
		printf("D2D1CreateFactory Error\n");
		return E_FAIL;
	}

	if (!mpRenderTarget)
	{
		GetClientRect(mhPlayWnd, &mrcWnd);

		D2D1_SIZE_U dst_size = D2D1::SizeU(mrcWnd.right - mrcWnd.left, mrcWnd.bottom - mrcWnd.top);
		if (FAILED(mpD2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			//第三个参数设置不等待垂直同步，默认垂直同步时最高刷新频率为显卡刷新频率，一般60FPS 
			D2D1::HwndRenderTargetProperties(mhPlayWnd, dst_size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
			&mpRenderTarget)))
		{
			return E_FAIL;
		}

		// 创建位图
		D2D1_SIZE_U img_size = D2D1::SizeU(mnSrcWidth, mnSrcHeight);
		//该参数设置图像数据区的像素格式，现为RGBA，可根据需要改为别的格式，只是后面的数据拷贝要做相应的调整,默认 DXGI_FORMAT_B8G8R8A8_UNORM
		D2D1_PIXEL_FORMAT pix_fmt = { DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED };
		// 位图具体信息描述
		D2D1_BITMAP_PROPERTIES bmp_prop = {pix_fmt,img_size.width,img_size.height};
		long pitch = img_size.width * 4;

		mpImgData = new uint8_t[img_size.width * img_size.height * 4];
		ZeroMemory(mpImgData, img_size.width * img_size.height * 4);

		// 设置为抗锯齿模式
		mpRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

		HRESULT hr = mpRenderTarget->CreateBitmap(img_size, mpImgData, pitch, &bmp_prop, &mpBitmap);
		if (FAILED(hr))
		{
			return E_FAIL;
		}

		mrcImage.left = 0;
		mrcImage.right = img_size.width;
		mrcImage.top = 0;
		mrcImage.bottom = img_size.height;
	}

	return S_OK;
}

void CDirect2D::FreeDevicesResource()
{
	if (mpImgData)
	{
		delete[] mpImgData;
		mpImgData = NULL;
	}
	if (mpRenderTarget)
	{
		mpRenderTarget->Release();
		mpRenderTarget = nullptr;
	}
	if (mpBitmap)
	{
		mpBitmap->Release();
		mpBitmap = nullptr;
	}
	mnDstHeight = 0;
	mnDstWidth = 0;
	mnSrcHeight = 0;
	mnSrcWidth = 0;
	mhPlayWnd = NULL;
	mpImgData = NULL;
}

void CDirect2D::Render(uint8_t* pData)
{
	mpRenderTarget->BeginDraw();//跟显示刷新频率有关系  
	mpBitmap->CopyFromMemory(&mrcImage, pData, mnSrcWidth * 4);
	//该矩形大小会受到"更改文本、应用和其他项目的大小:xxx%"的影响  
	mpRenderTarget->DrawBitmap(mpBitmap, D2D1::RectF(0, 0, mrcWnd.right - mrcWnd.left, mrcWnd.bottom - mrcWnd.top));
	mpRenderTarget->EndDraw();
}
