//
// Created by Rahul  Kushwaha on 5/17/24.
//

#include "BufferPool.h"

std::string convertToString(char *ptr, length_t length) {
  std::stringstream ss;
  for (int i = 0; i < length; i++) {
    ss << ptr[i];
  }

  return ss.str();
}

node_id_t getNext() {
  static node_id_t id{NODE_ID_START};
  return (++id);
}

void writeNonLeafNode(node_id_t childNodeId, const std::string &key, void *destBlock, length_t blockSize) {
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
  char *baseAddr = reinterpret_cast<char *>(block);
  NonLeafKeyValue nonLeafKeyValue{
      .childNodeId = reinterpret_cast<node_id_t *>(baseAddr),
      .keyLength = reinterpret_cast<length_t *>(baseAddr + sizeof(node_id_t)),
      .key = reinterpret_cast<char *>(baseAddr + sizeof(node_id_t) + sizeof(length_t)),
  };

  return nonLeafKeyValue;
}

LeafKeyValue readFromLeafNode(void *block, length_t blockSize) {
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