/*=================================================================================
模块名:公共模块
文件名:streamdef.h
相关文件:
实现功能:
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#ifndef STREAMDEF_H
#define STREAMDEF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/*=================================================================================
模块名:PES包的读写
文件名:pes.ch
相关文件:kdvtspslib.h, kdmpstscommon.h
实现功能:PES包的读写
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/

// 增加高清支持，最大帧长度可达512k，实际pes最大包长度为512k+19，ligeng@2012.10.25
#define ES_PACEKT_MAX_LENGTH		512*1024+19	// 定义ES流最大帧长度(512k)，再加上pes头，额外加上19字节	
//PES有关宏定义
#define PES_PACKET_MAX_LENGTH       65535   //PES包(负载)最大长度
#define PES_DATA_HEAD_LENGTH        6       //PES包头长度(固定6字节)
#define PES_ES_INFO_LENGTH          3       //ES流特有信息(固定3字节)
#define PES_PACK_MAX_LEN            (512*1024)//(PES_PACKET_MAX_LENGTH + PES_DATA_HEAD_LENGTH)

//PES包STREAM ID
#define PROGRAM_STREAM_MAP          0xBC    //节目映射流
#define PADDING_STREAM              0xBE    //填充流
#define PRIVATE_STREAM_1            0xBD    //私有流
#define PRIVATE_STREAM_2            0xBF    //私有流
#define AUDIO_STREAM0               0xC0    //音频流0
#define AUDIO_STREAM1               0xC1    //音频流1
#define VIDEO_STREAM                0xE0    //音频流
#define ECM_STREAM                  0xF0    //ECM流
#define EMM_STREAM                  0xF1    //EMM流
#define DSM_CC_STREAM               0xF2    //DSM流
#define ISOIEC_13522_STREAM         0xF3    //13522流
#define RESERVED_DATA_STREAM        0xF0    //保留数据流
#define PROGRAM_STREAM_DIRECTORY    0xFF    //节目流路径

u8 TsPsGetStreamIdPrefix(u8 u8StreamType);
    

//PES结构定义
typedef struct tagPesInfo
{
    u64 u64Pts;                                         //PTS
    u64 u64Dts;                                         //DTS
    u64 u64EscrBase;                                    //参考时钟(90khz)
    
    u32 u32PacketLength;                                //包长度(从此字段最后一位算起,不包括该位)
    u32 u32EsRate;                                      //码率
    
    u32 u32PayloadLength;                               //ES流长度
    u32 u32PesLength;                                    //PES包长度
    
    u16 u16EscrExtension;                                //参考时钟(90Mhz)
    u16 u16PreviousPesPacketCrc;                         //没有使用
    u16 u16PStdBufferSize;                               //没有使用
    
    u8 u8StreamId;                                      //流号
    u8 u8ScramblingControl;                             //加密控制
    u8 u8Priority;                                      //优先级
    u8 u8DataAlignment;                                 //数据对齐标志
    u8 u8Copyrigth;                                     //版权
    u8 u8OriginalOrCopy;                                //原始数据/拷贝
    u8 u8PtsDtsFlag;                                    //PTS,DTS信息标志
    u8 u8EscrFlag;                                      //ESCR标志
    u8 u8EsRateFlag;                                    //编码流码率标志
    u8 u8DsmTrickModeFlag;                              //DSM策略模式
    u8 u8AdditionalCopyInfoFlag;                        //额外信息
    u8 u8CrcFlag;                                       //CRC标志
    u8 u8ExtensionFlag;                                 //拓展标志
    u8 u8HeadDataLength;                                //头长度

	BOOL32  bWriteDts;  //该帧是否写入DTS， 写入再标记为有效，需在回调之后，回调的是前一帧
	BOOL32  bDtsValid;  //该帧dts是否有效
    //以下特性没有使用
    u8 u8TrickModeControl;                              //策略控制
    u8 u8FieldId;
    u8 u8IntraSliceRefresh;
    u8 u8FrequencyTruncation;
    u8 u8FieldRepCntrl;
    u8 u8AdditionalCopyInfo;                            //额外信息
    
    u8 u8PrivateDataFlag;
    u8 u8PackHeaderFieldFlag;
    u8 u8ProgramPackSequenceCounterFlag;
    u8 u8PStdBufferFlag;
    u8 u8ExtensionFlag2;
    u8 u8PrivateData[128 / 8];
    u8 u8PackFieldLength;
    u8 u8ProgramPacketSequenceCounter;
    u8 u8Mpeg1Mpeg2Identifier;
    u8 u8OriginalStuffLength;
    u8 u8PStdBufferScale;
    u8 u8ExtensionFieldLength;
    
    u8 *pu8PayloadBuffer;                               //ES流指针
    u8 *pu8PesBuffer;                                    //PES包指针
}TPesInfo;

/*=================================================================================
函数名:PESReadInfo
功能:分析PES包内容,获得ES流
算法实现: 
参数说明:
         [I/O] ptpPesInfo 存储PES包信息的结构
         [I]   pu8BufInput PES包
         [I]   u32LenInput PES包长度
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 PesReadInfo(TPesInfo *ptPesInfo, u8 *pu8BufInput, u32 u32LenInput, u32* u32LenOutput);

/*=================================================================================
函数名:PESWriteInfo
功能:将一帧ES流写入PES包
算法实现: (在进行PES打包过程时,必须填写正确的stream id,DTS或者PTS(必须))
参数说明:
         [I/O] ptpPesInfo 存储PES包信息的结构
         [I/O] pu8BufOutput PES包
         [I/O] u32LenOutput PES包长度
返回值说明:成功返回TSPS_OK,否则返回错误号
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 PesWriteInfo(TPesInfo *ptPesInfo, u8 *pu8BufOutput, u32 *pu32LenOutput);



/*=================================================================================
模块名:PS流读写
文件名:ps.h
相关文件:kdvtspslib.h, kdmpstscommon.h
实现功能:PS流读写
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#define PACK_HEADER_START_CODE              0x000001BA  //PS包头开始标记
#define SYSTEM_HEADER_START_CODE            0x000001BB  //PS系统头开始标记
#define MAP_START_CODE                      0x000001BC  //映射段头开始标记
#define STREAM_END_CODE                     0x000001B9  //流开始标记

#define PS_HEAD_BYTE                        (u8)(PACK_HEADER_START_CODE   & 0xff)
#define PS_SYSTEM_HEAD_BYTE                 (u8)(SYSTEM_HEADER_START_CODE & 0xff)
#define PS_MAP_BYTE                         (u8)(MAP_START_CODE           & 0xff)
#define PS_END_BYTE                         (u8)(STREAM_END_CODE          & 0xff)

#define MAX_STREAM_NUM_IN_PROGRAM           4           //最大的流数量

#define PS_SYSHEAD_BUFSCALE_0               128
#define PS_SYSHEAD_BUFSCALE_1               1024

//ps头长度
#define PS_HEAD_LEN             14
//pes数据前头的长度 = pes头长 + pes中es流字段长度 + PTS和DTS字段长度
#define PS_PES_PREDATA_LEN      (PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + 10)
//切分每段es的长度 = ps每段长度 - ps头长 - pes数据前头的长度
#define PS_ES_CUT_LEN   (PS_PACKET_LENGTH - PS_HEAD_LEN - PS_PES_PREDATA_LEN)       

//PS流信息
typedef struct tagPsProgramInfo
{
    u8  au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];
    u8  au8StreamType[MAX_STREAM_NUM_IN_PROGRAM];       //流类型
    u8  u8StreamNum;                                    //此节目中有多少路流
} TPsProgramInfo;

//PS头结构定义
typedef struct tagPsHeadInfo
{
    u64   u64SCRBase;                                   //时间基准
    u16   u16SCRExt;
    
    u32   u32ProgramMuxRate;                            //流复合码率
    
    u8   *pu8Buffer;                                    //存储PS头的缓冲 
    u32   u32BuffLength;                                //PS头长度
} TPsHeadInfo;

//系统头结构定义
typedef struct tagPsSysHeaderInfo
{
    u16   u16HeaderLength;                              //头长度

    u32   u32RateBound;
    u8    u8AudioBound;
    u8    u8FixedFlag;
    u8    u8CspsFlag;
    u8    u8SystemAudioLockFlag;
    u8    u8SystemVideoLockFlag;
    u8    u8VideoBound;
    u8    u8PacketRateRestrictionFlag;

    u8    au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];       //流ID
    u8    au8PStdBufferBoundScale[MAX_STREAM_NUM_IN_PROGRAM];
    u16   au16PStdBufferSizeBound[MAX_STREAM_NUM_IN_PROGRAM];

    u8    u8StreamNum;
    u8   *pu8Buffer;                                    //存储PS系统头的缓冲
    u32   u32BuffLength;                                //PS系统头长度
} TPsSysHeaderInfo;

//映射段结构定义
typedef struct tagPsMapInfo
{
    u16   u16MapLength;                                 //映射段长度
    u8    u8CurrentNextIndicator;                       //该段是否有效
    u8    u8Version;                                    //版本号
    u16   u16PsInfoLength;
    u16   u16EsMapLength;                               //ES映射长度

    u8    au8StreamType[MAX_STREAM_NUM_IN_PROGRAM];     //流ID
    u8    au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];       //原始流所在PES分组的STREAM_ID
    u16   au16EsInfoLength[MAX_STREAM_NUM_IN_PROGRAM];  //ES描述长度
    
    u8    u8StreamNum;                                  //流数量
    u8   *pu8Buffer;                                    //存储映射段的缓冲
    u32   u32BuffLength;                                //映射段长度
} TPsMapInfo;

// ps write——内存管理策略：全部使用pswrite中分配的内存
typedef struct tagPsWrite
{
    TPsHeadInfo tHead;
    TPsSysHeaderInfo tSysHead;
    TPsMapInfo tMap;
    
    TspsSectionCallback pfCallback;                    //回调函数
    void *pvContext;                                   //回调上下文指针

    u8 *pu8Buf;                                        //ps frame buf
    u32 u32Len;                                        //缓存长度

    TPesInfo *ptPesInfo;

    //一个ps流我们只加入一对音频和视频
    u8 u8VideoType;
    u8 u8AudioType;
    u32 u32LastTimestamp;

//记录上层传递下来的最大帧数
    u32 dwmaxframesize;
} TPsWrite;

// ps read
typedef struct tagPsRead
{
    TPsHeadInfo tHead;
    TPsSysHeaderInfo tSysHead;
    TPsMapInfo tMap;
    
    TspsFrameCallback pfFrameCB;                         //帧回调函数
    void *pvFrameContext;                                //帧回调上下文指针
    TspsProgramCallback pfProgramCB;                     //节目信息回调函数
    void *pvProgramContext;                              //节目信息回调上下文指针
    
    u8 *pu8FrameBuf;                                     //帧缓冲
    u32 u32FrameLen;                                     //帧长度

    u8 *pu8InBuf;
    u32 u32InLen;
    
    TspsFRAMEHDR *ptFrame;
    TPesInfo *ptPesInfo;

    BOOL32 bNotSupport;    //如果超过两个流且不是一个音频一个视频，暂时不支持解析
    u8 u8VideoID;
    u8 u8AudioID0;
	u8 u8AudioID1;
    u8 u8VideoType;
    u8 u8AudioType0;
	u8 u8AudioType1;
    u32 u32LastTS;
    u8 u8LastType;
	u8 u8LastStreamID;

	u32 u32Width;
	u32 u32Height;

	BOOL32 bReadHead;

	BOOL32 bFirstPacket;

	u32    dwVideoFrameID;
	u32    dwAudioFrameID0;
	u32    dwAudioFrameID1;

	u32    dwmaxframesize;
    
} TPsRead;

/*=================================================================================
模块名:TS流的读写
文件名:ts.h
相关文件:kdvtspslib.h, kdmpstscommon.h
实现功能:TS流的读写
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
//TS包头标记
#define TS_PACKET_SYNC          0x47
#define TS_HEADER_LEN           4

#define TS_SEND_PSI_STEP        40      //隔多少包发送一次psi信息

#define MAX_SECTION_LENGTH      4 //一个PSI信息表中最多有多少段(8位)
#define MAX_PROGRAM_MAP_NUM     MAX_SECTION_LENGTH //PAT信息表可以保存多少PMT
#define MAX_STREAM_NUM          MAX_SECTION_LENGTH //PMT信息表中有可以保存多少流信息

//各种PSI信息表ID
#define TS_PAT_TABLE_ID         0x00    //PAD表ID
#define TS_PMT_TABLE_ID         0x02    //PMT表ID
#define TS_SDT_ID               0x42
#define TS_NULL_PACKET_ID       0x1fff//空包

#define TS_AFC_TYPE_PAYLOAD     0x01
#define TS_AFC_TYPE_AF          0x02
#define TS_AFC_TYPE_AF_PAYLOAD  0x03

#define TS_RAND_MAX             8192    //随机数最大值

#define TS_SEGMENT_FILE_LEN				128		//切片文件名称长度

//流属性(形成TS流时的初始化信息,一个结构代表一个PMT结构所需要的信息)
typedef struct tagTsProgramInfo
{
    u16 au16StreamPid[MAX_PROGRAM_MAP_NUM];
    u8  au8StreamType[MAX_PROGRAM_MAP_NUM];              //流类型
    u8  u8StreamNum;                                     //此节目中有多少路流
}TTsProgramInfo;

typedef struct tagTsPatPrograms 
{
    TTsProgramInfo atProgram[MAX_STREAM_NUM];
    u8 u8ProgramNum;
} TTsPatPrograms;

//PAT表结构定义
typedef struct tagTsPatInfo
{
    u32 u32Crc;
    
    u16 u16SectionLength;                                //段长度
    u16 u16TransportStreamId;                            //流ID
    
    u16 au16ProgramNumber[MAX_PROGRAM_MAP_NUM];          //节目号
    u16 au16NetworkPid[MAX_PROGRAM_MAP_NUM];             //网络PID
    u16 au16ProgramMapPid[MAX_PROGRAM_MAP_NUM];          //PMT表PID
    
    u8 u8VersionNumber;                                  //版本号
    u8 u8CurrentNextIndicator;                           //此包是否有效
    u8 u8SectionNumber;                                  //段号
    u8 u8LastSectionNumber;                              //最大的段号
    u8 u8ProgramMapNum;                                  //PAT表中有多少个PMT
    u8 u8ContinuityCounter;                              //记数(0x0-0xf循环)
    
    u8 *pu8Buffer;                                       //存储PAT包的缓冲
}TTsPatInfo;

//PMT表结构定义
typedef struct tagTsPmtInfo
{
    u32 u32Crc;
    
    u16 u16SectionLength;                                //段长度
    u16 u16ProgramNumber;                                //PMT PID
    u16 u16PcrPid;                                       //PCR基准的流PID
    u16 u16ProgramInfoLength;                            //存储信息长度
    
    u16 au16ElementaryPid[MAX_STREAM_NUM];               //流PID
    u16 au16EsInfoLength[MAX_STREAM_NUM];                //流描述信息长度
    
    u8 u8VersionNumber;                                  //版本号
    u8 u8CurrentNextIndicator;                           //此包是否有效
    u8 u8SectionNumber;                                  //段号
    u8 u8LastSectionNumber;                              //最大的段号
    
    u8 au8StreamType[MAX_STREAM_NUM];                    //流类型
    
    u8 u8StreamNum;                                      //PMT表中有多少个流
    u8 u8ContinuityCounter;                              //记数(0x0-0xf循环)
    
    u8 *pu8Buffer;                                       //存储PMT包的缓冲
}TTsPmtInfo;

//ts头信息
typedef struct tagTsHeader
{
    u16 u16Pid;                                          //PID值
    u8 u8TransportErrorIndicator;                        //传输错误标记
    u8 u8PayloadUnitStartIndicator;                      //是否有一帧的第一个字节
    u8 u8TransportPriority;                              //传输优先级
    u8 u8TransportScramblingControl;                     //是否加密
    u8 u8AdaptationFieldControl;                         //调整字段
    u8 u8ContinuityCounter;                              //记数(0x0-0xf循环)
    u8 u8AdaptationFieldLength;
    u8 u8AdaptationFlags;
    u8 u8HeadLen;
} TTsHeader;

//切片结构
typedef struct tagTsSegment
{
	u32 u32LastSegmentTime;				// 最后一次切割时间
	u32 u32FirstSegment;			// 索引文件中第一个切片记数
	u32 u32LastSegment;				// 索引文件中最后一个切片记数
	u32 u32SegmentDuration;				// 分片时间间隔（默认取10s）
	/*u8 aau8FileDir[TS_SEGMENT_FILE_LEN];	// 索引文件、分片文件路径，应该以 / 结尾*/
	s8 aau8IndexFileName[TS_SEGMENT_FILE_LEN];		// 索引文件名称（m3u8）
	s8 aau8OutputFilePrefix[TS_SEGMENT_FILE_LEN];	// 输出切片文件前缀
	s8 aau8HttpPrefix[TS_SEGMENT_FILE_LEN];		// http地址前缀
	u32 u32SegmentWindow;			// 切片窗口（0为不设置）
	FILE *fpSegment;		// 切片文件指针
}TTsSegment;

#ifndef DES_ENCRYPT_MODE
#define DES_ENCRYPT_MODE         (u8)0      //DES加密模式
#define AES_ENCRYPT_MODE         (u8)1      //AES加密模式
#define ENCRYPT_KEY_SIZE         (u8)32     //密钥长度 取其中的较大值
#define DES_ENCRYPT_KEY_SIZE     (u8)8      //DES密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_A (u8)16  //AES Mode-A 密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_B (u8)24  //AES Mode-B 密钥长度
#define AES_ENCRYPT_KEY_SIZE_MODE_C (u8)32  //AES Mode-C 密钥长度
#define AES_ENCRYPT_BYTENUM    (s32)16// DES 加密为16字节对齐
#endif

//TS写结构定义
typedef struct tagTsWrite
{
    //以下计算PCR使用
    u64 u64PCRBase;                                      //基准时间
    u16 u16PCRExt;
    
    TTsHeader tHeader;

    u16 au16Pid[MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM];   //记录已获得的PID,避免PID重复
    
    TTsPatInfo tPatInfo;
    TTsPmtInfo *ptPmtInfo;
    
    u8 aau8ContinuityCounter[MAX_PROGRAM_MAP_NUM][MAX_STREAM_NUM]; //每个流的记数

    u8 *pu8TsBuf;                                        //TS包缓冲

    TPesInfo *ptPesInfo;

    u8 *pu8PesBuf;                                       //PES包缓冲
    u32 u32Peslen;                                       //PES包长度
    
    TspsSectionCallback pfCallback;                      //回调函数
    void *pvContext;                                     //回调上下文指针

    u32 u32TsPesCount;                                   //记录section数，固定间隔加psi

    //一个ts流我们只加入一对音频和视频
    u8 u8VideoType;
    u8 u8AudioType;

	TTsSegment *ptTsSegment;

	SEMHANDLE hWriteSem;		// 写ts文件需要加锁，上次会有音视频两个线程进行写文件

	s8 *pszKeyBuf;
	u16 wKeySize;
	s8 szIV[16];
	s8 szFirstIV[16];
	s8 *pszUrlBuf;
	u16 wUrlLen;
	u8 byTempEncryptBuf[176];
	u16 wTempEncryptLen;
} TTsWrite;

typedef struct  tagTsInfo
{
	u8 *pu8PesBuf;
	u32 u32Peslen;
	TPesInfo *ptPesInfo;
	u16 u16CurPesLen;
	BOOL32 bWaitNextPes;
	u8 u8Type;
	u8 u8LastType;
	u32 u32FrameID;
}TTsInfo;

//TS读结构体
typedef struct tagTsRead
{
    TTsHeader tHeader;

    TTsPatInfo tPatInfo;
    TTsPmtInfo *ptPmtInfo;

    TspsFrameCallback pfFrameCB;                         //帧回调函数
    void *pvFrameContext;                                //帧回调上下文指针
    TspsProgramCallback pfProgramCB;                     //节目信息回调函数
    void *pvProgramContext;                              //节目信息回调上下文指针
	BOOL32 bSupport;    //如果超过两个流且不是一个音频一个视频，暂时不支持解析
	TspsFRAMEHDR *ptFrame;

	TTsInfo  atTsInfo[MAX_PROGRAM_MAP_NUM];
	u32 u32Width;
	u32 u32Height;

	BOOL32 bFirstPack;
	u8     byTempTSBuf[188];
	u32    dwTempBufLen;


//     u8 *pu8VideoPesBuf;                                       //视频PES包缓冲
//     u32 u32VideoPeslen;                                       //视频PES包长度
// 
// 	
//     u8 *pu8AudioPesBuf;                                       //音频PES包缓冲
//     u32 u32AudioPeslen;                                       //音频PES包长度
// 
// 
//     
//     TPesInfo *ptVideoPesInfo;
// 	TPesInfo *ptAudioPesInfo;
//     u16 u16CurVideoPesLen;
// 	u16 u16CurAudioPesLen;
//     BOOL32 bVideoWaitNextPes;
// 	BOOL32 bAudioWaitNextPes;
// 
//     
//     u8 u8VideoType;
//     u8 u8AudioType;
//     u8 u8LastVideoType;
// 	u8 u8LastAudioType;
// 
// 	u32 u32VideoFrameID;
// 	u32 u32AudioFrameID;

} TTsRead;

/************************************************************************/
/* internal API                                                         */
/************************************************************************/

// write to ts
TTsWrite *TsWriteOpen(TspsSectionCallback pfCallback, void *pvContext);
u16 TsWriteClose(TTsWrite *ptTsInfo);
u16 TsWriteSetProgram(TTsWrite *ptTsInfo, u8 u8VideoType, u8 u8AudioType);
u16 TsWriteSegmentParam(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment);
u16 TsWriteWriteEsFrame(TTsWrite *ptTsInfo, TspsFRAMEHDR *ptFrame);
u16 TsWriteSetEncryptKey(TTsWrite *ptTsInfo, s8 *pszKeyBuf, s8* pszIV, u16 wKeySize, s8 *pszUrlBuf, u16 wUrlLen);
u16 TsWriteEncryptBuffer(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment, u8* pbyBuffer, u32 dwBuffLen);

// write to ps
TPsWrite *PsWriteOpen(TspsSectionCallback pfCallback, void *pvContext, u32 dwMaxFrameSize);
u16 PsWriteClose(TPsWrite *ptPsInfo);
u16 PsWriteSetProgram(TPsWrite *ptPsInfo, u8 u8VideoType, u8 u8AudioType);
u16 PsWriteWriteEsFrame(TPsWrite *ptPsInfo, TspsFRAMEHDR *ptFrame);
u16 PsWriteWriteEnd(TPsWrite *ptPsInfo);

// read from ts
TTsRead *TsReadOpen(TspsFrameCallback pfCallback, void *pvContext);
u16 TsReadClose(TTsRead *ptTsInfo);
u16 TsReadSetProgramCallback(TTsRead *ptTsInfo, TspsProgramCallback pfCallback, void *pvContext);
u16 TsReadInputStream(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len);
u16 TsGetArrayByStreamType(TTsRead *ptTsInfo, u8 u8StreamType, u8* pu8Id);

// read from ps
TPsRead *PsReadOpen(TspsFrameCallback pfCallback, void *pvContext, u32 dwMaxFrameSize);
u16 PsReadClose(TPsRead *ptPsInfo);
u16 PsReadSetProgramCallback(TPsRead *ptPsInfo, TspsProgramCallback pfCallback, void *pvContext);
u16 PsReadInputStream(TPsRead *ptPsInfo, u8 *pu8Buf, u32 u32Len);
u16 PsReadResetStream(TPsRead *ptPsInfo);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // TSPS_H

