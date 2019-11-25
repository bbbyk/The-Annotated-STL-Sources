/** 
 * Author: bbbyk
 * Date: 2019.11.19
 * 
 * Contain Vecotor 
 * **/
#ifndef MTL_VECTOR_H
#define MTL_VECTOR_H

#include "mtl_alloc.h"
#include "mtl_uninitialized.h"
#include <string>
// 内存分配管理基本成员
template <class T, class Alloc>
class Vector_base {
public:
    typedef Alloc allocator_type;
    allocator_type get_allocator() const{ return allocator_type();}
    Vector_base(const Alloc&): _start(0), _end(0), _end_of_storage(0) { };
    Vector_base(size_t n, const Alloc&): _start(0), _end(0), _end_of_storage(0) {
        // 分配
        _start = _data_allocate(n);
        _end = _start;
        _end_of_storage = _start + n;
    }
    ~Vector_base() { _data_deallocate(_start, _end_of_storage - _start); }
protected:
    typedef simple_alloc<T, Alloc> Date_allocator;
    T* _start; // 使用空间头
    T* _end; // 使用空间尾
    T* _end_of_storage; // 可用空间尾

    T* _data_allocate(size_t n) { return Data_allocator::allocate(n);}
    void _data_deallocate(T* p, size_t n) { Data_allocator::deallocate(p, n);}
};

template <class T, class Alloc = alloc>
class Vector: protected Vector_base<T, Alloc> {
private:
    typedef Vector_base<T, Alloc> _Base;
protected:
    using _Base::_start;
    using _Base::_end;
    using _Base::_end_of_storage;
    using _Base::_data_allocate;
    using _Base::_data_deallocate;
public:
    // Member types
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    typedef typename _Base::allocator_type allocator_type;

    /** Member functions **/
    allocator_type get_allocator() const { return _Base::get_allocator(); }
    void insert_aux(iterator pos, const T& val); // 基本工具，插入一个元素
    void insert_aux(iterator pos); // 插入T()
    // construction
    explicit Vector(const allocator_type& _a = get_allocator()) :_Base(_a){ }
    Vector(size_type count, const T& value = T(), const allocator_type& _a = get_allocator()) : _Base(count, _a) { 
        _start = uninitialized_fill_n(__start, n, value);
    }
    explicit Vector(size_type count) : _Base(_n, allocator_type()){ uninitialized_fill(_start, _start + n, T());} 
    // copy cons
    Vector(const Vector<T, Alloc>& other): _Base(other.size(), other.get_allocator()) 
            { _end = uninitialized_copy(other.begin(), other.end(), _start);}
    // range for cons
    // no-template
    Vector(const T* first, const T* last,
         const allocator_type& _a = allocator_type())
    : _Base(last - first, _a) 
    { _end = uninitialized_copy(first, last, _start); }
    // decons 析构各个元素， 空间的释放交给父类完成
    ~Vector() { destroy(_start, _end_of_storage - _start); }

    void deallocate() { _data_deallocate(_start, capacity());}
    Vector<T, Alloc>& operator =(const Vector<T, Alloc>& other); // 需考虑细节
    // assign函数，两个版本 单元素 和 range, range版本要dispatch。
    void assign(size_type n, const T& val) { _fill_assign(n, val); }
    void _fill_assign(size_type __n, const _Tp& __val);

    /** element access **/
    reference operator[](size_type n) { return *(begin() + n); }
    reference front() { return *begin() ;}
    reference back() { return *(end() - 1);}

    /** iterators **/
    iterator begin() { return _start; }
    iterator end() { return _end; }

    /** capacity **/
    size_type size() { return _end - _start;}
    size_type capacity() { return _end_of_storage - _start;}
    bool empty() { return _start == _end; }

    /** modifiers **/
    void push_back( const T& value) {
        if (_end != _end_of_storage) 
            construct(_end++, value); 
        else 
            insert_aux(end(), value);
    }
    void pop_back() {
        --_end;
        destroy(_end);
    }
    // @优化：给出一个移动赋值版的函数，能够大大的节约赋值的时间 solve:丢给客户端去做，？在uninitialed_copy中
    // implecment new 会调用移动语义。　＠＠待测试
    // 清除[first, last)中的元素
    // erase的元素析构交给operator=
    iterator erase(iterator first, iterator last) { 
        iterator i = copy(last, _end, first);
        destroy(i, _end);
        _end -= (last - first);
        return first;
    }
    iterator erase(iterator pos) { // 若pos = end()-1则return end()
        if (pos+1 != _end) 
            copy(pos+1, _end, pos);
        --_end;
        destroy(_end);
        return pos;
    }
    void insert(iterator pos, const T& value)
        { insert_aux(pos, value); }
    void insert(iterator pos, size_type count, const T& value);
    void resize(size_type count, T value = T()) { // 改变可存储元素的个数
         if (count == size()) return ;
         else if(count < size()) 
            erase(_start+count , _end);
         else 
            insert(_end, count-size(), value);
    }
    
};

template<class T, class Alloc>
void Vector<T, Alloc>::insert_aux(iterator pos, const T& val) {
    if (_end != _end_of_storage) {
        construct(_end, *(_end-1));
        ++_end;
        copy_backward(pos, end-2, end-1);
        *pos = val;
    } else { // 空间不够
        const size_type old_sz = size();
        const size_type len = old_sz? 1: 2 * old_sz*2;
        
        iterator new_start = _data_allocate(len);
        iterator new_end = new_start;
        iterator new_end_of_storage = new_start + len;
        try { 
            new_end = uninitialized_copy(_start, pos, new_start);
            construct(pos, val);
            new_end++;
            new_end = uninitialized_copy(pos, _end, new_end); 
        } catch(...) { // 分配失败，还原
            destroy(new_start, new_end);
            _data_deallocate(new_start, len);
            throw // 重抛错误
        }
        // 析构释放原函数
        destroy(_start, _end);
        // _data_deallocate(_start, capacity());
        deallocate();
        // rest
        _start = new_start;
        _end = new_end;
        _end_of_storage = new_end_of_storage;
    }
}

template<class T, class Alloc>
void Vector<T, Alloc>::insert_aux(iterator pos) {
      if (_end != _end_of_storage) {
        construct(_end, *(_end-1));
        ++_end;
        copy_backward(pos, end-2, end-1);
        *pos = T();
    } else { // 空间不够
        const size_type old_sz = size();
        const size_type len = old_sz? 1: 2 * old_sz*2;
        
        iterator new_start = _data_allocate(len);
        iterator new_end = new_start;
        iterator new_end_of_storage = new_start + len;
        try { 
            new_end = uninitialized_copy(_start, pos, new_start);
            construct(pos, val);
            new_end++;
            new_end = uninitialized_copy(pos, _end, new_end); 
        } catch(...) { // 分配失败，还原
            destroy(new_start, new_end);
            _data_deallocate(new_start, len);
            throw // 重抛错误
        }
        // 析构释放原函数
        destroy(_start, _end);
        _data_deallocate(_start, capacity());
        // rest
        _start = new_start;
        _end = new_end;
        _end_of_storage = new_end_of_storage;
    }
}

template <class T, class Alloc>
void Vector<T, Alloc>::insert(iterator pos, size_type count, const T& value) {
    if (count! = 0) { // n不是０
        if (_end_of_storage >= count) {// 备用空间足够
            const size_t pos_to_rear = _end - pos;
            iterator old_end = _end;
            if (pos_to_end > count) {  
                uninitialized_copy(_end - count, _end, _end);
                _end += n;
                copy_backward(pos, old_end-n, old_end);
                fill(pos, pos + n, value);
            }
            else {
                uninitialized_fill_n(_end, count - pos_to_rear, value);
                _end += count - pos_to_rear;
                uninitialized_copy(pos, old_end, _end);
                _end += pos_to_rear;
                fill(pos, old_end, value);
            }
        }
        else { // 备用空间不足
            // 新配置内存
            const size_type old_size = size();
            const size_type len = old_size + (old_size, n);
            iterator new_start = _data_allocate(len);
            iterator new_end = new_start;
            try {
                new_end = uninitialized_copy(_start, pos, new_start);
                new_end = uninitialized_fill_n(new_end, count, value);
                new_end = uninitialized_copy(pos, _end, new_end);
            }
            catch(...) {
                // 有异常回滚
                destroy(new_start, new_end);
                _data_deallocate(new_start, len);
                throw;
            }
            destroy(_start, _end);
            deallocate();
            // 更新标记
            _start = new_start;
            _end = new_end;
            _end_of_storage = new_start + len;
        }
    }
}

#endif