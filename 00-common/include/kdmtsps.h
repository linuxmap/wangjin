/*=================================================================================
模块名:TS/PS模块接口文件
文件名:tsps.h
相关文件:kdvtype.h osp_small.h
实现功能:TS/PS模块接口
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/

#ifndef TSPS_H
#define TSPS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  

/************************************************************************/
/* Tsps(C) version                                                      */
/************************************************************************/
#define TSPS_VERSION    "Tsps(C) for KDM 1.0.0.20081117"
API void tspsver();

/************************************************************************/
/* TS和PS包长度定义                                                     */
/************************************************************************/
#define TS_PACKET_LENGTH                        (188)     //TS包长度
#define PS_PACKET_LENGTH                        (65535)    //PS包最大长度(最大65535)

/************************************************************************/
/* 错误码                                                               */
/************************************************************************/
#define TSPS_OK                                 0
#define ERROR_TSPS_BASE                         (u16)18000
#define TSPS_ERROR(n)                           (u16)(ERROR_TSPS_BASE + n)

//PES函数返回信息
#define ERROR_PES_READ_INPUT_PARAM              TSPS_ERROR(1) //输入参数错误
#define ERROR_PES_READ_CODE_PREFIX              TSPS_ERROR(2) //前缀位错误
#define ERROR_PES_READ_LENGTH                   TSPS_ERROR(3) //pes包长度错误
#define ERROR_PES_READ_FIX_STREAMID             TSPS_ERROR(4) //STREAM ID固定位错误
#define ERROR_PES_READ_FIX_PTS                  TSPS_ERROR(5) //PTS固定位错误
#define ERROR_PES_READ_FIX_PTS_DTS_03           TSPS_ERROR(6) //PTS DTS固定位错误
#define ERROR_PES_READ_FIX_PTS_DTS_01           TSPS_ERROR(7) //PTS DTS固定位错误
#define ERROR_PES_READ_FIX_STD                  TSPS_ERROR(8) //STD固定位错误                                
#define ERROR_PES_READ_TYPE_NOT_SUPPORT         TSPS_ERROR(9) //不支持的格式
#define ERROR_PES_WRITE_INPUT_PARAM             TSPS_ERROR(10) //输入参数错误

//TS函数返回信息
#define ERROR_TS_WRITE_INPUT_PARAM              TSPS_ERROR(101) //输入参数错误
#define ERROR_TS_WRITE_STREAM_NUM               TSPS_ERROR(102) //输入流数错误
#define ERROR_TS_WRITE_PROGRAM_NUM              TSPS_ERROR(103) //输入节目数错误
#define ERROR_TS_WRITE_STREAM_TYPE              TSPS_ERROR(104) //输入流类型错误
#define ERROR_TS_WRITE_INPUT_FRAME              TSPS_ERROR(105) //输入帧错误
#define ERROR_TS_WRITE_NOT_SUPPORT              TSPS_ERROR(106) //不支持
#define ERROR_TS_WRITE_SET_ENCRYPTKEY           TSPS_ERROR(107) //设置密钥错误
#define ERROR_TS_WRITE_MEMORY                   TSPS_ERROR(108) //内存分配错误

#define ERROR_TS_READ_INPUT_PARAM               TSPS_ERROR(201) //输入参数错误
#define ERROR_TS_READ_UNIT_START                TSPS_ERROR(202) //UNIT_START错误
#define ERROR_TS_READ_ADAPTATION                TSPS_ERROR(203) //ADAPTATION错误
#define ERROR_TS_READ_HEAD_LENGTH               TSPS_ERROR(204) //解析头长错误
#define ERROR_TS_READ_PSI_LENGTH                TSPS_ERROR(205) //psi长度错误
#define ERROR_TS_READ_TABLE_ID                  TSPS_ERROR(206) //表id错误
#define ERROR_TS_READ_SECTION_SYNTAX            TSPS_ERROR(207) //段语法错误
#define ERROR_TS_READ_FIX_0                     TSPS_ERROR(208) //固定0比特错误
#define ERROR_TS_READ_NOT_SUPPORT               TSPS_ERROR(209) //不支持当前媒体
#define ERROR_TS_READ_PID_NOT_FOUND             TSPS_ERROR(210) //未在pmt中找到流
#define ERROR_TS_READ_STREAM_TYPE               TSPS_ERROR(211) //流类型错误
#define ERROR_TS_READ_BUFF_FULL                 TSPS_ERROR(212) //帧接收缓冲满
#define ERROR_TS_READ_HEAD_SYNC                 TSPS_ERROR(213) //同步字节错误
#define ERROR_TS_SEGMENT_FILE_ERROR				TSPS_ERROR(214)	// 切片文件打开失败

//PS函数返回信息
#define ERROR_PS_WRITE_INPUT_PARAM              TSPS_ERROR(301) //输入参数错误
#define ERROR_PS_WRITE_STREAM_NUM               TSPS_ERROR(302) //输入流数错误
#define ERROR_PS_WRITE_INPUT_FRAME              TSPS_ERROR(303) //输入帧错误
#define ERROR_PS_WRITE_STREAM_TYPE              TSPS_ERROR(304) //输入流类型错误

#define ERROR_PS_READ_INPUT_PARAM               TSPS_ERROR(401) //输入参数错误
#define ERROR_PS_READ_PARSE_FAIL                TSPS_ERROR(402) //流数据解析失败
#define ERROR_PS_READ_BUFF_FULL                 TSPS_ERROR(403) //解码pes缓冲满


/************************************************************************/
/* PsTs接口                                                             */
/************************************************************************/

//句柄声明
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name;
#endif

DECLARE_HANDLE(HTspsWrite);
DECLARE_HANDLE(HTspsRead);

//流类型，ps或ts
typedef enum tagEStreamType
{
    TRANSPORT_STREAM,
    PROGRAM_STREAM
} EStreamType;

#define TspsGetStreamType(handle)       (*(EStreamType *)handle)
//帧数据结构，由于不能包含c++库，重新定义，与medianet相同
typedef struct tagTspsFrameHeader
{
	u8     m_byMediaType; /*媒体类型*/
    u8    *m_pbyData;       /*数据缓冲*/
    u32    m_dwPreBufSize;/*m_pData缓冲前预留了多少空间，用于加*/
	/* RTP option的时候偏移指针一般为12+4+12*/
	/* (FIXED HEADER + Extence option + Extence bit)*/
    u32    m_dwDataSize;  /*m_pData指向的实际缓冲大小缓冲大小*/
    u8     m_byFrameRate; /*发送帧率,用于接收端*/
    u32    m_dwFrameID;   /*帧标识，用于接收端*/
    u32    m_dwTimeStamp; /*时间戳, 用于接收端*/
    u32    m_dwSSRC;      /*同步源, 用于接收端*/
    union
    {
		
        struct{
			BOOL32    m_bKeyFrame;    /*频帧类型（I or P）*/
			u16       m_wVideoWidth;  /*视频帧宽*/
			u16       m_wVideoHeight; /*视频帧宽*/
		}m_tVideoParam;
        u8    m_byAudioMode;/*音频模式*/
    }x;
	u8   m_byStreamID;
} TspsFRAMEHDR, *PTspsFRAMEHDR;

//回调函数声明
typedef u16 (*TspsFrameCallback)(TspsFRAMEHDR *ptFrame, KD_PTR pvContext);
typedef u16 (*TspsProgramCallback)(u8 u8VideoPT, u8 u8AudioPT0, u8 u8AudioPT1, KD_PTR pvContext);
typedef u16 (*TspsSectionCallback)(u8 *pu8Buf, u32 u32Len, KD_PTR pvContext, TspsFRAMEHDR *ptFrame);

//write
HTspsWrite TspsWriteOpen(EStreamType eType, TspsSectionCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize);
u16 TspsWriteClose(HTspsWrite hWrite);
u16 TspsWriteSetProgram(HTspsWrite hWrite, u8 u8VideoType, u8 u8AudioType);

/** 设置TS切片文件参数
*	u32SegmentDuration 切片片段时间
*	pu8FileDir	切片文件、索引文件目录（NULL为执行程序当前目录），应该为apache发布目录，即pu8HttpPrefix目录
*	pu8OutputPrefix 切片输出文件前缀
*	pu8IndexFileName 索引文件名称（m3u8）
*	pu8HttpPrefix http前缀
*	u32SegmentWindow 切片窗口（0为不设置窗口）  
*/
u16 TsSetSegmentParam(HTspsWrite hWrite, const u32 u32SegmentDuration, const s8* pu8FileDir, const s8* pu8OutputPrefix,
					  const s8* pu8IndexFileName, const s8* pu8HttpPrefix, const u32 u32SegmentWindow );

u16 TspsWriteWriteFrame(HTspsWrite hWrite, TspsFRAMEHDR *ptFrame);
u16 TspsSetEncryptKey(HTspsWrite hWrite, s8 *pszKeyBuf, u16 wKeySize, s8* pszIV, s8 *pszUrlBuf, u16 wUrlLen);
u16 TspsWriteWriteEnd(HTspsWrite hWrite);

//read
HTspsRead TspsReadOpen(EStreamType eType, TspsFrameCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize);
u16 TspsReadClose(HTspsRead hRead);
u16 TspsReadSetProgramCallback(HTspsRead hRead, TspsProgramCallback pfCallback, KD_PTR pvContext);
u16 TspsReadInputStream(HTspsRead hRead, u8 *pu8Buf, u32 u32Len);
u16 TspsReadResetStream(HTspsRead hRead);
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif //TSPS_H








