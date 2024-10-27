//
// Created by Rahul  Kushwaha on 7/14/24.
//

#pragma once
#include "Common.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct InsertIfNotExistsOperation {
  std::string key;
  std::string value;
};

struct UpsertOperation {
  std::string key;
  std::string value;
};

struct DeleteOperation {
  std::string key;
};

struct GetOperation {
  std::string key;
};

struct GetOperationResponse {
  std::string key;
  std::string value;
};

using insert_if_not_exists_operation_t = InsertIfNotExistsOperation;
using insert_if_not_exists_operation_result_t = bool;
using upsert_operation_t = UpsertOperation;
using upsert_operation_result_t = bool;
using delete_operation_t = DeleteOperation;
using delete_operation_result_t = bool;
using get_operation_t = GetOperation;
using get_operation_result_t = GetOperationResponse;

using operation_t =
    std::variant<insert_if_not_exists_operation_t, upsert_operation_t,
                 delete_operation_t, get_operation_t>;

using operation_result_t =
    std::variant<insert_if_not_exists_operation_result_t,
                 upsert_operation_result_t, delete_operation_result_t,
                 get_operation_result_t>;

class Api {
 public:
  txn_id_t beginTxn();

  std::vector<operation_result_t> execute(txn_id_t txnId,
                                          std::vector<operation_t> operations);

  void abortTxn(txn_id_t txnId);
  bool commitTxn(txn_id_t txnId);

 private:
};
