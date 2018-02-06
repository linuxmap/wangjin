/*=================================================================================
模块名:公共模块
文件名:kdmpstscommon.h
相关文件:
实现功能:
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#ifndef KDMPSTSCOMMON_H
#define KDMPSTSCOMMON_H
#include "kdvtype.h"

#include "osp.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "kdvdef.h"
// #include "osp_c.h"


#include "bits.h"
    
//安全销毁空间
#define TSPSFREE(ptr)   if(NULL != (ptr)) { free(ptr); (ptr) = NULL; }

/************************************************************************/
/* 媒体定义                                                             */
/************************************************************************/
#define MAX_VIDEO_FRAME_LEN     512*1024
    
//负载类型
#define PT_STREAM_TYPE_NULL                         (u8)(0x00)
    
//视频类型
#define PT_STREAM_TYPE_MPEG1                        (u8)(0x01)
#define PT_STREAM_TYPE_MPEG2                        (u8)(0x02)
#define PT_STREAM_TYPE_MPEG4                        (u8)(0x10)
#define PT_STREAM_TYPE_H264							(u8)(0x1B)
#define PT_STREAM_TYPE_H265_Old                     (u8)(0xa4)
#define PT_STREAM_TYPE_H265                         (u8)(0x24)
#define PT_STREAM_TYPE_SVACV						(u8)(0x80)
    
//音频类型
#define PT_STREAM_TYPE_MP1                          (u8)(0x03)
#define PT_STREAM_TYPE_MP2                          (u8)(0x04)

#define PT_STREAM_TYPE_AACLC                        (u8)(0x11)
#define PT_STREAM_TYPE_MP2AAC						(u8)(0x0f)
	
#define PT_STREAM_TYPE_G711A                        (u8)(0x90)
#define PT_STREAM_TYPE_G7221                        (u8)(0x92)
#define PT_STREAM_TYPE_G7231                         (u8)(0x93)
#define PT_STREAM_TYPE_G729                         (u8)(0x99)
#define PT_STREAM_TYPE_SVACA                        (u8)(0x9B)
    

/*=================================================================================
模块名:信息显示
文件名:kdmpstscommon.h
相关文件:
实现功能:调试信息显示
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#define PD_TS_WRITE                 (u8)(1<<1)
#define PD_TS_READ                  (u8)(1<<2)
#define PD_PS_WRITE                 (u8)(1<<3)
#define PD_PS_READ                  (u8)(1<<4)

//定义打印缓冲大小1024byte
#define MAX_PT_DEBUG_PRINT          1024

void TspsPrintf(u8 byDebugValue, const s8 *szFormat, ...);

//设置debug打印值，使用者需要注册进osp
//0 -- 不打印
//1 -- 打印TS编码过程
//2 -- 打印TS解码过程
//3 -- 打印PS编码过程
//4 -- 打印PS解码过程
//255 -- 打印所有
API void tspspd(u8 byDebugValue);

API void tswopen();

/*=================================================================================
模块名:CRC校验
文件名:crc.h
相关文件:kdvtspslib.h, kdmpstscommon.h
实现功能:对一串字符进行CRC校验
作者:
版权:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u32 CRCGetCRC32(u8* pu8Data, u32 u32Length);


u8 TsPsPTCovertRtp2Stream(u8 u8RtpType);
u8 TsPsPTCovertStream2Rtp(u8 u8StreamType);

void tspswritelog(char *message, int length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // KDMPSTSCOMMON_H



