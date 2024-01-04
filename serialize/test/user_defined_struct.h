/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-04 19:23:33
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 22:19:28
 */

#include "detail/reflection.hpp"

#pragma once

// struct
struct person1 {
  int age;
  double weight;
  int add;

  constexpr bool operator==(const person1& other) const {
    return age == other.age && weight == other.weight && add == other.add;
  }
};
STRUCT_PACK_REFL(person1, age, weight, add);

struct person2 {
  int age;
  double weight;
  std::vector<int> name;

  constexpr bool operator==(const person2& other) const {
    return age == other.age && weight == other.weight && name == other.name;
  }
};

STRUCT_PACK_REFL(person2, age, weight, name);

struct person3 {
  person1 p1;
  person2 p2;

  constexpr bool operator==(const person3& other) const {
    return p1 == other.p1 && p2 == other.p2;
  }
};
STRUCT_PACK_REFL(person3, p1, p2);
