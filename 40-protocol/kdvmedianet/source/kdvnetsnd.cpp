/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvNetSnd.cpp
����ļ�    : KdvNetSnd.h
�ļ�ʵ�ֹ���: CKdvNetSnd Implement
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
2004/04/13  3.0         ����      �����ش�������
2004/06/10  3.0         ����      ����H.263���� Bģʽ �ķ�������
2004/08/13  3.0         ����      ����H.263/H.261 ��Ϊ���뻺�����������Ա���ⷢ������
2004/09/22  3.0         ����      ���ӷ��Ͷ˶���H.264 ��̬�غɵ�֧��
2004/09/29  3.0         ����      ���ӷ��Ͷ˶���H.224 ��̬�غɵ�֧��
2004/09/29  2.0         ����      ����linux�汾����֧��
2004/09/30  3.0         ����      ���ӷ��Ͷ˶��ڷ��������ļ��ܵ�֧��
2004/10/08  3.0         ����      ���ӷ��Ͷ˶���H.263+ ��̬�غɵ�֧��
2004/10/12  3.0         ����      �������ݴ������
2004/11/02  3.0         ����      ��Ƶ���������뻺�岢����������ֽ��� �����������ݰ�
2004/11/16  3.0         ����      ���Ӷ��� mpeg2 ���Զ����а���ʽ�������շ���֧��
2004/11/20  3.0         ����      ���ͻ�������ʱ������ǰ��ԭ�л������ݽ��з���
2004/11/30  3.0         ����      ����AES����ģʽ֧��
2004/12/01  3.0         ����      �޸�AES�ӽ��ܵĳ�ʼ������ֵ����SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
2004/12/29  3.0         ����      ���� ���Ͷ˼������ýӿڣ����Ӷ�̬�غ�ֵ�����ýӿ�
2005/01/18  3.5         ����      ���Ӷ�Ƕ��ʽ����ϵͳ������̫��ƽ�����ͽӿ�����ƽ������ʱ���ã�
2005/01/26  3.5         ����      ���ӷ��Ͷ˶��� G.729 �غɵ�֧��
2005/05/14  3.6         ����        ����RTCP�鲥������SR��
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

//ͳһ�ṩ�շ������SSRC�Ĳ���
extern u32 GetExclusiveSSRC();

//H.261 �� H.263 ����ԭʼrtp��ֱ�ӷ��͵Ŀ���
extern s32   g_nRtpDSend;

//���Ͷ�ģ��С������Ĳ������
extern s32   g_nConfuedSpan;

//���Ͷ�ģ�� H.263С������Ĳ�������Ƿ����仯
extern s32   g_nSpanChanged;

// ��ѯ�Ƿ�ʹ�ö�ʱ������̫�������Դﵽƽ������
extern BOOL32 g_bUseSmoothSnd;
// ��ǰOS�Ƿ�֧������ƽ������
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

/*��ʼ�����Ա*/
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

/*���ٶ���*/
CKdvNetSnd::~CKdvNetSnd()
{
    FreeBuf();
}

/*=============================================================================
    ������        ��FreeBuf
    ����        ����Create�������Ӧ�����Create�����ﴴ���Ķ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������
    ����ֵ˵����  ��
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
    // ֪ͨ���������ͷ���Ӧ���ʹ��� 2005-01-18
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
    ������        ��FreeBuf
    ����        ����ʼ������������ռ䣬���ɶ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  dwMaxFrameSize  ���֡��С,����Ԥ����ռ�.
                  dwNetBand       ����ƽ��һ���ӷ��͵�����������һ��
                                  ����ϵ����1.5,���ڼ���ƽ��ÿ��Ӧ��
                                  ���͵�����������λ��bit
                  byFrameRate     ֡�ʣ�һ���ӷ��͵����ݴ�����
                  byMediaType     ý������
    ����ֵ˵���� �μ������붨��
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

    FreeBuf(); //������ж���

    if(0 == byFrameRate)
    {
        byFrameRate = 25;
    }

    m_byBuffMultipleTimes = dwMaxFrameSize / MAX_VIDEO_FRAME_SIZE;
    if(!m_byBuffMultipleTimes) // if its 0,set to 1.
        m_byBuffMultipleTimes = 1;
	
	//��NAT ̽��ṹ������ڴ�
	if(NULL == m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
	{
		m_tNatProbeProp.tRtpNatProbePack.pbyBuf = (u8 *)malloc(sizeof(u8) * MAX_SND_PACK_SIZE);
	}

	if(NULL == m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
	{
		m_tNatProbeProp.tRtcpNatProbePack.pbyBuf = (u8 *)malloc(sizeof(u8) * MAX_SND_PACK_SIZE);
	}


	//����rtp����
    m_pcRtp = new CKdvRtp;
    if(m_pcRtp == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    //����rtcp����
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

    //����mp4��mp2��h.264��h.261��h.263 h.263+ ��Ƶ����
    //���� ��Ҫ���뻷�λ�����ٽ��з��ͣ��Ա������������
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

    //���ú�RTP��Ӧ��RTCP��
    m_pcRtp->SetRtcp(m_pcRtcp);

    //���ú�RTCP��Ӧ��RTP��
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
    //h.263+ ��Ϊ�����h.263�������д���
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

    //С��2M�ĳ���3������2M�ĳ���2
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
    //���� ��������/֡�� ����ÿ������Ͱ���;
    s32  nAveFrame = (s32)(nSendRate/(byFrameRate*8));
    m_nMaxSendNum  = (nAveFrame + g_dwMaxExtendPackSize - 1) / g_dwMaxExtendPackSize;
    //���Ӧ�������λ���洢�İ�����
    if(m_nMaxSendNum > LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes)
    {
        m_nMaxSendNum = LOOP_BUF_UINT_NUM*m_byBuffMultipleTimes;
    }

    // ��Ƶ�ı�������ͳһ���� MAX_AUDIO_BITRATE ����
    u32 dwMediaBand = MAX_AUDIO_BITRATE; //Kbps
    // �ϲ������Ƶ�ı������ʣ��ڻ���ɷ�������ʱ������ϵ�����������ｫ�������ȡ��������
    if( TRUE == bVidPayload )
    {
        dwMediaBand = dwNetBand/1024; //Kbps
    }
    m_bVidPayload    = bVidPayload;
    m_dwCurBandWidth = dwMediaBand;

    //���� ��������/֡�� ����ÿ��������ֽ���;
    m_nMaxSendSize = nAveFrame;
    //���Ӧ�������λ���洢���ֽڳߴ�����
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
    ������        ��SetSndInfo
    ����        ���������緢�ʹ����֡�ʲ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  dwNetBand     �������
                  byFrameRate   ֡��

    ����ֵ˵���� �μ������붨��
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

    //���� ��������/֡�� ����ÿ������Ͱ���;
	if (0 == byFrameRate)
	{
		byFrameRate = 25;
	}
	s32  nAveFrame = (s32)(nSendRate / (byFrameRate * 8));
    m_nMaxSendNum  = (nAveFrame +  g_dwMaxExtendPackSize - 1) / g_dwMaxExtendPackSize;
    //���Ӧ�������λ���洢�İ�����
    if(m_nMaxSendNum > LOOP_BUF_UINT_NUM)
    {
        m_nMaxSendNum = LOOP_BUF_UINT_NUM;
    }

    // ��Ƶ�ı�������ͳһ���� MAX_AUDIO_BITRATE ����
    u32 dwMediaBand = MAX_AUDIO_BITRATE; //Kbps
    // �ϲ������Ƶ�ı������ʣ��ڻ���ɷ�������ʱ������ϵ�����������ｫ�������ȡ��������
    if( TRUE == m_bVidPayload )
    {
        dwMediaBand = dwNetBand/1024;   //Kbps
    }

    // ���Ͷ�ݵ�ý����������ƽ�����ʣ��Ƿ��б仯��
    // ����֪ͨ��������������Ӧ���ʹ��� 2005-01-18
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

    //���� ��������/֡�� ����ÿ��������ֽ���;
    m_nMaxSendSize = nAveFrame;
    //���Ӧ�������λ���洢���ֽڳߴ�����
    if(m_nMaxSendSize > LOOP_BUF_UINT_NUM*MAX_SND_ENCRYPT_PACK_SIZE)
    {
        m_nMaxSendSize = LOOP_BUF_UINT_NUM*MAX_SND_ENCRYPT_PACK_SIZE;
    }

    m_byFrameRate = byFrameRate;
    m_tKdvSndStatus.m_dwNetBand      = dwNetBand;
    m_tKdvSndStatus.m_dwMaxFrameSize = m_dwMaxFrameSize;
    m_tKdvSndStatus.m_byFrameRate    = m_byFrameRate;
    m_tSelfFrmHdr.m_byFrameRate         = m_byFrameRate;

    //�����ش����ͻ���ʱ�䳤�ȣ��ش�����������֡�ʣ���Ҫ�������
    if(TRUE == m_bRepeatSend)
    {
        ResetRSFlag(m_wBufTimeSpan, TRUE);
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetNetSndParam
    ����        ���������緢�Ͳ���(���еײ��׽��ӵĴ������󶨶˿�,�Լ�����Ŀ���ַ���趨�ȶ���)
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  tNetSndParam  �������,�μ��ṹ���岿��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SetNetSndParam ( TNetSndParam tNetSndParam )
{
    CREATE_CHECK    //�Ƿ񴴽�

    if(tNetSndParam.m_byNum > MAX_NETSND_DEST_NUM)
    {
        return ERROR_SND_PARAM;
    }

    u16 wRet = MEDIANET_NO_ERROR;


    //����RTP���ص�ַ
    wRet= m_pcRtp->SetLocalAddr(tNetSndParam.m_tLocalNet.m_dwRTPAddr,
                                tNetSndParam.m_tLocalNet.m_wRTPPort,
                                FALSE,
                                tNetSndParam.m_tLocalNet.m_dwRtpUserDataLen,
                                tNetSndParam.m_tLocalNet.m_abyRtpUserData);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //����RTP���͵�ַ��
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

        // ����ʵ��Ͷ��Ŀ���ַʱӦ���� ���ؼ���Ч��ַ
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

    //����RTCP���ص�ַ
    wRet= m_pcRtcp->SetLocalAddr(tNetSndParam.m_tLocalNet.m_dwRTCPAddr,
                                 tNetSndParam.m_tLocalNet.m_wRTCPPort,
                                 tNetSndParam.m_tLocalNet.m_dwRtcpUserDataLen,
                                 tNetSndParam.m_tLocalNet.m_abyRtcpUserData);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //����RTCP���͵�ַ��
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

    // ���Ͷ�ݵ�Ŀ���ַ�Ƿ��б仯��
    // ����֪ͨ��������������Ӧ���ʹ��� 2005-01-18
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
    CREATE_CHECK    //�Ƿ񴴽�

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
    ������        RemoveNetSndLocalParam
    ����        ���Ƴ����緢�ͱ��ص�ַ����(���еײ��׽��ӵ�ɾ�����ͷŶ˿ڵȶ���)
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  tNetSndParam  �������,�μ��ṹ���岿��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::RemoveNetSndLocalParam ()
{
    CREATE_CHECK    //�Ƿ񴴽�

    m_pcRtp->RemoveLocalAddr();

    m_pcRtcp->RemoveLocalAddr();

    // ֪ͨ���������ͷ���Ӧ���ʹ��� 2005-01-18
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
    CREATE_CHECK    //�Ƿ񴴽�
    m_pcRtp->SetSrcAddr(tSrcNet.m_dwRTPAddr, tSrcNet.m_wRTPPort);
    m_pcRtcp->SetSrcAddr(tSrcNet.m_dwRTCPAddr, tSrcNet.m_wRTCPPort);
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetActivePT
    ����        ������ ��̬�غɵ� Playloadֵ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����byLocalActivePT ���˷��͵Ķ�̬�غ�PTֵ, �ɶԺ�ʱԼ��
                  0-��ʾ��� ���˶�̬�غɱ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SetActivePT( u8 byLocalActivePT )
{
    m_byLocalActivePT = byLocalActivePT;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetEncryptKey
    ����        �����ü���key�ִ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pszKeyBuf      ����key�ִ�����ָ��
                    wKeySize       ����key�ִ����峤��
                    byEncryptMode  ����ģʽ Aes ���� Des

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SetEncryptKey( s8 *pszKeyBuf, u16 wKeySize, u8 byEncryptMode )
{
    if(NULL == pszKeyBuf)
    {
        //ȡ������
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
            //�����µĽ��ܼ�ֵ
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
            //�����µĽ��ܼ�ֵ
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
    ������        ��IncreaseTimestamp
    ����        ��ʱ�������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  pFrmHdr ֡����
                  bAvalid �Ƿ���ʵ�ʷ������ݳ���Ϊ������λ������Ƶ���ã�
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::IncreaseTimestamp(PFRAMEHDR pFrmHdr, BOOL32 bAvalid)
{
    CREATE_CHECK    //�Ƿ񴴽�

    u16 wRet = MEDIANET_NO_ERROR;
    if (TRUE == m_bFirstFrame)
    {
        m_dwFirstTimeStamps = pFrmHdr->m_dwTimeStamp;
        m_bFirstFrame = FALSE;
    }

    //ΪʲôH264��ʱ�����3600��MP4��ʱ�����40��
    //��׼H��Ƶϵ�е�ʱ�����¼����Ϊ90kHz������ÿ������3600������ý�����͵�ʱ�����¼����Ϊ1ms������ÿ������40
    switch(pFrmHdr->m_byMediaType)
    {
        case MEDIA_TYPE_MJPEG:
            {  //����Ҫ��
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
                    if (pFrmHdr->m_dwDataSize > 32)  /*ADPCMͷ��32�ֽ�*/
                    {
                        m_dwTimeStamp += 2 * (pFrmHdr->m_dwDataSize - 32);
                    }
                    else
                    {
                        m_dwTimeStamp += 32*8; //��Ĭ��32ms*8K����
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
                    m_dwTimeStamp += 320; /*ʱ��20ms��ʱ���������polycom��ʱ�̶�Ϊ320*/
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
		//ʱ��δ������dwnowts ��Ϊ0��ֱ��return��
		return MEDIANET_NO_ERROR;
	}

	 // ���� rtp ��rtcp �������
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

	//��natlastts ��ֵ��
	m_dwNatLastTs = OspTickGet();	
	
	return wRet;
}

u16 CKdvNetSnd::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
	if(ptNatProbeProp->tRtpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtpNatProbePack.pbyBuf == NULL ||
		ptNatProbeProp->tRtcpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtcpNatProbePack.pbyBuf == NULL)
	{
		return ERROR_NET_RCV_PARAM; //���ò�������
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

		m_tNatProbeProp.dwInterval = (ptNatProbeProp->dwInterval) * 1000; //ת��Ϊms �洢 , 0s �򲻷���

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
	������		��ResetFrameId
    ����        ��������������֡��ʶ��ʹ���ն�����ȷ���֡��������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::ResetFrameId()
{
    CREATE_CHECK //�Ƿ񴴽�
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��ResetSSRC
    ����        ����������ͬ��Դ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::ResetSSRC(u32 dwSSRC /*= 0*/)
{
    CREATE_CHECK //�Ƿ񴴽�

    if(0 == dwSSRC)
    {
        dwSSRC = GetExclusiveSSRC();
    }

    m_pcRtp->ResetSSRC(dwSSRC);

    m_pcRtcp->ResetSSRC(dwSSRC);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��ResetRSFlag
    ����        �����ö���mpeg4����H.264���õ��ش�����Ŀ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����bRepeatSnd  �Ƿ��ش�
                  wBufTimeSpan �ش����͵Ļ������Ļ���ʱ�䳤��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd /*=TRUE*/)
{
    CREATE_CHECK //�Ƿ񴴽�

    u16 wRet = MEDIANET_NO_ERROR;
    u16 wRLBUnitNum;
    u16 wScale = 1; //�Ŵ����

    if (wBufTimeSpan > 3000)
    {
        OspPrintf(TRUE, FALSE, "[WARN] ResetRSFlag Timespan(%d) too long.\n", wBufTimeSpan);
    }

    if (!m_bVidPayload) //��Ƶ
    {
        //��Ƶ��20ms����
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

    //�������֡��512k֡�Ĵ�С�����б����͵ķŴ�
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
    ������        ��EncryptRtpData
    ����        �����ݼ������ý�����Ӧ�ļ��ܴ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����
                   bUseNewBuf   �Ƿ�ʹ���»���, ����H261/H263 ����ebit�İ�����Ӧ��ʹ���»���

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::EncryptRtpData(TRtpPack *pRtpPack, BOOL32 bUseNewBuf/*=FALSE*/)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //a. �ж��Ƿ������˼���
    if( NULL == m_pszMaterialBuf )
    {
        //û�����ü�����ֱ�ӷ��سɹ�
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

    //b. �������ܲ�������
    if( AES_ENCRYPT_MODE == m_byEncryptMode )
    {

        //1. �������ܵ������ܳ���, �ж��Ƿ���ڶ�������, ��¼ tRtpPack.m_byPadNum
        //   ������ڶ������⣬�����㲹���������������tRtpPack.m_byPadNum�ֽڣ�ͬʱ�ܳ���Ӧ (+tRtpPack.m_byPadNum)
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

        //2. ��������

        // ��ʼ key �� ���к���ʱ������  SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
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
        //1. �������ܵ������ܳ���, �ж��Ƿ���ڶ�������, ��¼ tRtpPack.m_byPadNum
        //   ������ڶ������⣬�����㲹���������������tRtpPack.m_byPadNum�ֽڣ�ͬʱ�ܳ���Ӧ (+tRtpPack.m_byPadNum)
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

        //2. ��������

        // ��ʼ key �� ���к���ʱ������  SSTTTTSS (S-Sequence, T-TimeStamp)
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

    //c. ���м��ܺ���ܳ����ж�, Ӧ�������� MAX_SND_ENCRYPT_PACK_SIZE, ��ע���ж�
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
    //d. �����ڼ��ܺ��������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    pRtpPack->m_byPayload = m_byLocalActivePT;
*/

    return wRet;
}

/*=============================================================================
    ������        ��Send
    ����        ���������ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����
                   bAvalid  ����֡�Ƿ���Ч

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::Send(PFRAMEHDR pFrmHdr, BOOL32 bAvalid, BOOL32 bSendRtp)
{
    CREATE_CHECK //�Ƿ񴴽�

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
        //��create snd����ʱ��psinifo��Ϣֻ������Ƶ������Ƶ�ڴ����䣬��֮��Ȼ��
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

        //��������pt������Ӧ���͵�������,return ������
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
        //hual 2005-07-25 ��Mpeg 4, ��Ϊ���Ϳ�֡
        if (2 == g_nShowDebugInfo && 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME. \n");
        }

        if (NULL == pFrmHdr)
        {
            OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME Must set frame Header\n");
            return ERROR_SND_PARAM;
        }

        //��Mpeg4��ԭ�ȵĴ�����У�����ʱ����������п�֡�ķ��ͣ�
        if ((MEDIA_TYPE_MP4 != pFrmHdr->m_byMediaType) && (MEDIA_TYPE_H262 != pFrmHdr->m_byMediaType)
            && (MEDIA_TYPE_MJPEG != pFrmHdr->m_byMediaType))
        {
            IncreaseTimestamp(pFrmHdr, FALSE);
            //����������ֽ��� �����������ݰ�
            return m_pcRtp->SendBySize(m_nMaxSendSize);
        }
        else
        {
            //hual ��ȫ���жϣ���֤�ϲ������˳���Ϊ0
            if (0 != pFrmHdr->m_dwDataSize)
            {
                OspPrintf(1, 0, "[CKdvNetSnd::Send] SEND NULL FRAME Must set frame len equal 0. \n");
                return ERROR_SND_PARAM;
            }
        }

    }

        //������Ч���жϣ�pFrmHdr->m_dwDataSizeΪ�㣬��ʾ���Ϳհ�
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

    //֡ID�ۼ�
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwFrameNum++;

    //ʱ����ۼ�
    IncreaseTimestamp(pFrmHdr, TRUE);

    //���ݲ�ͬ��ý����������ͬ�Ĵ���Ŀǰֻ�����ַ��ͷ�ʽ:
    //1.��׼���ķ��ͣ���h261��h263��h264��G.711��G.722��G.723��G.728��H.224��H.239��T.120
    //2.��չ���ķ���, ��Mpeg4��mp3

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
    ������        ��SelfTestSend
    ����        ���Բⷢ�����ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SelfTestSend(s32 nFrameLen, s32 nSndNum, s32 nSpan)
{
    CREATE_CHECK //�Ƿ񴴽�

    u16 wRet = MEDIANET_NO_ERROR;

    if(FALSE == g_bSelfSnd)
    {
        return wRet;
    }

    //������Ч���жϣ�pFrmHdr->m_dwDataSizeΪ�㣬��ʾ���Ϳհ�
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
        //֡ID�ۼ�
        m_dwFrameId++;
        m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
        m_tKdvSndStatistics.m_dwFrameNum++;

        //ʱ����ۼ�
        IncreaseTimestamp(&m_tSelfFrmHdr, TRUE);

        dwTimeStamp = OspTickGet();

        //���ݲ�ͬ��ý����������ͬ�Ĵ���Ŀǰֻ�����ַ��ͷ�ʽ:
        //1.��׼���ķ��ͣ���h261��h263��h264��G.711��G.722��G.723��G.728��H.224��H.239��T.120
        //2.��չ���ķ���, ��Mpeg4��mp3

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
    ������        ��DealRtcpTimer
    ����        ��rtcp��ʱrtcp���ϱ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::DealRtcpTimer()
{
    CREATE_CHECK //�Ƿ񴴽�

    return m_pcRtcp->DealTimer();
}

/*=============================================================================
    ������        ��Send
    ����        ��ֱ�ӷ���RTP���ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   bTrans   �Ƿ���͸��ת������������кż�ʱ�������
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::Send(TRtpPack *pRtpPack, BOOL32 bTrans, BOOL32 bAvalid, BOOL32 bSendRtp)
{
    CREATE_CHECK //�Ƿ񴴽�

    u16 wRet = MEDIANET_NO_ERROR;

    if(FALSE == bAvalid)
    {
        //���ı�ԭ�е�ʱ��������ⲿָ��
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

    //֡ID�ۼ�
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwPackSendNum++;

    if(TRUE == bTrans)
    {
        m_tOldRtp = *pRtpPack;
        //���Ԥ����Ŀռ䣬��ֹ͸��ת��ʱ�۸�rtp������
        m_tOldRtp.m_nPreBufSize = 0;
        wRet = m_pcRtp->Write(m_tOldRtp, TRUE, TRUE, bSendRtp);
    }
    else
    {
        //���ı�ԭ�е�ʱ��������ⲿָ��
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
    ������        ��SendExPack
    ����        ��������չ��RTP�����Ƚ�һ֡�����гɺܶ��С�������һ��
                  ��������Ϣ�а���֡��ȫ����Ϣ������������չ��Ϣ��ֻ����
                  �ð���һ֡�����е�λ�á�
                  ��չ��Ϣ�ṹ����:u8 TotalNum + u8 PackIndex + u8 FrameMode +
                  u8 FrameRate + u32 FrameID + u32 Width +
                  u32 Height

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendExPack(PFRAMEHDR pFrmHdr,BOOL32 bAudio)
{
    u16 wRet         = MEDIANET_NO_ERROR;
    s32 nRealPackLen = g_dwMaxExtendPackSize;
    s32 nPackNum     = (pFrmHdr->m_dwDataSize  + nRealPackLen - 1) / nRealPackLen;

    //ʼ�ջ���һ������
    if(0 == nPackNum)
    {
        nPackNum = 1;
    }

    if(nPackNum > 0xff) //���ĸ������ܳ������ֽ����ܱ���ֵ
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_SND_FRAME;
    }

    //����CKdvRtp���RTP���ݰ���Ϣ
    TRtpPack  tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp    = m_dwTimeStamp;
    tRtpPack.m_byExtence    = 1;//��չ��
    tRtpPack.m_nRealSize    = nRealPackLen;
    tRtpPack.m_nExSize        = 1;//��λΪsizeof(u32)���ȡ�
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    s32 nPackIndex;

    //ѭ������n-1��
    for(nPackIndex=1; nPackIndex<nPackNum; nPackIndex++)
    {
        tRtpPack.m_nRealSize = nRealPackLen;
        //��Ч����
        tRtpPack.m_pRealData = pFrmHdr->m_pData + (nPackIndex-1)*nRealPackLen;
        //��չ����
        //�ܰ���
        m_byExBuf[EX_TOTALNUM_POS] = (u8)nPackNum;
        //�����
        m_byExBuf[EX_INDEX_POS]    = (u8)nPackIndex;
        tRtpPack.m_pExData           = m_byExBuf;


        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //����ʧ��
            m_tKdvSndStatistics.m_dwFrameLoseNum++;

            if(FALSE == bAudio)
            {
                //����������ֽ��� �����������ݰ�
                m_pcRtp->SendBySize(m_nMaxSendSize);
            }
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //д��һ������һ��ֱ�ӷ��ͣ�ȡ���ڵڶ�������
        wRet = m_pcRtp->Write(tRtpPack, bAudio, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //������
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            if(FALSE == bAudio)
            {
                //����������ֽ��� �����������ݰ�
                m_pcRtp->SendBySize(m_nMaxSendSize);
            }
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //ͳ��
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    /*���һ���Ĵ�����ͬ��ǰn-1���������һ�������֡��Ϣ����֡ID��
      �Ƿ�ؼ�֡����Ƶ֡��ߡ�
    */
    s32 nLastPackLen = pFrmHdr->m_dwDataSize - nRealPackLen * (nPackNum - 1);

    tRtpPack.m_byMark            = 1;
    tRtpPack.m_nRealSize        = nLastPackLen;
    tRtpPack.m_pRealData        = pFrmHdr->m_pData + (nPackNum - 1) * nRealPackLen;
    //��Ƶ֡
    if(bAudio)
    {
        tRtpPack.m_nExSize            = 1;
        //�ܰ���
        m_byExBuf[EX_TOTALNUM_POS]  = (u8)nPackNum;
        //�����
        m_byExBuf[EX_INDEX_POS]        = (u8)nPackIndex;
        //��Ƶģʽ
        m_byExBuf[EX_FRAMEMODE_POS]    = (u8)pFrmHdr->m_byAudioMode;
        tRtpPack.m_pExData          = m_byExBuf;

        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //����ʧ��
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }
        //��Ƶֱ֡�ӷ���
        wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //����ʧ��
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }

        //ͳ��
        m_tKdvSndStatistics.m_dwPackSendNum++;

        return wRet;
    }
    else
    {
        //��Ƶ֡����
        tRtpPack.m_nExSize            = MAX_PACK_EX_LEN / sizeof(u32);//
        //�ܰ���
        m_byExBuf[EX_TOTALNUM_POS]  = (u8)nPackNum;
        //�����
        m_byExBuf[EX_INDEX_POS]        = (u8)nPackIndex;
        //�Ƿ�ؼ�֡
        m_byExBuf[EX_FRAMEMODE_POS] = (u8)pFrmHdr->m_tVideoParam.m_bKeyFrame;
        //֡��
        m_byExBuf[EX_FRAMERATE_POS]    = (u8)m_byFrameRate;
        //֡ID
        *((u32 *)(&(m_byExBuf[EX_FRAMEID_POS]))) = htonl(m_dwFrameId) ;
        //��Ƶ֡��
        *((u16 *)(&(m_byExBuf[EX_WIDTH_POS])))   =
                                       htons(pFrmHdr->m_tVideoParam.m_wVideoWidth);
        //��Ƶ֡��
        *((u16 *)(&(m_byExBuf[EX_HEIGHT_POS])))  =
                                      htons(pFrmHdr->m_tVideoParam.m_wVideoHeight);

        tRtpPack.m_pExData          = m_byExBuf;

        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //����ʧ��
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            //����������ֽ��� �����������ݰ�
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();
            return wRet;
        }
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //������
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            //����������ֽ��� �����������ݰ�
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }

        //ͳ��
        m_tKdvSndStatistics.m_dwPackSendNum++;

        //����������ֽ��� �����������ݰ�
        wRet = m_pcRtp->SendBySize(m_nMaxSendSize);
    }

    return wRet;
}

/*=============================================================================
    ������        ��ParasH261Head
    ����        ��h.261 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
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
        //RTP����һ֡�е���ʼλ��
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwStartPos = *(pdwPos++);
        //RTP���ĳ���
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen  = *(pdwPos++);

        nPackLen = m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen + nHeadLen;

        // ����հ�
        if( (nPackLen < 0) ||
            (nPackLen > MAX_SND_PACK_SIZE) )
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            OspPrintf(TRUE, FALSE, "H261 pack too long. PackLen=%d\n", nPackLen);
            return ERROR_H263_PACK_TOOMUCH;
        }

        {
            //sbit ebit У��
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

        //����RTP���ĳ��� (����playloadͷ����)
        m_ptH261HeaderList->m_tkdvH261Header[nIndex].m_dwPackLen = nPackLen;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SendOneH261Pack
    ����        �����ͱ�׼�� һ�� h.261 RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����
                   pPackBuf  �����ݻ���
                   nPos      ������

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendOneH261Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//����չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�з�����֡,��������֡����H261header, �ض���MAX_H261_HEADER_LENָ���Ŀռ䡣
    tRtpPack.m_nPreBufSize  = MAX_H261_HEADER_LEN;

    //last pack
    if(nPos == (m_ptH261HeaderList->m_nNum-1))
    {
        tRtpPack.m_byMark     = 1; //���һ���ı�ʶλ
    }

    //д�������
    tRtpPack.m_nRealSize = m_ptH261HeaderList->m_tkdvH261Header[nPos].m_dwPackLen;

    //���ϰ�ͷ
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

    //�������ܲ���
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        return wRet;
    }

    //h.261 Ҳ���뻺�巢��
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    ������        ��SendH261Pack
    ����        �����ͱ�׼�� h.261 RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendH261Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //rtp ͷ��Ϣ�������������Է���������¼
    wRet = ParasH261Head(pFrmHdr);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //�õ���Ч���ݵ���ʼλ��
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

            //����������ֽ��� �����������ݰ�
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    //����������ֽ��� ��ʵ�������ݰ�
    return m_pcRtp->SendBySize(m_nMaxSendSize);//��������
}

/*=============================================================================
    ������        ��ParasH263Head
    ����        ��h.263/h.263+ RTP head info ���� h.263+ ��Ϊ�����h.263�������д���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
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
        // RTPͷ
        // 0 - A MODE ; 1 - B MODE ; 2 - C MODE ; 3 - D MODE-H.263+
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwMode     = *(pdwPos++);
        // Head Info (A MODE - 4�ֽڣ� B MODE - 8�ֽڣ� C MODE - 12�ֽڣ� D MODE - 2�ֽ�)
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwHeader1  = *(pdwPos++);
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwHeader2  = *(pdwPos++);
        //RTP����һ֡�е���ʼλ��
        m_ptH263HeaderList->m_tKdvH263Header[nIndex].m_dwStartPos = *(pdwPos++);
        //RTP���ĳ���
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
        //����Լ��MODE_H263_D��Ϊh.263+�� ��Ϊ�����h.263�������д���
        else if(MODE_H263_D == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
        {
            nHeadLen = sizeof(u16);
        }

        //����ÿһ����rtp����
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
            //sbit ebit У��
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

        // ����հ�
        if( (nPackLen < 0) ||
            (nPackLen > MAX_SND_PACK_SIZE) )
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            OspPrintf(TRUE, FALSE, "H263 pack too long. PackLen=%d\n", nPackLen);
            return ERROR_H263_PACK_TOOMUCH;
        }

        //����RTP���ĳ��� (����playloadͷ����)
        m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwDataLen = nPackLen;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��ParasH263PlusHead
    ����        ��h.263+ RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   wH263PlusHeader  h.263+ payload format
                   ptH263PlusHeader ���ṹָ��

    ����ֵ˵���� ��
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

    //����λ��Ӧ��Ϊ0�������������ͷ��Ϣ����
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
    ������        ��SendOneH263Pack
    ����        �����ͱ�׼�� һ�� h.263/h.263+ RTP�� h.263+ ��Ϊ�����h.263�������д���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����
                   pPackBuf  �����ݻ���
                   nPos      ������

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendOneH263Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//����չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;

    //�з�����֡,��������֡����H263header, �ض���MAX_H263_HEADER_LENָ���Ŀռ䡣
    tRtpPack.m_nPreBufSize  = MAX_H263_HEADER_LEN;

    //last pack
    if(nPos == (m_ptH263HeaderList->m_nNum-1))
    {
        tRtpPack.m_byMark     = 1; //���һ���ı�ʶλ
    }

    //д�������
    tRtpPack.m_nRealSize = m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwDataLen;

    //���ϰ�ͷ
    //h263 RTP Header MODE A
    s32 nHeadLen = sizeof(u32);
    //h263 RTP Header MODE B
    if(MODE_H263_B == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
    {
        nHeadLen = 2*sizeof(u32);
    }

    //����H263 ����ebit�İ�����Ӧ��ʹ���»���
    BOOL32 bUseNewBuf = FALSE;

    //����Լ��MODE_H263_D��Ϊh.263+�� ��Ϊ�����h.263�������д���
    if(MODE_H263_D == m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwMode)
    {
        //�Ƿ���� PSC/GSC�� ��Ҫ�ڴ���ʱ�ĺ��Ե����ֽ�,������ڣ����Ⱥ���2�ֽڵ�0
        u16   wHead = (u16)GetBitField(m_ptH263HeaderList->m_tKdvH263Header[nPos].m_dwHeader1, 0, 16);
        TH263PlusHeader tH263PlusHeader;
        memset(&tH263PlusHeader, 0, sizeof(tH263PlusHeader));
        if(FALSE == ParasH263PlusHead(wHead, &tH263PlusHeader))
        {
            //h.263+ payload format ����
            wRet = ERROR_H263_PACK_TOOMUCH;
            return wRet;
        }
        if( (1 == tH263PlusHeader.pBit) && (0 == *((u16*)pPackBuf)) &&
            (tRtpPack.m_nRealSize > 2) )
        {
            pPackBuf += 2; //�ж�pebit��ȥ�������ֽڵ�0
            tRtpPack.m_nRealSize -= 2;
        }

        nHeadLen = sizeof(u16);
        tRtpPack.m_byPayload = MEDIA_TYPE_H263PLUS; //���Ϊh.263+��̬�غ�����
        tRtpPack.m_pRealData = pPackBuf - nHeadLen;
        wHead = htons(wHead);
        *((u16*)tRtpPack.m_pRealData) = wHead;
    }
    else
    {
        //���ϰ�ͷ
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

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        return wRet;
    }

    //h.263 Ҳ���뻺�巢��
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    ������        ��SendH263Pack
    ����        �����ͱ�׼�� h.263/h.263+ RTP�� h.263+ ��Ϊ�����h.263�������д���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendH263Pack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //rtp ͷ��Ϣ�������������Է���������¼
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

    //�õ���Ч���ݵ���ʼλ��
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

            //����������ֽ��� �����������ݰ�
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    //����������ֽ��� ��ʵ�������ݰ�
    return m_pcRtp->SendBySize(m_nMaxSendSize);//��������
}

/*=============================================================================
    ������        ��DecodeH264SPS
    ����        ������ h.264 �����е� sps ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� TRUE - �ɹ�
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
    ������        ��DecodeH264PPS
    ����        ������ h.264 �����е� pps ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� TRUE - �ɹ�
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
    ������        ��ParseH264RtpHead
    ����        ��Std h.264 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPackBuf     RTP����ָ��
                   pdwHeaderLen    ���ݰ���ͷ���� *pdwHeaderLen - 0 ��Ч��ͷ
                   ptH264Header    ͷ�ṹָ��

    ����ֵ˵���� ��
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

    u32 dwNaluType = byNaluTypeOctet & 0x1F;  // ȡ�õ�5λ
    TBitStream tBitStream;
    memset(&tBitStream, 0, sizeof(tBitStream));
    // TBitStream�ṹ��RTP���ĵ�14�ֽڿ�ʼ��ǰ12�ֽ�ΪRTPͷ����13�ֽ�ΪNalu���ͣ�
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
    ������        ��ParseH264Head
    ����        ��std h.264 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::ParseH264Head(PFRAMEHDR pFrmHdr)
{
    u32 *pdwPos = (u32 *)pFrmHdr->m_pData;
    memset(m_ptH264HeaderList, 0, sizeof(TH264HeaderList));

    //��������������ֽ�lengthΪ����λ���ֽڡ���
    //WIN32�洢ҲΪ����λ���ֽڡ������跭ת
    //VXWORKS�洢ҲΪ����λ���ֽڡ����跭ת

    u32 dwIndex = 0;
    u32 dwPackLen = 0;

    //�ж�оƬ�ֽ�����
    if (0x1234 == ntohs(0x1234))
    {   //big endian
        while((dwIndex < MAX_NALU_NUM) &&
            ((dwPackLen = pdwPos[dwIndex]) != 0))
        {
            //�ײ㴫���ÿ��������little endian������ת��
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
    ������        ��SendH264Pack
    ����        �����ͱ�׼�� һ�� h.264 RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr   ����֡��Ϣ���μ��ṹ����
                   pPackBuf  �����ݻ���
                   nPos      ������

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack(PFRAMEHDR pFrmHdr, u8 *pPackBuf, s32 nPos)
{
    // 264patch for 8180
    if (m_ptH264HeaderList->m_dwH264NaluArr[nPos] > g_dwMaxExtendPackSize)
    {
        //��FU���ֱ���
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

    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;//�������һ��
    tRtpPack.m_byExtence    = 1;//��չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);

    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    *((u32 *)(&(m_byExBuf[0])))=((pFrmHdr->m_tVideoParam.m_bKeyFrame<<24)&0xFF000000)|((pFrmHdr->m_tVideoParam.m_wVideoWidth<<12)&0x00FFF000)|(pFrmHdr->m_tVideoParam.m_wVideoHeight&0x00000FFF);
    //֡��
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    // ��������֡Nalu��������RTP���ݰ���ÿ��NaluΪһ��RTP����
    // ��������ǰMAX_NALU_UNIT_SIZE�ֽ�ΪNalu���ȵĿռ䡣
    //tRtpPack.m_nPreBufSize  = MAX_NALU_UNIT_SIZE;
    tRtpPack.m_nPreBufSize  = 0;

    //last pack
    if(nPos == (s32)(m_ptH264HeaderList->m_dwNaluNum - 1))
    {
        tRtpPack.m_byMark = 1; //���һ���ı�ʶλ
    }

    // д�������
    tRtpPack.m_nRealSize  = m_ptH264HeaderList->m_dwH264NaluArr[nPos];

    tRtpPack.m_pRealData  = pPackBuf;

    //test later del  -- h.264 RTP head info ����
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

    //�������ܲ���
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        return wRet;
    }

    //���뻺�巢��
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    ������        ��SendH264Pack
    ����        �����ͱ�׼�� h.264 RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
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
    //����0001�Լ�001��ϵ������������������001�ķָʽ��sony������0001��001��ϵ������
    if ((pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 0 )||
        (pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 1 ))
    {
        //Ŀǰ���ն��Ѿ�ȫ������0001������֡��ʽ����˷���Ҳȫ������0001��ʽ
         return SendH264Pack_Flag0001(pFrmHdr);
    }
    else
    {
        //rtp ͷ��Ϣ�������������Է���������¼
        wRet = ParseH264Head(pFrmHdr);
        if(KDVFAILED(wRet))
        {
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->ResetSeq();
            return wRet;
        }

        //�õ���Ч���ݵ���ʼλ��
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

                //����������ֽ��� �����������ݰ�
                m_pcRtp->SendBySize(m_nMaxSendSize);
                m_pcRtp->ResetSeq();

                return wRet;
            }

            pBuf += m_ptH264HeaderList->m_dwH264NaluArr[nPos];

            m_tKdvSndStatistics.m_dwPackSendNum++;
        }

        //����������ֽ��� ��ʵ�������ݰ�
        return m_pcRtp->SendBySize(m_nMaxSendSize);//��������
    }
}

/*=============================================================================
    ������        ��SendH264Pack_Flag0001
    ����        ����0001��ʽ����H264rtp��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendH264Pack_Flag0001(PFRAMEHDR pFrmHdr)//����0001�Լ�001���ģʽ
{
    u8 *pbyData = pFrmHdr->m_pData;
    u32 dwFrameLen = pFrmHdr->m_dwDataSize;
    u32 i;
    u32 dwNaluLen = 0;
    u8 *pbyNaluData = NULL;
    u8 byInterval = 4;//Ĭ��Ϊ0001
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
            //������չͷ�Ļ���һ����С���ܳ���MAX_EXTEND_PACK_SIZE
            if (dwNaluLen > g_dwMaxExtendPackSize)
            {
                //��FU���ֱ���
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

    //����������ֽ��� ��ʵ�������ݰ�
    return m_pcRtp->SendBySize(m_nMaxSendSize);//��������
}

/*=============================================================================
    ������        ��SendOneH264Pack_LongSlice
    ����        ������Ƭ������H264rtp��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����
                   u8 *pbyNalu  һ��nalu
                   u32 dwNaluLen  nalu ����
                   BOOL32 bMark   ֡�߽��־
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pbyNalu,
                                          u32 dwNaluLen, BOOL32 bMark)
{
    u16 wRet = MEDIANET_NO_ERROR;
    //��ǰ��H264������չͷ������Ϊ��֧��30֡����Ҫ����12�ֽڵ���չͷ
    const s32 FU_MAX_LEN = g_dwMaxExtendPackSize - 2;
    // ��һ���ֽ���indicator������
    const s32 FU_NUM = (dwNaluLen - 1  + FU_MAX_LEN - 1) / FU_MAX_LEN;
    s32 i;
    u8 *pbyFuData;
    u8 abyFuPack[MAX_EXTEND_PACK_SIZE];
    s32 nFuSize;
    BOOL32 bLastFU = FALSE;

    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 1;//��չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);//Ϊ֧�ַ���30֡��������H264�����3���ֽڵ���չͷ

    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    *((u32 *)(&(m_byExBuf[0]))) = htonl(((pFrmHdr->m_tVideoParam.m_bKeyFrame<<24)&0xFF000000)|((pFrmHdr->m_tVideoParam.m_wVideoWidth<<12)&0x00FFF000)|(pFrmHdr->m_tVideoParam.m_wVideoHeight&0x00000FFF));
    //֡��
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack_LongSlice]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    // �����ú�indicator
    abyFuPack[0] = (pbyNalu[0] & 0xe0) | 28;//28��FU-A��type

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

        // ��ǰ�������ֽڼ���ȥ
        memcpy(abyFuPack + 2, pbyFuData, nFuSize);

        tRtpPack.m_pRealData    = abyFuPack;
        tRtpPack.m_nRealSize    = nFuSize + 2;
        tRtpPack.m_byMark       = (u8)(bMark & bLastFU);

        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            break;
        }

        //���뻺�巢��
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
    ������        ��SendOneH264Pack_Flag0001
    ����        ����0001��ʽ����H264rtp��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����
                   u8 *pbyNalu  һ��nalu
                   u32 dwNaluLen  nalu ����
                   BOOL32 bMark   ֡�߽��־
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendOneH264Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf,
                                         u32 dwNaluLen, BOOL32 bMark)
{
    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 1;//��չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = MAX_PACK_EX_LEN / sizeof(u32);

    tRtpPack.m_pRealData    = pPackBuf;
    tRtpPack.m_nRealSize    = dwNaluLen;
    tRtpPack.m_byMark       = (u8)bMark;

    //H264��չͷ��ʽ
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
    //֡��
    *((u32 *)(&(m_byExBuf[4]))) = htonl(pFrmHdr->m_byFrameRate);

    tRtpPack.m_pExData          = m_byExBuf;

    if (g_nShowDebugInfo == 110)
    {
        OspPrintf(TRUE, FALSE, "[SendOneH264Pack_Flag0001]tRtpPack.m_byExtence : %d, tRtpPack.m_nExSize : %d, pFrmHdr->m_byFrameRate :%d \n", tRtpPack.m_byExtence, tRtpPack.m_nExSize, pFrmHdr->m_byFrameRate);
    }
    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        return wRet;
    }

    //���뻺�巢��
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

/*=============================================================================
    ������        ��SendSdAudioPack
    ����        �����ͱ�׼����ƵRTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendSdAudioPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //һֱ֡�ӷ��ͣ������з�
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
    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//����չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 0;
    if (pFrmHdr->m_byMediaType == MEDIA_TYPE_AMR)
    {
        u8 byRtpData[MAX_SND_PACK_SIZE] = {0};
        byRtpData[0] = 0xf0;//CMR-15,4λ���
        memcpy(byRtpData+1, pFrmHdr->m_pData, pFrmHdr->m_dwDataSize);
        tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize+1;
        tRtpPack.m_pRealData    = byRtpData;
    }
    else
    {
        //�ϵ�G.711��׼�����ݿ�ǰ���ӱ�ʶλ��
        tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize;// + G711_REVERSE_BIT;
        tRtpPack.m_pRealData    = pFrmHdr->m_pData;// - G711_REVERSE_BIT;
    }


    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //��Ƶֱ֡�ӷ���
    wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }
    m_tKdvSndStatistics.m_dwPackSendNum++; //hual

    //��׼RTP��,ֱ�ӷ��Ͳ��������塣
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendSdAACAudioPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;
    u8 byExtence[12] = {0};
    //һֱ֡�ӷ��ͣ������з�
    if(pFrmHdr->m_dwDataSize > MAX_EXTEND_PACK_SIZE-4)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }

    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 1;//����չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 1;//AACLC��markλһ��Ҫ��1������vlc������������⣨������Ϊaaclc�Ǳ䳤��Ե�ʣ�

    if (TRUE == tRtpPack.m_byExtence)
    {
        byExtence[2] = pFrmHdr->m_byAudioMode;
        tRtpPack.m_pExData = byExtence;
        tRtpPack.m_nExSize      = 3;
    }

    if (!m_bAaclcNoHead)
    {
        //rfc3640�Ϲ涨��aaclc����rtp��ʽ����������Ҫ����4�ֽڵ�ͷ
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

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //��Ƶֱ֡�ӷ���
    wRet = m_pcRtp->Write(tRtpPack, TRUE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }
    m_tKdvSndStatistics.m_dwPackSendNum++; //hual

    //��׼RTP��,ֱ�ӷ��Ͳ��������塣
    return MEDIANET_NO_ERROR;
}
/*=============================================================================
    ������        ��SendDataPayloadPack
    ����        ���Ա�׼������ H.224/H.239/T.120 RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::SendDataPayloadPack(PFRAMEHDR pFrmHdr)
{
    u16 wRet = MEDIANET_NO_ERROR;

    //һֱ֡�ӷ��ͣ������з�
    if(pFrmHdr->m_dwDataSize > MAX_SND_PACK_SIZE)
    {
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        return ERROR_PACK_TOO_LEN;
    }

    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byExtence    = 0;//����չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;
    tRtpPack.m_nPreBufSize  = 0;//pFrmHdr->m_dwPreBufSize;
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_nRealSize    = pFrmHdr->m_dwDataSize;
    tRtpPack.m_pRealData    = pFrmHdr->m_pData;

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        m_pcRtp->ResetSeq();
        return wRet;
    }

    //����ֱ֡�ӷ���
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
    ������        ��SendSdPack
    ����        �����ͱ�׼��RTP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pFrmHdr  ����֡��Ϣ���μ��ṹ����

    ����ֵ˵���� �μ������붨��
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
    ������        ��GetStatus
    ����        ����ѯ����״̬
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvSndStatus  Ҫ���صķ���״̬���μ��ṹ���塣

    ����ֵ˵���� �μ������붨��
==============================================================================*/
u16 CKdvNetSnd::GetStatus(TKdvSndStatus &tKdvSndStatus)
{
    tKdvSndStatus = m_tKdvSndStatus;
     return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetStatistics
    ����        ����ѯ����ͳ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvSndStatistics  Ҫ���صķ���״̬���μ��ṹ���塣

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetSnd::GetStatistics(TKdvSndStatistics &tKdvSndStatistics)
{
    tKdvSndStatistics = m_tKdvSndStatistics;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetAdvancedSndInfo
    ����        ����ѯ���Ͷ˸߼����ò������ش����������ŵȣ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tAdvancedSndInfo  Ҫ���صķ���״̬���μ��ṹ���塣

    ����ֵ˵���� �μ������붨��
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
    CREATE_CHECK    //�Ƿ񴴽�
    m_pcRtp->GetSndSocketInfo(tRtpSockInfo);
    m_pcRtcp->GetSndSocketInfo(tRtcpSockInfo);
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetSnd::SendPSFrame(PFRAMEHDR pFrmHdr)
{
    CREATE_CHECK;    //�Ƿ񴴽�
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

    //֡ID�ۼ�
    m_dwFrameId++;
    m_tKdvSndStatus.m_dwFrameID = m_dwFrameId;
    m_tKdvSndStatistics.m_dwFrameNum++;

    //ʱ����ۼ�
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

    //ʼ�ջ���һ������
    if(0 == nPackNum)
    {
        nPackNum = 1;
    }

    //����CKdvRtp���RTP���ݰ���Ϣ
    TRtpPack  tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    memset(&m_byExBuf, 0, MAX_PACK_EX_LEN);
    tRtpPack.m_byMark        = 0;
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp    = m_dwTimeStamp;
    tRtpPack.m_byExtence    = 0;//��չ��
    tRtpPack.m_nRealSize    = nRealPackLen;
    tRtpPack.m_nExSize        = 0;
    tRtpPack.m_nPreBufSize  = pFrmHdr->m_dwPreBufSize;

    tRtpPack.m_byPayload = MEDIA_TYPE_PS;

    s32 nPackIndex = 0;

    //ѭ������n-1��
    for(nPackIndex=1; nPackIndex<nPackNum; nPackIndex++)
    {
        tRtpPack.m_nRealSize = nRealPackLen;
        //��Ч����
        tRtpPack.m_pRealData = pFrmHdr->m_pData + (nPackIndex-1)*nRealPackLen;

        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            //����ʧ��
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();

            return wRet;
        }
        //д��һ��
        wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
        if(KDVFAILED(wRet))
        {
            //������
            m_tKdvSndStatistics.m_dwFrameLoseNum++;
            m_pcRtp->SendBySize(m_nMaxSendSize);
            m_pcRtp->ResetSeq();
            return wRet;
        }
        //ͳ��
        m_tKdvSndStatistics.m_dwPackSendNum++;
    }

    /*���һ��*/
    s32 nLastPackLen = pFrmHdr->m_dwDataSize - nRealPackLen * (nPackNum - 1);

    tRtpPack.m_byMark            = 1;
    tRtpPack.m_nRealSize        = nLastPackLen;
    tRtpPack.m_pRealData        = pFrmHdr->m_pData + (nPackNum - 1) * nRealPackLen;

    //�������ܲ���
    wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        //����������ֽ��� �����������ݰ�
        m_pcRtp->SendBySize(m_nMaxSendSize);
        m_pcRtp->ResetSeq();

        return wRet;
    }
    wRet = m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
    if(KDVFAILED(wRet))
    {
        //������
        m_tKdvSndStatistics.m_dwFrameLoseNum++;
        //����������ֽ��� �����������ݰ�
        m_pcRtp->SendBySize(m_nMaxSendSize);
        m_pcRtp->ResetSeq();

        return wRet;
    }

    //ͳ��
    m_tKdvSndStatistics.m_dwPackSendNum++;

    //����������ֽ��� �����������ݰ�
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
    //����0001�Լ�001��ϵ������������������001�ķָʽ��sony������0001��001��ϵ������
    if ((pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 0 )||
        (pFrmHdr->m_pData[0] == 0 && pFrmHdr->m_pData[1] == 0 && pFrmHdr->m_pData[2] == 1 ))
    {
        //Ŀǰ���ն��Ѿ�ȫ������0001������֡��ʽ����˷���Ҳȫ������0001��ʽ
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
    u8 byInterval = 4;//Ĭ��Ϊ0001
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
            //������չͷ�Ļ���һ����С���ܳ���MAX_EXTEND_PACK_SIZE
            if (dwNaluLen > g_dwMaxExtendPackSize)
            {
                //��FU���ֱ���
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

    //����������ֽ��� ��ʵ�������ݰ�
    return m_pcRtp->SendBySize(m_nMaxSendSize);//��������
}

u16 CKdvNetSnd::SendOneH265Pack_Flag0001(PFRAMEHDR pFrmHdr, u8 *pPackBuf, u32 dwNaluLen, BOOL32 bMark)
{
    //�����ײ��RTP�ṹ
    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 0;//��չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;

    tRtpPack.m_pRealData    = pPackBuf;
    tRtpPack.m_nRealSize    = dwNaluLen;
    tRtpPack.m_byMark       = (u8)bMark;

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
    if( 0 != m_byLocalActivePT)
    {
        tRtpPack.m_byPayload = m_byLocalActivePT;
    }

    //�������ܲ���
    u16 wRet = EncryptRtpData(&tRtpPack, TRUE);
    if(KDVFAILED(wRet))
    {
        //����ʧ��
        return wRet;
    }

    //���뻺�巢��
    return m_pcRtp->Write(tRtpPack, FALSE, FALSE, m_bSendRtp);
}

u16 CKdvNetSnd::SendOneH265Pack_LongSlice(PFRAMEHDR pFrmHdr, u8 *pbyNalu, u32 dwNaluLen, BOOL32 bMark)
{
    u16 wRet = MEDIANET_NO_ERROR;
    const s32 FU_MAX_LEN = g_dwMaxExtendPackSize - 3;
    // ��һ���ֽ���indicator������
    const s32 FU_NUM = (dwNaluLen - 1  + FU_MAX_LEN - 1) / FU_MAX_LEN;
    s32 i;
    u8 *pbyFuData;
    u8 abyFuPack[MAX_EXTEND_PACK_SIZE];
    s32 nFuSize;
    BOOL32 bLastFU = FALSE;

    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(TRtpPack));
    tRtpPack.m_byExtence    = 0;//��չ��
    tRtpPack.m_byPayload    = pFrmHdr->m_byMediaType;
    tRtpPack.m_dwTimeStamp  = m_dwTimeStamp;//ʱ���
    tRtpPack.m_nExSize      = 0;

    //�Ƿ������˶�̬�غ�ֵ, ����������ʵ�غ�PTֵ�滻Ϊ��̬�غ�PTֵ
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

    abyFuPack[0] = (pbyNalu[0] & 0x81) | 0x62;//49��FU-A��type
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

        // ��ǰ�������ֽڼ���ȥ
        memcpy(abyFuPack + 3, pbyFuData, nFuSize);

        tRtpPack.m_pRealData    = abyFuPack;
        tRtpPack.m_nRealSize    = nFuSize + 3;
        tRtpPack.m_byMark       = (u8)(bMark & bLastFU);

        //�������ܲ���
        wRet = EncryptRtpData(&tRtpPack, TRUE);
        if(KDVFAILED(wRet))
        {
            break;
        }

        //���뻺�巢��
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

