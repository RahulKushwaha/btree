//
// Created by Rahul  Kushwaha on 3/2/25.
//
#pragma once
#include <cstdint>
#include <limits>

namespace bee_tree::block_store {

using lsn_t = std::uint64_t;
using row_block_id_t = std::uint64_t;

constexpr lsn_t lowest_unused_lsn_t = std::numeric_limits<lsn_t>::min();

}