//
// Created by Rahul  Kushwaha on 5/19/24.
//

#include "BufferPage.h"
#include "BufferPool.h"

#include <gtest/gtest.h>

#include <random>

std::string generateRandomString(std::string::size_type length) {
  static auto& chrs =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz";

  std::mt19937 rg{std::random_device{}()};
  std::uniform_int_distribution<std::string::size_type> pick(0,
                                                             sizeof(chrs) - 2);

  std::string s;
  s.reserve(length);

  while (length--)
    s += chrs[pick(rg)];

  return s;
}

TEST(BufferPageTests, NewBufferPageFreeSpace) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  ASSERT_EQ(pageControl.getTotalFreeSpace(),
            PAGE_SIZE - HEADER_SIZE - sizeof(DataLocation));
}

TEST(BufferPageTests, GetFreeSpaceFromNewBufferPage) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  void* location = pageControl.getFreeBlock(4).value();
  ASSERT_EQ((void*)&bufferPage.data, location);

  auto dataList = pageControl.getDataList();
  ASSERT_EQ(dataList.size(), 1);
  for (const auto& loc : dataList) {
    std::cout << loc;
  }
}

TEST(BufferPageTests, GetMultipleFreeSpaceFromNewBufferPage) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  int blockSize{4};
  auto totalExpectedBlocks =
      (PAGE_SIZE - HEADER_SIZE) / (blockSize + sizeof(data_location_t));

  for (int i = 0; i < totalExpectedBlocks; i++) {
    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  // Asking for more blocks should return nullptr
  for (int i = 0; i < 100; i++) {
    ASSERT_FALSE(pageControl.getFreeBlock(blockSize).has_value());
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), totalExpectedBlocks);
  }
}

TEST(BufferPageTests, FetchDataFromBlockInSortedOrder) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  int blockSize{8};
  auto totalExpectedBlocks =
      (PAGE_SIZE - HEADER_SIZE) / (blockSize + sizeof(data_location_t));

  for (int i = 0; i < totalExpectedBlocks; i++) {
    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto locationPtr = location.value();
    auto intKey = reinterpret_cast<std::int64_t*>(locationPtr);
    *intKey = i + 1;
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  auto lastValue{totalExpectedBlocks};
  for (auto& dataLocation : pageControl.getDataList()) {
    auto optionalBlockLocation = pageControl.getBlock(dataLocation.id);
    ASSERT_TRUE(optionalBlockLocation.has_value());
    auto blockLocation = optionalBlockLocation.value();

    auto value = *reinterpret_cast<std::int64_t*>(blockLocation.ptr);
    ASSERT_EQ(value, lastValue);
    std::cout << "Block Location: " << blockLocation << " | " << value
              << std::endl;

    lastValue--;
  }
}

TEST(BufferPageTests, SortDataList) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);

  int blockSize{8};
  auto limit =
      (PAGE_SIZE - HEADER_SIZE) / (blockSize + sizeof(data_location_t));

  for (int i = 0; i < limit; i++) {
    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto locationPtr = location.value();
    auto intKey = reinterpret_cast<std::int64_t*>(locationPtr);
    *intKey = i + 1;
    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  pageControl.sortDataList(memcmp_diff_size);

  auto lastValue{1};
  for (auto& dataLocation : pageControl.getDataList()) {
    auto optionalBlockLocation = pageControl.getBlock(dataLocation.id);
    ASSERT_TRUE(optionalBlockLocation.has_value());
    auto blockLocation = optionalBlockLocation.value();

    auto value = *reinterpret_cast<std::int64_t*>(blockLocation.ptr);
    ASSERT_EQ(value, lastValue);
    std::cout << "Block Location: " << blockLocation << " | " << value
              << std::endl;

    lastValue++;
  }
}

TEST(BufferPageTests, SortStringData) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);
  auto limit = 10;

  for (int i = 0; i < limit; i++) {
    std::int32_t blockSize{i + 1};

    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto locationPtr = location.value();
    auto dataPtr = reinterpret_cast<char*>(locationPtr);

    auto data = generateRandomString(blockSize);
    for (auto c : data) {
      *dataPtr = c;
      dataPtr++;
    }

    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  for (auto dataLocation : pageControl.getDataList()) {
    std::cout << dataLocation << std::endl;
  }

  pageControl.sortDataList(memcmp_diff_size);

  for (auto& dataLocation : pageControl.getDataList()) {
    auto optionalBlockLocation = pageControl.getBlock(dataLocation.id);
    ASSERT_TRUE(optionalBlockLocation.has_value());
    auto blockLocation = optionalBlockLocation.value();

    auto dataPtr = reinterpret_cast<char*>(blockLocation.ptr);
    std::stringstream ss;
    for (int i = 0; i < dataLocation.length; i++) {
      ss << (*dataPtr);
      dataPtr++;
    }

    std::cout << "Block Location: " << blockLocation << " | " << ss.str()
              << std::endl;
  }
}

TEST(BufferPageTests, SortBTreeNodes) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);
  std::vector<std::string> inputs{
      "z", "xy", "pov", "a", "b", "ab", "abc", "c", "ca", "dab", "abcde",
  };

  auto limit = inputs.size();

  for (int i = 0; i < limit; i++) {
    auto& input = inputs[i];
    auto blockSize = getBlockSizeForNonLeafNode(input.length());

    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto locationPtr = location.value();
    writeNonLeafNode(i, input, locationPtr, blockSize);

    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  pageControl.sortDataList(nodeCompareFunction);

  for (auto dataLocation : pageControl.getDataList()) {
    std::cout << dataLocation << std::endl;
  }

  for (auto& dataLocation : pageControl.getDataList()) {
    auto optionalBlockLocation = pageControl.getBlock(dataLocation.id);
    ASSERT_TRUE(optionalBlockLocation.has_value());
    auto blockLocation = optionalBlockLocation.value();
    non_leaf_key_value_t nonLeafKeyValue =
        readFromNonLeafNode(blockLocation.ptr, blockLocation.length);

    std::stringstream ss;
    for (int i = 0; i < *nonLeafKeyValue.keyLength; i++) {
      ss << nonLeafKeyValue.key[i];
    }

    std::cout << "Block Location: " << blockLocation << " | " << ss.str()
              << std::endl;
  }
}

TEST(BufferPageTests, FindInsertLocationForBTreeNode) {
  BufferPage bufferPage{};
  BufferPageControl pageControl(&bufferPage);
  std::vector<std::string> inputs{
      "z", "xy", "pov", "a", "b", "ab", "abc", "c", "ca", "dab", "abcde",
  };

  auto limit = inputs.size();

  for (int i = 0; i < limit; i++) {
    auto& input = inputs[i];
    auto blockSize = getBlockSizeForNonLeafNode(input.length());

    auto location = pageControl.getFreeBlock(blockSize);
    ASSERT_TRUE(location.has_value());
    auto locationPtr = location.value();
    writeNonLeafNode(i, input, locationPtr, blockSize);

    auto dataList = pageControl.getDataList();
    ASSERT_EQ(dataList.size(), i + 1);
  }

  pageControl.sortDataList(nodeCompareFunction);

  for (auto& dataLocation : pageControl.getDataList()) {
    auto optionalBlockLocation = pageControl.getBlock(dataLocation.id);
    ASSERT_TRUE(optionalBlockLocation.has_value());
    auto blockLocation = optionalBlockLocation.value();
    non_leaf_key_value_t nonLeafKeyValue =
        readFromNonLeafNode(blockLocation.ptr, blockLocation.length);

    std::stringstream ss;
    for (int i = 0; i < *nonLeafKeyValue.keyLength; i++) {
      ss << nonLeafKeyValue.key[i];
    }

    std::cout << "Block Location: " << blockLocation << " | " << ss.str()
              << std::endl;
  }

  std::vector<std::pair<std::string, std::string>> insertLocations{
      {"a", "a"},  {"b", "b"},   {"d", "ca"},  {"dabe", "dab"},
      {"zz", "z"}, {"s", "pov"}, {"w", "pov"},
  };

  btree_node_1_t node{&pageControl};
  for (auto& insertLocation : insertLocations) {
    auto location = node.search(insertLocation.first);

    auto optionalBlock = pageControl.getBlock(location.id);
    ASSERT_TRUE(optionalBlock.has_value());

    auto block = optionalBlock.value();
    NonLeafKeyValue nonLeafKeyValue =
        readFromNonLeafNode(block.ptr, block.length);
    std::cout << "key to insert: " << insertLocation.first << " | "
              << "insert location: " << nonLeafKeyValue.getKeyStr() << " | "
              << "expected insert location: " << insertLocation.second
              << std::endl;
    ASSERT_EQ(insertLocation.second, nonLeafKeyValue.getKeyStr());
  }
}
