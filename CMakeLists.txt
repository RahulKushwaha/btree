cmake_minimum_required(VERSION 3.22)
project(btree)

set(CMAKE_CXX_STANDARD 23)

add_executable(btree main.cpp
        btree.h
        buffer_pool.h
        common.h
        BufferPage.h
        FileIO.h
        FileIO.cc
        buffer_pool.cc)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        tests
        tests/BufferPageTests.cc
        FileIO.h
        FileIO.cc
        buffer_pool.cc
        buffer_pool.h
        tests/FileIOTests.cc
)

target_link_libraries(
        tests
        gtest
        GTest::gtest_main
        explain
)

include(GoogleTest)
gtest_discover_tests(tests)