#ifndef TRAITS_H
#define TRAITS_H
// 用来萃取迭代器的特性， VALUE_TYPE是迭代器的特性之一
// I可能是迭代器和原始指针 若为原始指针则使用接下来定义的特化版本
// 对于迭代器模板类型， 定义了value_type 表示我们实际的类型
#include <cstddef>

// 型别类，用来标记迭代器的类别
struct input_iterator_tag { };
struct output_iterator_tag { };
struct forward_iterator_tag : public input_iterator_tag { };
struct bidirect_iterator_tag : public forward_iterator_tag { };
struct random_access_iterator_tag : public bidirect_iterator_tag { };

template < class I>
class iterator_traits
{
public:
    typedef typename I::value_type;    
};

// 当I并不是类，即在I中没有value_type
// 采用特化类型
template <class T>
class iterator_traits<T*>
{
public:
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T& reference;
    typedef T* pointer; 
    typedef random_access_iterator_tag iterator_category;
}; 
/**
 * 使用方式
 ***/
// 迭代器类型 和 原生指针都适用
template <class I>
typename iterator_traits<I>::value_type
func(I iter)
{ }


#endif