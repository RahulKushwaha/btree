//
// Created by Rahul  Kushwaha on 3/9/25.
//

#include "RowBlock.h"
#include <boost/assert.hpp>

namespace bee_tree::common {

RowBlock::RowBlock(raw_page_ptr_t rawPage, cmp_func_t cmpFunc) :
  m_page{page_t{rawPage}}, m_compareFunction{std::move(cmpFunc)} {}

RowKey RowBlock::getSmallestKey() const {
  auto dataList = m_page.getDataList();
  BOOST_ASSERT(!dataList.empty() && "datalist cannot be empty");
  auto first = dataList.at(0);
  return getKey(first);
}

RowKey RowBlock::getLargestKey() const {
  auto dataList = m_page.getDataList();
  BOOST_ASSERT(!dataList.empty() && "datalist cannot be empty");
  auto first = dataList.at(dataList.size() - 1);
  return getKey(first);
}

RowKey RowBlock::getKey(data_location_t dataLocation) const {
  auto dataBlock = m_page.getDataBlock(dataLocation.id);
  BOOST_ASSERT(dataBlock.has_value() && "data block must exist");

  // Assuming the key is stored at the beginning of the data block
  const char* dataPtr = static_cast<const char*>(dataBlock.value().ptr);
  std::uint64_t rowVersion;
  std::memcpy(&rowVersion, dataPtr + dataLocation.length - sizeof(rowVersion),
              sizeof(rowVersion));

  return RowKey{
      std::string_view(dataPtr, dataLocation.length - sizeof(rowVersion)),
      rowVersion};
}

std::optional<void*> RowBlock::allocate(std::uint32_t size) {
  auto dataBlock = m_page.allocateDataBlock(size);
  return dataBlock;
}

bool RowBlock::erase(data_location_t dataLocation) {
  return m_page.freeDataBlock(dataLocation.id);
}

void RowBlock::copyDataBlock(data_location_t srcDataLocation,
                             row_block_ptr_t dest) const {
  auto dataBlock = m_page.getDataBlock(srcDataLocation.id);
  BOOST_ASSERT(dataBlock.has_value() && "source data block must exist");

  auto destPage = dest->m_page;
  auto destFreeSpace = destPage.getUsableFreeSpace();
  BOOST_ASSERT(
      destFreeSpace < dataBlock->length &&
      "not enough space in destination block");

  auto destDataBlock = destPage.allocateDataBlock(dataBlock->length);
  BOOST_ASSERT(
      destDataBlock.has_value() && "destination data block must exist");

  std::memcpy(destDataBlock.value(), dataBlock->ptr, dataBlock->length);
}

std::int64_t RowBlock::getUsableFreeSpace() const {
  return m_page.getUsableFreeSpace();
}

std::int64_t RowBlock::getTotalFreeSpace() const {
  return m_page.getTotalFreeSpace();
}

void RowBlock::eraseAll() {
  auto dataDictionaryList = m_page.getDataList();

  for (auto dataBlock : dataDictionaryList) {
    auto result = m_page.freeDataBlock(dataBlock.id);
    BOOST_ASSERT(result && "failed to free data block");
  }

  BOOST_ASSERT(m_page.getDataListSize() == 0 && "data list must be empty");
}

void RowBlock::sortDataList() {
  m_page.sortDataList(m_compareFunction);
}

}