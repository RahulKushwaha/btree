//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include "BlockStore.h"
#include "LogStore.h"
#include "RowBlockStore.h"
#include "common/RowBlock.h"

namespace bee_tree::block_store {

class Applicator {
public:
  virtual void apply(LogEntry logEntry, common::RowBlock rowBlock) =
  0;
  virtual ~Applicator() = default;
};

}