//
// Created by Rahul  Kushwaha on 10/27/24.
//

#include "BTreeCursor.h"

void BTreeCursor::findPageForFetch(const std::string& key) {
  std::cout << "findPageForFetch: " << key
            << " running on thread: " << std::this_thread::get_id()
            << std::endl;
  btree_node_ptr_t parent{nullptr};
  btree_node_ptr_t node{nullptr};

  node_id_t nodeId = bTree_->getRootId();
  while (nodeId != EMPTY_NODE_ID) {
    auto optionalNode = bufferPool_->get(nodeId);
    assert(optionalNode.has_value() && "node not found in buffer_pool");
    node = optionalNode.value();
    assert(node && "node cannot be null");
    node->rLock();

    if (parent) {
      parent->rUnlock();
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
  node_ = parent;
}
