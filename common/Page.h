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

namespace bee_tree::common {

constexpr std::uint64_t HEADER_SIZE = 64;

struct PageHeader {
  std::uint32_t data_list_size;
  std::uint32_t padding[15];
};

struct RawPage {
  PageHeader header;
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

static_assert(std::is_standard_layout_v<PageHeader>);
static_assert(std::is_standard_layout_v<RawPage>);
static_assert(sizeof(PageHeader) == HEADER_SIZE);
static_assert(sizeof(RawPage) == PAGE_SIZE);

using page_header_t = PageHeader;
using raw_page_t = RawPage;
using raw_page_ptr_t = raw_page_t*;
using data_location_t = DataLocation;
using data_location_ptr_t = data_location_t*;

struct Location {
  void* ptr;
  std::uint32_t length;

  friend std::ostream& operator<<(std::ostream& os,
                                  const Location& location) {
    os << "ptr: " << location.ptr << " length: " << location.length;
    return os;
  }
};

using block_location_t = Location;
using page_header_location_t = Location;

class Page;
using page_t = Page;
using page_ptr_t = page_t*;

class Page {
public:
  explicit Page(raw_page_ptr_t page);

  std::int64_t getTotalFreeSpace() const;
  std::int64_t getUsableFreeSpace() const;
  page_header_location_t getHeaderLocation() const;
  std::optional<block_location_t> getDataBlock(
      data_location_id_t locationId) const;
  bool freeDataBlock(data_location_id_t locationId);
  std::optional<void*> allocateDataBlock(std::uint32_t size);
  std::vector<data_location_ptr_t> getDataListPtrs() const;
  std::vector<data_location_t> getDataList() const;
  std::uint32_t getDataListSize() const;
  void sortDataList(const cmp_func_t& compareFunction);
  raw_page_ptr_t getPage() const;

private:
  raw_page_ptr_t bufferPage_;
  data_location_ptr_t dataList_;
  std::vector<data_location_t> freeList_;
};
}