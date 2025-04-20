//
// Created by Rahul  Kushwaha on 3/18/25.
//

#include "PageReader.h"

namespace bee_tree::page_reader {

block_store::row_block_ptr_t PageReader::read(block_store::lsn_t lsn,
                                              block_store::row_block_id_t
                                              rowBlockId) {
  auto segmentId = common::getSegmentId(rowBlockId);
  auto segment = m_dataStore->getSegment(segmentId);
  auto rowBlock = segment->getRowBlock(rowBlockId, lsn);

  return rowBlock;
}

}