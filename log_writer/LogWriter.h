//
// Created by Rahul  Kushwaha on 3/16/25.
//
#pragma once
#include <map>
#include <memory>
#include "DataStoreMetadata.h"
#include "block_store/include/DataStore.h"
#include "block_store/include/LogStore.h"

namespace bee_tree::log_writer {

class LogWriter {
public:
  explicit LogWriter();
  void appendLog(std::vector<block_store::LogEntry> logs);

private:
  block_store::lsn_t associateSegmentBacklink();

private:
  std::shared_ptr<MetadataManager> m_metadataManager;
  std::map<block_store::lsn_t, block_store::LogEntry> m_logs;
  std::shared_ptr<block_store::DataStore> m_dataStore;
};

}