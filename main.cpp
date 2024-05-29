#include "BTree.h"
#include <algorithm>
#include <iostream>
#include <random>

int main() {
  // First create an instance of an engine.
  std::random_device rnd_device;
  // Specify the engine and distribution.
  std::mt19937 mersenne_engine{rnd_device()};// Generates random integers
  std::uniform_int_distribution<int> dist{5000, 9000};

  auto gen = [&dist, &mersenne_engine]() {
    return dist(mersenne_engine);
  };

  auto fileIO = makeFileIO("/tmp/input.txt");
  node_id_t rootId = -1;
  {
    std::cout << "Iteration: 0" << std::endl;
    auto bufferPool = std::make_shared<BufferPool>(fileIO);
    BTree tree{bufferPool};
    tree.insert(std::to_string(0), std::to_string(0));

    std::vector<int> vec(40, gen());
    std::generate(begin(vec), end(vec), gen);

    for (auto c: vec) {
      tree.insert(std::to_string(c), std::to_string(c));
    }

    tree.debug_print();
    std::cout << std::endl;

    bufferPool->flushAll();

    rootId = tree.getRootId();
  }

  {
    std::cout << "printing elements" << std::endl;
    auto bufferPool = std::make_shared<BufferPool>(fileIO);
    BTree tree{bufferPool, rootId};
    tree.debug_print();
    std::cout << std::endl;
  }

  //  btree tree{};
  //  tree.insert(std::numeric_limits<key_t>::min());
  //
  //  std::vector<int> vec{8, 5, 6, 4, 5, 7, 5, 5, 5, 5};
  //
  //  for (auto c: vec) {
  //    tree.insert(c);
  //    tree.debug_print();
  //  }
  //
  //  auto elements = tree.elements();
  //  assert(std::is_sorted(elements.begin(), elements.end()));

  return 0;
}
