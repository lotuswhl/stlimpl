#include "hashtable.h"
#include <gtest/gtest.h>
#include <iterator>

TEST(hashtableTest,insertAndFind){
    hash_table ht(10);
    ht.insert(22);
    ht.insert(232);
    ASSERT_TRUE(ht.find(22));
    ASSERT_FALSE(ht.find(11));
}

TEST(hashtableTest,rangeInsert){
    hash_table ht(20);
    int a[] = {1,2,3,4,4,2};

    ht.insert(std::begin(a),std::end(a));

    ASSERT_TRUE(ht.size() == 4);
    ASSERT_TRUE(ht.find(3));
    ASSERT_FALSE(ht.find(12));
}

