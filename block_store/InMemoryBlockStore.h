//
// Created by Rahul  Kushwaha on 3/16/25.
//
#pragma once

#include <list>
#include "include/BlockStore.h"

namespace bee_tree::block_store {

class InMemoryBlockStore : BlockStore {
public:
  explicit InMemoryBlockStore(std::size_t blockSize)
    : m_blockSize{blockSize},
      maxNumberOfBlocks{100},
      m_freeBlockIds{[this]() {
        std::list<block_id_t> freeBlockIds;
        for (int i = 0; i < maxNumberOfBlocks; ++i) {
          freeBlockIds.push_back(i);
        }
        return freeBlockIds;
      }()} {}

  block_id_t allocateNew(std::uint32_t numberOfBlocks) override;
  void free(block_id_t) override;
  ~InMemoryBlockStore() override;

private:
  std::size_t m_blockSize;
  std::int32_t maxNumberOfBlocks;
  std::list<block_id_t> m_freeBlockIds;
  std::unordered_map<block_id_t, block_t> m_blocks;
};

}