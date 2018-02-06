#include "StdAfx.h"
#include "H264Dec.h"


CH264Dec::CH264Dec(void)
{
	this->Init();
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

uint16_t CH264Dec::Init()
{
	av_register_all();
	avformat_network_init();

	mThrDecode = CreateThread(NULL,0,ThreadDecode,this,0,NULL);

	BOOL bRet = SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	if(!bRet)
	{
		printf("SetPriorityClass Error.\n");
		DWORD d = GetLastError();
		printf("Error:%d.\n",d);
	}
	bRet = SetThreadPriority(mThrDecode,THREAD_PRIORITY_TIME_CRITICAL);
	if(!bRet)
	{
		printf("SetThreadPriority Error.\n");
	}
	AVCodec *pVideoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if(!pVideoCodec)
	{
		printf("Can't find decoder��");
		return -2;
	}
	mpCodecCtx = avcodec_alloc_context3(pVideoCodec);
	if(!mpCodecCtx)
	{
		printf("CodecContext alloc error��");
		return -2;
	}
	//��ʼ������������Ĳ���Ӧ���ɾ����ҵ�����  
	mpCodecCtx->time_base.num = 1;  
	mpCodecCtx->frame_number = 1;                       //ÿ��һ����Ƶ֡  
	mpCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	mpCodecCtx->bit_rate = 0;  
	mpCodecCtx->time_base.den = 30;                     //֡��  
	mpCodecCtx->width = 1280;                           //��Ƶ��  
	mpCodecCtx->height = 720;                           //��Ƶ��
	mpCodecCtx->thread_count = 2;						// ���ԣ����߳̽���
	mpCodecCtx->thread_type = 2;						// ֡�����̻߳���Slice���߳�

	if(avcodec_open2(mpCodecCtx,pVideoCodec,NULL) < 0)
	{
		printf("Open Codec error��");
		return -2;
	}

	mfnRGBHander = NULL;
	mfnpContext	= NULL;
	mhWndPlay = NULL;

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

void CH264Dec::DecodeProc()
{
	CFrameNode Node;
	int GotPicture = -1;
	AVPacket Packet = {0};

	AVFrame* pFrame = av_frame_alloc();
	AVFrame* pYUVFrame = av_frame_alloc();

	uint8_t* mpOutBuffer = NULL;
	SwsContext* pSwsCxt = NULL;

	mbThreadStop = FALSE;
	while(!mbThreadStop)
	{
		if(mMainBuf.Read(&Node))
		{
			// ���뵽һ��AVPacket��
			Packet.data = Node.mpDataBuf;
			Packet.size = Node.mnDataSize;

			// ��һ֡��ʼ��һ��
			if(!mpOutBuffer)
			{
				mpOutBuffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_RGB24, Node.mtVideoParam.mnWidth, Node.mtVideoParam.mnHeight)];  
				avpicture_fill((AVPicture *)pYUVFrame, mpOutBuffer, AV_PIX_FMT_RGB24, Node.mtVideoParam.mnWidth, Node.mtVideoParam.mnHeight);
			}

			DWORD dwBegin = GetTickCount();
			int Ret = avcodec_decode_video2(mpCodecCtx,pFrame,&GotPicture,&Packet);
			DWORD dwEnd = GetTickCount();

			mnConsuming = dwEnd - dwBegin;

			if(mnConsuming > 20)
			{
				printf("�����ʱ:%dms\n",mnConsuming);
			}
			if(Ret < 0)
			{
				printf("Decode Error.\n");
			}

			if(GotPicture)
			{
				printf("Decode Sucess.\n");
				RECT Rect;
				if(mhWndPlay)
				{
					GetClientRect(mhWndPlay,&Rect);
				}
				int nHeight = Rect.bottom - Rect.top;
				int nWidth = Rect.right - Rect.left;

				pSwsCxt = sws_getContext(mpCodecCtx->width, mpCodecCtx->height, mpCodecCtx->pix_fmt,
					mpCodecCtx->width, mpCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

				sws_scale(pSwsCxt, (const uint8_t* const*)pFrame->data, pFrame->linesize, 
					0, mpCodecCtx->height, pYUVFrame->data, pYUVFrame->linesize);

				// ��ͼ
				BITMAPINFO bmpInfo;
				bmpInfo.bmiHeader.biBitCount = 24;
				bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmpInfo.bmiHeader.biPlanes = 1;
				bmpInfo.bmiHeader.biWidth = mpCodecCtx->width;
				bmpInfo.bmiHeader.biHeight = -mpCodecCtx->height;
				bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth * bmpInfo.bmiHeader.biHeight * 3;
				bmpInfo.bmiHeader.biCompression = BI_RGB;
				bmpInfo.bmiHeader.biClrUsed = 0;
				bmpInfo.bmiHeader.biXPelsPerMeter = 0;
				bmpInfo.bmiHeader.biYPelsPerMeter = 0;
				bmpInfo.bmiHeader.biClrImportant = 0;

				HDC hdc = GetDC(mhWndPlay);
				SetStretchBltMode(hdc,HALFTONE);
				StretchDIBits(hdc,									// �豸�������
					0,												//Ŀ��X����
					0,												// Ŀ��Y����
					nWidth,											// Ŀ����
					nHeight,										// Ŀ��߶�
					0,												// ԴX����
					0,												// ԴY����
					mpCodecCtx->width,								// Դ���
					mpCodecCtx->height,								// Դ�߶�
					pYUVFrame->data[0],								//ͼ������ָ��BYTE*
					&bmpInfo,										// ָ��λͼ��Ϣ�ṹ��ָ��
					DIB_RGB_COLORS,									// ʹ�õ���ɫ��Ŀ
					SRCCOPY);
				ReleaseDC(mhWndPlay,hdc);

				sws_freeContext(pSwsCxt);
			}
		}
		//Sleep(1);
	}
}
