#include "numeric.h"
#include <gtest/gtest.h>
#include <iterator>
using namespace std;

TEST(accumulate,integerArray){
    int a[] = {1,2,3,4,5};

    ASSERT_EQ(lotus::accumulate(begin(a),end(a),0),15);
}
