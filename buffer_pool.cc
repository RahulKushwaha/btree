//
// Created by Rahul  Kushwaha on 5/17/24.
//

#include "buffer_pool.h"
node_id_t get_next() {
  static node_id_t id{NODE_ID_START};
  return (++id);
}