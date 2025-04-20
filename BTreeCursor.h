//
// Created by Rahul  Kushwaha on 10/27/24.
//

#pragma once
#include "BTree.h"

#include <cassert>

class BTreeCursor {
  BTreeCursor(std::shared_ptr<BTree> bTree,
              std::shared_ptr<BufferPool> bufferPool)
      : bTree_{std::move(bTree)},
        bufferPool_{std::move(bufferPool)},
        node_{nullptr} {}

  void findPageForFetch(const std::string& key);

  BTreeNode* getNode() { return node_; }

 private:
  std::shared_ptr<BTree> bTree_;
  std::shared_ptr<BufferPool> bufferPool_;
  BTreeNode* node_;
};
