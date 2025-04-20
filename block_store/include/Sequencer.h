//
// Created by Rahul  Kushwaha on 3/3/25.
//

#pragma once
#include <utility>
#include "Common.h"

namespace bee_tree::block_store {
struct LsnResult {
  lsn_t prev;
  lsn_t current;
};

using lsn_result_t = LsnResult;

class Sequencer {
public:
  virtual lsn_result_t getNext(std::uint32_t gap) = 0;
  virtual ~Sequencer() = default;
};
}