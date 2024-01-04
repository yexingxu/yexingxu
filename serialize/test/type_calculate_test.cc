/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 07:10:11
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 19:19:54
 */

#include "detail/type_calculate.hpp"

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "detail/calculate_size.hpp"

template <typename T>
struct test_has_container0 {
  T hi;
};

template <typename T>
struct test_has_container {
  int a;
  double b;
  std::array<std::tuple<std::pair<int, test_has_container0<T>>, int, int>, 10>
      c;
};

TEST(SerializerTest, TypeCalculateTest) {
  using namespace serialize::detail;
  //   static_assert(std::is_same<std::tuple<int, std::string>,
  //                              decltype(get_types<person>())>::value,
  //                 "");

  //   static_assert(
  //       std::is_same<std::tuple<int, std::string>,
  //                    decltype(get_types<std::tuple<int,
  //                    std::string>>())>::value,
  //       "");

  static_assert(
      std::is_same<std::tuple<int>, decltype(get_types<int>())>::value, "");
  static_assert(std::is_same<std::tuple<std::vector<int>>,
                             decltype(get_types<std::vector<int>>())>::value,
                "");

  static_assert(serialize::detail::check_circle<std::array<int, 3>>() == 0, "");
  static_assert(!serialize::detail::check_if_has_container<int>(), "");
  static_assert(serialize::detail::check_if_has_container<std::vector<int>>(),
                "");
  // TODO struct
  using array_int = std::array<int, 3>;
  static_assert(!serialize::detail::check_if_has_container<array_int>(), "");
  static_assert(serialize::detail::check_if_has_container<std::set<int>>(), "");

  static_assert(serialize::detail::check_if_has_container<std::string>(), "");
  static_assert(
      serialize::detail::check_if_has_container<std::map<int, double>>(), "");
  static_assert(
      !serialize::detail::check_if_has_container<std::pair<int, double>>(), "");
  static_assert(
      serialize::detail::check_if_has_container<std::pair<int, std::string>>(),
      "");
  //   static_assert(serialize::detail::check_if_has_container<
  //                 test_has_container<std::vector<int>>>());
  //   static_assert(serialize::detail::check_if_has_container<
  //                 test_has_container<std::string>>());
  //   static_assert(serialize::detail::check_if_has_container<
  //                 test_has_container<std::map<int, int>>>());
  //   static_assert(serialize::detail::check_if_has_container<
  //                 test_has_container<std::set<int>>>());
}

TEST(SerializerTest, GetRuntimeInfoTest) {
  using namespace serialize::detail;
  // int a = 1;
  // auto info_int = get_serialize_runtime_info<0b11>(a);
  std::vector<int> int_vec{1, 2, 3};
  // auto info_int_vec = get_serialize_runtime_info<0b11>(int_vec);

  std::array<int, 3> int_arr{1, 3, 4};
  // auto info_arr = get_serialize_runtime_info<0b11>(int_arr);
  static_assert(!check_if_has_container<get_args_type<decltype(int_arr)>>(),
                "");
  static_assert(
      std::is_same<std::vector<int>, get_args_type<decltype(int_vec)>>::value,
      "");
}
