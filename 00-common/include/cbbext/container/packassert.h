#ifndef _XPACK_ASSERT_INCLUDED_
#define _XPACK_ASSERT_INCLUDED_

#ifdef _MSC_VER
typedef __int64			s64;
typedef unsigned __int64 u64;
#else 
typedef long long		s64;
typedef unsigned long long u64;
#endif 

//��ֹ���� push �� 1, ���¼��㲻��ȷ
//����ָ������ֵ, ʹָ������ֵʧЧ,�Ա�֤ T ʹ���������ֵ.
#ifdef _MSC_VER
#pragma pack(push, 16)
#elif defined (_LINUX_)
#pragma pack(16)
#else
#error not supports this platform
#endif


template <class T>
class ispointer_traits
{
private:   
    class Small
    {
    public:
        unsigned char by;
    };
    
    
    class Big
    {
    public:
        unsigned char by;
        unsigned char by1;
    };

    static Small IsPointerHelper(...);    
    static Big IsPointerHelper(void*);

    static T makeT();
public:
    enum {
        ispointer = 
            sizeof(IsPointerHelper(makeT())) - 1
    };
};

template <class T>
class CAlignmentTraits
{
public:
    /**
     *  ���b��c֮��û��pading�Ļ�,����1�ֽڶ����
     *  padingֵ = sizeof(AlignmentFinder) - sizeof(T) - 1, ��ʵ���� T ������packֵ.
     */
    struct AlignmentFinder
    {
        char b;
        T c;
    };
    enum {ispointer = ispointer_traits<T>::ispointer};
    enum {IsAlignByOneByte = ispointer || ( 1 == sizeof(AlignmentFinder) - sizeof(T) ) };
};

template <>
class CAlignmentTraits<signed char>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<unsigned char>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<signed short>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<unsigned short>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<signed long>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<unsigned long>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<signed int>
{
	public:    
		enum {IsAlignByOneByte = 1};
};


template <>
class CAlignmentTraits<unsigned int>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<s64>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

template <>
class CAlignmentTraits<u64>
{
	public:    
		enum {IsAlignByOneByte = 1};
};

#ifdef _LINUX_

template<class T> 
class CAlignmentTraits<T*>
{
public:    
		enum {IsAlignByOneByte = 1};
};

#endif

template<BOOL32>
struct MUST_ALIGN_WITH_ONE_BYTE;        //����һ��ģ���࣬�����壬ģ�����Ϊfasle���б������

template<>
struct MUST_ALIGN_WITH_ONE_BYTE<true> // ���ģ�����Ϊtrue�����ػ�
{
};

template<class T>
struct pack_assert
{
	pack_assert()
	{
		MUST_ALIGN_WITH_ONE_BYTE<CAlignmentTraits<T>::IsAlignByOneByte>();
	}
};

#ifdef _MSC_VER
#pragma pack(pop)
#elif defined (_LINUX_)
#pragma pack()
#else
#error not supports this platform
#endif

#endif  //#ifndef _XPACK_ASSERT_INCLUDED_




