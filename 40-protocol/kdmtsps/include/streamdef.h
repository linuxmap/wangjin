/*=================================================================================
ģ����:����ģ��
�ļ���:streamdef.h
����ļ�:
ʵ�ֹ���:
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
#ifndef STREAMDEF_H
#define STREAMDEF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/*=================================================================================
ģ����:PES���Ķ�д
�ļ���:pes.ch
����ļ�:kdvtspslib.h, kdmpstscommon.h
ʵ�ֹ���:PES���Ķ�д
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/

// ���Ӹ���֧�֣����֡���ȿɴ�512k��ʵ��pes��������Ϊ512k+19��ligeng@2012.10.25
#define ES_PACEKT_MAX_LENGTH		512*1024+19	// ����ES�����֡����(512k)���ټ���pesͷ���������19�ֽ�	
//PES�йغ궨��
#define PES_PACKET_MAX_LENGTH       65535   //PES��(����)��󳤶�
#define PES_DATA_HEAD_LENGTH        6       //PES��ͷ����(�̶�6�ֽ�)
#define PES_ES_INFO_LENGTH          3       //ES��������Ϣ(�̶�3�ֽ�)
#define PES_PACK_MAX_LEN            (512*1024)//(PES_PACKET_MAX_LENGTH + PES_DATA_HEAD_LENGTH)

//PES��STREAM ID
#define PROGRAM_STREAM_MAP          0xBC    //��Ŀӳ����
#define PADDING_STREAM              0xBE    //�����
#define PRIVATE_STREAM_1            0xBD    //˽����
#define PRIVATE_STREAM_2            0xBF    //˽����
#define AUDIO_STREAM0               0xC0    //��Ƶ��0
#define AUDIO_STREAM1               0xC1    //��Ƶ��1
#define VIDEO_STREAM                0xE0    //��Ƶ��
#define ECM_STREAM                  0xF0    //ECM��
#define EMM_STREAM                  0xF1    //EMM��
#define DSM_CC_STREAM               0xF2    //DSM��
#define ISOIEC_13522_STREAM         0xF3    //13522��
#define RESERVED_DATA_STREAM        0xF0    //����������
#define PROGRAM_STREAM_DIRECTORY    0xFF    //��Ŀ��·��

u8 TsPsGetStreamIdPrefix(u8 u8StreamType);
    

//PES�ṹ����
typedef struct tagPesInfo
{
    u64 u64Pts;                                         //PTS
    u64 u64Dts;                                         //DTS
    u64 u64EscrBase;                                    //�ο�ʱ��(90khz)
    
    u32 u32PacketLength;                                //������(�Ӵ��ֶ����һλ����,��������λ)
    u32 u32EsRate;                                      //����
    
    u32 u32PayloadLength;                               //ES������
    u32 u32PesLength;                                    //PES������
    
    u16 u16EscrExtension;                                //�ο�ʱ��(90Mhz)
    u16 u16PreviousPesPacketCrc;                         //û��ʹ��
    u16 u16PStdBufferSize;                               //û��ʹ��
    
    u8 u8StreamId;                                      //����
    u8 u8ScramblingControl;                             //���ܿ���
    u8 u8Priority;                                      //���ȼ�
    u8 u8DataAlignment;                                 //���ݶ����־
    u8 u8Copyrigth;                                     //��Ȩ
    u8 u8OriginalOrCopy;                                //ԭʼ����/����
    u8 u8PtsDtsFlag;                                    //PTS,DTS��Ϣ��־
    u8 u8EscrFlag;                                      //ESCR��־
    u8 u8EsRateFlag;                                    //���������ʱ�־
    u8 u8DsmTrickModeFlag;                              //DSM����ģʽ
    u8 u8AdditionalCopyInfoFlag;                        //������Ϣ
    u8 u8CrcFlag;                                       //CRC��־
    u8 u8ExtensionFlag;                                 //��չ��־
    u8 u8HeadDataLength;                                //ͷ����

	BOOL32  bWriteDts;  //��֡�Ƿ�д��DTS�� д���ٱ��Ϊ��Ч�����ڻص�֮�󣬻ص�����ǰһ֡
	BOOL32  bDtsValid;  //��֡dts�Ƿ���Ч
    //��������û��ʹ��
    u8 u8TrickModeControl;                              //���Կ���
    u8 u8FieldId;
    u8 u8IntraSliceRefresh;
    u8 u8FrequencyTruncation;
    u8 u8FieldRepCntrl;
    u8 u8AdditionalCopyInfo;                            //������Ϣ
    
    u8 u8PrivateDataFlag;
    u8 u8PackHeaderFieldFlag;
    u8 u8ProgramPackSequenceCounterFlag;
    u8 u8PStdBufferFlag;
    u8 u8ExtensionFlag2;
    u8 u8PrivateData[128 / 8];
    u8 u8PackFieldLength;
    u8 u8ProgramPacketSequenceCounter;
    u8 u8Mpeg1Mpeg2Identifier;
    u8 u8OriginalStuffLength;
    u8 u8PStdBufferScale;
    u8 u8ExtensionFieldLength;
    
    u8 *pu8PayloadBuffer;                               //ES��ָ��
    u8 *pu8PesBuffer;                                    //PES��ָ��
}TPesInfo;

/*=================================================================================
������:PESReadInfo
����:����PES������,���ES��
�㷨ʵ��: 
����˵��:
         [I/O] ptpPesInfo �洢PES����Ϣ�Ľṹ
         [I]   pu8BufInput PES��
         [I]   u32LenInput PES������
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PesReadInfo(TPesInfo *ptPesInfo, u8 *pu8BufInput, u32 u32LenInput, u32* u32LenOutput);

/*=================================================================================
������:PESWriteInfo
����:��һ֡ES��д��PES��
�㷨ʵ��: (�ڽ���PES�������ʱ,������д��ȷ��stream id,DTS����PTS(����))
����˵��:
         [I/O] ptpPesInfo �洢PES����Ϣ�Ľṹ
         [I/O] pu8BufOutput PES��
         [I/O] u32LenOutput PES������
����ֵ˵��:�ɹ�����TSPS_OK,���򷵻ش����
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 PesWriteInfo(TPesInfo *ptPesInfo, u8 *pu8BufOutput, u32 *pu32LenOutput);



/*=================================================================================
ģ����:PS����д
�ļ���:ps.h
����ļ�:kdvtspslib.h, kdmpstscommon.h
ʵ�ֹ���:PS����д
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
#define PACK_HEADER_START_CODE              0x000001BA  //PS��ͷ��ʼ���
#define SYSTEM_HEADER_START_CODE            0x000001BB  //PSϵͳͷ��ʼ���
#define MAP_START_CODE                      0x000001BC  //ӳ���ͷ��ʼ���
#define STREAM_END_CODE                     0x000001B9  //����ʼ���

#define PS_HEAD_BYTE                        (u8)(PACK_HEADER_START_CODE   & 0xff)
#define PS_SYSTEM_HEAD_BYTE                 (u8)(SYSTEM_HEADER_START_CODE & 0xff)
#define PS_MAP_BYTE                         (u8)(MAP_START_CODE           & 0xff)
#define PS_END_BYTE                         (u8)(STREAM_END_CODE          & 0xff)

#define MAX_STREAM_NUM_IN_PROGRAM           4           //����������

#define PS_SYSHEAD_BUFSCALE_0               128
#define PS_SYSHEAD_BUFSCALE_1               1024

//psͷ����
#define PS_HEAD_LEN             14
//pes����ǰͷ�ĳ��� = pesͷ�� + pes��es���ֶγ��� + PTS��DTS�ֶγ���
#define PS_PES_PREDATA_LEN      (PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + 10)
//�з�ÿ��es�ĳ��� = psÿ�γ��� - psͷ�� - pes����ǰͷ�ĳ���
#define PS_ES_CUT_LEN   (PS_PACKET_LENGTH - PS_HEAD_LEN - PS_PES_PREDATA_LEN)       

//PS����Ϣ
typedef struct tagPsProgramInfo
{
    u8  au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];
    u8  au8StreamType[MAX_STREAM_NUM_IN_PROGRAM];       //������
    u8  u8StreamNum;                                    //�˽�Ŀ���ж���·��
} TPsProgramInfo;

//PSͷ�ṹ����
typedef struct tagPsHeadInfo
{
    u64   u64SCRBase;                                   //ʱ���׼
    u16   u16SCRExt;
    
    u32   u32ProgramMuxRate;                            //����������
    
    u8   *pu8Buffer;                                    //�洢PSͷ�Ļ��� 
    u32   u32BuffLength;                                //PSͷ����
} TPsHeadInfo;

//ϵͳͷ�ṹ����
typedef struct tagPsSysHeaderInfo
{
    u16   u16HeaderLength;                              //ͷ����

    u32   u32RateBound;
    u8    u8AudioBound;
    u8    u8FixedFlag;
    u8    u8CspsFlag;
    u8    u8SystemAudioLockFlag;
    u8    u8SystemVideoLockFlag;
    u8    u8VideoBound;
    u8    u8PacketRateRestrictionFlag;

    u8    au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];       //��ID
    u8    au8PStdBufferBoundScale[MAX_STREAM_NUM_IN_PROGRAM];
    u16   au16PStdBufferSizeBound[MAX_STREAM_NUM_IN_PROGRAM];

    u8    u8StreamNum;
    u8   *pu8Buffer;                                    //�洢PSϵͳͷ�Ļ���
    u32   u32BuffLength;                                //PSϵͳͷ����
} TPsSysHeaderInfo;

//ӳ��νṹ����
typedef struct tagPsMapInfo
{
    u16   u16MapLength;                                 //ӳ��γ���
    u8    u8CurrentNextIndicator;                       //�ö��Ƿ���Ч
    u8    u8Version;                                    //�汾��
    u16   u16PsInfoLength;
    u16   u16EsMapLength;                               //ESӳ�䳤��

    u8    au8StreamType[MAX_STREAM_NUM_IN_PROGRAM];     //��ID
    u8    au8StreamId[MAX_STREAM_NUM_IN_PROGRAM];       //ԭʼ������PES�����STREAM_ID
    u16   au16EsInfoLength[MAX_STREAM_NUM_IN_PROGRAM];  //ES��������
    
    u8    u8StreamNum;                                  //������
    u8   *pu8Buffer;                                    //�洢ӳ��εĻ���
    u32   u32BuffLength;                                //ӳ��γ���
} TPsMapInfo;

// ps write�����ڴ������ԣ�ȫ��ʹ��pswrite�з�����ڴ�
typedef struct tagPsWrite
{
    TPsHeadInfo tHead;
    TPsSysHeaderInfo tSysHead;
    TPsMapInfo tMap;
    
    TspsSectionCallback pfCallback;                    //�ص�����
    void *pvContext;                                   //�ص�������ָ��

    u8 *pu8Buf;                                        //ps frame buf
    u32 u32Len;                                        //���泤��

    TPesInfo *ptPesInfo;

    //һ��ps������ֻ����һ����Ƶ����Ƶ
    u8 u8VideoType;
    u8 u8AudioType;
    u32 u32LastTimestamp;

//��¼�ϲ㴫�����������֡��
    u32 dwmaxframesize;
} TPsWrite;

// ps read
typedef struct tagPsRead
{
    TPsHeadInfo tHead;
    TPsSysHeaderInfo tSysHead;
    TPsMapInfo tMap;
    
    TspsFrameCallback pfFrameCB;                         //֡�ص�����
    void *pvFrameContext;                                //֡�ص�������ָ��
    TspsProgramCallback pfProgramCB;                     //��Ŀ��Ϣ�ص�����
    void *pvProgramContext;                              //��Ŀ��Ϣ�ص�������ָ��
    
    u8 *pu8FrameBuf;                                     //֡����
    u32 u32FrameLen;                                     //֡����

    u8 *pu8InBuf;
    u32 u32InLen;
    
    TspsFRAMEHDR *ptFrame;
    TPesInfo *ptPesInfo;

    BOOL32 bNotSupport;    //��������������Ҳ���һ����Ƶһ����Ƶ����ʱ��֧�ֽ���
    u8 u8VideoID;
    u8 u8AudioID0;
	u8 u8AudioID1;
    u8 u8VideoType;
    u8 u8AudioType0;
	u8 u8AudioType1;
    u32 u32LastTS;
    u8 u8LastType;
	u8 u8LastStreamID;

	u32 u32Width;
	u32 u32Height;

	BOOL32 bReadHead;

	BOOL32 bFirstPacket;

	u32    dwVideoFrameID;
	u32    dwAudioFrameID0;
	u32    dwAudioFrameID1;

	u32    dwmaxframesize;
    
} TPsRead;

/*=================================================================================
ģ����:TS���Ķ�д
�ļ���:ts.h
����ļ�:kdvtspslib.h, kdmpstscommon.h
ʵ�ֹ���:TS���Ķ�д
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
//TS��ͷ���
#define TS_PACKET_SYNC          0x47
#define TS_HEADER_LEN           4

#define TS_SEND_PSI_STEP        40      //�����ٰ�����һ��psi��Ϣ

#define MAX_SECTION_LENGTH      4 //һ��PSI��Ϣ��������ж��ٶ�(8λ)
#define MAX_PROGRAM_MAP_NUM     MAX_SECTION_LENGTH //PAT��Ϣ����Ա������PMT
#define MAX_STREAM_NUM          MAX_SECTION_LENGTH //PMT��Ϣ�����п��Ա����������Ϣ

//����PSI��Ϣ��ID
#define TS_PAT_TABLE_ID         0x00    //PAD��ID
#define TS_PMT_TABLE_ID         0x02    //PMT��ID
#define TS_SDT_ID               0x42
#define TS_NULL_PACKET_ID       0x1fff//�հ�

#define TS_AFC_TYPE_PAYLOAD     0x01
#define TS_AFC_TYPE_AF          0x02
#define TS_AFC_TYPE_AF_PAYLOAD  0x03

#define TS_RAND_MAX             8192    //��������ֵ

#define TS_SEGMENT_FILE_LEN				128		//��Ƭ�ļ����Ƴ���

//������(�γ�TS��ʱ�ĳ�ʼ����Ϣ,һ���ṹ����һ��PMT�ṹ����Ҫ����Ϣ)
typedef struct tagTsProgramInfo
{
    u16 au16StreamPid[MAX_PROGRAM_MAP_NUM];
    u8  au8StreamType[MAX_PROGRAM_MAP_NUM];              //������
    u8  u8StreamNum;                                     //�˽�Ŀ���ж���·��
}TTsProgramInfo;

typedef struct tagTsPatPrograms 
{
    TTsProgramInfo atProgram[MAX_STREAM_NUM];
    u8 u8ProgramNum;
} TTsPatPrograms;

//PAT��ṹ����
typedef struct tagTsPatInfo
{
    u32 u32Crc;
    
    u16 u16SectionLength;                                //�γ���
    u16 u16TransportStreamId;                            //��ID
    
    u16 au16ProgramNumber[MAX_PROGRAM_MAP_NUM];          //��Ŀ��
    u16 au16NetworkPid[MAX_PROGRAM_MAP_NUM];             //����PID
    u16 au16ProgramMapPid[MAX_PROGRAM_MAP_NUM];          //PMT��PID
    
    u8 u8VersionNumber;                                  //�汾��
    u8 u8CurrentNextIndicator;                           //�˰��Ƿ���Ч
    u8 u8SectionNumber;                                  //�κ�
    u8 u8LastSectionNumber;                              //���Ķκ�
    u8 u8ProgramMapNum;                                  //PAT�����ж��ٸ�PMT
    u8 u8ContinuityCounter;                              //����(0x0-0xfѭ��)
    
    u8 *pu8Buffer;                                       //�洢PAT���Ļ���
}TTsPatInfo;

//PMT��ṹ����
typedef struct tagTsPmtInfo
{
    u32 u32Crc;
    
    u16 u16SectionLength;                                //�γ���
    u16 u16ProgramNumber;                                //PMT PID
    u16 u16PcrPid;                                       //PCR��׼����PID
    u16 u16ProgramInfoLength;                            //�洢��Ϣ����
    
    u16 au16ElementaryPid[MAX_STREAM_NUM];               //��PID
    u16 au16EsInfoLength[MAX_STREAM_NUM];                //��������Ϣ����
    
    u8 u8VersionNumber;                                  //�汾��
    u8 u8CurrentNextIndicator;                           //�˰��Ƿ���Ч
    u8 u8SectionNumber;                                  //�κ�
    u8 u8LastSectionNumber;                              //���Ķκ�
    
    u8 au8StreamType[MAX_STREAM_NUM];                    //������
    
    u8 u8StreamNum;                                      //PMT�����ж��ٸ���
    u8 u8ContinuityCounter;                              //����(0x0-0xfѭ��)
    
    u8 *pu8Buffer;                                       //�洢PMT���Ļ���
}TTsPmtInfo;

//tsͷ��Ϣ
typedef struct tagTsHeader
{
    u16 u16Pid;                                          //PIDֵ
    u8 u8TransportErrorIndicator;                        //���������
    u8 u8PayloadUnitStartIndicator;                      //�Ƿ���һ֡�ĵ�һ���ֽ�
    u8 u8TransportPriority;                              //�������ȼ�
    u8 u8TransportScramblingControl;                     //�Ƿ����
    u8 u8AdaptationFieldControl;                         //�����ֶ�
    u8 u8ContinuityCounter;                              //����(0x0-0xfѭ��)
    u8 u8AdaptationFieldLength;
    u8 u8AdaptationFlags;
    u8 u8HeadLen;
} TTsHeader;

//��Ƭ�ṹ
typedef struct tagTsSegment
{
	u32 u32LastSegmentTime;				// ���һ���и�ʱ��
	u32 u32FirstSegment;			// �����ļ��е�һ����Ƭ����
	u32 u32LastSegment;				// �����ļ������һ����Ƭ����
	u32 u32SegmentDuration;				// ��Ƭʱ������Ĭ��ȡ10s��
	/*u8 aau8FileDir[TS_SEGMENT_FILE_LEN];	// �����ļ�����Ƭ�ļ�·����Ӧ���� / ��β*/
	s8 aau8IndexFileName[TS_SEGMENT_FILE_LEN];		// �����ļ����ƣ�m3u8��
	s8 aau8OutputFilePrefix[TS_SEGMENT_FILE_LEN];	// �����Ƭ�ļ�ǰ׺
	s8 aau8HttpPrefix[TS_SEGMENT_FILE_LEN];		// http��ַǰ׺
	u32 u32SegmentWindow;			// ��Ƭ���ڣ�0Ϊ�����ã�
	FILE *fpSegment;		// ��Ƭ�ļ�ָ��
}TTsSegment;

#ifndef DES_ENCRYPT_MODE
#define DES_ENCRYPT_MODE         (u8)0      //DES����ģʽ
#define AES_ENCRYPT_MODE         (u8)1      //AES����ģʽ
#define ENCRYPT_KEY_SIZE         (u8)32     //��Կ���� ȡ���еĽϴ�ֵ
#define DES_ENCRYPT_KEY_SIZE     (u8)8      //DES��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_A (u8)16  //AES Mode-A ��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_B (u8)24  //AES Mode-B ��Կ����
#define AES_ENCRYPT_KEY_SIZE_MODE_C (u8)32  //AES Mode-C ��Կ����
#define AES_ENCRYPT_BYTENUM    (s32)16// DES ����Ϊ16�ֽڶ���
#endif

//TSд�ṹ����
typedef struct tagTsWrite
{
    //���¼���PCRʹ��
    u64 u64PCRBase;                                      //��׼ʱ��
    u16 u16PCRExt;
    
    TTsHeader tHeader;

    u16 au16Pid[MAX_PROGRAM_MAP_NUM * MAX_STREAM_NUM];   //��¼�ѻ�õ�PID,����PID�ظ�
    
    TTsPatInfo tPatInfo;
    TTsPmtInfo *ptPmtInfo;
    
    u8 aau8ContinuityCounter[MAX_PROGRAM_MAP_NUM][MAX_STREAM_NUM]; //ÿ�����ļ���

    u8 *pu8TsBuf;                                        //TS������

    TPesInfo *ptPesInfo;

    u8 *pu8PesBuf;                                       //PES������
    u32 u32Peslen;                                       //PES������
    
    TspsSectionCallback pfCallback;                      //�ص�����
    void *pvContext;                                     //�ص�������ָ��

    u32 u32TsPesCount;                                   //��¼section�����̶������psi

    //һ��ts������ֻ����һ����Ƶ����Ƶ
    u8 u8VideoType;
    u8 u8AudioType;

	TTsSegment *ptTsSegment;

	SEMHANDLE hWriteSem;		// дts�ļ���Ҫ�������ϴλ�������Ƶ�����߳̽���д�ļ�

	s8 *pszKeyBuf;
	u16 wKeySize;
	s8 szIV[16];
	s8 szFirstIV[16];
	s8 *pszUrlBuf;
	u16 wUrlLen;
	u8 byTempEncryptBuf[176];
	u16 wTempEncryptLen;
} TTsWrite;

typedef struct  tagTsInfo
{
	u8 *pu8PesBuf;
	u32 u32Peslen;
	TPesInfo *ptPesInfo;
	u16 u16CurPesLen;
	BOOL32 bWaitNextPes;
	u8 u8Type;
	u8 u8LastType;
	u32 u32FrameID;
}TTsInfo;

//TS���ṹ��
typedef struct tagTsRead
{
    TTsHeader tHeader;

    TTsPatInfo tPatInfo;
    TTsPmtInfo *ptPmtInfo;

    TspsFrameCallback pfFrameCB;                         //֡�ص�����
    void *pvFrameContext;                                //֡�ص�������ָ��
    TspsProgramCallback pfProgramCB;                     //��Ŀ��Ϣ�ص�����
    void *pvProgramContext;                              //��Ŀ��Ϣ�ص�������ָ��
	BOOL32 bSupport;    //��������������Ҳ���һ����Ƶһ����Ƶ����ʱ��֧�ֽ���
	TspsFRAMEHDR *ptFrame;

	TTsInfo  atTsInfo[MAX_PROGRAM_MAP_NUM];
	u32 u32Width;
	u32 u32Height;

	BOOL32 bFirstPack;
	u8     byTempTSBuf[188];
	u32    dwTempBufLen;


//     u8 *pu8VideoPesBuf;                                       //��ƵPES������
//     u32 u32VideoPeslen;                                       //��ƵPES������
// 
// 	
//     u8 *pu8AudioPesBuf;                                       //��ƵPES������
//     u32 u32AudioPeslen;                                       //��ƵPES������
// 
// 
//     
//     TPesInfo *ptVideoPesInfo;
// 	TPesInfo *ptAudioPesInfo;
//     u16 u16CurVideoPesLen;
// 	u16 u16CurAudioPesLen;
//     BOOL32 bVideoWaitNextPes;
// 	BOOL32 bAudioWaitNextPes;
// 
//     
//     u8 u8VideoType;
//     u8 u8AudioType;
//     u8 u8LastVideoType;
// 	u8 u8LastAudioType;
// 
// 	u32 u32VideoFrameID;
// 	u32 u32AudioFrameID;

} TTsRead;

/************************************************************************/
/* internal API                                                         */
/************************************************************************/

// write to ts
TTsWrite *TsWriteOpen(TspsSectionCallback pfCallback, void *pvContext);
u16 TsWriteClose(TTsWrite *ptTsInfo);
u16 TsWriteSetProgram(TTsWrite *ptTsInfo, u8 u8VideoType, u8 u8AudioType);
u16 TsWriteSegmentParam(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment);
u16 TsWriteWriteEsFrame(TTsWrite *ptTsInfo, TspsFRAMEHDR *ptFrame);
u16 TsWriteSetEncryptKey(TTsWrite *ptTsInfo, s8 *pszKeyBuf, s8* pszIV, u16 wKeySize, s8 *pszUrlBuf, u16 wUrlLen);
u16 TsWriteEncryptBuffer(TTsWrite *ptTsInfo, TTsSegment *ptTsSegment, u8* pbyBuffer, u32 dwBuffLen);

// write to ps
TPsWrite *PsWriteOpen(TspsSectionCallback pfCallback, void *pvContext, u32 dwMaxFrameSize);
u16 PsWriteClose(TPsWrite *ptPsInfo);
u16 PsWriteSetProgram(TPsWrite *ptPsInfo, u8 u8VideoType, u8 u8AudioType);
u16 PsWriteWriteEsFrame(TPsWrite *ptPsInfo, TspsFRAMEHDR *ptFrame);
u16 PsWriteWriteEnd(TPsWrite *ptPsInfo);

// read from ts
TTsRead *TsReadOpen(TspsFrameCallback pfCallback, void *pvContext);
u16 TsReadClose(TTsRead *ptTsInfo);
u16 TsReadSetProgramCallback(TTsRead *ptTsInfo, TspsProgramCallback pfCallback, void *pvContext);
u16 TsReadInputStream(TTsRead *ptTsInfo, u8 *pu8Buf, u32 u32Len);
u16 TsGetArrayByStreamType(TTsRead *ptTsInfo, u8 u8StreamType, u8* pu8Id);

// read from ps
TPsRead *PsReadOpen(TspsFrameCallback pfCallback, void *pvContext, u32 dwMaxFrameSize);
u16 PsReadClose(TPsRead *ptPsInfo);
u16 PsReadSetProgramCallback(TPsRead *ptPsInfo, TspsProgramCallback pfCallback, void *pvContext);
u16 PsReadInputStream(TPsRead *ptPsInfo, u8 *pu8Buf, u32 u32Len);
u16 PsReadResetStream(TPsRead *ptPsInfo);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // TSPS_H

