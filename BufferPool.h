//
// Created by Rahul  Kushwaha on 5/17/24.
//

#pragma once
#include "BTreeNode.h"
#include "BufferPage.h"
#include "Common.h"
#include "FileIO.h"

#include <map>
#include <set>

constexpr node_id_t NODE_ID_START = 0;

class BufferPool {
 public:
  explicit BufferPool(std::shared_ptr<FileIO> fileIO);

  btree_node_ptr_t createNew(bool isLeaf);
  std::optional<btree_node_ptr_t> get(node_id_t id);
  void flushAll();

  ~BufferPool();

 private:
  std::map<node_id_t, btree_node_ptr_t> lookup_;
  std::shared_ptr<FileIO> fileIO_;
  std::unique_ptr<std::mutex> mtx_;
};

using buffer_pool_t = BufferPool;
using btree_node_1_t = BTreeNode;