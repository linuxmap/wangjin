/*=================================================================================
模块名:BIT位操作
文件名:bits.h
相关文件:kdvtspslib.h, kdmpstscommon.h
实现功能:BIT位操作
作者:
版权:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#ifndef BITS_H
#define BITS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct tagBit
{
    u8 *pu8Start;       //字符串开始指针
    u8 *pu8Current;     //当前指针
    u8 *pu8End;         //字符串结束指针

    s32 s32Left;        //当前字节的可用位
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


