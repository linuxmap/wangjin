 /*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvRtcp.h
����ļ�    : KdvRtcp.cpp
�ļ�ʵ�ֹ���: RTCP implementation
����        : Jason
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         Jason      Create
2003/06/03  2.0         Jason      Add RTP Option
2004/05/12  2.0         ����     ȥ��vx�µ�settimer��ʹ��task��taskdelay��ɶ�ʱ����
2004/06/29  2.0         ����     ʹ��ȫ���߳� ��task��taskdelay�� ��ɶ�ʱ����
2005/05/14  3.6         ����       �޸����ñ��ص�ַ(ԭ���б���ʵ������Ϊ0)
                                   ����RTCP�����鲥����
******************************************************************************/

#include "kdvrtcp.h"
#include "kdvrtp.h"

extern BOOL32 g_bUseMemPool;
extern s32   g_nShowDebugInfo;        //�Ƿ���ʾ���ص�һЩ����״̬��Ϣ

void RtcpDataCallBack(u8 *pBuf, s32 nBufSize, void* pContext);
//init class member
CKdvRtcp::CKdvRtcp()
{
    m_pRtp                = NULL;
    m_pSocket            = NULL;
    m_pbyBuf            = NULL;
    m_pbyCustomBuf        = NULL;
    m_nStartTime        = 0;
    m_nStartMilliTime   = 0;
    m_hSem = NULL;
    m_pRtcpInfoCallbackHandler = NULL;
    m_pContext = NULL;

    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));
    memset(&m_tRemoteAddr, 0, sizeof(m_tRemoteAddr));
    memset(&m_tMyInfo, 0, sizeof(m_tMyInfo));
    memset(&m_tRtcpInfoList, 0, sizeof(m_tRtcpInfoList));
}
//Destroy class member
CKdvRtcp::~CKdvRtcp()
{
    FreeBuf();
}

/*=============================================================================
    ������        ��create
    ����        �� init memory ,alloc memory
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwSSRC,��RTPƥ���ͬ��Դ

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::Create(u32 dwSSRC)
{
    if(dwSSRC == 0)
    {
        return ERROR_RTP_SSRC;
    }
    FreeBuf();

    m_tMyInfo.m_dwSSRC = dwSSRC;

#ifndef _EQUATOR_

    if(SOCKET_ERROR == gethostname(m_tMyInfo.m_tCName.m_szValue,
                                   sizeof(m_tMyInfo.m_tCName.m_szValue )))
    {
        return ERROR_SOCK_INIT;
    }

#endif

    m_tMyInfo.m_tCName.m_byLength = strlen(m_tMyInfo.m_tCName.m_szValue);

    m_tMyInfo.m_tCName.m_byType = 1;

    m_pSocket = new CKdvSocket;
    if(m_pSocket == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    m_pSocket->SetCallBack(RtcpDataCallBack, (void*)this);

    MEDIANET_MALLOC(m_pbyBuf, (MAX_RTCP_PACK + sizeof(u32) -1)/sizeof(u32) * sizeof(u32));
    if(m_pbyBuf == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    MEDIANET_MALLOC(m_pbyCustomBuf, (MAX_RTCP_PACK + sizeof(u32) -1)/sizeof(u32) * sizeof(u32));
    if(m_pbyCustomBuf == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }
    if(!OspSemBCreate( &m_hSem))
    {
        m_hSem=NULL;
        FreeBuf();
        return ERROR_CREATE_SEMAPORE;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��FreeBuf
    ����        �� Reset class member;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��: ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
void CKdvRtcp::FreeBuf()
{
    SAFE_DELETE(m_pSocket)
    MEDIANET_SAFE_FREE(m_pbyBuf)
    MEDIANET_SAFE_FREE(m_pbyCustomBuf)

    m_nStartTime        = 0;
    m_nStartMilliTime   = 0;

    m_pRtp = NULL;

    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));
    memset(&m_tRemoteAddr, 0, sizeof(m_tRemoteAddr));
    memset(&m_tMyInfo, 0, sizeof(m_tMyInfo));
    if(m_hSem !=  NULL)
    {
        OspSemDelete(m_hSem);
        m_hSem=NULL;
    }
}

/*=============================================================================
    ������        ��ResetSSRC
    ����        ������ͬ��Դ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwSSRC,��RTPƥ���ͬ��Դ

    ����ֵ˵���� �μ������붨��
=============================================================================*/
void CKdvRtcp::ResetSSRC(u32 dwSSRC)
{
    m_tMyInfo.m_dwSSRC = dwSSRC;
}

/*=============================================================================
    ������        ��SetRtcp
    ����        �����úͱ�RTCP��Ӧ��RTP����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtp  RTP�����ڲ����ڻص���ӦRTP

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtcp::SetRtp(CKdvRtp *pRtp)
{
    m_pRtp = pRtp;
    return MEDIANET_NO_ERROR;
}

u16 CKdvRtcp::SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext)
{
    MEDIANET_SEM_TAKE(m_hSem);
    m_pRtcpInfoCallbackHandler = pRtcpInfoCallback;
    m_pContext = pContext;
    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SendRSQ
    ����        ����������֡�ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   ptRSQ �ط�����ṹָ��
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::SendRSQ(TRtcpSDESRSQ &tRSQ)
{
    if(m_pbyCustomBuf == NULL|| m_pSocket== NULL)
    {
        return ERROR_RTCP_NO_INIT;
    }

    TBuffer tBuf;
    TRtcpSDESRSQ tMyRtpRSQ = tRSQ;
    tBuf = BufCreate(m_pbyCustomBuf, MAX_RTCP_PACK);

    CreateCustomRTCPPacket(&tBuf, tMyRtpRSQ);

    //���˵����ֿ���
    if( (m_tLocalAddr.m_dwIP == m_tRemoteAddr.m_tAddr[0].m_dwIP ||
         0 == m_tRemoteAddr.m_tAddr[0].m_dwIP) &&
        (m_tLocalAddr.m_wPort == m_tRemoteAddr.m_tAddr[0].m_wPort) )
    {
        return MEDIANET_NO_ERROR;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    //����RTCP������ַ�趨�����ش�����
    if( (m_tRemoteAddr.m_tAddr[0].m_dwIP != 0)&&
        (m_tRemoteAddr.m_tAddr[0].m_wPort != 0) )
    {
        if (0 == m_tRemoteAddr.m_tAddr[0].m_dwUserDataLen)
        {
            m_pSocket->SendTo(tBuf.m_pBuf, tBuf.m_dwLen,
                              m_tRemoteAddr.m_tAddr[0].m_dwIP,
                              m_tRemoteAddr.m_tAddr[0].m_wPort);

        }
        else
        {
            u8 abyBuff[1500+MAX_USERDATA_LEN];
            memcpy(abyBuff,
                    m_tRemoteAddr.m_tAddr[0].m_abyUserData,
                    m_tRemoteAddr.m_tAddr[0].m_dwUserDataLen);

            memcpy(abyBuff+m_tRemoteAddr.m_tAddr[0].m_dwUserDataLen,
                    tBuf.m_pBuf,
                    tBuf.m_dwLen);

            m_pSocket->SendTo(abyBuff,
                              tBuf.m_dwLen + m_tRemoteAddr.m_tAddr[0].m_dwUserDataLen,
                              m_tRemoteAddr.m_tAddr[0].m_dwIP,
                              m_tRemoteAddr.m_tAddr[0].m_wPort);
        }
    }

    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetLocalAddr
    ����        �� Set  socket local address and Port;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     dwIp  local IP
                     wPort local Port;

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::SetLocalAddr(u32 dwIp, u16 wPort, u32 dwUserDataLen, u8* pbyUserData, u32 dwFlag, void* pRegFunc, void* pUnregFunc)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTCP_NO_INIT;
    }

    //the same to last set
    if(dwIp == m_tLocalAddr.m_dwIP&&
        wPort == m_tLocalAddr.m_wPort &&
        dwUserDataLen == m_tLocalAddr.m_dwUserDataLen &&
        (0 == memcmp(pbyUserData, m_tLocalAddr.m_abyUserData, dwUserDataLen)))
    {
        return MEDIANET_NO_ERROR;
    }

    BOOL32 bRet;
    if (IsMultiCastAddr(dwIp) || IsBroadCastAddr(dwIp))
    {
        //����ǽ��գ��������鲥��㲥���������鲥��ַ
        bRet = m_pSocket->Create(SOCK_DGRAM, wPort, 0, dwIp, TRUE, dwFlag, pRegFunc, pUnregFunc);
    }
    else
    {
        bRet = m_pSocket->Create(SOCK_DGRAM, wPort, dwIp, 0, TRUE, dwFlag, pRegFunc, pUnregFunc);
    }

    //create local socket ,no bind local ip
    if(!bRet)
    {
        return ERROR_SND_CREATE_SOCK;
    }

    m_tLocalAddr.m_dwIP  = dwIp;
    m_tLocalAddr.m_wPort = wPort;
    if (dwUserDataLen <= MAX_USERDATA_LEN)
    {
        m_tLocalAddr.m_dwUserDataLen = dwUserDataLen;
        memcpy(m_tLocalAddr.m_abyUserData, pbyUserData, m_tLocalAddr.m_dwUserDataLen);
    }
    else
    {
        return ERROR_SND_PARAM;
    }

    bRet = m_pSocket->SetLocalUserData(dwUserDataLen, pbyUserData);
    if(!bRet)
    {
        return ERROR_SET_USERDATA;
    }

    return MEDIANET_NO_ERROR;
}


u16  CKdvRtcp::GetLocalAddr(u32* pdwIp, u16* pwPort)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    if (NULL != pdwIp)
    {
        *pdwIp = m_tLocalAddr.m_dwIP;
    }

    if (NULL != pwPort)
    {
        *pwPort = m_tLocalAddr.m_wPort;
    }

    return MEDIANET_NO_ERROR;
}


u16 CKdvRtcp::RemoveLocalAddr()
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    m_pSocket->Close(TRUE);

    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));

    return MEDIANET_NO_ERROR;
}

u16 CKdvRtcp::SetSrcAddr(u32 dwSrcIP, u16 wSrcPort)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    m_pSocket->SetSrcAddr(dwSrcIP, wSrcPort);

    return MEDIANET_NO_ERROR;
}


u16 CKdvRtcp::GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo)
{
    if (m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    m_pSocket->GetSndSocketInfo(tSocketInfo);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        SetMCastOpt
    ����        �������鲥��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  dwLocalIP�� �ӿ�IP(net order)
                  dwMCastIP�� �鲥��IP��ַ(net order)

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtcp::SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv /*=FALSE*/)
{
    u16 wRet = ERROR_SND_CREATE_SOCK;

    if(m_pSocket == NULL)
    {
        return wRet;
    }

    if(TRUE == m_pSocket->SetMCastOpt(dwLocalIP, dwMCastIP, bRcv))
    {
        wRet = MEDIANET_NO_ERROR;
    }

    return wRet;
}

u16 CKdvRtcp::SendUserDefinedBuff(u8 *pbyBuf, u16 wBufLen, u32 dwPeerAddr, u16 wPeerPort)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTCP_NO_INIT;
    }
        u16 wRet = MEDIANET_NO_ERROR;

    wRet =  m_pSocket->SendUserDefinedBuff( pbyBuf, wBufLen, dwPeerAddr, wPeerPort);

    if(g_nShowDebugInfo == 66)
    {
        OspPrintf(1,0, "[medianet : SendUserDefinedBuff] send nat pack to remote ip=%8x port=%d \n", dwPeerAddr,wPeerPort);
    }
    return wRet;
}

/*=============================================================================
    ������        SetBCastOpt
    ����        ������㲥��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtcp::SetBCastOpt( )
{
    u16 wRet = ERROR_SND_CREATE_SOCK;

    if(m_pSocket == NULL)
    {
        return wRet;
    }

    if(TRUE == m_pSocket->SetBCastOpt())
    {
        wRet = MEDIANET_NO_ERROR;
    }

    return wRet;
}


/*=============================================================================
    ������        ��SetRemoteAddr
    ����        �� Set  socket Remote  address and Port;
                   set timer to send RTCP packets;

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     tRemoteAddr  Զ�˵�ַ��.

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::SetRemoteAddr(TRemoteAddr &tRemoteAddr)
{
    m_tRemoteAddr = tRemoteAddr;

    return MEDIANET_NO_ERROR;
}

u16 CKdvRtcp::GetRemoteAddr(TRemoteAddr &tRemoteAddr)
{
    tRemoteAddr = m_tRemoteAddr;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        :  UpdateSend
    ����        �� update RTCP local status

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     nSendDataSize  octet that already send
                     dwTimeStamp    local time;

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::UpdateSend(s32 nSendDataSize, u32 dwTimeStamp)
{
    MEDIANET_SEM_TAKE(m_hSem);
    m_tMyInfo.m_bActive = TRUE;
    m_tMyInfo.m_tSR.m_dwPackets++;
    m_tMyInfo.m_tSR.m_dwBytes += nSendDataSize;
    m_tMyInfo.m_tSR.m_tNNTP   = GetNNTPTime();
    m_tMyInfo.m_tSR.m_dwRTP   = dwTimeStamp;
    MEDIANET_SEM_GIVE(m_hSem);

/*
    if (m_tMyInfo.m_nCollision)
    {
        return ERROR_RTP_SSRC_COLLISION;
    }
*/

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        :  UpdateRcv
    ����        �� update RTP session info

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     dwSSRC         Remote SSRC
                     dwTimeStamp    local time;
                     dwTimestamp    remote rtp timestamp
                     wSequence      remote rtp packet sequence number
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtcp::UpdateRcv(u32 dwSSRC, u32 dwLocalTimestamp,
                         u32 dwTimestamp, u16 wSequence )
{
    MEDIANET_SEM_TAKE(m_hSem);
    TRtcpInfo  *pRtcpInfo = NULL;

    if (dwSSRC == m_tMyInfo.m_dwSSRC)
    {
        m_tMyInfo.m_nCollision = 1;
        MEDIANET_SEM_GIVE(m_hSem);
        return ERROR_RTP_SSRC_COLLISION;
    }

    pRtcpInfo = GetRtcpInfo( dwSSRC);

    if (NULL == pRtcpInfo && m_pSocket != NULL) /* New source */
    {
        // Initialize info
        TRtcpInfo Info;
        memset(&Info, 0, sizeof(Info));
        Info.m_dwSSRC                = dwSSRC;
        Info.m_tToRR.m_dwSSRC        = Info.m_dwSSRC;
        Info.m_bActive                = FALSE;
        Info.m_tSrc.m_dwProbation    = MIN_SEQUENTIAL - 1;//���յ�����չ��������к�

        // Add to list
        pRtcpInfo = (TRtcpInfo *)AddRtcpInfo(Info);

        if (pRtcpInfo == NULL) // can't add to list?
        {
            //array full
        }
    }
    else
    {
        if (!pRtcpInfo->m_nInvalid)
        {
            pRtcpInfo->m_bActive = TRUE;
            UpdateSeq(&(pRtcpInfo->m_tSrc), wSequence,
                         dwLocalTimestamp, dwTimestamp);
        }
    }

    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        :  DealTimer
    ����        �� ����ʱ����RTCP��

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:

    ����ֵ˵���� ��
=============================================================================*/
u16 CKdvRtcp::DealTimer()
{
    if(m_pbyBuf == NULL || m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    TBuffer tBuf;
    tBuf = BufCreate(m_pbyBuf, MAX_RTCP_PACK);

    CreateRTCPPacket(&tBuf);

    for(s32 i=0; i<m_tRemoteAddr.m_byNum; i++)
    {
        if( (m_tRemoteAddr.m_tAddr[i].m_dwIP != 0) &&
            (m_tRemoteAddr.m_tAddr[i].m_wPort != 0) )
        {
            m_pSocket->SendTo(tBuf.m_pBuf, tBuf.m_dwLen,
                              m_tRemoteAddr.m_tAddr[i].m_dwIP,
                              m_tRemoteAddr.m_tAddr[i].m_wPort);
        }
    }

    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        :  DealData
    ����        �� �����յ�RTCP���ݿ�

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  pBuf       �յ������ݻ���
                  nBufSize   �����С

    ����ֵ˵���� ��
=============================================================================*/
void CKdvRtcp::DealData(u8 *pBuf, s32 nBufSize)//, TAddr &tRemoteAddr)
{
    if (NULL == pBuf)
    {
        return;
    }

    //check if need to del userhead, add by hual 2006-08-26
    //���ǵ����ܴ��ڴ���ͷǴ�����ʹ�õ������Ŀǰ���ж�ͷ�ӽڵ������λ�Ƿ�ΪRTP�İ汾��
    if (m_tLocalAddr.m_dwUserDataLen > 0  && (u32)nBufSize > m_tLocalAddr.m_dwUserDataLen &&
        (((*pBuf)&0xC0) != 0x80))
    {
        //���ڽ���ʱͷ�������д���ǶԶ˵�Դ��ַ��Ŀǰ������ƥ��
        //if (0 == memcmp(pBuf, m_tLocalAddr.m_dwUserDataLen, m_tLocalAddr.m_dwUserDataLen))
        {
            pBuf += m_tLocalAddr.m_dwUserDataLen;
            nBufSize -= m_tLocalAddr.m_dwUserDataLen;
        }
    }

    MEDIANET_SEM_TAKE(m_hSem);

    TRtcpHeader *ptHead  = NULL;
    u8       *currPtr = pBuf, *dataPtr, *compoundEnd;
    s32       hdr_count, hdr_len;
    TRtcpType   hdr_type;

    compoundEnd = pBuf + nBufSize;

    //loop to deal compound pack
    while (currPtr < compoundEnd)
    {

        ptHead = (TRtcpHeader *)(currPtr);
        ConvertN2H(currPtr, 0, 1);

        hdr_count = GetBitField(ptHead->m_dwBits, HEADER_RC, HDR_LEN_RC);
        hdr_type  = (TRtcpType)GetBitField(ptHead->m_dwBits,
                                          HEADER_PT, HDR_LEN_PT);
        hdr_len   = sizeof(u32) *
            (GetBitField(ptHead->m_dwBits, HEADER_len, HDR_LEN_len));

        if ((compoundEnd - currPtr) < hdr_len)
        {
            OspPrintf(1, 0, "[CKdvRtcp::DealData] Invalid RTCP packet.\n");
            MEDIANET_SEM_GIVE(m_hSem);
            return ;
        }

        dataPtr = (u8 *)ptHead + sizeof(u32);

        // deal RTCP packet
        ProcessRTCPPacket(dataPtr, hdr_len, hdr_type, hdr_count,
            GetNNTPTime());

        currPtr += hdr_len + sizeof(u32);
    }
    MEDIANET_SEM_GIVE(m_hSem);
}

/*=============================================================================
    ������        :  ProcessRTCPPacket
    ����        �� ����һ���ݰ����͵�RTCP��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  pData       ���ݻ���
                  nDataLen    �����С
                  type        RTCP type
                  nRCount     report Count
                  myTime      local current wallclock time
    ����ֵ˵���� ��
=============================================================================*/
void  CKdvRtcp::ProcessRTCPPacket(u8 *pData, s32 nDataLen,
                                  TRtcpType type, s32  nRCount,
                                  TUint64 myTime)
{
    s32 nScanned = 0;
    TRtcpInfo Info, *pInfo=NULL;

    if (nDataLen == 0)
        return;

    switch(type)
    {
        case RTCP_SR:
        case RTCP_RR:
        {
            ConvertN2H(pData, 0, 1);
            Info.m_dwSSRC = *(u32 *)(pData);
            nScanned      = sizeof(u32);

            //SSRC��������ֹ�������Լ�SSRC��ͬ�����
            if (Info.m_dwSSRC == m_tMyInfo.m_dwSSRC)
            {
                m_tMyInfo.m_nCollision = 1;

                return ;
            }

            pInfo = (TRtcpInfo *)GetRtcpInfo(Info.m_dwSSRC);

            if (pInfo == NULL) /* New source */
            {
                /* Initialize info */
                memset(&Info, 0, sizeof(Info));
                Info.m_dwSSRC                = *(u32 *)(pData);
                Info.m_tToRR.m_dwSSRC        = Info.m_dwSSRC;
                Info.m_bActive                = FALSE;
                Info.m_tSrc.m_dwProbation    = MIN_SEQUENTIAL - 1;

                /* Add to list */
                 pInfo = (TRtcpInfo *)AddRtcpInfo(Info);

                if (pInfo == NULL) /* can't add to list? */
                {
                    /*array full*/
                }
            }
            break;
        }

        default:
            break;
    }

    /* process the information */
    switch(type)
    {
        case RTCP_SR:
        {
            ConvertN2H(pData + nScanned, 0, W32Len(sizeof(TRtcpSR)));

            if (pInfo)
            {
                pInfo->m_tSR            = *(TRtcpSR *)(pData + nScanned);
                pInfo->m_tToRR.m_dwLSR  = reduceNNTP(pInfo->m_tSR.m_tNNTP);
                pInfo->m_dwLSRMyTime    = reduceNNTP(myTime);
                pInfo->m_emRtcpInfoType = RTCP_SR;
                if (m_pRtcpInfoCallbackHandler)
                {
                    m_pRtcpInfoCallbackHandler(pInfo, (KD_PTR)m_pContext);
                }
            }

            nScanned += SIZEOF_SR;
            /*break;*///maybe SDES
        }

        /* fall into RR */
         case RTCP_RR:
        {
        if (pInfo)
        {
            TRtcpRR* rr = (TRtcpRR *)(pData + nScanned);

            ConvertN2H(pData + nScanned, 0,
                              nRCount * W32Len(sizeof(TRtcpRR)));

            for (s32 i=0; i < nRCount; i++)
            {
                //ֻ�����Լ�������SR����RR
                if (rr[i].m_dwSSRC == m_tMyInfo.m_dwSSRC)
                {
                    pInfo->m_tFromRR = rr[i];
                    break;
                }
            }
            if(type == RTCP_RR)
            {
                 pInfo->m_emRtcpInfoType = RTCP_RR;
                if (m_pRtcpInfoCallbackHandler)
                {
                    m_pRtcpInfoCallbackHandler(pInfo, (KD_PTR)m_pContext);
                }

            }
          }
         break;//RR is packet'end;
      }

        case RTCP_SDES:
        {
            TRtcpSDES *pSdes;

            for (s32 i = 0; i < nRCount; i++)
            {
                ConvertN2H(pData + nScanned, 0, 1);
                Info.m_dwSSRC = *(u32 *)(pData + nScanned);

                pSdes = (TRtcpSDES *)(pData + nScanned + sizeof(Info.m_dwSSRC));

                if(RTCP_SDES_NOTE == pSdes->m_byType)
                {
                    ParseRSQ(pSdes, nDataLen);
                }

                pInfo = (TRtcpInfo*)GetRtcpInfo(Info.m_dwSSRC);

                if (pInfo != NULL)
                {
                    switch(pSdes->m_byType)
                    {
                        case RTCP_SDES_CNAME:
                            memcpy(&(pInfo->m_tCName), pSdes,
                                   SIZEOF_SDES(*pSdes));
                            pInfo->m_tCName.m_szValue[pSdes->m_byLength] = 0;
                            break;
/* known SDES types that are not handled:
                        case RTCP_SDES_END:
                        case RTCP_SDES_NAME:
                        case RTCP_SDES_EMAIL:
                        case RTCP_SDES_PHONE:
                        case RTCP_SDES_LOC:
                        case RTCP_SDES_TOOL:
                        case RTCP_SDES_NOTE:
                        case RTCP_SDES_PRIV:
                            break;
*/
                        }
                    }

                    nScanned += SIZEOF_SDES(*pSdes) + sizeof(u32);
                }

            break;
        }

        case RTCP_BYE:
        {
            s32 i;

            for (i = 0; i < nRCount; i++)
            {
                ConvertN2H(pData + nScanned, 0, 1);
                Info.m_dwSSRC = *(u32 *)(pData + nScanned);
                nScanned += sizeof(Info.m_dwSSRC);

                pInfo = (TRtcpInfo *)GetRtcpInfo(Info.m_dwSSRC);

                if (pInfo)
                {
                    pInfo->m_nInvalid  = TRUE;
                    pInfo->m_dwSSRC    = 0;
                }
            }
            break;
        }
        case RTCP_APP:
      {
            break;
        }
    }
}

/*=============================================================================
    ������        :  ParseRSQ
    ����        �� �����Զ�����ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:

    ����ֵ˵���� ��
=============================================================================*/
void CKdvRtcp::ParseRSQ(TRtcpSDES* pSdes, u32 dwLen)
{
    if(NULL == m_pRtp ||
       NULL == pSdes ||
       sizeof(TRtcpSDESRSQ)+sizeof(u32) != dwLen)
    {
        return;
    }
    if((sizeof(TRtcpSDESRSQ) - 2*sizeof(u8)) != pSdes->m_byLength)
    {
        return;
    }

    TRtcpSDESRSQ *ptRSQ = (TRtcpSDESRSQ *)pSdes;
    m_pRtp->DealRSQBackQuest(ptRSQ);
}

/*=============================================================================
    ������        :  CreateCustomRTCPPacket
    ����        �� create Custom Rtcp packet ���ڴ����Զ�����ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  ptBuf   data block to fill

    ����ֵ˵���� ��
=============================================================================*/
void CKdvRtcp::CreateCustomRTCPPacket(TBuffer *ptBuf, TRtcpSDESRSQ &tRSQ)
{
    TRtcpHeader   tHead;
    u32         dwAllocated = 0;
    TRtcpSDesType emsdestype = RTCP_SDES_NOTE;

    /* add an NOTE SDES (TRtcpSDESRSQ) packet to the compound packet */
    if (BufValid(ptBuf, SIZEOF_RTCPHEADER + sizeof(TRtcpSDESRSQ)))
    {
        TBuffer tSdesBuf;

        /* 'tSdesBuf' is inside the compound buffer 'buf' */
        tSdesBuf = BufCreate(ptBuf->m_pBuf + dwAllocated,
                             SIZEOF_RTCPHEADER + sizeof(TRtcpSDESRSQ));

        tHead = MakeHeader(m_tMyInfo.m_dwSSRC, 1, RTCP_SDES,
                                  (u16)tSdesBuf.m_dwLen);

        tRSQ.m_byType = emsdestype;
        tRSQ.m_byLength = sizeof(TRtcpSDESRSQ) - 2*sizeof(u8);
        memcpy(tSdesBuf.m_pBuf, (s8 *)&tHead, SIZEOF_RTCPHEADER);
        memcpy(tSdesBuf.m_pBuf + SIZEOF_RTCPHEADER, (s8 *)&tRSQ, sizeof(TRtcpSDESRSQ));

        dwAllocated += tSdesBuf.m_dwLen;
    }

    ptBuf->m_dwLen = dwAllocated;
    return ;
}

/*=============================================================================
    ������        :  CreateRTCPPacket
    ����        �� create Rtcp packet
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  ptBuf   data block to fill

    ����ֵ˵���� ��
=============================================================================*/
void CKdvRtcp::CreateRTCPPacket(TBuffer   *ptBuf)
{
    TRtcpHeader  tHead;
    u32        dwAllocated = 0;
    TBuffer      tBufC;
    TRtcpType    type = RTCP_SR;

    if (BufValid(ptBuf, SIZEOF_RTCPHEADER + SIZEOF_SR))
    {
        TUint64 myTime =m_tMyInfo.m_tSR.m_tNNTP;

        dwAllocated = SIZEOF_RTCPHEADER;
        if (m_tMyInfo.m_bActive)
        {
            //SR packet
            m_tMyInfo.m_bActive = FALSE;
            tBufC = BufCreate(&(m_tMyInfo.m_tSR), SIZEOF_SR);
            BufAddToBuffer(ptBuf, &tBufC, dwAllocated);
            ConvertH2N(ptBuf->m_pBuf + dwAllocated, 0, W32Len(tBufC.m_dwLen));
            dwAllocated += SIZEOF_SR;
        }
        else
        {
            type = RTCP_RR;
        }

        //RR packets
        u8 cc = 0;
        TRtcpInfo * pInfo = NULL;
        for(s32 Index=0; Index<m_tRtcpInfoList.m_nSessionNum; Index++)
        {
            pInfo = (TRtcpInfo *)&(m_tRtcpInfoList.m_tRtcpInfo[Index]);
            if (pInfo->m_bActive)
            {
                pInfo->m_tToRR.m_dwFLost     = GetLost    (&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_dwJitter    = GetJitter  (&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_dwExtMaxSeq = GetSequence(&(pInfo->m_tSrc));
                pInfo->m_tToRR.m_dwDLSR     =
                    (pInfo->m_dwLSRMyTime) ?
                    (reduceNNTP(myTime)-pInfo->m_dwLSRMyTime) : 0;

                tBufC = BufCreate(&(pInfo->m_tToRR), SIZEOF_RR);

                if (BufAddToBuffer(ptBuf, &tBufC, dwAllocated))
                {
                    cc++;
                    if (cc == 32) break;
                    ConvertH2N(ptBuf->m_pBuf + dwAllocated, 0,
                                          W32Len(tBufC.m_dwLen));
                    dwAllocated += SIZEOF_RR;
                }
                pInfo->m_bActive = FALSE;
            }
        }

        tHead = MakeHeader(m_tMyInfo.m_dwSSRC, cc, type, (u16)dwAllocated);
           tBufC = BufCreate(&tHead, SIZEOF_RTCPHEADER);
        BufAddToBuffer(ptBuf, &tBufC, 0);

        /* add an CNAME SDES packet to the compound packet */
        if (BufValid(ptBuf,
            dwAllocated + SIZEOF_RTCPHEADER + SIZEOF_SDES(m_tMyInfo.m_tCName)))
        {
            TBuffer tSdesBuf;

            /* 'tSdesBuf' is inside the compound buffer 'buf' */
            tSdesBuf = BufCreate(ptBuf->m_pBuf + dwAllocated,
                        (SIZEOF_RTCPHEADER + SIZEOF_SDES(m_tMyInfo.m_tCName)));

            tHead = MakeHeader(m_tMyInfo.m_dwSSRC, 1, RTCP_SDES,
                                (u16)tSdesBuf.m_dwLen);

            memcpy(tSdesBuf.m_pBuf, (s8 *)&tHead, SIZEOF_RTCPHEADER);
            memcpy(tSdesBuf.m_pBuf + SIZEOF_RTCPHEADER, &(m_tMyInfo.m_tCName),
                                     SIZEOF_SDES(m_tMyInfo.m_tCName));

            dwAllocated += tSdesBuf.m_dwLen;
        }

        if (m_tMyInfo.m_nCollision == 1  &&
            BufValid(ptBuf, dwAllocated + SIZEOF_RTCPHEADER))
        {
            tHead = MakeHeader(m_tMyInfo.m_dwSSRC, 1, RTCP_BYE,
                                SIZEOF_RTCPHEADER);

            tBufC = BufCreate(&tHead, SIZEOF_RTCPHEADER);
            BufAddToBuffer(ptBuf, &tBufC, dwAllocated);
            m_tMyInfo.m_nCollision = 2;
            dwAllocated += SIZEOF_RTCPHEADER;
        }
    }

    ptBuf->m_dwLen = dwAllocated;
    return ;
}

/*=============================================================================
    ������        :  GetLost
    ����        �� calculate lose packets,see the standard
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  pRtpSource   rtp source info

    ����ֵ˵���� ��
=============================================================================*/
u32  CKdvRtcp::GetLost(TRtpSource *pRtpSource)
{
    u32 extended_max;
    u32 expected;
    s32  received_interval;
    s32  expected_interval;
    s32  lost;
    s32  lost_interval;
    u8  fraction;

    extended_max = pRtpSource->m_dwCycles + pRtpSource->m_wMaxSeq;
    expected = extended_max - pRtpSource->m_dwBaseSeq + 1;
    lost = expected - pRtpSource->m_dwReceived;
    expected_interval = expected - pRtpSource->m_dwExpectedPrior;
    pRtpSource->m_dwExpectedPrior = expected;
    received_interval = pRtpSource->m_dwReceived - pRtpSource->m_dwReceivedPrior;
    pRtpSource->m_dwReceivedPrior = pRtpSource->m_dwReceived;
    lost_interval = expected_interval - received_interval;

    if (expected_interval == 0  ||  lost_interval <= 0)
        fraction = 0;
    else
        fraction = (u8)((lost_interval << 8) / expected_interval);

    return (fraction << 24) + lost;
}

/*=============================================================================
    ������        :  GetJitter
    ����        �� Get net jitter ,see the standard
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  pRtpSource   rtp source info

    ����ֵ˵���� ��
=============================================================================*/
u32 CKdvRtcp::GetJitter(TRtpSource *pRtpSource)
{
    return pRtpSource->m_dwJitter >> 4;
}

/*=============================================================================
    ������        :  GetSequence
    ����        �� Get packet max sequence ,see the standard
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                  pRtpSource   rtp source info

    ����ֵ˵���� ��
=============================================================================*/
u32 CKdvRtcp::GetSequence(TRtpSource *pRtpSource)
{
    return pRtpSource->m_wMaxSeq + pRtpSource->m_dwCycles;
}
/*=============================================================================
    ������        :  GetNNTPTime
    ����        �� Get  time from 1900

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��: ��

    ����ֵ˵���� 64 bits time
=============================================================================*/
TUint64 CKdvRtcp::GetNNTPTime()
{
    TUint64 nntpTime;

    if (!m_nStartTime)
    {
        m_nStartTime      = (u32)time(NULL) + FROM1900TILL1970;
        m_nStartMilliTime = OspTickGet();
    }

    nntpTime.msdw = m_nStartTime;
    nntpTime.lsdw = OspTickGet() - m_nStartMilliTime;
    nntpTime.msdw += nntpTime.lsdw/1000;
    nntpTime.lsdw %= 1000;
    nntpTime.lsdw *= 4294967;//65536*65536/1000;

    return nntpTime;
}

/*=============================================================================
    ������        :  GetRtcpInfo
    ����        �� get rtcp info

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     dwSSRC ͬ��Դ

    ����ֵ˵���� �ɹ�Ϊָ��rtcpinfo �ṹ, ʧ�ܷ���NULL;
=============================================================================*/
TRtcpInfo *CKdvRtcp::GetRtcpInfo(u32 dwSSRC)
{
    for(s32 i=0; i<m_tRtcpInfoList.m_nSessionNum; i++)
    {
        if(m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == dwSSRC)
        {
            return &(m_tRtcpInfoList.m_tRtcpInfo[i]);
        }
    }
    return NULL;
}

/*=============================================================================
    ������        :  AddRtcpInfo
    ����        �� Add rtcp info

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                    Info   Ҫ����rtcpinfo

    ����ֵ˵���� �ɹ�Ϊָ��rtcpinfo �ṹ, ʧ�ܷ���NULL;
=============================================================================*/
TRtcpInfo *CKdvRtcp::AddRtcpInfo(TRtcpInfo &Info)
{
    for(s32 i=0; i<m_tRtcpInfoList.m_nSessionNum; i++)
    {
        if(m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == Info.m_dwSSRC)
        {
            return &(m_tRtcpInfoList.m_tRtcpInfo[i]);
        }

        //�����ЧSSRC�����Ա
        if(m_tRtcpInfoList.m_tRtcpInfo[i].m_nInvalid&&
            m_tRtcpInfoList.m_tRtcpInfo[i].m_dwSSRC == 0)
        {
            m_tRtcpInfoList.m_tRtcpInfo[i] = Info;
            return &Info;
        }
    }

    if(m_tRtcpInfoList.m_nSessionNum < MAX_SESSION_NUM)
    {
        m_tRtcpInfoList.m_tRtcpInfo[m_tRtcpInfoList.m_nSessionNum] = Info;
        m_tRtcpInfoList.m_nSessionNum++;
        return &Info;
    }
    else
    {
        //find the Earliset time pos to update;
        s32 nMaxEarlyTimePos = 0;
        u32 dwTime = 0;
        s32 j;
        for(j=0; j<m_tRtcpInfoList.m_nSessionNum; j++)
        {
            if(m_tRtcpInfoList.m_tRtcpInfo[j].m_dwLSRMyTime < dwTime)
            {
                dwTime = m_tRtcpInfoList.m_tRtcpInfo[j].m_dwLSRMyTime;
                nMaxEarlyTimePos = j;
            }
        }
        m_tRtcpInfoList.m_tRtcpInfo[nMaxEarlyTimePos] = Info;
        return &Info;
    }
    return NULL;
}

/*=============================================================================
    ������        :  InitSeq
    ����        �� init  one peer rtcp info;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                   pRtpSource    the pointer to one Rtcp info struct
                   seq           the sequence number to init

    ����ֵ˵���� ��
=============================================================================*/
void CKdvRtcp::InitSeq(TRtpSource *pRtpSource, u16 seq)
{
    pRtpSource->m_dwBaseSeq       = seq;
    pRtpSource->m_wMaxSeq        = seq;
    pRtpSource->m_dwBadSeq        = RTP_SEQ_MOD + 1;
    pRtpSource->m_dwCycles        = 0;
    pRtpSource->m_dwReceived      = 0;
    pRtpSource->m_dwReceivedPrior = 0;
    pRtpSource->m_dwExpectedPrior = 0;
}

/*=============================================================================
    ������        :  UpdateSeq
    ����        �� update  one peer rtcp info;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                   pRtpSource    the pointer to one Rtcp info struct
                   seq           the sequence number to update
                   dwArrivalTS   the timestamp from the incoming packet
                   dwTimeStamp   the current time in the same  units

    ����ֵ˵���� �ɹ� TRUE,ʧ��FALSE;
=============================================================================*/

BOOL32 CKdvRtcp::UpdateSeq(TRtpSource *pRtpSource, u16 seq,
                         u32 dwArrivalTS,u32 dwTimeStamp)
{
       u16 uDelta = (u16)(seq - pRtpSource->m_wMaxSeq);

       /*
       * Source is not valid until MIN_SEQUENTIAL packets with
       * sequential sequence numbers have been received.
       */
       if (pRtpSource->m_dwProbation)
       {
           /* packet is in sequence */
           if (seq == pRtpSource->m_wMaxSeq + 1)
           {
               pRtpSource->m_dwProbation--;
               pRtpSource->m_wMaxSeq = seq;
               if (0 == pRtpSource->m_dwProbation)
               {
                   InitSeq(pRtpSource, seq);
                   pRtpSource->m_dwReceived++;
                   return TRUE;
               }
           }
           else
           {
               pRtpSource->m_dwProbation = MIN_SEQUENTIAL - 1;
               pRtpSource->m_wMaxSeq = seq;
           }
           return FALSE;
       }
       else if (uDelta < MAX_DROPOUT)
       {
           /* in order, with permissible gap */
           if (seq < pRtpSource->m_wMaxSeq)
           {
               /* Sequence number wrapped - count another 64K cycle.*/
                pRtpSource->m_dwCycles += RTP_SEQ_MOD;
           }
           pRtpSource->m_wMaxSeq = seq;
       }
       else if (uDelta <= RTP_SEQ_MOD - MAX_MISORDER)
       {
           if (seq == pRtpSource->m_dwBadSeq)
           {
                /*
                 * Two sequential packets -- assume that the other side
                 * restarted without telling us so just re-sync
                 * (i.e., pretend this was the first packet).
                 */
                InitSeq(pRtpSource, seq);
           }
           else
           {
               pRtpSource->m_dwBadSeq = (seq + 1) & (RTP_SEQ_MOD-1);
               return FALSE;
           }
       }
       else
       {
                /* duplicate or reordered packet */ /*the same to the standard*/
       }
       {// for C
           s32  nTransit = (s32)(dwArrivalTS - dwTimeStamp);
           s32  nDelta = (s32)(nTransit - pRtpSource->m_dwTransit);
           pRtpSource->m_dwTransit = nTransit;
           if (nDelta < 0)
           {
               nDelta = -nDelta;
           }

           //reduce round-off error
           pRtpSource->m_dwJitter += (nDelta -
                                    ((pRtpSource->m_dwJitter + 8) >> 4));
       }
       pRtpSource->m_dwReceived++;

    return TRUE;
}

/*=============================================================================
    ������        :  BufAddToBuffer
    ����        �� Add one Buffer to the other's end
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                    pTo     the other's;
                    pFrom   one buffer;
                    Offset  offset

    ����ֵ˵���� �ɹ�TRUE,ʧ��FALSE;
=============================================================================*/
BOOL32 CKdvRtcp::BufAddToBuffer(TBuffer *pTo, TBuffer *pFrom, u32 Offset)
{
    if (pFrom->m_dwLen + Offset <= pTo->m_dwLen)
    {
        memcpy((u8*)pTo->m_pBuf + Offset, pFrom->m_pBuf, pFrom->m_dwLen);
        return TRUE;
    }
    return FALSE;
}

/*=============================================================================
    ������        :  BufValid
    ����        �� check buffer validity;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                   pBuf      buffer to check
                   dwSize    size to check

    ����ֵ˵���� Valid TRUE,Invalid FALSE;
=============================================================================*/
BOOL32 CKdvRtcp::BufValid(TBuffer *pBuf, u32 dwSize)
{
    return (dwSize <= pBuf->m_dwLen  &&  pBuf->m_pBuf);
}

/*=============================================================================
    ������        :  BufCreate
    ����        �� create TBuffer;
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                   pData   buffer that add to Tbuffer
                   dwSize  size   that add to Tbuffer

    ����ֵ˵���� Valid TRUE,Invalid FALSE;
=============================================================================*/
TBuffer CKdvRtcp::BufCreate(void* pData, u32 dwSize)
{
    TBuffer tBuf;
    tBuf.m_pBuf  = (u8 *)pData;
    tBuf.m_dwLen = dwSize;
    return tBuf;
}

/*=============================================================================
    ������        :  MakeHeader
    ����        �� make RTCP fixed header

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��: ��

    ����ֵ˵���� 64 bits time
=============================================================================*/
TRtcpHeader CKdvRtcp::MakeHeader(u32 dwSSRC, u8 count,
                                 TRtcpType type, u16 dataLen)
{
    TRtcpHeader header;

    header.m_dwSSRC = dwSSRC;

    header.m_dwBits = RTCP_HEADER_INIT;

    //set reception report count
    header.m_dwBits = SetBitField(header.m_dwBits, count, HEADER_RC, HDR_LEN_RC);
    //set report type
    header.m_dwBits = SetBitField(header.m_dwBits, type,  HEADER_PT, HDR_LEN_PT);
    //set one report len  ,see the standard
    header.m_dwBits = SetBitField(header.m_dwBits, W32Len(dataLen) - 1,
        HEADER_len, HDR_LEN_len);

    header.m_dwSSRC = htonl(header.m_dwSSRC);
    header.m_dwBits = htonl(header.m_dwBits);

    return header;
}

void RtcpDataCallBack(u8 *pBuf, s32 nBufSize, void* pContext)//, TAddr &tRemoteAddr)
{
   CKdvRtcp *pMain=(CKdvRtcp *)pContext;
   if(pMain != NULL)
   {
       pMain->DealData(pBuf, nBufSize);//, tRemoteAddr);
   }
}

