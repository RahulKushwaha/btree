//
// Created by Rahul  Kushwaha on 3/8/25.
//

#pragma once
#include <memory>
#include "include/Applicator.h"
#include "include/RowBlockStore.h"

namespace bee_tree::block_store {
class PageWriter : public Applicator {
public:
  void apply(LogEntry logEntry, common::RowBlock rowBlock) override;
  ~PageWriter() override;
};
}