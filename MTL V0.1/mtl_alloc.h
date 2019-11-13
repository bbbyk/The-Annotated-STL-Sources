/**
 * 二级配置器
 * **/
#include<cstddef>
const int __ALIGN = 8; // 上调边界
const int __MAX_BYTES = 128; // 小型区块的内存上限
const int __NFREELISTS = __ALIGN / __ALIGN; // freelists个数



// 二级适配器
class __default_alloc_template {
private:
    // 
    union obj // freelists节点构造
    {
        union obj* free_list_link; // 未使用区块 
        char client_data[1]; // 客户端使用的区块
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
    static char *heap_size;

public:
    static void *allocate(size_t n);
    static void *reallocate(void *p, size_t old_n, size_t new_n);
    static void deallocate(void *p, size_t n);
};

void *__default_alloc_template::allocate(size_t n) {
    obj* volatile* my_free_list;
    obj* result;
    // 大于128则调用一级适配器
    if (n > __MAX_BYTES) {
        return malloc_alloc::allocate(n);
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

void __default_alloc_template::deallocate(void *p, size_t n) {
    obj* volatile* my_free_list;
    obj *ptr = (obj*) p;

    if (n > 128) {
        malloc_alloc::deallocate(ptr, n);
    }
    // 将释放区块重新插入到指定链表中
    my_free_list = free_list + FREELSITS_INDEX(n);
    obj* tem = (*my_free_list)->free_list_link;
    ptr->free_list_link = *my_free_list;
    *my_free_list = ptr;
}

// 内存池空间不足,refill一下给对应位置缺省分配新的20个区块
// 可能因为内存池不足的原因 返回少于20

void *__default_alloc_template::refill(size_t n) {
    obj* volatile* my_free_list;
    obj* result;
    obj* current_obj, *next_obj;
    int nobjs = 20;

    char *chunk = chunk_alloc(n, nobjs); // &传入
    if (1 == nobjs) return chunk;
    my_free_list = free_list + FREELSITS_INDEX(n);
    // 在chunk空间内建立free list
    result = (obj*) chunk; // 第一块返回给客户端
    // 指导free list指向新配置的空间
    *my_free_list = next_obj = (obj*) (chunk + n);
    // 将free list各节点连起来
    for (int i = 1; ;i++) {
        if (nobjs == i + 1)
    }
}

