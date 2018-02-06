
#include "common.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

extern s32   g_nShowDebugInfo;        //是否显示隐藏的一些调试状态信息

//不同格式对应的图像的高度和宽度
s32 as32Width[8] = {-1,128,176,352,704,1408,352,-1};
s32 as32Height[8]  = {-1,96,144,288,576,1152,288,-1};

/*=============================================================================
    函数名        ：SetBitField
    功能        ：给一个u32 值一系列位设值。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
             dwValue      要处理的u32值.

             dwBitField   要设置的值

             nStartBit    开始位. if nStartBit is set to 1,
                           and nBits is set to 3,
                           the bit sequence will occupy bits 1 through 4.

             nBits        位数。

    返回值说明：处理后的u32 值.
=============================================================================*/
u32 SetBitField(u32 dwValue, u32 dwBitField, s32 nStartBit, s32 nBits)
{
    s32 nMask = (1 << nBits) - 1;

    return (dwValue & ~(nMask << nStartBit)) +
          ((dwBitField & nMask) << nStartBit);
}

/*=============================================================================
    函数名        ：GetBitField
    功能        ：给一个u32 值一系列位设值。
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
             dwValue      要处理的u32值.


             nStartBit    开始位. if nStartBit is set to 1,
                           and nBits is set to 3,
                           the bit sequence will occupy bits 1 through 4.

             nBits        位数。

    返回值说明：处理后的u32 值.
=============================================================================*/
u32 GetBitField(u32 dwValue, s32 nStartBit, s32 nBits)
{
    s32  nMask = (1 << nBits) - 1;

    return (dwValue >> nStartBit) & nMask;
}

/*=============================================================================
    函数名        :  ConvertH2N
    功能        ： machine order to net order, batch convert;
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                   pBuf    buffer to convert
                   nStartIndex  start position
                   nSize        size to covert

    返回值说明： Valid TRUE,Invalid FALSE;
=============================================================================*/
void ConvertH2N(u8 *pBuf, s32 nStartIndex, s32 nSize)
{
    for(s32 i=nStartIndex; i<(nStartIndex+nSize); i++)
    {
        ((u32 *)pBuf)[i] = htonl(((u32 *)pBuf)[i]);
    }

    return;
}

/*=============================================================================
    函数名        :  ConvertN2H
    功能        ： net order to machine order, batch convert;
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明:
                   pBuf    buffer to convert
                   nStartIndex  start position
                   nSize        size to covert

    返回值说明： Valid TRUE,Invalid FALSE;
=============================================================================*/

void ConvertN2H(u8 *pBuf, s32 nStartIndex, s32 nSize)
{
    for(s32 i=nStartIndex; i<(nStartIndex+nSize); i++)
    {
        ((u32 *)pBuf)[i] = ntohl(((u32 *)pBuf)[i]);
    }
    return;
}

/*=============================================================================
    函数名        ：IsMCastAddr
    功能        ：判断是否是组播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 IsMCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    u8 byLOByte = *pch;

    if( (byLOByte >= 224) && (byLOByte <= 239))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*=============================================================================
    函数名        ：IsBCastAddr
    功能        ：判断是否是广播地址
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   dwIP  IP (net order)

    返回值说明：是TRUE,否FALSE;
=============================================================================*/
BOOL32 IsBCastAddr(u32 dwIP)
{
    u8* pch = (u8*)&dwIP;
    pch += 3;

    u8 byHIByte = *pch;

    if( 255 == byHIByte )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*=============================================================================
    函数集        ：stdh264_bs_init ....
    功能        ：h264 码流信息分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   无

    返回值说明： 无
=============================================================================*/
void stdh264_bs_init( TBitStream *s, void *p_data, s32 i_data )
{
    s->pu8Start = (u8*) p_data;
    s->pu8P     = (u8*) p_data;
    s->pu8End   = (u8*) s->pu8P + i_data;
    s->s32Left  = 8;
}

s32 stdh264_bs_pos( TBitStream *s )
{
    return( 8 * ( s->pu8P - s->pu8Start ) + 8 - s->s32Left );
}

s32 stdh264_bs_eof( TBitStream *s )
{
    return( s->pu8P >= s->pu8End ? 1: 0 );
}

u32 stdh264_bs_read( TBitStream *s, s32 i_count )
{
    static u32 dwstdh264MaskArr[33] =
    {
        0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
        0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
        0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff };

    s32 i_shr;
    u32 i_result = 0;

    while( i_count > 0 )
    {
        if( s->pu8P >= s->pu8End )
        {
            break;
        }

        if( ( i_shr = s->s32Left - i_count ) >= 0 )
        {
            /* more in the buffer than requested */
            i_result |= ( *s->pu8P >> i_shr )&dwstdh264MaskArr[i_count];
            s->s32Left -= i_count;
            if( s->s32Left == 0 )
            {
                s->pu8P++;
                s->s32Left = 8;
            }
            return( i_result );
        }
        else
        {
            /* less in the buffer than requested */
           i_result |= (*s->pu8P&dwstdh264MaskArr[s->s32Left]) << -i_shr;
           i_count  -= s->s32Left;
           s->pu8P++;
           s->s32Left = 8;
        }
    }

    return( i_result );
}

u32 stdh264_bs_read1( TBitStream *s )
{

    if( s->pu8P < s->pu8End )
    {
        u32 i_result;

        s->s32Left--;
        i_result = ( *s->pu8P >> s->s32Left )&0x01;
        if( s->s32Left == 0 )
        {
            s->pu8P++;
            s->s32Left = 8;
        }
        return i_result;
    }

    return 0;
}

u32 stdh264_bs_show( TBitStream *s, s32 i_count )
{
    if( s->pu8P < s->pu8End && i_count > 0 )
    {
        u32 i_cache = ((s->pu8P[0] << 24)+(s->pu8P[1] << 16)+(s->pu8P[2] << 8)+s->pu8P[3]) << (8-s->s32Left);
        return( i_cache >> ( 32 - i_count) );
    }
    return 0;
}

/* TODO optimize */
void stdh264_bs_skip( TBitStream *s, s32 i_count )
{
    s->s32Left -= i_count;

    while( s->s32Left <= 0 )
    {
        s->pu8P++;
        s->s32Left += 8;
    }
}

s32 stdh264_bs_read_ue( TBitStream *s )
{
    s32 i = 0;

    while( stdh264_bs_read1( s ) == 0 && s->pu8P < s->pu8End && i < 32 )
    {
        i++;
    }
    return( ( 1 << i) - 1 + stdh264_bs_read( s, i ) );
}

s32 stdh264_bs_read_se( TBitStream *s )
{
    s32 val = stdh264_bs_read_ue( s );

    return val&0x01 ? (val+1)/2 : -(val/2);
}

s32 stdh264_bs_read_te( TBitStream *s, s32 x )
{
    if( x == 1 )
    {
        return 1 - stdh264_bs_read1( s );
    }
    else if( x > 1 )
    {
        return stdh264_bs_read_ue( s );
    }
    return 0;
}

s32 stdh264_FirstPartOfSliceHeader(TBitStream *s, Tstdh264Dec_SliceHeaderData *dec_slice_header)
{
    s32 tmp;
    dec_slice_header->first_mb_in_slice = stdh264_bs_read_ue( s );
    tmp = stdh264_bs_read_ue( s );
    if (tmp>4)
    {
        tmp -=5;
    }
    dec_slice_header->slice_type = (stdh264SliceType) tmp;

    return 0;
}

/*=============================================================================
    函数集        ：stdh265_bs_init ....
    功能        ：h265 码流信息分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   无

    返回值说明： 无
=============================================================================*/
void stdh265_bs_init( TBitStream *s, void *p_data, s32 i_data )
{
    u32 u32Next24bit;
    u8 *pu8TempData = NULL, *pu8CurData = (u8 *)p_data, *pu8Head = NULL;
    u8 u8Code;
    s32 s32Len = i_data < 30 ? i_data : 30;
    s32 s32Len1 = s32Len;
    pu8Head = s->au8HeadInfo;

    s->pu8Start = pu8Head;
    s->pu8P     = pu8Head;
    s->pu8End   = (u8*) s->pu8P + s32Len;
    s->s32Left  = 8;
    u32Next24bit = 0xFFFFFFFF;
    memcpy(pu8Head, (u8 *)p_data, i_data < 30 ? i_data : 30);
    pu8TempData = pu8Head;
    do
    {
        u8Code = *pu8CurData++;
        u32Next24bit = ((u32Next24bit << 8) | u8Code) & 0xFFFFFF;

        if(u32Next24bit != 0x000003 &&
           u32Next24bit != 0x000001 &&
           u32Next24bit != 0x000000)
        {
            //复制-除了竞争码
            *pu8TempData++ = u8Code;
        }
        else
        {
            s32Len -= 1;
        }
    }while((u32Next24bit != 0x000001) && (u32Next24bit != 0x000000) && (pu8CurData < ((u8 *)p_data + s32Len1)));
    s->pu8End = s->pu8Start + s32Len;
}

s32 stdh265_bs_pos( TBitStream *s )
{
    return( 8 * ( s->pu8P - s->pu8Start ) + 8 - s->s32Left );
}

s32 stdh265_bs_eof( TBitStream *s )
{
    return( s->pu8P >= s->pu8End ? 1: 0 );
}

u32 stdh265_bs_read( TBitStream *s, s32 i_count )
{
    static u32 dwstdh264MaskArr[33] =
    {
        0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
        0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
        0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff };

    s32 i_shr;
    u32 i_result = 0;

    while( i_count > 0 )
    {
        if( s->pu8P >= s->pu8End )
        {
            break;
        }

        if( ( i_shr = s->s32Left - i_count ) >= 0 )
        {
            /* more in the buffer than requested */
            i_result |= ( *s->pu8P >> i_shr )&dwstdh264MaskArr[i_count];
            s->s32Left -= i_count;
            if( s->s32Left == 0 )
            {
                s->pu8P++;
                s->s32Left = 8;
            }
            return( i_result );
        }
        else
        {
            /* less in the buffer than requested */
           i_result |= (*s->pu8P&dwstdh264MaskArr[s->s32Left]) << -i_shr;
           i_count  -= s->s32Left;
           s->pu8P++;
           s->s32Left = 8;
        }
    }

    return( i_result );
}

u32 stdh265_bs_read1( TBitStream *s )
{

    if( s->pu8P < s->pu8End )
    {
        u32 i_result;

        s->s32Left--;
        i_result = ( *s->pu8P >> s->s32Left )&0x01;
        if( s->s32Left == 0 )
        {
            s->pu8P++;
            s->s32Left = 8;
        }
        return i_result;
    }

    return 0;
}

u32 stdh265_bs_show( TBitStream *s, s32 i_count )
{
    if( s->pu8P < s->pu8End && i_count > 0 )
    {
        u32 i_cache = ((s->pu8P[0] << 24)+(s->pu8P[1] << 16)+(s->pu8P[2] << 8)+s->pu8P[3]) << (8-s->s32Left);
        return( i_cache >> ( 32 - i_count) );
    }
    return 0;
}

/* TODO optimize */
void stdh265_bs_skip( TBitStream *s, s32 i_count )
{
    s->s32Left -= i_count;

    while( s->s32Left <= 0 )
    {
        s->pu8P++;
        s->s32Left += 8;
    }
}

/*====================================================================
函数名      ：  H265DecBitstreamGetBits
功能        ：  读取一定数量bit
算法实现    ：  （可选项）
引用全局变量：  无
输入参数说明：  ptBs：熵解码结构指针[in]
                l32NBits：待跳过的bit数目[in]
返回值说明  ：  bit所表示的数值
====================================================================*/
u32 H265DecBitstreamGetBits(TBitStream *ptBs, s32 l32NBits)
{
    u32 u32Tmp;

    u32Tmp = stdh265_bs_show(ptBs, l32NBits);

    stdh265_bs_skip(ptBs, l32NBits);

    return u32Tmp;
}


s32 stdh265_bs_read_ue( TBitStream *s )
{
    s32 i = 0;

    while( stdh265_bs_read1( s ) == 0 && s->pu8P < s->pu8End && i < 32 )
    {
        i++;
    }
    return( ( 1 << i) - 1 + stdh265_bs_read( s, i ) );
}

s32 stdh265_bs_read_se( TBitStream *s )
{
    s32 val = stdh265_bs_read_ue( s );

    return val&0x01 ? (val+1)/2 : -(val/2);
}

s32 stdh265_bs_read_te( TBitStream *s, s32 x )
{
    if( x == 1 )
    {
        return 1 - stdh265_bs_read1( s );
    }
    else if( x > 1 )
    {
        return stdh265_bs_read_ue( s );
    }
    return 0;
}

s32 stdh265_FirstPartOfSliceHeader(TBitStream *s, Tstdh265Dec_SliceHeaderData *dec_slice_header, TKdvH265Header *ptH265Header, s32 l32NalType)
{
    u32 first_slice_segment_in_pic_flag;
    s32 BitsSliceSegmentAddress = 0;
    s32 tmp;
    u32 WidthInCU;
    u32 HeightInCU;
    u32 NumOfLcuInFrame;

    if(ptH265Header->m_dwMaxPartionWidth == 0)
    {
        return 0;
    }

    WidthInCU = (ptH265Header->m_wWidth % ptH265Header->m_dwMaxPartionWidth) ? (ptH265Header->m_wWidth / ptH265Header->m_dwMaxPartionWidth + 1) : (ptH265Header->m_wWidth / ptH265Header->m_dwMaxPartionWidth);
    HeightInCU = (ptH265Header->m_wHeight % ptH265Header->m_dwMaxPartionWidth) ? (ptH265Header->m_wHeight / ptH265Header->m_dwMaxPartionWidth + 1) : (ptH265Header->m_wHeight / ptH265Header->m_dwMaxPartionWidth);
    NumOfLcuInFrame = WidthInCU * HeightInCU;

    //跳过first_slice_segment_in_pic_flag
    first_slice_segment_in_pic_flag = H265DecBitstreamGetBits(s, 1);

    if(l32NalType >=  16 && l32NalType <= 23)
    {
        //读取no_output_of_prior_pics_flag,暂不支持非零情况，todo
        tmp = H265DecBitstreamGetBits(s, 1);
    }

    //slice_pic_parameter_set_id
    dec_slice_header->pic_parameters_set_id = stdh265_bs_read_ue( s );

    if(!first_slice_segment_in_pic_flag)
    {
        //calculate number of bits required for slice address
        while(NumOfLcuInFrame > (1 << BitsSliceSegmentAddress))
        {
            BitsSliceSegmentAddress++;
        }
        //读取slice_segment_address
        dec_slice_header->first_mb_in_slice = H265DecBitstreamGetBits(s, BitsSliceSegmentAddress);
    }
    else
    {
        dec_slice_header->first_mb_in_slice = 0;
    }

    //num_extra_slice_header_bits等于0，直接读取slice_type
    dec_slice_header->slice_type = stdh265_bs_read_ue(s);

    return 0;
}

/*=============================================================================
    函数集        ：h263Pinitbits ....
    功能        ：h263/h263+ 码流信息分析
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：
                   无

    返回值说明： 无
=============================================================================*/
/*====================================================================
    函数名        ：Stdh263_BsInit
    功能        ：bs初始化
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：*ptBS，要初始化的指针，*pvData,传入数据，s32Data,传入数据的大小
    返回值说明  ：void
====================================================================*/
void Stdh263_BsInit( TBitStream *ptBS, void *pvData, s32 s32Data )
{
    ptBS->pu8Start = (u8*)pvData;
    ptBS->pu8P     = (u8*)pvData;
    ptBS->pu8End   = (u8*)ptBS->pu8P + s32Data;
    ptBS->s32Left  = 8;
}

/*====================================================================
    函数名        ：Stdh263_BsRead
    功能        ：读取bs，共读i_count位
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：*ptBS，bs指针，s32Count要读取的位数
    返回值说明  ：返回读取到的数值
====================================================================*/
u32 Stdh263_BsRead( TBitStream *ptBS, s32 s32Count )
{
     static u32 au32Mask[33]={0x00,
                              0x01,      0x03,      0x07,      0x0f,
                              0x1f,      0x3f,      0x7f,      0xff,
                              0x1ff,     0x3ff,     0x7ff,     0xfff,
                              0x1fff,    0x3fff,    0x7fff,    0xffff,
                              0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
                              0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
                              0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
                              0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};
    s32  s32Shr;
    u32 u32Result = 0;

    while( s32Count > 0 )
    {
        if( ptBS->pu8P >= ptBS->pu8End )
        {
            break;
        }

        if( ( s32Shr = ptBS->s32Left - s32Count ) >= 0 )
        {
            /* more in the buffer than requested */
            u32Result |= ( *ptBS->pu8P >> s32Shr ) & au32Mask[s32Count];
            ptBS->s32Left -= s32Count;
            if( ptBS->s32Left == 0 )
            {
                ptBS->pu8P++;
                ptBS->s32Left = 8;
            }
            return( u32Result );
        }
        else
        {
            /* less in the buffer than requested */
           u32Result |= (*ptBS->pu8P & au32Mask[ptBS->s32Left]) << -s32Shr;
           s32Count  -= ptBS->s32Left;
           ptBS->pu8P++;
           ptBS->s32Left = 8;
        }
    }

    return( u32Result );
}
/*====================================================================
    函数名        ：Stdh263_BsRead1
    功能        ：读取bs，共读1位
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：*ptBS，bs指针
    返回值说明  ：返回读取到的数值
====================================================================*/
u32 Stdh263_BsRead1( TBitStream *ptBS )
{

    if( ptBS->pu8P < ptBS->pu8End )
    {
        u32 u32Result;

        ptBS->s32Left--;
        u32Result = ( *ptBS->pu8P >> ptBS->s32Left ) & 0x01;
        if( ptBS->s32Left == 0 )
        {
            ptBS->pu8P++;
            ptBS->s32Left = 8;
        }
        return u32Result;
    }

    return 0;
}
/*====================================================================
    函数名        ：Stdh263_BsShow
    功能        ：显示i_count位的数据值
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：*s，bs指针, s32Count，位数
    返回值说明  ：返回要显示的数值
====================================================================*/
u32 Stdh263_BsShow( TBitStream *ptBS, s32 s32Count)
{
    if( ptBS->pu8P < ptBS->pu8End && s32Count > 0 )
    {
        u32 u32Cache = ((ptBS->pu8P[0] << 24)+(ptBS->pu8P[1] << 16)+(ptBS->pu8P[2] << 8) + ptBS->pu8P[3]) << (8 - ptBS->s32Left);
        return( u32Cache >> ( 32 - s32Count) );
    }
    return 0;
}

/* TODO optimize */
/*====================================================================
    函数名        ：Stdh263_BsSkip
    功能        ：跳过i_count位
    算法实现    ：（可选项）
    引用全局变量：无
    输入参数说明：*ptBS，bs指针, s32Count，跳过的位数
    返回值说明  ：void
====================================================================*/
void Stdh263_BsSkip( TBitStream *ptBS, s32 s32Count )
{
    ptBS->s32Left -= s32Count;

    while( ptBS->s32Left <= 0 )
    {
        ptBS->pu8P++;
        ptBS->s32Left += 8;
    }
}

/*====================================================================
函数名      ：Stdh263_GetH263PicInfo
功能        ：解码图像头
算法实现    ：
引用全局变量：无
输入参数说明：
            TBitStream *ptBS : 指向码流结构体的指针
            TH263DecPictureHeader *ptH263DecPicHeader : 指向图像头结构体的指针

返回值说明  ：解码成功返回H263DECOK, 否则返回H263DECERROR;
----------------------------------------------------------------------
修改记录    ：
日  期      版本        修改人        修改内容
05/01/27             ZouWenyi            创建
====================================================================*/
BOOL32 Stdh263_GetH263PicInfo( u8 *pu8BitStream, s32 s32BitSteamLen,
                               TH263DecFrameHeader *ptH263DecPicHeader )
{
    BOOL32 bRet = FALSE;

    s32 s32Indicator;
    s16 s16TypeInformation;
    s32 s32PictWidthIndication;
    s32 s32PictHeightIndication;
    s32 s32PSCLast5Bits;
    s32 s32Index = 0;

    TBitStream tBts;

    s16 s16UFEP = 0;
    s16 s16SourceFormat = 0;
    s16 s16PixelAspectRatio = 0;
    s16 s16PictureType = 0;

    s16UFEP = 0;

    //初始化解码码流
    Stdh263_BsInit(&tBts, pu8BitStream, s32BitSteamLen);

    //解码Picture Start Code
    // search for new picture start code
    while (Stdh263_BsShow(&tBts, 17) != 1l)
    {
        Stdh263_BsSkip(&tBts, 1);
        s32Index++;
        if(s32Index > 40000)
        {
            return bRet;
        }
    }

    //Picture Start Code (PSC) (22 bits)
    Stdh263_BsSkip(&tBts, 17);
    s32PSCLast5Bits = Stdh263_BsRead(&tBts, 5);

    if (s32PSCLast5Bits != 0)
    {
        return bRet;
    }

    //Temporal Reference
    Stdh263_BsRead(&tBts, 8);

    //avoid start code emulation
    s16TypeInformation = (s16) Stdh263_BsRead1(&tBts);
    if (!s16TypeInformation)
    {
        return bRet;
    }

    //distinction with Recommendation H.261
    s16TypeInformation = (s16) Stdh263_BsRead1(&tBts);
    if (s16TypeInformation)
    {
        return bRet;
    }

    //Split screen indicator
    s16TypeInformation = (s16) Stdh263_BsRead1(&tBts);

    //Document camera indicator
    s16TypeInformation = (s16) Stdh263_BsRead1(&tBts);

    //Full Picture Freeze Release
    s16TypeInformation = (s16) Stdh263_BsRead1(&tBts);

    //Source format
    s16TypeInformation = (s16) Stdh263_BsRead(&tBts, 3);

    if (s16TypeInformation == 0)
    {
        return bRet;
    }

    if (s16TypeInformation == 0x07)
    {
        //Update Full Extended PTYPE
        (s16UFEP) = (s16) Stdh263_BsRead(&tBts, 3);

        if ((s16UFEP) == 1)
        {
            // The Optional Part of PLUSPTYPE (OPPTYPE)
            (s16SourceFormat) = (s16) Stdh263_BsRead(&tBts, 3);

            // optional custom picture clock frequency
            Stdh263_BsRead1(&tBts);

            //Optional Unrestricted Motion Vector (UMV) mode
            Stdh263_BsRead1(&tBts);

            //Optional Syntax-based Arithmetic Coding (SAC) mode
            Stdh263_BsRead1(&tBts);

            //Optional Advanced Prediction (AP) mode
            Stdh263_BsRead1(&tBts);

            //Optional Advanced INTRA Coding (AIC) mode
            Stdh263_BsRead1(&tBts);

            //Optional Deblocking Filter (DF) mode
            Stdh263_BsRead1(&tBts);

            //Optional Slice Structured (SS) mode
            Stdh263_BsRead1(&tBts);

            //Optional Reference Picture Selection (RPS) mode
            Stdh263_BsRead1(&tBts);

            //Optional Independent Segment Decoding (ISD) mode
            Stdh263_BsRead1(&tBts);

            //Optional Alternative INTER VLC (AIV) mode
            Stdh263_BsRead1(&tBts);

            //Optional Modified Quantization (MQ) mode
            Stdh263_BsRead1(&tBts);

            //The Optional Part of PLUSPTYPE Mode 的一些标志位
            s32Indicator = Stdh263_BsRead(&tBts, 4);
        }

        if (((s16UFEP) == 1) || ((s16UFEP) == 0))
        {
            // The mandatory part of PLUSPTYPE when PLUSPTYPE present(MMPTYPE)
            (s16PictureType) = (s16) Stdh263_BsRead(&tBts, 3);

            //Optional Reference Picture Resampling (RPR) mode
            Stdh263_BsRead1(&tBts);

            //Optional Reduced-Resolution Update (RRU) mode
            Stdh263_BsRead1(&tBts);

            //Rounding Type
            Stdh263_BsRead1(&tBts);

            //MPPTYPE 的一些标志位
            s32Indicator = Stdh263_BsRead(&tBts, 3);
            if (s32Indicator != 1)
            {
                return bRet;
            }
        }
        else
        {
            // s16UFEP is neither 001 nor 000
            //printf ("UFEP should be either 001 or 000.\n");
        }

        //CPM
        Stdh263_BsRead1(&tBts);

        if ((s16UFEP) && ((s16SourceFormat) == 6))
        {
            //Pixel Aspect Ratio
            (s16PixelAspectRatio) = (s16) Stdh263_BsRead(&tBts, 4);

            if ((s16PixelAspectRatio) == 0)
            {
                return bRet;
            }

            //Picture Width Indication
            s32PictWidthIndication = Stdh263_BsRead(&tBts, 9);

            (ptH263DecPicHeader->s32Width) = (s32PictWidthIndication + 1 ) * 4;

            //Custom Picture Format 的标志位
            s32Indicator = Stdh263_BsRead1(&tBts);
            if (!s32Indicator)
            {
                return bRet;
            }

            //Picture Height Indication
            s32PictHeightIndication = Stdh263_BsRead(&tBts, 9);
            (ptH263DecPicHeader->s32Height) = s32PictHeightIndication * 4;


            //Extended Pixel Aspect Ratio
            if ((s16PixelAspectRatio) == 15)
            {
                Stdh263_BsRead(&tBts, 8);
                Stdh263_BsRead(&tBts, 8);
            }
        }

        if ((s16SourceFormat) != 6)
        {
            (ptH263DecPicHeader->s32Width) = (s16) as32Width[(s16SourceFormat)];
            (ptH263DecPicHeader->s32Height) = (s16) as32Height[(s16SourceFormat)];
        }
    }
    else
    {
        (s16SourceFormat) = s16TypeInformation;

        (ptH263DecPicHeader->s32Width)  = (s16) as32Width[(s16SourceFormat)];
        (ptH263DecPicHeader->s32Height) = (s16) as32Height[(s16SourceFormat)];

        //I帧或者P帧
        (s16PictureType) = (s16) Stdh263_BsRead1(&tBts);
    }

    ptH263DecPicHeader->s16PictureType = s16PictureType;

    bRet = TRUE;
    return bRet;
}


//H261分析
typedef struct
{
    int         CurrPos;
    unsigned char     CurrValue;      //一定要unsigned，否则ReadBitString在右移时会拖出一串符号位
    unsigned char *   CurrPointer;
}TBitReader;                    //用于写码流


static void Stdh261_SkipBitString(TBitReader * pBitReader, int BitLen)
{
    int LeftBits;

    LeftBits = pBitReader->CurrPos + BitLen - 8;
    if (LeftBits<0)
    {
        pBitReader->CurrPos += BitLen;
    }
    else
    {
        pBitReader->CurrPointer += LeftBits / 8 + 1;
        pBitReader->CurrValue = * pBitReader->CurrPointer;
        pBitReader->CurrPos = LeftBits % 8;
    }
}

static int Stdh261_ReadBitString(TBitReader * pBitReader, int BitLen)
{
    int Value = 0;
    int RemainBitsInValue, RemainBitsInBitReader, RemainBitsInValueAfterRead, RemainBitsInBitReaderAfterRead, ReadLen;

    RemainBitsInValue = BitLen;
    do
    {
        RemainBitsInBitReader = 8 - pBitReader->CurrPos;
        ReadLen = min(RemainBitsInBitReader, RemainBitsInValue);
        RemainBitsInBitReaderAfterRead = RemainBitsInBitReader - ReadLen;
        RemainBitsInValueAfterRead = RemainBitsInValue - ReadLen;
        Value |= (pBitReader->CurrValue>>RemainBitsInBitReaderAfterRead) << RemainBitsInValueAfterRead;
        RemainBitsInValue -= ReadLen;
        pBitReader->CurrPos += ReadLen;
        if (8 == pBitReader->CurrPos)
        {
            pBitReader->CurrPointer ++;
            pBitReader->CurrValue = * (pBitReader->CurrPointer);
            pBitReader->CurrPos = 0;
        }
    }while (RemainBitsInValue > 0);
    //做出BitLen个1，把Value中BitLen以外部分清零（来自CurrValue中已经被读过的bit）
    if (BitLen < 32)
    {
        Value &= (1 << BitLen) - 1;
    }
    return Value;
}

static int Stdh261_ShowBitString(TBitReader * pBitReader, int BitLen)
{
    TBitReader SaveBitReader;
    int Value;

    memcpy(&SaveBitReader, pBitReader, sizeof(TBitReader));
    Value = Stdh261_ReadBitString(pBitReader, BitLen);
    memcpy(pBitReader, &SaveBitReader, sizeof(TBitReader));
    return Value;
}


static void Stdh261_InitBitReader(TBitReader * pBitReader, unsigned char * InputBuf)
{
    pBitReader->CurrPointer = InputBuf;
    pBitReader->CurrValue = * InputBuf;
    pBitReader->CurrPos = 0;
}

static BOOL32 Stdh261_FindPSC(TBitReader * pBitReader)
{
    int StartBitPos;

    for (StartBitPos = 0; StartBitPos < 8; StartBitPos++)
    {
        if (0x10 == Stdh261_ShowBitString(pBitReader, 20))
        {
            return TRUE;
        }
        Stdh261_SkipBitString(pBitReader, 1);
    }
    return FALSE;
}

static void Stdh261_ReadPicHeader(TBitReader * pBitReader, int * FrameFormat)
{
    int PicHeader;

    PicHeader = Stdh261_ReadBitString(pBitReader,32);
    if ((PicHeader & 8) != 0)
    {
        * FrameFormat = 1;
    }
    else
    {
        * FrameFormat = 0;
    }
}

BOOL32 Stdh261_H261Analyzer(u8 *pBitstream, u16 *Width, u16 *Height)
{
    TBitReader BitReader;
    s32 Format;

    Stdh261_InitBitReader(&BitReader, pBitstream);

    if (FALSE == Stdh261_FindPSC(&BitReader))
    {
        return FALSE;
    }

    Stdh261_ReadPicHeader(&BitReader, &(Format));

    if (Format==1)
    {
        *Width = 352;
        *Height = 288;
    }
    else
    {
        *Width = 176;
        *Height = 144;
    }

    return TRUE;
}

void MedianetPrintf(s8* lpszFormat, ...)
{
    s8 szMsg[2048] = {0};
    va_list pvList;
    u32 dwLen = 0;
    va_start( pvList, lpszFormat);
    dwLen = MedianetVsnprintf(szMsg, sizeof(szMsg), lpszFormat, pvList);
    if( dwLen <= 0 || dwLen >= sizeof(szMsg))
    {
        OspPrintf(TRUE, FALSE, "[error]: Medianet vsprintf() failed.\n");

        return;
    }
    va_end( pvList );

    OspPrintf(TRUE, FALSE, "[Medianet]%s\n", szMsg);
}

void MedianetLog(u8 byLevel, char* lpszFormat, ...)
{
    s8 szMsg[2048] = {0};
    va_list pvList;
    u32 dwLen = 0;
    if (byLevel == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        va_start( pvList, lpszFormat);
        dwLen = MedianetVsnprintf(szMsg, sizeof(szMsg), lpszFormat, pvList);
        if( dwLen <= 0 || dwLen >= sizeof(szMsg))
        {
            OspPrintf(TRUE, FALSE, "[error]: Medianet vsprintf() failed.\n");

            return;
        }
        va_end( pvList );

        OspPrintf(TRUE, FALSE, "[Medianet]%s\n", szMsg);
    }
}













