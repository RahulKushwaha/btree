//
// Created by Rahul  Kushwaha on 3/2/25.
//

#include "InMemoryLogStore.h"

#include <boost/assert.hpp>

namespace bee_tree::block_store {

void InMemoryLogStore::setLowestLsn(lsn_t lsn) {
  m_lowestSegmentLsn = lsn;
}

LogStore::Result InMemoryLogStore::append(std::vector<LogEntry> logs) {
  for (auto& log : logs) {
    auto result = m_logs.emplace(log.lsn, log);
    BOOST_ASSERT(result.second);
  }

  return Result::Ok;
}

std::vector<LogEntry> InMemoryLogStore::getLogs(
    lsn_t start, lsn_t endExcl) {
  std::vector<LogEntry> result;
  for (auto itr = m_logs.lower_bound(start); itr != m_logs.lower_bound(endExcl);
       ++itr) {
    result.emplace_back(itr->second);
  }

  return result;
}

void InMemoryLogStore::trim(lsn_t lsn) {}

}