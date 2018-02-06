/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvRtp.cpp
����ļ�    : KdvRtp.h
�ļ�ʵ�ֹ���: RTP Format Implementation
����        : Jason
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         Jason       Create
2003/06/03  2.0         Jason       Add RTP Option
2004/10/08  2.0         ����      �������λ��嵥Ԫ��������Ӧ��������
2004/12/28  2.0         ����      ����SendTo�ķ���ֵ������ľ�������
2005/01/25  2.0         ����      SendToʱ������ЧĿ���ַ��Ͷ��
2005/02/03  2.0         ����      rtp��Ͷ��ʧ�ܺ��������Ͷ��
2005/05/14  3.6         ����        �޸����ñ��ص�ַ(ԭ��ʵ����ʹ�ñ��ص�ַ�������鲥)
2005/06/03  3.6         ����        ����ԭʼ��������ת������
******************************************************************************/



#include "kdvrtp.h"

extern s32   g_nDiscardSpan;        //���Ͷ�ģ�ⶪ��С���Ĳ������
extern s32   g_nRepeatSnd;            //�ش�����
extern s32   g_nShowDebugInfo;        //�Ƿ���ʾ���ص�һЩ����״̬��Ϣ
extern u32   g_dwAdditionMulticastIf;   //������鲥��ַ
extern BOOL32 g_bUseMemPool;

void RtpCallBackProc(u8 *pBuf, s32 nSize, void* pContext);

//��ʼ�����Ա
CKdvRtp::CKdvRtp()
{
    m_pRtcp     = NULL;
    m_dwSSRC    = 0;
    m_wSeqNum   = 0xFFFF;
    m_pSocket   = NULL;
    m_pAdditionMSocket = NULL;
    m_pLoopBuf  = NULL;
    m_pPackBuf  = NULL;
    m_pCallBack = NULL;
    m_pContext = NULL;

    m_pRlbPackStart = 0;
    m_pRlbPackEnd   = 0;
    m_pRlbBufStart  = 0;
    m_pRlbBufEnd    = 0;


    m_bRepeatSend = FALSE;
    m_atRLBPackets = NULL;
    m_pRSPackBuf= NULL;
    m_pRLBBuf = NULL;

    m_nRLBUnitNum  = 0;
    m_wRLBLastSN = 0;

    m_dwTotalPackNum = 0;

    m_hSynSem = NULL;
    m_pRawRtpTrans = NULL;

    //m_hSynSem ��ʼ���ź�
    OspSemBCreate( &m_hSynSem);

    memset(&m_tRemoteAddr, 0, sizeof(m_tRemoteAddr));
    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));
    for (u8 byNum = 0; byNum < RTP_FIXEDHEADER_SIZE/sizeof(u32); byNum++)
    {
        m_dwRtpHeader[byNum] = 0;
    }
}

//���Ա��������
CKdvRtp::~CKdvRtp()
{
    FreeBuf();
    if(m_hSynSem != NULL)
    {
        OspSemDelete(m_hSynSem);
        m_hSynSem = NULL;
    }
}

/*=============================================================================
    ������        ��Create
    ����        ����ʼ����������ͬ��Դ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwSSRC  ͬ��Դ
                   bAllocLPBuf �Ƿ���л��λ���ķ���
                               ��Ϊ���� �� H.264�Լ�mp4 ����ֱ�ӷ��ͣ���δʹ�û��λ���
                   bVidPayload ��Ƶ������Ƶ
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtp::Create( u32 dwSSRC, BOOL32 bAllocLPBuf /*=FALSE*/,
                     BOOL32 bVidPayload /*= FALSE*/, u8 byBuffMultipleTimes /*default value is 1*/)
{
    FreeBuf();
    if(dwSSRC == 0)
    {
        return ERROR_RTP_SSRC;
    }
    if(!byBuffMultipleTimes)
        return ERROR_CREATE_LOOP_BUF;

    m_dwSSRC  = dwSSRC;
      m_wSeqNum = 0xFFFF;

    m_pSocket  = new CKdvSocket();
    if(m_pSocket == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }
    if( TRUE == bVidPayload )
    {
        m_pSocket->SetSockMediaType( SOCKET_TYPE_VIDEO );
    }
    else
    {
        m_pSocket->SetSockMediaType( SOCKET_TYPE_AUDIO );
    }

    m_pSocket->SetCallBack(RtpCallBackProc, (void*)this );

    if(TRUE == bAllocLPBuf)
    {
        m_pLoopBuf = new CKdvLoopBuf;
        if(m_pLoopBuf == NULL)
        {
            FreeBuf();
            return ERROR_SND_MEMORY;
        }

        //������״����
        u16 wRet = m_pLoopBuf->Create(MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE, LOOP_BUF_UINT_NUM*byBuffMultipleTimes);
        if(wRet != LOOPBUF_NO_ERROR)
        {
            FreeBuf();
            return ERROR_LOOPBUF_CREATE;
        }
    }

    MEDIANET_MALLOC(m_pPackBuf, MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE);
    if(m_pPackBuf  == NULL)
    {
        FreeBuf();
        return ERROR_SND_MEMORY;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��FreeBuf
    ����        ����λ�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������

    ����ֵ˵���� ��
=============================================================================*/

void CKdvRtp::FreeBuf()
{
    SAFE_DELETE(m_pSocket)
    SAFE_DELETE(m_pAdditionMSocket)
    SAFE_DELETE(m_pLoopBuf)
    MEDIANET_SAFE_FREE(m_pPackBuf)

    memset(&m_tRemoteAddr, 0, sizeof(m_tRemoteAddr));
    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));
    m_pRtcp     = NULL;
    m_dwSSRC    = 0;
    m_dwTotalPackNum = 0;
    m_wSeqNum   = 0xFFFF;
    m_pCallBack = NULL;
    m_pContext = NULL;
    m_pRawRtpTrans = NULL;
    m_pRlbPackStart = NULL;
    m_pRlbPackEnd   = NULL;
    m_pRlbBufStart  = NULL;
    m_pRlbBufEnd    = NULL;

    MEDIANET_SEM_TAKE(m_hSynSem);
    MEDIANET_SAFE_FREE(m_pRSPackBuf)

    SYS_SAFE_FREE(m_pRLBBuf);
    if (m_atRLBPackets)
    {
        free(m_atRLBPackets);
        m_atRLBPackets = NULL;
    }
    m_bRepeatSend = FALSE;
    m_nRLBUnitNum  = 0;
    MEDIANET_SEM_GIVE(m_hSynSem);
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
void CKdvRtp::ResetSSRC(u32 dwSSRC)
{
    m_dwSSRC = dwSSRC;
}

/*=============================================================================
    ������        ��ResetSSRC
    ����        �����÷��Ͷ˶���mpeg4����H.264���õ��ش�����Ŀ���,�رպ�
                  �������Ѿ����͵����ݰ����л���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����bRepeatSnd  �Ƿ��ش�
                  wBufTimeSpan �ش����͵Ļ������Ļ���ʱ�䳤��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvRtp::ResetRSFlag(u16 wRLBUnitNum, BOOL32 bRepeatSnd)
{
    if(m_hSynSem == NULL)
    {
        return ERROR_CREATE_SEMAPORE;
    }

    MEDIANET_SEM_TAKE(m_hSynSem);

    m_bRepeatSend  = bRepeatSnd;

    if (bRepeatSnd)
    {
        u16 wRet = CreateRLB(wRLBUnitNum);
        if (KDVFAILED(wRet))
        {
            m_nRLBUnitNum = 0;  //����ʧ�ܣ������ش�����Ϊ0
            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_SND_MEMORY;
        }

        //�����ط���״����
        if (NULL == m_pRSPackBuf)
        {
            MEDIANET_MALLOC(m_pRSPackBuf, MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE);
            if(NULL == m_pRSPackBuf)
            {
                m_nRLBUnitNum = 0; //����ʧ�ܣ������ش�����Ϊ0
                MEDIANET_SEM_GIVE(m_hSynSem);
                return ERROR_SND_MEMORY;
            }
        }
    }

    MEDIANET_SEM_GIVE(m_hSynSem);

    return MEDIANET_NO_ERROR;
}


#define RLB_PACK_SIZE           (MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE)
#define MAX_PACKET_BUFF_NUM     128

u16 CKdvRtp::CreateRLB(s32 nUnitBufNum)
{
    if (nUnitBufNum < m_nRLBUnitNum)
    {
        return MEDIANET_NO_ERROR;
    }

    //����
    SYS_SAFE_FREE(m_pRLBBuf);
    if (m_atRLBPackets)
    {
        free(m_atRLBPackets);
        m_atRLBPackets = NULL;
    }

    //������Ҫ����Ϊ2����
    s32 i;
    for (i = 0; i < 7; i++)
    {
        if (nUnitBufNum <= (MAX_PACKET_BUFF_NUM << i))
            break;
    }
    nUnitBufNum = MAX_PACKET_BUFF_NUM << i;

    s32 nBufLen   = (RLB_PACK_SIZE) * nUnitBufNum; //������Ч���ȵ�Ԫ

    m_nRLBUnitNum = nUnitBufNum;

    //���������ݻ���
    SYS_MALLOC(m_pRLBBuf, nBufLen)

    //��¼��ַ�������Ų�Խ�����
    m_pRlbBufStart = m_pRLBBuf;
    m_pRlbBufEnd = m_pRLBBuf + nBufLen;

    //����һ��Ҫmemset
    memset(m_pRLBBuf, 0, nBufLen);
    //��������ϢArray
    m_atRLBPackets = (TLRSPacketInfo*)malloc(nUnitBufNum * sizeof(TLRSPacketInfo));

    //��¼��ַ�������Ų�Խ�����
    m_pRlbPackStart = m_atRLBPackets;
    m_pRlbPackEnd = m_atRLBPackets + nUnitBufNum*sizeof(TLRSPacketInfo);

    if ((NULL == m_pRLBBuf) ||
        (NULL == m_atRLBPackets))
    {
        SYS_SAFE_FREE(m_pRLBBuf);
        if (NULL == m_atRLBPackets)
        {
            free(m_atRLBPackets);
            m_atRLBPackets = NULL;
        }
        return ERROR_SND_MEMORY;
    }

    //��ʼ��Array
    u8* tmpBuf = m_pRLBBuf;
    for(i = 0; i < m_nRLBUnitNum; i++)
    {
        m_atRLBPackets[i].m_pbyBuff = tmpBuf;
        m_atRLBPackets[i].n_nDataSize = 0;
        m_atRLBPackets[i].m_dwTS = 0;
        m_atRLBPackets[i].m_wSN = 0;
        tmpBuf += RLB_PACK_SIZE;
    }

    m_wRLBLastSN = 0;

    return MEDIANET_NO_ERROR;
}



/*=============================================================================
    ������        ��SetRtcp
    ����        �����úͱ�RTP��Ӧ��RTCP����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtcp  RTCP�����ڲ����ڿ���RTP

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SetRtcp(CKdvRtcp *pRtcp)
{
    m_pRtcp = pRtcp;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        : SetCallBack
    ����        �������ϲ�ص�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pCallBackHandle    �ص�����ָ��
                   dwContext          �ص����û�����

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SetCallBack(PRCVPROC pCallBackHandle, void* pContext)
{
    m_pCallBack = pCallBackHandle;
    m_pContext = pContext;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��CheckPackAvailale
    ����        ���������͵����ݰ��ĺϷ���
                   ����LOOPBUF,��ֱ�ӷ��ͳ�ȥ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   ptRtpPack   RTP���ݰ��ṹ,�μ��ṹ����.
                   pnHeadLen   ����ʵ�ʵ�ͷ������, �����ж�Ԥ���ռ��Ƿ��㹻

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::CheckPackAvailabe(const TRtpPack *ptRtpPack, s32 *pnHeadLen)
{
    *pnHeadLen = RTP_FIXEDHEADER_SIZE;

    // SET RTP EXTENCE BITS;
    if(ptRtpPack->m_byExtence == 1)
    {
        if( (ptRtpPack->m_nExSize < 0))
        {
            return ERROR_LOOP_BUF_SIZE;
        }

        *pnHeadLen += sizeof(u32);
        *pnHeadLen += ptRtpPack->m_nExSize * sizeof(u32);
    }

    //���ȴ�С�ڲ����ƣ���������쳣
    if( (*pnHeadLen < 0) ||
        (ptRtpPack->m_nRealSize < 0) ||
        ((ptRtpPack->m_nRealSize+*pnHeadLen) > (MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE)) )
    {
        if (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "HeadLen=%d, RealSize=%d\n",
                      *pnHeadLen, ptRtpPack->m_nRealSize);
        }
        return ERROR_LOOP_BUF_SIZE;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��DirectSend
    ����        ����Ҫ���͵����ݼ���RTP FIXED HEADER ֱ�ӷ��ͳ�ȥ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pPackBuf    С������
                   nPackLen    С������
                   dwTimeStamp ��С����ʱ���

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::DirectSend(u8 *pPackBuf, s32 nPackLen, u32 dwTimeStamp)
{
    u16 wRet = MEDIANET_NO_ERROR;

    m_dwTotalPackNum++;

    //DIRECT SEND
    for(s32 i=0; i<m_tRemoteAddr.m_byNum; i++)
    {
        //���Ͷ�ģ�ⶪ��
        if( (g_nDiscardSpan > 0) &&
            ((m_dwTotalPackNum%g_nDiscardSpan) == 0) )
        {
        }
        else
        {
            if( (0 != m_tRemoteAddr.m_tAddr[i].m_dwIP) &&
                (0 != m_tRemoteAddr.m_tAddr[i].m_wPort))
            {
                if (0 == m_tRemoteAddr.m_tAddr[i].m_dwUserDataLen)
                {
                    wRet = m_pSocket->SendTo( pPackBuf, nPackLen,
                                              m_tRemoteAddr.m_tAddr[i].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[i].m_wPort);
                }
                else
                {
                    u8 abyBuff[1500+MAX_USERDATA_LEN];
                    memcpy(abyBuff,
                           m_tRemoteAddr.m_tAddr[i].m_abyUserData,
                           m_tRemoteAddr.m_tAddr[i].m_dwUserDataLen);

                    memcpy(abyBuff+m_tRemoteAddr.m_tAddr[i].m_dwUserDataLen,
                           pPackBuf,
                           nPackLen);

                    wRet = m_pSocket->SendTo( abyBuff,
                                              nPackLen+m_tRemoteAddr.m_tAddr[i].m_dwUserDataLen,
                                              m_tRemoteAddr.m_tAddr[i].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[i].m_wPort);
                }

                //�������鲥socket���з��� add by hual 2005-07-26
                if (!KDVFAILED(wRet))
                {
                    if ((NULL != m_pAdditionMSocket) &&
                        IsMultiCastAddr(m_tRemoteAddr.m_tAddr[i].m_dwIP))
                    {
                        wRet = m_pAdditionMSocket->SendTo(pPackBuf, nPackLen,
                            m_tRemoteAddr.m_tAddr[i].m_dwIP,
                            m_tRemoteAddr.m_tAddr[i].m_wPort );
                    }
                }
            }
            if(KDVFAILED(wRet))
            {
                if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(1, 0, "[CKdvRtp::DirectSend] Direct Send Error, ErrCode=%d   \n", wRet);
                }
                continue;
            }
        }

        //update RTCP Staus
        wRet = m_pRtcp->UpdateSend(nPackLen, dwTimeStamp);
        if(KDVFAILED(wRet))
        {
            continue;
        }
    }

    return wRet;
}

/*=============================================================================
    ������        ��SaveIntoLPBuf
    ����        ����Ҫ���͵����ݰ����뻷�λ��塣
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pPackBuf    С������
                   nPackLen    С������
                   bTrans     �Ƿ���͸��ת������

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SaveIntoLPBuf(u8 *pPackBuf, s32 nPackLen, BOOL32 bTrans)
{
    if(NULL != m_pLoopBuf)
    {
        //д��������λ���
        if(LOOPBUF_NO_ERROR != m_pLoopBuf->Write(pPackBuf, nPackLen))
        {
            return ERROR_LOOPBUF_FULL;
        }
    }
    else
    {
        return ERROR_LOOPBUF_FULL;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��Write
    ����        ����Ҫ���͵����ݼ���RTP FIXED HEADER
                   ����LOOPBUF,��ֱ�ӷ��ͳ�ȥ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tRtpPack   RTP���ݰ��ṹ,�μ��ṹ����.
                   bSend      �Ƿ�ֱ�ӷ��͡�
                   bTrans     �Ƿ���͸��ת������

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::Write(TRtpPack &tRtpPack, BOOL32 bSend/*=FALSE*/, BOOL32 bTrans/*=FALSE*/, BOOL32 bSendRtp/*TRUE*/)
{
    if(m_dwSSRC == 0|| m_pRtcp == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }
    u16 wRet = MEDIANET_NO_ERROR;
    s32 nHeadLen = 0;
    s32 nMove = 0;

    wRet = CheckPackAvailabe(&tRtpPack, &nHeadLen);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //�ж��Ƿ����㹻��Ԥ���ռ�.
    //û��
    if(tRtpPack.m_nPreBufSize < nHeadLen)
    {
        if (TRUE == bSendRtp)
        {
            // SET RTP FIXED HEADER
            m_dwRtpHeader[0] = 0;
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], 2, 30, 2);//version bit
            if( 0 != tRtpPack.m_byPadNum )
            {
                m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], 1, 29, 1);//padding bit
            }
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byExtence, 28, 1);//X bit
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byMark, 23, 1);//mark bit
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byPayload, 16, 7);//payload

            //��͸��ת��ʱ, ������кż�ʱ�������
            if(FALSE == bTrans)
            {
                m_wSeqNum++;
                m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], m_wSeqNum, 0, 16);
                m_dwRtpHeader[2] = m_dwSSRC; //SSRC bit
            }
            else
            {
                m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_wSequence, 0, 16);
                m_dwRtpHeader[2] = tRtpPack.m_dwSSRC; //SSRC bit
            }

            m_dwRtpHeader[1] = tRtpPack.m_dwTimeStamp;//timestamp bit
            //m_dwRtpHeader[2] = m_dwSSRC; //SSRC bit

            // order convert
            m_dwRtpHeader[0] = htonl( m_dwRtpHeader[0]);
            m_dwRtpHeader[1] = htonl( m_dwRtpHeader[1]);
            m_dwRtpHeader[2] = htonl( m_dwRtpHeader[2]);

            memcpy(m_pPackBuf, m_dwRtpHeader, sizeof(m_dwRtpHeader));
            nMove += sizeof(m_dwRtpHeader);

            // SET RTP EXTENCE BITS;
            if(tRtpPack.m_byExtence == 1)
            {
                u32 dwExHeader = 0;

                // set extence Header bit
                // ������ʵӦͳһΪ�ֽ����������ǵ�����ǰ�汾�ļ��ݣ���ʱ���Ķ�
                dwExHeader = SetBitField(dwExHeader, tRtpPack.m_nExSize, 0, 16);
                dwExHeader = htonl(dwExHeader);
                memcpy(m_pPackBuf + nMove, &dwExHeader, sizeof(u32));
                nMove += sizeof(u32);

                memcpy(m_pPackBuf + nMove, tRtpPack.m_pExData, tRtpPack.m_nExSize * sizeof(u32));
                nMove += tRtpPack.m_nExSize * sizeof(u32);
            }
        }
        //copy real data memory
        memcpy(m_pPackBuf+nMove, tRtpPack.m_pRealData, tRtpPack.m_nRealSize);

        //��㴦��
        /*
        // SET Padding Octor:
        if( 0 != tRtpPack.m_byPadNum )
        {
            //Ӧ�ð�������1�ֽڳ���
            *(m_pPackBuf+nMove+tRtpPack.m_nRealSize) = tRtpPack.m_byPadNum + RTP_PADDING_SIZE;
            tRtpPack.m_nRealSize += RTP_PADDING_SIZE;
        }
        */

        if(FALSE == bTrans)
        {
            //д���ط����λ���
            WriteRLB(m_wSeqNum, tRtpPack.m_dwTimeStamp, m_pPackBuf, nMove+tRtpPack.m_nRealSize);
        }

        if(bSend)
        {
            wRet = DirectSend(m_pPackBuf, nMove+tRtpPack.m_nRealSize, tRtpPack.m_dwTimeStamp);
        }
        else
        {
            wRet = SaveIntoLPBuf(m_pPackBuf, nMove+tRtpPack.m_nRealSize, bTrans);
        }
    }
    else
    {
        m_dwRtpHeader[0] = 0;
        m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], 2, 30, 2);//version bit
        if( 0 != tRtpPack.m_byPadNum )
        {
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], 1, 29, 1);//padding bit
        }
        m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byExtence, 28, 1);//X bit
        m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byMark, 23, 1);   //marker bit
        m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_byPayload, 16, 7);//payload bit

        //��͸��ת��ʱ, ������кż�ʱ�������
        if(FALSE == bTrans)
        {
            m_wSeqNum++;
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], m_wSeqNum, 0, 16);
        }
        else
        {
            m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], tRtpPack.m_wSequence, 0, 16);
        }

        m_dwRtpHeader[1] = tRtpPack.m_dwTimeStamp; //timestamp bit
        m_dwRtpHeader[2] = m_dwSSRC;               //SSRC bit

        //order convert
        m_dwRtpHeader[0] = htonl( m_dwRtpHeader[0]);
        m_dwRtpHeader[1] = htonl( m_dwRtpHeader[1]);
        m_dwRtpHeader[2] = htonl( m_dwRtpHeader[2]);

        //have space to save��reduce memory copy;
        u32 *pdwFixedHeader = NULL;
        if(tRtpPack.m_byExtence == 1)
        {
            pdwFixedHeader = (u32 *)(tRtpPack.m_pRealData -
                RTP_FIXEDHEADER_SIZE - EX_HEADER_SIZE -
                tRtpPack.m_nExSize * sizeof(u32));
        }
        else
        {
            pdwFixedHeader = (u32 *)(tRtpPack.m_pRealData - RTP_FIXEDHEADER_SIZE);
        }

        memcpy(pdwFixedHeader, m_dwRtpHeader, sizeof(m_dwRtpHeader));
        nMove += RTP_FIXEDHEADER_SIZE;

        // SET RTP EXTENCE BITS;
        if(tRtpPack.m_byExtence == 1)
        {
            u32 dwExHeader = 0;

            // set extence Header bit
            // ������ʵӦͳһΪ�ֽ����������ǵ�����ǰ�汾�ļ��ݣ���ʱ���Ķ�
            dwExHeader = SetBitField(dwExHeader,tRtpPack.m_nExSize, 0, 16);
            dwExHeader = htonl(dwExHeader);
            memcpy(((u8 *)pdwFixedHeader) + nMove, &dwExHeader, sizeof(u32));
            nMove += sizeof(u32);

            memcpy(((u8 *)pdwFixedHeader) + nMove, tRtpPack.m_pExData, tRtpPack.m_nExSize * sizeof(u32));
            nMove += tRtpPack.m_nExSize * sizeof(u32);
        }

        //here must not copy real data memory

        //��㴦��
        /*
        // SET Padding Octor:
        if( 0 != tRtpPack.m_byPadNum )
        {
            //Ӧ�ð�������1�ֽڳ���
            *((u8*)pdwFixedHeader+nMove+tRtpPack.m_nRealSize) = tRtpPack.m_byPadNum + RTP_PADDING_SIZE;
            tRtpPack.m_nRealSize += RTP_PADDING_SIZE;
        }
        */

        if(FALSE == bTrans)
        {
            //д���ط����λ���
            WriteRLB(m_wSeqNum, tRtpPack.m_dwTimeStamp, (u8 *)pdwFixedHeader, nMove+tRtpPack.m_nRealSize);
        }

        if(bSend)
        {
            wRet = DirectSend( (u8 *)pdwFixedHeader, nMove+tRtpPack.m_nRealSize,
                               tRtpPack.m_dwTimeStamp );
        }
        else
        {
            wRet = SaveIntoLPBuf((u8 *)pdwFixedHeader, nMove+tRtpPack.m_nRealSize, bTrans);
        }
    }

    return wRet;
}

/*=============================================================================
    ������        ��WriteRLB
    ����        ����Ҫ���͵����ݼ���ʱ���������С
                   �����ط����λ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pPacketBuf   RTP���ݰ�����ָ��.
                   nPacketSize  RTP���ݰ��ߴ硣

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::WriteRLB(u16 wSequence, u32 dwTimestamp, u8 *pPacketBuf, s32 nPacketSize)
{
    if(m_hSynSem == NULL)
    {
        return ERROR_CREATE_SEMAPORE;
    }

    MEDIANET_SEM_TAKE(m_hSynSem);

    if(FALSE == m_bRepeatSend || 0 == m_nRLBUnitNum || 0 == g_nRepeatSnd)
    {
        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NOCREATE;
    }

    u16 wRet = LOOPBUF_NO_ERROR;

    if( /*(nPacketSize < RTP_FIXEDHEADER_SIZE)  || */
        (nPacketSize > RLB_PACK_SIZE) ||
        (NULL == pPacketBuf) || nPacketSize < 0)
    {
        OspPrintf(TRUE, FALSE, "Save packet to RLB failed. nPacketSize=%d, PacketBuf=0x%x\n",
                  nPacketSize, pPacketBuf);

        wRet = ERROR_LOOP_BUF_PARAM;
    }
    else
    {
        s32 nPos = wSequence % m_nRLBUnitNum;
        TLRSPacketInfo* ptPackInfo;

        ptPackInfo = &m_atRLBPackets[nPos];
        //NULL����
        if(ptPackInfo == NULL)
        {
            OspPrintf(1,0,"[WriteRLB]ERROR ptPackInfo is NULL\n");

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }

        //Խ�籣��
        if(ptPackInfo < m_pRlbPackStart || ptPackInfo > m_pRlbPackEnd)
        {
            OspPrintf(1,0,"[WriteRLB]ERROR ptPackInfo=%p,m_pRlbPackStart=%p,m_pRlbPackEnd=%p,Nowm_atRlbPacks=%p,wSequence=%u\n", \
                           ptPackInfo, m_pRlbPackStart, m_pRlbPackEnd, m_atRLBPackets, wSequence);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }

        //null ����
        if(ptPackInfo->m_pbyBuff == NULL)
        {
            OspPrintf(1,0, "[WriteRLB]ERROR ptPackInfo->m_pbyBuff= %p \n", ptPackInfo->m_pbyBuff);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }
        //Խ�籣��
        if((ptPackInfo->m_pbyBuff) < m_pRlbBufStart || (ptPackInfo->m_pbyBuff) > m_pRlbBufEnd || (ptPackInfo->m_pbyBuff) + ptPackInfo->n_nDataSize > m_pRlbBufEnd)
        {
            OspPrintf(1,0, "[WriteRLB]ERROR, ptPackInfo->m_pbyBuff=%8x, m_RlbAddrStart=%p, m_RlbAddrEnd=%p, NOWm_RLBbuf=%p, datasize = %d, wSequence =%d\n", ptPackInfo->m_pbyBuff,\
                                    m_pRlbBufStart, m_pRlbBufEnd, m_pRLBBuf, ptPackInfo->n_nDataSize, wSequence);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }
        //��¼SN
        ptPackInfo->m_wSN = wSequence;
        //��¼ʱ���
        ptPackInfo->m_dwTS = dwTimestamp;
        //���ݰ�����
        ptPackInfo->n_nDataSize = nPacketSize;
        //����RTP���ݰ�
        memcpy(ptPackInfo->m_pbyBuff, pPacketBuf, nPacketSize);

        m_wRLBLastSN = wSequence;
    }

    MEDIANET_SEM_GIVE(m_hSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��ReadRLBBySN
    ����        ������ SN ���ط����λ�����ȡ����Ҫ�ط������а�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pBuf [out]         ����
                    nBufSize [out]     ���峤��
                    dwTimeStamp[in]    ������֡��ʱ���
                    wSeqNum[in]        ���İ�����
    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::ReadRLBBySN(u8 *pBuf, s32 &nBufSize, u32 dwTimeStamp, u16 wSeqNum)
{
    if(m_hSynSem == NULL)
    {
        return ERROR_CREATE_SEMAPORE;
    }
    if(NULL == pBuf)
    {
        return ERROR_LOOP_BUF_PARAM;
    }

    MEDIANET_SEM_TAKE(m_hSynSem);

    if(FALSE == m_bRepeatSend || 0 == m_nRLBUnitNum || 0 == g_nRepeatSnd)
    {
        MEDIANET_SEM_GIVE(m_hSynSem);
        nBufSize = 0;
        return ERROR_LOOP_BUF_NOCREATE;
    }


    s32 nPos = wSeqNum % m_nRLBUnitNum;

    TLRSPacketInfo* ptPackInfo = &m_atRLBPackets[nPos];

    //NULL����
    if(ptPackInfo == NULL)
    {
        OspPrintf(1,0,"[ReadRLBBySN]ERROR ptPackInfo is NULL\n");

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }

    //Խ�籣��
    if(ptPackInfo < m_pRlbPackStart || ptPackInfo > m_pRlbPackEnd)
    {
        OspPrintf(1,0,"[ReadRLBBySN]ERROR ptPackInfo=%p,m_pRlbPackStart=%p,m_pRlbPackEnd=%p,m_atRLBPackets=%p,seq =%u\n", \
                       ptPackInfo, m_pRlbPackStart, m_pRlbPackEnd, m_atRLBPackets, wSeqNum);

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    if (wSeqNum != ptPackInfo->m_wSN)
    {
        MEDIANET_SEM_GIVE(m_hSynSem);

        if (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[ReadRLBBySN:FAIL] Acquire wSeqNum=%d, Real nSequence=%u\n",
                      wSeqNum, ptPackInfo->m_wSN);
        }
        nBufSize = 0;
        return ERROR_LOOP_BUF_NULL;
    }

    if (dwTimeStamp != 0 && dwTimeStamp != ptPackInfo->m_dwTS)
    {
        MEDIANET_SEM_GIVE(m_hSynSem);

        if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[ReadRLBBySN:FAIL] Acquire dwTimeStamp=%u, Real dwTimeStamp=%u\n",
                dwTimeStamp, ptPackInfo->m_dwTS);
        }

        nBufSize = 0;
        return ERROR_LOOP_BUF_NULL;
    }

    //ȡ��RTP���ݰ�
    //��С����
    if( ptPackInfo->n_nDataSize > RLB_PACK_SIZE || ptPackInfo->n_nDataSize < 0)
    {
        OspPrintf(1,0, "[ReadRLBBySN]ERROR ptPackInfo->n_nDataSize > RLB_PACK_SIZE or < 0\n");

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    //null ����
    if(pBuf == NULL || ptPackInfo->m_pbyBuff == NULL)
    {
        OspPrintf(1,0, "[ReadRLBBySN]ERROR pbuf=%p,ptPackInfo->m_pbyBuff=%p\n", pBuf, ptPackInfo->m_pbyBuff);

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    //Խ�籣��
    if((ptPackInfo->m_pbyBuff) < m_pRlbBufStart || (ptPackInfo->m_pbyBuff) > m_pRlbBufEnd || (ptPackInfo->m_pbyBuff) + ptPackInfo->n_nDataSize > m_pRlbBufEnd)
    {
        OspPrintf(1,0, "[ReadRLBBySN]ERROR,ptPackInfo->m_pbyBuff=%p,m_pRlbBufStart=%p,m_pRlbBufEnd=%p,NOWm_RLBbuf=%p,size=%d,SN=%u\n", (ptPackInfo->m_pbyBuff),\
                                m_pRlbBufStart, m_pRlbBufEnd, m_pRLBBuf, ptPackInfo->n_nDataSize, wSeqNum);

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    memcpy(pBuf, ptPackInfo->m_pbyBuff, ptPackInfo->n_nDataSize);
    nBufSize = ptPackInfo->n_nDataSize;

    if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        u8 byPayload;
        u32 dwHeader0;
        memcpy(&dwHeader0, ptPackInfo->m_pbyBuff, sizeof(dwHeader0));

        dwHeader0 = ntohl(dwHeader0);
        byPayload = (u8)GetBitField(dwHeader0, 16, 7);//payload
        OspPrintf(TRUE, FALSE, "[ReadRLBBySN:%d] Acquire wSeqNum=%u OK\n",
                         byPayload, wSeqNum);
    }

    MEDIANET_SEM_GIVE(m_hSynSem);

    return LOOPBUF_NO_ERROR;
}

/*=============================================================================
    ������        ��FindRLBByTS
    ����        ������ TIMESTAMP ���ط����λ�����ȡ����Ҫ�ط�����ʼ���а�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pBuf [out]         ����
                    byPackNum [out]    �ܱ���
                    dwTimeStamp[in]    ������֡��ʱ���
    ����ֵ˵����-1 û�ҵ����ɹ�������ʼ���а�
=============================================================================*/

#define PREVPACKPOS(a) (((a) > 0) ? ((a)-1) : (m_nRLBUnitNum-1))

//��ȡָ��ʱ��������һ��
s32 CKdvRtp::FindRLBByTS(u32 dwTimeStamp)
{
    if(m_hSynSem == NULL)
    {
        return -1;
    }

    MEDIANET_SEM_TAKE(m_hSynSem);

    if (FALSE == m_bRepeatSend || 0 == m_nRLBUnitNum || 0 == g_nRepeatSnd)
    {
        MEDIANET_SEM_GIVE(m_hSynSem);
        return -1;
    }

    s32 nLastPos = m_wRLBLastSN % m_nRLBUnitNum;
    s32 nStartPos = nLastPos;

    //���ǵ�����ʱ��һ�����õñȽϳ���������������
    do
    {
        if (m_atRLBPackets[nStartPos].m_dwTS == dwTimeStamp)
        {
            s32 nSN = m_atRLBPackets[nStartPos].m_wSN;
            MEDIANET_SEM_GIVE(m_hSynSem);
            return nSN;
        }

        nStartPos = PREVPACKPOS(nStartPos);

    } while(nStartPos != nLastPos);

    MEDIANET_SEM_GIVE(m_hSynSem);

    if (2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        OspPrintf(TRUE, FALSE, "[FindRLBByTS:FAIL] Acquire dwTimeStamp=%u\n",
                  dwTimeStamp);
    }

    return -1;
}

/*=============================================================================
    ������        ��DealRSQBackQuest
    ����        �������ط����󣬴��ط����λ�����ȡ�������ط���С���ط���ȥ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����ptRSQ �ط�����ṹָ��
    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::DealRSQBackQuest(TRtcpSDESRSQ *ptRSQ)
{
    if(g_nRepeatSnd == 0)
    {
        return MEDIANET_NO_ERROR;
    }

    if(FALSE == m_bRepeatSend || NULL == m_pSocket || 0 == g_nRepeatSnd)
    {
        return ERROR_LOOP_BUF_NOCREATE;
    }

    u16 wRet = MEDIANET_NO_ERROR;

    s32 nRSPackSize = 0;

    if(SN_RSQ_TYPE == ptRSQ->m_byRSQType)
    {
        u8  byPackNum = 0;
        byPackNum = ptRSQ->m_byPackNum;

        if(byPackNum >= MAX_PACK_NUM)
        {
            return wRet;
        }
        if(byPackNum == 0)
        {
            byPackNum = MAX_PACK_NUM;
        }

        BOOL32  bRS = 0; //�Ƿ��ط�
        u32 *pdwMaskBit = (u32*)ptRSQ->m_byMaskBit;

        //order rs rtcp convert
        ptRSQ->m_dwTimeStamp = ntohl(ptRSQ->m_dwTimeStamp);
        ptRSQ->m_wStartSeqNum = ntohs(ptRSQ->m_wStartSeqNum);
        s32 nSize = MAX_PACK_NUM / (8*sizeof(u32));
        ConvertN2H( (u8*)pdwMaskBit, 0, nSize);

        s32 nMuti = 0;
        s32 nResi = 0;

        //u32 dwTimeStamp = OspTickGet();

        for(s32 nOffset=0; nOffset<byPackNum; nOffset++)
        {
            //�����ط�����֡�и������ط�����λ��00010101 ��1����Ҫ�ط�, 0�����ط���
            nMuti = nOffset/32;
            nResi = nOffset%32;
            bRS = (BOOL32)GetBitField(pdwMaskBit[nMuti], nResi, 1);
            if(FALSE == bRS)
            {
                continue;
            }
            //read a pack
            if( LOOPBUF_NO_ERROR != ReadRLBBySN( m_pRSPackBuf, nRSPackSize,
                                                 ptRSQ->m_dwTimeStamp,
                                                 (ptRSQ->m_wStartSeqNum+nOffset) ) )
            {
                break;
            }

            s32 nSndNum = 0;
            //send all remote peer
            for(s32 j=0; j<m_tRemoteAddr.m_byNum; j++)
            {
                if( (0 != m_tRemoteAddr.m_tAddr[j].m_dwIP) &&
                    (0 != m_tRemoteAddr.m_tAddr[j].m_wPort) )
                {
                    if (0 == m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen)
                    {
                        wRet = m_pSocket->SendTo( m_pRSPackBuf, nRSPackSize,
                                                  m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                                  m_tRemoteAddr.m_tAddr[j].m_wPort );

                    }
                    else
                    {
                        u8 abyBuff[1500+MAX_USERDATA_LEN];
                        memcpy(abyBuff,
                                m_tRemoteAddr.m_tAddr[j].m_abyUserData,
                                m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen);

                        memcpy(abyBuff+m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                                m_pRSPackBuf,
                                nRSPackSize);

                        wRet = m_pSocket->SendTo( abyBuff,
                                                  nRSPackSize + m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                                                  m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                                  m_tRemoteAddr.m_tAddr[j].m_wPort );
                    }
                }
                if(KDVFAILED(wRet))
                {
                    continue;
                }
            }
        }
    }
    if(TIMESTAMP_RSQ_TYPE == ptRSQ->m_byRSQType)
    {
        //u32 dwTimeStamp = OspTickGet();

        //order rs rtcp convert
        ptRSQ->m_dwTimeStamp = ntohl(ptRSQ->m_dwTimeStamp);

        s32 nRet = FindRLBByTS(ptRSQ->m_dwTimeStamp);

        if(nRet < 0 || nRet > 0xFFFF)
        {
            return wRet;
        }

        u16 wSequence = nRet;
        s32 nPackRendNum = 0;

        //�Ӻ���ǰ����
        while (nPackRendNum < MAX_PACK_NUM)
        {
            //�����ط�����֡��ʱ�����������������Ӧ���ݰ�
            if( LOOPBUF_NO_ERROR != ReadRLBBySN( m_pRSPackBuf, nRSPackSize,
                                                 ptRSQ->m_dwTimeStamp, wSequence ) )
            {
                break;
            }

            s32 nSndNum = 0;
            //send all remote peer
            for(s32 j=0; j<m_tRemoteAddr.m_byNum; j++)
            {

                if( (0 != m_tRemoteAddr.m_tAddr[j].m_dwIP) &&
                    (0 != m_tRemoteAddr.m_tAddr[j].m_wPort) )
                {
                    wRet = m_pSocket->SendTo( m_pRSPackBuf, nRSPackSize,
                                              m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[j].m_wPort );
                }
                if(KDVFAILED(wRet))
                {
                    if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                    {
                        OspPrintf(1, 0, "[CKdvRtp::DealRSQBackQuest] Direct Send By Rtp TIMESTAMP_RSQ_TYPE Error, ErrCode=%d   \n", wRet);
                    }
                    continue;
                }
            }

            nPackRendNum++;
            wSequence--; //�Ӻ���ǰ

            //���ж�һ�£���ֹReadRLBBySN�д�ӡ����ʧ����Ϣ
            if (ptRSQ->m_dwTimeStamp != m_atRLBPackets[wSequence%m_nRLBUnitNum].m_dwTS)
            {
                break;
            }
        }

        OspPrintf(1, 0, "[TIMESTAMP_RSQ]: timestamp=%u resend pack num=%d\n",
                  ptRSQ->m_dwTimeStamp,nPackRendNum);
    }

    return wRet;
}

/*=============================================================================
    ������        ��SendByPackNum
    ����        ����LOOPBUF��ȡ��ָ���������ͳ�ȥ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   nPackNum Ҫ���͵İ���

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SendByPackNum(s32 nPackNum)
{
    if(m_pLoopBuf == NULL || m_pSocket == NULL || m_pRtcp == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    u16 wRet = MEDIANET_NO_ERROR;
    BOOL32 bSendErr = FALSE; //���η����Ƿ��й�ʧ��
    s32 nPackSize   = 0;

    for(s32 i=0; i<nPackNum; i++)
    {
        //read a pack
        if(LOOPBUF_NO_ERROR != m_pLoopBuf->Read(m_pPackBuf,nPackSize))
        {
            return MEDIANET_NO_ERROR;
        }

        m_dwTotalPackNum++;

        //���Ͷ�ģ�ⶪ��
        if(g_nDiscardSpan > 0 &&
          (m_dwTotalPackNum%g_nDiscardSpan) == 0)
        {
            continue;
        }

        s32 nSndNum = 0;
        //send all remote peer
        for(s32 j=0; j<m_tRemoteAddr.m_byNum; j++)
        {

            if( (0 != m_tRemoteAddr.m_tAddr[j].m_dwIP) &&
                (0 != m_tRemoteAddr.m_tAddr[j].m_wPort) )
            {
                if (0 == m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen)
                {
                    wRet = m_pSocket->SendTo( m_pPackBuf, nPackSize,
                                          m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                          m_tRemoteAddr.m_tAddr[j].m_wPort );
                }
                else
                {
                    u8 abyBuff[1500+MAX_USERDATA_LEN];
                    memcpy(abyBuff,
                            m_tRemoteAddr.m_tAddr[j].m_abyUserData,
                            m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen);

                    memcpy(abyBuff+m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                            m_pPackBuf,
                            nPackSize);

                    wRet = m_pSocket->SendTo( abyBuff,
                                              nPackSize+m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                                              m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[j].m_wPort );
                }

                //�������鲥socket���з��� add by hual 2005-07-26
                if (!KDVFAILED(wRet))
                {
                    if ((NULL != m_pAdditionMSocket) &&
                        IsMultiCastAddr(m_tRemoteAddr.m_tAddr[j].m_dwIP))
                    {
                        wRet = m_pAdditionMSocket->SendTo(m_pPackBuf, nPackSize,
                            m_tRemoteAddr.m_tAddr[j].m_dwIP,
                            m_tRemoteAddr.m_tAddr[j].m_wPort );
                    }
                }
            }
            if(KDVFAILED(wRet))
            {
                if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(1, 0, "[CKdvRtp::Send] Write And Then Send Rtp Error, ErrCode=%d   \n", wRet);
                }
                continue;
            }

            //update RTCP Staus
            u32 dwTimeStamp = *((u32 *)(m_pPackBuf + sizeof(u32)));
            dwTimeStamp     = ntohl(dwTimeStamp);
            wRet = m_pRtcp->UpdateSend(nPackSize, dwTimeStamp);
            if(KDVFAILED(wRet))
            {
                continue;
            }
        }
     }

    if( TRUE == bSendErr )
    {
        wRet = ERROR_SND_SEND_UDP;
    }
    return wRet;
}

u16 CKdvRtp::SendUserDefinedBuff(u8 *pbyBuf, u16 wBufLen, u32 dwPeerAddr, u16 wPeerPort)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
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
    ������        ��SendBySize
    ����        ����LOOPBUF��ȡ��ָ���ֽ������ͳ�ȥ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   nTotalPackSize Ҫ���͵��ֽ���

    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SendBySize(s32 nTotalPackSize)
{
    if(m_pLoopBuf == NULL|| m_pSocket == NULL|| m_pRtcp == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    u16 wRet = MEDIANET_NO_ERROR;
    BOOL32 bSendErr = FALSE; //���η����Ƿ��й�ʧ��
    s32 nPackSize   = 0;
    s32 nCurTotalSize = 0;   //��ǰ�Ѿ����͵�С�����ֽ���

    //����ָ����С���ֽ��� ��С��
    while( nCurTotalSize < nTotalPackSize )
    {
        //read a pack
        if(LOOPBUF_NO_ERROR != m_pLoopBuf->Read(m_pPackBuf, nPackSize))
        {
            return MEDIANET_NO_ERROR;
        }

        m_dwTotalPackNum++;
        nCurTotalSize += nPackSize;

        //���Ͷ�ģ�ⶪ��
        if(g_nDiscardSpan > 0 &&
          (m_dwTotalPackNum%g_nDiscardSpan) == 0)
        {
            continue;
        }

        s32 nSndNum = 0;
        //send all remote peer
        for(s32 j=0; j<m_tRemoteAddr.m_byNum; j++)
        {

            if( (0 != m_tRemoteAddr.m_tAddr[j].m_dwIP) &&
                (0 != m_tRemoteAddr.m_tAddr[j].m_wPort) )
            {
                if (0 == m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen)
                {
                    wRet = m_pSocket->SendTo( m_pPackBuf, nPackSize,
                                              m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[j].m_wPort );
                }
                else
                {
                    u8 abyBuff[1500+MAX_USERDATA_LEN];
                    memcpy(abyBuff,
                            m_tRemoteAddr.m_tAddr[j].m_abyUserData,
                            m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen);

                    memcpy(abyBuff+m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                           m_pPackBuf,
                           nPackSize);
                    wRet = m_pSocket->SendTo( abyBuff,
                                              nPackSize + m_tRemoteAddr.m_tAddr[j].m_dwUserDataLen,
                                              m_tRemoteAddr.m_tAddr[j].m_dwIP,
                                              m_tRemoteAddr.m_tAddr[j].m_wPort );
                }

                //�������鲥socket���з��� add by hual 2005-07-26
                if (!KDVFAILED(wRet))
                {
                    if ((NULL != m_pAdditionMSocket) &&
                        IsMultiCastAddr(m_tRemoteAddr.m_tAddr[j].m_dwIP))
                    {
                        wRet = m_pAdditionMSocket->SendTo(m_pPackBuf, nPackSize,
                            m_tRemoteAddr.m_tAddr[j].m_dwIP,
                            m_tRemoteAddr.m_tAddr[j].m_wPort );
                    }
                }
            }
            if(KDVFAILED(wRet))
            {
                if(2 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                {
                    OspPrintf(1, 0, "[CKdvRtp::Send] Write And Then Send Rtp Error, ErrCode=%d   \n", wRet);
                }
                continue;
            }

            //update RTCP Status
            u32 dwTimeStamp = *((u32 *)(m_pPackBuf + sizeof(u32)));
            dwTimeStamp     = ntohl(dwTimeStamp);
            wRet = m_pRtcp->UpdateSend(nPackSize, dwTimeStamp);
            if(KDVFAILED(wRet))
            {
                return wRet;
            }
        }
     }

    //�˶δ�������
    if( TRUE == bSendErr )
    {
        wRet = ERROR_SND_SEND_UDP;
    }

    return wRet;
}

/*=============================================================================
    ������        ��DealData
    ����        ���������紫����������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pBuf   ���ݻ���
                   nSize  ���ݴ�С
    ����ֵ˵������
=============================================================================*/
void CKdvRtp::DealData(u8 *pBuf, s32 nSize)
{
    if( (NULL == pBuf) || (nSize < RTP_FIXEDHEADER_SIZE) )
    {
        return;
    }

    //check if need to del userhead, add by hual 2006-08-26
    //���ǵ����ܴ��ڴ���ͷǴ�����ʹ�õ������Ŀǰ���ж�ͷ�ӽڵ������λ�Ƿ�ΪRTP�İ汾��
    if (m_tLocalAddr.m_dwUserDataLen > 0  && (u32)nSize > m_tLocalAddr.m_dwUserDataLen &&
        (((*pBuf)&0xC0) != 0x80))
    {
        //���ڽ���ʱͷ�������д���ǶԶ˵�Դ��ַ��Ŀǰ������ƥ��
        //if (0 == memcmp(pBuf, m_tLocalAddr.m_dwUserDataLen, m_tLocalAddr.m_dwUserDataLen))
        {
            pBuf += m_tLocalAddr.m_dwUserDataLen;
            nSize -= m_tLocalAddr.m_dwUserDataLen;
        }
    }

    u32 *header=(u32*)pBuf;

    //ԭʼ���ݰ�ת��
    if (NULL != m_pRawRtpTrans)
    {
        m_pRawRtpTrans(pBuf, nSize);
    }

    /*///test 2222//////////////////////////////////////////////////////////////////////////

    //���rtp����ת����������������
    if(MEDIA_TYPE_H263PLUS == tRtpPack.m_byPayload)
    {
        m_tRemoteAddr.m_byNum = 1;
        m_tRemoteAddr.m_tAddr[0].m_dwIP = (u32)(inet_addr("172.16.24.162"));
        m_tRemoteAddr.m_tAddr[0].m_wPort = 2330;


        Write(tRtpPack, TRUE, TRUE);

        return;
    }

    /*///test 2222//////////////////////////////////////////////////////////////////////////

    TRtpPack tRtpPack;
    memset(&tRtpPack, 0, sizeof(tRtpPack));

    //RTP fixed Header Convert
    ConvertN2H(pBuf, 0, 3);
    //CSRC convert
    ConvertN2H(pBuf, 3, GetBitField(header[0], 24, 4));
    tRtpPack.m_dwTimeStamp = header[1];
    u8 m_byVertion  = (u8)GetBitField(header[0], 30, 2);
    if (m_byVertion != 2)//rtp���汾��Ϊ2����Ϊ2ʱΪ��rtp��
    {
        if (255 == g_nShowDebugInfo)
        {
            OspPrintf(TRUE, FALSE, "[medianet]not rtp! \n");
        }
        return;
    }
    tRtpPack.m_wSequence   = (u16)GetBitField(header[0], 0, 16);
    tRtpPack.m_dwSSRC      = header[2];
    tRtpPack.m_byMark      = (u8)GetBitField(header[0], 23, 1);
    tRtpPack.m_byPayload   = (u8)GetBitField(header[0], 16, 7);

    s32 nOffSet            = RTP_FIXEDHEADER_SIZE +
                             GetBitField(header[0], 24, 4) * sizeof(u32);//CSRC
    if(nSize < nOffSet) //rtpͷ �쳣���
    {
            if(255 == g_nShowDebugInfo)
            {
                OspPrintf(1,0, "rtpheadr(12) + csrclen(%d) > total rtp size(%d), error, \n", GetBitField(header[0], 24, 4) * sizeof(u32), nSize);
            }
            return;
    }

    tRtpPack.m_nRealSize   = nSize - nOffSet;
    tRtpPack.m_pRealData   = pBuf + nOffSet;
    tRtpPack.m_byExtence   = (u8)GetBitField(header[0], 28, 1);

    //��չͷ �� ������ Fixed Header����, Ŀǰ���ڴ����Զ����mp4��mp3��
    if (tRtpPack.m_byExtence)/*Extension Bit Set*/
    {
        if(tRtpPack.m_nRealSize < EX_HEADER_SIZE+MIN_PACK_EX_LEN)
        {
            if(7 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
            {
                OspPrintf(1, 0, "[CKdvRtp::DealData] RTP REALSIZE EXCEPTION:m_nRealSize=%d , Sequence = %d In Extence   \n", tRtpPack.m_nRealSize, tRtpPack.m_wSequence);
            }
            return;
        }
        s32 xStart = nOffSet/sizeof(u32);
        ConvertN2H(pBuf, xStart, 1);
        tRtpPack.m_nExSize    = (u16)GetBitField(header[xStart], 0, 16);
        if((xStart+1 + tRtpPack.m_nExSize) * sizeof(u32) < nSize && tRtpPack.m_nRealSize > ((tRtpPack.m_nExSize + 1)*sizeof(u32))) //rtpͷ �쳣���
            {
                tRtpPack.m_pExData    = pBuf + (xStart+1) * sizeof(u32);
            tRtpPack.m_nRealSize -= ((tRtpPack.m_nExSize + 1)*sizeof(u32));
            tRtpPack.m_pRealData += ((tRtpPack.m_nExSize + 1)*sizeof(u32));
        }
        else
        {
            if(255 == g_nShowDebugInfo)
            {
                OspPrintf(1,0, "rtp exdata Start (%d)+ exsize(%d) > rtp total size(%d), error\n", (xStart+1) *sizeof(u32), tRtpPack.m_nExSize*sizeof(u32), nSize);
            }
            return;
        }

    }

    //Padding Bit Set
    if (GetBitField(header[0], 29, 1))
    {
                //�۳�padding����ֶ�
                tRtpPack.m_byPadNum   = *(pBuf+nSize-RTP_PADDING_SIZE);
		if( 0 != tRtpPack.m_byPadNum )
		{
			tRtpPack.m_nRealSize -= tRtpPack.m_byPadNum;
		}
    }
    
	if (tRtpPack.m_nRealSize < 0 || tRtpPack.m_nRealSize > MAX_RCV_PACK_SIZE)
	{
        if(1 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvRtp::DealData] RTP REALSIZE EXCEPTION:m_nRealSize=%d   \n", tRtpPack.m_nRealSize);
        }
        return;
    }

    if(7 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        OspPrintf(1, 0, "[CKdvRtp::DealData] RTP Info:m_nRealSize=%d, Padding Bit=%d, m_byExtence=%d, m_dwTimeStamp=%d, m_wSequence=%d, m_byPayload=%d, m_dwSSRC=%d, m_byMark=%d   \n",
                         tRtpPack.m_nRealSize, GetBitField(header[0], 29, 1), tRtpPack.m_byExtence, tRtpPack.m_dwTimeStamp, tRtpPack.m_wSequence, tRtpPack.m_byPayload, tRtpPack.m_dwSSRC, tRtpPack.m_byMark);
    }

    if(m_pCallBack != NULL)
    {
        m_pCallBack(&tRtpPack, m_pContext);
    }
    else
    {
        if(1 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[CKdvRtp::DealData] m_pCallBack == NULL   \n");
        }
    }

    if(m_pRtcp != NULL)
    {
        m_pRtcp->UpdateRcv(tRtpPack.m_dwSSRC, OspTickGet(), tRtpPack.m_dwTimeStamp,
                           tRtpPack.m_wSequence);
    }
}

/*=============================================================================
    ������        ��SetLocalAddr
    ����        �����õ���socket ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwIp   local IP(net order)
                   wPort  local Port(machine order)
                   bRcv   receive or not
    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SetLocalAddr(u32 dwIp, u16 wPort, BOOL32 bRcv, u32 dwUserDataLen, u8* pbyUserData, u32 dwFlag, void* pRegFunc, void* pUnregFunc)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }
/*
    if(0 == wPort)
    {
        return ERROR_SND_CREATE_SOCK;
    }
*/

    //the same to last set
    if(wPort != 0 && dwIp == m_tLocalAddr.m_dwIP &&
       wPort == m_tLocalAddr.m_wPort &&
       dwUserDataLen == m_tLocalAddr.m_dwUserDataLen &&
       (0 == memcmp(pbyUserData, m_tLocalAddr.m_abyUserData, dwUserDataLen)))
    {
        return MEDIANET_NO_ERROR;
    }

    // change user data only
    if (wPort != 0 &&
        dwIp == m_tLocalAddr.m_dwIP &&
        wPort == m_tLocalAddr.m_wPort)
    {
        if (dwUserDataLen < 0 || dwUserDataLen > MAX_USERDATA_LEN)
        {
            return ERROR_SET_USERDATA;
        }

        if (!m_pSocket->SetLocalUserData(dwUserDataLen, pbyUserData))
        {
            return ERROR_SET_USERDATA;
        }

        m_tLocalAddr.m_dwUserDataLen = dwUserDataLen;
        memcpy(m_tLocalAddr.m_abyUserData, pbyUserData, m_tLocalAddr.m_dwUserDataLen);
        return MEDIANET_NO_ERROR;
    }

    BOOL32 bRet;
    if (bRcv && (IsMultiCastAddr(dwIp) || IsBroadCastAddr(dwIp)))
    {
        //����ǽ��գ��������鲥��㲥���������鲥��ַ
        bRet = m_pSocket->Create(SOCK_DGRAM, wPort, 0, dwIp, bRcv, dwFlag, pRegFunc, pUnregFunc);
    }
    else
    {
        bRet = m_pSocket->Create(SOCK_DGRAM, wPort, dwIp, 0, bRcv, dwFlag, pRegFunc, pUnregFunc);
    }

    //create local socket ,no bind local ip
    if(!bRet)
    {
        return ERROR_SND_CREATE_SOCK;
    }

    m_tLocalAddr.m_dwIP  = dwIp;
    m_tLocalAddr.m_wPort = wPort;

    if (dwUserDataLen >= 0 && dwUserDataLen <= MAX_USERDATA_LEN)
    {
        m_tLocalAddr.m_dwUserDataLen = dwUserDataLen;
        memcpy(m_tLocalAddr.m_abyUserData, pbyUserData, m_tLocalAddr.m_dwUserDataLen);
    }
    else
    {
        return ERROR_SET_USERDATA;
    }

    bRet = m_pSocket->SetLocalUserData(dwUserDataLen, pbyUserData);
    if(!bRet)
    {
        return ERROR_SET_USERDATA;
    }

    return MEDIANET_NO_ERROR;
}


u16  CKdvRtp::GetLocalAddr(u32* pdwIp, u16* pwPort)
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

u16 CKdvRtp::RemoveLocalAddr()
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    m_pSocket->Close(TRUE);

    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));

    return MEDIANET_NO_ERROR;
}

u16 CKdvRtp::SetSrcAddr(u32 dwSrcIP, u16 wSrcPort)
{
    if(m_pSocket == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    m_pSocket->SetSrcAddr(dwSrcIP, wSrcPort);

    return MEDIANET_NO_ERROR;
}

u16 CKdvRtp::GetSndSocketInfo(TKdvSndSocketInfo &tSocketInfo)
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
u16 CKdvRtp::SetMCastOpt(u32 dwLocalIP, u32 dwMCastIP, BOOL32 bRcv /*=FALSE*/)
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

    //add by hual 2005-07-26
    //������鲥��ַ
    if ((wRet == MEDIANET_NO_ERROR) &&
        (!bRcv) &&
        (g_dwAdditionMulticastIf != 0) &&
        (g_dwAdditionMulticastIf != dwLocalIP))
    {
        if (NULL == m_pAdditionMSocket)
        {
            m_pAdditionMSocket  = new CKdvSocket();
            if(m_pAdditionMSocket == NULL)
            {
                return ERROR_SND_MEMORY;
            }

            if(!m_pAdditionMSocket->Create(SOCK_DGRAM, m_tLocalAddr.m_wPort,
                                g_dwAdditionMulticastIf, dwMCastIP, bRcv))
            {
                SAFE_DELETE(m_pAdditionMSocket);
                return ERROR_SND_CREATE_SOCK;
            }
        }

        if(TRUE == m_pAdditionMSocket->SetMCastOpt(g_dwAdditionMulticastIf, dwMCastIP, bRcv))
        {
            wRet = MEDIANET_NO_ERROR;
        }
        else
        {
            OspPrintf(1, 0, "[CKdvRtp::SetMCastOpt] Set Addition Broadcast Error, ErrCode=%d\n", wRet);
        }
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
u16 CKdvRtp::SetBCastOpt( )
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

u16 CKdvRtp::InputRtpPack(u8 *pRtpBuf, u32 dwRtpBuf)
{
    DealData(pRtpBuf, dwRtpBuf);
    return MEDIANET_NO_ERROR;
}
/*=============================================================================
    ������        ��SetRemoteAddr
    ����        ������Ҫ���͵���Զ�˵�ַ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   TRemoteAddr Զ�˵�ַ�ṹ���μ��ṹ����
    ����ֵ˵�����μ������붨��
=============================================================================*/
u16 CKdvRtp::SetRemoteAddr(TRemoteAddr &tRemoteAddr)
{
    m_tRemoteAddr = tRemoteAddr;
    return MEDIANET_NO_ERROR;
}

u16 CKdvRtp::GetRemoteAddr(TRemoteAddr &tRemoteAddr)
{
    tRemoteAddr = m_tRemoteAddr;
    return MEDIANET_NO_ERROR;
}

void CKdvRtp::ResetSeq()
{
    /*//65535 -> 1
    if(0xFFFF == m_wSeqNum)
    {
        m_wSeqNum = 1;
    }
    else
    {
        m_wSeqNum++;
    }*/
    //65535 -> 0
    m_wSeqNum++;

    /*
    //���ط�������в���հ����Կճ�λ�ã���SN�⣬������Ϣȡ����һ�η���
    m_dwRtpHeader[0] = ntohl(m_dwRtpHeader[0]);
    m_dwRtpHeader[0] = SetBitField(m_dwRtpHeader[0], m_wSeqNum, 0, 16);
    m_dwRtpHeader[0] = htonl( m_dwRtpHeader[0]);

    WriteRLB((u8*)m_dwRtpHeader, sizeof(m_dwRtpHeader));
    */
}

void CKdvRtp::SetRawTransHandle(RAwRTPTRANSPROC pRawRtpTrans)
{
    m_pRawRtpTrans = pRawRtpTrans;
}



// ���ݻص�
void RtpCallBackProc(u8 *pBuf, s32 nSize, void* pContext)//, TAddr &tRemoteAddr)
{
    CKdvRtp *pMain = (CKdvRtp *)pContext;
    if(pMain != NULL)
    {
        pMain->DealData(pBuf, nSize);//, tRemoteAddr);
    }
    else
    {
        if(1 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
        {
            OspPrintf(1, 0, "[RtpCallBackProc] pContext == NULL   \n");
        }
    }
}
























