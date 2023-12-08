/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-08 02:47:46
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 09:58:56
 */
#include <gtest/gtest.h>

#include <array>
#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "serialize/alignment.hpp"

namespace test_alignas {

template <typename A, typename B>
bool operator==(const A& a, const B& b) {
  return a.a == b.a && a.b == b.b;
}
struct dummy {
  char a;
  short b;
};
}  // namespace test_alignas

TEST(SerializerTest, NoAlignTest) {
  using T = test_alignas::dummy;
  static_assert(std::alignment_of<T>::value == 2, "");
  static_assert(serialize::pack_alignment_v<T> == 0, "");
  // static_assert(serialize::details::align::pack_alignment_v<T> == 2, "");
  // static_assert(serialize::details::align::alignment_v<T> == 2, "");
  static_assert(alignof(T) == 2, "");
  static_assert(sizeof(T) == 4, "");
  static_assert(offsetof(T, a) == 0, "");
  static_assert(offsetof(T, b) == 2, "");
  //   T t{'a', 666};
  //   auto literal = serialize::get_type_literal<T>();
  //   string_literal<char, 6> val{
  //       {(char)-3, 12, 7, (char)131, (char)131, (char)-1}};
  //   REQUIRE(literal == val);
  //   auto buf = serialize::serialize(t);
  //   auto ret = serialize::deserialize<T>(buf);
  //   REQUIRE_MESSAGE(ret, serialize::error_message(ret.error()));
  //   T d_t = ret.value();
  //   CHECK(t == d_t);
}