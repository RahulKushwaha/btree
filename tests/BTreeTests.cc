//
// Created by Rahul  Kushwaha on 7/14/24.
//

#include "BTree.h"
#include "BufferPool.h"
#include "gtest/gtest.h"

TEST(BTreeTests, EmptyTree) {
  auto bufferPool = std::make_shared<BufferPool>(nullptr);
  BTree bTree{bufferPool};
}

TEST(BTreeTests, SingleElement) {
  auto bufferPool = std::make_shared<BufferPool>(nullptr);
  BTree bTree{bufferPool};
  std::string key{"hello"};
  std::string value{"value"};

  auto result = bTree.insert(key, value);
  ASSERT_TRUE(result);

  auto response = bTree.search(key);
  ASSERT_TRUE(response.has_value());
  ASSERT_EQ(response.value(), value);
}

TEST(BTreeTests, NonExistingKey) {
  auto bufferPool = std::make_shared<BufferPool>(nullptr);
  BTree bTree{bufferPool};
  std::string key{"hello"};
  std::string value{"value"};

  auto result = bTree.insert(key, value);
  ASSERT_TRUE(result);

  auto response = bTree.search("hello1");
  ASSERT_FALSE(response.has_value());
}

TEST(BTreeTests, DeleteOperation) {
  auto bufferPool = std::make_shared<BufferPool>(nullptr);
  BTree bTree{bufferPool};
  std::string key{"hello"};
  std::string value{"value"};

  // Insert
  auto result = bTree.insert(key, value);
  ASSERT_TRUE(result);

  // Find it
  auto searchResponse = bTree.search(key);
  ASSERT_TRUE(searchResponse.has_value());
  ASSERT_EQ(searchResponse.value(), value);

  // Delete Response
  auto delResponse = bTree.del(std::string_view{key});
  ASSERT_TRUE(delResponse);

  // Try finding it again
  searchResponse = bTree.search(key);
  ASSERT_FALSE(searchResponse.has_value());
}