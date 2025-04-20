//
// Created by Rahul  Kushwaha on 3/16/25.
//

#include "LogWriter.h"

#include <boost/assert.hpp>

#include "common/Common.h"

namespace bee_tree::log_writer {

LogWriter::LogWriter() : m_metadataManager{}, m_logs{}, m_dataStore{} {}

void LogWriter::appendLog(std::vector<block_store::LogEntry> logs) {
  // Update global consistency point.
  for (auto& log : logs) {
    auto emplace = m_logs.emplace(log.lsn, log);
    BOOST_ASSERT(emplace.second);
    m_metadataManager->updateGlobalLatestLsn(log.lsn, log.globalPrev);
  }

  m_metadataManager->updateGlobalConsistencyPoint();

  // Associated Segment backlinks.
  auto globalConsistencyPoint = associateSegmentBacklink();

  // Write the logs to the data store based on segment.
  for (auto& [lsn, log] : m_logs) {
    if (lsn <= globalConsistencyPoint) {
      auto segmentId = common::getSegmentId(log.lsn);
      auto segment = m_dataStore->getSegment(segmentId);
      segment->append({log});
    } else {
      break;
    }
  }
}

block_store::lsn_t LogWriter::associateSegmentBacklink() {
  auto globalConsistencyPoint = m_metadataManager->getGlobalConsistencyPoint();
  for (auto& [lsn, log] : m_logs) {
    if (lsn <= globalConsistencyPoint) {
      auto segmentId = common::getSegmentId(log.lsn);
      auto segmentConsistencyPoint = m_metadataManager->
          getSegmentConsistencyPoint(segmentId);

      log.segmentPrev = segmentConsistencyPoint;

      m_metadataManager->updateSegmentLatestLsn(segmentId, log.lsn,
                                                log.segmentPrev);
    } else {
      break;
    }
  }

  return globalConsistencyPoint;
}

}