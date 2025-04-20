//
// Created by Rahul  Kushwaha on 3/16/25.
//

#pragma once
#include "block_store/include/LogMessages.h"

namespace bee_tree::block_store::commands {

  class AllocateRowBlock {
  void apply(AllocatedBlockCommand cmd);
};

}