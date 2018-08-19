#include <gtest/gtest.h>
#include "accumulate_test.cpp"
#include <unordered_set>
#include "permutations_test.cpp"
#include "hashtable_with_stl_test.cpp"
int main(int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
