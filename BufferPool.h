//
// Created by Rahul  Kushwaha on 5/17/24.
//

#pragma once
#include "BufferPage.h"
#include "Common.h"
#include "FileIO.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <sstream>

constexpr node_id_t NODE_ID_START = 0;

node_id_t getNext();

struct BTreeNodeHeader {
  node_id_t id;
  node_id_t parent;
  std::uint8_t isLeaf;
  std::uint8_t isInitialized;
};

using btree_node_header_t = BTreeNodeHeader;

struct BTreeNodeEndSentinel {
  std::uint64_t dummy;
};
constexpr static const BTreeNodeEndSentinel BTREE_NODE_END_SENTINEL{};
constexpr static const void *BTREE_NODE_END = &BTREE_NODE_END_SENTINEL;

using length_t = std::uint16_t;

std::string convertToString(char *ptr, length_t length);

struct NonLeafKeyValue {
  node_id_t *childNodeId;
  length_t *keyLength;
  char *key;

  std::string getKeyStr() const {
    return convertToString(key, *keyLength);
  }
};

struct LeafKeyValue {
  length_t *keyLength;
  char *key;
  length_t *valueLength;
  char *value;

  std::string getKeyStr() const {
    return convertToString(key, *keyLength);
  }

  std::string getValueStr() const {
    return convertToString(value, *valueLength);
  }
};

inline length_t getBlockSizeForNonLeafNode(std::size_t keyLength) {
  return sizeof(node_id_t) + sizeof(length_t) + keyLength;
}

inline length_t getBlockSizeForLeafNode(std::size_t keyLength, std::size_t valueLength) {
  return sizeof(length_t) * 2 + keyLength + valueLength;
}

void writeNonLeafNode(node_id_t childNodeId, const std::string &key, void *destBlock, length_t blockSize);
void writeLeafNode(const std::string &key, const std::string &value, void *destBlock, length_t blockSize);

NonLeafKeyValue readFromNonLeafNode(void *block, length_t blockSize);
LeafKeyValue readFromLeafNode(void *block, length_t blockSize);

bool nodeCompareFunction(void *block1, void *block2, length_t block1Size, length_t block2Size);

using non_leaf_key_value_t = NonLeafKeyValue;

struct BTreeNode {
  enum class Result {
    NO_SPACE_LEFT,
    NOT_FOUND,
    SUCCESS,
  };

  explicit BTreeNode(BufferPageControl *bufferPageControl) : bufferPageControl_{bufferPageControl}, btreeNodeHeader{reinterpret_cast<btree_node_header_t *>(bufferPageControl_->getSubHeaderLocation().ptr)} {}

  std::pair<Result, data_location_t> insertNonLeaf(node_id_t childNode, const std::string &key) {
    assert(isLeaf() == false && "cannot insert in leaf");

    auto size = sizeof(node_id_t) + sizeof(length_t) + key.size();

    if (size > bufferPageControl_->getTotalFreeSpace()) {
      return {Result::NO_SPACE_LEFT, data_location_t{}};
    }

    auto optionalBlock = bufferPageControl_->getFreeBlock(size);
    assert(optionalBlock.has_value());

    auto block = optionalBlock.value();

    writeNonLeafNode(childNode, key, block, size);

    return {Result::SUCCESS, data_location_t{}};
  }

  std::pair<Result, data_location_t> insertLeaf(const std::string &key, const std::string &value) {
    assert(isLeaf() && "cannot insert in non-leaf");

    auto size = sizeof(length_t) * 2 + key.size() + value.size();

    if (size > bufferPageControl_->getTotalFreeSpace()) {
      return {Result::NO_SPACE_LEFT, data_location_t{}};
    }

    auto optionalBlock = bufferPageControl_->getFreeBlock(size);
    assert(optionalBlock.has_value());

    auto block = optionalBlock.value();

    writeLeafNode(key, value, block, size);

    return {Result::SUCCESS, data_location_t{}};
  }

  bool erase(data_location_t dataLocation) {
    return true;
  }

  std::string getSmallestKey() const {
    auto dataList = bufferPageControl_->getDataList();
    auto first = dataList.at(0);
    return getKey(first);
  }

  std::string getLargest() const {
    auto dataList = bufferPageControl_->getDataList();
    auto first = dataList.at(dataList.size() - 1);
    return getKey(first);
  }

  std::string getKey(data_location_t dataLocation) const {
    auto optionalBlock = bufferPageControl_->getBlock(dataLocation.id);
    assert(optionalBlock.has_value());
    auto block = optionalBlock.value();

    return [&block, this]() {
      if (isLeaf()) {
        auto keyValue = readFromLeafNode(block.ptr, block.length);
        return keyValue.getKeyStr();
      }

      auto keyValue = readFromNonLeafNode(block.ptr, block.length);
      return keyValue.getKeyStr();
    }();
  }

  data_location_t search(std::string key) {
    auto dataList = bufferPageControl_->getDataListPtrs();

    std::unordered_map<data_location_id_t, void *> locationLookup;
    for (auto &dataLocation: dataList) {
      auto blockLocation = bufferPageControl_->getBlock(dataLocation->id);
      assert(blockLocation.has_value());
      auto result = locationLookup.emplace(dataLocation->id, blockLocation.value().ptr);
      assert(result.second && "failed to insert in the locationLookup");
    }

    auto result = std::upper_bound(dataList.begin(), dataList.end(), key, [&locationLookup, this](const auto &x, const auto &y) {
      assert(locationLookup.contains(y->id));

      auto dataY = locationLookup[y->id];

      struct Key {
        uint32_t length;
        void *data;
      };
      BTreeNode *node{this};

      Key key = [&dataY, node, &y]() {
        if (node->btreeNodeHeader->isLeaf) {
          auto keyValue = readFromLeafNode(dataY, y->length);
          return Key{
              .length = *keyValue.keyLength,
              .data = keyValue.key,
          };
        }

        auto keyValue = readFromNonLeafNode(dataY, y->length);
        return Key{
            .length = *keyValue.keyLength,
            .data = keyValue.key,
        };
      }();

      return memcmp_diff_size(x.c_str(), key.data, x.length(), key.length);
    });

    result--;

    return **result;
  }

  void copyDataBlock(data_location_t srcDataLocation, BTreeNode *dest) {
    auto dataDictionaryList = bufferPageControl_->getDataList();
    auto itr = std::find(dataDictionaryList.begin(), dataDictionaryList.end(), srcDataLocation);

    if (itr == dataDictionaryList.end()) {
      // data block not found.
      return;
    }

    auto optionalBlock = bufferPageControl_->getBlock(srcDataLocation.id);
    assert(optionalBlock.has_value());

    auto block = optionalBlock.value();

    // 1 means it is leaf, otherwise 0.
    if (isLeaf()) {
      auto leafKeyValue = readFromLeafNode(block.ptr, block.length);
      dest->insertLeaf(leafKeyValue.getKeyStr(), leafKeyValue.getValueStr());
    } else {
      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
      dest->insertNonLeaf(*nonLeafKeyValue.childNodeId, nonLeafKeyValue.getKeyStr());
    }
  }

  std::int64_t getUsableFreeSpace() {
    return bufferPageControl_->getUsableFreeSpace();
  }

  std::int64_t getTotalFreeSpace() {
    return bufferPageControl_->getTotalFreeSpace();
  }

  void sortDataDictionary() {
    bufferPageControl_->sortDataList(nodeCompareFunction);
  }

  std::vector<data_location_t *> getDataListPtrs() const {
    return bufferPageControl_->getDataListPtrs();
  }

  std::vector<data_location_t> getDataList() const {
    return bufferPageControl_->getDataList();
  }

  bool isLeaf() const {
    return btreeNodeHeader->isLeaf == 1;
  }

  void setLeaf() {
    btreeNodeHeader->isLeaf = 1;
  }

  node_id_t getId() const {
    return btreeNodeHeader->id;
  }

  void setId(node_id_t id) {
    btreeNodeHeader->id = id;
  }

  node_id_t getParent() const {
    return btreeNodeHeader->parent;
  }

  void setParent(node_id_t parentId) {
    btreeNodeHeader->parent = parentId;
  }

  buffer_page_control_t *bufferPageControl_;
  btree_node_header_t *btreeNodeHeader;
};

using btree_node_t = BTreeNode;
using btree_node_ptr_t = btree_node_t *;

class BufferPool {
 public:
  explicit BufferPool(std::shared_ptr<FileIO> fileIO) : fileIO_{std::move(fileIO)} {}

  btree_node_ptr_t createNew(bool isLeaf) {
    auto page = std::aligned_alloc(PAGE_SIZE, PAGE_SIZE);
    auto bufferPage = reinterpret_cast<BufferPage *>(page);
    auto *pageControl = new BufferPageControl{bufferPage};
    auto *node = new BTreeNode{pageControl};

    auto nodeId = getNext();

    node->setId(nodeId);
    if (isLeaf) {
      node->setLeaf();
    }

    auto result = lookup_.emplace(nodeId, node);
    assert(result.second && "duplicate entry detected in buffer_pool");

    return node;
  }

  std::optional<btree_node_ptr_t> get(node_id_t id) {
    auto itr = lookup_.find(id);
    if (itr == lookup_.end()) {
      auto page = std::aligned_alloc(PAGE_SIZE, PAGE_SIZE);
      fileIO_->fRead(id * PAGE_SIZE, page);
      auto bufferPage = reinterpret_cast<BufferPage *>(page);
      auto *pageControl = new BufferPageControl{bufferPage};
      auto *node = new BTreeNode{pageControl};

      auto result = lookup_.emplace(id, node);
      assert(result.second && "failed to add to buffer pool");
      return {result.first->second};
    }

    return {itr->second};
  }

  void flushAll() {
    for (auto [nodeId, node]: lookup_) {
      auto offset = nodeId * PAGE_SIZE;
      fileIO_->fWrite(offset, node->bufferPageControl_->getBufferPage());
      fileIO_->sync();
    }
  }

  ~BufferPool() {
    for (auto [nodeId, node]: lookup_) {
      free(node->bufferPageControl_->getBufferPage());
    }
  }

 private:
  std::map<node_id_t, btree_node_ptr_t> lookup_;
  std::shared_ptr<FileIO> fileIO_;
};

using buffer_pool_t = BufferPool;
using btree_node_1_t = BTreeNode;