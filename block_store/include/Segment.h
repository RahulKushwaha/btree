//
// Created by Rahul  Kushwaha on 3/2/25.
//
#pragma once
#include <limits>
#include <memory>
#include <stdexcept>
#include "BlockStore.h"
#include "LogStore.h"
#include "Common.h"
#include "RowBlockStore.h"
#include "common/Common.h"

namespace bee_tree::block_store {

using segment_id_t = std::uint64_t;
static constexpr segment_id_t unknown_segment_id = std::numeric_limits<
  segment_id_t>::max();

class SclCompletionHandler : public common::CompletionHandler<lsn_t> {
public:
  explicit SclCompletionHandler(lsn_t lsn): m_lsn{lsn} {}

  void onError(const std::runtime_error& error) override {}
  void onComplete(lsn_t lsn) override {}

  lsn_t getSclLsn() const {
    return m_lsn;
  }

private:
  lsn_t m_lsn;
};

class Segment {
public:
  virtual LogStore::Result append(std::vector<LogEntry> logs) = 0;
  virtual std::vector<LogEntry> getLogs(lsn_t start, lsn_t endExcl) =
  0;
  virtual row_block_ptr_t getRowBlock(row_block_id_t rowBlockId, lsn_t lsn) = 0;

  virtual void setSclCallbackHandler(
      std::shared_ptr<SclCompletionHandler> handler) = 0;

  virtual ~Segment() = default;
};

}