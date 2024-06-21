#include "BTree.h"
#include <algorithm>
#include <future>
#include <iostream>
#include <random>
#include <thread>

std::string what(const std::exception_ptr &eptr = std::current_exception()) {
  if (!eptr) { throw std::bad_exception(); }

  try {
    std::rethrow_exception(eptr);
  } catch (const std::exception &e) { return e.what(); } catch (const std::string &e) {
    return e;
  } catch (const char *e) { return e; } catch (...) {
    return "who knows";
  }
}

int main() {
  try {
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine{rnd_device()};// Generates random integers
    std::uniform_int_distribution<int> dist{5000, std::numeric_limits<int>::max()};

    auto gen = [&dist, &mersenne_engine]() {
      return dist(mersenne_engine);
    };

    std::string path{std::format("/tmp/input_{}.txt", std::chrono::system_clock::now().time_since_epoch().count())};
    auto fileIO = makeFileIO(path);
    node_id_t rootId = -1;
    {
      std::cout << "Iteration: 0" << std::endl;
      auto bufferPool = std::make_shared<BufferPool>(fileIO);
      BTree tree{bufferPool};
      tree.insert(std::to_string(0), std::to_string(0));

      std::vector<int> vec(10000, gen());
      std::generate(begin(vec), end(vec), gen);

      std::vector<std::future<bool>> results;

      for (auto c: vec) {
        std::cout << "Insert " << c << std::endl;
        std::future<bool> insertionResult = std::async(std::launch::async, &BTree::insert, tree, std::to_string(c), std::to_string(c));
//                insertionResult.get();
        results.emplace_back(std::move(insertionResult));
      }


      for (auto &result: results) {
        result.get();
      }

      tree.debug_print();
      std::cout << std::endl;
      auto elements = tree.elements();
      auto sorted = std::is_sorted(elements.begin(), elements.end());
      std::cout << "Printing Elements" << std::endl;
      for (auto &element: elements) {

        std::cout << element << std::endl;
      }

      assert(sorted);

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

  } catch (...) {
    std::cout << what() << std::endl;
    exit(1);
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
