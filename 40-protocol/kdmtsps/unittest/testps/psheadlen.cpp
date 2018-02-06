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

u16 main()
{//14��ps��ͷ+12�ֽ�ϵͳͷ+20�ֽ�ӳ��ͷ+ 19�ֽ�pesͷ
	u8 RtpData[1024] = {0x00, 0x00, 0x01, 0xBA, 0x45, 0x8D, 0x8D, 0x68,
			0xB4, 0x01, 0x00, 0x00, 0x03, 0xF8, 
			0x00, 0x00, 0x01, 0xBB, 0x00, 0x06, 0x80, 0x00, 0x01, 0x00, 0x20, 0x7F, 
			0x00, 0x00, 0x01, 0xBC, 0x00, 0x0E, 0xE5, 0xFF, 0x00, 0x00,
			0x00, 0x04, 0x1B, 0xE0, 0x00, 0x00, 0x48, 0x0C, 0xD3, 0xEF, 
			0x00, 0x00, 0x01, 0xE0, 0x04, 0x0A, 0x84, 0xC0, 0x0A, 0x31,
			0x63, 0x63, 0x5A, 0x2D, 0x11, 0x63, 0x63, 0x5A, 0x2D, 0x00,
			0x00, 0x00, 0x01, 0x61, 0x9A, 0x58 };
	u16 HeadLen = 0;
	u16 wRet = get_ps_head_len(RtpData, 100, &HeadLen);
	return 0;
}
/*=================================================================================
������:get_ps_head_len
����:��ӡ
�㷨ʵ��:
����˵��:[IN]Rtp��������, [IN]�������ݳ�, [OUT] psͷ����
[IN]u8 *ptRtpData, [IN]u32 dwRtpDataSize, [OUT]u16 *pwPsHeadLen
����ֵ˵��:�����Ƿ���ps��
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼

=================================================================================*/

u16 get_ps_head_len(u8 *ptRtpData, u32 dwRtpDataSize, u16 *pwPsHeadLen)
{
	u32 dwRemain = dwRtpDataSize;
	u32 dwStartPos = 0;
	u8	byTypeByte;			//�ֶ����ͱ�ʶ
	u32 dwPackLen = 0;
	u16 wRet = RPS_OK;

	while(dwRemain > 0)
	{
		if (PsReadFindHead(ptRtpData, dwRtpDataSize, &dwStartPos) != RPS_OK)
		{
			break;
// 			return RPS_ERR_STARTCODE;
		}


		//����һ��
		byTypeByte = ptRtpData[3];

		switch(byTypeByte)
		{
		case PS_HEAD_BYTE:
			wRet = PsReadParseHead(ptRtpData, dwRemain, &dwPackLen);
			break;

		case PS_SYSTEM_HEAD_BYTE:
			wRet = PsReadParseSysHead(ptRtpData, dwRemain, &dwPackLen);
			break;

        case PS_MAP_BYTE:
            wRet = PsReadParseSysHead(ptRtpData, dwRemain, &dwPackLen);
            break;
			
        case PS_END_BYTE:
            //���������ѻ��������ݻص���ȥ
            dwPackLen = 4;
            break;
			
        case VIDEO_STREAM:
        case AUDIO_STREAM:
            wRet = PsReadParsePesHead(ptRtpData, dwRemain, &dwPackLen);
			*pwPsHeadLen = dwRtpDataSize - dwRemain + dwPackLen;
			return wRet;
				
		}
		if (wRet != RPS_OK)
		{
			return wRet;
		}
		//λ��ƫ��ǰ��
		ptRtpData += dwPackLen;
		dwRemain -= dwPackLen; 
	}

	return wRet;
}


//����0x000001��ʼ��   [OUT]u32 *pdwPostion
static s32 PsReadFindHead(u8 *pbyBuf, u32 dwLen, u32 *pdwPostion)
{
    u32 dwPos = 0;
    
    while ((dwPos + 2) < dwLen)
    {
        if (pbyBuf[dwPos] == 0 && pbyBuf[dwPos+1] == 0 && pbyBuf[dwPos+2] == 1)
        {
			*pdwPostion = dwPos;
            return RPS_OK;
        }		
        dwPos++;
    }	
    return RPS_ERR_STARTCODE;
}
//��psͷ  [OUT]u32 *pPackLen
static u16 PsReadParseHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	u8 byStufLen = 0;
	if (dwLen < 14)
	{
		return RPS_ERR_PARAM;
	}
	byStufLen = pbyBuf[13] & 0x07; //  padding���� 0000 0111
	*pPackLen = 14 + byStufLen;
	return RPS_OK;
}

//��ϵͳͷ/����psm������ӳ��  [OUT]u32 *pPackLen
static u16 PsReadParseSysHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	u32 dwHeaderLenth = 0;
	if (dwLen < 12)
	{
		return RPS_ERR_PARAM;
	}
	dwHeaderLenth = (pbyBuf[4]<< 8) + pbyBuf[5];
	if ((4 + dwHeaderLenth) > dwLen)
	{
		return RPS_ERR_PARAM;
	}
	*pPackLen = 6 + dwHeaderLenth;
	return RPS_OK;
}

//����pesͷ [OUT]u32 *pPackLen
static s32 PsReadParsePesHead(u8 *pbyBuf, u32 dwLen, u32 *pPackLen)
{
	if (dwLen < 9)
	{
		return RPS_ERR_PARAM;
	}
	*pPackLen = PES_DATA_HEAD_LENGTH + PES_ES_INFO_LENGTH + pbyBuf[8];

	return RPS_OK;
}

