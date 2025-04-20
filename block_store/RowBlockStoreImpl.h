//
// Created by Rahul  Kushwaha on 3/2/25.
//
#pragma once

#include <memory>
#include "common/RowBlock.h"
#include "include/RowBlockStore.h"

namespace bee_tree::block_store {
class RowBlockStoreImpl : public RowBlockStore {
public:
  row_block_ptr_t getRowBlock(row_block_id_t) override;
  row_block_id_t allocate(row_block_id_t rowBlockId) override;
  void free(row_block_id_t rowBlockId) override;
  ~RowBlockStoreImpl() override = default;

private:
  std::shared_ptr<BlockStore> m_blockStore;
  std::unordered_map<row_block_id_t, block_id_t> m_blockIdLookup;
};
}