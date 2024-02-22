#include "sem/semaphore_interface.hpp"

#include <string>

#include "sem/unnamed_semaphore.hpp"
#include "shm/system_call.hpp"

namespace shm {
namespace details {

SemaphoreError errnoToEnum(const int32_t errnum) noexcept {
  switch (errnum) {
    case EINVAL:
      spdlog::error(
          "The semaphore handle is no longer valid. This can indicate a "
          "corrupted system.");
      return SemaphoreError::INVALID_SEMAPHORE_HANDLE;
    case EOVERFLOW:
      spdlog::error("Semaphore overflow. The maximum value of " +
                    std::to_string(SEM_VALUE_MAX) + " would be exceeded.");
      return SemaphoreError::SEMAPHORE_OVERFLOW;
    case EINTR:
      spdlog::error(
          "The semaphore call was interrupted multiple times by the "
          "operating system. Abort operation!");
      return SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER;
    default:
      spdlog::error("This should never happen. An unknown error occurred.");
      break;
  }
  return SemaphoreError::UNDEFINED;
}

template <typename SemaphoreChild>
sem_t* SemaphoreInterface<SemaphoreChild>::getHandle() noexcept {
  return static_cast<SemaphoreChild*>(this)->getHandle();
}

template <typename SemaphoreChild>
tl::expected<void, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::post() noexcept {
  auto result =
      SYSTEM_CALL(sem_post)(getHandle()).failureReturnValue(-1).evaluate();

  if (result.has_value()) {
    return tl::make_unexpected(errnoToEnum(result.error().errnum));
  }

  return {};
}

template <typename SemaphoreChild>
tl::expected<SemaphoreWaitState, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::timedWait(
    const Duration& timeout) noexcept {
  const timespec timeoutAsTimespec = timeout.timespec(TimeSpecReference::Epoch);
  auto result = SYSTEM_CALL(sem_timedwait)(getHandle(), &timeoutAsTimespec)
                    .failureReturnValue(-1)
                    .ignoreErrnos(ETIMEDOUT)
                    .evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(errnoToEnum(result.error().errnum));
  }

  return (result.value().errnum == ETIMEDOUT) ? SemaphoreWaitState::TIMEOUT
                                              : SemaphoreWaitState::NO_TIMEOUT;
}

template <typename SemaphoreChild>
tl::expected<bool, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::tryWait() noexcept {
  auto result = SYSTEM_CALL(sem_trywait)(getHandle())
                    .failureReturnValue(-1)
                    .ignoreErrnos(EAGAIN)
                    .evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(errnoToEnum(result.error().errnum));
  }

  return result.value().errnum != EAGAIN;
}

template <typename SemaphoreChild>
tl::expected<void, SemaphoreError>
SemaphoreInterface<SemaphoreChild>::wait() noexcept {
  auto result =
      SYSTEM_CALL(sem_wait)(getHandle()).failureReturnValue(-1).evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(errnoToEnum(result.error().errnum));
  }

  return {};
}

template class SemaphoreInterface<UnnamedSemaphore>;

}  // namespace details

}  // namespace shm