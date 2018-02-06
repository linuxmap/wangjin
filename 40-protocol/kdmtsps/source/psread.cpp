/*=================================================================================
模块名:PS流读写
文件名:ps.cpp
相关文件:ps.h
实现功能:PS流读写
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#include "common.h"
#include "kdmtsps.h"
#include "streamdef.h"
#include "videopredec.h"

/*=================================================================================
函数名:PSProgramOpen
功能:打开PS句柄,分配内存空间
算法实现:
参数说明:
         [I/O] ptpPSInfo PS流结构
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
TPsRead *PsReadOpen(TspsFrameCallback pfCallback, void *pvContext, u32 dwMaxFrameSize)
{
    TPsRead *ptPSInfo = NULL;
    BOOL32 bFail = FALSE;

    //分配相应空间
    do 
    {
        ptPSInfo = (TPsRead *)malloc(sizeof(TPsRead));
        if (NULL == ptPSInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo, 0, sizeof(TPsRead));
		ptPSInfo->u8LastStreamID = 255;

	//get max frame size
	if(dwMaxFrameSize < MAX_VIDEO_FRAME_LEN)
		dwMaxFrameSize = MAX_VIDEO_FRAME_LEN;

	//record 
	ptPSInfo->dwmaxframesize = dwMaxFrameSize;
        ptPSInfo->pu8FrameBuf = (u8 *)malloc(ptPSInfo->dwmaxframesize);
        if (NULL == ptPSInfo->pu8FrameBuf)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->pu8FrameBuf, 0, ptPSInfo->dwmaxframesize);

        //存放没读完的pes缓冲，一包pes最大长度为65535+6（包括包头）
        ptPSInfo->pu8InBuf = (u8 *)malloc(ptPSInfo->dwmaxframesize);
        if (NULL == ptPSInfo->pu8InBuf)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->pu8InBuf, 0, ptPSInfo->dwmaxframesize);

        ptPSInfo->ptPesInfo = (TPesInfo *)malloc(sizeof(TPesInfo));
        if (NULL == ptPSInfo->ptPesInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->ptPesInfo, 0, sizeof(TPesInfo));
        
        ptPSInfo->ptFrame = (TspsFRAMEHDR *)malloc(sizeof(TspsFRAMEHDR));
        if (NULL == ptPSInfo->ptFrame)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->ptFrame, 0, sizeof(TspsFRAMEHDR));

        ptPSInfo->pfFrameCB = pfCallback;
        ptPSInfo->pvFrameContext = pvContext;
		ptPSInfo->bFirstPacket = TRUE;
    } while (0);
    
    if (bFail)
    {
        TspsPrintf(PD_PS_READ, "PsReadOpen fail: memory malloc error.");
        PsReadClose(ptPSInfo);
        ptPSInfo = NULL;
    }

    return ptPSInfo;
}

/*=================================================================================
函数名:PSProgramClose
功能:关闭PS句柄,释放内存空间
算法实现:
参数说明:
         [I/O] ptpPSInfo PS流结构
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 PsReadClose(TPsRead *ptPsInfo)
{
    if (NULL == ptPsInfo)
    {
        return ERROR_PS_READ_INPUT_PARAM;
    }

    TSPSFREE(ptPsInfo->ptFrame);
    TSPSFREE(ptPsInfo->ptPesInfo);
    TSPSFREE(ptPsInfo->pu8FrameBuf);
    TSPSFREE(ptPsInfo->pu8InBuf);

    TSPSFREE(ptPsInfo);

    return TSPS_OK;
}

//设置节目信息的回调
u16 PsReadSetProgramCallback(TPsRead *ptPsInfo, TspsProgramCallback pfCallback, void *pvContext)
{
    if (NULL == ptPsInfo)
    {
        return ERROR_PS_READ_INPUT_PARAM;
    }
    
    ptPsInfo->pfProgramCB = pfCallback;
    ptPsInfo->pvProgramContext = pvContext;
    
    return TSPS_OK;
}

//查找0x000001起始符
static s32 PsReadFindHead(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    u32 u32Pos = 0;
    
    while (u32Pos + 2 < u32Len)
    {
        if (pu8Buf[u32Pos] == 0 && pu8Buf[u32Pos+1] == 0 && pu8Buf[u32Pos+2] == 1)
        {
            return u32Pos;
        }

        u32Pos++;
    }

//     TspsPrintf(PD_PS_READ, "PsRead didnot find 0x000001 in length[%d].", u32Len);
    return -1;
}

//读ps头
static s32 TsReadParseHead(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    TPsHeadInfo *ptHead = &ptPsRead->tHead;
    u16 u16Tmp;

    if (u32Len < 14)
    {
//        TspsPrintf(PD_PS_READ, "PsRead fail: head length[%d] < 14.", u32Len);
		ptPsRead->bReadHead = TRUE;
        return -1;
    }
	ptPsRead->bReadHead = FALSE;
    BitsInit(&tBitsBuf, pu8Buf, u32Len);

    BitsSkip(&tBitsBuf, 32);
    //scr base
    u16Tmp = BitsRead8(&tBitsBuf, 2);
    if (1 != u16Tmp)
    {
        TspsPrintf(PD_PS_READ, "PsRead fail: head scr fix 1 bit wrong.");
        return -1;
    }
    u16Tmp = BitsRead8(&tBitsBuf, 3);
    ptHead->u64SCRBase = (u16Tmp << 30);
    BitsSkip(&tBitsBuf, 1);
    u16Tmp = BitsRead16(&tBitsBuf, 15);
    ptHead->u64SCRBase += (u16Tmp << 15);
    BitsSkip(&tBitsBuf, 1);
    u16Tmp = BitsRead16(&tBitsBuf, 15);
    ptHead->u64SCRBase += u16Tmp;
    //scr ext
    BitsSkip(&tBitsBuf, 1);
    u16Tmp = BitsRead16(&tBitsBuf, 9);
    ptHead->u16SCRExt = u16Tmp;
    //mux rate
    BitsSkip(&tBitsBuf, 1);
    ptHead->u32ProgramMuxRate = BitsRead32(&tBitsBuf, 22);
    BitsSkip(&tBitsBuf, 7);
    //padding
    u16Tmp = BitsRead8(&tBitsBuf, 3);
    if (14 + (u32)u16Tmp > u32Len)
    {
//         TspsPrintf(PD_PS_READ, "PsRead fail: padding bit length error.");
        return -1;
    }

    TspsPrintf(PD_PS_READ, "PsRead read a ps header.");

    return 14 + u16Tmp;
}

//读系统头
static s32 TsReadParseSysHead(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    TPsSysHeaderInfo *ptSysHead = &ptPsRead->tSysHead;

    BitsInit(&tBitsBuf, pu8Buf, u32Len);
    
    //读取填充长度
    BitsSkip(&tBitsBuf, 32);
    ptSysHead->u16HeaderLength = BitsRead16(&tBitsBuf, 16);
    if (6 + (u32)ptSysHead->u16HeaderLength > u32Len)
    {
//         TspsPrintf(PD_PS_READ, "PsRead fail: syshead length[%d] error.", 
//             4 + ptSysHead->u16HeaderLength);
		ptPsRead->bReadHead = TRUE;
        return -1;
    }
	ptPsRead->bReadHead = FALSE;
    TspsPrintf(PD_PS_READ, "PsRead read a system header.");

    return 6 + ptSysHead->u16HeaderLength;
}

//解析其他流
static s32 TsReadParseOtherStream(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    u16 u16HeadLen;

	if (ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID0 ||
		ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID1 ||
		ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
	{
		if (ptPsRead->u32FrameLen < ptPsRead->dwmaxframesize)
		{
			//拷入帧缓冲
			memcpy(ptPsRead->pu8FrameBuf + ptPsRead->u32FrameLen, 
				pu8Buf, 3);
			ptPsRead->u32FrameLen += 3;
		}
		
		return 3;
	}
    BitsInit(&tBitsBuf, pu8Buf, u32Len);

    //读取填充长度
    BitsSkip(&tBitsBuf, 32);
    u16HeadLen = BitsRead16(&tBitsBuf, 16);
    if (4 + (u32)u16HeadLen > u32Len)
    {
//         TspsPrintf(PD_PS_READ, "PsRead fail: other stream length[%d] error.", 
//             4 + u16HeadLen);
        return -1;
    }

    TspsPrintf(PD_PS_READ, "PsRead read an unknown pack<%02X> and skip.", pu8Buf[3]);

    return 4 + u16HeadLen;
}

//读取节目信息
static u16 PsReadGetProgramInfo(TPsRead *ptPsRead)
{
    TPsMapInfo *ptMap = &ptPsRead->tMap;
    u8 u8VideoType = PT_STREAM_TYPE_NULL;
    u8 u8AudioType0 = PT_STREAM_TYPE_NULL;
	u8 u8AudioType1 = PT_STREAM_TYPE_NULL;
    u8 u8OldAid0 = ptPsRead->u8AudioID0;
	u8 u8OldAid1 = ptPsRead->u8AudioID1;
    u8 u8OldVid = ptPsRead->u8VideoID;

	for (u8 u8StreamNum = 0; u8StreamNum < ptMap->u8StreamNum; u8StreamNum++)
	{
		if (AUDIO_STREAM0 == ptMap->au8StreamId[u8StreamNum])
		{
			u8AudioType0 =  ptMap->au8StreamType[u8StreamNum];;
			ptPsRead->u8AudioID0 = ptMap->au8StreamId[u8StreamNum];
		}
		else if (AUDIO_STREAM1 == ptMap->au8StreamId[u8StreamNum])
		{
			u8AudioType1 =  ptMap->au8StreamType[u8StreamNum];;
			ptPsRead->u8AudioID1 = ptMap->au8StreamId[u8StreamNum];
		}
		else if (VIDEO_STREAM == ptMap->au8StreamId[u8StreamNum])
		{
			u8VideoType = ptMap->au8StreamType[u8StreamNum];
            ptPsRead->u8VideoID = ptMap->au8StreamId[u8StreamNum];
		}
	}

    ptPsRead->u8AudioType0 = u8AudioType0;
	ptPsRead->u8AudioType1 = u8AudioType1;
    ptPsRead->u8VideoType = u8VideoType;
    
    //节目流id改变了才回调
    if (ptPsRead->pfProgramCB &&
        (ptPsRead->u8AudioID0 != u8OldAid0 || ptPsRead->u8AudioID1 != u8OldAid1 || ptPsRead->u8VideoID != u8OldVid))
    {
        ptPsRead->pfProgramCB(TsPsPTCovertStream2Rtp(u8VideoType), 
                              TsPsPTCovertStream2Rtp(u8AudioType0), 
							  TsPsPTCovertStream2Rtp(u8AudioType1), 
                              (KD_PTR)ptPsRead->pvProgramContext);
    }
    
    return TSPS_OK;
}

//解析psm
static s32 TsReadParsePsm(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    TPsMapInfo *ptMap = &ptPsRead->tMap;
    s32 s32i;
    
    BitsInit(&tBitsBuf, pu8Buf, u32Len);
    
    //读取填充长度
    BitsSkip(&tBitsBuf, 32);
    ptMap->u16MapLength = BitsRead16(&tBitsBuf, 16);
    if (u32Len < (u32)ptMap->u16MapLength + 6)
    {
		ptPsRead->bReadHead = TRUE;
        return -1;
    }
	ptPsRead->bReadHead = FALSE;
#if 1
    ptMap->u8CurrentNextIndicator = BitsRead8(&tBitsBuf, 1);
    BitsSkip(&tBitsBuf, 2);
    ptMap->u8Version = BitsRead8(&tBitsBuf, 5);
    BitsSkip(&tBitsBuf, 8);
    ptMap->u16PsInfoLength = BitsRead16(&tBitsBuf, 16);
    for (s32i=0; s32i<ptMap->u16PsInfoLength; s32i++)
    {
        BitsSkip(&tBitsBuf, 8);
    }

    ptMap->u16EsMapLength = BitsRead16(&tBitsBuf, 16);
    if (10 + (u32)ptMap->u16PsInfoLength + 2 + (u32)ptMap->u16EsMapLength > u32Len)
    {
        return -1;
    }


	if (ptMap->u16EsMapLength > 1024)
	{
		TspsPrintf(PD_PS_READ, "EsMapLength is larger than 1024, error!\n");
	}
	if (ptMap->u16EsMapLength / 4 < MAX_STREAM_NUM_IN_PROGRAM)
	{
		ptMap->u8StreamNum = ptMap->u16EsMapLength / 4;
		for (s32i=0; s32i<ptMap->u8StreamNum; s32i++)
		{
			s32 s32k;
			
			if (s32i >= MAX_STREAM_NUM_IN_PROGRAM)
			{
				break;
			}
			
			ptMap->au8StreamType[s32i]    = BitsRead8(&tBitsBuf, 8);
			ptMap->au8StreamId[s32i]      = BitsRead8(&tBitsBuf, 8);
			ptMap->au16EsInfoLength[s32i] = BitsRead16(&tBitsBuf, 16);
			for (s32k=0; s32k<ptMap->au16EsInfoLength[s32i]; s32k++)
			{
				BitsSkip(&tBitsBuf, 8);
			}
		}
	}
	else
	{
		u16 u16EsMapLength = ptMap->u16EsMapLength;
		s32i = 0;
		while (u16EsMapLength > 0)
		{
			ptMap->au8StreamType[s32i]    = BitsRead8(&tBitsBuf, 8);
			ptMap->au8StreamId[s32i]      = BitsRead8(&tBitsBuf, 8);
			ptMap->au16EsInfoLength[s32i] = BitsRead16(&tBitsBuf, 16);		
			u16EsMapLength -= 4;
			for (s32 s32k=0; s32k<ptMap->au16EsInfoLength[s32i]; s32k++)
			{
				BitsSkip(&tBitsBuf, 8);
				u16EsMapLength -= 1;
			}
			s32i++;
			if (s32i >= MAX_STREAM_NUM_IN_PROGRAM)
			{
				break;
			}
		}
		ptMap->u8StreamNum = s32i;
	}
	
    PsReadGetProgramInfo(ptPsRead);
#endif

    TspsPrintf(PD_PS_READ, "PsRead read a ps map.");

    return ptMap->u16MapLength + 6;
}

//获得一包es
static u16 PsReadCallback(TPsRead *ptPsRead, u8 u8StreamType, u8 u8StreamID)
{
    TspsFRAMEHDR *ptFrame = ptPsRead->ptFrame;
	s32 nRet = 0;

    ptFrame->m_byMediaType = TsPsPTCovertStream2Rtp(u8StreamType);
	if (MEDIA_TYPE_NULL == ptFrame->m_byMediaType)
	{
		ptPsRead->u32FrameLen = 0; //防止未收到关键帧，ptpsread帧buf堆积。
		return ERROR_PES_READ_TYPE_NOT_SUPPORT;
	}

	if (AUDIO_STREAM1 == u8StreamID)
	{
		ptFrame->m_byStreamID = 1;
	}
	else
	{
		ptFrame->m_byStreamID = 0;
	}
	
    ptFrame->m_pbyData = ptPsRead->pu8FrameBuf;
    ptFrame->m_dwDataSize = ptPsRead->u32FrameLen;
    ptFrame->m_dwTimeStamp = ptPsRead->u32LastTS;

	if (VIDEO_STREAM == u8StreamID)
	{
		ptPsRead->dwVideoFrameID++;
		ptFrame->m_dwFrameID = ptPsRead->dwVideoFrameID;
	}
	else if (AUDIO_STREAM0 == u8StreamID)
	{
		ptPsRead->dwAudioFrameID0++;
		ptFrame->m_dwFrameID = ptPsRead->dwAudioFrameID0;
	}
	else if(AUDIO_STREAM1 == u8StreamID)
	{
		ptPsRead->dwAudioFrameID1++;
		ptFrame->m_dwFrameID = ptPsRead->dwAudioFrameID1;
	}
	
    TspsPrintf(PD_PS_READ, "PsRead get a frame successfully. <frameid=%d> <len=%d> <type=%d> <TimeStamp=%d>",
        ptFrame->m_dwFrameID, ptFrame->m_dwDataSize, ptFrame->m_byMediaType, ptFrame->m_dwTimeStamp);

    //dts 无效则不回调.等到有效的pes头才回调。
    if(!(ptPsRead->ptPesInfo->bDtsValid))
    {
        TspsPrintf(PD_PS_READ,"[PsReadCallback] ptPsRead->ptPesInfo->bDtsValid==FALSE ,notCallBack \n");
        return TSPS_OK;
    }

    if (ptPsRead->pfFrameCB && ptFrame->m_dwDataSize > 0)
    {
		TVideoInfo tVideoInfo;
		memset(&tVideoInfo, 0, sizeof(TVideoInfo));
		if (MEDIA_TYPE_H264 == ptFrame->m_byMediaType)
		{
			s32 nRet = GetH264Info(ptFrame->m_pbyData, ptFrame->m_dwDataSize, &tVideoInfo);
			if (-1 == nRet)
			{
				TspsPrintf(PD_PS_READ, "GetH264Info error!\n");
				TspsPrintf(PD_PS_READ, "get a frame. <frameid=%d> <len=%d> <type=%d> <TimeStamp=%d>",
				ptFrame->m_dwFrameID, ptFrame->m_dwDataSize, ptFrame->m_byMediaType, ptFrame->m_dwTimeStamp);

				TspsPrintf(PD_PS_READ, "streamNum:%d 0,stream id:%d, type:%d, 0 streamid:%d, type:%d\n",
					ptPsRead->tMap.u8StreamNum, ptPsRead->tMap.au8StreamId[0], ptPsRead->tMap.au8StreamType[0], 
			ptPsRead->tMap.au8StreamId[1], ptPsRead->tMap.au8StreamType[1]);
			}
		}
		else if (MEDIA_TYPE_MP4 == ptFrame->m_byMediaType)
		{
			GetMP4Info(ptFrame->m_pbyData, ptFrame->m_dwDataSize, &tVideoInfo);
		}
		else if (MEDIA_TYPE_SVACV == ptFrame->m_byMediaType)
		{
			nRet = GetSVACInfo(ptFrame->m_pbyData, ptFrame->m_dwDataSize, &tVideoInfo);
			if (-1 == nRet)
			{
				TspsPrintf(PD_PS_READ, "GetSvacInfo error!\n");
				TspsPrintf(PD_PS_READ, "get a frame. <frameid=%d> <len=%d> <type=%d> <TimeStamp=%d>",
					ptFrame->m_dwFrameID, ptFrame->m_dwDataSize, ptFrame->m_byMediaType, ptFrame->m_dwTimeStamp);
				
				TspsPrintf(PD_PS_READ, "streamNum:%d 0,stream id:%d, type:%d, 0 streamid:%d, type:%d\n",
					ptPsRead->tMap.u8StreamNum, ptPsRead->tMap.au8StreamId[0], ptPsRead->tMap.au8StreamType[0], 
					ptPsRead->tMap.au8StreamId[1], ptPsRead->tMap.au8StreamType[1]);
			}
		}
		else if (MEDIA_TYPE_H265 == ptFrame->m_byMediaType)
		{
			//OspPrintf(1,0,"frame size = %d \n", ptFrame->m_dwDataSize);
			nRet = GetH265Info(ptFrame->m_pbyData, ptFrame->m_dwDataSize, &tVideoInfo);
			if (-1 == nRet)
			{
				TspsPrintf(PD_PS_READ, "GetH265Info error!\n");
				TspsPrintf(PD_PS_READ, "get a frame. <frameid=%d> <len=%d> <type=%d> <TimeStamp=%d>",
					ptFrame->m_dwFrameID, ptFrame->m_dwDataSize, ptFrame->m_byMediaType, ptFrame->m_dwTimeStamp);
				
				TspsPrintf(PD_PS_READ, "streamNum:%d 0,stream id:%d, type:%d, 0 streamid:%d, type:%d\n",
					ptPsRead->tMap.u8StreamNum, ptPsRead->tMap.au8StreamId[0], ptPsRead->tMap.au8StreamType[0], 
					ptPsRead->tMap.au8StreamId[1], ptPsRead->tMap.au8StreamType[1]);
			}
		}
		
		if ( 0 != tVideoInfo.l32Width)
		{
			ptPsRead->u32Width = tVideoInfo.l32Width;
		}

		if (0 != tVideoInfo.l32Height)
		{
			ptPsRead->u32Height = tVideoInfo.l32Height;
		}
		ptFrame->x.m_tVideoParam.m_bKeyFrame = tVideoInfo.l32IsIFrame;
		ptFrame->x.m_tVideoParam.m_wVideoWidth = ptPsRead->u32Width;
		ptFrame->x.m_tVideoParam.m_wVideoHeight = ptPsRead->u32Height;

        ptPsRead->pfFrameCB(ptFrame, (KD_PTR)ptPsRead->pvFrameContext);
    }

    //缓冲清零
    ptPsRead->u32FrameLen = 0;

    //帧id持续相加
    ptFrame->m_dwFrameID++;
    
    return TSPS_OK;
}

//解析pes
static s32 TsReadParsePes(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    u16 u16PesLen = 0;
    TPesInfo *ptPesInfo = ptPsRead->ptPesInfo;
    u16 u16Ret;
	u32 u32LenOutput;
	u8 u8StreamID = 255;
    BitsInit(&tBitsBuf, pu8Buf, u32Len);

    BitsSkip(&tBitsBuf, 32);
    u16PesLen = BitsRead16(&tBitsBuf, 16);

    if (/*u16PesLen == 0 ||*/ (u32)u16PesLen + 6 > u32Len)
    {
		ptPsRead->bReadHead = TRUE;
        return -1;
    }
	ptPsRead->bReadHead = FALSE;
	if (0 == u16PesLen)
	{
		u16Ret = PesReadInfo(ptPesInfo, pu8Buf, u32Len, &u32LenOutput);
	}
	else
	{
		u16Ret = PesReadInfo(ptPesInfo, pu8Buf, u16PesLen + 6, &u32LenOutput);
	}
    
    if (TSPS_OK == u16Ret)
    {
        u8 u8StreamType = PT_STREAM_TYPE_NULL;

        if (ptPesInfo->u8StreamId == ptPsRead->u8AudioID0)
        {
            u8StreamType = ptPsRead->u8AudioType0;
			u8StreamID = ptPsRead->u8AudioID0;
        }
		else if (ptPesInfo->u8StreamId == ptPsRead->u8AudioID1)
		{
			u8StreamType = ptPsRead->u8AudioType1;
			u8StreamID = ptPsRead->u8AudioID1;
		}
        else if (ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
        {
            u8StreamType = ptPsRead->u8VideoType;
			u8StreamID = ptPsRead->u8VideoID;
        }
		
        TspsPrintf(PD_PS_READ, "PsRead parsed a pes<%02X>[%d]. pes_packet_length[%d], es_length[%d], dts[%d]", 
            ptPesInfo->u8StreamId, u8StreamType, u16PesLen, ptPesInfo->u32PayloadLength, (u32)ptPesInfo->u64Dts);

        //时间戳或者流类型不同，回调上一帧
        if ((u32)ptPesInfo->u64Dts != ptPsRead->u32LastTS ||
            u8StreamType != ptPsRead->u8LastType ||
			u8StreamID != ptPsRead->u8LastStreamID)
        {
            PsReadCallback(ptPsRead, ptPsRead->u8LastType, ptPsRead->u8LastStreamID);

            //写入dts，标记该帧Dts有效
            if(ptPesInfo->bWriteDts)
            {
                ptPesInfo->bDtsValid = TRUE;
            }
                
            ptPsRead->u32LastTS = (u32)ptPesInfo->u64Dts;
            ptPsRead->u8LastType = u8StreamType;
			ptPsRead->u8LastStreamID = u8StreamID;
        }
		
        if (ptPesInfo->u8StreamId == ptPsRead->u8AudioID0 ||
			ptPesInfo->u8StreamId == ptPsRead->u8AudioID1 ||
            ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
        {
			if (0 != u16PesLen)
			{
				if (ptPsRead->u32FrameLen + ptPesInfo->u32PayloadLength < ptPsRead->dwmaxframesize)
				{
					//拷入帧缓冲
					memcpy(ptPsRead->pu8FrameBuf + ptPsRead->u32FrameLen, 
						ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
					ptPsRead->u32FrameLen += ptPesInfo->u32PayloadLength; 
				}
				else
				{
					ptPesInfo->u32PayloadLength = 0;
					ptPsRead->u32FrameLen = 0;
				}
			}			       
        }
    }
    else
    {
        TspsPrintf(PD_PS_READ, "PsRead parse pes error.");

		// 出错时,跳过改PES分组,直接返回改分组长度
		return (u16PesLen + 6);
    }

	if (0 == u16PesLen)
	{
		return u32LenOutput + 6;
	}
	else
	{
		return u16PesLen + 6;
	}    
}

//割开每个ps包
static s32 PsReadExplodePacks(TPsRead *ptPsRead, u8 *pu8Buf, u32 u32Len)
{
    u32 u32Remain = u32Len;
    s32 s32StartPos = 0;
    u8 *pu8PackBuf = NULL;
    s32 s32PackLen = -1;
    u8 u8TypeByte;
    
    while (u32Remain > 0)
    {
        //找到下一个0x000001的位置
        s32StartPos = PsReadFindHead(ptPsRead, pu8Buf, u32Remain);
        if (s32StartPos < 0)//没找到
        {					
            if (0x00 == pu8Buf[u32Remain - 1])
            {
                //如果最后一个字节为0x00，那么有可能是后面字节流的起始符号
                //保留最后2个字节
                if (u32Remain >= 2)
                {
					if (!ptPsRead->bReadHead)
					{						
						if (ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID0 ||
							ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID1 ||
							ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
						{
							if (ptPsRead->u32FrameLen + u32Remain-2 < ptPsRead->dwmaxframesize)
							{
								//拷入帧缓冲
								memcpy(ptPsRead->pu8FrameBuf + ptPsRead->u32FrameLen, 
									pu8Buf, u32Remain-2);
								ptPsRead->u32FrameLen += u32Remain-2;						
							}	
						}
					}
					u32Remain = 2;
                }
                else
                {         
					//u32Remain = 1	，需要保留一个字节，因此不拷贝				
					u32Remain = 1;
                }
            }
            else
            {
				//没有找到000001并且没有读到头的情况下，认为是码流帧中的一部分
				if (!ptPsRead->bReadHead)
				{
					if (ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID0 ||
						ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID1 ||
						ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
					{
						//拷入帧缓冲
						if (ptPsRead->u32FrameLen + u32Remain < ptPsRead->dwmaxframesize)
						{
							memcpy(ptPsRead->pu8FrameBuf + ptPsRead->u32FrameLen, pu8Buf, u32Remain);
						}
						else
						{	
							s8 szLog[256] = {0};
							sprintf(szLog, "Warning!!! now Framelen:%d + u32Remain:%d > ptPsRead->dwmaxframesize: %d\n", ptPsRead->u32FrameLen, u32Remain, ptPsRead->dwmaxframesize);
							tspswritelog(szLog, strlen(szLog));
							OspPrintf(TRUE, FALSE, "Warning!!! now Framelen:%d + u32Remain:%d > ptPsRead->dwmaxframesize: %d\n", ptPsRead->u32FrameLen, u32Remain, ptPsRead->dwmaxframesize);
						}
						
						ptPsRead->u32FrameLen += u32Remain;
					}
				}
                u32Remain = 0;				
            }
            break;
        }
		else
		{
//			OspPrintf(TRUE, FALSE, "s32StartPos:%d, u32Remain:%d  %x %x %x %x\n", 
//				s32StartPos, u32Remain, pu8Buf[s32StartPos], pu8Buf[s32StartPos+1], pu8Buf[s32StartPos+2], pu8Buf[s32StartPos+3]);
			if (!ptPsRead->bReadHead && s32StartPos > 0)
			{
				if (ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID0 ||
					ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8AudioID1 ||
					ptPsRead->ptPesInfo->u8StreamId == ptPsRead->u8VideoID)
				{
					if (ptPsRead->u32FrameLen + s32StartPos < ptPsRead->dwmaxframesize)
					{
						//拷入帧缓冲
						memcpy(ptPsRead->pu8FrameBuf + ptPsRead->u32FrameLen, 
							pu8Buf, s32StartPos);
						ptPsRead->u32FrameLen += s32StartPos;
					}							
				}
			}			
		}

        pu8Buf += s32StartPos;
		if (s32StartPos > u32Len)
		{
			s8 szLog[256] = {0};
			sprintf(szLog, "Warning!!!s32StartPos > u32Len! s32StartPos:%d u32Remain:%d\n", s32StartPos, u32Remain);
			tspswritelog(szLog, strlen(szLog));
		}
		if (u32Remain < s32StartPos)
		{
			s8 szLog[256] = {0};
			sprintf(szLog, "Warning!!!u32Remain < s32StartPos! s32StartPos:%d u32Remain:%d\n", s32StartPos, u32Remain);
			tspswritelog(szLog, strlen(szLog));
		}
        u32Remain -= s32StartPos;
		if (u32Remain > 100000)
		{
			s8 szLog[256] = {0};
			sprintf(szLog, "Warning!!!u32Remain > 100000! u32Remain:%d\n", u32Remain);
			tspswritelog(szLog, strlen(szLog));
		}
        //解析一包
        u8TypeByte = pu8Buf[3];

        if (VIDEO_STREAM == (u8TypeByte & 0xF0))
        {
            u8TypeByte = VIDEO_STREAM;
        }

		if (AUDIO_STREAM1 == (u8TypeByte & 0xE1))
        {
            u8TypeByte = AUDIO_STREAM1;
        }
		else if (AUDIO_STREAM0 == (u8TypeByte & 0xE0))
        {
            u8TypeByte = AUDIO_STREAM0;
        }

        switch (u8TypeByte)
        {
        case PS_HEAD_BYTE:
            s32PackLen = TsReadParseHead(ptPsRead, pu8Buf, u32Remain);
            break;

        case PS_SYSTEM_HEAD_BYTE:
            s32PackLen = TsReadParseSysHead(ptPsRead, pu8Buf, u32Remain);
            break;

        case PS_MAP_BYTE:
            s32PackLen = TsReadParsePsm(ptPsRead, pu8Buf, u32Remain);
            break;

        case PS_END_BYTE:
            //遇到结束把缓冲内数据回调出去
            TspsPrintf(PD_PS_READ, "PsRead read a ps endian.");
            PsReadCallback(ptPsRead, ptPsRead->u8LastType, 0);
            s32PackLen = 4;
            break;

        case VIDEO_STREAM:
        case AUDIO_STREAM0:
		case AUDIO_STREAM1:
            s32PackLen = TsReadParsePes(ptPsRead, pu8Buf, u32Remain);
            break;
            
        default:
            s32PackLen = TsReadParseOtherStream(ptPsRead, pu8Buf, u32Remain);
            break;
        }

        //出错返回已读的字节数
        if (s32PackLen < 0)
        {
            break;
        }

        //位置偏移前进
        pu8Buf += s32PackLen;
	
        u32Remain -= s32PackLen;
    }


    return u32Len - u32Remain;
}

//解析码流
u16 PsReadInputStream(TPsRead *ptPsInfo, u8 *pu8Buf, u32 u32Len)
{
    s32 s32ReadLen = 0;
    s32 s32Start = 0;
    u16 u16Ret = TSPS_OK;
    u8 *pu8BufPos;
    u32 u32BufLen;

    if (NULL == ptPsInfo)
    {
        return ERROR_PS_READ_INPUT_PARAM;
    }

    if (NULL == pu8Buf || 0 == u32Len)
    {
        return ERROR_PS_READ_INPUT_PARAM;
    }
    
    //先前有数据没读完，那么将现有数据拷入
    if (ptPsInfo->u32InLen > 0)
    {
        if (ptPsInfo->u32InLen + u32Len > ptPsInfo->dwmaxframesize)
        {
			//清空原来的数据
			ptPsInfo->u32InLen = 0;
			return ERROR_PS_READ_BUFF_FULL;
        }
		
        memcpy(ptPsInfo->pu8InBuf + ptPsInfo->u32InLen, pu8Buf, u32Len);
        ptPsInfo->u32InLen += u32Len;

        pu8BufPos = ptPsInfo->pu8InBuf;
        u32BufLen = ptPsInfo->u32InLen;
    }
    //先前数据已读完，则直接处理现有数据
    else
    {
		if (TRUE == ptPsInfo->bFirstPacket)
		{
			//去掉海康头
			u8* pByData = pu8Buf;
			if(((pByData[0]==0x49 && pByData[1] == 0x4D && pByData[2] == 0x4B && pByData[3] == 0x48)
				|| (pByData[0]==0x34 && pByData[1] == 0x48 && pByData[2] == 0x4B && pByData[3] == 0x48)) && u32Len > 40)
			{
				pu8BufPos = pu8Buf + 40;
				u32BufLen = u32Len - 40;
			}
			else
			{
				pu8BufPos = pu8Buf;
				u32BufLen = u32Len;
			}
			ptPsInfo->bFirstPacket = FALSE;
		}
		else
		{
			pu8BufPos = pu8Buf;
			u32BufLen = u32Len;
		}
    }
    
    s32ReadLen = PsReadExplodePacks(ptPsInfo, pu8BufPos, u32BufLen);
    if (s32ReadLen < 0)
    {
        u16Ret = ERROR_PS_READ_PARSE_FAIL;
    }
    else if ((u32)s32ReadLen <= u32BufLen)
    {
        u32 u32Left = u32BufLen - s32ReadLen;

        //读不完，暂时拷贝进缓冲
        if (u32Left > ptPsInfo->dwmaxframesize)
        {
            u16Ret = ERROR_PS_READ_BUFF_FULL;
        }
        else if (u32Left > 0)
        {
            u8 *pu8Left = pu8BufPos + s32ReadLen;
            memcpy(ptPsInfo->pu8InBuf, pu8Left, u32Left);
            ptPsInfo->u32InLen = u32Left;
        }
		else 
		{
			//数据分析完成
			ptPsInfo->u32InLen = 0;
		}
    }

    return u16Ret;
}

u16 PsReadResetStream(TPsRead *ptPsInfo)
{
	if (NULL == ptPsInfo)
    {
        return ERROR_PS_READ_INPUT_PARAM;
    }

	ptPsInfo->u32InLen = 0;
	ptPsInfo->u32FrameLen = 0;
	memset(ptPsInfo->pu8InBuf, 0, ptPsInfo->dwmaxframesize);
	memset(ptPsInfo->pu8FrameBuf, 0, ptPsInfo->dwmaxframesize);
	ptPsInfo->u32FrameLen = 0;
	ptPsInfo->u32InLen = 0;
	ptPsInfo->dwAudioFrameID0++;
	ptPsInfo->dwVideoFrameID++;
	ptPsInfo->bReadHead = FALSE;
	ptPsInfo->bFirstPacket = TRUE;
	ptPsInfo->u32LastTS = 0;
	ptPsInfo->u8LastType = 0;
	ptPsInfo->u8LastStreamID = 255;
    ptPsInfo->u32FrameLen = 0; //size set 0, not call back
	memset(&ptPsInfo->tHead, 0, sizeof(TPsHeadInfo));
	memset(&ptPsInfo->tSysHead, 0, sizeof(TPsSysHeaderInfo));
	memset(&ptPsInfo->tMap, 0, sizeof(TPsMapInfo));

    //包括dts标记位和wirtedts标记位也标记为空
	memset(ptPsInfo->ptPesInfo, 0, sizeof(TPesInfo));
	memset(ptPsInfo->ptFrame, 0, sizeof(TspsFRAMEHDR));
	return TSPS_OK;
}