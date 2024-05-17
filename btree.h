//
// Created by Rahul  Kushwaha on 3/15/24.
//

#pragma once
#include "buffer_pool.h"
#include "common.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <shared_mutex>
#include <vector>

constexpr node_id_t EMPTY_NODE_ID = 0;

class btree {
 public:
  btree() : buffer_pool_{std::make_shared<buffer_pool_t>()},
            root_{buffer_pool_->create_new(true)} {}

  void insert(key_t key) {
    std::shared_ptr<btree_node_t> grandParent{nullptr};
    std::shared_ptr<btree_node_t> parent{nullptr};
    std::shared_ptr<btree_node_t> node{nullptr};

    node_id_t node_id = root_->id;
    while (node_id != EMPTY_NODE_ID) {
      auto optionalNode = buffer_pool_->get(node_id);
      assert(optionalNode.has_value() && "node not found in buffer_pool");
      node = optionalNode.value();

      if (node->keys.size() + 1 >= LIMIT) {
        bool root_split{false};
        if (node == root_) {
          auto new_root = buffer_pool_->create_new(false);
          root_ = new_root;
          parent = root_;
          root_split = true;
        }

        split(node, parent, root_split);
        auto optionalParentNode = buffer_pool_->get(node->parent);
        node = optionalParentNode.value_or(nullptr);
        parent = optionalParentNode.value_or(nullptr);
      }

      if (node == nullptr) {
        break;
      }

      grandParent = parent;
      parent = node;

      if (!node->keys.empty()) {
        auto itr = node->keys.upper_bound(key);
        itr--;

        node_id = itr->second;

      } else {
        node_id = EMPTY_NODE_ID;
      }
    }

    parent->keys.insert(std::make_pair(key, EMPTY_NODE_ID));
  }

  std::vector<key_t> elements() {
    std::vector<key_t> result;
    elements(root_, result);

    return result;
  }

  void debug_print() {
    debug_print(root_);
    std::cout << std::endl;
  }

 private:
  void split(std::shared_ptr<btree_node_t> node, std::shared_ptr<btree_node_t> parent, bool root_split) {
    assert(node != nullptr);
    assert(parent != nullptr);

    auto new_node = buffer_pool_->create_new(node->is_leaf);

    auto split_limit = LIMIT / 2;
    auto count{0};
    for (auto itr = node->keys.begin(); itr != node->keys.end(); itr++) {
      count++;

      if (count >= split_limit) {
        new_node->keys.insert(itr, node->keys.end());
        node->keys.erase(itr, node->keys.end());
        break;
      }
    }

    node->parent = parent->id;
    new_node->parent = parent->id;

    auto smallest_key_in_new_node = new_node->keys.begin()->first;
    parent->keys.insert(std::make_pair(smallest_key_in_new_node, new_node->id));

    if (root_split) {
      auto smallest_key_in_old_node = node->keys.begin()->first;
      parent->keys.insert(std::make_pair(smallest_key_in_old_node, node->id));
    }
  }

  void debug_print(std::shared_ptr<btree_node_t> node) {
    if (node == nullptr) {
      return;
    }

    if (node->is_leaf) {
      std::cout << "Leaf, ";
      std::cout << "node id: " << node->id << std::endl;
      for (const auto &c: node->keys) {
        std::cout << c.first << " ";
      }

      std::cout << std::endl
                << std::endl;
      return;
    }

    if (node == root_) {
      std::cout << "Root, ";
    }
    std::cout << "Non Leaf, ";
    std::cout << "node id: " << node->id << std::endl;
    for (const auto &c: node->keys) {
      if (c.second) {
        std::cout << c.first << " | ";
      } else {
        assert(c.second && "non leaf cannot have nullptr as child");
      }
    }
    std::cout << std::endl
              << std::endl;

    for (const auto &c: node->keys) {
      auto optionalNode = buffer_pool_->get(c.second);
      debug_print(optionalNode.value_or(std::shared_ptr<btree_node_t>{}));
    }
  }

  void elements(std::shared_ptr<btree_node_t> node, std::vector<key_t> &keys) {
    if (node == nullptr) {
      return;
    }

    if (node->is_leaf) {
      for (const auto c: node->keys) {
        keys.push_back(c.first);
      }
    }

    for (const auto &c: node->keys) {
      auto optionalNode = buffer_pool_->get(c.second);
      elements(optionalNode.value_or(std::shared_ptr<btree_node_t>{}), keys);
    }
  }

 private:
  std::shared_ptr<buffer_pool_t> buffer_pool_;
  std::shared_ptr<btree_node_t> root_;
};
