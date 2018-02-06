/*========================================================================================
模块名    ：stack容器
文件名    ：xstack.h
相关文件  ：
实现功能  ：实现stack的摸板
作者      ：fanxg
版权      ：<Copyright(C) 2003-2008 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-------------------------------------------------------------------------------------------
修改记录：
日期               版本              修改人             走读人              修改记录
2010/06/08         V1.0              fanxg                                  新建文件
=========================================================================================*/

#ifndef _XSTACK_INCLUDED_
#define _XSTACK_INCLUDED_

#ifndef assert
#define  assert
#endif

//typedef unsigned int u32;

template <class T>
class CXStack
{
public:
	CXStack(u32 nCapacity = 1)  :m_pData(0), m_nSize(0), m_nCapacity(nCapacity)
	{
		m_pData = new T[m_nCapacity];
		assert(m_pData);
	}

	~CXStack()
	{
		Empty();
	}

public:

	CXStack(const CXStack& cOther)
	{
		Copy(cOther);
	}

	CXStack& operator=(const CXStack& cOther)
	{
		if (this != &cOther)
		{
			Empty();
			Copy(cOther);
		}

		return *this;
	}

public:

	BOOL32 Push(const T& rNewElem)
	{
		if (m_nCapacity == m_nSize)
		{
			if (!Resize(m_nCapacity * 2))
				return false;
		}

		m_pData[m_nSize++] = rNewElem;

		return true;
	}

	BOOL32 Pop(T& rElem)
	{
		if (m_nSize <= 0) return false;

		rElem = m_pData[--m_nSize];
		return true;
	}

	u32 GetSize() const
	{
		return m_nSize;
	}

	void Empty()
	{
		delete[] m_pData;
		m_pData = 0;
		m_nSize = 0;
		m_nCapacity = 0;
	}

	BOOL32 IsEmpty() const { return (m_nSize == 0); }

private:
	BOOL32 Resize(u32 nNewCapacity)
	{
		if (nNewCapacity == 0) nNewCapacity = 1;

		if (nNewCapacity <= m_nSize) return false;

		T* pNewData = new T[nNewCapacity];
		if (pNewData == 0) return false;

		for(u32 i=0; i<m_nSize; i++)
		{
			pNewData[i] = m_pData[i];
		}
		m_nCapacity = nNewCapacity;
		delete[] m_pData;
		m_pData = pNewData;

		return true;
	}

	BOOL32 Copy(const CXStack& cOther)
	{
		m_nCapacity = cOther.m_nCapacity;
		if (m_nCapacity <= 0) return false;

		m_pData = new T[m_nCapacity];
		if (m_pData == 0) return false;

		m_nSize = cOther.m_nSize;
		for(u32 i=0; i<cOther.m_nSize; i++)
		{
			m_pData[i] = cOther.m_pData[i];
		}

		return true;
	}

private:
	T* m_pData;
	u32 m_nSize;
	u32 m_nCapacity;
};

#endif //#ifndef _XSTACK_INCLUDED_