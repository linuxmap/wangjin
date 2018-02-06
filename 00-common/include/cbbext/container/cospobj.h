/*****************************************************************************
工程名		: KTM2.0
模块名		: CMU
文件名		: cospobj.h
相关文件	: cospobj.h
文件实现功能:
作者		:
版本		: <Copyright(C) 1997-2010 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-----------------------------------------------------------------------------
修改记录 :
日  期      版本        修改人       走读人      修改记录
2010/07/20  V1R4B3      fanxg                     创建
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
 * COspObj 作为所有类的基类，该类重载了new 和 delete，
   通过他们的重载可以实现使用osp的内存池来管理所有类
   的对象，避免了频繁new/delete给系统带来的内存碎片。

   用法：
   对于任何COspObj的子类CxxxObj，

   创建：
   CxxxObj* pObj = new CxxxObj;
   也可以这样：
   COspObj* pObj = new CxxxObj;

   销毁：
   delete pObj;

   这里的new/delete会调用重载的operator new/operator delete,
   也就是让osp的内存池接管内存的分配和释放。

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
    //VC6编译器的Bug, 避免Warning
    static void operator delete(void *p)
#else
	static void operator delete(void *p, u32 size)
#endif
	{
		MemPoolFree(p);
	}
};



/*
 * 对于并非从COspObj继承的类，如果想使用osp内存池管理对象，
   可以使用以下两个模板。

   用法：
   对于类CxxxObj创建：
   CxxxObj* pObj = OspAllocObj<CxxxObj>();

   对于类CxxxObj销毁：
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

