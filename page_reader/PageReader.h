//
// Created by Rahul  Kushwaha on 3/18/25.
//
#pragma once
#include <memory>
#include "block_store/include/DataStore.h"
#include "common/RowBlock.h"

namespace bee_tree::page_reader {


class PageReader {
public:
  block_store::row_block_ptr_t read(block_store::lsn_t lsn,
                                    block_store::row_block_id_t rowBlockId);

private:
  std::shared_ptr<block_store::DataStore> m_dataStore;
};


}