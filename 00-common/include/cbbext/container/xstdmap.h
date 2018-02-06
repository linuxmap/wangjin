/********************************************************************
	created:	2010/09/27
	file base:	xstdmap
	author:		张应能

	purpose:	将CXMap与std::map兼容之.
    版权        : Copyright(C) 2003-2010 Suzhou Keda Technology Co., Ltd. All rights reserved
*********************************************************************/

#ifndef CX_STD_MAP_H
#define CX_STD_MAP_H

#ifndef __GNUC__
#pragma warning( disable : 4786 )
#endif

#include "xmap.h"
#include <utility>

/**
 *  这个类只是配合 CXStdMap, 用来与std::pair保持一致.
 */
template<class KEY, class VALUE>
class CXPairAdapter
{
public:
    CXPairAdapter(KEY& theKey,VALUE& theValue) : first(theKey),second(theValue)
    {

    }
    CXPairAdapter* operator->()
    {
        return this;
    }
    const CXPairAdapter* operator->()const
    {
        return this;
    }
public:
    KEY& first;
    VALUE& second;
};

/**
 *  目前并不是完全兼容, 只是我用到的函数进行了兼容.
 */
template<class KEY, class VALUE, class HASH = CHashFunctor<KEY>, class EQUAL = CEqualFunctor<KEY> >
class CXStdMap : protected CXMap<KEY,VALUE,HASH,EQUAL>
{
public:
    typedef CXMap<KEY,VALUE,HASH,EQUAL> ContainerBase;
    using ContainerBase::operator[];

    typedef Iterator CXIterator;

    /**
     *  模拟std map 的 iterator
     */
    class CStdIterator : protected CXIterator
    {
    public:
        explicit CStdIterator(ContainerBase& container,BOOL32 bEnd = false): m_pKey(0),m_pValue(0),
            m_bEnd(false),
            m_container(container)
        {
        }
        CStdIterator(ContainerBase& container,CXIterator iter,BOOL32 bEnd = false):m_pKey(0),m_pValue(0),
            m_bEnd(bEnd), m_container(container)
        {
            (CXIterator&)(*this) = iter;
        }

        CStdIterator& operator ++ ()
        {
            BOOL32 bGet = m_container.Iterate(*this,m_pKey,m_pValue);
            if (!bGet)
            {
                CXIterator iterEnd(ITERATOR_END);
                (CXIterator&)(*this) = iterEnd;
                m_bEnd = true;
            }
            return *this;
        }
        CStdIterator& operator ++ ()const
        {
            return const_cast<CStdIterator*>(this)->operator++();
        }

        BOOL32 operator==(const CStdIterator& iter)const
        {
            BOOL32 bEqual = (const CXIterator&)iter == &(const CXIterator&)(*this);
            bEqual &= (m_bEnd == iter.m_bEnd);
            return bEqual;
        }

        BOOL32 operator!=(const CStdIterator& iter)const
        {
            return !(operator==(iter));
        }


        CXPairAdapter<KEY,VALUE> operator*()
        {
            CXPairAdapter<KEY,VALUE> thePair(*m_pKey,*m_pValue);
            return thePair;
        }
        CXPairAdapter<KEY,VALUE> operator->()
        {
            CXPairAdapter<KEY,VALUE> thePair(*m_pKey,*m_pValue);
            return thePair;
        }

        const CXPairAdapter<KEY,VALUE> operator*()const
        {
            return const_cast<CStdIterator*>(this)->operator*();
        }
        const CXPairAdapter<KEY,VALUE> operator->()const
        {
            return const_cast<CStdIterator*>(this)->operator->();
        }

    public:
        CXIterator& GetCXIter()
        {
            return (CXIterator&)(*this);
        }
    public:
        KEY* m_pKey; //std的当前信息是保存在iterator中的.CXMap中的Iterator是下一个Entry.
        VALUE* m_pValue;
        BOOL32 m_bEnd;//Iterate遍历时最后一个有效的item时CXIterator就已经到End了.
    protected:
        ContainerBase& m_container;
    };

    typedef CStdIterator iterator;
    typedef const CStdIterator const_iterator;
public:
    iterator begin()
    {
        CStdIterator iter(*this,ITERATOR_BEGIN);

        BOOL32 bGet = ContainerBase::Iterate(iter.GetCXIter(),iter.m_pKey,iter.m_pValue);
        if(!bGet)
        {
            return end();
        }

        return iter;
    }
    const_iterator begin()const
    {
        return const_cast<CXStdMap*>(this)->begin();
    }

    iterator end()
    {
        CStdIterator iter(*this,ITERATOR_END,true);
        return iter;
    }
    const_iterator end()const
    {
        return const_cast<CXStdMap*>(this)->end();
    }

    typedef std::pair<KEY,VALUE> CPairKeyValue;
    void insert(const CPairKeyValue& pairKeyValue)
    {
        ContainerBase::Insert(pairKeyValue.first,pairKeyValue.second);
    }

    u32 size()const
    {
        return ContainerBase::GetSize();
    }
    void erase(const KEY& key)
    {
        ContainerBase::Erase(key);
    }
    iterator find(const KEY& theKey)
    {
        u32 nHash = 0;
        typename ContainerBase::CEntry* pEntryRet = ContainerBase::GetEntryAt(theKey, nHash);
        if (!pEntryRet)
        {
            return end();
        }

        // find next entry
		typename ContainerBase::CEntry* pEntryNext = 0;
		if ((pEntryNext = pEntryRet->pNext) == 0)
		{
			// go to next bucket
            for (u32 nBucket = pEntryRet->nHashValue + 1; nBucket < ContainerBase::m_nHashTableSize; nBucket++)
			{
                if ((pEntryNext = ContainerBase::m_pHashTable[nBucket]) != 0)
					break;
			}
		}

        CXIterator cxIter;
        cxIter = pEntryNext;
        CStdIterator iter(*this,cxIter);
        iter.m_pKey = &pEntryRet->key;
        iter.m_pValue = &pEntryRet->value;
        return iter;
    }
    const_iterator find(const KEY& theKey)const
    {
        return const_cast<CXStdMap*>(this)->find(theKey);
    }
    void clear()
    {
        ContainerBase::Empty();
    }
};

#endif



//END OF FILE
