//
// Created by Rahul  Kushwaha on 5/19/24.
//
#pragma once
#include "Common.h"
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <libexplain/pwrite.h>
#include <memory>
#include <unistd.h>

#include <type_traits>

bool is_aligned(void *p);

class FileIO;

std::shared_ptr<FileIO> makeFileIO(std::string filePath);

using offset_t = __off_t;

class FileIO {
 public:
  explicit FileIO(int fd) : fd_{fd} {}

  bool fWrite(offset_t offset, void *buffer) {
    //Check if the input memory buffer is aligned.
    assert(is_aligned(buffer));
    assert(offset % PAGE_SIZE == 0);

    auto result = pwrite(fd_, buffer, PAGE_SIZE, offset);
    if (result == -1) {
      //      std::cout << explain_pwrite(fd_, buffer, BLOCK_SIZE);
      return false;
    }

    return true;
  }

  bool fRead(offset_t offset, void *buffer) {
    //Check if the input memory buffer is aligned.
    assert(is_aligned(buffer));
    assert(offset % PAGE_SIZE == 0);

    auto result = pread(fd_, buffer, PAGE_SIZE, offset);
    if (result == -1) {
      return false;
    }

    return true;
  }

  bool sync() {
    auto result = fsync(fd_);
    if (result == -1) {
      std::cout << "failed to sync" << std::endl;
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