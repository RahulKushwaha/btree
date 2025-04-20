//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include <map>
#include "include/LogStore.h"

namespace bee_tree::block_store {

class InMemoryLogStore : public LogStore {
public:
  InMemoryLogStore() : m_lowestSegmentLsn{lowest_unused_lsn_t} {}

  void setLowestLsn(lsn_t lsn) override;
  Result append(std::vector<LogEntry> logs) override;
  std::vector<LogEntry> getLogs(lsn_t start, lsn_t endExcl) override;
  void trim(lsn_t lsn) override;
  ~InMemoryLogStore() override = default;

private:
  std::map<lsn_t, LogEntry> m_logs;
  lsn_t m_lowestSegmentLsn;
};


}