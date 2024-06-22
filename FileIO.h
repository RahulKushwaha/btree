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
#include <memory>
#include <unistd.h>

#include <type_traits>

bool is_aligned(void *p);

class FileIO;

std::shared_ptr<FileIO> makeFileIO(std::string filePath);

using offset_t = __off_t;

class FileIO {
 public:
  explicit FileIO(int fd);

  bool fWrite(offset_t offset, void *buffer);
  bool fRead(offset_t offset, void *buffer);
  bool sync();

  ~FileIO() noexcept;

 private:
  int fd_;
};