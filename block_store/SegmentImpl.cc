//
// Created by Rahul  Kushwaha on 3/2/25.
//

#include "SegmentImpl.h"

namespace bee_tree::block_store {

SegmentImpl::SegmentImpl(lsn_t startLsn,
                         std::shared_ptr<RowBlockStore> rowBlockStore,
                         std::shared_ptr<LogStore> logStore) :
  m_segmentConsistencyPoint{startLsn},
  m_rowBlockStore{std::move(rowBlockStore)},
  m_logStore{std::move(logStore)},
  m_callbackHandlers{} {}

row_block_ptr_t SegmentImpl::getRowBlock(row_block_id_t rowBlockId, lsn_t lsn) {
  return m_rowBlockStore->getRowBlock(rowBlockId);
}

LogStore::Result SegmentImpl::append(std::vector<LogEntry> logs) {
  auto result = m_logStore->append(std::move(logs));

  for (const auto& log : logs) {
    common::back_link_t backLink{
        .m_lsn = log.lsn,
        .m_prev = log.segmentPrev,
    };

    m_backLinks.emplace(backLink);
  }

  triggerCallbacks();

  return result;
}

void SegmentImpl::triggerCallbacks() {
  for (const auto& backLink : m_backLinks) {
    if (backLink.m_prev == m_segmentConsistencyPoint) {
      m_segmentConsistencyPoint = backLink.m_lsn;
    } else {
      break;
    }
  }

  auto itr = m_callbackHandlers.begin();
  while (itr != m_callbackHandlers.end()) {
    if (itr->first <= m_segmentConsistencyPoint) {
      itr->second->onComplete(m_segmentConsistencyPoint);
      ++itr;
    } else {
      break;
    }
  }

  m_callbackHandlers.erase(m_callbackHandlers.begin(), itr);
}

void SegmentImpl::setSclCallbackHandler(
    std::shared_ptr<SclCompletionHandler> handler) {
  m_callbackHandlers.emplace(
      handler->getSclLsn(), std::move(handler));
}


}