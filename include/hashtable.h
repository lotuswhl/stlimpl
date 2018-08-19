#pragma once

struct snode{
    int val;
    snode *next;
    snode(){
        this->val = 0;
        this->next = nullptr;
    }

    snode(int n):val(n){
        this->next = nullptr;
    }
};

class hash_table{
public:
    hash_table(const int size);
    ~hash_table();
    bool insert(int n);
    void insert(int *beg,int *end);
    bool find(int n);
    int size();
    int hashFun(int n);

private:
    int m_nTableSize;
    int m_nNodeSize;
    snode ** m_pptable;
};
