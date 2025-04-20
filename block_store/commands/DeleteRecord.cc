//
// Created by Rahul  Kushwaha on 3/9/25.
//

#include "DeleteRecord.h"

#include <boost/assert.hpp>

namespace bee_tree::block_store::commands {
void DeleteRecord::apply(DeleteCommand deleteCommand,
                         common::RowBlock rowBlock) {
  // Steps:
  // 1. Get a data block of appropriate size from the block.
  // 2. Write the key tombstone to the data block.
  // 3. Sort the data dictionary.

  // 1. Get a data block of appropriate size from the block.
  auto size = sizeof(common::key_length_t) +
              deleteCommand.key.size() + sizeof(deleteCommand.rowVersion) +
              sizeof(common::flag_length_t);

  auto dataBlock = rowBlock.allocate(size);
  BOOST_ASSERT(dataBlock.has_value() && "data block must be allocated");

  auto dataBlockPtr = dataBlock.value();

  // 2. Write the tombstone to the data block.
  auto size = deleteCommand.key.size();
  std::memcpy(dataBlockPtr, &size, sizeof(common::key_length_t));
  dataBlockPtr += sizeof(common::key_length_t);

  std::memcpy(dataBlockPtr, &deleteCommand.key, deleteCommand.key.size());
  dataBlockPtr += deleteCommand.key.size();

  std::memcpy(dataBlockPtr, &deleteCommand.rowVersion,
              sizeof(common::row_version_t));
  dataBlockPtr += sizeof(common::row_version_t);

  std::memcpy(dataBlockPtr, &common::K_TOMBSTONE_FLAG,
              sizeof(common::flag_length_t));
  dataBlockPtr += sizeof(common::flag_length_t);

  // 3. Sort the data dictionary.
  rowBlock.sortDataList();
}
}