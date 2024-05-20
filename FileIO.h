//
// Created by Rahul  Kushwaha on 5/19/24.
//
#pragma once
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <libexplain/write.h>
#include <memory>
#include <unistd.h>

#include <type_traits>

bool is_aligned(void *p);

class FileIO;

std::shared_ptr<FileIO> makeFileIO(std::string filePath);

constexpr int BLOCK_SIZE = 512;
using offset_t = __off_t;

class FileIO {
 public:
  explicit FileIO(int fd) : fd_{fd} {}

  bool fileIOWrite(offset_t offset, void *buffer) {
    //Check if the input memory buffer is aligned.
    assert(is_aligned(buffer));

    assert(offset % BLOCK_SIZE == 0);
    alignas(BLOCK_SIZE) char nu_buffer[BLOCK_SIZE];
    auto result = pwrite(fd_, nu_buffer, BLOCK_SIZE, offset);
    if (result == -1) {
      std::cout << explain_write(fd_, nu_buffer, BLOCK_SIZE);
      return false;
    }

    return true;
  }

  bool read(offset_t offset, void *buffer) {
    auto result = pread(fd_, buffer, BLOCK_SIZE, offset);
    if (result == -1) {
      return false;
    }

    return true;
  }

  ~FileIO() noexcept {
    auto result = close(fd_);
    // log result here.
  }

 private:
  int fd_;
};