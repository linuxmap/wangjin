#include "stdafx.h"
#include "ScreenDraw.h"


CScreenDraw::CScreenDraw()
{
	mnWidth = mnHeight = 0;
	::CoInitialize(NULL);
}


CScreenDraw::~CScreenDraw()
{
	::CoUninitialize();
}

BOOL CScreenDraw::Create(uint32_t nWidth, uint32_t nHeight)
{
	mnWidth = nWidth;
	mnHeight = nHeight;

	if (!InitDDraw())
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CScreenDraw::SetPlayWnd(HWND hPlayWnd)
{
	LPDIRECTDRAWCLIPPER pcClipper = NULL;
	if (!mlpdd)
	{
		return FALSE;
	}
	if (FAILED(mlpdd->CreateClipper(0, &pcClipper,NULL)))
	{
		return FALSE;
	}
	if (FAILED(pcClipper->SetHWnd(0, hPlayWnd)))
	{
		pcClipper->Release();
		return FALSE;
	}
	if (FAILED(mlpddsPrimary->SetClipper(pcClipper)))
	{
		pcClipper->Release();
		return FALSE;
	}

	mhPlayWnd = hPlayWnd;
	return TRUE;
}

BOOL CScreenDraw::InitDDraw()
{
	HRESULT		ddrval;

	if (mhMonitor)
	{
		ddrval = DirectDrawCreateEx(NULL, (VOID**)&mlpdd, IID_IDirectDraw7, NULL);
	}
	else
	{

	}

	if (!FAILED(ddrval))
	{
		ddrval = mlpdd->SetCooperativeLevel(NULL, DDSCL_NORMAL);
		if (!FAILED(ddrval))
		{
			ddrval = CreatePrimarySurface();
			if (!FAILED(ddrval))
			{
				return TRUE;
			}
		}
	}
	FreeDDraw();
	return FALSE;
}

HRESULT CScreenDraw::CreatePrimarySurface()
{
	DDSURFACEDESC2	ddsd;
	HRESULT			ddrval;

	if (!mlpdd)
	{
		return E_FAIL;
	}

	// create primary surface
	INIT_DIRECTDRAW_STRUCT(ddsd);
	ddsd.dwFlags		= DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	ddrval = mlpdd->CreateSurface(&ddsd, &mlpddsPrimary, NULL);

	if (FAILED(ddrval))
	{
		return ddrval;
	}

	// create back surface
	INIT_DIRECTDRAW_STRUCT(ddsd);
	ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwHeight		= mnHeight;
	ddsd.dwWidth		= mnWidth;

	DDPIXELFORMAT ddpfFormat = { sizeof(DDPIXELFORMAT), DDPF_RGB,BI_RGB,32,0x00ff0000,0x00ff00,0x0000ff,0 };
	ddsd.ddpfPixelFormat = ddpfFormat;

	ddrval = mlpdd->CreateSurface(&ddsd, &mlpddsBack, NULL);
	if (FAILED(ddrval))
	{
		return E_FAIL;
	}

	return ddrval;
}

void CScreenDraw::FreeDDraw()
{

	if (mlpddsBack)
	{
		mlpddsBack->Release();
		mlpddsBack = NULL;
	}

	if (mlpddsPrimary)
	{
		mlpddsPrimary->Release();
		mlpddsPrimary = NULL;
	}

	if (mlpdd)
	{
		mlpdd->Release();
		mlpdd = NULL;
	}
}
