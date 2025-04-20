//
// Created by Rahul  Kushwaha on 3/9/25.
//

#pragma once
#include <cstdint>
#include "Common.h"
#include "Page.h"

namespace bee_tree::common {
/**
 * FLAGS
 * 0x0000000000000001 - Tombstone
 */
inline constexpr std::uint32_t K_TOMBSTONE_FLAG = 0x0000000000000001;

/**
 * FLAGS
 * 0x0000000000000010 - Value
 */
inline constexpr std::uint32_t K_NORMAL_VALUE_FLAG = 0x0000000000000010;

/**
 * TOMBSTONE FORMAT:
 * <key_length_t> <key> <rowVersion> <flags>
 */


using key_length_t = std::uint32_t;
using value_length_t = std::uint32_t;
using flag_length_t = std::uint32_t;
using row_version_t = std::uint64_t;

struct RowKey {
  std::string_view key;
  std::uint64_t rowVersion;
};

struct RowValue {
  std::string_view value;
};

using key_t = RowKey;
using value_t = RowValue;

class RowBlock;
using row_block_t = RowBlock;
using row_block_ptr_t = row_block_t*;

class RowBlock {
public:
  explicit RowBlock(raw_page_ptr_t rawPage, cmp_func_t compareFunction);

  void eraseAll();

  RowKey getSmallestKey() const;
  RowKey getLargestKey() const;
  RowKey getKey(data_location_t dataLocation) const;
  std::optional<void*> allocate(std::uint32_t size);
  bool erase(data_location_t dataLocation);
  void copyDataBlock(data_location_t srcDataLocation,
                     row_block_ptr_t dest) const;

  std::int64_t getUsableFreeSpace() const;
  std::int64_t getTotalFreeSpace() const;

  void sortDataList();

private:
  page_t m_page;
  cmp_func_t m_compareFunction;
};

}