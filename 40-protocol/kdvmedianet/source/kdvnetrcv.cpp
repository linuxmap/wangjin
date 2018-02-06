  /*****************************************************************************
ģ����      : KdvMediaNet
�ļ���      : KdvNetRcv.cpp
����ļ�    : KdvNetRcv.h
�ļ�ʵ�ֹ���: CKdvNetRcv Implementation
����        : κ�α�
�汾        : V2.0  Copyright(C) 2003-2005 KDC, All rights reserved.
-----------------------------------------------------------------------------
�޸ļ�¼:
��  ��      �汾        �޸���      �޸�����
2003/06/03  2.0         κ�α�      Create
2003/06/03  2.0         κ�α�      Add RTP Option
2004/04/13  3.0         ����      ����mp4�ش�������
2004/04/26  3.0         ����      ��rtcp�ش����м��뱾��rtp����ip��port����Ϣ���Ա��ش�����
2004/06/02  3.0         ����      ����H.261\H.263���Ӷ�ͬһ֡�ĸ���С����ʱ������б�
2004/06/08  3.0         ����      ���� H.263 ���� B MODE �� C MODE �Ľ�������
2004/06/14  3.0         ����      ���� H.261\H.263 ����������Ĵ���
2004/06/15  3.0         ����      ��������
2004/06/17  3.0         ����      ��� H.261\H.263 ��������� �����Ĺ��������
2004/07/08  3.0         ����      ��������
2004/07/23  3.0         ����      Ĭ�ϴ���Ƶ��������
2004/08/03  3.0         ����      ����g.723��30ms����֡���
2004/08/05  3.0         ����      mp3 �������Ŵ�������
2004/08/05  3.0         ����      ���� g.7xx �������Ŵ�������
2004/08/14  3.0         ����      ȥ��H.263/H.261������mark����
2004/08/24  3.0         ����      ��u16 �� +1��-1 �������ܳ��ַ�ת��Ӧ��ǿ��ת��Ϊu16��������¼��Ƚ�
2004/09/01  3.0         ������      ���ӱ�׼H264���ܹ���̩�ö�ͨ��RTP���Ľ��չ���
2004/08/25  3.0         ����      ������Ƶ֡��ţ��Ա�������ʶ�������л�����ƵԴ
2004/09/22  3.0         ����      ������Ƶ֡��ţ��Ա�������ʶ�������л�����ƵԴ
2004/09/22  3.0         ����      ���ӽ��ն˶���H.263+��̬�غɵĽ�����֡����
2004/09/22  3.0         ����      ���� H.26X ����ʱ�����ת/���ڵĴ������
2004/09/22  3.0         ����      ���� H.26X ���ڿ�֡���жϱ�׼��������ʱ���==0,����m_byPackNum==0Ϊ��׼
2004/09/23  3.0         ����      ����H.263���ɱ�ʶ�ķֱ��ʣ��ص����ϲ㣬���ϲ����ȡ��
2004/09/27  3.0         ����      ���ӽ��ն� H.263+/H.264�� ��̬�غɵ�PTֵ���趨�ӿ�
2004/09/27  3.0         ����      ���ӽ��ն˶��� H.263+/H.264 ��̬�غɵ���֧֡��
2004/09/29  3.0         ����      ���� ��mp4 �����غɣ�ĿǰΪֹ���������ش����ʵ���������г���Ϊȱʡֵ(DEFAULT_PACK_QUENE_NUM)
                                    ��ֹ�������ش����ص�ʵ�ʲ�δ֧���ش������²���Ҫ����ʱ����
2004/09/29  3.0         ����      ���ӽ��ն˶���H.224 ��̬�غɵĽ��ջص�֧��
2004/09/29  2.0         ����      ����linux�汾����֧��
2004/09/30  3.0         ����      ���ն����Ӷ��ڼ��������Ľ��ܵ�֧��
2004/10/11  3.0         ����      ���ն����� ���� H.263+ ֡��ߡ��ؼ�֡����Ϣ����ȡ
2004/10/12  3.0         ����      ���ն����� ssrc ��֡��Ϣ�ϱ�
2004/11/04  3.0         ����      ���� ssrc ������ϱ����ð������ssrc
2004/11/08  3.0         ����      ���ն˶���H.263֡��ߡ��ؼ�֡����Ϣ���غ������е���ȡ
2004/11/16  3.0         ����      ���Ӷ��� mpeg2 ���Զ����а���ʽ�������շ���֧��
2004/11/30  3.0         ����      ����AES����ģʽ֧��
2004/12/01  3.0         ����      �޸�AES�ӽ��ܵĳ�ʼ������ֵ����SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
2004/12/06  3.0         ����      ���Ӽ��ǰ����֡������ԣ�������Ӧ��ֲ������߶���
                                    Ӧ��plycom���������ҵ�ǰ����֡H263����֡�໥���
2004/12/21  3.0         ����      �޸�H264��NALU�ֽ���תʱ��bug
2004/12/29  3.0         ����      ���� ����ʱ���ڶ�̬�غɡ����ܵĴ�����
2004/01/25  3.0         ����      �޸Ķ�̬�غɵ���ʵ�غɵ�ӳ�䷽ʽ
                                    �����˽��ܣ�������̬�غ�ֵӦ�����õĶ�̬�غ�ֵһ��
                                    ������������ò�����������̬�غ�ֵ�������˶�̬�غ�ֵ����ֱ���滻Ϊ��ʵ�غ�
2005/01/26  3.6         ����      ���ӽ��ն˶��� G.729 �غɵ�֧��
2005/02/03  3.6         ����      ���� ����ʱ�����غɵĴ�����
                                    ��������Ϊ���ࣺ323��׼�������Զ�����������չ��323�Ǳ���������̬�غ�������
                                    1/���ȴ���323��׼������
                                    2/δ���ö�̬�غ���ֻ����323��׼������
                                      �����˶�̬�غ����Ȼ�ԭΪģ���ʶ���غɣ�������������
2005/02/23  3.6         ����      ���Ӷ���mp4��mp3��������չ�ռ�������ж�
2005/03/04  3.6         ����      �޸Ľ��ն˶���H.263֡��ߡ��ؼ�֡����Ϣ����
2005/06/03  3.6         ����        ����ԭʼ��������ת������
2005/06/04  3.6         ����        ����H261����ͷ������������
******************************************************************************/
/*
�й�֡�ص���˵����2006-05-16 ����
��ʼ����ʱ�򣬷�����֡���壬��С�������֡ͷ��С�����֡��С��
��m_pFrameBufָ��֡��������ʼλ�ã�������֡ͷ��
h261��h263��֡ͷֱ��д��m_pFrameBuf֮ǰ��λ�á���Ӧ��datasize������֡ͷ��
�ϲ���Ҫ֡ͷ�Ļ�����ƫ����Ӧ��λ�á�
h264��ͷ������m_pFrameBuf�У���Ӧ��datasize����֡ͷ��
���ڷ��ͣ�m_pDataָ������ݰ���֡ͷ����Ӧ��С����֡ͷ��С��
*/

#include "kdvnetrcv.h"

#ifdef _LINUX_
#ifndef _IOS_ //for ios complie
    #include <sys/sysinfo.h>
#endif
#endif

#ifdef WIN32
#include <windows.h>
#endif

//h261 ����ֽ�n��00000001111
const u8  byH261End[] =
{
    0x01,0xe0,0x3c,0x07,0x80,0xf0,0x1e,0x03,0xc0,0x78,0x0f
};

extern s32   g_nShowDebugInfo;        //�Ƿ���ʾ���ص�һЩ����״̬��Ϣ
extern BOOL32  g_bInterRcvCallBack;

extern s32   g_nRcvDiscardSpan;        //���ն�ģ�ⶪ��С���Ĳ������

extern s32   g_nRepeatSnd;            //�ش�����

//ͳһ�ṩ�շ������SSRC�Ĳ���
extern u32 GetExclusiveSSRC();

extern s32   g_nh263RcvSp;

extern BOOL32 g_bUseMemPool;

#define NEXTPACKPOS(a)      (((a) < m_nPacketBuffNum-1) ? ((a)+1) : 0)
#define PREVPACKPOS(a)      (((a) > 0) ? ((a)-1) : (m_nPacketBuffNum-1))

extern s32   g_nShowDebugInfo;        //�Ƿ���ʾ���ص�һЩ����״̬��Ϣ
extern BOOL32  g_bInterRcvCallBack;

extern s32   g_nRepeatSnd;            //�ش�����

void RcvCallBack(TRtpPack *pRtpPack, void* pContext);
u16 FrameCallback(TspsFRAMEHDR *ptFrame, KD_PTR pvContext);

typedef enum
{
    FRAME_RELY_NORMAL = 0,
    FRAME_RELY_MARK,
    FRAME_RELY_TIMESTAMP,

    FRAME_RELY_END
} FrameRelayMode;

//-----  ֡�ж�ģʽ�� 0��normal, 1- ����Mark��־, 2-����ʱ���
s32 g_nFrameRelyMode = FRAME_RELY_NORMAL;   //0��normal, 1- rely on Mark, 2-rely on timestamp

//-----  �������˶�̬�غɵ�����£�H263����ǿ�ƴ����H263+��0����ǿ�ƣ�1��ǿ��
BOOL32 g_bForceH263Plus = FALSE;

u16 FrameCallback(TspsFRAMEHDR *ptFrame, void *pvContext);

CListBuffMgr::CListBuffMgr()
{
    m_dwBlockSize       =   0;
    m_dwFreeBlockNum    =   0;
    m_pBuff             =   NULL;
    m_ptFreeBuffList    =   NULL;
    m_dwBuffSize        =   0;
    m_dwTotalNum        =   0;
    m_dwMinFreeBlocks   =   0;
    m_bHad4kH264Stream = false;
}

CListBuffMgr::~CListBuffMgr()
{
    Close();
}

/*=============================================================================
    ������        ��Create
    ����        ���������������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   u32 dwBlockSize  ���С
                   u32 dwBlockNum   ����
    ����ֵ˵���� �ɹ�-TRUE ʧ��-FALSE
=============================================================================*/
BOOL32 CListBuffMgr::Create(u32 dwBlockSize, u32 dwBlockNum)
{
    if (0 == dwBlockSize || 0 == dwBlockNum)
    {
        return FALSE;
    }

    MedianetLog(Api, "CListBuffMgr::Create dwBlockSize=%d dwBlockNum=%d\n", dwBlockSize, dwBlockNum);

    m_dwBlockSize = dwBlockSize;
    u32 dwNewBuffSize = (BLOCK_DATA_OFFSET+dwBlockSize) * dwBlockNum;

    //�ж��Ƿ���Ҫ���·���
    if (dwNewBuffSize > m_dwBuffSize || NULL == m_pBuff)
    {
        //OspPrintf(TRUE, FALSE, "Delete m_pBuff=0x%x size=%d\n", m_pBuff, m_dwBuffSize);
        if (NULL != m_pBuff)
        {
            free(m_pBuff);
            m_pBuff = NULL;
            m_ptFreeBuffList = NULL;
        }

        m_dwBuffSize = dwNewBuffSize;

        //���ش��������ýϸߵ�����£�m_dwBuffSize�����4M��OspAllocMem��֧�ַ������4M���ڴ�
        m_pBuff = (u8 *)malloc(m_dwBuffSize);
        memset(m_pBuff, 0, m_dwBuffSize);
        if (m_pBuff == NULL)
        {
            OspPrintf(TRUE, FALSE, "Medianet Malloc Buff failed, size=%d\n", m_dwBuffSize);
            m_dwBuffSize = 0;
            return FALSE;
        }
    }

    //����������
    u32 i;
    TListBuffBlock* ptTmp;
    ptTmp = (TListBuffBlock*)m_pBuff;
    m_ptFreeBuffList = ptTmp;

    for(i = 1;  i < dwBlockNum; i++)
    {
        ptTmp->m_ptNext = (TListBuffBlock*)((u8*)ptTmp + BLOCK_DATA_OFFSET + dwBlockSize);
        ptTmp = ptTmp->m_ptNext;
    }
    ptTmp->m_ptNext = NULL;

    m_dwTotalNum      = dwBlockNum;
    m_dwFreeBlockNum  = dwBlockNum;
    m_dwMinFreeBlocks = dwBlockNum;

    return TRUE;
}

/*=============================================================================
    ������        ��ReBuild
    ����        ���ؽ����������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   u32 dwBlockSize  ���С
    ����ֵ˵���� �ɹ�-TRUE ʧ��-FALSE
=============================================================================*/
BOOL32 CListBuffMgr::ReBuild(u32 dwBlockSize)
{
    if (0 == dwBlockSize || NULL == m_pBuff)
    {
        return FALSE;
    }

    //ȡʵ�ʵ�ʹ�õ���buff��С
    u32 dwUsedSize = (BLOCK_DATA_OFFSET + m_dwBlockSize) * m_dwTotalNum;

    if (dwUsedSize > m_dwBuffSize)
    {
        OspPrintf(TRUE, FALSE, "CListBuffMgr::ReBuild Error. BuffSize=%d dwUsedSize=%d\n",
                  m_dwBuffSize, dwUsedSize);

        dwUsedSize = m_dwBuffSize;
    }

    //���µĴ�С���зֿ�
    m_dwBlockSize = dwBlockSize;
    m_dwTotalNum  = dwUsedSize / (BLOCK_DATA_OFFSET + dwBlockSize);

    //����������
    u32 i;
    TListBuffBlock* ptTmp;
    ptTmp = (TListBuffBlock*)m_pBuff;
    m_ptFreeBuffList = ptTmp;

    for(i = 1; i < m_dwTotalNum; i++)
    {
        ptTmp->m_ptNext = (TListBuffBlock*)((u8*)ptTmp + BLOCK_DATA_OFFSET + m_dwBlockSize);
        ptTmp = ptTmp->m_ptNext;
    }

    ptTmp->m_ptNext = NULL;

    m_dwFreeBlockNum  = m_dwTotalNum;
    m_dwMinFreeBlocks = m_dwTotalNum;

    MedianetLog(Api, "BUFF Rebuild. Total block num:%d Block size=%d\n",
              m_dwFreeBlockNum, dwBlockSize);

    return TRUE;
}

/*=============================================================================
    ������        ��Close
    ����        ���ͷ����������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   ��
    ����ֵ˵���� ��
=============================================================================*/
void CListBuffMgr::Close()
{
    if (m_pBuff != NULL)
    {
        //OspPrintf(TRUE, FALSE, "Free m_pBuff=0x%x size=%d\n", m_pBuff, m_dwBuffSize);
        free(m_pBuff);
        m_pBuff = NULL;
    }

    m_ptFreeBuffList    =   NULL;
    m_dwBlockSize       =   0;
    m_dwFreeBlockNum    =   0;
    m_dwBuffSize        =   0;
    m_bHad4kH264Stream = false;
}

/*=============================================================================
    ������        ��AllocOneBlock
    ����        ������һ�����������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� ��ָ��
=============================================================================*/
TListBuffBlock* CListBuffMgr::AllocOneBlock()
{
    TListBuffBlock* ptNewBlockList;

    if (NULL == m_ptFreeBuffList)
    {
        return NULL;
    }

    ptNewBlockList = m_ptFreeBuffList;
    m_ptFreeBuffList = m_ptFreeBuffList->m_ptNext; //�ƶ�������ָ��
    ptNewBlockList->m_ptNext = NULL;    //�����������־
    ptNewBlockList->m_dwRealLen = 0;
    
    m_dwFreeBlockNum --;

    //��¼��С���п���
    if (m_dwFreeBlockNum < m_dwMinFreeBlocks)
    {
        m_dwMinFreeBlocks = m_dwFreeBlockNum;
    }

    return ptNewBlockList;
}

/*=============================================================================
    ������        ��WriteBuff
    ����        ��д��buffer

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    u8 *pbyBuff
                    u32 dwBuffSize

    ����ֵ˵���� ��ָ��
=============================================================================*/
TListBuffBlock* CListBuffMgr::WriteBuff(u8 *pbyBuff, u32 dwBuffSize)
{
    u32 nNeedNum;

    if (pbyBuff == NULL || dwBuffSize == 0 || m_ptFreeBuffList == NULL ||
        (m_bHad4kH264Stream?  dwBuffSize > H264_4K_MAX_FRAME_SIZE : dwBuffSize > MAX_VIDEO_FRAME_SIZE))
    {
        return NULL;
    }

    nNeedNum = (dwBuffSize + m_dwBlockSize - 1) / m_dwBlockSize;
    if (m_dwFreeBlockNum < nNeedNum)
    {
        return NULL;
    }

    u32 i, dwOffset = 0;
    TListBuffBlock* ptNewBlockList;
    TListBuffBlock* ptCurBlock;
    ptNewBlockList = m_ptFreeBuffList;
    ptCurBlock = ptNewBlockList;

    for(i = 0; i < nNeedNum; i++)
    {
        if (NULL == ptCurBlock)
        {
            //����FreeBlockNum != m_ptFreeBuffList�еĿ�
            OspPrintf(TRUE, FALSE, "CListBuffMgr: FreeBlockNum error! Free Num=%d But block ptr=NULL\n",
                      m_dwFreeBlockNum-i);
            m_dwFreeBlockNum -= i;
            return NULL;
        }
        if(i < nNeedNum -1)
        {
            ptCurBlock->m_dwRealLen = m_dwBlockSize;
            memcpy(&ptCurBlock->m_BlockData, pbyBuff+dwOffset, m_dwBlockSize);
            dwOffset += ptCurBlock->m_dwRealLen;

            ptCurBlock = ptCurBlock->m_ptNext;
        }
        else
        {
            //���һ��
            ptCurBlock->m_dwRealLen = dwBuffSize - i * m_dwBlockSize;
            memcpy(&ptCurBlock->m_BlockData, pbyBuff + dwOffset, ptCurBlock->m_dwRealLen);

            m_ptFreeBuffList = ptCurBlock->m_ptNext; //�ƶ�������ָ��
            ptCurBlock->m_ptNext = NULL;    //�����������־

            
        }
    }

    m_dwFreeBlockNum -= nNeedNum;

    //��¼��С���п���
    if (m_dwFreeBlockNum < m_dwMinFreeBlocks)
    {
        m_dwMinFreeBlocks = m_dwFreeBlockNum;
    }

    return ptNewBlockList;
}

/*=============================================================================
    ������        ��GetBuffSize
    ����        ����ȡ������С

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    TListBuffBlock* ptBuffBlock

    ����ֵ˵���� ������С
=============================================================================*/
u32 CListBuffMgr::GetBuffSize(TListBuffBlock* ptBuffBlock)
{
    u32 dwLen = 0;
    while(ptBuffBlock != NULL)
    {
        dwLen += ptBuffBlock->m_dwRealLen;
        ptBuffBlock = ptBuffBlock->m_ptNext;
    }
    return dwLen;
}

/*=============================================================================
    ������        ��GetAllocSize
    ����        ����ȡbuffer��С

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� buffer��С
=============================================================================*/
u32 CListBuffMgr::GetAllocSize()
{
    return m_dwBuffSize;
}

/*=============================================================================
    ������        ��GetBlockSize
    ����        ����ȡÿ�����ݴ�С

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �����ݴ�С
=============================================================================*/
u32 CListBuffMgr::GetBlockSize()
{
    return m_dwBlockSize;
}

/*=============================================================================
    ������        ��GetFreeBlocks
    ����        ����ȡ���п���

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� ���п���
=============================================================================*/
u32 CListBuffMgr::GetFreeBlocks()
{
    return m_dwFreeBlockNum;
}

/*=============================================================================
    ������        ��GetMinFreeBlocks
    ����        ����ȡ��С������

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� ��С������
=============================================================================*/
u32 CListBuffMgr::GetMinFreeBlocks()
{
    return m_dwMinFreeBlocks;
}

/*=============================================================================
    ������        ��ReadBuff
    ����        �������������ݶ���ָ��buffer

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵����
=============================================================================*/
u32 CListBuffMgr::ReadBuff(TListBuffBlock* ptBuffBlock, u8* pbyBuf , s32 nSize)
{
    u32 dwLen = 0;
    if (pbyBuf == NULL)
    {
        return 0;
    }

    while(ptBuffBlock != NULL)
    {
        if(ptBuffBlock->m_dwRealLen > (u32)nSize && nSize >= 0)
        {
            OspPrintf(TRUE , FALSE , "CListBuffMgr::ReadBuff pbyBuf Size = %d < %d\n" , nSize , ptBuffBlock->m_dwRealLen);
            return 0;
        }
        memcpy(pbyBuf, &ptBuffBlock->m_BlockData, ptBuffBlock->m_dwRealLen);
        pbyBuf += ptBuffBlock->m_dwRealLen;
        dwLen += ptBuffBlock->m_dwRealLen;
        nSize -= ptBuffBlock->m_dwRealLen;
        ptBuffBlock = ptBuffBlock->m_ptNext;
    }
    return dwLen;
}

/*=============================================================================
    ������        ��ReadBuff2
    ����        �������������ݶ���ָ��buffer

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵����
=============================================================================*/
u32 CListBuffMgr::ReadBuff2(TListBuffBlock* ptBuffBlock, u8* pbyBuf, u32 dwOffset)
{
    u32 dwLen = 0;
    if (pbyBuf == NULL)
    {
        return 0;
    }

    BOOL32 bFirst = TRUE;
    while(ptBuffBlock != NULL)
    {
        if (bFirst)
        {
            memcpy(pbyBuf, (&ptBuffBlock->m_BlockData) + dwOffset,
                ptBuffBlock->m_dwRealLen - dwOffset);
            pbyBuf += (ptBuffBlock->m_dwRealLen - dwOffset);
            dwLen += (ptBuffBlock->m_dwRealLen - dwOffset);
            bFirst = FALSE;
        }
        else
        {
            memcpy(pbyBuf, &ptBuffBlock->m_BlockData, ptBuffBlock->m_dwRealLen);
            pbyBuf += ptBuffBlock->m_dwRealLen;
            dwLen += ptBuffBlock->m_dwRealLen;
        }

        ptBuffBlock = ptBuffBlock->m_ptNext;
    }
    return dwLen;
}

/*=============================================================================
    ������        ��FreeBuff
    ����        ���ͷ�buffer

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵����
=============================================================================*/
u32 CListBuffMgr::FreeBuff(TListBuffBlock* ptBuffBlock)
{

    u32 dwBlockNum;

    if (ptBuffBlock == NULL)
    {
        return 0;
    }

    TListBuffBlock* ptTmp;

    ptTmp = ptBuffBlock;
    dwBlockNum = 1;
    while(ptTmp->m_ptNext != NULL)
    {
        dwBlockNum++;
        ptTmp = ptTmp->m_ptNext;
    }
    ptTmp->m_ptNext     =   m_ptFreeBuffList;
    m_ptFreeBuffList    =   ptBuffBlock;
    m_dwFreeBlockNum    +=  dwBlockNum;

    if (m_dwFreeBlockNum > m_dwTotalNum)
    {
        OspPrintf(TRUE, FALSE, "CListBuffMgr::FreeBuff Total Block num=%d, but Free Block num=%d\n",
                  m_dwFreeBlockNum, m_dwTotalNum);

        m_dwFreeBlockNum = m_dwTotalNum;
    }

    return dwBlockNum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ�����Ա
CKdvNetRcv::CKdvNetRcv ()
{
    m_pcRtp                = NULL;
    m_pcRtcp            = NULL;

    m_dwFrameId            = 0;
    m_dwTimeStamp        = 0;
    m_dwMaxFrameSize    = MAX_VIDEO_FRAME_SIZE;
    m_pFrameCallBackHandler  = NULL;
    m_pRtpCallBackHandler    = NULL;
    m_pContext         = NULL;
    m_pRtpContext      = NULL;
    m_bRcvStart         = FALSE;

    m_pszMaterialBuf    = NULL;
    m_pbyDecryptOutBuf  = NULL;
    m_wMaterialBufLen   = 0;
    m_byRmtActivePT     = 0;
    m_byRealPT          = 0;

    m_bRepeatSend        = FALSE;
    m_bNoPPSSPSStillCallback = FALSE;

    m_bAudio            = FALSE;
    m_nBuffNumTH        = 2;

    memset(&m_tRSParam, 0, sizeof(TRSParam));

    m_hSem              = NULL;

    memset(&m_tRcvStatus, 0, sizeof(m_tRcvStatus));
    memset(&m_tRcvStatistics, 0, sizeof(m_tRcvStatistics));
    memset(&m_tLocalNetParam, 0, sizeof(m_tLocalNetParam));
    memset(&m_tLastInfo, 0, sizeof(m_tLastInfo));
    memset(&m_FrmHdr, 0, sizeof(m_FrmHdr));
    memset(&m_FirstFrmHdr, 0, sizeof(m_FirstFrmHdr));
    m_dwRecvFrmId = 0;

    memset(&m_tAudioHeader, 0, sizeof(m_tAudioHeader));

    memset(&m_tH264Header, 0, sizeof(m_tH264Header));
	memset(&m_tH265Header, 0, sizeof(m_tH265Header));
    memset(m_byRSFrameDistance, 0, sizeof(m_byRSFrameDistance));

    m_tLastInfo.m_byMediaType = MEDIA_TYPE_NULL;
    m_pFrameBuf = NULL;

    m_nCurFrameStartPos = INVALID_PACKET_POS;
    m_atPackets = NULL;
    m_nPacketBuffNum = 0;
    m_nAllocPacketBuffNum = 0;

    m_nSSRCErrCount = 0;
    m_nSNErrCount = 0;

    m_nMP4PackLen = 0;

    m_dwRtpPublicIp = 0;
    m_wRtpPublicPort = 0;
    m_dwTimeStampSample = 0;
    memset(&m_tKdvSpsPpsInfo, 0, sizeof(TKdvSpsPpsInfo));

    m_pRtpUnkownPtHandler = NULL;
    m_pRtpUnkownPtContext = 0;
    m_dwSample = 1;
    m_hTspsRead = NULL;
    m_dwRtpTime = 0;
    m_hSynSem = NULL;

    m_dwPsVideoLastFrameId = 0;
    m_dwPsVideoLastFrameTimeStamp = 0;

    m_pPsFrameCallBackProc = NULL;
    m_bCallBackFrameWithOutPs = TRUE;
    m_pPsFrameCallBackContext = 0;

    // 4k������ʼ��
    m_dwTempRcvBufLen = 0;
    m_bHad4kH264Stream = false;
    m_bisRcv4kStream = false;

    memset(&m_tNatProbeProp, 0, sizeof(TNatProbeProp));
    m_dwNatLastTs = 0;
    m_dwLastFrameTimeStamp = 0;
    m_byOldFrameRate = 25;
    m_bCompFrameByTimeStamp = FALSE;
    m_eTransMode = AACLC_TRANSMODE_MPEG4_GENERIC;
	m_eStreamType = EStreamType_Null;
}

//�������Ա����
CKdvNetRcv::~CKdvNetRcv ()
{
    FreeBuf();
}

/*=============================================================================
    ������        ��FreeBuf
    ����        �����create ���������Ա����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����


    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::FreeBuf()
{
    SAFE_DELETE(m_pcRtp)
    SAFE_DELETE(m_pcRtcp)

    if (NULL != m_hTspsRead)
    {
        TspsReadClose(m_hTspsRead);
        m_hTspsRead = NULL;
    }

    if (NULL != m_atPackets)
    {
        free(m_atPackets);
        m_atPackets = NULL;
        m_nAllocPacketBuffNum = 0;
        m_nPacketBuffNum = 0;
    }

    if( NULL != m_pFrameBuf )
    {
        if(m_bHad4kH264Stream == false)
        {
            u32 dwH26XHeadLen = MAX_H263_HEADER_LEN;
            if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
            {
                dwH26XHeadLen = MAX_H261_HEADER_LEN;
            }
            //ע�������ֽڿռ�������ʶ
            dwH26XHeadLen += sizeof(u32);
            m_pFrameBuf -= dwH26XHeadLen;
        }
        MEDIANET_SAFE_FREE(m_pFrameBuf)
    }

    m_bRcvStart         = FALSE;
    m_dwFrameId            = 0;
    m_dwTimeStamp        = 0;
    m_dwMaxFrameSize    = MAX_VIDEO_FRAME_SIZE;

    m_pFrameCallBackHandler  = NULL;
    m_pRtpCallBackHandler    = NULL;
    m_pContext         = NULL;
    m_pRtpContext      = NULL;

    SAFE_DELETE(m_pszMaterialBuf)
    MEDIANET_SAFE_FREE(m_pbyDecryptOutBuf)
    m_wMaterialBufLen   = 0;
    m_byRmtActivePT     = 0;
    m_byRealPT          = 0;

    m_bRepeatSend        = FALSE;
    memset(&m_tRSParam, 0, sizeof(TRSParam));

    memset(&m_tRcvStatus, 0, sizeof(m_tRcvStatus));
    memset(&m_tRcvStatistics, 0, sizeof(m_tRcvStatistics));
    memset(&m_tLocalNetParam, 0, sizeof(m_tLocalNetParam));
    memset(&m_tLastInfo, 0, sizeof(m_tLastInfo));
    memset(&m_FrmHdr, 0, sizeof(m_FrmHdr));

    MEDIANET_SAFE_FREE(m_FirstFrmHdr.m_pData)
    memset(&m_FirstFrmHdr, 0, sizeof(m_FirstFrmHdr));
    m_dwRecvFrmId = 0;

    memset(&m_tAudioHeader, 0, sizeof(m_tAudioHeader));

    memset(&m_tH264Header, 0, sizeof(m_tH264Header));

    m_tLastInfo.m_byMediaType = MEDIA_TYPE_NULL;

    if(m_hSem !=  NULL)
    {
        OspSemDelete(m_hSem);
        m_hSem=NULL;
    }

    m_nMP4PackLen = 0;

    //�ָ�spsppsinfo ��ֵ
    MEDIANET_SAFE_FREE(m_tKdvSpsPpsInfo.pbyPpsBuf);
    MEDIANET_SAFE_FREE(m_tKdvSpsPpsInfo.pbySpsBuf);
    memset(&m_tKdvSpsPpsInfo, 0, sizeof(TKdvSpsPpsInfo));

    m_dwRtpPublicIp = 0;
    m_wRtpPublicPort = 0;
    m_pRtpUnkownPtHandler = NULL;
    m_pRtpUnkownPtContext = NULL;
    m_dwTimeStampSample = 0;
    if (NULL != m_hSynSem)
    {
        OspSemDelete(m_hSynSem);
        m_hSynSem = NULL;
    }

    m_dwPsVideoLastFrameId = 0;
    m_dwPsVideoLastFrameTimeStamp = 0;

    m_pPsFrameCallBackProc = NULL;
    m_bCallBackFrameWithOutPs = TRUE;
    m_pPsFrameCallBackContext = 0;

    m_dwTempRcvBufLen = 0;
    m_bHad4kH264Stream = false;
    m_bisRcv4kStream = false;

    //free buf
    if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
    {
        MEDIANET_SAFE_FREE(m_tNatProbeProp.tRtpNatProbePack.pbyBuf);
    }

    if(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
    {
        MEDIANET_SAFE_FREE(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf);
    }
    memset(&m_tNatProbeProp, 0, sizeof(TNatProbeProp));
    m_dwNatLastTs = 0;
}

/*=============================================================================
    ������        ��Create
    ����        �������������ģ�飬��ʼ���ڲ�����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwMaxFrameSize       ���֡
                   pFrameCallBackProc   �ϲ�ص�����
                   dwContext             �ص�ʱ���û�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::Create ( u32 dwMaxFrameSize,
                         PFRAMEPROC pFrameCallBackProc,
                         void* pContext,
                         u32 dwSSRC /* = 0*/)
{
    if( dwMaxFrameSize == 0 || dwMaxFrameSize > MAX_VIDEO_FRAME_SIZE ||
        pFrameCallBackProc == NULL )
    {
        return ERROR_NET_RCV_PARAM;
    }

    FreeBuf(); //������ж���

    //��NAT ̽��ṹ������ڴ�
    if(NULL == m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
    {
        MEDIANET_MALLOC(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, MAX_SND_PACK_SIZE);
    }

    if(NULL == m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
    {
        MEDIANET_MALLOC(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, MAX_SND_PACK_SIZE);
    }


    MEDIANET_MALLOC(m_FirstFrmHdr.m_pData, MAX_VIDEO_FRAME_SIZE)
    if(m_FirstFrmHdr.m_pData == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //����RTP����
    m_pcRtp   = new CKdvRtp;
    if(m_pcRtp == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //����RTCP����
    m_pcRtcp  = new CKdvRtcp;
    if(m_pcRtcp == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    m_pFrameCallBackHandler  = pFrameCallBackProc;
    m_pContext              = pContext;
    m_dwMaxFrameSize         = dwMaxFrameSize;

    if(0 == dwSSRC)
    {
        m_dwTimeStamp = GetExclusiveSSRC();
    }
    else
    {
        m_dwTimeStamp = dwSSRC;
    }

    //����RTP����
    u16 wRet = MEDIANET_NO_ERROR;
    wRet = m_pcRtp->Create(m_dwTimeStamp);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //����RTCP����
    wRet = m_pcRtcp->Create(m_dwTimeStamp);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //���ú�RTP��Ӧ��RTCP��
    m_pcRtp->SetRtcp(m_pcRtcp);

    //���ú�RTCP��Ӧ��RTP��
    m_pcRtcp->SetRtp(m_pcRtp);

    //����RTP���ݻص�
    m_pcRtp->SetCallBack(RcvCallBack, (void*)this);
    MEDIANET_MALLOC(m_pbyDecryptOutBuf, MAX_RCV_PACK_SIZE);
    if( NULL == m_pbyDecryptOutBuf )
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //���� H.261/H.263/H.263+ ���Ƕ��ⷵ��rtplistͷ��Ϣ�� ����ÿ֡���ݵ�ͷ��
    u32 dwH26XHeadLen = MAX_H263_HEADER_LEN;
    if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
    {
        dwH26XHeadLen = MAX_H261_HEADER_LEN;
    }
    //ע�������ֽڿռ�������ʶ
    dwH26XHeadLen += sizeof(u32);
    MEDIANET_MALLOC(m_pFrameBuf, dwH26XHeadLen+m_dwMaxFrameSize);
    if( NULL == m_pFrameBuf )
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }
    m_pFrameBuf += dwH26XHeadLen; //m_pFrameBufָ��ʵ��������ǰ���д��ͷ��Ϣ

    if(!OspSemBCreate( &m_hSem))
    {
        m_hSem=NULL;
        FreeBuf();
        return ERROR_CREATE_SEMAPORE;
    }

    //��ʼ��Rtp����
    m_nCurFrameStartPos = INVALID_PACKET_POS;

    m_nBuffFrameNum = 0;
    m_nSSRCErrCount = 0;
    m_nSNErrCount = 0;

    if (dwMaxFrameSize <= 8*1024)  // <= 8K ��Ϊ����Ƶ
    {
        m_bAudio = TRUE;
    }
    else
    {
        m_bAudio = FALSE;
    }

    //����Rtp��������
    wRet =  CreateBuff(DEFAULT_BUFF_TIMELEN);
    if (KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    TRSParam tRSParam;
    memset(&tRSParam, 0, sizeof(TRSParam));

    //�����ش�������Ĭ�ϲ��ش�
    wRet = ResetRSFlag(tRSParam, FALSE);
    if (KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    OspSemBCreate(&m_hSynSem);
       return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��Create
    ����        �������������ģ�飬��ʼ���ڲ�����

    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   dwMaxFrameSize       ���֡
                   pRtpCallBackProc      �ϲ�ص�����
                   dwContext             �ص�ʱ���û�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::Create ( u32 dwMaxFrameSize,
                         PRTPCALLBACK pRtpCallBackProc,
                         void* pContext,
                         u32 dwSSRC /* = 0*/ )
{
    if( dwMaxFrameSize == 0 || dwMaxFrameSize > MAX_VIDEO_FRAME_SIZE ||
        pRtpCallBackProc == NULL )
    {
        return ERROR_NET_RCV_PARAM;
    }

    FreeBuf(); //������ж���

        //��NAT ̽��ṹ������ڴ�
    if(NULL == m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
    {
        MEDIANET_MALLOC(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, MAX_SND_PACK_SIZE);
    }

    if(NULL == m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
    {
        MEDIANET_MALLOC(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, MAX_SND_PACK_SIZE);
    }

    MEDIANET_MALLOC(m_FirstFrmHdr.m_pData, MAX_VIDEO_FRAME_SIZE)
    if(m_FirstFrmHdr.m_pData == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //����RTP����
    m_pcRtp = new CKdvRtp;
    if(m_pcRtp == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //����RTCP����
    m_pcRtcp = new CKdvRtcp;
    if(m_pcRtcp == NULL)
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    m_pRtpCallBackHandler  = pRtpCallBackProc;
    m_pRtpContext         = pContext;
    m_dwMaxFrameSize       = dwMaxFrameSize;

    if(0 == dwSSRC)
    {
        m_dwTimeStamp = GetExclusiveSSRC();
    }
    else
    {
        m_dwTimeStamp = dwSSRC;
    }

    //����RTP����
    u16 wRet = MEDIANET_NO_ERROR;
    wRet = m_pcRtp->Create(m_dwTimeStamp);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //����RTCP����
    wRet = m_pcRtcp->Create(m_dwTimeStamp);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //���ú�RTP��Ӧ��RTCP��
    m_pcRtp->SetRtcp(m_pcRtcp);

    //���ú�RTCP��Ӧ��RTP��
    m_pcRtcp->SetRtp(m_pcRtp);

    //����RTP���ݻص�
    m_pcRtp->SetCallBack(RcvCallBack, (void*)this);

    MEDIANET_MALLOC(m_pbyDecryptOutBuf, MAX_RCV_PACK_SIZE);
    if( NULL == m_pbyDecryptOutBuf )
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }

    //���� H.261/H.263/H.263+ ���Ƕ��ⷵ��rtplistͷ��Ϣ�� ����ÿ֡���ݵ�ͷ��
    u32 dwH26XHeadLen = MAX_H263_HEADER_LEN;
    if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
    {
        dwH26XHeadLen = MAX_H261_HEADER_LEN;
    }
    //ע�������ֽڿռ�������ʶ
    dwH26XHeadLen += sizeof(u32);
    MEDIANET_MALLOC(m_pFrameBuf, dwH26XHeadLen+m_dwMaxFrameSize);
    if( NULL == m_pFrameBuf )
    {
        FreeBuf();
        return ERROR_NET_RCV_MEMORY;
    }
    m_pFrameBuf += dwH26XHeadLen;

    if(!OspSemBCreate( &m_hSem))
    {
        m_hSem=NULL;
        FreeBuf();
        return ERROR_CREATE_SEMAPORE;
    }

    m_nCurFrameStartPos = INVALID_PACKET_POS;

    m_nBuffFrameNum = 0;
    m_nSSRCErrCount = 0;
    m_nSNErrCount = 0;

    if (dwMaxFrameSize <= 8*1024)  // <= 8K ��Ϊ����Ƶ
    {
        m_bAudio = TRUE;
    }
    else
    {
        m_bAudio = FALSE;
    }

    //����Rtp��������
    wRet = CreateBuff(DEFAULT_BUFF_TIMELEN);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    //�����ش�������Ĭ�ϲ��ش�
    TRSParam tRSParam;
    memset(&tRSParam, 0, sizeof(TRSParam));

    wRet = ResetRSFlag(tRSParam, FALSE);

    if (KDVFAILED(wRet))
    {
        FreeBuf();
        return wRet;
    }

    m_dwBufPackNum = 0;

       return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetNetRcvLocalParam
    ����        �����ý��յ�ַ����(���еײ��׽��ӵĴ������󶨶˿ڵȶ���)
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tLocalNetParam      ���ص�ַ����,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::SetNetRcvLocalParam (TLocalNetParam tLocalNetParam, u32 dwFlag, void* pRegFunc, void* pUnregFunc)
{
    if(m_pcRtp == NULL|| m_pcRtcp == NULL)
    {
        if(NULL == m_pcRtp && NULL != m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtp == NULL \n");
        }
        if(NULL != m_pcRtp && NULL == m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtcp == NULL \n");
        }
        if(NULL == m_pcRtp && NULL == m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtcp == NULL && m_pcRtp == NULL\n");
        }
        return ERROR_RCV_NO_CREATE;
    }

    //��ͬ�ĵ�ַ��������
    if( (m_tLocalNetParam.m_tLocalNet.m_wRTCPPort == tLocalNetParam.m_tLocalNet.m_wRTCPPort) &&
        (m_tLocalNetParam.m_tLocalNet.m_wRTPPort == tLocalNetParam.m_tLocalNet.m_wRTPPort) &&
        (m_tLocalNetParam.m_tLocalNet.m_dwRTCPAddr == tLocalNetParam.m_tLocalNet.m_dwRTCPAddr) &&
        (m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr == tLocalNetParam.m_tLocalNet.m_dwRTPAddr) &&
        (m_tLocalNetParam.m_dwRtcpBackAddr == tLocalNetParam.m_dwRtcpBackAddr) &&
        (m_tLocalNetParam.m_wRtcpBackPort == tLocalNetParam.m_wRtcpBackPort) &&
        (m_tLocalNetParam.m_tLocalNet.m_dwRtpUserDataLen == tLocalNetParam.m_tLocalNet.m_dwRtpUserDataLen) &&
        (m_tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen == tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen))
    {
        return MEDIANET_NO_ERROR;
    }

    m_tLocalNetParam        = tLocalNetParam;
    m_tRcvStatus.m_tRcvAddr = tLocalNetParam;
    m_tRcvStatus.m_dwRcvFlag = dwFlag;
    m_tRcvStatus.m_pRegFunc = pRegFunc;
    m_tRcvStatus.m_pUnregFunc = pUnregFunc;

    m_dwBufPackNum = 0;

    //����RTP���ص�ַ
    u16 wRet = MEDIANET_NO_ERROR;
    wRet = m_pcRtp->SetLocalAddr( tLocalNetParam.m_tLocalNet.m_dwRTPAddr,
                                  tLocalNetParam.m_tLocalNet.m_wRTPPort, TRUE,
                                  tLocalNetParam.m_tLocalNet.m_dwRtpUserDataLen,
                                  tLocalNetParam.m_tLocalNet.m_abyRtpUserData, dwFlag, pRegFunc, pUnregFunc);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //����RTCP���ص�ַ
    wRet = m_pcRtcp->SetLocalAddr( tLocalNetParam.m_tLocalNet.m_dwRTCPAddr,
                                   tLocalNetParam.m_tLocalNet.m_wRTCPPort,
                                   tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen,
                                   tLocalNetParam.m_tLocalNet.m_abyRtcpUserData, dwFlag, pRegFunc, pUnregFunc);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    //����RTCP������ַ
    TRemoteAddr tRemoteAddr;
    tRemoteAddr.m_byNum               = 1;
    tRemoteAddr.m_tAddr[0].m_dwIP  = tLocalNetParam.m_dwRtcpBackAddr;
    tRemoteAddr.m_tAddr[0].m_wPort = tLocalNetParam.m_wRtcpBackPort;
    if (tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen >= 0 && tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen <= MAX_USERDATA_LEN)
    {
        tRemoteAddr.m_tAddr[0].m_dwUserDataLen = tLocalNetParam.m_tLocalNet.m_dwRtcpUserDataLen;
        memcpy(tRemoteAddr.m_tAddr[0].m_abyUserData, tLocalNetParam.m_tLocalNet.m_abyRtcpUserData,
               tRemoteAddr.m_tAddr[0].m_dwUserDataLen);
    }

    wRet = m_pcRtcp->SetRemoteAddr(tRemoteAddr);
    if(KDVFAILED(wRet))
    {
        return wRet;
    }

    m_tRcvStatus.m_tRcvAddr = tLocalNetParam;

    MEDIANET_SEM_TAKE(m_hSem);

    m_tLastInfo.m_byMediaType = MEDIA_TYPE_NULL;

    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

//�ش���natʱ�����ñ�����rtp���ն˿ڶ�Ӧ�Ĺ�����ַ,Ŀ��Ϊʹ�ش�ʱ���ù㲥
u16 CKdvNetRcv::SetRtpPublicAddr(u32 dwRtpPublicIp, u16 wRtpPublicPort)
{
    MEDIANET_SEM_TAKE(m_hSem);

    m_dwRtpPublicIp = dwRtpPublicIp;
    m_wRtpPublicPort = wRtpPublicPort;

    MEDIANET_SEM_GIVE(m_hSem);
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SetDecryptKey
    ����        �����ý���key�ִ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  pszKeyBuf      ����key�ִ�����ָ��
                  wKeySize       ����key�ִ����峤��
                  byDecryptMode  ����ģʽ Aes ���� Des

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::SetDecryptKey(s8 *pszKeyBuf, u16 wKeySize, u8 byDecryptMode)
{
    MEDIANET_SEM_TAKE(m_hSem);

    if(NULL == pszKeyBuf)
    {
        //ȡ������
        SAFE_DELETE(m_pszMaterialBuf)
        m_wMaterialBufLen = 0;
    }
    else
    {
        if( (AES_ENCRYPT_MODE != byDecryptMode) &&
            (DES_ENCRYPT_MODE != byDecryptMode) )
        {
            MEDIANET_SEM_GIVE(m_hSem);
            return ERROR_SET_DECRYPTKEY;
        }

        if( AES_ENCRYPT_MODE == byDecryptMode )
        {
            //�����µĽ��ܼ�ֵ
            if( (AES_ENCRYPT_KEY_SIZE_MODE_A != wKeySize) &&
                (AES_ENCRYPT_KEY_SIZE_MODE_B != wKeySize) &&
                (AES_ENCRYPT_KEY_SIZE_MODE_C != wKeySize) )
            {
                MEDIANET_SEM_GIVE(m_hSem);
                return ERROR_SET_DECRYPTKEY;
            }

            SAFE_DELETE(m_pszMaterialBuf)
            m_wMaterialBufLen = 0;

            m_pszMaterialBuf = new s8[wKeySize+4];
            if(NULL == m_pszMaterialBuf)
            {
                MEDIANET_SEM_GIVE(m_hSem);
                return ERROR_SET_DECRYPTKEY;
            }
            memset(m_pszMaterialBuf, 0, wKeySize+4);
            memcpy(m_pszMaterialBuf, pszKeyBuf, wKeySize);

            /// m_byAesMode = MODE_CBC;
        }

        if( DES_ENCRYPT_MODE == byDecryptMode )
        {
            //�����µĽ��ܼ�ֵ
            if( DES_ENCRYPT_KEY_SIZE != wKeySize )
            {
                MEDIANET_SEM_GIVE(m_hSem);
                return ERROR_SET_DECRYPTKEY;
            }

            SAFE_DELETE(m_pszMaterialBuf)
            m_wMaterialBufLen = 0;

            m_pszMaterialBuf = new s8[wKeySize+4];
            if(NULL == m_pszMaterialBuf)
            {
                MEDIANET_SEM_GIVE(m_hSem);
                return ERROR_SET_DECRYPTKEY;
            }
            memset(m_pszMaterialBuf, 0, wKeySize+4);
            memcpy(m_pszMaterialBuf, pszKeyBuf, wKeySize);

            /// m_emDesMode = qfDES_cbc;
        }

        m_byDecryptMode   = byDecryptMode;
        m_wMaterialBufLen = wKeySize;
    }

    MEDIANET_SEM_GIVE(m_hSem);
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        RemoveNetRcvLocalParam
    ����        ���Ƴ����յ�ַ����(���еײ��׽��ӵ�ɾ�����ͷŶ˿ڵȶ���)
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::RemoveNetRcvLocalParam()
{
    if(m_pcRtp == NULL|| m_pcRtcp == NULL)
    {
        if(NULL == m_pcRtp && NULL != m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtp == NULL \n");
        }
        if(NULL != m_pcRtp && NULL == m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtcp == NULL \n");
        }
        if(NULL == m_pcRtp && NULL == m_pcRtcp)
        {
            MedianetLog(Api, "\n RemoveNetRcvLocalParam Error--m_pcRtcp == NULL && m_pcRtp == NULL\n");
        }

        return ERROR_RCV_NO_CREATE;
    }

    //��ֹͣ����
    BOOL32 bRcvStart = m_bRcvStart;
    m_bRcvStart = FALSE;

    m_pcRtp->RemoveLocalAddr();

    m_pcRtcp->RemoveLocalAddr();

    TRemoteAddr tRemoteAddr;
    memset(&tRemoteAddr, 0, sizeof(tRemoteAddr));
    m_pcRtcp->SetRemoteAddr(tRemoteAddr);

    //���ʱ��rtp��Ӧ�Ĺ�����ַҲ������Ч
    m_dwRtpPublicIp = 0;
    m_wRtpPublicPort = 0;

    memset(&m_tLocalNetParam, 0, sizeof(m_tLocalNetParam));
    m_tRcvStatus.m_tRcvAddr = m_tLocalNetParam;
    m_tRcvStatus.m_tRcvAddr = m_tLocalNetParam;

    //�ָ�ԭ״
    m_bRcvStart = bRcvStart;

    return MEDIANET_NO_ERROR;
}

u16 CKdvNetRcv::SetTimestampSample(u32 dwSample)
{
   // 1-999 considered as invalid , 0 treated as restore default .
    if(dwSample < 1000 && dwSample > 0)
    {
        OspPrintf(1,0,"[CKdvNetRcv::SetTimestampSample] Error, 0 < dwSample(%d) < 1000 \n", dwSample);
        return ERROR_NET_RCV_PARAM;
    }
    m_dwTimeStampSample = dwSample;

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        : SetActivePT
    ����        ������ ��̬�غɵ� Playloadֵ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  byRmtActivePT ���յ��Ķ�̬�غɵ�Playloadֵ, �ɶԺ�ʱ�Է���֪, 0-��ʾ��� ���˶�̬�غɱ��
                  byRealPT      �ö�̬�غ�ʵ�ʴ���ĵ�Playload���ͣ���ͬ�����Ƿ���ʱ��PTԼ��

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::SetActivePT( u8 byRmtActivePT, u8 byRealPT )
{
    m_byRmtActivePT = byRmtActivePT;
    m_byRealPT      = byRealPT;

    return MEDIANET_NO_ERROR;
}


/*=============================================================================
    ������        : ResetCAFlag
    ����        �����ý��ն˶��� (mp3/gxx) ��Ƶ��ϵ���Ƿ���������������Ŀ���,
                  �رպ󣬽�������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::ResetCAFlag(BOOL32 bConfuedAdjust)
{
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        : ResetRSFlag
    ����        �����ý��ն˶���mpeg4����H.264���õ��ش�����Ŀ���,
                  �رպ󣬽��������ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::ResetRSFlag(TRSParam tRSParam, BOOL32 bRepeatSnd)
{
    MEDIANET_SEM_TAKE(m_hSem);

    m_bRepeatSend = bRepeatSnd;

    //�����ش����������̶�������
    /*
    m_nBuffNumTH = 2;

    //������Ƶ�����֡������Ƶ֡��*4/3
    if (m_bAudio)
    {
        m_nBuffNumTH = m_nBuffNumTH*4/3;
    }
    */

    m_tRSParam.m_wFirstTimeSpan = VIDEO_TIME_SPAN *
        ((tRSParam.m_wFirstTimeSpan + VIDEO_TIME_SPAN - 1)/(VIDEO_TIME_SPAN));

    if (m_tRSParam.m_wFirstTimeSpan == 0)
    {
        m_tRSParam.m_wFirstTimeSpan = VIDEO_TIME_SPAN;
    }

    m_tRSParam.m_wSecondTimeSpan = VIDEO_TIME_SPAN *
        ((tRSParam.m_wSecondTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
    if (m_tRSParam.m_wSecondTimeSpan <= m_tRSParam.m_wFirstTimeSpan)
    {
        m_tRSParam.m_wSecondTimeSpan = m_tRSParam.m_wFirstTimeSpan + VIDEO_TIME_SPAN;
    }

    m_tRSParam.m_wThirdTimeSpan  = VIDEO_TIME_SPAN *
        ((tRSParam.m_wThirdTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
    if (m_tRSParam.m_wThirdTimeSpan <= m_tRSParam.m_wSecondTimeSpan)
    {
        m_tRSParam.m_wThirdTimeSpan = m_tRSParam.m_wSecondTimeSpan + VIDEO_TIME_SPAN;
    }

    m_tRSParam.m_wRejectTimeSpan = VIDEO_TIME_SPAN *
        ((tRSParam.m_wRejectTimeSpan + VIDEO_TIME_SPAN - 1)/VIDEO_TIME_SPAN);
    if (m_tRSParam.m_wRejectTimeSpan <= m_tRSParam.m_wThirdTimeSpan)
    {
        m_tRSParam.m_wRejectTimeSpan = m_tRSParam.m_wThirdTimeSpan + VIDEO_TIME_SPAN;
    }

    m_byRSFrameDistance[0] = m_tRSParam.m_wFirstTimeSpan / VIDEO_TIME_SPAN;
    m_byRSFrameDistance[1] = m_tRSParam.m_wSecondTimeSpan / VIDEO_TIME_SPAN;
    m_byRSFrameDistance[2] = m_tRSParam.m_wThirdTimeSpan / VIDEO_TIME_SPAN;
    m_byRSFrameDistance[MAX_RESEND_QUEST_TIMES] = m_tRSParam.m_wRejectTimeSpan / VIDEO_TIME_SPAN;

    s32 nBuffTimeLen;

    if (bRepeatSnd)
    {
        nBuffTimeLen = m_tRSParam.m_wRejectTimeSpan; //����ʱ��

        if (nBuffTimeLen < DEFAULT_BUFF_TIMELEN)
        {
            nBuffTimeLen = DEFAULT_BUFF_TIMELEN;
        }
        else if (nBuffTimeLen > MAX_BUFF_TIMELEN)
        {
            nBuffTimeLen = MAX_BUFF_TIMELEN;
        }
    }
    else
    {
        nBuffTimeLen = DEFAULT_BUFF_TIMELEN;
    }

    m_dwTempRcvBufLen = nBuffTimeLen;
    u16 wRet = MEDIANET_NO_ERROR;

    //���ݻ���ʱ������buff
    wRet = CreateBuff(nBuffTimeLen);
    if(KDVFAILED(wRet))
    {
        FreeBuf();
        MEDIANET_SEM_GIVE(m_hSem);
        return wRet;
    }

    ResetRSPos();

    MEDIANET_SEM_GIVE(m_hSem);

    return wRet;
}

//������ʱ�����㲢���仺��
u16 CKdvNetRcv::CreateBuff(s32 nBuffTimeLen)
{
    //���㻺�����Ŀ
    s32 nPacketBuffNum;
    if (m_bAudio)
    {
        //����ʱ��20ms����
        nPacketBuffNum = nBuffTimeLen / 20;          //�ܻ������
    }
    else
    {
        //�ܻ������,��ÿ֡52����
        //�ڸ�����52����Ȼ��������������Ŀ���Ǳ�֤�ش�������1�Լ�  ���ش�ʱ
        //Ĭ�ϻ���ʱ��200ms������£������Ļ������Ϊ512
        //��ֻ��256��������261k�Ĺؼ�֡���ս�������
        // ԭ��m_dwMaxFrameSize / 10000 ��ֵΪ52. 4k��Ŀ��Ϊ��
        nPacketBuffNum = nBuffTimeLen * (m_dwMaxFrameSize / 10000) / VIDEO_TIME_SPAN;
    }

    //������Ҫ����Ϊ2����
    s32 i;
    // 8 means  max bufnum is  1024*32, MAX_BUFF_TIMELEN.warning, this value can not to be bigger.
    for (i = 0; i < 8; i++)
    {
        if (nPacketBuffNum <= (MAX_PACKET_BUFF_NUM << i))
        {
            break;
        }
    }
    nPacketBuffNum = MAX_PACKET_BUFF_NUM << i;

    if (nPacketBuffNum > m_nAllocPacketBuffNum)
    {
        //��Ҫ���¹���
        if (NULL != m_atPackets)
        {
            delete [] m_atPackets;
        }
        m_atPackets = (TPacketInfo*)malloc(nPacketBuffNum * sizeof(TPacketInfo));
        if (NULL == m_atPackets)
        {
            return ERROR_NET_RCV_MEMORY;
        }
        m_nAllocPacketBuffNum = nPacketBuffNum;
    }

    m_nPacketBuffNum = nPacketBuffNum;

    //��ʼ��Rtp����
    m_nCurFrameStartPos = INVALID_PACKET_POS;
    m_nBuffFrameNum = 0;

    for(i = 0; i < m_nPacketBuffNum; i++)
    {
        m_atPackets[i].m_bUsed = FALSE;
        m_atPackets[i].m_bMark = FALSE;
        m_atPackets[i].m_ptDataBuff = NULL;
    }

    //����������С
    s32 nPackBuffSize;
    s32 nBlockSize;

    if (m_bAudio)
    {
        nBlockSize = BUFF_BLOCK_LENGTH;
        nPackBuffSize = m_nPacketBuffNum * nBlockSize; //��Ƶ�����С����������
    }
    else
    {
        //��Ƶ��
        nBlockSize = BUFF_BLOCK_LENGTH * 2;
        nPackBuffSize = (MAX_SND_PACK_SIZE * m_nPacketBuffNum);

        /*
        //��Ƶ ��ÿ֡���128K����, MAX_FRAME_SIZEΪ256K
        nPackBuffSize = (nBuffTimeLen * 128 * 1024) / VIDEO_TIME_SPAN; //�ܻ����С
        nBlockSize = BUFF_BLOCK_LENGTH * 2; //��ƵС��buffÿ��512�ֽ�
        */
    }

    //if (nPackBuffSize != (s32)m_cListBuffMgr.GetAllocSize())
    {
        //����Rtp��������
        if (!m_cListBuffMgr.Create(nBlockSize, nPackBuffSize / nBlockSize))
        {
            return ERROR_NET_RCV_MEMORY;
        }
    }

    //�����Buff��صĲ���
    m_tLastInfo.m_byMediaType = MEDIA_TYPE_NULL;
    m_dwBufPackNum = 0;

    return MEDIANET_NO_ERROR;
}

u16 CKdvNetRcv::DealNatProbePack(u32 dwNowTs)
{
    if(NULL == m_pcRtp || NULL == m_hSem || NULL == m_pcRtcp)
    {
        return ERROR_NET_RCV_MEMORY;
    }
    u16 wRet = ERROR_NET_RCV_MEMORY;

    u32 dwSpan = (dwNowTs - m_dwNatLastTs)*1000 / OspClkRateGet();

    MedianetLog(Api, "[medianet: DealNatProbePack ] dwNowTs=%lu  m_dwNatLastTs=%lu dwspan=%lu interval=%lu \n",
        dwNowTs, m_dwNatLastTs, dwSpan, m_tNatProbeProp.dwInterval);

    if(dwNowTs != 0 && (dwSpan < m_tNatProbeProp.dwInterval || m_tNatProbeProp.dwInterval == 0))
    {
        //ʱ��δ������dwnowts ��Ϊ0��ֱ��return��
        return MEDIANET_NO_ERROR;
    }

     OspSemTake((SEMHANDLE)m_hSem);

     // ���� rtp ��rtcp �������
    if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf == NULL || m_tNatProbeProp.tRtpNatProbePack.wBufLen == 0 ||
        m_tNatProbeProp.tRtcpNatProbePack.pbyBuf == NULL || m_tNatProbeProp.tRtcpNatProbePack.wBufLen == 0)
    {
        OspSemGive((SEMHANDLE)m_hSem);
        return ERROR_NET_RCV_NOCREATE;
    }

    if(m_pcRtp->SendUserDefinedBuff(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, m_tNatProbeProp.tRtpNatProbePack.wBufLen, m_tNatProbeProp.tRtpNatProbePack.m_dwPeerAddr, m_tNatProbeProp.tRtpNatProbePack.m_wPeerPort) == MEDIANET_NO_ERROR
        &&     m_pcRtcp->SendUserDefinedBuff(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, m_tNatProbeProp.tRtcpNatProbePack.wBufLen, m_tNatProbeProp.tRtcpNatProbePack.m_dwPeerAddr, m_tNatProbeProp.tRtcpNatProbePack.m_wPeerPort) == MEDIANET_NO_ERROR)
    {
        wRet = MEDIANET_NO_ERROR;
    }

    //��natlastts ��ֵ��
    m_dwNatLastTs = OspTickGet();
    OspSemGive((SEMHANDLE)m_hSem);

    return wRet;
}

u16 CKdvNetRcv::SetNatProbeProp(TNatProbeProp *ptNatProbeProp)
{
    if(NULL == m_hSem)
    {
        return ERROR_NET_RCV_MEMORY;
    }

    OspSemTake((SEMHANDLE)m_hSem);

    if(ptNatProbeProp->tRtpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtpNatProbePack.pbyBuf == NULL ||
        ptNatProbeProp->tRtcpNatProbePack.wBufLen > MAX_SND_PACK_SIZE || ptNatProbeProp->tRtcpNatProbePack.pbyBuf == NULL)
    {
        OspSemGive((SEMHANDLE)m_hSem);
        return ERROR_NET_RCV_PARAM; //���ò�������
    }
    else
    {
        if(m_tNatProbeProp.tRtpNatProbePack.pbyBuf)
        {
            memcpy(m_tNatProbeProp.tRtpNatProbePack.pbyBuf, ptNatProbeProp->tRtpNatProbePack.pbyBuf, ptNatProbeProp->tRtpNatProbePack.wBufLen);
            m_tNatProbeProp.tRtpNatProbePack.wBufLen = ptNatProbeProp->tRtpNatProbePack.wBufLen;
        }

        if(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf)
        {
            memcpy(m_tNatProbeProp.tRtcpNatProbePack.pbyBuf, ptNatProbeProp->tRtcpNatProbePack.pbyBuf, ptNatProbeProp->tRtcpNatProbePack.wBufLen);
            m_tNatProbeProp.tRtcpNatProbePack.wBufLen = ptNatProbeProp->tRtcpNatProbePack.wBufLen;
        }

        m_tNatProbeProp.dwInterval = (ptNatProbeProp->dwInterval) * 1000; //ת��Ϊms �洢 , 0s �򲻷���

        //record rtp peer addr and port
        m_tNatProbeProp.tRtpNatProbePack.m_dwPeerAddr = ptNatProbeProp->tRtpNatProbePack.m_dwPeerAddr;
        m_tNatProbeProp.tRtpNatProbePack.m_wPeerPort = ptNatProbeProp->tRtpNatProbePack.m_wPeerPort;

        //record rtcp peer addr and port
        m_tNatProbeProp.tRtcpNatProbePack.m_dwPeerAddr = ptNatProbeProp->tRtcpNatProbePack.m_dwPeerAddr;
        m_tNatProbeProp.tRtcpNatProbePack.m_wPeerPort = ptNatProbeProp->tRtcpNatProbePack.m_wPeerPort;
    }

    OspSemGive((SEMHANDLE)m_hSem);

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        : ResetRtpCalllback
    ����        ������RTP�ص��ӿ���Ϣ
                  �رպ󣬽�������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::ResetRtpCalllback(PRTPCALLBACK pRtpCallBackProc, void* pContext, TRtpCallbackType emCallbackType)
{
    MEDIANET_SEM_TAKE(m_hSem);

    if( NORMAL_RTP_CALL_BACK == emCallbackType )
    {
        m_pRtpCallBackHandler            = pRtpCallBackProc;
        m_pRtpContext                     = pContext;
    }
    else if( UNKOWN_PT_RTP_CALL_BACK == emCallbackType )
    {
        m_pRtpUnkownPtHandler        = pRtpCallBackProc;
        m_pRtpUnkownPtContext        = pContext;
    }

    MEDIANET_SEM_GIVE(m_hSem);

    return MEDIANET_NO_ERROR;
}

//���ps֡�ص��ӿڣ��������Ƿ�ص�ȥpsͷ��֡ ��־λ��
u16 CKdvNetRcv::AddPsFrameCallBack(PFRAMEPROC pFrameCallBackProc, BOOL32 bCallBackFrameWithOutPs, void* pContext)
{
    u16 dwRet = MEDIANET_NO_ERROR;
    MEDIANET_SEM_TAKE(m_hSem);

    if(pFrameCallBackProc)
    {
        m_pPsFrameCallBackProc = pFrameCallBackProc;
        m_bCallBackFrameWithOutPs = bCallBackFrameWithOutPs;
        m_pPsFrameCallBackContext = pContext;

        MedianetLog(Api, "[Medianet: AddPsFrameCallBack] m_pPsFrameCallBackProc=%8x,m_bCallBackFrameWithOutPs=%d,m_pPsFrameCallBackContext=%x \n",
            m_pPsFrameCallBackProc, m_bCallBackFrameWithOutPs, m_pPsFrameCallBackContext);
    }
    else
    {
        MedianetLog(Api, "[Medianet ::AddPsFrameCallBack]Error  pFrameCallBackProc == NULL\n");
        dwRet = ERROR_SET_USERDATA;
    }

    MEDIANET_SEM_GIVE(m_hSem);

    return dwRet;
}

u16 CKdvNetRcv::SetIs4k(BOOL32 bis4k)
{
    u16 dwRet = MEDIANET_NO_ERROR;
    MEDIANET_SEM_TAKE(m_hSem);

    m_bisRcv4kStream = bis4k;

    MEDIANET_SEM_GIVE(m_hSem);
    return dwRet;
}

u16 CKdvNetRcv::InputRtpPack(u8 * pRtpBuf, u32 dwRtpBuf)
{
    if(NULL == m_pcRtp || NULL == m_hSem)
    {
        return ERROR_NET_RCV_MEMORY;
    }

    u16 wRet = ERROR_NET_RCV_MEMORY;
    if(NULL != m_pcRtp)
    {
        wRet = m_pcRtp->InputRtpPack(pRtpBuf, dwRtpBuf);
    }
    return wRet;
}
u16 CKdvNetRcv::SetRtcpInfoCallback(PRTCPINFOCALLBACK pRtcpInfoCallback, void * pContext)
{
    if(NULL == m_pcRtcp || NULL == m_hSem)
    {
        return ERROR_NET_RCV_MEMORY;
     }

    u16 wRet = ERROR_NET_RCV_MEMORY;
    MEDIANET_SEM_TAKE((SEMHANDLE)m_hSem);
    if(NULL != m_pcRtcp)
    {
        wRet = m_pcRtcp->SetRtcpInfoCallback(pRtcpInfoCallback, pContext);
    }
    MEDIANET_SEM_GIVE((SEMHANDLE)m_hSem);

    return wRet;
}

/*=============================================================================
    ������        ��StartRcv
    ����        ����ʼ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tLocalNetParam      ���ص�ַ����,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::StartRcv()
{
        m_bRcvStart = TRUE;
    m_tRcvStatus.m_bRcvStart = TRUE;
    m_dwRecvFrmId = 0;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��StopRcv
    ����        ��ֹͣ���ս���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tLocalNetParam      ���ص�ַ����,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::StopRcv()
{
    m_bRcvStart = FALSE;
    m_tRcvStatus.m_bRcvStart = FALSE;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��DealRtcpTimer
    ����        ��rtcp��ʱrtcp���ϱ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::DealRtcpTimer()
{
    //�Ƿ񴴽�
    if( NULL == m_pcRtp || NULL == m_pcRtcp )
    {
        return ERROR_RCV_NO_CREATE;
    }

    return m_pcRtcp->DealTimer();
}

/*=============================================================================
    ������        ��GetStatus
    ����        ��ֹͣ���ս���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvRcvStatus     Ҫ���صĽ���״̬�ṹ,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::GetStatus(TKdvRcvStatus &tKdvRcvStatus)
{
    tKdvRcvStatus = m_tRcvStatus;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetStatistics
    ����        ��ֹͣ���ս���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvRcvStatistics     Ҫ���صĽ���ͳ�ƽṹ,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::GetStatistics(TKdvRcvStatistics &tKdvRcvStatistics)
{
    tKdvRcvStatistics = m_tRcvStatistics;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetMaxFrameSize
    ����        ����ȡ���ն�������֡����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvRcvStatistics     Ҫ���صĽ���ͳ�ƽṹ,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::GetMaxFrameSize(u32 &dwMaxFrameSize)
{
    dwMaxFrameSize = m_dwMaxFrameSize;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetLocalMediaType
    ����        ����ȡ���ն��� ��ǰ���յ�����������
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tKdvRcvStatistics     Ҫ���صĽ���ͳ�ƽṹ,�μ�����

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::GetLocalMediaType(u8 &byMediaType)
{
    byMediaType = m_tLastInfo.m_byMediaType;
    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��GetAdvancedRcvInfo
    ����        ����ѯ���ն˸߼����ò������ش��ȣ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   tAdvancedRcvInfo  Ҫ���صķ���״̬���μ��ṹ���塣

    ����ֵ˵���� �μ������붨��
=============================================================================*/
u16 CKdvNetRcv::GetAdvancedRcvInfo(TAdvancedRcvInfo &tAdvancedRcvInfo)
{
    //tAdvancedRcvInfo.m_bConfuedAdjust = m_bConfuedAdjust;
    tAdvancedRcvInfo.m_bRepeatSend    = m_bRepeatSend;
    tAdvancedRcvInfo.m_tRSParam       = m_tRSParam;
    tAdvancedRcvInfo.m_byRmtActivePT  = m_byRmtActivePT;
    tAdvancedRcvInfo.m_byRealPT       = m_byRealPT;
	tAdvancedRcvInfo.m_eStreamType    = m_eStreamType;
    if(NULL != m_pszMaterialBuf)
    {
        tAdvancedRcvInfo.m_bDecryption   = TRUE;
        tAdvancedRcvInfo.m_byDecryptMode = m_byDecryptMode;
        tAdvancedRcvInfo.m_wKeySize      = m_wMaterialBufLen;
        memcpy(tAdvancedRcvInfo.m_szKeyBuf, m_pszMaterialBuf, tAdvancedRcvInfo.m_wKeySize);
    }
    else
    {
        tAdvancedRcvInfo.m_bDecryption   = FALSE;
        tAdvancedRcvInfo.m_byDecryptMode = 0;
        tAdvancedRcvInfo.m_wKeySize      = 0;
    }

    return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��DecryptRtpData
    ����        ���ж��յ���RTP�������ʲ�������Ӧ�Ľ��ܴ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - �ɹ�
=============================================================================*/
BOOL32 CKdvNetRcv::DecryptRtpData(TRtpPack *pRtpPack)
{
    BOOL32 bRet = TRUE;

    MEDIANET_SEM_TAKE(m_hSem);

    //a. �ж��Ƿ������˽���
    if( NULL == m_pszMaterialBuf )
    {
        //û�����ý�����ֱ�ӷ��سɹ�
        MEDIANET_SEM_GIVE(m_hSem);
        return bRet;
    }
    if( NULL == m_pbyDecryptOutBuf )
    {
        bRet = FALSE;
        MEDIANET_SEM_GIVE(m_hSem);

        if( (15 == g_nShowDebugInfo) && 0 == (m_tRcvStatistics.m_dwPackNum%20) )
        {
            OspPrintf(1, 0, "[DecryptRtpData] step 0 (NULL==m_pbyDecryptOutBuf) return ... \n");
        }

        return bRet;
    }

    //a. �ж��Ƿ������õĽ����غ�ֵ��һ��
    if( m_byRmtActivePT != pRtpPack->m_byPayload )
    {
        MEDIANET_SEM_GIVE(m_hSem);

        //����ʶ���غɣ��ص����ϲ�
        if( m_pRtpUnkownPtHandler != NULL )
        {
            m_pRtpUnkownPtHandler(pRtpPack, (KD_PTR)m_pRtpUnkownPtContext);
        }

        MEDIANET_SEM_TAKE(m_hSem);

        if( m_byRmtActivePT != pRtpPack->m_byPayload )
        {
            bRet = FALSE;
            MEDIANET_SEM_GIVE(m_hSem);

            if( (15 == g_nShowDebugInfo) && 0 == (m_tRcvStatistics.m_dwPackNum%20) )
            {
                OspPrintf(1, 0, "[DecryptRtpData] step 1 (%d != %d) return ... \n", m_byRmtActivePT, pRtpPack->m_byPayload);
            }

            return bRet;
        }
    }

    //b. �������ܲ�������
    if( AES_ENCRYPT_MODE == m_byDecryptMode )
    {
        //1. �����ܵ�ԭʼ����Ӧ���Ѿ�����, �����ж�Ϊ��������
        if( 0 != (pRtpPack->m_nRealSize%AES_ENCRYPT_BYTENUM) )
        {
            bRet = FALSE;
            MEDIANET_SEM_GIVE(m_hSem);

            if( (15 == g_nShowDebugInfo) && 0 == (m_tRcvStatistics.m_dwPackNum%20) )
            {
                OspPrintf(1, 0, "[DecryptRtpData AES] step 3 (m_nRealSize=%d) return ... \n", pRtpPack->m_nRealSize);
            }

            return bRet;
        }

        //2. ��������

        // ��ʼ key �� ���к���ʱ������ SSTTTTSSTTTTSSTT (S-Sequence, T-TimeStamp)
        s8  szInitKey[AES_ENCRYPT_BYTENUM] = {0};
        u16 wSequence    = htons( pRtpPack->m_wSequence );
        u32 dwTimeStamp  = htonl( pRtpPack->m_dwTimeStamp );
        u16 wOffset      = 0;
        memcpy( szInitKey+wOffset, &wSequence, sizeof(wSequence) );
        wOffset += sizeof(wSequence);
        memcpy( szInitKey+wOffset, &dwTimeStamp, sizeof(dwTimeStamp) );
        wOffset += sizeof(dwTimeStamp);
        memcpy( szInitKey+wOffset, szInitKey, sizeof(wSequence)+sizeof(dwTimeStamp) );
        wOffset += sizeof(wSequence)+sizeof(dwTimeStamp);
        memcpy( szInitKey+wOffset, &wSequence, sizeof(wSequence) );
        wOffset += sizeof(wSequence);
        memcpy( szInitKey+wOffset, &dwTimeStamp, AES_ENCRYPT_BYTENUM-wOffset );
#if 0
        s32 nAesRet = KdvAES( m_pszMaterialBuf, m_wMaterialBufLen,
                              m_byAesMode, qfDES_decrypt, szInitKey,
                              pRtpPack->m_pRealData, pRtpPack->m_nRealSize,
                              m_pbyDecryptOutBuf );
        if( nAesRet < 0 )
        {
            bRet = FALSE;
            MEDIANET_SEM_GIVE(m_hSem);

            if( (15 == g_nShowDebugInfo) && 0 == (m_tRcvStatistics.m_dwPackNum%20) )
            {
                OspPrintf(1, 0, "[DecryptRtpData AES] step 4 KdvAES Err, nAesRet=%d return ... \n", nAesRet);
            }

            return bRet;
        }
#endif
        pRtpPack->m_pRealData = m_pbyDecryptOutBuf;

        //3. �������ܵ������ܳ���, �ж��Ƿ���ڶ�������
        //   ������ڶ������⣬���ܳ���Ӧ (-tRtpPack.m_byPadNum)�ֽ�

        // Padding Bit Set
        // ���ڼ���������padding���ȵ��ռ� - ���һ���ֽ�
        if( 0 != pRtpPack->m_byPadNum )
        {
            pRtpPack->m_byPadNum   = *(pRtpPack->m_pRealData+pRtpPack->m_nRealSize-RTP_PADDING_SIZE);
            pRtpPack->m_nRealSize -= pRtpPack->m_byPadNum;
        }
    }

    if( DES_ENCRYPT_MODE == m_byDecryptMode )
    {
        //1. �����ܵ�ԭʼ����Ӧ���Ѿ�����, �����ж�Ϊ��������
        if( 0 != (pRtpPack->m_nRealSize%DES_ENCRYPT_BYTENUM) )
        {
            bRet = FALSE;
            MEDIANET_SEM_GIVE(m_hSem);

            if( (15 == g_nShowDebugInfo) && 0 == (m_tRcvStatistics.m_dwPackNum%20) )
            {
                OspPrintf(1, 0, "[DecryptRtpData DES] step 3 (m_nRealSize=%d) return ... \n", pRtpPack->m_nRealSize);
            }

            return bRet;
        }

        //2. ��������

        // ��ʼ key �� ���к���ʱ������ SSTTTTSS (S-Sequence, T-TimeStamp)
        u8  szInitKey[DES_ENCRYPT_BYTENUM] = {0};
        u16 wSequence    = htons( pRtpPack->m_wSequence );
        u32 dwTimeStamp  = htonl( pRtpPack->m_dwTimeStamp );
        memcpy( szInitKey, &wSequence, sizeof(wSequence) );
        memcpy( szInitKey+sizeof(wSequence), &dwTimeStamp, sizeof(dwTimeStamp) );
        memcpy( szInitKey+sizeof(wSequence)+sizeof(dwTimeStamp), &wSequence, sizeof(wSequence) );
#if 0
        KdvDES( (u8*)m_pszMaterialBuf,
                pRtpPack->m_pRealData,  (u32)pRtpPack->m_nRealSize,
                qfDES_decrypt, m_emDesMode,
                szInitKey );
#endif
        //3. �������ܵ������ܳ���, �ж��Ƿ���ڶ�������
        //   ������ڶ������⣬���ܳ���Ӧ (-tRtpPack.m_byPadNum)�ֽ�

        // Padding Bit Set
        // ���ڼ���������padding���ȵ��ռ� - ���һ���ֽ�
        if( 0 != pRtpPack->m_byPadNum )
        {
            pRtpPack->m_byPadNum   = *(pRtpPack->m_pRealData+pRtpPack->m_nRealSize-RTP_PADDING_SIZE);
            pRtpPack->m_nRealSize -= pRtpPack->m_byPadNum;
        }
    }

    MEDIANET_SEM_GIVE(m_hSem);
    return bRet;
}

/*=============================================================================
    ������        ��DealNetData
    ����        �������յ������ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealNetData(TRtpPack *pRtpPack)
{
    if(pRtpPack == NULL)
    {
        return;
    }

    m_tRcvStatistics.m_dwPackNum++;

    if(m_bRcvStart&&m_pRtpCallBackHandler != NULL)
    {
        m_pRtpCallBackHandler(pRtpPack, (KD_PTR)m_pRtpContext);
    }

    // ���ܶ�ģ�ⶪ��
    if( (g_nRcvDiscardSpan > 0) &&
        0 == (m_tRcvStatistics.m_dwPackNum%g_nRcvDiscardSpan) )
    {
        // ����֡��ţ��Ա�������ʶ����ܶ�����ģ�ⶪ��
        return;
    }

    // �������ܲ���
    if(FALSE == DecryptRtpData(pRtpPack) )
    {
        //����֡��ţ��Ա�������ʶ��������������ʧ��
        m_FrmHdr.m_dwFrameID++;
        m_tRcvStatistics.m_dwPackLose++;
        m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;
        return;
    }

	if (EStreamType_Null != m_eStreamType)
	{
		switch (m_eStreamType)
		{
		case EstreamType_PS:
			pRtpPack->m_byPayload = MEDIA_TYPE_PS;
			break;
		default:
			break;
		}
	}

    // ����Ƿ�Ϊ323��׼����
    if( TRUE == DealStdPayload(pRtpPack) )
    {
        return;
    }
    // ����Ƿ�Ϊ�Զ������� ���� ��չ�� 323�Ǳ���������̬�غ�������
    else if( TRUE == DealExtPayload(pRtpPack) )
    {
        return;
    }

    return;
}

/*=============================================================================
    ������        ��DealStdPayload
    ����        ������ H323�ѹ涨�ı�׼�غ�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - �ɹ�����
=============================================================================*/
BOOL32 CKdvNetRcv::DealStdPayload(TRtpPack *pRtpPack)
{
    BOOL32 bRet = TRUE;

    //���ݲ�ͬ���غɣ�����ͬ�Ĵ���
    switch(pRtpPack->m_byPayload)
    {
    case MEDIA_TYPE_H261:
        {
            DealH261( pRtpPack );
               break;
        }
    case MEDIA_TYPE_H263:
        {
            if ((MEDIA_TYPE_H263PLUS == m_byRealPT) &&
                (g_bForceH263Plus))
            {
                pRtpPack->m_byPayload = m_byRealPT;
                DealH263Plus(pRtpPack);
            }
            else
            {
                DealH263(pRtpPack);
            }

            break;
        }
    case MEDIA_TYPE_PCMU:
    case MEDIA_TYPE_PCMA:
    case MEDIA_TYPE_G7231:
    case MEDIA_TYPE_G728:
	case MEDIA_TYPE_G722:
	case MEDIA_TYPE_G729:
    case MEDIA_TYPE_ADPCM:
	case MEDIA_TYPE_AACLC:
	case MEDIA_TYPE_G726_16:
	case MEDIA_TYPE_G726_24:
	case MEDIA_TYPE_G726_32:
	case MEDIA_TYPE_G726_40:
	/*case MEDIA_TYPE_G7221C:  */  
        {
            DealG7xx( pRtpPack );
            break;
        }
    // ��H323��׼�غ�����......
    default:
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

/*=============================================================================
    ������        ��DealExtPayload
    ����        ������ ģ���������ܹ�ʶ����غ�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - �ɹ�����
=============================================================================*/
BOOL32 CKdvNetRcv::DealExtPayload(TRtpPack *pRtpPack)
{
    BOOL32 bRet = TRUE;

    // 1. ����Ƿ������˶�̬�غɣ�û�����ã���ֻ�ܴ����Զ����غ�����
    if( 0 == m_byRmtActivePT )
    {
        // Ŀǰ mpeg2��mpeg4��mp3��h.264 ���Զ��巽ʽ����, �ʷ��ڷǱ����չ���ִ���
        if( MEDIA_TYPE_H262 == pRtpPack->m_byPayload ||
            MEDIA_TYPE_MP4 == pRtpPack->m_byPayload )
        {
            DealMpg4( pRtpPack );
        }
        else if( MEDIA_TYPE_MP3 == pRtpPack->m_byPayload)
        {
            DealMp3( pRtpPack );
        }
        else if (MEDIA_TYPE_H264 == pRtpPack->m_byPayload)
        {
            DealH264(pRtpPack);
        }
        else if (MEDIA_TYPE_H265 == pRtpPack->m_byPayload)
        {
            DealH265(pRtpPack);
        }
        else if (MEDIA_TYPE_H224 == pRtpPack->m_byPayload)
        {
            DealDataPayload(pRtpPack);
        }
        else if (MEDIA_TYPE_G7221C == pRtpPack->m_byPayload || MEDIA_TYPE_AACLC == pRtpPack->m_byPayload
            || MEDIA_TYPE_AACLD == pRtpPack->m_byPayload || MEDIA_TYPE_AACLC_PCM == pRtpPack->m_byPayload
            || MEDIA_TYPE_AMR == pRtpPack->m_byPayload)
        {
            DealG7xx(pRtpPack);
        }
        else if (MEDIA_TYPE_PS == pRtpPack->m_byPayload)
        {
            DealPS(pRtpPack);
        }
        else
        {
            bRet = FALSE;
        }
    }
    // 2. �����ˣ������ӳ��, ��ԭΪ���˿�ʶ����غ�PTֵ,
    // b. �Ǽ�����������ò�����������̬�غ�ֵ���������˶�̬�غ�ֵ����ֱ���滻Ϊ��ʵ�غ�
    else
    {
        if( pRtpPack->m_byPayload != m_byRmtActivePT )
        {
            MedianetLog(Pack, "[DealExtPayload]m_byRmtActivePT:%3d not match rtp pack payload:%3d ...\n",
                m_byRmtActivePT, pRtpPack->m_byPayload);

            return FALSE;
        }

        pRtpPack->m_byPayload = m_byRealPT;

        switch(pRtpPack->m_byPayload)
        {
        // 2.1 ��׼ 323����
        case MEDIA_TYPE_H261:
            {
                DealH261( pRtpPack );
                   break;
            }
        case MEDIA_TYPE_H263:
            {
                DealH263( pRtpPack );
                break;
            }
        case MEDIA_TYPE_PCMU:
        case MEDIA_TYPE_PCMA:
        case MEDIA_TYPE_G7231:
        case MEDIA_TYPE_G728:
        case MEDIA_TYPE_G722:
        case MEDIA_TYPE_G729:
        case MEDIA_TYPE_ADPCM:
        case MEDIA_TYPE_G7221C:
        case MEDIA_TYPE_AACLC:
        case MEDIA_TYPE_AACLD:
		case MEDIA_TYPE_AACLC_PCM: 
		case MEDIA_TYPE_AMR:
		case MEDIA_TYPE_G726_16:
		case MEDIA_TYPE_G726_24:
		case MEDIA_TYPE_G726_32:
		case MEDIA_TYPE_G726_40:
			{
				DealG7xx( pRtpPack );
				break;
            }
        // 2.2 �Զ�������
        case MEDIA_TYPE_MP4:  //mpeg4
        case MEDIA_TYPE_H262: //mpeg2
        case MEDIA_TYPE_MJPEG://mjpeg
        {
            DealMpg4( pRtpPack );
            break;
        }
        case MEDIA_TYPE_MP3:  //mp3
        {
            DealMp3( pRtpPack );
            break;
        }
        // 2.3 ��չ�� 323�Ǳ�����
        case MEDIA_TYPE_H263PLUS:
            {
                DealH263Plus(pRtpPack);
                break;
            }
        case MEDIA_TYPE_H264:
            {
                DealH264(pRtpPack);
                break;
            }
        case MEDIA_TYPE_H265:
            {
                DealH265(pRtpPack);
                break;
            }
        case MEDIA_TYPE_H224: //H224�����غ�
            {
                DealDataPayload(pRtpPack);
                break;
            }
        case MEDIA_TYPE_PS:
            {
                DealPS(pRtpPack);
            }
            break;
        // �޷�ʶ����غ�......
        default:
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}


/*=============================================================================
    ������        ��DealDataPayload
    ����        �������յ� H224/H239/T120 ϵ���������غɵ����ݰ� (����������)
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealDataPayload(TRtpPack *pRtpPack)
{
    //a. ��ͬԴ���߲�ͬý�����͹��������ݣ���ն���
    if( (m_tLastInfo.m_dwSSRC != pRtpPack->m_dwSSRC) ||
        (m_tLastInfo.m_byMediaType != pRtpPack->m_byPayload) )
    {
        //����֡��ţ��Ա�������ʶ�������л�������Դ
        m_FrmHdr.m_dwFrameID++;
    }

    //b. ���кŲ����ᣬ�����¼
    if( (m_tLastInfo.m_dwSSRC == pRtpPack->m_dwSSRC) &&
        (pRtpPack->m_wSequence <= m_tLastInfo.m_wSeq) &&
        (pRtpPack->m_wSequence != 0) )
    {
        m_tRcvStatistics.m_dwPackIndexError++;
    }

    //c. �����������������,  ֱ�ӷ���
    m_FrmHdr.m_byMediaType = pRtpPack->m_byPayload;
    m_FrmHdr.m_dwDataSize  = pRtpPack->m_nRealSize;
    m_FrmHdr.m_pData       = pRtpPack->m_pRealData;
    m_FrmHdr.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
    m_FrmHdr.m_dwSSRC      = pRtpPack->m_dwSSRC;

    //����֡id��¼
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;
    m_tRcvStatistics.m_dwFrameNum++;

    if(m_bRcvStart && m_pFrameCallBackHandler != NULL && g_bInterRcvCallBack)
    {
        MedianetLog(Pack, "[DealDataPayload] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_byAudioMode=%d   \n",
                        m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize);

        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }

    //d. ��¼��С����Ϣ��������һ���������ж�
    m_tLastInfo.m_dwSSRC = pRtpPack->m_dwSSRC;
    m_tLastInfo.m_wSeq = pRtpPack->m_wSequence;
    m_tLastInfo.m_byMediaType = pRtpPack->m_byPayload;
    m_tLastInfo.m_dwTimeStamp = pRtpPack->m_dwTimeStamp;
    m_tLastInfo.m_byMark      = pRtpPack->m_byMark;

    return;
}



/*=============================================================================
    ������        ��ParasH261Head
    ����        ��h.261 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pRtpPackBuf     RTP����ָ��
                    pdwHeaderLen ���ݰ���ͷ���� *pdwHeaderLen - 0 ��Ч��ͷ

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::ParasH261Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH261Header *ptH261Header)
{
    if(NULL == pdwHeaderLen || NULL == ptH261Header)
    {
        return;
    }

    u32 dwH261Header = *((u32 *)(pRtpPackBuf));

    //get h261 header
    dwH261Header        = ntohl(dwH261Header);
    ptH261Header->vMvd  = GetBitField(dwH261Header, 0, 5);
    ptH261Header->hMvd  = GetBitField(dwH261Header, 5, 5);
    ptH261Header->quant = GetBitField(dwH261Header, 10, 5);
    ptH261Header->mbaP  = GetBitField(dwH261Header, 15, 5);
    ptH261Header->gobN  = GetBitField(dwH261Header, 20, 4);
    ptH261Header->v     = GetBitField(dwH261Header, 24, 1);
    ptH261Header->i        = GetBitField(dwH261Header, 25, 1);
    ptH261Header->eBit  = GetBitField(dwH261Header, 26, 3);
    ptH261Header->sBit  = GetBitField(dwH261Header, 29, 3);

    *pdwHeaderLen = sizeof(u32);

    return;
}

/*=============================================================================
    ������        ��ParasH263Head
    ����        ��h.263 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPackBuf     RTP����ָ��
                   pdwHeaderLen ���ݰ���ͷ���� *pdwHeaderLen - 0 ��Ч��ͷ

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::ParasH263Head(u8 *pRtpPackBuf, u32 *pdwHeaderLen, TH263Header *ptH263Header)
{
    if(NULL == pdwHeaderLen || NULL == ptH263Header)
    {
        return;
    }

    u32  dwHeaderArr[3];
    u32 *pdwHeader = (u32 *)(pRtpPackBuf);

    *pdwHeaderLen = 0;
    memset(dwHeaderArr, 0, sizeof(dwHeaderArr));

    dwHeaderArr[0] = *pdwHeader;

    //get h263 header
    dwHeaderArr[0]     = ntohl(dwHeaderArr[0]);
    ptH263Header->f    = GetBitField(dwHeaderArr[0], 31, 1);
    ptH263Header->p    = GetBitField(dwHeaderArr[0], 30, 1);
    ptH263Header->sBit = GetBitField(dwHeaderArr[0], 27, 3);
    ptH263Header->eBit = GetBitField(dwHeaderArr[0], 24, 3);
    ptH263Header->src  = GetBitField(dwHeaderArr[0], 21, 3);

    //A mode
    if((0 == ptH263Header->f) && (0 == ptH263Header->p))
    {
        *pdwHeaderLen     = sizeof(u32);

        ptH263Header->i   = GetBitField(dwHeaderArr[0], 20, 1);
        ptH263Header->u   = GetBitField(dwHeaderArr[0], 19, 1);
        ptH263Header->s   = GetBitField(dwHeaderArr[0], 18, 1);
        ptH263Header->a   = GetBitField(dwHeaderArr[0], 17, 1);
        ptH263Header->r   = GetBitField(dwHeaderArr[0], 13, 4);
        ptH263Header->dbq = GetBitField(dwHeaderArr[0], 11, 2);
        ptH263Header->trb = GetBitField(dwHeaderArr[0], 8, 3);
        ptH263Header->tr  = GetBitField(dwHeaderArr[0], 0, 8);
    }
    //B mode
    else if((1 == ptH263Header->f) && (0 == ptH263Header->p))
    {
        *pdwHeaderLen    = 2*sizeof(u32);

        pdwHeader++;
        dwHeaderArr[1]   = *pdwHeader;
        dwHeaderArr[1]   = ntohl(dwHeaderArr[1]);

        u32 dwQuant      = GetBitField(dwHeaderArr[0], 16, 5);
        u32 dwGobN       = GetBitField(dwHeaderArr[0], 11, 5);
        u32 dwMBA        = GetBitField(dwHeaderArr[0], 2, 9);
        ptH263Header->r  = GetBitField(dwHeaderArr[0], 0, 2);

        ptH263Header->i  = GetBitField(dwHeaderArr[1], 31, 1);
        ptH263Header->u  = GetBitField(dwHeaderArr[1], 30, 1);
        ptH263Header->s  = GetBitField(dwHeaderArr[1], 29, 1);
        ptH263Header->a  = GetBitField(dwHeaderArr[1], 28, 1);
        u32 dwHMV1       = GetBitField(dwHeaderArr[1], 21, 7);
        u32 dwVMV1       = GetBitField(dwHeaderArr[1], 14, 7);
        u32 dwHMV2       = GetBitField(dwHeaderArr[1], 7, 7);
        u32 dwVMV2       = GetBitField(dwHeaderArr[1], 0, 7);
    }
    //C mode
    else if((1 == ptH263Header->f) && (1 == ptH263Header->p))
    {
        *pdwHeaderLen     = 3*sizeof(u32);
        pdwHeader++;
        dwHeaderArr[1]    = *pdwHeader;
        dwHeaderArr[1]    = ntohl(dwHeaderArr[1]);
        pdwHeader++;
        dwHeaderArr[2]    = *pdwHeader;
        dwHeaderArr[2]    = ntohl(dwHeaderArr[2]);

        u32 dwQuant       = GetBitField(dwHeaderArr[0], 16, 5);
        u32 dwGobN        = GetBitField(dwHeaderArr[0], 11, 5);
        u32 dwMBA         = GetBitField(dwHeaderArr[0], 2, 9);
        ptH263Header->r   = GetBitField(dwHeaderArr[0], 0, 2);

        ptH263Header->i   = GetBitField(dwHeaderArr[1], 31, 1);
        ptH263Header->u   = GetBitField(dwHeaderArr[1], 30, 1);
        ptH263Header->s   = GetBitField(dwHeaderArr[1], 29, 1);
        ptH263Header->a   = GetBitField(dwHeaderArr[1], 28, 1);
        u32 dwHMV1        = GetBitField(dwHeaderArr[1], 21, 7);
        u32 dwVMV1        = GetBitField(dwHeaderArr[1], 14, 7);
        u32 dwHMV2        = GetBitField(dwHeaderArr[1], 7, 7);
        u32 dwVMV2        = GetBitField(dwHeaderArr[1], 0, 7);

        u32 dwRR          = GetBitField(dwHeaderArr[2], 13, 19);
        ptH263Header->dbq = GetBitField(dwHeaderArr[2], 11, 2);
        ptH263Header->trb = GetBitField(dwHeaderArr[2], 8, 3);
        ptH263Header->tr  = GetBitField(dwHeaderArr[2], 0, 8);
    }
}

static const u8 au8ZZSCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const u8 au8ZZSCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static void ScalingList(s32 l32SizeOfScalingList, TBitStream *s)
{
    s32 l32Index, l32Scanj, l32TmpScale;
    s32 l32DeltaScale, l32LastScale, l32NextScale;

    l32LastScale      = 8;
    l32NextScale      = 8;

    for(l32Index = 0; l32Index < l32SizeOfScalingList; l32Index++)
    {
        l32Scanj = (l32SizeOfScalingList==16) ? au8ZZSCAN[l32Index]:au8ZZSCAN8[l32Index];

        if(l32NextScale!=0)
        {
            //delta_scale
            l32DeltaScale = stdh264_bs_read_se( s );
            l32NextScale = (l32LastScale + l32DeltaScale + 256) % 256;
        }

        l32TmpScale = (l32NextScale==0) ? l32LastScale:l32NextScale;
        l32LastScale = l32TmpScale;
    }
}

/*=============================================================================
������        ��DecodeH264SPS
����        ������ h.264 �����е� sps ��Ϣ
�㷨ʵ��    ������ѡ�
����ȫ�ֱ�������
�������˵����

  ����ֵ˵���� TRUE - �ɹ�
=============================================================================*/
BOOL32 CKdvNetRcv::DecodeH264SPS( TBitStream *s, TSeqParameterSetRBSP *sps,
                                 TKdvH264Header *pStdH264Header )
{
    u32 i;
    s32 chroma_format_idc, seq_scaling_matrix_present_flag;
    s32 seq_scaling_list_present_flag;
    u32 crop_unit_x, crop_unit_y;
    u32 sps_pic_height;

    sps->profile_idc               = stdh264_bs_read( s, 8 );

    sps->constrained_set0_flag     = stdh264_bs_read( s, 1 );
    sps->constrained_set1_flag     = stdh264_bs_read( s, 1 );
    sps->constrained_set2_flag     = stdh264_bs_read( s, 1 );
    stdh264_bs_skip( s, 5 );        //reserved_zero

    sps->level_idc                 = stdh264_bs_read( s, 8 );

    sps->seq_parameter_set_id      = stdh264_bs_read_ue( s );
    if(sps->profile_idc  == 100) //HighProfile
    {
        chroma_format_idc = stdh264_bs_read_ue( s );         //chroma_format_idc

        if(chroma_format_idc == 3)
            stdh264_bs_read( s, 1 );//residual_colour_transform_flag

        stdh264_bs_read_ue( s );//bit_depth_luma_minus8
        stdh264_bs_read_ue( s );//bit_depth_chroma_minus8

        stdh264_bs_read( s, 1 );// lossless_qpprime_flag
        seq_scaling_matrix_present_flag = stdh264_bs_read( s, 1 );//seq_scaling_matrix_present_flag

        //ScalingMatrix;
        if(seq_scaling_matrix_present_flag)
        {
            for(i = 0; i < 8; i ++)
            {
                seq_scaling_list_present_flag = stdh264_bs_read( s, 1 );//seq_scaling_list_present_flag

                if(seq_scaling_list_present_flag)
                {
                    if(i < 6)
                    {
                        ScalingList(16, s);
                    }
                    else
                    {
                        ScalingList(64, s);
                    }
                }
            }
        }
    }

    sps->log2_max_frame_num_minus4 = stdh264_bs_read_ue( s );
    sps->pic_order_cnt_type        = stdh264_bs_read_ue( s );

    if (sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = stdh264_bs_read_ue( s );
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag      = stdh264_bs_read( s, 1 );
        sps->offset_for_non_ref_pic                = stdh264_bs_read_se( s );
        sps->offset_for_top_to_bottom_field        = stdh264_bs_read_se( s );
        sps->num_ref_frames_in_pic_order_cnt_cycle = stdh264_bs_read_ue( s );
        if (sps->num_ref_frames_in_pic_order_cnt_cycle > MAXnum_ref_frames_in_pic_order_cnt_cycle)
        {
            return FALSE;
        }
        for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame[i]           = stdh264_bs_read_se( s );
        }
    }

    sps->num_ref_frames                        = stdh264_bs_read_ue( s );
    sps->gaps_in_frame_num_value_allowed_flag  = stdh264_bs_read( s, 1 );
    sps->pic_width_in_mbs_minus1               = stdh264_bs_read_ue( s );  //  Width
    sps->pic_height_in_map_units_minus1        = stdh264_bs_read_ue( s );  //  Height
    sps->frame_mbs_only_flag                   = stdh264_bs_read( s, 1 );

    sps_pic_height = (2 - sps->frame_mbs_only_flag) * (sps->pic_height_in_map_units_minus1 + 1) * 16;

    if (!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag      = stdh264_bs_read( s, 1 );
    }
    sps->direct_8x8_inference_flag             = stdh264_bs_read( s, 1 );
    sps->frame_cropping_flag                   = stdh264_bs_read( s, 1 );

    if (sps->frame_cropping_flag)
    {
        crop_unit_x = 2;
        crop_unit_y = 2 * (2 - sps->frame_mbs_only_flag);

        sps->frame_cropping_rect_left_offset   = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_right_offset  = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_top_offset    = stdh264_bs_read_ue( s );
        sps->frame_cropping_rect_bottom_offset = stdh264_bs_read_ue( s );

        pStdH264Header->m_wWidth      = (u16)(sps->pic_width_in_mbs_minus1 + 1) * 16
            - (u16)(sps->frame_cropping_rect_left_offset + sps->frame_cropping_rect_right_offset) * (u16)crop_unit_x;

        pStdH264Header->m_wHeight     = (u16)sps_pic_height
            - (u16)(sps->frame_cropping_rect_top_offset + sps->frame_cropping_rect_bottom_offset) * (u16)crop_unit_y;;
    }
    else
    {
        pStdH264Header->m_wWidth      = (u16)(sps->pic_width_in_mbs_minus1 + 1) * 16;
        pStdH264Header->m_wHeight     = (u16)sps_pic_height;
    }

    sps->vui_parameters_present_flag           = stdh264_bs_read( s, 1 );

    if (sps->vui_parameters_present_flag)
    {
        //OspPintf( 1, 0, "VUI sequence parameters present but not supported, ignored, proceeding to next NALU\n");
    }

    sps->bIsValid = TRUE;

    pStdH264Header->m_bIsValidSPS = TRUE;
    pStdH264Header->m_dwSPSId     = sps->seq_parameter_set_id;

    return TRUE;
}

/*=============================================================================
������        ��DecodeH264PPS
����        ������ h.264 �����е� pps ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� TRUE - �ɹ�
=============================================================================*/
BOOL32 CKdvNetRcv::DecodeH264PPS( TBitStream *s, TPicParameterSetRBSP *pps,
                                  TKdvH264Header *pStdH264Header )
{
    u32 i;
    s32 NumberBitsPerSliceGroupId;

    pps->pic_parameter_set_id     = stdh264_bs_read_ue( s );
    pps->seq_parameter_set_id     = stdh264_bs_read_ue( s );
    pps->entropy_coding_mode_flag = stdh264_bs_read( s, 1 );
    pps->pic_order_present_flag   = stdh264_bs_read( s, 1 );
    pps->num_slice_groups_minus1  = stdh264_bs_read_ue( s );

    // FMO stuff begins here
    //hual modi 2005-06-24
    if (pps->num_slice_groups_minus1 > MAXnum_slice_groups_minus1)
    {
        return FALSE;
    }

    if (pps->num_slice_groups_minus1 > 0)
    {
        pps->slice_group_map_type = stdh264_bs_read_ue( s );

        switch( pps->slice_group_map_type )
        {
            case 0:
            {
                for (i=0; i<=pps->num_slice_groups_minus1; i++)
                {
                    pps->run_length_minus1[i] = stdh264_bs_read_ue( s );
                }
                break;
            }
            case 2:
            {
                for (i=0; i<pps->num_slice_groups_minus1; i++)
                {
                    //! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
                    pps->top_left [i]                  = stdh264_bs_read_ue( s );
                    pps->bottom_right [i]              = stdh264_bs_read_ue( s );
                }
                break;
            }
            case 3:
            case 4:
            case 5:
            {
                pps->slice_group_change_direction_flag = stdh264_bs_read( s, 1 );
                pps->slice_group_change_rate_minus1    = stdh264_bs_read_ue( s );
                break;
            }
            case 6:
            {
                if (pps->num_slice_groups_minus1+1 >4)
                {
                    NumberBitsPerSliceGroupId = 3;
                }
                else if (pps->num_slice_groups_minus1+1 > 2)
                {
                    NumberBitsPerSliceGroupId = 2;
                }
                else
                {
                    NumberBitsPerSliceGroupId = 1;
                }

                //! JVT-F078, exlicitly signal number of MBs in the map
                pps->num_slice_group_map_units_minus1      = stdh264_bs_read_ue( s );

                //hual add 2005-06-24
                if (NULL == pps->slice_group_id)
                {
                    break;
                }

                for (i=0; i<=pps->num_slice_group_map_units_minus1; i++)
                {
                    pps->slice_group_id[i] = stdh264_bs_read(s,NumberBitsPerSliceGroupId );//maywrong
                }
                break;
            }
            default:
                break;
        }
    }

    // End of FMO stuff

    pps->num_ref_idx_l0_active_minus1           = stdh264_bs_read_ue( s );
    pps->num_ref_idx_l1_active_minus1           = stdh264_bs_read_ue( s );
    pps->weighted_pred_flag                     = stdh264_bs_read( s, 1 );
    pps->weighted_bipred_idc                    = stdh264_bs_read( s, 2 );
    pps->pic_init_qp_minus26                    = stdh264_bs_read_se( s );
    pps->pic_init_qs_minus26                    = stdh264_bs_read_se( s );
    pps->chroma_qp_index_offset                 = stdh264_bs_read_se( s );
    pps->deblocking_filter_control_present_flag = stdh264_bs_read( s, 1 );
    pps->constrained_intra_pred_flag            = stdh264_bs_read( s, 1 );
    pps->redundant_pic_cnt_present_flag         = stdh264_bs_read( s, 1 );

    pps->bIsValid = TRUE;
    pStdH264Header->m_bIsValidPPS = TRUE;

    return TRUE;
}

/*=============================================================================
    ������        ��ParseH264Head
    ����        ��Std h.264 RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPackBuf     RTP����ָ��
                   pdwHeaderLen    ���ݰ���ͷ���� *pdwHeaderLen - 0 ��Ч��ͷ
                   ptH264Header    ͷ�ṹָ��

    ����ֵ˵���� ��
=============================================================================*/
BOOL32 CKdvNetRcv::ParseH264Head( u8 *pRtpPackBuf, s32 nRtpPackBufSize,
                                TKdvH264Header *ptH264Header, BOOL32 bISBitStream /*TRUE*/)
{
    if (nRtpPackBufSize <= 1 || NULL == ptH264Header)
    {
        return FALSE;
    }

    BOOL32 bRet = TRUE;
    // the format of the NAL unit type octet is reprinted below:
    //  +---------------+
    //    |0|1|2|3|4|5|6|7|
    //    +-+-+-+-+-+-+-+-+
    //    |F|NRI|  Type   |
    //    +---------------+
    u8  byNaluTypeOctet = (u8) (*pRtpPackBuf);

    u32 dwNaluType = byNaluTypeOctet & 0x1F;  // ȡ�õ�5λ
    TBitStream tBitStream;
    memset(&tBitStream, 0, sizeof(tBitStream));

    // xp090224
    if (28 == dwNaluType)
    {
        if ((pRtpPackBuf[1] & 0x80) > 0)
        {
            // ����ǵ�һ����Ƭ����Ҫ���ƫ��һ���ֽ�
            dwNaluType = pRtpPackBuf[1] & 0x1F;
            stdh264_bs_init(&tBitStream, (pRtpPackBuf+2), (nRtpPackBufSize-2));
        }
        else
        {
            // ���ǵ�һ����ʲô������
            return TRUE;
        }
    }
    else
    {
        // TBitStream�ṹ��RTP���ĵ�14�ֽڿ�ʼ��ǰ12�ֽ�ΪRTPͷ����13�ֽ�ΪNalu���ͣ�
        stdh264_bs_init(&tBitStream, (pRtpPackBuf+1), (nRtpPackBufSize-1));
    }

    TSeqParameterSetRBSP tSPS;
    TPicParameterSetRBSP tPPS;
    Tstdh264Dec_SliceHeaderData tSlice_header;
    memset(&tSPS, 0, sizeof(tSPS));
    memset(&tPPS, 0, sizeof(tPPS));
    memset(&tSlice_header, 0, sizeof(tSlice_header));

    switch(dwNaluType)
    {
    case 1:
    case 5:  // NALU_TYPE_IDR
        stdh264_FirstPartOfSliceHeader(&tBitStream, &tSlice_header);
        if( I_SLICE == tSlice_header.slice_type)
        {
            ptH264Header->m_bIsKeyFrame = TRUE;
        }
        //OspPrintf( 1, 0, "[CKdvNetRcv::ParseH264Head] NALU_TYPE_IDR ...........\n",  );
        break;
    case 7:  // NALU_TYPE_SPS
        bRet = DecodeH264SPS(&tBitStream, &tSPS, ptH264Header);

        //ʵʱ���ҽ���sps ok����־λ�������ѻ����Чsps
        if(bISBitStream && bRet)
        {
            m_tKdvSpsPpsInfo.m_bHaveSps = TRUE;
        }
        //OspPrintf( 1, 0, "[CKdvNetRcv::ParseH264Head] DecodeH264SPS ...........\n" );
        break;
    case 8:  // NALU_TYPE_PPS
        bRet = DecodeH264PPS(&tBitStream, &tPPS, ptH264Header);

        //ʵʱ���ҽ���pps ok����־λ�������ѻ����Чpps
        if(bISBitStream && bRet)
        {
            m_tKdvSpsPpsInfo.m_bHavePps = TRUE;
        }
        //OspPrintf( 1, 0, "[CKdvNetRcv::ParseH264Head] DecodeH264PPS ...........\n" );
        break;
    default:
        break;
    }

    return bRet;
}

BOOL32 CKdvNetRcv::ParseH265Head( u8 *pRtpPackBuf, s32 nRtpPackBufSize,
                                TKdvH265Header *ptH265Header)
{
    if (nRtpPackBufSize <= 1 || NULL == ptH265Header)
    {
        MedianetLog(Parse, "[ParseH265Head] nalu length too small\n");
        return FALSE;
    }

    BOOL32 bRet = TRUE;
    //+---------------+---------------+
    //|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|F|   Type    |  LayerId  | TID |
    //+-------------+-----------------+
    u8  byNaluTypeOctet = (u8) (*pRtpPackBuf);

    u32 dwNaluType = (byNaluTypeOctet & 0x7E)>>1;  // ȡ���м�6λ
    TBitStream tBitStream;
    memset(&tBitStream, 0, sizeof(tBitStream));

    if (49 == dwNaluType)//��Ƭ��
    {
        if ((pRtpPackBuf[2] & 0x80) > 0)//��һ��
        {
            // ����ǵ�һ����Ƭ����Ҫ���ƫ�������ֽ�
            dwNaluType = pRtpPackBuf[2] & 0x3F;
            stdh265_bs_init(&tBitStream, (pRtpPackBuf+3), (nRtpPackBufSize-3));
        }
        else
        {
            // ���ǵ�һ����ʲô������
            return TRUE;
        }
    }
    else
    {
        // TBitStream�ṹ��RTP���������ݵĵ�3�ֽڿ�ʼ��ǰ2�ֽ�ΪNaluͷ��
        stdh265_bs_init(&tBitStream, (pRtpPackBuf+2), (nRtpPackBufSize-2));
    }

    TSPS tSPS;
    TPPS tPPS;
    Tstdh265Dec_SliceHeaderData tSlice_header;
    memset(&tSPS, 0, sizeof(tSPS));
    memset(&tPPS, 0, sizeof(tPPS));
    memset(&tSlice_header, 0, sizeof(tSlice_header));

    switch(dwNaluType)
    {
    case 1:
    case 19:  // NALU_TYPE_IDR
        stdh265_FirstPartOfSliceHeader(&tBitStream, &tSlice_header, ptH265Header, dwNaluType);
        if(I_SLICE == tSlice_header.slice_type)
        {
            ptH265Header->m_bIsKeyFrame = TRUE;
        }
        //OspPrintf( 1, 0, "[CKdvNetRcv::ParseH264Head] NALU_TYPE_IDR ...........\n",  );
        break;
    case 33:  // NALU_TYPE_SPS
        bRet = DecodeH265SPS(&tBitStream, &tSPS, ptH265Header);
        break;
    case 34:  // NALU_TYPE_PPS
        bRet = DecodeH265PPS(&tBitStream, &tPPS, ptH265Header);
        break;
    default:
        break;
    }

    bRet = TRUE;

    return bRet;
}

/*=============================================================================
    ������        ��ParasH263PlusHead
    ����        ��h.263+ RTP head info ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPackBuf      RTP����ָ��
                   pdwHeaderLen     ���ݰ���ͷ���� *pdwHeaderLen - 0 ��Ч��ͷ
                   ptH263PlusHeader ���ṹָ��

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::ParasH263PlusHead( u8 *pRtpPackBuf, u32 *pdwHeaderLen,
                                    TH263PlusHeader *ptH263PlusHeader )
{
    if(NULL == pdwHeaderLen || NULL == ptH263PlusHeader)
    {
        return;
    }

    u16 wH263PlusHeader = *((u16 *)(pRtpPackBuf));

    //get h263+ header
    wH263PlusHeader          = ntohs(wH263PlusHeader);
    ptH263PlusHeader->revBit = GetBitField(wH263PlusHeader, 11, 5);
    ptH263PlusHeader->pBit   = GetBitField(wH263PlusHeader, 10, 1);
    ptH263PlusHeader->vrcBit = GetBitField(wH263PlusHeader, 9,  1);
    ptH263PlusHeader->phLen  = GetBitField(wH263PlusHeader, 3,  6);
    ptH263PlusHeader->pheBit = GetBitField(wH263PlusHeader, 0,  3);

    //����λ��Ӧ��Ϊ0�������������ͷ��Ϣ����
    if( 0 == ptH263PlusHeader->revBit )
    {
        *pdwHeaderLen = sizeof(u16);
    }

    //OspPrintf( 1, 0, "ParasH263PlusHead Info -- revBit:%d, pBit:%d, vrcBit:%d, phLen:%d, pheBit:%d ....  \n",
    //                 ptH263PlusHeader->revBit, ptH263PlusHeader->pBit,
    //                 ptH263PlusHeader->vrcBit, ptH263PlusHeader->phLen, ptH263PlusHeader->pheBit );

    return;
}

/*=============================================================================
    ������        ��GetH263PicInfo
    ����        ����ȡ H.263/H.263+ ֡��ߡ��ؼ�֡����Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pbyBitstream     h.263/h.263+ ֡����ָ��
                   nBitstreamLen    h.263/h.263+ ֡���峤��
                   pbIsKeyFrame     ֡�Ĺؼ�֡��Ϣ
                   pwWidth          ֡����Ϣ
                   pwHeight         ֡����Ϣ

    ����ֵ˵���� ��
=============================================================================*/
BOOL32 CKdvNetRcv::GetH263PicInfo( u8 *pbyBitstream, s32 nBitstreamLen,
                                   BOOL32 *pbIsKeyFrame, u16 *pwWidth, u16 *pwHeight )
{
    TH263DecFrameHeader tH263DecPicHeader;
    memset( &tH263DecPicHeader, 0, sizeof(tH263DecPicHeader) );
    BOOL32 bRet = Stdh263_GetH263PicInfo( pbyBitstream, nBitstreamLen, &tH263DecPicHeader );

    *pwWidth  = 0;
    *pwHeight = 0;

    if( 0 == tH263DecPicHeader.s16PictureType )
    {
        *pbIsKeyFrame = TRUE;
    }
    else
    {
        *pbIsKeyFrame = FALSE;
    }
    *pwWidth  = (u16)tH263DecPicHeader.s32Width;
    *pwHeight = (u16)tH263DecPicHeader.s32Height;

    return bRet;
}

/*=============================================================================
    ������        ��CheckH26XPack
    ����        ��h.261 h.263 Std H.264 RTP ����Ч�� ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - ��Ч���ݰ�
=============================================================================*/
BOOL32 CKdvNetRcv::CheckH26XPack(TRtpPack *pRtpPack)
{
    BOOL32 bRet = FALSE;

    u32 dwHeaderLen = 0;

    switch(pRtpPack->m_byPayload)
    {
    case MEDIA_TYPE_H261:
        {
            //h.261 RTP head info ����
            TH261Header tH261Header;
            ParasH261Head(pRtpPack->m_pRealData, &dwHeaderLen, &tH261Header);
            if(0 == dwHeaderLen)
            {
                 MedianetLog(Parse, "[CheckH26XPack] h.261 dwHeaderLen:%d, Not A&B&C MODE  \n", dwHeaderLen);
            }
            else
            {
                s32 nValidLen = pRtpPack->m_nRealSize - dwHeaderLen;
                if( (nValidLen < 0) ||  //nValidLen ���� ==0������Ϊ�հ�
                    (pRtpPack->m_nRealSize > MAX_RCV_PACK_SIZE) )
                {
                    MedianetLog(Parse, "[CheckH26XPack] h.261 m_nRealSize Exception, nValidLen=%d, m_nRealSize=%d   \n",
                                  nValidLen, pRtpPack->m_nRealSize);
                }
                bRet = TRUE;
            }
        } break;

    case MEDIA_TYPE_H263:
        {
            //h.263 RTP head info ����
            TH263Header tH263Header;
            ParasH263Head(pRtpPack->m_pRealData, &dwHeaderLen, &tH263Header);
            if(0 == dwHeaderLen)
            {
                MedianetLog(Parse, "[CheckH26XPack] h.263 dwHeaderLen:%d, Not A&B&C MODE  \n", dwHeaderLen);
            }
            else
            {
                s32 nValidLen = pRtpPack->m_nRealSize - dwHeaderLen;
                if( (nValidLen < 0) ||  //nValidLen ���� ==0������Ϊ�հ�
                    (pRtpPack->m_nRealSize > MAX_RCV_PACK_SIZE) )
                {
                    MedianetLog(Parse, "[CheckH26XPack] h.263 m_nRealSize Exception, nValidLen=%d, m_nRealSize=%d   \n",
                        nValidLen, pRtpPack->m_nRealSize);
                }
                bRet = TRUE;
            }
        } break;

    case MEDIA_TYPE_H263PLUS:
        {
            //h.263+ RTP head info ����
            TH263PlusHeader tH263PlusHeader;
            ParasH263PlusHead(pRtpPack->m_pRealData, &dwHeaderLen, &tH263PlusHeader);
            if(0 == dwHeaderLen)
            {
                MedianetLog(Parse, "[CheckH26XPack] h.263+ dwHeaderLen:%d, Not A&B&C MODE  \n", dwHeaderLen);
            }
            else
            {
                s32 nValidLen = pRtpPack->m_nRealSize - dwHeaderLen;
                if( (nValidLen < 0) ||  //nValidLen ���� ==0������Ϊ�հ�
                    (pRtpPack->m_nRealSize > MAX_RCV_PACK_SIZE) )
                {
                    MedianetLog(Parse, "[CheckH26XPack] h.263+ m_nRealSize Exception, nValidLen=%d, m_nRealSize=%d   \n",
                            nValidLen, pRtpPack->m_nRealSize);
                }
                bRet = TRUE;
            }
        } break;

    case MEDIA_TYPE_H264:
        {
            //h.264 RTP head info ����
            //���ؼ�֡��Ϣ��ΪFALSE, ������Ϣ����
            m_tH264Header.m_bIsKeyFrame = FALSE;

            //H.264����ÿһ֡������ sps��pps��Ϣ
            //Paraseʧ�ܣ����ش��� hual 2006-06-21
            bRet = ParseH264Head(pRtpPack->m_pRealData, pRtpPack->m_nRealSize, &m_tH264Header);
            if (!bRet)
            {
                MedianetLog(Parse, "[CheckH26XPack] Parse h.264 Head Error\n");
            }
        } break;
    case MEDIA_TYPE_H265:
        {
            m_tH265Header.m_bIsKeyFrame = FALSE;
            bRet = ParseH265Head(pRtpPack->m_pRealData, pRtpPack->m_nRealSize, &m_tH265Header);
            if (!bRet)
            {
                MedianetLog(Parse, "[CheckH26XPack] Parse h.264 Head Error\n");
            }
        }
        break;
    default:
        break;
    }

    return bRet;
}

//����sps��ppsbuf
u16 CKdvNetRcv::ParseSpsPpsBuf(u8 *pbySpsBuf, s32 nSpsBufLen, u8 *pbyPpsBuf, s32 nPpsBufLen)
{
    TKdvH264Header tKdvH264Header;

     MEDIANET_SEM_TAKE(m_hSem);

    if((u32)nPpsBufLen > MAX_SPSPPS_BUF_LEN || (u32)nSpsBufLen > MAX_SPSPPS_BUF_LEN ||
        pbySpsBuf == NULL || pbyPpsBuf == NULL)
    {
        MedianetLog(Parse,"[CKdvNetRcv::ParseSpsPpsBuf] the length of sps(pps) longer than 128 B\n");
        MEDIANET_SEM_GIVE(m_hSem);
        return ERROR_PARSE_SPSPPS;
    }

    //����sps��ppsbuf , ���һ��������ʾ��ʵʱ��
    if(!ParseH264Head(pbySpsBuf, nSpsBufLen, &tKdvH264Header, FALSE) || \
         !ParseH264Head(pbyPpsBuf, nPpsBufLen, &tKdvH264Header, FALSE))
    {
        MedianetLog(Parse,"[CKdvNetRcv::ParseSpsPpsBuf] Parse sps(pps) buf Error");
        MEDIANET_SEM_GIVE(m_hSem);
        return ERROR_PARSE_SPSPPS;
    }

    //��H264ͷ��ֵ
    m_tH264Header.m_bIsValidPPS = tKdvH264Header.m_bIsValidPPS;;
    m_tH264Header.m_bIsValidSPS = tKdvH264Header.m_bIsValidSPS;
    m_tH264Header.m_dwSPSId = tKdvH264Header.m_dwSPSId;
    m_tH264Header.m_wHeight = tKdvH264Header.m_wHeight;
    m_tH264Header.m_wWidth = tKdvH264Header.m_wWidth;

    //��һ�η����ڴ�
    if(!m_tKdvSpsPpsInfo.pbyPpsBuf)
    {
        //��һ�����sps rtp�������ڴ�
        MEDIANET_MALLOC(m_tKdvSpsPpsInfo.pbyPpsBuf, MAX_SPSPPS_BUF_LEN);
    }
    if(!m_tKdvSpsPpsInfo.pbySpsBuf)
    {
        MEDIANET_MALLOC(m_tKdvSpsPpsInfo.pbySpsBuf, MAX_SPSPPS_BUF_LEN);
    }
    if(m_tKdvSpsPpsInfo.pbyPpsBuf == NULL|| m_tKdvSpsPpsInfo.pbySpsBuf == NULL)
    {
        MEDIANET_SEM_GIVE(m_hSem);
        return ERROR_NET_RCV_MEMORY;
    }

    //������copy
    if(m_tKdvSpsPpsInfo.pbyPpsBuf)
    {
        //��sps��pps info ��ֵ������sps��ppsԴ����
        m_tKdvSpsPpsInfo.nPpsBufLen = nPpsBufLen;
        memcpy(m_tKdvSpsPpsInfo.pbyPpsBuf, pbyPpsBuf, m_tKdvSpsPpsInfo.nPpsBufLen);
    }
    if(m_tKdvSpsPpsInfo.pbySpsBuf)
    {
        m_tKdvSpsPpsInfo.nSpsBufLen = nSpsBufLen;
        memcpy(m_tKdvSpsPpsInfo.pbySpsBuf, pbySpsBuf, m_tKdvSpsPpsInfo.nSpsBufLen);
    }

    MedianetLog(Parse,"[CKdvNetRcv::ParseSpsPpsBuf]m_tH264Header.m_bIsValidPPS=%d, m_tH264Header.m_bIsValidSPS=%d,m_tH264Header.m_dwSPSId=%d,  \
        m_tH264Header.m_wHeight=%d,m_tH264Header.m_wWidth=%d\n",m_tH264Header.m_bIsValidPPS, m_tH264Header.m_bIsValidSPS, m_tH264Header.m_dwSPSId, \
        m_tH264Header.m_wHeight, m_tH264Header.m_wWidth);
     MEDIANET_SEM_GIVE(m_hSem);
     return MEDIANET_NO_ERROR;
}

/*=============================================================================
    ������        ��SaveH261HeadList
    ����        ������ h.261 ��Ӧ֡��ͷ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pPackDataBuf    ������(������ͷ)
                    dwPackValidLen  �����յ������ݳ���(������ͷ�Լ�ebit)
                    byLastEbit      ��һ����ebitֵ
                    dwHeaderLen     ��ͷ����

    ����ֵ˵����
=============================================================================*/
void CKdvNetRcv::SaveH261HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit)
{
    //���ɳ������С����
    if( m_tH261HeaderList.m_nNum >= MAX_H261_PACK_NUM )
    {
        return;
    }

    u32 dwHeader = (*((u32 *)(pPackDataBuf)));
    dwHeader = ntohl(dwHeader);

    m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum].m_dwHeader = dwHeader;

    m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum].m_dwPackLen = dwPackLen;

    if(0 == m_tH261HeaderList.m_nNum)
    {
        m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum].m_dwStartPos = 0;
    }
    else
    {
        m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum].m_dwStartPos =
            m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum-1].m_dwStartPos +
            m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum-1].m_dwPackLen;
        if(0 != byLastEbit)
        {
            m_tH261HeaderList.m_tkdvH261Header[m_tH261HeaderList.m_nNum].m_dwStartPos -= 1;
        }
    }

    m_tH261HeaderList.m_nNum++;

    return;
}

/*=============================================================================
    ������        ��SaveH263HeadList
    ����        ������ h.263 ��Ӧ֡��ͷ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                  pPackDataBuf    ������(������ͷ)
                  dwPackValidLen  �����յ������ݳ���(������ͷ�Լ�ebit)
                  byLastEbit      ��һ����ebitֵ
                  dwHeaderLen     ��ͷ����

    ����ֵ˵����
=============================================================================*/
void CKdvNetRcv::SaveH263HeadList(u8* pPackDataBuf, u32 dwPackLen, u8 byLastEbit, u32 dwHeaderLen)
{
    //���ɳ������С����
    if( m_tH263HeaderList.m_nNum >= MAX_H263_PACK_NUM )
    {
        return;
    }

    u32  dwHeaderArr[3];
    u32 *pdwHeader = (u32 *)(pPackDataBuf);

    memset(dwHeaderArr, 0, sizeof(dwHeaderArr));
    dwHeaderArr[0] = *pdwHeader;

    //get h263 header ��¼����rtp����h263ͷ

    if(sizeof(u32) == dwHeaderLen) //A mode
    {
        dwHeaderArr[0] = ntohl(dwHeaderArr[0]);
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwMode = MODE_H263_A;
    }
    else if(2*sizeof(u32) == dwHeaderLen) //B mode
    {
        dwHeaderArr[0] = ntohl(dwHeaderArr[0]);
        pdwHeader++;
        dwHeaderArr[1] = *pdwHeader;
        dwHeaderArr[1] = ntohl(dwHeaderArr[1]);
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwMode = MODE_H263_B;
    }
    else if(3*sizeof(u32) == dwHeaderLen) //C mode
    {
        dwHeaderArr[0] = ntohl(dwHeaderArr[0]);
        pdwHeader++;
        dwHeaderArr[1] = *pdwHeader;
        dwHeaderArr[1] = ntohl(dwHeaderArr[1]);
        pdwHeader++;
        dwHeaderArr[2] = *pdwHeader;
        dwHeaderArr[2] = ntohl(dwHeaderArr[2]);
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwMode = MODE_H263_C;
    }
    else
    {
        return;
    }

    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwHeader1 = dwHeaderArr[0];
    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwHeader2 = dwHeaderArr[1];
    //m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwHeader3 = dwHeaderArr[2];

    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwDataLen = dwPackLen;

    if(0 == m_tH263HeaderList.m_nNum)
    {
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwStartPos = 0;
    }
    else
    {
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwStartPos =
          m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum-1].m_dwStartPos +
          m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum-1].m_dwDataLen;
        if(0 != byLastEbit)
        {
            m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwStartPos -= 1;
        }
    }

    m_tH263HeaderList.m_nNum++;

    return;
}

/*=============================================================================
    ������        ��SaveH263PlusHeadList
    ����        ������ h.263+ ��Ӧ֡��ͷ��Ϣ h.263+ ��Ϊ�����h.263�������д���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    pPackDataBuf    ������(������ͷ)
                    dwPackValidLen  �����յ������ݳ���(������ͷ�Լ�ebit)
                    dwHeaderLen     ��ͷ����

    ����ֵ˵����
=============================================================================*/
void CKdvNetRcv::SaveH263PlusHeadList(u8* pPackDataBuf, u32 dwPackLen)
{
    //���ɳ������С����
    if( m_tH263HeaderList.m_nNum >= MAX_H263_PACK_NUM )
    {
        return;
    }

    u16 wHeader = (*((u16 *)(pPackDataBuf)));
    wHeader = ntohs(wHeader);

    //����Լ��MODE_H263_D��Ϊh.263+�� ��Ϊ�����h.263�������д���
    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwMode    = MODE_H263_D;
    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwHeader1 =
        SetBitField(m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwHeader1, wHeader, 0, 16);

    m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwDataLen = dwPackLen;

    if(0 == m_tH263HeaderList.m_nNum)
    {
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwStartPos = 0;
    }
    else
    {
        m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum].m_dwStartPos =
            m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum-1].m_dwStartPos +
            m_tH263HeaderList.m_tKdvH263Header[m_tH263HeaderList.m_nNum-1].m_dwDataLen;
    }

    m_tH263HeaderList.m_nNum++;

    return;
}

/*=============================================================================
    ������        ��CheckMp3Pack
    ����        ��mp3 RTP ����Ч�� ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - ��Ч���ݰ�
=============================================================================*/
BOOL32 CKdvNetRcv::CheckMp3Pack(TRtpPack *pRtpPack)
{
    BOOL32 bRet = FALSE;

    // ������Ч���ж�
    if( (pRtpPack->m_nRealSize <= 0) ||
        (pRtpPack->m_nRealSize > MAX_EXTEND_PACK_SIZE) )
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckMp3Pack] m_nRealSize Exception, m_byPayload=%d, m_nRealSize=%d   \n",
                             pRtpPack->m_byPayload, pRtpPack->m_nRealSize);

        return bRet;
    }

    // ��չ�ռ��Ƿ����
    if( pRtpPack->m_nExSize != (MIN_PACK_EX_LEN/sizeof(u32)) )
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckMp3Pack] m_nRealSize Exception, m_byPayload=%d, m_nExSize=%d   \n",
                             pRtpPack->m_byPayload, pRtpPack->m_nExSize);

        return bRet;
    }

    // ��ÿ��RTP����չ���� - ��ȡ��ƵС��������Ϣ
    m_tAudioHeader.m_byPackNum = *(pRtpPack->m_pExData + EX_TOTALNUM_POS);
    m_tAudioHeader.m_byIndex = *(pRtpPack->m_pExData + EX_INDEX_POS);
    m_tAudioHeader.m_nMode = (s32)(pRtpPack->m_pExData[EX_FRAMEMODE_POS]);
                            //NET_AUDIO_MODE_BEST

    //
    if( (m_tAudioHeader.m_byIndex < 1) ||
        (m_tAudioHeader.m_byIndex > MAX_EXTEND_PACK_NUM) ||
        (m_tAudioHeader.m_byIndex > m_tAudioHeader.m_byPackNum) ) //+by lxx
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckMp3Pack] m_byIndex Exception, m_byPayload=%d, m_byIndex=%d, m_byPackNum=%d\n",
                pRtpPack->m_byPayload, m_tAudioHeader.m_byIndex, m_tAudioHeader.m_byPackNum);

        return bRet;
    }

    bRet = TRUE;
    return bRet;
}

/*=============================================================================
������        ��CheckMpg4Pack
����        ��mpg4 h.264 RTP ����Ч�� ����
�㷨ʵ��    ������ѡ�
����ȫ�ֱ�������
�������˵����
pRtpPack     RTP���ݰ��ṹ,�μ�����

  ����ֵ˵���� TRUE - ��Ч���ݰ�
=============================================================================*/
BOOL32 CKdvNetRcv::CheckMpg4Pack(TRtpPack *pRtpPack)
{
    BOOL32 bRet = FALSE;

    // ������Ч���ж�
    if( (pRtpPack->m_nRealSize < 0) ||
        (pRtpPack->m_nRealSize > 1460) )    //��MTU 1500���㣬�����ܵ�RTP�����ݳ���
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckMpg4Pack] m_nRealSize Exception, m_byPayload=%d, m_nRealSize=%d   \n",
                pRtpPack->m_byPayload, pRtpPack->m_nRealSize);

        return bRet;
    }

    // ��չ�ռ��Ƿ����
    if( (pRtpPack->m_byMark && pRtpPack->m_nExSize != (MAX_PACK_EX_LEN/sizeof(u32))) ||
        (!pRtpPack->m_byMark && pRtpPack->m_nExSize != (MIN_PACK_EX_LEN/sizeof(u32))) )
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckMpg4Pack] m_nRealSize Exception, m_byPayload=%d, m_nExSize=%d   \n",
                pRtpPack->m_byPayload, pRtpPack->m_nExSize);

        return bRet;
    }

    // ��ÿ��RTP����չ���� - ��ȡ��ƵС��������Ϣ
    m_tAudioHeader.m_byPackNum  = *(pRtpPack->m_pExData + EX_TOTALNUM_POS);
    m_tAudioHeader.m_byIndex  = *(pRtpPack->m_pExData + EX_INDEX_POS);

    //
    if( (m_tAudioHeader.m_byIndex < 1) ||
        (m_tAudioHeader.m_byIndex > MAX_EXTEND_PACK_NUM) ||
        (m_tAudioHeader.m_byIndex > m_tAudioHeader.m_byPackNum) ) //+by lxx
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, 0, "[CKdvNetRcv::CheckMpg4Pack] m_byIndex Exception, m_byPayload=%d, m_byIndex=%d, m_byPackNum=%d\n",
                pRtpPack->m_byPayload, m_tAudioHeader.m_byIndex, m_tAudioHeader.m_byPackNum);

        return bRet;
    }

    bRet = TRUE;
    return bRet;
}


/*=============================================================================
    ������        ��CheckG7xxPack
    ����        ��g7xx RTP ����Ч�� ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� TRUE - ��Ч���ݰ�
=============================================================================*/
BOOL32 CKdvNetRcv::CheckG7xxPack(TRtpPack *pRtpPack)
{
    BOOL32 bRet = FALSE;

    // ������Ч���ж�
    if( (pRtpPack->m_nRealSize <= 0) ||
        (pRtpPack->m_nRealSize > MAX_SND_PACK_SIZE) )
    {
        m_tRcvStatistics.m_dwPackLose++;
        MedianetLog(LostPack, "[CKdvNetRcv::CheckG7xxPack] m_nRealSize Exception, m_byPayload=%d, m_nRealSize=%d   \n",
                             pRtpPack->m_byPayload, pRtpPack->m_nRealSize);

        return bRet;
    }

    switch(pRtpPack->m_byPayload)
    {
        case MEDIA_TYPE_G7231:
        {
            if( (pRtpPack->m_nRealSize != 4) &&
                (pRtpPack->m_nRealSize != 20) &&
                (pRtpPack->m_nRealSize != 24) )
            {
                MedianetLog(LostPack, "[CKdvNetRcv::CheckG7xxPack] MEDIA_TYPE_G7231 m_nRealSize Exception, m_nRealSize=%d   \n",
                        pRtpPack->m_nRealSize);

                return bRet;
            }

            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_G723_6;
            break;
        }
        case MEDIA_TYPE_PCMU:
        {
            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_PCMU;
            break;
        }
        case MEDIA_TYPE_PCMA:
        {
            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_PCMA;
            break;
        }
        case MEDIA_TYPE_G722:
        {
            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_G722;
            break;
        }
        case MEDIA_TYPE_G728:
        {
            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_G728;
            break;
        }
        case MEDIA_TYPE_G729:
        {
            m_tAudioHeader.m_nMode = NET_AUDIO_MODE_G729;
            break;
        }
        case MEDIA_TYPE_ADPCM:
        {
            m_tAudioHeader.m_nMode = MEDIA_TYPE_ADPCM;
            break;
        }
        case MEDIA_TYPE_G7221C:
        {
            m_tAudioHeader.m_nMode = MEDIA_TYPE_G7221C;
            break;
        }
        case MEDIA_TYPE_AACLC:
        {
            if(pRtpPack->m_byExtence)
            {
                m_tAudioHeader.m_nMode = pRtpPack->m_pExData[EX_FRAMEMODE_POS];
            }
            else
            {
                m_tAudioHeader.m_nMode = 0;
            }

            pRtpPack->m_byPayload = MEDIA_TYPE_AACLC;
            break;
        }
        case MEDIA_TYPE_AACLD:
        {
            m_tAudioHeader.m_nMode = MEDIA_TYPE_AACLD;
            break;
        }
        case MEDIA_TYPE_AACLC_PCM:
        {
            m_tAudioHeader.m_nMode = MEDIA_TYPE_AACLC_PCM;
            break;
        }
        case MEDIA_TYPE_AMR:
        {
            m_tAudioHeader.m_nMode = MEDIA_TYPE_AMR;
            break;
		}
		case MEDIA_TYPE_G726_16:
		{
			m_tAudioHeader.m_nMode = MEDIA_TYPE_G726_16;
			break;
		}
		case MEDIA_TYPE_G726_24:
		{
			m_tAudioHeader.m_nMode = MEDIA_TYPE_G726_24;
			break;
		}
		case MEDIA_TYPE_G726_32:
		{
			m_tAudioHeader.m_nMode = MEDIA_TYPE_G726_32;
			break;
		}
		case MEDIA_TYPE_G726_40:
		{
			m_tAudioHeader.m_nMode = MEDIA_TYPE_G726_40;
			break;
		}
        default:
        {
            return bRet;
        }
    }

    bRet = TRUE;
    return bRet;
}


/*=============================================================================
    ������        ��MyDealH264
    ����        �������յ���h.264���ݰ������ں�̩�õ����ʼ���
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealH264(TRtpPack *pRtpPack)
{

    //xpȥ����Ϊ�˳�֡���յ��߼�����ȷ���˶�ͨ��
//     pRtpPack->m_byMark = FALSE; //hual for tandberg
    //���ӽӿڣ�Ϊ��ͨ�ȵ������˺�ͼ�ȳ��ҵ�������SPS PPSΪ����markλ��
    if (m_bCompFrameByTimeStamp)
    {
        pRtpPack->m_byMark = FALSE;
    }

    //a. ������Ч���ж�
    if(FALSE == CheckH26XPack(pRtpPack))
    {
        MedianetLog(Parse, "[DealH264] step 2 CheckH26XPack return SPS=%d, PPS=%d ... \n",
                             m_tH264Header.m_bIsValidSPS, m_tH264Header.m_bIsValidPPS );
        return;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;

}
//��ȡ��ǰʱ��
u32 CKdvNetRcv::GetCurTime()
{
#ifdef _LINUX_
    return OspTickGet()*(1000/OspClkRateGet());
#else
    //windows�¾�ȷʱ�侫ȷ��ms
    //QueryPerformanceCounter, QueryPerformanceFrequency
    BOOL32 bRet = FALSE;
    u32 dwTime = 0;
    do
    {
        LARGE_INTEGER tFrequency = {0};
        LARGE_INTEGER tNowTime = {0};

        bRet = QueryPerformanceFrequency(&tFrequency);
        if (FALSE == bRet)
        {
            MedianetLog(Time, "QueryPerformanceFrequency error! \n");
            break;
        }

        bRet = QueryPerformanceCounter(&tNowTime);
        if (FALSE == bRet)
        {
            MedianetLog(Time, "QueryPerformanceCounter error! \n");
            break;
        }

        dwTime = (u32)(tNowTime.QuadPart * 1000 / tFrequency.QuadPart);
    } while(0);

    if (FALSE == bRet)
    {
        return OspTickGet()*(1000/OspClkRateGet());
    }
    else
    {
        return dwTime;
    }
#endif
}
//���е�rtp����֡����Ҫ�ߵ������������
/*=============================================================================
    ������        DealRtpPacket
    ����        ������rtp��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 TRtpPack *ptRtpPack   RTP���ݰ��ṹ,�μ�����
    ����ֵ˵������
=============================================================================*/
void CKdvNetRcv::DealRtpPacket(TRtpPack *ptRtpPack)
{

    if (ptRtpPack->m_dwTimeStamp != m_tLastInfo.m_dwTimeStamp)
    {
        ptRtpPack->m_dwRcvTimeStamps = GetCurTime();
    }
    else
    {
        ptRtpPack->m_dwRcvTimeStamps  = m_tLastInfo.m_dwRcvTimeSTamp;
    }

    MedianetLog(Pack, "[CKdvRtp::DealData] RTP Info:m_nRealSize=%d, m_byExtence=%d, m_dwTimeStamp=%d(%d), m_wSequence=%d, m_byPayload=%d, m_dwSSRC=%d, m_byMark=%d   \n",
                         ptRtpPack->m_nRealSize, ptRtpPack->m_byExtence, ptRtpPack->m_dwTimeStamp, ptRtpPack->m_dwRcvTimeStamps, ptRtpPack->m_wSequence, ptRtpPack->m_byPayload, ptRtpPack->m_dwSSRC, ptRtpPack->m_byMark);
    //�����һ���Ƿ�ò���
    if (!CheckIfInsert(ptRtpPack))
    {
        return;
    }

    //���в��봦��
    s32 nNewPackPos;//����һ�����ݷ��ص�λ��
    nNewPackPos = InsertPacket(ptRtpPack);

    if (nNewPackPos < 0) //����ʧ��
    {
        MedianetLog(LostPack, "Insert packet error sn:%d\n", ptRtpPack->m_wSequence);
        return;
    }

    if (INVALID_PACKET_POS == m_nCurFrameStartPos) //��ʼ״̬
    {
        //����Current
        m_nCurFrameStartPos = nNewPackPos;
        if (m_bRepeatSend)
        {
            ResetRSPos();   //���������ش�λ��Pos
        }
    }

    //�����ж�֡�Ƿ�����
    s32 nDistance = nNewPackPos - m_nCurFrameStartPos;
    if (nDistance < 0)
    {
        nDistance += m_nPacketBuffNum;
    }

    //����������п��ܷ�����ǿ��֡�ص���ȥ�Ļ��ᵼ�±������һص���ȥҲû������
    if (nDistance > CURRENT_FRAME_DISCARD_DISTANCE)//���������3/4
    {
        ResetAllPackets();
        return;
        //ProcessCurrentFrame(TRUE);
    }

    //������,����֡���ʱ��
    if ((u16)(m_tLastInfo.m_wSeq + 1) == ptRtpPack->m_wSequence)
    {
        //����һ֮֡���ʱ��
        m_dwFrameTime = ptRtpPack->m_dwRcvTimeStamps  - m_tLastInfo.m_dwRcvTimeSTamp;
        if (m_dwFrameTime > 25 * VIDEO_TIME_SPAN) //�������жϣ�����Сÿ��1֡�������֡���
        {
            m_dwFrameTime = 0;  //��Ҫ���¼���
        }
    }
    else if (m_tLastInfo.m_dwTimeStamp != 0)
    {
        // ����������ʱ�䳬���ش�ʱ�䣬ǿ�ƻص�֡
        //��֡����

        s32 nDiffTime = (s32)(ptRtpPack->m_dwRcvTimeStamps - m_tLastInfo.m_dwRcvTimeSTamp);
        if (nDiffTime > (s32)VIDEO_TIME_SPAN * m_byRSFrameDistance[MAX_RESEND_QUEST_TIMES])
        {
            MedianetLog(Time, "thisTimeStamp = %d, LastTimeStamp = %d \n",ptRtpPack->m_dwRcvTimeStamps, m_tLastInfo.m_dwRcvTimeSTamp);

            ProcessCurrentFrame(TRUE);
        }

        m_dwFrameTime = 0;
    }

    //����һ֡������TRUE˵������֡�߽�
    BOOL32 bGetNewFrame = UpdateFrameNum(nNewPackPos);
    if( bGetNewFrame )
    {
        ProcessCurrentFrame(FALSE);
    }

    //���buff���Ѿ�û�а��ˣ��Ǽ�������ȥҲû��������
    if (0 == m_dwBufPackNum)
    {
        m_tLastInfo.m_dwTimeStamp  = ptRtpPack->m_dwTimeStamp;
        m_tLastInfo.m_wSeq         = ptRtpPack->m_wSequence;
        m_tLastInfo.m_dwRcvTimeSTamp = ptRtpPack->m_dwRcvTimeStamps;
        return;
    }
    //����ʱ������ж�
    if (m_dwFrameTime > 0 && INVALID_PACKET_POS != m_nCurFrameStartPos)
    {
        u32 dwDiffTime = ptRtpPack->m_dwRcvTimeStamps - m_atPackets[m_nCurFrameStartPos].m_dwRcvTS;
        if (dwDiffTime < VIDEO_TIME_SPAN * 250) //����������жϣ��ش�����ʱ��Ŵ�10�룬�˴�Ӧ��Ϊ250֡ modify  by gxk 20100916 ֧������ǰ����Ҫ
        {
            if (dwDiffTime > (u32)VIDEO_TIME_SPAN * m_byRSFrameDistance[MAX_RESEND_QUEST_TIMES])
            {
                ProcessCurrentFrame(TRUE);
            }
        }
        else
        {
            //���ڰ�������ʱ�����̫��
        }
    }

    if (m_nBuffFrameNum > m_nBuffNumTH)
    {
        //������ڵ���2*m_nBuffNumTH, ǿ�ƻص�
        if (ProcessCurrentFrame(FALSE))
        //if (ProcessCurrentFrame(m_nBuffFrameNum >= 4*m_nBuffNumTH))
        {
            //�������Ƶ�����ٻص�һ�Σ��Լ����ۻ�
            if (m_bAudio && m_nBuffFrameNum > m_nBuffNumTH)
            {
                ProcessCurrentFrame(FALSE);
            }
        }
    }
    else if (bGetNewFrame && m_nBuffFrameNum == m_nBuffNumTH)
    {
        //����յ��µ�֡�����лص�
        ProcessCurrentFrame(FALSE);
    }
    else if( m_nBuffNumTH == 1 && m_nBuffFrameNum > 0)
    {
        //zhx [07-08-10]
        //�����ֵΪ1��ÿ֡�ص������Ұѻ����л�ѹ��֡����Ļص����ϲ�
        ProcessCurrentFrame(FALSE);
    }

    //�����ش����
    DealRSCheck(ptRtpPack->m_wSequence, ptRtpPack->m_dwRcvTimeStamps);

    m_tLastInfo.m_dwTimeStamp  = ptRtpPack->m_dwTimeStamp;
    m_tLastInfo.m_wSeq         = ptRtpPack->m_wSequence;
    m_tLastInfo.m_dwRcvTimeSTamp = ptRtpPack->m_dwRcvTimeStamps;

    return;
}

/*=============================================================================
    ������        UpdateFrameNum
    ����        ������һ֡
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                    s32 nNewPackPos   ��Ҫ���µİ���λ��
    ����ֵ˵����TRUE-�ɹ� FALSE-ʧ��
=============================================================================*/
BOOL32 CKdvNetRcv::UpdateFrameNum(s32 nNewPackPos)
{
    BOOL32 bUpdate = FALSE;

    //�ж���һ��λ���Ƿ�����һ֡����������һ֡Mark���
    TPacketInfo* ptPrePacket;
    s32 nPrePos;
    nPrePos = PREVPACKPOS(nNewPackPos);

    ptPrePacket = &m_atPackets[nPrePos];

    //�ж���һ����mark
    if ((ptPrePacket->m_bUsed) &&
        (!ptPrePacket->m_bMark) &&
        (m_atPackets[nNewPackPos].m_wSN == ptPrePacket->m_wSN + 1) &&
        (ptPrePacket->m_dwTS != m_atPackets[nNewPackPos].m_dwTS))
    {

        ptPrePacket->m_bMark = TRUE;
        //֡����1
        m_nBuffFrameNum++;
        m_dwIntervalFrameRecvd++;
        bUpdate = TRUE;
        //OspPrintf(1, 0, "User Set Pre Mark:%u\n", ptPrePacket->m_wSN);
    }

    //�жϱ�����mark
    if (m_atPackets[nNewPackPos].m_bMark)
    {
        m_nBuffFrameNum++;
        m_dwIntervalFrameRecvd++;
        bUpdate = TRUE;
    }
    else
    {
        //ͨ����һ�����жϱ�����mark
        //�ж���һ��λ���Ƿ�����һ֡��������Mark���
        TPacketInfo* ptNextPacket;
        s32 nNextPos;
        nNextPos = NEXTPACKPOS(nNewPackPos);

        ptNextPacket = &m_atPackets[nNextPos];

        if (ptNextPacket->m_bUsed)
        {
            m_tRcvStatistics.m_dwPackIndexError++;  //�����¼
            //�ж���һ��λ���Ƿ�����һ֡�������ñ�֡��Mark���
            if ((ptNextPacket->m_wSN == m_atPackets[nNewPackPos].m_wSN + 1) &&
                (ptNextPacket->m_dwTS != m_atPackets[nNewPackPos].m_dwTS))
            {
                m_atPackets[nNewPackPos].m_bMark = TRUE;
                //֡����1
                m_nBuffFrameNum++;
                m_dwIntervalFrameRecvd++;
                bUpdate = TRUE;
                //OspPrintf(1, 0, "User Set Mark by Next:%u\n", pRtpPack->m_wSequence);
            }
        }
    }

    //zhx ��ʼʱ��2֡Ϊ��ֵ
    if( m_dwLastUpdateTick == 0 )
    {
        m_nBuffNumTH = 2;
        m_dwIntervalFrameRecvd = 0;
        m_dwLastUpdateTick = OspTickGet();
    }
    else
    {
        //֡��ͳ�Ƽ����5s
        static u32 dwTickSec = 5 * OspClkRateGet();
        //��ʱ��û�������ļ���� 10s
        static u32 dwTickPause = 10 * OspClkRateGet();
        u32 dwInter = OspTickGet() - m_dwLastUpdateTick;

        //��ʱ��û������
        if( dwInter > dwTickPause )
        {
            m_nBuffNumTH = 2;
            m_dwIntervalFrameRecvd = 0;
            m_dwLastUpdateTick = 0;
            m_dwLastFrameRecvd = 0;
        }
        else if( dwInter > dwTickSec )
        {
            //��ͳ��֡��С��6ʱ
            if( m_dwLastFrameRecvd < 5 * 6 )
            {
                m_nBuffNumTH = 1;
            }
            else
            {
                m_nBuffNumTH = 2;
            }

            m_dwLastFrameRecvd = m_dwIntervalFrameRecvd;
            m_dwIntervalFrameRecvd = 0;
            m_dwLastUpdateTick = OspTickGet();
        }
    }

    return bUpdate;
}


/*=============================================================================
    ������        ResetAllPackets
    ����        ����հ����У���������״̬
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵������
    ����ֵ˵������
=============================================================================*/
void CKdvNetRcv::ResetAllPackets()
{
    m_nCurFrameStartPos = INVALID_PACKET_POS;

    MedianetLog(Pack, "[PayLoad: %d] Block Buff, FreeBlocks:%u, MinFreeBlocks:%u\n",
                  m_tLastInfo.m_byMediaType, m_cListBuffMgr.GetFreeBlocks(), m_cListBuffMgr.GetMinFreeBlocks());

    s32 i;
    for(i = 0; i < m_nPacketBuffNum; i++)
    {
        //if (m_atPackets[i].m_bUsed)
        if (NULL != m_atPackets[i].m_ptDataBuff)
        {
            m_cListBuffMgr.FreeBuff(m_atPackets[i].m_ptDataBuff);
            m_atPackets[i].m_ptDataBuff = NULL;
        }
        m_atPackets[i].m_bUsed = FALSE;
        m_atPackets[i].m_bMark = FALSE ;
        memset(&m_atPackets[i], 0, sizeof(TPacketInfo));
    }

    m_nSSRCErrCount = 0;
    m_nSNErrCount = 0;

    m_nBuffFrameNum = 0;
    m_dwFrameTime = 0;

    //����֡��ţ��Ա��ϲ��֪��֡
    m_FrmHdr.m_dwFrameID++;
    m_byOldFrameRate = 25;
    m_nMP4PackLen = 0;
    m_nBuffNumTH = 2;
    m_dwLastFrameTimeStamp = 0;
    if (m_hTspsRead)
    {
//        TspsReadResetStream(m_hTspsRead);
        TspsReadClose(m_hTspsRead);

        m_hTspsRead = TspsReadOpen(PROGRAM_STREAM, FrameCallback, (KD_PTR)this, m_dwMaxFrameSize);
        if (NULL == m_hTspsRead)
        {
            OspPrintf(TRUE, FALSE, "TspsReadOpen error! \n");
        }
    }
}

//#define MAX_SSRCERR_COUNT       50
/*=============================================================================
    ������        CheckIfInsert
    ����        ������Ƿ����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 TRtpPack *pRtpPack rtp���ṹ���μ�����
    ����ֵ˵������
=============================================================================*/
BOOL32 CKdvNetRcv::CheckIfInsert(TRtpPack *pRtpPack)
{
    BOOL32 bRet = TRUE;

    //��¼��С����Ϣ��������һ���������ж�

    if ((m_tLastInfo.m_dwSSRC != pRtpPack->m_dwSSRC) ||
        (m_tLastInfo.m_byMediaType != pRtpPack->m_byPayload))
    {
        //��֪��Ϊʲôԭ��ȥ��
        //hual
        /*
        if (m_nSSRCErrCount < MAX_SSRCERR_COUNT)
        {
            m_nSSRCErrCount++;
            return FALSE;
        }
        ��Ҫ���SSRC�����顣�����յ�MAX_SSRCERR_COUNT���������SSRCת��
        */

        //SSRC��ͬ��payload��ͬ������
        ResetAllPackets();

        if (MEDIA_TYPE_H262  == pRtpPack->m_byPayload || MEDIA_TYPE_MP4 == pRtpPack->m_byPayload)
        {
            if (m_bHad4kH264Stream? m_cListBuffMgr.GetBlockSize() < H264_4K_MAX_FRAME_SIZE : m_cListBuffMgr.GetBlockSize() < MAX_VIDEO_FRAME_SIZE)

            {
                u32 nBuffTimeLen;

                if (m_bRepeatSend)
                {
                    nBuffTimeLen = m_tRSParam.m_wRejectTimeSpan; //����ʱ��

                    if (nBuffTimeLen < DEFAULT_BUFF_TIMELEN)
                    {
                        nBuffTimeLen = DEFAULT_BUFF_TIMELEN;
                    }
                    else if (nBuffTimeLen > MAX_BUFF_TIMELEN)
                    {
                        nBuffTimeLen = MAX_BUFF_TIMELEN;
                    }
                }
                else
                {
                    nBuffTimeLen = DEFAULT_BUFF_TIMELEN;
                }

                m_bHad4kH264Stream? m_cListBuffMgr.Create(H264_4K_MAX_FRAME_SIZE, nBuffTimeLen/VIDEO_TIME_SPAN):
            m_cListBuffMgr.Create(MAX_VIDEO_FRAME_SIZE, nBuffTimeLen/VIDEO_TIME_SPAN);
                //m_cListBuffMgr.ReBuild(128*1024);
            }
            //m_cListBuffMgr.ReBuild(m_dwMaxFrameSize);
            //m_cListBuffMgr.ReBuild(128*1024);
        }
        else
        {
            if (m_bAudio)
            {
                if (m_cListBuffMgr.GetBlockSize() > BUFF_BLOCK_LENGTH)
                {
                    //����Ҫ���´�����ֱ�����·��伴��
                    m_cListBuffMgr.ReBuild(BUFF_BLOCK_LENGTH);
                }
            }
            else
            {
                if (m_cListBuffMgr.GetBlockSize() > BUFF_BLOCK_LENGTH*2)
                {
                    //����Ҫ���´�����ֱ�����·��伴��
                    m_cListBuffMgr.ReBuild(BUFF_BLOCK_LENGTH*2);
                }
            }
        }

        MedianetLog(LostPack, "this:0x%x SSRC(%u) Payload(%u) -> SSRC(%u) Payload(%u)\n",
                             this, m_tLastInfo.m_dwSSRC, m_tLastInfo.m_byMediaType, pRtpPack->m_dwSSRC, pRtpPack->m_byPayload);

        m_tLastInfo.m_dwSSRC       = pRtpPack->m_dwSSRC;
        m_tLastInfo.m_byMediaType  = pRtpPack->m_byPayload;

        //recv id set to 0
        m_dwRecvFrmId = 0;
        return TRUE;
    }
//    else
//    {
//        m_nSSRCErrCount = 0;
//    }

    //�����ǰ֡����δȷ����ֱ�Ӳ���
    if (INVALID_PACKET_POS == m_nCurFrameStartPos)
    {
//        m_nSSRCErrCount = 0;
        m_nSNErrCount = 0;
        return TRUE;
    }

    u16 wCurSN = m_atPackets[m_nCurFrameStartPos].m_wSN;

    u16 wDiff = pRtpPack->m_wSequence - wCurSN;

    //500Ϊ����ֵ
    if (wDiff > ((u16)-1) - 500)
    {
        //���ڰ�
        MedianetLog(LostPack, "[PayLoad: %d] Old packet not insert: CurSN=%d, CurTS=%u, ThisSN=%u, ThisTS=%d\n",
                m_tLastInfo.m_byMediaType,
                m_atPackets[m_nCurFrameStartPos].m_wSN,
                m_atPackets[m_nCurFrameStartPos].m_dwTS,
                pRtpPack->m_wSequence,
                pRtpPack->m_dwTimeStamp);

        return FALSE;
    }

    if (wDiff > (u16)m_nPacketBuffNum)
    {
        //�����ʱ�����������ݣ����Բ���
        if ( m_dwBufPackNum == 0 )
        {
            return TRUE;
        }
        //������������
        ProcessCurrentFrame(TRUE);

        MedianetLog(Pack, "[PayLoad: %d] Continue SN Error, Reset recv buff. SN=%d, CurSN=%d\n",
            m_tLastInfo.m_byMediaType, pRtpPack->m_wSequence, wCurSN);

        ResetAllPackets();

//         m_nSNErrCount++;
//         if (m_nSNErrCount > 100) //����SN���󣬽���Reset
//         {
//             OspPrintf(TRUE, FALSE, "[PayLoad: %d] Continue SN Error count=100, Reset recv buff. SN=%d, CurSN=%d\n",
//                 m_tLastInfo.m_byMediaType, pRtpPack->m_wSequence, wCurSN);
//             ResetAllPackets();
//         }
//
//         return FALSE;
    }

/*
    u16 wCurSN;
    u16 wDiff;

    while ( 1)
    {
        wCurSN = m_atPackets[m_nCurFrameStartPos].m_wSN;
        wDiff = pRtpPack->m_wSequence - wCurSN;

        if (wDiff > (u16)m_nPacketBuffNum)
        {
            if ( !ProcessCurrentFrame(FALSE))
            {
                if ( m_dwBufPackNum == 0 )
                {
                    break;
                }
                else
                {
                    //���ڰ�
                    if (17 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
                    {
                        OspPrintf(1, 0, "[PayLoad: %d] Old packet not insert: CurSN=%d, CurTS=%u, ThisSN=%u, ThisTS=%d\n",
                            m_tLastInfo.m_byMediaType,
                            m_atPackets[m_nCurFrameStartPos].m_wSN,
                            m_atPackets[m_nCurFrameStartPos].m_dwTS,
                            pRtpPack->m_wSequence,
                            pRtpPack->m_dwTimeStamp);
                    }

                    m_nSNErrCount++;
                    if (m_nSNErrCount > 100) //����SN���󣬽���Reset
                    {
                        OspPrintf(TRUE, FALSE, "[PayLoad: %d] Continue SN Error count=100, Reset recv buff. SN=%d, CurSN=%d\n",
                            m_tLastInfo.m_byMediaType, pRtpPack->m_wSequence, wCurSN);
                        ResetAllPackets();
                    }

                    return FALSE;
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            break;
        }
    }
*/


    //ʱ���У��
    u32 dwCurTS = m_atPackets[m_nCurFrameStartPos].m_dwTS;
    if (pRtpPack->m_dwTimeStamp < dwCurTS)
    {
        MedianetLog(Time, "Timestamp is less than Cur. TimeStamp=%u SN=%d, CurTS=%u, CurSN=%d\n",
                  pRtpPack->m_dwTimeStamp, pRtpPack->m_wSequence, dwCurTS, wCurSN);
    }

    m_nSNErrCount = 0;

    return bRet;
}

/*=============================================================================
    ������        InsertPacket
    ����        ������Rtp��������λ��
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 TRtpPack *pRtpPack rtp���ṹ���μ�����
    ����ֵ˵��������λ��
=============================================================================*/
s32 CKdvNetRcv::InsertPacket(TRtpPack *pRtpPack)
{
    u16 wH264VideoWidth = 0;
    u16 wH264VideoHight = 0;
    //analysis resolution depended on  extern data in h264 stream.
	if((pRtpPack->m_byExtence && pRtpPack->m_nExSize >= 2 && pRtpPack->m_byPayload == MEDIA_TYPE_H264) || 
		(m_bisRcv4kStream && (pRtpPack->m_byPayload == MEDIA_TYPE_H264||pRtpPack->m_byPayload == MEDIA_TYPE_PS)) )  //��������4k�����
    {
        if(pRtpPack->m_byExtence && pRtpPack->m_nExSize >= 2 && pRtpPack->m_byPayload == MEDIA_TYPE_H264)
		{
            *(u32 *)pRtpPack->m_pExData  = ntohl(*(u32 *)pRtpPack->m_pExData); // trans net order to host
            wH264VideoWidth = (*(u32 *)(&(pRtpPack->m_pExData[0])) & 0x00FFF000) >> 12; // resolution in extend  data
             wH264VideoHight = (*(u32 *)(&(pRtpPack->m_pExData[0])) & 0x00000FFF);
        }

        // 4k  , recreat recv  buf. then initialize.�ϲ��������Ƿ����4k���������ߣ��ӷֱ����н�����4k�����������·�����ڴ�
        if((wH264VideoWidth >= H264_4K_RESOLUTION_WIDTH || wH264VideoHight >= H264_4K_RESOLUTION_HIGHT ||
            m_bisRcv4kStream) && !m_bHad4kH264Stream)
        {
            ResetAllPackets();

            //reset maxframesize value
            m_dwMaxFrameSize = H264_4K_MAX_FRAME_SIZE;

            //reset recv buffer
            u16 wRet;
            if(m_dwTempRcvBufLen * (H264_4K_MAX_FRAME_SIZE/MAX_VIDEO_FRAME_SIZE) <    MAX_BUFF_TIMELEN)
            {
                wRet = CreateBuff(m_dwTempRcvBufLen * (H264_4K_MAX_FRAME_SIZE/MAX_VIDEO_FRAME_SIZE) );
            }
            else
            {
                wRet = CreateBuff(MAX_BUFF_TIMELEN);
            }
             if(KDVFAILED(wRet))
                {
                    FreeBuf();
                     return INVALID_PACKET_POS;
                }

            // free and recreate frame buffer,
            //copy from free function.
            u32 dwH26XHeadLen = 0;
             if( NULL != m_pFrameBuf )
             {
                dwH26XHeadLen = MAX_H263_HEADER_LEN;
                if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
                {
                    dwH26XHeadLen = MAX_H261_HEADER_LEN;
                }
                dwH26XHeadLen += sizeof(u32);
                m_pFrameBuf -= dwH26XHeadLen;
                MEDIANET_SAFE_FREE(m_pFrameBuf)
             }
//             // copy from create function
//              dwH26XHeadLen = MAX_H263_HEADER_LEN;
//             if(MAX_H263_HEADER_LEN < MAX_H261_HEADER_LEN)
//             {
//                 dwH26XHeadLen = MAX_H261_HEADER_LEN;
//             }
//             dwH26XHeadLen += sizeof(u32);
            MEDIANET_MALLOC(m_pFrameBuf, m_dwMaxFrameSize);
            if( NULL == m_pFrameBuf )
            {
                //FreeBuf();
                return INVALID_PACKET_POS;
            }
//            m_pFrameBuf += dwH26XHeadLen;//���ṩH264 4k��ͷ����ƫ�ƣ����ͷ�ʱҲ����������ͷ����ƫ��

            //first frame buff need remalloc
            MEDIANET_SAFE_FREE(m_FirstFrmHdr.m_pData)
            memset(&m_FirstFrmHdr, 0, sizeof(m_FirstFrmHdr));
            m_dwRecvFrmId = 0;

            MEDIANET_MALLOC(m_FirstFrmHdr.m_pData, H264_4K_MAX_FRAME_SIZE)
            if(m_FirstFrmHdr.m_pData == NULL)
            {
                //FreeBuf();
                return INVALID_PACKET_POS;
            }

            //mark
            m_bHad4kH264Stream = true; //only recreate buffer once
            m_cListBuffMgr.m_bHad4kH264Stream = true;

            // erase message when free buf. set at here.
            m_tLastInfo.m_dwSSRC       = pRtpPack->m_dwSSRC;
            m_tLastInfo.m_byMediaType  = pRtpPack->m_byPayload;
        }
    }

    MedianetLog(Other," OUT CKdvNetRcv::InsertPacket m_dwMaxFrameSize=%d, m_bisRcv4kStream=%d  m_bHad4kH264Stream=%d \n", m_dwMaxFrameSize, m_bisRcv4kStream, m_bHad4kH264Stream);

    s32 nNewPacketPos;  //�ð��Ĵ��λ��
    nNewPacketPos = pRtpPack->m_wSequence % m_nPacketBuffNum;

    TPacketInfo* ptNewPacket;

    ptNewPacket = &m_atPackets[nNewPacketPos];
    if (ptNewPacket->m_bUsed)
    {
        if (0 == (m_tRcvStatistics.m_dwPackNum%20))
        {
            MedianetLog(LostPack, "[PayLoad: %d] InsertPacket error. Pos %d Used sn:%u(%u)\n",
                      m_tLastInfo.m_byMediaType, nNewPacketPos, pRtpPack->m_wSequence, ptNewPacket->m_wSN);
        }

        return INVALID_PACKET_POS;
    }

    //���и����������ݰ��ķ�������������Ӧͷ���ݴ���
    s32 nDataOffset = 0;

    switch(pRtpPack->m_byPayload)
    {
    case MEDIA_TYPE_H261:
        {
            //ע��:��ʼ��ʱ��ͽ���
            TH261Header tH261Header;
            u32 dwHeaderLen = 0;
            //h.261 RTP head info ����
            ParasH261Head(pRtpPack->m_pRealData, &dwHeaderLen, &tH261Header);
            if(0 == dwHeaderLen)
            {
                return INVALID_PACKET_POS;
            }
            memcpy(ptNewPacket->m_abyHeader, pRtpPack->m_pRealData, dwHeaderLen);
            nDataOffset = dwHeaderLen;
        }
        break;

    case MEDIA_TYPE_H263:
        {
            //ע��:��ʼ��ʱ��ͽ���
            TH263Header tH263Header;
            u32 dwHeaderLen = 0;
            //h.263 RTP head info ����
            ParasH263Head(pRtpPack->m_pRealData, &dwHeaderLen, &tH263Header);
            if(0 == dwHeaderLen)
            {
                return INVALID_PACKET_POS;
            }
            memcpy(ptNewPacket->m_abyHeader, pRtpPack->m_pRealData, dwHeaderLen);
            nDataOffset = dwHeaderLen;
        }
        break;

    case MEDIA_TYPE_H263PLUS:
        {
            //ע��:��ʼ��ʱ��ͽ���
            TH263PlusHeader tH263PlusHeader;
            u32 dwHeaderLen = 0;
            //h.263 RTP head info ����
            ParasH263PlusHead(pRtpPack->m_pRealData, &dwHeaderLen, &tH263PlusHeader);
            if(0 == dwHeaderLen)
            {
                return INVALID_PACKET_POS;
            }
            memcpy(ptNewPacket->m_abyHeader, pRtpPack->m_pRealData, dwHeaderLen);
            nDataOffset = dwHeaderLen;
        }
        break;

    case MEDIA_TYPE_H264:
        {
            ptNewPacket->m_bKeyFrame = m_tH264Header.m_bIsKeyFrame;
            if(pRtpPack->m_byExtence && pRtpPack->m_nExSize >= 2)    // ��չͷǰ1��u32Ϊ��ߣ���1��u32Ϊ֡�� [1/13/2010 yanhaiquan]
            {
                ptNewPacket->m_byFrameRate = *((u8*)pRtpPack->m_pExData+7);    // ԭ�������: ntohl(*((u32*)pRtpPack->m_pExData+4));

                //��֧��60F/S���ϵ�֡��
                if (ptNewPacket->m_byFrameRate > 60)
                {
                    ptNewPacket->m_byFrameRate = 25;
                }
            }
            else
            {
                ptNewPacket->m_byFrameRate = 25;    // ֻ��1����չu32�ģ�Ĭ��Ϊ25
            }
            //zhx07-07-17 ����һ��rtpͷ����
            memcpy(ptNewPacket->m_abyHeader, &m_tH264Header.m_wWidth, sizeof(u16) );
            memcpy(ptNewPacket->m_abyHeader + sizeof(u16), &m_tH264Header.m_wHeight, sizeof(u16) );
            memcpy(ptNewPacket->m_abyHeader + 2*sizeof(u16), &m_tH264Header.m_bIsValidPPS, sizeof(BOOL32));
            memcpy(ptNewPacket->m_abyHeader + 2*sizeof(u16) + sizeof(BOOL32), &m_tH264Header.m_bIsValidSPS, sizeof(BOOL32));
        }
        break;
    case MEDIA_TYPE_H265:
        {
            ptNewPacket->m_bKeyFrame = m_tH265Header.m_bIsKeyFrame;
            if(pRtpPack->m_byExtence && pRtpPack->m_nExSize >= 2)    // ��չͷǰ1��u32Ϊ��ߣ���1��u32Ϊ֡�� [1/13/2010 yanhaiquan]
            {
                ptNewPacket->m_byFrameRate = *((u8*)pRtpPack->m_pExData+7);    // ԭ�������: ntohl(*((u32*)pRtpPack->m_pExData+4));

                //��֧��60F/S���ϵ�֡��
                if (ptNewPacket->m_byFrameRate > 60)
                {
                    ptNewPacket->m_byFrameRate = 25;
                }
            }
            else
            {
                ptNewPacket->m_byFrameRate = 25;    // ֻ��1����չu32�ģ�Ĭ��Ϊ25
            }
            //zhx07-07-17 ����һ��rtpͷ����
            memcpy(ptNewPacket->m_abyHeader, &m_tH265Header.m_wWidth, sizeof(u16) );
            memcpy(ptNewPacket->m_abyHeader + sizeof(u16), &m_tH265Header.m_wHeight, sizeof(u16) );
            memcpy(ptNewPacket->m_abyHeader + 2*sizeof(u16), &m_tH265Header.m_bIsValidPPS, sizeof(BOOL32));
            memcpy(ptNewPacket->m_abyHeader + 2*sizeof(u16) + sizeof(BOOL32), &m_tH265Header.m_bIsValidSPS, sizeof(BOOL32));
        }
        break;
    case MEDIA_TYPE_H262:
    case MEDIA_TYPE_MP4:
    case MEDIA_TYPE_MJPEG:
        {
            if (pRtpPack->m_nExSize*sizeof(u32) <= MAX_RPT_HEAD_BUFF_LEN)
            {
                //ǰN-1��
                memcpy(ptNewPacket->m_abyHeader, pRtpPack->m_pExData, pRtpPack->m_nExSize*sizeof(u32));
            }
            else
            {
                //���һ��
                memset(ptNewPacket->m_abyHeader, 0, MAX_RPT_HEAD_BUFF_LEN);
            }
        }
        break;
    case MEDIA_TYPE_PS:
        {

        }
        break;
    case MEDIA_TYPE_PCMU:
    case MEDIA_TYPE_PCMA:
    case MEDIA_TYPE_G7231:
    case MEDIA_TYPE_G728:
    case MEDIA_TYPE_G722:
    case MEDIA_TYPE_G729:
    case MEDIA_TYPE_ADPCM:
    case MEDIA_TYPE_G7221C:
    case MEDIA_TYPE_AACLC:
    case MEDIA_TYPE_AACLD:
	case MEDIA_TYPE_AACLC_PCM:
	case MEDIA_TYPE_AMR:
	case MEDIA_TYPE_G726_16:
	case MEDIA_TYPE_G726_24:
	case MEDIA_TYPE_G726_32:
	case MEDIA_TYPE_G726_40:
        {
        }
        break;
    default:
        break;

    }

    if (MEDIA_TYPE_H262 == pRtpPack->m_byPayload || MEDIA_TYPE_MP4 == pRtpPack->m_byPayload || MEDIA_TYPE_MJPEG == pRtpPack->m_byPayload)
    {
        u8 byPackNum;
        u8 byIndex;
        byPackNum = *(pRtpPack->m_pExData + EX_TOTALNUM_POS);
        byIndex  = *(pRtpPack->m_pExData + EX_INDEX_POS);

        if (m_nMP4PackLen != pRtpPack->m_nRealSize && byIndex != byPackNum)
        {
            //���ȱ仯ʱ�������������
            if (byIndex != 1)
            {
                return INVALID_PACKET_POS;
            }

            m_nMP4PackLen = pRtpPack->m_nRealSize;
        }

        //check frame num
        /*
        {
            s32 i;
            s32 nPos = m_nCurFrameStartPos;
            s32 nCount = 0;
            for(i = 0; i < m_nPacketBuffNum; i++)
            {
                if (m_atPackets[nPos].m_bUsed && m_atPackets[nPos].m_bMark)
                {
                    nCount++;
                }
                nPos = NEXTPACKPOS(nPos);
                if (nPos == nNewPacketPos)
                    break;
            }
            if (nCount != m_nBuffFrameNum)
            {
                OspPrintf(TRUE, FALSE, "nCount=%d, m_nBuffFrameNum=%d\n", nCount, m_nBuffFrameNum);
            }
        }
        */

        //ÿ֡���һ������֡����ָ��
        TPacketInfo* ptLastPacket; //����֡�����һ��
        s32 nLastPackPos;//���һ����λ��


        nLastPackPos = nNewPacketPos + byPackNum - byIndex;
        if (nLastPackPos >= m_nPacketBuffNum)
        {
            nLastPackPos -= m_nPacketBuffNum;
        }

        ptLastPacket  = &m_atPackets[nLastPackPos];

        //��������֡�İ�
        if (NULL == ptLastPacket->m_ptDataBuff)
        {
            TListBuffBlock* ptDataBuff;
            ptDataBuff = m_cListBuffMgr.AllocOneBlock();
            if (NULL == ptDataBuff)
            {
                MedianetLog(Pack, "[PayLoad: %d] Data Buff Full\n",
                        m_tLastInfo.m_byMediaType);

                ProcessCurrentFrame(TRUE);
                //���·���
                ptDataBuff = m_cListBuffMgr.AllocOneBlock();
                if (NULL == ptDataBuff)
                {
                    MedianetLog(LostPack, "[PayLoad: %d] ERROR: ReAllocate Buff error\n",
                            m_tLastInfo.m_byMediaType);

                    return INVALID_PACKET_POS;
                }
            }
            ptLastPacket->m_ptDataBuff = ptDataBuff;
            ptLastPacket->m_ptDataBuff->m_dwRealLen = 0;
        }

        u32 dwSavePos = m_nMP4PackLen*(byIndex-1);
        //OspPrintf(TRUE,FALSE,"Index:%d PackLen:%d SavePos:%d\n",byIndex,m_nMP4PackLen,dwSavePos);
        u32 dwLen = dwSavePos + pRtpPack->m_nRealSize;

        //�ۼƳ���
        if (m_bHad4kH264Stream?  dwLen > H264_4K_MAX_FRAME_SIZE : dwLen > MAX_VIDEO_FRAME_SIZE )
        {
            MedianetLog(LostPack, "[PayLoad: %u] Frame Len:%u > %u\n",
                      m_tLastInfo.m_byMediaType, dwLen, m_bHad4kH264Stream? H264_4K_MAX_FRAME_SIZE : MAX_VIDEO_FRAME_SIZE);
            return INVALID_PACKET_POS;
        }

        s8* pRtpData = ptLastPacket->m_ptDataBuff->GetBuffPtr() + dwSavePos;
        //�������ݵ�֡������
        //OspPrintf(TRUE,FALSE,"B:pRtpData:%p------Next:%p\n",ptLastPacket->m_ptDataBuff, ptLastPacket->m_ptDataBuff->m_ptNext);
        memcpy(pRtpData, pRtpPack->m_pRealData, pRtpPack->m_nRealSize);
        //OspPrintf(TRUE,FALSE,"A:pRtpData:%p------Next:%p\n",ptLastPacket->m_ptDataBuff, ptLastPacket->m_ptDataBuff->m_ptNext);
        ptLastPacket->m_ptDataBuff->m_dwRealLen += pRtpPack->m_nRealSize;

    }
    else
    {
        ptNewPacket->m_ptDataBuff = m_cListBuffMgr.WriteBuff(pRtpPack->m_pRealData + nDataOffset,
            pRtpPack->m_nRealSize - nDataOffset);

        if (pRtpPack->m_nRealSize != 0 && NULL == ptNewPacket->m_ptDataBuff)
        {
            MedianetLog(Pack, "[PayLoad: %d] Data Buff Full\n", m_tLastInfo.m_byMediaType);

            //ǿ�ƻص�
            ProcessCurrentFrame(TRUE);

            //���·���
            ptNewPacket->m_ptDataBuff = m_cListBuffMgr.WriteBuff(pRtpPack->m_pRealData + nDataOffset,
                pRtpPack->m_nRealSize - nDataOffset);
            if (NULL == ptNewPacket->m_ptDataBuff)
            {
                MedianetLog(LostPack, "[PayLoad: %d] ERROR: ReAllocate Buff error\n", m_tLastInfo.m_byMediaType);

                return INVALID_PACKET_POS;
            }
        }
    }

    //���ñ�־
    ptNewPacket->m_bUsed = TRUE;
    ptNewPacket->m_wSN = pRtpPack->m_wSequence;
    ptNewPacket->m_dwTS = pRtpPack->m_dwTimeStamp;
    ptNewPacket->m_bMark = pRtpPack->m_byMark;
    ptNewPacket->m_dwRcvTS = pRtpPack->m_dwRcvTimeStamps;
    m_dwBufPackNum++;

    return nNewPacketPos;
}

/*=============================================================================
    ������        ProcessCurrentFrame
    ����        ����ϵ�ǰ����֡�����ݰ������лص�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 BOOL32 bForce �Ƿ�ǿ�ƴ���bForce= FALSE, ���ֶ���ʱ������
    ����ֵ˵����TRUE-�ɹ� FALSE-ʧ��
=============================================================================*/
BOOL32 CKdvNetRcv::ProcessCurrentFrame(BOOL32 bForce)
{
    u32 dwCurFrameTS;
    u32 dwCurFrameRcvTS;
    s32 wLastSN;
    s32 nStartPos;
    s32 nLostPackNum = 0;
    BOOL32    bCanCombine = TRUE;

    if (m_nCurFrameStartPos < 0)
    {
        MedianetLog(VideoFrame, "[PayLoad: %d] CurFrameStartPos error\n", m_tLastInfo.m_byMediaType);

        //�����صĻ��ᵼ�������±�Խ��
        return FALSE;
    }

    if (0 == m_dwBufPackNum)
    {
        return TRUE;
    }
    nStartPos = m_nCurFrameStartPos;

    u16 wLostPackSN = m_atPackets[nStartPos].m_wSN;

    //Ѱ����ʼ��һ����Ч��
    while (!m_atPackets[nStartPos].m_bUsed)
    {
        //������ӡ
        if (bForce)
        {
            MedianetLog(LostPack, "[PayLoad: %d] Lost Pack: SN= %d. Timestamp=%u\n",
                      m_tLastInfo.m_byMediaType, wLostPackSN, 0);
        }

        wLostPackSN++;

        if (nLostPackNum >= m_nPacketBuffNum)
        {
            ResetAllPackets();
            return FALSE;
        }

        nStartPos = NEXTPACKPOS(nStartPos);
        nLostPackNum++;
    }

    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;
    dwCurFrameRcvTS = m_atPackets[nStartPos].m_dwRcvTS;

    s32 nPos = nStartPos;

    //Ѱ�����һ����Ч��
    s32 i;
    for(i = 0; i < m_nPacketBuffNum; i++)
    {
        if (!m_atPackets[nPos].m_bUsed)
        {
            //����
            nLostPackNum++;
            bCanCombine = FALSE ;

            if (bForce)
            {
                MedianetLog(LostPack, "[PayLoad: %d] Lost Pack: SN= %d. Timestamp=%u\n",
                          m_tLastInfo.m_byMediaType, wLostPackSN, dwCurFrameTS);
            }
        }
        //���ַ����ж�֡�Ƿ������1�����λm_bMark�Ƿ���1  2��ʱ����Ƿ�ı�
        //�����֡�����������
        else if (m_atPackets[nPos].m_bMark)
        {
            break;
        }

        s32 nNextPos;
        nNextPos = NEXTPACKPOS(nPos);
        if ((m_atPackets[nNextPos].m_bUsed) &&
            (m_atPackets[nNextPos].m_dwTS != dwCurFrameTS)) //�����һ��ʱ�����ͬ�������
        {
            break;
        }

        nPos = nNextPos;
        wLostPackSN++;
    }

    //û���ҵ�
    if (i == m_nPacketBuffNum)
    {
        m_nCurFrameStartPos = nStartPos;
//        OspPrintf(TRUE, FALSE, "[PayLoad: %d] ERROR:ProcessCurrentFrame is Full.\n", m_tLastInfo.m_byMediaType);
//        ResetAllPackets();
        return FALSE;
    }

    //���¶�[nStartPos, nPos]������֡�ص������ܰ����հ���
    if (nLostPackNum > 0)
    {
        if (!bForce)
        {
            return FALSE;
        }

        m_FrmHdr.m_dwFrameID++; //���ϲ��֪��֡
        if (m_hTspsRead)
        {
            TspsReadResetStream(m_hTspsRead);
        }
        m_tRcvStatistics.m_dwPackLose += nLostPackNum;
    }

    wLastSN = m_atPackets[nPos].m_wSN;

    //��֡��mark���
    if (m_atPackets[nPos].m_bUsed && m_atPackets[nPos].m_bMark)
    {
        //����֡������
        m_nBuffFrameNum--;
    }

    MedianetLog(Other, "[medianet]ProcessCurrentFrame: m_dwLastFrameRecvd:[%d], m_nBuffFrameNum:[%d], m_nBuffNumTH:[%d]\n",
            m_dwLastFrameRecvd, m_nBuffFrameNum, m_nBuffNumTH);

    // ֻ��Force=TRUE�Ż��߹���
    // ��ʱ�����ж���Ҳ�ص���ȥ
    //�ж�����bCanCombine = FALSE �򲻽����ص���ȥ
    switch(m_tLastInfo.m_byMediaType)
    {
    case MEDIA_TYPE_H261:
        {
            if (bCanCombine)
            {
                CompagesH261AndCB(nStartPos, nPos);
            }
        }
        break;

    case MEDIA_TYPE_H263:
        {
            if (bCanCombine)
            {
                CompagesH263AndCB(nStartPos, nPos);
            }

        }
        break;

    case MEDIA_TYPE_H263PLUS:
        {
            if (bCanCombine)
            {
                CompagesH263PlusAndCB(nStartPos, nPos);
            }
        }
        break;

    case MEDIA_TYPE_H264:
        {
            if (bCanCombine)
            {
                CompagesH264AndCB(nStartPos, nPos);
            }
        }
        break;
    case MEDIA_TYPE_H265:
        {
            if (bCanCombine)
            {
                CompagesH265AndCB(nStartPos, nPos);
            }
        }
        break;
    case MEDIA_TYPE_H262:
    case MEDIA_TYPE_MP4:
    case MEDIA_TYPE_MJPEG:
        {
            if (bCanCombine)
            {
                CompagesMpeg4AndCB(nStartPos, nPos);
            }
        }
        break;

    case MEDIA_TYPE_PCMU:
    case MEDIA_TYPE_PCMA:
    case MEDIA_TYPE_G7231:
    case MEDIA_TYPE_G728:
    case MEDIA_TYPE_G722:
    case MEDIA_TYPE_G729:
    case MEDIA_TYPE_MP3:
    case MEDIA_TYPE_ADPCM:
    case MEDIA_TYPE_G7221C:
    case MEDIA_TYPE_AACLC:
    case MEDIA_TYPE_AACLD:
	case MEDIA_TYPE_AACLC_PCM:
	case MEDIA_TYPE_G726_16:
	case MEDIA_TYPE_G726_24:
	case MEDIA_TYPE_G726_32:
	case MEDIA_TYPE_G726_40:
        {
            if (bCanCombine)
            {   
                SendFrameAudio(m_nCurFrameStartPos, nPos);
            }
        }
        break;
    case MEDIA_TYPE_AMR:
        {
            if (bCanCombine)
            {
                SendAMRFrameAudio(m_nCurFrameStartPos, nPos);
            }
        }
		break;
	case MEDIA_TYPE_PS:
		{
			if (bCanCombine)
			{
				CompagesPSAndCB(nStartPos, nPos);
			}
		}
		break;
    default:
        break;
    }

    //���[nStartPos, nPos]֮��Ŀ�
    //OspPrintf(TRUE, FALSE, "Call Back Pack[%d-%d]\n", nStartPos, nPos);
    s32 nNextFramePos = NEXTPACKPOS(nPos);
    while(nStartPos != nNextFramePos)
    {
        if (nStartPos < 0 )
        {
            break;
        }
        if (NULL != m_atPackets[nStartPos].m_ptDataBuff)
        {
            m_cListBuffMgr.FreeBuff(m_atPackets[nStartPos].m_ptDataBuff); //�ͷŸÿ�
            m_atPackets[nStartPos].m_ptDataBuff = NULL;
        }
        if ( m_atPackets[nStartPos].m_bUsed )
        {
            m_dwBufPackNum--;
        }

        m_atPackets[nStartPos].m_bUsed = FALSE;
        m_atPackets[nStartPos].m_bMark = FALSE ;
        nStartPos = NEXTPACKPOS(nStartPos);
    }

    m_nCurFrameStartPos = nNextFramePos;

    //���º�ǰ������ǿյġ���¼ʱ�������ţ����ڰ������ж�
    if (m_atPackets[m_nCurFrameStartPos].m_bUsed == FALSE)
    {
        m_atPackets[m_nCurFrameStartPos].m_dwTS = dwCurFrameTS + 1;
        m_atPackets[m_nCurFrameStartPos].m_dwRcvTS = dwCurFrameRcvTS + 1;
        m_atPackets[m_nCurFrameStartPos].m_wSN =  wLastSN + 1;
    }

    //�����ش���
    if (m_bRepeatSend)
    {
        UpdateRSPos();
    }

    return TRUE;
}

/************************************************************************/
/* 090224 modified for 264 fu-a format                                  */
/************************************************************************/
#define H264_NALU_CUT_MODE_512HEAD  0
#define H264_NALU_CUT_MODE_FLAG0001 1
/*=============================================================================
    ������        CompagesH264AndCB
    ����        ����ϵ�ǰH264��nStartPos��nEndPos�����ݰ��������лص�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 s32 nStartPos ֡��ͷλ��
                 s32 nEndPos   ֡��βλ��

    ����ֵ˵����0 -�ɹ�
=============================================================================*/
s32 CKdvNetRcv::CompagesH264AndCB(s32 nStartPos, s32 nEndPos)
{
    //Ŀǰ��ض�����0001��ʽ���а���֡��֮ǰ�Ǹ���512�ֽ�ͷ����֡��
    u8 byCutMode = H264_NALU_CUT_MODE_FLAG0001;

    u8* pFrameBuff;
    u8* pFrameNALU;
    u8 *pFrameBuffStart;

    u32 dwMaxFrameSize = m_bHad4kH264Stream ? H264_4K_MAX_FRAME_SIZE : MAX_VIDEO_FRAME_SIZE;

    pFrameNALU = m_pFrameBuf;

    u8 flag[4] = {0, 0, 0, 1};
    if (H264_NALU_CUT_MODE_512HEAD == byCutMode)
    {
        pFrameBuffStart = pFrameNALU + MAX_NALU_UNIT_SIZE;
    }
    else if (H264_NALU_CUT_MODE_FLAG0001 == byCutMode)
    {
        pFrameBuffStart = pFrameNALU;
    }
    pFrameBuff = pFrameBuffStart;

    memset(pFrameNALU, 0, MAX_NALU_UNIT_SIZE);

    BOOL32 bIsKeyFrame = FALSE;
    u32 dwCurFrameTS;

    BOOL32 bPackEnd = FALSE;
    BOOL32 bNaluFuStart = FALSE;
    m_dwLastPackLen = 0;

    //��¼��һ������λ�ã����ܻ��ڵ�һ��д��sps��pps��Ϣ
    s32 nTempStartPos = nStartPos;
    //��һ����ӦΪ��Ч�����ɴ�����Ʊ�֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;
    while(TRUE)
    {
        //���������û���յ�sps��pps���ڹؼ�֡��һ����λ�ò���sps��pps��Ϣ
        if(nTempStartPos == nStartPos && m_atPackets[nStartPos].m_bKeyFrame)
        {
            MedianetLog(VideoFrame, "###[CompagesH264AndCB] spsbuf=%8x,bHavesps=%d, pspbuf=%8x,bHavepps=%d ###\n", \
                        m_tKdvSpsPpsInfo.pbySpsBuf, m_tKdvSpsPpsInfo.m_bHaveSps, m_tKdvSpsPpsInfo.pbyPpsBuf, \
                        m_tKdvSpsPpsInfo.m_bHavePps);

            if(m_tKdvSpsPpsInfo.pbySpsBuf && !m_tKdvSpsPpsInfo.m_bHaveSps)
            {
                //����0001���
                memcpy(pFrameBuff, flag, 4);
                pFrameBuff += 4;
                memcpy(pFrameBuff, m_tKdvSpsPpsInfo.pbySpsBuf, m_tKdvSpsPpsInfo.nSpsBufLen);
                pFrameBuff += m_tKdvSpsPpsInfo.nSpsBufLen;

                MedianetLog(VideoFrame,"\n#######SPSBUF###########\n");
                for(u8 i = 0; i < m_tKdvSpsPpsInfo.nSpsBufLen; i++)
                {
                    MedianetLog(VideoFrame,"%x ", m_tKdvSpsPpsInfo.pbySpsBuf[i]);
                }
                MedianetLog(VideoFrame,"\n########SPSBUF-End######\n");
            }
            if(m_tKdvSpsPpsInfo.pbyPpsBuf && !m_tKdvSpsPpsInfo.m_bHavePps)
            {
                //����0001���
                memcpy(pFrameBuff, flag, 4);
                pFrameBuff += 4;
                memcpy(pFrameBuff, m_tKdvSpsPpsInfo.pbyPpsBuf, m_tKdvSpsPpsInfo.nPpsBufLen);
                pFrameBuff += m_tKdvSpsPpsInfo.nPpsBufLen;

                if(g_nShowDebugInfo == 101)
                {
                    OspPrintf(1,0,"\n#######PPSBUF###########\n");
                    for(u8 i = 0; i < m_tKdvSpsPpsInfo.nPpsBufLen; i++)
                    {
                        OspPrintf(1,0,"%x ", m_tKdvSpsPpsInfo.pbyPpsBuf[i]);
                    }
                    OspPrintf(1,0,"\n########PPSBUF-End######\n");
                }
            }
        }

        if (m_atPackets[nStartPos].m_bUsed && m_atPackets[nStartPos].m_ptDataBuff)
        {
            TListBuffBlock *ptPack = m_atPackets[nStartPos].m_ptDataBuff;
            u8 byH264Indicator = ((u8 *)ptPack->GetBuffPtr())[0];
            u8 byFuHeader = ((u8 *)ptPack->GetBuffPtr())[1];
            u32 dwPackLen;

            s32 nCopiedLen = pFrameBuff - pFrameBuffStart;
            if ((u32)nCopiedLen >= m_dwMaxFrameSize - 1500)
            {
                MedianetLog(VideoFrame, "[medianet] rcv frame larger than %d bytes, discard!\n", m_dwMaxFrameSize);
                return -1;
            }

            //���֮ǰ�г��ȣ����µİ�������֮ǰ��nalu����ô��ʾ֮ǰ�İ��������
            if (m_dwLastPackLen > 0)
            {
                if ((byH264Indicator & 0x1f) == 28 && (byFuHeader & 0x80) > 0)
                {
                    bPackEnd = TRUE;
                }

                if ((byH264Indicator & 0x1f) != 28)
                {
                    bPackEnd = TRUE;
                }

                if (bPackEnd)
                {
                    if (H264_NALU_CUT_MODE_512HEAD == byCutMode)
                    {
                        //���ݸ���������������ֽ�lengthΪ����λ���ֽڡ�
                        *pFrameNALU++ = (u8)((m_dwLastPackLen) & 0xff);
                        *pFrameNALU++ = (u8)((m_dwLastPackLen >> 8) & 0xff);
                        *pFrameNALU++ = (u8)((m_dwLastPackLen >> 16) & 0xff);
                        *pFrameNALU++ = (u8)((m_dwLastPackLen >> 24) & 0xff);
                    }

                    m_dwLastPackLen = 0;
                    bPackEnd = FALSE;
                    bNaluFuStart = FALSE;
                }
            }

            //�������Ƭ��
            if ((byH264Indicator & 0x1f) == 28)
            {
                if ((byFuHeader & 0x80) > 0)
                {
                    //��һ����Ƭ�����µõ�H264Indicator
                    if (H264_NALU_CUT_MODE_FLAG0001 == byCutMode)
                    {
                        memcpy(pFrameBuff, flag, 4);
                        pFrameBuff += 4;
                        m_dwLastPackLen += 4;
                    }

                    byH264Indicator = (byH264Indicator & 0xe0) | (byFuHeader & 0x1f);
                    memcpy(pFrameBuff, &byH264Indicator, 1);
                    pFrameBuff += 1;
                    m_dwLastPackLen += 1;

                    bNaluFuStart = TRUE;
                }

                if (bNaluFuStart)
                {
                    dwPackLen = m_cListBuffMgr.ReadBuff2(ptPack, pFrameBuff, 2);
                    if (0 == dwPackLen)
                    {
                        MedianetLog(AllOpen, "ReadBuff2 return 0\n");
                    }
                    pFrameBuff += dwPackLen;
                    m_dwLastPackLen += dwPackLen;
                }

                if ((byFuHeader & 0x40) > 0)
                {
                    // ���һ����Ƭ
                    bNaluFuStart = FALSE;
                    bPackEnd = TRUE;
                }
            }
            //��ͨ��
            else
            {
                if (H264_NALU_CUT_MODE_FLAG0001 == byCutMode)
                {
                    //����0001���
                    memcpy(pFrameBuff, flag, 4);
                    pFrameBuff += 4;
                    m_dwLastPackLen += 4;
                }

                //����һ������
                dwPackLen = m_cListBuffMgr.ReadBuff(ptPack, pFrameBuff , m_dwMaxFrameSize - (u32)(pFrameBuff - m_pFrameBuf));
                if(0 == dwPackLen)
                {
                    MedianetLog(AllOpen, "CompagesH264AndCB ReadBuff failed , Discard Frame TimeStamp = %d!\n", dwCurFrameTS);
                    return 0;
                }
                pFrameBuff += dwPackLen;
                m_dwLastPackLen = dwPackLen;
                bPackEnd = TRUE;
            }

            if (TRUE == m_atPackets[nStartPos].m_bKeyFrame)
            {
                bIsKeyFrame = TRUE;
            }

            if (bPackEnd)
            {
                if (H264_NALU_CUT_MODE_512HEAD == byCutMode)
                {
                   //���ݸ���������������ֽ�lengthΪ����λ���ֽڡ�
                    *pFrameNALU++ = (u8)((m_dwLastPackLen) & 0xff);
                    *pFrameNALU++ = (u8)((m_dwLastPackLen >> 8) & 0xff);
                    *pFrameNALU++ = (u8)((m_dwLastPackLen >> 16) & 0xff);
                    *pFrameNALU++ = (u8)((m_dwLastPackLen >> 24) & 0xff);
                }

                m_dwLastPackLen = 0;
                bPackEnd = FALSE;
            }
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    u16 wWidth = 0;
    u16 wHeight = 0;
    BOOL32 bIsValidPPS = FALSE;
    BOOL32 bIsValidSPS = FALSE;

    memcpy( &wWidth, m_atPackets[nStartPos].m_abyHeader, sizeof(u16) );
    memcpy( &wHeight, m_atPackets[nStartPos].m_abyHeader + sizeof(u16), sizeof(u16) );
    memcpy( &bIsValidPPS, m_atPackets[nStartPos].m_abyHeader + 2*sizeof(u16), sizeof(BOOL32));
    memcpy( &bIsValidSPS, m_atPackets[nStartPos].m_abyHeader + 2*sizeof(u16) + sizeof(BOOL32), sizeof(BOOL32));

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_byFrameRate                = m_atPackets[nStartPos].m_byFrameRate;//25;//cif,Qcif;
    m_FrmHdr.m_tVideoParam.m_wVideoWidth  = wWidth;//m_tH264Header.m_wWidth;  //352
    m_FrmHdr.m_tVideoParam.m_wVideoHeight = wHeight;//m_tH264Header.m_wHeight; //288
    m_FrmHdr.m_byMediaType                = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize                 = pFrameBuff - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp                = dwCurFrameTS;
    m_FrmHdr.m_dwSSRC                     = m_tLastInfo.m_dwSSRC;
    m_FrmHdr.m_tVideoParam.m_bKeyFrame    = bIsKeyFrame;
    m_FrmHdr.m_pData                      = m_pFrameBuf;

    BOOL32 bCallback = FALSE;
    if (m_bNoPPSSPSStillCallback)
    {
        bCallback = TRUE;
    }
    else
    {
        if ((bIsValidPPS) && (bIsValidSPS))
        {
            bCallback = TRUE;
        }
    }

    //��H264��SPS��PPS��������Ч�󣬲ſ��Խ��������ݻص����ϲ�
    MedianetLog(Info, "original timestamp:%d \n", m_FrmHdr.m_dwTimeStamp);

    if(m_dwTimeStampSample >= 1000)
    {
        if( (m_bRcvStart) && (bCallback) &&
        (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack)  && 0 != m_FrmHdr.m_dwDataSize)
        {
            m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);

            MedianetLog(VideoFrame, "[CompagesH264AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_bIsValidPPS=%d, m_bIsValidSPS=%d , m_byFrameRate = %d  m_dwTimeStampSample =%d\n",
                                 m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, bIsValidPPS, bIsValidSPS, m_FrmHdr.m_byFrameRate, \
                                 m_dwTimeStampSample);

			MedianetLog(VideoAndAudioFrame, "[CompagesH264AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_bIsValidPPS=%d, m_bIsValidSPS=%d , m_byFrameRate = %d  m_dwTimeStampSample =%d\n",
				m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, bIsValidPPS, bIsValidSPS, m_FrmHdr.m_byFrameRate, \
                                 m_dwTimeStampSample);

            m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
        }
        else
        {
            MedianetLog(VideoFrame, "h264 not callback. RcvStart=%d IsValidPPS=%d IsValidSPS=%d\n",
                    m_bRcvStart, m_tH264Header.m_bIsValidPPS, m_tH264Header.m_bIsValidSPS);
        }
        return 0;
    }

    if( (m_bRcvStart) && (bCallback) &&
        (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack)  && 0 != m_FrmHdr.m_dwDataSize)
    {
        //δ������Чʱ��������ʴ�ӡ
        MedianetLog(VideoFrame, "[CompagesH264AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_bIsValidPPS=%d, m_bIsValidSPS=%d , m_byFrameRate = %d  m_dwTimeStampSample =%d m_dwSample=%d\n",
                             m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, bIsValidPPS, bIsValidSPS, m_FrmHdr.m_byFrameRate, \
                             m_dwTimeStampSample, m_dwSample);

        if(m_dwRecvFrmId <= 3)
            m_dwRecvFrmId++;

        u32 dwInterval = 0;
        u8 *ptr = NULL;   //save firstframe pbydata ptr.
        switch(m_dwRecvFrmId)
        {
            case 1:
                {
                    ptr = m_FirstFrmHdr.m_pData;
                    m_FirstFrmHdr = m_FrmHdr;
                    m_FirstFrmHdr.m_pData = ptr;

                    if(m_FirstFrmHdr.m_pData && m_FrmHdr.m_pData && m_FirstFrmHdr.m_dwDataSize <= dwMaxFrameSize)
                    {
                        memcpy(m_FirstFrmHdr.m_pData, m_FrmHdr.m_pData, m_FirstFrmHdr.m_dwDataSize);
                    }

                    break;
                }
            case 2:
                {
                    dwInterval = m_FrmHdr.m_dwTimeStamp - m_FirstFrmHdr.m_dwTimeStamp;

                    if(dwInterval % 90)
                    {
                        m_dwSample = 1;
                    }
                    else
                    {
                        m_dwSample = 90;
                    }

                    m_FirstFrmHdr.m_dwTimeStamp /= m_dwSample;
                    m_FrmHdr.m_dwTimeStamp /= m_dwSample;

                    // ��ε�һ֡�͵ڶ�֡
                    m_pFrameCallBackHandler(&m_FirstFrmHdr, (KD_PTR)m_pContext);
                    m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);

                    break;
                }
            default:
                {
                    m_FrmHdr.m_dwTimeStamp /= m_dwSample;
                    m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);

                    break;
                }
        }
    }
    else
    {
        MedianetLog(VideoFrame, "h264 not callback. RcvStart=%d IsValidPPS=%d IsValidSPS=%d\n",
                m_bRcvStart, m_tH264Header.m_bIsValidPPS, m_tH264Header.m_bIsValidSPS);
    }

    return 0;
}

s32 CKdvNetRcv::CompagesH265AndCB(s32 nStartPos, s32 nEndPos)
{
    /*0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |    PayloadHdr (Type=49)       |   FU header   | DONL (cond)   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
    | DONL (cond)   |                                               |
    |-+-+-+-+-+-+-+-+                                               |
    |                         FU payload                            |
    |                                                               |
    |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                               :...OPTIONAL RTP padding        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
    //The structure of FU header
    //+---------------+
    //|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+
    //|S|E|  FuType   |
    //+---------------+
    //The structure of HEVC NAL unit header
    //+---------------+---------------+
    //|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //|F|   Type    |  LayerId  | TID |
    //+-------------+-----------------+

u8* pFrameBuff;
    u8* pFrameNALU;
    u8 *pFrameBuffStart;

    pFrameNALU = m_pFrameBuf;

    u8 flag[4] = {0, 0, 0, 1};
    pFrameBuffStart = pFrameNALU;
    pFrameBuff = pFrameBuffStart;

    memset(pFrameNALU, 0, MAX_NALU_UNIT_SIZE);

    BOOL32 bIsKeyFrame = FALSE;
    u32 dwCurFrameTS;

    BOOL32 bPackEnd = FALSE;
    BOOL32 bNaluFuStart = FALSE;
    m_dwLastPackLen = 0;

    //��һ����ӦΪ��Ч�����ɴ�����Ʊ�֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed && m_atPackets[nStartPos].m_ptDataBuff)
        {
            TListBuffBlock *ptPack = m_atPackets[nStartPos].m_ptDataBuff;
            u8 byH265PayloadHdr1 = ((u8 *)ptPack->GetBuffPtr())[0];
            u8 byH265PayloadHdr2 = ((u8 *)ptPack->GetBuffPtr())[1];
            u8 byFuHeader = ((u8 *)ptPack->GetBuffPtr())[2];
            u32 dwPackLen;

            s32 nCopiedLen = pFrameBuff - pFrameBuffStart;
            if ((u32)nCopiedLen >= m_dwMaxFrameSize - 1500)
            {
                MedianetLog(VideoFrame, "[medianet] rcv frame larger than %d bytes, discard!\n", m_dwMaxFrameSize);
                return -1;
            }

            //���֮ǰ�г��ȣ����µİ�������֮ǰ��nalu����ô��ʾ֮ǰ�İ��������
            if (m_dwLastPackLen > 0)
            {
                if (((byH265PayloadHdr1 & 0x7e)>>1) == 49 && (byFuHeader & 0x80) > 0)
                {
                    bPackEnd = TRUE;
                }

                if (((byH265PayloadHdr1 & 0x7e)>>1) != 49)
                {
                    bPackEnd = TRUE;
                }

                if (bPackEnd)
                {
                    m_dwLastPackLen = 0;
                    bPackEnd = FALSE;
                    bNaluFuStart = FALSE;
                }
            }

            //�������Ƭ��
            if (((byH265PayloadHdr1 & 0x7e)>>1) == 49)
            {
                if ((byFuHeader & 0x80) > 0)
                {
                    //��һ����Ƭ
                    memcpy(pFrameBuff, flag, 4);
                    pFrameBuff += 4;
                    m_dwLastPackLen += 4;

                    byH265PayloadHdr1 = (byH265PayloadHdr1 & 0x81) | ((byFuHeader & 0x3f)<<1);
                    memcpy(pFrameBuff, &byH265PayloadHdr1, 1);
                    pFrameBuff += 1;
                    memcpy(pFrameBuff, &byH265PayloadHdr2, 1);
                    pFrameBuff += 1;
                    m_dwLastPackLen += 2;

                    bNaluFuStart = TRUE;
                }

                if (bNaluFuStart)
                {
                    dwPackLen = m_cListBuffMgr.ReadBuff2(ptPack, pFrameBuff, 3);
                    if (0 == dwPackLen)
                    {
                        MedianetLog(VideoFrame, "ReadBuff2 return 0\n");
                    }
                    pFrameBuff += dwPackLen;
                    m_dwLastPackLen += dwPackLen;
                }

                if ((byFuHeader & 0x40) > 0)
                {
                    // ���һ����Ƭ
                    bNaluFuStart = FALSE;
                    bPackEnd = TRUE;
                }
            }
            //��ͨ��
            else
            {
                //����0001���
                memcpy(pFrameBuff, flag, 4);
                pFrameBuff += 4;
                m_dwLastPackLen += 4;

                //����һ������
                dwPackLen = m_cListBuffMgr.ReadBuff(ptPack, pFrameBuff , m_dwMaxFrameSize - (u32)(pFrameBuff - m_pFrameBuf));
                if(0 == dwPackLen)
                {
                    MedianetLog(VideoFrame, "CompagesH265AndCB ReadBuff failed , Discard Frame TimeStamp = %d!\n", dwCurFrameTS);
                    return 0;
                }
                pFrameBuff += dwPackLen;
                m_dwLastPackLen = dwPackLen;
                bPackEnd = TRUE;
            }

            if (TRUE == m_atPackets[nStartPos].m_bKeyFrame)
            {
                bIsKeyFrame = TRUE;
            }

            if (bPackEnd)
            {
                m_dwLastPackLen = 0;
                bPackEnd = FALSE;
            }
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    u16 wWidth = 0;
    u16 wHeight = 0;
    BOOL32 bIsValidPPS = FALSE;
    BOOL32 bIsValidSPS = FALSE;

    memcpy( &wWidth, m_atPackets[nStartPos].m_abyHeader, sizeof(u16) );
    memcpy( &wHeight, m_atPackets[nStartPos].m_abyHeader + sizeof(u16), sizeof(u16) );
    memcpy( &bIsValidPPS, m_atPackets[nStartPos].m_abyHeader + 2*sizeof(u16), sizeof(BOOL32));
    memcpy( &bIsValidSPS, m_atPackets[nStartPos].m_abyHeader + 2*sizeof(u16) + sizeof(BOOL32), sizeof(BOOL32));

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_byFrameRate                = 25;//cif,Qcif;
    m_FrmHdr.m_tVideoParam.m_wVideoWidth  = wWidth;//m_tH265Header.m_wWidth;  //352
    m_FrmHdr.m_tVideoParam.m_wVideoHeight = wHeight;//m_tH265Header.m_wHeight; //288
    m_FrmHdr.m_byMediaType                = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize                 = pFrameBuff - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp                = dwCurFrameTS;
    m_FrmHdr.m_dwSSRC                     = m_tLastInfo.m_dwSSRC;
    m_FrmHdr.m_tVideoParam.m_bKeyFrame    = bIsKeyFrame;
    m_FrmHdr.m_pData                      = m_pFrameBuf;
    //��һ֡���߻������ù�֮��֡��Ĭ��Ϊ25
    if (0 != m_dwLastFrameTimeStamp)
    {
        //֡ID����
        if (1 == m_FrmHdr.m_dwFrameID - m_dwLastFrameID && dwCurFrameTS != m_dwLastFrameTimeStamp)
        {
            if (0 != (dwCurFrameTS - m_dwLastFrameTimeStamp)/90)
            {
                m_FrmHdr.m_byFrameRate = 1020/((dwCurFrameTS - m_dwLastFrameTimeStamp)/90);
            }
            else
            {
                m_FrmHdr.m_byFrameRate = m_byOldFrameRate;
            }
        }
        else//֡ID��������֡�������ϵ�
        {
            m_FrmHdr.m_byFrameRate = m_byOldFrameRate;
        }
    }
    m_dwLastFrameID = m_FrmHdr.m_dwFrameID;
    m_byOldFrameRate = m_FrmHdr.m_byFrameRate;
    m_dwLastFrameTimeStamp = dwCurFrameTS;
    BOOL32 bCallback = FALSE;

    if ((bIsValidPPS) && (bIsValidSPS))
    {
        bCallback = TRUE;
    }


	if(m_dwTimeStampSample >= 1000)
	{
		m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
	}
    else
    {
        m_FrmHdr.m_dwTimeStamp /= 90;
    }


    if( (m_bRcvStart) && (bCallback) &&
        (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack)  && 0 != m_FrmHdr.m_dwDataSize)
    {
        // frame call back
        MedianetLog(VideoFrame, "[CompagesH265AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_bIsValidPPS=%d, m_bIsValidSPS=%d , m_byFrameRate = %d  m_dwTimeStampSample =%d\n",
			m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, bIsValidPPS, bIsValidSPS, m_FrmHdr.m_byFrameRate, \
			m_dwTimeStampSample);
		
		MedianetLog(VideoAndAudioFrame, "[CompagesH265AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_bIsValidPPS=%d, m_bIsValidSPS=%d , m_byFrameRate = %d  m_dwTimeStampSample =%d\n",
			m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, bIsValidPPS, bIsValidSPS, m_FrmHdr.m_byFrameRate, \
			m_dwTimeStampSample);

        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }
    else
    {
        MedianetLog(VideoFrame, "h265 not callback. RcvStart=%d IsValidPPS=%d IsValidSPS=%d\n",
                m_bRcvStart, m_tH265Header.m_bIsValidPPS, m_tH265Header.m_bIsValidSPS);
    }

    return 0;
}

u16 CKdvNetRcv::SetNoPPSSPSStillCallback(BOOL32 bNoPPSSPSStillCallback)
{
    m_bNoPPSSPSStillCallback = bNoPPSSPSStillCallback;

    return MEDIANET_NO_ERROR;
}

//��nStartPos��nEndPos�����ݰ��������лص�
//nStartPos��nEndPos�ֱ���֡��ͷλ�ú�βλ�ã�֡��ȷ���ɵ��ú�������
s32 CKdvNetRcv::CompagesH263AndCB(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //��һ����ӦΪ��Ч�������ⲿ��֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    BOOL32 bSaveHeadList = TRUE; //�Ƿ�Ϻ��洢����������ͷ��Ϣ
    memset(&m_tH263HeaderList, 0, sizeof(m_tH263HeaderList));


    u8* pbyCurBuffPos = m_pFrameBuf;
    u8  byLastEndBit = 0;

    TH263Header tH263Header;
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwHeaderLen = 0;

            memset(&tH263Header, 0, sizeof(tH263Header));
            //h.263 RTP head info ����
            ParasH263Head(m_atPackets[nStartPos].m_abyHeader, &dwHeaderLen, &tH263Header);
            if(0 == dwHeaderLen)
            {
                return bRet;
            }

            u8 byLastByte;
            if (byLastEndBit != 0)
            {
                if ((byLastEndBit + tH263Header.sBit) != 8)
                {
                    MedianetLog(VideoFrame, "h263 byLastEndBit + tH263Header.sBit) != 8 \n");
                }
                pbyCurBuffPos--;             //����һ���ֽ�
                byLastByte = *pbyCurBuffPos & (0xff << byLastEndBit); //������һ�������һ���ֽ�
            }

            u32 dwPackLen;

            //�������ݰ���ָ��λ��, ������ʽͷ��rtp����
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, pbyCurBuffPos);

            if (byLastEndBit != 0)
            {
                //ƴ�ӵ�һ���ֽ�
                u8 tmp = (u8)(*pbyCurBuffPos) & (0xff>>(8 - byLastEndBit));
                *pbyCurBuffPos = byLastByte + tmp; //������һ�������λ
            }

            if(TRUE == bSaveHeadList)
            {
                //����C MODE ������ʱ��������
                if(3*sizeof(u32) == dwHeaderLen)
                {
                    bSaveHeadList = FALSE;
                    memset(&m_tH263HeaderList, 0, sizeof(m_tH263HeaderList));
                }

                if(TRUE == bSaveHeadList)
                {
                    SaveH263HeadList(m_atPackets[nStartPos].m_abyHeader, dwPackLen, byLastEndBit, dwHeaderLen);
                }
            }

            pbyCurBuffPos += dwPackLen;
            byLastEndBit = tH263Header.eBit;
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;


    m_FrmHdr.m_byFrameRate = 25;//cif,Qcif;
    m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize  = pbyCurBuffPos - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
    m_FrmHdr.m_pData       = m_pFrameBuf;
    m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;
    //m_FrmHdr.m_tVideoParam.m_bKeyFrame = !tH263Header.i;

    //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
    if (m_dwTimeStampSample >= 1000)
    {
        m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
    }
    else
    {
        m_FrmHdr.m_dwTimeStamp /= VIDEO_TIMESTAMP_SPAN;
    }

    //����plycom������������ H.263֡��ߡ��ؼ�֡ ����Ϣδ�����غ�ͷ����ȷ��¼
    //�ʸ�Ϊ���غ���������ȡ H.263֡��ߡ��ؼ�֡ ����Ϣ
    BOOL32 bPasRet = GetH263PicInfo(m_pFrameBuf, m_FrmHdr.m_dwDataSize,
                                    &m_FrmHdr.m_tVideoParam.m_bKeyFrame,
                                    &m_FrmHdr.m_tVideoParam.m_wVideoWidth,
                                    &m_FrmHdr.m_tVideoParam.m_wVideoHeight );
    if( FALSE == bPasRet )
    {
        MedianetLog(Error, "[CompagesH263AndCB] Exception, GetH263PicInfo Err   \n");
        return bPasRet;
    }

    //����rtplist ͷ��Ϣ
    u8 *pExHeadBuf = m_pFrameBuf - MAX_H263_HEADER_LEN;
    memcpy(pExHeadBuf, &m_tH263HeaderList, MAX_H263_HEADER_LEN);
    MedianetLog(VideoFrame, "[CompagesH263AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d   \n",
            m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth);

    //�ص����ϲ�
    if( (m_bRcvStart) && (!tH263Header.p) &&
        (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack) )//������B֡
    {
        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }

    return 0;
}



//��nStartPos��nEndPos�����ݰ��������лص�
//nStartPos��nEndPos�ֱ���֡��ͷλ�ú�βλ�ã�֡��ȷ���ɵ��ú�������
s32 CKdvNetRcv::CompagesH263PlusAndCB(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //��һ����ӦΪ��Ч�������ⲿ��֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    BOOL32 bSaveHeadList = TRUE; //�Ƿ�Ϻ��洢����������ͷ��Ϣ
    memset(&m_tH263HeaderList, 0, sizeof(m_tH263HeaderList));


    u8* pbyCurBuffPos = m_pFrameBuf;
    u8  byLastEndBit = 0;

    TH263PlusHeader tH263PlusHeader;
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwHeaderLen = 0;

            memset(&tH263PlusHeader, 0, sizeof(tH263PlusHeader));
            //h.263 RTP head info ����
            ParasH263PlusHead(m_atPackets[nStartPos].m_abyHeader, &dwHeaderLen, &tH263PlusHeader);
            if(0 == dwHeaderLen)
            {
                return bRet;
            }

            u32 dwPackLen;
            u8* pbyLastPackPos = pbyCurBuffPos;


            //�Ƿ���� PSC/GSC�����ֽڵĴ���ʱ�ĺ���
            if (1 == tH263PlusHeader.pBit )
            {
                memset(pbyCurBuffPos, 0, 2);
                pbyCurBuffPos += 2;
            }

            //�������ݰ���ָ��λ��, ������ʽͷ��rtp����
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, pbyCurBuffPos);
            pbyCurBuffPos += dwPackLen;

            if (TRUE == bSaveHeadList)
            {
                SaveH263PlusHeadList(m_atPackets[nStartPos].m_abyHeader, pbyCurBuffPos - pbyLastPackPos);
            }
        }


        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_byFrameRate = 25;//cif,Qcif;
    m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize  = pbyCurBuffPos - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
    m_FrmHdr.m_pData       = m_pFrameBuf;
    m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;
    //m_FrmHdr.m_tVideoParam.m_bKeyFrame = !tH263Header.i;

    //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
    if (m_dwTimeStampSample >= 1000)
    {
        m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
    }
    else
    {
        m_FrmHdr.m_dwTimeStamp /= VIDEO_TIMESTAMP_SPAN;
    }

    //����plycom������������ H.263֡��ߡ��ؼ�֡ ����Ϣδ�����غ�ͷ����ȷ��¼
    //�ʸ�Ϊ���غ���������ȡ H.263֡��ߡ��ؼ�֡ ����Ϣ
    BOOL32 bPasRet = GetH263PicInfo(m_pFrameBuf, m_FrmHdr.m_dwDataSize,
                                    &m_FrmHdr.m_tVideoParam.m_bKeyFrame,
                                    &m_FrmHdr.m_tVideoParam.m_wVideoWidth,
                                    &m_FrmHdr.m_tVideoParam.m_wVideoHeight );
    if( FALSE == bPasRet )
    {
        MedianetLog(Error, "[CompagesH263AndCB] Exception, GetH263PicInfo Err   \n");
        return bPasRet;
    }

    //����rtplist ͷ��Ϣ
    u8 *pExHeadBuf = m_pFrameBuf - MAX_H263_HEADER_LEN;
    memcpy(pExHeadBuf, &m_tH263HeaderList, MAX_H263_HEADER_LEN);

    MedianetLog(VideoFrame, "[CompagesH263AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d   \n",
            m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth);

    //�ص����ϲ�
    if( (m_bRcvStart) && (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack) )
    {
        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }

    return 0;
}





//��nStartPos��nEndPos�����ݰ��������лص�
//nStartPos��nEndPos�ֱ���֡��ͷλ�ú�βλ�ã�֡��ȷ���ɵ��ú�������
s32 CKdvNetRcv::CompagesH261AndCB(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //��һ����ӦΪ��Ч�������ⲿ��֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    BOOL32 bSaveHeadList = TRUE; //�Ƿ�Ϻ��洢����������ͷ��Ϣ
    memset(&m_tH261HeaderList, 0, sizeof(m_tH261HeaderList));


    u8* pbyCurBuffPos = m_pFrameBuf;
    u8  byLastEndBit = 0;

    TH261Header tH261Header;
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwHeaderLen = 0;

            memset(&tH261Header, 0, sizeof(tH261Header));
            //h.261 RTP head info ����
            ParasH261Head(m_atPackets[nStartPos].m_abyHeader, &dwHeaderLen, &tH261Header);
            if(0 == dwHeaderLen)
            {
                return bRet;
            }

            u8 byLastByte;
            if (byLastEndBit != 0)
            {
                pbyCurBuffPos--;             //����һ���ֽ�
                byLastByte = *pbyCurBuffPos & (0xff << byLastEndBit); //������һ�������һ���ֽ�
            }

            u32 dwPackLen;
            u8 *pPackDataBuf = NULL;

            //�������ݰ���ָ��λ��, ������ʽͷ��rtp����
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, pbyCurBuffPos);

            if (byLastEndBit != 0)
            {
                //ƴ�ӵ�һ���ֽ�
                u8 tmp = (u8)(*pbyCurBuffPos) & (0xff>>(8 - byLastEndBit));
                *pbyCurBuffPos = byLastByte + tmp; //������һ�������λ
            }

            if(TRUE == bSaveHeadList)
            {
                SaveH261HeadList(m_atPackets[nStartPos].m_abyHeader, dwPackLen, byLastEndBit);
            }

            pbyCurBuffPos += dwPackLen;
            byLastEndBit = tH261Header.eBit;
        }


        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    BOOL32 bPasRet = Stdh261_H261Analyzer(m_pFrameBuf,
        &m_FrmHdr.m_tVideoParam.m_wVideoWidth,
        &m_FrmHdr.m_tVideoParam.m_wVideoHeight);
    if (TRUE != bPasRet)
    {
        //����֡ͷ����������֡
        MedianetLog(Error, "[CompagesH261AndCB] Exception, GetH261PicInfo Err. timestamp=%u\n",
                m_FrmHdr.m_dwTimeStamp);
    }

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_FrmHdr.m_dwFrameID++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_byFrameRate = 25;//cif
    //m_FrmHdr.m_tVideoParam.m_wVideoWidth  = 352;//cif
    //m_FrmHdr.m_tVideoParam.m_wVideoHeight = 288;//cif
    m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize  = pbyCurBuffPos - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
    m_FrmHdr.m_pData       = m_pFrameBuf;
    m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;
    m_FrmHdr.m_tVideoParam.m_bKeyFrame = tH261Header.i;

    //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
    if (m_dwTimeStampSample >= 1000)
    {
        m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
    }
    else
    {
        m_FrmHdr.m_dwTimeStamp /= VIDEO_TIMESTAMP_SPAN;
    }

    //����rtplist ͷ��Ϣ
    u8 *pExHeadBuf = m_pFrameBuf - MAX_H261_HEADER_LEN;
    memcpy(pExHeadBuf, &m_tH261HeaderList, MAX_H261_HEADER_LEN);

    //�����һ֡���ݲ����ֽڶ���ʱ��
    //���h261 �涨����������00000001111,ֱ���ֽڶ���
    if( (tH261Header.eBit > 0) &&
        (tH261Header.eBit < 8) &&
        (m_FrmHdr.m_dwDataSize > 0) &&
        (m_FrmHdr.m_dwDataSize < (m_dwMaxFrameSize - sizeof(byH261End))) )
    {
        u8 *pBuf = m_FrmHdr.m_pData + m_FrmHdr.m_dwDataSize - 1;

        u8 byBit = tH261Header.eBit;

        //��ʹ�õİ�λ���0
        *pBuf &= (0xff<<byBit);

        s32 nPos;
        for(nPos =0; nPos<sizeof(byH261End); nPos++)
        {
            if(nPos > 0) (*(pBuf + nPos)) = (*(byH261End + nPos - 1)) << byBit;
            (*(pBuf + nPos)) += (((*(byH261End + nPos)) >> (8 - byBit)));

            if(*(pBuf + nPos) == 0x0f) break;//�Ѿ��ֽڶ���
        }
        m_FrmHdr.m_dwDataSize += nPos;

    }

    MedianetLog(VideoFrame, "[CompagesH261AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d   \n",
            m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth);

    //�ص����ϲ�
    if( (m_bRcvStart) && (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack) )
    {
        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }

    return 0;
}

/*=============================================================================
    ������        CompagesH264AndCB
    ����        ����ϵ�ǰMPEG4��nStartPos��nEndPos�����ݰ��������лص�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 s32 nStartPos ֡��ͷλ��
                 s32 nEndPos   ֡��βλ��

    ����ֵ˵����0 -�ɹ�
=============================================================================*/
s32 CKdvNetRcv::CompagesMpeg4AndCB(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //ʵ��֡���ݰ���Ϊ�գ��˳�
    if ((!m_atPackets[nStartPos].m_bUsed) ||
        (NULL == m_atPackets[nEndPos].m_ptDataBuff))
    {
        //����֡ID
        m_FrmHdr.m_dwFrameID++;
        m_tRcvStatistics.m_dwFrameNum++;
        return 0;
    }

    //�жϵ�һ���Ƿ�Ϊ��ʼ��

    {
        u8* pbyExData = m_atPackets[nStartPos].m_abyHeader;
        u8 byPackNum = pbyExData[EX_TOTALNUM_POS];
        u8 byIndex  = pbyExData[EX_INDEX_POS];
        if (1 != byIndex)
        {
            m_FrmHdr.m_dwFrameID++;
            m_tRcvStatistics.m_dwFrameNum++;

//            OspPrintf(TRUE, FALSE, "Index Error.\n");
            return 0;
        }

        s32 nDistance;
        nDistance = nEndPos - nStartPos + 1;
        if (nDistance < 0)
            nDistance += m_nPacketBuffNum;
        if (byPackNum != nDistance )
        {
            m_FrmHdr.m_dwFrameID++;
            m_tRcvStatistics.m_dwFrameNum++;
            MedianetLog(Error, "Pack Num Error.\n");
            return 0;
        }
    }

    //���±�֤nStartPos����Ч
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    //�����߲�����������֮ǰ���й��ж�
    if (NULL == m_atPackets[nEndPos].m_ptDataBuff)
    {
        MedianetLog(Error, "Mpeg4 Buff Error\n");
        return 0;
    }

    m_FrmHdr.m_pData       = (u8*)m_atPackets[nEndPos].m_ptDataBuff->GetBuffPtr();
    m_FrmHdr.m_dwDataSize  = m_atPackets[nEndPos].m_ptDataBuff->m_dwRealLen;

    //Ϊʲô������ֱ�Ӵ�m_atPackets[nEndPos]�л�ȡ������
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed && m_atPackets[nStartPos].m_bMark)
        {
            u8* pbyExData = m_atPackets[nStartPos].m_abyHeader;

            m_FrmHdr.m_byFrameRate     = (s32)(pbyExData[EX_FRAMERATE_POS]);
            m_FrmHdr.m_dwFrameID =  ntohl(*((u32 *)(pbyExData + EX_FRAMEID_POS)));
            m_FrmHdr.m_tVideoParam.m_wVideoWidth    = ntohs(*((u16 *)(pbyExData + EX_WIDTH_POS)));
            m_FrmHdr.m_tVideoParam.m_wVideoHeight   = ntohs(*((u16 *)(pbyExData + EX_HEIGHT_POS)));

            m_FrmHdr.m_tVideoParam.m_bKeyFrame    = pbyExData[EX_FRAMEMODE_POS];
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_dwPreBufSize    = 0;
    m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
    m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;

    //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
    if (m_dwTimeStampSample >= 1000)
    {
        m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
    }

    MedianetLog(VideoFrame, "[CompagesMpeg4AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_byFrameRate = %d \n",
		m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame,
		m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, m_FrmHdr.m_byFrameRate);

	MedianetLog(VideoAndAudioFrame, "[CompagesMpeg4AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d, m_wVideoHeight=%d, m_byFrameRate = %d \n",
		m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame,
            m_FrmHdr.m_tVideoParam.m_wVideoWidth, m_FrmHdr.m_tVideoParam.m_wVideoHeight, m_FrmHdr.m_byFrameRate);

    //�ص����ϲ�
    if( (m_bRcvStart) && (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack) )
    {
        m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
    }

    return 0;
}


//��С������
/*
s32 CKdvNetRcv::CompagesMpeg4AndCB(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //ʵ��֡���ݰ���Ϊ�գ��˳�
    if (!m_atPackets[nStartPos].m_bUsed)
    {
        //����֡ID
        m_FrmHdr.m_dwFrameID++;
        m_tRcvStatistics.m_dwFrameNum++;
        return 0;
    }

    //���±�֤nStartPos����Ч
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    u8* pbyCurBuffPos = m_pFrameBuf;


    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwHeaderLen = 0;
            u8  byPackNum;
            u8  byIndex;
            u32 dwPackLen;

            // ��ÿ��RTP����չ���� - ��ȡ��ƵС��������Ϣ
            byPackNum  = *(m_atPackets[nStartPos].m_abyHeader + EX_TOTALNUM_POS);
            byIndex  = *(m_atPackets[nStartPos].m_abyHeader + EX_INDEX_POS);

            pbyCurBuffPos = m_pFrameBuf + MAX_EXTEND_PACK_SIZE*(byIndex-1);

            //�������ݰ���ָ��λ��, ������ʽͷ��rtp����
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, pbyCurBuffPos);
            if (m_atPackets[nStartPos].m_bMark)
            {
                u8* pbyExData = m_atPackets[nStartPos].m_abyHeader;

                m_FrmHdr.m_byFrameRate     = (s32)(pbyExData[EX_FRAMERATE_POS]);
                m_FrmHdr.m_dwFrameID =  ntohl(*((u32 *)(pbyExData + EX_FRAMEID_POS)));
                m_FrmHdr.m_tVideoParam.m_wVideoWidth    = ntohs(*((u16 *)(pbyExData + EX_WIDTH_POS)));
                m_FrmHdr.m_tVideoParam.m_wVideoHeight   = ntohs(*((u16 *)(pbyExData + EX_HEIGHT_POS)));

                m_FrmHdr.m_tVideoParam.m_bKeyFrame    = pbyExData[EX_FRAMEMODE_POS];
            }

            pbyCurBuffPos += dwPackLen;
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }



    //����֡id��¼
    m_tRcvStatistics.m_dwFrameNum++;
    m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;

    m_FrmHdr.m_dwPreBufSize    = 0;
    m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;
    m_FrmHdr.m_dwDataSize  = pbyCurBuffPos - m_pFrameBuf;
    m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
    m_FrmHdr.m_pData       = m_pFrameBuf;
    m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;


    if(8 == g_nShowDebugInfo || 255 == g_nShowDebugInfo)
    {
        OspPrintf(1, 0, "[CompagesMpeg4AndCB] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_bKeyFrame=%d, m_wVideoWidth=%d   \n",
            m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID, m_FrmHdr.m_dwDataSize, m_FrmHdr.m_tVideoParam.m_bKeyFrame, m_FrmHdr.m_tVideoParam.m_wVideoWidth);
    }

    //�ص����ϲ�
    if( (m_bRcvStart) && (m_pFrameCallBackHandler != NULL) && (g_bInterRcvCallBack) )
    {
        m_pFrameCallBackHandler(&m_FrmHdr, m_dwContext);
    }

    return 0;
}
*/

s32 CKdvNetRcv::SendAMRFrameAudio(s32 nStartPos, s32 nEndPos)
{
    BOOL32 bRet = FALSE;

    u32 dwCurFrameTS;

    //��һ����ӦΪ��Ч���������洦����Ʊ�֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    u8* pbyCurBuffPos = m_pFrameBuf;


    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwPackLen;

            //�������ݰ���ָ��λ��
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, m_pFrameBuf);

            m_FrmHdr.m_byAudioMode = m_tAudioHeader.m_nMode;
            m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;

//            m_FrmHdr.m_dwDataSize  = dwPackLen;
//            m_FrmHdr.m_pData       = m_pFrameBuf;

            m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
            m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;

            /*////////////////////////////////////////////////////////////////////
            �μ�RFC4867
            +--------------+------------------+---------------+
            |payload header|table of contents | speech data   |
            +--------------+------------------+---------------+
            ��Ϊ��ʡ����ģʽ���ֽڶ���ģʽ����������ģʽ���ϲ�ͨ������Э��
            Ŀǰ���õ����ֽڶ���ģʽ��ע�����ձ�׼�������ֽڶ���ģʽ��rtpͷ�е�PӦ��Ϊ1��ǰ��δ�
            payload headerΪ4λ��CMR(codec mode request)����ģʽ����ȡֵ��ΧΪ0-7����ʾ8������ģʽ��
            ȡֵ15��ζ�ŵ�ǰ��û��ָ���ĸ�ģʽ������
            ��ʡ����ģʽ��
            0 1 2 3
            +------+
            | CMR  |
            +------+

            0 1 2 3 4 5
            +----------+
            |F|  FT  |Q|
            +----------+
            ��ͨ����֡
            0               1               2               3
            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            +--------------------------------------------------------------+
            |CMR=15|0| FT=4  |1|d(0)                                       |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                    d(147)|P|P|
            +--------------------------------------------------------------+
            ��ͨ����֡
            0               1               2               3
            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            +--------------------------------------------------------------+
            |CMR=15|0| FT=0  |1|1| FT=9  |1|1| FT=15 |1|0| FT=1  |1|d(0)   |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                        d(131)|
            +--------------------------------------------------------------+
            |g(0)                                                          |
            +--------------------------------------------------------------+
            |        g(39)|h(0)                                            |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                                              |
            +--------------------------------------------------------------+
            |                                          h(176)|P|P|P|P|P|P|P|
            +--------------------------------------------------------------+

            �ֽڶ���ģʽ��
            0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
            +--------------+---------------------+
            |CMR   |R|R|R|R|ILL      |ILP        |
            +--------------+---------------------+
            RΪ�����ֶΣ����ֽ�ʡ����ģʽ����Щ����λ����֮һ�����ֽڶ��롣
            ILL��ILPΪ��ѡ�������ILL��ILP���������ϸ��ӣ���ʱ�����ǡ�
            �غ����ݱ�ToC(table of contents)

            +-------------------+
            |list of ToC entries|
            +-------------------+
            |list of frame CRCs | (Optional)
            +-------------------+

            0 1 2 3 4 5 6 7
            +--------------+
            |F|FT    |Q|P|P|
            +--------------+
            F:���ڱ�־�Ƿ����һ��֡��0��ʾ���1֡.����Ƕ�֡���ò���Toc�Ż��ж������ֻ��1�
            FT����־����֡��������ģʽ��������־��ģʽ��ȡֵ��ΧͬCMRȡֵ
            FT=15��ʾû�е�ǰ֡û�о��ɣ�ȡֵ10-13��֡Ҫ������
            Q:ָʾ֡������Ϊ0��ʾ��Ӧ֡���ƻ���1��ʾδ���ƻ��������֡�ѱ��ƻ�����ô����ͨ��ֱ�Ӷ�����֡������
            p�����ֽڶ��롣
            speech data��������
            ÿ֡���ݵ�������ToCÿһ���Ӧ���������ݳ���ȡ���ڶ�ӦģʽToC���е�FT��ʶ��ģʽ��
            �ص����ϲ��һ֡������speech data��Ӧ��ToC��speech data���

            0               1               2               3
            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            +--------------------------------------------------------------+
            |CMR=6 |R|R|R|R|1| FT#1=5|Q|P|P|0| FT#2=5|Q|P|P|f1(0..7)       |
            +--------------------------------------------------------------+
            | f1(8..15)    | f1(16..23)    | ....                          |
            +--------------------------------------------------------------+
            :  ....                                                        :
            +--------------------------------------------------------------+
            |                         .... |f1(152..158) |P|f2(0..7)       |                                                   |
            +--------------------------------------------------------------+
            |                                                        d(131)|
            +--------------------------------------------------------------+
            |f2(8..15)    | f2(16..23)     | ....                          |
            +--------------------------------------------------------------+
            :   ....                                                       :
            +--------------------------------------------------------------+
            |                         .... |f1(152..158)  |P|
            +-----------------------------------------------+

            ////////////////////////////////////////////////////////////////////*/

            u32 dwFrameNum = 1;
            for (u32 dwNum = 1; dwNum < dwPackLen; dwNum++)
            {
                if ((m_pFrameBuf[dwNum]&0x80)==0x00)
                {
                    break;
                }
                dwFrameNum++;
            }

            u8 byAmrHeader = m_pFrameBuf[dwFrameNum];
            static u32 adwPackedSize[8] = {12, 13, 15, 17, 19, 20, 26, 31};
            u8 ft = (byAmrHeader >> 3) & 0x0F;
            if(ft >= 8)
            {
                MedianetLog(Error, "AMR ft error %d \n", ft);
                return -1;
            }

            if(dwPackLen != (1+adwPackedSize[ft])*dwFrameNum + 1)
            {
                MedianetLog(Error, "AMR Frame size is not match!");
                return -1;
            }

             u8* pSrc = ( u8*)m_pFrameBuf + dwFrameNum + 1;

             for (u32 i=0; i<dwFrameNum; i++)
             {
                 u8 byAmr[32];
                 byAmr[0] = byAmrHeader;
                 memcpy(byAmr+1, pSrc, adwPackedSize[ft]);
                 m_FrmHdr.m_pData = byAmr;
                 m_FrmHdr.m_dwDataSize = adwPackedSize[ft]+1;
                 m_FrmHdr.m_dwFrameID++;
                 m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;
                 m_tRcvStatistics.m_dwFrameNum++;

                 //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
                 if (m_dwTimeStampSample >= 1000)
                 {
                    m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
                 }

                 if(m_bRcvStart && m_pFrameCallBackHandler != NULL && g_bInterRcvCallBack)
                 {
                     MedianetLog(AudioFrame, "[CallBackOneAudioFrame] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_byAudioMode=%d   \n",
                             m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID,
                             m_FrmHdr.m_dwDataSize, m_FrmHdr.m_byAudioMode);

					 MedianetLog(VideoAndAudioFrame, "[CallBackOneAudioFrame] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_byAudioMode=%d   \n",
						 m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID,
                             m_FrmHdr.m_dwDataSize, m_FrmHdr.m_byAudioMode);

                     m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
                 }
                 pSrc += adwPackedSize[ft];
             }
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    return 0;
}

/*=============================================================================
    ������        SendFrameAudio
    ����        ��������Ƶ֡
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                 s32 nStartPos ֡��ͷλ��
                 s32 nEndPos   ֡��βλ��

    ����ֵ˵����0 -�ɹ�
=============================================================================*/
s32 CKdvNetRcv::SendFrameAudio(s32 nStartPos, s32 nEndPos)
{
    u32 dwCurFrameTS;

    //��һ����ӦΪ��Ч���������洦����Ʊ�֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;

    u8* pbyCurBuffPos = m_pFrameBuf;


    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed)
        {
            u32 dwPackLen;

            //�������ݰ���ָ��λ��
            dwPackLen = m_cListBuffMgr.ReadBuff(m_atPackets[nStartPos].m_ptDataBuff, m_pFrameBuf);

            m_FrmHdr.m_byAudioMode = m_tAudioHeader.m_nMode;
            m_FrmHdr.m_byMediaType = m_tLastInfo.m_byMediaType;

            if (MEDIA_TYPE_AACLC == m_FrmHdr.m_byMediaType || MEDIA_TYPE_AACLD == m_FrmHdr.m_byMediaType|| MEDIA_TYPE_AACLC_PCM == m_FrmHdr.m_byMediaType)
            {
                if (AACLC_TRANSMODE_MPEG4_GENERIC == m_eTransMode)
                {
                    u8 abyAACHead[4] = {0};
                    memcpy(abyAACHead, m_pFrameBuf, 4);
                    u32 dwLength = abyAACHead[0];
                    if (0 == abyAACHead[0] && 0x10 == abyAACHead[1] && (u8)(((dwPackLen-4) & 0x1fe0) >> 5) == abyAACHead[2]
                        && (u8)(((dwPackLen-4) & 0x1f) << 3) == abyAACHead[3])
                    {
                        m_FrmHdr.m_dwDataSize  = dwPackLen - 4;
                        m_FrmHdr.m_pData       = m_pFrameBuf + 4;
                    }
                    else
                    {
                        m_FrmHdr.m_dwDataSize  = dwPackLen;
                        m_FrmHdr.m_pData       = m_pFrameBuf;
                    }
                }
                else
                {
                    m_FrmHdr.m_dwDataSize  = dwPackLen - 1;
                    m_FrmHdr.m_pData       = m_pFrameBuf + 1;
                }
            }
            else
            {
                m_FrmHdr.m_dwDataSize  = dwPackLen;
                m_FrmHdr.m_pData       = m_pFrameBuf;
            }
            m_FrmHdr.m_dwTimeStamp = dwCurFrameTS;
            m_FrmHdr.m_dwSSRC      = m_tLastInfo.m_dwSSRC;

            //����֡id��¼
            m_FrmHdr.m_dwFrameID++;
            m_tRcvStatus.m_dwFrameID = m_FrmHdr.m_dwFrameID;
            m_tRcvStatistics.m_dwFrameNum++;

            //���ݲ��������ûص�ʱ���,��ֹ����Ϊ0
            if (m_dwTimeStampSample >= 1000)
            {
                m_FrmHdr.m_dwTimeStamp = m_FrmHdr.m_dwTimeStamp/(m_dwTimeStampSample/1000);
            }

            if(m_bRcvStart && m_pFrameCallBackHandler != NULL && g_bInterRcvCallBack)
            {
                MedianetLog(AudioFrame, "[CallBackOneAudioFrame] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_byAudioMode=%d   \n",
                        m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID,
                        m_FrmHdr.m_dwDataSize, m_FrmHdr.m_byAudioMode);
				MedianetLog(VideoAndAudioFrame, "[CallBackOneAudioFrame] FRAME Info:m_byMediaType=%d, m_dwTimeStamp=%d, m_dwFrameID=%d, m_dwDataSize=%d, m_byAudioMode=%d   \n",
					m_FrmHdr.m_byMediaType, m_FrmHdr.m_dwTimeStamp, m_FrmHdr.m_dwFrameID,
                        m_FrmHdr.m_dwDataSize, m_FrmHdr.m_byAudioMode);

                m_pFrameCallBackHandler(&m_FrmHdr, (KD_PTR)m_pContext);
            }
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }

        nStartPos = NEXTPACKPOS(nStartPos);
    }

    return 0;
}



void CKdvNetRcv::ResetRSPos()
{
    if (INVALID_PACKET_POS == m_nCurFrameStartPos)
    {
        return;
    }

    s32 i;
    for (i = 0; i < MAX_RESEND_QUEST_TIMES; i++)
    {
        m_wRSPoint[i].m_wSequence = m_atPackets[m_nCurFrameStartPos].m_wSN;
        m_wRSPoint[i].m_dwTimestamp = m_atPackets[m_nCurFrameStartPos].m_dwRcvTS;
    }
}


void CKdvNetRcv::UpdateRSPos()
{
    s32 i;
    u16 wCurSN;
    u32 dwTimestamp;

    if (INVALID_PACKET_POS == m_nCurFrameStartPos)
    {
        return;
    }

    wCurSN = m_atPackets[m_nCurFrameStartPos].m_wSN;
    dwTimestamp = m_atPackets[m_nCurFrameStartPos].m_dwRcvTS;

    for (i = 0; i < MAX_RESEND_QUEST_TIMES; i++)
    {
        // if CurSN > RSSequence, Move RSSequence to Cur
        if ((u16)(wCurSN - m_wRSPoint[i].m_wSequence) < (u16)m_nPacketBuffNum)
        {
            m_wRSPoint[i].m_wSequence = wCurSN;
            m_wRSPoint[i].m_dwTimestamp = dwTimestamp;
        }
    }
}

/*=============================================================================
    ������        ��DealRSCheck
    ����        ���ش����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�����

    �������˵����
                 u16 wLastSN    seqnum
                 u32 dwLastTS   ttimestamps

    ����ֵ˵������
=============================================================================*/
void CKdvNetRcv::DealRSCheck(u16 wLastSN, u32 dwLastTS)
{
    u16 wCurSN;     //��ǰ֡����ʼSN

    //δ��ʼ��
    if (INVALID_PACKET_POS == m_nCurFrameStartPos)
    {
        return;
    }

    if (0 == m_dwBufPackNum)
    {
        return;
    }
      if (!m_bRepeatSend)
    {
        return;
    }
    if (0 == m_dwFrameTime)
    {
        return;
    }

    wCurSN = m_atPackets[m_nCurFrameStartPos].m_wSN;

    //����3���ش��㣬��෢��3���ش�����
    s32 i;
    for (i = 0; i < MAX_RESEND_QUEST_TIMES; i++)
    {
        u16 wRSSequence = m_wRSPoint[i].m_wSequence;//�ش�������к�

        if ((u16)(wLastSN - wRSSequence) >= (u16)m_nPacketBuffNum) //�ϵİ�
        {
            continue;
        }

        s32 nPos = wRSSequence % m_nPacketBuffNum;
        //BOOL32 bDoFrameCheck = FALSE;//�˱���û��ʹ�õ�

        TPacketInfo* ptPackInfo = &m_atPackets[nPos];

        u32 dwRSTimeStamp = m_wRSPoint[i].m_dwTimestamp;//�ش����ʱ���
        u32 dwDiff = dwLastTS - dwRSTimeStamp;//ʱ�����ֵ
        u32 dwtimestampTH = VIDEO_TIME_SPAN * m_byRSFrameDistance[i];

        //ʱ���ﵽ�ش�ʱ�䣬ѭ�����һ�û���յ��İ��������ش�����
        u32 dwCount = 0;
        u32 dwUsedCount =0;
        while (dwDiff > dwtimestampTH)
        {
            if (dwCount++ > (u32)m_nPacketBuffNum) //+by lxx : ������ѭ��
            {
                break;
            }

            //ʱ���ﵽ�ش�ʱ��
            u32 dwRSFrameTS = 0;
            if (!ptPackInfo->m_bUsed)
            {
                //�����ش�
                MedianetLog(Resend, "This:0x%x RS %d ---- %d\n", this, i, wRSSequence);
                SendRSQSndQuest(wRSSequence);
            }
            else
            {
                if (0 == dwRSFrameTS)//������жϿ���ȥ��
                {
                    dwRSFrameTS = ptPackInfo->m_dwRcvTS;
                }
                //�Ѵ���������ڻ������ܰ���������Ҫ�ټ���ѭ���������İ���
                dwUsedCount++;
                if (dwUsedCount == m_dwBufPackNum)
                {
                    break;
                }
            }

            m_wRSPoint[i].m_wSequence++;

            //����ҵ�һ��֡�߽磬������������߼�����֣���Զ�߲���ȥ
            if ((ptPackInfo->m_bUsed) &&
                (ptPackInfo->m_dwRcvTS != dwRSFrameTS))
            {
                break;
            }

            if ((ptPackInfo->m_bUsed) && (ptPackInfo->m_dwRcvTS != dwRSTimeStamp))
            {
                //�����ش�ʱ���
                m_wRSPoint[i].m_dwTimestamp = ptPackInfo->m_dwRcvTS;
                dwRSTimeStamp = ptPackInfo->m_dwRcvTS;
                dwDiff = dwLastTS - dwRSTimeStamp;
            }
            //��һ��λ��
            nPos = NEXTPACKPOS(nPos);
            ptPackInfo = &m_atPackets[nPos];

            wRSSequence++;
        }
    }
}

/*=============================================================================
    ������        SendRSQSndQuest
    ����        �������ش�����
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�����

    �������˵����
                 u16 wSN    seqnum

    ����ֵ˵������
=============================================================================*/
void CKdvNetRcv::SendRSQSndQuest(u16 wSN)
{
    TRtcpSDESRSQ tSndRSQ;
    memset(&tSndRSQ, 0, sizeof(tSndRSQ));

    ///����ط�֡�ĸ������ط�����λ
    u32 *pdwMaskBit = (u32*)&tSndRSQ.m_byMaskBit;
    pdwMaskBit[0] = SetBitField(pdwMaskBit[0], 1, 0, 1); //���õ�һ��λ��

    //Ŀǰ�̶����ð�seq�ش��ķ�ʽ
    tSndRSQ.m_byRSQType = SN_RSQ_TYPE;

    //����ط�֡��ʱ�������С��������ʼ������
    tSndRSQ.m_byPackNum = 1;
    tSndRSQ.m_wStartSeqNum = wSN;
    tSndRSQ.m_dwTimeStamp = 0;

    //order rs rtcp convert
    tSndRSQ.m_dwTimeStamp = htonl(tSndRSQ.m_dwTimeStamp);
    tSndRSQ.m_wStartSeqNum = htons(tSndRSQ.m_wStartSeqNum);

    s32 nSize = MAX_PACK_NUM / (8 * sizeof(u32));
    ConvertH2N( (u8*)pdwMaskBit, 0, nSize);

    if( m_dwRtpPublicIp != 0 && m_wRtpPublicPort != 0 )
    {
        tSndRSQ.m_dwRtpIP = m_dwRtpPublicIp;
        tSndRSQ.m_wRtpPort = htons(m_wRtpPublicPort);
    }
    else
    {
        tSndRSQ.m_dwRtpIP = m_tLocalNetParam.m_tLocalNet.m_dwRTPAddr;
        tSndRSQ.m_wRtpPort = htons(m_tLocalNetParam.m_tLocalNet.m_wRTPPort);
    }

    u32 dwTick = OspTickGet() * (1000 / OspClkRateGet());

    MedianetLog(Resend, "[PayLoad: %d] Acquire Resend SN:%u  tick = %d\n",
              m_tLastInfo.m_byMediaType, wSN, dwTick);

    m_pcRtcp->SendRSQ(tSndRSQ);
}



//------------------------------------H263--------------------------------------------
/*=============================================================================
    ������        ��DealH263
    ����        �������յ�h263���ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealH263(TRtpPack *pRtpPack)
{
    //a. ������Ч���ж�
    if (FALSE == CheckH26XPack(pRtpPack))
    {
        MedianetLog(LostPack, "[DealH263] step 2 CheckH26XPack return ... \n");
        return;
    }

    //mody by hual
    //��������ʱ�����ʱ�����ʱ������루for DST��
    if (FRAME_RELY_MARK == g_nFrameRelyMode)
    {
        if ((u16)(m_tLastInfo.m_wSeq+1) == pRtpPack->m_wSequence)
        {

            if (m_tLastInfo.m_byMark) //��һ֡����
            {
                //ʱ���ͬ��һ֡, ǿ��ʱ���+1
                if (pRtpPack->m_dwTimeStamp == m_tLastInfo.m_dwTimeStamp)
                {
                    pRtpPack->m_dwTimeStamp++;
                }
            }
            else
            {
                //�޸ĸð���ʱ���Ϊ��һ����ʱ���
                pRtpPack->m_dwTimeStamp = m_tLastInfo.m_dwTimeStamp;
            }
        }
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;
}



/*=============================================================================
    ������        ��DealH263Plus
    ����        �������յ���h.263+ ���ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealH263Plus(TRtpPack *pRtpPack)
{
    if(NULL == m_pFrameBuf)
    {
        MedianetLog(Error, "[DealH263Plus] step 1 return ...\n");

        return;
    }

    //a. ������Ч���ж�
    if(FALSE == CheckH26XPack(pRtpPack))
    {
        MedianetLog(Error, "[DealH263Plus] step 2 CheckH26XPack return ...\n");
        return;
    }


    //mody by hual
    //��������ʱ�����ʱ�����ʱ������루for DST��
    if (FRAME_RELY_MARK == g_nFrameRelyMode)
    {
        if ((u16)(m_tLastInfo.m_wSeq+1) == pRtpPack->m_wSequence)
        {

            if (m_tLastInfo.m_byMark) //��һ֡����
            {
                //ʱ���ͬ��һ֡, ǿ��ʱ���+1
                if (pRtpPack->m_dwTimeStamp == m_tLastInfo.m_dwTimeStamp)
                {
                    pRtpPack->m_dwTimeStamp++;
                }
            }
            else
            {
                //�޸ĸð���ʱ���Ϊ��һ����ʱ���
                pRtpPack->m_dwTimeStamp = m_tLastInfo.m_dwTimeStamp;
            }
        }
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;
}


//------------------------------------H263--------------------------------------------
/*=============================================================================
    ������        ��DealH261
    ����        �������յ�h261���ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealH261(TRtpPack *pRtpPack)
{
    //a. ������Ч���ж�
    if (FALSE == CheckH26XPack(pRtpPack))
    {
        MedianetLog(Error, "[DealH263]1CheckH26XPack return false \n");
        return;
    }

    //mody by hual
    if (FRAME_RELY_MARK == g_nFrameRelyMode) //������Mark��for DST��
    {
        if ((u16)(m_tLastInfo.m_wSeq+1) == pRtpPack->m_wSequence)
        {

            if (m_tLastInfo.m_byMark) //��һ֡����
            {
                //ʱ���ͬ��һ֡, ǿ��ʱ���+1
                if (pRtpPack->m_dwTimeStamp == m_tLastInfo.m_dwTimeStamp)
                {
                    pRtpPack->m_dwTimeStamp++;
                }
            }
            else
            {
                //�޸ĸð���ʱ���Ϊ��һ����ʱ���
                pRtpPack->m_dwTimeStamp = m_tLastInfo.m_dwTimeStamp;
            }
        }
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;
}



//------------------------------------Mpg4--------------------------------------------
/*=============================================================================
    ������        ��DealMpg4
    ����        �������յ�Mpg4���ݰ�
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����
                   pRtpPack     RTP���ݰ��ṹ,�μ�����

    ����ֵ˵���� ��
=============================================================================*/
void CKdvNetRcv::DealMpg4(TRtpPack *pRtpPack)
{
    //a. ������Ч���ж�
    if (FALSE == CheckMpg4Pack(pRtpPack))
    {
        MedianetLog(Error, "[DealMpg4] step 2 CheckMpg4Pack return ... \n");
        return;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

   MEDIANET_SEM_GIVE(m_hSem);

    return;
}





//------------------------------------Mpg4--------------------------------------------
void CKdvNetRcv::DealG7xx(TRtpPack *pRtpPack)
{
    //a. ������Ч���ж�
    if(FALSE == CheckG7xxPack(pRtpPack))
    {
        MedianetLog(Error, "[DealG7xx:%d] step 1 CheckG7xxPack return ... \n", pRtpPack->m_byPayload);
        return;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;
}


void CKdvNetRcv::DealMp3(TRtpPack *pRtpPack)
{
    //a. ������Ч���ж�
    if(FALSE == CheckMp3Pack(pRtpPack))
    {
        MedianetLog(Error, "[DealG7xx:%d] step 1 CheckG7xxPack return ... \n", pRtpPack->m_byPayload);
        return;
    }

    MEDIANET_SEM_TAKE(m_hSem);

    DealRtpPacket(pRtpPack);

    MEDIANET_SEM_GIVE(m_hSem);

    return;
}


//�²����ݻص�����
void RcvCallBack(TRtpPack *pRtpPack, void* pContext)
{
    CKdvNetRcv *pMain = (CKdvNetRcv *)pContext;
    if(pMain != NULL)
    {
        OspSemTake(pMain->m_hSynSem);
        pMain->DealNetData(pRtpPack);
        OspSemGive(pMain->m_hSynSem);
    }

    return;
}

//ת��Rtp�������ڵ���
static CKdvSocket* g_st_pcDebugSocket = NULL;
static u32 g_st_dwRtpTransIP = 0;
static u16 g_st_wRtpTransPort = 0;

void TransRtpCallBack(u8 *pBuf, u32 dwSize)
{
    if (NULL != g_st_pcDebugSocket && g_st_dwRtpTransIP != 0 && g_st_wRtpTransPort != 0)
    {
        g_st_pcDebugSocket->SendTo(pBuf, dwSize, g_st_dwRtpTransIP, g_st_wRtpTransPort);
    }
}

u16 CKdvNetRcv::RelayRtpStart(u32 dwIP, u16 wPort)
{
    if (NULL != g_st_pcDebugSocket)
    {
        MedianetLog(AllOpen, "Debug has Started. Please stop first!\n");
        return 0;
    }
    if (NULL == m_pcRtp)
    {
        MedianetLog(AllOpen, "NetRcv is not ready! (NULL == m_pcRtp)\n");
        return 0;
    }
    g_st_pcDebugSocket = new CKdvSocket;
    if (NULL == g_st_pcDebugSocket)
    {
        return ERROR_SND_MEMORY;
    }
    //g_st_pcDebugSocket->Create(TRUE);
    g_st_pcDebugSocket->Create(SOCK_DGRAM, 0, 0, 0, FALSE);
    g_st_dwRtpTransIP = dwIP;
    g_st_wRtpTransPort = wPort;

    m_pcRtp->SetRawTransHandle(TransRtpCallBack);
    return 0;
}

//ת��Rtp��
u16 CKdvNetRcv::RelayRtpStop()
{
    m_pcRtp->SetRawTransHandle(NULL);

    if (NULL != g_st_pcDebugSocket)
    {
        delete g_st_pcDebugSocket;
        g_st_pcDebugSocket = NULL;
    }
    g_st_pcDebugSocket = NULL;
    g_st_dwRtpTransIP = 0;
    g_st_wRtpTransPort = 0;
    return 0;
}

void CKdvNetRcv::DealPS(TRtpPack *pRtpPack)
{
    MEDIANET_SEM_TAKE(m_hSem);
    DealRtpPacket(pRtpPack);
    MEDIANET_SEM_GIVE(m_hSem);
}

u16 FrameCallback(TspsFRAMEHDR *ptFrame, KD_PTR pvContext)
{
    CKdvNetRcv* pMain = (CKdvNetRcv*)pvContext;
    if (NULL == pMain)
    {
        return 1;
    }
	FRAMEHDR tFrame;
	memset(&tFrame, 0,sizeof(FRAMEHDR));
	tFrame.m_byFrameRate = ptFrame->m_byFrameRate;
	tFrame.m_byMediaType = ptFrame->m_byMediaType;
	tFrame.m_dwDataSize = ptFrame->m_dwDataSize;
	tFrame.m_dwFrameID = ptFrame->m_dwFrameID;
	tFrame.m_dwPreBufSize = ptFrame->m_dwPreBufSize;
	tFrame.m_dwSSRC = ptFrame->m_dwSSRC;
	tFrame.m_dwTimeStamp = ptFrame->m_dwTimeStamp;
	tFrame.m_pData = ptFrame->m_pbyData;
	tFrame.m_tVideoParam.m_bKeyFrame = ptFrame->x.m_tVideoParam.m_bKeyFrame;
	tFrame.m_tVideoParam.m_wVideoHeight = ptFrame->x.m_tVideoParam.m_wVideoHeight;
	tFrame.m_tVideoParam.m_wVideoWidth = ptFrame->x.m_tVideoParam.m_wVideoWidth;
	tFrame.m_byAudioMode = ptFrame->x.m_byAudioMode;
	tFrame.m_byStreamID = ptFrame->m_byStreamID;

    pMain->m_tRcvStatistics.m_dwFrameNum++;
	tFrame.m_dwTimeStamp = tFrame.m_dwTimeStamp / 90;
    BOOL32 bPPSSPSOK = TRUE;
	if ((MEDIA_TYPE_H264 == tFrame.m_byMediaType || MEDIA_TYPE_MP4 == tFrame.m_byMediaType || MEDIA_TYPE_H265 == tFrame.m_byMediaType) && 
		(0 == tFrame.m_tVideoParam.m_wVideoWidth ||
		0 == tFrame.m_tVideoParam.m_wVideoHeight))
    {
        bPPSSPSOK = FALSE;
    }

    if (pMain->m_bRcvStart && g_bInterRcvCallBack && pMain->m_pFrameCallBackHandler != NULL && bPPSSPSOK)
    {
		if(tFrame.m_byMediaType == MEDIA_TYPE_H264 || tFrame.m_byMediaType == MEDIA_TYPE_MP4 || tFrame.m_byMediaType == MEDIA_TYPE_H265)
        {
			if(tFrame.m_dwFrameID - pMain->m_dwPsVideoLastFrameId == 1&& tFrame.m_dwTimeStamp - pMain->m_dwPsVideoLastFrameTimeStamp != 0)
            {
                //count framerate by timestamp
				tFrame.m_byFrameRate = (1000) / (tFrame.m_dwTimeStamp - pMain->m_dwPsVideoLastFrameTimeStamp);
            }
            else
            {
                //default value 25
				tFrame.m_byFrameRate = 25;
            }
			pMain->m_dwPsVideoLastFrameId = tFrame.m_dwFrameID;
			pMain->m_dwPsVideoLastFrameTimeStamp = tFrame.m_dwTimeStamp;
        }

        MedianetLog(VideoFrame, "callback frame ID:%d, timestamp:%d size:%d ,width:%d, height:%d, bKeyFrame:%d, ptFrame->mediatype %d  framerate=%d\n",
				tFrame.m_dwFrameID, tFrame.m_dwTimeStamp, tFrame.m_dwDataSize,
				tFrame.m_tVideoParam.m_wVideoWidth, tFrame.m_tVideoParam.m_wVideoHeight, tFrame.m_tVideoParam.m_bKeyFrame, tFrame.m_byMediaType,
				tFrame.m_byFrameRate);
        pMain->m_pFrameCallBackHandler((PFRAMEHDR)ptFrame, (KD_PTR)pMain->m_pContext);
    }

    return 0;
}

s32 CKdvNetRcv::CompagesPSAndCB(s32 nStartPos, s32 nEndPos)
{
    u8* pFrameBuff;
    FRAMEHDR tFrameHdr;
    memset(&tFrameHdr, 0, sizeof(FRAMEHDR));

    pFrameBuff = m_pFrameBuf;

    u32 dwCurFrameTS;

    BOOL32 bPackEnd = FALSE;
    BOOL32 bNaluFuStart = FALSE;
    m_dwLastPackLen = 0;
    s32 nCopiedLen = 0;

    //��һ����ӦΪ��Ч�����ɴ�����Ʊ�֤��
    dwCurFrameTS = m_atPackets[nStartPos].m_dwTS;
    while(TRUE)
    {
        if (m_atPackets[nStartPos].m_bUsed && m_atPackets[nStartPos].m_ptDataBuff)
        {
            TListBuffBlock *ptPack = m_atPackets[nStartPos].m_ptDataBuff;
            u32 dwPackLen;
            nCopiedLen = pFrameBuff - m_pFrameBuf;
            if ((u32)nCopiedLen >= m_dwMaxFrameSize - 1500)
            {
                MedianetLog(Error, "[medianet] rcv frame larger than %d bytes, discard!\n", m_dwMaxFrameSize);
                return -1;
            }

            //����һ������
            dwPackLen = m_cListBuffMgr.ReadBuff(ptPack, pFrameBuff, m_dwMaxFrameSize - (u32)(pFrameBuff - m_pFrameBuf));
            if(0 == dwPackLen)
            {
                MedianetLog(Error, "CompagesH264AndCB ReadBuff failed , Discard Frame TimeStamp = %d!\n", dwCurFrameTS);
                return 0;
            }
            pFrameBuff += dwPackLen;
            m_dwLastPackLen = dwPackLen;
        }

        if (nStartPos == nEndPos)
        {
            break; //����
        }
        nStartPos = NEXTPACKPOS(nStartPos);
    }

    if (NULL == m_hTspsRead)
    {
        m_hTspsRead = TspsReadOpen(PROGRAM_STREAM, FrameCallback, (KD_PTR)this, m_dwMaxFrameSize);
        if (NULL == m_hTspsRead)
        {
            MedianetLog(Error, "TspsReadOpen error! \n");
            return -1;
        }
    }
    m_dwRtpTime = dwCurFrameTS;


    if(m_pPsFrameCallBackProc)
    {
        tFrameHdr.m_byMediaType = MEDIA_TYPE_PS;
        tFrameHdr.m_dwTimeStamp = m_dwRtpTime;
        tFrameHdr.m_dwDataSize = pFrameBuff-m_pFrameBuf;
        tFrameHdr.m_pData = m_pFrameBuf;

        MedianetLog(VideoFrame,"[CompagesPSAndCB] call ps frame m_byMediaType=%d, m_dwTimeStamp=%d, m_dwDataSize=%d,m_pData=%8x\n",
                tFrameHdr.m_byMediaType, tFrameHdr.m_dwTimeStamp, tFrameHdr.m_dwDataSize, tFrameHdr.m_pData);

        m_pPsFrameCallBackProc(&tFrameHdr, (KD_PTR)m_pPsFrameCallBackContext);
    }

    //Ĭ�ϻص�ȥpsͷ��֡����ֻ��ps֡�������ϲ����ñ�־λ��
    if(m_bCallBackFrameWithOutPs)
    {
        u16 wRet = TspsReadInputStream(m_hTspsRead, m_pFrameBuf, pFrameBuff-m_pFrameBuf);
        if (wRet != TSPS_OK)
        {
            MedianetLog(Error, "TspsReadInputStream error(%d)!\n", wRet);
            return wRet;
        }
    }
    else
    {
        if(33 == g_nShowDebugInfo)
        {
            MedianetLog(VideoFrame,"[CompagesPSAndCB] call ps frame m_bCallBackFrameWithOutPs=%d (0 means not callback frame with out ps)\n",m_bCallBackFrameWithOutPs);
        }
    }

    return 0;

}


/*====================================================================
������      ��  DecodePTL
����        ��  ����PTL��Ϣ
�㷨ʵ��    ��  ����ѡ�
����ȫ�ֱ�����  ��
�������˵����  ptBs��ָ���������ָ��[in]
                u8MaxSubLayersMinus1������[in]
����ֵ˵��  ��  �ɹ����أ�VIDEO_SUCCESS�����󷵻���Ӧ������
====================================================================*/
BOOL32 CKdvNetRcv::DecodePTL(TBitStream *ptBs, u8 u8MaxSubLayersMinus1)
{
    s32 l32Index;
    u8 au8SubLayerProfilePresentFlag[6];
    u8 au8SubLayerLevelPresentFlag[6];

    stdh265_bs_skip(ptBs, 8); //��ȡgeneral_profile_space��general_tier_flag��general_profile_idc

    stdh265_bs_skip(ptBs, 32); //��ȡgeneral_profile_compatibility_flag[ i ]

    //��ȡʣ�µ��﷨Ԫ��general_progressive_source_flag��general_level_idc
    stdh265_bs_skip(ptBs, 32);
    stdh265_bs_skip(ptBs, 24);

    for(l32Index = 0; l32Index < u8MaxSubLayersMinus1; l32Index++)
    {
        au8SubLayerProfilePresentFlag[l32Index] = H265DecBitstreamGetBits(ptBs, 1); //��ȡsub_layer_profile_present_flag[ i ]
        au8SubLayerLevelPresentFlag[l32Index] = H265DecBitstreamGetBits(ptBs, 1);  //��ȡsub_layer_level_present_flag[ i ]
    }
    if(u8MaxSubLayersMinus1 > 0)
    {
        for(l32Index = u8MaxSubLayersMinus1; l32Index < 8; l32Index++)
        {
            stdh265_bs_skip(ptBs, 2);  //��ȡreserved_zero_2bits[ i ]
        }
    }
    for(l32Index = 0; l32Index < u8MaxSubLayersMinus1; l32Index++)
    {
        if(au8SubLayerProfilePresentFlag[l32Index])
        {
            stdh265_bs_skip(ptBs, 8);  //��ȡsub_layer_profile_space[ i ]��sub_layer_tier_flag[ i ]��sub_layer_profile_idc[ i ]
            stdh265_bs_skip(ptBs, 32);  //��ȡsub_layer_profile_compatibility_flag[ i ][ j ]
            stdh265_bs_skip(ptBs, 48);  //��ȡsub_layer_progressive_source_flag[ i ]��sub_layer_reserved_zero_44bits[i ]
        }
        if(au8SubLayerLevelPresentFlag[l32Index])
        {
            stdh265_bs_skip(ptBs, 8);  //��ȡsub_layer_level_idc[ i ]
        }
    }

    return 0;
}


/*=============================================================================
������        ��DecodeH265SPS
����        ������ h.265 �����е� pps ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� TRUE - �ɹ�
=============================================================================*/
BOOL32 CKdvNetRcv::DecodeH265SPS( TBitStream *s, TSPS *ptSps,
                                 TKdvH265Header *pStdH265Header )
{
    s32 l32RetCode = 0;
//    u8 u8SpsId;
    s32 l32Val;
    s32 l32Ret;
//    u32 u32AddCUDepth, u32MaxDepth;
    u8 u8MaxSubLayersMinus1;
    s32 l32Index;

    ptSps->u8VpsId = H265DecBitstreamGetBits(s, 4); //��ȡsps_video_parameter_set_id

    u8MaxSubLayersMinus1 = H265DecBitstreamGetBits(s, 3); //��ȡsps_max_sub_layers_minus1,todo

    ptSps->u8SpsTemporalIdNestingFlag = H265DecBitstreamGetBits(s, 1); //��ȡsps_temporal_id_nesting_flag

    //����PTL�﷨�ṹ
    l32Ret = DecodePTL(s, u8MaxSubLayersMinus1);

    //��ȡsps_seq_parameter_set_id
    l32Val = stdh265_bs_read_ue(s);
    ptSps->u8SpsId = l32Val;

    //��ȡchroma_format_idc
    l32Val = stdh265_bs_read_ue(s);
    ptSps->u8ChromaFormatIdc = l32Val;

    //��ȡpic_width_in_luma_samples��pic_height_in_luma_samples
    ptSps->u16Width = stdh265_bs_read_ue(s);
    ptSps->u16Height = stdh265_bs_read_ue(s);

    //��ȡconformance_window_flag
    ptSps->l32ConformanceWindow_flag = H265DecBitstreamGetBits(s, 1);
    if(ptSps->l32ConformanceWindow_flag)
    {
        ptSps->l32ConfWinLeftOffset = stdh265_bs_read_ue(s);
        ptSps->l32ConfWinRightOffset = stdh265_bs_read_ue(s);
        ptSps->l32ConfWinTopOffset = stdh265_bs_read_ue(s);
        ptSps->l32ConfWinBottomOffset = stdh265_bs_read_ue(s);

        pStdH265Header->m_wWidth = ptSps->u16Width - (ptSps->l32ConfWinLeftOffset + ptSps->l32ConfWinRightOffset) * 2;
        pStdH265Header->m_wHeight = ptSps->u16Height - (ptSps->l32ConfWinTopOffset + ptSps->l32ConfWinBottomOffset) * 2;
    }
    else
    {
        pStdH265Header->m_wWidth = ptSps->u16Width;
        pStdH265Header->m_wHeight = ptSps->u16Height;
    }

    //��ȡbit_depth_luma_minus8��bit_depth_chroma_minus8
    ptSps->u8BitDepthLumaMinus8 = stdh265_bs_read_ue(s);
    ptSps->u8BitDepthChromaMinus8 = stdh265_bs_read_ue(s);

    //��ȡlog2_max_pic_order_cnt_lsb_minus4
    ptSps->u8Log2MaxPicOrderCntLsbMinus4 = stdh265_bs_read_ue(s);

    //��ȡsps_sub_layer_ordering_info_present_flag
    l32Val = H265DecBitstreamGetBits(s, 1);

    l32Index = (l32Val ?  0 : u8MaxSubLayersMinus1);
    for(; l32Index <= u8MaxSubLayersMinus1; l32Index++)
    {
        //��ȡsps_max_dec_pic_buffering_minus1[i]
        ptSps->au8SpsMaxDecPicBufferingMinus1[l32Index] = stdh265_bs_read_ue(s);

        //��ȡsps_max_num_reorder_pics[i]
        ptSps->au8SpsMaxNumReorderPics[l32Index] = stdh265_bs_read_ue(s);

        //��ȡsps_max_latency_increase_plus1[i]
        ptSps->au8SpsMaxLatencyIncreasePlus1[l32Index] = stdh265_bs_read_ue(s);
    }
    if(!l32Val)
    {
        for(l32Index = 0; l32Index < u8MaxSubLayersMinus1; l32Index++)
        {
            ptSps->au8SpsMaxDecPicBufferingMinus1[l32Index] = ptSps->au8SpsMaxDecPicBufferingMinus1[
u8MaxSubLayersMinus1];
            ptSps->au8SpsMaxNumReorderPics[l32Index] = ptSps->au8SpsMaxNumReorderPics[u8MaxSubLayersMinus1];
            ptSps->au8SpsMaxLatencyIncreasePlus1[l32Index] = ptSps->au8SpsMaxLatencyIncreasePlus1[u8MaxSubLayersMinus1
];
        }
    }

    //��ȡlog2_min_coding_block_size_minus3
    ptSps->u8Log2MinCodingBlockSizeMinus3 = stdh265_bs_read_ue(s);

    //��ȡlog2_diff_max_min_coding_block_size
    ptSps->u8Log2DiffMaxMinLumaCodingBlockSize = stdh265_bs_read_ue(s);

    ptSps->u32MaxPartionWidth = (1 << (ptSps->u8Log2MinCodingBlockSizeMinus3 + 3 + ptSps->u8Log2DiffMaxMinLumaCodingBlockSize));

    ptSps->u8Valid = TRUE;
    pStdH265Header->m_dwMaxPartionWidth = ptSps->u32MaxPartionWidth;
    pStdH265Header->m_dwSPSId = ptSps->u8SpsId;
    pStdH265Header->m_bIsValidSPS = ptSps->u8Valid;

    return l32RetCode;
}

/*=============================================================================
������        ��DecodeH265PPS
����        ������ h.265 �����е� pps ��Ϣ
    �㷨ʵ��    ������ѡ�
    ����ȫ�ֱ�������
    �������˵����

    ����ֵ˵���� TRUE - �ɹ�
=============================================================================*/
BOOL32 CKdvNetRcv::DecodeH265PPS( TBitStream *s, TPPS *ptPps,
                                  TKdvH265Header *pStdH265Header )
{
    s32 l32RetCode = 0;
    s32 l32Val;

    //��ȡpic_parameter_set_id
    ptPps->u8PpsId = stdh265_bs_read_ue(s);

    //��ȡpps_seq_parameter_set_id
    ptPps->u8SpsId = stdh265_bs_read_ue(s);

    ptPps->u8dDependentSliceSegmentsEnabledFlag = H265DecBitstreamGetBits(s, 1);//��ȡdependent_slice_segments_enabled_flag��Ŀǰ�ݲ�֧�֣�todo

    ptPps->u8OutputFlagPresentFlag = H265DecBitstreamGetBits(s, 1);//��ȡoutput_flag_present_flag��Ŀǰ�ݲ�֧�֣�todo

    //��ȡnum_extra_slice_header_bits todo
    l32Val = H265DecBitstreamGetBits(s, 3);

    //��ȡsign_data_hiding_flag
    ptPps->u8SignDataHidingFlag = H265DecBitstreamGetBits(s, 1);

    ptPps->u8CabacInitPresentFlag = H265DecBitstreamGetBits(s, 1);  //��ȡcabac_init_present_flag

    //��ȡnum_ref_idx_l0_default_active_minus1��num_ref_idx_l1_default_active_minus1
    ptPps->u8NumRefIdxL0DefaultActiveMinus1 = stdh265_bs_read_ue(s);

    ptPps->u8NumRefIdxL1DefaultActiveMinus1 = stdh265_bs_read_ue(s);

    //��ȡpic_init_qp_minus26
    ptPps->s8PicInitQpMinus26 = stdh265_bs_read_se(s);

    //��ȡconstrained_intra_pred_flag��transform_skip_enabled_flag��cu_qp_delta_enabled_flag
    l32Val = H265DecBitstreamGetBits(s, 3);
    ptPps->l32ConstrainedIntrPredFlag = ((l32Val >> 2) & 1);
    ptPps->l32Transform_SkipEnabled_Flag = ((l32Val >> 1) & 1);
    ptPps->l32CUQpDeltaEnabledFlag = (l32Val & 1);
    if(ptPps->l32CUQpDeltaEnabledFlag)
    {
        ptPps->u8DiffCUQpDeltaDepth = stdh265_bs_read_ue(s);  //��ȡdiff_cu_qp_delta_depth
    }

    //��ȡpps_cb_qp_offset��pps_cr_qp_offset���ݲ�֧�ַ���ֵ��todo
    l32Val = stdh265_bs_read_se(s);
    l32Val = stdh265_bs_read_se(s);

    //��ȡpps_slice_chroma_qp_offsets_present_flag��entropy_coding_sync_enabled_flag
    l32Val = H265DecBitstreamGetBits(s, 6);
    //�ݲ�֧��pps_slice_chroma_qp_offsets_present_flag��transquant_bypass_enabled_flag,todo
    ptPps->u8TransquantBypassEnabledFlag = ((l32Val >> 2) & 1);//��ȡtransquant_bypass_enabled_flag
    ptPps->l32TilesEnabledFlag = ((l32Val >> 1) & 1);  //��ȡtiles_enabled_flag
    //�ݲ�֧��entropy_coding_sync_enabled_flag,todo
    ptPps->u8EntropyCodingSyncEnabledFlag = (l32Val & 1);

    if(ptPps->l32TilesEnabledFlag)
    {
        //��ȡnum_tile_columns_minus1��num_tile_rows_minus1
        ptPps->l32NumTileColumnsMinus1 = stdh265_bs_read_ue(s);
        ptPps->l32NumTileRowsMinus1 = stdh265_bs_read_ue(s);

        //��ȡuniform_spacing_flag  //�ݲ�֧���Զ���tile��ߣ�todo
        ptPps->u8UniformSpacingFlag = H265DecBitstreamGetBits(s, 1);

        //��ȡloop_filter_across_tiles_enabled_flag
        ptPps->l32LoopFilterAcrossTilesEnabledFlag = H265DecBitstreamGetBits(s, 1);
    }
    else
    {
        ptPps->u8UniformSpacingFlag = 1;
        ptPps->l32LoopFilterAcrossTilesEnabledFlag = 1;
    }

    ptPps->l32LoopFilterAcrossSlicesEnabledFlag = H265DecBitstreamGetBits(s, 1);  //��ȡloop_filter_across_slices_enabled_flag

    ptPps->u8DeblockingFilterControlPresentFlag = H265DecBitstreamGetBits(s, 1);  //��ȡdeblocking_filter_control_present_flag

    if(ptPps->u8DeblockingFilterControlPresentFlag)
    {
        ptPps->u8DeblockingFilterOverrideEnabledFlag = H265DecBitstreamGetBits(s, 1); //��ȡdeblocking_filter_override_enabled_flag
        //�ݲ�֧��deblocking_filter_override_enabled_flag������1,todo

        ptPps->l32DeblockingFilterDisableFlag = H265DecBitstreamGetBits(s, 1); //��ȡpps_deblocking_filter_disabled_flag ,todo

        if(!ptPps->l32DeblockingFilterDisableFlag)
        {
            //�ݲ�֧��pps_beta_offset_div2��pps_tc_offset_div2
            ptPps->s8PPSBetaOffsetDiv2 = stdh265_bs_read_se(s);
            ptPps->s8PPSTcOffsetDiv2 = stdh265_bs_read_se(s);
        }
    }

    //��ȡpps_scaling_list_data_present_flag
    l32Val = H265DecBitstreamGetBits(s, 1);

    //��ȡlists_modification_present_flag
    ptPps->u8ListsModificationPresentFlag = H265DecBitstreamGetBits(s, 1);

    //��ȡlog2_parallel_merge_level_minus2
    ptPps->u8Log2ParallelMergeLevelMinus2 = stdh265_bs_read_ue(s);

    //��ȡslice_segment_header_extension_present_flag��pps_extension_flag
    l32Val = H265DecBitstreamGetBits(s, 2);

    ptPps->u8Valid = TRUE;
    pStdH265Header->m_bIsValidPPS = TRUE;

    return 0;
}

void CKdvNetRcv::DealH265(TRtpPack *pRtpPack)
{

    //xpȥ����Ϊ�˳�֡���յ��߼�����ȷ���˶�ͨ��
    //     pRtpPack->m_byMark = FALSE; //hual for tandberg

    //a. ������Ч���ж�
    if(FALSE == CheckH26XPack(pRtpPack))
    {
        MedianetLog(Error, "[DealH265] step 2 CheckH26XPack return SPS=%d, PPS=%d ... \n",
                m_tH264Header.m_bIsValidSPS, m_tH264Header.m_bIsValidPPS );

        return;
    }

    if(m_hSem != NULL)
    {
        OspSemTake(m_hSem);
    }

    DealRtpPacket(pRtpPack);

    if(m_hSem != NULL)
    {
        OspSemGive(m_hSem);
    }

    return;

}

u16 CKdvNetRcv::SetCompFrameByTimeStamp(BOOL32 bCompFrameByTimeStamp/* = FALSE*/)
{
    m_bCompFrameByTimeStamp = bCompFrameByTimeStamp;
    return MEDIANET_NO_ERROR;
}

u16 CKdvNetRcv::SetAaclcTransMode(EAacTransMode eTransMode)
{
    m_eTransMode = eTransMode;
    return MEDIANET_NO_ERROR;
}

BOOL32 CKdvNetRcv::SetStreamType(ENetStreamType eStreamType)
{
	m_eStreamType = eStreamType;
	return TRUE;
}
