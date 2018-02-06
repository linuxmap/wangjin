/*****************************************************************************
   模块名      : KDV type 
   文件名      : kdvtype.h
   相关文件    : 
   文件实现功能: KDV宏定义
   作者        : 魏治兵
   版本        : V3.0  Copyright(C) 2001-2002 KDC, All rights reserved.
-----------------------------------------------------------------------------
   修改记录:
   日  期      版本        修改人      修改内容
   2004/02/17  3.0         魏治兵        创建
******************************************************************************/
#ifndef _KDV_TYPE_H_
#define _KDV_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	  /* Type definition */
/*-----------------------------------------------------------------------------
系统公用文件，开发人员严禁修改
------------------------------------------------------------------------------*/

typedef int		s32,BOOL32;
typedef unsigned long   u32;
typedef unsigned char	u8;
typedef unsigned short  u16;
typedef short           s16;
typedef char            s8;

#ifdef _MSC_VER
typedef __int64			s64;
#else 
typedef long long		s64;
#endif 

#ifdef _MSC_VER
typedef unsigned __int64 u64;
#else 
typedef unsigned long long u64;
#endif

#ifndef _MSC_VER
#ifndef LPSTR
#define LPSTR   char *
#endif
#ifndef LPCSTR
#define LPCSTR  const char *
#endif
#endif



#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef struct tagFrameHeader
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
} FRAMEHDR, *PFRAMEHDR;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _KDV_def_H_ */

/* end of file kdvdef.h */

