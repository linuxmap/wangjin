#include "kdvtype.h"

//ps关键字定义
#define PACK_HEADER_START_CODE              0x000001BA  //PS包头开始标记
#define SYSTEM_HEADER_START_CODE            0x000001BB  //PS系统头开始标记
#define MAP_START_CODE                      0x000001BC  //映射段头开始标记
#define STREAM_END_CODE                     0x000001B9  //流开始标记

#define PS_HEAD_BYTE                        (u8)(PACK_HEADER_START_CODE   & 0xff)
#define PS_SYSTEM_HEAD_BYTE                 (u8)(SYSTEM_HEADER_START_CODE & 0xff)
#define PS_MAP_BYTE                         (u8)(MAP_START_CODE           & 0xff)
#define PS_END_BYTE                         (u8)(STREAM_END_CODE          & 0xff)

//PES有关宏定义
#define PES_PACKET_MAX_LENGTH       65535   //PES包(负载)最大长度
#define PES_DATA_HEAD_LENGTH        6       //PES包头长度(固定6字节)
#define PES_ES_INFO_LENGTH          3       //ES流特有信息(固定3字节)
#define PES_PACK_MAX_LEN            (PES_PACKET_MAX_LENGTH + PES_DATA_HEAD_LENGTH)

//PES包STREAM ID
#define PROGRAM_STREAM_MAP          0xBC    //节目映射流
#define PADDING_STREAM              0xBE    //填充流
#define PRIVATE_STREAM_1            0xBD    //私有流
#define PRIVATE_STREAM_2            0xBF    //私有流
#define AUDIO_STREAM                0xC0    //视频流
#define VIDEO_STREAM                0xE0    //音频流
#define ECM_STREAM                  0xF0    //ECM流
#define EMM_STREAM                  0xF1    //EMM流
#define DSM_CC_STREAM               0xF2    //DSM流
#define ISOIEC_13522_STREAM         0xF3    //13522流
#define RESERVED_DATA_STREAM        0xF0    //保留数据流
#define PROGRAM_STREAM_DIRECTORY    0xFF    //节目流路径

//错误码定义
#define RPS_OK                                 0


#define RPS_ERR_NOT_PS			1//非ps流的错误码
#define RPS_ERR_PARAM			2//参数错误
#define RPS_ERR_STARTCODE		3//找不到001起始码

u16 get_ps_head_len(u8 *ptRtpData, u32 dwRtpDataSize, u16 *pwPsHeadLen);

s32 PsReadFindHead(u8 *pbyBuf, u32 dwLen, u32 *pdwPostion);

u16 PsReadParseHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);
u16 PsReadParseSysHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);
s32 PsReadParsePesHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);

u16 main()
{//14字ps节头+12字节系统头+20字节映射头+ 19字节pes头
	u8 RtpData[1024] = {0x00, 0x00, 0x01, 0xBA, 0x45, 0x8D, 0x8D, 0x68,
			0xB4, 0x01, 0x00, 0x00, 0x03, 0xF8, 
			0x00, 0x00, 0x01, 0xBB, 0x00, 0x06, 0x80, 0x00, 0x01, 0x00, 0x20, 0x7F, 
			0x00, 0x00, 0x01, 0xBC, 0x00, 0x0E, 0xE5, 0xFF, 0x00, 0x00,
			0x00, 0x04, 0x1B, 0xE0, 0x00, 0x00, 0x48, 0x0C, 0xD3, 0xEF, 
			0x00, 0x00, 0x01, 0xE0, 0x04, 0x0A, 0x84, 0xC0, 0x0A, 0x31,
			0x63, 0x63, 0x5A, 0x2D, 0x11, 0x63, 0x63, 0x5A, 0x2D, 0x00,
			0x00, 0x00, 0x01, 0x61, 0x9A, 0x58 };
	u16 HeadLen = 0;
	u16 wRet = get_ps_head_len(RtpData, 100, &HeadLen);
	return 0;
}
/*=================================================================================
函数名:get_ps_head_len
功能:打印
算法实现:
参数说明:[IN]Rtp负载数据, [IN]输入数据长, [OUT] ps头长度
[IN]u8 *ptRtpData, [IN]u32 dwRtpDataSize, [OUT]u16 *pwPsHeadLen
返回值说明:数据是否是ps流
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录

=================================================================================*/

u16 get_ps_head_len(u8 *ptRtpData, u32 dwRtpDataSize, u16 *pwPsHeadLen)
{
	u32 dwRemain = dwRtpDataSize;
	u32 dwStartPos = 0;
	u8	byTypeByte;			//字段类型标识
	u32 dwPackLen = 0;
	u16 wRet = RPS_OK;

	while(dwRemain > 0)
	{
		if (PsReadFindHead(ptRtpData, dwRtpDataSize, &dwStartPos) != RPS_OK)
		{
			break;
// 			return RPS_ERR_STARTCODE;
		}


		//解析一包
		byTypeByte = ptRtpData[3];

		switch(byTypeByte)
		{
		case PS_HEAD_BYTE:
			wRet = PsReadParseHead(ptRtpData, dwRemain, &dwPackLen);
			break;

		case PS_SYSTEM_HEAD_BYTE:
			wRet = PsReadParseSysHead(ptRtpData, dwRemain, &dwPackLen);
			break;

        case PS_MAP_BYTE:
            wRet = PsReadParseSysHead(ptRtpData, dwRemain, &dwPackLen);
            break;
			
        case PS_END_BYTE:
            //遇到结束把缓冲内数据回调出去
            dwPackLen = 4;
            break;
			
        case VIDEO_STREAM:
        case AUDIO_STREAM:
            wRet = PsReadParsePesHead(ptRtpData, dwRemain, &dwPackLen);
			*pwPsHeadLen = dwRtpDataSize - dwRemain + dwPackLen;
			return wRet;
				
		}
		if (wRet != RPS_OK)
		{
			return wRet;
		}
		//位置偏移前进
		ptRtpData += dwPackLen;
		dwRemain -= dwPackLen; 
	}

	return wRet;
}


//查找0x000001起始符   [OUT]u32 *pdwPostion
static s32 PsReadFindHead(u8 *pbyBuf, u32 dwLen, u32 *pdwPostion)
{
    u32 dwPos = 0;
    
    while ((dwPos + 2) < dwLen)
    {
        if (pbyBuf[dwPos] == 0 && pbyBuf[dwPos+1] == 0 && pbyBuf[dwPos+2] == 1)
        {
			*pdwPostion = dwPos;
            return RPS_OK;
        }		
        dwPos++;
    }	
    return RPS_ERR_STARTCODE;
}
//读ps头  [OUT]u32 *pPackLen
static u16 PsReadParseHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	u8 byStufLen = 0;
	if (dwLen < 14)
	{
		return RPS_ERR_PARAM;
	}
	byStufLen = pbyBuf[13] & 0x07; //  padding长度 0000 0111
	*pPackLen = 14 + byStufLen;
	return RPS_OK;
}

//读系统头/解析psm程序流映射  [OUT]u32 *pPackLen
static u16 PsReadParseSysHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	u32 dwHeaderLenth = 0;
	if (dwLen < 12)
	{
		return RPS_ERR_PARAM;
	}
	dwHeaderLenth = (pbyBuf[4]<< 8) + pbyBuf[5];
	if ((4 + dwHeaderLenth) > dwLen)
	{
		return RPS_ERR_PARAM;
	}
	*pPackLen = 6 + dwHeaderLenth;
	return RPS_OK;
}

//解析pes头 [OUT]u32 *pPackLen
static s32 PsReadParsePesHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	if (dwLen < 9)
	{
		return RPS_ERR_PARAM;
	}
	*pPackLen = PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + pbyBuf[8];

	return RPS_OK;
}

