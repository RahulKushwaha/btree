//
// Created by Rahul  Kushwaha on 3/16/25.
//

#pragma once
#include <cstdint>
#include "block_store/include/Common.h"

namespace bee_tree::common {

inline constexpr std::uint64_t BLOCK_SIZE = 4096;

inline constexpr std::uint64_t BLOCK_PER_ROW_BLOCK = 5;

inline constexpr std::uint64_t ROW_BLOCK_SIZE = BLOCK_PER_ROW_BLOCK * 4096;

inline constexpr std::uint64_t ROW_BLOCKS_PER_SEGMENT = 10;

inline constexpr std::uint64_t getSegmentId(std::uint64_t blockId) {
  return blockId / ROW_BLOCKS_PER_SEGMENT;
}

template <class T>
class CompletionHandler {
public:
  virtual void onError(const std::runtime_error& error) = 0;
  virtual void onComplete(T t) = 0;

  virtual ~CompletionHandler() = default;
};

struct BackLink {
  block_store::lsn_t m_prev;
  block_store::lsn_t m_lsn;

  bool operator<(const BackLink& other) const {
    if (m_lsn != other.m_lsn) {
      return m_lsn < other.m_lsn;
    }
    return m_lsn < other.m_lsn; // If x is equal, compare y
  }
};

using back_link_t = BackLink;

}