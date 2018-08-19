#include "hashtable_with_stl.h"
#include <gtest/gtest.h>

TEST(hashtableWithSTL,insertAndFind){
    hash_table<int>  ht;

    ht.insert(2323);
    ht.insert(21);

    ASSERT_TRUE(ht.find(21));
    ASSERT_FALSE(ht.find(2));

}
