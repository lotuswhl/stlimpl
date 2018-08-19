#include "hashtable.h"
#include <malloc.h>
#include <memory.h>

hash_table::hash_table(const int size){

    m_pptable = (snode**)malloc(sizeof(snode*)*size);
    if(!m_pptable)return;
    m_nTableSize = size;
    m_nNodeSize = 0;
    memset(m_pptable,0,sizeof(snode*)*size);
}

hash_table::~hash_table(){
    free(this->m_pptable);
    m_pptable = nullptr;
    m_nTableSize = 0;
    m_nNodeSize = 0;
}

int hash_table::size(){
    return this->m_nNodeSize;
}

int hash_table::hashFun(int n){
    return (n^0xdeadbeaf) % m_nTableSize;
}

bool hash_table::insert(int n){
    int hashKey = hashFun(n);
    for(snode* p = this->m_pptable[hashKey];p!=nullptr;p=p->next){
        if(p->val==n)return true;
    }
    
    auto node = new snode(n);
    if(!node)return false;
    node->next = this->m_pptable[hashKey];
    this->m_pptable[hashKey]= node;
    ++m_nNodeSize;
    return true;
}

void hash_table::insert(int *beg,int *end){
    for(;beg!= end;++beg){
        if(this->insert(*beg))
            ++this->m_nNodeSize;
    }
}

bool hash_table::find(int n){

    int hashKey = hashFun(n);

    for(auto p = this->m_pptable[hashKey];p!=nullptr;p=p->next){
        if(p->val == n)return true;
    }
    return false;
}


