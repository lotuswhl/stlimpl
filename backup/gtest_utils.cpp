#include <gtest/gtest.h>
#include "utils.h"


TEST(utils,construct){
    int *a = new int(22);
    faststl::construct(a,66);
    ASSERT_EQ(*a,66);
    delete a;
}

