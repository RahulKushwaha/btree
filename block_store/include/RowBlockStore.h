//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include "BlockStore.h"
#include "Common.h"

namespace bee_tree::block_store {

static constexpr std::uint32_t kBlocksCount = 5;
using row_block_t = block_t[kBlocksCount];
using row_block_ptr_t = row_block_t*;

class RowBlockStore {
public:
  virtual row_block_ptr_t getRowBlock(row_block_id_t rowBlockId) = 0;
  virtual row_block_id_t allocate(row_block_id_t rowBlockId) = 0;
  virtual void free(row_block_id_t) = 0;
  virtual ~RowBlockStore() = default;
};
}