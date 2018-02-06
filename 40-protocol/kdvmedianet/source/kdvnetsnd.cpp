/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvNetSnd.cpp
相关文件    : KdvNetSnd.h
文件实现功能: CKdvNetSnd Implement
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2003/06/03  2.0         魏治兵      Add RTP Option
2004/04/13  3.0         万春雷      增加重传处理功能
2004/06/10  3.0         万春雷      增加H.263对于 B模式 的发送请求
2004/08/13  3.0         万春雷      对于H.263/H.261 改为放入缓冲批量发送以便均衡发送码流
2004/09/22  3.0         万春雷      增加发送端对于H.264 动态载荷的支持
2004/09/29  3.0         万春雷      增加发送端对于H.224 动态载荷的支持
2004/09/29  2.0         万春雷      增加linux版本编译支持
2004/09/30  3.0         万春雷      增加发送端对于发送码流的加密的支持
2004/10/08  3.0         万春雷      增加发送端对于H.263+ 动态载荷的支持
2004/10/12  3.0         万春雷      加密数据处理调整
2004/11/02  3.0         万春雷      视频码流均放入缓冲并按照最大发送字节数 批量发送数据包
2004/11/16  3.0         万春雷      增加对于 mpeg2 以自定义切包方式的网络收发的支持
2004/11/20  3.0         万春雷      发送缓冲已满时，返回前将原有缓冲数据进行发送
2004/11/30  3.0         万春雷      增加AES加密模式支持
2004/12/01  3.0         万春雷      修改AES加解密的初始化向量值计算SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
2004/12/29  3.0         万春雷      调整 发送端加密设置接口，增加动态载荷值的设置接口
2005/01/18  3.5         万春雷      增加对嵌入式操作系统调用以太网平滑发送接口设置平滑（暂时禁用）
2005/01/26  3.5         万春雷      增加发送端对于 G.729 载荷的支持
2005/05/14  3.6         华亮        增加RTCP组播（发送SR）
******************************************************************************/


#include "kdvnetsnd.h"

#define CREATE_CHECK                      \
    if(m_pcRtp == NULL|| m_pcRtcp == NULL)\
    {                                      \
        return ERROR_SND_NOCREATE;      \
    }                                      \

extern s32  g_dwMaxExtendPackSize;

extern s32   g_nShowDebugInfo;
extern BOOL32  g_bSelfSnd;
extern BOOL32 g_bUseMemPool;

//统一提供收发对象的SSRC的操作
extern u32 GetExclusiveSSRC();

//H.261 、 H.263 按照原始rtp包直接发送的开关
extern s32   g_nRtpDSend;

//发送端模拟小包乱序的步长间隔
extern s32   g_nConfuedSpan;

//发送端模拟 H.263小包乱序的步长间隔是否发生变化
extern s32   g_nSpanChanged;

// 查询是否使用定时发送以太网数据以达到平滑发送
extern BOOL32 g_bUseSmoothSnd;
// 当前OS是否支持网络平滑发送
extern BOOL32 g_bBrdEthSndUseTimer;

//2006-06-08 hual
extern s32 g_nRateRadioLess1M; //300
extern s32 g_nRateRadioMore1M; //200

u16 TsPsCallBack(u8 *pu8Buf, u32 u32Len, KD_PTR pvContext, TspsFRAMEHDR *ptFrame)
{
    CKdvNetSnd *pMain = (CKdvNetSnd*)pvContext;
    if (NULL == pvContext)
    {
        return -1;
    }
    if(ptFrame)
    {
        ptFrame->m_dwDataSize = u32Len;
        ptFrame->m_pbyData = pu8Buf;
        ptFrame->m_byMediaType = MEDIA_TYPE_PS;
        pMain->SendPS(ptFrame);
    }

    return 0;
}

/*初始化类成员*/
CKdvNetSnd::CKdvNetSnd()
{
    m_pcRtp                = NULL;
    m_pcRtcp            = NULL;
    m_ptH261HeaderList  = NULL;
    m_ptH263HeaderList  = NULL;
    m_ptH264HeaderList  = NULL;
    m_dwFrameId            = 0;
    m_dwTimeStamp        = 0;
    m_byMediaType        = 0;
    m_dwMaxFrameSize    = 0;
    m_nMaxSendNum        = 0;
    m_nMaxSendSize      = 0;
    m_byFrameRate       = 0;

    m_pSelfFrameBuf     = NULL;

    m_byLastEBit        = 0;

    m_bRepeatSend       = FALSE;
    m_wBufTimeSpan      = 0;

    m_pszMaterialBuf    = NULL;
    m_pbyEncryptInBuf   = NULL;
    m_pbyEncryptOutBuf  = NULL;
    m_byLocalActivePT   = 0;
    m_wMaterialBufLen   = 0;

    m_bVidPayload       = FALSE;
    m_byRmtAddrNum      = 0;
    m_dwCurBandWidth    = 0;

    m_dwFirstTimeStamps = 0;
    m_bFirstFrame       = TRUE;
    m_hTspsWrite        = NULL;
    m_tPSInfo.m_byAudioType = MEDIA_TYPE_NULL;
    m_tPSInfo.m_byVideoType = MEDIA_TYPE_NULL;
    memset(&m_tKdvSndStatus, 0, sizeof(m_tKdvSndStatus));
    memset(&m_tKdvSndStatistics, 0, sizeof(m_tKdvSndStatistics));
    memset(&m_tOldRtp, 0, sizeof(m_tOldRtp));
    m_bSendRtp          = TRUE;

    m_byBuffMultipleTimes = 1;
    m_nSendRate = 0;
    m_bAaclcNoHead = FALSE;
	
	memset(&m_tNatProbeProp, 0, sizeof(TNatProbeProp));
	m_dwNatLastTs = 0;

}

/*销毁对象*/
CKdvNetSnd::~CKdvNetSnd()
{
    FreeBuf();
}

/*=============================================================================
    函数名        ：FreeBuf
    功能        ：和Create函数相对应，清除Create函数里创建的对象
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无
    返回值说明：  无
=============================================================================*/
void CKdvNetSnd::FreeBuf()
{
    SAFE_DELETE(m_pcRtcp)
    SAFE_DELETE(m_pcRtp)

    SAFE_DELETE(m_ptH261HeaderList)
    SAFE_DELETE(m_ptH263HeaderList)
    if (m_ptH264HeaderList)
    {
        free(m_ptH264HeaderList);
        m_ptH264HeaderList = NULL;
    }

    MEDIANET_SAFE_FREE(m_pSelfFrameBuf)

    memset(&m_tSelfFrmHdr, 0, sizeof(m_tSelfFrmHdr));

    if( NULL != m_pbyEncryptOutBuf )
    {
       u32 dwH26XHeadLen = MAX_H263_HEADER_LEN;
       if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
       {
           dwH26XHeadLen = MAX_H261_HEADER_LEN;
       }
       m_pbyEncryptOutBuf -= dwH26XHeadLen;
       MEDIANET_SAFE_FREE(m_pbyEncryptOutBuf)
    }
    MEDIANET_SAFE_FREE(m_pbyEncryptInBuf)

    SAFE_DELETE(m_pszMaterialBuf)
    m_byLocalActivePT = 0;
    m_wMaterialBufLen = 0;

    m_dwFrameId         = 0;
    m_dwTimeStamp     = 0;
    m_byMediaType     = 0;
    m_dwMaxFrameSize = 0;
    m_nMaxSendNum     = 0;
    m_nMaxSendSize   = 0;
    m_byFrameRate    = 0;

    m_byLastEBit     = 0;

    m_bRepeatSend    = FALSE;
    m_wBufTimeSpan   = 0;
    m_bSendRtp       = TRUE;

    m_byBuffMultipleTimes = 1;
    // 通知网络驱动释放相应发送带宽 2005-01-18
#ifdef _VXWORKS_
    if( (TRUE == g_bUseSmoothSnd) && (TRUE == g_bBrdEthSndUseTimer) &&
        (0 != m_byRmtAddrNum) && (0 != m_byRmtAddrNum) )
    {
        // RemoveBR
        BrdMoveEthBautRate( 0, m_dwCurBandWidth*m_byRmtAddrNum );
        BrdMoveEthBautRate( 1, m_dwCurBandWidth*m_byRmtAddrNum );

        if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf( TRUE, FALSE, "[FreeBuf] RemoveBR Total.%d, CurBandWidth.%d, byRmtAddrNum.%d\n" ,
                      m_dwCurBandWidth*m_byRmtAddrNum, m_dwCurBandWidth, m_byRmtAddrNum );
        }

    }
#endif
    m_bVidPayload    = FALSE;
    m_byRmtAddrNum   = 0;
    m_dwCurBandWidth = 0;

    memset(&m_tKdvSndStatus, 0, sizeof(m_tKdvSndStatus));
    memset(&m_tKdvSndStatistics, 0, sizeof(m_tKdvSndStatistics));
    memset(&m_tOldRtp, 0, sizeof(m_tOldRtp));
    m_tPSInfo.m_byVideoType = MEDIA_TYPE_NULL;
    m_tPSInfo.m_byAudioType = MEDIA_TYPE_NULL;
    if (NULL != m_hTspsWrite)
    {
        TspsWriteWriteEnd(m_hTspsWrite);
        TspsWriteClose(m_hTspsWrite);
        m_hTspsWrite = NULL;
    }
    m_nSendRate = 0;
    	//free buf
	if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
	{
		free(m_tNatProbeProp.tRtpNatProbePack.pbyBuf);
		m_tNatProbeProp.tRtpNatProbePack.pbyBuf = NULL;
	}

	if(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
	{
		free(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf);
		m_tNatProbeProp.tRtcpNatProbePack.pbyBuf = NULL;
	}
	memset(&m_tNatProbeProp, 0, sizeof(TNatProbeProp));	
	m_dwNatLastTs = 0;

	return;
}

/*=============================================================================
    函数名        ：FreeBuf
    功能        ：初始化变量，分配空间，生成对象。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  dwMaxFrameSize  最大帧大小,用于预分配空间.
                  dwNetBand       等于平均一秒钟发送的数据量乘以一个
                                  发送系数如1.5,用于计算平均每次应该
                                  发送的数据量，单位是bit
                  byFrameRate     帧率，一秒钟发送的数据次数。
                  byMediaType     媒体类型
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::Create ( u32 dwMaxFrameSize, u32 dwNetBand,
                         u8 byFrameRate, u8 byMediaType,
                         u32 dwSSRC /* = 0*/, TPSInfo *ptPSInfo /*= NULL*/)
{
    if( (dwMaxFrameSize == 0) || (dwMaxFrameSize > MAX_FRAME_SIZE) || 
		(dwNetBand == 0) || 
		(byMediaType != MEDIA_TYPE_MP3 &&
		 byMediaType != MEDIA_TYPE_MP4 &&  //mpeg4
		 byMediaType != MEDIA_TYPE_H262 &&  //mpeg2
		 byMediaType != MEDIA_TYPE_MJPEG && //mjpeg
		 byMediaType != MEDIA_TYPE_H264 &&
		 byMediaType != MEDIA_TYPE_H265 &&
		 byMediaType != MEDIA_TYPE_H261 &&
         byMediaType != MEDIA_TYPE_H263 &&
         byMediaType != MEDIA_TYPE_H263PLUS &&
         byMediaType != MEDIA_TYPE_PCMU &&
         byMediaType != MEDIA_TYPE_PCMA &&
         byMediaType != MEDIA_TYPE_G7231 &&
         byMediaType != MEDIA_TYPE_G722 &&
         byMediaType != MEDIA_TYPE_G728 &&
         byMediaType != MEDIA_TYPE_G729 &&
         byMediaType != MEDIA_TYPE_ADPCM &&
         byMediaType != MEDIA_TYPE_G7221C &&
         byMediaType != MEDIA_TYPE_H224 &&
         byMediaType != MEDIA_TYPE_AACLC &&
         byMediaType != MEDIA_TYPE_AACLD &&
		 byMediaType != MEDIA_TYPE_AACLC_PCM &&
		 byMediaType != MEDIA_TYPE_AMR &&
		 byMediaType != MEDIA_TYPE_PS &&
		 byMediaType != MEDIA_TYPE_G726_16 &&
		 byMediaType != MEDIA_TYPE_G726_24 &&
		 byMediaType != MEDIA_TYPE_G726_32 &&
		 byMediaType != MEDIA_TYPE_G726_40) )
    {
        return ERROR_SND_PARAM;
    }

    FreeBuf(); //清空所有对象

    if(0 == byFrameRate)
    {
        byFrameRate = 25;
    }

    m_byBuffMultipleTimes = dwMaxFrameSize / MAX_VIDEO_FRAME_SIZE;
    if(!m_byBuffMultipleTimes) // if its 0,set to 1.
        m_byBuffMultipleTimes = 1;
	
	//给NAT 探测结构体分配内存
	if(NULL == m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
	{
		m_tNatProbeProp.tRtpNatProbePack.pbyBuf = (u8 *)malloc(sizeof(u8) * MAX_SND_PACK_SIZE);
	}

	if(NULL == m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
	{
		m_tNatProbeProp.tRtcpNatProbePack.pbyBuf = (u8 *)malloc(sizeof(u8) * MAX_SND_PACK_SIZE);
	}


	//创建rtp对象
    m_pcRtp = new CKdvRtp;
    if(m_pcRtp == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    //创建rtcp对象
    m_pcRtcp = new CKdvRtcp;
    if(m_pcRtcp == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }
    MEDIANET_MALLOC(m_pbyEncryptInBuf, MAX_SND_ENCRYPT_PACK_SIZE+4);
    if( NULL == m_pbyEncryptInBuf )
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    u32 dwH26XHeadLen = MAX_H263_HEADER_LEN;
    if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
    {
        dwH26XHeadLen = MAX_H261_HEADER_LEN;
    }
    MEDIANET_MALLOC(m_pbyEncryptOutBuf, dwH26XHeadLen+MAX_SND_ENCRYPT_PACK_SIZE+4);
    if( NULL == m_pbyEncryptOutBuf )
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }
    memset(m_pbyEncryptOutBuf, 0, (dwH26XHeadLen+MAX_SND_ENCRYPT_PACK_SIZE+4));
    m_pbyEncryptOutBuf += dwH26XHeadLen;

    if(0 == dwSSRC)
    {
        dwSSRC = GetExclusiveSSRC();
    }

    m_dwTimeStamp = 0;
    if (g_nShowDebugInfo == 11)
    {
        OspPrintf(TRUE, FALSE, "Create m_dwTimeStamp = 0 \n");
    }

    m_dwFrameId++;
    m_byFrameRate = byFrameRate;

    u16 wRet = MEDIANET_NO_ERROR;
    BOOL32 bAllocLPBuf = FALSE;
    BOOL32 bVidPayload = FALSE;

    //对于mp4、mp2、h.264、h.261、h.263 h.263+ 视频数据
    //我们 需要放入环形缓冲后，再进行发送，以便进行码流控制
    if( byMediaType == MEDIA_TYPE_MP4 ||  //mpeg4
        byMediaType == MEDIA_TYPE_H262 ||  //mpeg2
        byMediaType == MEDIA_TYPE_MJPEG || //mjpeg
        byMediaType == MEDIA_TYPE_H264 ||
        byMediaType == MEDIA_TYPE_H265 ||
        byMediaType == MEDIA_TYPE_H261 ||
        byMediaType == MEDIA_TYPE_H263 ||
        byMediaType == MEDIA_TYPE_H263PLUS ||
        ptPSInfo != NULL)
    {
        bAllocLPBuf = TRUE;
        bVidPayload = TRUE;
    }
    if (NULL != ptPSInfo)
    {
        m_hTspsWrite = TspsWriteOpen(PROGRAM_STREAM, TsPsCallBack, (KD_PTR)this, dwMaxFrameSize);
        if (NULL == m_hTspsWrite)
        {
            FreeBuf();
            return ERROR_SND_PSOPEN;
        }
        memcpy(&m_tPSInfo, ptPSInfo, sizeof(TPSInfo));
        wRet = TspsWriteSetProgram(m_hTspsWrite, m_tPSInfo.m_byVideoType, m_tPSInfo.m_byAudioType);
        if (wRet != TSPS_OK)
        {
            FreeBuf();
            return ERROR_SND_PSSETPROGRAM;
        }
    }
    wRet = m_pcRtp->Create( dwSSRC, bAllocLPBuf, bVidPayload, m_byBuffMultipleTimes);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    wRet = m_pcRtcp->Create(dwSSRC);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //设置和RTP对应的RTCP。
    m_pcRtp->SetRtcp(m_pcRtcp);

    //设置和RTCP对应的RTP。
    m_pcRtcp->SetRtp(m_pcRtp);

    m_byMediaType     = byMediaType;
    m_dwMaxFrameSize = dwMaxFrameSize;
    if(m_byMediaType == MEDIA_TYPE_H261)
    {
        m_ptH261HeaderList = new TH261HeaderList;
        if(m_ptH261HeaderList == NULL)
        {
            FreeBuf();
            return ERROR_SND_MEMORY;
        }
    }
    //h.263+ 作为特殊的h.263码流集中处理
    else if(m_byMediaType == MEDIA_TYPE_H263 || m_byMediaType == MEDIA_TYPE_H263PLUS)
    {
        m_ptH263HeaderList = new TH263HeaderList;
        if(m_ptH263HeaderList == NULL)
        {
            FreeBuf();
            return ERROR_SND_MEMORY;
        }
    }
    else if(m_byMediaType == MEDIA_TYPE_H264)
    {
        m_ptH264HeaderList = (TH264HeaderList*)malloc(sizeof(TH264HeaderList));
        if(m_ptH264HeaderList == NULL)
        {
            FreeBuf();
            return ERROR_SND_MEMORY;
        }
    }

    //OspPrintf(TRUE, FALSE, "Create:%d,%d\n", dwNetBand, byFrameRate);

    s32 nSendRate = dwNetBand;

    //小于2M的乘以3，大于2M的乘以2
    if (TRUE == m_bVidPayload)
    {
        if( dwNetBand < (1<<20))
        {
            nSendRate = dwNetBand * g_nRateRadioLess1M / 100;
        }
        else
        {
            nSendRate = dwNetBand * g_nRateRadioMore1M / 100;
        }
    }

    m_nSendRate = nSendRate; //Record Send Rate
    //根据 发送码流/帧率 计算每次最大发送包数;
    s32  nAveFrame = (s32)(nSendRate/(byFrameRate*8));
    m_nMaxSendNum  = (nAveFrame + g_dwMaxExtendPackSize - 1) / g_dwMaxExtendPackSize;
    //最大不应超过环形缓冲存储的包总数
    if(m_nMaxSendNum > LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes)
    {
        m_nMaxSendNum = LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes;
    }

    // 音频的编码码率统一按照 MAX_AUDIO_BITRATE 计算
    u32 dwMediaBand = MAX_AUDIO_BITRATE; //Kbps
    // 上层对于视频的编码码率，在换算成发送码流时进行了系数调整，这里将其折算获取编码码率
    if( TRUE == bVidPayload )
    {
        dwMediaBand = dwNetBand/1024; //Kbps
    }
    m_bVidPayload    = bVidPayload;
    m_dwCurBandWidth = dwMediaBand;

    //根据 发送码流/帧率 计算每次最大发送字节数;
    m_nMaxSendSize = nAveFrame;
    //最大不应超过环形缓冲存储的字节尺寸总数
    if(m_nMaxSendSize > LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes*MAX_SND_ENCRYPT_PACK_SIZE)
    {
        m_nMaxSendSize = LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes*MAX_SND_ENCRYPT_PACK_SIZE;
    }

    m_tKdvSndStatus.m_byMediaType    = m_byMediaType;
    m_tKdvSndStatus.m_dwFrameID        = m_dwFrameId;
    m_tKdvSndStatus.m_dwMaxFrameSize= m_dwMaxFrameSize;
    m_tKdvSndStatus.m_dwNetBand     = dwNetBand;
    m_tKdvSndStatus.m_byFrameRate    = m_byFrameRate;

    memset(&m_tSelfFrmHdr, 0, sizeof(m_tSelfFrmHdr));
    m_tSelfFrmHdr.m_byMediaType        = m_byMediaType;
    m_tSelfFrmHdr.m_byFrameRate        = m_byFrameRate;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SetSndInfo
    功能        ：设置网络发送带宽和帧率参数
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  dwNetBand     网络带宽
                  byFrameRate   帧率

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SetSndInfo(u32 dwNetBand, u8 byFrameRate)
{
    //OspPrintf(TRUE, FALSE, "SetSndInfo:%d,%d\n", dwNetBand, byFrameRate);

    s32 nSendRate = dwNetBand;

    if (TRUE == m_bVidPayload)
    {
        if( dwNetBand < (1<<20))
        {
            nSendRate = dwNetBand * g_nRateRadioLess1M / 100;
        }
        else
        {
            nSendRate = dwNetBand * g_nRateRadioMore1M / 100;
        }
    }

    //根据 发送码流/帧率 计算每次最大发送包数;
	if (0 == byFrameRate)
	{
		byFrameRate = 25;
	}
	s32  nAveFrame = (s32)(nSendRate / (byFrameRate * 8));
    m_nMaxSendNum  = (nAveFrame +  g_dwMaxExtendPackSize - 1) / g_dwMaxExtendPackSize;
    //最大不应超过环形缓冲存储的包总数
    if(m_nMaxSendNum > LOOP_BUF_UINT_NUM)
    {
        m_nMaxSendNum = LOOP_BUF_UINT_NUM;
    }

    // 音频的编码码率统一按照 MAX_AUDIO_BITRATE 计算
    u32 dwMediaBand = MAX_AUDIO_BITRATE; //Kbps
    // 上层对于视频的编码码率，在换算成发送码流时进行了系数调整，这里将其折算获取编码码率
    if( TRUE == m_bVidPayload )
    {
        dwMediaBand = dwNetBand/1024;   //Kbps
    }

    // 检测投递的媒体码流带宽（平均码率）是否有变化，
    // 有则通知网络驱动调整相应发送带宽 2005-01-18
#ifdef _VXWORKS_
    if( (0 != m_byRmtAddrNum) &&
        (m_dwCurBandWidth != dwMediaBand) &&
        (TRUE == g_bUseSmoothSnd) && (TRUE == g_bBrdEthSndUseTimer) )
    {
        if( m_dwCurBandWidth < dwMediaBand )
        {
            // AddBR
            BrdAddEthBautRate( 0, (dwMediaBand-m_dwCurBandWidth)*m_byRmtAddrNum );
            BrdAddEthBautRate( 1, (dwMediaBand-m_dwCurBandWidth)*m_byRmtAddrNum );

            if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf( TRUE, FALSE, "[SetSndInfo] AddBR Total.%d, CurBandWidth.%d, byRmtAddrNum.%d\n" ,
                         (dwMediaBand-m_dwCurBandWidth)*m_byRmtAddrNum, m_dwCurBandWidth, dwMediaBand, m_byRmtAddrNum );
            }
        }
        else if( m_dwCurBandWidth > dwMediaBand )
        {
            // RemoveBR
            BrdMoveEthBautRate( 0, (m_dwCurBandWidth-dwMediaBand)*m_byRmtAddrNum );
            BrdMoveEthBautRate( 1, (m_dwCurBandWidth-dwMediaBand)*m_byRmtAddrNum );

            if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf( TRUE, FALSE, "[SetSndInfo] RemoveBR Total.%d, CurBandWidth.%d, byRmtAddrNum.%d\n" ,
                          (m_dwCurBandWidth-dwMediaBand)*m_byRmtAddrNum, m_dwCurBandWidth, dwMediaBand, m_byRmtAddrNum );
            }
        }
    }
#endif
    m_dwCurBandWidth = dwMediaBand;

    //根据 发送码流/帧率 计算每次最大发送字节数;
    m_nMaxSendSize = nAveFrame;
    //最大不应超过环形缓冲存储的字节尺寸总数
    if(m_nMaxSendSize > LOOP_BUF_UINT_NUM*MAX_SND_ENCRYPT_PACK_SIZE)
    {
        m_nMaxSendSize = LOOP_BUF_UINT_NUM*MAX_SND_ENCRYPT_PACK_SIZE;
    }

    m_byFrameRate = byFrameRate;
    m_tKdvSndStatus.m_dwNetBand      = dwNetBand;
    m_tKdvSndStatus.m_dwMaxFrameSize = m_dwMaxFrameSize;
    m_tKdvSndStatus.m_byFrameRate    = m_byFrameRate;
    m_tSelfFrmHdr.m_byFrameRate         = m_byFrameRate;

    //重置重传发送缓冲时间长度，重传参数依赖于帧率，需要最后设置
    if(TRUE == m_bRepeatSend)
    {
        ResetRSFlag(m_wBufTimeSpan, TRUE);
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SetNetSndParam
    功能        ：设置网络发送参数(进行底层套结子的创建，绑定端口,以及发送目标地址的设定等动作)
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  tNetSndParam  网络参数,参见结构定义部分

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SetNetSndParam ( TNetSndParam tNetSndParam )
{
    CREATE_CHECK    //是否创建

    if(tNetSndParam.m_byNum > MAX_NETSND_DEST_NUM)
    {
        return ERROR_SND_PARAM;
    }

    u16 wRet = MEDIANET_NO_ERROR;


    //设置RTP当地地址
    wRet= m_pcRtp->SetLocalAddr(tNetSndParam.m_tLocalNet.m_dwRTPAddr,
                                tNetSndParam.m_tLocalNet.m_wRTPPort,
                                FALSE,
                                tNetSndParam.m_tLocalNet.m_dwRtpUserDataLen,
                                tNetSndParam.m_tLocalNet.m_abyRtpUserData);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //设置RTP发送地址对
    u8  byCastType = 0;
    u32 dwCastIP   = 0;
    u8  byRmtAddrNum = 0;
    TRemoteAddr tRemoteAddr;
    memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
    tRemoteAddr.m_byNum = tNetSndParam.m_byNum;
    for(s32 i=0; i<tRemoteAddr.m_byNum; i++)
    {
        tRemoteAddr.m_tAddr[i].m_dwIP = tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;

        if(TRUE == IsBCastAddr(tRemoteAddr.m_tAddr[i].m_dwIP))
        {
            byCastType = 2;
            dwCastIP  = tRemoteAddr.m_tAddr[i].m_dwIP;
        }
        if(TRUE == IsMCastAddr(tRemoteAddr.m_tAddr[i].m_dwIP))
        {
            byCastType = 1;
            dwCastIP  = tRemoteAddr.m_tAddr[i].m_dwIP;
        }

        // 计算实际投递目标地址时应过滤 环回及无效地址
        if( (0 != tRemoteAddr.m_tAddr[i].m_dwIP) &&
            (inet_addr("127.0.0.1") != tRemoteAddr.m_tAddr[i].m_dwIP) )
        {
            byRmtAddrNum++;
        }

        tRemoteAddr.m_tAddr[i].m_wPort = tNetSndParam.m_tRemoteNet[i].m_wRTPPort;

        if (tNetSndParam.m_tRemoteNet[i].m_dwRtpUserDataLen >= 0 && tNetSndParam.m_tRemoteNet[i].m_dwRtpUserDataLen <= MAX_USERDATA_LEN)
        {
            tRemoteAddr.m_tAddr[i].m_dwUserDataLen = tNetSndParam.m_tRemoteNet[i].m_dwRtpUserDataLen;
            memcpy(tRemoteAddr.m_tAddr[i].m_abyUserData, tNetSndParam.m_tRemoteNet[i].m_abyRtpUserData,
                   tRemoteAddr.m_tAddr[i].m_dwUserDataLen);
        }
    }

    if(0 != byCastType)
    {
        if(1 == byCastType)
        {
            m_pcRtp->SetMCastOpt(tNetSndParam.m_tLocalNet.m_dwRTPAddr, dwCastIP, FALSE);
        }
        if(2 == byCastType)
        {
            m_pcRtp->SetBCastOpt();
        }
    }

    wRet= m_pcRtp->SetRemoteAddr(tRemoteAddr);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //设置RTCP当地地址
    wRet= m_pcRtcp->SetLocalAddr(tNetSndParam.m_tLocalNet.m_dwRTCPAddr,
                                 tNetSndParam.m_tLocalNet.m_wRTCPPort,
                                 tNetSndParam.m_tLocalNet.m_dwRtcpUserDataLen,
                                 tNetSndParam.m_tLocalNet.m_abyRtcpUserData);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //设置RTCP发送地址对
    byCastType = 0;
    dwCastIP = 0;
    memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
    tRemoteAddr.m_byNum = tNetSndParam.m_byNum;
    for(s32 j=0; j<tRemoteAddr.m_byNum; j++)
    {
        tRemoteAddr.m_tAddr[j].m_dwIP = tNetSndParam.m_tRemoteNet[j].m_dwRTCPAddr;

        if(TRUE == IsBCastAddr(tRemoteAddr.m_tAddr[j].m_dwIP))
        {
            byCastType = 2;
            dwCastIP  = tRemoteAddr.m_tAddr[j].m_dwIP;
        }
        if(TRUE == IsMCastAddr(tRemoteAddr.m_tAddr[j].m_dwIP))
        {
            byCastType = 1;
            dwCastIP  = tRemoteAddr.m_tAddr[j].m_dwIP;
        }

        tRemoteAddr.m_tAddr[j].m_wPort = tNetSndParam.m_tRemoteNet[j].m_wRTCPPort;

        if (tNetSndParam.m_tRemoteNet[j].m_dwRtcpUserDataLen >= 0 && tNetSndParam.m_tRemoteNet[j].m_dwRtcpUserDataLen <= MAX_USERDATA_LEN)
        {
            tRemoteAddr.m_tAddr[j].m_dwUserDataLen = tNetSndParam.m_tRemoteNet[j].m_dwRtcpUserDataLen;
            memcpy(tRemoteAddr.m_tAddr[j].m_abyUserData, tNetSndParam.m_tRemoteNet[j].m_abyRtcpUserData,
                   tRemoteAddr.m_tAddr[j].m_dwUserDataLen);
        }
    }

    if(0 != byCastType)
    {
        if(1 == byCastType)
        {
            m_pcRtcp->SetMCastOpt(tNetSndParam.m_tLocalNet.m_dwRTCPAddr, dwCastIP, FALSE);
        }
        if(2 == byCastType)
        {
            m_pcRtcp->SetBCastOpt();
        }
    }

    wRet= m_pcRtcp->SetRemoteAddr(tRemoteAddr);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    m_tKdvSndStatus.m_tSendAddr = tNetSndParam;

    // 检测投递的目标地址是否有变化，
    // 有则通知网络驱动调整相应发送带宽 2005-01-18
#ifdef _VXWORKS_
    if( (0 != m_dwCurBandWidth) &&
        (m_byRmtAddrNum != byRmtAddrNum) &&
        (TRUE == g_bUseSmoothSnd) && (TRUE == g_bBrdEthSndUseTimer) )
    {
        if( m_byRmtAddrNum < byRmtAddrNum )
        {
            // AddBR
            BrdAddEthBautRate( 0, (byRmtAddrNum-m_byRmtAddrNum)*m_dwCurBandWidth );
            BrdAddEthBautRate( 1, (byRmtAddrNum-m_byRmtAddrNum)*m_dwCurBandWidth );

            if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf( TRUE, FALSE, "[SetNetSndParam] AddBR Total.%d, OldRmtAddrNum.%d, NewRmtAddrNum.%d, CurBandWidth.%d\n" ,
                           (byRmtAddrNum-m_byRmtAddrNum)*m_dwCurBandWidth, m_byRmtAddrNum, byRmtAddrNum, m_dwCurBandWidth );
            }
        }
        else if( m_byRmtAddrNum > byRmtAddrNum )
        {
            // RemoveBR
            BrdMoveEthBautRate( 0, (m_byRmtAddrNum-byRmtAddrNum)*m_dwCurBandWidth );
            BrdMoveEthBautRate( 1, (m_byRmtAddrNum-byRmtAddrNum)*m_dwCurBandWidth );

            if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf( TRUE, FALSE, "[SetNetSndParam] RemoveBR Total.%d, OldRmtAddrNum.%d, NewRmtAddrNum.%d, CurBandWidth.%d\n" ,
                          (m_byRmtAddrNum-byRmtAddrNum)*m_dwCurBandWidth, m_byRmtAddrNum, byRmtAddrNum, m_dwCurBandWidth );
            }

        }
    }
#endif
    m_byRmtAddrNum = byRmtAddrNum;

    return MEDIANET_NO_ERROR;
}


u16 CKdvNetSnd::GetNetSndParam(TNetSndParam* ptNetSndParam)
{
    CREATE_CHECK    //是否创建

    if (NULL == ptNetSndParam)
    {
        return ERROR_SND_PARAM;
    }

    memset(ptNetSndParam, 0, sizeof(TNetSndParam));

    u16 wRet = MEDIANET_NO_ERROR;
    u32 dwLocalIP;
    u16 wLocalPort;

    wRet= m_pcRtp->GetLocalAddr(&dwLocalIP, &wLocalPort);

    ptNetSndParam->m_tLocalNet.m_dwRTPAddr = dwLocalIP;
    ptNetSndParam->m_tLocalNet.m_wRTPPort = wLocalPort;

    TRemoteAddr tRemoteAddr;
    wRet= m_pcRtp->GetRemoteAddr(tRemoteAddr);

    ptNetSndParam->m_byNum = tRemoteAddr.m_byNum;

    s32 i;
    for(i = 0; i < tRemoteAddr.m_byNum; i++)
    {
        ptNetSndParam->m_tRemoteNet[i].m_dwRTPAddr = tRemoteAddr.m_tAddr[i].m_dwIP;
        ptNetSndParam->m_tRemoteNet[i].m_wRTPPort = tRemoteAddr.m_tAddr[i].m_wPort;
    }

    m_pcRtcp->GetLocalAddr(&dwLocalIP, &wLocalPort);

    //check
    ptNetSndParam->m_tLocalNet.m_dwRTCPAddr = dwLocalIP;
    ptNetSndParam->m_tLocalNet.m_wRTCPPort = wLocalPort;

    wRet= m_pcRtcp->GetRemoteAddr(tRemoteAddr);
    for(i = 0; i < tRemoteAddr.m_byNum; i++)
    {
        ptNetSndParam->m_tRemoteNet[i].m_dwRTCPAddr = tRemoteAddr.m_tAddr[i].m_dwIP;
        ptNetSndParam->m_tRemoteNet[i].m_wRTCPPort = tRemoteAddr.m_tAddr[i].m_wPort;
    }

    return MEDIANET_NO_ERROR;
}
/*=============================================================================
    函数名        RemoveNetSndLocalParam
    功能        ：移除网络发送本地地址参数(进行底层套结子的删除，释放端口等动作)
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  tNetSndParam  网络参数,参见结构定义部分

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::RemoveNetSndLocalParam ()
{
    CREATE_CHECK    //是否创建

    m_pcRtp->RemoveLocalAddr();

    m_pcRtcp->RemoveLocalAddr();

    // 通知网络驱动释放相应发送带宽 2005-01-18
#ifdef _VXWORKS_
    if( (TRUE == g_bUseSmoothSnd) && (TRUE == g_bBrdEthSndUseTimer) &&
        (0 != m_byRmtAddrNum) && (0 != m_byRmtAddrNum) )
    {
        // RemoveBR
        BrdMoveEthBautRate( 0, m_dwCurBandWidth*m_byRmtAddrNum );
        BrdMoveEthBautRate( 1, m_dwCurBandWidth*m_byRmtAddrNum );

        if(4 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf( TRUE, FALSE, "[RemoveNetSndLocalParam] RemoveBR Total.%d, CurBandWidth.%d, byRmtAddrNum.%d\n" ,
                      m_dwCurBandWidth*m_byRmtAddrNum, m_dwCurBandWidth, m_byRmtAddrNum );
        }

    }
#endif
    m_byRmtAddrNum = 0;

    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SetSrcAddr(TNetSession tSrcNet)
{
    CREATE_CHECK    //是否创建
    m_pcRtp->SetSrcAddr(tSrcNet.m_dwRTPAddr, tSrcNet.m_wRTPPort);
    m_pcRtcp->SetSrcAddr(tSrcNet.m_dwRTCPAddr, tSrcNet.m_wRTCPPort);
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SetActivePT
    功能        ：设置 动态载荷的 Playload值
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：byLocalActivePT 本端发送的动态载荷PT值, 由对呼时约定
                  0-表示清空 本端动态载荷标记

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SetActivePT( u8 byLocalActivePT )
{
    m_byLocalActivePT = byLocalActivePT;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SetEncryptKey
    功能        ：设置加密key字串
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pszKeyBuf      加密key字串缓冲指针
                    wKeySize       加密key字串缓冲长度
                    byEncryptMode  加密模式 Aes 或者 Des

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SetEncryptKey( s8 *pszKeyBuf, u16 wKeySize, u8 byEncryptMode )
{
    if(NULL == pszKeyBuf)
    {
        //取消加密
        SAFE_DELETE(m_pszMaterialBuf)
        m_wMaterialBufLen = 0;
    }
    else
    {
        if( (AES_ENCRYPT_MODE != byEncryptMode) &&
            (DES_ENCRYPT_MODE != byEncryptMode) )
        {
            return ERROR_SET_ENCRYPTKEY;
        }

        if( AES_ENCRYPT_MODE == byEncryptMode )
        {
            //设置新的解密键值
            if( (AES_ENCRYPT_KEY_SIZE_MODE_A != wKeySize) &&
                (AES_ENCRYPT_KEY_SIZE_MODE_B != wKeySize) &&
                (AES_ENCRYPT_KEY_SIZE_MODE_C != wKeySize) )
            {
                return ERROR_SET_ENCRYPTKEY;
            }

            SAFE_DELETE(m_pszMaterialBuf)
            m_wMaterialBufLen = 0;

            m_pszMaterialBuf = new s8[wKeySize+4];
            if(NULL == m_pszMaterialBuf)
            {
                return ERROR_SET_ENCRYPTKEY;
            }
            memset(m_pszMaterialBuf, 0, wKeySize+4);
            memcpy(m_pszMaterialBuf, pszKeyBuf, wKeySize);

            // m_byAesMode = MODE_CBC;
        }

        if( DES_ENCRYPT_MODE == byEncryptMode )
        {
            //设置新的解密键值
            if( DES_ENCRYPT_KEY_SIZE != wKeySize )
            {
                return ERROR_SET_ENCRYPTKEY;
            }

            SAFE_DELETE_ARRAY(m_pszMaterialBuf)
            m_wMaterialBufLen = 0;

            m_pszMaterialBuf = new s8[wKeySize+4];
            if(NULL == m_pszMaterialBuf)
            {
                return ERROR_SET_ENCRYPTKEY;
            }
            memset(m_pszMaterialBuf, 0, wKeySize+4);
            memcpy(m_pszMaterialBuf, pszKeyBuf, wKeySize);

            // m_emDesMode = qfDES_cbc;
        }
        m_byEncryptMode   = byEncryptMode;
        m_wMaterialBufLen = wKeySize;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：IncreaseTimestamp
    功能        ：时间戳递增
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  pFrmHdr 帧缓冲
                  bAvalid 是否以实际发送数据长度为增长单位（仅音频有用）
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::IncreaseTimestamp(PFRAMEHDR pFrmHdr, BOOL32 bAvalid)
{
    CREATE_CHECK    //是否创建

    u16 wRet = MEDIANET_NO_ERROR;
    if (TRUE == m_bFirstFrame)
    {
        m_dwFirstTimeStamps = pFrmHdr->m_dwTimeStamp;
        m_bFirstFrame = FALSE;
    }

    //为什么H264的时间戳加3600而MP4的时间戳加40？
    //标准H视频系列的时间戳记录精度为90kHz，所以每次增长3600；其他媒体类型的时间戳记录精度为1ms，所以每次增长40
    switch(pFrmHdr->m_byMediaType)
    {
        case MEDIA_TYPE_MJPEG:
            {  //可能要改
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += VIDEO_TIME_SPAN;
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
                break;
            }
        case MEDIA_TYPE_MP4: //mpeg4
        case MEDIA_TYPE_MP3:
        case MEDIA_TYPE_H262: //mpeg2
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += VIDEO_TIME_SPAN;
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
                break;
            }
        case MEDIA_TYPE_H261:
        case MEDIA_TYPE_H263:
        case MEDIA_TYPE_H263PLUS:
        case MEDIA_TYPE_H264:
        case MEDIA_TYPE_H265:
        case MEDIA_TYPE_PS:
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += VIDEO_TIME_SPAN * VIDEO_TIMESTAMP_SPAN;
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
                break;
            }

        case MEDIA_TYPE_G728:
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    if(TRUE == bAvalid)
                    {
                        m_dwTimeStamp += 4 * pFrmHdr->m_dwDataSize;
                    }
                    else
                    {
                        m_dwTimeStamp += VIDEO_TIME_SPAN;
                    }
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }

            } break;

        case MEDIA_TYPE_ADPCM:
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
            {
                if(TRUE == bAvalid)
                {
                    if (pFrmHdr->m_dwDataSize > 32)  /*ADPCM头长32字节*/
                    {
                        m_dwTimeStamp += 2 * (pFrmHdr->m_dwDataSize - 32);
                    }
                    else
                    {
                        m_dwTimeStamp += 32*8; //按默认32ms*8K计算
                    }
                }
                else
                {
                    m_dwTimeStamp += VIDEO_TIME_SPAN;
                }
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
            } break;

        case MEDIA_TYPE_G7221C:
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += 320; /*时长20ms，时间戳增长按polycom暂时固定为320*/
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
            } break;

        case MEDIA_TYPE_PCMU:
        case MEDIA_TYPE_PCMA:
        case MEDIA_TYPE_G7231:
        case MEDIA_TYPE_G722:
		case MEDIA_TYPE_G729:
		case MEDIA_TYPE_AMR:
		case MEDIA_TYPE_G726_16:
		case MEDIA_TYPE_G726_24:
		case MEDIA_TYPE_G726_32:
		case MEDIA_TYPE_G726_40:
			{
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    if(TRUE == bAvalid)
                    {
                        m_dwTimeStamp += pFrmHdr->m_dwDataSize;
                    }
                    else
                    {
                        m_dwTimeStamp += VIDEO_TIME_SPAN;
                    }
                }
                else
                {
                    m_dwTimeStamp = (pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps);
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }

                break;
            }
    case MEDIA_TYPE_AACLC:
    case MEDIA_TYPE_AACLD:
    case MEDIA_TYPE_AACLC_PCM:
        {
            if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += 1024;
                }
                else
                {
                    m_dwTimeStamp = pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps;
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }
        }
        break;
        case MEDIA_TYPE_H224:
            {
                if (0 == pFrmHdr->m_dwTimeStamp)
                {
                    m_dwTimeStamp += VIDEO_TIME_SPAN;
                }
                else
                {
                    m_dwTimeStamp = pFrmHdr->m_dwTimeStamp - m_dwFirstTimeStamps;
                    if (g_nShowDebugInfo == 11)
                    {
                        OspPrintf(TRUE, FALSE, "mediatype :%d, IncreaseTimestamp m_dwTimeStamp :%d\n",pFrmHdr->m_byMediaType, m_dwTimeStamp);
                    }
                }

                break;
            }
        default:
            wRet = ERROR_SND_PARAM;
            break;
       }

    return wRet;
}

u16 CKdvNetSnd::DealNatProbePack(u32 dwNowTs)
{
	if(NULL == m_pcRtp || NULL == m_pcRtcp)
	{	
	    return ERROR_NET_RCV_MEMORY;
	}
	u16 wRet = ERROR_NET_RCV_MEMORY;

	u32 dwSpan = (dwNowTs - m_dwNatLastTs)*1000 / OspClkRateGet();

	if(g_nShowDebugInfo == 66)
	{
		OspPrintf(1,0, "[medianet: DealNatProbePack ] dwNowTs=%lu  m_dwNatLastTs=%lu dwspan=%lu interval=%lu \n",
			dwNowTs, m_dwNatLastTs, dwSpan, m_tNatProbeProp.dwInterval);
	}

	if(dwNowTs != 0 && (dwSpan < m_tNatProbeProp.dwInterval || m_tNatProbeProp.dwInterval == 0))
	{
		//时间未到，且dwnowts 不为0。直接return。
		return MEDIANET_NO_ERROR;
	}

	 // 发送 rtp 和rtcp 保活包。
	if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf == NULL || m_tNatProbeProp.tRtpNatProbePack.wBufLen == 0 ||
		m_tNatProbeProp.tRtcpNatProbePack.pbyBuf == NULL || m_tNatProbeProp.tRtcpNatProbePack.wBufLen == 0)
	{		
		return ERROR_NET_RCV_NOCREATE;
	}
	 
	if(m_pcRtp->SendUserDefinedBuff(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, m_tNatProbeProp.tRtpNatProbePack.wBufLen, m_tNatProbeProp.tRtpNatProbePack.m_dwPeerAddr, m_tNatProbeProp.tRtpNatProbePack.m_wPeerPort) == MEDIANET_NO_ERROR 
		&&	 m_pcRtcp->SendUserDefinedBuff(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, m_tNatProbeProp.tRtcpNatProbePack.wBufLen, m_tNatProbeProp.tRtcpNatProbePack.m_dwPeerAddr, m_tNatProbeProp.tRtcpNatProbePack.m_wPeerPort) == MEDIANET_NO_ERROR)
	{
		wRet = MEDIANET_NO_ERROR;
	}

	//给natlastts 赋值。
	m_dwNatLastTs = OspTickGet();	
	
	return wRet;
}

u16 CKdvNetSnd::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
	if(ptNatProbeProp->tRtpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtpNatProbePack.pbyBuf == NULL ||
		ptNatProbeProp->tRtcpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtcpNatProbePack.pbyBuf == NULL)
	{
		return ERROR_NET_RCV_PARAM; //设置参数错误
	}
	else
	{
		if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
		{
			memcpy(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, ptNatProbeProp->tRtpNatProbePack.pbyBuf, ptNatProbeProp->tRtpNatProbePack.wBufLen);
			m_tNatProbeProp.tRtpNatProbePack.wBufLen = ptNatProbeProp->tRtpNatProbePack.wBufLen;
		}

		if(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
		{
			memcpy(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, ptNatProbeProp->tRtcpNatProbePack.pbyBuf, ptNatProbeProp->tRtcpNatProbePack.wBufLen);
			m_tNatProbeProp.tRtcpNatProbePack.wBufLen = ptNatProbeProp->tRtcpNatProbePack.wBufLen;
		}

		m_tNatProbeProp.dwInterval = (ptNatProbeProp->dwInterval) * 1000; //转换为ms 存储 , 0s 则不发送

		//record rtp peer addr and port 
		m_tNatProbeProp.tRtpNatProbePack.m_dwPeerAddr = ptNatProbeProp->tRtpNatProbePack.m_dwPeerAddr;
		m_tNatProbeProp.tRtpNatProbePack.m_wPeerPort = ptNatProbeProp->tRtpNatProbePack.m_wPeerPort;

		//record rtcp peer addr and port
		m_tNatProbeProp.tRtcpNatProbePack.m_dwPeerAddr = ptNatProbeProp->tRtcpNatProbePack.m_dwPeerAddr;
		m_tNatProbeProp.tRtcpNatProbePack.m_wPeerPort = ptNatProbeProp->tRtcpNatProbePack.m_wPeerPort;
	}
	
	return MEDIANET_NO_ERROR;
}
/*=============================================================================
	函数名		：ResetFrameId
    功能        ：重新设置数据帧标识。使接收端能正确检测帧的连贯性
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ResetFrameId()
{
    CREATE_CHECK //是否创建
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：ResetSSRC
    功能        ：重新设置同步源
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ResetSSRC(u32 dwSSRC /*= 0*/)
{
    CREATE_CHECK //是否创建

    if(0 == dwSSRC)
    {
        dwSSRC = GetExclusiveSSRC();
    }

    m_pcRtp->ResetSSRC(dwSSRC);

    m_pcRtcp->ResetSSRC(dwSSRC);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：ResetRSFlag
    功能        ：重置对于mpeg4或者H.264采用的重传处理的开关
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：bRepeatSnd  是否重传
                  wBufTimeSpan 重传发送的缓冲区的缓冲时间长度

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd /*=TRUE*/)
{
    CREATE_CHECK //是否创建

    u16 wRet = MEDIANET_NO_ERROR;
    u16 wRLBUnitNum;
    u16 wScale = 1; //放大比例

    if (wBufTimeSpan > 3000)
    {
        OspPrintf(TRUE, FALSE, "[WARN] ResetRSFlag Timespan(%d) too long.\n", wBufTimeSpan);
    }

    if (!m_bVidPayload) //音频
    {
        //音频按20ms计算
        m_wBufTimeSpan  = 20 * ((wBufTimeSpan + 20 - 1)/(20));
        wRLBUnitNum = m_wBufTimeSpan * wScale / 20;
    }
    else
    {
        m_wBufTimeSpan  = VIDEO_TIME_SPAN *
                          ((wBufTimeSpan + VIDEO_TIME_SPAN - 1)/(VIDEO_TIME_SPAN));

        wRLBUnitNum = (m_wBufTimeSpan * m_nMaxSendNum * m_byFrameRate * wScale) / 1000;
        if(wRLBUnitNum < MIN_RS_UNIT_NUM)
        {
            wRLBUnitNum = MIN_RS_UNIT_NUM;
        }
    }

    //根据最大帧与512k帧的大小，进行比例型的放大。
    wRLBUnitNum *= m_byBuffMultipleTimes;

    if(wRLBUnitNum > MAX_RS_UNIT_NUM)
    {
        wRLBUnitNum = MAX_RS_UNIT_NUM;
    }

    m_bRepeatSend = bRepeatSnd;


    if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        OspPrintf(1, 0, "[CKdvNetSnd::ResetRSFlag] bRepeatSnd:%d, wBufTimeSpan:%d, real bRepeatSnd:%d, real wBufTimeSpan:%d, wRLBUnitNum:%d \n",
                         bRepeatSnd, m_wBufTimeSpan, m_bRepeatSend, m_wBufTimeSpan, wRLBUnitNum);
    }

    wRet = m_pcRtp->ResetRSFlag(wRLBUnitNum, m_bRepeatSend);

    return wRet;
}

/*=============================================================================
    函数名        ：EncryptRtpData
    功能        ：根据加密设置进行相应的加密处理
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pRtpPack     RTP数据包结构,参见定义
                   bUseNewBuf   是否使用新缓冲, 对于H261/H263 存在ebit的包，均应该使用新缓冲

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::EncryptRtpData(TRtpPack *pRtpPack, BOOL32 bUseNewBuf/*=FALSE*/)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //a. 判断是否设置了加密
    if( NULL == m_pszMaterialBuf )
    {
        //没有设置加密则直接返回成功
        return wRet;
    }
    if( (NULL == m_pbyEncryptInBuf) || (NULL == m_pbyEncryptOutBuf) )
    {
        if( (14 == g_nShowDebugInfo) && (m_tKdvSndStatistics.m_dwPackSendNum%20) )
        {
            OspPrintf(1, 0, "[EncryptRtpData] step 1 (NULL==m_pbyEncryptOut/InBuf) return ... \n");
        }

        wRet = ERROR_ENCRYPT_FRAME;
        return wRet;
    }

    //b. 码流加密操作处理
    if( AES_ENCRYPT_MODE == m_byEncryptMode )
    {

        //1. 检测待加密的码流总长度, 判断是否存在对齐问题, 记录 tRtpPack.m_byPadNum
        //   如果存在对齐问题，则以零补齐待加密码流最后的tRtpPack.m_byPadNum字节，同时总长度应 (+tRtpPack.m_byPadNum)
        s32 nRealSize        = pRtpPack->m_nRealSize;
        u8 *pbyEncryptBuf    = pRtpPack->m_pRealData;
        pRtpPack->m_byPadNum = nRealSize%AES_ENCRYPT_BYTENUM;
        if( 0 != pRtpPack->m_byPadNum )
        {
            pRtpPack->m_byPadNum = AES_ENCRYPT_BYTENUM - pRtpPack->m_byPadNum;
            nRealSize += pRtpPack->m_byPadNum;
            memcpy(m_pbyEncryptInBuf, pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
            memset((m_pbyEncryptInBuf+pRtpPack->m_nRealSize), 0, pRtpPack->m_byPadNum);
            // SET Padding Octor:
            *(m_pbyEncryptInBuf+nRealSize-RTP_PADDING_SIZE) = pRtpPack->m_byPadNum;
            pbyEncryptBuf = m_pbyEncryptInBuf;
        }

        //2. 码流加密

        // 初始 key 由 序列号与时间戳组成  SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
        s8  szInitKey[AES_ENCRYPT_BYTENUM] = {0};
        u16 wSequence    = htons( (u16)(m_pcRtp->m_wSeqNum+1) );
        u32 dwTimeStamp  = htonl( pRtpPack->m_dwTimeStamp );
        u16 wOffset      = 0;
        memcpy( szInitKey+wOffset, &wSequence, sizeof(wSequence) );
        wOffset += sizeof(wSequence);
        memcpy( szInitKey+wOffset, &dwTimeStamp, sizeof(dwTimeStamp) );
        wOffset += sizeof(dwTimeStamp);
        memcpy( szInitKey+wOffset, szInitKey, sizeof(wSequence)+sizeof(dwTimeStamp) );
        wOffset += sizeof(wSequence)+sizeof(dwTimeStamp);
        memcpy( szInitKey+wOffset, &wSequence, sizeof(wSequence) );
        wOffset += sizeof(wSequence);
        memcpy( szInitKey+wOffset, &dwTimeStamp, AES_ENCRYPT_BYTENUM-wOffset );
#if 0
        s32 nAesRet = KdvAES( m_pszMaterialBuf, m_wMaterialBufLen,
                              m_byAesMode, qfDES_encrypt, szInitKey,
                              pbyEncryptBuf, nRealSize, m_pbyEncryptOutBuf );
        if( nAesRet < 0 )
        {
            if( (14 == g_nShowDebugInfo) && (m_tKdvSndStatistics.m_dwPackSendNum%20) )
            {
                OspPrintf(1, 0, "[EncryptRtpData AES] step 2 (NULL==m_pbyEncryptOutstep 4 KdvAES Err, nAesRet=%d return ... \n", nAesRet);
            }

            wRet = ERROR_ENCRYPT_FRAME;
            return wRet;
        }
#endif
        pRtpPack->m_nRealSize = nRealSize;
        pRtpPack->m_pRealData = m_pbyEncryptOutBuf;
    }

    if( DES_ENCRYPT_MODE == m_byEncryptMode )
    {
        //1. 检测待加密的码流总长度, 判断是否存在对齐问题, 记录 tRtpPack.m_byPadNum
        //   如果存在对齐问题，则以零补齐待加密码流最后的tRtpPack.m_byPadNum字节，同时总长度应 (+tRtpPack.m_byPadNum)
        s32 nRealSize        = pRtpPack->m_nRealSize;
        u8 *pbyEncryptBuf    = pRtpPack->m_pRealData;
        pRtpPack->m_byPadNum = nRealSize%DES_ENCRYPT_BYTENUM;
        if( 0 != pRtpPack->m_byPadNum )
        {
            pRtpPack->m_byPadNum = DES_ENCRYPT_BYTENUM - pRtpPack->m_byPadNum;
            nRealSize += pRtpPack->m_byPadNum;
            memcpy(m_pbyEncryptOutBuf, pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
            memset((m_pbyEncryptOutBuf+pRtpPack->m_nRealSize), 0, pRtpPack->m_byPadNum);
            // SET Padding Octor:
            *(m_pbyEncryptOutBuf+nRealSize-RTP_PADDING_SIZE) = pRtpPack->m_byPadNum;
            pbyEncryptBuf = m_pbyEncryptOutBuf;
        }
        else if( TRUE == bUseNewBuf )
        {
            memcpy(m_pbyEncryptOutBuf, pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
            pbyEncryptBuf = m_pbyEncryptOutBuf;
        }

        //2. 码流加密

        // 初始 key 由 序列号与时间戳组成  SSTTTTSS (S-Sequence, T-TimeStamp)
        u8  szInitKey[DES_ENCRYPT_BYTENUM] = {0};
        u16 wSequence    = htons( (u16)(m_pcRtp->m_wSeqNum+1) );
        u32 dwTimeStamp  = htonl( pRtpPack->m_dwTimeStamp );
        memcpy( szInitKey, &wSequence, sizeof(wSequence) );
        memcpy( szInitKey+sizeof(wSequence), &dwTimeStamp, sizeof(dwTimeStamp) );
        memcpy( szInitKey+sizeof(wSequence)+sizeof(dwTimeStamp), &wSequence, sizeof(wSequence) );
#if 0
        KdvDES( (u8*)m_pszMaterialBuf,
                pbyEncryptBuf, (u32)nRealSize,
                qfDES_encrypt, m_emDesMode,
                szInitKey );
#endif
        pRtpPack->m_nRealSize = nRealSize;
        pRtpPack->m_pRealData = pbyEncryptBuf;
    }

    //c. 进行加密后的总长度判断, 应当不超过 MAX_SND_ENCRYPT_PACK_SIZE, 请注意判断
    if( pRtpPack->m_nRealSize > MAX_SND_ENCRYPT_PACK_SIZE )
    {
        if( (14 == g_nShowDebugInfo) && (m_tKdvSndStatistics.m_dwPackSendNum%20) )
        {
            OspPrintf(1, 0, "[EncryptRtpData] step 3 (m_nRealSize=%d>MAX_SND_ENCRYPT_PACK_SIZE) return ... \n",
                             pRtpPack->m_nRealSize, MAX_SND_ENCRYPT_PACK_SIZE );
        }

        wRet = ERROR_ENCRYPT_FRAME;
        return wRet;
    }

/*
    //d. 将对于加密后的码流真实载荷PT值替换为动态载荷PT值
    pRtpPack->m_byPayload = m_byLocalActivePT;
*/

    return wRet;
}

/*=============================================================================
    函数名        ：Send
    功能        ：发送数据包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义
                   bAvalid  数据帧是否有效

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::Send(PFRAMEHDR pFrmHdr, BOOL32 bAvalid, BOOL32 bSendRtp)
{
    CREATE_CHECK //是否创建

    u16 wRet = MEDIANET_NO_ERROR;
    m_bSendRtp = bSendRtp;
	//update send maxsize 
	if(pFrmHdr->m_byFrameRate && m_nSendRate)
	{
		m_nMaxSendSize = (s32)(m_nSendRate/(pFrmHdr->m_byFrameRate*8));
		if(g_nShowDebugInfo == 67)
		{
			OspPrintf(1,0,"[CKdvNetSnd::Send] sendrate=%d frameate=%d sendmaxszie=%d  \n",
			m_nSendRate, pFrmHdr->m_byFrameRate, m_nMaxSendSize);
		}
	}
	if (MEDIA_TYPE_NULL != m_tPSInfo.m_byVideoType || MEDIA_TYPE_NULL != m_tPSInfo.m_byAudioType)
	{
        //若create snd对象时，psinifo信息只填了视频，那音频在此适配，反之亦然。
        if(m_tPSInfo.m_byVideoType == MEDIA_TYPE_NULL && (pFrmHdr->m_byMediaType == MEDIA_TYPE_H264 || pFrmHdr->m_byMediaType == MEDIA_TYPE_MP4 \
            || pFrmHdr->m_byMediaType == MEDIA_TYPE_SVACV))
        {
            m_tPSInfo.m_byVideoType = pFrmHdr->m_byMediaType;

            wRet = TspsWriteSetProgram(m_hTspsWrite, m_tPSInfo.m_byVideoType, m_tPSInfo.m_byAudioType);
            if (wRet != TSPS_OK)
            {
                return ERROR_SND_PSSETPROGRAM;
            }

        }
        else if(m_tPSInfo.m_byAudioType == MEDIA_TYPE_NULL && (pFrmHdr->m_byMediaType == MEDIA_TYPE_PCMA || pFrmHdr->m_byMediaType == MEDIA_TYPE_MP2 \
            || pFrmHdr->m_byMediaType == MEDIA_TYPE_G7221 || pFrmHdr->m_byMediaType == MEDIA_TYPE_G7231 || pFrmHdr->m_byMediaType == MEDIA_TYPE_G729 \
            || pFrmHdr->m_byMediaType == MEDIA_TYPE_SVACA))
        {
            m_tPSInfo.m_byAudioType = pFrmHdr->m_byMediaType;

            wRet = TspsWriteSetProgram(m_hTspsWrite, m_tPSInfo.m_byVideoType, m_tPSInfo.m_byAudioType);
            if (wRet != TSPS_OK)
            {
                return ERROR_SND_PSSETPROGRAM;
            }
        }

        if (g_nShowDebugInfo == 12)
        {
            OspPrintf(1,0,"###SendDebug m_tPSInfo.m_byVideoType %d, m_tPSInfo.m_byAudioType %d,pFrmHdr->m_byMediaType %d\n",m_tPSInfo.m_byVideoType, \
                m_tPSInfo.m_byAudioType,pFrmHdr->m_byMediaType);
        }

        //发送码流pt，不在应发送的类型里,return 错误码
        if (pFrmHdr->m_byMediaType != m_tPSInfo.m_byVideoType && pFrmHdr->m_byMediaType != m_tPSInfo.m_byAudioType)
        {
            if (g_nShowDebugInfo == 12)
            {
                OspPrintf(1,0, " Frm %d   PSInfo %d %d \n",pFrmHdr->m_byMediaType ,m_tPSInfo.m_byVideoType, m_tPSInfo.m_byAudioType);
            }
            return ERROR_SND_PARAM;
        }

        wRet = SendPSFrame(pFrmHdr);
        return wRet;
    }

    if (g_nShowDebugInfo == 12)
    {
        OspPrintf(TRUE, FALSE, "send frame payload:%d, bKeyFrame :%d timestamps:%d \n",
            pFrmHdr->m_byMediaType, pFrmHdr->m_tVideoParam.m_bKeyFrame,
            pFrmHdr->m_dwTimeStamp);
    }
    //
    if(FALSE == bAvalid)
    {
        //hual 2005-07-25 对Mpeg 4, 改为发送空帧
        if (2 == g_nShowDebugInfo && 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME. \n");
        }

        if (NULL == pFrmHdr)
        {
            OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME Must set frame Header\n");
            return ERROR_SND_PARAM;
        }

        //非Mpeg4按原先的处理进行（增加时间戳，不进行空帧的发送）
        if ((MEDIA_TYPE_MP4 != pFrmHdr->m_byMediaType) && (MEDIA_TYPE_H262 != pFrmHdr->m_byMediaType)
            && (MEDIA_TYPE_MJPEG != pFrmHdr->m_byMediaType))
        {
            IncreaseTimestamp(pFrmHdr, FALSE);
            //按照最大发送字节数 批量发送数据包
            return m_pcRtp->SendBySize(m_nMaxSendSize);
        }
        else
        {
            //hual 安全性判断，保证上层设置了长度为0
            if (0 != pFrmHdr->m_dwDataSize)
            {
                OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME Must set frame len equal 0. \n");
                return ERROR_SND_PARAM;
            }
        }

    }

        //参数有效性判断，pFrmHdr->m_dwDataSize为零，表示发送空包
    if( (pFrmHdr == NULL)||
        //(pFrmHdr->m_dwDataSize > m_dwMaxFrameSize) ||
        (pFrmHdr->m_dwDataSize > MAX_FRAME_SIZE) ||
        (pFrmHdr->m_byMediaType != m_byMediaType) ||
        ((pFrmHdr->m_dwDataSize != 0) && (pFrmHdr->m_pData == NULL)) )
    {
        return ERROR_SND_PARAM;
    }

    if(TRUE == g_bSelfSnd)
    {
        return wRet;
    }

    //帧ID累加
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwFrameNum++;

    //时间戳累加
    IncreaseTimestamp(pFrmHdr, TRUE);

    //根据不同的媒体类型作不同的处理，目前只有两种发送方式:
    //1.标准包的发送，如h261、h263、h264、G.711、G.722、G.723、G.728、H.224、H.239、T.120
    //2.扩展包的发送, 如Mpeg4、mp3

    switch(pFrmHdr->m_byMediaType)
    {
    case MEDIA_TYPE_MP4: //mpeg4
    case MEDIA_TYPE_H262: //mpeg2
    case MEDIA_TYPE_MJPEG://mjpeg
        {
            wRet = SendExPack(pFrmHdr,FALSE);
            break;
        }
    case MEDIA_TYPE_MP3:
        {
            wRet = SendExPack(pFrmHdr,TRUE);
            break;
        }
    case MEDIA_TYPE_H261:
    case MEDIA_TYPE_H263:
    case MEDIA_TYPE_H263PLUS:
    case MEDIA_TYPE_H264:
    case MEDIA_TYPE_H265:
        {
            wRet = SendSdPack(pFrmHdr);
            break;
        }
    case MEDIA_TYPE_PCMU:
    case MEDIA_TYPE_PCMA:
    case MEDIA_TYPE_G7231:
    case MEDIA_TYPE_G722:
    case MEDIA_TYPE_G728:
    case MEDIA_TYPE_G729:
    case MEDIA_TYPE_ADPCM:
    case MEDIA_TYPE_G7221C:
	case MEDIA_TYPE_AMR:
	case MEDIA_TYPE_G726_16:
	case MEDIA_TYPE_G726_24:
	case MEDIA_TYPE_G726_32:
	case MEDIA_TYPE_G726_40:
		{
            wRet = SendSdAudioPack(pFrmHdr);
            break;
        }
    case MEDIA_TYPE_AACLC:
    case MEDIA_TYPE_AACLD:
    case MEDIA_TYPE_AACLC_PCM:
        {
            wRet = SendSdAACAudioPack(pFrmHdr);
            break;
        }
    case MEDIA_TYPE_H224:
        {
            wRet = SendDataPayloadPack(pFrmHdr);
            break;
        }
    default:
        wRet = ERROR_SND_PARAM;
        break;
       }

    if( (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo) && (MEDIANET_NO_ERROR != wRet) )
    {
        OspPrintf(1, 0, "[CKdvNetSnd::Send] MEDIA_TYPE:%d ERROE CODE is %d \n",
                         pFrmHdr->m_byMediaType, wRet);
    }

    return wRet;
}

/*=============================================================================
    函数名        ：SelfTestSend
    功能        ：自测发送数据包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SelfTestSend(s32 nFrameLen, s32 nSndNum, s32 nSpan)
{
    CREATE_CHECK //是否创建

    u16 wRet = MEDIANET_NO_ERROR;

    if(FALSE == g_bSelfSnd)
    {
        return wRet;
    }

    //参数有效性判断，pFrmHdr->m_dwDataSize为零，表示发送空包
    if( nFrameLen > (s32)m_dwMaxFrameSize || 0 == m_dwMaxFrameSize )
    {
        return ERROR_SND_PARAM;
    }
    if( NULL == m_pSelfFrameBuf )
    {
        MEDIANET_MALLOC(m_pSelfFrameBuf, m_dwMaxFrameSize);
        if( NULL == m_pSelfFrameBuf )
        {
            return ERROR_SND_MEMORY;
        }
        m_tSelfFrmHdr.m_pData = m_pSelfFrameBuf;
    }
    if( NULL == m_tSelfFrmHdr.m_pData )
    {
        return ERROR_SND_MEMORY;
    }

    m_tSelfFrmHdr.m_dwDataSize = nFrameLen;

    u32 dwTimeStamp = 0;
    u32 dwEndTimeStamp = 0;
    u32 dwSpanTime = 0;
    for(s32 nSndIndex = 0; nSndIndex < nSndNum; nSndIndex++)
    {
        //帧ID累加
        m_dwFrameId++;
        m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
        m_tKdvSndStatistics.m_dwFrameNum++;

        //时间戳累加
        IncreaseTimestamp(&m_tSelfFrmHdr, TRUE);

        dwTimeStamp = OspTickGet();

        //根据不同的媒体类型作不同的处理，目前只有两种发送方式:
        //1.标准包的发送，如h261、h263、h264、G.711、G.722、G.723、G.728、H.224、H.239、T.120
        //2.扩展包的发送, 如Mpeg4、mp3

        switch(m_tSelfFrmHdr.m_byMediaType)
        {
        case MEDIA_TYPE_MP4: //mpeg4
        case MEDIA_TYPE_H262: //mpeg2
        case MEDIA_TYPE_MJPEG://mjpeg
            {
                wRet = SendExPack(&m_tSelfFrmHdr, FALSE);
                break;
            }
        case MEDIA_TYPE_MP3:
            {
                wRet = SendExPack(&m_tSelfFrmHdr, TRUE);
                break;
            }
        case MEDIA_TYPE_H261:
        case MEDIA_TYPE_H263:
        case MEDIA_TYPE_H263PLUS:
        case MEDIA_TYPE_H264:
        case MEDIA_TYPE_H265:
            {
                wRet = SendSdPack(&m_tSelfFrmHdr);
                break;
            }
        case MEDIA_TYPE_PCMU:
        case MEDIA_TYPE_PCMA:
        case MEDIA_TYPE_G7231:
        case MEDIA_TYPE_G722:
        case MEDIA_TYPE_G728:
        case MEDIA_TYPE_G729:
        case MEDIA_TYPE_ADPCM:
        case MEDIA_TYPE_G7221C:
        case MEDIA_TYPE_AACLC:
        case MEDIA_TYPE_AACLD:
		case MEDIA_TYPE_AACLC_PCM:
		case MEDIA_TYPE_AMR:
		case MEDIA_TYPE_G726_16:
		case MEDIA_TYPE_G726_24:
		case MEDIA_TYPE_G726_32:
		case MEDIA_TYPE_G726_40:
			{
                wRet = SendSdPack(&m_tSelfFrmHdr);
                break;
            }
        case MEDIA_TYPE_H224:
            {
                wRet = SendDataPayloadPack(&m_tSelfFrmHdr);
                break;
            }
        default:
            wRet = ERROR_SND_PARAM;
            break;
           }

        if( (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo) && (MEDIANET_NO_ERROR != wRet) )
        {
            OspPrintf(1, 0, "[CKdvNetSnd::Send] MEDIA_TYPE:%d ERROE CODE is %d \n",
                             m_tSelfFrmHdr.m_byMediaType, wRet);
        }

        dwEndTimeStamp = OspTickGet();
        dwSpanTime = nSpan;
        if(dwEndTimeStamp > dwTimeStamp)
        {

            dwSpanTime = (dwEndTimeStamp - dwTimeStamp)*1000/OspClkRateGet();
        }
        if((s32)dwSpanTime < nSpan)
        {
            OspTaskDelay((nSpan - dwSpanTime));
        }

    }

    return wRet;
}

/*=============================================================================
    函数名        ：DealRtcpTimer
    功能        ：rtcp定时rtcp包上报
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::DealRtcpTimer()
{
    CREATE_CHECK //是否创建

    return m_pcRtcp->DealTimer();
}

/*=============================================================================
    函数名        ：Send
    功能        ：直接发送RTP数据包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   bTrans   是否做透明转发，是则包序列号及时间戳不变
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::Send(TRtpPack *pRtpPack, BOOL32 bTrans, BOOL32 bAvalid, BOOL32 bSendRtp)
{
    CREATE_CHECK //是否创建

    u16 wRet = MEDIANET_NO_ERROR;

    if(FALSE == bAvalid)
    {
        //不改变原有的时间戳，由外部指定
        /*
        if( (MEDIA_TYPE_H261 == m_byMediaType) ||
            (MEDIA_TYPE_H263 == m_byMediaType) ||
            (MEDIA_TYPE_H263PLUS == m_byMediaType) ||
            (MEDIA_TYPE_H264 == m_byMediaType) )
        {
            m_dwTimeStamp += VIDEO_TIME_SPAN * VIDEO_TIMESTAMP_SPAN;
        }
        else
        {
            m_dwTimeStamp += VIDEO_TIME_SPAN;
        }
        */
        return wRet;
    }

    if(pRtpPack == NULL)
    {
        return ERROR_SND_PARAM;
    }

    //帧ID累加
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwPackSendNum++;

    if(TRUE == bTrans)
    {
        m_tOldRtp = *pRtpPack;
        //清空预分配的空间，防止透明转发时篡改rtp包数据
        m_tOldRtp.m_nPreBufSize = 0;
        wRet = m_pcRtp->Write(m_tOldRtp, TRUE, TRUE, bSendRtp);
    }
    else
    {
        //不改变原有的时间戳，由外部指定
        /*
        if( pRtpPack->m_byPayload == MEDIA_TYPE_MP4 || //mpeg4
            pRtpPack->m_byPayload == MEDIA_TYPE_H262 || //mpeg2
            pRtpPack->m_byPayload == MEDIA_TYPE_H264 ||
            pRtpPack->m_byPayload == MEDIA_TYPE_H263 ||
            pRtpPack->m_byPayload == MEDIA_TYPE_H263PLUS ||
            pRtpPack->m_byPayload == MEDIA_TYPE_H261 ||
            pRtpPack->m_nRealSize <= 0)
        {
            if(pRtpPack->m_byPayload == MEDIA_TYPE_H263 ||
                pRtpPack->m_byPayload == MEDIA_TYPE_H261 )
            {
                m_dwTimeStamp += VIDEO_TIME_SPAN * VIDEO_TIMESTAMP_SPAN;
            }
            else
            {
                m_dwTimeStamp += VIDEO_TIME_SPAN;
            }
        }
        else
        {
            m_dwTimeStamp += pRtpPack->m_nRealSize;
        }

        pRtpPack->m_dwTimeStamp = m_dwTimeStamp;
        */

        if(pRtpPack->m_wSequence != m_tOldRtp.m_wSequence+1)
        {
            m_pcRtp->ResetSeq();
        }
        wRet = m_pcRtp->Write(*pRtpPack, TRUE, FALSE, bSendRtp);
        m_tOldRtp = *pRtpPack;
    }

    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return wRet;
    }

    return wRet;
}


u16 CKdvNetSnd::SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext)
{
    u16 wRet;

    if(NULL == m_pcRtcp)
    {
        return ERROR_NET_RCV_MEMORY;
     }
    else
    {
        wRet = m_pcRtcp->SetRtcpInfoCallback(pRtcpInfoCallback, pContext);
    }
    return wRet;
}
/*=============================================================================
    函数名        ：SendExPack
    功能        ：发送扩展的RTP包，先将一帧数据切成很多个小包，最后一包
                  的扩张信息中包含帧的全部信息，其他包的扩展信息中只包含
                  该包在一帧数据中的位置。
                  扩展信息结构如下:u8 TotalNum + u8 PackIndex + u8 FrameMode +
                  u8 FrameRate + u32 FrameID + u32 Width +
                  u32 Height

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendExPack(PFRAMEHDR pFrmHdr,BOOL32 bAudio)
{
    u16 wRet         = MEDIANET_NO_ERROR;
    s32 nRealPackLen = g_dwMaxExtendPackSize;
    s32 nPackNum     = (pFrmHdr->m_dwDataSize  + nRealPackLen - 1) / nRealPackLen;

    //始终会有一包发送
    if(0 == nPackNum)
    {
        nPackNum = 1;
    }

    if(nPackNum > 0xff) //包的个数不能超过单字节所能表达的值
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_SND_FRAME;
    }

    //传给CKdvRtp类的RTP数据包信息
    TRtpPack  tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp    = m_dwTimeStamp;
    tRtpPack.m_byExtence    = 1;//扩展包
    tRtpPack.m_nRealSize    = nRealPackLen;
    tRtpPack.m_nExSize        = 1;//单位为sizeof(u32)长度。
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    s32 nPackIndex;

    //循环处理n-1包
    for(nPackIndex=1; nPackIndex<nPackNum; nPackIndex++)
    {
        tRtpPack.m_nRealSize = nRealPackLen;
        //有效数据
        tRtpPack.m_pRealData = pFrmHdr->m_pData + (nPackIndex-1)*nRealPackLen;
        //扩展数据
        //总包数
        m_byExBuf[EX_TOTALNUM_POS] = (u8)nPackNum;
        //包序号
        m_byExBuf[EX_INDEX_POS]    = (u8)nPackIndex;
        tRtpPack.m_pExData           = m_byExBuf;


        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //加密失败
            m_tKdvSndStatistics.m_dwFrameLoseNum++;

            if(FALSE == bAudio)
            {
                //按照最大发送字节数 批量发送数据包
                m_pcRtp->SendBySize(m_nMaxSendSize);
            }
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //写入一包，不一定直接发送，取决于第二个参数
        wRet = m_pcRtp->Write(tRtpPack, bAudio, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //缓冲满
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            if(FALSE == bAudio)
            {
                //按照最大发送字节数 批量发送数据包
                m_pcRtp->SendBySize(m_nMaxSendSize);
            }
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //统计
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    /*最后一包的处理，不同于前n-1包，在最后一包会放入帧信息，如帧ID、
      是否关键帧、视频帧宽高。
    */
    s32 nLastPackLen = pFrmHdr->m_dwDataSize - nRealPackLen * (nPackNum - 1);

    tRtpPack.m_byMark            = 1;
    tRtpPack.m_nRealSize        = nLastPackLen;
    tRtpPack.m_pRealData        = pFrmHdr->m_pData + (nPackNum - 1) * nRealPackLen;
    //音频帧
    if(bAudio)
    {
        tRtpPack.m_nExSize            = 1;
        //总包数
        m_byExBuf[EX_TOTALNUM_POS]  = (u8)nPackNum;
        //包序号
        m_byExBuf[EX_INDEX_POS]        = (u8)nPackIndex;
        //音频模式
        m_byExBuf[EX_FRAMEMODE_POS]    = (u8)pFrmHdr->m_byAudioMode;
        tRtpPack.m_pExData          = m_byExBuf;

        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //加密失败
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }
        //音频帧直接发送
        wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //发送失败
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }

        //统计
        m_tKdvSndStatistics.m_dwPackSendNum++;

        return wRet;
    }
    else
    {
        //视频帧处理
        tRtpPack.m_nExSize            = MAX_PACK_EX_LEN / sizeof(u32);//
        //总包数
        m_byExBuf[EX_TOTALNUM_POS]  = (u8)nPackNum;
        //包序号
        m_byExBuf[EX_INDEX_POS]        = (u8)nPackIndex;
        //是否关键帧
        m_byExBuf[EX_FRAMEMODE_POS] = (u8)pFrmHdr->m_tVideoParam.m_bKeyFrame;
        //帧率
        m_byExBuf[EX_FRAMERATE_POS]    = (u8)m_byFrameRate;
        //帧ID
        *((u32 *)(&(m_byExBuf[EX_FRAMEID_POS]))) = htonl(m_dwFrameId) ;
        //视频帧宽
        *((u16 *)(&(m_byExBuf[EX_WIDTH_POS])))   =
                                       htons(pFrmHdr->m_tVideoParam.m_wVideoWidth);
        //视频帧高
        *((u16 *)(&(m_byExBuf[EX_HEIGHT_POS])))  =
                                      htons(pFrmHdr->m_tVideoParam.m_wVideoHeight);

        tRtpPack.m_pExData          = m_byExBuf;

        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //加密失败
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            //按照最大发送字节数 批量发送数据包
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();
            return wRet;
        }
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //缓冲满
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            //按照最大发送字节数 批量发送数据包
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }

        //统计
        m_tKdvSndStatistics.m_dwPackSendNum++;

        //按照最大发送字节数 批量发送数据包
        wRet = m_pcRtp->SendBySize(m_nMaxSendSize);
    }

    return wRet;
}

/*=============================================================================
    函数名        ：ParasH261Head
    功能        ：h.261 RTP head info 分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ParasH261Head(PFRAMEHDR pFrmHdr)
{
    u32 *pdwPos = (u32 *)pFrmHdr->m_pData;
    memset(m_ptH261HeaderList, 0, sizeof(TH261HeaderList));

    //get Rtp pack num
    m_ptH261HeaderList->m_nNum = (s32)(*pdwPos++);
    //protect
    if(m_ptH261HeaderList->m_nNum > MAX_H261_PACK_NUM ||
       m_ptH261HeaderList->m_nNum == 0 )
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_H261_PACK_NUM;
    }

    //Get Rtp Header
    for(s32 nIndex=0; nIndex<m_ptH261HeaderList->m_nNum; nIndex++)
    {
        s32 nHeadLen = sizeof(u32);
        s32 nPackLen = 0;

        // Head Info
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwHeader   = *(pdwPos++);
        //RTP包在一帧中的起始位置
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwStartPos = *(pdwPos++);
        //RTP包的长度
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen  = *(pdwPos++);

        nPackLen = m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen + nHeadLen;

        // 允许空包
        if( (nPackLen < 0) ||
            (nPackLen > MAX_SND_PACK_SIZE) )
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            OspPrintf(TRUE, FALSE, "H261 pack too long. PackLen=%d\n", nPackLen);
            return ERROR_H263_PACK_TOOMUCH;
        }

        {
            //sbit ebit 校验
            u32 sbit = GetBitField(m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwHeader, 29, 3);
            if( ((0 != m_byLastEBit) || (0 != sbit)) &&
                (8 != (m_byLastEBit+sbit)) )
            {
                if( (11 == g_nShowDebugInfo || 255 == g_nShowDebugInfo) )
                {
                    OspPrintf(1, 0, "[CKdvNetSnd::ParasH261Head] Total:%d Cur pos:%d Last ebit:%d plus cur sbit:%d is not equal 8 \n",
                                     m_ptH261HeaderList->m_nNum, nIndex, m_byLastEBit, sbit );
                }
            }
            m_byLastEBit = (u8)GetBitField(m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwHeader, 26, 3);
        }

        //更新RTP包的长度 (包含playload头长度)
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen = nPackLen;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SendOneH261Pack
    功能        ：发送标准的 一包 h.261 RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义
                   pPackBuf  包数据缓冲
                   nPos      包索引

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendOneH261Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//非扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //切分数据帧,由于数据帧带有H261header, 必定有MAX_H261_HEADER_LEN指定的空间。
    tRtpPack.m_nPreBufSize  = MAX_H261_HEADER_LEN;

    //last pack
    if(nPos == (m_ptH261HeaderList->m_nNum-1))
    {
        tRtpPack.m_byMark     = 1; //最后一包的标识位
    }

    //写入包长度
    tRtpPack.m_nRealSize = m_ptH261HeaderList->m_tkdvH261Header[nPos].m_dwPackLen;

    //加上包头
    BOOL32 bUseNewBuf = FALSE;
    u32    dwHeaderArr[1];
    dwHeaderArr[0] = m_ptH261HeaderList->m_tkdvH261Header[nPos].m_dwHeader;

    if( 0 != GetBitField(dwHeaderArr[0], 26, 3) )
    {
        bUseNewBuf = TRUE;
    }

    dwHeaderArr[0] = htonl(dwHeaderArr[0]);
    s32 nHeadLen = sizeof(dwHeaderArr);
    tRtpPack.m_pRealData = pPackBuf - nHeadLen;
    memcpy(tRtpPack.m_pRealData, dwHeaderArr, nHeadLen);

    //码流加密操作
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        return wRet;
    }

    //h.261 也放入缓冲发送
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    函数名        ：SendH261Pack
    功能        ：发送标准的 h.261 RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendH261Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //rtp 头信息分析，做合理性分析，并记录
    wRet = ParasH261Head(pFrmHdr);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //得到有效数据的起始位置
    u8 *pBuf = pFrmHdr->m_pData + MAX_H261_HEADER_LEN +
        m_ptH261HeaderList->m_tkdvH261Header[0].m_dwStartPos;

    //send Rtp pack
    for(s32 nPos=0; nPos<m_ptH261HeaderList->m_nNum; nPos++)
    {
        pBuf = pFrmHdr->m_pData + MAX_H261_HEADER_LEN +
               m_ptH261HeaderList->m_tkdvH261Header[nPos].m_dwStartPos;

        wRet = SendOneH261Pack(pFrmHdr, pBuf, nPos);
        if(KDVFAILED(wRet))
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;

            //按照最大发送字节数 批量发送数据包
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    //按照最大发送字节数 真实发送数据包
    return m_pcRtp->SendBySize(m_nMaxSendSize);//批量发送
}

/*=============================================================================
    函数名        ：ParasH263Head
    功能        ：h.263/h.263+ RTP head info 分析 h.263+ 作为特殊的h.263码流集中处理
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ParasH263Head(PFRAMEHDR pFrmHdr)
{
    u32 *pdwPos = (u32 *)pFrmHdr->m_pData;
    memset(m_ptH263HeaderList, 0, sizeof(TH263HeaderList));

    //get Rtp pack num
    m_ptH263HeaderList->m_nNum = (s32)(*pdwPos++);
    //protect
    if(m_ptH263HeaderList->m_nNum > MAX_H263_PACK_NUM ||
       m_ptH263HeaderList->m_nNum == 0 )
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_H263_PACK_NUM;
    }

    //Get Rtp Header
    for(s32 nIndex=0; nIndex<m_ptH263HeaderList->m_nNum; nIndex++)
    {
        // RTP头
        // 0 - A MODE ; 1 - B MODE ; 2 - C MODE ; 3 - D MODE-H.263+
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwMode     = *(pdwPos++);
        // Head Info (A MODE - 4字节； B MODE - 8字节； C MODE - 12字节； D MODE - 2字节)
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwHeader1  = *(pdwPos++);
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwHeader2  = *(pdwPos++);
        //RTP包在一帧中的起始位置
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwStartPos = *(pdwPos++);
        //RTP包的长度
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwDataLen  = *(pdwPos++);
    }

    for(s32 nPos=0; nPos<m_ptH263HeaderList->m_nNum; nPos++)
    {
        s32 nHeadLen = sizeof(u32);
        s32 nPackLen = 0;

        if(MODE_H263_B == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
        {
            nHeadLen = 2*sizeof(u32);
        }
        //这里约定MODE_H263_D即为h.263+， 作为特殊的h.263码流集中处理
        else if(MODE_H263_D == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
        {
            nHeadLen = sizeof(u16);
        }

        //计算每一包的rtp长度
        if(nPos == (m_ptH263HeaderList->m_nNum-1))
        {
            nPackLen = pFrmHdr->m_dwDataSize - MAX_H263_HEADER_LEN -
                       m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwStartPos + nHeadLen;
        }
        else
        {
            nPackLen = m_ptH263HeaderList->m_tKdvH263Header[nPos+1].m_dwStartPos -
                       m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwStartPos + nHeadLen;

            if(MODE_H263_D != m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
            {
                u32 ebit = GetBitField(m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1, 24, 3);
                if(ebit) nPackLen += 1;
            }
        }

        if(MODE_H263_D != m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
        {
            //sbit ebit 校验
            u32 sbit = GetBitField(m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1, 27, 3);
            if( ((0 != m_byLastEBit) || (0 != sbit)) &&
                (8 != (m_byLastEBit+sbit)) )
            {
                if( (11 == g_nShowDebugInfo || 255 == g_nShowDebugInfo) )
                {
                    OspPrintf(1, 0, "[CKdvNetSnd::ParasH263Head] Total:%d Cur pos:%d Last ebit:%d plus cur sbit:%d is not equal 8 \n",
                                     m_ptH263HeaderList->m_nNum, nPos, m_byLastEBit, sbit );
                }
            }
            m_byLastEBit = (u8)GetBitField(m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1, 24, 3);
        }

        // 允许空包
        if( (nPackLen < 0) ||
            (nPackLen > MAX_SND_PACK_SIZE) )
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            OspPrintf(TRUE, FALSE, "H263 pack too long. PackLen=%d\n", nPackLen);
            return ERROR_H263_PACK_TOOMUCH;
        }

        //更新RTP包的长度 (包含playload头长度)
        m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwDataLen = nPackLen;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：ParasH263PlusHead
    功能        ：h.263+ RTP head info 分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   wH263PlusHeader  h.263+ payload format
                   ptH263PlusHeader 包结构指针

    返回值说明： 无
=============================================================================*/
BOOL32 CKdvNetSnd::ParasH263PlusHead( u16 wH263PlusHeader,
                                      TH263PlusHeader *ptH263PlusHeader )
{
    if( NULL == ptH263PlusHeader )
    {
        return FALSE;
    }

    //get h263+ header
    ptH263PlusHeader->revBit = GetBitField(wH263PlusHeader, 11, 5);
    ptH263PlusHeader->pBit   = GetBitField(wH263PlusHeader, 10, 1);
    ptH263PlusHeader->vrcBit = GetBitField(wH263PlusHeader, 9,  1);
    ptH263PlusHeader->phLen  = GetBitField(wH263PlusHeader, 3,  6);
    ptH263PlusHeader->pheBit = GetBitField(wH263PlusHeader, 0,  3);

    //保留位，应该为0，否则解析码流头信息有误
    if( 0 != ptH263PlusHeader->revBit )
    {
        return FALSE;
    }

    if( 2 == g_nShowDebugInfo && (0 == (m_tKdvSndStatistics.m_dwPackSendNum%100)) )
    {
        OspPrintf( 1, 0, "[CKdvNetSnd::ParasH263PlusHead] Info -- revBit:%d, pBit:%d, vrcBit:%d, phLen:%d, pheBit:%d ....  \n",
                         ptH263PlusHeader->revBit, ptH263PlusHeader->pBit,
                         ptH263PlusHeader->vrcBit, ptH263PlusHeader->phLen, ptH263PlusHeader->pheBit );
    }

    return TRUE;
}

/*=============================================================================
    函数名        ：SendOneH263Pack
    功能        ：发送标准的 一包 h.263/h.263+ RTP包 h.263+ 作为特殊的h.263码流集中处理
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义
                   pPackBuf  包数据缓冲
                   nPos      包索引

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendOneH263Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//非扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;

    //切分数据帧,由于数据帧带有H263header, 必定有MAX_H263_HEADER_LEN指定的空间。
    tRtpPack.m_nPreBufSize  = MAX_H263_HEADER_LEN;

    //last pack
    if(nPos == (m_ptH263HeaderList->m_nNum-1))
    {
        tRtpPack.m_byMark     = 1; //最后一包的标识位
    }

    //写入包长度
    tRtpPack.m_nRealSize = m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwDataLen;

    //加上包头
    //h263 RTP Header MODE A
    s32 nHeadLen = sizeof(u32);
    //h263 RTP Header MODE B
    if(MODE_H263_B == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
    {
        nHeadLen = 2*sizeof(u32);
    }

    //对于H263 存在ebit的包，均应该使用新缓冲
    BOOL32 bUseNewBuf = FALSE;

    //这里约定MODE_H263_D即为h.263+， 作为特殊的h.263码流集中处理
    if(MODE_H263_D == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
    {
        //是否存在 PSC/GSC等 需要在传输时的忽略的两字节,如果存在，则先忽略2字节的0
        u16   wHead = (u16)GetBitField(m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1, 0, 16);
        TH263PlusHeader tH263PlusHeader;
        memset(&tH263PlusHeader, 0, sizeof(tH263PlusHeader));
        if(FALSE == ParasH263PlusHead(wHead, &tH263PlusHeader))
        {
            //h.263+ payload format 有误
            wRet = ERROR_H263_PACK_TOOMUCH;
            return wRet;
        }
        if( (1 == tH263PlusHeader.pBit) && (0 == *((u16*)pPackBuf)) &&
            (tRtpPack.m_nRealSize > 2) )
        {
            pPackBuf += 2; //判断pebit，去除两个字节的0
            tRtpPack.m_nRealSize -= 2;
        }

        nHeadLen = sizeof(u16);
        tRtpPack.m_byPayload = MEDIA_TYPE_H263PLUS; //变更为h.263+动态载荷类型
        tRtpPack.m_pRealData = pPackBuf - nHeadLen;
        wHead = htons(wHead);
        *((u16*)tRtpPack.m_pRealData) = wHead;
    }
    else
    {
        //加上包头
        u32 dwHeaderArr[2];
        memset(&dwHeaderArr, 0, sizeof(dwHeaderArr));
        dwHeaderArr[0] = m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1;
        dwHeaderArr[1] = m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader2;

        if( 0 != GetBitField(dwHeaderArr[0], 24, 3) )
        {
            bUseNewBuf = TRUE;
        }

        dwHeaderArr[0] = htonl(dwHeaderArr[0]);
        dwHeaderArr[1] = htonl(dwHeaderArr[1]);

        tRtpPack.m_pRealData = pPackBuf - nHeadLen;
        memcpy(tRtpPack.m_pRealData, dwHeaderArr, nHeadLen);
    }

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        return wRet;
    }

    //h.263 也放入缓冲发送
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    函数名        ：SendH263Pack
    功能        ：发送标准的 h.263/h.263+ RTP包 h.263+ 作为特殊的h.263码流集中处理
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendH263Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //rtp 头信息分析，做合理性分析，并记录
    wRet = ParasH263Head(pFrmHdr);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();

        //add test by hual 2005-12-07
        if (ERROR_H263_PACK_TOOMUCH == wRet && 100 == g_nShowDebugInfo)
        {
            static s32 st_nSaveCount = 0;
            if (st_nSaveCount < 10)
            {
                FILE* file;
                s8 szFileName[32];
                sprintf(szFileName, "h263_err%d", st_nSaveCount);
                file = fopen(szFileName, "wb");
                fwrite(pFrmHdr->m_pData, sizeof(char), pFrmHdr->m_dwDataSize, file);
                fclose(file);

                OspPrintf(TRUE, FALSE, "H263 Frame is saved. Count=%d\n", st_nSaveCount);
                st_nSaveCount++;
            }
        }

        return wRet;
    }

    //得到有效数据的起始位置
    u8 *pBuf = pFrmHdr->m_pData + MAX_H263_HEADER_LEN +
               m_ptH263HeaderList->m_tKdvH263Header[0].m_dwStartPos;

    //send Rtp pack
    for(s32 nPos=0; nPos<m_ptH263HeaderList->m_nNum; nPos++)
    {
        pBuf = pFrmHdr->m_pData + MAX_H263_HEADER_LEN +
               m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwStartPos;

        wRet = SendOneH263Pack(pFrmHdr, pBuf, nPos);
        if(KDVFAILED(wRet))
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;

            //按照最大发送字节数 批量发送数据包
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    //按照最大发送字节数 真实发送数据包
    return m_pcRtp->SendBySize(m_nMaxSendSize);//批量发送
}

/*=============================================================================
    函数名        ：DecodeH264SPS
    功能        ：解析 h.264 码流中的 sps 信息
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明： TRUE - 成功
=============================================================================*/
BOOL32 CKdvNetSnd::DecodeH264SPS( TBitStream *s, TSeqParameterSetRBSP *sps,
                                  TKdvH264Header *pStdH264Header )
{
    u32 i;

    sps->profile_idc               = stdh264_bs_read( s, 8 );

    sps->constrained_set0_flag     = stdh264_bs_read( s, 1 );
    sps->constrained_set1_flag     = stdh264_bs_read( s, 1 );
    sps->constrained_set2_flag     = stdh264_bs_read( s, 1 );
    stdh264_bs_skip( s, 5 );        //reserved_zero

    sps->level_idc                 = stdh264_bs_read( s, 8 );

    sps->seq_parameter_set_id      = stdh264_bs_read_ue( s );
    sps->log2_max_frame_num_minus4 = stdh264_bs_read_ue( s );
    sps->pic_order_cnt_type        = stdh264_bs_read_ue( s );

    if (sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = stdh264_bs_read_ue( s );
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag      = stdh264_bs_read( s, 1 );
        sps->offset_for_non_ref_pic                = stdh264_bs_read_se( s );
        sps->offset_for_top_to_bottom_field        = stdh264_bs_read_se( s );
        sps->num_ref_frames_in_pic_order_cnt_cycle = stdh264_bs_read_ue( s );
        for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame[i]           = stdh264_bs_read_se( s );
        }
    }

    sps->num_ref_frames                        = stdh264_bs_read_ue( s );
    sps->gaps_in_frame_num_value_allowed_flag  = stdh264_bs_read( s, 1 );
    sps->pic_width_in_mbs_minus1               = stdh264_bs_read_ue( s );  //  Width
    sps->pic_height_in_map_units_minus1        = stdh264_bs_read_ue( s );  //  Height
    sps->frame_mbs_only_flag                   = stdh264_bs_read( s, 1 );
    if (!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag      = stdh264_bs_read( s, 1 );
    }
    sps->direct_8x8_inference_flag             = stdh264_bs_read( s, 1 );
    sps->frame_cropping_flag                   = stdh264_bs_read( s, 1 );

    if (sps->frame_cropping_flag)
    {
        sps->frame_cropping_rect_left_offset   = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_right_offset  = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_top_offset    = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_bottom_offset = stdh264_bs_read_ue( s );
    }

    sps->vui_parameters_present_flag           = stdh264_bs_read( s, 1 );
    if (sps->vui_parameters_present_flag)
    {
        //OspPintf( 1, 0, "VUI sequence parameters present but not supported, ignored, proceeding to next NALU\n");
    }

    sps->bIsValid = TRUE;

    pStdH264Header->m_bIsValidSPS = TRUE;
    pStdH264Header->m_dwSPSId     = sps->seq_parameter_set_id;
    pStdH264Header->m_wWidth      = (u16)(sps->pic_width_in_mbs_minus1 + 1) * 16;
    pStdH264Header->m_wHeight     = (u16)(sps->pic_height_in_map_units_minus1 + 1) * 16;

    return TRUE;
}

/*=============================================================================
    函数名        ：DecodeH264PPS
    功能        ：解析 h.264 码流中的 pps 信息
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明： TRUE - 成功
=============================================================================*/
BOOL32 CKdvNetSnd::DecodeH264PPS( TBitStream *s, TPicParameterSetRBSP *pps,
                                  TKdvH264Header *pStdH264Header )
{
    u32 i;
    s32 NumberBitsPerSliceGroupId;

    pps->pic_parameter_set_id     = stdh264_bs_read_ue( s );
    pps->seq_parameter_set_id     = stdh264_bs_read_ue( s );
    pps->entropy_coding_mode_flag = stdh264_bs_read( s, 1 );
    pps->pic_order_present_flag   = stdh264_bs_read( s, 1 );
    pps->num_slice_groups_minus1  = stdh264_bs_read_ue( s );

    // FMO stuff begins here
    if (pps->num_slice_groups_minus1 > 0)
    {
        pps->slice_group_map_type = stdh264_bs_read_ue( s );

        switch( pps->slice_group_map_type )
        {
            case 0:
            {
                for (i=0; i<=pps->num_slice_groups_minus1; i++)
                {
                    pps->run_length_minus1 [i] = stdh264_bs_read_ue( s );
                }
                break;
            }
            case 2:
            {
                for (i=0; i<pps->num_slice_groups_minus1; i++)
                {
                    //! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
                    pps->top_left [i]                  = stdh264_bs_read_ue( s );
                    pps->bottom_right [i]              = stdh264_bs_read_ue( s );
                }
                break;
            }
            case 3:
            case 4:
            case 5:
            {
                pps->slice_group_change_direction_flag = stdh264_bs_read( s, 1 );
                pps->slice_group_change_rate_minus1    = stdh264_bs_read_ue( s );
                break;
            }
            case 6:
            {
                if (pps->num_slice_groups_minus1+1 >4)
                {
                    NumberBitsPerSliceGroupId = 3;
                }
                else if (pps->num_slice_groups_minus1+1 > 2)
                {
                    NumberBitsPerSliceGroupId = 2;
                }
                else
                {
                    NumberBitsPerSliceGroupId = 1;
                }

                //! JVT-F078, exlicitly signal number of MBs in the map
                pps->num_slice_group_map_units_minus1      = stdh264_bs_read_ue( s );
                for (i=0; i<=pps->num_slice_group_map_units_minus1; i++)
                {
                    pps->slice_group_id[i] = stdh264_bs_read(s,NumberBitsPerSliceGroupId );//maywrong
                }
                break;
            }
            default:
                break;
        }
    }

    // End of FMO stuff

    pps->num_ref_idx_l0_active_minus1           = stdh264_bs_read_ue( s );
    pps->num_ref_idx_l1_active_minus1           = stdh264_bs_read_ue( s );
    pps->weighted_pred_flag                     = stdh264_bs_read( s, 1 );
    pps->weighted_bipred_idc                    = stdh264_bs_read( s, 2 );
    pps->pic_init_qp_minus26                    = stdh264_bs_read_se( s );
    pps->pic_init_qs_minus26                    = stdh264_bs_read_se( s );
    pps->chroma_qp_index_offset                 = stdh264_bs_read_se( s );
    pps->deblocking_filter_control_present_flag = stdh264_bs_read( s, 1 );
    pps->constrained_intra_pred_flag            = stdh264_bs_read( s, 1 );
    pps->redundant_pic_cnt_present_flag         = stdh264_bs_read( s, 1 );

    pps->bIsValid = TRUE;
    pStdH264Header->m_bIsValidPPS = TRUE;

    return TRUE;
}

/*=============================================================================
    函数名        ：ParseH264RtpHead
    功能        ：Std h.264 RTP head info 分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pRtpPackBuf     RTP数据指针
                   pdwHeaderLen    数据包包头长度 *pdwHeaderLen - 0 无效包头
                   ptH264Header    头结构指针

    返回值说明： 无
=============================================================================*/
void CKdvNetSnd::ParseH264RtpHead( u8 *pRtpPackBuf, s32 nRtpPackBufSize,
                                   TKdvH264Header *ptH264Header )
{
    if(nRtpPackBufSize == 0 || NULL == ptH264Header)
    {
        return;
    }
    // the format of the NAL unit type octet is reprinted below:
    //  +---------------+
    //    |0|1|2|3|4|5|6|7|
    //    +-+-+-+-+-+-+-+-+
    //    |F|NRI|  Type   |
    //    +---------------+
    u8  byNaluTypeOctet = (u8) (*pRtpPackBuf);

    u32 dwNaluType = byNaluTypeOctet & 0x1F;  // 取得低5位
    TBitStream tBitStream;
    memset(&tBitStream, 0, sizeof(tBitStream));
    // TBitStream结构从RTP包的第14字节开始（前12字节为RTP头，第13字节为Nalu类型）
    stdh264_bs_init(&tBitStream, pRtpPackBuf + 1, nRtpPackBufSize);
    TSeqParameterSetRBSP tSPS;
    TPicParameterSetRBSP tPPS;
    Tstdh264Dec_SliceHeaderData tSlice_header;
    memset(&tSPS, 0, sizeof(tSPS));
    memset(&tPPS, 0, sizeof(tPPS));
    memset(&tSlice_header, 0, sizeof(tSlice_header));

    switch(dwNaluType)
    {
    case 1:
    case 5:  // NALU_TYPE_IDR
        stdh264_FirstPartOfSliceHeader(&tBitStream, &tSlice_header);
        if( I_SLICE == tSlice_header.slice_type)
        {
            ptH264Header->m_bIsKeyFrame = TRUE;
        }
        break;
    case 7:  // NALU_TYPE_SPS
        DecodeH264SPS(&tBitStream, &tSPS, ptH264Header);
        //OspPrintf( 1, 0, "[CKdvNetSnd::ParseH264RtpHead] DecodeH264SPS ...........\n" );
        break;
    case 8:  // NALU_TYPE_PPS
        DecodeH264PPS(&tBitStream, &tPPS, ptH264Header);
        //OspPrintf( 1, 0, "[CKdvNetSnd::ParseH264RtpHead] DecodeH264PPS ...........\n" );
        break;
    default:
        break;
    }

    return;
}

/*=============================================================================
    函数名        ：ParseH264Head
    功能        ：std h.264 RTP head info 分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::ParseH264Head(PFRAMEHDR pFrmHdr)
{
    u32 *pdwPos = (u32 *)pFrmHdr->m_pData;
    memset(m_ptH264HeaderList, 0, sizeof(TH264HeaderList));

    //编码器传输的四字节length为【低位低字节】，
    //WIN32存储也为【低位低字节】，无需翻转
    //VXWORKS存储也为【低位高字节】，需翻转

    u32 dwIndex = 0;
    u32 dwPackLen = 0;

    //判断芯片字节类型
    if (0x1234 == ntohs(0x1234))
    {   //big endian
        while((dwIndex < MAX_NALU_NUM) &&
            ((dwPackLen = pdwPos[dwIndex]) != 0))
        {
            //底层传入的每包长度是little endian，进行转换
            m_ptH264HeaderList->m_dwH264NaluArr[dwIndex] = (dwPackLen << 24) |
                                                            ((dwPackLen & 0x0000ff00) << 8) |
                                                            ((dwPackLen & 0x00ff0000) >> 8) |
                                                            (dwPackLen >> 24);
            dwIndex++;
        }
    }
    else
    {
        //little endian
        while((dwIndex < MAX_NALU_NUM) &&
            ((dwPackLen = pdwPos[dwIndex]) != 0))
        {
            m_ptH264HeaderList->m_dwH264NaluArr[dwIndex] = dwPackLen;
            dwIndex++;
        }
    }


    m_ptH264HeaderList->m_dwNaluNum = dwIndex;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SendH264Pack
    功能        ：发送标准的 一包 h.264 RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr   数据帧信息，参见结构定义
                   pPackBuf  包数据缓冲
                   nPos      包索引

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    // 264patch for 8180
    if (m_ptH264HeaderList->m_dwH264NaluArr[nPos] > g_dwMaxExtendPackSize)
    {
        //切FU包分别发送
        if (222 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[MediaNet] h264 pack long slice, cut to fu-a segment.\n");
        }

        u16 wRet;
        wRet = SendOneH264Pack_LongSlice(pFrmHdr, pPackBuf,
            m_ptH264HeaderList->m_dwH264NaluArr[nPos],
            (nPos == (s32)(m_ptH264HeaderList->m_dwNaluNum - 1)) );

        return wRet;
    }

    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;//不是最后一包
    tRtpPack.m_byExtence    = 1;//扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);

    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    *((u32 *)(&(m_byExBuf[0])))=((pFrmHdr->m_tVideoParam.m_bKeyFrame<<24)&0xFF000000)|((pFrmHdr->m_tVideoParam.m_wVideoWidth<<12)&0x00FFF000)|(pFrmHdr->m_tVideoParam.m_wVideoHeight&0x00000FFF);
    //帧率
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    // 按照数据帧Nalu个数发送RTP数据包，每个Nalu为一个RTP包；
    // 缓冲数据前MAX_NALU_UNIT_SIZE字节为Nalu长度的空间。
    //tRtpPack.m_nPreBufSize  = MAX_NALU_UNIT_SIZE;
    tRtpPack.m_nPreBufSize  = 0;

    //last pack
    if(nPos == (s32)(m_ptH264HeaderList->m_dwNaluNum - 1))
    {
        tRtpPack.m_byMark = 1; //最后一包的标识位
    }

    // 写入包长度
    tRtpPack.m_nRealSize  = m_ptH264HeaderList->m_dwH264NaluArr[nPos];

    tRtpPack.m_pRealData  = pPackBuf;

    //test later del  -- h.264 RTP head info 分析
    /*
    if(nPos == (s32)(m_ptH264HeaderList->m_dwNaluNum - 1))
    {
        TKdvH264Header tH264Header;
        memset( &tH264Header, 0, sizeof(tH264Header) );
        ParseH264RtpHead(tRtpPack.m_pRealData, tRtpPack.m_nRealSize, &tH264Header);

        OspPrintf( 1, 0, "m_tVideoParam.m_bKeyFrame=%d, tH264Header.m_bIsKeyFrame=%d   \n",
                          pFrmHdr->m_tVideoParam.m_bKeyFrame, tH264Header.m_bIsKeyFrame );
    }
    */

    //码流加密操作
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        return wRet;
    }

    //放入缓冲发送
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    函数名        ：SendH264Pack
    功能        ：发送标准的 h.264 RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendH264Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //////////////////////////////////////////////////////////////////////////
    // xp090224 for flag0001 264 frame
    if (pFrmHdr->m_dwDataSize < 4)
    {
        OspPrintf(TRUE, FALSE, "[MediaNet] error : h264 frame length < 4.\n");
//        PrintDebugInfo(0, FI, "err: h264 frame length < 4");
        return ERROR_SND_PARAM;
    }
    //兼容0001以及001混合的情况（松下码流采用001的分割方式，sony码流有0001和001混合的情况）
    if ((pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 0 )||
        (pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 1 ))
    {
        //目前接收端已经全部采用0001接收组帧方式，因此发送也全部采用0001方式
         return SendH264Pack_Flag0001(pFrmHdr);
    }
    else
    {
        //rtp 头信息分析，做合理性分析，并记录
        wRet = ParseH264Head(pFrmHdr);
        if(KDVFAILED(wRet))
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }

        //得到有效数据的起始位置
        u8 *pBuf = pFrmHdr->m_pData + MAX_NALU_UNIT_SIZE;
        u32 dwSendLength = 0;
        //send Rtp pack
        for(u32 nPos=0; nPos<m_ptH264HeaderList->m_dwNaluNum; nPos++)
        {
            dwSendLength += m_ptH264HeaderList->m_dwH264NaluArr[nPos];
            if (dwSendLength > pFrmHdr->m_dwDataSize)
            {
                OspPrintf(TRUE, FALSE, "sendlength:%d > frame length:%d \n", dwSendLength, pFrmHdr->m_dwDataSize);
                break;
            }
            wRet = SendOneH264Pack(pFrmHdr, pBuf, nPos);
            if(KDVFAILED(wRet))
            {
                m_tKdvSndStatistics.m_dwFrameLoseNum++;

                //按照最大发送字节数 批量发送数据包
                m_pcRtp->SendBySize(m_nMaxSendSize);
                m_pcRtp->ResetSeq();

                return wRet;
            }

            pBuf += m_ptH264HeaderList->m_dwH264NaluArr[nPos];

            m_tKdvSndStatistics.m_dwPackSendNum++;
        }

        //按照最大发送字节数 真实发送数据包
        return m_pcRtp->SendBySize(m_nMaxSendSize);//批量发送
    }
}

/*=============================================================================
    函数名        ：SendH264Pack_Flag0001
    功能        ：以0001方式发送H264rtp包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendH264Pack_Flag0001(PFRAMEHDR pFrmHdr)//兼容0001以及001混合模式
{
    u8 *pbyData = pFrmHdr->m_pData;
    u32 dwFrameLen = pFrmHdr->m_dwDataSize;
    u32 i;
    u32 dwNaluLen = 0;
    u8 *pbyNaluData = NULL;
    u8 byInterval = 4;//默认为0001
    if (0 == pbyData[0] &&  0 == pbyData[1] && 0 == pbyData[2])//0001
    {
        pbyNaluData = pbyData + 4;
        byInterval = 4;
    }
    else//001
    {
        pbyNaluData = pbyData + 3;
        byInterval = 3;
    }

    BOOL32 bNaluEnd = FALSE;
    BOOL32 bMark = FALSE;
    for (i=byInterval; i<dwFrameLen; i++)
    {
        if (0 == pbyData[i] &&  0 == pbyData[i + 1]
            && 0 == pbyData[i + 2] && 1 == pbyData[i + 3]  && dwFrameLen - i > 6)//0001
        {
            bNaluEnd = TRUE;
            dwNaluLen = i - (pbyNaluData - pbyData);
            byInterval = 4;
        }
        else if (0 == pbyData[i] &&  0 == pbyData[i + 1]
            && 1 == pbyData[i + 2]  && dwFrameLen - i > 6)//001
        {
            bNaluEnd = TRUE;
            dwNaluLen = i - (pbyNaluData - pbyData);
            byInterval = 3;
        }
        else if (i >= dwFrameLen - 4)
        {
            bNaluEnd = TRUE;
            dwNaluLen = dwFrameLen - (pbyNaluData - pbyData);
        }

        if (i >= dwFrameLen - 4)
        {
            bMark = TRUE;
        }

        if (bNaluEnd)
        {
            // send one nalu
            //带有扩展头的话，一包大小不能超过MAX_EXTEND_PACK_SIZE
            if (dwNaluLen > g_dwMaxExtendPackSize)
            {
                //切FU包分别发送
                if (222 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(TRUE, FALSE, "h264 pack long slice, cut to fu-a segment.\n");
                }
                SendOneH264Pack_LongSlice(pFrmHdr, pbyNaluData, dwNaluLen, bMark);
            }
            else
            {
                SendOneH264Pack_Flag0001(pFrmHdr, pbyNaluData, dwNaluLen, bMark);
            }

            if (bMark)
            {
                break;
            }
            else
            {
                bNaluEnd = FALSE;
                pbyNaluData = pbyData + i + byInterval;
                i += byInterval - 1;
            }
        }
    }

    //按照最大发送字节数 真实发送数据包
    return m_pcRtp->SendBySize(m_nMaxSendSize);//批量发送
}

/*=============================================================================
    函数名        ：SendOneH264Pack_LongSlice
    功能        ：切碎片包发送H264rtp包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义
                   u8 *pbyNalu  一个nalu
                   u32 dwNaluLen  nalu 长度
                   BOOL32 bMark   帧边界标志
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pbyNalu,
                                          u32 dwNaluLen, BOOL32 bMark)
{
    u16 wRet = MEDIANET_NO_ERROR;
    //以前的H264不带扩展头，现在为了支持30帧，需要加上12字节的扩展头
    const s32 FU_MAX_LEN = g_dwMaxExtendPackSize - 2;
    // 第一个字节是indicator，不算
    const s32 FU_NUM = (dwNaluLen - 1  + FU_MAX_LEN - 1) / FU_MAX_LEN;
    s32 i;
    u8 *pbyFuData;
    u8 abyFuPack[MAX_EXTEND_PACK_SIZE];
    s32 nFuSize;
    BOOL32 bLastFU = FALSE;

    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 1;//扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);//为支持发送30帧率码流，H264需加上3个字节的扩展头

    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    *((u32 *)(&(m_byExBuf[0]))) = htonl(((pFrmHdr->m_tVideoParam.m_bKeyFrame<<24)&0xFF000000)|((pFrmHdr->m_tVideoParam.m_wVideoWidth<<12)&0x00FFF000)|(pFrmHdr->m_tVideoParam.m_wVideoHeight&0x00000FFF));
    //帧率
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack_LongSlice]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    // 先设置好indicator
    abyFuPack[0] = (pbyNalu[0] & 0xe0) | 28;//28是FU-A的type

    for (i=0; i<FU_NUM; i++)
    {
        // fu buffer
        pbyFuData = pbyNalu + 1 + i * FU_MAX_LEN;
        if (i == FU_NUM - 1)
        {
            nFuSize = dwNaluLen - 1 - i * FU_MAX_LEN;
        }
        else
        {
            nFuSize = FU_MAX_LEN;
        }

        // first pack
        if (i == 0)
        {
            abyFuPack[1] = (pbyNalu[0] & 0x1f) | 0x80;//S
        }
        // last pack
        else if (i == FU_NUM - 1)
        {
            abyFuPack[1] = (pbyNalu[0] & 0x1f) | 0x40;//E
            bLastFU = TRUE;
        }
        else
        {
            abyFuPack[1] = pbyNalu[0] & 0x1f;
        }

        // 把前面两个字节加上去
        memcpy(abyFuPack + 2, pbyFuData, nFuSize);

        tRtpPack.m_pRealData    = abyFuPack;
        tRtpPack.m_nRealSize    = nFuSize + 2;
        tRtpPack.m_byMark       = (u8)(bMark & bLastFU);

        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            break;
        }

        //放入缓冲发送
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            if (g_nShowDebugInfo == 220 || g_nShowDebugInfo == 255)
            {
                OspPrintf(TRUE, FALSE, "[SendOneH264Pack_LongSlice]write to buff failed!\n");
            }
            break;
        }
    }

    return wRet;
}

/*=============================================================================
    函数名        ：SendOneH264Pack_Flag0001
    功能        ：以0001方式发送H264rtp包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义
                   u8 *pbyNalu  一个nalu
                   u32 dwNaluLen  nalu 长度
                   BOOL32 bMark   帧边界标志
    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf,
                                         u32 dwNaluLen, BOOL32 bMark)
{
    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 1;//扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);

    tRtpPack.m_pRealData    = pPackBuf;
    tRtpPack.m_nRealSize    = dwNaluLen;
    tRtpPack.m_byMark       = (u8)bMark;

    //H264扩展头格式
    /////////////////////////////////////////////////////////////////////////////////////////
    //    0          7               19             31
    //|----------|---------------|--------------|
    //   key          width             wheigh
    //dwExtData=((byKeyFrame<<24)&0xFF000000)|((wWidth<<12)&0x00FFF000)|(wHeigh&0x00000fff);
    //
    //0                                         31
    //|-----------------------------------------|
    //               framerate
    //#define H264_EX_FRAMERTAE_POS    (s32)4
    //
    //0                                         31
    //|-----------------------------------------|
    //              mobiletime
    //#define H264_EX_MOBILETIME_POS    (s32)8
    /////////////////////////////////////////////////////////////////////////////////////////////

    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    *((u32 *)(&(m_byExBuf[0]))) = htonl(((pFrmHdr->m_tVideoParam.m_bKeyFrame<<24)&0xFF000000)|((pFrmHdr->m_tVideoParam.m_wVideoWidth<<12)&0x00FFF000)|(pFrmHdr->m_tVideoParam.m_wVideoHeight&0x00000FFF));
    //帧率
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack_Flag0001]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }
    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        return wRet;
    }

    //放入缓冲发送
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    函数名        ：SendSdAudioPack
    功能        ：发送标准的音频RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendSdAudioPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //一帧直接发送，不做切分
    if(pFrmHdr->m_dwDataSize > MAX_SND_PACK_SIZE)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }

    if (pFrmHdr->m_byMediaType == MEDIA_TYPE_AMR && pFrmHdr->m_dwDataSize + 16 > MAX_SND_PACK_SIZE)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }
    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//非扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 0;
    if (pFrmHdr->m_byMediaType == MEDIA_TYPE_AMR)
    {
        u8 byRtpData[MAX_SND_PACK_SIZE] = {0};
        byRtpData[0] = 0xf0;//CMR-15,4位填充
        memcpy(byRtpData+1, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
        tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize+1;
        tRtpPack.m_pRealData    = byRtpData;
    }
    else
    {
        //老的G.711标准：数据块前不加标识位。
        tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize;// + G711_REVERSE_BIT;
        tRtpPack.m_pRealData    = pFrmHdr->m_pData;// - G711_REVERSE_BIT;
    }


    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //音频帧直接发送
    wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }
    m_tKdvSndStatistics.m_dwPackSendNum++; //hual

    //标准RTP包,直接发送不经过缓冲。
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendSdAACAudioPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;
    u8 byExtence[12] = {0};
    //一帧直接发送，不做切分
    if(pFrmHdr->m_dwDataSize > MAX_EXTEND_PACK_SIZE-4)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }

    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 1;//非扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 1;//AACLC的mark位一定要置1，否则vlc解出来会有问题（可能因为aaclc是变长的缘故）

    if (TRUE == tRtpPack.m_byExtence)
    {
        byExtence[2] = pFrmHdr->m_byAudioMode;
        tRtpPack.m_pExData = byExtence;
        tRtpPack.m_nExSize      = 3;
    }

    if (!m_bAaclcNoHead)
    {
        //rfc3640上规定，aaclc在以rtp方式打包的情况下要加上4字节的头
        m_byAACPacket[0] = 0x00;
        m_byAACPacket[1] = 0x10;
        m_byAACPacket[2] = (pFrmHdr->m_dwDataSize & 0x1fe0) >> 5;
        m_byAACPacket[3] = (pFrmHdr->m_dwDataSize & 0x1f) << 3;
        memcpy(m_byAACPacket+4, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
        tRtpPack.m_pRealData = m_byAACPacket;
        tRtpPack.m_nRealSize = pFrmHdr->m_dwDataSize+4;
    }
    else
    {
        tRtpPack.m_pRealData = pFrmHdr->m_pData;
        tRtpPack.m_nRealSize = pFrmHdr->m_dwDataSize;
    }

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //音频帧直接发送
    wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }
    m_tKdvSndStatistics.m_dwPackSendNum++; //hual

    //标准RTP包,直接发送不经过缓冲。
    return MEDIANET_NO_ERROR;
}
/*=============================================================================
    函数名        ：SendDataPayloadPack
    功能        ：以标准包发送 H.224/H.239/T.120 RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendDataPayloadPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //一帧直接发送，不做切分
    if(pFrmHdr->m_dwDataSize > MAX_SND_PACK_SIZE)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }

    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//非扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = 0;//pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize;
    tRtpPack.m_pRealData    = pFrmHdr->m_pData;

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //数据帧直接发送
    wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }
    m_tKdvSndStatistics.m_dwPackSendNum++;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：SendSdPack
    功能        ：发送标准的RTP包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pFrmHdr  数据帧信息，参见结构定义

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::SendSdPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    switch( pFrmHdr->m_byMediaType )
    {
    case MEDIA_TYPE_H261:
        wRet = SendH261Pack(pFrmHdr);
        break;
    case MEDIA_TYPE_H263:
    case MEDIA_TYPE_H263PLUS:
        wRet = SendH263Pack(pFrmHdr);
        break;
    case MEDIA_TYPE_H264:
        wRet = SendH264Pack(pFrmHdr);
        break;
    case MEDIA_TYPE_H265:
        wRet = SendH265Pack(pFrmHdr);
        break;
    case MEDIA_TYPE_PCMU:
    case MEDIA_TYPE_PCMA:
    case MEDIA_TYPE_G722:
    case MEDIA_TYPE_G728:
    case MEDIA_TYPE_G729:
    case MEDIA_TYPE_G7231:
    case MEDIA_TYPE_ADPCM:
    case MEDIA_TYPE_G7221C:
    case MEDIA_TYPE_AACLC:
    case MEDIA_TYPE_AACLD:
    case MEDIA_TYPE_AACLC_PCM:
	case MEDIA_TYPE_AMR:
	case MEDIA_TYPE_G726_16:
	case MEDIA_TYPE_G726_24:
	case MEDIA_TYPE_G726_32:
	case MEDIA_TYPE_G726_40:
		wRet = SendSdAudioPack(pFrmHdr);
        break;
    case MEDIA_TYPE_H224: //H.224 Data Payload
        wRet = SendDataPayloadPack(pFrmHdr);
        break;
    default:
        wRet = ERROR_SND_PARAM;
        break;
    }

    return wRet;
}

/*=============================================================================
    函数名        ：GetStatus
    功能        ：查询发送状态
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   tKdvSndStatus  要返回的发送状态，参见结构定义。

    返回值说明： 参见错误码定义
==============================================================================*/
u16 CKdvNetSnd::GetStatus(TKdvSndStatus &tKdvSndStatus)
{
    tKdvSndStatus = m_tKdvSndStatus;
     return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：GetStatistics
    功能        ：查询发送统计
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   tKdvSndStatistics  要返回的发送状态，参见结构定义。

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::GetStatistics(TKdvSndStatistics &tKdvSndStatistics)
{
    tKdvSndStatistics = m_tKdvSndStatistics;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：GetAdvancedSndInfo
    功能        ：查询发送端高级设置参数（重传、乱序重排等）
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   tAdvancedSndInfo  要返回的发送状态，参见结构定义。

    返回值说明： 参见错误码定义
=============================================================================*/
u16 CKdvNetSnd::GetAdvancedSndInfo(TAdvancedSndInfo &tAdvancedSndInfo)
{
    tAdvancedSndInfo.m_bRepeatSend     = m_bRepeatSend;
    tAdvancedSndInfo.m_nMaxSendNum     = m_nMaxSendNum;
    tAdvancedSndInfo.m_wBufTimeSpan    = m_wBufTimeSpan;
    tAdvancedSndInfo.m_byLocalActivePT = m_byLocalActivePT;

    if(NULL != m_pszMaterialBuf)
    {
        tAdvancedSndInfo.m_bEncryption   = TRUE;
        tAdvancedSndInfo.m_byEncryptMode = m_byEncryptMode;
        tAdvancedSndInfo.m_wKeySize      = m_wMaterialBufLen;
        memcpy(tAdvancedSndInfo.m_szKeyBuf, m_pszMaterialBuf, tAdvancedSndInfo.m_wKeySize);
    }
    else
    {
        tAdvancedSndInfo.m_bEncryption   = FALSE;
        tAdvancedSndInfo.m_byEncryptMode = 0;
        tAdvancedSndInfo.m_wKeySize      = 0;
    }

    return MEDIANET_NO_ERROR;
}


u16 CKdvNetSnd::GetSndSocketInfo(TKdvSndSocketInfo &tRtpSockInfo, TKdvSndSocketInfo &tRtcpSockInfo)
{
    CREATE_CHECK    //是否创建
    m_pcRtp->GetSndSocketInfo(tRtpSockInfo);
    m_pcRtcp->GetSndSocketInfo(tRtcpSockInfo);
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendPSFrame(PFRAMEHDR pFrmHdr)
{
    CREATE_CHECK;    //是否创建
    u16 wRet = MEDIANET_NO_ERROR;
    if (NULL != pFrmHdr && NULL != m_hTspsWrite)
    {
        wRet = TspsWriteWriteFrame(m_hTspsWrite, (TspsFRAMEHDR *)pFrmHdr);
        if (wRet != TSPS_OK)
        {
            return ERROR_SND_PSSEND;
        }
    }
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendPS(TspsFRAMEHDR *ptFrame)
{
    if (NULL == ptFrame)
    {
        return ERROR_SND_PARAM;
    }

    //帧ID累加
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwFrameNum++;

    //时间戳累加
    IncreaseTimestamp((PFRAMEHDR)ptFrame, TRUE);

    u16 wRet = SendPSPack((PFRAMEHDR)ptFrame);
    if (wRet != MEDIANET_NO_ERROR)
    {
        if (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvNetSnd::SendPS] ERROE CODE is %d \n",  wRet);
        }
        return wRet;
    }

    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendPSPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    if (NULL == pFrmHdr)
    {
        return ERROR_SND_PARAM;
    }

    s32 nRealPackLen = g_dwMaxExtendPackSize;
    s32 nPackNum     = (pFrmHdr->m_dwDataSize  + nRealPackLen - 1) / nRealPackLen;

    //始终会有一包发送
    if(0 == nPackNum)
    {
        nPackNum = 1;
    }

    //传给CKdvRtp类的RTP数据包信息
    TRtpPack  tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp    = m_dwTimeStamp;
    tRtpPack.m_byExtence    = 0;//扩展包
    tRtpPack.m_nRealSize    = nRealPackLen;
    tRtpPack.m_nExSize        = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;

    tRtpPack.m_byPayload = MEDIA_TYPE_PS;

    s32 nPackIndex = 0;

    //循环处理n-1包
    for(nPackIndex=1; nPackIndex<nPackNum; nPackIndex++)
    {
        tRtpPack.m_nRealSize = nRealPackLen;
        //有效数据
        tRtpPack.m_pRealData = pFrmHdr->m_pData + (nPackIndex-1)*nRealPackLen;

        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //加密失败
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //写入一包
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //缓冲满
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();
            return wRet;
        }
        //统计
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    /*最后一包*/
    s32 nLastPackLen = pFrmHdr->m_dwDataSize - nRealPackLen * (nPackNum - 1);

    tRtpPack.m_byMark            = 1;
    tRtpPack.m_nRealSize        = nLastPackLen;
    tRtpPack.m_pRealData        = pFrmHdr->m_pData + (nPackNum - 1) * nRealPackLen;

    //码流加密操作
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        //按照最大发送字节数 批量发送数据包
        m_pcRtp->SendBySize(m_nMaxSendSize);
        m_pcRtp->ResetSeq();

        return wRet;
    }
    wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        //缓冲满
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        //按照最大发送字节数 批量发送数据包
        m_pcRtp->SendBySize(m_nMaxSendSize);
        m_pcRtp->ResetSeq();

        return wRet;
    }

    //统计
    m_tKdvSndStatistics.m_dwPackSendNum++;

    //按照最大发送字节数 批量发送数据包
    wRet = m_pcRtp->SendBySize(m_nMaxSendSize);

    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendH265Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //////////////////////////////////////////////////////////////////////////
    if (pFrmHdr->m_dwDataSize < 4)
    {
        OspPrintf(TRUE, FALSE, "[MediaNet] error : h265 frame length < 4.\n");
        return ERROR_SND_PARAM;
    }
    //兼容0001以及001混合的情况（松下码流采用001的分割方式，sony码流有0001和001混合的情况）
    if ((pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 0 )||
        (pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 1 ))
    {
        //目前接收端已经全部采用0001接收组帧方式，因此发送也全部采用0001方式
        return SendH265Pack_Flag0001(pFrmHdr);
    }
    else
    {
        return     ERROR_SND_H265FRAME;
    }
}

u16 CKdvNetSnd::SendH265Pack_Flag0001(PFRAMEHDR pFrmHdr)
{
    u8 *pbyData = pFrmHdr->m_pData;
    u32 dwFrameLen = pFrmHdr->m_dwDataSize;
    u32 i;
    u32 dwNaluLen = 0;
    u8 *pbyNaluData = NULL;
    u8 byInterval = 4;//默认为0001
    if (0 == pbyData[0] &&  0 == pbyData[1] && 0 == pbyData[2])//0001
    {
        pbyNaluData = pbyData + 4;
        byInterval = 4;
    }
    else//001
    {
        pbyNaluData = pbyData + 3;
        byInterval = 3;
    }

    BOOL32 bNaluEnd = FALSE;
    BOOL32 bMark = FALSE;
    for (i=byInterval; i<dwFrameLen; i++)
    {
        if (0 == pbyData[i] &&  0 == pbyData[i + 1]
            && 0 == pbyData[i + 2] && 1 == pbyData[i + 3]  && dwFrameLen - i > 6)//0001
        {
            bNaluEnd = TRUE;
            dwNaluLen = i - (pbyNaluData - pbyData);
            byInterval = 4;
        }
        else if (0 == pbyData[i] &&  0 == pbyData[i + 1]
            && 1 == pbyData[i + 2]  && dwFrameLen - i > 6)//001
        {
            bNaluEnd = TRUE;
            dwNaluLen = i - (pbyNaluData - pbyData);
            byInterval = 3;
        }
        else if (i >= dwFrameLen - 4)
        {
            bNaluEnd = TRUE;
            dwNaluLen = dwFrameLen - (pbyNaluData - pbyData);
        }

        if (i >= dwFrameLen - 4)
        {
            bMark = TRUE;
        }

        if (bNaluEnd)
        {
            // send one nalu
            //带有扩展头的话，一包大小不能超过MAX_EXTEND_PACK_SIZE
            if (dwNaluLen > g_dwMaxExtendPackSize)
            {
                //切FU包分别发送
                if (222 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(TRUE, FALSE, "h265 pack long slice, cut to fu-a segment.\n");
                }
                SendOneH265Pack_LongSlice(pFrmHdr, pbyNaluData, dwNaluLen, bMark);
            }
            else
            {
                SendOneH265Pack_Flag0001(pFrmHdr, pbyNaluData, dwNaluLen, bMark);
            }

            if (bMark)
            {
                break;
            }
            else
            {
                bNaluEnd = FALSE;
                pbyNaluData = pbyData + i + byInterval;
                i += byInterval - 1;
            }
        }
    }

    //按照最大发送字节数 真实发送数据包
    return m_pcRtp->SendBySize(m_nMaxSendSize);//批量发送
}

u16 CKdvNetSnd::SendOneH265Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf, u32 dwNaluLen, BOOL32 bMark)
{
    //传给底层的RTP结构
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 0;//扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;

    tRtpPack.m_pRealData    = pPackBuf;
    tRtpPack.m_nRealSize    = dwNaluLen;
    tRtpPack.m_byMark       = (u8)bMark;

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //码流加密操作
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //加密失败
        return wRet;
    }

    //放入缓冲发送
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

u16 CKdvNetSnd::SendOneH265Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pbyNalu, u32 dwNaluLen, BOOL32 bMark)
{
    u16 wRet = MEDIANET_NO_ERROR;
    const s32 FU_MAX_LEN = g_dwMaxExtendPackSize - 3;
    // 第一个字节是indicator，不算
    const s32 FU_NUM = (dwNaluLen - 1  + FU_MAX_LEN - 1) / FU_MAX_LEN;
    s32 i;
    u8 *pbyFuData;
    u8 abyFuPack[MAX_EXTEND_PACK_SIZE];
    s32 nFuSize;
    BOOL32 bLastFU = FALSE;

    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 0;//扩展包
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//时间戳
    tRtpPack.m_nExSize      = 0;

    //是否设置了动态载荷值, 是则将码流真实载荷PT值替换为动态载荷PT值
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }
    /*0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |    PayloadHdr (Type=49)       |   FU header   | DONL (cond)   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
    | DONL (cond)   |                                               |
    |-+-+-+-+-+-+-+-+                                               |
    |                         FU payload                            |
    |                                                               |
    |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                               :...OPTIONAL RTP padding        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    //The structure of FU header
    //+---------------+
    //|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+
    //|S|E|  FuType   |
    //+---------------+
    //The structure of HEVC NAL unit header
    //+---------------+---------------+
    //|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|F|   Type    |  LayerId  | TID |
    //+-------------+-----------------+

    abyFuPack[0] = (pbyNalu[0] & 0x81) | 0x62;//49是FU-A的type
    abyFuPack[1] = pbyNalu[1];
    for (i=0; i<FU_NUM; i++)
    {
        // fu buffer
        pbyFuData = pbyNalu + 2 + i * FU_MAX_LEN;
        if (i == FU_NUM - 1)
        {
            nFuSize = dwNaluLen - 2 - i * FU_MAX_LEN;
        }
        else
        {
            nFuSize = FU_MAX_LEN;
        }

        // first pack
        if (i == 0)
        {
            abyFuPack[2] = ((pbyNalu[0] & 0x7e)>>1) | 0x80;//S
        }
        // last pack
        else if (i == FU_NUM - 1)
        {
            abyFuPack[2] = ((pbyNalu[0] & 0x7e)>>1) | 0x40;//E
            bLastFU = TRUE;
        }
        else
        {
            abyFuPack[2] = ((pbyNalu[0] & 0x7e)>>1) | 0x00;
        }

        // 把前面两个字节加上去
        memcpy(abyFuPack + 3, pbyFuData, nFuSize);

        tRtpPack.m_pRealData    = abyFuPack;
        tRtpPack.m_nRealSize    = nFuSize + 3;
        tRtpPack.m_byMark       = (u8)(bMark & bLastFU);

        //码流加密操作
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            break;
        }

        //放入缓冲发送
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            if (g_nShowDebugInfo == 220 || g_nShowDebugInfo == 255)
            {
                OspPrintf(TRUE, FALSE, "[SendOneH265Pack_LongSlice]write to buff failed!\n");
            }
            break;
        }
    }

    return wRet;
}

u16 CKdvNetSnd::SetAaclcSend(BOOL32 bNoHead)
{
    m_bAaclcNoHead = bNoHead;

    return MEDIANET_NO_ERROR;
}
/* End of File*/

