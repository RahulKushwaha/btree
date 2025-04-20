//
// Created by Rahul  Kushwaha on 3/9/25.
//

#include "TimeBasedBlockSplit.h"

namespace bee_tree::block_store::commands {

void TimeBasedBlockSplit::apply(SplitBlockCommand command,
                                common::RowBlock rowBlock) {
  // Steps:
  // 1. Determine the split point.
  // 2. Copy all the keys and values to the new block.
  // 3. Sort the data dictionary.
}

}