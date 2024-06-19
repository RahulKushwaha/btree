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

auto getCompareFunction(bool isLeaf) {
  auto compareFunction = [leaf = isLeaf](void *block1, void *block2, length_t block1Size, length_t block2Size) {
    if (leaf) {
      auto node1 = readFromLeafNode(block1, block1Size);
      auto node2 = readFromLeafNode(block2, block2Size);
      return memcmp_diff_size(node1.key, node2.key, *node1.keyLength, *node2.keyLength);
    }

    auto node1 = readFromNonLeafNode(block1, block1Size);
    auto node2 = readFromNonLeafNode(block2, block2Size);
    return memcmp_diff_size(node1.key, node2.key, *node1.keyLength, *node2.keyLength);
  };

  return compareFunction;
}

class BTree {
 public:
  explicit BTree(std::shared_ptr<BufferPool> bufferPool)
      : bufferPool_{std::move(bufferPool)},
        root_{bufferPool_->createNew(true)} {}

  BTree(std::shared_ptr<BufferPool> bufferPool, node_id_t rootId)
      : bufferPool_{std::move(bufferPool)}, root_{bufferPool_->get(rootId).value_or(nullptr)} {
    assert(root_ != nullptr);
    assert(rootId == root_->getId());
  }

  node_id_t getRootId() {
    return root_->getId();
  }

  bool insert(std::string key, std::string value) {
    btree_node_ptr_t parent{nullptr};
    btree_node_ptr_t node{nullptr};

    node_id_t nodeId = root_->getId();
    while (nodeId != EMPTY_NODE_ID) {
      std::cout << nodeId << std::endl;
      auto optionalNode = bufferPool_->get(nodeId);
      assert(optionalNode.has_value() && "node not found in buffer_pool");
      node = optionalNode.value();

      node->lock();
      if (parent) {
        parent->unlock();
      }

      if (node->getDataList().size() > 5) {
        bool rootSplit{false};
        if (node == root_) {
          auto new_root = bufferPool_->createNew(false);
          root_ = new_root;
          parent = root_;
          parent->lock();
          rootSplit = true;
        }

        split(node, parent, rootSplit);

        node->unlock();

        auto optionalParentNode = bufferPool_->get(node->getParent());
        node = optionalParentNode.value_or(nullptr);
      }

      parent = node;

      if (node->isLeaf()) {
        nodeId = EMPTY_NODE_ID;
      } else {
        auto location = node->search(key);
        auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
        assert(optionalBlock.has_value());
        auto block = optionalBlock.value();
        auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
        nodeId = *nonLeafKeyValue.childNodeId;
      }
    }

    if (!parent->getDataList().empty()) {
      auto location = parent->search(key);
      auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
      assert(optionalBlock.has_value());

      assert(parent->isLeaf() && "last step of insertion should always land on leaf node");
      auto block = optionalBlock.value();
      auto leafKeyValue = readFromLeafNode(block.ptr, block.length);

      if (leafKeyValue.getKeyStr() == key) {
        return false;
      }
    }

    // The leaf does not contain the same key. We can proceed with insertion.
    auto result = parent->insertLeaf(key, value);
    assert(result.first == BTreeNode::Result::SUCCESS);

    parent->bufferPageControl_->sortDataList(getCompareFunction(parent->isLeaf()));
    parent->unlock();

    return true;
  }

  std::vector<std::string> elements() {
    std::vector<std::string> result;
    elements(root_, result);

    return result;
  }

  void debug_print() {
    debugPrint(root_);
    std::cout << std::endl;
  }

 private:
  void split(btree_node_ptr_t node, btree_node_ptr_t parent, bool rootSplit) {
    assert(node != nullptr);
    assert(parent != nullptr);

    auto newNode = bufferPool_->createNew(node->isLeaf());

    auto dataLocations = node->getDataList();
    auto splitLimit = dataLocations.size() / 2;
    auto count{0};
    for (auto &dataLocation: node->getDataList()) {
      count++;

      if (count >= splitLimit) {
        node->copyDataBlock(dataLocation, newNode);
        node->bufferPageControl_->releaseBlock(dataLocation.id);
      }
    }

    node->setParent(parent->getId());
    newNode->setParent(parent->getId());

    node->sortDataDictionary(getCompareFunction(node->isLeaf()));
    newNode->sortDataDictionary(getCompareFunction(newNode->isLeaf()));

    auto smallestKeyInNewNode = newNode->getSmallestKey();
    parent->insertNonLeaf(newNode->getId(), smallestKeyInNewNode);

    if (rootSplit) {
      auto smallestKey = node->getSmallestKey();
      parent->insertNonLeaf(node->getId(), smallestKey);
    }

    parent->sortDataDictionary(getCompareFunction(parent->isLeaf()));
  }

  void debugPrint(btree_node_ptr_t node) const {
    if (node == nullptr) {
      return;
    }

    if (node->isLeaf()) {
      std::cout << "Leaf, ";
      std::cout << "node id: " << node->getId() << std::endl;
      for (const auto &c: node->getDataList()) {
        auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
        assert(optionalBlock.has_value());
        auto block = optionalBlock.value();
        auto leafKeyValue = readFromLeafNode(block.ptr, block.length);
        std::cout << "key: " << leafKeyValue.getKeyStr() << ", value: " << leafKeyValue.getValueStr() << " | ";
      }

      std::cout << std::endl
                << std::endl;
      return;
    }

    if (node == root_) {
      std::cout << "Root, ";
    }
    std::cout << "Non Leaf, ";
    std::cout << "node id: " << node->getId() << std::endl;
    for (const auto &c: node->getDataList()) {
      auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();
      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);

      assert(*nonLeafKeyValue.childNodeId != EMPTY_NODE_ID && "non leaf cannot have nullptr as child");
      std::cout << "key: " << nonLeafKeyValue.getKeyStr() << ", child: " << *nonLeafKeyValue.childNodeId << " | ";
    }

    std::cout << std::endl
              << std::endl;

    for (const auto &c: node->getDataList()) {
      auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();
      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);

      auto page = bufferPool_->get(*nonLeafKeyValue.childNodeId);
      debugPrint(page.value_or(nullptr));
    }
  }

  void elements(btree_node_ptr_t node, std::vector<std::string> &keys) const {
    if (node == nullptr) {
      return;
    }

    if (node->isLeaf()) {
      for (const auto &c: node->getDataList()) {
        auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
        assert(optionalBlock.has_value());
        auto block = optionalBlock.value();

        auto leafKeyValue = readFromLeafNode(block.ptr, block.length);
        keys.push_back(leafKeyValue.getKeyStr());
      }

      return;
    }

    for (const auto &c: node->getDataList()) {
      auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();

      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
      auto childNodeId = *nonLeafKeyValue.childNodeId;
      auto page = bufferPool_->get(childNodeId);

      assert(page.has_value() && "every non-leaf node should always have a valid value for child");
      elements(page.value(), keys);
    }
  }

 private:
  std::shared_ptr<buffer_pool_t> bufferPool_;
  btree_node_ptr_t root_;
};
