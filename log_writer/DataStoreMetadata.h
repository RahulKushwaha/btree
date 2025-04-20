//
// Created by Rahul  Kushwaha on 3/17/25.
//
#pragma once
#include <set>
#include <unordered_map>
#include <utility>
#include "block_store/include/Common.h"
#include "block_store/include/Segment.h"

namespace bee_tree::log_writer {

struct SegmentMetadata {
  block_store::segment_id_t segmentId;
  block_store::lsn_t consistencyPoint;
  block_store::lsn_t latestLsn;
  std::set<common::back_link_t> backLinks;
};

struct DataStoreMetadata {
  block_store::lsn_t consistencyPoint;
  block_store::lsn_t latestLsn;
  std::set<common::back_link_t> backLinks;
};

class MetadataManager {
public:
  explicit MetadataManager(DataStoreMetadata dataStoreMetadata,
                           std::vector<SegmentMetadata> segments);

  void updateGlobalLatestLsn(block_store::lsn_t lsn,
                             block_store::lsn_t prev);

  void updateSegmentLatestLsn(block_store::segment_id_t segmentId,
                              block_store::lsn_t lsn,
                              block_store::lsn_t prev) {
    m_segmentMetadata[segmentId].latestLsn = lsn;
    m_segmentMetadata[segmentId].backLinks.insert({prev, lsn});
  }

  block_store::lsn_t getGlobalLatestLsn() const {
    return m_dataStoreMetadata.latestLsn;
  }

  block_store::lsn_t getSegmentLatestLsn(
      block_store::segment_id_t segmentId) const {
    auto it = m_segmentMetadata.find(segmentId);
    if (it == m_segmentMetadata.end()) {
      return 0;
    }

    return it->second.latestLsn;
  }

  block_store::lsn_t getSegmentConsistencyPoint(
      block_store::segment_id_t segmentId) const {
    auto itr = m_segmentMetadata.find(segmentId);
    if (itr == m_segmentMetadata.end()) {
      return 0;
    }

    return itr->second.consistencyPoint;
  }

  block_store::lsn_t getGlobalConsistencyPoint() const {
    return m_dataStoreMetadata.consistencyPoint;
  }

  void updateGlobalConsistencyPoint();

private:
  DataStoreMetadata m_dataStoreMetadata;
  std::unordered_map<block_store::segment_id_t, SegmentMetadata>
  m_segmentMetadata;
};

}