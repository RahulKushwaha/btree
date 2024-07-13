//
// Created by Rahul  Kushwaha on 6/21/24.
//

#pragma once
#include "BufferPage.h"

#include <memory>
#include <set>
#include <sstream>
#include <thread>

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

using length_t = std::uint16_t;

std::string convertToString(char* ptr, length_t length);

struct NonLeafKeyValue {
  node_id_t* childNodeId;
  length_t* keyLength;
  char* key;

  std::string getKeyStr() const { return convertToString(key, *keyLength); }
};

struct LeafKeyValue {
  length_t* keyLength;
  char* key;
  length_t* valueLength;
  char* value;

  std::string getKeyStr() const { return convertToString(key, *keyLength); }

  std::string getValueStr() const {
    return convertToString(value, *valueLength);
  }
};

inline length_t getBlockSizeForNonLeafNode(std::size_t keyLength) {
  return sizeof(node_id_t) + sizeof(length_t) + keyLength;
}

inline length_t getBlockSizeForLeafNode(std::size_t keyLength,
                                        std::size_t valueLength) {
  return sizeof(length_t) * 2 + keyLength + valueLength;
}

void writeNonLeafNode(node_id_t childNodeId, const std::string& key,
                      void* destBlock, length_t blockSize);
void writeLeafNode(const std::string& key, const std::string& value,
                   void* destBlock, length_t blockSize);

NonLeafKeyValue readFromNonLeafNode(void* block, length_t blockSize);
LeafKeyValue readFromLeafNode(void* block, length_t blockSize);

bool nodeCompareFunction(void* block1, void* block2, length_t block1Size,
                         length_t block2Size);

using non_leaf_key_value_t = NonLeafKeyValue;

struct BTreeNode {
  enum class Result {
    NO_SPACE_LEFT,
    NOT_FOUND,
    SUCCESS,
  };

  explicit BTreeNode(BufferPageControl* bufferPageControl);

  std::pair<Result, data_location_t> insertNonLeaf(node_id_t childNode,
                                                   const std::string& key);
  std::pair<Result, data_location_t> insertLeaf(const std::string& key,
                                                const std::string& value);
  void eraseAll();
  bool erase(data_location_t dataLocation);
  std::string getSmallestKey() const;
  std::string getLargest() const;
  std::string getKey(data_location_t dataLocation) const;
  data_location_t search(std::string key);
  void copyAllDataBlocks(BTreeNode* dest);
  void copyDataBlock(data_location_t srcDataLocation, BTreeNode* dest);
  std::int64_t getUsableFreeSpace();
  std::int64_t getTotalFreeSpace();
  void sortDataDictionary(const cmp_func_t& compareFunction);
  std::vector<data_location_t*> getDataListPtrs() const;
  std::vector<data_location_t> getDataList() const;
  bool isLeaf() const;
  void setLeaf();
  void setNonLeaf();
  node_id_t getId() const;
  void setId(node_id_t id);

  void lock();
  void unlock();

  buffer_page_control_t* bufferPageControl_;
  btree_node_header_t* btreeNodeHeader_;
  std::shared_ptr<std::mutex> mtx_;
};

using btree_node_t = BTreeNode;
using btree_node_ptr_t = btree_node_t*;