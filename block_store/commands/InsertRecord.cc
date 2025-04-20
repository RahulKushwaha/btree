//
// Created by Rahul  Kushwaha on 3/9/25.
//

#include "InsertRecord.h"

#include <boost/assert.hpp>

namespace bee_tree::block_store::commands {

void InsertRecord::apply(InsertCommand insertCommand,
                         common::RowBlock rowBlock) {
  // Steps:
  // 1. Get a data block of appropriate size from the block.
  // 2. Write the key and value to the data block.
  // 3. Sort the data dictionary.

  // 1. Get a data block of appropriate size from the block.
  auto size = sizeof(common::key_length_t) +
              insertCommand.key.size() + sizeof(insertCommand.rowVersion) +
              sizeof(common::flag_length_t) + sizeof(common::value_length_t) +
              insertCommand.value.size();

  auto dataBlock = rowBlock.allocate(size);
  BOOST_ASSERT(dataBlock.has_value() && "data block must be allocated");

  auto dataBlockPtr = dataBlock.value();

  // 2. Write the tombstone to the data block.
  auto keySize = static_cast<common::key_length_t>(insertCommand.key.size());
  std::memcpy(dataBlockPtr, &keySize,
              sizeof(common::key_length_t));
  dataBlockPtr += sizeof(common::key_length_t);

  std::memcpy(dataBlockPtr, &insertCommand.key, insertCommand.key.size());
  dataBlockPtr += insertCommand.key.size();

  std::memcpy(dataBlockPtr, &insertCommand.rowVersion,
              sizeof(common::row_version_t));
  dataBlockPtr += sizeof(common::row_version_t);

  std::memcpy(dataBlockPtr, &common::K_NORMAL_VALUE_FLAG,
              sizeof(common::flag_length_t));
  dataBlockPtr += sizeof(common::flag_length_t);

  std::memcpy(dataBlockPtr, &insertCommand.value, insertCommand.value.size());
  dataBlockPtr += insertCommand.value.size();

  // 3. Sort the data dictionary.
  rowBlock.sortDataList();
}

}