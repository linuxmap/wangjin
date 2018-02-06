/*=================================================================================
ģ����:PS����д
�ļ���:ps.cpp
����ļ�:ps.h
ʵ�ֹ���:PS����д
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

/*=================================================================================
������:PSProgramOpen
����:��PS���,�����ڴ�ռ�
�㷨ʵ��:
����˵��:
         [I/O] ptpPSInfo PS���ṹ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
TPsWrite *PsWriteOpen(TspsSectionCallback pfCallback, void *pvContext, u32 dwMaxFrameSize)
{
    TPsWrite *ptPSInfo = NULL;
    BOOL32 bFail = FALSE;

    //������Ӧ�ռ�
    do 
    {
        //����ṹ��ռ�
        ptPSInfo = (TPsWrite *)malloc(sizeof(TPsWrite));
        if (NULL == ptPSInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo, 0, sizeof(TPsWrite));

	//get max frame size
	if(dwMaxFrameSize < MAX_VIDEO_FRAME_LEN)
		dwMaxFrameSize = MAX_VIDEO_FRAME_LEN;

	//record 
	ptPSInfo->dwmaxframesize = dwMaxFrameSize;
        //����ps ֡����
        ptPSInfo->pu8Buf = (u8 *)malloc(ptPSInfo->dwmaxframesize);
        if (NULL == ptPSInfo->pu8Buf)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->pu8Buf, 0, ptPSInfo->dwmaxframesize);
        
        //����pes��Ϣ�ṹ��ռ�
        ptPSInfo->ptPesInfo = (TPesInfo *)malloc(sizeof(TPesInfo));
        if (NULL == ptPSInfo->ptPesInfo)
        {
            bFail = TRUE;
            break;
        }
        memset(ptPSInfo->ptPesInfo, 0, sizeof(TPesInfo));

        //���ûص�
        ptPSInfo->pfCallback = pfCallback;
        ptPSInfo->pvContext = pvContext;
        
    } while (0);

    if (bFail)
    {
        TspsPrintf(PD_PS_WRITE, "PsReadOpen fail: memory malloc error.");
        PsWriteClose(ptPSInfo);
        ptPSInfo = NULL;
    }

    return ptPSInfo;
}

/*=================================================================================
������:PSProgramClose
����:�ر�PS���,�ͷ��ڴ�ռ�
�㷨ʵ��:
����˵��:
         [I/O] ptpPSInfo PS���ṹ
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PsWriteClose(TPsWrite *ptPsInfo)
{
    if (NULL == ptPsInfo)
    {
        return ERROR_PS_WRITE_INPUT_PARAM;
    }

    TSPSFREE(ptPsInfo->pu8Buf);

    TSPSFREE(ptPsInfo->ptPesInfo);

    TSPSFREE(ptPsInfo);

    return TSPS_OK;
}


/*=================================================================================
������:PSProgramSetParam
����:��TS��������һ·ָ��������(��Ƶ/��Ƶ������)��������(����ı����ʽ)����
�㷨ʵ��:
����˵��:
         [I/O] ptpPSInfo PS���ṹ
         [I]   u8StreamID ������
         [I]   u8StreamType ������
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 PsWriteSetPsm(TPsWrite *ptPSInfo, TPsProgramInfo *ptPsProgramInfo)
{
    s32 s32i = 0;
    TPsMapInfo *ptMap = NULL;

    ptMap = &ptPSInfo->tMap;
    ptMap->u8Version = 0;
    
    //���PS��������Ŀ�Ƿ�ﵽ���
    if (ptPsProgramInfo->u8StreamNum == 0 ||
        ptPsProgramInfo->u8StreamNum > MAX_STREAM_NUM_IN_PROGRAM)
    {
        TspsPrintf(PD_PS_WRITE, "PsWrite fail: stream number[%d] incorrect.", 
            ptPsProgramInfo->u8StreamNum);
        return ERROR_PS_WRITE_STREAM_NUM;
    }

    //������Ϣд��ӳ���
    for (s32i=0; s32i < ptPsProgramInfo->u8StreamNum; s32i++)
    {
        ptMap->au8StreamId[s32i] = ptPsProgramInfo->au8StreamId[s32i];
        ptMap->au8StreamType[s32i] = ptPsProgramInfo->au8StreamType[s32i];
    }
    
    ptMap->u8StreamNum = ptPsProgramInfo->u8StreamNum;

    return TSPS_OK;
}

/*=================================================================================
������:PsWriteSetProgram
����:������Ƶ��Ƶ��
�㷨ʵ��:
����˵��:
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PsWriteSetProgram(TPsWrite *ptPsInfo, u8 u8VideoType, u8 u8AudioType)
{
    u16 u16Ret = TSPS_OK;
    TPsMapInfo *ptMap = NULL;
    TPsProgramInfo tProgInfo;
    u8   u8AudioTransType;
	u8   u8VideoTransType;
    if (NULL == ptPsInfo)
    {
        return ERROR_PS_WRITE_INPUT_PARAM;
    }
    
    ptMap = &ptPsInfo->tMap;
    memset(&tProgInfo, 0, sizeof(TPsProgramInfo));

    //��ת��һ��
    u8AudioTransType = TsPsPTCovertRtp2Stream(u8AudioType);
    u8VideoTransType = TsPsPTCovertRtp2Stream(u8VideoType);

	if ((u8AudioType != MEDIA_TYPE_NULL && u8AudioTransType == PT_STREAM_TYPE_NULL) ||
		(u8VideoType != MEDIA_TYPE_NULL && u8VideoTransType == PT_STREAM_TYPE_NULL))
	{
		return ERROR_PS_WRITE_INPUT_PARAM; 
	}

    if (u8VideoTransType != PT_STREAM_TYPE_NULL)
    {
        tProgInfo.au8StreamType[tProgInfo.u8StreamNum] = u8VideoTransType;
        tProgInfo.au8StreamId[tProgInfo.u8StreamNum] = TsPsGetStreamIdPrefix(u8VideoTransType);
        tProgInfo.u8StreamNum++;
    }

    if (u8AudioTransType != PT_STREAM_TYPE_NULL)
    {
        tProgInfo.au8StreamType[tProgInfo.u8StreamNum] = u8AudioTransType;
        tProgInfo.au8StreamId[tProgInfo.u8StreamNum] = TsPsGetStreamIdPrefix(u8AudioTransType);
        tProgInfo.u8StreamNum++;
    }

    u16Ret = PsWriteSetPsm(ptPsInfo, &tProgInfo);
    if (TSPS_OK == u16Ret)
    {
        ptPsInfo->u8AudioType = u8AudioTransType;
        ptPsInfo->u8VideoType = u8VideoTransType;
    }

    return u16Ret;
}

/*=================================================================================
������:PSProgramWritePsMap
����:дӳ����Ϣ
�㷨ʵ��:
����˵��:
         [I/O] ptpPSInfo PS���ṹ
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 PsWriteWritePsm(TPsWrite *ptPSInfo)
{
    s32 s32i = 0;

    TBit tBitsBuf;
    u32 u32Crc32;
    u8 *pu8WriteStart = ptPSInfo->pu8Buf + ptPSInfo->u32Len;
    s32 s32BuffLen = PS_PACKET_LENGTH - ptPSInfo->u32Len;

    TPsMapInfo *ptMap = &ptPSInfo->tMap;

    ptMap->u8Version++;
    ptMap->u8CurrentNextIndicator = 1;
    //����γ���
    ptMap->u16EsMapLength = 4 * ptMap->u8StreamNum;
    ptMap->u16MapLength = 6 + ptMap->u16EsMapLength + 4;//4B CRC

    //�����λд��
    BitsInit(&tBitsBuf, pu8WriteStart, s32BuffLen);

    BitsWrite32(&tBitsBuf, 32, MAP_START_CODE);//��ʼ��000001BC
    BitsWrite16(&tBitsBuf, 16, ptMap->u16MapLength);//�γ���
    BitsWrite8(&tBitsBuf,  1, ptMap->u8CurrentNextIndicator);//��ǰ�ο��ñ�־
    BitsWrite8(&tBitsBuf,  2, 3);            
    BitsWrite8(&tBitsBuf,  5, ptMap->u8Version);//�汾��
    BitsWrite8(&tBitsBuf,  7, 127);
    BitsWrite8(&tBitsBuf,  1, 1);    
    BitsWrite16(&tBitsBuf, 16, 0);    
    BitsWrite16(&tBitsBuf, 16, ptMap->u16EsMapLength);

    //д������Ϣ
    for(s32i = 0; s32i < ptMap->u8StreamNum; s32i++)
    {
        BitsWrite8(&tBitsBuf, 8, ptMap->au8StreamType[s32i]);
        BitsWrite8(&tBitsBuf, 8, ptMap->au8StreamId[s32i]);
        BitsWrite16(&tBitsBuf, 16, 0);                        
    }

    ptMap->pu8Buffer = pu8WriteStart;
    ptMap->u32BuffLength = 6 + ptMap->u16MapLength;

    ptPSInfo->u32Len += ptMap->u32BuffLength;

    //дCRC
    u32Crc32 = CRCGetCRC32(ptMap->pu8Buffer, ptMap->u32BuffLength);
    BitsWrite32(&tBitsBuf, 32, u32Crc32);
    
    return TSPS_OK;
}

/*=================================================================================
������:PSProgramWritePsSysHead
����:дPSϵͳͷ��Ϣ
�㷨ʵ��:
����˵��:
        [I/O] ptpPSInfo PS���ṹ
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 PsWriteWriteSysHead(TPsWrite *ptPSInfo)
{
    s32 s32i = 0;

    TBit tBitsBuf;
    u8 *pu8WriteStart = ptPSInfo->pu8Buf + ptPSInfo->u32Len;
    s32 s32BuffLen = PS_PACKET_LENGTH - ptPSInfo->u32Len;

    TPsSysHeaderInfo *ptSysHead = &ptPSInfo->tSysHead;

    //����ϵͳ�γ���
    ptSysHead->u16HeaderLength = 6 + 3 * ptSysHead->u8StreamNum;

    BitsInit(&tBitsBuf, pu8WriteStart, s32BuffLen);

    BitsWrite32(&tBitsBuf, 32, SYSTEM_HEADER_START_CODE);//��ʼ���000001BB
    BitsWrite16(&tBitsBuf, 16, ptSysHead->u16HeaderLength);//����

    BitsWrite8(&tBitsBuf, 1, 1);//marker bit
    BitsWrite32(&tBitsBuf, 22, ptSysHead->u32RateBound);
    BitsWrite8(&tBitsBuf, 1, 1);//marker bit
    BitsWrite8(&tBitsBuf, 6, ptSysHead->u8AudioBound);
    BitsWrite8(&tBitsBuf, 1, ptSysHead->u8FixedFlag);
    BitsWrite8(&tBitsBuf, 1, ptSysHead->u8CspsFlag);
    BitsWrite8(&tBitsBuf, 1, ptSysHead->u8SystemAudioLockFlag);
    BitsWrite8(&tBitsBuf, 1, ptSysHead->u8SystemVideoLockFlag);
    BitsWrite8(&tBitsBuf, 1, 1);//marker bit
    BitsWrite8(&tBitsBuf, 5, ptSysHead->u8VideoBound);
    BitsWrite8(&tBitsBuf, 1, ptSysHead->u8PacketRateRestrictionFlag);
    BitsWrite8(&tBitsBuf, 7, 127);//reserved

    for (s32i=0; s32i<ptSysHead->u8StreamNum; s32i++)
    {
        BitsWrite8(&tBitsBuf, 8, ptSysHead->au8StreamId[s32i]);
        BitsWrite8(&tBitsBuf, 2, 3);
        BitsWrite8(&tBitsBuf, 1, ptSysHead->au8PStdBufferBoundScale[s32i]);
        BitsWrite16(&tBitsBuf, 13, ptSysHead->au16PStdBufferSizeBound[s32i]);
    }

    ptSysHead->pu8Buffer = pu8WriteStart;
    ptSysHead->u32BuffLength = 6 + ptSysHead->u16HeaderLength;

    ptPSInfo->u32Len += ptSysHead->u32BuffLength;

    return TSPS_OK;
}

/*=================================================================================
������:PSProgramWritePSHead
����:дPSͷ��Ϣ
�㷨ʵ��:
����˵��:
         [I/O]ptpPSInfo PS���ṹ
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 PsWriteWriteHead(TPsWrite *ptPSInfo)
{
    TBit tBitsBuf;

    TPsHeadInfo *ptHead = &ptPSInfo->tHead;

    //�ӻ����ʼ��дpsͷ
    u8 *pu8WriteStart = ptPSInfo->pu8Buf;
    s32 s32BuffLen = PS_PACKET_LENGTH;

    BitsInit(&tBitsBuf, pu8WriteStart, s32BuffLen);
	//psͷ
    BitsWrite32(&tBitsBuf, 32, PACK_HEADER_START_CODE);//��ʼ���000001BA
    BitsWrite8(&tBitsBuf,  2, 1);// 01   
    BitsWrite64(&tBitsBuf, 3, ptHead->u64SCRBase >> 30);//SCRʼ����Ϣ
    BitsWrite8(&tBitsBuf,  1, 1);//mark
    BitsWrite64(&tBitsBuf,  15, ptHead->u64SCRBase >> 15);
    BitsWrite8(&tBitsBuf,  1, 1);//mark
    BitsWrite64(&tBitsBuf, 15, ptHead->u64SCRBase);
    BitsWrite8(&tBitsBuf,  1, 1);//mark
    BitsWrite16(&tBitsBuf, 9, ptHead->u16SCRExt);//system_clock_reference_base_extension
    BitsWrite8(&tBitsBuf,  1, 1);//mark

    BitsWrite32(&tBitsBuf, 22, ptHead->u32ProgramMuxRate);//��Ŀ��������
    BitsWrite8(&tBitsBuf,  7, 127);//2��mark�Լ�5λ��reserved
    BitsWrite8(&tBitsBuf,  3, 0); //pack_stuffing_length   
	//����pack_stuffing_length����0������stuffing_byte�Ͳ�������

    ptHead->pu8Buffer = pu8WriteStart;
    ptHead->u32BuffLength = 14;

    ptPSInfo->u32Len = ptHead->u32BuffLength;

    return TSPS_OK;
}


/*=================================================================================
������:PsWriteSetSysHead
����:����ϵͳͷ
�㷨ʵ��:
����˵��:
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
static u16 PsWriteSetSysHead(TPsWrite *ptPSInfo)
{
    s32 s32i;

    TPsSysHeaderInfo *ptSysHead = &ptPSInfo->tSysHead;
    TPsMapInfo *ptMap = &ptPSInfo->tMap;

    //д��������Ƶ��Ƶ��඼ֻ��1����
    ptSysHead->u8AudioBound = 1;
    ptSysHead->u8VideoBound = 1;

	ptSysHead->u32RateBound = 3967;   //zxh  ���ʣ�������
    //�̶����ʱ�־
    ptSysHead->u8SystemAudioLockFlag = 1;
    ptSysHead->u8SystemVideoLockFlag = 1;

    //����������
    ptSysHead->u8StreamNum = ptMap->u8StreamNum;

    for (s32i=0; s32i<ptSysHead->u8StreamNum; s32i++)
    {
        ptSysHead->au8StreamId[s32i] = ptMap->au8StreamId[s32i];
        if (VIDEO_STREAM == ptSysHead->au8StreamId[s32i])
        {
            ptSysHead->au8PStdBufferBoundScale[s32i] = 1;
            //��Ƶ������Ϊ256KB
            ptSysHead->au16PStdBufferSizeBound[s32i] = 256;
        }
        else
        {
            ptSysHead->au8PStdBufferBoundScale[s32i] = 0;
            //��Ƶ������Ϊ4000B
            ptSysHead->au16PStdBufferSizeBound[s32i] = 32;
        }
    }
    
    return TSPS_OK;
}

static u16 PsWriteCallback(TPsWrite *ptPSInfo, TspsFRAMEHDR *ptFrame)
{
    if (ptPSInfo->pfCallback)
    {
        ptPSInfo->pfCallback(ptPSInfo->pu8Buf,  ptPSInfo->u32Len,  (KD_PTR)ptPSInfo->pvContext, ptFrame);
    }

    return TSPS_OK;
}

/*=================================================================================
������:PSProgramWritePES
����:��һ��PES��д�뵽PS����
�㷨ʵ��:
����˵��:
         [I/O] ptpPSInfo PS���ṹ
         [I]   pu8Buf PES��
         [I]   u32Len PES������
         [I]   u8Type ������,��ʱֻ֧��PES����
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PsWriteWriteEsFrame(TPsWrite *ptPsInfo, TspsFRAMEHDR *ptFrame)
{
    u32 u32PayloadLen = 0;

    TPsHeadInfo *ptHead;
    TPsSysHeaderInfo *ptSysHead;
    TPesInfo *ptPesInfo;

    u8 *pu8Buf = NULL;
    u32 u32Len = 0;
    u8 u8PT;
	u32 u32Tick = 0;
    
    //����������
    if (NULL == ptPsInfo || NULL == ptFrame)
    {
        return ERROR_PS_WRITE_INPUT_PARAM;
    }
    
    ptHead = &ptPsInfo->tHead;
    ptSysHead = &ptPsInfo->tSysHead;
    ptPesInfo = ptPsInfo->ptPesInfo;

    pu8Buf = ptFrame->m_pbyData;
    u32Len = ptFrame->m_dwDataSize;
    u8PT = TsPsPTCovertRtp2Stream(ptFrame->m_byMediaType);

    //����������
    if(NULL == pu8Buf || 0 == u32Len)
    {
        return ERROR_PS_WRITE_INPUT_FRAME;
    }

    //��������Ƿ����
    if (PT_STREAM_TYPE_NULL == u8PT || 
        (ptPsInfo->u8AudioType != u8PT && ptPsInfo->u8VideoType != u8PT))
    {
        return ERROR_PS_WRITE_STREAM_TYPE;
    }


    //���ݱ�׼�Ƚ� ͷ д��
    ptHead->u64SCRBase = ptFrame->m_dwTimeStamp;//SCR��PTS����
    ptHead->u16SCRExt = 0;
    ptHead->u32ProgramMuxRate = 0;
    PsWriteWriteHead(ptPsInfo);

    //���ݱ�׼�Ƚ� ӳ��� д��
	if (ptFrame->x.m_tVideoParam.m_bKeyFrame)
	{
		PsWriteWriteSysHead(ptPsInfo);
		PsWriteWritePsm(ptPsInfo);
	}
	//ֻ����Ƶ������£�ÿһ��Ӹ�psmͷ����ֹû����Ƶ�����������û��psmͷ���޷�����
	u32Tick = OspTickGet();
	if (ptPsInfo->u8VideoType == PT_STREAM_TYPE_NULL && 
		u32Tick - ptPsInfo->u32LastTimestamp > 1000)
	{
		PsWriteWriteSysHead(ptPsInfo);
		PsWriteWritePsm(ptPsInfo);		
		ptPsInfo->u32LastTimestamp = u32Tick;		
	}
	

	BOOL32 bFirstWhile = TRUE;
	u32  dwoffset =  ptPsInfo->u32Len; //the offset for writed  psm, next byte is pes
    do
    {
        u32 u32PesLen = 0;
		//ptPsInfo->u32Len  PS,PSM��ͷ����
		//��2��ѭ���а�ʱӦΪ0
		//u32PesLen pes���ĳ���
		//u32PayloadLen pes�����صĳ��ȣ���������pes�Ŀ�ʼ��
		//
 		if (FALSE == bFirstWhile)
 		{
 			ptPsInfo->u32Len = 0;
 		}
        
        //��һ�����PES��,����1000�ֽ�ÿ���ֱ�洢
        if (u32Len > PS_PACKET_LENGTH - PS_PES_PREDATA_LEN - ptPsInfo->u32Len)
        {
            u32PayloadLen = PS_PACKET_LENGTH - PS_PES_PREDATA_LEN - ptPsInfo->u32Len;
        }
        else
        {
            u32PayloadLen = u32Len;
        }
        
        //����pes��
        memset(ptPesInfo, 0, sizeof(TPesInfo));

        //��ʾ����ͬʱ����DTS��PTS
        ptPesInfo->u8PtsDtsFlag = 0x03;
        ptPesInfo->u64Pts = ptFrame->m_dwTimeStamp;
        ptPesInfo->u64Dts = ptFrame->m_dwTimeStamp;
        ptPesInfo->pu8PayloadBuffer = pu8Buf;
        ptPesInfo->u32PayloadLength = u32PayloadLen;
        ptPesInfo->u8StreamId = TsPsGetStreamIdPrefix(u8PT);
        
        u32PesLen = PS_PACKET_LENGTH - ptPsInfo->u32Len;		

        //��һС��ES��д��PES��
        if(dwoffset + u32PesLen < ptPsInfo->dwmaxframesize)
		{
			PesWriteInfo(ptPesInfo, ptPsInfo->pu8Buf + dwoffset, &u32PesLen);
	        ptPsInfo->u32Len += u32PesLen;
			dwoffset  += u32PesLen;
        }
		else
		{
			OspPrintf(1,0, "[PsWriteWriteEsFrame] Error, ps len + frame len > %d\n" , ptPsInfo->dwmaxframesize);
			return ERROR_PS_WRITE_INPUT_PARAM;
		}
		
        pu8Buf += u32PayloadLen;
        u32Len -= u32PayloadLen;
        bFirstWhile = FALSE;
    } while (0 < u32Len);
	
    ptPsInfo->u32Len = dwoffset;
    PsWriteCallback(ptPsInfo, ptFrame);

    //�ɹ��ڹ������ӡһ��
    TspsPrintf(PD_PS_WRITE, "PsWrite write a frame successfully. <len=%d>", ptFrame->m_dwDataSize);

    return TSPS_OK;
}

/*=================================================================================
������:PSProgramEndWrite
����:дPS��������־
�㷨ʵ��:
����˵��:
         [I/O]ptpPSInfo TS���ṹ
����ֵ˵��:����0
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PsWriteWriteEnd(TPsWrite *ptPsInfo)
{
    u8 pu8Buf[4];
    TBit tBitsBuf;
    TPsWrite *ptPSInfo = NULL;
    
    if (NULL == ptPsInfo)
    {
        return ERROR_PS_WRITE_INPUT_PARAM;
    }

    BitsInit(&tBitsBuf, pu8Buf, 4);
    
    //д�������
    BitsWrite32(&tBitsBuf, 32, STREAM_END_CODE);
    
    if (ptPsInfo->pfCallback)
    {
        ptPsInfo->pfCallback(pu8Buf, 4, (KD_PTR)ptPsInfo->pvContext, NULL);
    }

    //�ɹ��ڹ������ӡһ��
    TspsPrintf(PD_PS_WRITE, "PsWrite write an endian.");

    return TSPS_OK;
}

