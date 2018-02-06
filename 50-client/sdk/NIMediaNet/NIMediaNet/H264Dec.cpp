#include "StdAfx.h"
#include "H264Dec.h"

void dec_write(char *szFormat, ...);

static int gnCurIdealCpuMask = 0;		// 记录当前应该取的cpuMask，线程绑定cpu，从cpu的最大数向前取值，先用空余cpu
int GetIdealCpuMask()
{
	gnCurIdealCpuMask = gnCurIdealCpuMask >> 1;
	if (!gnCurIdealCpuMask)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		gnCurIdealCpuMask = 1 << (info.dwNumberOfProcessors - 1);
	}
	return gnCurIdealCpuMask;
}

CH264Dec::CH264Dec(void)
{

}

CH264Dec::~CH264Dec(void)
{
	this->Deinit();
}

DWORD WINAPI ThreadDecode(LPVOID lpThreadParameter)
{
	if(lpThreadParameter)
	{
		CH264Dec* pThis = (CH264Dec*)lpThreadParameter;
		pThis->DecodeProc();
	}
	return 0;
}

void CH264Dec::Create(HWND hPlayWnd, EMRenderMode RenderMode)
{
	mhWndPlay = hPlayWnd;
	mRenderMode = RenderMode;
	mdwFramePTS = 0;
	Init();
}

uint16_t CH264Dec::Init()
{
	av_register_all();
	avformat_network_init();

	mThrDecode = CreateThread(NULL,0,ThreadDecode,this,0,NULL);

	//BOOL bRet = SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	//if (!bRet)
	//{
	//	printf("SetPriorityClass Error.\n");
	//	DWORD d = GetLastError();
	//	printf("Error:%d.\n", d);
	//}
	//bRet = SetThreadPriority(mThrDecode, THREAD_PRIORITY_TIME_CRITICAL);
	//if (!bRet)
	//{
	//	printf("SetThreadPriority Error.\n");
	//}

	AVCodec *pVideoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if(!pVideoCodec)
	{
		printf("Can't find decoder！");
		return -2;
	}
	mpCodecCtx = avcodec_alloc_context3(pVideoCodec);
	if(!mpCodecCtx)
	{
		printf("CodecContext alloc error！");
		return -2;
	}
	//初始化参数，下面的参数应该由具体的业务决定  
	mpCodecCtx->time_base.num = 1;  
	mpCodecCtx->frame_number = 1;                       //每包一个视频帧  
	mpCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	mpCodecCtx->bit_rate = 0;  
	mpCodecCtx->time_base.den = 30;                     //帧率  
	mpCodecCtx->width = 1280;                           //视频宽  
	mpCodecCtx->height = 720;                           //视频高
	mpCodecCtx->thread_count = 2;						// 测试，多线程解码
	mpCodecCtx->thread_type = 2;						// 帧级多线程或者Slice多线程

	if(avcodec_open2(mpCodecCtx,pVideoCodec,NULL) < 0)
	{
		printf("Open Codec error！");
		return -2;
	}

	mfnRGBHander = NULL;
	mfnpContext	= NULL;

	return 0;
}

void CH264Dec::Deinit()
{
	mbThreadStop = TRUE;
	WaitForSingleObject(mThrDecode,INFINITE);

	if(INVALID_HANDLE_VALUE != mThrDecode)
	{
		CloseHandle(mThrDecode);
		mThrDecode = NULL;
	}

	avformat_network_deinit();
}

void CH264Dec::SetRGB24DataCallBack(RGB24CallBack fnCB,uint32_t pContext)
{
	mfnRGBHander = fnCB;
	mfnpContext = pContext;
}

void CH264Dec::InputData(CFrameNode& node)
{
	mMainBuf.Write(node);
}

void SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index, int bpp);

extern void trace(const TCHAR * format, ...);

// 该函数精度高，但耗费资源严重
void SleepC(double dMilliseconds)
{
	__int64 nFreq = 0; //频率
	__int64 nStart = 0; //起始计数
	if (QueryPerformanceCounter((LARGE_INTEGER*)&nStart) && QueryPerformanceFrequency((LARGE_INTEGER*)&nFreq) && nFreq > 0)
	{
		__int64 nEnd = 0; //终止计数
		double k = 1000.0 / (double)nFreq; //将计数转换为毫秒
		for (;;)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&nEnd);
			if (dMilliseconds <= (double)(nEnd - nStart) * k)
			{
				break;
			}
		}
	}
}

void CH264Dec::DecodeProc()
{
	// 提升解码线程的效率
	// 绑定至特定的cpu内核
#if 0
	int nCpuMask = GetIdealCpuMask();
	DWORD dwRet = SetThreadAffinityMask(GetCurrentThread(),nCpuMask);
#endif

	CFrameNode Node;
	int GotPicture = -1;
	AVPacket Packet = {0};

	AVFrame* pSrcFrame = av_frame_alloc();
	AVFrame* pDstFrame;

	// 上一次的宽度和长度
	uint32_t lastWidth = 0;
	uint32_t lastHeight = 0;

	uint8_t* mpOutBuffer = NULL;
	SwsContext* pSwsCxt = NULL;

	BOOL bOne = FALSE;
	//设置图像转换上下文

	mbThreadStop = FALSE;
	uint8_t* outBuf = NULL;

	HDC hdc = GetDC(mhWndPlay);

	while(!mbThreadStop)
	{
		if(mMainBuf.Read(&Node))
		{
			// 放入到一个AVPacket中
			Packet.data = Node.mpDataBuf;
			Packet.size = Node.mnDataSize;

			// 解码关键函数
			int Ret = avcodec_decode_video2(mpCodecCtx, pSrcFrame,&GotPicture,&Packet);

			if(GotPicture)
			{				
				AVPixelFormat curPixFmt = AV_PIX_FMT_NONE;
				if (WindowIsChanged(mhWndPlay, lastWidth,lastHeight))
				{
					if (mpOutBuffer)
					{
						delete mpOutBuffer;
						av_frame_free(&pDstFrame);
					}
					pDstFrame = av_frame_alloc();

					if (mRenderMode == RENDER_DIRECT2D)
					{
						curPixFmt = AV_PIX_FMT_BGRA;
						mD2Draw.Free();
						BOOL bRet = mD2Draw.Create(mhWndPlay, lastWidth, lastHeight, lastWidth, lastHeight);
						if (!bRet)
						{
							MessageBox(NULL, TEXT("D2D初始化失败！"), TEXT("ERROR"), 0);
							break;
						}
						int nBmpSize = avpicture_get_size(curPixFmt, lastWidth, lastHeight);
						try
						{
							mpOutBuffer = new uint8_t[nBmpSize];
						}
						catch (...)
						{
							printf("创建绘图缓存块失败\n");
							break;
						}
						avpicture_fill((AVPicture *)pDstFrame, mpOutBuffer, curPixFmt, lastWidth, lastHeight);
						pSwsCxt = sws_getContext(mpCodecCtx->width, mpCodecCtx->height, mpCodecCtx->pix_fmt,
							lastWidth, lastHeight,
							curPixFmt,
							SWS_BICUBIC, NULL, NULL, NULL);
					}
					else if (mRenderMode == RENDER_GDI)
					{
						curPixFmt = AV_PIX_FMT_BGR24;
						int nBmpSize = avpicture_get_size(curPixFmt, mpCodecCtx->width, mpCodecCtx->height);

						try
						{
							mpOutBuffer = new uint8_t[nBmpSize];
						}
						catch (...)
						{
							printf("创建绘图缓存块失败\n");
							break;
						}
						avpicture_fill((AVPicture *)pDstFrame, mpOutBuffer, curPixFmt, mpCodecCtx->width, mpCodecCtx->height);
						pSwsCxt = sws_getContext(mpCodecCtx->width, mpCodecCtx->height, mpCodecCtx->pix_fmt,
							mpCodecCtx->width, mpCodecCtx->height,
							curPixFmt,
							SWS_BICUBIC, NULL, NULL, NULL);
					}
					
					
				}

				sws_scale(pSwsCxt, pSrcFrame->data, pSrcFrame->linesize, 0, mpCodecCtx->height, pDstFrame->data, pDstFrame->linesize);

				DWORD dwNow = timeGetTime();

				// 平滑策略				
				if (mdwFramePTS > 0)
				{
					if (mdwFramePTS > dwNow)
					{
						DWORD dwSleepTime = mdwFramePTS - dwNow;
						//if (dwSleepTime > 2)
						//{
						//	dwSleepTime = dwSleepTime - 2;
						//}
						mVideoTimer.Sleep(dwSleepTime);
					}
					else if (mdwFramePTS < dwNow)
					{
						mdwFramePTS = dwNow + Node.mnInterval;
						//dec_write("丢帧处理\n");
						continue;
					}
				}

				// 下一帧的绘图时间：当前解码完成时间 + 每秒帧率耗时
				mdwFramePTS = timeGetTime() + Node.mnInterval;

				if (mRenderMode == RENDER_DIRECT2D)
				{
					Render(hdc, pDstFrame->data[0], lastWidth, lastHeight, lastWidth, lastHeight);
				}
				else
				{
					Render(hdc, pDstFrame->data[0], mpCodecCtx->width, mpCodecCtx->height, lastWidth, lastHeight);
				}

				//dec_write("%d %d %d\n", mdwFramePTS, mMainBuf.GetFrameLen(), Node.mnInterval);
			}
		}
		else
		{
			mMainBuf.Wait();
		}
	}
	if (pSwsCxt)
	{
		sws_freeContext(pSwsCxt);
		pSwsCxt = NULL;
	}
	ReleaseDC(mhWndPlay, hdc);
}

BOOL CH264Dec::WindowIsChanged(HWND hWnd, uint32_t& nlastWidth, uint32_t&nlastHeight)
{
	int height, width;
	RECT Rect;
	if (GetClientRect(hWnd, &Rect))
	{
		height = Rect.bottom - Rect.top;
		width = Rect.right - Rect.left;
	}
	if ((height != nlastHeight) || (width != nlastWidth))
	{
		nlastHeight = height;
		nlastWidth = width;
		return TRUE;
	}
	return FALSE;
}

void CH264Dec::Render(HDC hdc, uint8_t* pData, uint32_t nSrcWidth, uint32_t nSrcHeight, uint32_t nDstWidth, uint32_t nDstHeight)
{
	switch (mRenderMode)
	{
	case RENDER_GDI:
	{
		// 绘图
		BITMAPINFO bmpInfo = { 0 };
		bmpInfo.bmiHeader.biBitCount = 24;
		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biWidth = nSrcWidth;
		bmpInfo.bmiHeader.biHeight = -nSrcHeight;
		bmpInfo.bmiHeader.biSizeImage = (nSrcWidth * 24 + 31) / 32 * 4 * nSrcHeight;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biClrUsed = 0;
		bmpInfo.bmiHeader.biXPelsPerMeter = 100;
		bmpInfo.bmiHeader.biYPelsPerMeter = 100;
		bmpInfo.bmiHeader.biClrImportant = 0;

		SetStretchBltMode(hdc, HALFTONE);
		StretchDIBits(hdc,									// 设备环境句柄
			0,												//目标X坐标
			0,												// 目标Y坐标
			nDstWidth,										// 目标宽度
			nDstHeight,										// 目标高度
			0,												// 源X坐标
			0,												// 源Y坐标
			nSrcWidth,										// 源宽度
			nSrcHeight,										// 源高度
			pData,											//图像数据指针BYTE*
			&bmpInfo,										// 指向位图信息结构的指针
			DIB_RGB_COLORS,									// 使用的颜色数目
			SRCCOPY);
	}
		break;
	case RENDER_DIRECT2D:
	{
		mD2Draw.Render(pData);
	}
		break;
	default:
		break;
	}
}

//保存BMP文件的函数   
void SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index, int bpp)
{
	char buf[5] = { 0 };
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	FILE *fp;

	char *filename = new char[255];
	//文件存放路径，根据自己的修改   
	sprintf_s(filename, 255, "%s%d.bmp", "D:/Mtest", index);
	if ((fp = fopen(filename, "wb+")) == NULL)
	{
		printf("open file failed!\n");
		return;
	}

	bmpheader.bfType = 0x4d42;
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp / 8;

	bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.biWidth = width;
	bmpinfo.biHeight = -height;
	bmpinfo.biPlanes = 1;
	bmpinfo.biBitCount = bpp;
	bmpinfo.biCompression = BI_RGB;
	bmpinfo.biSizeImage = (width*bpp + 31) / 32 * 4 * height;
	bmpinfo.biXPelsPerMeter = 100;
	bmpinfo.biYPelsPerMeter = 100;
	bmpinfo.biClrUsed = 0;
	bmpinfo.biClrImportant = 0;

	fwrite(&bmpheader, sizeof(bmpheader), 1, fp);
	fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp);
	fwrite(pFrameRGB->data[0], width*height*bpp / 8, 1, fp);

	fclose(fp);
}

void dec_write(char *szFormat, ...)
{
	char szMsg[8192] = { 0 };
	struct tm *now;
	time_t curtime;
	int nlen;
	va_list pvlist;
	int  nstrLen;

	time(&curtime);
	now = localtime(&curtime);
	nlen = sprintf(szMsg, "%d-%d-%d %02d:%02d:%02d ",
		now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

	va_start(pvlist, szFormat);
	nstrLen = vsprintf(szMsg + nlen, szFormat, pvlist);
	if (nstrLen <= 0 || nstrLen >= 8192)
	{
		va_end(pvlist);
		return;
	}
	va_end(pvlist);

	FILE *pFile;
	char achLogPathName[255];
	sprintf(achLogPathName, "ClientSDK.log");

	pFile = fopen(achLogPathName, "a+");
	if (pFile != NULL)
	{
		int dwFileLen;
		fseek(pFile, 0, SEEK_END);
		dwFileLen = ftell(pFile);
	}

	fputs(szMsg, pFile);
	fclose(pFile);
}
