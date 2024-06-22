//
// Created by Rahul  Kushwaha on 6/21/24.
//
#include "BTreeNode.h"

std::string convertToString(char *ptr, length_t length) {
  std::stringstream ss;
  for (int i = 0; i < length; i++) {
    ss << ptr[i];
  }

  return ss.str();
}

void writeNonLeafNode(node_id_t childNodeId, const std::string &key, void *destBlock, length_t blockSize) {
  assert(destBlock && "destBlock cannot be null");

  auto totalSize = sizeof(node_id_t) + sizeof(length_t) + key.length();
  assert(totalSize == blockSize && "block size is not equal to the key size");

  auto baseAddr = reinterpret_cast<char *>(destBlock);
  NonLeafKeyValue nonLeafKeyValue{};
  nonLeafKeyValue.childNodeId = reinterpret_cast<node_id_t *>(baseAddr);
  nonLeafKeyValue.keyLength = reinterpret_cast<length_t *>(baseAddr + sizeof(node_id_t));
  nonLeafKeyValue.key = reinterpret_cast<char *>(baseAddr + sizeof(node_id_t) + sizeof(length_t));

  *nonLeafKeyValue.childNodeId = childNodeId;
  *nonLeafKeyValue.keyLength = key.size();

  auto keyAddr = nonLeafKeyValue.key;
  for (auto c: key) {
    *keyAddr = c;
    keyAddr++;
  }
}

void writeLeafNode(const std::string &key, const std::string &value, void *destBlock, length_t blockSize) {
  assert(destBlock && "destBlock cannot be null");

  auto totalSize = sizeof(length_t) * 2 + key.length() + value.length();
  assert(totalSize == blockSize && "block size is not equal to the accommodate key and value");

  auto baseAddr = reinterpret_cast<char *>(destBlock);
  LeafKeyValue leafKeyValue{};
  leafKeyValue.keyLength = reinterpret_cast<length_t *>(baseAddr);
  leafKeyValue.key = reinterpret_cast<char *>(baseAddr + sizeof(length_t));
  leafKeyValue.valueLength = reinterpret_cast<length_t *>(baseAddr + sizeof(length_t) + key.length());
  leafKeyValue.value = reinterpret_cast<char *>(baseAddr + sizeof(length_t) + key.length() + sizeof(length_t));

  *leafKeyValue.keyLength = (length_t) key.size();
  auto keyAddr = leafKeyValue.key;
  for (auto c: key) {
    *keyAddr = c;
    keyAddr++;
  }

  *leafKeyValue.valueLength = (length_t) value.size();
  auto valueAddr = leafKeyValue.value;
  for (auto c: value) {
    *valueAddr = c;
    valueAddr++;
  }
}

NonLeafKeyValue readFromNonLeafNode(void *block, length_t blockSize) {
  assert(block && "block cannot be null");
  char *baseAddr = reinterpret_cast<char *>(block);
  NonLeafKeyValue nonLeafKeyValue{
      .childNodeId = reinterpret_cast<node_id_t *>(baseAddr),
      .keyLength = reinterpret_cast<length_t *>(baseAddr + sizeof(node_id_t)),
      .key = reinterpret_cast<char *>(baseAddr + sizeof(node_id_t) + sizeof(length_t)),
  };

  return nonLeafKeyValue;
}

LeafKeyValue readFromLeafNode(void *block, length_t blockSize) {
  assert(block && "block cannot be null");
  char *baseAddr = reinterpret_cast<char *>(block);
  LeafKeyValue leafKeyValue{
      .keyLength = reinterpret_cast<length_t *>(baseAddr),
      .key = reinterpret_cast<char *>(baseAddr + sizeof(length_t)),
  };

  leafKeyValue.valueLength = reinterpret_cast<length_t *>(baseAddr + sizeof(length_t) + *leafKeyValue.keyLength);
  leafKeyValue.value = reinterpret_cast<char *>(baseAddr + sizeof(length_t) + *leafKeyValue.keyLength + sizeof(length_t));

  return leafKeyValue;
}

bool nodeCompareFunction(void *block1, void *block2, length_t block1Size, length_t block2Size) {
  auto lambda = [block1, block2, block1Size, block2Size]() {
    NonLeafKeyValue node1 = readFromNonLeafNode(block1, block1Size);
    NonLeafKeyValue node2 = readFromNonLeafNode(block2, block2Size);

    return memcmp_diff_size(node1.key, node2.key, *node1.keyLength, *node2.keyLength);
  };

  return lambda();
}

BTreeNode::BTreeNode(BufferPageControl *bufferPageControl) : bufferPageControl_{bufferPageControl}, btreeNodeHeader_{reinterpret_cast<btree_node_header_t *>(bufferPageControl_->getSubHeaderLocation().ptr)}, mtx_{std::make_shared<std::mutex>()} {
  assert(bufferPageControl && "bufferpage control cannot be null");
}

std::pair<BTreeNode::Result, data_location_t> BTreeNode::insertNonLeaf(node_id_t childNode, const std::string &key) {
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

std::pair<BTreeNode::Result, data_location_t> BTreeNode::insertLeaf(const std::string &key, const std::string &value) {
  assert(isLeaf() && "cannot insert in non-leaf");

  auto size = sizeof(length_t) * 2 + key.size() + value.size();

  if (size > bufferPageControl_->getTotalFreeSpace()) {
    return {Result::NO_SPACE_LEFT, data_location_t{}};
  }

  auto optionalBlock = bufferPageControl_->getFreeBlock(size);
  assert(optionalBlock.has_value() && "free block not found");

  auto block = optionalBlock.value();

  writeLeafNode(key, value, block, size);

  return {Result::SUCCESS, data_location_t{}};
}

void BTreeNode::eraseAll() {
  auto dataDictionaryList = bufferPageControl_->getDataList();

  for (auto dataBlock: dataDictionaryList) {
    auto result = bufferPageControl_->releaseBlock(dataBlock.id);
    assert(result);
  }

  assert(bufferPageControl_->getBufferPage()->header.data_list_size == 0 && "datalist should be empty now");
}

bool BTreeNode::erase(data_location_t dataLocation) {
  auto dataDictionaryList = bufferPageControl_->getDataList();

  for (auto dataBlock: dataDictionaryList) {
    if (dataBlock == dataLocation) {
      auto result = bufferPageControl_->releaseBlock(dataBlock.id);
      assert(result);
      return true;
    }
  }

  return false;
}

std::string BTreeNode::getSmallestKey() const {
  auto dataList = bufferPageControl_->getDataList();
  assert(dataList.size() != 0 && "datalist cannot be empty");
  auto first = dataList.at(0);
  return getKey(first);
}

std::string BTreeNode::getLargest() const {
  auto dataList = bufferPageControl_->getDataList();
  auto first = dataList.at(dataList.size() - 1);
  return getKey(first);
}

std::string BTreeNode::getKey(data_location_t dataLocation) const {
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

data_location_t BTreeNode::search(std::string key) {
  auto dataList = bufferPageControl_->getDataListPtrs();
  assert(dataList.empty() == false && "datalist cannot be empty");

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
      if (node->btreeNodeHeader_->isLeaf) {
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

void BTreeNode::copyAllDataBlocks(BTreeNode *dest) {
  assert(dest && "dest cannot be null");
  auto dataDictionaryList = bufferPageControl_->getDataList();

  for (auto dataBlock: dataDictionaryList) {
    auto optionalBlock = bufferPageControl_->getBlock(dataBlock.id);
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
}

void BTreeNode::copyDataBlock(data_location_t srcDataLocation, BTreeNode *dest) {
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

std::int64_t BTreeNode::getUsableFreeSpace() {
  return bufferPageControl_->getUsableFreeSpace();
}

std::int64_t BTreeNode::getTotalFreeSpace() {
  return bufferPageControl_->getTotalFreeSpace();
}

void BTreeNode::sortDataDictionary(const cmp_func_t &compareFunction) {
  bufferPageControl_->sortDataList(compareFunction);
}

std::vector<data_location_t *> BTreeNode::getDataListPtrs() const {
  return bufferPageControl_->getDataListPtrs();
}

std::vector<data_location_t> BTreeNode::getDataList() const {
  return bufferPageControl_->getDataList();
}

bool BTreeNode::isLeaf() const {
  return btreeNodeHeader_->isLeaf == 1;
}

void BTreeNode::setLeaf() {
  btreeNodeHeader_->isLeaf = 1;
}

void BTreeNode::setNonLeaf() {
  btreeNodeHeader_->isLeaf = 0;
}

node_id_t BTreeNode::getId() const {
  return btreeNodeHeader_->id;
}

void BTreeNode::setId(node_id_t id) {
  btreeNodeHeader_->id = id;
}

void BTreeNode::lock() {
  mtx_->lock();

  auto nodeId = btreeNodeHeader_->id;
  std::cout << "threadId: " << std::this_thread::get_id() << " locked: " << nodeId << std::endl;
}

void BTreeNode::unlock() {
  auto nodeId = btreeNodeHeader_->id;

  mtx_->unlock();
  std::cout << "threadId: " << std::this_thread::get_id() << " unlocked: " << nodeId << std::endl;
}