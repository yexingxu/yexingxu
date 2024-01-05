/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:23:40
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 19:57:51
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
#include "ser_config.hpp"
#include "user_defined_struct.h"

TEST(SerializerTest, CalculateSizeTest) {
  using namespace serialize;
  // fundamental type
  int a = 1;
  EXPECT_TRUE(detail::is_trivial_serializable<int>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(a).total,
            sizeof(int));
  auto len1 = detail::get_serialize_runtime_info<serialize_props>(a);
  EXPECT_EQ(len1, 4);
  __int128_t b = 3;
  EXPECT_TRUE(detail::is_trivial_serializable<__int128_t>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(b).total,
            sizeof(__int128_t));
  double c = 4.0;
  EXPECT_TRUE(detail::is_trivial_serializable<double>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(c).total,
            sizeof(double));
  enum d { A, B };
  EXPECT_TRUE(detail::is_trivial_serializable<d>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(d::A).total,
            sizeof(d));
  enum class e : std::uint8_t { good = 0, failed = 1, incomplate = 100 };
  e e1 = e::failed;
  EXPECT_TRUE(detail::is_trivial_serializable<e>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(e1).total,
            sizeof(std::uint8_t));
  std::bitset<128> f;
  EXPECT_TRUE(detail::is_trivial_serializable<std::bitset<128>>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(f).total,
            sizeof(f));
  auto len_bitest = detail::get_serialize_runtime_info<serialize_props>(f);
  EXPECT_EQ(len_bitest, sizeof(std::bitset<128>));
  // std container
  EXPECT_FALSE(detail::is_trivial_serializable<std::string>::value);
  std::string str = "abc";
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(str).total, 4);
  EXPECT_FALSE(detail::is_trivial_serializable<std::list<int>>::value);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(str), 6);
  std::list<int> list_int{1, 2, 3, 4};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(list_int).total,
            list_int.size() * sizeof(int));
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(list_int), 20);

  std::list<std::string> list_str{"aa", "bbb", "cccc"};
  uint32_t size = 0;
  for (auto& s : list_str) {
    size += s.size() + 1;
  }
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(list_str).total,
            size);
  EXPECT_FALSE(detail::is_trivial_serializable<std::deque<int>>::value);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(list_str), 22);

  std::deque<int> deque_int{1, 2, 3, 4};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(deque_int).total,
            deque_int.size() * sizeof(int));
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(deque_int), 20);
  std::deque<std::string> deque_str{"aa", "bbb", "cccc"};
  size = 0;
  for (auto& s : deque_str) {
    size += s.size() + 1;
  }
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(deque_str).total,
            size);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(deque_str), 22);
  // TODO support queue
  //   EXPECT_FALSE(
  // detail::is_trivial_serializable<std::queue<int>>::value);
  EXPECT_FALSE(detail::is_trivial_serializable<std::vector<int>>::value);
  std::vector<int> vec_int{1, 2, 3, 4};
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(vec_int), 20);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(vec_int).total,
            vec_int.size() * sizeof(int));
  std::vector<std::string> vec_str{"aa", "bbb", "cccc"};
  size = 0;
  for (auto& s : vec_str) {
    size += s.size() + 1;
  }
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(vec_str).total,
            size);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(vec_str), 22);

  // pair
  using pair1 = std::pair<int, double>;
  pair1 pi_1{1, 2.0};
  EXPECT_TRUE(detail::is_trivial_serializable<pair1>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(pi_1).total, 12);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(pi_1), 12);
  using pair2 = std::pair<int, std::string>;
  pair2 pi_2{1, "aaa"};

  EXPECT_FALSE(detail::is_trivial_serializable<pair2>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(pi_2).total, 8);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(pi_2), 10);

  // map set
  EXPECT_FALSE(detail::is_trivial_serializable<std::set<int>>::value);
  std::set<int> set_int{1, 2, 3, 4, 5};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(set_int).total, 20);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(set_int), 24);

  std::set<std::string> set_str{"a", "bb", "ccc"};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(set_str).total, 9);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(set_str), 19);
  using map1 = std::map<int, double>;
  EXPECT_FALSE(detail::is_trivial_serializable<map1>::value);
  std::map<int, double> map_i_d{{1, 2.0}, {2, 3.0}};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(map_i_d).total, 24);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(map_i_d), 28);
  std::map<int, std::vector<int>> map_i_v{{1, {1, 2}}, {2, {2, 3}}};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(map_i_d).total, 24);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(map_i_v), 36);
  // array
  using arr_int = std::array<int, 10>;
  EXPECT_TRUE(detail::is_trivial_serializable<arr_int>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(arr_int{}).total,
            sizeof(arr_int));
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(map_i_v), 36);
  using arr_dou = std::array<double, 10>;
  EXPECT_TRUE(detail::is_trivial_serializable<arr_dou>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(arr_dou{}).total,
            sizeof(arr_dou));
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(arr_dou{}), 80);
  using arr_str = std::array<std::string, 10>;
  EXPECT_FALSE(detail::is_trivial_serializable<arr_str>::value);
  arr_str a_str;
  a_str.fill("aaa");
  size = 0;
  for (auto& s : a_str) {
    size += s.size() + 1;
  }
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(a_str).total, size);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(a_str), 60);
  // c_array
  int c_array_int[2]{1, 2};
  EXPECT_TRUE(detail::is_trivial_serializable<
              detail::remove_cvref_t<decltype(c_array_int[2])>>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(c_array_int).total,
            8);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(c_array_int),
            8);
  std::string c_array_str[2][2]{{"aa", "bbb"}, {"cc", "ddd"}};
  EXPECT_FALSE(detail::is_trivial_serializable<
               detail::remove_cvref_t<decltype(c_array_str[2][3])>>::value);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(c_array_str).total,
            14);
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(c_array_str).total,
            14);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(c_array_str),
            22);

  person1 p1{1, 1.2, 2};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(p1).total, 16);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(p1), 24);
  person2 p2{1, 1.2, {1, 2}};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(p2).total, 20);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(p2), 32);

  person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  EXPECT_EQ(detail::calculate_payload_size<serialize_props>(p3).total, 36);
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(p3), 64);

  // get runtime info
  std::vector<person3> vec_per{p3, p3, p3};
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(vec_per),
            64 * 3 + 4);

  std::tuple<int, double> tuple_1{1, 2.0};
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(tuple_1), 12);
  mpark::variant<int, double> variant_1{1};
  EXPECT_EQ(detail::get_serialize_runtime_info<serialize_props>(variant_1), 5);
}
