/*========================================================================================
ģ����    ��ospext.lib
�ļ���    ��osptool.h
����ļ�  ��osp.h xstring.h
ʵ�ֹ���  ������OSP��һЩ������ͺ���
����      ��fanxg
��Ȩ      ��<Copyright(C) 2003-2008 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-------------------------------------------------------------------------------------------
�޸ļ�¼��
����               �汾              �޸���             �߶���              �޸ļ�¼
2011/04/21         V1.0              fanxg                                  �½��ļ�
=========================================================================================*/

#ifndef _OSP_TOOL_H_INCLUDED_
#define _OSP_TOOL_H_INCLUDED_
#include "osp.h"

#ifndef UNREFERENCED_PARA   //pclint:715
#define UNREFERENCED_PARA(P)          (P==P)
#endif

/*
buffer[pos]����һ���ַ�
pos        buffer����ʼλ��
free_size  dst_buffer�Ĵ�pos��ʼ���е��ֽ���
value      ������ַ�
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
����len�ֽ�+'\0'
pos        dst_buffer����ʼλ��
free_size  dst_buffer�Ĵ�pos��ʼ���е��ֽ���
len        ��������ֹ���Ĵ�����
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

//��ȡ����ַ���  vsnprintfVc6.0 ��֧�� ��ʹ��_vsnprintf
#define GETVALISTSTR(Format, Output) \
            va_list pvlist;\
            va_start(pvlist, Format);\
            int nLen = _vsnprintf((char *)Output,sizeof(Output) -1, Format, pvlist);\
            if( nLen <= 0 || nLen >= (int)sizeof(Output) ) Output[sizeof(Output)-1] = 0;\
            va_end(pvlist);

//ʱ��
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

//��ȡ��ǰmsʱ��
TOspTimeMs OspGetTimeMs();

//�������ʱ��������1193��Сʱ��u32���޷����棬�ú��������
//ҵ�����Ӧ��ʹ�øú�����ȡ��ʱ���ڵĺ����
u32 OspGetTimeDiffMs(const TOspTimeMs& tEndTime, const TOspTimeMs& tStartTime);

//����ϵͳticks����ȡ�뼶ʱ�䣬��������ʱ���Ĳ�����Ϊ�޸���ϵͳʱ��������쳣
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

    //��ȡ��ǰʱ��
    static COspTimeInfo GetCurrTime();

    //��ȡ��ǰ�ַ�����ʽʱ��(��ȷ����)
    static u32 GetCurrStrTime(u32 dwBufLen, char *pchBuffer);

    //��ȡ��ǰ�ַ�����ʽʱ��(��ȷ������)
	static u32 GetCurrStrTime_ms(u32 dwBufLen, char *pchBuffer);

    //ʱ������
    void Clear();

    //����ʱ��
    void SetTime( const time_t *ptTime );

    //�õ�ʱ��ṹ
    void GetTime( time_t &tTime ) const;

    //�õ�ʱ��ṹ
    time_t GetTime(void) const;

    //ʱ��Ƚ�
    BOOL32 operator == ( const COspTimeInfo &tObj ) const;

    //��ȡʱ���Ӧ�ĸ�ʽ�ִ�
    u32 GetString(u32 dwBuffLen, s8* pchBuff) const;

private:
    u16  m_wYear;   //��
    u8   m_byMonth; //��
    u8   m_byMDay;  //��
    u8   m_byHour;  //ʱ
    u8   m_byMinute;//��
    u8   m_bySecond;//��
    u16  m_wMillSec;//����
};

//�ź���
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
