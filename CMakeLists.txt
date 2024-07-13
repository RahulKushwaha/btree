cmake_minimum_required(VERSION 3.22)
project(btree)

set(CMAKE_CXX_STANDARD 23)
include(FetchContent)
FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)

file(GLOB_RECURSE BACKWARD_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(
        btree main.cpp

        BTree.h
        BufferPool.h
        BufferPool.cc
        Common.h
        BufferPage.h
        FileIO.h
        FileIO.cc
        BTreeNode.h
        BTreeNode.cc
        BTree.cc
        LockManager.h
        Transaction.h
        Transaction.h
)

target_link_libraries(
        btree
        ${BACKWARD_SOURCES}
        ${BACKWARD_ENABLE}
)

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
        BufferPool.cc
        BufferPool.h
        tests/FileIOTests.cc
        BTreeNode.h
)

target_link_libraries(
        tests
        gtest
        GTest::gtest_main
        explain
)

include(GoogleTest)
gtest_discover_tests(tests)