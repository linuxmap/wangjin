#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//NALU����
typedef enum 
{
    NAL_UNIT_CODED_SLICE_TRAIL_N = 0,   // 0
    NAL_UNIT_CODED_SLICE_TRAIL_R,   // 1

    NAL_UNIT_CODED_SLICE_TSA_N,     // 2
    NAL_UNIT_CODED_SLICE_TLA,       // 3   // Current name in the spec: TSA_R

    NAL_UNIT_CODED_SLICE_STSA_N,    // 4
    NAL_UNIT_CODED_SLICE_STSA_R,    // 5

    NAL_UNIT_CODED_SLICE_RADL_N,    // 6
    NAL_UNIT_CODED_SLICE_DLP,       // 7 // Current name in the spec: RADL_R

    NAL_UNIT_CODED_SLICE_RASL_N,    // 8
    NAL_UNIT_CODED_SLICE_TFD,       // 9 // Current name in the spec: RASL_R

    NAL_UNIT_RESERVED_10,
    NAL_UNIT_RESERVED_11,
    NAL_UNIT_RESERVED_12,
    NAL_UNIT_RESERVED_13,
    NAL_UNIT_RESERVED_14,
    NAL_UNIT_RESERVED_15,

    NAL_UNIT_CODED_SLICE_BLA,       // 16   // Current name in the spec: BLA_W_LP
    NAL_UNIT_CODED_SLICE_BLANT,     // 17   // Current name in the spec: BLA_W_DLP
    NAL_UNIT_CODED_SLICE_BLA_N_LP,  // 18
    NAL_UNIT_CODED_SLICE_IDR,       // 19  // Current name in the spec: IDR_W_DLP
    NAL_UNIT_CODED_SLICE_IDR_N_LP,  // 20
    NAL_UNIT_CODED_SLICE_CRA,       // 21
    NAL_UNIT_RESERVED_22,
    NAL_UNIT_RESERVED_23,

    NAL_UNIT_RESERVED_24,
    NAL_UNIT_RESERVED_25,
    NAL_UNIT_RESERVED_26,
    NAL_UNIT_RESERVED_27,
    NAL_UNIT_RESERVED_28,
    NAL_UNIT_RESERVED_29,
    NAL_UNIT_RESERVED_30,
    NAL_UNIT_RESERVED_31,

    NAL_UNIT_VPS,                   // 32
    NAL_UNIT_SPS,                   // 33
    NAL_UNIT_PPS,                   // 34
    NAL_UNIT_ACCESS_UNIT_DELIMITER, // 35
    NAL_UNIT_EOS,                   // 36
    NAL_UNIT_EOB,                   // 37
    NAL_UNIT_FILLER_DATA,           // 38
    NAL_UNIT_SEI,                   // 39 Prefix SEI
    NAL_UNIT_SEI_SUFFIX,            // 40 Suffix SEI
    NAL_UNIT_RESERVED_41,
    NAL_UNIT_RESERVED_42,
    NAL_UNIT_RESERVED_43,
    NAL_UNIT_RESERVED_44,
    NAL_UNIT_RESERVED_45,
    NAL_UNIT_RESERVED_46,
    NAL_UNIT_RESERVED_47,
    NAL_UNIT_UNSPECIFIED_48,
    NAL_UNIT_UNSPECIFIED_49,
    NAL_UNIT_UNSPECIFIED_50,
    NAL_UNIT_UNSPECIFIED_51,
    NAL_UNIT_UNSPECIFIED_52,
    NAL_UNIT_UNSPECIFIED_53,
    NAL_UNIT_UNSPECIFIED_54,
    NAL_UNIT_UNSPECIFIED_55,
    NAL_UNIT_UNSPECIFIED_56,
    NAL_UNIT_UNSPECIFIED_57,
    NAL_UNIT_UNSPECIFIED_58,
    NAL_UNIT_UNSPECIFIED_59,
    NAL_UNIT_UNSPECIFIED_60,
    NAL_UNIT_UNSPECIFIED_61,
    NAL_UNIT_UNSPECIFIED_62,
    NAL_UNIT_UNSPECIFIED_63,
    NAL_UNIT_INVALID,
}ENalUnitType;


//NAL��Ԫ����
enum
{
    NAL_SLICE_NOPART = 1,    //��IDRͼ��
    NAL_SLICE_IDR,           //IDRͼ���е�Ƭ
    NAL_SVC,                 //��IDRͼ��SVC��ǿ�����Ƭ
    NAL_IDR_SVC,             //IDRͼ��SVC��ǿ�����Ƭ
    NAL_SURVCILIANCE_EXT,     //��ز�����չ
    NAL_SVAC_SEI,                 //������ǿ��Ϣ
    NAL_SEQ_SET,             //���в�����
    NAL_PIC_SET,             //ͼ�������
    NAL_SEC_SET,             //��ȫ������
    NAL_END_SEQ,             //���н���
    NAL_END_STREAM,          //��������
    NAL_FILTER,              //���
    NAL_CUSTOM_SET = 14      //������δʹ��
};

//svac profile����
#define SVAC_BASELINE    0x11
#define SVAC_MAIN        0x22
#define SVAC_HIGH        0x33

// NAL����
enum
{
    NAL_SLICE = 1,
    NAL_PARTA,
    NAL_PARTB,
    NAL_PARTC,
    NAL_IDRSLICE,
    NAL_SEI,
    NAL_SPS,
    NAL_PPS,    
    NAL_DELIMITER,
    NAL_FILL = 12
};

#define NAL_KEDA 26
#define NAL_CUSTOM_GPS 30
#define BASELINE    66
#define MAIN        77
#define HIGH        100
#define HIGH10      110
#define HIGH422     122

#define START_CODE          0x00000100		// start code , 27 bits
#define VO_START_CODE       0x00000100	
#define VOL_START_CODE	    0x00000120	
#define VO_SEQ_START_CODE   0x000001b0
#define USERDATA_START_CODE 0x000001b2
#define GOV_START_CODE      0x000001b3
#define VSO_START_CODE      0x000001b5		// Visual Object start code
#define VOP_START_CODE      0x000001b6
#define SLICE_START_CODE    0x000001b7

#define VOL_TYPE_DEFAULT                            0
#define VOL_TYPE_SIMPLE                             1
#define VOL_TYPE_SIMPLE_SCALABLE                    2
#define VOL_TYPE_CORE                               3
#define VOL_TYPE_MAIN                               4
#define VIDEOJLAY_TYPE_CORE_STUDIO                  16
#define VIDOBJLAY_TYPE_ASP                          17

// Aspect ratio
#define VOL_AR_EXTPAR               15
//Shape
#define VOL_SHAPE_RECTANGULAR       0
#define VOL_SHAPE_BINARY            1
#define VOL_SHAPE_BINARY_ONLY       2
#define VOL_SHAPE_GRAYSCALE         3

#define VIDEO_SUCCESS 0
#define MARKER()    BsSkip(ptBs, 1)

/* picture structure */
#define TOP_FIELD     1
#define BOTTOM_FIELD  2
#define FRAME_PICTURE 3

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

/* scalable_mode */
#define SC_NONE 0
#define SC_DP   1
#define SC_SPAT 2
#define SC_SNR  3
#define SC_TEMP 4

#define PICTURE_START_CODE      0x100
#define SLICE_START_CODE_MIN    0x101
#define SLICE_START_CODE_MAX    0x1AF
#define USER_DATA_START_CODE    0x1B2
#define SEQUENCE_HEADER_CODE    0x1B3
#define SEQUENCE_ERROR_CODE     0x1B4
#define EXTENSION_START_CODE    0x1B5
#define SEQUENCE_END_CODE       0x1B7
#define GROUP_START_CODE        0x1B8

/* extension start code IDs */
#define SEQUENCE_EXTENSION_ID                    1
#define SEQUENCE_DISPLAY_EXTENSION_ID            2
#define QUANT_MATRIX_EXTENSION_ID                3
#define COPYRIGHT_EXTENSION_ID                   4
#define SEQUENCE_SCALABLE_EXTENSION_ID           5
#define PICTURE_DISPLAY_EXTENSION_ID             7
#define PICTURE_CODING_EXTENSION_ID              8
#define PICTURE_SPATIAL_SCALABLE_EXTENSION_ID    9
#define PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID  10

const u8 scan[2][64] =
{
    { /* Zig-Zag scan pattern  */
        0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
			12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
			35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
			58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
    },
    { /* Alternate scan pattern */
			0,8,16,24,1,9,
				2,10,17,25,32,40,48,56,57,49,
				41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
				51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
				53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63
		}
};
#define ZIG_ZAG                                  0

static const u8 default_intra_quantizer_matrix[64] =
{
    8, 16, 19, 22, 26, 27, 29, 34,
		16, 16, 22, 24, 27, 29, 34, 37,
		19, 22, 26, 27, 29, 34, 34, 38,
		22, 22, 26, 27, 29, 34, 37, 40,
		22, 26, 27, 29, 32, 35, 40, 48,
		26, 27, 29, 32, 35, 40, 48, 58,
		26, 27, 29, 34, 38, 46, 56, 69,
		27, 29, 35, 38, 46, 56, 69, 83
		
};

typedef struct 
{
  /* bit input */
  u8 u8Rdbfr[2048];
  u8 *pu8Rdptr;
  /* from mpeg2play */
  unsigned int u32Bfr;
  u8 *pu8Rdmax;
  s32 l32Incnt;
  /* sequence header and quant_matrix_extension() */
  s32 al32IntraQuantizerMatrix[64];
  s32 al32NonIntraQuantizerMatrix[64];
  s32 al32ChromaIntraQuantizerMatrix[64];
  s32 al32ChromaNonIntraQuantizerMatrix[64];
  
  s32 l32LoadIntraQuantizerMatrix;
  s32 l32LoadNonIntraQuantizerMatrix;
  s32 l32LoadChromaIntraQuantizerMatrix;
  s32 l32LoadChromaNonIntraQuantizerMatrix;

  /* sequence scalable extension */
  s32 l32ScalableMode;
  /* picture coding extension */
  s32 l32QScaleType;
  s32 l32AlternateScan;
  /* picture spatial scalable extension */
  s32 l32PicScal;
  /* slice/macroblock */
  s32 l32QuantizerScale;
  s32 l32IntraSlice;
  s16 s16block[12][64];
  s16 *ps16TransData;
}layer_data;

typedef struct  
{
    s32 l32FillBufferCounter;
    u8 *pu8BsBufferInput;
    layer_data base;
    s32 l32BitstreamLen;
    s32 l32PicHeadPos;
    s32 l32BitCount;
    s32 l32EndBit;
}TMp2BitStream;

typedef struct
{
    s32 l32HorizontalSize;
    s32 l32VerticalSize;
    s32 l32AspectRatioInformation;
    s32 l32FrameRateCode; 
    s32 l32BitrateVvalue; 
    s32 l32VbvBufferSize;
    s32 l32ConstrainedParametersFlag;
}TSeqHeader;

typedef struct
{
    s32 l32TemporalReference;
    s32 l32PicCodingType;
    s32 l32VbvDelay;
    s32 l32FullPelFwVector;
    s32 l32ForwardFCode;
    s32 l32FullPelBwVector;
    s32 l32BackwardFCode;
}TPicHeader;

typedef struct
{
    s32 l32ProfileandLevelIndication;
    s32 l32ProgressiveSequence;
    s32 l32ChromaFormat;
    s32 l32LowDelay;
    s32 l32FrameRateExtensionN;
    s32 l32FrameRateExtensionD;
}TSeqExt;

typedef struct
{
    s32 l32DropFlag;
    s32 l32Hour;
    s32 l32Minute;
    s32 l32Sec;
    s32 l32Frame;
    s32 l32ClosedGop;
    s32 l32BrokenLink;
}TGOPHeader;

typedef struct
{
    s32 l32TemporalReferenceGOPReset;
    s32 l32TemporalReferenceBase;
    s32 l32TrueFramenumMax;
    s32 l32TemporalReferenceWrap;
    s32 l32TemporalReferenceOld;
}TTempRef;

typedef struct
{
    s32 al32F_Code[2][2];
    s32 l32IntraDCPrecision;
    s32 l32PicStructure;
    s32 l32TopFieldFirst;
    s32 l32FramePredFrameDct;
    s32 l32ConcealmentMV;
    s32 l32IntraVlcFormat;
    s32 l32RepeatFirstField;
    s32 l32Chroma420Type;
    s32 l32ProgressiveFrame;
    s32 l32CompositeDispFlag;
    s32 l32VAxis;
    s32 l32FieldSequence;
    s32 l32SubCarrier;
    s32 l32BurstAmplitude;
    s32 l32SubCarrierPhase;
}TPicCodingExt;

typedef struct
{
    s32 l32FrameCenterHorzOffset[3];
    s32 l32FrameCenterVertOffset[3];
}TPicDispExt;

typedef struct
{
    s32 l32LayerID;
    s32 l32LowerLayerPredHorzSize;
    s32 l32LowerLayerPredVertSize;
    s32 l32HorzSubsamplingFactorM;
    s32 l32HorzSubsamplingFactorN;
    s32 l32VertSubsamplingFactorM;
    s32 l32VertSubsamplingFactorN;
}TSeqScalExt;

typedef struct
{
    s32 l32LowerLayerTemporalRef;
    s32 l32LowerLayerHorzOffset;
    s32 l32LowerLayerVertOffset;
    s32 l32SpatTempWeightCodeTabIdx;
    s32 l32LowerLayerProgressiveFrame;
    s32 l32LowerLayerDeinterlacedFieldSlt;
}TPicSpatScalExt;

typedef struct
{
    s32 l32CopyrightFlag;
    s32 l32CopyrightIdentifier;
    s32 l32OriginalOrCopy;
    s32 l32CopyrightNumber1;
    s32 l32CopyrightNumber2;
    s32 l32CopyrightNumber3;
}TCopyRightExt;

typedef struct
{
    s32 l32VideoFormat;  
    s32 l32ColorDescription;
    s32 l32ColorPrimaries;
    s32 l32TransferCharacteristics;
    s32 l32MatrixCoefficients;
    s32 l32DisplayHorzSize;
    s32 l32DisplayVertSize;
}TSeqDispExt;

typedef struct
{
    s32 l32CodedPicWidth;
    s32 l32CodedPicHeight;
    s32 l32ChromaWidth;
    s32 l32ChromaHeight;

    /* header beagin */
    TSeqHeader tSeqHeader;
    TSeqExt tSeqExt;
    TSeqDispExt tSeqDisExt;
    TPicHeader tPicHeader;
    TPicCodingExt tPicCodingExt;
    TPicSpatScalExt tPicSpatScalExt;
    TPicDispExt tPicDispExt;
    TSeqScalExt tSeqScalExt;
    TCopyRightExt tCopyRightExt;
    TGOPHeader tGOPHeader;
    /* header end */
    TMp2BitStream tBs;
    TTempRef tTempRef;
}TMPEG2Decoder;

// �������ṹ
typedef struct
{
    u8 *pu8Buffer;                  // ����bufferָ��
    unsigned int *pu32Tail;                  // ����bufferβ��ָ��
    s32 l32BufferLen;               // �������ȣ���bit���ȼ��㣩
    unsigned int u32BufA;                    // �����32 bit
    unsigned int u32BufB;                    // �����32 bit
    s32 l32Pos;                     // ��ǰ����λ��
} TBitstream;

// ����buffer�ṹ
typedef struct
{
    u8 *pu8Buffer;                  // ����bufferָ��
    s32 l32BufferLen;               // ��������
    u8 *pu8SliceStart;              // slice�Ŀ�ʼָ��
} TBitstreamBuf;

//������Ϣ�ṹ
typedef struct
{
    unsigned int u32Cur;      //��ǰ��������
    unsigned int u32Next;     //�ֽڷ�ת����������
    unsigned int u32Pos;      //����λ��
    unsigned int *u32Ptr;     //����ָ��
    unsigned int *u32Buf;     //����Buffer
    unsigned int u32Length;   //��������
    unsigned int u32DecBsErr; //�������������
    unsigned int u32Offset;   //ƫ��
} TBitReader;

typedef struct
{
    s32 l32IsIFrame; //�Ƿ�I֡(1:�ǣ�0����
    s32 l32Width;    //ͼ����
    s32 l32Height;   //ͼ��߶�
}TVideoInfo;

static const u8 au8ZzScan[16]  =
{  
    0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const u8 au8ZzScan8[64] =
{  
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static unsigned int Bswap32(unsigned int u32Data)
{
	return ((((u32Data) >> 24) & 0xFF) | (((u32Data) >> 8) & 0xFF00) | (((u32Data) << 8) & 0xFF0000) | (((u32Data) << 24) & 0xFF000000));
}

/*====================================================================
������      ��  BitstreamShowBits
����        ��  ��һ������bit
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
                l32NBits����ȡ��bit��Ŀ[in]
����ֵ˵��  ��  bit����ʾ����ֵ
====================================================================*/
static unsigned int BitstreamShowBits(TBitstream *ptBs, s32 l32NBits)
{
    s32 l32NewBit = (l32NBits + ptBs->l32Pos) - 32;
    s32 l32Val;
    
    if(l32NewBit > 0) 
    {
        l32Val = ((ptBs->u32BufA & (0xffffffff >> ptBs->l32Pos)) << l32NewBit) | 
            (ptBs->u32BufB >> (32 - l32NewBit));
    } 
    else 
    {
        l32Val = (ptBs->u32BufA & (0xffffffff >> ptBs->l32Pos)) >> (32 - ptBs->l32Pos - l32NBits);
    }

    return l32Val;
}

/*====================================================================
������      ��  BitstreamSkip
����        ��  ����һ������bit
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
                l32NBits����������bit��Ŀ[in]
����ֵ˵��  ��  ��
====================================================================*/
static void BitstreamSkip(TBitstream *ptBs, s32 l32NBits)
{
    unsigned int u32Tmp;

    ptBs->l32Pos += l32NBits;
    
    if(ptBs->l32Pos >= 32) 
    {
        ptBs->u32BufA = ptBs->u32BufB;
        u32Tmp = *ptBs->pu32Tail ++;
        u32Tmp = Bswap32(u32Tmp);
        ptBs->u32BufB = u32Tmp;
        ptBs->l32Pos -= 32;
    }
}

/*====================================================================
������      ��  BitstreamGetBits
����        ��  ��ȡһ������bit
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
                l32NBits����������bit��Ŀ[in]
����ֵ˵��  ��  bit����ʾ����ֵ
====================================================================*/
static unsigned int BitstreamGetBits(TBitstream *ptBs, s32 l32NBits)
{
    unsigned int u32Tmp;

    u32Tmp = BitstreamShowBits(ptBs, l32NBits);

    BitstreamSkip(ptBs, l32NBits);

    return u32Tmp;
}

/*====================================================================
������      ��  BitstreamGetBit
����        ��  ��ȡһ��bit
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
����ֵ˵��  ��  bit����ʾ����ֵ
====================================================================*/
static unsigned int BitstreamGetBit(TBitstream *ptBs)
{
    return BitstreamGetBits(ptBs, 1);
}

/*====================================================================
������      ��  BitstreamReadUe
����        ��  ����ue�ķ�ʽ��ȡ����
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
����ֵ˵��  ��  ue����ʾ����ֵ
====================================================================*/
static unsigned int BitstreamReadUe(TBitstream *ptBs)
{
    s32 l32M, l32Info;
    
    l32M = 0;
    
    while (!BitstreamGetBits(ptBs, 1))
    {
        l32M ++;
    }
    
    if (l32M == 0)
        return 0;
    
    l32Info = BitstreamGetBits(ptBs, l32M);
    
    return (1 << l32M) + l32Info - 1;
}

/*====================================================================
������      ��  BitstreamReadSe
����        ��  ����se�ķ�ʽ��ȡ����
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[in]
����ֵ˵��  ��  se����ʾ����ֵ
====================================================================*/
static unsigned int BitstreamReadSe(TBitstream *ptBs)
{
    s32 l32Tmp;
    s32 l32Val;

    l32Tmp = BitstreamReadUe(ptBs);
    
    if(l32Tmp & 1)
    {
        l32Val = (l32Tmp + 1) >> 1;
    }
    else
    {
        l32Val = - (l32Tmp >> 1);
    }
    
    return l32Val;
}

/*====================================================================
������      ��  BitstreamResync
����        ��  �ؽ�������ͬ��
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBsBuf������bufferָ��[in]
����ֵ˵��  ��  �ɹ�����VIDEO_SUCCESS��ʧ�ܷ��ش�����
====================================================================*/
static s32 BitstreamResync(TBitstreamBuf *ptBsBuf)
{    
    u8 *pu8Buf = ptBsBuf->pu8SliceStart;
    u8 *pu8End = ptBsBuf->pu8Buffer + ptBsBuf->l32BufferLen;
    s32 l32Found = 0;

	while(pu8Buf + 2 < pu8End)
    {
		if((pu8Buf[0] == 0x00) && (pu8Buf[1] == 0x00) && (pu8Buf[2] == 0x01))
        {
            l32Found = 1;
            ptBsBuf->pu8SliceStart = pu8Buf;
            break;
        }
        else
        {
            pu8Buf++;
        }
    }

    return l32Found;
}

static int BitScanForward(s32 l32Data)
{
	unsigned int u32Data = (unsigned int)l32Data;
	s32 l32Res = 0;
	if(u32Data == 0)
	{
		 l32Res = 32;
	}
	else
	{
		while(!(u32Data & 0x01))
		{
			u32Data = (u32Data >> 1);
			l32Res++;
		}
		return l32Res + 1;
	}
	return 0;
}
/*====================================================================
������      ��  BitstreamInit
����        ��  �ؽ���ṹ��ʼ��
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs���ؽ���ṹָ��[out]
                pu8Buf������bufferָ��[in]
                l32Len������buffer����[in]
����ֵ˵��  ��  ��
====================================================================*/
static void BitstreamInit(TBitstream *ptBs, u8 *pu8Buf, s32 l32Len)
{
    s32 l32Index;
    s32 l32End;
    unsigned int *pu32Start;
    unsigned int u32Tmp;
    s32 l32PadLen;

    // ��������32bit����
    pu32Start = (unsigned int*)((unsigned long)pu8Buf & (unsigned long)~3);
    l32PadLen = (unsigned long)pu8Buf - (unsigned long)pu32Start;
    u32Tmp = pu32Start[0];
    u32Tmp = Bswap32(u32Tmp);

    ptBs->pu8Buffer = pu8Buf;
    ptBs->l32Pos = l32PadLen * 8;
    ptBs->u32BufA = u32Tmp;
    u32Tmp = pu32Start[1];
    u32Tmp = Bswap32(u32Tmp);
    ptBs->u32BufB = u32Tmp;
    ptBs->pu32Tail = pu32Start + 2;

    // Ѱ��slice��stopλ
    l32End = pu8Buf[l32Len - 1];

    //��β�ֽڵ����һ������bit��������������λ��
	l32Index = BitScanForward(l32End);

    ptBs->l32BufferLen = l32Len * 8 - l32Index;
}

/*====================================================================
������      ��  BitstreamBufInit
����        ��  ��ʼ���ؽ�������buffer
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBsBuf������bufferָ��[in]
                pu8BsBuffer������bufferָ��[in]
                l32BsLen������buffer����[in]
����ֵ˵��  ��  ��
====================================================================*/
static void BitstreamBufInit(TBitstreamBuf *ptBsBuf, u8 *pu8BsBuffer, s32 l32BsLen)
{
// 	u8 *pu8End = pu8BsBuffer + l32BsLen;
//     // Ϊһ֡��β����4���ֽڵ���ʼ��ȷ�����һ��slice�ܱ�����
//     //*(unsigned int *)(pu8BsBuffer + l32BsLen) = 0x01000000;
// 	
// 	//Ϊһ֡��β����3���ֽڵ���ʼ��ȷ�����һ��slice�ܱ�����
// 	pu8End[0] = 0x00;
// 	pu8End[1] = 0x00;
// 	pu8End[2] = 0x01;
// 
//     ptBsBuf->pu8Buffer = pu8BsBuffer;
//     ptBsBuf->l32BufferLen = l32BsLen + 3;
// 
//     ptBsBuf->pu8SliceStart = pu8BsBuffer;

	ptBsBuf->pu8Buffer = pu8BsBuffer;
    ptBsBuf->l32BufferLen = l32BsLen;
	
    ptBsBuf->pu8SliceStart = pu8BsBuffer;
}

/*====================================================================
������      ��  BitstreamDecodeNal
����        ��  �ؽ���NAL��Ԫ��ȥ�������룬������NAL���ͣ�
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  
                ptBsBuf������bufferָ��[in]
����ֵ˵��  ��  ��
====================================================================*/
static void BitstreamDecodeNal(TBitstream *ptBs, TBitstreamBuf *ptBsBuf)
{
    u8 *pu8End;
    u8 *pu8Start;
    u8 *pu8Write;
    unsigned int u32Code1;
    unsigned int u32Code2;
    u8 u8Code;
    s32 l32RealLen;
    s32 l32SliceLen;
    unsigned int u32Mark=0;
    s32 l32ZeroNum = 0;
	BOOL32 bFindFlag = FALSE;

    // ��ȡһ��slice���ȣ�ͬʱȥ��������
    pu8End = ptBsBuf->pu8Buffer + ptBsBuf->l32BufferLen;
    pu8Start = ptBsBuf->pu8SliceStart + 3;      // ������ʼ��
    pu8Write = pu8Start;
    u32Code1 = 0xffffff00;                      // ���ڼ�⾺����
    u32Code2 = 0xffffffff;                      // ���ڼ����һ��slice����ʼ��

    while(pu8Start < pu8End)
    {
        u8Code = *pu8Start ++;

        u32Code1 = (u32Code1 | u8Code) << 8;
        if(u32Code1 == 0x00000300)              // ������
        {
            u32Code1 = 0xffffff00;
            u32Code2 |= 0xff;

            u32Mark = 1;
            continue;
        }

        u32Code2 = ((u32Code2 << 8) | u8Code) & 0x00FFFFFF;
        if(u32Code2 == 0x00000001)              // ��һ��slice����ʼ��
        {
			bFindFlag = TRUE;
            break;
        }

        //��¼��ʼ�������������м�¼
        if(u32Code2 == 0)
        {
            l32ZeroNum++;
        }

        if(u32Mark)
        {
            *pu8Write ++ = u8Code;
        }
		else
        {
            pu8Write ++;
        }
    }

    // ȥ����һ��slice����ʼ�볤��
    l32RealLen = pu8Write - ptBsBuf->pu8SliceStart - 2 - l32ZeroNum;
	if (TRUE == bFindFlag)
	{
		l32SliceLen = pu8Start - ptBsBuf->pu8SliceStart - 3;
	}
 	else
 	{
 		l32SliceLen = pu8Start - ptBsBuf->pu8SliceStart ;
 	}
    
    BitstreamInit(ptBs, ptBsBuf->pu8SliceStart + 3, l32RealLen - 3);

    // ָ����һ��slice�Ŀ�ʼ��
    ptBsBuf->pu8SliceStart += l32SliceLen;
}

/*====================================================================
������       :	ScalingList 
����         :	�������scalingList�����ֵ
�㷨ʵ��     :	����ѡ�
����ȫ�ֱ��� :	��
�������˵�� :	pu8ScalingList:  ScailingList����[in]
                                          u8SizeOfScalingList: ScailingList����Ĵ�С[in]
                                          pu8UseDefaultScalingMatrix: �Ƿ�ʹ��Ĭ�ϵ�ScailingList����ı�־[in]
                                          ptBs: �ؽ���ṹָ��[in]
����ֵ˵��   :	�ɹ�����VIDEO_SUCCESS��ʧ�ܷ��ش�����
====================================================================*/
static void ScalingList(u8 *pu8ScalingList, u8 u8SizeOfScalingList, u8 *pu8UseDefaultScalingMatrix, TBitstream *ptBs)
{
    s32 l32J, l32Scanj;
    s32 l32DeltaScale, l32LastScale, l32NextScale;

    l32LastScale = 8;
    l32NextScale = 8;

    for(l32J = 0; l32J < u8SizeOfScalingList; l32J++)
    {
        l32Scanj = (u8SizeOfScalingList == 16) ? au8ZzScan[l32J] : au8ZzScan8[l32J];

        if(l32NextScale != 0)
        {
            l32DeltaScale = BitstreamReadSe(ptBs);
            l32NextScale = (l32LastScale + l32DeltaScale + 256) % 256;
            *pu8UseDefaultScalingMatrix = (s32)(l32Scanj==0 && l32NextScale==0); //(Boolean)
        }

        pu8ScalingList[l32Scanj] = (l32NextScale==0) ? l32LastScale : l32NextScale;
        l32LastScale = pu8ScalingList[l32Scanj];
    }
}

/*====================================================================
������      ��  BitstreamSPS
����        ��  ����SPS
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs��������Ϣ���[in]
����ֵ˵��  ��  �ɹ�����VIDEO_SUCCESS��ʧ�ܷ��ش�����
====================================================================*/
static s32 BitstreamSPS(TBitstream *ptBs, s32 *pl32Width, s32 *pl32Height)
{
	s32 l32RetCode = 0;
	s32 l32Val, l32CropUnitX, l32CropUnitY;
    s32 l32Profile, l32CheckPara;
	s32 l32Index;
    s32 l32ScalingList;
    u8 u8SeqScalingMatrixPresentFlag, u8SeqScalingListPresentFlag;
    u8 u8PicOrderCntType, u8FrameMbsOnlyFlag;
    s16 s16Width, s16Height, s16CropLeft, s16CropRight, s16CropTop, s16CropBottom;
    u8 au8ScalingList4x4[6][16];         //Inter��Intra��4x4���ű����б�                      
    u8 au8ScalingList8x8[2][64];         //Inter��Intra��8x8���ű����б�                
    u8 au8UseDefaultScalingMatrix4x4Flag[6]; //�Ƿ�ʹ��Ĭ�ϵ�4x4�����б�
    u8 au8UseDefaultScalingMatrix8x8Flag[2]; //�Ƿ�ʹ��Ĭ�ϵ�8x8�����б�
    
#define CHECKSPS(x, err) if((x)){ l32RetCode = err; goto Done; }

	l32Val = BitstreamGetBits(ptBs, 24);
    l32Profile = ((unsigned int)l32Val >> 16);
    l32CheckPara = (l32Profile == BASELINE) || (l32Profile == MAIN) || (l32Profile == HIGH) || (l32Profile == HIGH10) || (l32Profile == HIGH422);
    CHECKSPS(l32CheckPara != 1, -1);   // ����֧�ֵ�profile

    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val > 31, -1);

    //high_profile
    if(l32Profile == HIGH || l32Profile == HIGH10 || l32Profile == HIGH422)
    {
       //chroma_format_idc
       l32Val = BitstreamReadUe(ptBs);
       CHECKSPS((l32Val != 0 && l32Val != 1 &&l32Val != 2), -1);

       //bit_depth_luma_minus8Ϊ0
       l32Val = BitstreamReadUe(ptBs);
       //bit_depth_chroma_minus8Ϊ0
       l32Val = BitstreamReadUe(ptBs);

       //lossless_qpprime_flagΪ0
       l32Val = BitstreamGetBit(ptBs);  //������

       u8SeqScalingMatrixPresentFlag = BitstreamGetBit(ptBs);

       if(u8SeqScalingMatrixPresentFlag)
       {
           l32ScalingList = 8; //(ptSps->chroma_format_idc != YUV444) ? 8 : 12;

           for(l32Index = 0; l32Index < l32ScalingList; l32Index++)
           {
               u8SeqScalingListPresentFlag = BitstreamGetBit(ptBs);

               if(u8SeqScalingListPresentFlag)
               {
                   if(l32Index < 6)
                   {
                       ScalingList(au8ScalingList4x4[l32Index], 16, &au8UseDefaultScalingMatrix4x4Flag[l32Index], ptBs);
                   }
                   else
                   {
                       ScalingList(au8ScalingList8x8[l32Index - 6], 64, &au8UseDefaultScalingMatrix8x8Flag[l32Index - 6], ptBs);
                   }
               }
           }
       }
    }

    //high_profile
    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val > 12, -1);  

    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val > 2, -1);
    u8PicOrderCntType = (u8)l32Val;

    //���Ӵ�����
    if(u8PicOrderCntType == 0)
    {
        // log2_max_pic_order_cnt_lsb_minus4
        l32Val = BitstreamReadUe(ptBs);
        CHECKSPS(l32Val > 12, -1);
    }
    else if(u8PicOrderCntType == 1)
    {
        // delta_pic_order_always_zero_flag
        BitstreamGetBit(ptBs);
        
		// offset_for_non_ref_pic
        l32Val = BitstreamReadSe(ptBs);

        // offset_for_top_to_bottom_field
        l32Val = BitstreamReadSe(ptBs);
        
		// num_ref_frames_in_pic_order_cnt_cycle
        l32Val = BitstreamReadUe(ptBs);
        CHECKSPS(l32Val > 256, -1);
		for(l32Index = 0; l32Index < l32Val; l32Index++)
        {
            // offset_for_ref_frame[i]
            BitstreamReadSe(ptBs);
        }
    }

    // num_ref_frames
    l32Val = BitstreamReadUe(ptBs);

    // gaps_in_frame_num_value_allowed_flag(�����Ľ��)
    l32Val = BitstreamGetBit(ptBs);

    // pic_width_in_mbs_minus1
    l32Val = BitstreamReadUe(ptBs);
    l32Val = (l32Val + 1) * 16;
    s16Width = (s16)l32Val;

    // pic_height_in_mbs_minus1
    l32Val = BitstreamReadUe(ptBs);
    l32Val = (l32Val + 1) * 16;
    s16Height = (s16)l32Val;

    // frame_mbs_only_flag
    l32Val = BitstreamGetBit(ptBs);
    u8FrameMbsOnlyFlag = (u8)l32Val;
    //CHECKSPS(l32Val == 0, -1);

    s16Height = s16Height * (2 - l32Val);

    if(!u8FrameMbsOnlyFlag)
    {
        //mb_adaptive_frame_field_flag(�����Ľ��)
        l32Val = BitstreamGetBit(ptBs);
    }
    
    // direct_8x8_inference_flag(�����Ľ��)
    l32Val = BitstreamGetBit(ptBs);

    // frame_cropping_flag
    l32Val = BitstreamGetBit(ptBs);
	if(l32Val == 1)
	{
		l32CropUnitX = 2;
		l32CropUnitY = 2 * (2 - u8FrameMbsOnlyFlag);
		s16CropLeft = (s16)(BitstreamReadUe(ptBs) * l32CropUnitX);
		s16CropRight = (s16)(BitstreamReadUe(ptBs) * l32CropUnitX);
		s16CropTop = (s16)(BitstreamReadUe(ptBs) * l32CropUnitY);
		s16CropBottom = (s16)(BitstreamReadUe(ptBs) * l32CropUnitY);
        
        *pl32Width = s16Width - s16CropLeft - s16CropRight;
        *pl32Height = s16Height - s16CropTop - s16CropBottom;
	}
	else
	{
        *pl32Width = s16Width;
        *pl32Height = s16Height;
	}

	// vui_parameters_present_flag
	//l32Val = BitstreamGetBits(ptBs, 1);

Done:
	return l32RetCode;

#undef CHECKSPS
}

/*====================================================================
������      ��  H264DecBsHeader
����        ��  �ؽ���һ֡
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  pu8BsBuffer������bufferָ��[in]
                l32BsLen������buffer����[in]
                ptH264Info����ߵ���Ϣ
����ֵ˵��  ��  �ɹ�����VIDEO_SUCCESS��ʧ�ܷ��ش�����
====================================================================*/
static s32 H264DecBsHeader(u8* pu8BsBuffer, s32 l32BsLen, TVideoInfo *ptH264Info)
{
    s32 l32RetCode = 0;
    s32 l32NalType, l32BsSrcLen;
    u8 *pu8Src, *pu8BsSrc;
    u8 u8SPSFlag = FALSE, u8PPSFlag = FALSE;
	s16 s16FirstMBNum = 0;
	s32 l32Val = 0;

#define CHECKSLICEERR(ret) if(ret != 0) { goto SliceErr; }
    TBitstreamBuf tBsBuf;
    TBitstream tBs;

	pu8Src = pu8BsBuffer;
    //�жϴ��ģʽ
	while(((*pu8Src) == 0) && (pu8Src < (pu8BsBuffer + l32BsLen)))
    {
        pu8Src++;
    }

    if(((*pu8Src) == 0x01) && ((pu8Src - pu8BsBuffer) > 1))
    {
		pu8BsSrc = pu8Src - 2;
        l32BsSrcLen = l32BsLen - (pu8BsSrc - pu8BsBuffer);
        //Annex B ���ģʽ
        BitstreamBufInit(&tBsBuf, pu8BsSrc, l32BsSrcLen);

    }
    else
    {
        return -1;
    }
    
    while(tBsBuf.pu8SliceStart < pu8BsSrc + l32BsSrcLen)
    {
        // ����ͬ��
//         if(BitstreamResync(&tBsBuf) == 0)
//         {
//             // ��ʼ���Ҳ���������
//             l32RetCode = -1;
//             break;
//         }

        BitstreamDecodeNal(&tBs, &tBsBuf);
		// Nal type��齫�ŵ��ϲ���
        l32NalType = BitstreamGetBits(&tBs, 8);
		
        switch(l32NalType & 31)
        {
        case NAL_IDRSLICE:
			ptH264Info->l32IsIFrame = 1;
            break;
        case NAL_SLICE:
          if(u8PPSFlag && u8SPSFlag)
            {
              	s16FirstMBNum = (s32)BitstreamReadUe(&tBs);  
                // slice type
                l32Val = BitstreamReadUe(&tBs);

				if(l32Val > 4)
				{
					l32Val -= 5;
				}
				
				if(l32Val == 2)
				{
				    ptH264Info->l32IsIFrame = 1;
				}	  
            }  
            break;  

        case NAL_SPS:
			l32RetCode = BitstreamSPS(&tBs, &ptH264Info->l32Width, &ptH264Info->l32Height );
            CHECKSLICEERR(l32RetCode);
            u8SPSFlag = TRUE;
            break;
            
        case NAL_PPS:
          u8PPSFlag = TRUE;
        case NAL_SEI:
        case NAL_KEDA:
        case NAL_CUSTOM_GPS:
        case NAL_FILL:
        case NAL_PARTA:
        case NAL_PARTB:
        case NAL_PARTC:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:            
        case NAL_DELIMITER:
            break;

        default:
             break;
        }
        continue;
    }

SliceErr:
    return l32RetCode;
}

static s32 GetH264Info(u8 *pu8Stream, unsigned int u32BsLen, TVideoInfo *ptH264Info)
{
    s32 l32Ret = 0;

    if(NULL == pu8Stream)
    {
        return -1;
    }

    if(u32BsLen == 0)
    {
        return -1;
    }

	u8 *pu8H264Stream = (u8 *)malloc(u32BsLen+3);
	if (NULL == pu8H264Stream)
	{
		return -1;
	}
	memcpy(pu8H264Stream, pu8Stream, u32BsLen+3);

    ptH264Info->l32IsIFrame = 0;

    //����VOL��ͷ��Ϣ
    l32Ret = H264DecBsHeader(pu8H264Stream, u32BsLen, ptH264Info);

	if (pu8H264Stream)
	{
		free(pu8H264Stream);		
		pu8H264Stream = NULL;
	}
    return l32Ret;
}


////////////////////////////////////////getmp4info//////////////////////////////////////////////

/*=============================================================================
�� �� ��: BSWAP
��    ��: �ֽڷ�ת
ȫ�ֱ���: ��
��    ��: unsigned intVal          ����ֵ
�� �� ֵ: ������ֵ
=============================================================================*/
static unsigned int BSWAP(unsigned int u32Val)
{
    u32Val = ((u32Val >> 24) | ((u32Val & 0xff0000) >> 8) | ((u32Val & 0xff00) << 8) | ((u32Val & 0xff) << 24));
    
    return u32Val;
}

/*=============================================================================
�� �� ��: BsInit
��    ��: ��ʼ������Ϣ
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
          pu8BsBuf      ��������Buffer
          unsigned intLength     ��������
�� �� ֵ: ��
=============================================================================*/
static void BsInit(TBitReader *const ptBs, u8 *pu8BsBuf, unsigned int u32Length)
{
    unsigned int *pu32Start = NULL;
    s32 l32PadLen;

    //��������32bit����
    pu32Start = (unsigned int*)((unsigned long)pu8BsBuf & (unsigned long)~3);
    l32PadLen = (unsigned long)pu8BsBuf - (unsigned long)pu32Start;
    ptBs->u32Buf = ptBs->u32Ptr = (unsigned int *)pu32Start;
    ptBs->u32Pos = l32PadLen * 8;
    ptBs->u32Length = u32Length;
    
    ptBs->u32Cur = BSWAP(*ptBs->u32Ptr ++);
    ptBs->u32Next = BSWAP(*ptBs->u32Ptr ++);
    ptBs->u32DecBsErr = VIDEO_SUCCESS;
}

/*=============================================================================
�� �� ��: BsLength
��    ��: ������������
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
�� �� ֵ: ��������
=============================================================================*/
static unsigned int BsLength(TBitReader *const ptBs)
{
    return (((ptBs->u32Ptr - ptBs->u32Buf) << 2) - 4 - ((32 - ptBs->u32Pos) >> 3));
}

/*=============================================================================
�� �� ��: BsSkip
��    ��: ����nbit
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
          unsigned intBits       ȡ��Bit��
�� �� ֵ: ��
=============================================================================*/
static void BsSkip(TBitReader *const ptBs, const unsigned int u32Bits)
{
    unsigned int u32Consumed;
    
    ptBs->u32Pos += u32Bits;
    
    if(ptBs->u32Pos > 31) 
    {
        ptBs->u32Cur = ptBs->u32Next;
        
        ptBs->u32Next = BSWAP(*ptBs->u32Ptr++);
        ptBs->u32Pos -= 32;
        
        //��������bit��
        u32Consumed = BsLength(ptBs);
        
        if(u32Consumed > ptBs->u32Length)
        {
            ptBs->u32DecBsErr = -1; //���ý�������������
        }     
    }
}

/*=============================================================================
�� �� ��: BsByteAlign
��    ��: �ֽڶ���
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
�� �� ֵ: ��
=============================================================================*/
static void BsByteAlign(TBitReader *const ptBs)
{
    unsigned int u32Remained = ptBs->u32Pos & 7; 
    
    if(u32Remained)
    {
        BsSkip(ptBs, 8 - u32Remained);
    }
}

/*=============================================================================
�� �� ��: BsShow
��    ��: ȡ��nbitֵ(ָ�벻ǰ��)
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
          unsigned intBits       ȡ��Bit��
�� �� ֵ: nbitֵ
=============================================================================*/
static unsigned int BsShow(TBitReader *const ptBs, const unsigned int u32Bits)
{
    s32 nbit = (s32)(u32Bits + ptBs->u32Pos) - 32;
    
    if(nbit > 0) 
    {
        return ((ptBs->u32Cur & (0xffffffff >> ptBs->u32Pos)) << nbit) | (ptBs->u32Next >> (32 - nbit));
    }
    else 
    {
        return (ptBs->u32Cur & (0xffffffff >> ptBs->u32Pos)) >> (32 - ptBs->u32Pos - u32Bits);
    }
}

/*=============================================================================
�� �� ��: BsGet
��    ��: ȡ��nbitֵ(ָ��ǰ��)
ȫ�ֱ���: ��
��    ��: ptBs          ������Ϣ�ṹָ��
          unsigned intBits       ȡ��Bit��
�� �� ֵ: nbitֵ
=============================================================================*/
static unsigned int BsGet(TBitReader *const ptBs, const unsigned int u32Bits)
{
    unsigned int u32Val;
    
    u32Val = BsShow(ptBs, u32Bits);
    
    BsSkip(ptBs, u32Bits);
    
    return u32Val;
}

/*=============================================================================
�� �� ��: Log2Bin
��    ��: ��ȡLog2ֵ
ȫ�ֱ���: ��
��    ��: l32Value    Ԥ��Log2��ֵ[in]
�� �� ֵ: Log2ֵ
=============================================================================*/
static s32 Log2Bin(s32 l32Value)
{
    s32 l32M = 0;

    while(l32Value)
    {
        l32Value >>= 1;

        l32M++;
    }

    return l32M;
}

/*=============================================================================
�� �� ��: GetVolHeader
��    ��: ����VOLͷ��Ϣ
ȫ�ֱ���: ��
��    ��: ptBs  ������Ϣ�ṹָ��[in/out]  
          pl32Width ͼ����[out[
          pl32Width ͼ��߶�[out]
�� �� ֵ: VIDEO_SUCCESS �ɹ�, ���� ʧ��
=============================================================================*/
static s32 GetVolHeader(TBitReader *ptBs, s32 *pl32Width, s32 *pl32Height)
{
    unsigned int u32VolVerid, u32Width = 0, u32Height = 0;
    unsigned int u32TimeIncBits;
    unsigned int u32VoTypeInd;
    u16 u16Shape;
    s32 l32TimeResolution;

    BsSkip(ptBs, 1); // random_accessible_vol

    u32VoTypeInd = BsShow(ptBs, 8); // video_object_type_indication
        
    if((u32VoTypeInd != VOL_TYPE_SIMPLE) && (u32VoTypeInd != VIDOBJLAY_TYPE_ASP) && (u32VoTypeInd != VOL_TYPE_DEFAULT)&& (u32VoTypeInd != VIDEOJLAY_TYPE_CORE_STUDIO))
    {
        return -1;
    }

    BsSkip(ptBs, 8);

    if(BsGet(ptBs, 1)) // is_object_layer_identifier
    {
        u32VolVerid = BsGet(ptBs,4); // video_object_layer_verid
        BsSkip(ptBs, 3); // video_object_layer_priority
    }
    else
    {
        u32VolVerid = 1;
    }

    if(BsGet(ptBs, 4) == VOL_AR_EXTPAR)	// aspect_ratio_info
    {
        BsSkip(ptBs, 8); // par_width
        BsSkip(ptBs, 8); // par_height
    }

    if(BsGet(ptBs, 1)) // vol_control_parameters
    {
        BsSkip(ptBs, 2); // chroma_format
        BsSkip(ptBs, 1); // low_delay

        if(BsGet(ptBs, 1)) // vbv_parameters
        {
            unsigned int u32FirstHalfBitRate, u32LatterHalfBitRate;
            unsigned int u32FirstHalfVbvBufSize, u32LatterHalfVbvBufSize;
            unsigned int u32FisrtsHalfVbvOccup, u32LatterHalfVbvOccup;

            u32FirstHalfBitRate = BsGet(ptBs, 15); // first_half_bitrate
            MARKER();
            u32LatterHalfBitRate = BsGet(ptBs, 15);	// latter_half_bitrate
            MARKER();
            u32FirstHalfVbvBufSize = BsGet(ptBs, 15); // first_half_vbv_buffer_size
            MARKER();
            u32LatterHalfVbvBufSize = BsGet(ptBs, 3); // latter_half_vbv_buffer_size
            u32FisrtsHalfVbvOccup = BsGet(ptBs, 11); // first_half_vbv_occupancy
            MARKER();
            u32LatterHalfVbvOccup = BsGet(ptBs, 15); // latter_half_vbv_occupancy
            MARKER();
        }
    }

    u16Shape = (u16)BsGet(ptBs, 2); // video_object_layer_shape

    if(u16Shape != VOL_SHAPE_RECTANGULAR)
    {
        return -1;
    }

    MARKER();

    l32TimeResolution = BsGet(ptBs, 16); // vop_time_increment_resolution

    if(l32TimeResolution > 1)
    {
        u32TimeIncBits = Log2Bin(l32TimeResolution - 1);
    }
    else
    {
        u32TimeIncBits = 1;
    }

    MARKER();

    if(BsGet(ptBs, 1)) // fixed_vop_rate
    {
        BsSkip(ptBs, u32TimeIncBits); // fixed_vop_time_increment
    }

    if(u16Shape == VOL_SHAPE_RECTANGULAR)
    {
        MARKER();
        u32Width = BsGet(ptBs, 13);	// video_object_layer_width

        MARKER();
        u32Height = BsGet(ptBs, 13); // video_object_layer_height
        MARKER();
    }

    if(u32Width == 0 || u32Height == 0)
    {
        return -1;
    }

    *pl32Width = u32Width;
    *pl32Height = u32Height;

    return VIDEO_SUCCESS;
}

/*=============================================================================
�� �� ��: MP4BsHeaders
��    ��: ����ͷ��Ϣ
ȫ�ֱ���: ��
��    ��: ptBs                  �����ṹָ��[in]  
          ptMP4Info     ���ͷ��Ϣ�ṹ��ָ��[out]
�� �� ֵ: VIDEO_SUCCESS: �ɹ�������: ��Ӧ������
=============================================================================*/
static s32 MP4BsHeaders(TBitReader *ptBs, TVideoInfo *ptMP4Info)
{
    unsigned int u32StartCode;
    s32 l32Ret = VIDEO_SUCCESS;

    ptMP4Info->l32IsIFrame = 0;
    
    while(!ptBs->u32DecBsErr)
    {
        BsByteAlign(ptBs);
        
        u32StartCode = BsShow(ptBs, 32); //��ȡ��ʼ��
        
        if((u32StartCode & (~0x0ff)) == START_CODE)
        {
            if(u32StartCode == VO_SEQ_START_CODE)
            {
                BsSkip(ptBs, 32);
            }
            else if(u32StartCode == VSO_START_CODE)
            {
                BsSkip(ptBs, 32);
            }
            else if((u32StartCode & (~0xf)) == VOL_START_CODE) // 0x120 ... 0x12f
            {
				//����VOLͷ��Ϣ
                BsSkip(ptBs, 32);

                l32Ret = GetVolHeader(ptBs, &ptMP4Info->l32Width, &ptMP4Info->l32Height);

                if(l32Ret!= VIDEO_SUCCESS)
                {
                    ptBs->u32DecBsErr = l32Ret;
                }
            }
            else if(u32StartCode == GOV_START_CODE) //����GOVͷ
            {
                BsSkip(ptBs, 32);		
            }
            else if(u32StartCode == VOP_START_CODE)
            {
                s16 s16CodingType;

                BsSkip(ptBs, 32); // alread have the start_code in hands

                s16CodingType = (s16)BsGet(ptBs, 2);	// vop_coding_type

                ptMP4Info->l32IsIFrame = (s16CodingType == 0);

                break;
            }
            else if(u32StartCode == USERDATA_START_CODE)//�����û�����
            {
                BsSkip(ptBs, 32);
            }
            else
            {
                BsSkip(ptBs, 32);
            }
        }
        else  // continue seeking by forward 8 bits
        {
            BsSkip(ptBs, 8);
        }
    }

    return ptBs->u32DecBsErr; 
}

/*=============================================================================
�� �� ��: GetMP4Info
��    ��: ��ȡMpeg4����VOL��Ϣ
ȫ�ֱ���: ��
��    ��: pu8Stream             ��������ָ��[in]
          unsigned intBsLen              ������������[in]
          ptMP4Info             ��ȡMP4��Ϣ�ṹ��ָ��[out]
�� �� ֵ: ��Ӧ�Ĵ�����
=============================================================================*/
static s32 GetMP4Info(u8 *pu8Stream, unsigned int u32BsLen, TVideoInfo *ptMP4Info)
{
    TBitReader tBs;
    s32 l32Ret = VIDEO_SUCCESS;

    if(NULL == pu8Stream)
    {
        return -1;
    }

    if(u32BsLen == 0)
    {
        return -1;
    }
    
    //��ʼ��������Ϣ
    BsInit(&tBs, pu8Stream, u32BsLen);
    ptMP4Info->l32IsIFrame = 0;

    //����VOL��ͷ��Ϣ
    l32Ret = MP4BsHeaders(&tBs, ptMP4Info);

    return l32Ret;
}

//=========================================getsvacinfo=====================================================//

/*====================================================================
������      ��  SVACDecBitstreamParseSPS
����        ��  ����SPS��Ϣ
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs��ָ���������ָ��[in]
               ps16Width�����[out]
			   ps16PicHeight���߶�[out]
����ֵ˵��  ��  �ɹ����أ�VIDEO_SUCCESS�����󷵻���Ӧ������
====================================================================*/
static s32 SVACDecBitstreamParseSPS(TBitstream *ptBs, u8 *pu8ProgessiveSeqFlag ,s16 *ps16Width, s16 *ps16PicHeight)
{
    s32 l32RetCode = VIDEO_SUCCESS;
    s32 l32Val, l32CropUnitX, l32CropUnitY;
    s32 l32Profile, l32CheckPara;
    s32 l32Index;

#define CHECKSPS(x, err) if((x)){ l32RetCode = err; goto Done; }

    //profile_id level_id
    l32Val = BitstreamGetBits(ptBs, 16);
    l32Profile = l32Val >> 8;
    l32CheckPara = (l32Profile == SVAC_BASELINE) || (l32Profile == SVAC_MAIN) || (l32Profile == SVAC_HIGH);
    CHECKSPS(l32CheckPara != 1, -1);   // ����֧�ֵ�profile

    //seq_parameter_set_id
    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val > 31, -1);

    //chroma_format_idc
    l32Val = BitstreamGetBits(ptBs, 2);
    //ֻ֧��YUV420  l32CheckPara = (l32Val == 0) || (l32Val == 1); 
    CHECKSPS(l32Val != 1, -1);

    //bit_depth_luma_minus8
    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val != 0, -1);

    //bit_depth_chroma_minus8
    l32Val = BitstreamReadUe(ptBs);
    CHECKSPS(l32Val != 0, -1);

    // pic_width_in_mbs_minus1
    l32Val = BitstreamReadUe(ptBs);
    l32Val = (l32Val + 1) * 16;
    CHECKSPS(l32Val > 1920, -1);
    (*ps16Width) = (s16)l32Val;

    // pic_height_in_mbs_minus1 �������һ��ͼ��߶�(֡��)
    l32Val = BitstreamReadUe(ptBs);
    l32Val = (l32Val + 1) * 16;
    CHECKSPS(l32Val > 1088, -1);
    (*ps16PicHeight) = l32Val;

    // progressive_seq_flag(0:��Ϊ������Ϊ֡��1��ֻ��Ϊ֡)
    l32Val = BitstreamGetBits(ptBs, 1);
	(*pu8ProgessiveSeqFlag) = l32Val;

    //roi_flag
    l32Val = BitstreamGetBits(ptBs, 1);

    //SVC_flag
    l32Val = BitstreamGetBits(ptBs, 1);

    // vui_parameters_present_flag, ����Ϣ�ܳ�, ����Ҫ������²�������
    l32Val = BitstreamGetBits(ptBs, 1);
    //CHECKSPS(l32Val == 1, ERR_VID_SVACDEC_VUI_PARAMETERS_PRESENT_FLAG);

Done:

    return l32RetCode;

#undef CHECKSPS
}

/*====================================================================
������      SVACDecBsHeader
����        ������SPS��PPS��Ϣ������slice����ͷ��Ϣ�������������Ƿ��������ͼ��
�㷨ʵ��    �����ޣ�
����ȫ�ֱ�����  ��
�������˵����
             pu8BsBuffer: ָ��������������ָ��[in]
             l32BsLen: ��������[in]
             pl32CurPicLen: ��ǰ֡��������ָ��[in/out]
����ֵ˵��  ��  �ɹ����� VIDEO_SUCCESS��ʧ�ܷ�����Ӧ������
====================================================================*/
static s32 SVACDecBsHeader(u8 *pu8BsBuffer, s32 l32BsLen, TVideoInfo *ptSVACInfo)
{
    s32 l32RetCode = VIDEO_SUCCESS;
	u8 *pu8Src;
	u8 *pu8BsSrc;
    s32 l32BsLenLeft, l32NalType, l32NaluLength;
    s32 l32Index;
	TBitstreamBuf tBsBuf;
    TBitstream tBs;
	s32 l32SPSFlag = 0;
	s32 l32PPSFlag = 0;
	s32 l32Val;
	u8 u8ProgessiveSeqFlag = 0;
	s16 s16Width, s16Height;
	s32 l32BsSrcLen;

#define CHECKSLICEERR(ret) if(ret != VIDEO_SUCCESS) { goto SliceErr; }

	pu8Src = pu8BsBuffer;
    //�жϴ��ģʽ
	while(((*pu8Src) == 0) && (pu8Src < (pu8BsBuffer + l32BsLen)))
    {
        pu8Src++;
    }

    if(((*pu8Src) == 0x01) && ((pu8Src - pu8BsBuffer) > 1))
    {
		pu8BsSrc = pu8Src - 2;
        l32BsSrcLen = l32BsLen - (pu8BsSrc - pu8BsBuffer);
        //Annex B ���ģʽ
        BitstreamBufInit(&tBsBuf, pu8BsSrc, l32BsSrcLen);

    }
    else
    {
        return -1;
    }
    
    while(tBsBuf.pu8SliceStart < pu8BsSrc + l32BsSrcLen)
    {        
        //Ԥ��NALU
        BitstreamDecodeNal(&tBs, &tBsBuf);
        l32NalType = BitstreamGetBits(&tBs, 8);
        switch((l32NalType & 0x3C) >> 2)
        {
        case NAL_SLICE_IDR:
		ptSVACInfo->l32IsIFrame = 1;
		
		case NAL_SLICE_NOPART:
		
		if(l32SPSFlag && l32PPSFlag)
		{
            l32Val = l32NalType & 1;

            if(l32Val == 1)
            {
                l32Val = BitstreamGetBits(&tBs, 8);
            }

            l32Val = BitstreamReadUe(&tBs); //pic_parameter_set_id 
            if((l32Val >= 0) && (l32Val < 256))
            {
                //frame_num
                l32Val = BitstreamGetBits(&tBs, 8);

                if((((l32NalType & 0x3C) >> 2) == NAL_SLICE_IDR) || (((l32NalType & 0x3C) >> 2) == NAL_IDR_SVC))
                {
                    //idr_pic_id
                    l32Val = BitstreamReadUe(&tBs);
                }

                if(!u8ProgessiveSeqFlag)
                {
                    //field_pic_flag
                    l32Val = BitstreamGetBits(&tBs, 1);
                    if(l32Val)
                    {
                        //bottom_field_flag
                        l32Val = BitstreamGetBits(&tBs, 1);
                    }
                }
                else
                {
                    l32Val = 0;
                }
				
				//first_mb_in_slice
				l32Val = (s32)BitstreamReadUe(&tBs);
				
				l32Val = BitstreamReadUe(&tBs);
				if(l32Val > 2)
				{
					l32Val -= 3;
				}
				if(l32Val == 2)
				{
					ptSVACInfo->l32IsIFrame = 1;		
				}						
            }
		}            
            break;

        case NAL_SVC:
        case NAL_IDR_SVC:
            break;

        case NAL_SEI:
            break;

        case NAL_SEQ_SET:
            l32RetCode = SVACDecBitstreamParseSPS(&tBs, &u8ProgessiveSeqFlag, &s16Width, &s16Height);
            CHECKSLICEERR(l32RetCode);
			ptSVACInfo->l32Width = s16Width;
			ptSVACInfo->l32Height = s16Height;
			l32SPSFlag = 1;
            break;

        case NAL_PIC_SET:
			l32PPSFlag = 1;
            break;

        case NAL_CUSTOM_GPS:
            break;

        case NAL_SEC_SET:
            break;
        
        case NAL_FILTER:
            break;

        default:
            break;
        }
    }

SliceErr:
    return l32RetCode;

#undef CHECKSLICEERR
}

static s32 GetSVACInfo(u8 *pu8Stream, unsigned int u32BsLen, TVideoInfo *ptSVACInfo)
{
    s32 l32Ret = 0;

    if(NULL == pu8Stream)
    {
        return -1;
    }

    if(u32BsLen == 0)
    {
        return -1;
    }

	u8 *pu8SVACStream = (u8 *)malloc(u32BsLen+3);
	if (NULL == pu8SVACStream)
	{
		return -1;
	}
	memcpy(pu8SVACStream, pu8Stream, u32BsLen+3);

    ptSVACInfo->l32IsIFrame = 0;

    //����VOL��ͷ��Ϣ
    l32Ret = SVACDecBsHeader(pu8SVACStream, u32BsLen, ptSVACInfo);

	if (pu8SVACStream)
	{
		free(pu8SVACStream);		
		pu8SVACStream = NULL;
	}
    return l32Ret;
}


////////////////////////////////////////////////////////////////////////////////////////
/*=============================================================================
�� �� ��:  MemCopy64
��    ��:  ͼ�����ݿ���
ȫ�ֱ���:  ��
��    ��:  pu8Dst      Ŀ��ͼ��ָ��
           pu8Src      Դͼ��ָ��
           l32Size     ������С
�� �� ֵ:  ��
=============================================================================*/
static void MemCopy64(u8 *pu8Dst, u8 *pu8Src, s32 l32Size)
{
    u64 *pu64Src, *pu64Dst;
    s32 l32Size8, l32Index;

    l32Size8 = l32Size >> 3;

    pu64Src = (u64 *)(pu8Src);
    pu64Dst = (u64 *)(pu8Dst);

    for(l32Index = 0; l32Index < l32Size8; l32Index++)
    {
        pu64Dst[l32Index] = pu64Src[l32Index];
    }
}

/*====================================================================
������      :	FillBuffer
����	    :	������������ȡ��2K��Buffer�д���
����ȫ�ֱ���:	ld
�������˵��:	ptBs         ����ͼ��������Ϣ�ṹָ��[in]
����ֵ˵��  :	��
====================================================================*/
static void FillBuffer(TMp2BitStream *ptBs)
{
    s32 l32BufferLevel;
    layer_data *ld = &ptBs->base;
//xxx
    if(ptBs->l32BitstreamLen <= 0)
    {
        ptBs->l32EndBit = 1;

        return;
    }
    else if(ptBs->l32BitstreamLen < 2048)
    {
        l32BufferLevel = ptBs->l32BitstreamLen;
        ptBs->l32BitstreamLen = 0;
    }
    else
    {
        l32BufferLevel = 2048;
        ptBs->l32BitstreamLen -= 2048;
    }

    MemCopy64(ld->u8Rdbfr, ptBs->pu8BsBufferInput + 2048 * ptBs->l32FillBufferCounter, 2048);
    
//     if(ptBs->l32BitstreamLen < 2048)
//     {
//         l32BufferLevel = ptBs->l32BitstreamLen;
//     }
//     else
//     {
//         l32BufferLevel = 2048;
//         ptBs->l32BitstreamLen -= 2048;
//     }    

    ld->pu8Rdptr = ld->u8Rdbfr;
    ptBs->l32FillBufferCounter++;
    
    //�ж��Ƿ��������Ľ�β
    if(l32BufferLevel < 2048)
    {
        //�����ǰ�ȫ���
        if(l32BufferLevel < 0)
        {
            l32BufferLevel = 0;
        }
        
        //ƴ�ճ�32λ������ڲ���
        while(l32BufferLevel & 3)
        {
            ld->u8Rdbfr[l32BufferLevel++] = 0;
        }
        
        //�������ಿ�������н��������
        while(l32BufferLevel < 2048)
        {
            ld->u8Rdbfr[l32BufferLevel++] = SEQUENCE_END_CODE >> 24;
            ld->u8Rdbfr[l32BufferLevel++] = SEQUENCE_END_CODE >> 16;
            ld->u8Rdbfr[l32BufferLevel++] = SEQUENCE_END_CODE >> 8;
            ld->u8Rdbfr[l32BufferLevel++] = SEQUENCE_END_CODE & 0xff;
        }
    }
}

/*====================================================================
������      :	FlushBuffer
����	    :	ǰ�� l32Length������
����ȫ�ֱ���:	ld
�������˵��:	ptBs           ����ͼ��������Ϣ�ṹָ��[in]
                l32Length      ǰ��bit����[in]
����ֵ˵��  :	��
====================================================================*/
static void FlushBuffer(TMp2BitStream *ptBs, s32 l32Length)
{
    s32 l32Incnt;
    layer_data *ld = &ptBs->base;
    
    ptBs->l32BitCount += l32Length;

    ld->u32Bfr <<= l32Length;
    
    l32Incnt = ld->l32Incnt -= l32Length;
    
    if(l32Incnt <= 24)
    {
        if(ld->pu8Rdptr < (ld->u8Rdbfr + 2044))
        {
            do
            {
                ld->u32Bfr |= *ld->pu8Rdptr++ << (24 - l32Incnt);
                l32Incnt += 8;
            }while(l32Incnt <= 24);
        }
        else
        {
            do
            {
                if(ld->pu8Rdptr >= (ld->u8Rdbfr + 2048))
                {
                    FillBuffer(ptBs);
                }
                
                ld->u32Bfr |= ((*ld->pu8Rdptr++) << (24 - l32Incnt));
                l32Incnt += 8;
            }while(l32Incnt <= 24);
        }
        
        ld->l32Incnt = l32Incnt;
    }
}

/*====================================================================
������      :	FlushBuffer32
����	    :	ǰ��32������
����ȫ�ֱ���:	ld
�������˵��:	ptBs           ����ͼ��������Ϣ�ṹָ��[in]
����ֵ˵��  :	��
====================================================================*/
static void FlushBuffer32(TMp2BitStream *ptBs)
{
    s32 l32Incnt;
    layer_data *ld = &ptBs->base;
    
    ptBs->l32BitCount += 32;
    ld->u32Bfr = 0; 
    l32Incnt = ld->l32Incnt;
    l32Incnt -= 32;
    
    while(l32Incnt <= 24)
    {
        if(ld->pu8Rdptr >= ld->u8Rdbfr + 2048)
        {
            FillBuffer(ptBs);
        }
        
        ld->u32Bfr |= *ld->pu8Rdptr++ << (24 - l32Incnt);
        l32Incnt += 8;
    }
    
    ld->l32Incnt = l32Incnt;
}

/*====================================================================
������      :	GetBits
����	    :	��ȡn��bit
����ȫ�ֱ���:	ld
�������˵��:	ptBs           ����ͼ��������Ϣ�ṹָ��[in]
                l32Length      ��ȡbit����[in]
����ֵ˵��  :	�ؽ���ֵ
====================================================================*/
static unsigned int GetBits(TMp2BitStream *ptBs, s32 l32Length)
{
    unsigned int u32Val;
    s32 l32Incnt;
    layer_data *ld = &ptBs->base;
    
    ptBs->l32BitCount += l32Length;
    u32Val = ld->u32Bfr >> (32 - l32Length);
    ld->u32Bfr <<= l32Length;
    
    l32Incnt = ld->l32Incnt -= l32Length;
    
    if(l32Incnt <= 24)
    {
        if(ld->pu8Rdptr < (ld->u8Rdbfr + 2044))
        {
            do
            {
                ld->u32Bfr |= ((*ld->pu8Rdptr++) << (24 - l32Incnt));
                l32Incnt += 8;
            }
            while(l32Incnt <= 24);
        }
        else
        {
            do
            {
                if(ld->pu8Rdptr >= (ld->u8Rdbfr + 2048))
                {
                    FillBuffer(ptBs);
                }

                ld->u32Bfr |= ((*ld->pu8Rdptr++) << (24 - l32Incnt));
                l32Incnt += 8;
            }
			while(l32Incnt <= 24);
        }

        ld->l32Incnt = l32Incnt;
    }
    
    return u32Val;
}

/*====================================================================
������      :	BsMp2Init
����	    :	��ʼ��Buffer���ڵ�һ��getbits��showbitsǰ������һ��
����ȫ�ֱ���:	ld
�������˵��:	ptBs    ���������Ϣָ��[in]
����ֵ˵��  :	��
====================================================================*/
static void BsMp2Init(TMp2BitStream *ptBs)
{
    layer_data *ld = &ptBs->base;

    ld->l32Incnt = 0;
    ld->pu8Rdptr = ld->u8Rdbfr + 2048;
    ld->pu8Rdmax = ld->pu8Rdptr;
    ld->u32Bfr = 0;

    //����ʼ�����ݵ�ld->u32Bfr������
    FlushBuffer(ptBs, 0);
}

/*====================================================================
������       :	FindNextStartCode
����	     :	Ѱ����һ����ʼ��
����ȫ�ֱ��� :	��
�������˵�� :	ptBs         ����ͼ��������Ϣ�ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void FindNextStartCode(TMp2BitStream *ptBs)
{
    layer_data *ld = &ptBs->base;
    
    //�ֽڶ���
    FlushBuffer(ptBs, ld->l32Incnt & 7 );
    
    while(0x01L != (ld->u32Bfr >> (32 - 24)))
    {
        if(ptBs->l32EndBit == 1)
        {
            break;
        }
        
        FlushBuffer(ptBs, 8);
    }
}

/*====================================================================
������       :	DecExtraBitInformation
����	     :	���ڱ�����һ���ԵĲ���
����ȫ�ֱ��� :	��
�������˵�� :	ptBs         ����ͼ��������Ϣ�ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void MarkerBit(TMp2BitStream *ptBs, s8 *text)
{
	s32 l32Marker;

	l32Marker = GetBits(ptBs, 1);
}

/*====================================================================
������      :	ShowBits
����	    :	�ӱ�������������ȥ��l32Length�� bit�����ı��������ǰλ��
�㷨ʵ��    :	��
����ȫ�ֱ���:	��
�������˵��:	
����ֵ˵��  :	��
====================================================================*/
static unsigned int ShowBits(TMp2BitStream *ptBs, s32 l32Length)
{
    layer_data *ld = &ptBs->base;
    
    return ld->u32Bfr >> (32 - l32Length);
}

/*====================================================================
������       :	DecExtraBitInformation
����	     :	������չ������Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��չ�ֽ���
====================================================================*/
static s32 DecExtraBitInformation(TMPEG2Decoder *ptMp2Decoder)
{
    TMp2BitStream* ptBs = &ptMp2Decoder->tBs;
    s32 l32ByteCount = 0;

    while(GetBits(ptBs, 1))
    {
        FlushBuffer(ptBs, 8);

        l32ByteCount++;
    }

    return l32ByteCount;
}

/*====================================================================
������       :	UpdateTemporalReferenceTackingData
����	     :	ʱ��ο�����
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void UpdateTemporalReferenceTackingData(TMPEG2Decoder *ptMp2Decoder)
{
    TPicHeader *ptPicHeader = &ptMp2Decoder->tPicHeader;
    TTempRef *ptTempRef = &ptMp2Decoder->tTempRef;

    //����һ��
    if(B_TYPE != ptPicHeader->l32PicCodingType && ptPicHeader->l32TemporalReference != ptTempRef->l32TemporalReferenceOld)
    {							
        //��B֡
        if(ptTempRef->l32TemporalReferenceWrap) 		
        {
            // wrap ��������ǰ�� I- �� P-֡
            ptTempRef->l32TemporalReferenceBase += 1024;
            ptTempRef->l32TemporalReferenceWrap  = 0;
        }

        // ���������õ�������ֿ���
        if(ptPicHeader->l32TemporalReference < ptTempRef->l32TemporalReferenceOld && !ptTempRef->l32TemporalReferenceGOPReset)
        {
            ptTempRef->l32TemporalReferenceWrap = 1;
        }

        ptTempRef->l32TemporalReferenceOld = ptPicHeader->l32TemporalReference;
        ptTempRef->l32TemporalReferenceGOPReset = 0;
    }

    //ptMp2Decoder->tDecVidInfo.l32TrueFramenum = ptTempRef->l32TemporalReferenceBase + ptPicHeader->l32TemporalReference;

    // temporary wrap of TR at 1024 for M frames
    //if(ptTempRef->l32TemporalReferenceWrap && ptPicHeader->l32TemporalReference <= ptTempRef->l32TemporalReferenceOld)
    //{
    //    ptMp2Decoder->tDecVidInfo.l32TrueFramenum += 1024;
    //}

    //ptTempRef->l32TrueFramenumMax = (ptMp2Decoder->tDecVidInfo.l32TrueFramenum > ptTempRef->l32TrueFramenumMax) 
    //     ? ptMp2Decoder->tDecVidInfo.l32TrueFramenum : ptTempRef->l32TrueFramenumMax;
}

/*====================================================================
������       :	DecSequenceExtension
����	     :	����������չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecSequenceExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TSeqHeader *ptSeqHeader = &ptMp2Decoder->tSeqHeader;
    TSeqExt *ptSeqExt = &ptMp2Decoder->tSeqExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32HorizontalSizeExtension;
    s32 l32VerticalSizeExtension;
    s32 l32BitrateExtension;
    s32 l32VbvBufferSizeExtension;
    layer_data *ld = &ptBs->base;
  
    ld->l32ScalableMode = SC_NONE;//��ʼ����ֱ��������sequence_scalable_extension()�޸�
    ptMp2Decoder->tSeqScalExt.l32LayerID = 0;//��ʼ����ֱ��������sequence_scalable_extension()�޸�
    
    ptSeqExt->l32ProfileandLevelIndication = GetBits(ptBs, 8);
    ptSeqExt->l32ProgressiveSequence = GetBits(ptBs, 1);
    ptSeqExt->l32ChromaFormat = GetBits(ptBs, 2);
    l32HorizontalSizeExtension = GetBits(ptBs, 2);
    l32VerticalSizeExtension = GetBits(ptBs, 2);
    l32BitrateExtension = GetBits(ptBs, 12);
    MarkerBit(ptBs, "DecSequenceExtension");
    l32VbvBufferSizeExtension = GetBits(ptBs, 8);
    ptSeqExt->l32LowDelay = GetBits(ptBs, 1);
    ptSeqExt->l32FrameRateExtensionN = GetBits(ptBs, 2);
    ptSeqExt->l32FrameRateExtensionD = GetBits(ptBs, 5);
    
    ptSeqHeader->l32HorizontalSize = (l32HorizontalSizeExtension << 12) | (ptSeqHeader->l32HorizontalSize & 0x0fff);
    ptSeqHeader->l32VerticalSize = (l32VerticalSizeExtension << 12) | (ptSeqHeader->l32VerticalSize & 0x0fff);
    
    ptSeqHeader->l32BitrateVvalue += (l32BitrateExtension << 18);
    ptSeqHeader->l32VbvBufferSize += (l32VbvBufferSizeExtension << 10);
}

/*====================================================================
������       :	DecSequenceDisplayExtension
����	     :	����������ʾ��չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecSequenceDisplayExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TSeqDispExt *ptSeqDisExt = &ptMp2Decoder->tSeqDisExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;

    ptSeqDisExt->l32VideoFormat = GetBits(ptBs, 3);
    ptSeqDisExt->l32ColorDescription = GetBits(ptBs, 1);

    if(ptSeqDisExt->l32ColorDescription)
    {
        ptSeqDisExt->l32ColorPrimaries = GetBits(ptBs, 8);
        ptSeqDisExt->l32TransferCharacteristics = GetBits(ptBs, 8);
        ptSeqDisExt->l32MatrixCoefficients = GetBits(ptBs, 8);
    }

    ptSeqDisExt->l32DisplayHorzSize = GetBits(ptBs, 14);

    MarkerBit(ptBs, "DecSequenceDisplayExtension");

    ptSeqDisExt->l32DisplayVertSize   = GetBits(ptBs, 14);
}

/*====================================================================
������       :	DecQuantMatrixExtension
����	     :	��������������չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecQuantMatrixExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32Temp;
    layer_data *ld = &ptBs->base;

    if((ld->l32LoadIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32ChromaIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]] 
            = ld->al32IntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]]
            = GetBits(ptBs, 8);
        }
    }

    if((ld->l32LoadNonIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32ChromaNonIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]]
            = ld->al32NonIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]]
            = GetBits(ptBs, 8);
        }
    }

    if((ld->l32LoadChromaIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32ChromaIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]] = GetBits(ptBs, 8);
        }
    }

    if((ld->l32LoadChromaNonIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32ChromaNonIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]] = GetBits(ptBs, 8);
        }
    }
}

/*====================================================================
������       :	DecSequenceScalableExtension
����	     :	�������зֲ���չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static s32 DecSequenceScalableExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TSeqScalExt *ptSeqScalExt = &ptMp2Decoder->tSeqScalExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    layer_data *ld = &ptBs->base;
    s32 l32Ret = VIDEO_SUCCESS;

    ld->l32ScalableMode = GetBits(ptBs, 2) + 1; /* add 1 to make SC_DP != SC_NONE */

    ptSeqScalExt->l32LayerID = GetBits(ptBs, 4);

    if(SC_SPAT == ld->l32ScalableMode)
    {
        ptSeqScalExt->l32LowerLayerPredHorzSize = GetBits(ptBs, 14);
        MarkerBit(ptBs, "DecSequenceScalableExtension()");
        ptSeqScalExt->l32LowerLayerPredVertSize = GetBits(ptBs, 14); 
        ptSeqScalExt->l32HorzSubsamplingFactorM = GetBits(ptBs, 5);
        ptSeqScalExt->l32HorzSubsamplingFactorN = GetBits(ptBs, 5);
        ptSeqScalExt->l32VertSubsamplingFactorM = GetBits(ptBs, 5);
        ptSeqScalExt->l32VertSubsamplingFactorN = GetBits(ptBs, 5);
    }

    if(SC_TEMP == ld->l32ScalableMode)
    {
        l32Ret = -1;
    }

    return l32Ret;
}

/*====================================================================
������       :	DecPicDisplayExtension
����	     :	����ͼ����ʾ��չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecPicDisplayExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TSeqExt *ptSeqExt = &ptMp2Decoder->tSeqExt;
    TPicCodingExt *ptPicCodingExt = &ptMp2Decoder->tPicCodingExt;
    TPicDispExt *ptDisExt = &ptMp2Decoder->tPicDispExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32Temp;
    s32 l32NumofFrameCenterOffsets;

    //����number_of_frame_center_offsets
    if(ptSeqExt->l32ProgressiveSequence)
    {
        if(ptPicCodingExt->l32RepeatFirstField)
        {
            if(ptPicCodingExt->l32TopFieldFirst)
            {
                l32NumofFrameCenterOffsets = 3;
            }			
            else
            {
                l32NumofFrameCenterOffsets = 2;
            }
        }
        else
        {
            l32NumofFrameCenterOffsets = 1;
        }
    }
    else
    {
        if(FRAME_PICTURE != ptPicCodingExt->l32PicStructure)
        {
            l32NumofFrameCenterOffsets = 1;
        }
        else
        {
            if(ptPicCodingExt->l32RepeatFirstField)
            {
                l32NumofFrameCenterOffsets = 3;
            }
            else
            {
                l32NumofFrameCenterOffsets = 2;
            }
        }
    }

    //����number_of_frame_center_offsets
    for(l32Temp = 0; l32Temp < l32NumofFrameCenterOffsets; l32Temp++)
    {
        ptDisExt->l32FrameCenterHorzOffset[l32Temp] = GetBits(ptBs, 16);

        MarkerBit(ptBs, "DecPicDisplayExtension, first marker bit");

        ptDisExt->l32FrameCenterVertOffset[l32Temp] = GetBits(ptBs, 16);

        MarkerBit(ptBs, "DecPicDisplayExtension, second marker bit");
    }
}

/*====================================================================
������       :	DecPicCodingExtension
����	     :	����ͼ����չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecPicCodingExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TPicCodingExt *ptPicCodingExt = &ptMp2Decoder->tPicCodingExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    layer_data *ld = &ptBs->base;

    ptPicCodingExt->al32F_Code[0][0] = GetBits(ptBs, 4);
    ptPicCodingExt->al32F_Code[0][1] = GetBits(ptBs, 4);
    ptPicCodingExt->al32F_Code[1][0] = GetBits(ptBs, 4);
    ptPicCodingExt->al32F_Code[1][1] = GetBits(ptBs, 4);

    ptPicCodingExt->l32IntraDCPrecision = GetBits(ptBs, 2);
    ptPicCodingExt->l32PicStructure = GetBits(ptBs, 2);
    ptPicCodingExt->l32TopFieldFirst = GetBits(ptBs, 1);
    ptPicCodingExt->l32FramePredFrameDct = GetBits(ptBs, 1);
    ptPicCodingExt->l32ConcealmentMV = GetBits(ptBs, 1);
    ld->l32QScaleType = GetBits(ptBs, 1);
    ptPicCodingExt->l32IntraVlcFormat = GetBits(ptBs, 1);
    ld->l32AlternateScan = GetBits(ptBs, 1);
    ptPicCodingExt->l32RepeatFirstField  = GetBits(ptBs, 1);
    ptPicCodingExt->l32Chroma420Type = GetBits(ptBs, 1);
    ptPicCodingExt->l32ProgressiveFrame = GetBits(ptBs, 1);
    ptPicCodingExt->l32CompositeDispFlag = GetBits(ptBs, 1);

    if(ptPicCodingExt->l32CompositeDispFlag)
    {
        ptPicCodingExt->l32VAxis  = GetBits(ptBs, 1);
        ptPicCodingExt->l32FieldSequence = GetBits(ptBs, 3);
        ptPicCodingExt->l32SubCarrier = GetBits(ptBs, 1);
        ptPicCodingExt->l32BurstAmplitude = GetBits(ptBs, 7);
        ptPicCodingExt->l32SubCarrierPhase = GetBits(ptBs, 8);
    }
}

/*====================================================================
������       :	DecPicSpatialScalableExtension
����	     :	����ͼ����չ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecPicSpatialScalableExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TPicSpatScalExt *ptPicSpatScalExt = &ptMp2Decoder->tPicSpatScalExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    layer_data *ld = &ptBs->base;

    ld->l32PicScal = 1; //ʹ�ÿռ��������

    ptPicSpatScalExt->l32LowerLayerTemporalRef = GetBits(ptBs, 10);
    MarkerBit(ptBs, "DecPicSpatialScalableExtension(), first marker bit");
    ptPicSpatScalExt->l32LowerLayerHorzOffset  = GetBits(ptBs, 15);

    if(ptPicSpatScalExt->l32LowerLayerHorzOffset >= 16384)
    {
        ptPicSpatScalExt->l32LowerLayerHorzOffset -= 32768;
    }

    MarkerBit(ptBs, "DecPicSpatialScalableExtension(), second marker bit");

    ptPicSpatScalExt->l32LowerLayerVertOffset = GetBits(ptBs, 15);

    if(ptPicSpatScalExt->l32LowerLayerVertOffset >= 16384)
    {
        ptPicSpatScalExt->l32LowerLayerVertOffset -= 32768;
    }

    ptPicSpatScalExt->l32SpatTempWeightCodeTabIdx = GetBits(ptBs, 2);
    ptPicSpatScalExt->l32LowerLayerProgressiveFrame = GetBits(ptBs, 1);
    ptPicSpatScalExt->l32LowerLayerDeinterlacedFieldSlt = GetBits(ptBs, 1);
}

/*====================================================================
������       :	CopyrightExtension
����	     :	�汾��Ϣ��չ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void CopyrightExtension(TMPEG2Decoder *ptMp2Decoder)
{
    TCopyRightExt *ptCopyRightExt = &ptMp2Decoder->tCopyRightExt;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32ReservedData;

    ptCopyRightExt->l32CopyrightFlag = GetBits(ptBs, 1); 
    ptCopyRightExt->l32CopyrightIdentifier = GetBits(ptBs, 8);
    ptCopyRightExt->l32OriginalOrCopy = GetBits(ptBs, 1);

    l32ReservedData = GetBits(ptBs, 7);

    MarkerBit(ptBs, "CopyrightExtension(), first marker bit");

    ptCopyRightExt->l32CopyrightNumber1 =   GetBits(ptBs, 20);

    MarkerBit(ptBs, "CopyrightExtension(), second marker bit");

    ptCopyRightExt->l32CopyrightNumber2 =   GetBits(ptBs, 22);

    MarkerBit(ptBs, "CopyrightExtension(), third marker bit");

    ptCopyRightExt->l32CopyrightNumber3 =   GetBits(ptBs, 22);
}

/*====================================================================
������       :	UserData
����	     :	�����û�����
����ȫ�ֱ��� :	��
�������˵�� :	ptBs         ����ͼ��������Ϣ�ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void UserData(TMp2BitStream *ptBs)
{
    FindNextStartCode(ptBs);//������һ����ʼ��
}

/*====================================================================
������       :	DecExtensionAndUserData
����	     :	������չ��Ϣ���û�����
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static s32 DecExtensionAndUserData(TMPEG2Decoder *ptMp2Decoder)
{
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32code, l32Ext_ID;
	s32 l32Ret;
    
    FindNextStartCode(ptBs);
    
    while(EXTENSION_START_CODE == (l32code = ShowBits(ptBs, 32)) || USER_DATA_START_CODE == l32code)
    {
        if(EXTENSION_START_CODE == l32code)
        {
            FlushBuffer32(ptBs);
            
            l32Ext_ID = GetBits(ptBs, 4);
            
            switch(l32Ext_ID)
            {
            case SEQUENCE_EXTENSION_ID:

                DecSequenceExtension(ptMp2Decoder);
                break;

            case SEQUENCE_DISPLAY_EXTENSION_ID:

                DecSequenceDisplayExtension(ptMp2Decoder);
                break;

            case QUANT_MATRIX_EXTENSION_ID:

                DecQuantMatrixExtension(ptMp2Decoder);
                break;

            case SEQUENCE_SCALABLE_EXTENSION_ID:

                l32Ret = DecSequenceScalableExtension(ptMp2Decoder);
                if(VIDEO_SUCCESS != l32Ret)
                {
                    return l32Ret;
                }
                break;

            case PICTURE_DISPLAY_EXTENSION_ID:

                DecPicDisplayExtension(ptMp2Decoder);
                break;

            case PICTURE_CODING_EXTENSION_ID:

                DecPicCodingExtension(ptMp2Decoder);
                break;

            case PICTURE_SPATIAL_SCALABLE_EXTENSION_ID:

                DecPicSpatialScalableExtension(ptMp2Decoder);
                break;

            case PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID:

                return -1;
                break;

            case COPYRIGHT_EXTENSION_ID:

                CopyrightExtension(ptMp2Decoder);
                break;

            default:

                //printf("reserved extension start code ID %d\n",l32Ext_ID);
                break;
            }

            FindNextStartCode(ptBs);
        }
        else
        {
            FlushBuffer32(ptBs);

            UserData(ptBs);
        }
    }
    return VIDEO_SUCCESS;
}

/*====================================================================
������       :	DecPicHeader
����	     :	����ͼ��ͷ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecPicHeader(TMPEG2Decoder *ptMp2Decoder)
{
    TPicHeader *ptPicHeader = &ptMp2Decoder->tPicHeader;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32ExtraInformationByteCount;
    layer_data *ld = &ptBs->base;

    //��ʼ��Ϊ0��ֱ��������picture_spatial_scalable_extension()��д
    ld->l32PicScal = 0; 

    ptPicHeader->l32TemporalReference = GetBits(ptBs, 10);
    ptPicHeader->l32PicCodingType = GetBits(ptBs, 3);
    ptPicHeader->l32VbvDelay = GetBits(ptBs, 16);

    if((P_TYPE == ptPicHeader->l32PicCodingType) || (B_TYPE == ptPicHeader->l32PicCodingType))
    {
        ptPicHeader->l32FullPelFwVector = GetBits(ptBs, 1);
        ptPicHeader->l32ForwardFCode = GetBits(ptBs, 3);
    }

    if(B_TYPE == ptPicHeader->l32PicCodingType)
    {
        ptPicHeader->l32FullPelBwVector = GetBits(ptBs, 1);
        ptPicHeader->l32BackwardFCode = GetBits(ptBs, 3);
    }    

    l32ExtraInformationByteCount = DecExtraBitInformation(ptMp2Decoder);

    DecExtensionAndUserData(ptMp2Decoder);

    //���¸�����Ϣ�������ռ�������Ų���
    UpdateTemporalReferenceTackingData(ptMp2Decoder);
}

/*====================================================================
������       :	DecSequenceHeader
����	     :	��������ͷ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecSequenceHeader(TMPEG2Decoder *ptMp2Decoder)
{
    TSeqHeader *ptSeqHeader = &ptMp2Decoder->tSeqHeader;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    s32 l32Temp;
    layer_data *ld = &ptBs->base;
    
    ptSeqHeader->l32HorizontalSize = GetBits(ptBs, 12);
    ptSeqHeader->l32VerticalSize = GetBits(ptBs, 12);
    ptSeqHeader->l32AspectRatioInformation = GetBits(ptBs, 4);
    ptSeqHeader->l32FrameRateCode = GetBits(ptBs, 4);
    ptSeqHeader->l32BitrateVvalue = GetBits(ptBs, 18);
    MarkerBit(ptBs, "DecSequenceHeader()");
    ptSeqHeader->l32VbvBufferSize = GetBits(ptBs, 10);
    ptSeqHeader->l32ConstrainedParametersFlag = GetBits(ptBs, 1);
    
    if((ld->l32LoadIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32IntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]] = GetBits(ptBs, 8);
        }
    }
    else
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32IntraQuantizerMatrix[l32Temp] = default_intra_quantizer_matrix[l32Temp];
        }
    }
    
    if((ld->l32LoadNonIntraQuantizerMatrix = GetBits(ptBs, 1)))
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32NonIntraQuantizerMatrix[scan[ZIG_ZAG][l32Temp]] = GetBits(ptBs, 8);
        }
    }
    else
    {
        for(l32Temp = 0; l32Temp < 64; l32Temp++)
        {
            ld->al32NonIntraQuantizerMatrix[l32Temp] = 16;
        }
    }
    
    //�����������ݵ�ɫ�Ⱦ�����
    for(l32Temp = 0; l32Temp < 64; l32Temp++)
    {
        ld->al32ChromaIntraQuantizerMatrix[l32Temp] = ld->al32IntraQuantizerMatrix[l32Temp];
        ld->al32ChromaNonIntraQuantizerMatrix[l32Temp] = ld->al32NonIntraQuantizerMatrix[l32Temp];
    }
    
    DecExtensionAndUserData(ptMp2Decoder);
}

/*====================================================================
������       :	DecGOPHeader
����	     :	����ͼ����ͷ��Ϣ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static void DecGOPHeader(TMPEG2Decoder *ptMp2Decoder)
{
    TGOPHeader *ptGopHeader = &ptMp2Decoder->tGOPHeader;
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    TTempRef *ptTempRef = &ptMp2Decoder->tTempRef;

    ptTempRef->l32TemporalReferenceBase = ptTempRef->l32TrueFramenumMax + 1; 	/* *CH* */
    ptTempRef->l32TemporalReferenceGOPReset = 1;
    ptGopHeader->l32DropFlag = GetBits(ptBs, 1);
    ptGopHeader->l32Hour = GetBits(ptBs, 5);
    ptGopHeader->l32Minute = GetBits(ptBs, 6);
    MarkerBit(ptBs, "DecGOPHeader()");
    ptGopHeader->l32Sec = GetBits(ptBs, 6);
    ptGopHeader->l32Frame = GetBits(ptBs, 6);
    ptGopHeader->l32ClosedGop = GetBits(ptBs, 1);
    ptGopHeader->l32BrokenLink = GetBits(ptBs, 1);

    DecExtensionAndUserData(ptMp2Decoder);
}

/*====================================================================
������       :	GetMp2Header
����	     :	�������н���ͷ��Ϣ��ֱ���������н�β��ͼ����ʼ
����ȫ�ֱ��� :	��
�������˵�� :	ptMp2Decoder       �������ṹָ��[in]
����ֵ˵��   :	��
====================================================================*/
static s32 GetMp2Header(TMPEG2Decoder *ptMp2Decoder, TVideoInfo *ptVideoInfo)
{
    TMp2BitStream *ptBs = &ptMp2Decoder->tBs;
    TSeqHeader *ptSeqHeader = &ptMp2Decoder->tSeqHeader;
    unsigned int u32Code;
    s32 l32Times = 0;
    
    for(;;)
    {
        // ������һ����ʼ��
        FindNextStartCode(ptBs);
        
        u32Code = ShowBits(ptBs, 32);

        switch(u32Code)
        {
        case SEQUENCE_HEADER_CODE:
            
            FlushBuffer32(ptBs);
            DecSequenceHeader(ptMp2Decoder);
            ptVideoInfo->l32Width = 16 * ((ptSeqHeader->l32HorizontalSize + 15) / 16);
            ptVideoInfo->l32Height = 16 * ((!ptMp2Decoder->tSeqExt.l32ProgressiveSequence) ? 2 * ((ptSeqHeader->l32VerticalSize + 31) / 32) : (ptSeqHeader->l32VerticalSize + 15) / 16);
            break;
            
        case GROUP_START_CODE:
            FlushBuffer32(ptBs);
            DecGOPHeader(ptMp2Decoder);
            break;
            
        case PICTURE_START_CODE:
            
            ptBs->l32PicHeadPos = ptBs->l32BitCount >> 3;
            
            FlushBuffer32(ptBs);
            DecPicHeader(ptMp2Decoder);
            ptVideoInfo->l32IsIFrame = (ptMp2Decoder->tPicHeader.l32PicCodingType == 1) ? 1 : 0;
			return VIDEO_SUCCESS;
            break;
            
        case SEQUENCE_END_CODE:
            return VIDEO_SUCCESS;
            break;
            
        default:
            l32Times ++;

            if(l32Times > 8)
            {
                return -1;
            }
            break;
        }
    }
	return VIDEO_SUCCESS;
}

/*=============================================================================
�� �� ��: GetMP2Info
��    ��: ��ȡMpeg2����VOL��Ϣ
ȫ�ֱ���: ��
��    ��: pu8Stream             ��������ָ��[in]
          unsigned intBsLen              ������������[in]
          ptMP4Info             ��ȡMP4��Ϣ�ṹ��ָ��[out]
�� �� ֵ: ��Ӧ�Ĵ�����
=============================================================================*/
static s32 GetMP2Info(u8 *pu8Stream, unsigned int u32BsLen, TVideoInfo *ptMP2Info)
{
    TMPEG2Decoder tTMp2Decoder;
    TMp2BitStream *ptBs = &tTMp2Decoder.tBs;
    TSeqHeader *ptSeqHeader = &tTMp2Decoder.tSeqHeader;
    s32 l32Ret = VIDEO_SUCCESS;

    if(NULL == pu8Stream)
    {
        return -1;
    }

    if(u32BsLen == 0)
    {
        return -1;
    }
    
    //��ʼ��������Ϣ
    ptBs->pu8BsBufferInput = pu8Stream;
    ptBs->l32BitstreamLen = u32BsLen;
    ptBs->l32FillBufferCounter = 0;
    ptBs->l32EndBit = 0;
    BsMp2Init(ptBs);
    ptBs->l32BitCount = 0;
    ptMP2Info->l32IsIFrame = 0;
    
    //����VOL��ͷ��Ϣ
    l32Ret = GetMp2Header(&tTMp2Decoder, ptMP2Info);

    return l32Ret;
}

//=========================================getH265Info=============================================//
/*====================================================================
������      ��  DecodePTL
����        ��  ����PTL��Ϣ
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs��ָ���������ָ��[in]
               ptPtl��ָ��ptl��Ϣָ��[in]
����ֵ˵��  ��  �ɹ����أ�VIDEO_SUCCESS�����󷵻���Ӧ������
====================================================================*/
static s32 DecodePTL(TBitstream *ptBs, u8 u8MaxSubLayersMinus1)
{
	s32 l32Index;
	u8 au8SubLayerProfilePresentFlag[6];
	u8 au8SubLayerLevelPresentFlag[6];

	BitstreamSkip(ptBs, 8); //��ȡgeneral_profile_space��general_tier_flag��general_profile_idc

	BitstreamSkip(ptBs, 32); //��ȡgeneral_profile_compatibility_flag[ i ]

	//��ȡʣ�µ��﷨Ԫ��general_progressive_source_flag��general_level_idc
	BitstreamSkip(ptBs, 32);
	BitstreamSkip(ptBs, 24);

	for(l32Index = 0; l32Index < u8MaxSubLayersMinus1; l32Index++)
	{
		au8SubLayerProfilePresentFlag[l32Index] = BitstreamGetBits(ptBs, 1); //��ȡsub_layer_profile_present_flag[ i ]
		au8SubLayerLevelPresentFlag[l32Index] = BitstreamGetBits(ptBs, 1);  //��ȡsub_layer_level_present_flag[ i ]
	}
	if(u8MaxSubLayersMinus1 > 0)
	{
		for(l32Index = u8MaxSubLayersMinus1; l32Index < 8; l32Index++)
		{
			BitstreamSkip(ptBs, 2);  //��ȡreserved_zero_2bits[ i ]
		}
	}
	for(l32Index = 0; l32Index < u8MaxSubLayersMinus1; l32Index++) 
	{
		if(au8SubLayerProfilePresentFlag[l32Index]) 
		{
			BitstreamSkip(ptBs, 8);  //��ȡsub_layer_profile_space[ i ]��sub_layer_tier_flag[ i ]��sub_layer_profile_idc[ i ]
			BitstreamSkip(ptBs, 32);  //��ȡsub_layer_profile_compatibility_flag[ i ][ j ]
			BitstreamSkip(ptBs, 48);  //��ȡsub_layer_progressive_source_flag[ i ]��sub_layer_reserved_zero_44bits[ i ]
		}
		if(au8SubLayerLevelPresentFlag[l32Index])
		{
			BitstreamSkip(ptBs, 8);  //��ȡsub_layer_level_idc[ i ]
		}
	}

	return VIDEO_SUCCESS;
}

/*====================================================================
������      ��  DecodeSPS
����        ��  ����SPS��Ϣ
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs��ָ���������ָ��[in]
               ptSps��ָ��SPS��Ϣָ��[in]
����ֵ˵��  ��  �ɹ����أ�VIDEO_SUCCESS�����󷵻���Ӧ������
====================================================================*/
static s32 DecodeSPS(TBitstream *ptBs, s32 *pl32Width, s32 *pl32Height)
{
	s32 l32RetCode = VIDEO_SUCCESS;
	u8 u8SpsId;
	s32 l32Val;
	s32 l32Ret;
	u32 u32AddCUDepth, u32MaxDepth;
	u8 u8MaxSubLayersMinus1;
	s32 l32Index;
    s32 s32CropLeft, s32CropRight, s32CropTop, s32CropBottom;
    s32 l32ConfWinLeftOffset, l32ConfWinRightOffset;
    s32 l32ConfWinTopOffset, l32ConfWinBottomOffset;
    s32 l32Width, l32Height;

#define CHECKSPS(x, err) if((x)){ l32RetCode = err; goto SPS_Done; }

	l32Val = BitstreamGetBits(ptBs, 4); //��ȡsps_video_parameter_set_id

	u8MaxSubLayersMinus1 = BitstreamGetBits(ptBs, 3); //��ȡsps_max_sub_layers_minus1,todo
	CHECKSPS((u8MaxSubLayersMinus1 > 6), -1);

	l32Val = BitstreamGetBits(ptBs, 1); //��ȡsps_temporal_id_nesting_flag
	CHECKSPS((u8MaxSubLayersMinus1 == 0 && l32Val != 1), -1);

	//����PTL�﷨�ṹ
	l32Ret = DecodePTL(ptBs, u8MaxSubLayersMinus1);
	CHECKSPS((l32Ret != VIDEO_SUCCESS), l32Ret);

	//��ȡsps_seq_parameter_set_id
	l32Val = BitstreamReadUe(ptBs);
	CHECKSPS((l32Val > 15), -1);

	//��ȡchroma_format_idc
	l32Val = BitstreamReadUe(ptBs);
	CHECKSPS((l32Val != 1), -1);

	//��ȡpic_width_in_luma_samples��pic_height_in_luma_samples
	l32Width = BitstreamReadUe(ptBs);
	l32Height = BitstreamReadUe(ptBs);

	//��ȡconformance_window_flag
	l32Val = BitstreamGetBits(ptBs, 1);
	if(l32Val)
	{
		l32ConfWinLeftOffset = BitstreamReadUe(ptBs);
		l32ConfWinRightOffset = BitstreamReadUe(ptBs);
		l32ConfWinTopOffset = BitstreamReadUe(ptBs);
		l32ConfWinBottomOffset = BitstreamReadUe(ptBs);

        s32CropLeft = l32ConfWinLeftOffset << 1;
        s32CropRight = l32ConfWinRightOffset << 1;
        s32CropTop = l32ConfWinTopOffset << 1;
        s32CropBottom = l32ConfWinBottomOffset << 1;

        (*pl32Width) = l32Width - s32CropLeft - s32CropRight;
        (*pl32Height) = l32Height - s32CropTop - s32CropBottom;
	}
    else
    {
        (*pl32Width) = l32Width;
        (*pl32Height) = l32Height;
    }

SPS_Done:

#undef CHECKSPS

	return l32RetCode;
}

/*====================================================================
������      ��  H265DecBsHeader
����        ��  �ؽ���һ֡
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  pu8BsBuffer������bufferָ��[in]
                l32BsLen������buffer����[in]
                ptH264Info����ߵ���Ϣ
����ֵ˵��  ��  �ɹ�����VIDEO_SUCCESS��ʧ�ܷ��ش�����
====================================================================*/
static s32 H265DecBsHeader(u8* pu8BsBuffer, s32 l32BsLen, TVideoInfo *ptH265Info)
{
    s32 l32RetCode = 0;
    s32 l32NalHeader, l32NalType, l32BsSrcLen;
    u8 *pu8Src, *pu8BsSrc;
    u8 u8SPSFlag = FALSE, u8PPSFlag = FALSE;
	s16 s16FirstMBNum = 0;
	s32 l32Val = 0;
    s32 l32Width;    //ͼ����
    s32 l32Height;   //ͼ��߶�

#define CHECKSLICEERR(ret) if(ret != 0) { goto SliceErr; }
    TBitstreamBuf tBsBuf;
    TBitstream tBs;

	pu8Src = pu8BsBuffer;
    //�жϴ��ģʽ
	while(((*pu8Src) == 0) && (pu8Src < (pu8BsBuffer + l32BsLen)))
    {
        pu8Src++;
    }

    if(((*pu8Src) == 0x01) && ((pu8Src - pu8BsBuffer) > 1))
    {
		pu8BsSrc = pu8Src - 2;
        l32BsSrcLen = l32BsLen - (pu8BsSrc - pu8BsBuffer);
        //Annex B ���ģʽ
        BitstreamBufInit(&tBsBuf, pu8BsSrc, l32BsSrcLen);
    }
    else
    {
        return -1;
    }
    
    while(tBsBuf.pu8SliceStart < pu8BsSrc + l32BsSrcLen)
    {
        BitstreamDecodeNal(&tBs, &tBsBuf);
		// Nal type��齫�ŵ��ϲ���
        l32NalHeader = BitstreamGetBits(&tBs, 16);
        l32NalType = ((l32NalHeader >> 9) & 63); //��ȡNALU����
		
        switch(l32NalType)
        {
        case NAL_UNIT_VPS:
            break;
        case NAL_UNIT_SPS:
            l32RetCode = DecodeSPS(&tBs, &ptH265Info->l32Width, &ptH265Info->l32Height);
            CHECKSLICEERR(l32RetCode);
            u8SPSFlag = TRUE;
            break;

        case NAL_UNIT_PPS:
            u8PPSFlag = TRUE;
            break;

        case NAL_UNIT_CODED_SLICE_IDR:
        case NAL_UNIT_CODED_SLICE_IDR_N_LP:
        case NAL_UNIT_CODED_SLICE_TRAIL_R:
        case NAL_UNIT_CODED_SLICE_TRAIL_N:
        case NAL_UNIT_CODED_SLICE_CRA:
            if(u8SPSFlag && u8PPSFlag)
            {
                //��ȡfirst_slice_segment_in_pic_flag
                l32Val = BitstreamGetBits(&tBs, 1);

                if(l32NalType >= 16 && l32NalType <= 23)
                {
                    //��ȡno_output_of_prior_pics_flag,�ݲ�֧�ַ��������todo
                    l32Val = BitstreamGetBits(&tBs, 1);
                }

                //��ȡslice_pic_parameter_set_id
                l32Val = BitstreamReadUe(&tBs);

                //num_extra_slice_header_bits����0��ֱ�Ӷ�ȡslice_type
                l32Val = BitstreamReadUe(&tBs);

                if(l32Val == 2)
                {
                    ptH265Info->l32IsIFrame = 1;
                }	
            }
            
            break;

        case NAL_UNIT_RESERVED_10:
        case NAL_UNIT_RESERVED_11:
        case NAL_UNIT_RESERVED_12:
        case NAL_UNIT_RESERVED_13:
        case NAL_UNIT_RESERVED_14:
        case NAL_UNIT_RESERVED_15:
        case NAL_UNIT_RESERVED_22:
        case NAL_UNIT_RESERVED_23:
        case NAL_UNIT_RESERVED_24:
        case NAL_UNIT_RESERVED_25:
        case NAL_UNIT_RESERVED_26:
        case NAL_UNIT_RESERVED_27:
        case NAL_UNIT_RESERVED_28:
        case NAL_UNIT_RESERVED_29:
        case NAL_UNIT_RESERVED_30:
        case NAL_UNIT_RESERVED_31:
        case NAL_UNIT_ACCESS_UNIT_DELIMITER:
        case NAL_UNIT_SEI:
        case NAL_UNIT_SEI_SUFFIX:
        case NAL_UNIT_RESERVED_41:
        case NAL_UNIT_RESERVED_42:
        case NAL_UNIT_RESERVED_43:
        case NAL_UNIT_RESERVED_44:
        case NAL_UNIT_RESERVED_45:
        case NAL_UNIT_RESERVED_46:
        case NAL_UNIT_RESERVED_47:
        case NAL_UNIT_UNSPECIFIED_48:
        case NAL_UNIT_UNSPECIFIED_49:
        case NAL_UNIT_UNSPECIFIED_50:
        case NAL_UNIT_UNSPECIFIED_51:
        case NAL_UNIT_UNSPECIFIED_52:
        case NAL_UNIT_UNSPECIFIED_53:
        case NAL_UNIT_UNSPECIFIED_54:
        case NAL_UNIT_UNSPECIFIED_55:
        case NAL_UNIT_UNSPECIFIED_56:
        case NAL_UNIT_UNSPECIFIED_57:
        case NAL_UNIT_UNSPECIFIED_58:
        case NAL_UNIT_UNSPECIFIED_59:
        case NAL_UNIT_UNSPECIFIED_60:
        case NAL_UNIT_UNSPECIFIED_61:
        case NAL_UNIT_UNSPECIFIED_62:
        case NAL_UNIT_UNSPECIFIED_63:
            break;

        default:
             break;
        }
        continue;
    }

SliceErr:
    return l32RetCode;
}

static s32 GetH265Info(u8 *pu8Stream, unsigned int u32BsLen, TVideoInfo *ptH265Info)
{
    s32 l32Ret = 0;
    u16 wCopyByte = ((u32BsLen > 120) ? 123 : u32BsLen); //���Ч�ʣ�����֡����ǰ120�ֽڼ��ɽ�����ߣ�����3���ֽ����ڽ���

    if(NULL == pu8Stream || NULL == ptH265Info)
    {
        return -1;
    }

    if(u32BsLen == 0)
    {
        return -1;
    }

    u8 *pu8H265Stream = (u8 *)malloc(wCopyByte);
    if (NULL == pu8H265Stream)
    {
        return -1;
    }
    memcpy(pu8H265Stream, pu8Stream, wCopyByte);

    ptH265Info->l32IsIFrame = 0;
	
	wCopyByte = ((u32BsLen > 120) ? 120 : u32BsLen);//���ڽ�������Ч����
    //����VOL��ͷ��Ϣ
    l32Ret = H265DecBsHeader(pu8H265Stream, wCopyByte, ptH265Info);

    if (pu8H265Stream)
    {
        free(pu8H265Stream);		
        pu8H265Stream = NULL;
    }
    return l32Ret;
}
