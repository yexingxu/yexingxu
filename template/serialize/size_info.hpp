/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 11:04:08
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 11:04:12
 */
#pragma once
#include <algorithm>
struct size_info {
  std::size_t total;
  std::size_t size_cnt;
  std::size_t max_size;
  constexpr size_info &operator+=(const size_info &other) {
    this->total += other.total;
    this->size_cnt += other.size_cnt;
    this->max_size = (std::max)(this->max_size, other.max_size);
    return *this;
  }
  constexpr size_info operator+(const size_info &other) {
    return {this->total + other.total, this->size_cnt += other.size_cnt,
            (std::max)(this->max_size, other.max_size)};
  }
};