#ifndef ALLOC_H
#define ALLOC_H
#include "macros.h"
#include "utils.h"
#include <stddef.h>
#include <malloc.h>

USING_NAMESPACE_FASTSTL_BEGIN
using std::size_t;
//############################################
// ��һ���ڴ��������ʵ�֣�ʹ��malloc�����ڴ����
//############################################


/**
 * @brief The __malloc_alloc class
 * ʹ��malloc��ʵ�ֵĵ�һ���ڴ������
 */
template<int>
class __malloc_alloc{
private:
    // malloc������oom������������������
    using _handler_func=void(*)();
    // �����ڴ治������
    static void* oom_alloc(size_t );
    static void* oom_realloc(void*,size_t);
    // Ĭ�ϵ�oom��������Ӧ�����û��Լ�ʵ�ֲ���ֵ���������ָ�룬Ĭ��Ϊ0����ʵ��
    static _handler_func __malloc_alloc_oom_handler;
public:
    static void*allocate(size_t n){
        //ʹ��malloc�����ڴ�
        void* ret = malloc(n);
        // ����޷������ڴ����
        if(0 == ret) ret = oom_alloc(n);
        return ret;
    }

    static void deallocate(void*p,size_t){
        // ֱ��ʹ��free�ͷ��ڴ�
        free(p);
    }
    static void reallocate(void*p,size_t old_n,size_t new_n){
        void *ret = realloc(p,new_n);
        if(0==ret)oom_realloc(p,new_n);
        return ret;
    }

    // �����Զ����malloc oom������
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
    // ���ϵس���ʹ��oom handler�����ڴ�
    for(;;){
        my_oom_handler = __malloc_alloc_oom_handler;
        // ����û�û�ж���oom����������ôout of memory����
        if(0 == my_oom_handler){__THROW_BAD_ALLOC;}
        // �����ͷ��ڴ���߻���µ��ڴ�
        (*my_oom_handler)();
        ret = malloc(n);
        // ���ܻ��᲻�㣬��ô����ѭ������,���򷵻�
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
// ����һ��ȫ�ֵ�malloc alloc��ʵ��
using malloc_alloc = __malloc_alloc<0>;


//####################################################################
// �ڶ����ڴ��������ʵ�֣�ά��һ���ڴ�أ����ҵ��õ�һ���ڴ������ʵ�ֱ�Ҫ�ڴ�����
// *******************************************************************
// �����ڴ���������������128�ֽڵ������ֱ�ӵ��õ�һ���ڴ�������������������ֽ�
// ��С������Ӧ���ڴ��������з���
// *******************************************************************
// �����ڴ����������ά��һ����������freelist���飬Ĭ�Ͻ���ά��16����������ÿһ��
// ��������ֱ�ά�����Ե��ڴ������С�������ֱ���8,16����������128��Ҳ�������������е�
// ��һ��ά��һ��8�ֽ�Ϊ��λ���ڴ����鹹�ɵ�����������������16,32.�������ֽڵ��ڴ�����
// ���ɵ�����
// *******************************************************************
// �ڴ�����뽫��ʹ��malloc�Ӷ��н������룬��malloc�޷����ʱ��ʹ�õ�һ�����ڴ������
// ���г������룬Ҳ����ʹ�õ�һ����oom�������
//####################################################################



template<int inst>
class __default_alloc{
    enum {
        ALIGN=8,//�����ڴ���������ֽ����߽�
        MAX_BYTES=128,// �ܹ�ά��16����������ÿһ����������ά�����ڴ���С����Ϊ8,16,...,128
        NUM_FREELISTS=MAX_BYTES/ALIGN // �ܵ�����������
    };
private:
    /**
     * @brief ������������ֽ�����ALIGNΪ����߽磬����Ҳ���ǵ���Ϊ8�ı���
     * @param bytes ���������ֽ���
     * @return ����֮����϶����ֽ���
     */
    static size_t ALIGN_UP(size_t bytes){
        return ((bytes+ALIGN-1)&~(ALIGN-1));
    }
    /**
     * @brief ��������Ľڵ㶨�壬ʹ��union���ж��壬�����Ļ����Խ�ʡ�ڴ棬Ҳ���ǽڵ㱾����Դ洢
     * ��һ�����ɽڵ�ĵ�ַҲ���Դ洢�û������ݣ����ֽ�Ϊ��λ��
     */
    union free_node{
        union free_node * next_free_node;
        char data[1];
    };

private:
    // 16��������������
    static free_node* volatile freelists[NUM_FREELISTS];
    // ���ݴ�������ֽ������ض�Ӧ���������������е�index
    static size_t get_freelist_index(size_t bytes){
        return (bytes+ALIGN-1)/ALIGN-1;
    }
    /**
     * @brief refill ��������û�ж�Ӧ�ɷ����ڴ�飬������������n�϶����С���ڴ�飬����
     * nΪ13��������20����СΪ16�ֽڵ��ڴ�飬��������һ����ʣ�µ�19�������Ӧ�������б��
     * @param blockSize ��������ֽ���
     * @return
     */
    static void* refill(size_t blockSize);
    /**
     * @brief chunk_alloc ����numBlocks��blockSize���ڴ����飬��Ȼ������ڴ治�㣬��ô��������
     * ������numBlocks�����ڴ��
     * @param blockSize
     * @param numBlocks
     * @return
     */
    static char* chunk_alloc(size_t blockSize,int&numBlocks);

    // �ɷ����ڴ�صĲ���
    static char* start_free;
    static char* end_free;
    static size_t heap_size;
public:
    static void* allocate(size_t n);
    static void  deallocate(void*p,size_t n);
    static void* reallocate(void*p,size_t oldSize,size_t newSize);
};
// ��̬��Ա��ʼ��
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
    // �������������������ֽ����ֵ����ֱ��ʹ��һ��������
    if(n>(size_t)MAX_BYTES){
        return malloc_alloc::allocate(n);
    }
    // ����Ļ���ʹ�ö���������
    // ���Ȼ�ȡĿ����������ĵ�ַ
    free_node* volatile *p_freelist = freelists+get_freelist_index(n);
    // ��ȡ�ҵ�����������
    auto ret = *p_freelist;
    if(0==ret){
        // �����ǰ������û�п��õ��ڴ�飬��ô�������֮,ע�����֮ǰ�Ƚ����϶���
        void* r = refill(ALIGN_UP(n));
        return r;
    }

    // �����ǰ�������пɷ�����ڴ�飬����з��䣬���������õ���������ָ��
    *p_freelist = ret->next_free_node;
    return ret;
}

template<int inst>
void  __default_alloc<inst>::deallocate(void * p,size_t n){
    if(n>MAX_BYTES){
        malloc_alloc::deallocate(p,n);
    }

    // ���ҵ���������һ������Ȼ��ʹ��������л���
    free_node* volatile* p_freelist = freelists+get_freelist_index(n);

    auto freenode_p = (free_node*)p;
    freenode_p->next_free_node = *p_freelist;
    *p_freelist = freenode_p;
}

template<int inst>
void * __default_alloc<inst>::refill(size_t blockSize){
    int numBlocks = 20;
    // ����numBlocks��blockSize��С���ڴ�����
    char* chunk = chunk_alloc(blockSize,numBlocks);
    // ���ֻ���뵽��һ���ڴ���ֱ�ӷ��أ����򽫶���ļ��뵽��������
    if(1 == numBlocks)return chunk;

    // �ҵ�����չ������λ��
    free_node* volatile* p_freelist = freelists+get_freelist_index(blockSize);
    // ׼���÷����ڴ��
    auto ret = (free_node*)chunk;
    //׼������չ�ڴ�
    auto nextBlock = (free_node*)(chunk+blockSize);
    // *****************************************************
    // ע�⣬��Ϊֻ�е���ǰ�����С����������Ϊ��ʱ�Ż����refill����ˣ������ڽ�����չ��ʱ��
    // Ӧ��ֱ�Ӹ�����ֵ����ȻҲ�������ж�һ�������Ƿ�Ϊ�գ���Ϊ������Խ��µ��ڴ��ƴ�ӵ�����
    // ����ֱ�Ӹ�ֵ��
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
    // ���Ȳ鿴��ʣ�¶���δ�����ڴ�
    size_t bytes_left = end_free-start_free;
    // Ȼ�󿴿����������Ҫ��������ֽ�
    size_t bytes_needed = blockSize*numBlocks;
    //���ʣ�µ��ڴ湻�ˣ���ô�Ϳ���ֱ�Ӵ�ʣ�µ��ڴ���н��з���
    if(bytes_left>=bytes_needed){
        auto ret = start_free;
        start_free += bytes_needed;
        return ret;
    }//����Ļ������ǲ����㹻����һ������һ�����ϵ��ڴ������С
    else if(bytes_left>=blockSize){
        // �������Է�����ڴ���������
        numBlocks = bytes_left/blockSize;
        auto ret = start_free;
        start_free += numBlocks*blockSize;
        return ret;
    }//����Ҳ����ζ���ڴ����һ���ڴ涼�����ˣ���ô��heap������
    else{
        // �ڴӶ�������֮ǰ��������Ҫ���ڴ����ʣ�µ��ڴ���䵽���ʴ�С������������
        // ************************
        // ע�⣬������һ��ϸ�����⣬Ҳ����ʣ�µ��ڴ��ֽڴ�Сһ����ALIGNҲ����8�ı�������Ϊ����������
        // �ڴ��ʱ����ǰ����������ȥ����ģ���ˣ�ʣ�µ��ڴ���Կ��Է��䵽��Ӧ���ڴ����������У��������
        // ����֮˵��Ҳ���ǲ������6���ֽ�������������8���ֽڣ�����13���ֽڣ���ô������16���ֽ�....
        // ���Һ���Ҫ��һ�㣬Ҳ���ǻ�պ�����һ��С�ڵ�ǰҪ����������С�������С�����統ǰҪ����96���ֽ�
        // ��ôʣ�²�����96���ֽ�����Ӧ����88,80,72��...,16,8�е�һ��������ҵ���Ӧ���ڴ�����������һ��
        // ��ȥ�Ϳ����ˡ�
        // ************************
        if(bytes_left>0){
            // ���ҵ����ʵ��ڴ���������λ��
            free_node* volatile* p_freelist = freelists+get_freelist_index(bytes_left);
            // ��һ����ȥ
            free_node* freenode_start = (free_node*)start_free;
            freenode_start->next_free_node = *p_freelist;
            *p_freelist = freenode_start;
            start_free = end_free = 0;
        }

        // ������ǽ�������Ҫ������ڴ��ֽ��������������Ҽ���һ������������������ݴӶ��е�����������
        // �����Ӷ�����
        size_t bytes_to_alloc = 2*bytes_needed+ALIGN_UP(heap_size>>4);
        // ��������
        start_free = (char*)malloc(bytes_to_alloc);
        end_free = 0;
        if(0 == start_free){//���malloc����ʧ�ܣ���ô�ü�����취
            // malloc����ʧ���ˣ�˵�����е���Դ�ľ��ˣ���ˣ����ǽ����Դ��ڴ�����������Ѱ��һ���ȵ�ǰ
            // ������������С��������飬�ҵ��Ļ�����н�����䵽��ǰ��С�����������ϣ�
            // ע�⣺���ﲻ����ʹ�ø�С���ڴ������С������ƴ�ӣ������ڴ��п��ܻ���ֺܶ���ڴ���Ƭ


            for(size_t i = blockSize;i<=MAX_BYTES;i+=ALIGN){
                free_node* volatile*p_freelist = freelists+get_freelist_index(i);
                auto freep = *p_freelist;
                if(0 !=  freep){//����ҵ�һ������ô�����ͷŸ��ڴ�ػ��գ�Ȼ��ݹ�����Լ����з���
                    *p_freelist = freep->next_free_node;
                    start_free = (char*)freep;
                    end_free = start_free+i;
                    return chunk_alloc(blockSize,numBlocks);
                }
            }

            // ������������ʾû�п��õ��ڴ����飬��ôֻ�ܵ���һ���ڴ�����������ڴ����
            // ��Ҫʱ�ᴥ�������oom���ƣ����oomҲû�취��������׳��쳣

            start_free =(char*) malloc_alloc::allocate(bytes_to_alloc);
        }

        // �����˵������ɹ��ˣ���ô����heap_size��end_free���ҵݹ�����Լ������µ���numBlock
        // ��������Ӧ������ָ��
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
