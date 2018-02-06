/*=================================================================================
模块名:PES包的读写
文件名:pes.cpp
相关文件:pes.h
实现功能:PES包的读写
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

/*=================================================================================
函数名:PESReadInfo
功能:分析PES包内容,获得ES流
算法实现: 
参数说明:[I/O] ptPesInfo 存储PES包信息的结构
         [I]   pu8BufInput PES包
         [I]   u32LenInput PES包长度
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
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
    //输入参数判断
    if((NULL == ptPesInfo) || (NULL == pu8BufInput) || (0 == u32LenInput))
    {
        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: input error.");
        return ERROR_PES_READ_INPUT_PARAM;
    }

    ptPesInfo->pu8PesBuffer = pu8BufInput;
    ptPesInfo->u32PesLength = u32LenInput;

	BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

    //读24位code prefix
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

    //PES包长度
    ptPesInfo->u32PacketLength = BitsRead32(&tBitsBuf, 16);
//    if (PES_DATA_HEAD_LENGTH + ptPesInfo->u32PacketLength != ptPesInfo->u32PesLength)
//    {
//        TspsPrintf(PD_PS_READ | PD_TS_READ, 
//            "PesRead fail: wrong length. input[%d] and parsed[%d]",
//            ptPesInfo->u32PesLength, PES_DATA_HEAD_LENGTH + ptPesInfo->u32PacketLength);
//        return ERROR_PES_READ_LENGTH;
//    }

    //音频,视频流
    if((u8StreamId != PROGRAM_STREAM_MAP) 
        && (u8StreamId != PADDING_STREAM) 
        && (u8StreamId != PRIVATE_STREAM_2) 
        && (u8StreamId != ECM_STREAM) 
        && (u8StreamId != EMM_STREAM) 
        && (u8StreamId != PROGRAM_STREAM_DIRECTORY))
    {
        //固定标记
        u8Temp = BitsRead8(&tBitsBuf, 2);
        if(u8Temp != 0x02)
        {
            TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: fix flag 0x02 wrong.");
            return ERROR_PES_READ_FIX_STREAMID;
        }

        //ES特有信息
        ptPesInfo->u8ScramblingControl = BitsRead8(&tBitsBuf, 2);//加密
        ptPesInfo->u8Priority = BitsRead8(&tBitsBuf, 1);//优先级
        ptPesInfo->u8DataAlignment = BitsRead8(&tBitsBuf, 1);//数据对齐
        ptPesInfo->u8Copyrigth = BitsRead8(&tBitsBuf, 1);//版权
        ptPesInfo->u8OriginalOrCopy = BitsRead8(&tBitsBuf, 1);//原始数据/拷贝
        ptPesInfo->u8PtsDtsFlag = BitsRead8(&tBitsBuf, 2);//是否有PTS,DTS
        ptPesInfo->u8EscrFlag = BitsRead8(&tBitsBuf, 1);//是否有ESCR
        ptPesInfo->u8EsRateFlag = BitsRead8(&tBitsBuf, 1);//码率
        ptPesInfo->u8DsmTrickModeFlag = BitsRead8(&tBitsBuf, 1);//控制信息
        ptPesInfo->u8AdditionalCopyInfo = BitsRead8(&tBitsBuf, 1);//额外信息
        ptPesInfo->u8CrcFlag = BitsRead8(&tBitsBuf, 1);//CRC
        ptPesInfo->u8ExtensionFlag = BitsRead8(&tBitsBuf, 1);//拓展标志

        //可变头长度
        ptPesInfo->u8HeadDataLength = BitsRead8(&tBitsBuf, 8);
		*u32LenOutput += 3;
        //PTS,DTS信息
        //只有PTS信息
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

        //同时有PTS和DTS信息
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
        //PTS,DTS信息结束
#if 0
        //ESCR信息
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

        //ES RATE信息
        if(0x01 == ptPesInfo->u8EsRateFlag)
        {
            BitsSkip(&tBitsBuf, 1);
            ptPesInfo->u32EsRate = BitsRead32(&tBitsBuf, 22);
            BitsSkip(&tBitsBuf, 1);
        }

        //以下功能没有使用
        //TRICK MODE信息
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
        //没有使用功能到这里为止

        //版权
        if(0x01 == ptPesInfo->u8AdditionalCopyInfoFlag)
        {
            BitsSkip(&tBitsBuf, 1);
            ptPesInfo->u8AdditionalCopyInfo = BitsRead8(&tBitsBuf, 7);
        }

        //CRC信息
        if(0x01 == ptPesInfo->u8CrcFlag)
        {
            ptPesInfo->u16PreviousPesPacketCrc = BitsRead16(&tBitsBuf, 16);
        }

        //以下功能没有使用
        //拓展信息
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
        //没有使用功能到这里为止
#endif
        //负载开始位置
        u32PayloadStart = PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + ptPesInfo->u8HeadDataLength;

        //得到ES流位置和长度
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - u32PayloadStart;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + u32PayloadStart;
    }
    else if (u8StreamId == PADDING_STREAM)
    {
        //负载从6字节(48位)后开始
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - PES_DATA_HEAD_LENGTH;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH;

        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: received a padding stream.");
        return ERROR_PES_READ_TYPE_NOT_SUPPORT;
    }
    else
    {
        //负载从6字节(48位)后开始
        ptPesInfo->u32PayloadLength = ptPesInfo->u32PesLength - PES_DATA_HEAD_LENGTH;
        ptPesInfo->pu8PayloadBuffer = ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH;
        
        TspsPrintf(PD_PS_READ | PD_TS_READ, "PesRead fail: received an unknown stream.");
        return ERROR_PES_READ_TYPE_NOT_SUPPORT;
    }

    return TSPS_OK;
}

/*=================================================================================
函数名:PESWriteInfo
功能:分析PES包内容,获得ES流
算法实现: (在进行PES打包过程时,必须填写正确的stream id,DTS或者PTS)
参数说明:[I/O] ptPesInfo 存储PES包信息的结构
         [I/O] pu8BufOutput PES包
         [I/O] u32LenOutput PES包长度
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 PesWriteInfo(TPesInfo *ptPesInfo, u8 *pu8BufOutput, u32 *pu32LenOutput)
{
    TBit tBitsBuf;
    u8 u8StreamId = ptPesInfo->u8StreamId;
    s32 s32i = 0;
    u32 u32PayloadStart = 0;

    //输入参数检查
    if((NULL == ptPesInfo) || (NULL == pu8BufOutput) || (NULL == pu32LenOutput))
    {
        TspsPrintf(PD_PS_WRITE | PD_TS_WRITE, "PesWrite fail: input error.");
        return ERROR_PES_WRITE_INPUT_PARAM;
    }

    ptPesInfo->pu8PesBuffer = pu8BufOutput;
    ptPesInfo->u32PesLength = (*pu32LenOutput);

    BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

    //24位,开始码000001
    BitsWrite8(&tBitsBuf, 8, 0x00);
    BitsWrite8(&tBitsBuf, 8, 0x00);
    BitsWrite8(&tBitsBuf, 8, 0x01);

    //stream id
    BitsWrite8(&tBitsBuf, 8, u8StreamId);

    //包长度,现在是0
    BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);

    //音频,视频流
    if((u8StreamId != PROGRAM_STREAM_MAP) 
        && (u8StreamId != PADDING_STREAM) 
        && (u8StreamId != PRIVATE_STREAM_2) 
        && (u8StreamId != ECM_STREAM) 
        && (u8StreamId != EMM_STREAM) 
        && (u8StreamId != PROGRAM_STREAM_DIRECTORY))
    {
        //ES特有信息
        BitsWrite8(&tBitsBuf, 2, 0x02);
        BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8ScramblingControl);//加密控制
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8Priority);//优先级
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8DataAlignment);//数据对齐
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8Copyrigth);//版权
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8OriginalOrCopy);//原始数据/拷贝
        BitsWrite8(&tBitsBuf, 2, ptPesInfo->u8PtsDtsFlag);//PTS,DTS标志
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8EscrFlag);//ESCR
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8EsRateFlag);//码率
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8DsmTrickModeFlag);//控制信息
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8AdditionalCopyInfoFlag);//额外信息
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8CrcFlag);//CRC
        BitsWrite8(&tBitsBuf, 1, ptPesInfo->u8ExtensionFlag);//拓展
        BitsWrite8(&tBitsBuf, 8, ptPesInfo->u8HeadDataLength);//头长度

        //pts,dts开始
        //只有PTS
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

        //PTS/DTS都有
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
        //pts,dts结束

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

        //以下功能没有使用
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
        //没有使用的功能到这里为止

        //计算负载开始位置
        u32PayloadStart = ptPesInfo->u8HeadDataLength + PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH;

        memcpy(ptPesInfo->pu8PesBuffer + u32PayloadStart, ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
        
        //计算实际PES包长
        ptPesInfo->u32PesLength = u32PayloadStart + ptPesInfo->u32PayloadLength;
        *pu32LenOutput = ptPesInfo->u32PesLength;
        
		// bug fix,此处不应该用u16强制转换，ligeng@2012.10.25
        ptPesInfo->u32PacketLength = /*(u16)*/(ptPesInfo->u32PesLength) - PES_DATA_HEAD_LENGTH;

        //修改PES包长度
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        //跳过code prefix和stream id
        BitsSkip(&tBitsBuf, 24 + 8);

        //若大于65535,写入结果为0
        if (ptPesInfo->u32PacketLength > PES_PACKET_MAX_LENGTH)
        {
            ptPesInfo->u32PacketLength = 0;
        }
        BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);
        
        //跳过特有信息
        BitsSkip(&tBitsBuf, 16);

        //修改可变头长度
        BitsWrite8(&tBitsBuf, 8, ptPesInfo->u8HeadDataLength);
       
    }
    else if (u8StreamId == PADDING_STREAM)
    {
        memcpy(ptPesInfo->pu8PesBuffer + PES_DATA_HEAD_LENGTH, ptPesInfo->pu8PayloadBuffer, ptPesInfo->u32PayloadLength);
        ptPesInfo->u32PesLength = ptPesInfo->u32PayloadLength + PES_DATA_HEAD_LENGTH;
        *pu32LenOutput = ptPesInfo->u32PesLength;
        
        ptPesInfo->u32PacketLength = (u16)ptPesInfo->u32PayloadLength;
        
        //修改PES包长度
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);

        BitsSkip(&tBitsBuf, 24 + 8);
        //若大于65535,写入结果为0
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
        
        //修改PES包长度
        BitsInit(&tBitsBuf, ptPesInfo->pu8PesBuffer, ptPesInfo->u32PesLength);
        
        BitsSkip(&tBitsBuf, 24 + 8);
        
        //若大于65535,写入结果为0
        if (ptPesInfo->u32PacketLength > PES_PACKET_MAX_LENGTH)
        {
            TspsPrintf(PD_PS_WRITE | PD_TS_WRITE, "PesWrite warning: es frame length > 65535, force it to 0.");
            ptPesInfo->u32PacketLength = 0;
        }
        BitsWrite32(&tBitsBuf, 16, ptPesInfo->u32PacketLength);
    }

    return TSPS_OK;
}


// 根据流类型，获得流id前缀
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

