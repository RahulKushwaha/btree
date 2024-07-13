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
  std::vector<std::string> elements();
  void debug_print();

 private:
  btree_node_ptr_t findNodeForInsert(std::string_view key,
                                     std::string_view value);
  btree_node_ptr_t findNodeForSelect(std::string_view key);
  void split(btree_node_ptr_t node, btree_node_ptr_t parent, bool rootSplit);
  void debugPrint(btree_node_ptr_t node) const;
  void elements(btree_node_ptr_t node, std::vector<std::string>& keys) const;

 private:
  std::shared_ptr<buffer_pool_t> bufferPool_;
  btree_node_ptr_t root_;
};
