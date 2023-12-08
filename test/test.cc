/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-06 14:12:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 15:43:22
 */

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "serialize/serialize.hpp"
#include "shared_ptr.h"
// #include "template/type_id_test.h"

// struct A {
//   int a;
//   double b;
//   std::string c;
// };

// TEST(SharedPtrTest, OperatorTest) {
//   util::SharedPtr<A> test_a(new A{2, 3.0, "cc"});
//   EXPECT_EQ(test_a->a, 2);
//   EXPECT_EQ(test_a->b, 3.0);
//   EXPECT_EQ(test_a->c, "cc");
//   test_a->a = 1;
//   test_a->b = 2.0;
//   test_a->c = "ss";
//   EXPECT_EQ(test_a.use_count(), 1);
//   auto test_b = test_a;
//   EXPECT_EQ(test_a.use_count(), 2);
//   EXPECT_EQ(test_b.use_count(), 2);
//   EXPECT_EQ(test_b->a, 1);
//   EXPECT_EQ(test_b->b, 2.0);
//   EXPECT_EQ(test_b->c, "ss");
// }

int main() {
  std::cout << "test start..." << std::endl;
  testing::InitGoogleTest();

  // 1行代码序列化
  int a = 1;
  std::array<int, 3> b{1, 2, 3};
  std::vector<int> c{1, 2, 3};
  std::map<int, double> d;
  d.emplace(std::make_pair(1, 2.0));
  std::set<int> e{1, 2, 3};
  std::string f("test");
  auto result = serialize::serialize(a);
  // auto buffer = std::make_shared<std::vector<char>>();
  std::cout << serialize::details::check_circle<std::array<int, 3>>()
            << std::endl;
  std::cout << serialize::details::check_circle<int, void>() << std::endl;
  std::cout << serialize::details::check_circle<std::map<int, std::string>>()
            << std::endl;
  std::cout << serialize::details::check_circle<std::list<int>>() << std::endl;
  std::cout << serialize::details::check_circle<std::vector<int>>()
            << std::endl;
  std::cout << serialize::details::check_circle<std::set<int>>() << std::endl;
  // auto cycle = serialize::details::check_circle<double>();
  auto info = serialize::details::get_serialize_runtime_info<0>(a);
  auto info1 = serialize::details::get_serialize_runtime_info<0>(b);
  // auto info2 = serialize::details::get_serialize_runtime_info<0>(c);
  // auto info3 = serialize::details::get_serialize_runtime_info<0>(d);
  // auto info4 = serialize::details::get_serialize_runtime_info<0>(e);
  // auto info5 = serialize::details::get_serialize_runtime_info<0>(f);

  // std::array<int, 3> b{1, 2, 3};
  // auto info1 = serialize::details::get_serialize_runtime_info<0>(b);

  // auto result1 = serialize::serialize(b);
  // auto id1 = serialize::details::get_type_id<std::uint8_t>();
  // EXPECT_EQ(id1, serialize::details::type_id::uint8_t);

  return RUN_ALL_TESTS();
}
