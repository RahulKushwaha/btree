//
// Created by Rahul  Kushwaha on 5/19/24.
//

#pragma once
#include "Common.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <vector>

constexpr std::uint64_t HEADER_SIZE = 64;

struct BufferPageHeader {
  std::uint32_t data_list_size;
  std::uint32_t padding[15];
};

struct BufferPage {
  BufferPageHeader header;
  char data[PAGE_SIZE - HEADER_SIZE];
};

using data_location_id_t = std::uint32_t;

struct DataLocation {
  data_location_id_t id;
  std::uint32_t length;

  friend std::ostream& operator<<(std::ostream& os,
                                  const DataLocation& location) {
    os << "startLocation: " << location.id << " length: " << location.length;
    return os;
  }

  bool operator==(const DataLocation& other) const {
    return id == other.id && length == other.length;
  }
};

static_assert(std::is_standard_layout_v<BufferPageHeader>);
static_assert(std::is_standard_layout_v<BufferPage>);
static_assert(sizeof(BufferPageHeader) == HEADER_SIZE);
static_assert(sizeof(BufferPage) == PAGE_SIZE);

using buffer_page_header_t = BufferPageHeader;
using buffer_page_t = BufferPage;
using data_location_t = DataLocation;

struct BlockLocation {
  void* ptr;
  std::uint32_t length;

  friend std::ostream& operator<<(std::ostream& os,
                                  const BlockLocation& location) {
    os << "ptr: " << location.ptr << " length: " << location.length;
    return os;
  }
};

using block_location_t = BlockLocation;
using page_header_location_t = BlockLocation;

class BufferPageControl;
using buffer_page_control_t = BufferPageControl;

class BufferPageControl {
 public:
  explicit BufferPageControl(buffer_page_t* bufferPage);

  std::int64_t getTotalFreeSpace();
  std::int64_t getUsableFreeSpace();
  page_header_location_t getSubHeaderLocation();
  std::optional<block_location_t> getBlock(data_location_id_t locationId);
  bool releaseBlock(data_location_id_t locationId);
  std::optional<void*> getFreeBlock(std::uint32_t size);
  std::vector<data_location_t*> getDataListPtrs() const;
  std::vector<data_location_t> getDataList() const;
  void sortDataList(const cmp_func_t& compareFunction);
  buffer_page_t* getBufferPage();

 private:
  buffer_page_t* bufferPage_;
  data_location_t* dataList_;
  std::vector<data_location_t> freeList_;
};