/*****************************************************************************
   ģ����      : KDV type 
   �ļ���      : kdvtype.h
   ����ļ�    : 
   �ļ�ʵ�ֹ���: KDV�궨��
   ����        : κ�α�
   �汾        : V3.0  Copyright(C) 2001-2002 KDC, All rights reserved.
-----------------------------------------------------------------------------
   �޸ļ�¼:
   ��  ��      �汾        �޸���      �޸�����
   2004/02/17  3.0         κ�α�        ����
******************************************************************************/
#ifndef _KDV_TYPE_H_
#define _KDV_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	  /* Type definition */
/*-----------------------------------------------------------------------------
ϵͳ�����ļ���������Ա�Ͻ��޸�
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
	u8     m_byMediaType; /*ý������*/
    u8    *m_pbyData;       /*���ݻ���*/
    u32    m_dwPreBufSize;/*m_pData����ǰԤ���˶��ٿռ䣬���ڼ�*/
                          /* RTP option��ʱ��ƫ��ָ��һ��Ϊ12+4+12*/
                          /* (FIXED HEADER + Extence option + Extence bit)*/
    u32    m_dwDataSize;  /*m_pDataָ���ʵ�ʻ����С�����С*/
    u8     m_byFrameRate; /*����֡��,���ڽ��ն�*/
    u32    m_dwFrameID;   /*֡��ʶ�����ڽ��ն�*/
    u32    m_dwTimeStamp; /*ʱ���, ���ڽ��ն�*/
    u32    m_dwSSRC;      /*ͬ��Դ, ���ڽ��ն�*/
    union
    {

        struct{
                   BOOL32    m_bKeyFrame;    /*Ƶ֡���ͣ�I or P��*/
                   u16       m_wVideoWidth;  /*��Ƶ֡��*/
                   u16       m_wVideoHeight; /*��Ƶ֡��*/
              }m_tVideoParam;
        u8    m_byAudioMode;/*��Ƶģʽ*/
    }x;
} FRAMEHDR, *PFRAMEHDR;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _KDV_def_H_ */

/* end of file kdvdef.h */

