//
// Created by Rahul  Kushwaha on 3/9/25.
//

#pragma once

#include <variant>
#include "Parser.h"
#include "include/LogMessages.h"
#include "include/LogStore.h"

namespace bee_tree::block_store {
using command_t = std::variant<InsertCommand, DeleteCommand, SplitBlockCommand,
                               InvalidCommand>;

static command_t parse_command(const LogEntry& logEntry);
}