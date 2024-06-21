//
// Created by Rahul  Kushwaha on 5/19/24.
//

#include "FileIO.h"
#include <cassert>
#include <sys/fcntl.h>
#include <unistd.h>
#include <libexplain/pwrite.h>

bool is_aligned(void *p) {
  return !(reinterpret_cast<uintptr_t>(p) % 512);
}

std::shared_ptr<FileIO> makeFileIO(std::string filePath) {
  int fd = open(filePath.c_str(), O_CREAT | O_RDWR | O_DIRECT | O_TRUNC);
  if (fd == -1) {
    assert(fd != -1);
  }

  std::cout << "file created, fd: " << fd << std::endl;
  return std::make_shared<FileIO>(fd);
}

FileIO::FileIO(int fd) : fd_{fd} {}

bool FileIO::fWrite(offset_t offset, void *buffer) {
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

bool FileIO::fRead(offset_t offset, void *buffer) {
  //Check if the input memory buffer is aligned.
  assert(is_aligned(buffer));
  assert(offset % PAGE_SIZE == 0);

  auto result = pread(fd_, buffer, PAGE_SIZE, offset);
  if (result == -1) {
    return false;
  }

  return true;
}

bool FileIO::sync() {
  auto result = fsync(fd_);
  if (result == -1) {
    std::cout << "failed to sync" << std::endl;
    return false;
  }

  return true;
}

FileIO::~FileIO() noexcept {
  auto result = close(fd_);
  // log result here.
}