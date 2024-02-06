#pragma once

#include <mutex>

#include "types/optional.hpp"

namespace shm {
namespace entity {

class ThreadSafePolicy {
 public:
  ThreadSafePolicy() noexcept;

  // needs to be public since we want to use std::lock_guard
  void lock() const noexcept;
  void unlock() const noexcept;
  bool tryLock() const noexcept;

 private:
  mutable tl::optional<std::mutex> m_mutex;
};

class SingleThreadedPolicy {
 public:
  // needs to be public since we want to use std::lock_guard
  void lock() const noexcept;
  void unlock() const noexcept;
  bool tryLock() const noexcept;
};

}  // namespace entity
}  // namespace shm