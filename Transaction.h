//
// Created by Rahul  Kushwaha on 6/25/24.
//
#pragma once
#include "Common.h"

#include <condition_variable>
#include <memory>

struct Resource {
  node_id_t nodeId;
  LockMode mode;
};

struct Transaction {
  txn_id_t txnId;

  std::vector<Resource> waitingFor;
  std::vector<Resource> granted;

  std::unique_ptr<std::mutex> mtx_;
  std::unique_ptr<std::condition_variable> condVar_;
};