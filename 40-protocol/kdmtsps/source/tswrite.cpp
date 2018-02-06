/*=================================================================================
模块名:TS流的写
文件名:ts.cpp
相关文件:ts.h
实现功能:TS流的写
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

// just declare writeindexfile
int WriteIndexFile(const s8 aszIndexFile[], /*const u8 au8TmpIndexFile[], */const u32 u32SegmentDuration, \
				   const s8 au8OutputPrefix[], const s8 au8HttpPrefix[], s8* psz8Url, s8* IV, const u32 u32FirstSegment, \
				   const u32 u32LastSegment, BOOL bEnd, const u32 nWindow);
BOOL32 CheckIfZero(s8 *IV);

FILE * g_pfFile = NULL;
u32 g_dwFileID = 0;

extern BOOL32 g_bTswSave;

/*=================================================================================
函数名:TsProgramOpen
功能:打开TS句柄,分配内存空间
算法实现:
参数说明:
         [I/O] ptpTSInfo 存储TS包信息的结构
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
TTsWrite *TsWriteOpen(TspsSectionCallback pfCallback, void *pvContext)
{
    s32 s32i = 0;

    TTsWrite *ptTsInfo = NULL;
    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;
    BOOL32 bFail = FALSE;
	if (g_bTswSave)
	{
		g_dwFileID = 0;
		g_pfFile = fopen("old-0", "wb");
	}

    //分配相应内存
    do
    {
        ptTsInfo = (TTsWrite *)malloc(sizeof(TTsWrite));
        if(NULL == ptTsInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo, 0, sizeof(TTsWrite));

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

        ptTsInfo->pu8TsBuf = (u8 *)malloc(TS_PACKET_LENGTH);
        if(NULL == ptTsInfo->pu8TsBuf)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo->pu8TsBuf, 0, TS_PACKET_LENGTH);

        ptTsInfo->ptPesInfo = (TPesInfo *)malloc(sizeof(TPesInfo));
        if(NULL == ptTsInfo->ptPesInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo->ptPesInfo, 0, sizeof(TPesInfo));

		// lg update，test
        ptTsInfo->pu8PesBuf = (u8 *)malloc(/*PES_PACKET_MAX_LENGTH*/ES_PACEKT_MAX_LENGTH);
        if(NULL == ptTsInfo->pu8PesBuf)
        {
            bFail = TRUE;
            break;
        }
        memset(ptTsInfo->pu8PesBuf, 0, /*PES_PACKET_MAX_LENGTH*/ES_PACEKT_MAX_LENGTH);

        ptTsInfo->pfCallback = pfCallback;
        ptTsInfo->pvContext = pvContext;

		OspSemBCreate( &ptTsInfo->hWriteSem );
//		OspSemTake( ptTsInfo->hWriteSem );

    } while (0);

    if (bFail)
    {
        TspsPrintf(PD_TS_WRITE, "TsWriteOpen fail: memory malloc error.");
        TsWriteClose(ptTsInfo);
        ptTsInfo = NULL;
    }

	ptTsInfo->ptTsSegment = NULL;

    return ptTsInfo;
}

/*=================================================================================
函数名:TsProgramClose
功能:关闭TS句柄,释放内存空间
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
返回值说明:返回0
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 TsWriteClose(TTsWrite *ptTsInfo)
{
    u32 u32Index = 0;

    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;

    s8 au8SegmentFile[TS_PACKET_LENGTH];
	u8 abyEncryptOut[AES_ENCRYPT_BYTENUM] = {0};
	s32 nRet = 0;

    if (NULL == ptTsInfo)
    {
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

    ptPAT = &ptTsInfo->tPatInfo;

    TSPSFREE(ptPAT->pu8Buffer);

    ptPMT = ptTsInfo->ptPmtInfo;
    for(u32Index = 0; u32Index < MAX_PROGRAM_MAP_NUM; u32Index++)
    {
        TSPSFREE(ptPMT[u32Index].pu8Buffer);
    }

    TSPSFREE(ptTsInfo->ptPmtInfo);

    TSPSFREE(ptTsInfo->pu8TsBuf);
    TSPSFREE(ptTsInfo->pu8PesBuf);
    TSPSFREE(ptTsInfo->ptPesInfo);

	if( ptTsInfo->ptTsSegment != NULL )
	{
		if( ptTsInfo->ptTsSegment->fpSegment != NULL )
		{
			if (0 != ptTsInfo->wKeySize /*&& ptTsInfo->wTempEncryptLen != 0*/)
			{
				TsWriteEncryptBuffer(ptTsInfo, ptTsInfo->ptTsSegment, NULL, 0);
			}
			else
			{
				fclose(ptTsInfo->ptTsSegment->fpSegment);
				ptTsInfo->ptTsSegment->fpSegment = NULL;
			}
		}
		// 删除所有切片ts文件
		for( u32Index = ptTsInfo->ptTsSegment->u32FirstSegment; u32Index <= ptTsInfo->ptTsSegment->u32LastSegment; u32Index++ )
		{
			sprintf( au8SegmentFile, "%s-%d.ts", ptTsInfo->ptTsSegment->aau8OutputFilePrefix, u32Index );
			remove( au8SegmentFile );
			memset( au8SegmentFile, 0, sizeof( au8SegmentFile ) );
		}
		// 删除m3u8索引文件
		sprintf( au8SegmentFile, "%s", ptTsInfo->ptTsSegment->aau8IndexFileName );
		remove( au8SegmentFile );
	}

	TSPSFREE(ptTsInfo->ptTsSegment);

	if( ptTsInfo->hWriteSem != NULL )
	{
		OspSemDelete( ptTsInfo->hWriteSem );
	}

    TSPSFREE(ptTsInfo);

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramGetExclusivePID
功能:得到0x0010 - 0x1FFE的唯一随机数
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
返回值说明:返回唯一标识
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWriteGetExclusivePID(TTsWrite *ptTSInfo)
{
    u16 u16Count = 0;
    u32 u32Tick = OspTickGet();
    u16 u16PID = 0;
    s32 s32i = 0;
    BOOL32 bFlag = FALSE;

    srand(u32Tick);

    //循环8192次
    while(u16Count < TS_RAND_MAX)
    {
        u16Count ++;
        bFlag = FALSE;
        u16PID = (rand() % (8190 - 10)) + 10;

        //检查当前随机数是否已经用过
        for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM; s32i++)
        {
            if(u16PID == ptTSInfo->au16Pid[s32i])
            {
                bFlag = TRUE;
            }
        }

        //当前随机书没有被使用过
        if(FALSE == bFlag)
        {
            for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM; s32i++)
            {
                //记录这个随机数,防止下次使用
                if(0 == ptTSInfo->au16Pid[s32i])
                {
                    ptTSInfo->au16Pid[s32i] = u16PID;
                    return u16PID;
                }
            }
            return u16PID;
        }
    }

    TspsPrintf(PD_TS_WRITE, "TsWriteGetExclusivePID fail.");

    return -1;
}

/*=================================================================================
函数名:TsWriteWriteHeader
功能:写ts头
算法实现:
参数说明:
         [I]   u16elementaryPID 流PID
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWriteSetHeader(TTsWrite *ptTSInfo, TBit *ptBitsBuf)
{
    s32 s32i;
    TTsHeader *ptHead = &ptTSInfo->tHeader;
    u8 u8SubLen = 0;

    //写4字节TS包头
    BitsWrite8(ptBitsBuf, 8, TS_PACKET_SYNC);//0x47
    BitsWrite8(ptBitsBuf, 1, ptHead->u8TransportErrorIndicator);
    BitsWrite8(ptBitsBuf, 1, ptHead->u8PayloadUnitStartIndicator);
    BitsWrite8(ptBitsBuf, 1, ptHead->u8TransportPriority);
    BitsWrite16(ptBitsBuf, 13, ptHead->u16Pid);//写PMT包PID
    BitsWrite8(ptBitsBuf, 2, ptHead->u8TransportScramblingControl);
    BitsWrite8(ptBitsBuf, 2, ptHead->u8AdaptationFieldControl);//Adaptation_field followed by payload
    BitsWrite8(ptBitsBuf, 4, ptHead->u8ContinuityCounter);

    //自适应字段(填充长度，标志位)
    BitsWrite8(ptBitsBuf, 8, ptHead->u8AdaptationFieldLength);
    if (ptHead->u8AdaptationFieldLength > 0)
    {
        BitsWrite8(ptBitsBuf, 8, ptHead->u8AdaptationFlags);
    }

    if (0x10 == ptHead->u8AdaptationFlags)
    {
        //写PCR
        BitsWrite64(ptBitsBuf, 33, ptTSInfo->u64PCRBase);//时钟基准
        BitsWrite8(ptBitsBuf, 6, 63);
        BitsWrite64(ptBitsBuf, 9,  ptTSInfo->u16PCRExt);
        u8SubLen = 8;
    }

    //padding
    for(s32i = 0; s32i < ptHead->u8AdaptationFieldLength - u8SubLen - 1; s32i ++)
    {
        BitsWrite8(ptBitsBuf, 8, 0xff);
    }

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramAddPMTStream
功能:增加一路编码流
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   u16ProgramIndex 该PMT所在位置
         [I]   u8StreamType 流类型
         [I]   u16elementaryPID 流PID
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWriteSetPmt(TTsWrite *ptTSInfo, u16 u16ProgramIndex, TTsProgramInfo *ptInfo)
{
    u8 u8AdapLength = 0;
    u8 u8SectionLength = 0;
    s32 s32i = 0;
    //获得的CRC值
    u32 u32Crc32 = 0;
    //计算CRC的开始位置
    u32 u32CrcStart = 0;

    TTsPatInfo *ptPAT = &ptTSInfo->tPatInfo;
    TTsPmtInfo *ptPMT = &(ptTSInfo->ptPmtInfo[u16ProgramIndex]);

    TBit tBitsBuf;

    if (u16ProgramIndex >= MAX_STREAM_NUM ||
        0 == ptInfo->u8StreamNum || MAX_PROGRAM_MAP_NUM < ptInfo->u8StreamNum)
    {
        TspsPrintf(PD_TS_WRITE,
            "TsWrite fail: input programId[%d] or stream number[%d] error.",
            u16ProgramIndex, ptInfo->u8StreamNum);
        return ERROR_TS_WRITE_STREAM_NUM;
    }

    //填写PMT表
    for (s32i=0; s32i<ptInfo->u8StreamNum; s32i++)
    {
        ptPMT->au8StreamType[s32i] = ptInfo->au8StreamType[s32i];
        ptPMT[u16ProgramIndex].au16ElementaryPid[s32i] = ptInfo->au16StreamPid[s32i];
        ptPMT[u16ProgramIndex].u8StreamNum = ptInfo->u8StreamNum;
        ptPMT[u16ProgramIndex].u8VersionNumber++;
    }

    //计算PMT分段长度
    u8SectionLength = 9 + 5 * ptPMT[u16ProgramIndex].u8StreamNum + 4;

    //计算调整字段长度
    u8AdapLength = TS_PACKET_LENGTH - 4 - (u8SectionLength + 4) -1;

    BitsInit(&tBitsBuf, ptPMT[u16ProgramIndex].pu8Buffer, TS_PACKET_LENGTH);

    //写ts头
    memset(&ptTSInfo->tHeader, 0, sizeof(TTsHeader));
    ptTSInfo->tHeader.u8PayloadUnitStartIndicator = 1;
    ptTSInfo->tHeader.u16Pid = ptPAT->au16ProgramMapPid[u16ProgramIndex];
    ptTSInfo->tHeader.u8AdaptationFieldControl = 3;
    ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdapLength;
    TsWriteSetHeader(ptTSInfo, &tBitsBuf);

    //写PMT信息
    BitsWrite8(&tBitsBuf, 8, 0);//pointer_field
    BitsWrite8(&tBitsBuf, 8, 2);//table_id
    BitsWrite8(&tBitsBuf, 1, 1);//section_syntax_indicator
    BitsWrite8(&tBitsBuf, 1, 0);//set to '0'
    BitsWrite8(&tBitsBuf, 2, 3);//reserved
    BitsWrite16(&tBitsBuf, 12, u8SectionLength);//段长度
    BitsWrite16(&tBitsBuf, 16, (u16)(u16ProgramIndex + 1));//节目号
    BitsWrite8(&tBitsBuf, 2, 3);//reserved
    BitsWrite8(&tBitsBuf, 5, ptPMT[u16ProgramIndex].u8VersionNumber);//版本号
    BitsWrite8(&tBitsBuf, 1, 1);//current_nect_indicator
    BitsWrite8(&tBitsBuf, 8, 0);//section_number
    BitsWrite8(&tBitsBuf, 8, 0);//last_section_number
    BitsWrite8(&tBitsBuf, 3, 7);//reserved
    BitsWrite16(&tBitsBuf, 13, ptPMT[u16ProgramIndex].au16ElementaryPid[0]);//PCR基准的PID
    BitsWrite8(&tBitsBuf, 4, 15);//reserved
    BitsWrite16(&tBitsBuf, 12, 0);//program_info_length

    //写编码流信息
    for(s32i = 0; s32i < ptPMT[u16ProgramIndex].u8StreamNum; s32i ++)
    {
        BitsWrite8(&tBitsBuf, 8, ptPMT[u16ProgramIndex].au8StreamType[s32i]);
        BitsWrite8(&tBitsBuf, 3, 7);
        BitsWrite16(&tBitsBuf, 13, ptPMT[u16ProgramIndex].au16ElementaryPid[s32i]);
        BitsWrite8(&tBitsBuf, 4, 15);
        BitsWrite16(&tBitsBuf, 12, 0);
    }

    //写CRC
    //CRC校验的开始位置是从PAT的TABLE_ID字段开始的
    u32CrcStart = 4 +1 + u8AdapLength + 1;
    u32Crc32 = CRCGetCRC32(ptPMT[u16ProgramIndex].pu8Buffer + u32CrcStart, TS_PACKET_LENGTH - u32CrcStart - 4);
    BitsWrite32(&tBitsBuf, 32, u32Crc32);

    return TSPS_OK;
}

/*=================================================================================
函数名:TsProgramSetParam
功能:初始化一路TS流信息
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I/O] ptProgramInfo TS流信息
         [I]   u8ProgramNum TS流路数(复用时使用)
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWriteSetPat(TTsWrite *ptTsInfo, TTsPatPrograms *ptPrograms)
{
    //调整字段长度
    u8 u8AdaptationLength = 0;
    //获得的CRC值
    u32 u32Crc32 = 0;
    //计算CRC的开始位置
    u32 u32CrcStart = 0;

    TBit tBitsBuf;

    s32 s32i = 0;

    TTsPatInfo *ptPAT = NULL;

    if (NULL == ptTsInfo || NULL == ptPrograms)
    {
        TspsPrintf(PD_TS_WRITE, "TsWrite fail: set pat input error.");
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

    if (0 == ptPrograms->u8ProgramNum || ptPrograms->u8ProgramNum > MAX_STREAM_NUM)
    {
        TspsPrintf(PD_TS_WRITE, "TsWrite fail: input program number[%d] error.",
            ptPrograms->u8ProgramNum);
        return ERROR_TS_WRITE_PROGRAM_NUM;
    }

    ptPAT = &ptTsInfo->tPatInfo;

    //如果PAT未初始化，则产生PAT的PID
    if(0x00 == ptPAT->u8ProgramMapNum)
    {
        //填写流位置PID
        ptPAT->u16TransportStreamId = TsWriteGetExclusivePID(ptTsInfo);
        ptPAT->u8VersionNumber ++;
        ptPAT->u8CurrentNextIndicator = 0x01;
        ptPAT->u8SectionNumber = 0;
        ptPAT->u8LastSectionNumber = 0;
    }

    //重新设定节目数
    ptPAT->u8ProgramMapNum = ptPrograms->u8ProgramNum;

    //填写每个节目信息
    for(s32i = 0; s32i < ptPrograms->u8ProgramNum; s32i++)
    {
        ptPAT->au16ProgramNumber[s32i] = s32i + 1;
        //如果之前有pid，不用使用新的
        if (0x0000 == ptPAT->au16ProgramMapPid[s32i])
        {
            ptPAT->au16ProgramMapPid[s32i] = TsWriteGetExclusivePID(ptTsInfo);
        }
    }

    //计算分段长度,此分段长度是指每个节目号对应节目节目映射表的大小*节目数(节目数*4字节)
    //+分段长度后,到程序映射信息的长度(5字节)+CRC校验(4字节) from section_length
    ptPAT->u16SectionLength = 4 * ptPAT->u8ProgramMapNum + 5 + 4;

    //计算自适应区字段长度,长度是188-PAT所占的空间(含point_field)(bySectionLength + 4) - 4字节TS包头 - 1字节用于调整字段填写本身长度
    u8AdaptationLength = TS_PACKET_LENGTH - 4 - (ptPAT->u16SectionLength + 4)- 1;
    if(0 > u8AdaptationLength)
    {
        u8AdaptationLength = 0;
    }

    BitsInit(&tBitsBuf, ptPAT->pu8Buffer, TS_PACKET_LENGTH);

    //写ts头
    memset(&ptTsInfo->tHeader, 0, sizeof(TTsHeader));
    ptTsInfo->tHeader.u8PayloadUnitStartIndicator = 1;
    ptTsInfo->tHeader.u16Pid = TS_PAT_TABLE_ID;
    ptTsInfo->tHeader.u8AdaptationFieldControl = 3;
    ptTsInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
    TsWriteSetHeader(ptTsInfo, &tBitsBuf);

    //写PAT表
    BitsWrite8(&tBitsBuf, 8, 0);//指针
    BitsWrite8(&tBitsBuf, 8, 0);//表ID
    BitsWrite8(&tBitsBuf, 1, 1);//段语法标记
    BitsWrite8(&tBitsBuf, 1, 0);
    BitsWrite8(&tBitsBuf, 2, 3);
    BitsWrite16(&tBitsBuf, 12, ptPAT->u16SectionLength);//段长度
    BitsWrite16(&tBitsBuf, 16, ptPAT->u16TransportStreamId);//唯一PID，ts的PID
    BitsWrite8(&tBitsBuf, 2, 3);
    BitsWrite8(&tBitsBuf, 5, ptPAT->u8VersionNumber);//版本号
    BitsWrite8(&tBitsBuf, 1, ptPAT->u8CurrentNextIndicator);//当前包有效
    BitsWrite8(&tBitsBuf, 8, ptPAT->u8SectionNumber);//当前段号
    BitsWrite8(&tBitsBuf, 8, ptPAT->u8LastSectionNumber);//上次的段号

    //写入PMT的PID信息
    for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i++)
    {
        //PMT号
        BitsWrite16(&tBitsBuf, 16, ptPAT->au16ProgramNumber[s32i]);
        BitsWrite8(&tBitsBuf, 3, 7);
        //PMT的PID
        BitsWrite16(&tBitsBuf, 13, ptPAT->au16ProgramMapPid[s32i]);
    }

    //写CRC
    //CRC校验的开始位置是从PAT的TABLE_ID字段开始的
    u32CrcStart = 4 + 1 + u8AdaptationLength + 1;
    u32Crc32 = CRCGetCRC32(ptPAT->pu8Buffer + u32CrcStart, TS_PACKET_LENGTH - u32CrcStart - 4);
    BitsWrite32(&tBitsBuf, 32, u32Crc32);

    for(s32i = 0; s32i < ptPrograms->u8ProgramNum; s32i++)
    {
        //创建编码流ID,构造PMT
        s32 s32sn;
        for(s32sn = 0; s32sn < ptPrograms->atProgram[s32i].u8StreamNum; s32sn++)
        {
            TsWriteSetPmt(ptTsInfo, (u16)s32i, &ptPrograms->atProgram[s32i]);
        }
    }

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramPATAddContinuityCounter
功能:增加PAT包的记数
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWritePatAddContinuityCounter(TTsWrite *ptTSInfo)
{
    TBit tBitsBuf;

    TTsPatInfo *ptPAT = &ptTSInfo->tPatInfo;

    //直接跳到记数段,修改值
    BitsInit(&tBitsBuf, ptPAT->pu8Buffer, TS_PACKET_LENGTH);
    BitsSkip(&tBitsBuf, 28);
    ptPAT->u8ContinuityCounter = BitsRead8(&tBitsBuf, 4);
    ptPAT->u8ContinuityCounter++;

    //如果序号到达0xf,则回0
    if(0xf < ptPAT->u8ContinuityCounter)
    {
        ptPAT->u8ContinuityCounter = 0;
    }

    BitsInit(&tBitsBuf, ptPAT->pu8Buffer, TS_PACKET_LENGTH);
    BitsSkip(&tBitsBuf, 28);
    BitsWrite8(&tBitsBuf, 4, ptPAT->u8ContinuityCounter);

    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramPMTAddContinuityCounter
功能:增加PMT包的记数
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWritePmtAddContinuityCounter(TTsWrite *ptTSInfo)
{
    TBit tBitsBuf;

    TTsPatInfo *ptPAT = &ptTSInfo->tPatInfo;
    TTsPmtInfo *ptPMT = ptTSInfo->ptPmtInfo;

    s32 s32i = 0;

    for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i ++)
    {
        //直接跳到记数段,修改值
        BitsInit(&tBitsBuf, ptPMT[s32i].pu8Buffer, TS_PACKET_LENGTH);
        BitsSkip(&tBitsBuf, 28);
        ptPMT[s32i].u8ContinuityCounter = BitsRead8(&tBitsBuf, 4);
        ptPMT[s32i].u8ContinuityCounter++;

        if(0xf < ptPMT[s32i].u8ContinuityCounter)
        {
            ptPAT->u8ContinuityCounter = 0;
        }

        BitsInit(&tBitsBuf, ptPMT[s32i].pu8Buffer, TS_PACKET_LENGTH);
        BitsSkip(&tBitsBuf, 28);
        BitsWrite8(&tBitsBuf, 4, ptPMT[s32i].u8ContinuityCounter);
    }

    return TSPS_OK;

}

/*=================================================================================
函数名:TSProgramWriteTsPayload
功能:根据PID,是否是一个PES的首字节，初始化当前TS包信息
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I] byWritePos 是否是一个PES的首字节
         [I] u16PID 此路流的PID
         [I] bool32IsRealTime 是否是实时编码
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u16 TsWriteSetPayload(TTsWrite *ptTSInfo, s32 s32WritePos, u8 u8PmtPos, u8 u8StreamPos)
{
    s32 s32i = 0;

    //修改记数
    ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos]++;
    if(0xf < ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos])
    {
        ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos] = 0;
    }

    memset(&ptTSInfo->tHeader, 0, sizeof(TTsHeader));

    ptTSInfo->tHeader.u16Pid = ptTSInfo->ptPmtInfo[u8PmtPos].au16ElementaryPid[u8StreamPos];
    ptTSInfo->tHeader.u8ContinuityCounter = ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos];

    //PES第一包的payload_unit_start_indicator为1
    //同时写该路的PCR值
    //PCR是根据PTS得到的
    if(0 == s32WritePos)
    {
        ptTSInfo->tHeader.u8PayloadUnitStartIndicator = 0x1;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x10;//PCR_flag
    }
    else
    {
        ptTSInfo->tHeader.u8PayloadUnitStartIndicator = 0x0;
    }


    return TSPS_OK;
}

/*=================================================================================
函数名:TSProgramWriteTsPack
功能:根据TS结构信息,写TS包内容
算法实现:
参数说明:
         [I/O] ptTSInfo 存储TS包信息的结构
         [I]   pu8PesBuf PES包内容
         [I]   s32PesLen PES包长度
         [O]   pu8Buf 存储TS包的缓冲
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u32 TsWriteWriteTsPack(TTsWrite *ptTSInfo, u8 *pu8PesBuf, u32 u32PesLen)
{
    u32 u32UsedLen = 0;
    u8  u8AdaptationLength = 0;
    TBit tBitsBuf;
    u8 *pu8Buf = ptTSInfo->pu8TsBuf;

    BitsInit(&tBitsBuf, pu8Buf, TS_PACKET_LENGTH);

    //当前包既有一帧的第一个字节,又是PCR基准流
    if (1 == ptTSInfo->tHeader.u8PayloadUnitStartIndicator)
    {
        if (u32PesLen >= 176)
        {
            u8AdaptationLength = TS_PACKET_LENGTH - 4 - 176 - 1;
            u32UsedLen = 176;
        }
        else
        {
            u8AdaptationLength = TS_PACKET_LENGTH - 4 - (u8)u32PesLen -1;
            u32UsedLen = u32PesLen ;
        }

        //写ts头
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_AF_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x10;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + u8AdaptationLength + 1 + 4, pu8PesBuf, u32UsedLen);
    }
    else if (u32PesLen >= 184)//???????? maybe 183 ????????
    {
        u32UsedLen = 184;

        //写ts头
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = 0;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x00;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + 4, pu8PesBuf, u32UsedLen);
    }
    else
    {
        u32UsedLen = u32PesLen;
        u8AdaptationLength = TS_PACKET_LENGTH - 4 - (u8)u32PesLen - 1;//计算自适应区长度

        //写ts头
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_AF_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x00;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + 4 + 1 + u8AdaptationLength, pu8PesBuf, u32UsedLen);
    }

    return u32UsedLen;
}

/*=================================================================================
函数名:TsStreamSetProgram
功能:重设流
算法实现:
参数说明:
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 TsWriteSetProgram(TTsWrite *ptTsInfo, u8 u8VideoType, u8 u8AudioType)
{
    u16 wRet;
    TTsProgramInfo *ptNowProg;
    TTsPatPrograms tPatProgs;

    TTsPatInfo *pTTsPatInfo = NULL;
    TTsPmtInfo *pTTsPmtInfo = NULL;

    if (NULL == ptTsInfo)
    {
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

    pTTsPatInfo = &ptTsInfo->tPatInfo;
    pTTsPmtInfo = ptTsInfo->ptPmtInfo;

    u8VideoType = TsPsPTCovertRtp2Stream(u8VideoType);
    u8AudioType = TsPsPTCovertRtp2Stream(u8AudioType);

    //必须设置其中一个流
    if (VIDEO_STREAM != TsPsGetStreamIdPrefix(u8VideoType) &&
        AUDIO_STREAM0 != TsPsGetStreamIdPrefix(u8AudioType))
    {
        return ERROR_TS_WRITE_STREAM_TYPE;
    }

    if (VIDEO_STREAM == TsPsGetStreamIdPrefix(u8VideoType))
    {
        ptTsInfo->u8VideoType = u8VideoType;
    }
    else
    {
        ptTsInfo->u8VideoType = PT_STREAM_TYPE_NULL;
    }

    if (AUDIO_STREAM0 == TsPsGetStreamIdPrefix(u8AudioType))
    {
        ptTsInfo->u8AudioType = u8AudioType;
    }
    else
    {
        ptTsInfo->u8AudioType = PT_STREAM_TYPE_NULL;
    }

    memset(&tPatProgs, 0, sizeof(TTsPatPrograms));
    ptNowProg = &tPatProgs.atProgram[0];

    //只加一个节目，最多两个流，一个音频一个视频
    tPatProgs.u8ProgramNum = 1;

    //如果有视频流，先放第一个
    if (ptTsInfo->u8VideoType != PT_STREAM_TYPE_NULL)
    {
        ptNowProg->u8StreamNum = 1;
        ptNowProg->au8StreamType[0] = ptTsInfo->u8VideoType;
        ptNowProg->au16StreamPid[0] = TsWriteGetExclusivePID(ptTsInfo);

        wRet = TsWriteSetPat(ptTsInfo, &tPatProgs);
        if (TSPS_OK != wRet)
        {
            return wRet;
        }
    }

    //如果有音频流
    if (ptTsInfo->u8AudioType != PT_STREAM_TYPE_NULL)
    {
        if (ptTsInfo->u8VideoType == PT_STREAM_TYPE_NULL)
        {
            //前面无视频流，那就放第一个
            ptNowProg->u8StreamNum = 1;
            ptNowProg->au8StreamType[0] = ptTsInfo->u8AudioType;
            ptNowProg->au16StreamPid[0] = TsWriteGetExclusivePID(ptTsInfo);
        }
        else
        {
            //前面有视频流，放第二个
            ptNowProg->u8StreamNum = 2;
            ptNowProg->au8StreamType[1] = ptTsInfo->u8AudioType;
            ptNowProg->au16StreamPid[1] = TsWriteGetExclusivePID(ptTsInfo);
        }

    }

    wRet = TsWriteSetPat(ptTsInfo, &tPatProgs);
    if (TSPS_OK == wRet)
    {
        ptTsInfo->u32TsPesCount = 0;
    }

    return wRet;
}

u16 TsWriteSegmentParam(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment)
{
    if (NULL == ptTsInfo || NULL == ptTsSegment)
    {
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

	ptTsInfo->ptTsSegment = ptTsSegment;

	// 生成一个空的索引文件，ligeng@2013.2.28
	WriteIndexFile( ptTsSegment->aau8IndexFileName, ptTsSegment->u32SegmentDuration, \
		ptTsSegment->aau8OutputFilePrefix, ptTsSegment->aau8HttpPrefix, ptTsInfo->pszUrlBuf, ptTsInfo->szFirstIV, ptTsSegment->u32FirstSegment, ptTsSegment->u32LastSegment,\
				FALSE, ptTsSegment->u32SegmentWindow );

	return TSPS_OK;
}

// 写m3u8索引文件，ligeng@2012.10.10
int WriteIndexFile(const s8 aszIndexFile[], /*const u8 au8TmpIndexFile[], */const u32 u32SegmentDuration, \
				   const s8 au8OutputPrefix[], const s8 au8HttpPrefix[], s8* psz8Url, s8* IV, const u32 u32FirstSegment, \
				   const u32 u32LastSegment, BOOL bEnd, const u32 nWindow)
{
    FILE *fpIndex;
    s8 *pWriteBuf;
    s32 i;
	s8 au8TmpIndexFile[TS_SEGMENT_FILE_LEN];	// 临时索引文件，形式 .IndexFile
	s8 au8HttpOutPutFile[TS_SEGMENT_FILE_LEN];	// http url后面的索引文件名称，去掉索引目录

	u32 nFlag = 0;

	strcpy( au8TmpIndexFile, aszIndexFile );

	for( i = strlen( aszIndexFile ) - 1; i >=0 ; i-- )
	{
		au8TmpIndexFile[ i+1 ] = aszIndexFile[ i ];
		if( aszIndexFile[i] == '/' )
		{
			nFlag = i;
			break;
		}
	}

	if (0 == nFlag)
	{
		au8TmpIndexFile[nFlag] = '.';
	}
	else
	{
		au8TmpIndexFile[ nFlag + 1] = '.';
	}

	au8TmpIndexFile[ strlen( aszIndexFile ) + 1] = '\0';

	for( i = strlen( aszIndexFile ) - 1; i >=0 ; i-- )
	{
		if( au8OutputPrefix[ i ] == '/' )
		{
			nFlag = i;
			break;
		}
	}
	// 如果有 / ，需要把/和之前的字符都去掉，没有，原字符串拷贝
	nFlag = nFlag ? nFlag + 1 : nFlag;
	strcpy( au8HttpOutPutFile, au8OutputPrefix + nFlag );

    fpIndex = fopen(au8TmpIndexFile, "w");
    if ( !fpIndex )
	{
        TspsPrintf( PD_TS_WRITE, "Could not open temporary m3u8 index file (%s), no index file will be created\n", au8TmpIndexFile);
        return -1;
    }

    pWriteBuf = (s8 *)malloc(sizeof(u8) * 1024);
    if ( !pWriteBuf )
	{
        TspsPrintf( PD_TS_WRITE, "Could not allocate write buffer for index file, index file will be invalid\n");
        fclose(fpIndex);
        return -1;
    }

    if ( nWindow )
	{
		sprintf(pWriteBuf, "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-ALLOW-CACHE:YES\n#EXT-X-TARGETDURATION:%u\n#EXT-X-MEDIA-SEQUENCE:%u\n", u32SegmentDuration, u32FirstSegment);
    }
    else
	{
		sprintf(pWriteBuf, "#EXTM3U\n#EXT-X-TARGETDURATION:%u\n", u32SegmentDuration);
    }
    if (fwrite(pWriteBuf, strlen(pWriteBuf), 1, fpIndex) != 1)
	{
        TspsPrintf( PD_TS_WRITE, "Could not write to m3u8 index file, will not continue writing to index file\n");
        free(pWriteBuf);
        fclose(fpIndex);
        return -1;
    }

	if (psz8Url)
	{
		if (IV)
		{
			if (TRUE == CheckIfZero(IV))
			{
				sprintf(pWriteBuf, "#EXT-X-KEY:METHOD=AES-128,URI=\"%s\"\n", psz8Url);
			}
			else
			{
				sprintf(pWriteBuf, "#EXT-X-KEY:METHOD=AES-128,URI=\"%s\",IV=0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n",
				psz8Url, IV[0], IV[1], IV[2], IV[3], IV[4], IV[5], IV[6], IV[7], IV[8], IV[9], IV[10], IV[11], IV[12], IV[13], IV[14], IV[15], IV[16]);
			}
		}
		else
		{
			sprintf(pWriteBuf, "#EXT-X-KEY:METHOD=AES-128,URI=\"%s\"\n", psz8Url);
		}
		if (fwrite(pWriteBuf, strlen(pWriteBuf), 1, fpIndex) != 1)
		{
            TspsPrintf( PD_TS_WRITE, "Could not write to m3u8 index file, will not continue writing to index file\n");
            free(pWriteBuf);
            fclose(fpIndex);
            return -1;
        }
	}

	for (i = u32FirstSegment; i < u32LastSegment && u32LastSegment != 1; i++)
	{
		sprintf(pWriteBuf, "#EXTINF:%u,\n%s%s-%u.ts\n", u32SegmentDuration, au8HttpPrefix, au8HttpOutPutFile, i);
        if (fwrite(pWriteBuf, strlen(pWriteBuf), 1, fpIndex) != 1)
		{
            TspsPrintf( PD_TS_WRITE, "Could not write to m3u8 index file, will not continue writing to index file\n");
            free(pWriteBuf);
            fclose(fpIndex);
            return -1;
        }
    }

    if (bEnd)
	{
		sprintf(pWriteBuf, "#EXT-X-ENDLIST\n");
        if (fwrite(pWriteBuf, strlen(pWriteBuf), 1, fpIndex) != 1)
		{
            TspsPrintf( PD_TS_WRITE, "Could not write last file and endlist tag to m3u8 index file\n");
            free(pWriteBuf);
            fclose(fpIndex);
            return -1;
        }
    }

    free(pWriteBuf);
    fclose(fpIndex);

    return rename(au8TmpIndexFile, aszIndexFile);
}


/*=================================================================================
函数名:TSProgramWriteEsToTs
功能:将一个ES包进行TS打包
算法实现: 只能写一对音频和视频
返回值说明:成功返回TSPS_OK,否则返回错误代码
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 TsWriteWriteEsFrame(TTsWrite *ptTsInfo, TspsFRAMEHDR *ptFrame)
{
    u16 u16Ret = TSPS_OK;

    u8 u8StreamPos = 0;
    s32 s32Usedlength = 0;
    s32 s32Len = 0;
    s32 s32i = 0;

    u8 *pu8Buf;
    u32 u32Len;
    u8 u8StreamType;

    TPesInfo *ptPesInfo;
    TTsPatInfo *pTTsPatInfo = NULL;
	TTsPmtInfo *pTTsPmtInfo = NULL;

	TTsSegment *ptTsSegment = NULL;

	BOOL bSegment = FALSE;		// 是否切片
	BOOL bWriteIndex = FALSE;	// 是否写索引文件
	s8 au8SegmentFile[TS_SEGMENT_FILE_LEN];
	s8 au8RemoveFile[TS_SEGMENT_FILE_LEN];
	BOOL bRemoveFile = FALSE;	// 是否需要删除切片文件

	u8 abyEncryptOut[AES_ENCRYPT_BYTENUM] = {0};
	u16 wLen = 0;
	u16 wOffSet = 0;
	s32 nRet = 0;

    if (NULL == ptTsInfo || NULL == ptFrame)
    {
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

	bSegment = ptTsInfo->ptTsSegment ? TRUE : FALSE;
	ptTsSegment = ptTsInfo->ptTsSegment;

    ptPesInfo = ptTsInfo->ptPesInfo;
    pTTsPatInfo = &ptTsInfo->tPatInfo;
    pTTsPmtInfo = ptTsInfo->ptPmtInfo;

    pu8Buf = ptFrame->m_pbyData;
    u32Len = ptFrame->m_dwDataSize;
    u8StreamType = TsPsPTCovertRtp2Stream(ptFrame->m_byMediaType);

    if (NULL == pu8Buf || /*PES_PACKET_MAX_LENGTH*/ES_PACEKT_MAX_LENGTH < u32Len
        || PT_STREAM_TYPE_NULL == u8StreamType)
    {
        return ERROR_TS_WRITE_INPUT_FRAME;
    }

    if (ptTsInfo->u8AudioType != u8StreamType &&
        ptTsInfo->u8VideoType != u8StreamType)
    {
        return ERROR_TS_WRITE_STREAM_TYPE;
    }

	OspSemTake( ptTsInfo->hWriteSem );

	// 依照视频时间进行切割，音频忽略不计
	if( u8StreamType == PT_STREAM_TYPE_MPEG4 || u8StreamType == PT_STREAM_TYPE_MP2 || u8StreamType == PT_STREAM_TYPE_H264 )
	{
		// 第一帧，打开文件
		if( bSegment && ptTsSegment->u32LastSegmentTime == 0 )
		{
			// +1不影响时间计算，防止时间戳为0，影响第一帧判断
			ptTsSegment->u32LastSegmentTime = ptFrame->m_dwTimeStamp + 1;

			bWriteIndex = TRUE;
		}
		// 达到切片时间，必须是关键帧
		else if( ptFrame->x.m_tVideoParam.m_bKeyFrame && bSegment &&
			( ptFrame->m_dwTimeStamp - ptTsSegment->u32LastSegmentTime ) / 90000 >= ptTsSegment->u32SegmentDuration )
		{
			bWriteIndex = TRUE;
			// 赋值时间节点为当前时间戳
			ptTsSegment->u32LastSegmentTime = ptFrame->m_dwTimeStamp;
			// 关闭上次打开的文件
			if( ptTsSegment->fpSegment != NULL )
			{
				if (0 != ptTsInfo->wKeySize)
				{
					TsWriteEncryptBuffer(ptTsInfo, ptTsSegment, NULL, 0);
				}
				else
				{
					fclose(ptTsSegment->fpSegment);
					ptTsSegment->fpSegment = NULL;
				}
			}

			// 超过窗口大小，删除窗口前一个文件
			if( ptTsSegment->u32SegmentWindow && ptTsSegment->u32LastSegment - ptTsSegment->u32FirstSegment >= ptTsSegment->u32SegmentWindow )
			{
				sprintf( au8RemoveFile, "%s-%d.ts", ptTsSegment->aau8OutputFilePrefix, ptTsSegment->u32FirstSegment);
				remove( au8RemoveFile );
				ptTsSegment->u32FirstSegment++;
			}
		}
	}

	// aac 音频需要加adts头，此处理应该放在库外部处理，保证读写文件是能正确处理
// 	if( MEDIA_TYPE_AACLC == ptFrame->m_byMediaType )
// 	{
//
// 		BitsInit(&tBitsBuf, au8AudioBuf, 8);
//
// 		BitsWrite8(&tBitsBuf, 8, 0xff);
// 		BitsWrite8(&tBitsBuf, 8, 0xfb);
// 		BitsWrite8(&tBitsBuf, 2, 1);
// 		BitsWrite8(&tBitsBuf, 4, 5);		// 采用率固定为32k
// 		BitsWrite8(&tBitsBuf, 1, 0);
//
// 		BitsWrite8(&tBitsBuf, 3, 1);
// 		BitsWrite8(&tBitsBuf, 2, 0);
//
// 		BitsWrite8(&tBitsBuf, 2, 0);
// 		// 需要去掉帧头4字节sdp
// 		BitsWrite16(&tBitsBuf, 13, (ptFrame->m_dwDataSize+7));  //frame_length;
// 		BitsWrite16(&tBitsBuf, 11, 0x7ff);
//         BitsWrite8(&tBitsBuf, 2, 0);
//
// 		memcpy(au8AudioBuf+7, ptFrame->m_pbyData+7, ptFrame->m_dwDataSize-7);
//
// 		ptFrame->m_pbyData = au8AudioBuf;
// 		ptFrame->m_dwDataSize;
// 	}
//
// 	pu8Buf = ptFrame->m_pbyData;

	if( bWriteIndex )
	{
		sprintf( au8SegmentFile, "%s-%d.ts",  ptTsSegment->aau8OutputFilePrefix, ptTsSegment->u32LastSegment );
		ptTsSegment->fpSegment = fopen( au8SegmentFile, "wb+" );
		if( ptTsSegment->fpSegment == NULL)
		{
			TspsPrintf( PD_TS_WRITE, "[TsWriteWriteEsFrame]open file %s fail!", au8SegmentFile );
			OspSemGive( ptTsInfo->hWriteSem );
			return ERROR_TS_SEGMENT_FILE_ERROR;
		}
		memcpy(ptTsInfo->szIV, ptTsInfo->szFirstIV, 16);

		// 第一个切片时索引文件为空，ligeng@2013.2.28
		if( ptTsSegment->u32LastSegment > 0 )
		{
			WriteIndexFile( ptTsSegment->aau8IndexFileName, ptTsSegment->u32SegmentDuration, \
				ptTsSegment->aau8OutputFilePrefix, ptTsSegment->aau8HttpPrefix, ptTsInfo->pszUrlBuf, ptTsInfo->szFirstIV, ptTsSegment->u32FirstSegment, ptTsSegment->u32LastSegment,\
				FALSE, ptTsSegment->u32SegmentWindow );
		}
		ptTsSegment->u32LastSegment++;

	}

    //隔XX个包发送psi信息
    if (0 == ptTsInfo->u32TsPesCount % TS_SEND_PSI_STEP)
    {
        TsWritePatAddContinuityCounter(ptTsInfo);
        TsWritePmtAddContinuityCounter(ptTsInfo);
        if(ptTsInfo->pfCallback)
        {
            ptTsInfo->pfCallback(pTTsPatInfo->pu8Buffer, TS_PACKET_LENGTH,
                                 (KD_PTR)ptTsInfo->pvContext, ptFrame);

			if( bSegment && ptTsSegment->fpSegment != NULL )
			{
				if (0 != ptTsInfo->wKeySize)
				{
					TsWriteEncryptBuffer(ptTsInfo, ptTsSegment, pTTsPatInfo->pu8Buffer, TS_PACKET_LENGTH);
				}
				else
				{
					fwrite(pTTsPatInfo->pu8Buffer, sizeof(u8), TS_PACKET_LENGTH, ptTsSegment->fpSegment);
				}
			}

            for(s32i = 0; s32i < pTTsPatInfo->u8ProgramMapNum; s32i++)
            {
                ptTsInfo->pfCallback(pTTsPmtInfo[s32i].pu8Buffer, TS_PACKET_LENGTH,
                                     (KD_PTR)ptTsInfo->pvContext, ptFrame);

				if( bSegment && ptTsSegment->fpSegment != NULL )
				{
					if (0 != ptTsInfo->wKeySize)
					{
						TsWriteEncryptBuffer(ptTsInfo, ptTsSegment, pTTsPmtInfo[s32i].pu8Buffer, TS_PACKET_LENGTH);
					}
					else
					{
						fwrite( pTTsPmtInfo[s32i].pu8Buffer, sizeof(u8), TS_PACKET_LENGTH, ptTsSegment->fpSegment );
					}
				}
            }
        }
    }

    ptTsInfo->u32TsPesCount++;

    //取得该条流位置
    u8StreamPos = (pTTsPmtInfo[0].au8StreamType[0] == u8StreamType)? 0 : 1;

    //记录时间戳
    ptTsInfo->u64PCRBase = ptFrame->m_dwTimeStamp;

    //打pes包
    memset(ptPesInfo, 0, sizeof(TPesInfo));

    //表示该流同时具有DTS和PTS
    ptPesInfo->u8PtsDtsFlag = 0x03;

    ptPesInfo->u64Pts = ptFrame->m_dwTimeStamp;
    ptPesInfo->u64Dts = ptFrame->m_dwTimeStamp;
    ptPesInfo->u8StreamId = TsPsGetStreamIdPrefix(u8StreamType);
    ptPesInfo->pu8PayloadBuffer = pu8Buf;
    ptPesInfo->u32PayloadLength = u32Len;

    ptTsInfo->u32Peslen = MAX_VIDEO_FRAME_LEN;

    u16Ret = PesWriteInfo(ptPesInfo, ptTsInfo->pu8PesBuf, &ptTsInfo->u32Peslen);
    if(TSPS_OK != u16Ret)
    {
		OspSemGive( ptTsInfo->hWriteSem );
        return u16Ret;
    }

    //将PES包分成多个188字节的TS包
    while (0 < ptPesInfo->u32PesLength)
    {
        TsWriteSetPayload(ptTsInfo, s32Usedlength, 0, u8StreamPos);

        s32Len = TsWriteWriteTsPack(ptTsInfo, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        s32Usedlength += s32Len;

        ptPesInfo->pu8PesBuffer += s32Len;
        ptPesInfo->u32PesLength -= s32Len;

        //回调TS包
        if (ptTsInfo->pfCallback)
        {
            ptTsInfo->pfCallback(ptTsInfo->pu8TsBuf, TS_PACKET_LENGTH,
                                 (KD_PTR)ptTsInfo->pvContext, ptFrame);
        }

		//写切片文件
		if( bSegment && ptTsSegment->fpSegment != NULL )
		{
			if (0 != ptTsInfo->wKeySize)
			{
				TsWriteEncryptBuffer(ptTsInfo, ptTsSegment, ptTsInfo->pu8TsBuf, TS_PACKET_LENGTH);
			}
			else
			{
				fwrite(ptTsInfo->pu8TsBuf, sizeof(u8), TS_PACKET_LENGTH, ptTsSegment->fpSegment);
			}
		}
    }

	OspSemGive( ptTsInfo->hWriteSem );

    //成功在过程里打印一下
    TspsPrintf(PD_TS_WRITE, "TsWrite write a frame successfully. <len=%d>", u32Len);

    return TSPS_OK;
}

u16 TsWriteSetEncryptKey(TTsWrite *ptTsInfo, s8 *pszKeyBuf, s8* pszIV,  u16 wKeySize,s8 *pszUrlBuf, u16 wUrlLen)
{
	if (NULL == ptTsInfo || (0 != wKeySize && NULL == pszKeyBuf) || (0 != wUrlLen && NULL == pszUrlBuf))
    {
        return ERROR_TS_WRITE_INPUT_PARAM;
    }

	if (0 != wKeySize)
	{
		if( (AES_ENCRYPT_KEY_SIZE_MODE_A != wKeySize) &&
			(AES_ENCRYPT_KEY_SIZE_MODE_B != wKeySize) &&
			(AES_ENCRYPT_KEY_SIZE_MODE_C != wKeySize) )
		{
			return ERROR_TS_WRITE_SET_ENCRYPTKEY;
		}
		if (NULL == ptTsInfo->pszKeyBuf)
		{
			ptTsInfo->pszKeyBuf = (s8*)malloc(wKeySize);
			if (NULL == ptTsInfo->pszKeyBuf)
			{
				return ERROR_TS_WRITE_MEMORY;
			}
			memcpy(ptTsInfo->pszKeyBuf, pszKeyBuf, wKeySize);
			ptTsInfo->wKeySize = wKeySize;
		}
	}

	if (NULL != pszIV)
	{
		memcpy(ptTsInfo->szIV, pszIV, 16);
		memcpy(ptTsInfo->szFirstIV, pszIV, 16);
	}

	if (0 != wUrlLen)
	{
		if (NULL == ptTsInfo->pszUrlBuf)
		{
			ptTsInfo->pszUrlBuf = (s8*)malloc(wUrlLen);
			if (NULL == ptTsInfo->pszUrlBuf)
			{
				return ERROR_TS_WRITE_MEMORY;
			}
			sprintf(ptTsInfo->pszUrlBuf, "%s", pszUrlBuf);
			ptTsInfo->wUrlLen = wUrlLen;
		}
	}

	return TSPS_OK;
}

u16 TsWriteEncryptBuffer(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment, u8* pbyBuffer, u32 dwBuffLen)
{
	return TSPS_OK;
}

BOOL32 CheckIfZero(s8 *IV)
{
	BOOL32 bRet = TRUE;
	u8 byLoop = 0;

	if (NULL == IV)
	{
		return FALSE;
	}

	for (byLoop = 0; byLoop < 16; byLoop++)
	{
		if (IV[byLoop] != 0)
		{
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}