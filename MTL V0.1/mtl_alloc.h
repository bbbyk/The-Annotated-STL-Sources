/**
 * Author: bbbyk
 * Date: 2019.11.06
 * 
 * 实现第一级内存配置器和第二级内存配置器，并封装为simple_alloc
 * 默认将二级适配器define为alloc供所有容器默认使用
 * **/

#ifndef MTL_ALLOC_H
#define MTL_ALLOC_H

#include <new>
#include <cstdlib>
#include <memory>
#include <cstring>

#define __default_alloc alloc
// 一级内存配置器
class __base_alloc{
private:
    // 内存不足解决情况
    static void *oom_malloc(size_t n);
    static void *oom_realloc(void *p, size_t n);
    // 自定义oom处理函数，主要是去想办法释放内存
    static void (*__oom_handler)();

public:
    static void *allocate(size_t n) {
        void *result = malloc(n); 
        if (result == 0) {
            result = oom_malloc(n);
        }
        return result;
    }
    static void *deallocate(void *p, size_t n) {
        free(p); // 直接使用free
    }
    static void *realocate(void *p, size_t old_sz, size_t new_sz) {
        void *result = realloc(p, new_sz);
        if (result == 0) result = oom_realloc(p, new_sz);
        return result;
    }
    // set 自己的 handler
    static void (*set_oom_handler(void (*f)()))() {
        void (*old)() = __oom_handler;
        __oom_handler = f;
        return old;
    }
};
// static初始化
void (*__base_alloc::__oom_handler)() = 0;
// 不断尝试配置、调用
void *__base_alloc::oom_malloc(size_t n) {
    void *result = 0;
    void (*my_handler)();
    while(1) {
        my_handler = __oom_handler;
        if (my_handler == 0) throw std::bad_alloc();
        __oom_handler();
        result = malloc(n);
        if(result) return result; // 成功则返回
    }
}
void *__base_alloc::oom_realloc(void *p, size_t n) {
        void *result = 0;
    void (*my_handler)();
    while(1) {
        my_handler = __oom_handler;
        if (my_handler == 0) throw std::bad_alloc();
        __oom_handler();
        result = realloc(p, n);
        if (result) return result;
    }
}

/**
 * 二级配置器
 * **/
const int __ALIGN = 8; // 上调边界
const int __MAX_BYTES = 128; // 小型区块的内存上限
const int __NFREELISTS = __ALIGN / __ALIGN; // freelists个数

class __default_alloc{
private:
    union obj // freelists节点构造
    {
        union obj* free_list_link; // 未使用区块 
        char client_data[1]; // 客户端使用的区块 1 字节
    };
    // 字节上调
    static size_t ROUND_UP(size_t n) {
        return (n + __ALIGN - 1) & ~(__ALIGN - 1);
    }
    // 计算使用哪个区块
    static size_t FREELSITS_INDEX(size_t n) {
        return (n + __ALIGN - 1) / (__ALIGN - 1);
    }
    static obj * volatile free_list[__NFREELISTS]; // 16个节点

    static void *refill(size_t n); // 返回一个大小为n的区块
    static char *chunk_alloc(size_t size, int &nobjs); // 配置n个size大小区块，若不够则可能降低nobjs
    
    static char *start_free; // 内存池起始
    static char *end_free; // 内存池结束
    static size_t heap_size;

public:
    static void *allocate(size_t n);
    static void *reallocate(void *p, size_t old_n, size_t new_n);
    static void deallocate(void *p, size_t n);
};

void *__default_alloc::allocate(size_t n) {
    obj* volatile* my_free_list;
    obj* result;
    // 大于128则调用一级适配器
    if (n > __MAX_BYTES) {
        return __base_alloc::allocate(n);
    }
    // 找到对应的list
    my_free_list = free_list + FREELSITS_INDEX(n);
    result = *my_free_list;
    if (result == NULL) { // 没找到可用的freelist 准备重新填充
        return refill(ROUND_UP(n));
    }
    *my_free_list = result->free_list_link;
    return result; 
}

void __default_alloc::deallocate(void *p, size_t n) {
    obj* volatile* my_free_list;
    obj *ptr = (obj*) p;

    if (n > __MAX_BYTES) {
        __base_alloc::deallocate(ptr, n);
    }
    // 将释放区块重新插入到指定链表中
    my_free_list = free_list + FREELSITS_INDEX(n);
    obj* tem = (*my_free_list)->free_list_link;
    ptr->free_list_link = *my_free_list;
    *my_free_list = ptr;
}

void *__default_alloc::reallocate(void *p, size_t old_size, size_t new_size) {
    obj* volatile* my_free_list;
    void *result;
    if ( (old_size > __MAX_BYTES) && (new_size > __MAX_BYTES))
        __base_alloc::realocate(p, old_size, new_size);
    if (old_size == new_size) return p;

    result = allocate(new_size);
    // new_size可能 < old_size 部分拷贝
    memcpy(result, p, old_size < new_size? old_size: new_size);
    deallocate(p, old_size);
}

// 内存池空间不足,refill一下给对应位置缺省分配新的20个区块
// 可能因为内存池不足的原因 返回少于20

void *__default_alloc::refill(size_t n) {
    obj* volatile* my_free_list;
    obj* result;
    obj* current_obj, *next_obj;
    int nobjs = 20;
    // 内存池分配函数
    char *chunk = chunk_alloc(n, nobjs); // &传入
    if (1 == nobjs) return chunk;
    my_free_list = free_list + FREELSITS_INDEX(n);
    // 在chunk空间内建立free list
    result = (obj*) chunk; // 第一块返回给客户端
    // free list指向新配置的空间
    *my_free_list = next_obj = (obj*) (chunk + n);
    // 将free list各节点连起来
    for (int i = 1; ; i++) {
        current_obj = next_obj;
        next_obj = (obj*)((char*)chunk + n);
        if (nobjs == i+1) { // 最后一个区块指向NULL
            current_obj->free_list_link = NULL;
            break;
        }
        else 
            current_obj->free_list_link = next_obj;
        return result;
    }
}

char *__default_alloc::chunk_alloc(size_t size, int &nobjs) {
    char *result;
    size_t total_size = size * nobjs;
    size_t left_bytes = end_free - start_free;
    if (left_bytes >= total_size) { // 满足总量
        result = start_free;
        start_free += total_size;
        return result;
    }
    else if(left_bytes >= size) { // 不满足总量，但是有1个以上的size
        nobjs = left_bytes / size;
        result = start_free;
        start_free += size * nobjs;
        return result;
    }
    else { // 一个都不行
        size_t get_bytes = 2 * total_size + ROUND_UP(heap_size >> 4);
        //将剩余空间先配置
        if (left_bytes > 0) {
            obj * volatile* lists = free_list + FREELSITS_INDEX(left_bytes);
            obj* current = (obj*)start_free;
            current->free_list_link = *lists;
            *lists = current;
        }
        // 配置heap用来补充内存池
        start_free = (char*)malloc(get_bytes);
        if (start_free == 0) {// 空间不足，分配失败
            // 从更大的区块中拆分,
            obj* volatile* my_list;
            for (int i = size; i < __MAX_BYTES; i+=__ALIGN) {
                my_list = free_list +  FREELSITS_INDEX(i);
                obj *p = *my_list;
                if (*my_list)  {
                    // 释放区块到内存池中
                    *my_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = (char*)p + i;
                    // 递归重新配置
                    return chunk_alloc(size, nobjs); // 自动完成残余区域编入free_lsits
                }                
            }
            // 以上方法不管用，无路可走
            // end_free = NULL;
            start_free = (char*)__base_alloc::allocate(get_bytes); // 指望__base_alloc的异常处理handler
        }
        // 靠malloc成功
        heap_size += get_bytes;
        end_free = start_free + get_bytes;
        return chunk_alloc(size, nobjs); // 修正成员
    }
}

// 封装配置器
template<class T, class Alloc>
class simple_alloc {
public:
    static T *allocate(size_t n)
            { return n==0? NULL: (T*)Alloc::allocate(sizeof(T) * n); }
    static T *allocate()
            { return (T*)Alloca::allocate(sizeof(T)); }
    static void deallocate(T *p, size_t n)
            { if (n!=0)  Alloc::deallocate(p, sizeof(T)*n)); }
    static void deallocate(T *p)
            { Alloc::deallocate(p, sizeof(T)); }
    static T *reallocate(T *p, size_t old_n, size_t new_n)
            { return new_n == 0? NULL: Alloc::reallocate(p, old_n, new_n); }
};

#endif

