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

constexpr std::uint64_t PAGE_SIZE = 4096;
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

  friend std::ostream &operator<<(std::ostream &os, const DataLocation &location) {
    os << "startLocation: " << location.id << " length: " << location.length;
    return os;
  }

  bool operator==(const DataLocation &other) const {
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
  void *ptr;
  std::uint32_t length;

  friend std::ostream &operator<<(std::ostream &os, const BlockLocation &location) {
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
  explicit BufferPageControl(buffer_page_t *bufferPage)
      : bufferPage_{bufferPage},
        dataList_{[this]() {
          char *blockStart = reinterpret_cast<char *>(bufferPage_);
          blockStart += PAGE_SIZE - (bufferPage_->header.data_list_size) * sizeof(DataLocation);

          data_location_t *dataList{reinterpret_cast<data_location_t *>(blockStart)};
          return dataList;
        }()} {}

  std::int64_t getTotalFreeSpace() {
    // Subtract HeaderSize
    std::int64_t result = PAGE_SIZE - HEADER_SIZE;

    // Subtract each of the occupied data list
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      result -= dataLocation->length;

      // We have to subtract the space occupied by the data location itself.
      result -= sizeof(*dataLocation);
    }

    // Of the remaining free space we have to deduct the space taken
    // the data location object if this free space is given away.

    result -= sizeof(*dataLocation);

    return result;
  }

  std::int64_t getUsableFreeSpace() {
    if (bufferPage_->header.data_list_size == 0) {
      return PAGE_SIZE - HEADER_SIZE - sizeof(data_location_t);
    }

    // Usable free is the total space left after the last element.
    auto itr = std::max_element(dataList_, dataList_ + bufferPage_->header.data_list_size, [](const auto &x, const auto &y) {
      return x.id < y.id;
    });

    // Subtract each of the occupied data list
    auto optionalBlock = getBlock(itr->id);
    assert(optionalBlock.has_value());
    auto block = optionalBlock.value();

    auto result = std::abs(reinterpret_cast<char *>(block.ptr) - reinterpret_cast<char *>(dataList_));
    result -= sizeof(data_location_t);

    return result;
  }

  page_header_location_t getSubHeaderLocation() {
    return {
        .ptr = bufferPage_->header.padding,
        .length = sizeof(bufferPage_->header.padding),
    };
  }

  std::optional<block_location_t> getBlock(data_location_id_t locationId) {
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      if (dataLocation->id == locationId) {
        auto loc = reinterpret_cast<char *>(bufferPage_);
        return block_location_t{
            .ptr = (loc + HEADER_SIZE + dataLocation->id),
            .length = dataLocation->length,
        };
      }
      dataLocation++;
    }

    return {};
  }

  bool releaseBlock(data_location_id_t locationId) {
    if (bufferPage_->header.data_list_size == 0) {
      return false;
    }

    data_location_t *dataLocation = dataList_;
    data_location_t *newDataLocation = dataLocation + 1;

    bool found{false};
    int i = 0;
    for (; i < bufferPage_->header.data_list_size; i++) {
      if (dataLocation->id == locationId) {
        found = true;
        break;
      }
      dataLocation++;
    }

    if (found) {
      dataLocation = dataList_;
      for (int j = i; j - 1 >= 0; j--) {
        *(dataLocation) = *(dataLocation - 1);
      }

      dataList_ = newDataLocation;
      bufferPage_->header.data_list_size--;
    }

    return found;
  }

  std::optional<void *> getFreeBlock(std::uint32_t size) {
    if (auto freeSpace = getTotalFreeSpace(); freeSpace < size) {
      return {};
    }

    std::uint32_t offset{0};
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      offset = std::max(offset, dataLocation->id + dataLocation->length);
      dataLocation++;
    }

    // update data list
    bufferPage_->header.data_list_size++;
    dataList_--;
    dataList_->length = size;
    dataList_->id = offset;

    return reinterpret_cast<char *>(bufferPage_) + HEADER_SIZE + offset;
  }

  std::vector<data_location_t *> getDataListPtrs() const {
    std::vector<data_location_t *> result;
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      result.emplace_back(dataLocation);

      dataLocation++;
    }

    return result;
  }

  std::vector<data_location_t> getDataList() const {
    std::vector<data_location_t> result;
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      result.emplace_back(data_location_t{
          .id = dataLocation->id,
          .length = dataLocation->length,
      });

      dataLocation++;
    }

    return result;
  }

  void sortDataList(const cmp_func_t &compareFunction) {
    std::unordered_map<data_location_id_t, void *> locationLookup;
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      auto blockLocation = getBlock(dataLocation->id);
      assert(blockLocation.has_value());
      auto result = locationLookup.emplace(dataLocation->id, blockLocation.value().ptr);
      assert(result.second && "failed to insert in the locationLookup");
      dataLocation++;
    }

    std::sort(dataList_, dataList_ + bufferPage_->header.data_list_size,
              [this, &locationLookup, compareFunction](DataLocation &x, DataLocation &y) {
                assert(locationLookup.contains(x.id));
                assert(locationLookup.contains(y.id));

                auto dataX = locationLookup[x.id];
                auto dataY = locationLookup[y.id];

                return compareFunction(dataX, dataY, x.length, y.length);
              });
  }

 private:
  buffer_page_t *bufferPage_;
  data_location_t *dataList_;
  std::vector<data_location_t> freeList_;
};