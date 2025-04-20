//
// Created by Rahul  Kushwaha on 3/9/25.
//

#pragma once

#include "block_store/include/LogMessages.h"
#include "common/RowBlock.h"

namespace bee_tree::block_store::commands {

class DeleteRecord {
public:
  void apply(DeleteCommand deleteCommand, common::RowBlock rowBlock);
};

}