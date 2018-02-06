/*=================================================================================
ģ����:PES���Ķ�д
�ļ���:pes.cpp
����ļ�:pes.h
ʵ�ֹ���:PES���Ķ�д
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
������:PESReadInfo
����:����PES������,���ES��
�㷨ʵ��: 
����˵��:[I/O] ptPesInfo �洢PES����Ϣ�Ľṹ
         [I]   pu8BufInput PES��
         [I]   u32LenInput PES������
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PesReadInfo(TPesInfo *ptPesInfo, u8 *pu8BufInput, u32 u32LenInput, u32 * u32LenOutput)
{
    TBit tBitsBuf;
    u8 u8Prefix1 = 0, u8Prefix2 = 0, u8Prefix3 = 0;
    u8 u8StreamId = 0;
    u8 u8Temp = 0;
    u64 u64Temp1 = 0L, u64Temp2 = 0L, u64Temp3 = 0L;
    u32 u32PayloadStart = 0;
	*u32LenOutput = 0;
    //��������ж�
    if((NULL == ptPesInfo) || (NULL == pu8BufInput) || (0 == u32LenInput))
    {
        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: input error.");
        return ERROR_PES_READ_INPUT_PARAM;
    }

    ptPesInfo->pu8PesBuffer = pu8BufInput;
    ptPesInfo->u32PesLength = u32LenInput;

	BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

    //��24λcode prefix
    u8Prefix1 = BitsRead8(&tBitsBuf, 8);
    u8Prefix2 = BitsRead8(&tBitsBuf, 8);
    u8Prefix3 = BitsRead8(&tBitsBuf, 8);
    if((0x00  != u8Prefix1) || (0x00  != u8Prefix2) || (0x01 != u8Prefix3))
    {
        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: cannot find 0x000001.");
        return ERROR_PES_READ_CODE_PREFIX;
    }

    //STREAM ID
    u8StreamId = BitsRead8(&tBitsBuf, 8);
    ptPesInfo->u8StreamId = u8StreamId;

    //PES������
    ptPesInfo->u32PacketLength = BitsRead32(&tBitsBuf, 16);
//    if (PES_DATA_HEAD_LENGTH + ptPesInfo->u32PacketLength != ptPesInfo->u32PesLength)
//    {
//        TspsPrintf(PD_PS_READ | PD_TS_READ, 
//            "PesRead fail: wrong length. input[%d] and parsed[%d]",
//            ptPesInfo->u32PesLength, PES_DATA_HEAD_LENGTH + ptPesInfo->u32PacketLength);
//        return ERROR_PES_READ_LENGTH;
//    }

    //��Ƶ,��Ƶ��
    if((u8StreamId != PROGRAM_STREAM_MAP) 
        && (u8StreamId != PADDING_STREAM) 
        && (u8StreamId != PRIVATE_STREAM_2) 
        && (u8StreamId != ECM_STREAM) 
        && (u8StreamId != EMM_STREAM) 
        && (u8StreamId != PROGRAM_STREAM_DIRECTORY))
    {
        //�̶����
        u8Temp = BitsRead8(&tBitsBuf, 2);
        if(u8Temp != 0x02)
        {
            TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: fix flag 0x02 wrong.");
            return ERROR_PES_READ_FIX_STREAMID;
        }

        //ES������Ϣ
        ptPesInfo->u8ScramblingControl = BitsRead8(&tBitsBuf, 2);//����
        ptPesInfo->u8Priority = BitsRead8(&tBitsBuf, 1);//���ȼ�
        ptPesInfo->u8DataAlignment = BitsRead8(&tBitsBuf, 1);//���ݶ���
        ptPesInfo->u8Copyrigth = BitsRead8(&tBitsBuf, 1);//��Ȩ
        ptPesInfo->u8OriginalOrCopy = BitsRead8(&tBitsBuf, 1);//ԭʼ����/����
        ptPesInfo->u8PtsDtsFlag = BitsRead8(&tBitsBuf, 2);//�Ƿ���PTS,DTS
        ptPesInfo->u8EscrFlag = BitsRead8(&tBitsBuf, 1);//�Ƿ���ESCR
        ptPesInfo->u8EsRateFlag = BitsRead8(&tBitsBuf, 1);//����
        ptPesInfo->u8DsmTrickModeFlag = BitsRead8(&tBitsBuf, 1);//������Ϣ
        ptPesInfo->u8AdditionalCopyInfo = BitsRead8(&tBitsBuf, 1);//������Ϣ
        ptPesInfo->u8CrcFlag = BitsRead8(&tBitsBuf, 1);//CRC
        ptPesInfo->u8ExtensionFlag = BitsRead8(&tBitsBuf, 1);//��չ��־

        //�ɱ�ͷ����
        ptPesInfo->u8HeadDataLength = BitsRead8(&tBitsBuf, 8);
		*u32LenOutput += 3;
        //PTS,DTS��Ϣ
        //ֻ��PTS��Ϣ
        if(0x02 == ptPesInfo->u8PtsDtsFlag)
        {
            u8Temp = BitsRead8(&tBitsBuf, 4);
            if(0x02 != u8Temp)
            {
                TspsPrintf(PD_PS_READ | PD_TS_READ, 
                    "PesRead fail: incomfortable PTS_DTS_FLAG. input[%02X], parsed[%02X]",
                    ptPesInfo->u8PtsDtsFlag, u8Temp);
                return ERROR_PES_READ_FIX_PTS;
            }
            u64Temp1 = BitsRead8(&tBitsBuf, 3);
            BitsSkip(&tBitsBuf, 1);
            u64Temp2 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            u64Temp3 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
			*u32LenOutput += 5;
            ptPesInfo->u64Pts = (u64Temp1 << 30) + (u64Temp2 << 15) + u64Temp3;
			ptPesInfo->u64Dts = ptPesInfo->u64Pts;
            ptPesInfo->bWriteDts = TRUE;
        }

        //ͬʱ��PTS��DTS��Ϣ
        if(0x03 == ptPesInfo->u8PtsDtsFlag)
        {
            u8Temp = BitsRead8(&tBitsBuf, 4);
            if(0x03 != u8Temp)
            {
                TspsPrintf(PD_PS_READ | PD_TS_READ, 
                    "PesRead fail: incomfortable PTS_DTS_FLAG. input[%02X], parsed[%02X]",
                    ptPesInfo->u8PtsDtsFlag, u8Temp);
                return ERROR_PES_READ_FIX_PTS_DTS_03;
            }
            u64Temp1 = BitsRead8(&tBitsBuf, 3);
            BitsSkip(&tBitsBuf, 1);
            u64Temp2 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            u64Temp3 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            
            ptPesInfo->u64Pts = (u64Temp1 << 30) + (u64Temp2 << 15) + u64Temp3;

            u8Temp = BitsRead8(&tBitsBuf, 4);
            if(0x01 != u8Temp)
            {
                TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: fix bits in PTS 0x01 wrong.");
                return ERROR_PES_READ_FIX_PTS_DTS_01;
            }
            u64Temp1 = BitsRead8(&tBitsBuf, 3);
            BitsSkip(&tBitsBuf, 1);
            u64Temp2 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            u64Temp3 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            *u32LenOutput += 10;
            ptPesInfo->u64Dts = (u64Temp1 << 30) + (u64Temp2 << 15) + u64Temp3;
            ptPesInfo->bWriteDts = TRUE;
        }
        //PTS,DTS��Ϣ����
#if 0
        //ESCR��Ϣ
        if(0x01 == ptPesInfo->u8EscrFlag)
        {
            BitsSkip(&tBitsBuf, 2);
            u64Temp1 = BitsRead8(&tBitsBuf, 3);
            BitsSkip(&tBitsBuf, 1);
            u64Temp2 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);
            u64Temp3 = BitsRead16(&tBitsBuf, 15);
            BitsSkip(&tBitsBuf, 1);

            ptPesInfo->u64EscrBase = (u64Temp1 << 30) + (u64Temp2 << 15) + u64Temp3;
            ptPesInfo->u16EscrExtension = BitsRead16(&tBitsBuf, 9);
            BitsSkip(&tBitsBuf, 1);
        }

        //ES RATE��Ϣ
        if(0x01 == ptPesInfo->u8EsRateFlag)
        {
            BitsSkip(&tBitsBuf, 1);
            ptPesInfo->u32EsRate = BitsRead32(&tBitsBuf, 22);
            BitsSkip(&tBitsBuf, 1);
        }

        //���¹���û��ʹ��
        //TRICK MODE��Ϣ
        if(0x01 == ptPesInfo->u8DsmTrickModeFlag)
        {
            ptPesInfo->u8TrickModeControl = BitsRead8(&tBitsBuf, 3);

            if(0x00 == ptPesInfo->u8TrickModeControl)
            {
                ptPesInfo->u8FieldId = BitsRead8(&tBitsBuf, 2);
                ptPesInfo->u8IntraSliceRefresh = BitsRead8(&tBitsBuf, 1);
                ptPesInfo->u8FrequencyTruncation = BitsRead8(&tBitsBuf, 2);
            }
            else if(0x01 == ptPesInfo->u8TrickModeControl)
            {
                ptPesInfo->u8FieldRepCntrl = BitsRead8(&tBitsBuf, 5);
            }
            else if(0x02 == ptPesInfo->u8TrickModeControl)
            {
                ptPesInfo->u8FieldId = BitsRead8(&tBitsBuf, 2);
                BitsSkip(&tBitsBuf, 3);
            }
            else if(0x03 == ptPesInfo->u8TrickModeControl)
            {
                ptPesInfo->u8FieldId = BitsRead8(&tBitsBuf, 2);
                ptPesInfo->u8IntraSliceRefresh = BitsRead8(&tBitsBuf, 1);
                ptPesInfo->u8FrequencyTruncation = BitsRead8(&tBitsBuf, 2);
            }
            else if(0x04 == ptPesInfo->u8TrickModeControl)
            {
                ptPesInfo->u8FieldRepCntrl = BitsRead8(&tBitsBuf, 5);
            }
            else
            {
                BitsSkip(&tBitsBuf, 5);
            }
        }
        //û��ʹ�ù��ܵ�����Ϊֹ

        //��Ȩ
        if(0x01 == ptPesInfo->u8AdditionalCopyInfoFlag)
        {
            BitsSkip(&tBitsBuf, 1);
            ptPesInfo->u8AdditionalCopyInfo = BitsRead8(&tBitsBuf, 7);
        }

        //CRC��Ϣ
        if(0x01 == ptPesInfo->u8CrcFlag)
        {
            ptPesInfo->u16PreviousPesPacketCrc = BitsRead16(&tBitsBuf, 16);
        }

        //���¹���û��ʹ��
        //��չ��Ϣ
        if(0x01 == ptPesInfo->u8ExtensionFlag)
        {
            ptPesInfo->u8PrivateDataFlag = BitsRead8(&tBitsBuf, 1);
            ptPesInfo->u8PackHeaderFieldFlag = BitsRead8(&tBitsBuf, 1);
            ptPesInfo->u8ProgramPackSequenceCounterFlag = BitsRead8(&tBitsBuf, 1);
            ptPesInfo->u8PStdBufferFlag = BitsRead8(&tBitsBuf, 1);
            BitsSkip(&tBitsBuf, 3);
            ptPesInfo->u8ExtensionFlag2 = BitsRead8(&tBitsBuf, 1);

            if(0x01 == ptPesInfo->u8PrivateDataFlag)
            {
                memcpy(ptPesInfo->u8PrivateData, tBitsBuf.pu8Current, 128 / 8);
                BitsSkip(&tBitsBuf, 128);
            }
            if(0x01 == ptPesInfo->u8PackHeaderFieldFlag)
            {
                ptPesInfo->u8PackFieldLength = BitsRead8(&tBitsBuf, 8);
                BitsSkip(&tBitsBuf, ptPesInfo->u8PackFieldLength * 8);
            }
            if(0x01 == ptPesInfo->u8ProgramPackSequenceCounterFlag)
            {
                BitsSkip(&tBitsBuf, 1);
                ptPesInfo->u8ProgramPacketSequenceCounter = BitsRead8(&tBitsBuf, 7);
                BitsSkip(&tBitsBuf, 1);
                ptPesInfo->u8Mpeg1Mpeg2Identifier = BitsRead8(&tBitsBuf, 1);
                ptPesInfo->u8OriginalStuffLength = BitsRead8(&tBitsBuf, 6);
            }
            if(0x01 == ptPesInfo->u8PStdBufferFlag)
            {
                u8Temp = BitsRead8(&tBitsBuf, 2);
                if(u8Temp != 0x01)
                {
                    TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: fix bits in StdBufferFlag 0x01 wrong.");
                    return ERROR_PES_READ_FIX_STD;
                }
                ptPesInfo->u8PStdBufferScale = BitsRead8(&tBitsBuf, 1);
                ptPesInfo->u16PStdBufferSize = BitsRead16(&tBitsBuf, 13);
            }

            if(0x01 == ptPesInfo->u8ExtensionFlag2)
            {
                BitsSkip(&tBitsBuf, 1);
                ptPesInfo->u8ExtensionFieldLength = BitsRead8(&tBitsBuf, 7);
                BitsSkip(&tBitsBuf, ptPesInfo->u8ExtensionFieldLength * 8);
            }
        }
        //û��ʹ�ù��ܵ�����Ϊֹ
#endif
        //���ؿ�ʼλ��
        u32PayloadStart = PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + ptPesInfo->u8HeadDataLength;

        //�õ�ES��λ�úͳ���
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - u32PayloadStart;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + u32PayloadStart;
    }
    else if (u8StreamId == PADDING_STREAM)
    {
        //���ش�6�ֽ�(48λ)��ʼ
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - PES_DATA_HEAD_LENGTH;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH;

        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: received a padding stream.");
        return ERROR_PES_READ_TYPE_NOT_SUPPORT;
    }
    else
    {
        //���ش�6�ֽ�(48λ)��ʼ
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - PES_DATA_HEAD_LENGTH;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH;
        
        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: received an unknown stream.");
        return ERROR_PES_READ_TYPE_NOT_SUPPORT;
    }

    return TSPS_OK;
}

/*=================================================================================
������:PESWriteInfo
����:����PES������,���ES��
�㷨ʵ��: (�ڽ���PES�������ʱ,������д��ȷ��stream id,DTS����PTS)
����˵��:[I/O] ptPesInfo �洢PES����Ϣ�Ľṹ
         [I/O] pu8BufOutput PES��
         [I/O] u32LenOutput PES������
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PesWriteInfo(TPesInfo *ptPesInfo, u8 *pu8BufOutput, u32 *pu32LenOutput)
{
    TBit tBitsBuf;
    u8 u8StreamId = ptPesInfo->u8StreamId;
    s32 s32i = 0;
    u32 u32PayloadStart = 0;

    //����������
    if((NULL == ptPesInfo) || (NULL == pu8BufOutput) || (NULL == pu32LenOutput))
    {
        TspsPrintf(PD_PS_WRITE | PD_TS_WRITE, "PesWrite fail: input error.");
        return ERROR_PES_WRITE_INPUT_PARAM;
    }

    ptPesInfo->pu8PesBuffer = pu8BufOutput;
    ptPesInfo->u32PesLength = (*pu32LenOutput);

    BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

    //24λ,��ʼ��000001
    BitsWrite8(&tBitsBuf, 8, 0x00);
    BitsWrite8(&tBitsBuf, 8, 0x00);
    BitsWrite8(&tBitsBuf, 8, 0x01);

    //stream id
    BitsWrite8(&tBitsBuf, 8, u8StreamId);

    //������,������0
    BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);

    //��Ƶ,��Ƶ��
    if((u8StreamId != PROGRAM_STREAM_MAP) 
        && (u8StreamId != PADDING_STREAM) 
        && (u8StreamId != PRIVATE_STREAM_2) 
        && (u8StreamId != ECM_STREAM) 
        && (u8StreamId != EMM_STREAM) 
        && (u8StreamId != PROGRAM_STREAM_DIRECTORY))
    {
        //ES������Ϣ
        BitsWrite8(&tBitsBuf, 2, 0x02);
        BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8ScramblingControl);//���ܿ���
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8Priority);//���ȼ�
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8DataAlignment);//���ݶ���
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8Copyrigth);//��Ȩ
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8OriginalOrCopy);//ԭʼ����/����
        BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8PtsDtsFlag);//PTS,DTS��־
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8EscrFlag);//ESCR
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8EsRateFlag);//����
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8DsmTrickModeFlag);//������Ϣ
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8AdditionalCopyInfoFlag);//������Ϣ
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8CrcFlag);//CRC
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8ExtensionFlag);//��չ
        BitsWrite8(&tBitsBuf, 8, ptPesInfo->u8HeadDataLength);//ͷ����

        //pts,dts��ʼ
        //ֻ��PTS
        if(0x02 == ptPesInfo->u8PtsDtsFlag)
        {
            BitsWrite8(&tBitsBuf, 4, 0x02);
            BitsWrite64(&tBitsBuf, 3, (ptPesInfo->u64Pts >> 30));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, (ptPesInfo->u64Pts >> 15));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, ptPesInfo->u64Pts);
            BitsWrite8(&tBitsBuf, 1, 1);

            ptPesInfo->u8HeadDataLength += (40 / 8);
        }

        //PTS/DTS����
        if(0x03 == ptPesInfo->u8PtsDtsFlag)
        {
            BitsWrite8(&tBitsBuf, 4, 0x03);
            BitsWrite64(&tBitsBuf, 3, (ptPesInfo->u64Pts >> 30));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, (ptPesInfo->u64Pts >> 15));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, ptPesInfo->u64Pts);
            BitsWrite8(&tBitsBuf, 1, 1);

            BitsWrite8(&tBitsBuf, 4, 0x01);
            BitsWrite64(&tBitsBuf, 3, (ptPesInfo->u64Dts >> 30));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, (ptPesInfo->u64Dts >> 15));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, ptPesInfo->u64Dts);
            BitsWrite8(&tBitsBuf, 1, 1);
            
            ptPesInfo->u8HeadDataLength += (80 / 8);
        }
        //pts,dts����

        //ESCR
        if(0x01 == ptPesInfo->u8EscrFlag)
        {
            BitsWrite8(&tBitsBuf, 2, 0x03);
            BitsWrite64(&tBitsBuf, 3, (ptPesInfo->u64EscrBase >> 30));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, (ptPesInfo->u64EscrBase >> 15));
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite64(&tBitsBuf, 15, ptPesInfo->u64EscrBase);
            BitsWrite8(&tBitsBuf, 1, 1);

            BitsWrite16(&tBitsBuf, 9, ptPesInfo->u16EscrExtension);
            BitsWrite8(&tBitsBuf, 1, 1);

            ptPesInfo->u8HeadDataLength += (48 / 8);
        }

        //es rate
        if(0x01 == ptPesInfo->u8EsRateFlag)
        {
            BitsWrite8(&tBitsBuf, 1, 1);
            BitsWrite32(&tBitsBuf, 22, ptPesInfo->u32EsRate);
            BitsWrite8(&tBitsBuf, 1, 1);

            ptPesInfo->u8HeadDataLength += (24 / 8);
        }

        //���¹���û��ʹ��
        if(0x01 == ptPesInfo->u8DsmTrickModeFlag)
        {
            BitsWrite8(&tBitsBuf, 3, ptPesInfo->u8TrickModeControl);
            if(0x00 == ptPesInfo->u8TrickModeControl)
            {
                BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8FieldId);
                BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8IntraSliceRefresh);
                BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8FrequencyTruncation);
            }
            else if(0x01 == ptPesInfo->u8TrickModeControl)
            {
                BitsWrite8(&tBitsBuf, 5, ptPesInfo->u8FieldRepCntrl);
            }
            else if(0x02 == ptPesInfo->u8TrickModeControl)
            {
                BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8FieldId);
                BitsWrite8(&tBitsBuf, 3, 0x03);
            }
            else if(0x03 == ptPesInfo->u8TrickModeControl)
            {
                BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8FieldId);
                BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8IntraSliceRefresh);
                BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8FrequencyTruncation);
            }
            else if(0x04 == ptPesInfo->u8TrickModeControl)
            {
                BitsWrite8(&tBitsBuf, 5, ptPesInfo->u8FieldRepCntrl);
            }
            else
            {
                BitsWrite8(&tBitsBuf, 5, 0x1F);
            }
            ptPesInfo->u8HeadDataLength += (8 / 8);
        }

        if(0x01 == ptPesInfo->u8AdditionalCopyInfoFlag)
        {
            BitsWrite8(&tBitsBuf, 1, 0x01);
            BitsWrite8(&tBitsBuf, 7, ptPesInfo->u8AdditionalCopyInfo);

            ptPesInfo->u8HeadDataLength += (8 / 8);
        }

        if(0x01 == ptPesInfo->u8CrcFlag)
        {
            BitsWrite16(&tBitsBuf, 16, ptPesInfo->u16PreviousPesPacketCrc);

            ptPesInfo->u8HeadDataLength += (16 / 8);
        }

        if(0x01 == ptPesInfo->u8ExtensionFlag)
        {
            BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8PrivateDataFlag);
            BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8PackHeaderFieldFlag);
            BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8ProgramPackSequenceCounterFlag);
            BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8PStdBufferFlag);
            BitsWrite8(&tBitsBuf, 3, 0x07);
            BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8ExtensionFlag2);

            ptPesInfo->u8HeadDataLength += (8 / 8);

            if(0x01 == ptPesInfo->u8PrivateDataFlag)
            {
                memcpy(tBitsBuf.pu8Current, ptPesInfo->u8PrivateData, (128 / 8));
                BitsSkip(&tBitsBuf, 128);

                ptPesInfo->u8HeadDataLength += (128 / 8);
            }

            if(0x01 == ptPesInfo->u8PackHeaderFieldFlag)
            {
                BitsWrite8(&tBitsBuf, 8, ptPesInfo->u8PackFieldLength);
                BitsSkip(&tBitsBuf, ptPesInfo->u8PackFieldLength * 8);

                ptPesInfo->u8HeadDataLength += ((8 / 8) + ptPesInfo->u8PackFieldLength);
            }

            if(0x01 == ptPesInfo->u8ProgramPackSequenceCounterFlag)
            {
                BitsWrite8(&tBitsBuf, 1, 0x01);
                BitsWrite8(&tBitsBuf, 7, ptPesInfo->u8ProgramPacketSequenceCounter);
                BitsWrite8(&tBitsBuf, 1, 0x01);
                BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8Mpeg1Mpeg2Identifier);
                BitsWrite8(&tBitsBuf, 6, ptPesInfo->u8OriginalStuffLength);

                ptPesInfo->u8HeadDataLength += (16 / 8);
            }

            if(0x01 == ptPesInfo->u8PStdBufferFlag)
            {
                BitsWrite8(&tBitsBuf, 2, 0x01);
                BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8PStdBufferScale);
                BitsWrite16(&tBitsBuf, 13, ptPesInfo->u16PStdBufferSize);
                
                ptPesInfo->u8HeadDataLength += (16 / 8);
            }

            if(0x01 == ptPesInfo->u8ExtensionFlag2)
            {
                BitsWrite8(&tBitsBuf, 1, 0x01);
                BitsWrite8(&tBitsBuf, 7, ptPesInfo->u8ExtensionFieldLength);

                for(s32i = 0; s32i < ptPesInfo->u8ExtensionFieldLength; s32i++)
                {
                    BitsWrite8(&tBitsBuf, 8, 0xff);
                }                

                ptPesInfo->u8HeadDataLength += ((8 / 8) + ptPesInfo->u8ExtensionFieldLength);
            }
        }
        //û��ʹ�õĹ��ܵ�����Ϊֹ

        //���㸺�ؿ�ʼλ��
        u32PayloadStart = ptPesInfo->u8HeadDataLength + PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH;

        memcpy(ptPesInfo->pu8PesBuffer + u32PayloadStart, ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
        
        //����ʵ��PES����
        ptPesInfo->u32PesLength = u32PayloadStart + ptPesInfo->u32PayloadLength;
        *pu32LenOutput = ptPesInfo->u32PesLength;
        
		// bug fix,�˴���Ӧ����u16ǿ��ת����ligeng@2012.10.25
        ptPesInfo->u32PacketLength = /*(u16)*/(ptPesInfo->u32PesLength) - PES_DATA_HEAD_LENGTH;

        //�޸�PES������
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        //����code prefix��stream id
        BitsSkip(&tBitsBuf, 24 + 8);

        //������65535,д����Ϊ0
        if (ptPesInfo->u32PacketLength > PES_PACKET_MAX_LENGTH)
        {
            ptPesInfo->u32PacketLength = 0;
        }
        BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);
        
        //����������Ϣ
        BitsSkip(&tBitsBuf, 16);

        //�޸Ŀɱ�ͷ����
        BitsWrite8(&tBitsBuf, 8, ptPesInfo->u8HeadDataLength);
       
    }
    else if (u8StreamId == PADDING_STREAM)
    {
        memcpy(ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH, ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
        ptPesInfo->u32PesLength = ptPesInfo->u32PayloadLength + PES_DATA_HEAD_LENGTH;
        *pu32LenOutput = ptPesInfo->u32PesLength;
        
        ptPesInfo->u32PacketLength = (u16)ptPesInfo->u32PayloadLength;
        
        //�޸�PES������
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        BitsSkip(&tBitsBuf, 24 + 8);
        //������65535,д����Ϊ0
        if (ptPesInfo->u32PacketLength > PES_PACKET_MAX_LENGTH)
        {
            ptPesInfo->u32PacketLength = 0;
        }
        BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);
    }
    else
    {
        memcpy(ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH, ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
        ptPesInfo->u32PesLength = ptPesInfo->u32PayloadLength + PES_DATA_HEAD_LENGTH;
        *pu32LenOutput = ptPesInfo->u32PesLength;
        
        ptPesInfo->u32PacketLength = (u16)ptPesInfo->u32PayloadLength;
        
        //�޸�PES������
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);
        
        BitsSkip(&tBitsBuf, 24 + 8);
        
        //������65535,д����Ϊ0
        if (ptPesInfo->u32PacketLength > PES_PACKET_MAX_LENGTH)
        {
            TspsPrintf(PD_PS_WRITE | PD_TS_WRITE, "PesWrite warning: es frame length > 65535, force it to 0.");
            ptPesInfo->u32PacketLength = 0;
        }
        BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);
    }

    return TSPS_OK;
}


// ���������ͣ������idǰ׺
u8 TsPsGetStreamIdPrefix(u8 u8StreamType)
{
    switch (u8StreamType)
    {
    case PT_STREAM_TYPE_MP1:
    case PT_STREAM_TYPE_MP2:
	case PT_STREAM_TYPE_AACLC:
	case PT_STREAM_TYPE_MP2AAC:
	case PT_STREAM_TYPE_G711A:
	case PT_STREAM_TYPE_G7221:
	case PT_STREAM_TYPE_G7231:
	case PT_STREAM_TYPE_G729:
	case PT_STREAM_TYPE_SVACA:	
        return AUDIO_STREAM0;
        
    case PT_STREAM_TYPE_MPEG1:
    case PT_STREAM_TYPE_MPEG2:
    case PT_STREAM_TYPE_MPEG4:
	case PT_STREAM_TYPE_H264:
	case PT_STREAM_TYPE_H265_Old:
    case PT_STREAM_TYPE_H265:
	case PT_STREAM_TYPE_SVACV:
        return VIDEO_STREAM;
        
    default:
        return 0x00;
    }
}

