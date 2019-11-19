/**
 * Author: bbbyk
 * Date: 2019.11.16
 * 定义三个全局的未初始化内存初始化操作。
 * **/

#ifndef MTL_UNINITIALIZED_h
#define MTL_UNINITIALIZED_h
#include "mtl_iterator.h"
#include "type_traits.h"
#include "stl_algobase.h"
// 根据POD类型具体实现fill_n
template <class ForwardIterator, class Size, class T>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                                     const T& x, __true_type)
{
    // 调用高阶fill_n
    return fill_n(first, n, x);
}

template <class ForwardIterator, class Size, class T>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                                     const T& x, __false_type)
{
    // 每个都调用构造函数
    for (; n > 0; --n, ++first)
        construct(&*first, x); // 传入的是原始指针的引用
    return first;
}

// 得到iterator的type_value,根据此type_value具现化函数
template <class ForwardIterator, class Size, class T>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, T*) {
    typedef typename __type_traits<T>::is_POD_type is_POD;
    __uninitialized_fill_n_aux(first, n , x, is_POD);
}

template <class ForwardIterator, class Size, class T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
    __uninitialized_fill_n(first, n, x, vaule_type(first)); // 利用模板具限化函数
}

template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator second, const T& x,  __true_type) {
    fill(first, second, x);
}

// @可优化！
// 是否能继续根据迭代器的类型优化 first!=second的判断耗时？
template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator second, const T& x, __false_type) {
    for (; first!=second; first++)
        construct(&*first, x);
}

template <class ForwardIterator, class T>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator second, const T& x) {
    typedef typename __traits_type<T>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, second, x, is_POD);
}


template <class ForwardIterator, class T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator second, const T& x) {
    return __uninitialized_fill(first, second, vaule_type(first));
}

// copy函数
#endif