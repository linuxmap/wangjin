/*=================================================================================
ģ����:BITλ����
�ļ���:bits.cpp
����ļ�:bits.h
ʵ�ֹ���:BITλ����
����:
��Ȩ:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
#include "common.h"
#include "kdmtsps.h"

//����һ���ֽ�8λ
#define BYTE_BITS_COUNT 8

//�������ĺ���д��λ�����м��
#define CHECK_BITS_COUNT(count)                                                     \
    if(u8BitsCount > count)                                                         \
    {                                                                               \
        TspsPrintf(0, "op [%d]bits > [%d]bits.\n", u8BitsCount, count);                \
        return 0;                                                                   \
    }

//дһ���ַ���������λ
#define WRITE_BITS_BITS(count, value)                                               \
    while((count) > 0)                                                              \
    {                                                                               \
        if(ptBits->pu8Current >= ptBits->pu8End)/*��֤û�е��ַ���β*/              \
        {                                                                           \
            break;                                                                  \
        }                                                                           \
        (count)--;                                                                  \
        if(((value) >> (count)) & 0x01)/*дһλ���ַ����ĵ�ǰλ*/                   \
        {                                                                           \
            *ptBits->pu8Current |= 1 << (ptBits->s32Left - 1);                      \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            *ptBits->pu8Current &= ~(1 << (ptBits->s32Left - 1));                   \
        }                                                                           \
        ptBits->s32Left--;/*����λ������1*/                                         \
        if(ptBits->s32Left == 0)/*��ǰ�ֽ��Ѿ�û�п���λ*/                          \
        {                                                                           \
            ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                             \
            ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                    \
        }                                                                           \
    }

/*=================================================================================
������:BitsInit
����:��ʽ���ַ���
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] pvData Ҫ�������ַ���
		 [I] s32Length �ַ�������
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
void BitsInit(TBit *ptBits, void *pvData, s32 s32Length)
{
    ptBits->pu8Start   = (u8 *)pvData;
    ptBits->pu8Current = (u8 *)pvData;
    ptBits->pu8End     = ptBits->pu8Current + s32Length;
    ptBits->s32Left    = BYTE_BITS_COUNT;
}

/*=================================================================================
������:BitsSkip
����:��������λ��
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u32BitsCount ������λ��
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
������:BitsRead8
����:���ַ�����ָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫ����λ��
����ֵ˵��:ʧ�ܷ��ش����,���򷵻�8λ��ֵ
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
        if(ptBits->pu8Current >= ptBits->pu8End)/*��֤û�е��ַ���β*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*����λ��������Ҫ����λ��*/    
        {                                                                           
            u8Result |= (*ptBits->pu8Current >> s8Shr) & u8Mask8[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*��ǰָ���ֽ��Ѿ�û�п���λ*/                  
            {                                                                       
                ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                
            }                                                                       
            return u8Result;                                                        
        }                                                                           
        else/*�����ǰ�ֽڿ���λ����*/                                              
        {                                                                           
            u8Result |= (*ptBits->pu8Current & u8Mask8[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*���㻹Ҫ��ȡ��һ�ֽڵ�λ��*/              
            ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                    
        }                                                                           
    }

    return u8Result;
}

/*=================================================================================
������:BitsRead16
����:���ַ�����ָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫ����λ��
����ֵ˵��:����16λ��ֵ
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
        if(ptBits->pu8Current >= ptBits->pu8End)/*��֤û�е��ַ���β*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*����λ��������Ҫ����λ��*/    
        {                                                                           
            u16Result |= (*ptBits->pu8Current >> s8Shr) & u16Mask16[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*��ǰָ���ֽ��Ѿ�û�п���λ*/                  
            {                                                                       
                ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                
            }                                                                       
            return u16Result;                                                        
        }                                                                           
        else/*�����ǰ�ֽڿ���λ����*/                                              
        {                                                                           
            u16Result |= (*ptBits->pu8Current & u16Mask16[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*���㻹Ҫ��ȡ��һ�ֽڵ�λ��*/              
            ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                    
        }                                                                           
    }

    return u16Result;
}


/*=================================================================================
������:BitsRead32
����:���ַ�����ָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫ����λ��
����ֵ˵��:����32λ��ֵ
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
        if(ptBits->pu8Current >= ptBits->pu8End)/*��֤û�е��ַ���β*/              
        {                                                                           
            break;                                                                  
        }                                                                           
        if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)/*����λ��������Ҫ����λ��*/    
        {                                                                           
            u32Result |= (*ptBits->pu8Current >> s8Shr) & u32Mask32[u8BitsCount];             
            ptBits->s32Left -= u8BitsCount;                                             
            if(ptBits->s32Left == 0)/*��ǰָ���ֽ��Ѿ�û�п���λ*/                  
            {                                                                       
                ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                         
                ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                
            }                                                                       
            return u32Result;                                                        
        }                                                                           
        else/*�����ǰ�ֽڿ���λ����*/                                              
        {                                                                           
            u32Result |= (*ptBits->pu8Current & u32Mask32[ptBits->s32Left]) << -s8Shr;    
            u8BitsCount  -= ptBits->s32Left;/*���㻹Ҫ��ȡ��һ�ֽڵ�λ��*/              
            ptBits->pu8Current++;/*�ƶ�ָ�뵽��һ�ֽ�*/                             
            ptBits->s32Left = BYTE_BITS_COUNT;/*����λ�����ָ�*/                    
        }                                                                           
    }

    return u32Result;
}


/*=================================================================================
������:BitsRead64
����:���ַ�����ָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫ����λ��
����ֵ˵��:����64λ��ֵ
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
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
    //��֤û�е��ַ���β                                                                               
    if(ptBits->pu8Current >= ptBits->pu8End)              
    {                                                                           
        break;                                                                  
    }
    //����λ��������Ҫ����λ��                                                                            
    if((s8Shr = ptBits->s32Left - u8BitsCount) >= 0)   
    {                                                                           
        u64Result |= (*ptBits->pu8Current >> s8Shr) & u64Mask64[u8BitsCount];             
        ptBits->s32Left -= u8BitsCount; 
        //��ǰָ���ֽ��Ѿ�û�п���λ
        if(ptBits->s32Left == 0)                 
        {              
            //�ƶ�ָ�뵽��һ�ֽ� 
            ptBits->pu8Current++;   
            //����λ�����ָ�
            ptBits->s32Left = BYTE_BITS_COUNT;               
        }                                                                       
        return u64Result;                                                        
    }     
    //�����ǰ�ֽڿ���λ����  
    else                                            
    {                                                                           
        u64Result |= (*ptBits->pu8Current & u64Mask64[ptBits->s32Left]) << -s8Shr;
        //���㻹Ҫ��ȡ��һ�ֽڵ�λ��
        u8BitsCount  -= ptBits->s32Left;  
        //�ƶ�ָ�뵽��һ�ֽ�
        ptBits->pu8Current++;    
        //����λ�����ָ�
        ptBits->s32Left = BYTE_BITS_COUNT;                   
    }                                                                           
}
*/
    return u64Result;
}

/*=================================================================================
������:BitsWrite8
����:���ַ���дָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫд��λ��
		 [I] u16Value Ҫд��ֵ
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u8 BitsWrite8(TBit *ptBits, u8 u8BitsCount, u8 u8Value)
{
    CHECK_BITS_COUNT(8);

    WRITE_BITS_BITS(u8BitsCount, u8Value);

    return u8Value;
}

/*=================================================================================
������:BitsWrite16
����:���ַ���дָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫд��λ��
		 [I] u16Value Ҫд��ֵ
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u16 BitsWrite16(TBit *ptBits, u8 u8BitsCount, u16 u16Value)
{
    CHECK_BITS_COUNT(16);

    WRITE_BITS_BITS(u8BitsCount, u16Value);

    return u16Value;
}

/*=================================================================================
������:BitsWrite32
����:���ַ���дָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫд��λ��
         [I] u64Value Ҫд��ֵ
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u32 BitsWrite32(TBit *ptBits, u8 u8BitsCount, u32 u32Value)
{
    CHECK_BITS_COUNT(32);

    WRITE_BITS_BITS(u8BitsCount, u32Value);

    return u32Value;
}

/*=================================================================================
������:BitsWrite64
����:���ַ���дָ�����ȵ�λ
�㷨ʵ��: 
����˵��:
         [I/O] ptBits �洢�ַ���Ϣ�Ľṹ
         [I] u8BitsCount Ҫд��λ��
         [I] u64Value Ҫд��ֵ
����ֵ˵��:
-----------------------------------------------------------------------------------
�޸ļ�¼:
����        �汾        �޸���        �߶���        �޸ļ�¼
2008.06.13  0.1         ��ʤ��                      ���մ���淶�޸�
=================================================================================*/
u64 BitsWrite64(TBit *ptBits, u8 u8BitsCount, u64 u64Value)
{
    CHECK_BITS_COUNT(64);

    WRITE_BITS_BITS(u8BitsCount, u64Value);

    return u64Value;
}


