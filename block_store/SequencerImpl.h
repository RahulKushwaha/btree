//
// Created by Rahul  Kushwaha on 3/3/25.
//

#pragma once
#include <atomic>
#include "include/Sequencer.h"

namespace bee_tree::block_store {
class SequencerImpl : public Sequencer {
public:
  explicit SequencerImpl(lsn_t start) : m_generator(start) {}

  lsn_result_t getNext(std::uint32_t gap) override;
  ~SequencerImpl() override;

private:
  std::atomic<lsn_t> m_generator;
};
}