//
// Created by Rahul  Kushwaha on 3/3/25.
//

#include "SequencerImpl.h"

namespace bee_tree::block_store {

lsn_result_t SequencerImpl::getNext(std::uint32_t gap) {
  lsn_t prev = m_generator.fetch_add(gap, std::memory_order_relaxed);
  return {prev, prev + gap};
}

SequencerImpl::~SequencerImpl() = default;

}