#pragma once

#include "plog/Log.h"

namespace ipc {
namespace internal {
inline void Require(const bool condition, const char* file, const int line,
                    const char* function,
                    const char* conditionString) noexcept {
  if (!condition) {
    LOG_ERROR() << "Condition: " << conditionString << " in " << function
                << " is violated. (" << file << ":" << line << ")";
    std::exit(EXIT_FAILURE);
  }
}
}  // namespace internal
}  // namespace ipc

#define EXPECTS(condition)                                                   \
  ipc::internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                         #condition)

#define ENSURES(condition)                                                   \
  ipc::internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                         #condition)
