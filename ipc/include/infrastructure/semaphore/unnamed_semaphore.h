#pragma once

#include <semaphore.h>

#include <cstdint>

#include "infrastructure/duration.h"
#include "types/expected.hpp"
#include "types/optional.hpp"

namespace ipc {
namespace infra {

static constexpr std::uint64_t kSemValueMax = 2147483647U;

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

class UnnamedSemaphore {
 public:
  UnnamedSemaphore(const UnnamedSemaphore&) noexcept = delete;
  UnnamedSemaphore(UnnamedSemaphore&&) noexcept = delete;
  UnnamedSemaphore& operator=(const UnnamedSemaphore&) noexcept = delete;
  UnnamedSemaphore& operator=(UnnamedSemaphore&&) noexcept = delete;
  ~UnnamedSemaphore() noexcept;

  tl::expected<void, SemaphoreError> Post() noexcept;

  tl::expected<void, SemaphoreError> Wait() noexcept;

  tl::expected<bool, SemaphoreError> TryWait() noexcept;

  tl::expected<SemaphoreWaitState, SemaphoreError> TimedWait(
      const Duration& timeout) noexcept;

 private:
  friend class UnnamedSemaphoreBuilder;

  UnnamedSemaphore() noexcept = default;
  sem_t* GetHandle() noexcept;

  sem_t handle_;
  bool destroyHandle_ = true;
};

class UnnamedSemaphoreBuilder {
  uint32_t initialValue_ = 0U;
  bool isInterProcessCapable_ = true;

 public:
  tl::expected<void, SemaphoreError> Create(
      tl::optional<UnnamedSemaphore>& uninitializedSemaphore) const noexcept;
};

}  // namespace infra
}  // namespace ipc