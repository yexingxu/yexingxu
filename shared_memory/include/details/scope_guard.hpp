/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-15 10:59:43
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-15 14:05:22
 */

#pragma once

#include <cstdint>
#include <functional>

#include "optional.hpp"
namespace shm {

/// @brief The ScopeGuard class is a simple helper class to apply the C++ RAII
///             idiom quickly. You set 2 functions, one which is called in the
///             constructor and another function is called in the destructor
///             which can be useful when handling resources.
/// @tparam [in] CleanupCapacity The static storage capacity available to store
/// a cleanup callable in bytes
/// @code
/// // This example leads to a console output of:
/// // hello world
/// // I am doing stuff
/// // goodbye
/// void someFunc() {
///     ScopeGuard myScopeGuard{[](){ IOX_LOG(INFO, "hello world\n"); },
///                 [](){ IOX_LOG(INFO, "goodbye"; }});
///     IOX_LOG(INFO, "I am doing stuff");
///     // myScopeGuard goes out of scope here and the cleanupFunction is called
///     in the
///     // destructor
/// }
/// @endcode
class ScopeGuard final {
 public:
  /// @brief constructor which creates ScopeGuard that calls only the
  /// cleanupFunction on destruction
  /// @param[in] cleanupFunction callable which will be called in the destructor
  explicit ScopeGuard(const std::function<void()>& cleanupFunction) noexcept
      : m_cleanupFunction(cleanupFunction) {}

  /// @brief constructor which calls initFunction and stores the cleanupFunction
  /// which will be
  ///           called in the destructor
  /// @param[in] initFunction callable which will be called in the constructor
  /// @param[in] cleanupFunction callable which will be called in the destructor
  ScopeGuard(const std::function<void()> initFunction,
             const std::function<void()>& cleanupFunction) noexcept
      : ScopeGuard(cleanupFunction) {
    initFunction();
  }

  /// @brief calls m_cleanupFunction callable if it was set in the constructor
  ~ScopeGuard() noexcept { destroy(); }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;

  /// @brief move constructor which moves a ScopeGuard object without calling
  /// the cleanupFunction

  ScopeGuard(ScopeGuard&& rhs) noexcept { *this = std::move(rhs); }

  /// @brief move assignment which moves a ScopeGuard object without calling the
  /// cleanupFunction
  ScopeGuard& operator=(ScopeGuard&& rhs) noexcept {
    if (this != &rhs) {
      destroy();
      m_cleanupFunction = rhs.m_cleanupFunction;
      rhs.m_cleanupFunction.reset();
    }
    return *this;
  }

 private:
  void destroy() noexcept {
    if (m_cleanupFunction) {
      m_cleanupFunction.value()();
    }
  }

 private:
  tl::optional<std::function<void()>> m_cleanupFunction;
};

}  // namespace shm