//
// Created by Rahul  Kushwaha on 3/16/25.
//

#include "InMemoryBlockStore.h"

#include <stdexcept>

namespace bee_tree::block_store {

block_id_t InMemoryBlockStore::allocateNew(std::uint32_t numberOfBlocks) {
  if (m_freeBlockIds.empty()) {
    throw std::runtime_error{"no free blocks available"};
  }
  auto blockId = m_freeBlockIds.front();
  m_freeBlockIds.pop_front();

  m_blocks[blockId] = block_t{new std::byte[m_blockSize * numberOfBlocks],
                              m_blockSize * numberOfBlocks, blockId};
  return blockId;
}

void InMemoryBlockStore::free(block_id_t blockId) {
  auto it = m_blocks.find(blockId);
  if (it != m_blocks.end()) {
    delete[] it->second.data;
    m_blocks.erase(it);
    m_freeBlockIds.push_back(blockId);
  }
}

InMemoryBlockStore::~InMemoryBlockStore() = default;


}