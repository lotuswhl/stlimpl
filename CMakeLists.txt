cmake_minimum_required(VERSION 3.2)

project(stl_lotus)

set(CMAKE_BUILD_TYPE Release)

include_directories("include")

file(GLOB SOURCES "src/*.cpp" "test/*.cpp")

#enable_testing()
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})

#find_package(Threads REQUIRED)

#add_library(hash_table SHARED "src/hashtable.cpp")

add_executable(main_test "test/main_test.cpp")

target_link_libraries(main_test gtest pthread )



