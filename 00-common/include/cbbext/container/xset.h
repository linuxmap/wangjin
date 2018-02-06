/*========================================================================================
ģ����    ��set����
�ļ���    ��xset.h
����ļ�  ��
ʵ�ֹ���  ��ʵ��set������
����      ��fanxg
��Ȩ      ��<Copyright(C) 2003-2008 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-------------------------------------------------------------------------------------------
�޸ļ�¼��
����               �汾              �޸���             �߶���              �޸ļ�¼
2010/06/08         V1.0              fanxg                                  �½��ļ�
=========================================================================================*/

#ifndef _XSET_INCLUDED_
#define _XSET_INCLUDED_

#include "xmap.h"

class CXVoid{};

template<class T, class HASH = CHashFunctor<T>, class EQUAL = CEqualFunctor<T> >
class CXSet :private CXMap<T, CXVoid, HASH, EQUAL>
{
	typedef CXMap<T, CXVoid, HASH, EQUAL> Base;

public:
	explicit CXSet(u32 nSetCapacity = DEFAULT_HASH_BLOCK_SIZE) :Base(nSetCapacity){};
	~CXSet(){};

public:
//default copy constructor is ok
//default operator= is ok

public:
	u32 GetSize() const
	{
		return Base::GetSize();
	}

	void Insert(const T& key)
	{
		Base::Insert(key, CXVoid());
	}

	BOOL32 Exist(const T& key) const
	{
		return Base::Exist(key);
	}

	BOOL32 Iterate(Iterator& rNextPosition, T& rKey) const
	{
		CXVoid cVoid;
		return Base::Iterate(rNextPosition, rKey, cVoid);
	}

	BOOL32 Erase(const T& key)
	{
		return Base::Erase(key);
	}

	void Empty()
	{
		Base::Empty();
	}

	BOOL32 IsEmpty() const { return Base::IsEmpty(); }

	void DumpPerform() {Base::DumpPerform();}
};

#endif //#ifndef _XSET_INCLUDED_

//end of file

