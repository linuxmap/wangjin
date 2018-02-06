/************************************************************************/
/* 最外层封装                                                           */
/************************************************************************/
#include "common.h"
#include "kdmtsps.h"
#include "streamdef.h"

BOOL32 g_bTswSave = FALSE;

API void tspshelp();

static void TspsInit()
{
    static BOOL32 s_bInit;
    if (s_bInit)
    {
        return;
    }
    
#ifdef _LINUX_
    
    OspRegCommand("tspsver", (void*)tspsver, "show version");
    OspRegCommand("tspshelp", (void*)tspshelp, "show help");
    OspRegCommand("tspspd", (void*)tspspd, "set print debug value");
	OspRegCommand("tswopen", (void*)tswopen, "write no encrypt ts file");
    
#endif // _LINUX_
    
    s_bInit = TRUE;
}

/************************************************************************/
/* write                                                                */
/************************************************************************/
typedef struct tagTspsWrite
{
    EStreamType eStreamType;
    TTsWrite *ptTsWrite;
    TPsWrite *ptPsWrite;
} TTspsWrite;

#define TSPS_WRITE_CHECK                                                             \
    if (ptWrite == NULL) {                                                           \
        TspsPrintf(0, "object handle == NULL !");                                       \
        return ERROR_TSPS_BASE; }                                                    \
    if (PROGRAM_STREAM == ptWrite->eStreamType && ptWrite->ptPsWrite != NULL);       \
    else if (TRANSPORT_STREAM == ptWrite->eStreamType && ptWrite->ptTsWrite != NULL);\
    else { TspsPrintf(0, "Incorrect initialization!"); return ERROR_TSPS_BASE; }

HTspsWrite TspsWriteOpen(EStreamType eType, TspsSectionCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize)
{
    TTspsWrite *ptWrite = NULL;

    TspsInit();

    if (PROGRAM_STREAM != eType && TRANSPORT_STREAM != eType)
    {
        TspsPrintf(0, "TspsWriteOpen fail: StreamType not right.");
        return NULL;
    }

    ptWrite = (TTspsWrite *)malloc(sizeof(TTspsWrite));
    if (NULL == ptWrite)
    {
        TspsPrintf(0, "TspsWriteOpen malloc fail.");
        return NULL;
    }

    if (PROGRAM_STREAM == eType)
    {
        ptWrite->ptPsWrite = PsWriteOpen(pfCallback, (void*)pvContext, dwMaxFrameSize);
        if (NULL == ptWrite->ptPsWrite)
        {
            TSPSFREE(ptWrite);
        }
    }
    else if (TRANSPORT_STREAM == eType)
    {
        ptWrite->ptTsWrite = TsWriteOpen(pfCallback, (void*)pvContext);
        if (NULL == ptWrite->ptTsWrite)
        {
            TSPSFREE(ptWrite);
        }
    }

    if (NULL != ptWrite)
    {
        ptWrite->eStreamType = eType;
    }

    return (HTspsWrite)ptWrite;
}

u16 TspsWriteClose(HTspsWrite hWrite)
{
    u16 wRet = TSPS_OK;
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;

    TSPS_WRITE_CHECK;

    if (PROGRAM_STREAM == ptWrite->eStreamType)
    {
        wRet = PsWriteClose(ptWrite->ptPsWrite);
    }
    else if (TRANSPORT_STREAM == ptWrite->eStreamType)
    {
        wRet = TsWriteClose(ptWrite->ptTsWrite);
    }

    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsWriteClose fail. ErrCode[%d]", wRet);
    }
//这里存在内存泄露，应释放TTspsWrite分配的内存 zxh 2012.9.19
	TSPSFREE(ptWrite);
	ptWrite = NULL;

    return wRet;
}

u16 TspsWriteSetProgram(HTspsWrite hWrite, u8 u8VideoType, u8 u8AudioType)
{
    u16 wRet = TSPS_OK;
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;
    
    TSPS_WRITE_CHECK;
    
    if (PROGRAM_STREAM == ptWrite->eStreamType)
    {
        wRet = PsWriteSetProgram(ptWrite->ptPsWrite, u8VideoType, u8AudioType);
    }
    else if (TRANSPORT_STREAM == ptWrite->eStreamType)
    {
        wRet = TsWriteSetProgram(ptWrite->ptTsWrite, u8VideoType, u8AudioType);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsWriteSetProgram fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TsSetSegmentParam(HTspsWrite hWrite, const u32 u32SegmentDuration, const s8* pu8FileDir, const s8* pu8OutputPrefix, \
					  const s8* pu8IndexFileName, const s8* pu8HttpPrefix, const u32 u32SegmentWindow )
{
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;

	TTsSegment *ptTsSegment = NULL;	
    
    TSPS_WRITE_CHECK;

	if( pu8OutputPrefix == NULL || pu8IndexFileName == NULL || pu8HttpPrefix == NULL )
	{
//		TspsPrintf( 0, "TsSetSegmentParam:input param NULL!\n" );
		return ERROR_TS_WRITE_INPUT_PARAM;
	}

	if( ptWrite->ptTsWrite->ptTsSegment == NULL )
	{
		ptTsSegment = ( TTsSegment* )malloc( sizeof( TTsSegment ) );
		if( ptTsSegment == NULL )
		{
			return ERROR_TSPS_BASE;
		}
		if( pu8FileDir != NULL )
		{
			sprintf( ptTsSegment->aau8OutputFilePrefix, "%s%s", pu8FileDir, pu8OutputPrefix );
			sprintf( ptTsSegment->aau8IndexFileName, "%s%s", pu8FileDir, pu8IndexFileName );
		}
		else
		{			
			strcpy( ptTsSegment->aau8IndexFileName, pu8IndexFileName );
			strcpy( ptTsSegment->aau8OutputFilePrefix, pu8OutputPrefix );
		}

		strcpy( ptTsSegment->aau8HttpPrefix, pu8HttpPrefix );

		ptTsSegment->u32SegmentDuration  = u32SegmentDuration;
		ptTsSegment->u32SegmentWindow = u32SegmentWindow;

		// 切片索引应该从1开始
		// 协议详情见：http://tools.ietf.org/html/draft-pantos-http-live-streaming-08
		// EXT-X-MEDIA-SEQUENCE
		ptTsSegment->u32FirstSegment = 1;
		ptTsSegment->u32LastSegment = 1;
		ptTsSegment->u32LastSegmentTime = 0;
		ptTsSegment->fpSegment = NULL;

		return TsWriteSegmentParam( ptWrite->ptTsWrite, ptTsSegment );
	}
	else
	{
		return ERROR_TSPS_BASE;
	}

	return TSPS_OK;
}

u16 TspsWriteWriteFrame(HTspsWrite hWrite, TspsFRAMEHDR *ptFrame)
{
    u16 wRet = TSPS_OK;
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;
    
    TSPS_WRITE_CHECK;
    
    if (PROGRAM_STREAM == ptWrite->eStreamType)
    {
        wRet = PsWriteWriteEsFrame(ptWrite->ptPsWrite, ptFrame);
    }
    else if (TRANSPORT_STREAM == ptWrite->eStreamType)
    {
        wRet = TsWriteWriteEsFrame(ptWrite->ptTsWrite, ptFrame);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsWriteWriteFrame fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TspsSetEncryptKey(HTspsWrite hWrite, s8 *pszKeyBuf, u16 wKeySize, s8* pszIV, s8 *pszUrlBuf, u16 wUrlLen)
{
	u16 wRet = TSPS_OK;
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;
    
    TSPS_WRITE_CHECK;

	if (PROGRAM_STREAM == ptWrite->eStreamType)
    {
        wRet = ERROR_TS_WRITE_NOT_SUPPORT;
    }
    else if (TRANSPORT_STREAM == ptWrite->eStreamType)
    {
        wRet = TsWriteSetEncryptKey(ptWrite->ptTsWrite, pszKeyBuf, pszIV, wKeySize, pszUrlBuf, wUrlLen);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsWriteWriteEnd fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TspsWriteWriteEnd(HTspsWrite hWrite)
{
    u16 wRet = TSPS_OK;
    TTspsWrite *ptWrite = (TTspsWrite *)hWrite;
    
    TSPS_WRITE_CHECK;
    
    if (PROGRAM_STREAM == ptWrite->eStreamType)
    {
        wRet = PsWriteWriteEnd(ptWrite->ptPsWrite);
    }
    else if (TRANSPORT_STREAM == ptWrite->eStreamType)
    {
        wRet = TSPS_OK;
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsWriteWriteEnd fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}


/************************************************************************/
/* read                                                                 */
/************************************************************************/
typedef struct tagTspsRead
{
    EStreamType eStreamType;
    TTsRead *ptTsRead;
    TPsRead *ptPsRead;
} TTspsRead;

#define TSPS_READ_CHECK                                                           \
    if (ptRead == NULL) {                                                         \
        TspsPrintf(0, "object handle == NULL !");                                    \
        return ERROR_TSPS_BASE; }                                                 \
    if (PROGRAM_STREAM == ptRead->eStreamType && ptRead->ptPsRead != NULL);       \
    else if (TRANSPORT_STREAM == ptRead->eStreamType && ptRead->ptTsRead != NULL);\
    else { TspsPrintf(0, "Incorrect initialization!"); return ERROR_TSPS_BASE; }

HTspsRead TspsReadOpen(EStreamType eType, TspsFrameCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize)
{
    TTspsRead *ptRead = NULL;

    TspsInit();

    if (PROGRAM_STREAM != eType && TRANSPORT_STREAM != eType)
    {
        TspsPrintf(0, "TspsReadOpen fail: StreamType not right.");
        return NULL;
    }

    ptRead = (TTspsRead *)malloc(sizeof(TTspsRead));
    if (NULL == ptRead)
    {
        TspsPrintf(0, "TspsReadOpen malloc fail.");
        return NULL;
    }

    if (PROGRAM_STREAM == eType)
    {
        ptRead->ptPsRead = PsReadOpen(pfCallback, (void*)pvContext, dwMaxFrameSize);
        if (NULL == ptRead->ptPsRead)
        {
            TSPSFREE(ptRead);
        }
    }
    else if (TRANSPORT_STREAM == eType)
    {
        ptRead->ptTsRead = TsReadOpen(pfCallback, (void*)pvContext);
        if (NULL == ptRead->ptTsRead)
        {
            TSPSFREE(ptRead);
        }
    }

    if (NULL != ptRead)
    {
        ptRead->eStreamType = eType;
    }

    return (HTspsRead)ptRead;
}

u16 TspsReadClose(HTspsRead hRead)
{
    u16 wRet = TSPS_OK;
    TTspsRead *ptRead = (TTspsRead *)hRead;
    
    TSPS_READ_CHECK;
    
    if (PROGRAM_STREAM == ptRead->eStreamType)
    {
        wRet = PsReadClose(ptRead->ptPsRead);
    }
    else if (TRANSPORT_STREAM == ptRead->eStreamType)
    {
        wRet = TsReadClose(ptRead->ptTsRead);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsReadClose fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TspsReadSetProgramCallback(HTspsRead hRead, TspsProgramCallback pfCallback, KD_PTR pvContext)
{
    u16 wRet = TSPS_OK;
    TTspsRead *ptRead = (TTspsRead *)hRead;
    
    TSPS_READ_CHECK;
    
    if (PROGRAM_STREAM == ptRead->eStreamType)
    {
        wRet = PsReadSetProgramCallback(ptRead->ptPsRead, pfCallback, (void*)pvContext);
    }
    else if (TRANSPORT_STREAM == ptRead->eStreamType)
    {
        wRet = TsReadSetProgramCallback(ptRead->ptTsRead, pfCallback, (void*)pvContext);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsReadSetProgramCallback fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TspsReadInputStream(HTspsRead hRead, u8 *pu8Buf, u32 u32Len)
{
    u16 wRet = TSPS_OK;
    TTspsRead *ptRead = (TTspsRead *)hRead;
    
    TSPS_READ_CHECK;
    
    if (PROGRAM_STREAM == ptRead->eStreamType)
    {
        wRet = PsReadInputStream(ptRead->ptPsRead, pu8Buf, u32Len);
    }
    else if (TRANSPORT_STREAM == ptRead->eStreamType)
    {
        wRet = TsReadInputStream(ptRead->ptTsRead, pu8Buf, u32Len);
    }
    
    if (TSPS_OK != wRet)
    {
        TspsPrintf(0, "TspsReadInputStream fail. ErrCode[%d]", wRet);
    }
    
    return wRet;
}

u16 TspsReadResetStream(HTspsRead hRead)
{
	u16 wRet = TSPS_OK;
	TTspsRead *ptRead = (TTspsRead*)hRead;

	TSPS_READ_CHECK;

	if (PROGRAM_STREAM == ptRead->eStreamType)
	{
		wRet = PsReadResetStream(ptRead->ptPsRead);
	}
	else
	{

	}

	if (TSPS_OK != wRet)
	{
		TspsPrintf(0, "TspsReadREsetStream fail. ErrCode[%d]", wRet);
	}

	return wRet;
}

/************************************************************************/
/* API                                                                  */
/************************************************************************/
API void tspsver()
{
    OspPrintf(1, 0, " %s (compile: %s, %s)\n", TSPS_VERSION, __TIME__, __DATE__);
}

API void tspshelp()
{
    OspPrintf(1, 0, ">>> Tsps help:\n");
    tspsver();
    OspPrintf(1, 0, "-> APIs:\n");
    OspPrintf(1, 0, "   tspspd(ID): 调试打印\n");
    OspPrintf(1, 0, "      ID==0  : 不打印\n");
    OspPrintf(1, 0, "      ID==1  : 打印TS编码过程\n");
    OspPrintf(1, 0, "      ID==2  : 打印TS解码过程\n");
    OspPrintf(1, 0, "      ID==3  : 打印PS编码过程\n");
    OspPrintf(1, 0, "      ID==4  : 打印PS解码过程\n");
    OspPrintf(1, 0, "      ID==255: 打印所有\n");
	OspPrintf(1, 0, "tswopen : 保存未加密的原始文件\n");
}

API void tswopen()
{
	g_bTswSave = TRUE;
}