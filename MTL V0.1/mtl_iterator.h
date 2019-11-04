/**
 * Author: bbbyk
 * Time: 2019.11
 * 
 * Realize the contain's iterator.
 * **/

#ifndef MTL_ITERATOR_H
#define MTL_ITERATOR_H

#include <cstddef>

// five iterator type
struct input_iterator_tag { };
struct output_iterator_tag { };
struct forward_iterator_tag : public input_iterator_tag { };
struct bidirect_iterator_tag : public forward_iterator_tag { };
struct random_access_iterator_tag : public bidirect_iterator_tag { };

// iterator

// traits to get the type of contain we need
template < class I>
class iterator_traits
{
public:
    typedef typename I::value_type value_type;
    typedef typename I::difference_type difference_type;
    typedef typename I::reference reference;
    typedef typename I::pointer pointer;
    typedef typename I::random_access_iterator_tag iterator_category;
};

// the particularize of traits (native-pointer)
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

// the particularize of traits (pointer-to-const)
template <class T>
class iterator_traits<const T*>
{
public:
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef const T& reference;
    typedef const T* pointer; 
    typedef random_access_iterator_tag iterator_category;
}; 

/**
 * another function to use iterator eazily.
 * **/
// get the category of iterator
template <class I>
inline typename iterator_traits<I>::iterator_category
iterator_category(const I&) {
    typename iterator_traits<I>::iterator_category category;
    return category();
}

// distance type of iterators
// use pointer of '0' to represent
template <class I>
inline typename iterator_traits<I>::difference_type*
distant_type(const I&) {
    return static_cast<typename iterator_traits<I>::difference_type*>(0);
}

// vaule type
template <class I>
inline typename iterator_traits<I>::value_type*
vaule_type(const I&) {  
    return static_cast<typename iterator_traits<I>::value_type*>(0);
}

/**
 * distance function 
 * use iterator type to override
 * **/
template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    for (; first != last; first++)
        n++;
    return n;
}

template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, random_access_iterator_tag)
{
    return last - first;
}

// must override <=
template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
    if (first <= last)
        return  __distance(first, last, typename iterator_traits<InputIterator>::iterator_category());
    else
        return -(__distance(last, first, typename iterator_traits<InputIterator>::iterator_category()));
}

/**
 * advance function
 * advance n elements
 * **/
template <class InputIterator, class Distance>
inline void __advance(InputIterator& iter, Distance n, input_iterator_tag)
{
    while(n--) iter++;
}

template <class InputIterator, class Distance>
inline void __advance(InputIterator& iter, Distance n, bidirect_iterator_tag)
{
    if (n >= 0)
        while(n--) iter++;
    else 
        while(n++) iter--;
}

template <class InputIterator, class Distance>
inline void __advance(InputIterator& iter, Distance n, random_access_iterator_tag)
{
    iter += n;
}

template <class InputIterator, class Distance>
inline void advance (InputIterator& iter, Distance n)
{
    __advance(iter, n, iterator_category(iter));
}
#endif