/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-04 19:23:33
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 15:51:57
 */

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
