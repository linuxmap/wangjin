#ifndef _XSTRING_INCLUDED_
#define _XSTRING_INCLUDED_

#include "osp.h"

class CXString
{
public:
    CXString(const char* pStr=0)
	{
		CopyStrToSelf(pStr);
	}

    CXString(const CXString & other)
	{
		CopyStrToSelf(other.m_pData);
	}

	~CXString()
	{
		FreeStrBuffer(m_pData);
	}

	BOOL32 operator==(const CXString& other) const
	{
		return (strcmp(m_pData, other.m_pData) == 0);
	}

	BOOL32 operator==(const char* pStr) const
	{
		return (pStr == 0) ? false : (strcmp(m_pData, pStr) == 0);
	}

    const char& operator[](unsigned int nIndex) const
	{
		return m_pData[nIndex];
	}

	char& operator[](unsigned int nIndex)
	{
		return m_pData[nIndex];
	}

	operator const char*() const
	{
		return m_pData;
	}

    CXString& operator=(const CXString& other)
	{
		return ((this != &other) ? AssignStrToSelf(other.m_pData) : *this);
	}

	CXString& operator=(const char* pStr)
	{
		return AssignStrToSelf(pStr);
	}

	CXString& operator+=(const CXString& other)
	{
		return AddStrToSelf(other.m_pData);
	}

	CXString& operator+=(const char* pStr)
	{
		return AddStrToSelf(pStr);
	}

    CXString operator+(const CXString &other) const
	{
		CXString newStr;
		return((newStr += *this) += other);
	}

	CXString& operator+=(u8 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%u", nNum);
		return AddStrToSelf(szNum);
	}

	CXString& operator+=(u16 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%u", nNum);
		return AddStrToSelf(szNum);
	}

	CXString& operator+=(u32 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%lu", nNum);
		return AddStrToSelf(szNum);
	}

	CXString& operator+=(s8 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%d", nNum);
		return AddStrToSelf(szNum);
	}

	CXString& operator+=(s16 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%d", nNum);
		return AddStrToSelf(szNum);
	}

	CXString& operator+=(s32 nNum)
	{
		char szNum[64] = {0};
		sprintf(szNum, "%d", nNum);
		return AddStrToSelf(szNum);
	}

	u32 Size() const
	{
		return strlen(m_pData);
	}

	const char* c_str() const
	{
		return m_pData;
	}

	CXString& AddChar(char ch)
	{
		char szCh[2] = {0};
		szCh[0] = ch;
		return AddStrToSelf(szCh);
	}

	void Empty()
	{
		AssignStrToSelf("");
	}

private:
	void CopyStrToSelf(const char* pStr)
	{
		int nLen = (pStr==0) ? 0 : strlen(pStr);
		pStr = (pStr==0) ? "" : pStr;
		m_pData = AllocStrBuffer(nLen+1);
        if(m_pData)
        {
            strcpy(m_pData, pStr);
		}
	}

	CXString& AssignStrToSelf(const char* pStr)
	{
		FreeStrBuffer(m_pData);
        CopyStrToSelf(pStr);
		return *this;
	}

	CXString& AddStrToSelf(const char* pStr)
	{
		if (pStr)
		{
			char* pNewBuf = AllocStrBuffer(strlen(m_pData) + strlen(pStr) + 1);
			strcpy(pNewBuf, m_pData);
			strcat(pNewBuf, pStr);
			FreeStrBuffer(m_pData);
			m_pData = pNewBuf;
		}
		return *this;
	}

	char* AllocStrBuffer(u32 nSize)
	{
		return (char*)(::OspAllocMem(nSize));
	}

	void FreeStrBuffer(void* pStr)
	{
		::OspFreeMem(pStr);
	}

private:
    char* m_pData;
};

#endif //#ifndef _XSTRING_INCLUDED_


//end of file

