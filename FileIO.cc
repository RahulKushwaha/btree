//
// Created by Rahul  Kushwaha on 5/19/24.
//

#include "FileIO.h"
#include <cassert>
#include <sys/fcntl.h>
#include <unistd.h>

bool is_aligned(void *p) {
  return !(reinterpret_cast<uintptr_t>(p) % 512);
}

std::shared_ptr<FileIO> makeFileIO(std::string filePath) {
  int fd = open(filePath.c_str(), O_CREAT | O_RDWR | O_DIRECT | O_TRUNC);
  if (fd == -1) {
    int result = close(fd);
    assert(result != -1);
  }

  std::cout << "file created, fd: " << fd << std::endl;
  return std::make_shared<FileIO>(fd);
}