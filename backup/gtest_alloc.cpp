#include<gtest/gtest.h>
#include "alloc.h"

TEST(malloc_alloc,allocate_and_deallocate){
    using alloc = faststl::malloc_alloc;
    double *a = static_cast<double*>(alloc::allocate(sizeof(double)));
    faststl::construct(a,200.3);

    ASSERT_EQ(*a,200.3);

    alloc::deallocate(a,0);
}

TEST(default_alloc,allocate_and_deallocate){
    using alloc = faststl::alloc;

    long *a = static_cast<long*>(alloc::allocate(sizeof(long)));

    faststl::construct(a,2123213213213213);

    ASSERT_EQ(*a,2123213213213213);

    alloc::deallocate(a,sizeof(long));
}

TEST(default_alloc,simple_alloc){
    using alloc = faststl::simple_alloc<long>;

    size_t n = 10;
    long *a = alloc::allocate(n);

    for(size_t i=0;i<n;++i){
        faststl::construct(a+i,i*300+1);
    }

    for(size_t i=0;i<n;++i){
        ASSERT_EQ(*(a+i),i*300+1);
    }

    alloc::deallocate(a,10);
}
