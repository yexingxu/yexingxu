/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 18:30:38
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-07 23:06:30
 */
#include "serialize/type_id.hpp"

#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

TEST(SerializerTest, TypeIdTest) {
  EXPECT_EQ(serialize::details::get_type_id<std::uint8_t>(),
            serialize::details::type_id::uint8_t);
  EXPECT_EQ(serialize::details::get_type_id<std::uint16_t>(),
            serialize::details::type_id::uint16_t);
  EXPECT_EQ(serialize::details::get_type_id<std::uint32_t>(),
            serialize::details::type_id::uint32_t);
  EXPECT_EQ(serialize::details::get_type_id<std::uint64_t>(),
            serialize::details::type_id::uint64_t);
  EXPECT_EQ(serialize::details::get_type_id<std::int8_t>(),
            serialize::details::type_id::int8_t);
  EXPECT_EQ(serialize::details::get_type_id<std::int16_t>(),
            serialize::details::type_id::int16_t);
  EXPECT_EQ(serialize::details::get_type_id<std::int32_t>(),
            serialize::details::type_id::int32_t);
  EXPECT_EQ(serialize::details::get_type_id<std::int64_t>(),
            serialize::details::type_id::int64_t);
  EXPECT_EQ(serialize::details::get_type_id<__int128_t>(),
            serialize::details::type_id::int128_t);
  EXPECT_EQ(serialize::details::get_type_id<__uint128_t>(),
            serialize::details::type_id::uint128_t);

  EXPECT_EQ(serialize::details::get_type_id<char>(),
            serialize::details::type_id::char_8_t);
  EXPECT_EQ(serialize::details::get_type_id<char32_t>(),
            serialize::details::type_id::char_32_t);
  EXPECT_EQ(serialize::details::get_type_id<char16_t>(),
            serialize::details::type_id::char_16_t);
  EXPECT_EQ(serialize::details::get_type_id<wchar_t>(),
            serialize::details::type_id::w_char_t);

  EXPECT_EQ(serialize::details::get_type_id<bool>(),
            serialize::details::type_id::bool_t);

  EXPECT_EQ(serialize::details::get_type_id<float>(),
            serialize::details::type_id::float32_t);
  EXPECT_EQ(serialize::details::get_type_id<double>(),
            serialize::details::type_id::float64_t);
  EXPECT_EQ(serialize::details::get_type_id<long double>(),
            serialize::details::type_id::float128_t);

  enum Color { red, blue };
  EXPECT_EQ(serialize::details::get_type_id<Color>(),
            serialize::details::type_id::uint32_t);
  enum class Color1 : std::uint16_t { red = 1, blue };
  EXPECT_EQ(serialize::details::get_type_id<Color1>(),
            serialize::details::type_id::uint16_t);

  EXPECT_EQ(serialize::details::get_type_id<std::string>(),
            serialize::details::type_id::string_t);

  using map_int = std::map<int, int>;
  using mmap_int = std::multimap<int, int>;
  using umap_int = std::unordered_multimap<int, int>;
  EXPECT_EQ(serialize::details::get_type_id<map_int>(),
            serialize::details::type_id::map_container_t);
  EXPECT_EQ(serialize::details::get_type_id<mmap_int>(),
            serialize::details::type_id::map_container_t);
  EXPECT_EQ(serialize::details::get_type_id<umap_int>(),
            serialize::details::type_id::map_container_t);

  EXPECT_EQ(serialize::details::get_type_id<std::set<int>>(),
            serialize::details::type_id::set_container_t);
  using tuple_t = std::tuple<int, double, int>;
  EXPECT_EQ(serialize::details::get_type_id<tuple_t>(),
            serialize::details::type_id::tuple_t);

  using array_int = std::array<int, 16>;
  EXPECT_EQ(serialize::details::get_type_id<array_int>(),
            serialize::details::type_id::array_t);
  using array_str = std::array<std::string, 16>;
  EXPECT_EQ(serialize::details::get_type_id<array_str>(),
            serialize::details::type_id::array_t);
  EXPECT_EQ(serialize::details::get_type_id<std::vector<int>>(),
            serialize::details::type_id::container_t);  // TODO
  EXPECT_EQ(serialize::details::get_type_id<std::list<int>>(),
            serialize::details::type_id::container_t);
  using pair_int = std::pair<int, int>;
  EXPECT_EQ(serialize::details::get_type_id<pair_int>(),
            serialize::details::type_id::struct_t);
  struct ST {
    int a;
    double b;
  };
  EXPECT_EQ(serialize::details::get_type_id<ST>(),
            serialize::details::type_id::struct_t);
}