//
// Created by Rahul  Kushwaha on 3/8/25.
//

#pragma once

#include <string>
#include <cstdint>
#include "Common.h"

namespace bee_tree::block_store {
enum class CommandType {
  INSERT
};

enum class SplitType {
  KEY_SPLIT,
  TIME_SPLIT,
};

struct InsertCommand {
  lsn_t lsn;
  std::string_view key;
  std::uint64_t rowVersion;
  std::string_view value;
};

struct DeleteCommand {
  lsn_t lsn;
  std::string_view key;
  std::uint64_t rowVersion;
};

struct AllocatedBlockCommand {
  lsn_t lsn;
  row_block_id_t blockId;
};

struct SplitBlockCommand {
  lsn_t lsn;
  SplitType splitType;
  row_block_id_t sourceBlockId;
  row_block_id_t targetBlockId;
};

struct InvalidCommand {};

}