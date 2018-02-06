/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvRtp.cpp
相关文件    : KdvRtp.h
文件实现功能: RTP Format Implementation
作者        : Jason
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         Jason       Create
2003/06/03  2.0         Jason       Add RTP Option
2004/10/08  2.0         万春雷      调整环形缓冲单元长度以适应加密码流
2004/12/28  2.0         万春雷      增加SendTo的返回值错误码的具体区分
2005/01/25  2.0         万春雷      SendTo时过滤无效目标地址的投递
2005/02/03  2.0         万春雷      rtp包投递失败后，允许继续投递
2005/05/14  3.6         华亮        修改设置本地地址(原来实际是使用本地地址来设置组播)
2005/06/03  3.6         华亮        增加原始码流调试转发功能
******************************************************************************/



#include "kdvrtp.h"

extern s32   g_nDiscardSpan;        //发送端模拟丢弃小包的步长间隔
extern s32   g_nRepeatSnd;            //重传开关
extern s32   g_nShowDebugInfo;        //是否显示隐藏的一些调试状态信息
extern u32   g_dwAdditionMulticastIf;   //额外的组播地址
extern BOOL32 g_bUseMemPool;

void RtpCallBackProc(u8 *pBuf, s32 nSize, void* pContext);

//初始化类成员
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

    //m_hSynSem 开始有信号
    OspSemBCreate( &m_hSynSem);

    memset(&m_tRemoteAddr, 0, sizeof(m_tRemoteAddr));
    memset(&m_tLocalAddr, 0, sizeof(m_tLocalAddr));
    for (u8 byNum = 0; byNum < RTP_FIXEDHEADER_SIZE/sizeof(u32); byNum++)
    {
        m_dwRtpHeader[byNum] = 0;
    }
}

//类成员对象销毁
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
    函数名        ：Create
    功能        ：初始化对象及设置同步源
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwSSRC  同步源
                   bAllocLPBuf 是否进行环形缓冲的分配
                               因为对于 非 H.264以及mp4 码流直接发送，并未使用环形缓存
                   bVidPayload 视频或者音频
    返回值说明： 参见错误码定义
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

        //创建环状缓冲
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
    函数名        ：FreeBuf
    功能        ：复位类对象
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：无

    返回值说明： 无
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
    函数名        ：ResetSSRC
    功能        ：重置同步源
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwSSRC,与RTP匹配的同步源

    返回值说明： 参见错误码定义
=============================================================================*/
void CKdvRtp::ResetSSRC(u32 dwSSRC)
{
    m_dwSSRC = dwSSRC;
}

/*=============================================================================
    函数名        ：ResetSSRC
    功能        ：重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，
                  将不对已经发送的数据包进行缓存
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：bRepeatSnd  是否重传
                  wBufTimeSpan 重传发送的缓冲区的缓冲时间长度

    返回值说明： 参见错误码定义
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
            m_nRLBUnitNum = 0;  //创建失败，设置重传块数为0
            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_SND_MEMORY;
        }

        //创建重发环状缓冲
        if (NULL == m_pRSPackBuf)
        {
            MEDIANET_MALLOC(m_pRSPackBuf, MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE);
            if(NULL == m_pRSPackBuf)
            {
                m_nRLBUnitNum = 0; //创建失败，设置重传块数为0
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

    //创建
    SYS_SAFE_FREE(m_pRLBBuf);
    if (m_atRLBPackets)
    {
        free(m_atRLBPackets);
        m_atRLBPackets = NULL;
    }

    //包数需要调整为2的幂
    s32 i;
    for (i = 0; i < 7; i++)
    {
        if (nUnitBufNum <= (MAX_PACKET_BUFF_NUM << i))
            break;
    }
    nUnitBufNum = MAX_PACKET_BUFF_NUM << i;

    s32 nBufLen   = (RLB_PACK_SIZE) * nUnitBufNum; //加上有效长度单元

    m_nRLBUnitNum = nUnitBufNum;

    //创建包数据缓冲
    SYS_MALLOC(m_pRLBBuf, nBufLen)

    //记录地址，方便排查越界情况
    m_pRlbBufStart = m_pRLBBuf;
    m_pRlbBufEnd = m_pRLBBuf + nBufLen;

    //这里一定要memset
    memset(m_pRLBBuf, 0, nBufLen);
    //创建包信息Array
    m_atRLBPackets = (TLRSPacketInfo*)malloc(nUnitBufNum * sizeof(TLRSPacketInfo));

    //记录地址，方便排查越界情况
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

    //初始化Array
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
    函数名        ：SetRtcp
    功能        ：设置和本RTP对应的RTCP对象
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pRtcp  RTCP对象，内部用于控制RTP

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::SetRtcp(CKdvRtcp *pRtcp)
{
    m_pRtcp = pRtcp;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        : SetCallBack
    功能        ：设置上层回调
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pCallBackHandle    回调函数指针
                   dwContext          回调的用户数据

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::SetCallBack(PRCVPROC pCallBackHandle, void* pContext)
{
    m_pCallBack = pCallBackHandle;
    m_pContext = pContext;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    函数名        ：CheckPackAvailale
    功能        ：检测待发送的数据包的合法性
                   放入LOOPBUF,或直接发送出去。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   ptRtpPack   RTP数据包结构,参见结构定义.
                   pnHeadLen   返回实际的头部长度, 用于判断预留空间是否足够

    返回值说明：参见错误码定义
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

    //长度大小内部控制，不会出现异常
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
    函数名        ：DirectSend
    功能        ：把要发送的数据加上RTP FIXED HEADER 直接发送出去。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pPackBuf    小包缓冲
                   nPackLen    小包长度
                   dwTimeStamp 该小包的时间戳

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::DirectSend(u8 *pPackBuf, s32 nPackLen, u32 dwTimeStamp)
{
    u16 wRet = MEDIANET_NO_ERROR;

    m_dwTotalPackNum++;

    //DIRECT SEND
    for(s32 i=0; i<m_tRemoteAddr.m_byNum; i++)
    {
        //发送端模拟丢包
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

                //向额外的组播socket进行发送 add by hual 2005-07-26
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
    函数名        ：SaveIntoLPBuf
    功能        ：把要发送的数据包存入环形缓冲。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pPackBuf    小包缓冲
                   nPackLen    小包长度
                   bTrans     是否做透明转发处理。

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::SaveIntoLPBuf(u8 *pPackBuf, s32 nPackLen, BOOL32 bTrans)
{
    if(NULL != m_pLoopBuf)
    {
        //写入待发环形缓冲
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
    函数名        ：Write
    功能        ：把要发送的数据加上RTP FIXED HEADER
                   放入LOOPBUF,或直接发送出去。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   tRtpPack   RTP数据包结构,参见结构定义.
                   bSend      是否直接发送。
                   bTrans     是否做透明转发处理。

    返回值说明：参见错误码定义
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

    //判断是否有足够的预留空间.
    //没有
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

            //做透明转发时, 则包序列号及时间戳不变
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
                // 这里其实应统一为字节数，但考虑到与以前版本的兼容，暂时不改动
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

        //外层处理
        /*
        // SET Padding Octor:
        if( 0 != tRtpPack.m_byPadNum )
        {
            //应该包含本身1字节长度
            *(m_pPackBuf+nMove+tRtpPack.m_nRealSize) = tRtpPack.m_byPadNum + RTP_PADDING_SIZE;
            tRtpPack.m_nRealSize += RTP_PADDING_SIZE;
        }
        */

        if(FALSE == bTrans)
        {
            //写入重发环形缓冲
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

        //做透明转发时, 则包序列号及时间戳不变
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

        //have space to save，reduce memory copy;
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
            // 这里其实应统一为字节数，但考虑到与以前版本的兼容，暂时不改动
            dwExHeader = SetBitField(dwExHeader,tRtpPack.m_nExSize, 0, 16);
            dwExHeader = htonl(dwExHeader);
            memcpy(((u8 *)pdwFixedHeader) + nMove, &dwExHeader, sizeof(u32));
            nMove += sizeof(u32);

            memcpy(((u8 *)pdwFixedHeader) + nMove, tRtpPack.m_pExData, tRtpPack.m_nExSize * sizeof(u32));
            nMove += tRtpPack.m_nExSize * sizeof(u32);
        }

        //here must not copy real data memory

        //外层处理
        /*
        // SET Padding Octor:
        if( 0 != tRtpPack.m_byPadNum )
        {
            //应该包含本身1字节长度
            *((u8*)pdwFixedHeader+nMove+tRtpPack.m_nRealSize) = tRtpPack.m_byPadNum + RTP_PADDING_SIZE;
            tRtpPack.m_nRealSize += RTP_PADDING_SIZE;
        }
        */

        if(FALSE == bTrans)
        {
            //写入重发环形缓冲
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
    函数名        ：WriteRLB
    功能        ：把要发送的数据加上时间戳及包大小
                   放入重发环形缓冲
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pPacketBuf   RTP数据包缓冲指针.
                   nPacketSize  RTP数据包尺寸。

    返回值说明：参见错误码定义
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
        //NULL保护
        if(ptPackInfo == NULL)
        {
            OspPrintf(1,0,"[WriteRLB]ERROR ptPackInfo is NULL\n");

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }

        //越界保护
        if(ptPackInfo < m_pRlbPackStart || ptPackInfo > m_pRlbPackEnd)
        {
            OspPrintf(1,0,"[WriteRLB]ERROR ptPackInfo=%p,m_pRlbPackStart=%p,m_pRlbPackEnd=%p,Nowm_atRlbPacks=%p,wSequence=%u\n", \
                           ptPackInfo, m_pRlbPackStart, m_pRlbPackEnd, m_atRLBPackets, wSequence);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }

        //null 保护
        if(ptPackInfo->m_pbyBuff == NULL)
        {
            OspPrintf(1,0, "[WriteRLB]ERROR ptPackInfo->m_pbyBuff= %p \n", ptPackInfo->m_pbyBuff);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }
        //越界保护
        if((ptPackInfo->m_pbyBuff) < m_pRlbBufStart || (ptPackInfo->m_pbyBuff) > m_pRlbBufEnd || (ptPackInfo->m_pbyBuff) + ptPackInfo->n_nDataSize > m_pRlbBufEnd)
        {
            OspPrintf(1,0, "[WriteRLB]ERROR, ptPackInfo->m_pbyBuff=%8x, m_RlbAddrStart=%p, m_RlbAddrEnd=%p, NOWm_RLBbuf=%p, datasize = %d, wSequence =%d\n", ptPackInfo->m_pbyBuff,\
                                    m_pRlbBufStart, m_pRlbBufEnd, m_pRLBBuf, ptPackInfo->n_nDataSize, wSequence);

            MEDIANET_SEM_GIVE(m_hSynSem);
            return ERROR_LOOP_BUF_NULL;
        }
        //记录SN
        ptPackInfo->m_wSN = wSequence;
        //记录时间戳
        ptPackInfo->m_dwTS = dwTimestamp;
        //数据包长度
        ptPackInfo->n_nDataSize = nPacketSize;
        //拷贝RTP数据包
        memcpy(ptPackInfo->m_pbyBuff, pPacketBuf, nPacketSize);

        m_wRLBLastSN = wSequence;
    }

    MEDIANET_SEM_GIVE(m_hSynSem);

    return wRet;
}

/*=============================================================================
    函数名        ：ReadRLBBySN
    功能        ：根据 SN 从重发环形缓冲中取出需要重发的序列包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pBuf [out]         缓冲
                    nBufSize [out]     缓冲长度
                    dwTimeStamp[in]    包所在帧的时间戳
                    wSeqNum[in]        包的包序列
    返回值说明：参见错误码定义
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

    //NULL保护
    if(ptPackInfo == NULL)
    {
        OspPrintf(1,0,"[ReadRLBBySN]ERROR ptPackInfo is NULL\n");

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }

    //越界保护
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

    //取出RTP数据包
    //大小保护
    if( ptPackInfo->n_nDataSize > RLB_PACK_SIZE || ptPackInfo->n_nDataSize < 0)
    {
        OspPrintf(1,0, "[ReadRLBBySN]ERROR ptPackInfo->n_nDataSize > RLB_PACK_SIZE or < 0\n");

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    //null 保护
    if(pBuf == NULL || ptPackInfo->m_pbyBuff == NULL)
    {
        OspPrintf(1,0, "[ReadRLBBySN]ERROR pbuf=%p,ptPackInfo->m_pbyBuff=%p\n", pBuf, ptPackInfo->m_pbyBuff);

        MEDIANET_SEM_GIVE(m_hSynSem);
        return ERROR_LOOP_BUF_NULL;
    }
    //越界保护
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
    函数名        ：FindRLBByTS
    功能        ：根据 TIMESTAMP 从重发环形缓冲中取出需要重发的起始序列包
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pBuf [out]         缓冲
                    byPackNum [out]    总报数
                    dwTimeStamp[in]    包所在帧的时间戳
    返回值说明：-1 没找到，成功返回起始序列包
=============================================================================*/

#define PREVPACKPOS(a) (((a) > 0) ? ((a)-1) : (m_nRLBUnitNum-1))

//获取指定时间戳的最后一包
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

    //考虑到缓冲时间一般设置得比较长，采用逆向搜索
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
    函数名        ：DealRSQBackQuest
    功能        ：处理重发请求，从重发环形缓冲中取出请求重发的小包重发出去
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：ptRSQ 重发请求结构指针
    返回值说明：参见错误码定义
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

        BOOL32  bRS = 0; //是否重发
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
            //分析重发请求帧中各包的重发掩码位，00010101 （1－需要重发, 0－不重发）
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

        //从后往前检索
        while (nPackRendNum < MAX_PACK_NUM)
        {
            //分析重发请求帧中时间戳，检索并发出相应数据包
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
            wSequence--; //从后往前

            //先判断一下，防止ReadRLBBySN中打印查找失败信息
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
    函数名        ：SendByPackNum
    功能        ：从LOOPBUF中取出指定包数发送出去
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   nPackNum 要发送的包数

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::SendByPackNum(s32 nPackNum)
{
    if(m_pLoopBuf == NULL || m_pSocket == NULL || m_pRtcp == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    u16 wRet = MEDIANET_NO_ERROR;
    BOOL32 bSendErr = FALSE; //本次发送是否有过失败
    s32 nPackSize   = 0;

    for(s32 i=0; i<nPackNum; i++)
    {
        //read a pack
        if(LOOPBUF_NO_ERROR != m_pLoopBuf->Read(m_pPackBuf,nPackSize))
        {
            return MEDIANET_NO_ERROR;
        }

        m_dwTotalPackNum++;

        //发送端模拟丢包
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

                //向额外的组播socket进行发送 add by hual 2005-07-26
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
    函数名        ：SendBySize
    功能        ：从LOOPBUF中取出指定字节数发送出去
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   nTotalPackSize 要发送的字节数

    返回值说明：参见错误码定义
=============================================================================*/
u16 CKdvRtp::SendBySize(s32 nTotalPackSize)
{
    if(m_pLoopBuf == NULL|| m_pSocket == NULL|| m_pRtcp == NULL)
    {
        return ERROR_RTP_NO_INIT;
    }

    u16 wRet = MEDIANET_NO_ERROR;
    BOOL32 bSendErr = FALSE; //本次发送是否有过失败
    s32 nPackSize   = 0;
    s32 nCurTotalSize = 0;   //当前已经发送的小包的字节数

    //发送指定大小的字节数 的小包
    while( nCurTotalSize < nTotalPackSize )
    {
        //read a pack
        if(LOOPBUF_NO_ERROR != m_pLoopBuf->Read(m_pPackBuf, nPackSize))
        {
            return MEDIANET_NO_ERROR;
        }

        m_dwTotalPackNum++;
        nCurTotalSize += nPackSize;

        //发送端模拟丢包
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

                //向额外的组播socket进行发送 add by hual 2005-07-26
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

    //此段代码无用
    if( TRUE == bSendErr )
    {
        wRet = ERROR_SND_SEND_UDP;
    }

    return wRet;
}

/*=============================================================================
    函数名        ：DealData
    功能        ：处理网络传过来的数据
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   pBuf   数据缓冲
                   nSize  数据大小
    返回值说明：无
=============================================================================*/
void CKdvRtp::DealData(u8 *pBuf, s32 nSize)
{
    if( (NULL == pBuf) || (nSize < RTP_FIXEDHEADER_SIZE) )
    {
        return;
    }

    //check if need to del userhead, add by hual 2006-08-26
    //考虑到可能存在代理和非代理混合使用的情况，目前先判断头子节的最高两位是否为RTP的版本号
    if (m_tLocalAddr.m_dwUserDataLen > 0  && (u32)nSize > m_tLocalAddr.m_dwUserDataLen &&
        (((*pBuf)&0xC0) != 0x80))
    {
        //由于接收时头标记中填写的是对端的源地址，目前不进行匹配
        //if (0 == memcmp(pBuf, m_tLocalAddr.m_dwUserDataLen, m_tLocalAddr.m_dwUserDataLen))
        {
            pBuf += m_tLocalAddr.m_dwUserDataLen;
            nSize -= m_tLocalAddr.m_dwUserDataLen;
        }
    }

    u32 *header=(u32*)pBuf;

    //原始数据包转发
    if (NULL != m_pRawRtpTrans)
    {
        m_pRawRtpTrans(pBuf, nSize);
    }

    /*///test 2222//////////////////////////////////////////////////////////////////////////

    //组成rtp包后转发。。。。。。。
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
    if (m_byVertion != 2)//rtp包版本号为2，不为2时为非rtp包
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
    if(nSize < nOffSet) //rtp头 异常检测
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

    //扩展头 － 紧跟在 Fixed Header后面, 目前用于处理自定义的mp4、mp3包
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
        if((xStart+1 + tRtpPack.m_nExSize) * sizeof(u32) < nSize && tRtpPack.m_nRealSize > ((tRtpPack.m_nExSize + 1)*sizeof(u32))) //rtp头 异常检测
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
                //扣除padding填充字段
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
    函数名        ：SetLocalAddr
    功能        ：设置当地socket 参数
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIp   local IP(net order)
                   wPort  local Port(machine order)
                   bRcv   receive or not
    返回值说明：参见错误码定义
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
        //如果是接收，并且是组播或广播，则设置组播地址
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
    函数名        SetMCastOpt
    功能        ：加入组播组
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  dwLocalIP－ 接口IP(net order)
                  dwMCastIP－ 组播组IP地址(net order)

    返回值说明：参见错误码定义
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
    //额外的组播地址
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
    函数名        SetBCastOpt
    功能        ：加入广播组
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明：参见错误码定义
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
    函数名        ：SetRemoteAddr
    功能        ：设置要发送到的远端地址对
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   TRemoteAddr 远端地址结构，参见结构定义
    返回值说明：参见错误码定义
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
    //向重发缓冲队列补充空包，以空出位置，除SN外，其他信息取自上一次发送
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



// 数据回调
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
























