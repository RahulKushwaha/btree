//
// Created by Rahul  Kushwaha on 6/25/24.
//

#pragma once
#include "Common.h"

#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

struct LockRequest {
  Transaction* txn;
  node_id_t nodeId;
  LockMode mode;
  bool granted;
};

struct LockQueue {
  std::vector<std::shared_ptr<LockRequest>> grantedGroup;
  std::vector<std::shared_ptr<LockRequest>> waitingGroup;
};

bool lockMatrix[4][4] = {
    /*         IX    IS    S      X */
    /* IX */ {true, true, false, false},
    /* IS */ {true, true, true, false},
    /*  S */ {false, true, true, false},
    /*  X */ {false, false, false, false},
};

class LockManager {
 public:
  explicit LockManager();

  void lock(node_id_t nodeId, Transaction* txn, LockMode lockMode);
  void unlock(node_id_t nodeId, Transaction* transaction);
  bool areLocksConflicting(node_id_t nodeId1, LockMode mode1, node_id_t nodeId2,
                           LockMode mode2);

 private:
  std::unique_ptr<std::mutex> mtx_;
  std::unordered_map<node_id_t, LockQueue> lockQueue_;
};
