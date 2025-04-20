//
// Created by Rahul  Kushwaha on 3/2/25.
//

#pragma once
#include <functional>
#include "Common.h"

namespace bee_tree::block_store {

struct LogEntry {
  std::byte* data;
  size_t size;
  lsn_t lsn;
  lsn_t globalPrev;
  lsn_t segmentPrev;
  row_block_id_t blockId;
};

using log_entry_t = LogEntry;

class LogStore {
public:
  enum class Result {
    Ok,
    Failed_SizeExceeded,
    Failed_Unknown,
  };

  virtual void setLowestLsn(lsn_t lsn) = 0;
  virtual Result append(std::vector<log_entry_t> logs) = 0;
  virtual std::vector<log_entry_t> getLogs(lsn_t start, lsn_t endExcl) = 0;
  virtual void trim(lsn_t lsn) = 0;

  virtual ~LogStore() = default;
};
}