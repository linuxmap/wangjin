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
   2013/08/09  4.0         吴彬          扩展支持64位
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
//定长类型
typedef char     s8;
typedef unsigned char   u8;
typedef short           s16;
typedef unsigned short  u16;
typedef int    s32;


#ifdef _MSC_VER
    typedef __int64            s64;
    typedef unsigned __int64 u64;
#else
    typedef long long        s64;
    typedef unsigned long long u64;
#endif

	//兼容32流的 u32 定义
#if defined(_WIN64) || defined(WIN64) ||defined (__LP64__) || defined (__64BIT__) || defined (_LP64) || (__WORDSIZE == 64)
	typedef unsigned int    u32;
    #define KD_PTR          void*
#else
	typedef unsigned long    u32;
    #define KD_PTR           u32
#endif

typedef s32             BOOL32;
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

#ifndef _MSC_VER
#ifndef LPSTR
#define LPSTR   char *
#endif
#ifndef LPCSTR
#define LPCSTR  const char *
#endif
#endif


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _KDV_def_H_ */

/* end of file kdvdef.h */

