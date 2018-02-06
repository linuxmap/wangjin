#ifndef _COMMON_0603_H_
#define _COMMON_0603_H_

#include "kdvtype.h"

//���¶��� FD_SETSIZE
#undef      FD_SETSIZE
#ifdef WIN32
#define   FD_SETSIZE   (s32)1024//����socket��
#else
#define   FD_SETSIZE   (s32)1024//����socket��
#endif


#ifdef _LINUX_
#ifndef _ANDROID_
//#include "stropts.h"
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#endif
//    #include "osp_Linux.h"
//#else
#include "osp.h"
#include "kdmtsps.h"
#include "kdvdef.h"

#ifdef _VXWORKS_
   #include "brddrvlib.h"
#endif


#include "kdvmedianet.h"

#ifdef WIN32
   #include <memory.h>
   #include <mmsystem.h>
   #pragma comment(lib, "winmm")
#endif

#include <string.h>
#include <time.h>

#ifndef WIN32
#define ADDR_ANY    0
#endif


#ifndef NULL
#define NULL 0
#endif

#ifdef _LINUX_
#define MedianetVsnprintf       vsnprintf
#else
#define MedianetVsnprintf        _vsnprintf
#endif
//
// Define the IP header. Make the version and length field one
// character since we can't declare two 4 bit fields without
// the compiler aligning them on at least a 1 byte boundary.
//
typedef struct tagIPHdr
{
    u8  byIPVerLen;        // IP version & IHL
    u8  byIPTos;           // IP type of service
    u16 wIPTotalLen;       // Total length
    u16 wIPID;             // Unique identifier
    u16 wIPOffset;         // Fragment offset field
    u8  byIPTtl;           // Time to live
    u8  byIPProtocol;      // Protocol(TCP,UDP etc)
    u16 wIPCheckSum;       // IP checksum
    u32 dwSrcAddr;         // Source address
    u32 dwDstAddr;         // Destination address
} TIPHdr, *PTIPHdr;
//
// Define the UDP header
//
typedef struct tagUDPHdr
{
    u16 wSrcPort;          // Source port number
    u16 wDstPort;          // Destination port number
    u16 wUDPLen;           // UDP packet length
    u16 wUDPCheckSum;      // UDP checksum (optional)
} TUDPHdr, *PTUDPHdr;

typedef struct tagUDPPsdHdr //����TCPα�ײ�
{
    u32 dwSrcAddr;         // Source address
    u32 dwDstAddr;         // Destination address
    u8  byMbz;
    u8  byIPProtocol;      // Protocol(UDP==17)
    u16 wUDPLen;           // UDP  packet length
} TUDPPsdHdr;

#define MEDIANET_MALLOC(p, l) { if (g_bUseMemPool) { (p) = (u8 *)OspAllocMem(l); }else{ (p) = (u8 *)malloc(l); }}
#define MEDIANET_SAFE_FREE(p) if (NULL != (p)){if (g_bUseMemPool){OspFreeMem(p);}else{free(p);}(p) = NULL;}

#define SYS_MALLOC(p, l) { (p) = (u8 *)malloc(l); }
#define SYS_SAFE_FREE(p) if (NULL != (p)){free(p); (p) = NULL;}

#define SAFE_DELETE(p) {if(p){delete p;p=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p){delete [] p;p=NULL;}}
#define KDVFAILED(p) ((p)!=MEDIANET_NO_ERROR)

#define MEDIANET_SEM_TAKE(a)    if (NULL != a){OspSemTake(a);}
#define MEDIANET_SEM_GIVE(a)    if (NULL != a){OspSemGive(a);}

#define MEDIA_SETSIZE         (4096)

#define MAX_PACK_NUM           0x60 // �������256kbyte������֡���ԣ�С���� <  96
#define MAX_SND_PACK_SIZE      (s32)1452 // ����ʱ���İ�������XP Adslȷ��
#define MAX_SND_ENCRYPT_PACK_SIZE  (s32)1452 // ����ʱ���İ���

#define MAX_RCV_PACK_SIZE      (s32)8192// ����ʱ���İ��� ��AES/DES���ܻ��߷Ǽ���������
#define LOOP_BUF_UINT_NUM      (s32)400// ѭ���������
#define MAX_SESSION_NUM        (s32)128//
#define MAX_H261_PACK_NUM      (s32)32//

#define MAX_H263_PACK_NUM      (s32)150// A&B MODE
//#define MAX_H263_PACK_NUM      (s32)256// ���� A MODE
#define MODE_H263_A            (s32)0//
#define MODE_H263_B            (s32)1//
#define MODE_H263_C            (s32)2//
#define MODE_H263_D            (s32)3//

// 4k resolution
#define H264_4K_RESOLUTION_WIDTH  (s32) 4096
#define H264_4K_RESOLUTION_HIGHT  (s32) 2160
#define H264_4K_MAX_FRAME_SIZE (u32) MAX_VIDEO_FRAME_SIZE* 4
#define MAX_FRAME_SIZE (u32)(MAX_VIDEO_FRAME_SIZE * 4)


#ifndef MAX_NALU_NUM
#define MAX_NALU_NUM           (s32)128
#define    MAX_NALU_UNIT_SIZE       (s32)(4*MAX_NALU_NUM)   // (sizeof(int32_t) * MAX_NALU_NUM)
#endif



//��չ����������С������mpeg2/mpeg4/mp3��
/*************************************************************************************
    ����MTU, һ��涨����IP֡����Ϊ1500����̫֡����Ϊ1518������ͷ14�ֽ�
mac��ַ�����ͣ����4�ֽ�checksum����
    ��xp�£�adsl��Ҫ��IPͷǰ�м�20�ֽ�PPOEͷ��������ƽ̨�ϣ�һ��PPOEռ8���ֽڡ�
��ˣ�udp������غɳ���Ϊ��1500��20(adsl)-20(ip)-8(udp) = 1452��
    �ϰ汾medianet�ж�MP4�������
MAX_SND_PACK_SIZE(1450) - MAX_PACK_EX_LEN(12)-EX_HEADER_SIZE(4) = 1434 ���зָ
ʵ�����ټ�����rtp��չ����(4)�����������������MP4������Ϊ
1434 + 4(MP4_HEAD) + 4 (EX_LEN) + 12(RTP) = 1454 ������ xp��PPOE��
���·�Ƭ������ʹ��nip�汾dataswitchת��ʱ���ղ�����
    ���⣬�����������һ��������������������MP4���һ��������󳤶ȣ��������
һ����12�ֽ�mp4��չ������֡ID����͸ߣ����������1460��
    ������Ǽ��ܣ�����RTP�غ�Ҫ��16�ֽڶ�����ܣ������������£��غ�1434��>1440,
��������³���Ϊ1458, ���������Ϊ1466��
    ��Ӧ�����Ҫ�����ͳһ�޸ĳ�����Ĳ���������Ƭ�����޸ĳ�1424������ʵ�ʰ���Ϊ1444��
���ܺ�Ϊ1444�����������Ϊ1452��
    �޸ĺ󣬲��ܺ��ϴ����ͨ�������Ƶһֱû���޸ġ������ǵ����������޸ģ���Ƶ�ͼ��
�������˽�������Ӧ����
    Ӧȫ����Ҫ����Ҫ��RTPǰ����22�ֽ�ȫ����ͷ, ��˼��ͳһ�Ѱ���ֱ�ӱ��1392��
**************************************************************************************/
//#define MAX_EXTEND_PACK_SIZE   (s32)(MAX_SND_PACK_SIZE - MAX_PACK_EX_LEN - EX_HEADER_SIZE)
//[1452(���udp�غɳ�) - 12(rtpͷ) - 4(rtp��չ���ȵĳ���) - 12(��չ�����3*4)] ��16�ֽ�����ȡ��
#define MAX_EXTEND_PACK_SIZE   (s32)1392
#define MAX_EXTEND_PACK_NUM    (s32)((MAX_VIDEO_FRAME_SIZE+MAX_EXTEND_PACK_SIZE-1)/MAX_EXTEND_PACK_SIZE)//
#define MAX_SPSPPS_BUF_LEN     (s32)128

#define VIDEO_TIME_SPAN        (u16)40  //֡��25p/s ����IPC֡�ʲ�Ϊ25p/s���޸�
#define VIDEO_TIMESTAMP_SPAN   (u16)90  //��Ƶ������Ϊ90k

#define DEFAULT_RSND_TIME_SPAN (u16)1000// 1�� ������ƺ�û���õ�����
#define MIN_RS_UNIT_NUM        (u16)100 //  ��С�ش��������
#define MAX_RS_UNIT_NUM        (u16)6000//2048  ����ش��������

#define RTP_FIXEDHEADER_SIZE   (s32)12// rtp�̶�ͷ12�ֽڣ���׼ȷ����Ҫ����CCֵ�ж��Ƿ���CSRC�����ǹ�˾û��ʹ��CSRC��
#define RTP_PADDING_SIZE       (s32)1//  rtp���1����
#define EX_HEADER_SIZE           (s32)4//  rtp��չͷ��С
#define MAX_PACK_EX_LEN           (s32)12// 1 TOTALNUM 1Index,1mode,1rate,4FrameId,2width,2Height;
#define MIN_PACK_EX_LEN           (s32)4 // 1 TOTALNUM 1Index,1mode
#define EX_TOTALNUM_POS        (s32)0 // С�����ڵ�֡�������ܰ���
#define EX_INDEX_POS           (s32)1 // С��������֡�еİ����
#define EX_FRAMEMODE_POS       (s32)2 // ��֡Ϊ��Ƶ֡ʱ����Ƶģʽ
#define EX_FRAMERATE_POS       (s32)3 // ֡��
#define EX_FRAMEID_POS         (s32)4 // ֡ID
#define EX_WIDTH_POS           (s32)8 // ��֡Ϊ��Ƶ֡ʱ�Ŀ�
#define EX_HEIGHT_POS          (s32)10// ��֡Ϊ��Ƶ֡ʱ�ĸ�

#define DES_ENCRYPT_BYTENUM    (s32)8 // DES ����Ϊ8�ֽڶ���
#define AES_ENCRYPT_BYTENUM    (s32)16// DES ����Ϊ16�ֽڶ���


#define MAX_AUDIO_BITRATE      (s32)80// Kbit/s (64 + 20%��ipͷ����)

#define MAX_SND_PACK_SIZE_BY_RAW_IP (s32)(MAX_SND_ENCRYPT_PACK_SIZE+RTP_FIXEDHEADER_SIZE+sizeof(TIPHdr)+sizeof(TUDPHdr))


//���Եȼ���Ϊ��֮ǰmedianet��ӡ��ͬ
enum EMedianetDebug
{
    AllClose   = 0,
    LostPack   = 1,
    Resend     = 2,
    Error      = 3,
    Memory     = 4,
    KeepAlive  = 5,
    Api        = 6,
    Pack       = 7,
    VideoFrame = 8,
    Info       = 9,
    Debug      = 10,
    Time       = 11,
    Rtcp       = 12,
    AudioFrame = 13,
    Parse      = 14,
    Other      = 15,
	VideoAndAudioFrame = 138,
    AllOpen    = 255,
};


typedef struct tagAddr
{
    tagAddr(){m_dwUserDataLen=0;};
    u32  m_dwIP;
    u16  m_wPort;
    u32  m_dwUserDataLen;   //Rtp�û����ݳ���,�����ڴ��������,��Ҫ��ÿ��udpͷǰ��ӹ̶�ͷ���ݵ����
    u8   m_abyUserData[MAX_USERDATA_LEN]; //�û�����ָ��
}TAddr;

//�ƺ�ֻ����һ�ԡ�����
typedef struct tagRemoteAddr
{
    u8      m_byNum;                    //��ַ����
    TAddr m_tAddr[MAX_NETSND_DEST_NUM];//Զ�˵�ַ����
}TRemoteAddr;


//A MODE
typedef struct tagKdvH261Header
{
    u32  m_dwHeader;
    u32  m_dwStartPos;
    u32  m_dwPackLen;
}TKdvH261Header;

/*
//A MODE
typedef struct tagKdvH263Header
{
    u32  m_dwHeader;
    u32  m_dwStartPos;
    u32  m_dwDataLen;
}TKdvH263Header;
*/

//A&B MODE
typedef struct tagKdvH263Header
{
    u32  m_dwMode;    //0 - A MODE ; 1 - B MODE ; 2 - C MODE ; 3 - H263+ MODE
    u32  m_dwHeader1;
    u32  m_dwHeader2;
    u32  m_dwStartPos;
    u32  m_dwDataLen;
}TKdvH263Header;

typedef struct tagKdvH263PlusHeader
{
    u16  m_wHeader;
    u32  m_dwStartPos;
    u32  m_dwDataLen;
}TKdvH263PlusHeader;

typedef struct tagKdvH264Header
{
    u16         m_wWidth;            // ����֡�Ŀ��
    u16         m_wHeight;            // ����֡�ĸ߶�
    BOOL32     m_bIsKeyFrame;        // ����֡�Ƿ�Ϊ�ؼ�֡�� 1���ǹؼ�֡  0�����ǹؼ�֡
    u32         m_dwSPSId;            // SPS��ID
    BOOL32     m_bIsValidSPS;        // H264��SPS�Ƿ���Ч
    BOOL32     m_bIsValidPPS;        // H264��PPS�Ƿ���Ч
} TKdvH264Header;

typedef struct tagKdvH265Header
{
    u16         m_wWidth;            // ����֡�Ŀ��
    u16         m_wHeight;            // ����֡�ĸ߶�
    u32      m_dwMaxPartionWidth;//LCU���
    BOOL32     m_bIsKeyFrame;        // ����֡�Ƿ�Ϊ�ؼ�֡�� 1���ǹؼ�֡  0�����ǹؼ�֡
    u32         m_dwSPSId;            // SPS��ID
    BOOL32     m_bIsValidSPS;        // H265��SPS�Ƿ���Ч
    BOOL32     m_bIsValidPPS;        // H265��PPS�Ƿ���Ч

} TKdvH265Header;

typedef struct
{
    u16        first_mb_in_slice;                        //ue(v)  slice�е�һ��MB�ĵ�ַ
    u16        slice_type;                                //ue(v)  slice�ı�������
    u16        pic_parameters_set_id;
} Tstdh265Dec_SliceHeaderData;

//sps�ṹ
typedef struct tagTSPS
{
    u8 u8VpsId;  //sps_video_parameter_set_id
    u8 u8SpsId;  //sps_seq_parameter_set_id
    u8 u8Valid;
    u8 u8SpsTemporalIdNestingFlag; //sps_temporal_id_nesting_flag
    u8 u8ChromaFormatIdc;  //chroma_format_idc
    u16 u16Width;          //pic_width_in_luma_samples
    u16 u16Height;         //pic_height_in_luma_samples
    s32 l32ConformanceWindow_flag;  //conformance_window_flag
    s32 l32ConfWinLeftOffset;   //conf_win_left_offset
    s32 l32ConfWinRightOffset;   //conf_win_right_offset
    s32 l32ConfWinTopOffset;   //conf_win_top_offset
    s32 l32ConfWinBottomOffset;   //conf_win_bottom_offset
    u8 u8BitDepthLumaMinus8;   //bit_depth_luma_minus8
    u8 u8BitDepthChromaMinus8;  //bit_depth_Chroma_minus8
    u8 au8SpsMaxDecPicBufferingMinus1[7]; //sps_max_dec_pic_buffering_minus1
    u8 au8SpsMaxNumReorderPics[7]; //sps_max_num_reorder_pics
    u8 au8SpsMaxLatencyIncreasePlus1[7]; //sps_max_latency_increase_plus1
    u8 u8Log2MaxPicOrderCntLsbMinus4;  //log2_max_pic_order_cnt_lsb_minus4
    u8 u8Log2MinCodingBlockSizeMinus3;  //log2_min_coding_block_size_minus3
    u8 u8Log2DiffMaxMinLumaCodingBlockSize;  //log2_diff_max_min_coding_block_size
    u8 u8Log2MinTransformBlockSizeMinus2; //log2_min_transform_block_size_minus2
    u32 u32MaxPartionWidth;
} TSPS;

//pps�ṹ
typedef struct
{
    u8 u8SpsId;   //pps_seq_parameter_set_id
    u8 u8PpsId;   //pic_parameter_set_id
    u8 u8Valid;
    u8 u8CabacInitPresentFlag; //cabac_init_present_flag
    u8 u8dDependentSliceSegmentsEnabledFlag; //dependent_slice_segments_enabled_flag
    u8 u8OutputFlagPresentFlag;  //output_flag_present_flag
    u8 u8SignDataHidingFlag;      //sign_data_hiding_flag
    u8 u8NumRefIdxL0DefaultActiveMinus1; //num_ref_idx_l0_default_active_minus1
    u8 u8NumRefIdxL1DefaultActiveMinus1; //num_ref_idx_l1_default_active_minus1
    s8 s8PicInitQpMinus26; //pic_init_qp_minus26
    s32 l32ConstrainedIntrPredFlag;  // constrained_intra_pred_flag
    s32 l32Transform_SkipEnabled_Flag;  //transform_skip_enabled_flag
    s32 l32CUQpDeltaEnabledFlag;    //cu_qp_delta_enabled_flag
    u8 u8DiffCUQpDeltaDepth; //diff_cu_qp_delta_depth
    u8 u8PpsSliceChromaQpOffsetsPresentFlag;//pps_slice_chroma_qp_offsets_present_flag
    u8 u8TransquantBypassEnabledFlag;      //transquant_bypass_enable_flag
    s32 l32TilesEnabledFlag;  //tiles_enabled_flag
    u8 u8EntropyCodingSyncEnabledFlag; //entropy_coding_sync_enabled_flag
    s32 l32NumTileColumnsMinus1; //num_tile_columns_minus1
    s32 l32NumTileRowsMinus1; //num_tile_rows_minus1
    u8 u8UniformSpacingFlag; //uniform_spacing_flag
    s32 l32LoopFilterAcrossTilesEnabledFlag; //loop_filter_across_tiles_enabled_flag
    s32 l32LoopFilterAcrossSlicesEnabledFlag; //loop_filter_across_slices_enabled_flag
    u8 u8DeblockingFilterControlPresentFlag; //deblocking_filter_control_present_flag
    u8 u8DeblockingFilterOverrideEnabledFlag;//deblocking_filter_override_enabled_flag
    s32 l32DeblockingFilterDisableFlag; //pps_disable_deblocking_filter_flag
    s8 s8PPSBetaOffsetDiv2; //pps_beta_offset_div2
    s8 s8PPSTcOffsetDiv2; //pps_tc_offset_div2
    u8 u8ListsModificationPresentFlag; //lists_modification_present_flag
    u8 u8Log2ParallelMergeLevelMinus2; //log2_parallel_merge_level_minus2
} TPPS;
typedef struct tagKdvSpsPspInfo
{
    BOOL32 m_bHaveSps; //�������Ƿ���sps��pps��Ϣ
    BOOL32 m_bHavePps;
    u8 *pbySpsBuf; //sps����
    s32 nSpsBufLen; //sps����
    u8 *pbyPpsBuf;
    s32 nPpsBufLen;
}TKdvSpsPpsInfo;

typedef struct tagH263HeaderList
{
    s32              m_nNum;
    TKdvH263Header   m_tKdvH263Header[MAX_H263_PACK_NUM];
}TH263HeaderList;

typedef struct tagH261HeaderList
{
    s32             m_nNum;
    TKdvH261Header  m_tkdvH261Header[MAX_H261_PACK_NUM];
}TH261HeaderList;

typedef struct tagH264HeaderList
{
    u32         m_dwNaluNum;                    // ÿ֡��������ݱ���ֳ�NALU�ĸ�����RTP��������Ϊÿ��һ��NALU��
    u32         m_dwH264NaluArr[MAX_NALU_NUM];    // ÿ֡��������ݱ���ֳ�NALU�ĳ���
    u16         m_wSequence[MAX_NALU_NUM];    // ÿ֡��������ݱ���ֳ�RTP�������
} TH264HeaderList;

/*
//A&B&C MODE
typedef struct tagKdvExH263Header
{
    u32  m_dwMode;    //0 - A MODE ; 1 - B MODE ; 2 - C MODE
    u32  m_dwHeader1;
    u32  m_dwHeader2;
    u32  m_dwHeader3;
    u32  m_dwStartPos;
    u32  m_dwDataLen;
}TKdvExH263Header;

typedef struct tagExH263HeaderList
{
    s32              m_nNum;
    TKdvExH263Header m_tKdvExH263Header[MAX_H263_PACK_NUM];
}TExH263HeaderList;
*/

//H261Header �ṹ��Ϊ�˱�����⣬�ֶ����ͱ�׼һ��
typedef struct tagH261Header
{
    s32 sBit;
    s32 eBit;
    s32 i;
    s32 v;
    s32 gobN;
    s32 mbaP;
    s32 quant;
    s32 hMvd;
    s32 vMvd;
}TH261Header;

typedef struct tagH263Header
{
    s32 f;
    s32 p;
    s32 sBit;
    s32 eBit;
    s32 src;
    s32 i;
    s32 u;
    s32 s;
    s32 a;
    s32 r;
    s32 dbq;
    s32 trb;
    s32 tr;
}TH263Header;

typedef struct tagH263PlusHeader
{
    s32 revBit;  //����λ    ---5bit
    s32 pBit;    //�Ƿ���� PSC/GSC�����ֽڵĴ���ʱ�ĺ���    ---1bit
    s32 vrcBit;  //�Ƿ���VRC ��Ϣ    ---15bit
    s32 phLen;   //picture header length    ---6bit
    s32 pheBit;  //picture header ebit    ---3bit
}TH263PlusHeader;



//h264 ������Ϣ����
typedef struct
{
    u8 *pu8Start;
    u8 *pu8P;
    u8 *pu8End;
    s32 s32Left;        // i_count number of available bits
    u8 au8HeadInfo[30];
} TBitStream;

#define MAXIMUMVALUEOFcpb_cnt   32  //��һ��û���õ��ĺꡣ����֪����ɶ��
#define MAXnum_slice_groups_minus1  8
typedef struct  tagPicParameterSetRBSP
{
    BOOL32        bIsValid;                                            // indicates the parameter set is valid
    u32            pic_parameter_set_id;                             // ue(v)
    u32            seq_parameter_set_id;                             // ue(v)
    BOOL32        entropy_coding_mode_flag;                         // u(1)
    // if( pic_order_cnt_type < 2 )  in the sequence parameter set
    BOOL32      pic_order_present_flag;                        // u(1)
    u32            num_slice_groups_minus1;                          // ue(v)
    u32            slice_group_map_type;                                // ue(v)
    // if( slice_group_map_type = = 0 )
    u32            run_length_minus1[MAXnum_slice_groups_minus1];    // ue(v)
    // else if( slice_group_map_type = = 2 )
    u32            top_left[MAXnum_slice_groups_minus1];                // ue(v)
    u32            bottom_right[MAXnum_slice_groups_minus1];            // ue(v)
    // else if( slice_group_map_type = = 3 || 4 || 5
    BOOL32        slice_group_change_direction_flag;                // u(1)
    u32            slice_group_change_rate_minus1;                    // ue(v)
    // else if( slice_group_map_type = = 6 )
    u32            num_slice_group_map_units_minus1;                    // ue(v)
    u32            *slice_group_id;                                    // complete MBAmap u(v)
    u32            num_ref_idx_l0_active_minus1;                     // ue(v)
    u32            num_ref_idx_l1_active_minus1;                     // ue(v)
    BOOL32        weighted_pred_flag;                               // u(1)
    BOOL32        weighted_bipred_idc;                              // u(2)
    s32            pic_init_qp_minus26;                              // se(v)
    s32            pic_init_qs_minus26;                              // se(v)
    s32            chroma_qp_index_offset;                           // se(v)
    BOOL32        deblocking_filter_control_present_flag;           // u(1)
    BOOL32        constrained_intra_pred_flag;                      // u(1)
    BOOL32        redundant_pic_cnt_present_flag;                   // u(1)
    BOOL32        vui_pic_parameters_flag;                          // u(1)
    //#ifndef G50_SPS
    //    BOOL32   frame_cropping_flag;                              // u(1)
    //    u32  frame_cropping_rect_left_offset;                    // ue(v)
    //    u32  frame_cropping_rect_right_offset;                    // ue(v)
    //    u32  frame_cropping_rect_top_offset;                    // ue(v)
    //    u32  frame_cropping_rect_bottom_offset;                // ue(v)
    //#endif
} TPicParameterSetRBSP;

#define MAXnum_ref_frames_in_pic_order_cnt_cycle  255 //256
typedef struct tagSeqParameterSetRBSP
{
    BOOL32   bIsValid;                                                // indicates the parameter set is valid

    u32    profile_idc;                                        // u(8)
    //#ifdef G50_SPS
    BOOL32        constrained_set0_flag;                                // u(1)
    BOOL32        constrained_set1_flag;                                // u(1)
    BOOL32        constrained_set2_flag;                                // u(1)
    //#endif
    //uint_8  reserved_zero_5bits; /*equal to 0*/                    // u(5)
    u32            level_idc;                                            // u(8)
    //#ifndef G50_SPS
    //    BOOL32        more_than_one_slice_group_allowed_flag;                // u(1)
    //    BOOL32        arbitrary_slice_order_allowed_flag;                    // u(1)
    //    BOOL32        redundant_slices_allowed_flag;                        // u(1)
    //#endif
    u32            seq_parameter_set_id;                                // ue(v)
    u32            log2_max_frame_num_minus4;                            // ue(v)
    u32            pic_order_cnt_type;                                    // ue(v)
    // if( pic_order_cnt_type == 0 )
    u32            log2_max_pic_order_cnt_lsb_minus4;                    // ue(v)
    // else if( pic_order_cnt_type == 1 )
    BOOL32        delta_pic_order_always_zero_flag;                    // u(1)
    s32            offset_for_non_ref_pic;                                // se(v)
    s32            offset_for_top_to_bottom_field;                        // se(v)
    u32            num_ref_frames_in_pic_order_cnt_cycle;                // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
    s32            offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
    u32            num_ref_frames;                                        // ue(v)
    BOOL32        gaps_in_frame_num_value_allowed_flag;                // u(1)
    u32            pic_width_in_mbs_minus1;                            // ue(v)
    u32            pic_height_in_map_units_minus1;                        // ue(v)
    BOOL32        frame_mbs_only_flag;                                // u(1)
    // if( !frame_mbs_only_flag )
    BOOL32        mb_adaptive_frame_field_flag;                        // u(1)
    BOOL32        direct_8x8_inference_flag;                            // u(1)
    //#ifdef G50_SPS
    BOOL32        frame_cropping_flag;                                // u(1)
    u32            frame_cropping_rect_left_offset;                    // ue(v)
    u32            frame_cropping_rect_right_offset;                    // ue(v)
    u32            frame_cropping_rect_top_offset;                        // ue(v)
    u32            frame_cropping_rect_bottom_offset;                    // ue(v)
    //#endif
    BOOL32        vui_parameters_present_flag;                        // u(1)
    //not of syntax
    s32            MaxFrameNum;
} TSeqParameterSetRBSP;

typedef enum {
    P_SLICE = 0,
    B_SLICE,
    I_SLICE,
    SP_SLICE,
    SI_SLICE
} stdh264SliceType;

typedef struct
{
    u16        first_mb_in_slice;                        //ue(v)  slice�е�һ��MB�ĵ�ַ
    u16        slice_type;                                //ue(v)  slice�ı�������
    u16        pic_parameters_set_id;
} Tstdh264Dec_SliceHeaderData;


//h263/h263+ ������Ϣ����
typedef struct
{
    s16 s16PictureType; //����֡��ͼ������(I֡����P֡)
    s32 s32Width;       //��ǰ���ڽ����ͼ��Ŀ��
    s32 s32Height;      //��ǰ���ڽ����ͼ��Ŀ��

} TH263DecFrameHeader;


u32    SetBitField(u32 dwValue, u32 dwBitField, s32 nStartBit, s32 nBits);
u32    GetBitField(u32 dwValue, s32 nStartBit, s32 nBits);
void   ConvertH2N(u8 *pBuf, s32 nStartIndex, s32 nSize);
void   ConvertN2H(u8 *pBuf, s32 nStartIndex, s32 nSize);

BOOL32 IsMCastAddr(u32 dwIP);
BOOL32 IsBCastAddr(u32 dwIP);



//h264 ������Ϣ����
void   stdh264_bs_init( TBitStream *s, void *p_data, s32 i_data );
s32    stdh264_bs_pos( TBitStream *s );
s32    stdh264_bs_eof( TBitStream *s );
u32    stdh264_bs_read( TBitStream *s, s32 i_count );
u32    stdh264_bs_read1( TBitStream *s );
u32    stdh264_bs_show( TBitStream *s, s32 i_count );
void   stdh264_bs_skip( TBitStream *s, s32 i_count );
s32    stdh264_bs_read_ue( TBitStream *s );
s32    stdh264_bs_read_se( TBitStream *s );
s32    stdh264_bs_read_te( TBitStream *s, s32 x );
s32    stdh264_FirstPartOfSliceHeader(TBitStream *s, Tstdh264Dec_SliceHeaderData *dec_slice_header);

//h265 ������Ϣ����
void   stdh265_bs_init( TBitStream *s, void *p_data, s32 i_data );
s32    stdh265_bs_pos( TBitStream *s );
s32    stdh265_bs_eof( TBitStream *s );
u32    stdh265_bs_read( TBitStream *s, s32 i_count );
u32    stdh265_bs_read1( TBitStream *s );
u32    stdh265_bs_show( TBitStream *s, s32 i_count );
void   stdh265_bs_skip( TBitStream *s, s32 i_count );
s32    stdh265_bs_read_ue( TBitStream *s );
s32    stdh265_bs_read_se( TBitStream *s );
s32    stdh265_bs_read_te( TBitStream *s, s32 x );
u32    H265DecBitstreamGetBits(TBitStream *ptBs, s32 l32NBits);
s32    stdh265_FirstPartOfSliceHeader(TBitStream *s, Tstdh265Dec_SliceHeaderData *dec_slice_header, TKdvH265Header *ptH265Header, s32 l32NalType);

//h263/h263+ ������Ϣ����
void   Stdh263_BsInit( TBitStream *ptBS, void *pvData, s32 s32Data );
u32    Stdh263_BsRead( TBitStream *ptBS, s32 s32Count );
u32    Stdh263_BsRead1( TBitStream *ptBS );
u32    Stdh263_BsShow( TBitStream *ptBS, s32 s32Count);
void   Stdh263_BsSkip( TBitStream *ptBS, s32 s32Count );
BOOL32 Stdh263_GetH263PicInfo( u8 *pu8BitStream, s32 s32BitSteamLen,
                               TH263DecFrameHeader *ptH263DecPicHeader );
//h261 ������Ϣ����
BOOL32 Stdh261_H261Analyzer(u8 *pBitstream, u16 *Width, u16 *Height);

void MedianetLog(u8 byLevel, s8* lpszFormat, ...);
void MedianetPrintf(s8* lpszFormat, ...);

API void kdvmedianethelp();
API void setdiscardspan(s32 nDiscardSpan);
API void setrcvdiscardspan(s32 nRcvDiscardSpan);
API void setrepeatsend(s32 nRepeatSnd);
API void seth263dsend(s32 nRtpDSend);
API void setconfuedspan(s32 nConfuedSpan);
API void pdinfo(s32 nShowDebugInfo);
API void pbinfo(BOOL32 bShowRcv, BOOL32 bShowSnd);
API void pbrecv(u32 ObjectSeq);
API void pbsend(u32 ObjectSeq);
API void mediarecvframerate(s32 nRecvId, s32 nDelaySecond);
API void mediasndframerate(s32 nSndId, s32 nDelaySecond);
API void mediarelaystart(s32 nRecvId, s8* pchIpStr, u16 wPort);
API void mediarelaystop();
API void setsendrate(s32 nSendId, u32 dwRate);
API void pssinfo(s32 nSendId);
API void mediasendadd(s32 nSendId, s8* pchIpStr, u16 wPort);
API void mediasenddel(s32 nSendId, s8* pchIpStr, u16 wPort);
API void mediasocketinfo();
#endif //





















