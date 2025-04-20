//
// Created by Rahul  Kushwaha on 3/17/25.
//

#include "DataStoreMetadata.h"

namespace bee_tree::log_writer {

MetadataManager::MetadataManager(DataStoreMetadata dataStoreMetadata,
                                 std::vector<SegmentMetadata> segments)
  :
  m_dataStoreMetadata{std::move(dataStoreMetadata)},
  m_segmentMetadata{
      [this, segments = std::move(segments)]() {
        std::unordered_map<block_store::segment_id_t, SegmentMetadata> data;
        for (auto& segment : segments) {
          data.emplace(segment.segmentId, segment);
        }

        return data;
      }()} {}

void MetadataManager::updateGlobalLatestLsn(block_store::lsn_t lsn,
                                            block_store::lsn_t prev) {
  m_dataStoreMetadata.latestLsn = lsn;
  m_dataStoreMetadata.backLinks.insert({prev, lsn});
}

void MetadataManager::updateGlobalConsistencyPoint() {
  auto& globalConsistencyPoint = m_dataStoreMetadata.consistencyPoint;
  for (auto backLink : m_dataStoreMetadata.backLinks) {
    if (backLink.m_prev == globalConsistencyPoint) {
      globalConsistencyPoint = backLink.m_lsn;
    } else {
      break;
    }
  }
}

}