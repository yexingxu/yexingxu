

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <fstream>
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

  person1 p1{1, 1.2, 2};
  EXPECT_EQ(serialize::serialize<serialize_props>(p1).size(), 24);
  person2 p2{1, 1.2, {1, 2}};
  EXPECT_EQ(serialize::serialize<serialize_props>(p2).size(), 32);
  person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  EXPECT_EQ(serialize::serialize<serialize_props>(p3).size(), 64);

  EXPECT_EQ(serialize::serialize(p1).size(), 16);
  EXPECT_EQ(serialize::serialize(p2).size(), 24);
  EXPECT_EQ(serialize::serialize(p3).size(), 40);

  std::ofstream writer("struct_pack_demo.data",
                       std::ofstream::out | std::ofstream::binary);
  auto rr = serialize::serialize<serialize_props, std::string>(p3);
  std::string result = "The next line is struct_pack serialize result.\n";
  serialize::serialize_to<serialize_props>(result, p3);

  // std::stringstream ss;
  // std::string str{"hello"};
  // auto buffer_str = serialize::serialize(str);
  // //   std::cout << buffer_str.size() << "---cc" << std::endl;
  // for (auto b : buffer_str) {
  //   ss << b;
  // }
  // std::cout << buffer_str.size() << "---cc-- " << ss.str() << std::endl;
  // ss.clear();
  // int a = 4;
  // auto buffer = serialize::serialize(a);
  // for (auto b : buffer) {
  //   std::cout << (int)b << " ";
  // }
  // std::cout << "---cc-- " << std::endl;

  // std::vector<int> int_vec{1, 2, 3};
  // auto buffer_vec_int = serialize::serialize(int_vec);
  // for (auto b : buffer_vec_int) {
  //   std::cout << (int)b << " ";
  // }
  // std::cout << buffer_vec_int.size() << "---cc-- " << ss.str() << std::endl;
  // ss.clear();
  // std::vector<std::string> str_vec{"a", "bb", "cc"};
  // auto buffer_vec_str = serialize::serialize(str_vec);
  // for (auto b : buffer_vec_str) {
  //   ss << b;
  // }
  // std::cout << "---cc-- " << std::endl;

  // std::map<int, int> map_1{{1, 3}, {2, 4}};
  // auto buffer_map_1 = serialize::serialize(map_1);
  // for (auto b : buffer_map_1) {
  //   std::cout << (int)b << " ";
  // }
  // std::cout << "---cc-- " << std::endl;

  // std::cout << buffer_vec_str.size() << "---cc-- " << ss.str() << std::endl;
  // std::pair<int, double> pair1{1, 2.0};
  // auto buffer_pair1 = serialize::serialize(pair1);
  // std::cout << buffer_pair1.size() << "---cc-- " << std::endl;
}