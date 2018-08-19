#pragma once
#include <algorithm>

namespace lotus{

template<typename RandomIterator>
void next_permutation(RandomIterator beg,RandomIterator end){
    if(end - beg <=1) return;

    auto i = end -2;

    while(i!=beg && !(*i<*(i+1)) )--i;
    
    // 已经完成所有排列
    if(i == beg && !(*i < *(i+1)))return;

    auto j = end -1;

    while(!(*i<*j))--j;

    std::swap(*i,*j);

    std::reverse(i+1,end);
}




}
