/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 19:11:44
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 22:20:25
 */

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "serialize.hpp"
#include "user_defined_struct.h"

TEST(SerializerTest, DeserializeTest) {
  std::uint8_t a = 1;
  auto des = serialize::deserialize<std::uint8_t>(serialize::serialize(a));
  EXPECT_EQ(a, des.value());
  double b = 2.0;
  auto des2 = serialize::deserialize<double>(serialize::serialize(b));
  EXPECT_EQ(b, des2.value());
  bool c = true;
  auto des3 = serialize::deserialize<bool>(serialize::serialize(c));
  EXPECT_EQ(c, des3.value());
  enum d { red, bile };
  d testd = d::red;
  auto des4 = serialize::deserialize<d>(serialize::serialize(testd));
  EXPECT_EQ(testd, des4.value());
  enum class d1 : std::uint8_t { red, bile };
  auto des41 = serialize::deserialize<d1>(serialize::serialize(d1::red));
  EXPECT_EQ(d1::red, des41.value());
  std::bitset<100> bitset;
  bitset.set(99);
  // auto buffer = serialize::serialize(bitset);
  // for (auto& buf : buffer) {
  //   std::cout << (int)buf << " ";
  // }
  auto des5 =
      serialize::deserialize<std::bitset<100>>(serialize::serialize(bitset));
  EXPECT_EQ(bitset, des5.value());
  std::string str{"hello"};
  auto des6 = serialize::deserialize<std::string>(serialize::serialize(str));
  EXPECT_EQ(str, des6.value());
  std::vector<int> vec_int{1, 2, 3, 5, 6, 7, 8, 9};
  auto des7 =
      serialize::deserialize<std::vector<int>>(serialize::serialize(vec_int));
  EXPECT_EQ(vec_int, des7.value());
  std::vector<std::string> vec_str{"hello", "world", "IIIIIIIII"};
  auto des8 = serialize::deserialize<std::vector<std::string>>(
      serialize::serialize(vec_str));

  std::pair<int, double> pair1{1, 2.0};
  auto des9 = serialize::deserialize<std::pair<int, double>>(
      serialize::serialize(pair1));
  EXPECT_EQ(pair1, des9.value());
  std::pair<int, std::string> pair2{1, "yyyy"};
  auto des10 = serialize::deserialize<std::pair<int, std::string>>(
      serialize::serialize(pair2));
  EXPECT_EQ(pair2, des10.value());

  std::map<int, double> map1{{1, 2.0}, {3, 6.0}};
  auto des11 =
      serialize::deserialize<std::map<int, double>>(serialize::serialize(map1));
  EXPECT_EQ(map1, des11.value());
  std::map<int, std::string> map2{{1, "yyyy"}, {2, "cccc"}};
  auto des12 = serialize::deserialize<std::map<int, std::string>>(
      serialize::serialize(map2));
  EXPECT_EQ(map2, des12.value());

  // struct
  person1 p1{1, 1.2, 2};
  EXPECT_EQ(serialize::deserialize<person1>(serialize::serialize(p1)).value(),
            p1);
  person2 p2{1, 1.2, {1, 2}};
  EXPECT_EQ(serialize::deserialize<person2>(serialize::serialize(p2)).value(),
            p2);
  person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  EXPECT_EQ(serialize::deserialize<person3>(serialize::serialize(p3)).value(),
            p3);
}