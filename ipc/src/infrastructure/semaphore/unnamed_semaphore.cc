
#include "infrastructure/semaphore/unnamed_semaphore.h"

#include "plog/Log.h"
#include "utils/system_call.h"

namespace ipc {
namespace infra {

namespace {
SemaphoreError ErrnoToEnum(const int32_t errnum) noexcept {
  switch (errnum) {
    case EINVAL:
      LOG_ERROR()
          << "The semaphore handle is no longer valid. This can indicate a "
             "corrupted system.";
      return SemaphoreError::INVALID_SEMAPHORE_HANDLE;
    case EOVERFLOW:
      LOG_ERROR() << "Semaphore overflow. The maximum value of " << kSemValueMax
                  << " would be exceeded.";
      return SemaphoreError::SEMAPHORE_OVERFLOW;
    case EINTR:
      LOG_ERROR() << "The semaphore call was interrupted multiple times by the "
                     "operating system. Abort operation!";
      return SemaphoreError::INTERRUPTED_BY_SIGNAL_HANDLER;
    default:
      LOG_ERROR() << "This should never happen. An unknown error occurred.";
      break;
  }
  return SemaphoreError::UNDEFINED;
}
}  // namespace

sem_t* UnnamedSemaphore::GetHandle() noexcept { return &handle_; }

tl::expected<void, SemaphoreError> UnnamedSemaphore::Post() noexcept {
  auto result =
      SYSTEM_CALL(sem_post)(GetHandle()).FailureReturnValue(-1).Evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(ErrnoToEnum(result.error().errnum));
  }

  return {};
}

tl::expected<SemaphoreWaitState, SemaphoreError> UnnamedSemaphore::TimedWait(
    const Duration& timeout) noexcept {
  const timespec timeoutAsTimespec =
      timeout.Timespec(TimeSpecReference::kEpoch);
  auto result = SYSTEM_CALL(sem_timedwait)(GetHandle(), &timeoutAsTimespec)
                    .FailureReturnValue(-1)
                    .IgnoreErrnos(ETIMEDOUT)
                    .Evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(ErrnoToEnum(result.error().errnum));
  }

  return (result.value().errnum == ETIMEDOUT) ? SemaphoreWaitState::TIMEOUT
                                              : SemaphoreWaitState::NO_TIMEOUT;
}

tl::expected<bool, SemaphoreError> UnnamedSemaphore::TryWait() noexcept {
  auto result = SYSTEM_CALL(sem_trywait)(GetHandle())
                    .FailureReturnValue(-1)
                    .IgnoreErrnos(EAGAIN)
                    .Evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(ErrnoToEnum(result.error().errnum));
  }

  return result.value().errnum != EAGAIN;
}

tl::expected<void, SemaphoreError> UnnamedSemaphore::Wait() noexcept {
  auto result =
      SYSTEM_CALL(sem_wait)(GetHandle()).FailureReturnValue(-1).Evaluate();

  if (!result.has_value()) {
    return tl::make_unexpected(ErrnoToEnum(result.error().errnum));
  }

  return {};
}

}  // namespace infra
}  // namespace ipc
