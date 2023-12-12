

#pragma once

#include <cstddef>
#include <cstring>

namespace serialize {
namespace details {
struct memory_reader {
  const char *now;
  const char *end;
  constexpr memory_reader(const char *b, const char *e) noexcept
      : now(b), end(e) {}
  bool read(char *target, std::size_t len) {
    if (now + len > end) {
      return false;
    }
    std::memcpy(target, now, len);
    now += len;
    return true;
  }
  const char *read_view(std::size_t len) {
    if (now + len > end) {
      return nullptr;
    }
    auto ret = now;
    now += len;
    return ret;
  }
  bool ignore(std::size_t len) {
    if (now + len > end) {
      return false;
    }
    now += len;
    return true;
  }
  std::size_t tellg() { return (std::size_t)now; }
};
}  // namespace details
}  // namespace serialize