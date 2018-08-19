#ifndef UTILS_H
#define UTILS_H
#include "macros.h"
#include <type_traits>

USING_NAMESPACE_FASTSTL_BEGIN

// 使用定位new在指定的内存位置构造对象值
template<typename T1,typename T2>
inline void construct(T1*p,const T2&value){
    new (p) T2(value);
}

// 调用对象的析构函数
template<typename T>
inline void destroy(T* p){
    p->~T();
}

//对于拥有trival的析构函数，则啥也不做
template<typename ForwardIterator>
inline void _destroy(ForwardIterator beg,ForwardIterator end,std::true_type*){}
//对于拥有non-trival的析构函数，则循环调用元素的析构函数
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




// 对char*和wchar_t的特化版本,不需要做任何事情
template<>
inline void destroy(char*,char*){}
template<>
inline void destroy(wchar_t*,wchar_t*){}

USING_NAMESPACE_FASTSTL_END


#endif // UTILS_H
