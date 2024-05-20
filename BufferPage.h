//
// Created by Rahul  Kushwaha on 5/19/24.
//

#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <vector>

constexpr std::uint64_t PAGE_SIZE = 4096;
constexpr std::uint64_t HEADER_SIZE = 64;

inline bool memcmp_diff_size(const void *s1, const void *s2, std::uint32_t size1, std::uint32_t size2) {
  auto len = std::min(size1, size2);
  int result = memcmp(s1, s2, len);

  if (result != 0 || size1 == size2) {
    return result < 0;
  }

  return size1 < size2;
}

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

  void sortDataList() {
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
              [this, &locationLookup](DataLocation &x, DataLocation &y) {
                assert(locationLookup.contains(x.id));
                assert(locationLookup.contains(y.id));

                auto dataX = locationLookup[x.id];
                auto dataY = locationLookup[y.id];

                return memcmp_diff_size(dataX, dataY, x.length, y.length);
              });
  }

 private:
  buffer_page_t *bufferPage_;
  data_location_t *dataList_;
  std::vector<data_location_t> freeList_;
};