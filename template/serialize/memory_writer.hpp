/*
 * @Description: * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-07 15:58:03
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-07 15:58:47
 */

#pragma once

#include <cstddef>
#include <cstring>

namespace serialize {
namespace details {
struct memory_writer {
  char *buffer;
  void write(const char *data, std::size_t len) {
    std::memcpy(buffer, data, len);
    buffer += len;
  }
};

}  // namespace details
}  // namespace serialize