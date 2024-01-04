/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:23:40
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 20:43:26
 */

#include "detail/calculate_size.hpp"

#include <gtest/gtest.h>

#include <bitset>
#include <cstdint>
#include <deque>
#include <list>
#include <queue>
#include <string>

#include "detail/reflection.hpp"
#include "user_defined_struct.h"

TEST(SerializerTest, CalculateSizeTest) {
  // fundamental type
  int a = 1;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<int>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(a).total, sizeof(int));
  __int128_t b = 3;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<__int128_t>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(b).total,
            sizeof(__int128_t));
  double c = 4.0;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<double>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(c).total, sizeof(double));
  enum d { A, B };
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<d>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(d::A).total, sizeof(d));
  enum class e : std::uint8_t { good = 0, failed = 1, incomplate = 100 };
  e e1 = e::failed;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<e>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(e1).total,
            sizeof(std::uint8_t));
  std::bitset<128> f;
  EXPECT_TRUE(
      serialize::detail::is_trivial_serializable<std::bitset<128>>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(f).total, sizeof(f));

  // std container
  EXPECT_FALSE(serialize::detail::is_trivial_serializable<std::string>::value);
  std::string str = "abc";
  EXPECT_EQ(serialize::detail::calculate_one_size()(str).total, str.size());
  EXPECT_FALSE(
      serialize::detail::is_trivial_serializable<std::list<int>>::value);
  std::list<int> list_int{1, 2, 3, 4};
  EXPECT_EQ(serialize::detail::calculate_one_size()(list_int).total,
            list_int.size() * sizeof(int));
  std::list<std::string> list_str{"aa", "bbb", "cccc"};
  uint32_t size = 0;
  for (auto& s : list_str) {
    size += s.size();
  }
  EXPECT_EQ(serialize::detail::calculate_one_size()(list_str).total, size);
  EXPECT_FALSE(
      serialize::detail::is_trivial_serializable<std::deque<int>>::value);
  std::deque<int> deque_int{1, 2, 3, 4};
  EXPECT_EQ(serialize::detail::calculate_one_size()(deque_int).total,
            deque_int.size() * sizeof(int));
  std::deque<std::string> deque_str{"aa", "bbb", "cccc"};
  size = 0;
  for (auto& s : deque_str) {
    size += s.size();
  }
  EXPECT_EQ(serialize::detail::calculate_one_size()(deque_str).total, size);
  // TODO support queue
  //   EXPECT_FALSE(
  // serialize::detail::is_trivial_serializable<std::queue<int>>::value);
  EXPECT_FALSE(
      serialize::detail::is_trivial_serializable<std::vector<int>>::value);
  std::vector<int> vec_int{1, 2, 3, 4};
  EXPECT_EQ(serialize::detail::calculate_one_size()(vec_int).total,
            vec_int.size() * sizeof(int));
  std::vector<std::string> vec_str{"aa", "bbb", "cccc"};
  size = 0;
  for (auto& s : vec_str) {
    size += s.size();
  }
  EXPECT_EQ(serialize::detail::calculate_one_size()(vec_str).total, size);

  // pair
  using pair1 = std::pair<int, double>;
  pair1 pi_1{1, 2.0};
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<pair1>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(pi_1).total, 12);

  using pair2 = std::pair<int, std::string>;
  pair2 pi_2{1, "aaa"};

  EXPECT_FALSE(serialize::detail::is_trivial_serializable<pair2>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(pi_2).total, 7);

  // map set
  EXPECT_FALSE(
      serialize::detail::is_trivial_serializable<std::set<int>>::value);
  std::set<int> set_int{1, 2, 3, 4, 5};
  EXPECT_EQ(serialize::detail::calculate_one_size()(set_int).total, 20);
  std::set<std::string> set_str{"a", "bb", "ccc"};
  EXPECT_EQ(serialize::detail::calculate_one_size()(set_str).total, 6);
  using map1 = std::map<int, double>;
  EXPECT_FALSE(serialize::detail::is_trivial_serializable<map1>::value);
  std::map<int, double> map_i_d{{1, 2.0}, {2, 3.0}};
  EXPECT_EQ(serialize::detail::calculate_one_size()(map_i_d).total, 24);
  std::map<int, std::vector<int>> map_i_v{{1, {1, 2}}, {2, {2, 3}}};
  EXPECT_EQ(serialize::detail::calculate_one_size()(map_i_d).total, 24);
  // array
  using arr_int = std::array<int, 10>;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<arr_int>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(arr_int{}).total,
            sizeof(arr_int));
  using arr_dou = std::array<double, 10>;
  EXPECT_TRUE(serialize::detail::is_trivial_serializable<arr_dou>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(arr_dou{}).total,
            sizeof(arr_dou));
  using arr_str = std::array<std::string, 10>;
  EXPECT_FALSE(serialize::detail::is_trivial_serializable<arr_str>::value);
  arr_str a_str;
  a_str.fill("aaa");
  size = 0;
  for (auto& s : a_str) {
    size += s.size();
  }
  EXPECT_EQ(serialize::detail::calculate_one_size()(a_str).total, size);

  // c_array
  int c_array_int[2]{1, 2};
  EXPECT_TRUE(
      serialize::detail::is_trivial_serializable<
          serialize::detail::remove_cvref_t<decltype(c_array_int[2])>>::value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(c_array_int).total, 8);
  std::string c_array_str[2][2]{{"aa", "bbb"}, {"cc", "ddd"}};
  EXPECT_FALSE(serialize::detail::is_trivial_serializable<
               serialize::detail::remove_cvref_t<decltype(c_array_str[2][3])>>::
                   value);
  EXPECT_EQ(serialize::detail::calculate_one_size()(c_array_str).total, 10);
  EXPECT_EQ(serialize::detail::calculate_payload_size(c_array_str).total, 10);

  person1 p1{1, 1.2, 2};
  EXPECT_EQ(serialize::detail::calculate_one_size()(p1).total, 16);
  // person2 p2{1, 1.2, {1, 2}};
  // EXPECT_EQ(serialize::detail::calculate_one_size()(p2).total, 19);
  // person3 p3{{2, 2.2}, {1, 1.2, {1, 2}}};
  // EXPECT_EQ(serialize::detail::calculate_one_size()(p3).total, 31);
  // auto ass = serialize::detail::is_trivial_serializable<person1>::value;
  // auto bbv = serialize::detail::user_defined_refl<person1>;
  // auto id = serialize::detail::get_type_id<person1>();
  // serialize::detail::type_id::array_t;
  // serialize::detail::type_id::struct_t;
}
