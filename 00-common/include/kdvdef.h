/*****************************************************************************

   模块名      : KDV define 

   文件名      : kdvdef.h

   相关文件    : 

   文件实现功能: KDV宏定义

   作者        : 魏治兵

   版本        : V3.0  Copyright(C) 2001-2002 KDC, All rights reserved.

-----------------------------------------------------------------------------

   修改记录:

   日  期      版本        修改人      修改内容

   2004/02/17  3.0         魏治兵        创建

******************************************************************************/

#ifndef _KDV_DEFINE_H_

#define _KDV_DEFINE_H_


#include "kdvtype.h"
#include <stddef.h>

#ifdef _MSC_VER
//#include <windows.h>
#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>
#endif

#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */



	/*媒体类型定义*/
#define   MEDIA_TYPE_NULL                   (u8)255  /*媒体类型为空*/

/*音频*/
#define   MEDIA_TYPE_PCMU                   (u8)0   /*G.711 ulaw  mode 6*/
#define   MEDIA_TYPE_G721                   (u8)2   /*G.721*/
#define   MEDIA_TYPE_G7231                  (u8)4   /*G.7231*/
#define   MEDIA_TYPE_ADPCM                  (u8)5   /*DVI4 ADPCM*/
#define   MEDIA_TYPE_PCMA                   (u8)8   /*G.711 Alaw  mode 5*/
#define   MEDIA_TYPE_G722                   (u8)9   /*G.722*/
#define   MEDIA_TYPE_G7221                  (u8)13  /*G.7221*/
#define   MEDIA_TYPE_G728                   (u8)15  /*G.728*/
#define   MEDIA_TYPE_G729                   (u8)18  /*G.729*/
#define   MEDIA_TYPE_SVACA                  (u8)20
#define   MEDIA_TYPE_MP2                    (u8)95
#define   MEDIA_TYPE_G7221C                 (u8)98  /*G722.1.C Siren14*/
#define   MEDIA_TYPE_MP3                    (u8)99  /*mp3 mode 0-4*/
#define   MEDIA_TYPE_H224                   (u8)100
#define   MEDIA_TYPE_AACLC                  (u8)102
#define   MEDIA_TYPE_AACLD					(u8)103 /*AAC LD*/
#define   MEDIA_TYPE_AACLC_PCM				(u8)104
#define   MEDIA_TYPE_AMR                    (u8)105
#define   MEDIA_TYPE_G726_16                (u8)112
#define   MEDIA_TYPE_G726_24                (u8)113
#define   MEDIA_TYPE_G726_32                (u8)114
#define   MEDIA_TYPE_G726_40                (u8)115

/*视频*/
#define   MEDIA_TYPE_MJPEG                  (u8)26 
#define   MEDIA_TYPE_H261                   (u8)31  /*H.261*/
#define   MEDIA_TYPE_H262                   (u8)33  /*H.262 (MPEG-2)*/
#define   MEDIA_TYPE_H263                   (u8)34  /*H.263*/

#define	  MEDIA_TYPE_PS                     (u8)96
#define   MEDIA_TYPE_MP4                    (u8)97  /*MPEG-4*/
#define   MEDIA_TYPE_H263PLUS               (u8)101 /*H.263+*/
#define   MEDIA_TYPE_H264                   (u8)106 /*H.264*/
#define   MEDIA_TYPE_SVACV                  (u8)107
#define   MEDIA_TYPE_H265	                (u8)111 /*H.265*/

/*数据*/
#define   MEDIA_TYPE_METADATA               (u8)116 /*元数据*/
//以下会议用
#define   MEDIA_TYPE_H224                   (u8)100  /*H.224 Payload 暂定100*/
#define   MEDIA_TYPE_T120                   (u8)200  /*T.120媒体类型*/
#define   MEDIA_TYPE_H239                   (u8)239  /*H.239数据类型 */
#define   MEDIA_TYPE_MMCU                   (u8)120  /*级联数据通道 */




#define   MEDIA_RES_AUTO                    (u8)0      /* 自动、默认 */

#define   MEDIA_RES_QQCIF                   (u8)0x08   /* 96x80  */
#define   MEDIA_RES_SQCIF                   (u8)0x10   /* 128x96  */
#define   MEDIA_RES_QCIF                    (u8)0x20   /* 176x144 */

#define   MEDIA_RES_CIF                     (u8)0x40   /* 352x288 */
#define   MEDIA_RES_SIF                     (u8)0x41   /* 352x240 N制 CIF */
#define   MEDIA_RES_QVGA                    (u8)0x42   /* 320x240 */

#define   MEDIA_RES_2CIF                    (u8)0x60   /* 352x576 */

#define   MEDIA_RES_4CIF                    (u8)0x80   /* 704x576 */
#define   MEDIA_RES_4SIF                    (u8)0x81   /* 704x480 */
#define   MEDIA_RES_D1                      (u8)0x82   /* 720x576 */
#define   MEDIA_RES_VGA                     (u8)0x83   /* 640X480 */

#define   MEDIA_RES_SVGA                    (u8)0xA1   /* 800x600  */
#define   MEDIA_RES_XGA                     (u8)0xA2   /* 1024x768 */
#define   MEDIA_RES_XVGA                    (u8)0xA3   /* 1280x960 */

#define   MEDIA_RES_WXGA                    (u8)0xB1   /* 1280x800 */
#define   MEDIA_RES_WXGAP                   (u8)0xB2   /* 1440x900 */

#define   MEDIA_RES_720P                    (u8)0xD0   /* 1280x720 */

#define   MEDIA_RES_1080P                   (u8)0xE0   /* 1920x1080 P */
#define   MEDIA_RES_1080I                   (u8)0xE1   /* 1920x1080 I */


/*APP ID*/

#define AID_KDV_BASE                        (u16)0 



/*网管客户端内部应用号（1-10）*/

#define AID_NMC_BGN      (AID_KDV_BASE + 1)

#define AID_NMC_END      (AID_NMC_BGN + 9)



/*会议控制台内部应用号（11-20）*/

#define AID_MCS_BGN      (AID_NMC_END + 1)

#define AID_MCS_END      (AID_MCS_BGN + 9)



/*终端控制台内部应用号（21-30）*/

#define AID_MTC_BGN      (AID_MCS_END + 1)

#define AID_MTC_END      (AID_MTC_BGN + 9)



/*网管服务器内部应用号（31-40）*/

#define AID_SVR_BGN      (AID_MTC_END + 1)

#define AID_SVR_END      (AID_SVR_BGN + 9)



/*代理内部应用号（41-50）*/

#define AID_AGT_BGN      (AID_SVR_END + 1)

#define AID_AGT_END      (AID_AGT_BGN + 9)



/*MCU内部应用号（51-100）*/

#define AID_MCU_BGN      (AID_AGT_END + 1)

#define AID_MCU_END      (AID_MCU_BGN + 49)



/*MT内部应用号（101-150）*/

#define AID_MT_BGN       (AID_MCU_END + 1)

#define AID_MT_END       (AID_MT_BGN + 49)



/*RECORDER内部应用号（151-160）*/

#define AID_REC_BGN      (AID_MT_END + 1)

#define AID_REC_END      (AID_REC_BGN + 9)



/*混音器内部应用号（161-170）*/

#define AID_MIXER_BGN    (AID_REC_END + 1)

#define AID_MIXER_END    (AID_MIXER_BGN + 9)



/*电视墙内部应用号（171-181）*/

#define AID_TVWALL_BGN   (AID_MIXER_END + 1)

#define AID_TVWALL_END   (AID_TVWALL_BGN + 9)



/*T.120服务器内部应用号（181-200）*/

#define AID_DCS_BGN      (AID_TVWALL_END + 1)

#define AID_DCS_END      (AID_DCS_BGN + 19)



/*T.120控制台内部应用号（201-210）*/

#define AID_DCC_BGN      (AID_DCS_END + 1)

#define AID_DCC_END      (AID_DCC_BGN + 9)



/*码率适配服务器内部应用号（211-220）*/

#define AID_BAS_BGN      (AID_DCC_END + 1)

#define AID_BAS_END      (AID_BAS_BGN + 9)



/*GK服务器内部应用号(221 - 230)*/

#define AID_GKS_BGN      (AID_BAS_END + 1)

#define AID_GKS_END      (AID_GKS_BGN + 9)



/*GK控制台内部应用号(231 - 235)*/

#define AID_GKC_BGN      (AID_GKS_END + 1)

#define AID_GKC_END      (AID_GKC_BGN + 4)



/*用户管理内部应用号(236 - 240)*/

#define AID_UM_BGN      (AID_GKC_END + 1)

#define AID_UM_END      (AID_UM_BGN + 4)



/*地址簿内部应用号(241 - 250)*/

#define AID_ADDRBOOK_BGN    (AID_UM_END + 1)

#define AID_ADDRBOOK_END    (AID_ADDRBOOK_BGN + 9)

/*数据会议终端内部应用号(251 - 260)*/

#define AID_DCMT_BGN    (AID_ADDRBOOK_END + 1)

#define AID_DCMT_END    (AID_DCMT_BGN + 9)

/*mdsc hdsc watchdog 模块(261-265) hualiang add*/
#define AID_DSC_BGN     (AID_DCMT_END + 1) 
#define AID_DSC_END     (AID_DSC_BGN + 4)

/* radius 计费 模块(266-275) guozhongjun add*/
#define  AID_RS_BGN     (AID_DSC_END + 1)
#define  AID_RS_END     (AID_RS_BGN + 9)



/*2004/07/15 不同类型音频码率定义*/

#define AUDIO_BITRATE_G711A         64
#define AUDIO_BITRATE_G711U         64
#define AUDIO_BITRATE_G7231         6
#define AUDIO_BITRATE_MP3           48
#define AUDIO_BITRATE_G728          16
#define AUDIO_BITRATE_G729          8/*暂不支持*/
#define AUDIO_BITRATE_G722          64
#define AUDIO_BITRATE_G7221C        24

#define MAX_VIDEO_FRAME_SIZE (512 * 1024)
#define MAX_AUDIO_FRAME_SIZE (2 * 1024)

	enum MESSAGE_TYPE
	{
		MES_INTELLSEARCH = 1,			//用于智能搜索信息
		MES_GPS						//用于gps

	};



#define  MAX_VIDEO_BITRATE				(u32)(8<<20)
#define  MAX_AUDIO_BITRATE				(u32)(200<<10)

#define	 AUDIO_MEDIA_TYPE				(u8)0
#define  VIDEO_MEDIA_TYPE				(u8)1

#define  WAVE_FORMAT_G723_6000			0x1014 
#define  WAVE_FORMAT_G723_5333			0x1015
#define  WAVE_FORMAT_G729				0x0044
#define  WAVE_FORMAT_G7221C				0x0070
#define  WAVE_FORMAT_AACLC	            0xA106
#define  WAVE_FORMAT_AMR                0x7A22
#define  WAVE_FORMAT_AAC_MS	   	       (u16)255
	//add by linlifen 2015.7.24
#define  WAVE_FORMAT_KDM_IMA_ADPCM		0xA110

#define  WAVE_FORMAT_G723               0x0042
#define  WAVE_FORMAT_G728               0x0041
#define  WAVE_FORMAT_G722               0x0065
#define  WAVE_FORMAT_AAC		   	    0xA106 

#define  WAVE_FORMAT_SPG726             0x0045  //Microsoft Corporation
#define TIME_LEN_10MS					(u32)10
#define TIME_LEN_30MS					(u32)30
#define TIME_LEN_36MS					(u32)36
#define TIME_LEN_40MS					(u32)40
#define TIME_LEN_32MS					(u32)32
#define TIME_LEN_20MS					(u32)20
#define TIME_LEN_63MS                   (u32)63

#define GUID_WMMEDIASUBTYPE_KDM4  {0x344D444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_KDM2  {0x324D444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_KDH1  {0x3148444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_KDH3  {0x3348444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_KDH4  {0x3448444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_KDH5  {0x3548444B, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_MJPG  {0x47504A4D, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIASUBTYPE_SVAC  {0x43415653, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}

#define GUID_WMMEDIATYPE_MP3      {0x00000055, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G711A    {0x00000006, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G711U    {0x00000007, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G728     {0x00000041, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G722     {0x00000065, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G729     {0x00000044, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G723_6000     {0x00000042, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_IMA_ADPCM     {0x0000A110, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G7221C     {0x00000070, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_AACLC     {0x0000A106, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_AMR       {0x00007A22, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}}
#define GUID_WMMEDIATYPE_G726      {0x00000045, 0x0000, 0x0010, {0x82, 0x6d, 0x77, 0x78, 0x2e, 0x67, 0x71, 0x1b}}

	typedef struct tagVideoEncType
	{
		u32  m_dwFOURCC;
		u8   m_byBitCount;
		GUID m_subtype;
	}TVideoEncType;

	static const TVideoEncType g_tVideoEncTypeArr[8] =
	{
		{ mmioFOURCC('K','D','M','4'), 24, GUID_WMMEDIASUBTYPE_KDM4},
		{ mmioFOURCC('K','D','H','1'), 24, GUID_WMMEDIASUBTYPE_KDH1},
		{ mmioFOURCC('K','D','H','3'), 24, GUID_WMMEDIASUBTYPE_KDH3},
		{ mmioFOURCC('K','D','H','4'), 24, GUID_WMMEDIASUBTYPE_KDH4},
		{ mmioFOURCC('K','D','M','2'), 24, GUID_WMMEDIASUBTYPE_KDM2},
		{ mmioFOURCC('K','D','H','5'), 24, GUID_WMMEDIASUBTYPE_KDH5},
		{ mmioFOURCC('M','J','P','G'), 24, GUID_WMMEDIASUBTYPE_MJPG},
		{ mmioFOURCC('S','V','A','C'), 24, GUID_WMMEDIASUBTYPE_SVAC},
	};

	typedef struct tagAudioEncType
	{
		u16  m_wTWOCC;
		GUID m_subtype;
	}TAudioEncType;

	static const TAudioEncType g_tAudioEncTypeArr[12] =
	{
		{ WAVE_FORMAT_MPEGLAYER3, GUID_WMMEDIATYPE_MP3},
		{ WAVE_FORMAT_ALAW, GUID_WMMEDIATYPE_G711A},
		{ WAVE_FORMAT_MULAW, GUID_WMMEDIATYPE_G711U},
		{ WAVE_FORMAT_G728_CELP, GUID_WMMEDIATYPE_G728},
		{ WAVE_FORMAT_G722_ADPCM, GUID_WMMEDIATYPE_G722},
		{ WAVE_FORMAT_G729, GUID_WMMEDIATYPE_G729},
		{ WAVE_FORMAT_G723_6000, GUID_WMMEDIATYPE_G723_6000},
		{ WAVE_FORMAT_KDM_IMA_ADPCM, GUID_WMMEDIATYPE_IMA_ADPCM},
		{ WAVE_FORMAT_G7221C, GUID_WMMEDIATYPE_G7221C},
		{ WAVE_FORMAT_AACLC, GUID_WMMEDIATYPE_AACLC},
		{ WAVE_FORMAT_AMR, GUID_WMMEDIATYPE_AMR},
		{ WAVE_FORMAT_SPG726, GUID_WMMEDIATYPE_G726}
	};

	static const u16 audio_sample_PerSec[16] = {
		64000, 48000, 44100, 32000, 16000, 8000 };

	typedef struct tagKdvCodecContext
	{
		s32 sample_rate;///< samples per second
		s32 channels;///< number of audio channels
	}TKdvCodecContext;

	static BOOL32 KdvContextToAudioMode(TKdvCodecContext *ptAVcontext, u8 *pbyAudioMode, u8 byMediaType)
	{
		u16 wIndex = 0;
		if (NULL == ptAVcontext)
		{
			return FALSE;
		}
		switch (byMediaType)
		{
		case MEDIA_TYPE_AACLC:
			*pbyAudioMode = 0;
			for (wIndex = 0; wIndex < 16; wIndex++)
			{
				if (ptAVcontext->sample_rate == audio_sample_PerSec[wIndex])
					break;
			}
			if (wIndex == 16)
			{
				return FALSE;
			}
			*pbyAudioMode = ((ptAVcontext->channels << 4) & 0x30) | (wIndex & 0xF);
			break;
		case MEDIA_TYPE_MP3:
		{
			if (ptAVcontext->sample_rate == 11025 && ptAVcontext->channels == 1)
			{
				*pbyAudioMode = 0;
			}
			else if (ptAVcontext->sample_rate == 11025 && ptAVcontext->channels == 2)
			{
				*pbyAudioMode = 1;
			}
			else if (ptAVcontext->sample_rate == 22050 && ptAVcontext->channels == 2)
			{
				*pbyAudioMode = 2;
			}
			else if (ptAVcontext->sample_rate == 32000 && ptAVcontext->channels == 1)
			{
				*pbyAudioMode = 3;
			}
			else if (ptAVcontext->sample_rate == 32000 && ptAVcontext->channels == 2)
			{
				*pbyAudioMode = 4;
			}
			else
			{
				*pbyAudioMode = 0;
			}
			break;
		}
		default:
			break;
		}

		return TRUE;
	}

	static BOOL32 KdvAudioModeToContext(TKdvCodecContext *ptAVcontext, u8 byAudioMode, u8 byMediaType)
	{
		u16 wIndex = 0;

		switch (byMediaType)
		{
		case MEDIA_TYPE_PCMA:
		case MEDIA_TYPE_PCMU:
		case MEDIA_TYPE_ADPCM:
		case MEDIA_TYPE_AMR:
		case MEDIA_TYPE_G726_16:
		case MEDIA_TYPE_G726_24:
		case MEDIA_TYPE_G726_32:
		case MEDIA_TYPE_G726_40:
		{
			ptAVcontext->sample_rate = 8000;
			ptAVcontext->channels = 1;
			break;
		}
		case MEDIA_TYPE_G722:
		{
			ptAVcontext->sample_rate = 16000;
			ptAVcontext->channels = 2;
			break;
		}
		case MEDIA_TYPE_G7221C:
		{
			ptAVcontext->sample_rate = 32000;
			ptAVcontext->channels = 2;
			break;
		}

		case MEDIA_TYPE_AACLC:
			if (0 == byAudioMode)
			{
				ptAVcontext->sample_rate = 16000;
				ptAVcontext->channels = 1;
			}
			else
			{
				wIndex = byAudioMode & 0xF;
				ptAVcontext->sample_rate = audio_sample_PerSec[wIndex];
				ptAVcontext->channels = (byAudioMode & 0x30) >> 4;
			}
			break;
		case MEDIA_TYPE_MP3:
		{
			if (0 == byAudioMode)
			{
				ptAVcontext->sample_rate = 11025;
				ptAVcontext->channels = 1;
			}
			else if (1 == byAudioMode)
			{
				ptAVcontext->sample_rate = 11025;
				ptAVcontext->channels = 2;
			}
			else if (2 == byAudioMode)
			{
				ptAVcontext->sample_rate = 22050;
				ptAVcontext->channels = 2;
			}
			else if (3 == byAudioMode)
			{
				ptAVcontext->sample_rate = 32000;
				ptAVcontext->channels = 1;
			}
			else if (4 == byAudioMode)
			{
				ptAVcontext->sample_rate = 32000;
				ptAVcontext->channels = 2;
			}
			else
			{
				return FALSE;
			}
			break;
		}
		default:
			break;
		}

		return TRUE;
	}

	static s32 GetAudioEncTypeArrIdx(u8 byMediaType)
	{
		int nEncIndex = -1;

		switch (byMediaType)
		{
		case MEDIA_TYPE_MP3:
			nEncIndex = 0;
			break;

		case MEDIA_TYPE_PCMA:
			nEncIndex = 1;
			break;

		case MEDIA_TYPE_PCMU:
			nEncIndex = 2;
			break;
		case MEDIA_TYPE_G728:
			nEncIndex = 3;
			break;

		case MEDIA_TYPE_G722:
			nEncIndex = 4;
			break;

		case MEDIA_TYPE_G729:
			nEncIndex = 5;
			break;

		case MEDIA_TYPE_G7231:
			nEncIndex = 6;
			break;
		case MEDIA_TYPE_ADPCM:
			nEncIndex = 7;
			break;
		case MEDIA_TYPE_G7221C:
			nEncIndex = 8;
			break;
		case MEDIA_TYPE_AACLC:
			nEncIndex = 9;
			break;
		case MEDIA_TYPE_AMR:
			nEncIndex = 10;
			break;
		case MEDIA_TYPE_G726_16:
		case MEDIA_TYPE_G726_24:
		case MEDIA_TYPE_G726_32:
		case MEDIA_TYPE_G726_40:
			nEncIndex = 11;
			break;
		case MEDIA_TYPE_MP4:
			nEncIndex = 0;
			break;

		case MEDIA_TYPE_H261:
			nEncIndex = 1;
			break;

		case MEDIA_TYPE_H263:
			nEncIndex = 2;
			break;
		case MEDIA_TYPE_H264:
			nEncIndex = 3;
			break;
		case MEDIA_TYPE_H262:
			nEncIndex = 4;
			break;
		case MEDIA_TYPE_H265:
			nEncIndex = 5;
			break;
		case MEDIA_TYPE_MJPEG:
			nEncIndex = 6;
			break;
		case MEDIA_TYPE_SVACV:
			nEncIndex = 7;
			break;

		default:
			nEncIndex = -1;
			break;
		}

		return nEncIndex;
	}

	static s32  GetAudFormatIdx(u8 byMediaType, u8 byAudioMode)
	{
		s32 nAudFormatIdx = -1;

		switch (byMediaType)
		{
		case MEDIA_TYPE_MP3:
			nAudFormatIdx = 4;
			break;

		case MEDIA_TYPE_PCMA:
			nAudFormatIdx = 5;
			break;

		case MEDIA_TYPE_PCMU:
			nAudFormatIdx = 6;
			break;

		case MEDIA_TYPE_G728:
			nAudFormatIdx = 9;
			break;

		case MEDIA_TYPE_G722:
			nAudFormatIdx = 10;
			break;

		case MEDIA_TYPE_G729:
			nAudFormatIdx = 11;
			break;

		case MEDIA_TYPE_G7231:
			nAudFormatIdx = 7;
			break;
		case MEDIA_TYPE_ADPCM:
			nAudFormatIdx = 13;
			break;
		case MEDIA_TYPE_G7221C:
			nAudFormatIdx = 12;
			break;
		case MEDIA_TYPE_AMR:
			nAudFormatIdx = 21;
			break;
		case MEDIA_TYPE_AACLC:
		{
			TKdvCodecContext tAVcontext;
			KdvAudioModeToContext(&tAVcontext, byAudioMode, byMediaType);
			if (tAVcontext.channels == 1 && tAVcontext.sample_rate == 16000)
			{
				nAudFormatIdx = 14;
			}
			else if (tAVcontext.channels == 2 && tAVcontext.sample_rate == 44100)
			{
				nAudFormatIdx = 15;
			}
			else if (tAVcontext.channels == 1 && tAVcontext.sample_rate == 32000)
			{
				nAudFormatIdx = 16;
			}
			else if (tAVcontext.channels == 1 && tAVcontext.sample_rate == 48000)
			{
				nAudFormatIdx = 17;
			}
			else if (tAVcontext.channels == 2 && tAVcontext.sample_rate == 16000)
			{
				nAudFormatIdx = 18;
			}
			else if (tAVcontext.channels == 2 && tAVcontext.sample_rate == 32000)
			{
				nAudFormatIdx = 19;
			}
			else if (tAVcontext.channels == 2 && tAVcontext.sample_rate == 48000)
			{
				nAudFormatIdx = 20;
			}
		}
		break;
		case MEDIA_TYPE_G726_16:
			nAudFormatIdx = 22;
			break;
		case MEDIA_TYPE_G726_24:
			nAudFormatIdx = 23;
			break;
		case MEDIA_TYPE_G726_32:
			nAudFormatIdx = 24;
			break;
		case MEDIA_TYPE_G726_40:
			nAudFormatIdx = 25;
			break;
		default:
			break;
		}

		return nAudFormatIdx;
	}

	static u32 GetSampleRate(u8 byMediaType)
	{
		u32 dwSampleRate = 0;
		switch (byMediaType)
		{
		case MEDIA_TYPE_H264:
			dwSampleRate = 90000;
			break;
		case MEDIA_TYPE_MP4:
			dwSampleRate = 1000;
			break;
		case MEDIA_TYPE_PCMA:
			dwSampleRate = 8000;
			break;
		case MEDIA_TYPE_PCMU:
			dwSampleRate = 8000;
			break;
		case MEDIA_TYPE_ADPCM:
			dwSampleRate = 8000;
			break;
		case MEDIA_TYPE_AACLC:
			dwSampleRate = 16000;
			break;
		default:
			dwSampleRate = 1000;
			break;
		}
		return dwSampleRate;
	}

	static BOOL32 JdgAudio(u8 byMediaType, BOOL32 *pbAud)
	{
		if (NULL == pbAud)
		{
			return FALSE;
		}

		if (!((byMediaType == MEDIA_TYPE_PCMU) || (byMediaType == MEDIA_TYPE_PCMA) ||
			(byMediaType == MEDIA_TYPE_G728) || (byMediaType == MEDIA_TYPE_G722) ||
			(byMediaType == MEDIA_TYPE_G729) || (byMediaType == MEDIA_TYPE_MP3) ||
			(byMediaType == MEDIA_TYPE_ADPCM) || (byMediaType == MEDIA_TYPE_G7221C) ||
			(byMediaType == MEDIA_TYPE_G7231) || (byMediaType == MEDIA_TYPE_AACLC) ||
			(byMediaType == MEDIA_TYPE_AACLD) || (byMediaType == MEDIA_TYPE_AACLC_PCM) ||
			(byMediaType == MEDIA_TYPE_AMR) ||
			(byMediaType == MEDIA_TYPE_MP4) || (byMediaType == MEDIA_TYPE_H262) ||
			(byMediaType == MEDIA_TYPE_H261) ||
			(byMediaType == MEDIA_TYPE_H263) || (byMediaType == MEDIA_TYPE_H263PLUS) ||
			(byMediaType == MEDIA_TYPE_H264) || (byMediaType == MEDIA_TYPE_H265) ||
			(byMediaType == MEDIA_TYPE_MJPEG) || (byMediaType == MEDIA_TYPE_PS) ||
			(byMediaType == MEDIA_TYPE_G726_16) || (byMediaType == MEDIA_TYPE_G726_24) ||
			(byMediaType == MEDIA_TYPE_G726_32) || (byMediaType == MEDIA_TYPE_G726_40) ||
			(byMediaType == MEDIA_TYPE_SVACV)))
		{
			return FALSE;
		}

		if ((byMediaType == MEDIA_TYPE_PCMU) || (byMediaType == MEDIA_TYPE_PCMA) ||
			(byMediaType == MEDIA_TYPE_G728) || (byMediaType == MEDIA_TYPE_G722) ||
			(byMediaType == MEDIA_TYPE_G729) || (byMediaType == MEDIA_TYPE_G7231) ||
			(byMediaType == MEDIA_TYPE_ADPCM) || (byMediaType == MEDIA_TYPE_G7221C) ||
			(byMediaType == MEDIA_TYPE_MP3) || (byMediaType == MEDIA_TYPE_AACLC) ||
			(byMediaType == MEDIA_TYPE_AACLD) || (byMediaType == MEDIA_TYPE_AACLC_PCM) ||
			(byMediaType == MEDIA_TYPE_AMR) || (byMediaType == MEDIA_TYPE_G7221) ||
			(byMediaType == MEDIA_TYPE_G726_16) || (byMediaType == MEDIA_TYPE_G726_24) ||
			(byMediaType == MEDIA_TYPE_G726_32) || (byMediaType == MEDIA_TYPE_G726_40)
			)
		{
			*pbAud = TRUE;
		}
		else
		{
			*pbAud = FALSE;
		}

		return TRUE;
	}

#ifdef __cplusplus

}

#endif /* __cplusplus */

#endif /* _KDV_def_H_ */