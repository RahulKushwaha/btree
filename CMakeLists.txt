cmake_minimum_required(VERSION 3.27)
project(btree)

set(CMAKE_CXX_STANDARD 23)

add_executable(btree main.cpp
        btree.h
        buffer_pool.h
        common.h)
