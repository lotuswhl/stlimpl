#include <gtest/gtest.h>
#include "accumulate_test.cpp"
#include "hashtable_test.cpp"
#include <unordered_set>
int main(int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
