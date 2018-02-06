/*=================================================================================
ģ����:BITλ����
�ļ���:bits.h
����ļ�:kdvtspslib.h, kdmpstscommon.h
ʵ�ֹ���:BITλ����
����:
��Ȩ:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
#ifndef BITS_H
#define BITS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct tagBit
{
    u8 *pu8Start;       //�ַ�����ʼָ��
    u8 *pu8Current;     //��ǰָ��
    u8 *pu8End;         //�ַ�������ָ��

    s32 s32Left;        //��ǰ�ֽڵĿ���λ
}TBit;

void BitsInit(TBit *ptBits, void *pvData, s32 s32Length);

void BitsSkip(TBit *ptBits, u32 u32BitsCount);

u8 BitsRead8(TBit *ptBits, u8 u8BitsCount);
u16 BitsRead16(TBit *ptBits, u8 u8BitsCount);
u32 BitsRead32(TBit *ptBits, u8 u8BitsCount);
u64 BitsRead64(TBit *ptBits, u8 u8BitsCount);

u8 BitsWrite8(TBit *ptBits, u8 u8BitsCount, u8 u8Value);
u16 BitsWrite16(TBit *ptBits, u8 u8BitsCount, u16 u16Value);
u32 BitsWrite32(TBit *ptBits, u8 u8BitsCount, u32 u32Value);
u64 BitsWrite64(TBit *ptBits, u8 u8BitsCount, u64 u64Value);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif //BITS_H


