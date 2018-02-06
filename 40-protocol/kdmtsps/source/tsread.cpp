/*=================================================================================
ģ����:TS���Ķ�
�ļ���:ts.cpp
����ļ�:ts.h
ʵ�ֹ���:TS���Ķ�
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
#include "videopredec.h"

TTsRead *TsReadOpen(TspsFrameCallback pfCallback, void *pvContext)
{
    s32 s32i = 0;
    
    TTsRead *ptTsInfo = NULL;
    TTsPatInfo *ptPAT = NULL;
    TTsPmtInfo *ptPMT = NULL;
    BOOL32 bFail = FALSE;
	u8 u8Loop = 0;
    
    //������Ӧ�ڴ� 
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

//��tsͷ
static u16 TsReadGetHeader(TTsHeader *ptHead, u8 *pu8Buf, u32 u32Len)
{
    TBit tBitsBuf;

    BitsInit(&tBitsBuf, pu8Buf, u32Len);

    //����ts��ʼ����(0x47)(8)
    BitsSkip(&tBitsBuf, 8);
    
    //�����������
    ptHead->u8TransportErrorIndicator = BitsRead8(&tBitsBuf, 1);
    //��unit start indicator
    ptHead->u8PayloadUnitStartIndicator = BitsRead8(&tBitsBuf, 1);
    //�����ȼ�
    ptHead->u8TransportPriority = BitsRead8(&tBitsBuf, 1);
    //pid
    ptHead->u16Pid = BitsRead16(&tBitsBuf, 13);
    //���ܿ���
    ptHead->u8TransportScramblingControl = BitsRead8(&tBitsBuf, 2);
    //������Ӧ�ֶα��
    ptHead->u8AdaptationFieldControl = BitsRead8(&tBitsBuf, 2);
    //������
    ptHead->u8ContinuityCounter = BitsRead8(&tBitsBuf, 4);
    
    //�ж��Ƿ�������Ӧ�ֶ�,�����,����������
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
������:TSProgramPATReadInfo
����:��TS����ȡPAT��Ϣ
�㷨ʵ��: 
����˵��:
         [I/O] ptpPATInfo �洢PAT����Ϣ�Ľṹ
         [I]   pu8Buf TS��
         [I]   u32Len ����Ϊ188
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
    
    //�Ƚ���tsͷ
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    if (0x01 != ptHead->u8PayloadUnitStartIndicator)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat without UnitStart bit.");
        return ERROR_TS_READ_UNIT_START;
    }

    //���������
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

    //��ȡpoint_field
    u8Offset = pu8Buf[ptHead->u8HeadLen];

    //���pat����
    pu8PatBuf = pu8Buf + (ptHead->u8HeadLen + 1 + u8Offset);
    u8PatLen = TS_PACKET_LENGTH - (ptHead->u8HeadLen + 1 + u8Offset);
    if (u8PatLen < 12)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat pack incorrect length[%d].", u8PatLen);
        return ERROR_TS_READ_PSI_LENGTH;
    }

    BitsInit(&tBitsBuf, pu8PatBuf, u8PatLen);

    /*****������PAT������*****/
    //��TABLE ID
    u8Temp = BitsRead8(&tBitsBuf, 8);
    if(TS_PAT_TABLE_ID != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat table id wrong.");
        return ERROR_TS_READ_TABLE_ID;
    }
    
    //�����﷨���
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x01 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat SECTION_SYNTAX wrong.");
        return ERROR_TS_READ_SECTION_SYNTAX;
    }
    
    //���̶�0���
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x00 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat FIX_0 wrong.");
        return ERROR_TS_READ_FIX_0;
    }
    
    //��������λ(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPATInfo->u16SectionLength = BitsRead16(&tBitsBuf, 12);
    ptpPATInfo->u16TransportStreamId = BitsRead16(&tBitsBuf, 16);
    
    //��������λ(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPATInfo->u8VersionNumber = BitsRead8(&tBitsBuf, 5);
    ptpPATInfo->u8CurrentNextIndicator = BitsRead8(&tBitsBuf, 1);
    ptpPATInfo->u8SectionNumber = BitsRead8(&tBitsBuf, 8);
    ptpPATInfo->u8LastSectionNumber = BitsRead8(&tBitsBuf, 8);
    
    //����ʣ���ֽ�,Ӧ��ȥ��crc�ֶ�(4byte)
    u8ByRemain = ptpPATInfo->u16SectionLength - (40 / 8) - 4;
    u8Temp = ptpPATInfo->u8ProgramMapNum;
    
    //��PAT����Ϣ
    ptpPATInfo->u8ProgramMapNum = 0;
    for(s32i = u8Temp; s32i < ((u8ByRemain / (32 / 8)) + u8Temp); s32i++)
    {
        ptpPATInfo->au16ProgramNumber[s32i] = BitsRead16(&tBitsBuf, 16);
        //��������λ(3)
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
������:TSProgramPMTReadInfo
����:��TS����ȡPMT��Ϣ
�㷨ʵ��: 
����˵��:
         [I/O] ptpPMTInfo �洢PMT����Ϣ�Ľṹ
         [I]   pu8Buf TS��
         [I]   u32Len ����Ϊ188
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
    
    //�Ƚ���tsͷ
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    //��unit start indicator,Ϊ0x01��ʾ������point field�ֶ�
    if(0x01 != ptHead->u8PayloadUnitStartIndicator)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt without UnitStart bit.");
        return ERROR_TS_READ_UNIT_START;
    }

    //���������
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

    //��POINT FIELD,��������
    u8Offset = pu8Buf[ptHead->u8HeadLen];
    
    //���pmt����
    pu8PmtBuf = pu8Buf + (ptHead->u8HeadLen + 1 + u8Offset);
    u8PmtLen = TS_PACKET_LENGTH - (ptHead->u8HeadLen + 1 + u8Offset);
    if (u8PmtLen < 16)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt pack incorrect length[%d].", u8PmtLen);
        return ERROR_TS_READ_PSI_LENGTH;
    }

    
    /*****������PMT������*****/
    BitsInit(&tBitsBuf, pu8PmtBuf, u8PmtLen);

    //��TABLE ID
    u8Temp = BitsRead8(&tBitsBuf, 8);
    if(TS_PMT_TABLE_ID != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt table id wrong.");
        return ERROR_TS_READ_TABLE_ID;
    }
    
    //�����﷨���
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x01 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pmt SECTION_SYNTAX wrong.");
        return ERROR_TS_READ_SECTION_SYNTAX;
    }
    
    //���̶�0���
    u8Temp = BitsRead8(&tBitsBuf, 1);
    if(0x00 != u8Temp)
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: pat FIX_0 wrong.");
        return ERROR_TS_READ_FIX_0;
    }
    
    //��������λ(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPMTInfo->u16SectionLength = BitsRead16(&tBitsBuf, 12);
    ptpPMTInfo->u16ProgramNumber = BitsRead16(&tBitsBuf, 16);
    
    //��������λ(2)
    BitsSkip(&tBitsBuf, 2);
    ptpPMTInfo->u8VersionNumber = BitsRead8(&tBitsBuf, 5);
    ptpPMTInfo->u8CurrentNextIndicator = BitsRead8(&tBitsBuf, 1);
    ptpPMTInfo->u8SectionNumber = BitsRead8(&tBitsBuf, 8);
    ptpPMTInfo->u8LastSectionNumber = BitsRead8(&tBitsBuf, 8);
    
    //��������λ(3)
    BitsSkip(&tBitsBuf, 3);
    ptpPMTInfo->u16PcrPid = BitsRead16(&tBitsBuf, 13);
    
    //��������λ(4)
    BitsSkip(&tBitsBuf, 4);
    ptpPMTInfo->u16ProgramInfoLength = BitsRead16(&tBitsBuf, 12);
    BitsSkip(&tBitsBuf, ptpPMTInfo->u16ProgramInfoLength * 8);
    
    //����ʣ���ֽ�,Ӧ��ȥ��crc�ֶ�(4byte)
    u8ByRemain = ptpPMTInfo->u16SectionLength - 9 - ptpPMTInfo->u16ProgramInfoLength - 4;
    
    ptpPMTInfo->u8StreamNum = 0;
    while (u8ByRemain > 0)
    {
        ptpPMTInfo->au8StreamType[ptpPMTInfo->u8StreamNum] = BitsRead8(&tBitsBuf, 8);
        
        //��������λ(3)
        BitsSkip(&tBitsBuf, 3);
        ptpPMTInfo->au16ElementaryPid[ptpPMTInfo->u8StreamNum] = BitsRead16(&tBitsBuf, 13);
        
        //��������λ(4)
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
������:TSProgramIsPesPacket
����:�жϵ�ǰTS���Ƿ���PMT���еĺϷ���
�㷨ʵ��: 
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   u16PID TS����PID
         [O]   pu8StreamType �ð�����
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش������
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 TsReadIsPesPacket(TTsRead *ptTsInfo, u16 u16PID, u8 *pu8StreamType)
{
    TTsPatInfo *ptPAT = &ptTsInfo->tPatInfo;
    TTsPmtInfo *ptPMT = ptTsInfo->ptPmtInfo;

    s32 s32i = 0, s32j = 0;

    //���pmt��֧�־Ͳ������ֱ���յ���һ����֧�ֵ�pmt
    if (FALSE == ptTsInfo->bSupport) 
    {
        TspsPrintf(PD_TS_READ, "TsRead fail: streams not support.");
        return ERROR_TS_READ_NOT_SUPPORT;
    }

    //ѭ��PAT��,PMT������Ƿ��е�ǰPID����
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
������: ������������
����: ��һ����ts section���ۻ��õ�pes������������һ֡es��
�㷨ʵ��: 
����˵��:
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻�Ψһ�����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
// ��pes�еĵ�һ��ts����ȡ��pes_packet_length
static u16 TsReadGetPesLenFromFirstSection(u8 *pu8Buf)
{
    return (pu8Buf[4] << 8) | pu8Buf[5];
}

//��װ�õĻص�һ֡�ĺ���
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
        //��pesʧ�ܣ����������ڶ�������
        TspsPrintf(PD_TS_READ, "TsRead: pes decode failed. may be caused by fragmentized pes pack.");
    }

	ptFrame->m_dwFrameID = ptTsInfo->atTsInfo[u8Id].u32FrameID++;

    return u16Ret;
}

//��һ����ts section���ۻ��õ�pes��������
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
        //δ��¼����������£��ҵȵ�����һ��pes���ص���һ֡
		if (ptTsInfo->atTsInfo[u8Id].bWaitNextPes)
		{
			u16Ret = TsReadCallback(ptTsInfo, ptTsInfo->atTsInfo[u8Id].u8LastType);
			
			//��pes�������㣬����һ��ʹ�ã��˾䲻��ȥ�����Է��¸�pes��ʧ�˵�һ��
			ptTsInfo->atTsInfo[u8Id].u32Peslen = 0;
        }
        
        //��Ҫ����һ֡
		ptTsInfo->atTsInfo[u8Id].bWaitNextPes = TRUE;

        //��ȡpes����
        u16PesPackLen = TsReadGetPesLenFromFirstSection(pu8Buf);
		ptTsInfo->atTsInfo[u8Id].u16CurPesLen = PES_DATA_HEAD_LENGTH + u16PesPackLen;

        //����µ�����
		memcpy(ptTsInfo->atTsInfo[u8Id].pu8PesBuf, pu8Buf, u32Len);
		ptTsInfo->atTsInfo[u8Id].u32Peslen = u32Len;      
    }
    else
    {
		if (FALSE == ptTsInfo->atTsInfo[u8Id].bWaitNextPes)
        {
            //�����һ֡�ѻص�������һ֡������UnitStart��ͷ����ô˵�����˵�һ��
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

    //��¼����������£��ж�pes�����Ƿ��㹻�����У�ת����es�����ص�һ֡
    //������ΰ�����¼����ȷ�����ص�������һ���������ж�
	if (ptTsInfo->atTsInfo[u8Id].u32Peslen >= ptTsInfo->atTsInfo[u8Id].u16CurPesLen && 
		ptTsInfo->atTsInfo[u8Id].u16CurPesLen > PES_DATA_HEAD_LENGTH)
    {
        u16Ret = TsReadCallback(ptTsInfo, u8StreamType);

        //��pes�������㣬����һ��ʹ�ã��˾䲻��ȥ�����Է��¸�pes��ʧ�˵�һ��
		ptTsInfo->atTsInfo[u8Id].u32Peslen = 0;

        //�ѻص�������Ҫ����һ֡
		ptTsInfo->atTsInfo[u8Id].bWaitNextPes = FALSE;
	}   
    
    return u16Ret;
}

/*=================================================================================
������:TSProgramPayloadReadInfo
����:����һ��TS��
�㷨ʵ��: 
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   pu8Buf TS��
         [I]   u32Len ����Ϊ188
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻�Ψһ�����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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

    //�Ƚ���tsͷ
    TsReadGetHeader(ptHead, pu8Buf, u32Len);

    //�жϴ�PID�ڳ���ӳ������Ƿ����,�������,˵����TS���Ϸ�
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
������:TSProgramParsePacket
����:�ж�TS������,������Ӧ�Ľ���
�㷨ʵ��: 
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   pu8Buf TS��
         [I]   u32Len ����Ϊ188
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻�Ψһ�����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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

    //������ͷ
    BitsSkip(&tBitsBuf, 11);
    //���PID
    u16PID = BitsRead16(&tBitsBuf, 13);

    //����PID�ж�TS��������
    if (TS_PAT_TABLE_ID == u16PID)  //pat
    {
        u16Ret = TsReadGetPat(ptTsInfo, ptPAT, pu8Buf, u32Len);
        if (TSPS_OK == u16Ret)
        {
            if (ptPAT->u8ProgramMapNum != 1)
            {
                //�������pat��Ŀ����Ϊ1����ô��֧�֣�ֱ����һ��֧�ֵ�pat
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
                //��Ŀ��Ϊ1���ɽ�
                ptTsInfo->bSupport = TRUE;
            }
        }

        return u16Ret;
    }
	else if (TS_NULL_PACKET_ID == u16PID)//�հ�
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
            //�ж��Ƿ���PMT�������û��pat��pat����ʧ�ܣ�pid��������ȷ
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
                        //ֻ�����ı��˲Żص�
                        ptTsInfo->pfProgramCB(u8VPT, u8APT, MEDIA_TYPE_NULL, (KD_PTR)ptTsInfo->pvProgramContext);
                    }
                }

                return u16Ret;
            }
        }
    }

    //������ͨTS��
    u16Ret = TsReadGetPayload(ptTsInfo, pu8Buf, u32Len);
    
    return u16Ret;
}

/*=================================================================================
������:TSProgramParseBuffer
����:����һ��TS��
�㷨ʵ��: 
����˵��:
         [I/O] ptTSInfo �洢TS����Ϣ�Ľṹ
         [I]   pu8Buf TS��
         [I]   u16Len ������
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
		//���ܸ������������¼����ƫ��λ��
		ptTsInfo->bFirstPack = TRUE;
		ptTsInfo->dwTempBufLen = 0;
        return ERROR_TS_READ_HEAD_SYNC;
    }

    //����ֽ��㹻188�����������һ��
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
