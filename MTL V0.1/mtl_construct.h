/**
 * Author: bbbyk
 * Data: 2019.11.5
 * 
 * The global construct and destory function for the elements
 * **/
#ifndef MTL_CONSTRUCT_H
#define MTL_CONSTRUCT_H
#include <new>
#include "mtl_iterator.h"
#include "type_traits.h"
// 使用placment new 放置值到堆上
template <class T1, class T2>
inline void construct(T1* p, const T2& value) {
    new(p) T2(value);
}

template <class T>
inline void construct(T* p) {
    new (p) T1();
}

// 单元素析构
template <class T>
inline void destroy(T* p) {
    p->~T();
}

// no-trivial destructor 什么都不用做
template <class Iterator>
inline void __destroy_aux(Iterator first, Iterator last, __true_type) {

}

// trivial destructor 需要调用其析构函数进行释放
template <class Iterator>
inline void __destroy_aux(Iterator first, Iterator last, __false_type) {
    for (; first <= last; first++) 
        destroy(&*first); // 传native pointer
}

// T*根据实际的value_type生成相应的函数版本
template <class Iterator, class T>
inline void __destroy(Iterator first, Iterator last, T*) {
    typedef typename __type_traits<T>::has_trivial_destructor destructor;
    __destroy_aux(first, last, destructor());
}

// destroy函数， no-trivial对象则不做任何处理
template <class Iterator>
inline void destroy(Iterator first, Iterator last) {
    __destroy(first, last, value_type(first));
}

// 特化版本，对于内置类型的数组也不用管
// 其中部分指针类型在type_traits.h中没有定义成no-trivial destructor
inline void destroy(char*, char*) {}
inline void destroy(int*, int*) {}
inline void destroy(long*, long*) {}
inline void destroy(float*, float*) {}
inline void destroy(double*, double*) {}
#endif