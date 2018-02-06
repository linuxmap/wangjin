/*========================================================================================
模块名    ：ospext.lib
文件名    ：osptool.h
相关文件  ：osp.h xstring.h
实现功能  ：基于OSP的一些工具类和函数
作者      ：fanxg
版权      ：<Copyright(C) 2003-2008 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-------------------------------------------------------------------------------------------
修改记录：
日期               版本              修改人             走读人              修改记录
2011/04/21         V1.0              fanxg                                  新建文件
=========================================================================================*/

#ifndef _OSP_TOOL_H_INCLUDED_
#define _OSP_TOOL_H_INCLUDED_
#include "osp.h"

#ifndef UNREFERENCED_PARA   //pclint:715
#define UNREFERENCED_PARA(P)          (P==P)
#endif

/*
buffer[pos]插入一个字符
pos        buffer的起始位置
free_size  dst_buffer的从pos开始空闲的字节数
value      插入的字符
*/
#define INSERT_CHAR(buffer, pos, free_size, value) \
{\
    if(free_size - 1 >= 1)\
    {\
        buffer[pos] = value;\
        buffer[pos+1] = '\0';\
	    pos += 1;\
	    free_size -= 1;\
    }\
    else\
    {\
        buffer[pos] = '\0';\
    }\
}
/*
拷贝len字节+'\0'
pos        dst_buffer的起始位置
free_size  dst_buffer的从pos开始空闲的字节数
len        不包含截止符的串长度
*/
#define INSERT_C_STRING(dst_buffer, pos, free_size, src_buffer, len) \
{\
    if(free_size - 1 >= len)\
    {\
        memcpy(dst_buffer + pos, src_buffer, len);\
	    pos += len;\
	    dst_buffer[pos] = '\0';\
	    free_size -= len;\
    }\
}

//获取变参字符串  vsnprintfVc6.0 不支持 故使用_vsnprintf
#define GETVALISTSTR(Format, Output) \
            va_list pvlist;\
            va_start(pvlist, Format);\
            int nLen = _vsnprintf((char *)Output,sizeof(Output) -1, Format, pvlist);\
            if( nLen <= 0 || nLen >= (int)sizeof(Output) ) Output[sizeof(Output)-1] = 0;\
            va_end(pvlist);

//时间
struct TOspTimeMs
{
    TOspTimeMs()
    {
        tSecond = 0;
        tMilliSecond = 0;
    }

    time_t tSecond;
    u16 tMilliSecond;
};

//获取当前ms时间
TOspTimeMs OspGetTimeMs();

//如果两个时间间隔超过1193个小时，u32将无法保存，该函数会溢出
//业务程序应该使用该函数获取短时间内的毫秒差
u32 OspGetTimeDiffMs(const TOspTimeMs& tEndTime, const TOspTimeMs& tStartTime);

//根据系统ticks来获取秒级时间，避免依赖时间差的操作因为修改了系统时间而出现异常
u64 OspGetSecondByTicks();

class COspTimeInfo
{
public:
	enum
	{
		MAX_TIME_STRING_LENGTH = 64,
	};
public:
    COspTimeInfo();
    COspTimeInfo(time_t tTime);

public:
    COspTimeInfo(const COspTimeInfo& tObj);
    COspTimeInfo& operator = (const COspTimeInfo& tObj);

    //获取当前时间
    static COspTimeInfo GetCurrTime();

    //获取当前字符串格式时间(精确到秒)
    static u32 GetCurrStrTime(u32 dwBufLen, char *pchBuffer);

    //获取当前字符串格式时间(精确到豪秒)
	static u32 GetCurrStrTime_ms(u32 dwBufLen, char *pchBuffer);

    //时间清零
    void Clear();

    //设置时间
    void SetTime( const time_t *ptTime );

    //得到时间结构
    void GetTime( time_t &tTime ) const;

    //得到时间结构
    time_t GetTime(void) const;

    //时间比较
    BOOL32 operator == ( const COspTimeInfo &tObj ) const;

    //获取时间对应的格式字串
    u32 GetString(u32 dwBuffLen, s8* pchBuff) const;

private:
    u16  m_wYear;   //年
    u8   m_byMonth; //月
    u8   m_byMDay;  //日
    u8   m_byHour;  //时
    u8   m_byMinute;//分
    u8   m_bySecond;//秒
    u16  m_wMillSec;//毫秒
};

//信号量
#define MAX_SEMA_COUNT 100000
class COspSemaphore
{
public:
    COspSemaphore(u32 dwInitCnt = 0, u32 dwMaxCnt = MAX_SEMA_COUNT);
    ~COspSemaphore();

private:
    COspSemaphore(const COspSemaphore&);
    void operator=(const COspSemaphore&);

public:
    BOOL32 Take(u32 dwTimeout = INFINITE);
    BOOL32 Give();

private:
    SEMHANDLE m_hSemaphore;
};

u32 OspGetProcessPath(u32 dwBufferLen, char* pchBuffer);


#endif  //#ifndef _OSP_TOOL_H_INCLUDED_
