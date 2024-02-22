#include "sem/unnamed_semaphore.hpp"

#include <spdlog/spdlog.h>

#include <string>

#include "shm/system_call.hpp"
#include "types/expected.hpp"

namespace shm {
namespace details {

tl::expected<void, SemaphoreError> UnnamedSemaphoreBuilder::create(
    tl::optional<UnnamedSemaphore>& uninitializedSemaphore) const noexcept {
  if (m_initialValue > SEM_VALUE_MAX) {
    spdlog::error("The unnamed semaphore initial value of " +
                  std::to_string(m_initialValue) +
                  " exceeds the maximum semaphore value " +
                  std::to_string(SEM_VALUE_MAX));
    return tl::make_unexpected(SemaphoreError::SEMAPHORE_OVERFLOW);
  }

  //   uninitializedSemaphore.emplace();

  auto result = SYSTEM_CALL(sem_init)(&uninitializedSemaphore.value().m_handle,
                                      (m_isInterProcessCapable) ? 1 : 0,
                                      static_cast<unsigned int>(m_initialValue))
                    .failureReturnValue(-1)
                    .evaluate();

  if (!result.has_value()) {
    uninitializedSemaphore.value().m_destroyHandle = false;
    uninitializedSemaphore.reset();

    switch (result.error().errnum) {
      case EINVAL:
        spdlog::error("The initial value of " + std::to_string(m_initialValue) +
                      " exceeds " + std::to_string(SEM_VALUE_MAX));
        break;
      case ENOSYS:
        spdlog::error("The system does not support process-shared semaphores");
        break;
      default:
        spdlog::error("This should never happen. An unknown error occurred.");
        break;
    }
  }

  return {};
}

UnnamedSemaphore::~UnnamedSemaphore() noexcept {
  if (m_destroyHandle) {
    auto result =
        SYSTEM_CALL(sem_destroy)(getHandle()).failureReturnValue(-1).evaluate();
    if (!result.has_value()) {
      switch (result.error().errnum) {
        case EINVAL:
          spdlog::error(
              "The semaphore handle was no longer valid. This can indicate "
              "a corrupted system.");
          break;
        default:
          spdlog::error("This should never happen. An unknown error occurred.");
          break;
      }
    }
  }
}

sem_t* UnnamedSemaphore::getHandle() noexcept { return &m_handle; }

}  // namespace details
}  // namespace shm