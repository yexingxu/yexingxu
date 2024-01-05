/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 11:04:08
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 10:13:32
 */
#pragma once
#include <algorithm>
struct size_info {
  std::size_t total;
  std::size_t length_field;
  constexpr size_info &operator+=(const size_info &other) {
    this->total += other.total;
    this->length_field += other.length_field;
    return *this;
  }
  constexpr size_info operator+(const size_info &other) {
    return {this->total + other.total,
            this->length_field += other.length_field};
  }
};