//
// Created by Rahul  Kushwaha on 6/21/24.
//

#include "BTree.h"

auto getCompareFunction(bool isLeaf) {
  auto compareFunction = [leaf = isLeaf](void* block1, void* block2,
                                         length_t block1Size,
                                         length_t block2Size) {
    if (leaf) {
      auto node1 = readFromLeafNode(block1, block1Size);
      auto node2 = readFromLeafNode(block2, block2Size);
      return memcmp_diff_size(node1.key, node2.key, *node1.keyLength,
                              *node2.keyLength);
    }

    auto node1 = readFromNonLeafNode(block1, block1Size);
    auto node2 = readFromNonLeafNode(block2, block2Size);
    return memcmp_diff_size(node1.key, node2.key, *node1.keyLength,
                            *node2.keyLength);
  };

  return compareFunction;
}

BTree::BTree(std::shared_ptr<BufferPool> bufferPool)
    : bufferPool_{std::move(bufferPool)}, root_{bufferPool_->createNew(true)} {}

BTree::BTree(std::shared_ptr<BufferPool> bufferPool, node_id_t rootId)
    : bufferPool_{std::move(bufferPool)},
      root_{bufferPool_->get(rootId).value_or(nullptr)} {
  assert(root_ != nullptr);
  assert(rootId == root_->getId());
}

node_id_t BTree::getRootId() {
  return root_->getId();
}

bool BTree::insert(std::string key, std::string value) {
  auto node = findNodeForInsert(std::string_view{key});

  if (!node->getDataList().empty()) {
    auto location = node->search(key);
    auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
    assert(optionalBlock.has_value());

    assert(node->isLeaf() &&
           "last step of insertion should always land on leaf node");
    auto block = optionalBlock.value();
    auto leafKeyValue = readFromLeafNode(block.ptr, block.length);

    if (leafKeyValue.getKeyStr() == key) {
      node->unlock();
      return false;
    }
  }

  // The leaf does not contain the same key. We can proceed with insertion.
  auto result = node->insertLeaf(key, value);
  assert(result.first == BTreeNode::Result::SUCCESS);

  node->bufferPageControl_->sortDataList(getCompareFunction(node->isLeaf()));
  node->unlock();

  return true;
}

std::optional<std::string> BTree::search(std::string key) {
  auto node = findNodeForSelect(std::string_view{key});

  if (!node->getDataList().empty()) {
    auto location = node->search(key);
    auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
    assert(optionalBlock.has_value());

    assert(node->isLeaf() &&
           "last step of insertion should always land on leaf node");
    auto block = optionalBlock.value();
    auto leafKeyValue = readFromLeafNode(block.ptr, block.length);

    if (leafKeyValue.getKeyStr() != key) {
      node->unlock();
      return {};
    }

    node->unlock();
    return leafKeyValue.getValueStr();
  }

  return {};
}

bool BTree::del(std::string_view key) {
  auto node = findNodeForInsert(key);

  if (!node->getDataList().empty()) {
    auto location = node->search(std::string{key});
    auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
    assert(optionalBlock.has_value());

    assert(node->isLeaf() &&
           "last step of insertion should always land on leaf node");
    auto block = optionalBlock.value();
    auto leafKeyValue = readFromLeafNode(block.ptr, block.length);

    if (leafKeyValue.getKeyStr() == key) {
      auto releaseResult = node->bufferPageControl_->releaseBlock(location.id);
      assert(releaseResult && "release block should be success");
      node->unlock();
      return true;
    }

    node->unlock();
    return false;
  }

  return false;
}

std::vector<BTree::KeyValue> BTree::scan(ScanOperation scanOperation) {
  return {};
}

std::vector<std::string> BTree::elements() {
  std::vector<std::string> result;
  elements(root_, result);

  return result;
}

void BTree::debug_print() {
  debugPrint(root_);
  std::cout << std::endl;
}

btree_node_ptr_t BTree::findNodeForInsert(std::string_view key) {
  std::cout << "insert: " << key
            << " running on thread: " << std::this_thread::get_id()
            << std::endl;
  btree_node_ptr_t parent{nullptr};
  btree_node_ptr_t node{nullptr};

  node_id_t nodeId = root_->getId();
  while (nodeId != EMPTY_NODE_ID) {
    auto optionalNode = bufferPool_->get(nodeId);
    assert(optionalNode.has_value() && "node not found in buffer_pool");
    node = optionalNode.value();
    assert(node && "node cannot be null");
    node->lock();

    if (node->getDataList().size() > 5) {
      if (node == root_) {
        auto newNode = bufferPool_->createNew(node->isLeaf());
        node->copyAllDataBlocks(newNode);
        node->eraseAll();
        node->setNonLeaf();

        // No need to lock and unlock the new node as it is not
        // visible to anybody.
        //          newNode->lock();
        split(newNode, node, true);
        //          newNode->unlock();
      } else {
        assert(parent && "parent node cannot be null in non-root split");

        split(node, parent, false);

        node->unlock();

        auto location = parent->search(std::string{key});
        auto optionalBlock = parent->bufferPageControl_->getBlock(location.id);
        assert(optionalBlock.has_value());
        auto block = optionalBlock.value();
        assert(!parent->isLeaf());
        auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
        nodeId = *nonLeafKeyValue.childNodeId;

        auto optionalChildNode = bufferPool_->get(nodeId);
        assert(optionalChildNode.has_value() &&
               "node not found in buffer_pool");
        node = optionalChildNode.value();

        node->lock();
      }
    }

    if (parent) {
      parent->unlock();
    }

    if (node->isLeaf()) {
      nodeId = EMPTY_NODE_ID;
    } else {
      auto location = node->search(std::string{key});
      auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();
      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
      nodeId = *nonLeafKeyValue.childNodeId;
    }

    parent = node;
  }

  assert(parent && "parent cannot be null");

  return parent;
}

btree_node_ptr_t BTree::findNodeForSelect(std::string_view key) {
  std::cout << "insert: " << key
            << " running on thread: " << std::this_thread::get_id()
            << std::endl;
  btree_node_ptr_t parent{nullptr};
  btree_node_ptr_t node{nullptr};

  node_id_t nodeId = root_->getId();
  while (nodeId != EMPTY_NODE_ID) {
    auto optionalNode = bufferPool_->get(nodeId);
    assert(optionalNode.has_value() && "node not found in buffer_pool");
    node = optionalNode.value();
    assert(node && "node cannot be null");
    node->lock();

    if (parent) {
      parent->unlock();
    }

    if (node->isLeaf()) {
      nodeId = EMPTY_NODE_ID;
    } else {
      auto location = node->search(std::string{key});
      auto optionalBlock = node->bufferPageControl_->getBlock(location.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();
      auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
      nodeId = *nonLeafKeyValue.childNodeId;
    }

    parent = node;
  }

  assert(parent && "parent cannot be null");

  return parent;
}

void BTree::split(btree_node_ptr_t node, btree_node_ptr_t parent,
                  bool rootSplit) {
  assert(node != nullptr && "node cannot be null");
  assert(parent != nullptr && "parent cannot be null");

  auto newNode = bufferPool_->createNew(node->isLeaf());
  assert(newNode && "newNode cannot be null");

  auto dataLocations = node->getDataList();
  auto splitLimit = dataLocations.size() / 2;
  auto count{0};
  for (auto& dataLocation : dataLocations) {
    count++;

    if (count >= splitLimit) {
      node->copyDataBlock(dataLocation, newNode);
      bool result = node->erase(dataLocation);
      assert(result && "erasing block should be successful");
    }
  }

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

void BTree::debugPrint(btree_node_ptr_t node) const {
  if (node == nullptr) {
    return;
  }

  if (node->isLeaf()) {
    std::cout << "Leaf, ";
    std::cout << "node id: " << node->getId() << std::endl;
    for (const auto& c : node->getDataList()) {
      auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();
      auto leafKeyValue = readFromLeafNode(block.ptr, block.length);
      std::cout << "key: " << leafKeyValue.getKeyStr()
                << ", value: " << leafKeyValue.getValueStr() << " | ";
    }

    std::cout << std::endl << std::endl;
    return;
  }

  if (node == root_) {
    std::cout << "Root, ";
  }
  std::cout << "Non Leaf, ";
  std::cout << "node id: " << node->getId() << std::endl;
  for (const auto& c : node->getDataList()) {
    auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
    assert(optionalBlock.has_value());
    auto block = optionalBlock.value();
    auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);

    assert(*nonLeafKeyValue.childNodeId != EMPTY_NODE_ID &&
           "non leaf cannot have nullptr as child");
    std::cout << "key: " << nonLeafKeyValue.getKeyStr()
              << ", child: " << *nonLeafKeyValue.childNodeId << " | ";
  }

  std::cout << std::endl << std::endl;

  for (const auto& c : node->getDataList()) {
    auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
    assert(optionalBlock.has_value());
    auto block = optionalBlock.value();
    auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);

    auto page = bufferPool_->get(*nonLeafKeyValue.childNodeId);
    debugPrint(page.value_or(nullptr));
  }
}

void BTree::elements(btree_node_ptr_t node,
                     std::vector<std::string>& keys) const {
  if (node == nullptr) {
    return;
  }

  if (node->isLeaf()) {
    for (const auto& c : node->getDataList()) {
      auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
      assert(optionalBlock.has_value());
      auto block = optionalBlock.value();

      auto leafKeyValue = readFromLeafNode(block.ptr, block.length);
      keys.push_back(leafKeyValue.getKeyStr());
    }

    return;
  }

  for (const auto& c : node->getDataList()) {
    auto optionalBlock = node->bufferPageControl_->getBlock(c.id);
    assert(optionalBlock.has_value());
    auto block = optionalBlock.value();

    auto nonLeafKeyValue = readFromNonLeafNode(block.ptr, block.length);
    auto childNodeId = *nonLeafKeyValue.childNodeId;
    auto page = bufferPool_->get(childNodeId);

    assert(page.has_value() &&
           "every non-leaf node should always have a valid value for child");
    elements(page.value(), keys);
  }
}