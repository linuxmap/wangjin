/*=================================================================================
ģ����:TS/PSģ��ӿ��ļ�
�ļ���:tsps.h
����ļ�:kdvtype.h osp_small.h
ʵ�ֹ���:TS/PSģ��ӿ�
����:
��Ȩ:<Cpoyright(C) 2003-2007 Suzhou KeDa Technology Co.,Ltd. All rights reserved.>
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/

#ifndef TSPS_H
#define TSPS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  

/************************************************************************/
/* Tsps(C) version                                                      */
/************************************************************************/
#define TSPS_VERSION    "Tsps(C) for KDM 1.0.0.20081117"
API void tspsver();

/************************************************************************/
/* TS��PS�����ȶ���                                                     */
/************************************************************************/
#define TS_PACKET_LENGTH                        (188)     //TS������
#define PS_PACKET_LENGTH                        (65535)    //PS����󳤶�(���65535)

/************************************************************************/
/* ������                                                               */
/************************************************************************/
#define TSPS_OK                                 0
#define ERROR_TSPS_BASE                         (u16)18000
#define TSPS_ERROR(n)                           (u16)(ERROR_TSPS_BASE + n)

//PES����������Ϣ
#define ERROR_PES_READ_INPUT_PARAM              TSPS_ERROR(1) //�����������
#define ERROR_PES_READ_CODE_PREFIX              TSPS_ERROR(2) //ǰ׺λ����
#define ERROR_PES_READ_LENGTH                   TSPS_ERROR(3) //pes�����ȴ���
#define ERROR_PES_READ_FIX_STREAMID             TSPS_ERROR(4) //STREAM ID�̶�λ����
#define ERROR_PES_READ_FIX_PTS                  TSPS_ERROR(5) //PTS�̶�λ����
#define ERROR_PES_READ_FIX_PTS_DTS_03           TSPS_ERROR(6) //PTS DTS�̶�λ����
#define ERROR_PES_READ_FIX_PTS_DTS_01           TSPS_ERROR(7) //PTS DTS�̶�λ����
#define ERROR_PES_READ_FIX_STD                  TSPS_ERROR(8) //STD�̶�λ����                                
#define ERROR_PES_READ_TYPE_NOT_SUPPORT         TSPS_ERROR(9) //��֧�ֵĸ�ʽ
#define ERROR_PES_WRITE_INPUT_PARAM             TSPS_ERROR(10) //�����������

//TS����������Ϣ
#define ERROR_TS_WRITE_INPUT_PARAM              TSPS_ERROR(101) //�����������
#define ERROR_TS_WRITE_STREAM_NUM               TSPS_ERROR(102) //������������
#define ERROR_TS_WRITE_PROGRAM_NUM              TSPS_ERROR(103) //�����Ŀ������
#define ERROR_TS_WRITE_STREAM_TYPE              TSPS_ERROR(104) //���������ʹ���
#define ERROR_TS_WRITE_INPUT_FRAME              TSPS_ERROR(105) //����֡����
#define ERROR_TS_WRITE_NOT_SUPPORT              TSPS_ERROR(106) //��֧��
#define ERROR_TS_WRITE_SET_ENCRYPTKEY           TSPS_ERROR(107) //������Կ����
#define ERROR_TS_WRITE_MEMORY                   TSPS_ERROR(108) //�ڴ�������

#define ERROR_TS_READ_INPUT_PARAM               TSPS_ERROR(201) //�����������
#define ERROR_TS_READ_UNIT_START                TSPS_ERROR(202) //UNIT_START����
#define ERROR_TS_READ_ADAPTATION                TSPS_ERROR(203) //ADAPTATION����
#define ERROR_TS_READ_HEAD_LENGTH               TSPS_ERROR(204) //����ͷ������
#define ERROR_TS_READ_PSI_LENGTH                TSPS_ERROR(205) //psi���ȴ���
#define ERROR_TS_READ_TABLE_ID                  TSPS_ERROR(206) //��id����
#define ERROR_TS_READ_SECTION_SYNTAX            TSPS_ERROR(207) //���﷨����
#define ERROR_TS_READ_FIX_0                     TSPS_ERROR(208) //�̶�0���ش���
#define ERROR_TS_READ_NOT_SUPPORT               TSPS_ERROR(209) //��֧�ֵ�ǰý��
#define ERROR_TS_READ_PID_NOT_FOUND             TSPS_ERROR(210) //δ��pmt���ҵ���
#define ERROR_TS_READ_STREAM_TYPE               TSPS_ERROR(211) //�����ʹ���
#define ERROR_TS_READ_BUFF_FULL                 TSPS_ERROR(212) //֡���ջ�����
#define ERROR_TS_READ_HEAD_SYNC                 TSPS_ERROR(213) //ͬ���ֽڴ���
#define ERROR_TS_SEGMENT_FILE_ERROR				TSPS_ERROR(214)	// ��Ƭ�ļ���ʧ��

//PS����������Ϣ
#define ERROR_PS_WRITE_INPUT_PARAM              TSPS_ERROR(301) //�����������
#define ERROR_PS_WRITE_STREAM_NUM               TSPS_ERROR(302) //������������
#define ERROR_PS_WRITE_INPUT_FRAME              TSPS_ERROR(303) //����֡����
#define ERROR_PS_WRITE_STREAM_TYPE              TSPS_ERROR(304) //���������ʹ���

#define ERROR_PS_READ_INPUT_PARAM               TSPS_ERROR(401) //�����������
#define ERROR_PS_READ_PARSE_FAIL                TSPS_ERROR(402) //�����ݽ���ʧ��
#define ERROR_PS_READ_BUFF_FULL                 TSPS_ERROR(403) //����pes������


/************************************************************************/
/* PsTs�ӿ�                                                             */
/************************************************************************/

//�������
#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) typedef struct name##__ { int unused; } *name;
#endif

DECLARE_HANDLE(HTspsWrite);
DECLARE_HANDLE(HTspsRead);

//�����ͣ�ps��ts
typedef enum tagEStreamType
{
    TRANSPORT_STREAM,
    PROGRAM_STREAM
} EStreamType;

#define TspsGetStreamType(handle)       (*(EStreamType *)handle)
//֡���ݽṹ�����ڲ��ܰ���c++�⣬���¶��壬��medianet��ͬ
typedef struct tagTspsFrameHeader
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
	u8   m_byStreamID;
} TspsFRAMEHDR, *PTspsFRAMEHDR;

//�ص���������
typedef u16 (*TspsFrameCallback)(TspsFRAMEHDR *ptFrame, KD_PTR pvContext);
typedef u16 (*TspsProgramCallback)(u8 u8VideoPT, u8 u8AudioPT0, u8 u8AudioPT1, KD_PTR pvContext);
typedef u16 (*TspsSectionCallback)(u8 *pu8Buf, u32 u32Len, KD_PTR pvContext, TspsFRAMEHDR *ptFrame);

//write
HTspsWrite TspsWriteOpen(EStreamType eType, TspsSectionCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize);
u16 TspsWriteClose(HTspsWrite hWrite);
u16 TspsWriteSetProgram(HTspsWrite hWrite, u8 u8VideoType, u8 u8AudioType);

/** ����TS��Ƭ�ļ�����
*	u32SegmentDuration ��ƬƬ��ʱ��
*	pu8FileDir	��Ƭ�ļ��������ļ�Ŀ¼��NULLΪִ�г���ǰĿ¼����Ӧ��Ϊapache����Ŀ¼����pu8HttpPrefixĿ¼
*	pu8OutputPrefix ��Ƭ����ļ�ǰ׺
*	pu8IndexFileName �����ļ����ƣ�m3u8��
*	pu8HttpPrefix httpǰ׺
*	u32SegmentWindow ��Ƭ���ڣ�0Ϊ�����ô��ڣ�  
*/
u16 TsSetSegmentParam(HTspsWrite hWrite, const u32 u32SegmentDuration, const s8* pu8FileDir, const s8* pu8OutputPrefix,
					  const s8* pu8IndexFileName, const s8* pu8HttpPrefix, const u32 u32SegmentWindow );

u16 TspsWriteWriteFrame(HTspsWrite hWrite, TspsFRAMEHDR *ptFrame);
u16 TspsSetEncryptKey(HTspsWrite hWrite, s8 *pszKeyBuf, u16 wKeySize, s8* pszIV, s8 *pszUrlBuf, u16 wUrlLen);
u16 TspsWriteWriteEnd(HTspsWrite hWrite);

//read
HTspsRead TspsReadOpen(EStreamType eType, TspsFrameCallback pfCallback, KD_PTR pvContext, u32 dwMaxFrameSize);
u16 TspsReadClose(HTspsRead hRead);
u16 TspsReadSetProgramCallback(HTspsRead hRead, TspsProgramCallback pfCallback, KD_PTR pvContext);
u16 TspsReadInputStream(HTspsRead hRead, u8 *pu8Buf, u32 u32Len);
u16 TspsReadResetStream(HTspsRead hRead);
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif //TSPS_H








