//
// Created by Rahul  Kushwaha on 5/17/24.
//

#pragma once
#include "common.h"
#include <cassert>
#include <map>
#include <memory>
#include <shared_mutex>

constexpr node_id_t NODE_ID_START = 100000;

node_id_t get_next() {
  static node_id_t id{NODE_ID_START};
  return (++id);
}

struct btree_node {
  node_id_t id;
  bool is_leaf;
  std::map<key_t, node_id_t> keys;
  node_id_t parent;
  std::unique_ptr<std::shared_mutex> mtx;
};
using btree_node_t = btree_node;

class buffer_pool {
 public:
  std::shared_ptr<btree_node_t> create_new(bool is_leaf) {
    auto node = std::make_shared<btree_node_t>(btree_node_t{
        .id = get_next(),
        .is_leaf = is_leaf,
        .mtx = std::make_unique<std::shared_mutex>(),
    });

    auto result = lookup_.emplace(node->id, node);
    assert(result.second && "duplicate entry detected in buffer_pool");

    return node;
  }

  std::optional<std::shared_ptr<btree_node_t>> get(node_id_t id) {
    auto itr = lookup_.find(id);
    if (itr == lookup_.end()) {
      return {};
    }

    return {itr->second};
  }

 private:
  std::map<node_id_t, std::shared_ptr<btree_node_t>> lookup_;
};

using buffer_pool_t = buffer_pool;