/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 19:11:44
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-12 13:21:02
 */

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "serialize.hpp"
#include "user_defined_struct.h"

TEST(SerializerTest, DeserializeTest) {
  std::uint8_t a = 1;
  auto des = serialize::deserialize<std::uint8_t>(serialize::serialize(a));
  if (des.has_value()) {
    EXPECT_EQ(a, des.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  double b = 2.0;
  auto des2 = serialize::deserialize<double>(serialize::serialize(b));
  if (des2.has_value()) {
    EXPECT_EQ(b, des2.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  bool c = true;
  auto des3 = serialize::deserialize<bool>(serialize::serialize(c));
  if (des3.has_value()) {
    EXPECT_EQ(c, des3.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  enum d { red, bile };
  d testd = d::red;
  auto des4 = serialize::deserialize<d>(serialize::serialize(testd));
  if (des4.has_value()) {
    EXPECT_EQ(testd, des4.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  enum class d1 : std::uint8_t { red, bile };
  auto des41 = serialize::deserialize<d1>(serialize::serialize(d1::red));
  if (des41.has_value()) {
    EXPECT_EQ(d1::red, des41.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::bitset<100> bitset;
  bitset.set(99);

  auto des5 =
      serialize::deserialize<std::bitset<100>>(serialize::serialize(bitset));
  if (des5.has_value()) {
    EXPECT_EQ(bitset, des5.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::string str{"hello"};
  auto des6 = serialize::deserialize<std::string>(serialize::serialize(str));
  if (des6.has_value()) {
    EXPECT_EQ(str, des6.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::vector<int> vec_int{1, 2, 3, 5, 6, 7, 8, 9};
  auto des7 =
      serialize::deserialize<std::vector<int>>(serialize::serialize(vec_int));
  if (des7.has_value()) {
    EXPECT_EQ(vec_int, des7.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::vector<std::string> vec_str{"hello", "world", "IIIIIIIII"};
  auto des8 = serialize::deserialize<std::vector<std::string>>(
      serialize::serialize(vec_str));
  if (des8.has_value()) {
    EXPECT_EQ(vec_str, des8.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::pair<int, double> pair1{1, 2.0};
  auto des9 = serialize::deserialize<std::pair<int, double>>(
      serialize::serialize(pair1));
  if (des9.has_value()) {
    EXPECT_EQ(pair1, des9.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::pair<int, std::string> pair2{1, "yyyy"};
  auto des10 = serialize::deserialize<std::pair<int, std::string>>(
      serialize::serialize(pair2));
  if (des10.has_value()) {
    EXPECT_EQ(pair2, des10.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  std::map<int, double> map1{{1, 2.0}, {3, 6.0}};
  auto des11 =
      serialize::deserialize<std::map<int, double>>(serialize::serialize(map1));
  if (des11.has_value()) {
    EXPECT_EQ(map1, des11.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  std::map<int, std::string> map2{{1, "yyyy"}, {2, "cccc"}};
  auto des12 = serialize::deserialize<std::map<int, std::string>>(
      serialize::serialize(map2));
  if (des12.has_value()) {
    EXPECT_EQ(map2, des12.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  // struct
  person1 p1{1, 1.2, 2};
  auto des13 = serialize::deserialize<person1>(serialize::serialize(p1));
  if (des13.has_value()) {
    EXPECT_EQ(p1, des13.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  person2 p2{1, 1.2, {1, 2}};
  auto des14 = serialize::deserialize<person2>(serialize::serialize(p2));
  if (des14.has_value()) {
    EXPECT_EQ(p2, des14.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  auto des15 = serialize::deserialize<person3>(serialize::serialize(p3));
  if (des15.has_value()) {
    EXPECT_EQ(p3, des15.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  person3 p4{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  auto des16 = serialize::deserialize<serialize_props, person3>(
      serialize::serialize<serialize_props>(p4));
  auto des116 = serialize::deserialize<person3>(serialize::serialize(p4));
  if (des16.has_value()) {
    EXPECT_EQ(p4, des16.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  person1 p11{1, 1.2, 2};
  auto tuple1 = std::make_tuple(1, 1.2, 2);
  auto des132 =
      serialize::deserialize<int, double, int>(serialize::serialize(p11));
  if (des132.has_value()) {
    EXPECT_EQ(tuple1, des132.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  using tuple_t =
      std::tuple<int, double, std::string, std::map<int, std::string>, person3>;

  tuple_t test_tuple{1,
                     2.0,
                     "ttttest",
                     {{1, "ccc"}, {2, "eee"}},
                     {{2, 2.2, 2}, {1, 1.2, {1, 2}}}};
  auto des133 =
      serialize::deserialize<tuple_t>(serialize::serialize(test_tuple));
  if (des133.has_value()) {
    EXPECT_EQ(test_tuple, des133.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }

  using variant_t = mpark::variant<int, person3>;
  variant_t v{p4};
  auto des134 = serialize::deserialize<variant_t>(serialize::serialize(v));
  if (des134.has_value()) {
    EXPECT_EQ(v, des134.value());
  } else {
    std::cout << "error: " << __LINE__ << std::endl;
  }
  int ddd = 2;
  double ccc = 9.0;

  serialize::serialize<serialize_props>(ddd, ccc);

  obj1 o;
  o.ID = 1;
  o.name = "a";
  auto bufferr1e1 = serialize::serialize(o);
  auto resw1 = serialize::deserialize<obj1>(bufferr1e1);
  if (resw1.has_value()) {
    EXPECT_EQ(resw1.value(), o);
  } else {
    EXPECT_EQ(resw1.value(), o);
  }

  auto* o_pointer = &o;
  auto buffer_o_ptr = serialize::serialize(o_pointer);
  auto o_uptr = std::make_unique<obj1>();
  auto res_o_uptr = serialize::deserialize<decltype(o_uptr)>(buffer_o_ptr);
  if (res_o_uptr.has_value()) {
    EXPECT_EQ(*(res_o_uptr.value()), *o_pointer);
  } else {
    std::cout << "error" << std::endl;
  }

  auto o_sptr = std::make_shared<obj1>();
  auto res_o_sptr = serialize::deserialize<decltype(o_sptr)>(buffer_o_ptr);
  if (res_o_sptr.has_value()) {
    EXPECT_EQ(*(res_o_sptr.value()), *o_pointer);
  } else {
    std::cout << "error" << std::endl;
  }
}