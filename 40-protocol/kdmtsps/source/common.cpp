/*=================================================================================
模块名:信息显示
文件名:kdmpstscommon.cpp
相关文件:kdmpstscommon.h
实现功能:调试信息显示
作者:
版权:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#endif
#include "common.h"
#include "kdmtsps.h"


/*=================================================================================
函数名:TspsPrintf
功能:打印
算法实现:
参数说明:
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static u8 g_u8PrintDebugValue = 0;

static BOOL32 g_bLog = FALSE;

API void tspslog(BOOL32 bLog)
{
	g_bLog = bLog;
}

//设置debug打印值
API void tspspd(u8 byDebugValue)
{
    g_u8PrintDebugValue = byDebugValue;
}

//统一打印函数
void TspsPrintf(u8 byPrintValue, const s8 *ps8Format, ...)
{
    if ((byPrintValue & (1 << g_u8PrintDebugValue)) != 0 
        || 0 == byPrintValue || 255 == g_u8PrintDebugValue)
    {
        s8 as8PrintBuf[MAX_PT_DEBUG_PRINT];
        s32 nBufLen = 0;
        
        va_list ptpArg;
        va_start(ptpArg, ps8Format);
        nBufLen = vsprintf(as8PrintBuf, ps8Format, ptpArg);
        va_end(ptpArg);
        
        printf("[TsPs] %s\n", as8PrintBuf);
		OspPrintf(TRUE, FALSE, "[TsPs] %s\n", as8PrintBuf);
    }
}


/*=================================================================================
模块名:CRC校验
文件名:crc.c
相关文件:crc.h
实现功能:对一串字符进行CRC校验
作者:
版权:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
static const u32 au32CRCTable[256] = {
        0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
        0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
        0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
        0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
        0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
        0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
        0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
        0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
        0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
        0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
        0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
        0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
        0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
        0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
        0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
        0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
        0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
        0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
        0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
        0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
        0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
        0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
        0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
        0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
        0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
        0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
        0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
        0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
        0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
        0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
        0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
        0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
        0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
        0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
        0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
        0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
        0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
        0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
        0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
        0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
        0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
        0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
        0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
        0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
        0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
        0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
        0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
        0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
        0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
        0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
        0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
        0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
        0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
        0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
        0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
        0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
        0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
        0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
        0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
        0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
        0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
        0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
        0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
        0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

/*=================================================================================
函数名:CRCGetCRC32
功能:对一串字符进行CRC校验
算法实现:
参数说明:
         [I] pu8Data 字符串
         [I] u32Length 字符串的长度
返回值说明:32位无符号整数
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u32 CRCGetCRC32(u8* pu8Data, u32 u32Length)
{
    u32 u32CRC = 0xffffffff; 
    u32 u32i = 0;
    
    for(u32i = 0; u32i < u32Length; u32i++)
    {
        u32CRC = (u32CRC << 8) ^ au32CRCTable[((u32CRC >> 24) ^ (*pu8Data++)) & 0xFF];
    }

    return u32CRC;
}


/*=================================================================================
函数名:TsStreamPTCovertRtp2Stream
功能:
算法实现: 
参数说明:
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u8 TsPsPTCovertRtp2Stream(u8 u8RtpType)
{
    u8 u8StreamType;

    switch (u8RtpType)
    {
    case MEDIA_TYPE_MP4:
        u8StreamType = PT_STREAM_TYPE_MPEG4;
        break;

    case MEDIA_TYPE_MP2:
        u8StreamType = PT_STREAM_TYPE_MP2;
        break;
    case MEDIA_TYPE_H264:
		u8StreamType = PT_STREAM_TYPE_H264;
		break;
	case MEDIA_TYPE_H265:
		u8StreamType = PT_STREAM_TYPE_H265;
		break;
	case MEDIA_TYPE_SVACV:
		u8StreamType = PT_STREAM_TYPE_SVACV;
		break;
		
	case MEDIA_TYPE_PCMA:
		u8StreamType = PT_STREAM_TYPE_G711A;
		break;
	case MEDIA_TYPE_G7221:
		u8StreamType = PT_STREAM_TYPE_G7221;
		break;
	case MEDIA_TYPE_G7231:
		u8StreamType = PT_STREAM_TYPE_G7231;
		break;
	case MEDIA_TYPE_G729:
		u8StreamType = PT_STREAM_TYPE_G729;
		break;
	case MEDIA_TYPE_SVACA:
		u8StreamType = PT_STREAM_TYPE_SVACA;
		break;	
// 	case MEDIA_TYPE_AACLC:
//         u8StreamType = PT_STREAM_TYPE_MP2AAC;
//         break;	

    case MEDIA_TYPE_NULL:
        u8StreamType = PT_STREAM_TYPE_NULL;
        break;
        
    default:
        u8StreamType = PT_STREAM_TYPE_NULL;
    }

    return u8StreamType;
}

u8 TsPsPTCovertStream2Rtp(u8 u8StreamType)
{
    u8 u8RtpType;
    
    switch (u8StreamType)
    {
	case PT_STREAM_TYPE_H264:
		u8RtpType = MEDIA_TYPE_H264;
		break;

	case PT_STREAM_TYPE_H265_Old:
	case PT_STREAM_TYPE_H265:
		u8RtpType = MEDIA_TYPE_H265;
		break;

    case PT_STREAM_TYPE_MPEG4:
        u8RtpType = MEDIA_TYPE_MP4;
        break;
        
    case PT_STREAM_TYPE_MP2:
        u8RtpType = MEDIA_TYPE_MP2;
        break;
        
	case PT_STREAM_TYPE_G711A:
		u8RtpType = MEDIA_TYPE_PCMA;
		break;
		
	case PT_STREAM_TYPE_G7221:
		u8RtpType = MEDIA_TYPE_G7221;
		break;

	case PT_STREAM_TYPE_G7231:
		u8RtpType = MEDIA_TYPE_G7231;
		break;

	case PT_STREAM_TYPE_G729:
		u8RtpType = MEDIA_TYPE_G729;
		break;

	case PT_STREAM_TYPE_SVACA:
		u8RtpType = MEDIA_TYPE_SVACA;
		break;

	case PT_STREAM_TYPE_SVACV:
		u8RtpType = MEDIA_TYPE_SVACV;
		break;
// 	case PT_STREAM_TYPE_AACLC:
// 
// 	case PT_STREAM_TYPE_MP2AAC:
//         u8RtpType = MEDIA_TYPE_AACLC;
//         break;

    case PT_STREAM_TYPE_NULL:
        u8RtpType = MEDIA_TYPE_NULL;
        break;
        
    default:
        u8RtpType = MEDIA_TYPE_NULL;
    }
    
    return u8RtpType;
}


#define LOG_MAX_NUM         3
#define LOG_MAX_SIZE        (4*1024*1024)
s8 g_szLogPath[256] = {0};
void tspswritelog(char *message, int length)
{

	if (FALSE == g_bLog)
	{
		return;
	}

#ifdef WIN32
	sprintf(g_szLogPath, "./tspslog");
	CreateDirectory((LPCTSTR)g_szLogPath, NULL);
#elif defined _LINUX_
	sprintf(g_szLogPath, "/var/log/tspslog");
	mkdir(g_szLogPath, 0x1FF);
#endif /* WIN32 */
	static BOOL32 bFirstRun = TRUE; //程序是否刚启动
	static u8     byLogIdx  = 0;
	
	s8 szFileName[265] = {0};
	s8 szNextFileName[265] = {0};
	if (bFirstRun) 
	{
		bFirstRun = FALSE;
		
		//获取记录日志的文件序号
		u8 index = 0;
		for (; index < LOG_MAX_NUM; index++) 
		{
			sprintf(szFileName,"%s/tspserrlog%d.log", g_szLogPath, index);
			if (access(szFileName,0) != -1) 
			{
				struct stat tFileStruct;
				stat(szFileName,&tFileStruct);
				if (tFileStruct.st_size < LOG_MAX_SIZE) 
				{
					//找到上次未写满的日志文件
					byLogIdx = index;
					break;
				}
			}
			else
			{
				//找到一个不存在的文件
				byLogIdx = index;
				break;
			}
		}
		if (index == LOG_MAX_NUM) 
		{
			byLogIdx = 0;
			sprintf(szFileName,"%s/tspserrlog%d.log", g_szLogPath, byLogIdx);
			remove(szFileName);
		}
		//删除当前文件的下一个文件
		u8 byTempIdx = (byLogIdx + 1) % LOG_MAX_NUM;
		sprintf(szNextFileName,"%s/tspslog%d.log", g_szLogPath, byTempIdx);
		remove(szNextFileName);
	}
	
	sprintf(szFileName,"%s/tspslog%d.log", g_szLogPath, byLogIdx);
	const s8 *szOpenMode = "a+";
	
	//判断文件是否已经写满
	if (access(szFileName,0) != -1) 
	{
		struct stat tFileStruct;
		stat(szFileName,&tFileStruct);
		if (tFileStruct.st_size > LOG_MAX_SIZE) 
		{
			//文件已写满
			byLogIdx = (byLogIdx + 1) % LOG_MAX_NUM;
			sprintf(szFileName,"%s/tspslog%d.log", g_szLogPath, byLogIdx);
			remove(szFileName);//删除失败,下面设置的模式将进行覆盖写
			szOpenMode = "w+";
			
			//删除当前文件的下一个文件
			u8 byTempIdx = (byLogIdx + 1) % LOG_MAX_NUM;
			sprintf(szNextFileName,"%s/tspslog%d.log", g_szLogPath, byTempIdx);
			remove(szNextFileName);
		}
	}
	
	FILE* pFile = NULL;
	time_t nCurTime;
	time(&nCurTime);
	struct tm *newtime = localtime(&nCurTime);
	
	s8 abyTime[64] = {0};
	sprintf(abyTime, "%04d-%02d-%02d %02d:%2d:%02d", 
							 newtime->tm_year+1900, newtime->tm_mon+1, 
							 newtime->tm_mday, newtime->tm_hour, 
							 newtime->tm_min, newtime->tm_sec);
	
	s8 abyMesg[1024] = {0};
	sprintf(abyMesg, "[%s] : %s", abyTime, message);
	
	pFile = fopen(szFileName,szOpenMode);
	if (pFile) 
	{
		fwrite(abyMesg,strlen(abyMesg),1,pFile);
		fflush(pFile);
		fclose(pFile);
		pFile = NULL;
	}
	else
	{
		perror("fopen");
	}
}




