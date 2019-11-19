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
#include "mtl_construct.h"
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
    ForwardIterator cur = first;
    try {
        for (; n > 0; --n, ++cur)
            construct(&*cur, x); // 传入的是原始指针的引用
    return cur;
    } catch(...) {
        destory(first, cur);
        throw;
    }
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
    ForwardIterator cur = first;
    try {
        for (; cur!=second; cur++)
            construct(&*cur, x);
    } catch(...) {
        destroy(first, cur);
        throw;
    }
}

template <class ForwardIterator, class T>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator second, const T& x) {
    typedef typename __traits_type<T>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, second, x, is_POD);
}


template <class ForwardIterator, class T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator second, const T& x) {
    __uninitialized_fill(first, second, vaule_type(first));
}

// copy函数
template <class ForwardIterator, class InputIterator>
inline ForwardIterator __uninitialized_copy_aux(ForwardIterator first, ForwardIterator last, InputIterator result, __true_type) {
    // 调用copy帮助再细分
    return copy(first, last, result);
}


template <class ForwardIterator, class InputIterator>
inline ForwardIterator __uninitialized_copt_aux(ForwardIterator first, ForwardIterator last, InputIterator result, __false_type) {
    // no-POD 用construct调用构造函数
    // 异常处理
    ForwardIterator cur = first;
    try{
        for(; cur != last; cur++, result++) {
            construct(&*cur, *result);
        }
        return result;
    } catch(...) { // 中途出错则返回原始状态
        destroy(first, cur)
        throw;
    }
    
}

template <class ForwardIterator, class InputIterator, class T>
inline ForwardIterator __uninitialized_copy(ForwardIterator first, ForwardIterator last, InputIterator result, T*) {
    typedef typename __type_traits<T>::is_POD_type is_POD;
    return __uninitialized_copy_aux(first, last, result, is_POD);    
}

template <class ForwardIterator, class InputIterator>
inline ForwardIterator uninitialized_copy(ForwardIterator first, ForwardIterator last, InputIterator result) {
    return __uninitialized_copy(first, last, result, vaule_type(first));
}


#endif