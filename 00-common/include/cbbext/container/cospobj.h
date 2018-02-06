/*****************************************************************************
������		: KTM2.0
ģ����		: CMU
�ļ���		: cospobj.h
����ļ�	: cospobj.h
�ļ�ʵ�ֹ���:
����		:
�汾		: <Copyright(C) 1997-2010 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-----------------------------------------------------------------------------
�޸ļ�¼ :
��  ��      �汾        �޸���       �߶���      �޸ļ�¼
2010/07/20  V1R4B3      fanxg                     ����
******************************************************************************/
#ifndef _COSPOBJ_INCLUDED_
#define _COSPOBJ_INCLUDED_

#define USE_GOOGLE_TCMALLOC

#ifndef USE_GOOGLE_TCMALLOC

#include "osp.h"
#define MemPoolMalloc OspAllocMem
#define MemPoolFree   OspFreeMem

#else

#define MemPoolMalloc malloc
#define MemPoolFree   free

#endif


/*
 * COspObj ��Ϊ������Ļ��࣬����������new �� delete��
   ͨ�����ǵ����ؿ���ʵ��ʹ��osp���ڴ��������������
   �Ķ��󣬱�����Ƶ��new/delete��ϵͳ�������ڴ���Ƭ��

   �÷���
   �����κ�COspObj������CxxxObj��

   ������
   CxxxObj* pObj = new CxxxObj;
   Ҳ����������
   COspObj* pObj = new CxxxObj;

   ���٣�
   delete pObj;

   �����new/delete��������ص�operator new/operator delete,
   Ҳ������osp���ڴ�ؽӹ��ڴ�ķ�����ͷš�

*/

class COspObj
{
public:
	COspObj() {}
	virtual ~COspObj() {}

public:
	static void* operator new(u32 size)
	{
		return MemPoolMalloc(size);
	}

#ifndef _LINUX_
    //VC6��������Bug, ����Warning
    static void operator delete(void *p)
#else
	static void operator delete(void *p, u32 size)
#endif
	{
		MemPoolFree(p);
	}
};



/*
 * ���ڲ��Ǵ�COspObj�̳е��࣬�����ʹ��osp�ڴ�ع������
   ����ʹ����������ģ�塣

   �÷���
   ������CxxxObj������
   CxxxObj* pObj = OspAllocObj<CxxxObj>();

   ������CxxxObj���٣�
   OspFreeObj<CxxxObj>(pObj);

*/

template <class T>
T* OspAllocObj()
{
	void* pObjMem = MemPoolMalloc(sizeof(T));
	if (pObjMem == NULL)
		return NULL;

	::new (pObjMem) T;

	return (T*)pObjMem;
}

template <class T>
void OspFreeObj(void* p)
{
	if (p == NULL)
		return;

	T* pObj = (T*)p;
	pObj->~T();

	MemPoolFree(p);
}

#endif

