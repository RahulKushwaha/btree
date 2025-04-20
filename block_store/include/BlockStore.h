//
// Created by Rahul  Kushwaha on 3/2/25.
//
#pragma once

#include <cstdint>
#include <functional>
#include "Common.h"

namespace bee_tree::block_store {
using block_id_t = std::uint64_t;

struct block {
  std::byte* data;
  std::size_t size;
  block_id_t block_id;
};

using block_t = block;
using block_ptr_t = block*;

class BlockStore {
public:
  virtual block_t getBlock(block_id_t blockId, lsn_t lsn) =
  0;
  virtual block_id_t allocateNew(std::uint32_t numberOfBlocks) = 0;
  virtual void free(block_id_t) = 0;

  virtual ~BlockStore() = default;
};
}