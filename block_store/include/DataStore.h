//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include <memory>
#include "Segment.h"

namespace bee_tree::block_store {

class DataStore {
public:
  virtual segment_id_t allocateSegment() = 0;
  virtual std::shared_ptr<Segment> getSegment(segment_id_t) = 0;

  virtual ~DataStore() = default;
};

}