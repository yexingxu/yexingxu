#include "entity/locking_policy.hpp"

#include <spdlog/spdlog.h>

namespace shm {
namespace entity {

ThreadSafePolicy::ThreadSafePolicy() noexcept {
  MutexBuilder().create(m_mutex);
}

void ThreadSafePolicy::lock() const noexcept {
  if (!m_mutex->lock()) {
    spdlog::error(
        "Locking of an inter-process mutex failed! This indicates that the "
        "application holding the lock "
        "was terminated or the resources were cleaned up by RouDi due to "
        "an unresponsive application.");
    // errorHandler(PoshError::POPO__CHUNK_LOCKING_ERROR, ErrorLevel::FATAL);
  }
}

void ThreadSafePolicy::unlock() const noexcept {
  if (!m_mutex->unlock()) {
    spdlog::error(
        "Unlocking of an inter-process mutex failed! This indicates that "
        "the resources were cleaned up "
        "by RouDi due to an unresponsive application.");
    // errorHandler(PoshError::POPO__CHUNK_UNLOCKING_ERROR, ErrorLevel::FATAL);
  }
}

bool ThreadSafePolicy::tryLock() const noexcept {
  auto tryLockResult = m_mutex->try_lock();
  if (!tryLockResult.has_value()) {
    // errorHandler(PoshError::POPO__CHUNK_TRY_LOCK_ERROR, ErrorLevel::FATAL);
  }
  return *tryLockResult == MutexTryLock::LOCK_SUCCEEDED;
}

void SingleThreadedPolicy::lock() const noexcept {}

void SingleThreadedPolicy::unlock() const noexcept {}

bool SingleThreadedPolicy::tryLock() const noexcept { return true; }

}  // namespace entity
}  // namespace shm