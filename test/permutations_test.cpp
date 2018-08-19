#include "permutations.h"
#include <string>
#include <gtest/gtest.h>
#include <iostream>

template<typename Iter>
void print(Iter beg,Iter end){
    for(;beg != end;++beg){
        std::cout << *beg;
    }
    std::cout << endl;
}

TEST(permutationsTest,nextPermuation){
    std::string s("ABBC");
    for(int i=0;i<12;++i){
        print(s.begin(),s.end());
        lotus::next_permutation(s.begin(),s.end());
    }
    ASSERT_EQ(s,std::string("CBBA"));
}
