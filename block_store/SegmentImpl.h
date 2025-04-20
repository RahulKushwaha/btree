//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include <map>
#include <memory>
#include <set>
#include "include/Segment.h"

namespace bee_tree::block_store {
class SegmentImpl final : public Segment {
public:
  SegmentImpl(lsn_t startLsn, std::shared_ptr<RowBlockStore> rowBlockStore,
              std::shared_ptr<LogStore> logStore);
  LogStore::Result append(std::vector<LogEntry> logs) override;
  std::vector<LogEntry> getLogs(lsn_t start, lsn_t endExcl) override;
  row_block_ptr_t getRowBlock(row_block_id_t rowBlockId, lsn_t lsn) override;
  void setSclCallbackHandler(
      std::shared_ptr<SclCompletionHandler> handler) override;
  ~SegmentImpl() override = default;

private:
  void triggerCallbacks();

private:
  lsn_t m_segmentConsistencyPoint;
  std::set<common::back_link_t> m_backLinks;
  std::shared_ptr<RowBlockStore> m_rowBlockStore;
  std::shared_ptr<LogStore> m_logStore;
  std::map<lsn_t, std::shared_ptr<SclCompletionHandler>> m_callbackHandlers;
};
}