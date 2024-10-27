//
// Created by Rahul  Kushwaha on 7/14/24.
//

#include "Api.h"

#include <cassert>

txn_id_t Api::beginTxn() {
  return 0;
}

std::vector<operation_result_t> Api::execute(
    txn_id_t txnId, std::vector<operation_t> operations) {
  return {};
}

void Api::abortTxn(txn_id_t txnId) {
  return;
}

bool Api::commitTxn(txn_id_t txnId) {
  return {};
}