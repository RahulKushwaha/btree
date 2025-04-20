cmake_minimum_required(VERSION 3.28)
project(btree)

set(CMAKE_CXX_STANDARD 23)

include_directories(${PROJECT_SOURCE_DIR}/)

include(FetchContent)
FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)

file(GLOB_RECURSE BACKWARD_SOURCES CONFIGURE_DEPENDS src/*.cpp)

find_package(Boost REQUIRED COMPONENTS fiber context)

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
        BTreeCursor.cc
        BTreeCursor.h
        block_store/include/LogStore.h
        block_store/include/Segment.h
        block_store/include/DataStore.h
        block_store/include/Applicator.h
        block_store/include/RowBlockStore.h
        block_store/RowBlockStoreImpl.cc
        block_store/RowBlockStoreImpl.h
        block_store/include/Common.h
        block_store/SegmentImpl.cc
        block_store/SegmentImpl.h
        block_store/InMemoryLogStore.cc
        block_store/InMemoryLogStore.h
        block_store/include/Sequencer.h
        block_store/SequencerImpl.cc
        block_store/SequencerImpl.h
        block_store/include/LogMessages.h
        common/Page.cc
        common/Page.h
        common/RowBlock.cc
        common/RowBlock.h
        block_store/commands/TimeBasedBlockSplit.cc
        block_store/commands/TimeBasedBlockSplit.h
        block_store/commands/InsertRecord.cc
        block_store/commands/InsertRecord.h

        block_store/PageWriter.cc
        block_store/PageWriter.h
        block_store/commands/DeleteRecord.cc
        block_store/commands/DeleteRecord.h
        block_store/commands/AllocateRowBlock.cc
        block_store/commands/AllocateRowBlock.h
        block_store/include/BlockStore.h
        block_store/InMemoryBlockStore.cc
        block_store/InMemoryBlockStore.h
        log_writer/LogWriter.cc
        log_writer/LogWriter.h
        common/Common.h
        log_writer/DataStoreMetadata.cc
        log_writer/DataStoreMetadata.h
        page_reader/PageReader.cc
        page_reader/PageReader.h
        tests/PageReaderTests.cc
)

target_link_libraries(
        btree
        ${BACKWARD_SOURCES}
        ${BACKWARD_ENABLE}
        Boost::fiber Boost::context
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
        tests/PageReaderTests.cc

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