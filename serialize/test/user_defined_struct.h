/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-04 19:23:33
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-11 14:49:47
 */

#include <cstdint>

#include "detail/reflection.hpp"
#include "ser_config.hpp"

#pragma once

struct serialize_props {
  using LengthFieldType = std::uint8_t;
  using AlignmentType = serialize::alignment;
  using ByteOrderType = serialize::byte_order;
  static constexpr AlignmentType kAlignment = serialize::alignment::kAlignment4;
  static constexpr ByteOrderType kByteOrder =
      serialize::byte_order::kLittleEndian;
  static constexpr LengthFieldType kStringLengthField = 2u;
  static constexpr LengthFieldType kArrayLengthField = 4u;
  static constexpr LengthFieldType kUnionLengthField = 1u;
  static constexpr LengthFieldType kStructLengthField = 8u;
};

// struct
struct person1 {
  int age;
  double weight;
  int add;

  constexpr bool operator==(const person1& other) const {
    return age == other.age && weight == other.weight && add == other.add;
  }
};
SERIALIZE_REFL(person1, age, weight, add);

struct person2 {
  int age;
  double weight;
  std::vector<int> name;

  constexpr bool operator==(const person2& other) const {
    return age == other.age && weight == other.weight && name == other.name;
  }
};

SERIALIZE_REFL(person2, age, weight, name);

struct person3 {
  person1 p1;
  person2 p2;

  constexpr bool operator==(const person3& other) const {
    return p1 == other.p1 && p2 == other.p2;
  }
};
SERIALIZE_REFL(person3, p1, p2);

struct B {
  std::uint8_t a;
  std::uint16_t b;
};
SERIALIZE_REFL(B, a, b);

struct A {
  std::uint8_t a;
  std::uint16_t b;
  std::uint32_t c;
  B bb;
  std::uint64_t d;
};

SERIALIZE_REFL(A, a, b, c, bb, d);

struct AA {
  int aa;
  std::string bb;
  uint16_t cc;
  std::string be;
  uint32_t dd;
  std::string fe;
};
SERIALIZE_REFL(AA, aa, bb, cc, be, dd, fe);

namespace example {
struct Test {
  int a;
  double b;
  constexpr bool operator==(const Test& other) const {
    return a == other.a && b == other.b;
  }
};
SERIALIZE_REFL(Test, a, b);
}  // namespace example

namespace example1 {
struct Test {
  int a;
  double b;
  constexpr bool operator==(const Test& other) const {
    return a == other.a && b == other.b;
  }
};
SERIALIZE_REFL(Test, a, b);
}  // namespace example1