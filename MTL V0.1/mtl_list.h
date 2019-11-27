#ifndef MTL_LIST_H
#define MTL_LIST_H
#include "mtl_iterator.h"
#include "mtl_alloc.h"
#include "mtl_uninitialized.h"
#include "stl_algobase.h"
/** list node设计**/
template <class T>
struct _list_node
{
    _list_node<T>* prev;
    _list_node<T>* next;
    T data;
};
/** List迭代器 **/
// list双向链表采用　bidirectional_iterator_tag
template <class T, class Ref, class Ptr>
struct _list_iterator
{
    typedef _list_iterator<T, T&, T*> iterator;
    typedef _list_iterator<T, Ref, ptr> self;

    typedef bidirect_iterator_tag iterator_category;
    typedef T vaule_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef _list_node<T>* link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    link_type node; // 迭代器内部的节点指针
    // constructor
    _list_iterator() {}
    _list_iterator(link_type x): node(x) {}
    _list_iterator(const iterator& x): node(x.node) {}
    // operator
    bool operator==(const self& x) { return node == x.node; }
    bool operator!=(const self& x) { return node != x.node; }
    reference operator*() { return (*node).data; }
    reference operator->() { return &(operator*()); }
    self& operator++() {
        node = (link_type) ((*node).next);
        return *this;
    }
    self& operator--() {
        node = (link_type) ((*node).prev);
        return *this;
    }
    self operator-(int n) {
        link_type tem = node;
        while(n--)
            tem = tem->prev;
        return tem;
    }
    self operator+(int n) {
        link_type tem = node;
        while(n++)
            tem = tem->next;
        return tem;
    }
};

// 负责List的基本内存处理、构造析构处理、提供类内使用函数
template<class T, class Alloc = alloc>
class List_base {
public:
    typedef Alloc allocator_type;
    typedef _list_node<T>* link_type;
    allocator_type get_allocator() const { return allocator_type(); }
    // constructor
    List_base() { // 空list有一个NULL节点
        link_type node = get_node();
        node->next = node;
        node->prev = node;
    }
    ~list_node() {
        clear();
        put_node(node);
    }
protected:
    simple_alloc<T, Alloc> list_node_allocator;
    typedef _list_node<T> list_node;
    link_type node; // 只需要一个有效节点就可以循环整个环型链表

    link_type get_node() { return list_node_allocator::allocate();}
    void put_node(link_type p){ list_node_allocator::deallocate(p); }
    // 产生节点
    link_type create_node(const T& x) {
        link_type p = get_node();
        construct(&p->data, x);
        return p;
    }
    // 销毁节点
    void destroy_node(link_type p) {
        destroy(&p->data);
        put_node(p);
    }
    void clear(); 
};
// 删除其他节点，只剩一个空节点
template <class T, class Alloc>
void List_base<T, Alloc>::clear() {
    link_type cur = node->next;
    while (cur != node) {
        link_type tem = cur;
        cur = cur->next;
        destroy(&(tem->data));
        put_node(tem);
    }
    node->next = node;
    node->prev = node;
}

template <class T, class Alloc = alloc>
class List: protected List_base<T, Alloc> {
public:
    // typedef 
    typedef List_base<T, Alloc> _Base;
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef _list_node<T> _Node;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef typename _Base::allocator_type allocator_type;
    allocator_type get_allocator() const { return _Base::get_allocator(); }
    typedef _list_iterator<T, T&, T*> iterator;
    typedef _list_iterator<T, const T&, const T*> const_iterator;

protected:
    // 工具函数，连续范围内的节点移到pos节点之前
    void transfer(iterator pos, iterator first, iterator last)
    {
        if (pos == last) return;
        last.node->prev->next = pos.node;
        first.node->prev->next = last.node;
        pos.node->prev->next = first.node;
        link_type tem = pos.node->prev;
        pos.node->prev = last.node->prev;
        last.node->prev = first.node->prev;
        first.node->prev = tem;
    }

public:
    using _Base::node;
    using _Base::get_node;
    using _Base::put_node; 
    /** 构造函数 **/
    List(): _Base() { }
    explicit List(size_type count, const T& value = T());
    // 构造拥有[first,last)内容的list容器，若InputIt为整数类型则调用别的构造函数
    template <class InputIt>
    List(InputIt first, InputIt last);
    List(const List& other ):_Base() {} // 需要insert来做

    /** 元素访问 **/
    reference front() { return (node->next)->data; }
    const_reference front() const { return (node->next)->data; }
    reference back() { return (node->prev)->data; }
    const_reference back() const { return (node->prev)->data; }

    /** 迭代器 **/
    iterator begin() { return (_Node*)node->next; }
    const_iterator begin() const { return (_Node*)node->next; }
    iterator end() { return node; }
    const_iterator end() const { return node; }
    /** 容量 **/
    bool empty() { return node->next == node; }
    size_type size() { return distance(begin(), end()); }
    size_type max_size() const { return size_type(-1); } 
    
    /** 修改器**/
    using _Base::clear;
    // insert都是链表前插
    iterator insert( iterator pos, const T& value) {
        link_type new_node = create_node(value);
        new_node->prev = pos.node->prev;
        (new_node->prev)->next = new_node;
        new_node->next = pos.node;
        pos.node->prev = new_node;
        return new_node;
    }
    void insert(iterator pos, size_type count, const T& value) {
        for (; count > 0; count--)
            insert(pos, value);
    }
    void push_front(const T& x) { insert(begin(), x); }
    void push_back(const T& x) { insert(end(), x); }
    // 移除节点
    iterator erase(iterator pos) {
        pos.node->next->prev = pos.node->prev;
        pos.node->prev->next = pos.node->next;
        link_type next_node = pos.node->next;
        destroy_node(pos.node);
        return next_node;
    }
    void pop_front() { erase(begin()); }
    void pop_back() { erase(--end()); }
    
    /** 操作 **/
    // 移除所有数值为value的节点
    void remove(const T& value) {
        iterator first = begin();
        iterator last = end();
        while (first != last) {
            iterator next = first;
            ++next;
            if (*first == value) erase(first);
            first = next;
        }
    }

    template <class UnaryPredicate>
    remove_if(UnaryPredicate p) {
        iterator first = begin();
        iterator last = end();
        while (first != last) {
            iterator next = first;
            ++next;
            if (p(*first)) erase(first);
            first = next;
        }
    }
    // 数值相同的连续元素只保存一个
    void unique() {
        iterator first = begin();
        iterator last = end();
        if (empty()) return ;
        iterator next = first;
        while (++next != last) {
            if (*first == *next)
                erase(next);
            else 
                first = next;
            next = first;
        }
    }
    void swap(List &other) {
        ::swap(*this, other);
    }

    /** 操作 **/
    // 在C++11版本之后的splice要指明std::move进行splice操作，因为其本身就把other剪切
    // pos 和 other指向同一个list，其行为未定义
    #if __cplusplus > 201103L:
        void splice(iterator pos, List &&other)
    #else 
        void splice(iterator pos, List &other)
    #endif
    {
        if (!other.empty())
            transfer(pos, other.begin(), other.end());
    }
    // 将i所指元素置于pos之前 other和*this可指向同一个
    #if __cplusplus > 201103L:
        void splice(iterator pos, List &&other, iterator i)
    #else
        void splice(iterator pos, List &other, iterator i)
    #endif
    {
        if (pos == i || pos == i+1)
            return;
        transfer(pos, i, i+1); 
    }
    #if __cplusplus > 201103L:
        void splice(iterator pos, List &&other, iterator first, iterator last)
    #else 
        void splice(iterator pos, List &other, iterator first, iterator last)
    #endif
    {
        if (first != last)
            transfer(pos, first, last);
    }
    // merge合并两个已经递增排序的链表
    #if __cplusplus > 201103L:
        void merge(List &&other )
    #else 
        void merge(List &other)
    #endif
    {
        iterator first1 = begin(), last1 = end();
        iterator first2 = other.begin(), last2 = other.end();
        iterator next;
        while (first1 != last1 && first2 != last2) {
            while (first1 != last1 && *first1 < *first2)
                first1++;
            next = first2;
            while (netx != last2 && *first1 > *next)
                next++;
            transfer(first1, first2, next);
            first2 = next;
        }
        if (first2 != last2)
            transfer(last1, first2, last2);
    }
    
    void reverse(); // 内容逆向重置
    void sort(); //STL的sort只接受random_access_iterator
};

template <class T, class Alloc>
void List<T, Alloc>::reverse()
{
    // size()函数比较慢
    if (node->next == node || (node->next)->next == node)
        return;
    iterator first = begin();
    ++first;
    while (first != end()) {
        iterator old = first;
        ++first;
        transfer(begin(), old, first);
    }
}

// 非递归归并排序 时间复杂度O(nlogn) 空间O(n)
// SGI中的算法 很厉害
template <class T, class Alloc>
void List<T, Alloc>::sort()
{
    
}


#endif
    