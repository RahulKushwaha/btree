//
// Created by Rahul  Kushwaha on 5/17/24.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <variant>

enum class LockMode {
  IX = 0,
  IS = 1,
  S = 2,
  X = 3,
};

constexpr std::uint64_t PAGE_SIZE = 4096;

using node_id_t = std::uint64_t;
using txn_id_t = std::uint64_t;
using length_t = std::uint16_t;

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

  std::unique_ptr<std::mutex> mtx;
  std::unique_ptr<std::condition_variable> condVar;
};

using cmp_func_t = std::function<bool(void* s1, void* s2, std::uint32_t size1,
                                      std::uint32_t size2)>;

inline bool memcmp_diff_size(const void* s1, const void* s2,
                             std::uint32_t size1, std::uint32_t size2) {
  auto len = std::min(size1, size2);
  int result = memcmp(s1, s2, len);

  if (result != 0 || size1 == size2) {
    return result < 0;
  }

  return size1 < size2;
}