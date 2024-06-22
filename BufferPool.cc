//
// Created by Rahul  Kushwaha on 5/17/24.
//

#include "BufferPool.h"

node_id_t getNext() {
  static node_id_t id{NODE_ID_START};
  return (++id);
}

BufferPool::BufferPool(std::shared_ptr<FileIO> fileIO)
    : fileIO_{std::move(fileIO)}, mtx_{std::make_unique<std::mutex>()} {}

btree_node_ptr_t BufferPool::createNew(bool isLeaf) {
  std::lock_guard lg{*mtx_};

  auto page = std::aligned_alloc(PAGE_SIZE, PAGE_SIZE);
  assert(page && "empty memory returned from aligned alloc");
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

std::optional<btree_node_ptr_t> BufferPool::get(node_id_t id) {
  std::lock_guard lg{*mtx_};

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

void BufferPool::flushAll() {
  std::lock_guard lg{*mtx_};

  for (auto [nodeId, node]: lookup_) {
    auto offset = nodeId * PAGE_SIZE;
    fileIO_->fWrite(offset, node->bufferPageControl_->getBufferPage());
    fileIO_->sync();
  }
}

BufferPool::~BufferPool() {
  for (auto [nodeId, node]: lookup_) {
    free(node->bufferPageControl_->getBufferPage());
  }
}