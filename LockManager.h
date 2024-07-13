//
// Created by Rahul  Kushwaha on 6/25/24.
//

#pragma once
#include "Common.h"
#include "Transaction.h"

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
  void lock(node_id_t nodeId, Transaction* txn, LockMode lockMode) {
    bool suspend{false};
    std::shared_ptr<LockRequest> txnLockRequest =
        std::make_shared<LockRequest>(LockRequest{
            .txn = txn,
            .nodeId = nodeId,
            .mode = lockMode,
        });

    {
      std::lock_guard lg{*mtx_};

      bool addToWaitingGroup{false};
      LockQueue* lockQueue;
      if (auto itr = lockQueue_.find(nodeId); itr != lockQueue_.end()) {
        auto& queue = itr->second;

        if (!queue.waitingGroup.empty()) {
          addToWaitingGroup = true;
          lockQueue = &queue;
        } else {
          for (auto& lockRequest : queue.grantedGroup) {
            if (areLocksConflicting(lockRequest->nodeId, lockRequest->mode,
                                    nodeId, lockMode)) {
              addToWaitingGroup = true;
              lockQueue = &queue;
              break;
            }
          }

          if (!addToWaitingGroup) {
            txnLockRequest->granted = true;
            queue.grantedGroup.emplace_back(txnLockRequest);
          }
        }
      }

      if (addToWaitingGroup) {
        lockQueue->waitingGroup.emplace_back(txnLockRequest);
        suspend = true;
      }
    }

    if (suspend) {
      std::unique_lock uniqueLock{*txn->mtx_};
      txn->condVar_->wait(
          uniqueLock, [txnLockRequest]() { return txnLockRequest->granted; });
    }
  }

  void unlock(node_id_t nodeId, Transaction* transaction) {
    // Transactions to wake up.
    std::vector<std::shared_ptr<LockRequest>> wakeUp{};

    {
      std::lock_guard lg{*mtx_};
      auto itr = lockQueue_.find(nodeId);
      assert(itr != lockQueue_.end());
      auto& queue = itr->second;

      // Nobody is waiting.
      if (queue.waitingGroup.empty()) {
        // nothing to do
        return;
      }

      // Iterate through the waiting queue to wake up waiting transactions.
      for (const auto& lockRequest : queue.waitingGroup) {
        bool grantLock{true};
        for (const auto& grantedLockRequest : queue.grantedGroup) {
          if (areLocksConflicting(lockRequest->nodeId, lockRequest->mode,
                                  grantedLockRequest->nodeId,
                                  grantedLockRequest->mode)) {
            grantLock = false;
            break;
          }
        }

        for (const auto& toWakeUp : wakeUp) {
          if (areLocksConflicting(lockRequest->nodeId, lockRequest->mode,
                                  toWakeUp->nodeId, toWakeUp->mode)) {
            grantLock = false;
            break;
          }
        }

        if (grantLock) {
          wakeUp.emplace_back(lockRequest);
        } else {
          break;
        }
      }

      for (auto& lockRequest : wakeUp) {
        lockRequest->granted = true;
        queue.grantedGroup.emplace_back(lockRequest);
        queue.waitingGroup.erase(queue.waitingGroup.begin());
      }
    }

    for (auto& lockRequest : wakeUp) {
      std::unique_lock uniqueLock{*lockRequest->txn->mtx_};
      lockRequest->txn->condVar_->notify_one();
    }
  }

  bool areLocksConflicting(node_id_t nodeId1, LockMode mode1, node_id_t nodeId2,
                           LockMode mode2) {
    if (nodeId1 != nodeId2) {
      return false;
    }

    auto lock1 = std::to_underlying(mode1);
    auto lock2 = std::to_underlying(mode2);

    return lockMatrix[lock1][lock2];
  }

 private:
  std::unique_ptr<std::mutex> mtx_;
  std::unordered_map<node_id_t, LockQueue> lockQueue_;
};
