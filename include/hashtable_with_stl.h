#pragma once

#include <vector>
#include <forward_list>
#include <iterator>
#include <limits>
#include <algorithm>

template<typename T,typename C = std::vector<std::forward_list<T>> >
class hash_table{
public:
    hash_table();
    hash_table(int cumstom_size);
    ~hash_table();
    bool insert(const T& value);

    void insert(const T*beg,const T*end);

    bool find(const T& value);

    int size();
    void resize(int newSize);

    bool erase(const T& value);

    int hashFunc(const T& value);

    void clear(); 


private:
    int m_nTableSize;
    int m_nNodeSize;
    C m_c; // storage container
    // such primes as tablesize has good properties
    static const unsigned int primes[50];
    int getNextPrime(int currPime);
};

template<typename T,typename C>
inline
int hash_table<T,C>::hashFunc(const T&value){
    return (value ^ 0xdeadbeaf) % this->m_nTableSize;
}


template<typename T,typename C>
const unsigned int hash_table<T,C>::primes[]={
    53, 97, 193, 389, 769, 1453, 3079, 6151, 1289, 24593, 49157, 98317, 
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 
    25165843, 50331653, 100663319, 201326611, std::numeric_limits<unsigned int>::max()
};

template<typename T,typename C>
hash_table<T,C>::hash_table(){
    this->m_c = C(primes[0]);
    this->m_nTableSize = primes[0];
    this->m_nNodeSize = 0;
}

template<typename T,typename C>
hash_table<T,C>::hash_table(int custom_size){
    this->m_c = C(custom_size);
    this->m_nNodeSize = 0;
    this->m_nTableSize = custom_size;
}

template<typename T,typename C>
int hash_table<T,C>::getNextPrime(int currPrime){
    auto beg = std::begin(primes);
    auto end = std::end(primes);
    auto pos = std::upper_bound(beg,end,currPrime);
    if(pos != end) return *pos;
    return 0;
}

template<typename T,typename C>
void hash_table<T,C>::clear(){
    for(auto beg = this->m_c.begin();beg != this->m_c.end();++beg){
        beg->clear();
    }
    this->m_c.clear();
    this->m_nNodeSize = 0;
    this->m_nTableSize = 0;
}

template<typename T,typename C>
hash_table<T,C>::~hash_table(){
    clear();
}


template<typename T,typename C>
bool hash_table<T,C>::insert(const T& value){
    int hashKey = hashFunc(value);
    auto& plist = this->m_c[hashKey];
    auto ret  = std::find(plist.begin(),plist.end(),value);
    if(ret != plist.end()) return false;
    plist.push_front(value);
    ++this->m_nNodeSize;
    return true;
}

template<typename T,typename C>
bool hash_table<T,C>::find(const T& value){
    int hashKey = hashFunc(value);
    auto& plist = this->m_c[hashKey];
    auto beg = plist.begin();
    auto end = plist.end();
    auto pos = std::find(beg,end,value);
    if(end == pos) return false;
    return true;
}

template<typename T,typename C>
void hash_table<T,C>::insert(const T*beg,const T*end){
    for(;beg != end;){
        if(this->insert(*beg))
            ++this->m_nNodeSize;
    }
}

