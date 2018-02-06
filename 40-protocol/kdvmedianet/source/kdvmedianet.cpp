/*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvMediaNet.cpp
����ļ�    : KdvMediaNet .h
�ļ�ʵ�ֹ���: CKdvMediaSnd,CKdvMediaRcv Implement
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2004/04/13  3.0         ����      �����ش�������
2004/04/16  3.0         ����      ���� ���ն�ģ�ⶪ��С���Ĳ����������
2004/04/21  3.0         ����      �����ش��������е����緢�͵Ľṹ������Ϊ������
2004/04/30  3.0         ����      ���� H.261 ĩβ����
2004/05/18  3.0         ����      ���� H.261 �� H.263 ����ԭʼrtp��ֱ�ӷ��͵Ŀ���
                                    ���Ͷ������ H.264�Լ� mp4 �Ž��л��λ���ķ���
                                    ���ն����֡���尴���û����ݵ� m_dwMaxFrameSize Ϊ׼�����ڴ�
2004/06/08  3.0         ����      ���� H.263 ���� B MODE �� C MODE �Ľ�������
2004/06/08  3.0         ����      ��������ʱ�������� 1450 , ����ʱ�������� 8192,
                                    ���߷ֱ��Բ�ͬ�ĺ궨���ʶ
2004/09/29  2.0         ����      ����linux�汾����֧��
2004/09/30  3.0         ����      ���ն����Ӷ��ڼ��������Ľ��ܵ�֧��
2004/09/30  3.0         ����      ���Ͷ����Ӷ��ڷ��������ļ��ܵ�֧��
2005/06/03  3.6         ����        ����ԭʼ��������ת������
******************************************************************************/


#include "kdvnetsnd.h"
#include "kdvnetrcv.h"
#include "kdvmedianet.h"

BOOL32 g_bUseMemPool = FALSE;

//ģ��汾��Ϣ�ͱ���ʱ�� �� �����Ŀ�İ汾�ͱ���ʱ��
API void kdvmedianetver()
{
    OspPrintf(1,0," kdvmedianet version: %s   ", VER_KDVMEDIANET);
    OspPrintf(1,0," compile time: %s,%s \n", __TIME__, __DATE__);

#ifdef _LINUX_
#else
    ospver();
#endif
}

s8 g_achMedianetVersin[256]={0};
API void KdvGetMedianetVer(char** ppVerionString)
{
//    strncpy(g_achMedianetVersin, VER_KDVMEDIANET, 128);
    sprintf(g_achMedianetVersin, " kdvmedianet version: %s, compile time:%s, date:%s\n",
        VER_KDVMEDIANET, __TIME__, __DATE__);
    if( ppVerionString != 0 )
    {
        *ppVerionString = (char*)g_achMedianetVersin;
    }
}

//ģ�������Ϣ �� �����Ŀ�İ�����Ϣ
API void kdvmedianethelp()
{
    kdvmedianetver();

    MedianetPrintf("\nprintsock()  ���� �շ��׽��ӻ���״̬��Ϣ\n");
    MedianetPrintf("pbsend(u32 objectseq) ����ĳһ�����Ͷ������״̬��Ϣ��ͳ����Ϣ\n");
    MedianetPrintf("pbrecv(u32 objectseq) ����ĳһ�����ն������״̬��Ϣ��ͳ����Ϣ\n");

    MedianetPrintf("pbinfo(BOOL32 bShowRcv, BOOL32 bShowSnd) �����շ��������״̬��Ϣ��ͳ����Ϣ\n");
    MedianetPrintf("pdinfo(s32 nShowDebugInfo) �����Ƿ���ʾ���ص�һЩ����״̬��Ϣ\n");

    MedianetPrintf("0 -- ��������debug��Ϣ \n");
    MedianetPrintf("1 -- ʵʱ��ʾ���ն���debug��Ϣ \n");
    MedianetPrintf("2 -- ʵʱ��ʾ���Ͱ�debug(�����ش�����ķ��ͼ�Ӧ��״̬)��Ϣ \n");
    MedianetPrintf("3 -- ��ʾ���ն��󴴽�����Ϣ \n");
    MedianetPrintf("4 -- ��ʾ���Ͷ��󴴽�����Ϣ \n");
    MedianetPrintf("5 -- ��ʾȫ�ֽ����߳�״̬���� \n");
    MedianetPrintf("6 -- ��ʾȫ��RTCP    ����ʱ�ϱ��߳�״̬���� \n");
    MedianetPrintf("7 -- ʵʱ��ʾ���յ���ʵ�����ݰ���Ϣ \n");
    MedianetPrintf("8 -- ʵʱ��ʾ���յ�����Ƶ֡�ص���Ϣ \n");
    MedianetPrintf("9 -- ʵʱ��ʾ��֡�ص����� \n");
    MedianetPrintf("10 -- ʵʱ��ʾ ������ ������ģ��udp����Ϣ  \n");
    MedianetPrintf("11 -- ʵʱ��ʾ ���Ͷ˶��� sbit/ebitУ����Ϣ  \n");
    MedianetPrintf("12 -- ʵʱ��ʾ ���ն˶��� sbit/ebitУ����Ϣ  \n");
    MedianetPrintf("13 -- ʵʱ��ʾ ���յ�����Ƶ֡�ص���Ϣ  \n");
    MedianetPrintf("14 -- ʵʱ��ʾ ���ܹ��̵��쳣������Ϣ  \n");
    MedianetPrintf("15 -- ʵʱ��ʾ ���ܹ��̵��쳣������Ϣ  \n");
    MedianetPrintf("16 -- ʵʱ��ʾ��������Ϣ  \n");
    MedianetPrintf("17 -- ʵʱ��δ������Ϣ(���ڰ�)  \n");
    MedianetPrintf("33 -- �鿴��ӵ�ps֡�ص���Ϣ  \n");
    MedianetPrintf("255 -- ��ʾ��5������е�����Ϣ \n");

    MedianetPrintf("1) rsopen(BOOL32 bRcvCallback, BOOL32 bSelfSnd)  �����շ����ƿ���\n");
    MedianetPrintf("2) stest(s32 nSndObjIndex, s32 nFrameLen, s32 nSndNum, s32 nSpan)����ʹ�����ж������Բ��\n");
    MedianetPrintf("3) setconfuedadjust(int nbConfuedAdjust)����mp3/g.7xx ��Ƶ�����������Ŀ���\n");
    MedianetPrintf("4) setrepeatsend(s32 nRepeatSnd)���� �ش����ƿ���\n");
    MedianetPrintf("5) seth263dsend(s32 nRtpDSend)���� H.263 ����ԭʼrtp��ֱ�ӷ��͵Ŀ���\n");
    MedianetPrintf("6) setdiscardspan(s32 nDiscardSpan) �������Ͷ�ģ�ⶪ��С���Ĳ���������� \n");
    MedianetPrintf("7) setrcvdiscardspan(s32 nRcvDiscardSpan) �������ն�ģ�ⶪ��С���Ĳ���������� \n");
    MedianetPrintf("8) setconfuedspan(s32 nConfuedSpan) �������Ͷ�ģ�� H.263С������Ĳ������ \n");

    MedianetPrintf("9)  startrcvtask(int nCurRcvPort) ���� ���ã���ָ������udp�����ݽ���ģ�� \n");
    MedianetPrintf("10) showrcvresult( )              ���� ���ã���ʾudp�����ݽ���ģ���� \n");
    MedianetPrintf("11) startsndtask(int nPackNum, int nPackLen, int nRaw, int nCurSndPort, int nAddrIP=0) ���� ���ã���ָ������udp�����ݽ���ģ�� \n");
    MedianetPrintf("12) showsimresult( )              ���� ���ã���ʾudp�������շ�ģ���� \n");

    MedianetPrintf("13) showsmoothsnd( )              ���� ���ã���ʾ�Ƿ��Ƕ��ʽ����ϵͳ����ƽ������\n");
    MedianetPrintf("14) disablesmoothsnd( )           ���� ���ã��رն�Ƕ��ʽ����ϵͳ����ƽ������ \n");
    MedianetPrintf("15) setrawsend(s32 nRawSend)      ���� ���ã��Ƿ�ʹ�� raw-socket �������ݰ�Ͷ�� \n");
    MedianetPrintf("16) settos(s32 nTOS)              ���� ���ã����÷���socket�����ݰ���TOSֵ��Ĭ������Ϊ EF ���� \n");
    MedianetPrintf("17) setttl(s32 nTTL)              ���� ���ã����÷���socket�����ݰ���ttlֵ \n");
    MedianetPrintf("18) setchecksum(s32 nchecksum)    ���� ���ã���ʾudp�������շ�ģ���� \n");
    MedianetPrintf("19) mediarelaystart(s32 nRecvId, s8* pchIpStr, u16 wPort) ����rtpת����ʼ\n");
    MedianetPrintf("20) mediarelaystop()              ����rtpת��ֹͣ\n");
    MedianetPrintf("21) mediarecvframerate(s32 nRecvId, s32 nSecond)   ��������֡��ͳ��\n");
    MedianetPrintf("22) mediasndframerate(s32 nSndId, s32 nSecond))    ��������֡��ͳ��\n");
    MedianetPrintf("23) pssinfo(s32 nSndId))          ��������socket��Ϣ\n");
    MedianetPrintf("24) mediasendadd(s32 nSendId, s8* pchIpStr, u16 wPort) �����ֹ�����һ·��������\n");
    MedianetPrintf("25) mediasenddel(s32 nSendId, s8* pchIpStr, u16 wPort) �����ֹ�ɾ��һ·��������\n");

}

extern TMediaSndList   g_tMediaSndList;     //���Ͷ����б�ȫ�ֱ���
extern TMediaRcvList   g_tMediaRcvList;     //���ն����б�ȫ�ֱ���
extern TMediaSndList   g_tMediaSndListTmp;
extern TMediaRcvList   g_tMediaRcvListTmp;

extern SEMHANDLE   g_hMediaSndSem;    //���Ͷ����б�ķ���ά�����ź���
extern SEMHANDLE   g_hMediaRcvSem;    //���ն����б�ķ���ά�����ź���

s32   g_nDiscardSpan = 0;            //���Ͷ�ģ�ⶪ��С���Ĳ������

API void setdiscardspan(s32 nDiscardSpan)
{
    g_nDiscardSpan = nDiscardSpan;
}

s32   g_nRcvDiscardSpan = 0;        //���ն�ģ�ⶪ��С���Ĳ������

API void setrcvdiscardspan(s32 nRcvDiscardSpan)
{
    g_nRcvDiscardSpan = nRcvDiscardSpan;
}


s32   g_nRepeatSnd = 1;        //�ش�����

API void setrepeatsend(s32 nRepeatSnd)
{
    g_nRepeatSnd = nRepeatSnd;
}


s32   g_nRtpDSend = 0; //H.263 ����ԭʼrtp��ֱ�ӷ��͵Ŀ���
API void seth263dsend(s32 nRtpDSend)
{
    g_nRtpDSend = nRtpDSend;
}

s32   g_nConfuedSpan = 0;        //���Ͷ�ģ�� H.263С������Ĳ������
s32   g_nSpanChanged = 0;        //���Ͷ�ģ�� H.263С������Ĳ�������Ƿ����仯

API void setconfuedspan(s32 nConfuedSpan)
{
    if(g_nConfuedSpan != nConfuedSpan)
    {
        g_nSpanChanged = 1;
    }
    g_nConfuedSpan = nConfuedSpan;
}

s32  g_dwMaxExtendPackSize = MAX_EXTEND_PACK_SIZE;

API void setsendpacksize(u32 dwMaxSendPackSize)
{
    if (dwMaxSendPackSize > MAX_EXTEND_PACK_SIZE)
    {
        OspPrintf(TRUE, FALSE, "can't set size %d > MAX_EXTEND_PACK_SIZE \n", dwMaxSendPackSize, MAX_EXTEND_PACK_SIZE);
        return;
    }

    g_dwMaxExtendPackSize = dwMaxSendPackSize;
}

s32   g_nShowDebugInfo = 0;        //�Ƿ���ʾ���ص�һЩ����״̬��Ϣ

/*
0 -- ��������debug��Ϣ
1 -- ʵʱ��ʾ���ն���debug��Ϣ
2 -- ʵʱ��ʾ���Ͱ�debug(�����ش�����ķ��ͼ�Ӧ��״̬)��Ϣ
3 -- ��ʾ���ն��󴴽�����Ϣ
4 -- ��ʾ���Ͷ��󴴽�����Ϣ
5 -- ��ʾȫ�ֽ����߳�״̬����

7 -- ʵʱ��ʾ���յ���ʵ�����ݰ���Ϣ
8 -- ʵʱ��ʾ���յ���ʵ������֡��Ϣ

255 -- ��ʾ��5������е�����Ϣ
*/
API void pdinfo(s32 nShowDebugInfo)
{
    g_nShowDebugInfo = nShowDebugInfo;
}

//�շ����ƿ���
BOOL32  g_bInterRcvCallBack = TRUE;    //TRUE - ���ղ�����������֡�ر��ϲ�ص�
BOOL32  g_bSelfSnd = FALSE;            //TRUE - �����Բ����ݰ����ر��ⲿ��������

API void rsopen(BOOL32 bRcvCallback, BOOL32 bSelfSnd)
{
    g_bInterRcvCallBack = bRcvCallback;
    g_bSelfSnd = bSelfSnd;
}

//ʹ�����ж������Բ��
//nSndObjIndex - ���Ͷ�������
//nFrameLen - ģ�������֡��С < max
//nSndNum    - ��֡�ظ����͵Ĵ���
//nSpan        - �ط���ʱ����
API void stest(s32 nSndObjIndex, s32 nFrameLen, s32 nSndNum, s32 nSpan)
{
    g_bSelfSnd = TRUE;

    if(NULL == g_hMediaSndSem)
    {
        MedianetLog(Api, "\n NULL == g_hMediaSndSem \n" );
        return;
    }

    MEDIANET_SEM_TAKE(g_hMediaSndSem);

    if(nSndObjIndex < 1 )
    {
        nSndObjIndex = 1;
    }
    if(nSndObjIndex > g_tMediaSndList.m_nMediaSndCount )
    {
        nSndObjIndex = g_tMediaSndList.m_nMediaSndCount;
    }
    if(nSndNum < 1 )
    {
        nSndNum = 1;
    }
    if(nSpan < 0 )
    {
        nSpan = 0;
    }


    u32 dwSTimeStamp = OspTickGet();

    CKdvMediaSnd  *pcKdvMediaSnd = g_tMediaSndList.m_tMediaSndUnit[nSndObjIndex-1];

    pcKdvMediaSnd->SelfTestSend(nFrameLen, nSndNum, nSpan);


    u32 dwETimeStamp = OspTickGet();
    u32 dwSpanTime = nSpan;

    if(dwETimeStamp > dwSTimeStamp)
    {
#ifdef WIN32
        dwSpanTime = dwETimeStamp - dwSTimeStamp;
#else
        dwSpanTime = (dwETimeStamp - dwSTimeStamp)*1000/OspClkRateGet();
#endif
    }
    u32 dwBitrate = 0;

    if(0 != dwSpanTime)
    {
        dwBitrate = nFrameLen*nSndNum*8/dwSpanTime;
    }
    else if(0 != nSpan)
    {
        dwBitrate = nFrameLen*8/nSpan;
    }
    //��ӡƽ������
    MedianetLog(Api, "\n CKdvMediaSnd stest OK, bitrate:%dkbps, nFrameLen: %d, nSndNum: %d, nSpan: %d \n",
                    dwBitrate, nFrameLen, nSndNum, nSpan);

    MEDIANET_SEM_GIVE(g_hMediaSndSem);
}

//ͨ������ţ���ӡ��Ӧsend������Ϣ
API void pbsend(u32 ObjectSeq)
{

        if(NULL == g_hMediaSndSem)
        {
            MedianetPrintf("\n NULL == g_hMediaSndSem \n" );
            return;
        }

        MEDIANET_SEM_TAKE(g_hMediaSndSem);

        CKdvMediaSnd  *pcKdvMediaSnd;
        TAdvancedSndInfo  tAdvancedSndInfo;
        TKdvSndStatus     tKdvSndStatus;
        TKdvSndStatistics tKdvSndStatistics;
        memset( &tKdvSndStatus, 0, sizeof(tKdvSndStatus));
        memset( &tAdvancedSndInfo, 0, sizeof(tAdvancedSndInfo));
        memset( &tKdvSndStatistics, 0, sizeof(tKdvSndStatistics));

        MedianetPrintf("\n CKdvMediaSnd Object Num: %d \n",
                        g_tMediaSndList.m_nMediaSndCount);

        for(s32 nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
        {
            if(nPos != ObjectSeq)
            {
                continue;
            }

            pcKdvMediaSnd = g_tMediaSndList.m_tMediaSndUnit[nPos];
            if(NULL != pcKdvMediaSnd)
            {
                pcKdvMediaSnd->GetStatus(tKdvSndStatus);
                pcKdvMediaSnd->GetStatistics(tKdvSndStatistics);
                pcKdvMediaSnd->GetAdvancedSndInfo(tAdvancedSndInfo);

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Status -------------\n", nPos+1);
                MedianetPrintf("pcKdvMediaSnd Address:[%x], \n", pcKdvMediaSnd);
                MedianetPrintf("MaxFrameSize:[%d]\n", tKdvSndStatus.m_dwMaxFrameSize);
                MedianetPrintf("NetBand:[%d]     \n", tKdvSndStatus.m_dwNetBand);
                MedianetPrintf("MediaType:[%d]   \n", tKdvSndStatus.m_byMediaType);
                MedianetPrintf("LocalActivePT:[%d] (0-��ʾ��Ч)  \n", tAdvancedSndInfo.m_byLocalActivePT);

                MedianetPrintf("FrameRate:[%d]   \n", tKdvSndStatus.m_byFrameRate);
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_dwRTPAddr);
                    MedianetPrintf("m_tLocalNet.m_dwRTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                }
                MedianetPrintf("m_tLocalNet.m_wRTPPort:[%d]   \n", tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_wRTPPort);
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_dwRTCPAddr);
                    MedianetPrintf("m_tLocalNet.m_dwRTCPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                }
                MedianetPrintf("m_tLocalNet.m_wRTCPPort:[%d]   \n", tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_wRTCPPort);

                MedianetPrintf("SendAddr Num:[%d]\n", tKdvSndStatus.m_tSendAddr.m_byNum);
                for(s32 i=0; i<tKdvSndStatus.m_tSendAddr.m_byNum; i++)
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRTPAddr);
                    MedianetPrintf("m_tRemoteNet.m_dwRTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                    MedianetPrintf("m_tRemoteNet.m_dwRTPAddr:[%d]\n", tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_wRTPPort);
                    MedianetPrintf("m_tRemoteNet.m_dwRtpUserDataLen:[%d]\n", tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen);
                    if (tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen > 0)
                    {
                        MedianetPrintf("m_tRemoteNet.m_dwRtpUserData:[");
                        u8* pbyData = tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_abyRtpUserData;
                        s32 nDataLen = tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen;
                        s32 nDataIndex;
                        if (nDataLen <= 32)
                        {
                            for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                            {
                                MedianetPrintf("%x ", *pbyData++);
                            }
                        }
                        MedianetPrintf("]\n");
                    }
                }

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Advanced Setting Param-------------\n", nPos+1);
                MedianetPrintf("Encryption:[%d]  \n", tAdvancedSndInfo.m_bEncryption);
                if( TRUE == tAdvancedSndInfo.m_bEncryption )
                {
                    MedianetPrintf("EncryptMode:[%d] \n", tAdvancedSndInfo.m_byEncryptMode);
                    MedianetPrintf("KeySize:[%d] \n", tAdvancedSndInfo.m_wKeySize);
                    MedianetPrintf("KeyBuf:[ ");
                    for (s32 nIndex = 0; nIndex < tAdvancedSndInfo.m_wKeySize; nIndex++ )
                    {
                        OspPrintf( TRUE, FALSE, "%02x", (u8)(tAdvancedSndInfo.m_szKeyBuf[nIndex]) );
                    }
                    OspPrintf( TRUE, FALSE, " ] \n" );
                }
                MedianetPrintf("RepeatSend:[%d]  \n", tAdvancedSndInfo.m_bRepeatSend);
                MedianetPrintf("MaxSendNum:[%d]  \n", tAdvancedSndInfo.m_nMaxSendNum);
                MedianetPrintf("BufTimeSpan:[%d] \n", tAdvancedSndInfo.m_wBufTimeSpan);

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Statistics -------------\n", nPos+1);
                MedianetPrintf("Last Snd FrameID:[%d]\n", tKdvSndStatus.m_dwFrameID);
                MedianetPrintf("FrameNum:[%d]        \n", tKdvSndStatistics.m_dwFrameNum);
                MedianetPrintf("FrameLoseNum:[%d]    \n", tKdvSndStatistics.m_dwFrameLoseNum);
                MedianetPrintf("PackSendNum:[%d]     \n", tKdvSndStatistics.m_dwPackSendNum);
            }
        }

        MEDIANET_SEM_GIVE(g_hMediaSndSem);
}

//ͨ������ţ���ӡ��Ӧrecv������Ϣ
API void pbrecv(u32 ObjectSeq)
{
        if(NULL == g_hMediaRcvSem)
        {
            MedianetPrintf("\n NULL == g_hMediaRcvSem \n" );
        }

        MEDIANET_SEM_TAKE(g_hMediaRcvSem);

        CKdvMediaRcv     *pcKdvMediaRcv;
        u32               dwMaxFrameSize;
        u8                byMediaType;
        TAdvancedRcvInfo  tAdvancedRcvInfo;
        TKdvRcvStatus     tKdvRcvStatus;
        TKdvRcvStatistics tKdvRcvStatistics;
        memset( &tKdvRcvStatus, 0, sizeof(tKdvRcvStatus));
        memset( &tAdvancedRcvInfo, 0, sizeof(tAdvancedRcvInfo));
        memset( &tKdvRcvStatistics, 0, sizeof(tKdvRcvStatistics));

        MedianetPrintf("\n CKdvMediaRcv Object Num: %d \n",
                         g_tMediaRcvList.m_nMediaRcvCount);

        for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
        {
            if(nPos != ObjectSeq)
            {
                continue;
            }
            pcKdvMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
            if(NULL != pcKdvMediaRcv)
            {
                pcKdvMediaRcv->GetMaxFrameSize(dwMaxFrameSize);
                pcKdvMediaRcv->GetLocalMediaType(byMediaType);
                pcKdvMediaRcv->GetStatus(tKdvRcvStatus);
                pcKdvMediaRcv->GetStatistics(tKdvRcvStatistics);
                pcKdvMediaRcv->GetAdvancedRcvInfo(tAdvancedRcvInfo);

                MedianetPrintf("\n CKdvMediaRcv Object [%d] Status -------------\n", nPos+1);
                MedianetPrintf("pcKdvMediaRcv Address:[%x]  \n", pcKdvMediaRcv);
                MedianetPrintf("bRcvStart:[%d]     \n", tKdvRcvStatus.m_bRcvStart);
                MedianetPrintf("dwRcvFlag:[%d]        \n", tKdvRcvStatus.m_dwRcvFlag);
                MedianetPrintf("pregfunc:[%x]        \n", tKdvRcvStatus.m_pRegFunc);
                MedianetPrintf("punregfunc:[%x]        \n", tKdvRcvStatus.m_pUnregFunc);
                u8* pby;
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRTPAddr);
                MedianetPrintf("RTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RTPPort:[%d]    \n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_wRTPPort);
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRTCPAddr);
                MedianetPrintf("RTCPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RTCPPort:[%d]      \n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_wRTCPPort);
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_dwRtcpBackAddr);
                MedianetPrintf("RtcpBackAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RtcpBackPort:[%d]  \n", tKdvRcvStatus.m_tRcvAddr.m_wRtcpBackPort);
                MedianetPrintf("RtpUserDataLen:[%d]\n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtpUserDataLen);
                MedianetPrintf("RtcpUserDataLen:[%d]\n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen);
                if (tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen > 0)
                {
                    MedianetPrintf("m_tLocalNet.m_dwRtcpUserData:[");
                    u8* pbyData = tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_abyRtcpUserData;
                    s32 nDataLen = tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen;
                    s32 nDataIndex;
                    if (nDataLen <= 32)
                    {
                        for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                        {
                            MedianetPrintf("%x ", *pbyData++);
                        }
                    }
                    MedianetPrintf("]\n");
                }
                MedianetPrintf("\n CKdvMediaRcv Object [%d] Advanced Setting Param-------------\n", nPos+1);
                MedianetPrintf("RmtActivePT:[%d]   \n", tAdvancedRcvInfo.m_byRmtActivePT);
                MedianetPrintf("RealActivePT:[%d]  \n", tAdvancedRcvInfo.m_byRealPT);
                MedianetPrintf("Decryption:[%d]    \n", tAdvancedRcvInfo.m_bDecryption);
                if( TRUE == tAdvancedRcvInfo.m_bDecryption )
                {
                    MedianetPrintf("DecryptMode:[%d]   \n", tAdvancedRcvInfo.m_byDecryptMode);
                    MedianetPrintf("KeySize:[%d] \n", tAdvancedRcvInfo.m_wKeySize);
                    MedianetPrintf("KeyBuf:[ ");
                    for (s32 nIndex = 0; nIndex < tAdvancedRcvInfo.m_wKeySize; nIndex++ )
                    {
                        OspPrintf( TRUE, FALSE, "%02x", (u8)(tAdvancedRcvInfo.m_szKeyBuf[nIndex]) );
                    }
                    OspPrintf( TRUE, FALSE, " ] \n" );
                }
                MedianetPrintf("ConfuedAdjust:[%d] \n", tAdvancedRcvInfo.m_bConfuedAdjust);
                MedianetPrintf("RepeatSend:[%d]    \n", tAdvancedRcvInfo.m_bRepeatSend);
                MedianetPrintf("tRSParam.m_wFirstTimeSpan:[%d]   \n", tAdvancedRcvInfo.m_tRSParam.m_wFirstTimeSpan);
                MedianetPrintf("tRSParam.m_wSecondTimeSpan:[%d]  \n", tAdvancedRcvInfo.m_tRSParam.m_wSecondTimeSpan);
                MedianetPrintf("tRSParam.m_wThirdTimeSpan:[%d]   \n", tAdvancedRcvInfo.m_tRSParam.m_wThirdTimeSpan);
                MedianetPrintf("tRSParam.m_wRejectTimeSpan:[%d]  \n", tAdvancedRcvInfo.m_tRSParam.m_wRejectTimeSpan);
				OspPrintf(1, 0, "eStreamType:%d (1 - PS)\n", tAdvancedRcvInfo.m_eStreamType);
                MedianetPrintf("\n CKdvMediaRcv Object [%d] Statistics -------------\n", nPos+1);
                MedianetPrintf("Local MaxFrameSize:[%d] \n", dwMaxFrameSize);
                MedianetPrintf("Local MediaType:[%d] \n", byMediaType);
                MedianetPrintf("Last Rcv FrameID:[%d]  \n", tKdvRcvStatus.m_dwFrameID);
                MedianetPrintf("FrameNum:[%d]  \n", tKdvRcvStatistics.m_dwFrameNum);
                MedianetPrintf("PackNum:[%d]  \n", tKdvRcvStatistics.m_dwPackNum);
                MedianetPrintf("PackLose:[%d]  \n", tKdvRcvStatistics.m_dwPackLose);
                MedianetPrintf("PackIndexError:[%d]  \n", tKdvRcvStatistics.m_dwPackIndexError);
            }
        }

        MEDIANET_SEM_GIVE(g_hMediaRcvSem);
}

//�������״̬��Ϣ��ͳ����Ϣ
API void pbinfo(BOOL32 bShowRcv, BOOL32 bShowSnd)
{
    if(TRUE == bShowSnd)
    {
        if(NULL == g_hMediaSndSem)
        {
            MedianetPrintf("\n NULL == g_hMediaSndSem \n" );
            return;
        }

        MEDIANET_SEM_TAKE(g_hMediaSndSem);

        CKdvMediaSnd  *pcKdvMediaSnd;
        TAdvancedSndInfo  tAdvancedSndInfo;
        TKdvSndStatus     tKdvSndStatus;
        TKdvSndStatistics tKdvSndStatistics;
        memset( &tKdvSndStatus, 0, sizeof(tKdvSndStatus));
        memset( &tAdvancedSndInfo, 0, sizeof(tAdvancedSndInfo));
        memset( &tKdvSndStatistics, 0, sizeof(tKdvSndStatistics));

        MedianetPrintf("\n CKdvMediaSnd Object Num: %d \n",
                        g_tMediaSndList.m_nMediaSndCount);

        for(s32 nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
        {
            pcKdvMediaSnd = g_tMediaSndList.m_tMediaSndUnit[nPos];
            if(NULL != pcKdvMediaSnd)
            {
                pcKdvMediaSnd->GetStatus(tKdvSndStatus);
                pcKdvMediaSnd->GetStatistics(tKdvSndStatistics);
                pcKdvMediaSnd->GetAdvancedSndInfo(tAdvancedSndInfo);

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Status -------------\n", nPos+1);
                MedianetPrintf("pcKdvMediaSnd Address:[%x], \n", pcKdvMediaSnd);
                MedianetPrintf("MaxFrameSize:[%d]\n", tKdvSndStatus.m_dwMaxFrameSize);
                MedianetPrintf("NetBand:[%d]     \n", tKdvSndStatus.m_dwNetBand);
                MedianetPrintf("MediaType:[%d]   \n", tKdvSndStatus.m_byMediaType);
                MedianetPrintf("LocalActivePT:[%d] (0-��ʾ��Ч)  \n", tAdvancedSndInfo.m_byLocalActivePT);

                MedianetPrintf("FrameRate:[%d]   \n", tKdvSndStatus.m_byFrameRate);
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_dwRTPAddr);
                    MedianetPrintf("m_tLocalNet.m_dwRTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                }
                MedianetPrintf("m_tLocalNet.m_wRTPPort:[%d]   \n", tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_wRTPPort);
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_dwRTCPAddr);
                    MedianetPrintf("m_tLocalNet.m_dwRTCPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                }
                MedianetPrintf("m_tLocalNet.m_wRTCPPort:[%d]   \n", tKdvSndStatus.m_tSendAddr.m_tLocalNet.m_wRTCPPort);

                MedianetPrintf("SendAddr Num:[%d]\n", tKdvSndStatus.m_tSendAddr.m_byNum);
                for(s32 i=0; i<tKdvSndStatus.m_tSendAddr.m_byNum; i++)
                {
                    u8* pby;
                    pby = (u8*)&(tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRTPAddr);
                    MedianetPrintf("m_tRemoteNet.m_dwRTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                    MedianetPrintf("m_tRemoteNet.m_dwRTPAddr:[%d]\n", tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_wRTPPort);
                    MedianetPrintf("m_tRemoteNet.m_dwRtpUserDataLen:[%d]\n", tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen);
                    if (tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen > 0)
                    {
                        MedianetPrintf("m_tRemoteNet.m_dwRtpUserData:[");
                        u8* pbyData = tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_abyRtpUserData;
                        s32 nDataLen = tKdvSndStatus.m_tSendAddr.m_tRemoteNet[i].m_dwRtpUserDataLen;
                        s32 nDataIndex;
                        if (nDataLen <= 32)
                        {
                            for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                            {
                                MedianetPrintf("%x ", *pbyData++);
                            }
                        }
                        MedianetPrintf("]\n");
                    }
                }

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Advanced Setting Param-------------\n", nPos+1);
                MedianetPrintf("Encryption:[%d]  \n", tAdvancedSndInfo.m_bEncryption);
                if( TRUE == tAdvancedSndInfo.m_bEncryption )
                {
                    MedianetPrintf("EncryptMode:[%d] \n", tAdvancedSndInfo.m_byEncryptMode);
                    MedianetPrintf("KeySize:[%d] \n", tAdvancedSndInfo.m_wKeySize);
                    MedianetPrintf("KeyBuf:[ ");
                    for (s32 nIndex = 0; nIndex < tAdvancedSndInfo.m_wKeySize; nIndex++ )
                    {
                        OspPrintf( TRUE, FALSE, "%02x", (u8)(tAdvancedSndInfo.m_szKeyBuf[nIndex]) );
                    }
                    OspPrintf( TRUE, FALSE, " ] \n" );
                }
                MedianetPrintf("RepeatSend:[%d]  \n", tAdvancedSndInfo.m_bRepeatSend);
                MedianetPrintf("MaxSendNum:[%d]  \n", tAdvancedSndInfo.m_nMaxSendNum);
                MedianetPrintf("BufTimeSpan:[%d] \n", tAdvancedSndInfo.m_wBufTimeSpan);

                MedianetPrintf("\n CKdvMediaSnd Object [%d] Statistics -------------\n", nPos+1);
                MedianetPrintf("Last Snd FrameID:[%d]\n", tKdvSndStatus.m_dwFrameID);
                MedianetPrintf("FrameNum:[%d]        \n", tKdvSndStatistics.m_dwFrameNum);
                MedianetPrintf("FrameLoseNum:[%d]    \n", tKdvSndStatistics.m_dwFrameLoseNum);
                MedianetPrintf("PackSendNum:[%d]     \n", tKdvSndStatistics.m_dwPackSendNum);
            }
        }

        MEDIANET_SEM_GIVE(g_hMediaSndSem);
    }

    if(TRUE == bShowRcv)
    {
        if(NULL == g_hMediaRcvSem)
        {
            MedianetPrintf("\n NULL == g_hMediaRcvSem \n" );
        }

        MEDIANET_SEM_TAKE(g_hMediaRcvSem);

        CKdvMediaRcv     *pcKdvMediaRcv;
        u32               dwMaxFrameSize;
        u8                byMediaType;
        TAdvancedRcvInfo  tAdvancedRcvInfo;
        TKdvRcvStatus     tKdvRcvStatus;
        TKdvRcvStatistics tKdvRcvStatistics;
        memset( &tKdvRcvStatus, 0, sizeof(tKdvRcvStatus));
        memset( &tAdvancedRcvInfo, 0, sizeof(tAdvancedRcvInfo));
        memset( &tKdvRcvStatistics, 0, sizeof(tKdvRcvStatistics));

        MedianetPrintf("\n CKdvMediaRcv Object Num: %d \n",
                         g_tMediaRcvList.m_nMediaRcvCount);

        for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
        {
            pcKdvMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
            if(NULL != pcKdvMediaRcv)
            {
                pcKdvMediaRcv->GetMaxFrameSize(dwMaxFrameSize);
                pcKdvMediaRcv->GetLocalMediaType(byMediaType);
                pcKdvMediaRcv->GetStatus(tKdvRcvStatus);
                pcKdvMediaRcv->GetStatistics(tKdvRcvStatistics);
                pcKdvMediaRcv->GetAdvancedRcvInfo(tAdvancedRcvInfo);

                MedianetPrintf("\n CKdvMediaRcv Object [%d] Status -------------\n", nPos+1);
                MedianetPrintf("pcKdvMediaRcv Address:[%x]  \n", pcKdvMediaRcv);
                MedianetPrintf("bRcvStart:[%d]     \n", tKdvRcvStatus.m_bRcvStart);
                MedianetPrintf("dwRcvFlag:[%d]        \n", tKdvRcvStatus.m_dwRcvFlag);
                MedianetPrintf("pregfunc:[%x]        \n", tKdvRcvStatus.m_pRegFunc);
                MedianetPrintf("punregfunc:[%x]        \n", tKdvRcvStatus.m_pUnregFunc);
                u8* pby;
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRTPAddr);
                MedianetPrintf("RTPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RTPPort:[%d]    \n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_wRTPPort);
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRTCPAddr);
                MedianetPrintf("RTCPAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RTCPPort:[%d]      \n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_wRTCPPort);
                pby = (u8*)&(tKdvRcvStatus.m_tRcvAddr.m_dwRtcpBackAddr);
                MedianetPrintf("RtcpBackAddr:[%d.%d.%d.%d]\n", pby[0], pby[1], pby[2], pby[3]);
                MedianetPrintf("RtcpBackPort:[%d]  \n", tKdvRcvStatus.m_tRcvAddr.m_wRtcpBackPort);
                MedianetPrintf("RtpUserDataLen:[%d]\n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtpUserDataLen);
                MedianetPrintf("RtcpUserDataLen:[%d]\n", tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen);
                if (tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen > 0)
                {
                    MedianetPrintf("m_tLocalNet.m_dwRtcpUserData:[");
                    u8* pbyData = tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_abyRtcpUserData;
                    s32 nDataLen = tKdvRcvStatus.m_tRcvAddr.m_tLocalNet.m_dwRtcpUserDataLen;
                    s32 nDataIndex;
                    if (nDataLen <= 32)
                    {
                        for (nDataIndex = 0; nDataIndex < nDataLen; nDataIndex++)
                        {
                            MedianetPrintf("%x ", *pbyData++);
                        }
                    }
                    MedianetPrintf("]\n");
                }
                MedianetPrintf("\n CKdvMediaRcv Object [%d] Advanced Setting Param-------------\n", nPos+1);
                MedianetPrintf("RmtActivePT:[%d]   \n", tAdvancedRcvInfo.m_byRmtActivePT);
                MedianetPrintf("RealActivePT:[%d]  \n", tAdvancedRcvInfo.m_byRealPT);
                MedianetPrintf("Decryption:[%d]    \n", tAdvancedRcvInfo.m_bDecryption);
                if( TRUE == tAdvancedRcvInfo.m_bDecryption )
                {
                    MedianetPrintf("DecryptMode:[%d]   \n", tAdvancedRcvInfo.m_byDecryptMode);
                    MedianetPrintf("KeySize:[%d] \n", tAdvancedRcvInfo.m_wKeySize);
                    MedianetPrintf("KeyBuf:[ ");
                    for (s32 nIndex = 0; nIndex < tAdvancedRcvInfo.m_wKeySize; nIndex++ )
                    {
                        OspPrintf( TRUE, FALSE, "%02x", (u8)(tAdvancedRcvInfo.m_szKeyBuf[nIndex]) );
                    }
                    OspPrintf( TRUE, FALSE, " ] \n" );
                }
                MedianetPrintf("ConfuedAdjust:[%d] \n", tAdvancedRcvInfo.m_bConfuedAdjust);
                MedianetPrintf("RepeatSend:[%d]    \n", tAdvancedRcvInfo.m_bRepeatSend);
                MedianetPrintf("tRSParam.m_wFirstTimeSpan:[%d]   \n", tAdvancedRcvInfo.m_tRSParam.m_wFirstTimeSpan);
                MedianetPrintf("tRSParam.m_wSecondTimeSpan:[%d]  \n", tAdvancedRcvInfo.m_tRSParam.m_wSecondTimeSpan);
                MedianetPrintf("tRSParam.m_wThirdTimeSpan:[%d]   \n", tAdvancedRcvInfo.m_tRSParam.m_wThirdTimeSpan);
                MedianetPrintf("tRSParam.m_wRejectTimeSpan:[%d]  \n", tAdvancedRcvInfo.m_tRSParam.m_wRejectTimeSpan);

                MedianetPrintf("\n CKdvMediaRcv Object [%d] Statistics -------------\n", nPos+1);
                MedianetPrintf("Local MaxFrameSize:[%d] \n", dwMaxFrameSize);
                MedianetPrintf("Local MediaType:[%d] \n", byMediaType);
                MedianetPrintf("Last Rcv FrameID:[%d]  \n", tKdvRcvStatus.m_dwFrameID);
                MedianetPrintf("FrameNum:[%d]  \n", tKdvRcvStatistics.m_dwFrameNum);
                MedianetPrintf("PackNum:[%d]  \n", tKdvRcvStatistics.m_dwPackNum);
                MedianetPrintf("PackLose:[%d]  \n", tKdvRcvStatistics.m_dwPackLose);
                MedianetPrintf("PackIndexError:[%d]  \n", tKdvRcvStatistics.m_dwPackIndexError);
            }
        }

        MEDIANET_SEM_GIVE(g_hMediaRcvSem);
    }
}

//hual 2005-08-02
//medianet֡��ͳ��
API void mediarecvframerate(s32 nRecvId, s32 nDelaySecond)
{
    if ((nRecvId < 1) ||
        (nRecvId > g_tMediaRcvList.m_nMediaRcvCount) ||
        (NULL == g_tMediaRcvList.m_tMediaRcvUnit[nRecvId-1]))
    {
        MedianetPrintf("Receive object %d is invalid!\n", nRecvId);
        return;
    }
    if (nDelaySecond == 0)
    {
        nDelaySecond = 1;
    }

    CKdvMediaRcv* pcRecv;
    pcRecv = g_tMediaRcvList.m_tMediaRcvUnit[nRecvId-1];

    TKdvRcvStatistics tKdvRcvStatistics;
    memset(&tKdvRcvStatistics, 0, sizeof(tKdvRcvStatistics));

    pcRecv->GetStatistics(tKdvRcvStatistics);
    u32 dwFrameNum;
    dwFrameNum = tKdvRcvStatistics.m_dwFrameNum;

    OspDelay(nDelaySecond * 1000);

    memset(&tKdvRcvStatistics, 0, sizeof(tKdvRcvStatistics));

    pcRecv->GetStatistics(tKdvRcvStatistics);

    dwFrameNum = tKdvRcvStatistics.m_dwFrameNum - dwFrameNum;

    MedianetPrintf("Recv Object [%d] Frame Rate: %d [%d/%d]\n",
              nRecvId, dwFrameNum / nDelaySecond, dwFrameNum, nDelaySecond);

    return;
}

//hual 2005-08-02
//medianet֡��ͳ��
API void mediasndframerate(s32 nSndId, s32 nDelaySecond)
{
    if ((nSndId < 1) ||
        (nSndId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSndId-1]))
    {
        MedianetPrintf("Send object %d is invalid!\n", nSndId);
        return;
    }
    if (nDelaySecond == 0)
    {
        nDelaySecond = 1;
    }

    CKdvMediaSnd* pcSnd;
    pcSnd = g_tMediaSndList.m_tMediaSndUnit[nSndId-1];

    TKdvSndStatistics tKdvSndStatistics;
    memset(&tKdvSndStatistics, 0, sizeof(tKdvSndStatistics));

    pcSnd->GetStatistics(tKdvSndStatistics);
    u32 dwFrameNum;
    dwFrameNum = tKdvSndStatistics.m_dwFrameNum;

    OspDelay(nDelaySecond * 1000);

    memset(&tKdvSndStatistics, 0, sizeof(tKdvSndStatistics));

    pcSnd->GetStatistics(tKdvSndStatistics);

    dwFrameNum = tKdvSndStatistics.m_dwFrameNum - dwFrameNum;

    MedianetPrintf("Snd Object [%d] Frame Rate: %d [%d/%d]\n",
              nSndId, dwFrameNum / nDelaySecond, dwFrameNum, nDelaySecond);

    return;
}

//����medianet�м�ת��
API void mediarelaystart(s32 nRecvId, s8* pchIpStr, u16 wPort)
{
    if ((nRecvId < 1) ||
        (nRecvId > g_tMediaRcvList.m_nMediaRcvCount) ||
        (NULL == g_tMediaRcvList.m_tMediaRcvUnit[nRecvId-1]))
    {
        MedianetLog(Api, "Receive object %d is invalid!\n", nRecvId);
        return;
    }
    CKdvMediaRcv* pcRecv;
    pcRecv = g_tMediaRcvList.m_tMediaRcvUnit[nRecvId-1];
    if (NULL == pcRecv)
    {
        MedianetLog(Api, "Recv object is NULL\n");
        return;
    }

    u32 dwIP;
    dwIP = inet_addr(pchIpStr);
    if (0 == dwIP || 0 == wPort || (u32)-1 == dwIP)
    {
        MedianetLog(Api, "IP or Port is invalid!\n");
        return;
    }
    pcRecv->RelayRtpStart(dwIP, wPort);
}

//ֹͣmedianet�м�ת��
API void mediarelaystop()
{
    CKdvMediaRcv* pcKdvMediaRcv;
    for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
    {
        pcKdvMediaRcv = g_tMediaRcvList.m_tMediaRcvUnit[nPos];
        if(NULL != pcKdvMediaRcv)
        {
            pcKdvMediaRcv->RelayRtpStop();
        }
    }
}


//medianet����ĳ·����
API void mediasendadd(s32 nSendId, s8* pchIpStr, u16 wPort)
{
    if ((nSendId < 1) ||
        (nSendId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }


    CKdvMediaSnd* pcSend;
    pcSend = g_tMediaSndList.m_tMediaSndUnit[nSendId-1];
    if (NULL == pcSend)
    {
        MedianetLog(Api, "Send object is NULL\n");
        return;
    }

    u32 dwAddtionIP;
    u16 wAddtionPort;
    dwAddtionIP = inet_addr(pchIpStr);
    wAddtionPort = wPort;

    if (0 == dwAddtionIP || 0 == wAddtionPort || (u32)-1 == dwAddtionIP)
    {
        MedianetLog(Api, "IP or Port is invalid!\n");
        return;
    }

    TNetSndParam tNetSndParam;
    u16 wRet;

    wRet = pcSend->GetNetSndParam(&tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "GetNetSndParam return:%d\n", wRet);
        return;
    }


    //printf send socket info
    OspPrintf(TRUE, FALSE, " Before Add, Send info of Object:%d\n", nSendId);
    u8* pbyRtpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTPAddr;
    u8* pbyRtcpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTCPAddr;

    OspPrintf(TRUE, FALSE, "      Local: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
                            pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                            tNetSndParam.m_tLocalNet.m_wRTPPort,
                            pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
                            tNetSndParam.m_tLocalNet.m_wRTCPPort);

    s32 i;
    s32 nFindIndex = -1;

    for (i = 0; i < tNetSndParam.m_byNum; i++)
    {
        if ((dwAddtionIP == tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr) &&
            (wAddtionPort == tNetSndParam.m_tRemoteNet[i].m_wRTPPort))
        {
            nFindIndex = i;
        }

        pbyRtpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;
        pbyRtcpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTCPAddr;

        OspPrintf(TRUE, FALSE, "      Remote: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
                            pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                            tNetSndParam.m_tRemoteNet[i].m_wRTPPort,
                            pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
                            tNetSndParam.m_tRemoteNet[i].m_wRTCPPort);

    }

    if (nFindIndex != -1)
    {
        pbyRtpIP = (u8*)&dwAddtionIP;
        OspPrintf(TRUE, FALSE, "RTP:%d.%d.%d.%d(%d) is already existed.\n",
                    pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                    wAddtionPort);
        return;
    }

    if (tNetSndParam.m_byNum >= MAX_NETSND_DEST_NUM-1)
    {
        OspPrintf(TRUE, FALSE, "Send Addrnum:%D is max.\n", tNetSndParam.m_byNum);
        return;
    }

    tNetSndParam.m_tRemoteNet[tNetSndParam.m_byNum].m_dwRTPAddr = dwAddtionIP;
    tNetSndParam.m_tRemoteNet[tNetSndParam.m_byNum].m_wRTPPort  = wAddtionPort;
    tNetSndParam.m_byNum++;

    wRet = pcSend->SetNetSndParam(tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "SetNetSndParam return:%d\n", wRet);
        return;
    }

    wRet = pcSend->GetNetSndParam(&tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "GetNetSndParam return:%d\n", wRet);
        return;
    }


    //printf send socket info
    OspPrintf(TRUE, FALSE, " After Add, Send info of Object:%d\n", nSendId);
    pbyRtpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTPAddr;
    pbyRtcpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTCPAddr;

    OspPrintf(TRUE, FALSE, "      Local: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
        pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
        tNetSndParam.m_tLocalNet.m_wRTPPort,
        pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
        tNetSndParam.m_tLocalNet.m_wRTCPPort);

    for (i = 0; i < tNetSndParam.m_byNum; i++)
    {
        pbyRtpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;
        pbyRtcpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTCPAddr;

        OspPrintf(TRUE, FALSE, "      Remote: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
            pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
            tNetSndParam.m_tRemoteNet[i].m_wRTPPort,
            pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
            tNetSndParam.m_tRemoteNet[i].m_wRTCPPort);
    }
}

//medianetɾ��ĳ·����
API void mediasenddel(s32 nSendId, s8* pchIpStr, u16 wPort)
{
    if ((nSendId < 1) ||
        (nSendId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }


    CKdvMediaSnd* pcSend;
    pcSend = g_tMediaSndList.m_tMediaSndUnit[nSendId-1];
    if (NULL == pcSend)
    {
        MedianetLog(Api, "Send object is NULL\n");
        return;
    }

    u32 dwDeleteIP;
    u16 wDeletePort;
    dwDeleteIP = inet_addr(pchIpStr);
    wDeletePort = wPort;

    if (0 == dwDeleteIP || 0 == wDeletePort || (u32)-1 == dwDeleteIP)
    {
        MedianetLog(Api, "IP or Port is invalid!\n");
        return;
    }

    TNetSndParam tNetSndParam;
    u16 wRet;

    wRet = pcSend->GetNetSndParam(&tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "GetNetSndParam return:%d\n", wRet);
        return;
    }

    //printf send socket info
    OspPrintf(TRUE, FALSE, " Before Delete, Send info of Object:%d\n", nSendId);
    u8* pbyRtpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTPAddr;
    u8* pbyRtcpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTCPAddr;

    OspPrintf(TRUE, FALSE, "      Local: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
                            pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                            tNetSndParam.m_tLocalNet.m_wRTPPort,
                            pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
                            tNetSndParam.m_tLocalNet.m_wRTCPPort);

    s32 i;
    s32 nFindIndex = -1;
    for (i = 0; i < tNetSndParam.m_byNum; i++)
    {
        if ((dwDeleteIP == tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr) &&
            (wDeletePort == tNetSndParam.m_tRemoteNet[i].m_wRTPPort))
        {
            nFindIndex = i;
        }

        pbyRtpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;
        pbyRtcpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTCPAddr;

        OspPrintf(TRUE, FALSE, "      Remote: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
                                pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                                tNetSndParam.m_tRemoteNet[i].m_wRTPPort,
                                pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
                                tNetSndParam.m_tRemoteNet[i].m_wRTCPPort);
    }

    if (nFindIndex == -1)
    {
        pbyRtpIP = (u8*)&dwDeleteIP;
        OspPrintf(TRUE, FALSE, "Not Find: RTP:%d.%d.%d.%d(%d)\n",
                                pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
                                wDeletePort);
        return;
    }

    if (nFindIndex < tNetSndParam.m_byNum - 1)
    {
        //move
        u8* pSrc = (u8*)&tNetSndParam.m_tRemoteNet[nFindIndex];
        u8* pDst = (u8*)&tNetSndParam.m_tRemoteNet[nFindIndex-1];
        memcpy(pDst, pSrc, sizeof(TNetSession)*(tNetSndParam.m_byNum - nFindIndex - 1));
    }

    memset(&tNetSndParam.m_tRemoteNet[tNetSndParam.m_byNum - 1], 0, sizeof(TNetSession));
    tNetSndParam.m_byNum--;

    wRet = pcSend->SetNetSndParam(tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "SetNetSndParam return:%d\n", wRet);
        return;
    }

    wRet = pcSend->GetNetSndParam(&tNetSndParam);
    if (wRet != MEDIANET_NO_ERROR)
    {
        OspPrintf(TRUE, FALSE, "GetNetSndParam return:%d\n", wRet);
        return;
    }


    //printf send socket info
    OspPrintf(TRUE, FALSE, " After Delete, Send info of Object:%d\n", nSendId);
    pbyRtpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTPAddr;
    pbyRtcpIP = (u8*)&tNetSndParam.m_tLocalNet.m_dwRTCPAddr;

    OspPrintf(TRUE, FALSE, "      Local: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
        pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
        tNetSndParam.m_tLocalNet.m_wRTPPort,
        pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
        tNetSndParam.m_tLocalNet.m_wRTCPPort);

    for (i = 0; i < tNetSndParam.m_byNum; i++)
    {
        pbyRtpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTPAddr;
        pbyRtcpIP = (u8*)&tNetSndParam.m_tRemoteNet[i].m_dwRTCPAddr;

        OspPrintf(TRUE, FALSE, "      Remote: RTP:%d.%d.%d.%d(%d) RTCP:%d.%d.%d.%d(%d)\n",
            pbyRtpIP[0], pbyRtpIP[1], pbyRtpIP[2], pbyRtpIP[3],
            tNetSndParam.m_tRemoteNet[i].m_wRTPPort,
            pbyRtcpIP[0], pbyRtcpIP[1], pbyRtcpIP[2], pbyRtcpIP[3],
            tNetSndParam.m_tRemoteNet[i].m_wRTCPPort);
    }
}


//���÷��Ͷ�������
API void setsendrate(s32 nSendId, u32 dwRate)
{
    if ((nSendId < 1) ||
        (nSendId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }
    CKdvMediaSnd* pcSend;
    pcSend = g_tMediaSndList.m_tMediaSndUnit[nSendId-1];

    pcSend->SetSndInfo(dwRate, 25);
}

API void pssinfo(s32 nSendId)
{
    if ((nSendId < 1) ||
        (nSendId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }

    CKdvMediaSnd* pcSend;
    pcSend = g_tMediaSndList.m_tMediaSndUnit[nSendId-1];

    if (NULL == pcSend)
    {
        return;
    }

    TKdvSndSocketInfo tRtpSockInfo;
    TKdvSndSocketInfo tRtcpSockInfo;
    pcSend->GetSndSocketInfo(tRtpSockInfo, tRtcpSockInfo);

    MedianetLog(Api, "Send object %d  socket info:\n", nSendId);
    MedianetLog(Api, "     RTP  UseRawSocket:%d IP:0x%x(%d)\n",
              tRtpSockInfo.m_bUseRawSocket, tRtpSockInfo.m_dwSrcIP, tRtpSockInfo.m_wPort);
    MedianetLog(Api, "     RTCP UseRawSocket:%d IP:0x%x(%d)\n",
              tRtcpSockInfo.m_bUseRawSocket, tRtcpSockInfo.m_dwSrcIP, tRtcpSockInfo.m_wPort);

}


/*
API void testsend(s32 nSendId)
{
    if ((nSendId < 1) ||
        (nSendId > g_tMediaSndList.m_nMediaSndCount) ||
        (NULL == g_tMediaSndList.m_tMediaSndUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }

    CKdvMediaSnd* pcSend;
    pcSend = g_tMediaSndList.m_tMediaSndUnit[nSendId-1];

    if (NULL == pcSend)
    {
        return;
    }

    TNetSession tSrcNet;
    tSrcNet.m_dwRTPAddr  = inet_addr("172.16.5.10");
    tSrcNet.m_dwRTCPAddr = inet_addr("172.16.5.10");
    tSrcNet.m_wRTPPort   = 1234;
    tSrcNet.m_wRTCPPort  = 1235;

    pcSend->SetSrcAddr(tSrcNet);
}


API void prsinfo(s32 nRecvId)
{
    if ((nRecvId < 1) ||
        (nRecvId > g_tMediaSndList.m_nMediaRcvCount) ||
        (NULL == g_tMediaSndList.m_tMediaRcvUnit[nSendId-1]))
    {
        MedianetLog(Api, "Send object %d is invalid!\n", nSendId);
        return;
    }
    CKdvMediaRcv* pcRecv;
    pcRecv = g_tMediaSndList.m_tMediaRcvUnit[nSendId-1];

    pcRecv->printsockinfo();
}
*/

/***********************************���緢�Ϳ�********************************/
#define CHECK_POINT_SND                      \
    if(NULL == m_pcNetSnd|| NULL == m_hSndSynSem)\
{                                      \
    return ERROR_SND_MEMORY;      \
}


CKdvMediaSnd::CKdvMediaSnd()
{
#ifndef WIN32
    KdvSocketStartup();
#endif

    MEDIANET_SEM_TAKE(g_hMediaSndSem);

    //���뷢�Ͷ��������м�¼����ָ��
    BOOL32 bFind = FALSE;
    if(g_tMediaSndList.m_nMediaSndCount < MAX_SND_NUM)
    {
        for(s32 nPos=0; nPos<g_tMediaSndList.m_nMediaSndCount; nPos++)
        {
            if(g_tMediaSndList.m_tMediaSndUnit[nPos] == this)
            {
                bFind = TRUE;
                break;
            }
        }
        if(FALSE == bFind)
        {
            g_tMediaSndList.m_nMediaSndCount++;
            g_tMediaSndList.m_tMediaSndUnit[g_tMediaSndList.m_nMediaSndCount-1] = this;
        }
    }

    m_hSndSynSem = NULL;
    //m_hSndSynSem ��ʼ���ź�
    if(!OspSemBCreate((SEMHANDLE*)(&m_hSndSynSem)))
    {
        m_hSndSynSem = NULL;
    }

    m_pcNetSnd = NULL;
    m_pcNetSnd = new CKdvNetSnd;

    MEDIANET_SEM_GIVE(g_hMediaSndSem);
}

CKdvMediaSnd::~CKdvMediaSnd()
{
    MEDIANET_SEM_TAKE(g_hMediaSndSem);

    if(NULL != m_pcNetSnd)
    {
        if(NULL != m_hSndSynSem)
        {
            MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
            SAFE_DELETE(m_pcNetSnd)
            MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);
        }
        else
        {
            SAFE_DELETE(m_pcNetSnd)
        }
    }

    if(m_hSndSynSem != NULL)
    {
        OspSemDelete((SEMHANDLE)m_hSndSynSem);
        m_hSndSynSem = NULL;
    }

    //�ӷ��Ͷ���������ɾ������ָ��
    memset(&g_tMediaSndListTmp, 0, sizeof(g_tMediaSndListTmp));
    for(s32 i=0; i<g_tMediaSndList.m_nMediaSndCount; i++)
    {
        if(g_tMediaSndList.m_tMediaSndUnit[i] == this) continue;
        g_tMediaSndListTmp.m_tMediaSndUnit[g_tMediaSndListTmp.m_nMediaSndCount] \
            = g_tMediaSndList.m_tMediaSndUnit[i];
        g_tMediaSndListTmp.m_nMediaSndCount++;
    }
    g_tMediaSndList.m_nMediaSndCount = g_tMediaSndListTmp.m_nMediaSndCount;
    for(s32 j=0; j<g_tMediaSndListTmp.m_nMediaSndCount; j++)
    {
        g_tMediaSndList.m_tMediaSndUnit[j] =
            g_tMediaSndListTmp.m_tMediaSndUnit[j];
    }

    //g_tMediaSndList

    MEDIANET_SEM_GIVE(g_hMediaSndSem);

#ifndef WIN32
    KdvSocketCleanup();
#endif
}

/*=============================================================================
    ������        ��Create
    ����        �� �������Ͷ���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     u32 dwMaxFrameSize ���֡��С
                     u32 dwNetBand   ����
                     u8 ucFrameRate  ֡��
                     u8 ucMediaType  ý�����ͣ�MP4��H264�ȣ�
                     u32 dwSSRC    ͬ��Դ��Ĭ��Ϊ0��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::Create(u32 dwMaxFrameSize, u32 dwNetBand, u8 ucFrameRate,
                           u8 ucMediaType, u32 dwSSRC /* = 0*/, TPSInfo* ptPSInfo /*= NULL*/)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->Create(dwMaxFrameSize, dwNetBand, ucFrameRate, ucMediaType, dwSSRC, ptPSInfo);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��SetNetSndParam
    ����        �� �������緢�Ͳ���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     TNetSndParam tNetSndParam ���緢�Ͳ����ṹ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::SetNetSndParam( TNetSndParam tNetSndParam )
{
    CHECK_POINT_SND

    MedianetLog(Api, "\n SetNetSndParam Start, Print Argument List \n");
    MedianetLog(Api, "RTPAddr     RTPPort    RTCPAddr    RTCPPort  \n");
    MedianetLog(Api, "%x     %d     %x     %d      \n",
        tNetSndParam.m_tLocalNet.m_dwRTPAddr, tNetSndParam.m_tLocalNet.m_wRTPPort,
        tNetSndParam.m_tLocalNet.m_dwRTCPAddr, tNetSndParam.m_tLocalNet.m_wRTCPPort);

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->SetNetSndParam (tNetSndParam);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    MedianetLog(Api, "\n SetNetSndParam End, wRet:%d \n", wRet);

    return wRet;
}

u16 CKdvMediaSnd::GetNetSndParam (TNetSndParam* ptNetSndParam)
{
    CHECK_POINT_SND

    if (NULL == ptNetSndParam)
    {
        return ERROR_SND_PARAM;
    }

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->GetNetSndParam (ptNetSndParam);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::SetSrcAddr(TNetSession tSrcNet)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->SetSrcAddr(tSrcNet);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);
    
	return wRet;
}

//����NAT ̽��� ����, dwInterval=0 ʱ��ʾ�����͡�
u16 CKdvMediaSnd::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;

	if(NULL != m_pcNetSnd && NULL != m_hSndSynSem) 
	{ 
		OspSemTake((SEMHANDLE)m_hSndSynSem);
		if(NULL != m_pcNetSnd)
		{
			wRet = m_pcNetSnd->SetNatProbeProp(ptNatProbeProp);

			//����ok ����������һ��
			if(wRet == MEDIANET_NO_ERROR && ptNatProbeProp->dwInterval)
			{
				wRet = m_pcNetSnd->DealNatProbePack();
			}
		}
		OspSemGive((SEMHANDLE)m_hSndSynSem);
	}

	return wRet;
}

//����̽����ӿ�
u16 CKdvMediaSnd::DealNatProbePack(u32 dwNowTs)
{
	CHECK_POINT_SND
		
	u16 wRet = ERROR_SND_MEMORY;
	
	OspSemTake((SEMHANDLE)m_hSndSynSem);
	if(NULL != m_pcNetSnd)
	{
		wRet = m_pcNetSnd->DealNatProbePack(dwNowTs);
	}
	OspSemGive((SEMHANDLE)m_hSndSynSem);

	return wRet;
}
u16 CKdvMediaSnd::GetSndSocketInfo(TKdvSndSocketInfo &tRtpSockInfo, TKdvSndSocketInfo &tRtcpSockInfo)
{
    memset(&tRtpSockInfo, 0, sizeof(tRtpSockInfo));
    memset(&tRtcpSockInfo, 0, sizeof(tRtpSockInfo));

    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->GetSndSocketInfo(tRtpSockInfo, tRtcpSockInfo);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��RemoveNetSndLocalParam
    ����        �� �Ƴ����緢�Ͳ���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::RemoveNetSndLocalParam ()
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->RemoveNetSndLocalParam();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    MedianetLog(Api, "\n RemoveNetSndLocalParam End, wRet:%d \n", wRet);

    return wRet;
}

/*=============================================================================
    ������        ��SetActivePT
    ����        ������ ��̬�غɵ� Playloadֵ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����byLocalActivePT ���˷��͵Ķ�̬�غ�PTֵ, �ɶԺ�ʱԼ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::SetActivePT( u8 byLocalActivePT )
{
    MedianetLog(Api, "[CKdvMediaSnd::SetActivePT] byLocalActivePT:%d\n", byLocalActivePT);

    BOOL32 wRet = ERROR_SND_MEMORY;

    if(NULL != m_pcNetSnd)
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
        if(NULL != m_pcNetSnd)
        {
            wRet = m_pcNetSnd->SetActivePT(byLocalActivePT);
        }
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);
    }

    MedianetLog(Api, "[CKdvMediaSnd::SetActivePT] End, wRet:%d \n", wRet);

    return wRet;
}

/*=============================================================================
    ������        ��SetEncryptKey
    ����        �����ü���key�ִ������������Ķ�̬�غ�PTֵ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pszKeyBuf      ����key�ִ�����ָ�� NULL-��ʾ������
                    wKeySize       ����key�ִ����峤��
                    byEncryptPT    ����ģʽ Aes ���� Des -- default:des

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::SetEncryptKey( s8 *pszKeyBuf, u16 wKeySize,
                                 u8 byEncryptMode/*=DES_ENCRYPT_MODE*/ )
{
    MedianetLog(Api, "[CKdvMediaSnd::SetEncryptKey] Start, Print Argument List \n");
    if(NULL == pszKeyBuf)
    {
        MedianetLog(Api, "pszKeyBuf:NULL, wKeySize%d\n", wKeySize);
    }
    else
    {
        MedianetLog(Api, "pszKeyBuf:%02x, wKeySize%d, byEncryptMode:%d\n",
                         pszKeyBuf, wKeySize, byEncryptMode);
    }

    BOOL32 wRet = ERROR_SND_MEMORY;

    if(NULL != m_pcNetSnd )
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
        if(NULL != m_pcNetSnd)
            wRet = m_pcNetSnd->SetEncryptKey(pszKeyBuf, wKeySize, byEncryptMode);
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);
    }

    MedianetLog(Api, "[CKdvMediaSnd::SetEncryptKey] End, wRet:%d \n", wRet);

    return wRet;
}


//���� Snd rtcpinfo�ص�����
u16 CKdvMediaSnd::SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->SetRtcpInfoCallback(pRtcpInfoCallback, pContext);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::SetSndInfo(u32 dwNetBand, u8 byFrameRate)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->SetSndInfo(dwNetBand, byFrameRate);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::ResetFrameId()
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->ResetFrameId();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

//����ͬ��ԴSSRC
u16 CKdvMediaSnd::ResetSSRC(u32 dwSSRC /*= 0*/)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->ResetSSRC (dwSSRC);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

//���÷��Ͷ˶���mpeg4����H.264���õ��ش�����Ŀ���,�رպ󣬽������Ѿ����͵����ݰ����л���
u16 CKdvMediaSnd::ResetRSFlag(u16 wBufTimeSpan, BOOL32 bRepeatSnd /*=FALSE*/)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->ResetRSFlag (wBufTimeSpan, bRepeatSnd);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��Send
    ����        �� ����һ֡����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     PFRAMEHDR pFrmHdr
                     BOOL32 bAvalid ����֡�Ƿ���Ч��Ĭ��ΪTRUE��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaSnd::Send(PFRAMEHDR pFrmHdr, BOOL32 bAvalid /*= TRUE*/, BOOL32 bSendRtp)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->Send(pFrmHdr, bAvalid, bSendRtp);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::Send(TRtpPack *pRtpPack, BOOL32 bTrans /*= TRUE*/, BOOL32 bAvalid /*= TRUE*/, BOOL32 bSendRtp /*TRUE*/)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->Send(pRtpPack, bTrans, bAvalid, bSendRtp);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::GetStatus (TKdvSndStatus &tKdvSndStatus)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->GetStatus(tKdvSndStatus);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::GetStatistics ( TKdvSndStatistics &tKdvSndStatistics)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->GetStatistics(tKdvSndStatistics);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

u16 CKdvMediaSnd::GetAdvancedSndInfo(TAdvancedSndInfo &tAdvancedSndInfo)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->GetAdvancedSndInfo(tAdvancedSndInfo);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}

//�����Բ���
u16 CKdvMediaSnd::SelfTestSend(s32 nFrameLen, s32 nSndNum, s32 nSpan)
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->SelfTestSend(nFrameLen, nSndNum, nSpan);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

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
u16 CKdvMediaSnd::DealRtcpTimer()
{
    CHECK_POINT_SND

    u16 wRet = ERROR_SND_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSndSynSem);
    if(NULL != m_pcNetSnd)
    {
        wRet = m_pcNetSnd->DealRtcpTimer();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSndSynSem);

    return wRet;
}


u16 CKdvMediaSnd::SetMaxSendPackSize(u32 dwMaxSendPackSize)
{
    if (dwMaxSendPackSize > MAX_EXTEND_PACK_SIZE)
    {
        OspPrintf(TRUE, FALSE, "can't set size %d > MAX_EXTEND_PACK_SIZE \n", dwMaxSendPackSize, MAX_EXTEND_PACK_SIZE);
        return ERROR_SND_PARAM;
    }

    g_dwMaxExtendPackSize = dwMaxSendPackSize;

    return MEDIANET_NO_ERROR;
}

u16 CKdvMediaSnd::SetAaclcSend(BOOL32 bNoHead)
{
    u16 wRet = MEDIANET_NO_ERROR;
    CHECK_POINT_SND
    if(NULL != m_pcNetSnd)
        {
            wRet = m_pcNetSnd->SetAaclcSend(bNoHead);
    }

    return wRet;
}

/***************************������տ�****************************************/

#define CHECK_POINT_RCV                      \
    if(NULL == m_pcNetRcv|| NULL == m_hRcvSynSem)\
{                                      \
    return ERROR_SND_MEMORY;      \
}


CKdvMediaRcv::CKdvMediaRcv()
{
#ifndef WIN32
    KdvSocketStartup();
#endif

    MEDIANET_SEM_TAKE(g_hMediaRcvSem);

    //������ն��������м�¼����ָ��
    BOOL32 bFind = FALSE;
    if(g_tMediaRcvList.m_nMediaRcvCount < FD_SETSIZE)
    {
        for(s32 nPos=0; nPos<g_tMediaRcvList.m_nMediaRcvCount; nPos++)
        {
            if(g_tMediaRcvList.m_tMediaRcvUnit[nPos] == this)
            {
                bFind = TRUE;
                break;
            }
        }
        if(FALSE == bFind)
        {
            g_tMediaRcvList.m_nMediaRcvCount++;
            g_tMediaRcvList.m_tMediaRcvUnit[g_tMediaRcvList.m_nMediaRcvCount-1] = this;
        }
    }

    m_hRcvSynSem = NULL;
    //m_hRcvSynSem ��ʼ���ź�
    if(!OspSemBCreate((SEMHANDLE*)(&m_hRcvSynSem)))
    {
        m_hRcvSynSem = NULL;
    }

    m_pcNetRcv = NULL;
    m_pcNetRcv = new CKdvNetRcv;

    MEDIANET_SEM_GIVE(g_hMediaRcvSem);
}

CKdvMediaRcv::~CKdvMediaRcv()
{
    MEDIANET_SEM_TAKE(g_hMediaRcvSem);

    if(NULL != m_pcNetRcv)
    {
        if(NULL != m_hRcvSynSem)
        {
            MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
            SAFE_DELETE(m_pcNetRcv)
            MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);
        }
        else
        {
            SAFE_DELETE(m_pcNetRcv)
        }
    }

    if(NULL != m_hRcvSynSem)
    {
        OspSemDelete((SEMHANDLE)m_hRcvSynSem);
        m_hRcvSynSem = NULL;
    }

    //�ӽ��ն���������ɾ������ָ��
    memset(&g_tMediaRcvListTmp, 0, sizeof(g_tMediaRcvListTmp));
    for(s32 i=0; i<g_tMediaRcvList.m_nMediaRcvCount; i++)
    {
        if(g_tMediaRcvList.m_tMediaRcvUnit[i] == this) continue;
        g_tMediaRcvListTmp.m_tMediaRcvUnit[g_tMediaRcvListTmp.m_nMediaRcvCount] \
            = g_tMediaRcvList.m_tMediaRcvUnit[i];
        g_tMediaRcvListTmp.m_nMediaRcvCount++;
    }
    g_tMediaRcvList.m_nMediaRcvCount = g_tMediaRcvListTmp.m_nMediaRcvCount;
    for(s32 j=0; j<g_tMediaRcvListTmp.m_nMediaRcvCount; j++)
    {
        g_tMediaRcvList.m_tMediaRcvUnit[j] =
            g_tMediaRcvListTmp.m_tMediaRcvUnit[j];
    }
    //g_tMediaRcvList

    MEDIANET_SEM_GIVE(g_hMediaRcvSem);

#ifndef WIN32
    KdvSocketCleanup();
#endif
}

/*=============================================================================
    ������        ��Create
    ����        �� �������ն���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     u32 dwMaxFrameSize ���֡��С
                     PFRAMEPROC pFrameCallBackProc ֡�ص�����
                     u32 dwContext ������
                     u32 dwSSRC ͬ��Դ��Ĭ��Ϊ0��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::Create( u32 dwMaxFrameSize,
                           PFRAMEPROC pFrameCallBackProc, KD_PTR pContext,
                           u32 dwSSRC /* = 0*/)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->Create (dwMaxFrameSize, pFrameCallBackProc, (void*)pContext, dwSSRC);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��Create
    ����        �� �������ն���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     u32 dwMaxFrameSize ���֡��С
                     PRTPCALLBACK pRtpCallBackProc ���ص�����
                     u32 dwContext ������
                     u32 dwSSRC ͬ��Դ��Ĭ��Ϊ0��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::Create ( u32 dwMaxFrameSize,
                           PRTPCALLBACK pRtpCallBackProc, KD_PTR pContext,
                           u32 dwSSRC /* = 0*/)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->Create (dwMaxFrameSize, pRtpCallBackProc, (void*)pContext, dwSSRC);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��SetNetRcvLocalParam
    ����        �� ���ñ��ؽ����������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     TLocalNetParam tLocalNetParam �����������
                     u32 dwFlag    ��־������Դ����������յȣ�MEDIANETRCV_FLAG_FROM_RECVFROM��
                     u32 dwRegFunc   ע��ص������� Ĭ��Ϊ0��
                     u32 dwUnregFunc   ��Ĭ��Ϊ0��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::SetNetRcvLocalParam( TLocalNetParam tLocalNetParam, u32 dwFlag, void* pRegFunc, void* pUnregFunc )
{
    MedianetLog(Api, "\nCKdvMediaRcv::SetNetRcvLocalParam Start, Print Argument List \n");
    MedianetLog(Api, "RTPAddr     RTPPort    RTCPAddr    RTCPPort    RtcpBackAddr    RtcpBackPort\n");
    MedianetLog(Api, "%x     %d     %x     %d     %x     %d \n",
        tLocalNetParam.m_tLocalNet.m_dwRTPAddr, tLocalNetParam.m_tLocalNet.m_wRTPPort,
        tLocalNetParam.m_tLocalNet.m_dwRTCPAddr, tLocalNetParam.m_tLocalNet.m_wRTCPPort,
        tLocalNetParam.m_dwRtcpBackAddr, tLocalNetParam.m_wRtcpBackPort);
    MedianetLog(Api, "dwFlag:[%d], pRegFunc:[%x], pUnregFunc:[%x]\n", dwFlag, pRegFunc, pUnregFunc );
    u16 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv)
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
        {
            wRet = m_pcNetRcv->SetNetRcvLocalParam (tLocalNetParam, dwFlag, pRegFunc, pUnregFunc);
        }
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);
    }

    MedianetLog(Api, "\n SetNetRcvLocalParam End, wRet:%d \n", wRet);

    return wRet;
}

//�ش���natʱ�����ñ�����rtp���ն˿ڶ�Ӧ�Ĺ�����ַ,Ŀ��Ϊʹ�ش�ʱ���ù㲥
u16 CKdvMediaRcv::SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort)
{
    u16 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv && NULL != m_hRcvSynSem)
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
        {
            wRet = m_pcNetRcv->SetRtpPublicAddr( dwRtpPublicIp, wRtpPublicPort);
        }
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);
    }

    return wRet;
}

//����NAT ̽��� ����, dwInterval=0 ʱ��ʾ�����͡�
u16 CKdvMediaRcv::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
    u16 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv && NULL != m_hRcvSynSem)
    {
        OspSemTake((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
        {
            wRet = m_pcNetRcv->SetNatProbeProp(ptNatProbeProp);

            //����ok ����������һ��
            if(wRet == MEDIANET_NO_ERROR && ptNatProbeProp->dwInterval)
            {
                wRet = m_pcNetRcv->DealNatProbePack();
            }
        }
        OspSemGive((SEMHANDLE)m_hRcvSynSem);
    }

    return wRet;
}

//����̽����ӿ�
u16 CKdvMediaRcv::DealNatProbePack(u32 dwNowTs)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    OspSemTake((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->DealNatProbePack(dwNowTs);
    }
    OspSemGive((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��RemoveNetRcvLocalParam
    ����        �� �Ƴ����ؽ����������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::RemoveNetRcvLocalParam()
{
    MedianetLog(Api, "\n RemoveNetRcvLocalParam Start, Print Argument List \n");
    MedianetLog(Api, "RTPAddr     RTPPort    RTCPAddr    RTCPPort\n");
    MedianetLog(Api, "%x     %d     %x     %d   \n",
        m_pcNetRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr, m_pcNetRcv->m_tLocalNetParam.m_tLocalNet.m_wRTPPort,
        m_pcNetRcv->m_tLocalNetParam.m_tLocalNet.m_dwRTCPAddr, m_pcNetRcv->m_tLocalNetParam.m_tLocalNet.m_wRTCPPort);

    u16 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv && NULL != m_hRcvSynSem)
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
        {
            wRet = m_pcNetRcv->RemoveNetRcvLocalParam();
        }
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);
    }

    MedianetLog(Api, "\n RemoveNetRcvLocalParam End, wRet:%d \n", wRet);

    return wRet;
}

/*=============================================================================
    ������        : ResetRSFlag
    ����        �����ý��ն˶���mpeg4���õ��ش�����Ŀ���,�رպ󣬽��������ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::ResetRSFlag(TRSParam tRSParam, BOOL32 bRepeatSnd /*=FALSE*/)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->ResetRSFlag (tRSParam, bRepeatSnd);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//�����ϲ��������sps �� pps ��Ϣ��
u16 CKdvMediaRcv::ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->ParseSpsPpsBuf(pbySpsBuf, nSpsBufLen, pbyPpsBuf, nPpsBufLen);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        : SetActivePT
    ����        ������ ��̬�غɵ� Playloadֵ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  byRmtActivePT ���յ��Ķ�̬�غɵ�Playloadֵ, �ɶԺ�ʱ�Է���֪��
                                0-��ʾ��� Զ�˶�̬�غɱ��
                  byRealPT      �ö�̬�غ�ʵ�ʴ���ĵ�Playload���ͣ���ͬ�����Ƿ���ʱ��PTԼ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::SetActivePT( u8 byRmtActivePT, u8 byRealPT )
{
    MedianetLog(Api, "[CKdvMediaRcv::SetActivePT] byRmtActivePT:%d, byRealPT%d\n",
                     byRmtActivePT, byRealPT);

    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->SetActivePT( byRmtActivePT, byRealPT );
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    MedianetLog(Api, "[CKdvMediaRcv::SetActivePT] End, wRet:%d \n", wRet);

    return wRet;
}

/*=============================================================================
    ������        : InputRtpPack
    ����        ������rtp�������ؽM֡
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
            pRtpBuf  rtp��ָ�룬
            dwRtpBuf  rtp����С

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::InputRtpPack(u8 * pRtpBuf, u32 dwRtpBuf)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->InputRtpPack(pRtpBuf, dwRtpBuf);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//���ò����ʺ���
u16 CKdvMediaRcv::SetTimestampSample(u32 dwSample)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    OspSemTake((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
        wRet = m_pcNetRcv->SetTimestampSample(dwSample);
    OspSemGive((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        ��SetDecryptKey
    ����        �����ý��ս���key�ִ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pszKeyBuf      ����key�ִ�����ָ�� NULL-��ʾ������
                    wKeySize       ����key�ִ����峤��
                    byDecryptMode  ����ģʽ Aes ���� Des -- default:des

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::SetDecryptKey( s8 *pszKeyBuf, u16 wKeySize,
                                 u8 byDecryptMode/*=DES_ENCRYPT_MODE*/ )
{
    MedianetLog(Api, "[CKdvMediaRcv::SetDecryptKey] Start, Print Argument List \n");
    if(NULL == pszKeyBuf)
    {
        MedianetLog(Api, "pszKeyBuf:NULL, wKeySize%d\n", wKeySize);
    }
    else
    {
        MedianetLog(Api, "pszKeyBuf:%02x, wKeySize%d, byDecryptMode%d\n",
                         pszKeyBuf, wKeySize, byDecryptMode);
    }

    BOOL32 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv && NULL != m_hRcvSynSem)
    {
        MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
            wRet = m_pcNetRcv->SetDecryptKey(pszKeyBuf, wKeySize, byDecryptMode);
        MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);
    }

    MedianetLog(Api, "[CKdvMediaRcv::SetDecryptKey] End, wRet:%d \n", wRet);

    return wRet;
}

//���ý��ն˶��� (mp3) �Ƿ���������������Ŀ���, �رպ󣬽�������
u16 CKdvMediaRcv::ResetCAFlag(BOOL32 bConfuedAdjust /*= TRUE*/)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->ResetCAFlag (bConfuedAdjust);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//���ps֡�ص��ӿڣ��������Ƿ�ص�ȥpsͷ��֡ ��־λ��
u16 CKdvMediaRcv::AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->AddPsFrameCallBack(pFrameCallBackProc, bCallBackFrameWithOutPs, pContext);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//�����Ƿ����4k ����
u16 CKdvMediaRcv::SetIs4k(BOOL32 bis4k  /*false*/)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->SetIs4k(bis4k);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//����RTP�ص��ӿ���Ϣ
u16 CKdvMediaRcv::ResetRtpCalllback(PRTPCALLBACK pRtpCallBackProc, KD_PTR pContext, TRtpCallbackType emCallbackType )
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->ResetRtpCalllback(pRtpCallBackProc, (void*)pContext, emCallbackType);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//���� Rcv rtcpinfo�ص�����
u16 CKdvMediaRcv::SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, KD_PTR pContext)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->SetRtcpInfoCallback(pRtcpInfoCallback, (void*)pContext);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}
/*=============================================================================
    ������        ��StartRcv
    ����        �� ��ʼ����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::StartRcv()
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->StartRcv();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

/*=============================================================================
    ������        �� StopRcv
    ����        �� ֹͣ����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵��:
                     ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvMediaRcv::StopRcv()
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->StopRcv();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::GetStatus( TKdvRcvStatus &tKdvRcvStatus )
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->GetStatus(tKdvRcvStatus);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}


//��ȡ���ն�������֡����
u16 CKdvMediaRcv::GetMaxFrameSize(u32 &dwMaxFrameSize)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->GetMaxFrameSize(dwMaxFrameSize);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

//��ȡ���ն��� ��ǰ���յ�����������
u16 CKdvMediaRcv::GetLocalMediaType(u8 &byMediaType)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->GetLocalMediaType(byMediaType);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->GetAdvancedRcvInfo(tAdvancedRcvInfo);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::GetStatistics ( TKdvRcvStatistics &tKdvRcvStatistics )
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->GetStatistics(tKdvRcvStatistics);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

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
u16 CKdvMediaRcv::DealRtcpTimer()
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->DealRtcpTimer();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::RelayRtpStart(u32 dwIP, u16 wPort)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->RelayRtpStart(dwIP, wPort);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::RelayRtpStop()
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;

    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if(NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->RelayRtpStop();
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;
}

u16 CKdvMediaRcv::SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback)
{
    CHECK_POINT_RCV

    u16 wRet = ERROR_NET_RCV_MEMORY;
    MEDIANET_SEM_TAKE((SEMHANDLE)m_hRcvSynSem);
    if (NULL != m_pcNetRcv)
    {
        wRet = m_pcNetRcv->SetNoPPSSPSStillCallback(bNoPPSSPSStillCallback);
    }

    MEDIANET_SEM_GIVE((SEMHANDLE)m_hRcvSynSem);

    return wRet;

}

u16 CKdvMediaRcv::ResetAllPackets()
{
    CHECK_POINT_RCV;
    OspSemTake((SEMHANDLE)m_hRcvSynSem);
    OspSemTake(m_pcNetRcv->m_hSynSem);
    m_pcNetRcv->ResetAllPackets();
    OspSemGive(m_pcNetRcv->m_hSynSem);
    OspSemGive((SEMHANDLE)m_hRcvSynSem);
    return MEDIANET_NO_ERROR;
}

u16 CKdvMediaRcv::SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp/* = FALSE*/)
{
    CHECK_POINT_RCV;
    OspSemTake((SEMHANDLE)m_hRcvSynSem);
    OspSemTake(m_pcNetRcv->m_hSynSem);
    m_pcNetRcv->SetCompFrameByTimeStamp(bCompFrameByTimeStamp);
    OspSemGive(m_pcNetRcv->m_hSynSem);
    OspSemGive((SEMHANDLE)m_hRcvSynSem);
    return MEDIANET_NO_ERROR;
}

u16 CKdvMediaRcv::SetAaclcTransMode(EAacTransMode eTransMode)
{
    CHECK_POINT_RCV;
    OspSemTake((SEMHANDLE)m_hRcvSynSem);
    OspSemTake(m_pcNetRcv->m_hSynSem);
    m_pcNetRcv->SetAaclcTransMode(eTransMode);
    OspSemGive(m_pcNetRcv->m_hSynSem);
    OspSemGive((SEMHANDLE)m_hRcvSynSem);
    return MEDIANET_NO_ERROR;
}

u16 CKdvMediaRcv::SetStreamType(ENetStreamType eStreamType)
{
	CHECK_POINT_RCV;
	OspSemTake((SEMHANDLE)m_hRcvSynSem);
	OspSemTake(m_pcNetRcv->m_hSynSem);
	m_pcNetRcv->SetStreamType(eStreamType);
	OspSemGive(m_pcNetRcv->m_hSynSem);
	OspSemGive((SEMHANDLE)m_hRcvSynSem);
	return MEDIANET_NO_ERROR;
}

API void medianetrcvtest()
{

    CKdvMediaRcv* pcRcv;
    pcRcv = new CKdvMediaRcv;
    if( pcRcv == NULL )
    {
        return;
    }
    pcRcv->Create(1000, (PFRAMEPROC)0x8000000, NULL, 0x12334);

}


API void medianetsndtest()
{
    CKdvMediaSnd* pcSnd;
    pcSnd = new CKdvMediaSnd;
    if( pcSnd == NULL )
    {
        return;
    }
    pcSnd->Create(1000, 64*1024, 30, 0, 0x1234);
}

/*End of File*/
