//
// Created by Rahul  Kushwaha on 3/15/24.
//

#pragma once
#include "BufferPool.h"
#include "Common.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <shared_mutex>
#include <vector>

constexpr node_id_t EMPTY_NODE_ID = 0;

using key_condition_t = std::function<bool(const std::string&)>;
enum class ScanDirection {
  Forward,
  Backward,
};

struct ScanOperation {
  std::string startKey;
  key_condition_t keyCondition;
  ScanDirection scanDirection;
  std::int32_t limit;
  std::int32_t numberOfRowsToScan;
};

class BTree {
 public:
  struct KeyValue {
    std::string key;
    std::string value;
  };

  struct RangeResult {
    std::vector<KeyValue> keyValues;
  };

  explicit BTree(std::shared_ptr<BufferPool> bufferPool);
  BTree(std::shared_ptr<BufferPool> bufferPool, node_id_t rootId);

  node_id_t getRootId();

  bool insert(std::string key, std::string value);
  std::optional<std::string> search(std::string key);
  bool del(std::string_view key);
  std::vector<KeyValue> scan(ScanOperation scanOperation);

  std::vector<std::string> elements();
  void debug_print();

 private:
  btree_node_ptr_t findNodeForInsert(std::string_view key);
  btree_node_ptr_t findNodeForSelect(std::string_view key);
  void split(btree_node_ptr_t node, btree_node_ptr_t parent, bool rootSplit);
  void debugPrint(btree_node_ptr_t node) const;
  void elements(btree_node_ptr_t node, std::vector<std::string>& keys) const;

 private:
  std::shared_ptr<buffer_pool_t> bufferPool_;
  btree_node_ptr_t root_;
};
