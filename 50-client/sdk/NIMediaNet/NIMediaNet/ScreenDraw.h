#pragma once
#include <stdint.h>
#include <ddraw.h>

#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))

typedef struct {
	HMONITOR hMonitor;
	GUID*   lpGUID;
	GUID    GUID;
	BOOL    fFound;
}FindDeviceData;

class CScreenDraw
{
public:
	CScreenDraw();
	~CScreenDraw();
public:
	BOOL Create(uint32_t nWidth,uint32_t nHeight);
	BOOL SetPlayWnd(HWND hPlayWnd);
private:
	BOOL InitDDraw();
	VOID FreeDDraw();
	HRESULT CreatePrimarySurface();
private:
	uint32_t						mnWidth;
	uint32_t						mnHeight;
	HMONITOR						mhMonitor;
	LPDIRECTDRAW7					mlpdd;
	LPDIRECTDRAWSURFACE7			mlpddsPrimary;
	LPDIRECTDRAWSURFACE7			mlpddsBack;
	LPDIRECTDRAWSURFACE7			m_lpddsRgbDec;			// ��������yuvͼ���blt��ddsback��
	SIZE							mcBackSize;				// ������¼��ǰ��surface�Ĵ�С���Ա�Աȴ��ڴ�С�����Ƿ���Ҫ�ؽ�
	SIZE							mcZoomSize;
	HWND							mhPlayWnd;
};

