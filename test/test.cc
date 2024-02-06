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
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

class TestA final {
 public:
  explicit TestA(int) noexcept {}
  TestA(const TestA&) = default;
  TestA(TestA&&) noexcept = default;
  TestA& operator=(const TestA&) noexcept = delete;
  TestA& operator=(TestA&&) noexcept = default;
  ~TestA() noexcept = default;
};

class TestB {
 public:
  TestB(int) noexcept { std::cout << "1" << std::endl; }

  TestB(const TestB&) = default;
  TestB(TestB&&) = delete;
  TestB& operator=(const TestB&) = delete;
  TestB& operator=(TestB&&) = delete;
};

int main() {
  std::vector<TestB> vecB;
  TestA a(1);
  vecB.emplace_back(1);
  return 0;
}
