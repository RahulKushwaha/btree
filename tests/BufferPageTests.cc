//
// Created by Rahul  Kushwaha on 5/19/24.
//

#include "../BufferPage.h"
#include <gtest/gtest.h>

TEST(BufferPageTests, NewBufferPageFreeSpace) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  ASSERT_EQ(pageControl.getTotalFreeSpace(), PAGE_SIZE - HEADER_SIZE - sizeof(DataLocation));
}

TEST(BufferPageTests, GetFreeSpaceFromNewBufferPage) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  char *location = pageControl.getBlock(4).value();
  ASSERT_EQ((char *) &bufferPage.data, location);

  auto dataList = pageControl.getDataList();
  ASSERT_EQ(dataList.size(), 1);
  for (const auto &loc: dataList) {
    std::cout << loc;
  }
}

TEST(BufferPageTests, GetMultipleFreeSpaceFromNewBufferPage) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  int blockSize{4};
  auto totalExpectedBlocks = (PAGE_SIZE - HEADER_SIZE) / (blockSize + sizeof(data_location_t));

  for (int i = 0; i < totalExpectedBlocks; i++) {
    auto location = pageControl.getBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  // Asking for more blocks should return nullptr
  for (int i = 0; i < 100; i++) {
    ASSERT_FALSE(pageControl.getBlock(blockSize).has_value());
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), totalExpectedBlocks);
  }
}
