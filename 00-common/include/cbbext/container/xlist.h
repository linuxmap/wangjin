/*========================================================================================
模块名    ：双向链表容器
文件名    ：xlist.h
相关文件  ：
实现功能  ：实现双向链表的摸板
作者      ：fanxg
版权      ：<Copyright(C) 2003-2008 Suzhou Keda Technology Co., Ltd. All rights reserved.>
-------------------------------------------------------------------------------------------
修改记录：
日期               版本              修改人             走读人              修改记录
2010/06/08         V1.0              fanxg                                  新建文件
=========================================================================================*/

#ifndef _XLIST_INCLUDED_
#define _XLIST_INCLUDED_

#ifndef assert
#define  assert
#endif

//typedef unsigned int size_t;

#ifndef XITERATOR
#define XITERATOR

typedef struct Pos_Pointer {} *XPos;
static const XPos ITERATOR_BEGIN =  ((XPos)-1L);
static const XPos ITERATOR_END =  ((XPos)0L);
class Iterator
{
public:

	explicit Iterator(XPos Pos = ITERATOR_BEGIN) :m_Pos(Pos){}
	~Iterator(){m_Pos = ITERATOR_END;}

	Iterator(void* p) {m_Pos = (XPos)p;}
	Iterator& operator= (void* p) {m_Pos = (XPos)p;return *this;}
	BOOL operator== (void* p) const { return (m_Pos == (XPos)p);}
	XPos operator& () const {return m_Pos;}
	BOOL Begin() const { return (m_Pos == ITERATOR_BEGIN);}
	BOOL End() const { return (m_Pos == ITERATOR_END);}
	BOOL Valid() const { return (!Begin() && !End());}

private:
	XPos m_Pos;
};

#endif //#ifndef XITERATOR

template <class T>
class CXList
{
public:
	struct CLink
	{
		CLink* pNext;
		CLink* pPrev;
		T data;

		CLink() :pNext(0), pPrev(0){}
	};

public:
	CXList(u32 nCapacity = 1)
	{
		m_nSize = 0;
		m_nCapacity = 0;
		m_pNodeHead = m_pNodeTail = m_pNodeFree = 0;
		CreateFreeList(nCapacity);
	}

	~CXList()
	{
		Empty();
	}

public:
	CXList(const CXList& cOther)
	{
		Copy(cOther);
	}

	CXList& operator=(const CXList& cOther)
	{
		if (this != &cOther)
		{
			Empty();
			Copy(cOther);
		}

		return *this;
	}

public:

	u32 GetSize() const
	{
		return m_nSize;
	}

	BOOL32 IsEmpty()
	{
		return (m_nSize == 0);
	}

	u32 GetCapacity() const
	{
		return m_nCapacity;
	}

	T& GetHead()
	{
		assert(m_pNodeHead != 0);
		return m_pNodeHead->data;
	}

	BOOL32 GetHead(T& rHeadData) const
	{
		if (m_pNodeHead == 0) return false;

		rHeadData = m_pNodeHead->data;
		return true;
	}

	const T& GetHead() const
	{
		assert(m_pNodeHead != 0);
		return m_pNodeHead->data;
	}

	T& GetTail()
	{
		assert(m_pNodeTail != 0);
		return m_pNodeTail->data;
	}

	BOOL32 GetTail(T& rTailData) const
	{
		if (m_pNodeTail == 0) return false;

		rTailData = m_pNodeTail->data;
		return true;
	}

	const T& GetTail() const
	{
		assert(m_pNodeTail != 0);
		return m_pNodeTail->data;
	}

	T& Get(Iterator pPos)
	{
		assert(pPos.Valid());
		CLink* pCurr = (CLink*)&pPos;
		return pCurr->data;
	}

	BOOL32 Get(Iterator pPos, T& rData) const
	{
		if (!pPos.Valid()) return false;

		CLink* pCurr = (CLink*)&pPos;
		rData = pCurr->data;
		return true;
	}

	const T& Get(Iterator pPos) const
	{
		assert(pPos.Valid());
		CLink* pCurr = (CLink*)&pPos;
		return pCurr->data;
	}

	void Set(const T& rNewElement, Iterator pPos)
	{
		assert(pPos.Valid());
		CLink* pCurr = (CLink*)&pPos;
		pCurr->data = rNewElement;
	}

	Iterator GetHeadLink() const
	{
		return m_pNodeHead;
	}

	Iterator GetTailLink() const
	{
		return m_pNodeTail;
	}

	T& GetNext(Iterator& pPos)
	{
		assert(this);
		return const_cast<T&>(((const CXList*)this)->GetNext(pPos));
	}

	const T& GetNext(Iterator& pPos) const
	{
		assert(!pPos.End());

		if (pPos.Begin())
		{
			pPos = m_pNodeHead->pNext;
			return m_pNodeHead->data;
		}

		CLink* pCurr = (CLink*)&pPos;
		pPos = pCurr->pNext;

		return pCurr->data;
	}

	BOOL32 GetNext(Iterator& pPos, T& rData) const
	{
		if (m_pNodeHead == 0 || pPos.End()) return false;

		rData = ((const CXList*)this)->GetNext(pPos);
		return true;
	}

	T& GetPrev(Iterator& pPos)
	{
		assert(this);
		return const_cast<T&>(((const CXList*)this)->GetPrev(pPos));
	}

	const T& GetPrev(Iterator& pPos) const
	{
		assert(!pPos.Begin());

		if (pPos.End())
		{
			pPos = m_pNodeTail->pPrev;
			return m_pNodeTail->data;
		}

		CLink* pCurr = (CLink*)&pPos;
		pPos = pCurr->pPrev;

		return pCurr->data;
	}

	BOOL32 GetPrev(Iterator& pPos, T& rData) const
	{
		if (m_pNodeTail == 0 || pPos.Begin()) return false;

		rData = ((const CXList*)this)->GetPrev(pPos);
		return true;
	}

	Iterator InsertHead(const T& rNewElement)
	{
		CLink* pNewNode = CreateNode(0, m_pNodeHead);
		if (pNewNode == 0) return ITERATOR_END;

		pNewNode->data = rNewElement;
		if (m_pNodeHead != 0)
		{
			m_pNodeHead->pPrev = pNewNode;
		}
		else
		{
			m_pNodeTail = pNewNode;
			m_pNodeTail->pNext = (CLink*)ITERATOR_END;
		}

	    m_pNodeHead = pNewNode;
		m_pNodeHead->pPrev = (CLink*)ITERATOR_BEGIN;
		return pNewNode;
	}

	Iterator InsertTail(const T& rNewElement)
	{
		CLink* pNewNode = CreateNode(m_pNodeTail, 0);
		if (pNewNode == 0) return ITERATOR_END;

		pNewNode->data = rNewElement;

		if (m_pNodeTail != 0)
		{
			m_pNodeTail->pNext = pNewNode;
		}
		else
		{
			m_pNodeHead = pNewNode;
			m_pNodeHead->pPrev = (CLink*)ITERATOR_BEGIN;
		}

		m_pNodeTail = pNewNode;
		m_pNodeTail->pNext = (CLink*)ITERATOR_END;
	    return pNewNode;
	}

	Iterator InsertBefor(const T& rNewElement, Iterator pPos)
	{
		assert(pPos.Valid());
		if (pPos.End() || pPos.Begin()) return ITERATOR_END;

		if (pPos == m_pNodeHead)
		{
			return InsertHead(rNewElement);
		}

		CLink* pOldNode = (CLink*)&pPos;
		CLink* pNewNode = CreateNode(pOldNode->pPrev, pOldNode);
		if (pNewNode == 0) return ITERATOR_END;

		pNewNode->data = rNewElement;

		if (pOldNode->pPrev != 0)
		{
			pOldNode->pPrev->pNext = pNewNode;
		}
		else
		{
			m_pNodeHead = pNewNode;
		}

	    pOldNode->pPrev = pNewNode;
		return pNewNode;
	}

	Iterator InsertAfter(const T& rNewElement, Iterator pPos)
	{
		assert(pPos.Valid());
		if (pPos.End() || pPos.Begin()) return ITERATOR_END;

		if (pPos == m_pNodeTail)
		{
			return InsertTail(rNewElement);
		}

		CLink* pOldNode = (CLink*)&pPos;
		CLink* pNewNode = CreateNode(pOldNode, pOldNode->pNext);
		if (pNewNode == 0) return ITERATOR_END;

		pNewNode->data = rNewElement;

		if (pOldNode->pNext != 0)
		{
			pOldNode->pNext->pPrev = pNewNode;
		}
		else
		{
			m_pNodeTail = pNewNode;
		}

		pOldNode->pNext = pNewNode;
		return pNewNode;
	}

	Iterator Find(const T& rVal, Iterator pStartPos = ITERATOR_BEGIN) const
	{
		assert(m_pNodeHead != 0 && m_pNodeTail != 0 && !pStartPos.End());
		if (m_pNodeHead == 0 || m_pNodeTail == 0 || pStartPos.End())
			return ITERATOR_END;

		Iterator pPos = pStartPos.Begin() ? m_pNodeHead : pStartPos;
		while(!pPos.End())
		{
			Iterator pPrev = pPos;
			if (rVal == GetNext(pPos))
				return pPrev;
		}

		return ITERATOR_END;
	}

	BOOL32 RemoveHead()
	{
		assert(m_pNodeHead != 0);
		if (m_pNodeHead == 0) return false;

		CLink* pOldNode = m_pNodeHead;
		m_pNodeHead = (m_pNodeHead == m_pNodeTail) ? 0 : pOldNode->pNext;

		if (m_pNodeHead != 0)
			m_pNodeHead->pPrev = (CLink*)ITERATOR_BEGIN;
		else
			m_pNodeTail = 0;

		FreeNode(pOldNode);

		return true;
	}

	BOOL32 RemoveTail()
	{
		assert(m_pNodeTail != 0);
		if (m_pNodeTail == 0) return false;

		CLink* pOldNode = m_pNodeTail;
		m_pNodeTail = (m_pNodeHead == m_pNodeTail) ? 0 : pOldNode->pPrev;

		if (m_pNodeTail != 0)
			m_pNodeTail->pNext = (CLink*)ITERATOR_END;
		else
			m_pNodeHead = 0;

		FreeNode(pOldNode);

		return true;
	}

	BOOL32 Remove(Iterator pPos)
	{
		assert(pPos.Valid());
		if (pPos.Begin() || pPos.End())
			return false;

		CLink* pNode = (CLink*)&pPos;
		if (pNode == m_pNodeHead)
		{
			return RemoveHead();
		}
		else if (pNode == m_pNodeTail)
		{
			return RemoveTail();
		}
		else
		{
			pNode->pPrev->pNext = pNode->pNext;
			pNode->pNext->pPrev = pNode->pPrev;
			FreeNode(pNode);

			return true;
		}
	}

	u32 Erase(const T& cElem)
	{
		u32 nEraseNum = 0;
		if (m_pNodeHead == 0) return nEraseNum;

		Iterator pPos = m_pNodeHead;
		while(!pPos.End())
		{
			Iterator pPrev = pPos;
			T tElem;
			if (GetNext(pPos, tElem) && tElem == cElem && Remove(pPrev))
			{
				nEraseNum++;
			}
		}

		return nEraseNum;
	}

	void Empty()
	{
		DestroyNodeList(m_pNodeHead);
		DestroyNodeList(m_pNodeFree);

		m_pNodeHead = m_pNodeTail = m_pNodeFree = 0;
		m_nSize = 0;
		m_nCapacity = 0;
	}

private:
	BOOL32 CreateFreeList(u32 nSize)
	{
		if (m_pNodeFree == 0)
		{
			if (nSize == 0) nSize = 1;

			for(u32 i=0; i<nSize; i++)
			{
				CLink* pFreeNode = new CLink;
				if (pFreeNode == 0)
				{
					DestroyNodeList(m_pNodeFree);
					m_pNodeFree = 0;
					return false;
				}

				pFreeNode->pNext = m_pNodeFree;
				m_pNodeFree = pFreeNode;
				m_nCapacity++;
			}
		}

		return true;
	}

	void DestroyNodeList(CLink* pNode)
	{
		if (pNode == 0)
			return;

		CLink* pCurr = pNode;
		CLink* pNext = pNode->pNext;

		while(pCurr != 0)
		{
			pNext = pCurr->pNext;
			delete pCurr;
			pCurr = pNext;
			m_nCapacity--;
		}
	}

	CLink* CreateNode(CLink* pPrev, CLink* pNext)
	{
		if (m_pNodeFree == 0 && !CreateFreeList(m_nCapacity))
		{
			return 0;
		}

		CLink* pNode = m_pNodeFree;
		m_pNodeFree = m_pNodeFree->pNext;
		pNode->pPrev = pPrev;
		pNode->pNext = pNext;
		m_nSize++;

		return pNode;
	}

	void FreeNode(CLink* pNode)
	{
		if (pNode != 0)
		{
			pNode->data = T();
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
			m_nSize--;
		}
	}

	void Copy(const CXList& cOther)
	{
		u32 nCapacity = cOther.m_nCapacity;
		if (nCapacity <= 0) return;

		m_nSize = 0;
		m_nCapacity = 0;
		m_pNodeHead = m_pNodeTail = m_pNodeFree = 0;
		CreateFreeList(nCapacity);

		CLink* pHead = cOther.m_pNodeHead;
		if (pHead == 0) return;

		CLink* pCurr = pHead;

		while(pCurr != 0)
		{
			InsertTail(pCurr->data);
			pCurr = pCurr->pNext;
		}
	}

private:
	CLink* m_pNodeHead;
	CLink* m_pNodeTail;
	u32 m_nSize;
	CLink* m_pNodeFree;
	u32 m_nCapacity;
};

#endif //#ifndef _XLIST_INCLUDED_