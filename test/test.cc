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

template <typename... Args>
void des() {
  serialize::details::get_args_type<Args...> type;
  type = 1;
  std::cout << type << std::endl;
}

int main() {
  std::cout << "test start..." << std::endl;
  // testing::InitGoogleTest();

  // 1行代码序列化
  int a = 2;
  // std::array<int, 3> b{1, 2, 3};
  // std::vector<int> c{1, 2, 3};
  // std::map<int, double> d;
  // d.emplace(std::make_pair(1, 2.0));
  // std::set<int> e{1, 2, 3};
  // std::string f("test");
  auto result = serialize::serialize<0b11>(a);
  for (auto& r : result) {
    std::cout << (std::uint32_t)r << " ";
  }
  std::cout << std::endl;

  auto des = serialize::deserialize<0b11, int>(result);
  if (des.has_value()) {
    std::cout << "des1:" << des.has_value() << " * " << des.value()
              << std::endl;
  } else {
    std::cout << "des" << (int)des.error() << std::endl;
  }
  // auto result1 = serialize::serialize<0b11>(b);
  std::cout << "------------" << std::endl;

  // for (size_t i = 0; i < result.size(); ++i) {
  //   std::cout << (std::uint8_t)result.at(i) << " ";
  // }
  // std::cout << result.size() << "------------" << std::endl;

  // // auto buffer = std::make_shared<std::vector<char>>();
  // std::cout << serialize::details::check_circle<std::array<int, 3>>()
  //           << std::endl;
  // std::cout << serialize::details::check_circle<int, void>() << std::endl;
  // std::cout << serialize::details::check_circle<std::map<int, std::string>>()
  //           << std::endl;
  // std::cout << serialize::details::check_circle<std::list<int>>() <<
  // std::endl; std::cout <<
  // serialize::details::check_circle<std::vector<int>>()
  //           << std::endl;
  // std::cout << serialize::details::check_circle<std::set<int>>() <<
  // std::endl;
  // // auto cycle = serialize::details::check_circle<double>();
  // auto info = serialize::details::get_serialize_runtime_info<0>(a);
  // auto info1 = serialize::details::get_serialize_runtime_info<0>(b);
  // std::cout << info.size() << "  **  " << info.metainfo() << std::endl;
  // std::cout << info1.size() << "  **  " << info1.metainfo() << std::endl;
  // auto info2 = serialize::details::get_serialize_runtime_info<0>(c);
  // auto info3 = serialize::details::get_serialize_runtime_info<0>(d);
  // auto info4 = serialize::details::get_serialize_runtime_info<0>(e);
  // auto info5 = serialize::details::get_serialize_runtime_info<0>(f);

  // std::array<int, 3> b{1, 2, 3};
  // auto info1 = serialize::details::get_serialize_runtime_info<0>(b);

  // auto result1 = serialize::serialize(b);
  // auto id1 = serialize::details::get_type_id<std::uint8_t>();
  // EXPECT_EQ(id1, serialize::details::type_id::uint8_t);

  // des<int>();

  // return RUN_ALL_TESTS();
  return 0;
}
