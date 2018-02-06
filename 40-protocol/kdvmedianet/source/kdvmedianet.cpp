/*****************************************************************************
模块名      : KdvMediaNet
文件名      : KdvMediaNet.cpp
相关文件    : KdvMediaNet .h
文件实现功能: CKdvMediaSnd,CKdvMediaRcv Implement
作者        : 魏治兵
版本        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2003/06/03  2.0         魏治兵      Create
2004/04/13  3.0         万春雷      增加重传处理功能
2004/04/16  3.0         万春雷      增加 接收端模拟丢弃小包的步长间隔设置
2004/04/21  3.0         万春雷      调整重传处理功能中的网络发送的结构体数据为网络序
2004/04/30  3.0         万春雷      调整 H.261 末尾清零
2004/05/18  3.0         万春雷      增加 H.261 、 H.263 按照原始rtp包直接发送的开关
                                    发送对象对于 H.264以及 mp4 才进行环形缓冲的分配
                                    接收对象的帧缓冲按照用户传递的 m_dwMaxFrameSize 为准分配内存
2004/06/08  3.0         万春雷      增加 H.263 对于 B MODE 、 C MODE 的解析兼容
2004/06/08  3.0         万春雷      调整发送时的最大包长 1450 , 接收时的最大包长 8192,
                                    两者分别以不同的宏定义标识
2004/09/29  2.0         万春雷      增加linux版本编译支持
2004/09/30  3.0         万春雷      接收端增加对于加密码流的解密的支持
2004/09/30  3.0         万春雷      发送端增加对于发送码流的加密的支持
2005/06/03  3.6         华亮        增加原始码流调试转发功能
******************************************************************************/


#include "kdvnetsnd.h"
#include "kdvnetrcv.h"
#include "kdvmedianet.h"

BOOL32 g_bUseMemPool = FALSE;

//模块版本信息和编译时间 及 依赖的库的版本和编译时间
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

//模块帮助信息 及 依赖的库的帮助信息
API void kdvmedianethelp()
{
    kdvmedianetver();

    MedianetPrintf("\nprintsock()  －－ 收发套结子基本状态信息\n");
    MedianetPrintf("pbsend(u32 objectseq) －－某一个发送对象基本状态信息及统计信息\n");
    MedianetPrintf("pbrecv(u32 objectseq) －－某一个接收对象基本状态信息及统计信息\n");

    MedianetPrintf("pbinfo(BOOL32 bShowRcv, BOOL32 bShowSnd) －－收发对象基本状态信息及统计信息\n");
    MedianetPrintf("pdinfo(s32 nShowDebugInfo) －－是否显示隐藏的一些调试状态信息\n");

    MedianetPrintf("0 -- 隐藏所有debug信息 \n");
    MedianetPrintf("1 -- 实时显示接收丢包debug信息 \n");
    MedianetPrintf("2 -- 实时显示发送包debug(包括重传请求的发送及应答状态)信息 \n");
    MedianetPrintf("3 -- 显示接收对象创建的信息 \n");
    MedianetPrintf("4 -- 显示发送对象创建的信息 \n");
    MedianetPrintf("5 -- 显示全局接收线程状态活性 \n");
    MedianetPrintf("6 -- 显示全局RTCP    包定时上报线程状态活性 \n");
    MedianetPrintf("7 -- 实时显示接收到的实际数据包信息 \n");
    MedianetPrintf("8 -- 实时显示接收到的视频帧回调信息 \n");
    MedianetPrintf("9 -- 实时显示组帧回调过程 \n");
    MedianetPrintf("10 -- 实时显示 测试用 接收收模拟udp包信息  \n");
    MedianetPrintf("11 -- 实时显示 发送端对于 sbit/ebit校验信息  \n");
    MedianetPrintf("12 -- 实时显示 接收端对于 sbit/ebit校验信息  \n");
    MedianetPrintf("13 -- 实时显示 接收到的音频帧回调信息  \n");
    MedianetPrintf("14 -- 实时显示 加密过程的异常返回信息  \n");
    MedianetPrintf("15 -- 实时显示 解密过程的异常返回信息  \n");
    MedianetPrintf("16 -- 实时显示缓冲满信息  \n");
    MedianetPrintf("17 -- 实时包未插入信息(过期包)  \n");
    MedianetPrintf("33 -- 查看添加的ps帧回调信息  \n");
    MedianetPrintf("255 -- 显示除5外的所有调试信息 \n");

    MedianetPrintf("1) rsopen(BOOL32 bRcvCallback, BOOL32 bSelfSnd)  －－收发控制开关\n");
    MedianetPrintf("2) stest(s32 nSndObjIndex, s32 nFrameLen, s32 nSndNum, s32 nSpan)－－使用已有对象发送自测包\n");
    MedianetPrintf("3) setconfuedadjust(int nbConfuedAdjust)－－mp3/g.7xx 音频乱序调整处理的开关\n");
    MedianetPrintf("4) setrepeatsend(s32 nRepeatSnd)－－ 重传控制开关\n");
    MedianetPrintf("5) seth263dsend(s32 nRtpDSend)－－ H.263 按照原始rtp包直接发送的开关\n");
    MedianetPrintf("6) setdiscardspan(s32 nDiscardSpan) －－发送端模拟丢弃小包的步长间隔设置 \n");
    MedianetPrintf("7) setrcvdiscardspan(s32 nRcvDiscardSpan) －－接收端模拟丢弃小包的步长间隔设置 \n");
    MedianetPrintf("8) setconfuedspan(s32 nConfuedSpan) －－发送端模拟 H.263小包乱序的步长间隔 \n");

    MedianetPrintf("9)  startrcvtask(int nCurRcvPort) －－ 禁用，在指定进行udp包数据接收模拟 \n");
    MedianetPrintf("10) showrcvresult( )              －－ 禁用，显示udp包数据接收模拟结果 \n");
    MedianetPrintf("11) startsndtask(int nPackNum, int nPackLen, int nRaw, int nCurSndPort, int nAddrIP=0) －－ 禁用，在指定进行udp包数据接收模拟 \n");
    MedianetPrintf("12) showsimresult( )              －－ 禁用，显示udp包数据收发模拟结果 \n");

    MedianetPrintf("13) showsmoothsnd( )              －－ 禁用，显示是否对嵌入式操作系统设置平滑发送\n");
    MedianetPrintf("14) disablesmoothsnd( )           －－ 禁用，关闭对嵌入式操作系统设置平滑发送 \n");
    MedianetPrintf("15) setrawsend(s32 nRawSend)      －－ 禁用，是否使用 raw-socket 进行数据包投递 \n");
    MedianetPrintf("16) settos(s32 nTOS)              －－ 禁用，设置发送socket的数据包的TOS值，默认设置为 EF 策略 \n");
    MedianetPrintf("17) setttl(s32 nTTL)              －－ 禁用，设置发送socket的数据包的ttl值 \n");
    MedianetPrintf("18) setchecksum(s32 nchecksum)    －－ 禁用，显示udp包数据收发模拟结果 \n");
    MedianetPrintf("19) mediarelaystart(s32 nRecvId, s8* pchIpStr, u16 wPort) －－rtp转发开始\n");
    MedianetPrintf("20) mediarelaystop()              －－rtp转发停止\n");
    MedianetPrintf("21) mediarecvframerate(s32 nRecvId, s32 nSecond)   －－接收帧率统计\n");
    MedianetPrintf("22) mediasndframerate(s32 nSndId, s32 nSecond))    －－发送帧率统计\n");
    MedianetPrintf("23) pssinfo(s32 nSndId))          －－发送socket信息\n");
    MedianetPrintf("24) mediasendadd(s32 nSendId, s8* pchIpStr, u16 wPort) －－手工增加一路码流发送\n");
    MedianetPrintf("25) mediasenddel(s32 nSendId, s8* pchIpStr, u16 wPort) －－手工删除一路码流发送\n");

}

extern TMediaSndList   g_tMediaSndList;     //发送对象列表全局变量
extern TMediaRcvList   g_tMediaRcvList;     //接收对象列表全局变量
extern TMediaSndList   g_tMediaSndListTmp;
extern TMediaRcvList   g_tMediaRcvListTmp;

extern SEMHANDLE   g_hMediaSndSem;    //发送对象列表的访问维护的信号量
extern SEMHANDLE   g_hMediaRcvSem;    //接收对象列表的访问维护的信号量

s32   g_nDiscardSpan = 0;            //发送端模拟丢弃小包的步长间隔

API void setdiscardspan(s32 nDiscardSpan)
{
    g_nDiscardSpan = nDiscardSpan;
}

s32   g_nRcvDiscardSpan = 0;        //接收端模拟丢弃小包的步长间隔

API void setrcvdiscardspan(s32 nRcvDiscardSpan)
{
    g_nRcvDiscardSpan = nRcvDiscardSpan;
}


s32   g_nRepeatSnd = 1;        //重传开关

API void setrepeatsend(s32 nRepeatSnd)
{
    g_nRepeatSnd = nRepeatSnd;
}


s32   g_nRtpDSend = 0; //H.263 按照原始rtp包直接发送的开关
API void seth263dsend(s32 nRtpDSend)
{
    g_nRtpDSend = nRtpDSend;
}

s32   g_nConfuedSpan = 0;        //发送端模拟 H.263小包乱序的步长间隔
s32   g_nSpanChanged = 0;        //发送端模拟 H.263小包乱序的步长间隔是否发生变化

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

s32   g_nShowDebugInfo = 0;        //是否显示隐藏的一些调试状态信息

/*
0 -- 隐藏所有debug信息
1 -- 实时显示接收丢包debug信息
2 -- 实时显示发送包debug(包括重传请求的发送及应答状态)信息
3 -- 显示接收对象创建的信息
4 -- 显示发送对象创建的信息
5 -- 显示全局接收线程状态活性

7 -- 实时显示接收到的实际数据包信息
8 -- 实时显示接收到的实际数据帧信息

255 -- 显示除5外的所有调试信息
*/
API void pdinfo(s32 nShowDebugInfo)
{
    g_nShowDebugInfo = nShowDebugInfo;
}

//收发控制开关
BOOL32  g_bInterRcvCallBack = TRUE;    //TRUE - 接收并组包完成数据帧关闭上层回调
BOOL32  g_bSelfSnd = FALSE;            //TRUE - 发送自测数据包，关闭外部发送请求

API void rsopen(BOOL32 bRcvCallback, BOOL32 bSelfSnd)
{
    g_bInterRcvCallBack = bRcvCallback;
    g_bSelfSnd = bSelfSnd;
}

//使用已有对象发送自测包
//nSndObjIndex - 发送对象索引
//nFrameLen - 模拟的数据帧大小 < max
//nSndNum    - 该帧重复发送的次数
//nSpan        - 重发的时间间隔
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
    //打印平均码流
    MedianetLog(Api, "\n CKdvMediaSnd stest OK, bitrate:%dkbps, nFrameLen: %d, nSndNum: %d, nSpan: %d \n",
                    dwBitrate, nFrameLen, nSndNum, nSpan);

    MEDIANET_SEM_GIVE(g_hMediaSndSem);
}

//通过对象号，打印相应send对象信息
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
                MedianetPrintf("LocalActivePT:[%d] (0-表示无效)  \n", tAdvancedSndInfo.m_byLocalActivePT);

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

//通过对象号，打印相应recv对象信息
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

//对象基本状态信息及统计信息
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
                MedianetPrintf("LocalActivePT:[%d] (0-表示无效)  \n", tAdvancedSndInfo.m_byLocalActivePT);

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
//medianet帧率统计
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
//medianet帧率统计
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

//启动medianet中继转发
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

//停止medianet中继转发
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


//medianet增加某路发送
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

//medianet删除某路发送
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


//设置发送对象码率
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

/***********************************网络发送块********************************/
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

    //加入发送对象链表中记录对象指针
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
    //m_hSndSynSem 开始有信号
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

    //从发送对象链表中删除对象指针
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
    函数名        ：Create
    功能        ： 创建发送对象

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     u32 dwMaxFrameSize 最大帧大小
                     u32 dwNetBand   带宽
                     u8 ucFrameRate  帧率
                     u8 ucMediaType  媒体类型（MP4，H264等）
                     u32 dwSSRC    同步源（默认为0）

    返回值说明： 参见错误码定义
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
    函数名        ：SetNetSndParam
    功能        ： 设置网络发送参数

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     TNetSndParam tNetSndParam 网络发送参数结构体

    返回值说明： 参见错误码定义
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

//设置NAT 探测包 功能, dwInterval=0 时表示不发送。
u16 CKdvMediaSnd::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
	u16 wRet = ERROR_NET_RCV_MEMORY;

	if(NULL != m_pcNetSnd && NULL != m_hSndSynSem) 
	{ 
		OspSemTake((SEMHANDLE)m_hSndSynSem);
		if(NULL != m_pcNetSnd)
		{
			wRet = m_pcNetSnd->SetNatProbeProp(ptNatProbeProp);

			//设置ok 则立即发送一次
			if(wRet == MEDIANET_NO_ERROR && ptNatProbeProp->dwInterval)
			{
				wRet = m_pcNetSnd->DealNatProbePack();
			}
		}
		OspSemGive((SEMHANDLE)m_hSndSynSem);
	}

	return wRet;
}

//发送探测包接口
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
    函数名        ：RemoveNetSndLocalParam
    功能        ： 移除网络发送参数

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     无

    返回值说明： 参见错误码定义
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
    函数名        ：SetActivePT
    功能        ：设置 动态载荷的 Playload值
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：byLocalActivePT 本端发送的动态载荷PT值, 由对呼时约定

    返回值说明： 参见错误码定义
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
    函数名        ：SetEncryptKey
    功能        ：设置加密key字串及加密码流的动态载荷PT值
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pszKeyBuf      加密key字串缓冲指针 NULL-表示不加密
                    wKeySize       加密key字串缓冲长度
                    byEncryptPT    加密模式 Aes 或者 Des -- default:des

    返回值说明： 参见错误码定义
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


//设置 Snd rtcpinfo回调函数
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

//重置同步源SSRC
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

//重置发送端对于mpeg4或者H.264采用的重传处理的开关,关闭后，将不对已经发送的数据包进行缓存
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
    函数名        ：Send
    功能        ： 发送一帧数据

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     PFRAMEHDR pFrmHdr
                     BOOL32 bAvalid 数据帧是否有效（默认为TRUE）

    返回值说明： 参见错误码定义
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

//发送自测试
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
    函数名        ：DealRtcpTimer
    功能        ：rtcp定时rtcp包上报
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明： 参见错误码定义
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

/***************************网络接收块****************************************/

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

    //加入接收对象链表中记录对象指针
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
    //m_hRcvSynSem 开始有信号
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

    //从接收对象链表中删除对象指针
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
    函数名        ：Create
    功能        ： 创建接收对象

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     u32 dwMaxFrameSize 最大帧大小
                     PFRAMEPROC pFrameCallBackProc 帧回调函数
                     u32 dwContext 上下文
                     u32 dwSSRC 同步源（默认为0）

    返回值说明： 参见错误码定义
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
    函数名        ：Create
    功能        ： 创建接收对象

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     u32 dwMaxFrameSize 最大帧大小
                     PRTPCALLBACK pRtpCallBackProc 包回调函数
                     u32 dwContext 上下文
                     u32 dwSSRC 同步源（默认为0）

    返回值说明： 参见错误码定义
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
    函数名        ：SetNetRcvLocalParam
    功能        ： 设置本地接收网络参数

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     TLocalNetParam tLocalNetParam 本地网络参数
                     u32 dwFlag    标志数据来源，从网络接收等（MEDIANETRCV_FLAG_FROM_RECVFROM）
                     u32 dwRegFunc   注册回调函数（ 默认为0）
                     u32 dwUnregFunc   （默认为0）

    返回值说明： 参见错误码定义
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

//重传过nat时，设置本机的rtp接收端口对应的公网地址,目的为使重传时不用广播
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

//设置NAT 探测包 功能, dwInterval=0 时表示不发送。
u16 CKdvMediaRcv::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
    u16 wRet = ERROR_NET_RCV_MEMORY;

    if(NULL != m_pcNetRcv && NULL != m_hRcvSynSem)
    {
        OspSemTake((SEMHANDLE)m_hRcvSynSem);
        if(NULL != m_pcNetRcv)
        {
            wRet = m_pcNetRcv->SetNatProbeProp(ptNatProbeProp);

            //设置ok 则立即发送一次
            if(wRet == MEDIANET_NO_ERROR && ptNatProbeProp->dwInterval)
            {
                wRet = m_pcNetRcv->DealNatProbePack();
            }
        }
        OspSemGive((SEMHANDLE)m_hRcvSynSem);
    }

    return wRet;
}

//发送探测包接口
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
    函数名        ：RemoveNetRcvLocalParam
    功能        ： 移除本地接收网络参数

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     无

    返回值说明： 参见错误码定义
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
    函数名        : ResetRSFlag
    功能        ：重置接收端对于mpeg4采用的重传处理的开关,关闭后，将不发送重传请求
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
    返回值说明： 参见错误码定义
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

//解析上层输入的是sps 和 pps 信息。
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
    函数名        : SetActivePT
    功能        ：设置 动态载荷的 Playload值
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                  byRmtActivePT 接收到的动态载荷的Playload值, 由对呼时对方告知，
                                0-表示清空 远端动态载荷标记
                  byRealPT      该动态载荷实际代表的的Playload类型，等同于我们发送时的PT约定

    返回值说明： 参见错误码定义
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
    函数名        : InputRtpPack
    功能        ：输入rtp包，本地組帧
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
            pRtpBuf  rtp包指针，
            dwRtpBuf  rtp包大小

    返回值说明： 参见错误码定义
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

//设置采样率函数
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
    函数名        ：SetDecryptKey
    功能        ：设置接收解密key字串
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                    pszKeyBuf      解密key字串缓冲指针 NULL-表示不解密
                    wKeySize       解密key字串缓冲长度
                    byDecryptMode  解密模式 Aes 或者 Des -- default:des

    返回值说明： 参见错误码定义
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

//重置接收端对于 (mp3) 是否采用乱序调整处理的开关, 关闭后，将不调整
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

//添加ps帧回调接口，并设置是否回调去ps头的帧 标志位。
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

//设置是否接收4k 码流
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

//重置RTP回调接口信息
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

//设置 Rcv rtcpinfo回调函数
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
    函数名        ：StartRcv
    功能        ： 开始接收

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     无

    返回值说明： 参见错误码定义
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
    函数名        ： StopRcv
    功能        ： 停止接收

    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                     无

    返回值说明： 参见错误码定义
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


//获取接收对象的最大帧设置
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

//获取接收对象 当前接收到的码流类型
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
    函数名        ：DealRtcpTimer
    功能        ：rtcp定时rtcp包上报
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：

    返回值说明： 参见错误码定义
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
