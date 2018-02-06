/*=================================================================================
模块名:BIT位操作
文件名:bits.cpp
相关文件:bits.h
实现功能:BIT位操作
作者:
版权:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
#include "common.h"
#include "kdmtsps.h"

//定义一个字节8位
#define BYTE_BITS_COUNT 8

//对所读的和所写的位数进行检查
#define CHECK_BITS_COUNT(count)                                                     \
    if(u8BitsCount > count)                                                         \
    {                                                                               \
        TspsPrintf(0, "op [%d]bits > [%d]bits.\n", u8BitsCount, count);                \
        return 0;                                                                   \
    }

//写一个字符串的若干位
#define WRITE_BITS_BITS(count, value)                                               \
    while((count) > 0)                                                              \
    {                                                                               \
        if(ptBits->pu8Current >= ptBits->pu8End)/*保证没有到字符结尾*/              \
        {                                                                           \
            break;                                                                  \
        }                                                                           \
        (count)--;                                                                  \
        if(((value) >> (count)) & 0x01)/*写一位到字符串的当前位*/                   \
        {                                                                           \
            *ptBits->pu8Current |= 1 << (ptBits->s32Left - 1);                      \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            *ptBits->pu8Current &= ~(1 << (ptBits->s32Left - 1));                   \
        }                                                                           \
        ptBits->s32Left--;/*可用位数量减1*/                                         \
        if(ptBits->s32Left == 0)/*当前字节已经没有可用位*/                          \
        {                                                                           \
            ptBits->pu8Current++;/*移动指针到下一字节*/                             \
            ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                    \
        }                                                                           \
    }

/*=================================================================================
函数名:BitsInit
功能:格式化字符串
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] pvData 要操作的字符串
		 [I] s32Length 字符串长度
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
void BitsInit(TBit *ptBits, void *pvData, s32 s32Length)
{
    ptBits->pu8Start   = (u8 *)pvData;
    ptBits->pu8Current = (u8 *)pvData;
    ptBits->pu8End     = ptBits->pu8Current + s32Length;
    ptBits->s32Left    = BYTE_BITS_COUNT;
}

/*=================================================================================
函数名:BitsSkip
功能:跳过多少位数
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u32BitsCount 跳过的位数
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
void BitsSkip(TBit *ptBits, u32 u32BitsCount)
{
    ptBits->s32Left -= u32BitsCount;
	
    while(ptBits->s32Left <= 0)
    {
        ptBits->pu8Current++;
        ptBits->s32Left += BYTE_BITS_COUNT;
    }
}

/*=================================================================================
函数名:BitsRead8
功能:从字符串读指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要读的位数
返回值说明:失败返回错误号,否则返回8位的值
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u8 BitsRead8(TBit *ptBits, u8 u8BitsCount)
{
    s8 s8Shr = 0;
    u8 u8Result = 0;

    static u8 u8Mask8[9] =
    {  
       0x00,
       0x01,      0x03,      0x07,      0x0f,
       0x1f,      0x3f,      0x7f,      0xff,
    };

    CHECK_BITS_COUNT(8);

    while(u8BitsCount > 0)                                                              
    {                                                                               
        if(ptBits->pu8Current >= ptBits->pu8End)/*保证没有到字符结尾*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*可用位数大于需要读的位数*/    
        {                                                                           
            u8Result |= (*ptBits->pu8Current >> s8Shr) & u8Mask8[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*当前指针字节已经没有可用位*/                  
            {                                                                       
                ptBits->pu8Current++;/*移动指针到下一字节*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                
            }                                                                       
            return u8Result;                                                        
        }                                                                           
        else/*如果当前字节可用位不够*/                                              
        {                                                                           
            u8Result |= (*ptBits->pu8Current & u8Mask8[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*计算还要读取下一字节的位数*/              
            ptBits->pu8Current++;/*移动指针到下一字节*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                    
        }                                                                           
    }

    return u8Result;
}

/*=================================================================================
函数名:BitsRead16
功能:从字符串读指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要读的位数
返回值说明:返回16位的值
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 BitsRead16(TBit *ptBits, u8 u8BitsCount)
{
    s8  s8Shr = 0;
    u16 u16Result = 0;

    static u16 u16Mask16[17] =
    {  
        0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
    };

    CHECK_BITS_COUNT(16);

    while(u8BitsCount > 0)                                                              
    {                                                                               
        if(ptBits->pu8Current >= ptBits->pu8End)/*保证没有到字符结尾*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*可用位数大于需要读的位数*/    
        {                                                                           
            u16Result |= (*ptBits->pu8Current >> s8Shr) & u16Mask16[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*当前指针字节已经没有可用位*/                  
            {                                                                       
                ptBits->pu8Current++;/*移动指针到下一字节*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                
            }                                                                       
            return u16Result;                                                        
        }                                                                           
        else/*如果当前字节可用位不够*/                                              
        {                                                                           
            u16Result |= (*ptBits->pu8Current & u16Mask16[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*计算还要读取下一字节的位数*/              
            ptBits->pu8Current++;/*移动指针到下一字节*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                    
        }                                                                           
    }

    return u16Result;
}


/*=================================================================================
函数名:BitsRead32
功能:从字符串读指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要读的位数
返回值说明:返回32位的值
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u32 BitsRead32(TBit *ptBits, u8 u8BitsCount)
{
    s8  s8Shr = 0;
    u32 u32Result = 0;

    static u32 u32Mask32[33] =
    {  
        0x00,
        0x01,      0x03,      0x07,      0x0f,
        0x1f,      0x3f,      0x7f,      0xff,
        0x1ff,     0x3ff,     0x7ff,     0xfff,
        0x1fff,    0x3fff,    0x7fff,    0xffff,
        0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,
        0x1fffff,  0x3fffff,  0x7fffff,  0xffffff,
        0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
        0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
    };

    CHECK_BITS_COUNT(32);

    while(u8BitsCount > 0)                                                              
    {                                                                               
        if(ptBits->pu8Current >= ptBits->pu8End)/*保证没有到字符结尾*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*可用位数大于需要读的位数*/    
        {                                                                           
            u32Result |= (*ptBits->pu8Current >> s8Shr) & u32Mask32[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*当前指针字节已经没有可用位*/                  
            {                                                                       
                ptBits->pu8Current++;/*移动指针到下一字节*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                
            }                                                                       
            return u32Result;                                                        
        }                                                                           
        else/*如果当前字节可用位不够*/                                              
        {                                                                           
            u32Result |= (*ptBits->pu8Current & u32Mask32[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*计算还要读取下一字节的位数*/              
            ptBits->pu8Current++;/*移动指针到下一字节*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*可用位数量恢复*/                    
        }                                                                           
    }

    return u32Result;
}


/*=================================================================================
函数名:BitsRead64
功能:从字符串读指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要读的位数
返回值说明:返回64位的值
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u64 BitsRead64(TBit *ptBits, u8 u8BitsCount)
{
    s8  s8Shr = 0;
    u64 u64Result = 0;
/*
    static u64 u64Mask64[65] =
    { 
        0x00,
        0x01,              0x03,              0x07,              0x0f,
        0x1f,              0x3f,              0x7f,              0xff,
        0x1ff,             0x3ff,             0x7ff,             0xfff,
        0x1fff,            0x3fff,            0x7fff,            0xffff,
        0x1ffff,           0x3ffff,           0x7ffff,           0xfffff,
        0x1fffff,          0x3fffff,          0x7fffff,          0xffffff,
        0x1ffffff,         0x3ffffff,         0x7ffffff,         0xfffffff,
        0x1fffffff,        0x3fffffff,        0x7fffffff,        0xffffffff,
        0x1ffffffff,       0x3ffffffff,       0x7ffffffff,       0xfffffffff,
        0x1fffffffff,      0x3fffffffff,      0x7fffffffff,      0xffffffffff,
        0x1ffffffffff,     0x3ffffffffff,     0x7ffffffffff,     0xfffffffffff,
        0x1fffffffffff,    0x3fffffffffff,    0x7fffffffffff,    0xffffffffffff,
        0x1ffffffffffff,   0x3ffffffffffff,   0x7ffffffffffff,   0xfffffffffffff,
        0x1fffffffffffff,  0x3fffffffffffff,  0x7fffffffffffff,  0xffffffffffffff,
        0x1ffffffffffffff, 0x3ffffffffffffff, 0x7ffffffffffffff, 0xfffffffffffffff,
        0x1fffffffffffffff,0x3fffffffffffffff,0x7fffffffffffffff,0xffffffffffffffff
    };

    CHECK_BITS_COUNT("BitsRead64", 64, ERROR_BITS_COUNT_R64);

    while(u8BitsCount > 0)                                                              
    {
    //保证没有到字符结尾                                                                               
    if(ptBits->pu8Current >= ptBits->pu8End)              
    {                                                                           
        break;                                                                  
    }
    //可用位数大于需要读的位数                                                                            
    if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)   
    {                                                                           
        u64Result |= (*ptBits->pu8Current >> s8Shr) & u64Mask64[u8BitsCount];             
        ptBits->s32Left -= u8BitsCount; 
        //当前指针字节已经没有可用位
        if(ptBits->s32Left == 0)                 
        {              
            //移动指针到下一字节 
            ptBits->pu8Current++;   
            //可用位数量恢复
            ptBits->s32Left = BYTE_BITS_COUNT;               
        }                                                                       
        return u64Result;                                                        
    }     
    //如果当前字节可用位不够  
    else                                            
    {                                                                           
        u64Result |= (*ptBits->pu8Current & u64Mask64[ptBits->s32Left]) << -s8Shr;
        //计算还要读取下一字节的位数
        u8BitsCount  -= ptBits->s32Left;  
        //移动指针到下一字节
        ptBits->pu8Current++;    
        //可用位数量恢复
        ptBits->s32Left = BYTE_BITS_COUNT;                   
    }                                                                           
}
*/
    return u64Result;
}

/*=================================================================================
函数名:BitsWrite8
功能:向字符串写指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要写的位数
		 [I] u16Value 要写的值
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u8 BitsWrite8(TBit *ptBits, u8 u8BitsCount, u8 u8Value)
{
    CHECK_BITS_COUNT(8);

    WRITE_BITS_BITS(u8BitsCount, u8Value);

    return u8Value;
}

/*=================================================================================
函数名:BitsWrite16
功能:向字符串写指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要写的位数
		 [I] u16Value 要写的值
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u16 BitsWrite16(TBit *ptBits, u8 u8BitsCount, u16 u16Value)
{
    CHECK_BITS_COUNT(16);

    WRITE_BITS_BITS(u8BitsCount, u16Value);

    return u16Value;
}

/*=================================================================================
函数名:BitsWrite32
功能:向字符串写指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要写的位数
         [I] u64Value 要写的值
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u32 BitsWrite32(TBit *ptBits, u8 u8BitsCount, u32 u32Value)
{
    CHECK_BITS_COUNT(32);

    WRITE_BITS_BITS(u8BitsCount, u32Value);

    return u32Value;
}

/*=================================================================================
函数名:BitsWrite64
功能:向字符串写指定长度的位
算法实现: 
参数说明:
         [I/O] ptBits 存储字符信息的结构
         [I] u8BitsCount 要写的位数
         [I] u64Value 要写的值
返回值说明:
-----------------------------------------------------------------------------------
修改记录:
日期        版本        修改人        走读人        修改记录
2008.06.13  0.1         张胜潮                      按照代码规范修改
=================================================================================*/
u64 BitsWrite64(TBit *ptBits, u8 u8BitsCount, u64 u64Value)
{
    CHECK_BITS_COUNT(64);

    WRITE_BITS_BITS(u8BitsCount, u64Value);

    return u64Value;
}


