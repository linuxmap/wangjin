/*=================================================================================
模块名:TS流的读
文件名:ts.cpp
相关文件:ts.h
实现功能:TS流的读
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

TTsRead *TsReadOpen(TspsFrameCallback pfCallback, void *pvContext)
{
    s32 s32i = 0;
    
    TTsRead *ptTsInfo = NULL;
    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;
    BOOL32 bFail = FALSE;
	u8 u8Loop = 0;
    
    //分配相应内存 
    do 
    {
        ptTsInfo = (TTsRead *)malloc(sizeof(TTsRead));
        if(NULL == ptTsInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo, 0, sizeof(TTsRead));
        
        ptPAT = &ptTsInfo->tPatInfo;
        ptPAT->pu8Buffer = (u8 *)malloc(TS_PACKET_LENGTH);
        if(NULL == ptPAT->pu8Buffer)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPAT->pu8Buffer, 0, TS_PACKET_LENGTH);
        
        ptTsInfo->ptPmtInfo = (TTsPmtInfo *)malloc(sizeof(TTsPmtInfo) * MAX_PROGRAM_MAP_NUM);
        if(NULL == ptTsInfo->ptPmtInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo->ptPmtInfo, 0, sizeof(TTsPmtInfo) * MAX_PROGRAM_MAP_NUM);
        
        ptPMT = ptTsInfo->ptPmtInfo;
        for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM; s32i++)
        {
            ptPMT[s32i].pu8Buffer = (u8 *)malloc(TS_PACKET_LENGTH);
            if(NULL == ptPMT[s32i].pu8Buffer)
            {
                bFail = TRUE;
                break;
            }
            memset(ptPMT[s32i].pu8Buffer, 0, TS_PACKET_LENGTH);
        }
        
        ptTsInfo->ptFrame = (TspsFRAMEHDR *)malloc(sizeof(TspsFRAMEHDR));
        if(NULL == ptTsInfo->ptFrame)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo->ptFrame, 0, sizeof(TspsFRAMEHDR));
		
		for (u8Loop = 0; u8Loop<MAX_PROGRAM_MAP_NUM; u8Loop++)
		{
			ptTsInfo->atTsInfo[u8Loop].ptPesInfo = (TPesInfo *)malloc(sizeof(TPesInfo));
			if (NULL == ptTsInfo->atTsInfo[u8Loop].ptPesInfo)
			{
				bFail = TRUE;
				break;
			}
			memset(ptTsInfo->atTsInfo[u8Loop].ptPesInfo, 0, sizeof(TPesInfo));

			ptTsInfo->atTsInfo[u8Loop].pu8PesBuf = (u8 *)malloc(MAX_VIDEO_FRAME_LEN);
			if (NULL == ptTsInfo->atTsInfo[u8Loop].pu8PesBuf)
			{
				bFail = TRUE;
				break;
			}
			memset(ptTsInfo->atTsInfo[u8Loop].pu8PesBuf, 0, MAX_VIDEO_FRAME_LEN);
			ptTsInfo->atTsInfo[u8Loop].u8Type = 255;
		}

		if (TRUE == bFail)
		{
			break;
		}
        
        ptTsInfo->pfFrameCB = pfCallback;
        ptTsInfo->pvFrameContext = pvContext;
		ptTsInfo->dwTempBufLen = 0;
		ptTsInfo->bFirstPack = TRUE;
        
    } while (0);
    
    if (bFail)
    {
        TspsPrintf(PD_TS_READ, "TsReadOpen fail: memory malloc error.");
        TsReadClose(ptTsInfo);
        return NULL;
    }

    return ptTsInfo;
}


u16 TsReadClose(TTsRead *ptTsInfo)
{
    s32 s32i = 0;
    
    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;
	u8 u8Loop = 0;
    if (NULL == ptTsInfo)
    {
        return ERROR_TS_READ_INPUT_PARAM;
    }
    
    ptPAT = &ptTsInfo->tPatInfo;
    
    TSPSFREE(ptPAT->pu8Buffer);
    
    ptPMT = ptTsInfo->ptPmtInfo;
    for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM; s32i++)
    {
        TSPSFREE(ptPMT[s32i].pu8Buffer);
    }
    
    TSPSFREE(ptTsInfo->ptPmtInfo);
    
    TSPSFREE(ptTsInfo->ptFrame);
	for (u8Loop = 0; u8Loop < MAX_PROGRAM_MAP_NUM; u8Loop++)
	{
		TSPSFREE(ptTsInfo->atTsInfo[u8Loop].pu8PesBuf);
		TSPSFREE(ptTsInfo->atTsInfo[u8Loop].ptPesInfo);
	}
    
    TSPSFREE(ptTsInfo);
    
    return TSPS_OK;
}

u16 TsReadSetProgramCallback(TTsRead *ptTsInfo, TspsProgramCallback pfCallback, void *pvContext)
{
    if (NULL == ptTsInfo)
    {
        return ERROR_TS_READ_INPUT_PARAM;
    }

    ptTsInfo->pfProgramCB = pfCallback;
    ptTsInfo->pvProgramContext = pvContext;

    return TSPS_OK;
}

//读ts头
static u16 TsReadGetHeader(TTsHeader *ptHead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;

    BitsInit(&tBitsBuf, pu8Buf, u32Len);

    //跳过ts开始符号(0x47)(8)
    BitsSkip(&tBitsBuf, 8);
    
    //读传输错误标记
    ptHead->u8TransportErrorIndicator = BitsRead8(&tBitsBuf, 1);
    //读unit start indicator
    ptHead->u8PayloadUnitStartIndicator = BitsRead8(&tBitsBuf, 1);
    //读优先级
    ptHead->u8TransportPriority = BitsRead8(&tBitsBuf, 1);
    //pid
    ptHead->u16Pid = BitsRead16(&tBitsBuf, 13);
    //加密控制
    ptHead->u8TransportScramblingControl = BitsRead8(&tBitsBuf, 2);
    //读自适应字段标记
    ptHead->u8AdaptationFieldControl = BitsRead8(&tBitsBuf, 2);
    //记数器
    ptHead->u8ContinuityCounter = BitsRead8(&tBitsBuf, 4);
    
    //判断是否有自适应字段,如果有,做跳过处理
    if ((TS_AFC_TYPE_AF == ptHead->u8AdaptationFieldControl) || 
        (TS_AFC_TYPE_AF_PAYLOAD == ptHead->u8AdaptationFieldControl))
    {
        ptHead->u8AdaptationFieldLength = BitsRead8(&tBitsBuf, 8);
        if (ptHead->u8AdaptationFieldLength > 0)
        {
            ptHead->u8AdaptationFlags = BitsRead8(&tBitsBuf, 8);
        }

        ptHead->u8HeadLen = 4 + 1 + ptHead->u8AdaptationFieldLength;
    }
    else
    {
        ptHead->u8AdaptationFieldLength = 0;
        ptHead->u8AdaptationFlags = 0;

        ptHead->u8HeadLen = 4;
    }

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramPATReadInfo
功能:从TS包读取PAT信息
算法实现: 
参数说明:
         [I/O] ptpPATInfo 存储PAT包信息的结构
         [I]   pu8Buf TS包
         [I]   u32Len 必须为188
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsReadGetPat(TTsRead *ptTsInfo, TTsPatInfo *ptpPATInfo, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    u8 u8ByRemain = 0;
    u8 u8Offset = 0;
    u8 *pu8PatBuf;
    u8 u8PatLen;
    u8 u8Temp;
    s32 s32i = 0;

    TTsHeader *ptHead = &ptTsInfo->tHeader;
    
    //先解析ts头
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    if (0x01 != ptHead->u8PayloadUnitStartIndicator)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat without UnitStart bit.");
        return ERROR_TS_READ_UNIT_START;
    }

    //必须带负载
//     if ((TS_AFC_TYPE_AF != ptHead->u8AdaptationFieldControl) &&
//         (TS_AFC_TYPE_AF_PAYLOAD != ptHead->u8AdaptationFieldControl))
//     {
//         TspsPrintf(PD_TS_READ, "TsRead fail: pat pack without payload.");
//         return ERROR_TS_READ_ADAPTATION;
//     }

    if (ptHead->u8HeadLen >= TS_PACKET_LENGTH)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat header incorrect length[%d].", ptHead->u8HeadLen);
        return ERROR_TS_READ_HEAD_LENGTH;
    }

    //读取point_field
    u8Offset = pu8Buf[ptHead->u8HeadLen];

    //获得pat缓冲
    pu8PatBuf = pu8Buf + (ptHead->u8HeadLen + 1 + u8Offset);
    u8PatLen = TS_PACKET_LENGTH - (ptHead->u8HeadLen + 1 + u8Offset);
    if (u8PatLen < 12)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat pack incorrect length[%d].", u8PatLen);
        return ERROR_TS_READ_PSI_LENGTH;
    }

    BitsInit(&tBitsBuf, pu8PatBuf, u8PatLen);

    /*****下面是PAT表内容*****/
    //读TABLE ID
    u8Temp = BitsRead8(&tBitsBuf, 8);
    if(TS_PAT_TABLE_ID != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat table id wrong.");
        return ERROR_TS_READ_TABLE_ID;
    }
    
    //读段语法标记
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x01 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat SECTION_SYNTAX wrong.");
        return ERROR_TS_READ_SECTION_SYNTAX;
    }
    
    //读固定0标记
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x00 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat FIX_0 wrong.");
        return ERROR_TS_READ_FIX_0;
    }
    
    //跳过保留位(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPATInfo->u16SectionLength = BitsRead16(&tBitsBuf, 12);
    ptpPATInfo->u16TransportStreamId = BitsRead16(&tBitsBuf, 16);
    
    //跳过保留位(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPATInfo->u8VersionNumber = BitsRead8(&tBitsBuf, 5);
    ptpPATInfo->u8CurrentNextIndicator = BitsRead8(&tBitsBuf, 1);
    ptpPATInfo->u8SectionNumber = BitsRead8(&tBitsBuf, 8);
    ptpPATInfo->u8LastSectionNumber = BitsRead8(&tBitsBuf, 8);
    
    //计算剩余字节,应该去掉crc字段(4byte)
    u8ByRemain = ptpPATInfo->u16SectionLength - (40 / 8) - 4;
    u8Temp = ptpPATInfo->u8ProgramMapNum;
    
    //读PAT表信息
    ptpPATInfo->u8ProgramMapNum = 0;
    for(s32i = u8Temp; s32i < ((u8ByRemain / (32 / 8)) + u8Temp); s32i++)
    {
        ptpPATInfo->au16ProgramNumber[s32i] = BitsRead16(&tBitsBuf, 16);
        //跳过保留位(3)
        BitsSkip(&tBitsBuf, 3);
        if(ptpPATInfo->au16ProgramNumber[s32i] == 0x00)
        {
            ptpPATInfo->au16NetworkPid[s32i] = BitsRead16(&tBitsBuf, 13);
        }
        else
        {
            ptpPATInfo->au16ProgramMapPid[s32i] = BitsRead16(&tBitsBuf, 13);
            ptpPATInfo->u8ProgramMapNum ++;
        }
    }
    ptpPATInfo->u32Crc = BitsRead32(&tBitsBuf, 32);

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramPMTReadInfo
功能:从TS包读取PMT信息
算法实现: 
参数说明:
         [I/O] ptpPMTInfo 存储PMT包信息的结构
         [I]   pu8Buf TS包
         [I]   u32Len 必须为188
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsReadGetPmt(TTsRead *ptTsInfo, TTsPmtInfo *ptpPMTInfo, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    u8 u8Temp = 0;
    u8 u8ByRemain = 0;
    u8 u8Offset = 0;
    u8 *pu8PmtBuf;
    u8 u8PmtLen;

    TTsHeader *ptHead = &ptTsInfo->tHeader;
    
    //先解析ts头
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    //读unit start indicator,为0x01表示必须有point field字段
    if(0x01 != ptHead->u8PayloadUnitStartIndicator)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt without UnitStart bit.");
        return ERROR_TS_READ_UNIT_START;
    }

    //必须带负载
//     if ((TS_AFC_TYPE_AF != ptHead->u8AdaptationFieldControl) && 
//         (TS_AFC_TYPE_AF_PAYLOAD != ptHead->u8AdaptationFieldControl))
//     {
//         TspsPrintf(PD_TS_READ, "TsRead fail: pmt pack without payload.");
//         return ERROR_TS_READ_ADAPTATION;
//     }
    
    if (ptHead->u8HeadLen >= TS_PACKET_LENGTH)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt header incorrect length[%d].", ptHead->u8HeadLen);
        return ERROR_TS_READ_HEAD_LENGTH;
    }

    //读POINT FIELD,跳过处理
    u8Offset = pu8Buf[ptHead->u8HeadLen];
    
    //获得pmt缓冲
    pu8PmtBuf = pu8Buf + (ptHead->u8HeadLen + 1 + u8Offset);
    u8PmtLen = TS_PACKET_LENGTH - (ptHead->u8HeadLen + 1 + u8Offset);
    if (u8PmtLen < 16)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt pack incorrect length[%d].", u8PmtLen);
        return ERROR_TS_READ_PSI_LENGTH;
    }

    
    /*****下面是PMT表内容*****/
    BitsInit(&tBitsBuf, pu8PmtBuf, u8PmtLen);

    //读TABLE ID
    u8Temp = BitsRead8(&tBitsBuf, 8);
    if(TS_PMT_TABLE_ID != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt table id wrong.");
        return ERROR_TS_READ_TABLE_ID;
    }
    
    //读段语法标记
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x01 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt SECTION_SYNTAX wrong.");
        return ERROR_TS_READ_SECTION_SYNTAX;
    }
    
    //读固定0标记
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x00 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat FIX_0 wrong.");
        return ERROR_TS_READ_FIX_0;
    }
    
    //跳过保留位(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPMTInfo->u16SectionLength = BitsRead16(&tBitsBuf, 12);
    ptpPMTInfo->u16ProgramNumber = BitsRead16(&tBitsBuf, 16);
    
    //跳过保留位(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPMTInfo->u8VersionNumber = BitsRead8(&tBitsBuf, 5);
    ptpPMTInfo->u8CurrentNextIndicator = BitsRead8(&tBitsBuf, 1);
    ptpPMTInfo->u8SectionNumber = BitsRead8(&tBitsBuf, 8);
    ptpPMTInfo->u8LastSectionNumber = BitsRead8(&tBitsBuf, 8);
    
    //跳过保留位(3)
    BitsSkip(&tBitsBuf, 3);
    ptpPMTInfo->u16PcrPid = BitsRead16(&tBitsBuf, 13);
    
    //跳过保留位(4)
    BitsSkip(&tBitsBuf, 4);
    ptpPMTInfo->u16ProgramInfoLength = BitsRead16(&tBitsBuf, 12);
    BitsSkip(&tBitsBuf, ptpPMTInfo->u16ProgramInfoLength * 8);
    
    //计算剩余字节,应该去掉crc字段(4byte)
    u8ByRemain = ptpPMTInfo->u16SectionLength - 9 - ptpPMTInfo->u16ProgramInfoLength - 4;
    
    ptpPMTInfo->u8StreamNum = 0;
    while (u8ByRemain > 0)
    {
        ptpPMTInfo->au8StreamType[ptpPMTInfo->u8StreamNum] = BitsRead8(&tBitsBuf, 8);
        
        //跳过保留位(3)
        BitsSkip(&tBitsBuf, 3);
        ptpPMTInfo->au16ElementaryPid[ptpPMTInfo->u8StreamNum] = BitsRead16(&tBitsBuf, 13);
        
        //跳过保留位(4)
        BitsSkip(&tBitsBuf, 4);
        ptpPMTInfo->au16EsInfoLength[ptpPMTInfo->u8StreamNum] = BitsRead16(&tBitsBuf, 12);
        BitsSkip(&tBitsBuf, ptpPMTInfo->au16EsInfoLength[ptpPMTInfo->u8StreamNum] * 8);
        
        ptpPMTInfo->u8StreamNum ++;
        
        u8ByRemain -= (5 + ptpPMTInfo->au16EsInfoLength[ptpPMTInfo->u8StreamNum]);
    }
	ptpPMTInfo->u8StreamNum = 2;
    ptpPMTInfo->u32Crc = BitsRead32(&tBitsBuf, 32);

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramIsPesPacket
功能:判断当前TS包是否是PMT表中的合法包
算法实现: 
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   u16PID TS包的PID
         [O]   pu8StreamType 该包类型
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsReadIsPesPacket(TTsRead *ptTsInfo, u16 u16PID, u8 *pu8StreamType)
{
    TTsPatInfo *ptPAT = &ptTsInfo->tPatInfo;
    TTsPmtInfo *ptPMT = ptTsInfo->ptPmtInfo;

    s32 s32i = 0, s32j = 0;

    //如果pmt不支持就不解包，直到收到下一个可支持的pmt
    if (FALSE == ptTsInfo->bSupport) 
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: streams not support.");
        return ERROR_TS_READ_NOT_SUPPORT;
    }

    //循环PAT表,PMT表查找是否有当前PID存在
    for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i++)
    {
        for(s32j = 0; s32j < ptPMT->u8StreamNum; s32j++)
        {
            if(u16PID == ptPMT[s32i].au16ElementaryPid[s32j])
            {
                *pu8StreamType = ptPMT[s32i].au8StreamType[s32j];
                return TSPS_OK;
            }
        }

    }

    TspsPrintf(PD_TS_READ, "TsRead fail: stream pid[%d] not found.", u16PID);
    return ERROR_TS_READ_PID_NOT_FOUND;
}

/*=================================================================================
函数名: 以下三个函数
功能: 从一包包ts section中累积得到pes包，并解析出一帧es流
算法实现: 
参数说明:
返回值说明:成功返回TSPS_OK,否则返回唯一错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
// 从pes中的第一包ts中提取出pes_packet_length
static u16 TsReadGetPesLenFromFirstSection(u8 *pu8Buf)
{
    return (pu8Buf[4] << 8) | pu8Buf[5];
}

//封装好的回调一帧的函数
static u16 TsReadCallback(TTsRead *ptTsInfo, u8 u8StreamType)
{
    u8 u8StreamId;
    u16 u16Ret = TSPS_OK;
	TPesInfo *ptPesInfo = NULL;
    TspsFRAMEHDR *ptFrame = ptTsInfo->ptFrame;
	u32 u32LenOutput = 0;
	u8 u8Id = 0;
	TVideoInfo tVideoInfo = {0};
	
	u16Ret = TsGetArrayByStreamType(ptTsInfo, u8StreamType, &u8Id);
	if (TSPS_OK != u16Ret)
	{
		return u16Ret;
	}
	
	ptPesInfo = ptTsInfo->atTsInfo[u8Id].ptPesInfo;
    memset(ptPesInfo, 0 ,sizeof(TPesInfo));
	u16Ret = PesReadInfo(ptPesInfo, ptTsInfo->atTsInfo[u8Id].pu8PesBuf, ptTsInfo->atTsInfo[u8Id].u32Peslen, &u32LenOutput);
    if (TSPS_OK == u16Ret)
    {
        ptFrame->m_byMediaType = TsPsPTCovertStream2Rtp(u8StreamType);
        ptFrame->m_pbyData = ptPesInfo->pu8PayloadBuffer;
        ptFrame->m_dwDataSize = ptPesInfo->u32PayloadLength;
        ptFrame->m_dwTimeStamp = (u32)ptPesInfo->u64Pts;
        
        u8StreamId = TsPsGetStreamIdPrefix(u8StreamType);
		if (AUDIO_STREAM0 == u8StreamId || AUDIO_STREAM1 == u8StreamId || VIDEO_STREAM == u8StreamId)
        {
            TspsPrintf(PD_TS_READ, "TsRead get a frame successfully. <len=%d>",
                ptFrame->m_dwDataSize);
            
            if (ptTsInfo->pfFrameCB)
            {
                ptTsInfo->pfFrameCB(ptFrame, (KD_PTR)ptTsInfo->pvFrameContext);
            }
        }
        else
        {
            TspsPrintf(PD_TS_READ, "TsRead fail: stream type wrong.");
            u16Ret = ERROR_TS_READ_STREAM_TYPE;
        }
    }
    else
    {
        //解pes失败，可能是由于丢包引起
        TspsPrintf(PD_TS_READ, "TsRead: pes decode failed. may be caused by fragmentized pes pack.");
    }

	ptFrame->m_dwFrameID = ptTsInfo->atTsInfo[u8Id].u32FrameID++;

    return u16Ret;
}

//从一包包ts section中累积得到pes包并解析
static u16 TsReadDoPes(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len, u8 u8StreamType, 
                       u8 u8UnitStart)
{
    u16 u16Ret = TSPS_OK;
	u8 u8Id = 0;
	u16 u16PesPackLen;

	u16Ret = TsGetArrayByStreamType(ptTsInfo, u8StreamType, &u8Id);
	if (TSPS_OK != u16Ret)
	{
		return u16Ret;
	}
	TspsPrintf(PD_TS_READ, "u8UnitStart:%d, u8StreamType:%d, u8Id:%d\n", u8UnitStart, u8StreamType, u8Id);

    if (1 == u8UnitStart)
    {
        //未记录包长的情况下，且等到了下一包pes，回调上一帧
		if (ptTsInfo->atTsInfo[u8Id].bWaitNextPes)
		{
			u16Ret = TsReadCallback(ptTsInfo, ptTsInfo->atTsInfo[u8Id].u8LastType);
			
			//把pes缓冲清零，供下一次使用，此句不能去掉，以防下个pes丢失了第一包
			ptTsInfo->atTsInfo[u8Id].u32Peslen = 0;
        }
        
        //需要等下一帧
		ptTsInfo->atTsInfo[u8Id].bWaitNextPes = TRUE;

        //获取pes包长
        u16PesPackLen = TsReadGetPesLenFromFirstSection(pu8Buf);
		ptTsInfo->atTsInfo[u8Id].u16CurPesLen = PES_DATA_HEAD_LENGTH + u16PesPackLen;

        //存放新的数据
		memcpy(ptTsInfo->atTsInfo[u8Id].pu8PesBuf, pu8Buf, u32Len);
		ptTsInfo->atTsInfo[u8Id].u32Peslen = u32Len;      
    }
    else
    {
		if (FALSE == ptTsInfo->atTsInfo[u8Id].bWaitNextPes)
        {
            //如果上一帧已回调，而这一帧不是以UnitStart开头，那么说明丢了第一包
            TspsPrintf(PD_TS_READ, "TsRead warning: first pack in current pes may lose.");
			ptTsInfo->atTsInfo[u8Id].bWaitNextPes = TRUE;
        }
        
		if (ptTsInfo->atTsInfo[u8Id].u32Peslen + u32Len > MAX_VIDEO_FRAME_LEN)
        {
            TspsPrintf(PD_TS_READ, "TsRead fail: frame buffer full.");
            u16Ret = ERROR_TS_READ_BUFF_FULL;
        }
        else
        {
			memcpy(ptTsInfo->atTsInfo[u8Id].pu8PesBuf + ptTsInfo->atTsInfo[u8Id].u32Peslen, pu8Buf, u32Len);
			ptTsInfo->atTsInfo[u8Id].u32Peslen += u32Len;
		}       
    }

	ptTsInfo->atTsInfo[u8Id].u8LastType = u8StreamType;

    //记录包长的情况下：判断pes长度是否足够，若有，转换成es流，回调一帧
    //如果本次包长记录不正确，不回调，等下一包来再做判断
	if (ptTsInfo->atTsInfo[u8Id].u32Peslen >= ptTsInfo->atTsInfo[u8Id].u16CurPesLen && 
		ptTsInfo->atTsInfo[u8Id].u16CurPesLen > PES_DATA_HEAD_LENGTH)
    {
        u16Ret = TsReadCallback(ptTsInfo, u8StreamType);

        //把pes缓冲清零，供下一次使用，此句不能去掉，以防下个pes丢失了第一包
		ptTsInfo->atTsInfo[u8Id].u32Peslen = 0;

        //已回调，不需要等下一帧
		ptTsInfo->atTsInfo[u8Id].bWaitNextPes = FALSE;
	}   
    
    return u16Ret;
}

/*=================================================================================
函数名:TSProgramPayloadReadInfo
功能:解析一个TS包
算法实现: 
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   pu8Buf TS包
         [I]   u32Len 必须为188
返回值说明:成功返回TSPS_OK,否则返回唯一错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsReadGetPayload(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len)
{
    u16 u16Ret;
    u8 u8StreamType;
    u8 u8ByRemain = 0;
    u8 *pu8PesBuf;
    TTsHeader *ptHead = &ptTsInfo->tHeader;

	if( ptHead->u16Pid == 0x100 )
	{
		printf( "[TsReadGetPayload ] Debug. ptHead->u16Pid = 0x100\n" );
	}

	if( ptHead->u16Pid == 0x101 )
	{
		printf( "[TsReadGetPayload ] Debug. ptHead->u16Pid = 0x101\n" );
	}

    //先解析ts头
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    //判断此PID在程序映射表中是否存在,如果存在,说明此TS包合法
    u16Ret = TsReadIsPesPacket(ptTsInfo, ptHead->u16Pid, &u8StreamType);
    if (TSPS_OK != u16Ret)
    {
        return u16Ret;
    }

    // pes data start, len = u8ByRemain
    pu8PesBuf = pu8Buf + ptHead->u8HeadLen;

    u8ByRemain = TS_PACKET_LENGTH - ptHead->u8HeadLen;

    u16Ret = TsReadDoPes(ptTsInfo, pu8PesBuf, u8ByRemain, u8StreamType,
                         ptTsInfo->tHeader.u8PayloadUnitStartIndicator);

    return u16Ret;
}

/*=================================================================================
函数名:TSProgramParsePacket
功能:判断TS包内容,进行相应的解析
算法实现: 
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   pu8Buf TS包
         [I]   u32Len 必须为188
返回值说明:成功返回TSPS_OK,否则返回唯一错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsReadParsePacket(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;
    u16 u16PID = -1;
    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;
    s32 s32i = 0;
    u16 u16Ret = 0;
	u8 u8Loop = 0;

    ptPAT = &ptTsInfo->tPatInfo;
    ptPMT = ptTsInfo->ptPmtInfo;

    BitsInit(&tBitsBuf, pu8Buf, TS_PACKET_LENGTH);

    //跳过包头
    BitsSkip(&tBitsBuf, 11);
    //获得PID
    u16PID = BitsRead16(&tBitsBuf, 13);

    //根据PID判断TS包的属性
    if (TS_PAT_TABLE_ID == u16PID)  //pat
    {
        u16Ret = TsReadGetPat(ptTsInfo, ptPAT, pu8Buf, u32Len);
        if (TSPS_OK == u16Ret)
        {
            if (ptPAT->u8ProgramMapNum != 1)
            {
                //如果本次pat节目数不为1，那么不支持，直到下一个支持的pat
                ptTsInfo->bSupport = FALSE;
                TspsPrintf(PD_TS_READ, "TsRead fail: program num[%d]<>1.");
                u16Ret = ERROR_TS_READ_NOT_SUPPORT;

                if (ptTsInfo->pfProgramCB)
                {
                    ptTsInfo->pfProgramCB(MEDIA_TYPE_NULL, MEDIA_TYPE_NULL, MEDIA_TYPE_NULL,
                                          (KD_PTR)ptTsInfo->pvProgramContext);
                }
            }
            else
            {
                //节目数为1，可解
                ptTsInfo->bSupport = TRUE;
            }
        }

        return u16Ret;
    }
	else if (TS_NULL_PACKET_ID == u16PID)//空包
	{
		return u16Ret;
	}
    else    //pmt
    {
        for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i++)
        {
			if (ptPMT[s32i].u16PcrPid == u16PID)
			{
				return u16Ret;
			}
            //判断是否是PMT包，如果没有pat或pat解析失败，pid表都不会正确
            if (u16PID == ptPAT->au16ProgramMapPid[s32i])
            {
                u16Ret = TsReadGetPmt(ptTsInfo, &(ptPMT[s32i]), pu8Buf, u32Len);
                if (TSPS_OK == u16Ret)
                {
                    u8 u8VPT = MEDIA_TYPE_NULL;
                    u8 u8APT = MEDIA_TYPE_NULL;
					u8 u8OldVPT = TsPsPTCovertStream2Rtp(ptTsInfo->atTsInfo[0].u8Type);
					u8 u8OldAPT = TsPsPTCovertStream2Rtp(ptTsInfo->atTsInfo[1].u8Type);

					for (u8Loop = 0; u8Loop < ptPMT[s32i].u8StreamNum; u8Loop++)
					{
						if (TsPsGetStreamIdPrefix(ptPMT[s32i].au8StreamType[u8Loop]) == AUDIO_STREAM0)
                        {
                             ptTsInfo->atTsInfo[1].u8Type = ptPMT[s32i].au8StreamType[u8Loop];
                             u8APT = TsPsPTCovertStream2Rtp(ptTsInfo->atTsInfo[1].u8Type);
                        }
                        else if (TsPsGetStreamIdPrefix(ptPMT[s32i].au8StreamType[u8Loop]) == VIDEO_STREAM)
                        {
                             ptTsInfo->atTsInfo[0].u8Type = ptPMT[s32i].au8StreamType[u8Loop];
                             u8VPT = TsPsPTCovertStream2Rtp(ptTsInfo->atTsInfo[0].u8Type);
                        }
                    }

                    if (u8VPT == MEDIA_TYPE_NULL && u8APT == MEDIA_TYPE_NULL)
                    {
                        ptTsInfo->bSupport = TRUE;
                        u16Ret = ERROR_TS_READ_NOT_SUPPORT;
                    }

                    if (ptTsInfo->pfProgramCB && (u8APT != u8OldAPT || u8VPT != u8OldVPT))
                    {
                        //只有流改变了才回调
                        ptTsInfo->pfProgramCB(u8VPT, u8APT, MEDIA_TYPE_NULL, (KD_PTR)ptTsInfo->pvProgramContext);
                    }
                }

                return u16Ret;
            }
        }
    }

    //处理普通TS包
    u16Ret = TsReadGetPayload(ptTsInfo, pu8Buf, u32Len);
    
    return u16Ret;
}

/*=================================================================================
函数名:TSProgramParseBuffer
功能:分析一段TS流
算法实现: 
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   pu8Buf TS流
         [I]   u16Len 流长度
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 TsReadInputStream(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len)
{
    u16 u16Ret;
	u32 dwLoop = 0;
	u32 dwOffSize = 0;

    if (NULL == ptTsInfo)
    {
        return ERROR_TS_READ_INPUT_PARAM;
    }

    if (NULL == pu8Buf || TS_PACKET_LENGTH != u32Len)
    {
        return ERROR_TS_READ_INPUT_PARAM;
    }

	if (TRUE == ptTsInfo->bFirstPack)
	{
		for (dwLoop = 0; dwLoop < TS_PACKET_LENGTH; dwLoop++)
		{
			if (TS_PACKET_SYNC == pu8Buf[dwLoop] && 00 == pu8Buf[dwLoop+1])
			{
				break;
			}			
		}

		dwOffSize = dwLoop;
		OspPrintf(TRUE, FALSE, "[TsReadInputStream] offsize:%d\n", dwOffSize);
		
		if (TS_PACKET_LENGTH == dwOffSize)
		{
			dwOffSize = 0;
		}

		if (0 != dwOffSize)
		{
			memcpy(ptTsInfo->byTempTSBuf, pu8Buf+dwOffSize, TS_PACKET_LENGTH-dwOffSize);
		}
		ptTsInfo->dwTempBufLen = TS_PACKET_LENGTH - dwOffSize;
		ptTsInfo->bFirstPack = FALSE;
		return TSPS_OK;
	}
	else
	{
		if (0 != ptTsInfo->dwTempBufLen)
		{
			memcpy(ptTsInfo->byTempTSBuf+ptTsInfo->dwTempBufLen, pu8Buf, TS_PACKET_LENGTH-ptTsInfo->dwTempBufLen);
		}
		else
		{
			memcpy(ptTsInfo->byTempTSBuf, pu8Buf, TS_PACKET_LENGTH);
		}
	}

    if (TS_PACKET_SYNC != ptTsInfo->byTempTSBuf[0])
    {
		OspPrintf(TRUE, FALSE, "%x, len:%d \n", ptTsInfo->byTempTSBuf[0], ptTsInfo->dwTempBufLen);
		//可能更换码流，重新检测下偏移位置
		ptTsInfo->bFirstPack = TRUE;
		ptTsInfo->dwTempBufLen = 0;
        return ERROR_TS_READ_HEAD_SYNC;
    }

    //如果字节足够188个，则解析这一包
    u16Ret = TsReadParsePacket(ptTsInfo, ptTsInfo->byTempTSBuf, TS_PACKET_LENGTH);

	if (0 != ptTsInfo->dwTempBufLen)
	{
		memcpy(ptTsInfo->byTempTSBuf, pu8Buf+TS_PACKET_LENGTH-ptTsInfo->dwTempBufLen, ptTsInfo->dwTempBufLen);
	}		

    return u16Ret;
}

u16 TsGetArrayByStreamType(TTsRead *ptTsInfo, u8 u8StreamType, u8* pu8Id)
{
	u16 u16Ret = TSPS_OK;
	u8 u8Loop = 0;

	for (u8Loop = 0; u8Loop < MAX_PROGRAM_MAP_NUM; u8Loop++)
	{
		if (ptTsInfo->atTsInfo[u8Loop].u8Type == u8StreamType)
		{
			break;
		}
	}

	if (u8Loop == MAX_PROGRAM_MAP_NUM)
	{
		u16Ret = ERROR_TS_READ_PID_NOT_FOUND;
		return u16Ret;
	}

	*pu8Id = u8Loop;
	
	return u16Ret;
}
