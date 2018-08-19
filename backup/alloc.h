#ifndef ALLOC_H
#define ALLOC_H
#include "macros.h"
#include "utils.h"
#include <stddef.h>
#include <malloc.h>

USING_NAMESPACE_FASTSTL_BEGIN
using std::size_t;
//############################################
// 第一级内存分配器的实现，使用malloc进行内存管理
//############################################


/**
 * @brief The __malloc_alloc class
 * 使用malloc来实现的第一级内存分配器
 */
template<int>
class __malloc_alloc{
private:
    // malloc分配器oom处理器函数类型声明
    using _handler_func=void(*)();
    // 处理内存不足的情况
    static void* oom_alloc(size_t );
    static void* oom_realloc(void*,size_t);
    // 默认的oom处理器，应该由用户自己实现并赋值给这个函数指针，默认为0，不实现
    static _handler_func __malloc_alloc_oom_handler;
public:
    static void*allocate(size_t n){
        //使用malloc分配内存
        void* ret = malloc(n);
        // 如果无法满足内存分配
        if(0 == ret) ret = oom_alloc(n);
        return ret;
    }

    static void deallocate(void*p,size_t){
        // 直接使用free释放内存
        free(p);
    }
    static void reallocate(void*p,size_t old_n,size_t new_n){
        void *ret = realloc(p,new_n);
        if(0==ret)oom_realloc(p,new_n);
        return ret;
    }

    // 设置自定义的malloc oom处理器
    _handler_func set_malloc_handler(_handler_func new_handler){
        auto old_handler = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = new_handler;
        return old_handler;
    }

};

template<int inst>
typename __malloc_alloc<inst>::_handler_func __malloc_alloc<inst>::__malloc_alloc_oom_handler=0;

template<int inst>
void* __malloc_alloc<inst>::oom_alloc(size_t n){
    void* ret;
    _handler_func my_oom_handler;
    // 不断地尝试使用oom handler配置内存
    for(;;){
        my_oom_handler = __malloc_alloc_oom_handler;
        // 如果用户没有定义oom处理器，那么out of memory错误
        if(0 == my_oom_handler){__THROW_BAD_ALLOC;}
        // 尝试释放内存或者获得新的内存
        (*my_oom_handler)();
        ret = malloc(n);
        // 可能还会不足，那么继续循环处理,否则返回
        if(0 != ret)return ret;

    }
}
template<int inst>
void* __malloc_alloc<inst>::oom_realloc(void*p,size_t n){
    void * ret;
    for(;;){
        auto my_oom_handler = __malloc_alloc_oom_handler;
        if(0==my_oom_handler){__THROW_BAD_ALLOC;}
        ret = realloc(p,n);
        if(ret)return ret;
    }
}
// 定义一个全局的malloc alloc类实例
using malloc_alloc = __malloc_alloc<0>;


//####################################################################
// 第二级内存分配器的实现，维护一个内存池，并且调用第一级内存分配器实现必要内存申请
// *******************************************************************
// 二级内存分配器在申请大于128字节的情况下直接调用第一级内存分配器，否则按照申请字节
// 大小检索相应的内存块链表进行分配
// *******************************************************************
// 二级内存分配器将会维护一个自由链表freelist数组，默认将会维护16个自由链表，每一个
// 自由链表分别维护各自的内存区块大小的链表，分别是8,16，。。。，128；也就是链表数组中的
// 第一个维护一个8字节为单位的内存区块构成的链表，而后面依次是16,32.。。个字节的内存区块
// 构成的链表。
// *******************************************************************
// 内存的申请将会使用malloc从堆中进行申请，当malloc无法完成时会使用第一级的内存管理器
// 进行尝试申请，也就是使用第一级的oom申请机制
//####################################################################



template<int inst>
class __default_alloc{
    enum {
        ALIGN=8,//自由内存链表调整字节数边界
        MAX_BYTES=128,// 总共维护16个自由链表，每一个自由链表维护的内存块大小依次为8,16,...,128
        NUM_FREELISTS=MAX_BYTES/ALIGN // 总的自由链表数
    };
private:
    /**
     * @brief 调整待申请的字节数以ALIGN为对齐边界，这里也就是调整为8的倍数
     * @param bytes 待调整的字节数
     * @return 调整之后的上对齐字节数
     */
    static size_t ALIGN_UP(size_t bytes){
        return ((bytes+ALIGN-1)&~(ALIGN-1));
    }
    /**
     * @brief 自由链表的节点定义，使用union进行定义，这样的话可以节省内存，也就是节点本身可以存储
     * 下一个自由节点的地址也可以存储用户的数据（以字节为单位）
     */
    union free_node{
        union free_node * next_free_node;
        char data[1];
    };

private:
    // 16个自由链表数组
    static free_node* volatile freelists[NUM_FREELISTS];
    // 根据待申请的字节数返回对应的自由链表数组中的index
    static size_t get_freelist_index(size_t bytes){
        return (bytes+ALIGN-1)/ALIGN-1;
    }
    /**
     * @brief refill 自由链表没有对应可分配内存块，则申请更多的与n上对齐大小的内存块，比如
     * n为13，则申请20个大小为16字节的内存块，返回其中一个，剩下的19个纳入对应的自由列表后
     * @param blockSize 待申请的字节数
     * @return
     */
    static void* refill(size_t blockSize);
    /**
     * @brief chunk_alloc 分配numBlocks个blockSize的内存区块，当然，如果内存不足，那么可能申请
     * 到低于numBlocks个的内存块
     * @param blockSize
     * @param numBlocks
     * @return
     */
    static char* chunk_alloc(size_t blockSize,int&numBlocks);

    // 可分配内存池的参数
    static char* start_free;
    static char* end_free;
    static size_t heap_size;
public:
    static void* allocate(size_t n);
    static void  deallocate(void*p,size_t n);
    static void* reallocate(void*p,size_t oldSize,size_t newSize);
};
// 静态成员初始化
template<int inst>
char* __default_alloc<inst>::start_free = 0;
template<int inst>
char* __default_alloc<inst>::end_free = 0;
template<int inst>
size_t __default_alloc<inst>::heap_size = 0;

template<int inst>
typename __default_alloc<inst>::free_node* volatile
__default_alloc<inst>::freelists[NUM_FREELISTS]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

template<int inst>
void* __default_alloc<inst>::allocate(size_t n){
    // 超过二级配置器分配字节最大值，则直接使用一级分配器
    if(n>(size_t)MAX_BYTES){
        return malloc_alloc::allocate(n);
    }
    // 否则的话，使用二级分配器
    // 首先获取目标自由链表的地址
    free_node* volatile *p_freelist = freelists+get_freelist_index(n);
    // 获取找到的自由链表
    auto ret = *p_freelist;
    if(0==ret){
        // 如果当前的链表没有可用的内存块，那么分配填充之,注意分配之前先进行上对齐
        void* r = refill(ALIGN_UP(n));
        return r;
    }

    // 如果当前的链表还有可分配的内存块，则进行分配，并调整可用的自由链表指向
    *p_freelist = ret->next_free_node;
    return ret;
}

template<int inst>
void  __default_alloc<inst>::deallocate(void * p,size_t n){
    if(n>MAX_BYTES){
        malloc_alloc::deallocate(p,n);
    }

    // 先找到它属于哪一个链表，然后使用链表进行回收
    free_node* volatile* p_freelist = freelists+get_freelist_index(n);

    auto freenode_p = (free_node*)p;
    freenode_p->next_free_node = *p_freelist;
    *p_freelist = freenode_p;
}

template<int inst>
void * __default_alloc<inst>::refill(size_t blockSize){
    int numBlocks = 20;
    // 分配numBlocks个blockSize大小的内存区块
    char* chunk = chunk_alloc(blockSize,numBlocks);
    // 如果只申请到了一块内存则直接返回，否则将多余的加入到自由链表
    if(1 == numBlocks)return chunk;

    // 找到待扩展的链表位置
    free_node* volatile* p_freelist = freelists+get_freelist_index(blockSize);
    // 准备好返回内存块
    auto ret = (free_node*)chunk;
    //准备好扩展内存
    auto nextBlock = (free_node*)(chunk+blockSize);
    // *****************************************************
    // 注意，因为只有当当前区块大小的自由链表为空时才会进行refill，因此，这里在进行扩展的时候
    // 应该直接给链表赋值（当然也可以再判断一下链表是否为空，不为空则可以将新的内存块拼接到后面
    // 否则直接赋值）
    *p_freelist = nextBlock;

    for(int i=1;;++i){
        auto currBlock = nextBlock;
        nextBlock = (free_node*)(nextBlock+blockSize);
        if(i == numBlocks-1){
            currBlock->next_free_node = 0;
            break;
        }
        else{
            currBlock->next_free_node = nextBlock;
        }
    }


    return ret;
}

template<int inst>
char * __default_alloc<inst>::chunk_alloc(size_t blockSize,int& numBlocks){
    // 首先查看还剩下多少未分配内存
    size_t bytes_left = end_free-start_free;
    // 然后看看我们这次需要申请多少字节
    size_t bytes_needed = blockSize*numBlocks;
    //如果剩下的内存够了，那么就可以直接从剩下的内存池中进行分配
    if(bytes_left>=bytes_needed){
        auto ret = start_free;
        start_free += bytes_needed;
        return ret;
    }//否则的话看看是不是足够分配一个或者一个以上的内存区块大小
    else if(bytes_left>=blockSize){
        // 更正可以分配的内存区块数量
        numBlocks = bytes_left/blockSize;
        auto ret = start_free;
        start_free += numBlocks*blockSize;
        return ret;
    }//否则，也就意味着内存池中一块内存都不够了，那么从heap中申请
    else{
        // 在从堆中申请之前，我们需要将内存池中剩下的内存分配到合适大小的区块链表中
        // ************************
        // 注意，这里有一个细节问题，也就是剩下的内存字节大小一定是ALIGN也就是8的倍数，因为我们在申请
        // 内存的时候就是按照这个倍数去申请的，因此，剩下的内存绝对可以分配到对应的内存区块链表中，不会出现
        // 不足之说，也就是不会出现6个字节这样，至少是8个字节，或者13个字节，那么则至少16个字节....
        // 并且很重要的一点，也就是会刚好满足一个小于当前要申请的区块大小的区块大小：比如当前要申请96个字节
        // 那么剩下不满足96的字节数，应该是88,80,72，...,16,8中的一个，因此找到对应的内存区块链表，加一个
        // 进去就可以了。
        // ************************
        if(bytes_left>0){
            // 先找到合适的内存区块链表位置
            free_node* volatile* p_freelist = freelists+get_freelist_index(bytes_left);
            // 加一个进去
            free_node* freenode_start = (free_node*)start_free;
            freenode_start->next_free_node = *p_freelist;
            *p_freelist = freenode_start;
            start_free = end_free = 0;
        }

        // 这次我们将申请需要申请的内存字节数的两倍，并且加上一个附加项，这个附加项将根据从堆中的申请总数的
        // 的增加而增加
        size_t bytes_to_alloc = 2*bytes_needed+ALIGN_UP(heap_size>>4);
        // 尝试申请
        start_free = (char*)malloc(bytes_to_alloc);
        end_free = 0;
        if(0 == start_free){//如果malloc申请失败，那么得继续想办法
            // malloc申请失败了，说明堆中的资源耗尽了，因此，我们将尝试从内存区块链表中寻找一个比当前
            // 待申请的区块大小更大的区块，找到的话则进行将其分配到当前大小的区块链表上；
            // 注意：这里不考虑使用更小的内存区块大小来进行拼接，否则内存中可能会出现很多的内存碎片


            for(size_t i = blockSize;i<=MAX_BYTES;i+=ALIGN){
                free_node* volatile*p_freelist = freelists+get_freelist_index(i);
                auto freep = *p_freelist;
                if(0 !=  freep){//如果找到一个，那么则将其释放给内存池回收，然后递归调用自己进行分配
                    *p_freelist = freep->next_free_node;
                    start_free = (char*)freep;
                    end_free = start_free+i;
                    return chunk_alloc(blockSize,numBlocks);
                }
            }

            // 如果到这里则表示没有可用的内存区块，那么只能调用一级内存分配器进行内存分配
            // 必要时会触发那里的oom机制，如果oom也没办法处理则会抛出异常

            start_free =(char*) malloc_alloc::allocate(bytes_to_alloc);
        }

        // 到这里，说明申请成功了，那么调整heap_size，end_free并且递归调用自己来重新调整numBlock
        // 并返回相应的区块指针
        heap_size += bytes_to_alloc;
        end_free = start_free + bytes_to_alloc;
        return chunk_alloc(blockSize,numBlocks);
    }
}

using alloc = __default_alloc<0>;

template <typename Tp,typename Alloc=alloc>
class simple_alloc{
public:
    static Tp* allocate(size_t n){
        return n==0?0:(Tp*)Alloc::allocate(sizeof(Tp)*n);
    }
    static void deallocate(Tp* p,size_t n){
        Alloc::deallocate(p,sizeof(Tp)*n);
    }

    static Tp* allocate(){
        return (Tp*)Alloc::allocate(sizeof(Tp));
    }

    static void deallocate(Tp*p){
        Alloc::deallocate(p,sizeof(Tp));
    }
};

USING_NAMESPACE_FASTSTL_END

#endif // ALLOC_H
