
#pragma once

#include <semaphore.h>

#include "sem/duration.hpp"
#include "types/expected.hpp"

namespace shm {
namespace details {

#define SEM_VALUE_MAX (2147483647)

enum class SemaphoreError {
  INVALID_NAME,
  INVALID_SEMAPHORE_HANDLE,
  SEMAPHORE_OVERFLOW,
  INTERRUPTED_BY_SIGNAL_HANDLER,
  PERMISSION_DENIED,
  ALREADY_EXIST,
  FILE_DESCRIPTOR_LIMIT_REACHED,
  NO_SEMAPHORE_WITH_THAT_NAME_EXISTS,
  OUT_OF_MEMORY,
  UNDEFINED
};

enum class SemaphoreWaitState {
  TIMEOUT,
  NO_TIMEOUT,
};

/// @brief Defines the interface of a named and unnamed semaphore.
template <typename SemaphoreChild>
class SemaphoreInterface {
 public:
  SemaphoreInterface(const SemaphoreInterface&) noexcept = delete;
  SemaphoreInterface(SemaphoreInterface&&) noexcept = delete;
  SemaphoreInterface& operator=(const SemaphoreInterface&) noexcept = delete;
  SemaphoreInterface& operator=(SemaphoreInterface&&) noexcept = delete;
  ~SemaphoreInterface() noexcept = default;

  /// @brief Increments the semaphore by one
  /// @return Fails when the value of the semaphore overflows or when the
  ///         semaphore was removed from outside the process
  tl::expected<void, SemaphoreError> post() noexcept;

  /// @brief Decrements the semaphore by one. When the semaphore value is zero
  ///        it blocks until the semaphore value is greater zero
  /// @return Fails when semaphore was removed from outside the process
  tl::expected<void, SemaphoreError> wait() noexcept;

  /// @brief Tries to decrement the semaphore by one. When the semaphore value
  /// is zero
  ///        it returns false otherwise it returns true and decrement the value
  ///        by one.
  /// @return Fails when semaphore was removed from outside the process
  tl::expected<bool, SemaphoreError> tryWait() noexcept;

  /// @brief Tries to decrement the semaphore by one. When the semaphore value
  /// is zero
  ///        it waits until the timeout has passed.
  /// @return If during the timeout time the semaphore value increases to non
  /// zero
  ///         it returns SemaphoreWaitState::NO_TIMEOUT and decreases the
  ///         semaphore by one otherwise returns SemaphoreWaitState::TIMEOUT
  tl::expected<SemaphoreWaitState, SemaphoreError> timedWait(
      const Duration& timeout) noexcept;

 protected:
  SemaphoreInterface() noexcept = default;

 private:
  sem_t* getHandle() noexcept;
};

}  // namespace details
}  // namespace shm