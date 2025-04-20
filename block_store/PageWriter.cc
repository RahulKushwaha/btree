//
// Created by Rahul  Kushwaha on 3/8/25.
//

#include "PageWriter.h"

#include <boost/assert.hpp>

#include "Parser.h"

namespace bee_tree::block_store {

void PageWriter::apply(LogEntry logEntry, common::RowBlock rowBlock) {
  auto command = parse_command(logEntry);
  if (std::holds_alternative<InvalidCommand>(command)) {
    BOOST_ASSERT(false && "Invalid command");
    return;
  }

  if (std::holds_alternative<InsertCommand>(command)) {

  } else if (std::holds_alternative<DeleteCommand>(command)) {

  }
}

PageWriter::~PageWriter() = default;

}