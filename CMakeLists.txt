cmake_minimum_required(VERSION 3.22)
project(btree)

set(CMAKE_CXX_STANDARD 23)

include_directories(${PROJECT_SOURCE_DIR}/)

include(FetchContent)
FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)

file(GLOB_RECURSE BACKWARD_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(
        btree main.cpp

        BTree.cc
        BTree.h
        BTreeNode.cc
        BTreeNode.h
        BufferPage.cc
        BufferPage.h
        BufferPool.cc
        BufferPool.h
        Common.h
        FileIO.cc
        FileIO.h
        LockManager.cc
        LockManager.h
        Api.h
        Api.cc
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
        tests/FileIOTests.cc
        tests/BTreeTests.cc

        BTree.cc
        BTree.h
        BTreeNode.cc
        BTreeNode.h
        BufferPage.cc
        BufferPage.h
        BufferPool.cc
        BufferPool.h
        Common.h
        FileIO.cc
        FileIO.h
        LockManager.cc
        LockManager.h
        Api.h
        Api.cc
)

target_link_libraries(
        tests
        gtest
        GTest::gtest_main
        explain
)

include(GoogleTest)
gtest_discover_tests(tests)