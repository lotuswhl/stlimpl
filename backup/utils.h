#ifndef UTILS_H
#define UTILS_H
#include "macros.h"
#include <type_traits>

USING_NAMESPACE_FASTSTL_BEGIN

// ʹ�ö�λnew��ָ�����ڴ�λ�ù������ֵ
template<typename T1,typename T2>
inline void construct(T1*p,const T2&value){
    new (p) T2(value);
}

// ���ö������������
template<typename T>
inline void destroy(T* p){
    p->~T();
}

//����ӵ��trival��������������ɶҲ����
template<typename ForwardIterator>
inline void _destroy(ForwardIterator beg,ForwardIterator end,std::true_type*){}
//����ӵ��non-trival��������������ѭ������Ԫ�ص���������
template<typename ForwardIterator>
inline void _destroy(ForwardIterator beg,ForwardIterator end,std::false_type*){
    for(;beg!=end;++beg){
        destroy(&*beg);
    }
}

template<typename ForwardIterator>
inline void destroy(ForwardIterator beg,ForwardIterator end){
    _destroy(beg,end,
             (std::is_trivially_destructible<typename std::iterator_traits<ForwardIterator>::value_type >*)0);
}




// ��char*��wchar_t���ػ��汾,����Ҫ���κ�����
template<>
inline void destroy(char*,char*){}
template<>
inline void destroy(wchar_t*,wchar_t*){}

USING_NAMESPACE_FASTSTL_END


#endif // UTILS_H
