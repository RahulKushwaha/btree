//
// Created by Rahul  Kushwaha on 5/17/24.
//

#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>

using node_id_t = std::uint64_t;

using cmp_func_t = std::function<bool(void *s1, void *s2, std::uint32_t size1, std::uint32_t size2)>;

inline bool memcmp_diff_size(const void *s1, const void *s2, std::uint32_t size1, std::uint32_t size2) {
  auto len = std::min(size1, size2);
  int result = memcmp(s1, s2, len);

  if (result != 0 || size1 == size2) {
    return result < 0;
  }

  return size1 < size2;
}