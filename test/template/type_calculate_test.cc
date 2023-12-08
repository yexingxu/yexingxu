/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-08 00:23:42
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 08:24:12
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

  //   static_assert(std::is_same<decltype(get_types<person>()),
  //                              std::tuple<int, std::string>>::value,
  //                 "");
  //   static_assert(
  //       std::is_same<
  //           decltype(get_types<nested_object>()),
  //           std::tuple<int, std::string, person, complicated_object>>::value,
  //       "");

  //   serialize::get_type_code<std::list<int>>();
}