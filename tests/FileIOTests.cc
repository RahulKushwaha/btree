//
// Created by Rahul  Kushwaha on 5/19/24.
//

#include "../FileIO.h"
#include "gtest/gtest.h"

TEST(FileIOTests, WriteAndReadFile) {
  auto fileIO = makeFileIO("/tmp/hello.txt");
  alignas(512) char text[512];

  auto result = fileIO->fileIOWrite(0, (void *) text);
  ASSERT_TRUE(result);
}
