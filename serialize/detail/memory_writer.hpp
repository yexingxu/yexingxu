/*
 * @Description: * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-07 15:58:03
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 20:51:00
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <iostream>

namespace serialize {
namespace detail {
struct memory_writer {
  char *buffer;
  void write(const char *data, std::size_t len) {
    std::memcpy(buffer, data, len);
    buffer += len;
  }
};

}  // namespace detail
}  // namespace serialize