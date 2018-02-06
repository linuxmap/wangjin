#include "kdvtype.h"

//ps�ؼ��ֶ���
#define PACK_HEADER_START_CODE              0x000001BA  //PS��ͷ��ʼ���
#define SYSTEM_HEADER_START_CODE            0x000001BB  //PSϵͳͷ��ʼ���
#define MAP_START_CODE                      0x000001BC  //ӳ���ͷ��ʼ���
#define STREAM_END_CODE                     0x000001B9  //����ʼ���

#define PS_HEAD_BYTE                        (u8)(PACK_HEADER_START_CODE   & 0xff)
#define PS_SYSTEM_HEAD_BYTE                 (u8)(SYSTEM_HEADER_START_CODE & 0xff)
#define PS_MAP_BYTE                         (u8)(MAP_START_CODE           & 0xff)
#define PS_END_BYTE                         (u8)(STREAM_END_CODE          & 0xff)

//PES�йغ궨��
#define PES_PACKET_MAX_LENGTH       65535   //PES��(����)��󳤶�
#define PES_DATA_HEAD_LENGTH        6       //PES��ͷ����(�̶�6�ֽ�)
#define PES_ES_INFO_LENGTH          3       //ES��������Ϣ(�̶�3�ֽ�)
#define PES_PACK_MAX_LEN            (PES_PACKET_MAX_LENGTH + PES_DATA_HEAD_LENGTH)

//PES��STREAM ID
#define PROGRAM_STREAM_MAP          0xBC    //��Ŀӳ����
#define PADDING_STREAM              0xBE    //�����
#define PRIVATE_STREAM_1            0xBD    //˽����
#define PRIVATE_STREAM_2            0xBF    //˽����
#define AUDIO_STREAM                0xC0    //��Ƶ��
#define VIDEO_STREAM                0xE0    //��Ƶ��
#define ECM_STREAM                  0xF0    //ECM��
#define EMM_STREAM                  0xF1    //EMM��
#define DSM_CC_STREAM               0xF2    //DSM��
#define ISOIEC_13522_STREAM         0xF3    //13522��
#define RESERVED_DATA_STREAM        0xF0    //����������
#define PROGRAM_STREAM_DIRECTORY    0xFF    //��Ŀ��·��

//�����붨��
#define RPS_OK                                 0


#define RPS_ERR_NOT_PS			1//��ps���Ĵ�����
#define RPS_ERR_PARAM			2//��������
#define RPS_ERR_STARTCODE		3//�Ҳ���001��ʼ��

u16 get_ps_head_len(u8 *ptRtpData, u32 dwRtpDataSize, u16 *pwPsHeadLen);

s32 PsReadFindHead(u8 *pbyBuf, u32 dwLen, u32 *pdwPostion);

u16 PsReadParseHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);
u16 PsReadParseSysHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);
s32 PsReadParsePesHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen);
