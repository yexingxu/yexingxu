/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-05 04:05:37
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-12 14:52:02
 */

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "serialize.hpp"
#include "user_defined_struct.h"

TEST(SerializerTest, SerializeTest) {
  // enum color { red, blue };
  // auto buffer_enum = serialize::serialize(color::blue);

  // person1 p1{1, 1.2, 2};
  // EXPECT_EQ(serialize::serialize<serialize_props>(p1).size(), 24);
  // person2 p2{1, 1.2, {1, 2}};
  // EXPECT_EQ(serialize::serialize<serialize_props>(p2).size(), 32);
  // person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  // EXPECT_EQ(serialize::serialize<serialize_props>(p3).size(), 64);

  // EXPECT_EQ(serialize::serialize(p1).size(), 16);
  // EXPECT_EQ(serialize::serialize(p2).size(), 24);
  // EXPECT_EQ(serialize::serialize(p3).size(), 40);

  // std::ofstream writer("struct_pack_demo.data",
  //                      std::ofstream::out | std::ofstream::binary);
  // auto rr = serialize::serialize<serialize_props, std::string>(p3);
  // std::string result = "The next line is struct_pack serialize result.\n";
  // serialize::serialize_to<serialize_props>(result, p3);

  // std::uint16_t a = 1;
  // std::uint8_t b = 1;

  // auto bufferr = serialize::serialize(a, b);
  // EXPECT_EQ(bufferr.size(), 3);

  example::Test t{1, 1.0};
  auto bufferr1 = serialize::serialize(t);
  EXPECT_EQ(bufferr1.size(), 12);
  example1::Test t1{11, 11.0};
  auto bufferr11 = serialize::serialize(t1);
  EXPECT_EQ(bufferr11.size(), 12);
  auto res = serialize::deserialize<example::Test>(bufferr1);
  if (res.has_value()) {
    EXPECT_EQ(res.value(), t);
  }

  auto res1 = serialize::deserialize<example1::Test>(bufferr11);
  if (res1.has_value()) {
    EXPECT_EQ(res1.value(), t1);
  }

  // testclass tc;
  // auto tt1 = FuncImpl<1>()(tc);
  // auto tt0 = FuncImpl<0>()(tc);
  // EXPECT_EQ(1, tt1);
  // EXPECT_EQ(2.0, tt0);
}