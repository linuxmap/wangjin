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
