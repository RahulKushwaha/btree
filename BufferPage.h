//
// Created by Rahul  Kushwaha on 5/19/24.
//

#pragma once
#include <algorithm>
#include <cstdint>
#include <ostream>
#include <type_traits>
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

struct DataLocation {
  std::uint32_t startLocation;
  std::uint32_t length;

  friend std::ostream &operator<<(std::ostream &os, const DataLocation &location) {
    os << "startLocation: " << location.startLocation << " length: " << location.length;
    return os;
  }
};

static_assert(std::is_standard_layout_v<BufferPageHeader>);
static_assert(std::is_standard_layout_v<BufferPage>);
static_assert(sizeof(BufferPageHeader) == HEADER_SIZE);
static_assert(sizeof(BufferPage) == PAGE_SIZE);

using buffer_page_header_t = BufferPageHeader;
using buffer_page_t = BufferPage;
using data_location_t = DataLocation;

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

  std::optional<char *> getBlock(std::uint32_t size) {
    if (auto freeSpace = getTotalFreeSpace(); freeSpace < size) {
      return {};
    }

    std::uint32_t offset{0};
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      offset = std::max(offset, dataLocation->startLocation + dataLocation->length);
      dataLocation++;
    }

    // update data list
    bufferPage_->header.data_list_size++;
    dataList_--;
    dataList_->length = size;
    dataList_->startLocation = offset;

    return reinterpret_cast<char *>(bufferPage_) + HEADER_SIZE + offset;
  }

  std::vector<DataLocation> getDataList() {
    std::vector<DataLocation> result;
    data_location_t *dataLocation = dataList_;
    for (int i = 0; i < bufferPage_->header.data_list_size; i++) {
      result.emplace_back(data_location_t{
          .startLocation = dataLocation->startLocation,
          .length = dataLocation->length,
      });

      dataLocation++;
    }

    return result;
  }

 private:
  buffer_page_t *bufferPage_;
  data_location_t *dataList_;
  std::vector<data_location_t> freeList_;
};