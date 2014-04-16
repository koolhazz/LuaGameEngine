#ifndef PLEX_MEMORY_H
#define PLEX_MEMORY_H

/**
 * @file plex.h
 * @brief 预分配内存管理
 */

/*
#ifdef __KERNEL__
    #include <linux/types.h>
    #include <linux/stddef.h>
    #include <linux/kernel.h>
    #ifndef assert
        #define assert(s) BUG_ON(!(s))
    #endif
#else*/
    #include <stddef.h>
    #include <stdlib.h>
    #include <assert.h>
	#include <string.h>
//#endif

#ifdef __GNUC__
    #define PLEX_INLINE inline
    #define PLEX_STATIC static
#elif _MSC_VER
    #define PLEX_INLINE __inline
    #define PLEX_STATIC
#else
    #define PLEX_INLINE
    #define PLEX_STATIC static
#endif

#ifdef __cplusplus
extern "C" {
#endif

///定义大内存块 (里面分小内存片)
struct mem_block {
    struct mem_block* next;         //下一块的指针
#ifdef PLEX_ENABLE_RESET
    unsigned int      nmemb;        //预分配成员的数量，记录每一块内存切片的数量，用于重置(plex_reset)
#endif
    char              data[0];      //数据部分，即众多小内存片
};

///定义小内存片
union mem_item {
    union mem_item*   next;               //下一个内存片的指针
    char              data[sizeof(char*)];//内存片的数据部分
};

///定义内存plex的结构
struct plex {
    struct mem_block* head;         //记录从系统申请的内存块
    union mem_item*   free;         //记录可分配的内存块
    size_t            item_size;    //预分配成员的大小
#ifdef PLEX_DEBUG
    size_t            count_alloc;  //记录分配的次数
    size_t            count_free;   //记录释放的次数
#endif
};

typedef struct plex plex_t;

/**
 * plex初始化 （还没有从系统申请内存）
 * @param me plex对象指针，不能为空
 * @param item_size 预分配成员的大小
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_init(struct plex* me, size_t item_size) 
{
    assert(me && item_size);
    me->head = 0;
    me->free = 0;
    me->item_size = item_size<sizeof(void*) ? sizeof(void*) : item_size;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif
}

/**
 * 扩充plex，从系统申请内存
 *  申请的大小为sizeof(struct mem_block)+nmemb*size
 * @param me plex对象指针，不能为空
 * @param nmemb 预分配成员的数量
 * @param malloc_f 申请系统内存的函数指针，不能为空
 * @param ctx 申请系统内存的上下文
 * @return 0:ok; -1:error,see errno
 */
PLEX_STATIC PLEX_INLINE int plex_expand(struct plex* me, unsigned int nmemb, void*(*malloc_f)(void*, size_t), void* ctx) 
{
    struct mem_block* block = 0;
    union mem_item* item = 0;
    unsigned int i = 0;

    assert(me && malloc_f);

    block = (struct mem_block*)malloc_f(ctx, sizeof(struct mem_block) + nmemb * me->item_size);
    if (!block)
        return -1;
#ifdef PLEX_ENABLE_RESET
    //remember nmemb
    block->nmemb = nmemb; 
#endif

    //join to head
    block->next = me->head;
    me->head = block;

    //join to free_list
    item = (union mem_item*)block->data;
    for (; i<nmemb; i++) {
        item->next = me->free;
        me->free = item; 
        item = (union mem_item*)(item->data + me->item_size);
    }
    return 0;
}

/**
 * 清空plex，归还内存给系统
 * @param me plex对象指针，不能为空
 * @param free_f 释放系统内存的函数指针，不能为空
 * @param ctx 申请系统内存的上下文
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_clear(struct plex* me, void(*free_f)(void*ctx, void*ptr), void* ctx) 
{
    assert(me && free_f);
    while (me->head) {
        struct mem_block* bak = me->head;
        me->head = bak->next;
        free_f(ctx, bak);
    }
    me->head = 0;
    me->free = 0;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif
}

/**
 * 申请一个成员的内存
 * @param me plex对象指针，不能为空
 * @return 内存指针, 0为失败
 */
PLEX_STATIC PLEX_INLINE void* plex_alloc(struct plex* me) 
{
    void* ret = 0;

    assert(me);
    if (!me->free)
        return 0;

    ret = me->free;
    me->free = me->free->next;
#ifdef PLEX_DEBUG
    me->count_alloc++;
#endif
    return ret;
}

/**
 * 归还一个成员的内存
 * @param me plex对象指针，不能为空
 * @param ptr 成员的指针
 * @return 内存指针, 0为失败
 */
PLEX_STATIC PLEX_INLINE void plex_free(struct plex* me, void* ptr) 
{
    assert(me && ptr);
    ((union mem_item*)ptr)->next = me->free;
    me->free = (union mem_item*)ptr;
#ifdef PLEX_DEBUG
    me->count_free++;
#endif
}

#ifdef PLEX_ENABLE_RESET

/**
 * 重置plex。所有的内存可重新分配
 * @param me plex对象指针，不能为空
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_reset(struct plex* me)
{
    unsigned int i = 0;
    struct mem_block* block = me->head;

    me->head = 0;
    me->free = 0;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif

    //join to free_list
    while (block) {
        union mem_item* item = (union mem_item*)block->data;
        for (i=0; i<block->nmemb; i++) {
            item->next = me->free; 
            me->free = item; 
            item = (union mem_item*)(item->data + me->item_size);
        }
        block = block->next;
    }
}

/**
 * 获取plex内存池申请的所有内存大小
 * @param me plex对象指针，不能为空
 * @return plex内存池申请的所有内存大小
 */
PLEX_STATIC PLEX_INLINE  size_t plex_size(struct plex* me)
{
    size_t size = 0;
    struct mem_block* block = me->head;
    while (block) {
        size += (sizeof(struct mem_block) + block->nmemb * me->item_size);
        block = block->next;
    }
    return size;
}

/**
 * 所有内存片的数量
 * @param me plex对象指针，不能为空
 * @return 所有内存片的数量
 */
PLEX_STATIC PLEX_INLINE size_t plex_item_count(struct plex* me)
{
    size_t count = 0;
    struct mem_block* block = me->head;
    while (block) {
        count += block->nmemb;
        block = block->next;
    }
    return count;
}
#endif

///默认的系统内存申请函数
PLEX_STATIC PLEX_INLINE void* sys_malloc(void* ctx, size_t size)
{
    void* buf = malloc(size);
    memset(buf, 0, size); //防止系统“假”分配——用到时才真正分配
    return buf;
}

///默认的系统内存释放函数
PLEX_STATIC PLEX_INLINE void sys_free(void* ctx, void* ptr)
{
    free(ptr);
}

#define ITEM_MALLOC(plex, size) \
(plex)? plex_alloc(plex) : malloc(size); 

#define ITEM_FREE(plex, ptr) \
(plex)? plex_free(plex, ptr) : free(ptr); 



#ifdef __cplusplus
}
#endif

#endif /* PLEX_MEMORY_H */

