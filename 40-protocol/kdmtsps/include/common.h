/*=================================================================================
ģ����:����ģ��
�ļ���:kdmpstscommon.h
����ļ�:
ʵ�ֹ���:
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
    
//��ȫ���ٿռ�
#define TSPSFREE(ptr)   if(NULL != (ptr)) { free(ptr); (ptr) = NULL; }

/************************************************************************/
/* ý�嶨��                                                             */
/************************************************************************/
#define MAX_VIDEO_FRAME_LEN     512*1024
    
//��������
#define PT_STREAM_TYPE_NULL                         (u8)(0x00)
    
//��Ƶ����
#define PT_STREAM_TYPE_MPEG1                        (u8)(0x01)
#define PT_STREAM_TYPE_MPEG2                        (u8)(0x02)
#define PT_STREAM_TYPE_MPEG4                        (u8)(0x10)
#define PT_STREAM_TYPE_H264							(u8)(0x1B)
#define PT_STREAM_TYPE_H265_Old                     (u8)(0xa4)
#define PT_STREAM_TYPE_H265                         (u8)(0x24)
#define PT_STREAM_TYPE_SVACV						(u8)(0x80)
    
//��Ƶ����
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
ģ����:��Ϣ��ʾ
�ļ���:kdmpstscommon.h
����ļ�:
ʵ�ֹ���:������Ϣ��ʾ
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
#define PD_TS_WRITE                 (u8)(1<<1)
#define PD_TS_READ                  (u8)(1<<2)
#define PD_PS_WRITE                 (u8)(1<<3)
#define PD_PS_READ                  (u8)(1<<4)

//�����ӡ�����С1024byte
#define MAX_PT_DEBUG_PRINT          1024

void TspsPrintf(u8 byDebugValue, const s8 *szFormat, ...);

//����debug��ӡֵ��ʹ������Ҫע���osp
//0 -- ����ӡ
//1 -- ��ӡTS�������
//2 -- ��ӡTS�������
//3 -- ��ӡPS�������
//4 -- ��ӡPS�������
//255 -- ��ӡ����
API void tspspd(u8 byDebugValue);

API void tswopen();

/*=================================================================================
ģ����:CRCУ��
�ļ���:crc.h
����ļ�:kdvtspslib.h, kdmpstscommon.h
ʵ�ֹ���:��һ���ַ�����CRCУ��
����:
��Ȩ:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u32 CRCGetCRC32(u8* pu8Data, u32 u32Length);


u8 TsPsPTCovertRtp2Stream(u8 u8RtpType);
u8 TsPsPTCovertStream2Rtp(u8 u8StreamType);

void tspswritelog(char *message, int length);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // KDMPSTSCOMMON_H



