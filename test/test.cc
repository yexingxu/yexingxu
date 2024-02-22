/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-06 14:12:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-13 02:43:24
 */

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

// #include "types/expected.hpp"

// class TestA final {
//  public:
//   explicit TestA(int) noexcept {}
//   TestA(const TestA&) = default;
//   TestA(TestA&&) noexcept = default;
//   TestA& operator=(const TestA&) noexcept = delete;
//   TestA& operator=(TestA&&) noexcept = default;
//   ~TestA() noexcept = default;
// };

// class TestB {
//  public:
//   TestB(int) noexcept { std::cout << "1" << std::endl; }

//   TestB(const TestB&) = default;
//   TestB(TestB&&) = delete;
//   TestB& operator=(const TestB&) = delete;
//   TestB& operator=(TestB&&) = delete;
// };

// enum class Error { A, B };

// tl::expected<int, std::string> test() { return 1; }
// tl::expected<int, std::string> a(int sample) { return sample; }

// tl::expected<int, std::string> getInt3(int val) { return val; }

// tl::expected<int, std::string> getInt2(int val) { return val; }

// tl::expected<int, std::string> getInt1() {
//   return getInt2(5).and_then(getInt3);
// }

class TTA {
 public:
  TTA(std::unique_ptr<int, std::function<void(int*)>>&& ptr)
      : ptr_(std::move(ptr)) {}

 private:
  std::unique_ptr<int, std::function<void(int*)>> ptr_;
};

int main() {
  // std::vector<TestB> vecB;
  // TestA a(1);
  // vecB.emplace_back(1);
  std::vector<int> vecA{1, 2, 3, 4, 5};
  std::vector<int> vecB{vecA};

  for (auto iter = vecB.begin(); iter != vecB.end();) {
    if (*iter % 2 == 0) {
      std::cout << *iter << " ^ " << &(*iter) << std::endl;
      vecB.erase(iter);
    } else {
      std::cout << *iter << " * " << &(*iter) << std::endl;
      iter++;
    }
  }

  for (auto iter = vecA.begin(); iter != vecA.end();) {
    if (*iter % 2 == 0) {
      std::cout << *iter << " ^ " << &(*iter) << std::endl;
      vecA.erase(iter);
    } else {
      std::cout << *iter << " * " << &(*iter) << std::endl;
      iter++;
    }
  }

  // getInt2(5).and_then(getInt3);
  // test().and_then(
  //     [](auto&& sample) -> tl::expected<int, std::string> { return sample;
  //     });

  // std::unique_ptr<int, std::function<void(int* p)>> ptr(
  //     (new int(1)), [](int* p) { delete p; });

  // std::unique_ptr<int> ptr1((new int(1)));
  // TTA a(std::move(ptr1));

  return 0;
}
