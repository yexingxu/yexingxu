/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-08 00:23:42
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-12 14:20:28
 */
// #include "serialize/type_calculate.hpp"

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "serialize/reflection.hpp"
#include "serialize/serialize.hpp"

struct person {
  int a;
  double b;
};

TEST(SerializerTest, TypeCalculateTest) {
  using namespace serialize::details;
  //   static_assert(std::is_same<std::tuple<int, double>,
  //                              decltype(get_types<person>())>::value,
  //                 "");

  static_assert(
      std::is_same<std::tuple<int, std::string>,
                   decltype(get_types<std::tuple<int, std::string>>())>::value,
      "");

  static_assert(
      std::is_same<std::tuple<int>, decltype(get_types<int>())>::value, "");

  static_assert(std::is_same<std::tuple<std::vector<int>>,
                             decltype(get_types<std::vector<int>>())>::value,
                "");

  auto str = serialize::get_type_literal<int>();
  auto str1 = serialize::get_type_literal<int, int>();
  auto str2 = serialize::get_type_literal<int, double>();
  std::cout << str.data() << std::endl;
  std::cout << str1.data() << std::endl;
  std::cout << str2.data() << std::endl;
}

// TEST(SerializerTest, TypeCalculateTest) {
//   // serialize::string_literal<char, 0> a1;
//   // serialize::string_literal<char, 1> a2;
//   // auto a3 = a1 + a2;
//   //   a1 += a2;
//   auto str = serialize::get_type_literal<int>();
// }