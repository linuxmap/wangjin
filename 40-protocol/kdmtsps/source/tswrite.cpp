/*=================================================================================
ģ����:TS����д
�ļ���:ts.cpp
����ļ�:ts.h
ʵ�ֹ���:TS����д
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
������:TsProgramOpen
����:��TS���,�����ڴ�ռ�
�㷨ʵ��:
����˵��:
         [I/O] ptpTSInfo �洢TS����Ϣ�Ľṹ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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

    //������Ӧ�ڴ�
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

		// lg update��test
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
������:TsProgramClose
����:�ر�TS���,�ͷ��ڴ�ռ�
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
		// ɾ��������Ƭts�ļ�
		for( u32Index = ptTsInfo->ptTsSegment->u32FirstSegment; u32Index <= ptTsInfo->ptTsSegment->u32LastSegment; u32Index++ )
		{
			sprintf( au8SegmentFile, "%s-%d.ts", ptTsInfo->ptTsSegment->aau8OutputFilePrefix, u32Index );
			remove( au8SegmentFile );
			memset( au8SegmentFile, 0, sizeof( au8SegmentFile ) );
		}
		// ɾ��m3u8�����ļ�
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
������:TSProgramGetExclusivePID
����:�õ�0x0010 - 0x1FFE��Ψһ�����
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
����ֵ˵��:����Ψһ��ʶ
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWriteGetExclusivePID(TTsWrite *ptTSInfo)
{
    u16 u16Count = 0;
    u32 u32Tick = OspTickGet();
    u16 u16PID = 0;
    s32 s32i = 0;
    BOOL32 bFlag = FALSE;

    srand(u32Tick);

    //ѭ��8192��
    while(u16Count < TS_RAND_MAX)
    {
        u16Count ++;
        bFlag = FALSE;
        u16PID = (rand() % (8190 - 10)) + 10;

        //��鵱ǰ������Ƿ��Ѿ��ù�
        for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM; s32i++)
        {
            if(u16PID == ptTSInfo->au16Pid[s32i])
            {
                bFlag = TRUE;
            }
        }

        //��ǰ�����û�б�ʹ�ù�
        if(FALSE == bFlag)
        {
            for(s32i = 0; s32i < MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM; s32i++)
            {
                //��¼��������,��ֹ�´�ʹ��
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
������:TsWriteWriteHeader
����:дtsͷ
�㷨ʵ��:
����˵��:
         [I]   u16elementaryPID ��PID
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWriteSetHeader(TTsWrite *ptTSInfo, TBit *ptBitsBuf)
{
    s32 s32i;
    TTsHeader *ptHead = &ptTSInfo->tHeader;
    u8 u8SubLen = 0;

    //д4�ֽ�TS��ͷ
    BitsWrite8(ptBitsBuf, 8, TS_PACKET_SYNC);//0x47
    BitsWrite8(ptBitsBuf, 1, ptHead->u8TransportErrorIndicator);
    BitsWrite8(ptBitsBuf, 1, ptHead->u8PayloadUnitStartIndicator);
    BitsWrite8(ptBitsBuf, 1, ptHead->u8TransportPriority);
    BitsWrite16(ptBitsBuf, 13, ptHead->u16Pid);//дPMT��PID
    BitsWrite8(ptBitsBuf, 2, ptHead->u8TransportScramblingControl);
    BitsWrite8(ptBitsBuf, 2, ptHead->u8AdaptationFieldControl);//Adaptation_field followed by payload
    BitsWrite8(ptBitsBuf, 4, ptHead->u8ContinuityCounter);

    //����Ӧ�ֶ�(��䳤�ȣ���־λ)
    BitsWrite8(ptBitsBuf, 8, ptHead->u8AdaptationFieldLength);
    if (ptHead->u8AdaptationFieldLength > 0)
    {
        BitsWrite8(ptBitsBuf, 8, ptHead->u8AdaptationFlags);
    }

    if (0x10 == ptHead->u8AdaptationFlags)
    {
        //дPCR
        BitsWrite64(ptBitsBuf, 33, ptTSInfo->u64PCRBase);//ʱ�ӻ�׼
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
������:TSProgramAddPMTStream
����:����һ·������
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   u16ProgramIndex ��PMT����λ��
         [I]   u8StreamType ������
         [I]   u16elementaryPID ��PID
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWriteSetPmt(TTsWrite *ptTSInfo, u16 u16ProgramIndex, TTsProgramInfo *ptInfo)
{
    u8 u8AdapLength = 0;
    u8 u8SectionLength = 0;
    s32 s32i = 0;
    //��õ�CRCֵ
    u32 u32Crc32 = 0;
    //����CRC�Ŀ�ʼλ��
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

    //��дPMT��
    for (s32i=0; s32i<ptInfo->u8StreamNum; s32i++)
    {
        ptPMT->au8StreamType[s32i] = ptInfo->au8StreamType[s32i];
        ptPMT[u16ProgramIndex].au16ElementaryPid[s32i] = ptInfo->au16StreamPid[s32i];
        ptPMT[u16ProgramIndex].u8StreamNum = ptInfo->u8StreamNum;
        ptPMT[u16ProgramIndex].u8VersionNumber++;
    }

    //����PMT�ֶγ���
    u8SectionLength = 9 + 5 * ptPMT[u16ProgramIndex].u8StreamNum + 4;

    //��������ֶγ���
    u8AdapLength = TS_PACKET_LENGTH - 4 - (u8SectionLength + 4) -1;

    BitsInit(&tBitsBuf, ptPMT[u16ProgramIndex].pu8Buffer, TS_PACKET_LENGTH);

    //дtsͷ
    memset(&ptTSInfo->tHeader, 0, sizeof(TTsHeader));
    ptTSInfo->tHeader.u8PayloadUnitStartIndicator = 1;
    ptTSInfo->tHeader.u16Pid = ptPAT->au16ProgramMapPid[u16ProgramIndex];
    ptTSInfo->tHeader.u8AdaptationFieldControl = 3;
    ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdapLength;
    TsWriteSetHeader(ptTSInfo, &tBitsBuf);

    //дPMT��Ϣ
    BitsWrite8(&tBitsBuf, 8, 0);//pointer_field
    BitsWrite8(&tBitsBuf, 8, 2);//table_id
    BitsWrite8(&tBitsBuf, 1, 1);//section_syntax_indicator
    BitsWrite8(&tBitsBuf, 1, 0);//set to '0'
    BitsWrite8(&tBitsBuf, 2, 3);//reserved
    BitsWrite16(&tBitsBuf, 12, u8SectionLength);//�γ���
    BitsWrite16(&tBitsBuf, 16, (u16)(u16ProgramIndex + 1));//��Ŀ��
    BitsWrite8(&tBitsBuf, 2, 3);//reserved
    BitsWrite8(&tBitsBuf, 5, ptPMT[u16ProgramIndex].u8VersionNumber);//�汾��
    BitsWrite8(&tBitsBuf, 1, 1);//current_nect_indicator
    BitsWrite8(&tBitsBuf, 8, 0);//section_number
    BitsWrite8(&tBitsBuf, 8, 0);//last_section_number
    BitsWrite8(&tBitsBuf, 3, 7);//reserved
    BitsWrite16(&tBitsBuf, 13, ptPMT[u16ProgramIndex].au16ElementaryPid[0]);//PCR��׼��PID
    BitsWrite8(&tBitsBuf, 4, 15);//reserved
    BitsWrite16(&tBitsBuf, 12, 0);//program_info_length

    //д��������Ϣ
    for(s32i = 0; s32i < ptPMT[u16ProgramIndex].u8StreamNum; s32i ++)
    {
        BitsWrite8(&tBitsBuf, 8, ptPMT[u16ProgramIndex].au8StreamType[s32i]);
        BitsWrite8(&tBitsBuf, 3, 7);
        BitsWrite16(&tBitsBuf, 13, ptPMT[u16ProgramIndex].au16ElementaryPid[s32i]);
        BitsWrite8(&tBitsBuf, 4, 15);
        BitsWrite16(&tBitsBuf, 12, 0);
    }

    //дCRC
    //CRCУ��Ŀ�ʼλ���Ǵ�PAT��TABLE_ID�ֶο�ʼ��
    u32CrcStart = 4 +1 + u8AdapLength + 1;
    u32Crc32 = CRCGetCRC32(ptPMT[u16ProgramIndex].pu8Buffer + u32CrcStart, TS_PACKET_LENGTH - u32CrcStart - 4);
    BitsWrite32(&tBitsBuf, 32, u32Crc32);

    return TSPS_OK;
}

/*=================================================================================
������:TsProgramSetParam
����:��ʼ��һ·TS����Ϣ
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I/O] ptProgramInfo TS����Ϣ
         [I]   u8ProgramNum TS��·��(����ʱʹ��)
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWriteSetPat(TTsWrite *ptTsInfo, TTsPatPrograms *ptPrograms)
{
    //�����ֶγ���
    u8 u8AdaptationLength = 0;
    //��õ�CRCֵ
    u32 u32Crc32 = 0;
    //����CRC�Ŀ�ʼλ��
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

    //���PATδ��ʼ���������PAT��PID
    if(0x00 == ptPAT->u8ProgramMapNum)
    {
        //��д��λ��PID
        ptPAT->u16TransportStreamId = TsWriteGetExclusivePID(ptTsInfo);
        ptPAT->u8VersionNumber ++;
        ptPAT->u8CurrentNextIndicator = 0x01;
        ptPAT->u8SectionNumber = 0;
        ptPAT->u8LastSectionNumber = 0;
    }

    //�����趨��Ŀ��
    ptPAT->u8ProgramMapNum = ptPrograms->u8ProgramNum;

    //��дÿ����Ŀ��Ϣ
    for(s32i = 0; s32i < ptPrograms->u8ProgramNum; s32i++)
    {
        ptPAT->au16ProgramNumber[s32i] = s32i + 1;
        //���֮ǰ��pid������ʹ���µ�
        if (0x0000 == ptPAT->au16ProgramMapPid[s32i])
        {
            ptPAT->au16ProgramMapPid[s32i] = TsWriteGetExclusivePID(ptTsInfo);
        }
    }

    //����ֶγ���,�˷ֶγ�����ָÿ����Ŀ�Ŷ�Ӧ��Ŀ��Ŀӳ���Ĵ�С*��Ŀ��(��Ŀ��*4�ֽ�)
    //+�ֶγ��Ⱥ�,������ӳ����Ϣ�ĳ���(5�ֽ�)+CRCУ��(4�ֽ�) from section_length
    ptPAT->u16SectionLength = 4 * ptPAT->u8ProgramMapNum + 5 + 4;

    //��������Ӧ���ֶγ���,������188-PAT��ռ�Ŀռ�(��point_field)(bySectionLength + 4) - 4�ֽ�TS��ͷ - 1�ֽ����ڵ����ֶ���д������
    u8AdaptationLength = TS_PACKET_LENGTH - 4 - (ptPAT->u16SectionLength + 4)- 1;
    if(0 > u8AdaptationLength)
    {
        u8AdaptationLength = 0;
    }

    BitsInit(&tBitsBuf, ptPAT->pu8Buffer, TS_PACKET_LENGTH);

    //дtsͷ
    memset(&ptTsInfo->tHeader, 0, sizeof(TTsHeader));
    ptTsInfo->tHeader.u8PayloadUnitStartIndicator = 1;
    ptTsInfo->tHeader.u16Pid = TS_PAT_TABLE_ID;
    ptTsInfo->tHeader.u8AdaptationFieldControl = 3;
    ptTsInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
    TsWriteSetHeader(ptTsInfo, &tBitsBuf);

    //дPAT��
    BitsWrite8(&tBitsBuf, 8, 0);//ָ��
    BitsWrite8(&tBitsBuf, 8, 0);//��ID
    BitsWrite8(&tBitsBuf, 1, 1);//���﷨���
    BitsWrite8(&tBitsBuf, 1, 0);
    BitsWrite8(&tBitsBuf, 2, 3);
    BitsWrite16(&tBitsBuf, 12, ptPAT->u16SectionLength);//�γ���
    BitsWrite16(&tBitsBuf, 16, ptPAT->u16TransportStreamId);//ΨһPID��ts��PID
    BitsWrite8(&tBitsBuf, 2, 3);
    BitsWrite8(&tBitsBuf, 5, ptPAT->u8VersionNumber);//�汾��
    BitsWrite8(&tBitsBuf, 1, ptPAT->u8CurrentNextIndicator);//��ǰ����Ч
    BitsWrite8(&tBitsBuf, 8, ptPAT->u8SectionNumber);//��ǰ�κ�
    BitsWrite8(&tBitsBuf, 8, ptPAT->u8LastSectionNumber);//�ϴεĶκ�

    //д��PMT��PID��Ϣ
    for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i++)
    {
        //PMT��
        BitsWrite16(&tBitsBuf, 16, ptPAT->au16ProgramNumber[s32i]);
        BitsWrite8(&tBitsBuf, 3, 7);
        //PMT��PID
        BitsWrite16(&tBitsBuf, 13, ptPAT->au16ProgramMapPid[s32i]);
    }

    //дCRC
    //CRCУ��Ŀ�ʼλ���Ǵ�PAT��TABLE_ID�ֶο�ʼ��
    u32CrcStart = 4 + 1 + u8AdaptationLength + 1;
    u32Crc32 = CRCGetCRC32(ptPAT->pu8Buffer + u32CrcStart, TS_PACKET_LENGTH - u32CrcStart - 4);
    BitsWrite32(&tBitsBuf, 32, u32Crc32);

    for(s32i = 0; s32i < ptPrograms->u8ProgramNum; s32i++)
    {
        //����������ID,����PMT
        s32 s32sn;
        for(s32sn = 0; s32sn < ptPrograms->atProgram[s32i].u8StreamNum; s32sn++)
        {
            TsWriteSetPmt(ptTsInfo, (u16)s32i, &ptPrograms->atProgram[s32i]);
        }
    }

    return TSPS_OK;
}

/*=================================================================================
������:TSProgramPATAddContinuityCounter
����:����PAT���ļ���
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWritePatAddContinuityCounter(TTsWrite *ptTSInfo)
{
    TBit tBitsBuf;

    TTsPatInfo *ptPAT = &ptTSInfo->tPatInfo;

    //ֱ������������,�޸�ֵ
    BitsInit(&tBitsBuf, ptPAT->pu8Buffer, TS_PACKET_LENGTH);
    BitsSkip(&tBitsBuf, 28);
    ptPAT->u8ContinuityCounter = BitsRead8(&tBitsBuf, 4);
    ptPAT->u8ContinuityCounter++;

    //�����ŵ���0xf,���0
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
������:TSProgramPMTAddContinuityCounter
����:����PMT���ļ���
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWritePmtAddContinuityCounter(TTsWrite *ptTSInfo)
{
    TBit tBitsBuf;

    TTsPatInfo *ptPAT = &ptTSInfo->tPatInfo;
    TTsPmtInfo *ptPMT = ptTSInfo->ptPmtInfo;

    s32 s32i = 0;

    for(s32i = 0; s32i < ptPAT->u8ProgramMapNum; s32i ++)
    {
        //ֱ������������,�޸�ֵ
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
������:TSProgramWriteTsPayload
����:����PID,�Ƿ���һ��PES�����ֽڣ���ʼ����ǰTS����Ϣ
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I] byWritePos �Ƿ���һ��PES�����ֽ�
         [I] u16PID ��·����PID
         [I] bool32IsRealTime �Ƿ���ʵʱ����
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsWriteSetPayload(TTsWrite *ptTSInfo, s32 s32WritePos, u8 u8PmtPos, u8 u8StreamPos)
{
    s32 s32i = 0;

    //�޸ļ���
    ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos]++;
    if(0xf < ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos])
    {
        ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos] = 0;
    }

    memset(&ptTSInfo->tHeader, 0, sizeof(TTsHeader));

    ptTSInfo->tHeader.u16Pid = ptTSInfo->ptPmtInfo[u8PmtPos].au16ElementaryPid[u8StreamPos];
    ptTSInfo->tHeader.u8ContinuityCounter = ptTSInfo->aau8ContinuityCounter[u8PmtPos][u8StreamPos];

    //PES��һ����payload_unit_start_indicatorΪ1
    //ͬʱд��·��PCRֵ
    //PCR�Ǹ���PTS�õ���
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
������:TSProgramWriteTsPack
����:����TS�ṹ��Ϣ,дTS������
�㷨ʵ��:
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   pu8PesBuf PES������
         [I]   s32PesLen PES������
         [O]   pu8Buf �洢TS���Ļ���
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u32 TsWriteWriteTsPack(TTsWrite *ptTSInfo, u8 *pu8PesBuf, u32 u32PesLen)
{
    u32 u32UsedLen = 0;
    u8  u8AdaptationLength = 0;
    TBit tBitsBuf;
    u8 *pu8Buf = ptTSInfo->pu8TsBuf;

    BitsInit(&tBitsBuf, pu8Buf, TS_PACKET_LENGTH);

    //��ǰ������һ֡�ĵ�һ���ֽ�,����PCR��׼��
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

        //дtsͷ
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_AF_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x10;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + u8AdaptationLength + 1 + 4, pu8PesBuf, u32UsedLen);
    }
    else if (u32PesLen >= 184)//???????? maybe 183 ????????
    {
        u32UsedLen = 184;

        //дtsͷ
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = 0;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x00;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + 4, pu8PesBuf, u32UsedLen);
    }
    else
    {
        u32UsedLen = u32PesLen;
        u8AdaptationLength = TS_PACKET_LENGTH - 4 - (u8)u32PesLen - 1;//��������Ӧ������

        //дtsͷ
        ptTSInfo->tHeader.u8AdaptationFieldControl = TS_AFC_TYPE_AF_PAYLOAD;
        ptTSInfo->tHeader.u8AdaptationFieldLength = u8AdaptationLength;
        ptTSInfo->tHeader.u8AdaptationFlags = 0x00;
        TsWriteSetHeader(ptTSInfo, &tBitsBuf);

        memcpy(pu8Buf + 4 + 1 + u8AdaptationLength, pu8PesBuf, u32UsedLen);
    }

    return u32UsedLen;
}

/*=================================================================================
������:TsStreamSetProgram
����:������
�㷨ʵ��:
����˵��:
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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

    //������������һ����
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

    //ֻ��һ����Ŀ�������������һ����Ƶһ����Ƶ
    tPatProgs.u8ProgramNum = 1;

    //�������Ƶ�����ȷŵ�һ��
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

    //�������Ƶ��
    if (ptTsInfo->u8AudioType != PT_STREAM_TYPE_NULL)
    {
        if (ptTsInfo->u8VideoType == PT_STREAM_TYPE_NULL)
        {
            //ǰ������Ƶ�����Ǿͷŵ�һ��
            ptNowProg->u8StreamNum = 1;
            ptNowProg->au8StreamType[0] = ptTsInfo->u8AudioType;
            ptNowProg->au16StreamPid[0] = TsWriteGetExclusivePID(ptTsInfo);
        }
        else
        {
            //ǰ������Ƶ�����ŵڶ���
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

	// ����һ���յ������ļ���ligeng@2013.2.28
	WriteIndexFile( ptTsSegment->aau8IndexFileName, ptTsSegment->u32SegmentDuration, \
		ptTsSegment->aau8OutputFilePrefix, ptTsSegment->aau8HttpPrefix, ptTsInfo->pszUrlBuf, ptTsInfo->szFirstIV, ptTsSegment->u32FirstSegment, ptTsSegment->u32LastSegment,\
				FALSE, ptTsSegment->u32SegmentWindow );

	return TSPS_OK;
}

// дm3u8�����ļ���ligeng@2012.10.10
int WriteIndexFile(const s8 aszIndexFile[], /*const u8 au8TmpIndexFile[], */const u32 u32SegmentDuration, \
				   const s8 au8OutputPrefix[], const s8 au8HttpPrefix[], s8* psz8Url, s8* IV, const u32 u32FirstSegment, \
				   const u32 u32LastSegment, BOOL bEnd, const u32 nWindow)
{
    FILE *fpIndex;
    s8 *pWriteBuf;
    s32 i;
	s8 au8TmpIndexFile[TS_SEGMENT_FILE_LEN];	// ��ʱ�����ļ�����ʽ .IndexFile
	s8 au8HttpOutPutFile[TS_SEGMENT_FILE_LEN];	// http url����������ļ����ƣ�ȥ������Ŀ¼

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
	// ����� / ����Ҫ��/��֮ǰ���ַ���ȥ����û�У�ԭ�ַ�������
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
������:TSProgramWriteEsToTs
����:��һ��ES������TS���
�㷨ʵ��: ֻ��дһ����Ƶ����Ƶ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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

	BOOL bSegment = FALSE;		// �Ƿ���Ƭ
	BOOL bWriteIndex = FALSE;	// �Ƿ�д�����ļ�
	s8 au8SegmentFile[TS_SEGMENT_FILE_LEN];
	s8 au8RemoveFile[TS_SEGMENT_FILE_LEN];
	BOOL bRemoveFile = FALSE;	// �Ƿ���Ҫɾ����Ƭ�ļ�

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

	// ������Ƶʱ������и��Ƶ���Բ���
	if( u8StreamType == PT_STREAM_TYPE_MPEG4 || u8StreamType == PT_STREAM_TYPE_MP2 || u8StreamType == PT_STREAM_TYPE_H264 )
	{
		// ��һ֡�����ļ�
		if( bSegment && ptTsSegment->u32LastSegmentTime == 0 )
		{
			// +1��Ӱ��ʱ����㣬��ֹʱ���Ϊ0��Ӱ���һ֡�ж�
			ptTsSegment->u32LastSegmentTime = ptFrame->m_dwTimeStamp + 1;

			bWriteIndex = TRUE;
		}
		// �ﵽ��Ƭʱ�䣬�����ǹؼ�֡
		else if( ptFrame->x.m_tVideoParam.m_bKeyFrame && bSegment &&
			( ptFrame->m_dwTimeStamp - ptTsSegment->u32LastSegmentTime ) / 90000 >= ptTsSegment->u32SegmentDuration )
		{
			bWriteIndex = TRUE;
			// ��ֵʱ��ڵ�Ϊ��ǰʱ���
			ptTsSegment->u32LastSegmentTime = ptFrame->m_dwTimeStamp;
			// �ر��ϴδ򿪵��ļ�
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

			// �������ڴ�С��ɾ������ǰһ���ļ�
			if( ptTsSegment->u32SegmentWindow && ptTsSegment->u32LastSegment - ptTsSegment->u32FirstSegment >= ptTsSegment->u32SegmentWindow )
			{
				sprintf( au8RemoveFile, "%s-%d.ts", ptTsSegment->aau8OutputFilePrefix, ptTsSegment->u32FirstSegment);
				remove( au8RemoveFile );
				ptTsSegment->u32FirstSegment++;
			}
		}
	}

	// aac ��Ƶ��Ҫ��adtsͷ���˴���Ӧ�÷��ڿ��ⲿ������֤��д�ļ�������ȷ����
// 	if( MEDIA_TYPE_AACLC == ptFrame->m_byMediaType )
// 	{
//
// 		BitsInit(&tBitsBuf, au8AudioBuf, 8);
//
// 		BitsWrite8(&tBitsBuf, 8, 0xff);
// 		BitsWrite8(&tBitsBuf, 8, 0xfb);
// 		BitsWrite8(&tBitsBuf, 2, 1);
// 		BitsWrite8(&tBitsBuf, 4, 5);		// �����ʹ̶�Ϊ32k
// 		BitsWrite8(&tBitsBuf, 1, 0);
//
// 		BitsWrite8(&tBitsBuf, 3, 1);
// 		BitsWrite8(&tBitsBuf, 2, 0);
//
// 		BitsWrite8(&tBitsBuf, 2, 0);
// 		// ��Ҫȥ��֡ͷ4�ֽ�sdp
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

		// ��һ����Ƭʱ�����ļ�Ϊ�գ�ligeng@2013.2.28
		if( ptTsSegment->u32LastSegment > 0 )
		{
			WriteIndexFile( ptTsSegment->aau8IndexFileName, ptTsSegment->u32SegmentDuration, \
				ptTsSegment->aau8OutputFilePrefix, ptTsSegment->aau8HttpPrefix, ptTsInfo->pszUrlBuf, ptTsInfo->szFirstIV, ptTsSegment->u32FirstSegment, ptTsSegment->u32LastSegment,\
				FALSE, ptTsSegment->u32SegmentWindow );
		}
		ptTsSegment->u32LastSegment++;

	}

    //��XX��������psi��Ϣ
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

    //ȡ�ø�����λ��
    u8StreamPos = (pTTsPmtInfo[0].au8StreamType[0] == u8StreamType)? 0 : 1;

    //��¼ʱ���
    ptTsInfo->u64PCRBase = ptFrame->m_dwTimeStamp;

    //��pes��
    memset(ptPesInfo, 0, sizeof(TPesInfo));

    //��ʾ����ͬʱ����DTS��PTS
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

    //��PES���ֳɶ��188�ֽڵ�TS��
    while (0 < ptPesInfo->u32PesLength)
    {
        TsWriteSetPayload(ptTsInfo, s32Usedlength, 0, u8StreamPos);

        s32Len = TsWriteWriteTsPack(ptTsInfo, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        s32Usedlength += s32Len;

        ptPesInfo->pu8PesBuffer += s32Len;
        ptPesInfo->u32PesLength -= s32Len;

        //�ص�TS��
        if (ptTsInfo->pfCallback)
        {
            ptTsInfo->pfCallback(ptTsInfo->pu8TsBuf, TS_PACKET_LENGTH,
                                 (KD_PTR)ptTsInfo->pvContext, ptFrame);
        }

		//д��Ƭ�ļ�
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

    //�ɹ��ڹ������ӡһ��
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