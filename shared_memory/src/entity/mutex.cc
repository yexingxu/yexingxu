#include "entity/mutex.hpp"

#include "shm/system_call.hpp"

namespace shm {

struct MutexAttributes {
 public:
  MutexAttributes() noexcept = default;
  MutexAttributes(const MutexAttributes&) = delete;
  MutexAttributes(MutexAttributes&&) = delete;
  MutexAttributes& operator=(const MutexAttributes&) = delete;
  MutexAttributes& operator=(MutexAttributes&&) = delete;

  ~MutexAttributes() noexcept {
    if (m_attributes) {
      auto destroyResult =
          SYSTEM_CALL(pthread_mutexattr_destroy)(&*m_attributes)
              .returnValueMatchesErrno()
              .evaluate();
      if (!destroyResult.has_value()) {
        spdlog::error(
            "This should never happen. An unknown error occurred while "
            "cleaning up the mutex attributes.");
      }
    }
  }

  tl::expected<void, MutexCreationError> init() noexcept {
    m_attributes.emplace();
    auto result = SYSTEM_CALL(pthread_mutexattr_init)(&*m_attributes)
                      .returnValueMatchesErrno()
                      .evaluate();
    if (!result.has_value()) {
      switch (result.error().errnum) {
        case ENOMEM:
          spdlog::error(
              "Not enough memory to initialize required mutex attributes");
          return tl::make_unexpected(MutexCreationError::INSUFFICIENT_MEMORY);
        default:
          spdlog::error(
              "This should never happen. An unknown error occurred while "
              "initializing the mutex attributes.");
          return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
      }
    }

    return {};
  }

  tl::expected<void, MutexCreationError> enableIpcSupport(
      const bool enableIpcSupport) noexcept {
    auto result =
        SYSTEM_CALL(pthread_mutexattr_setpshared)(
            &*m_attributes,
            static_cast<int>((enableIpcSupport) ? PTHREAD_PROCESS_SHARED
                                                : PTHREAD_PROCESS_PRIVATE))
            .returnValueMatchesErrno()
            .evaluate();
    if (!result.has_value()) {
      switch (result.error().errnum) {
        case ENOTSUP:
          spdlog::error(
              "The platform does not support shared mutex (inter process "
              "mutex)");
          return tl::make_unexpected(
              MutexCreationError::INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM);
        default:
          spdlog::error(
              "This should never happen. An unknown error occurred while "
              "setting up the inter process "
              "configuration.");
          return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
      }
    }

    return {};
  }

  tl::expected<void, MutexCreationError> setType(
      const MutexType mutexType) noexcept {
    auto result = SYSTEM_CALL(pthread_mutexattr_settype)(
                      &*m_attributes, static_cast<int>(mutexType))
                      .returnValueMatchesErrno()
                      .evaluate();
    if (!result.has_value()) {
      spdlog::error(
          "This should never happen. An unknown error occurred while "
          "setting up the mutex type.");
      return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
    }

    return {};
  }

  tl::expected<void, MutexCreationError> setProtocol(
      const MutexPriorityInheritance priorityInheritance) {
    auto result = SYSTEM_CALL(pthread_mutexattr_setprotocol)(
                      &*m_attributes, static_cast<int>(priorityInheritance))
                      .returnValueMatchesErrno()
                      .evaluate();
    if (!result.has_value()) {
      switch (result.error().errnum) {
        case ENOSYS:
          spdlog::error("The system does not support mutex priorities");
          return tl::make_unexpected(
              MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
        case ENOTSUP:
          spdlog::error(
              "The used mutex priority is not supported by the platform");
          return tl::make_unexpected(
              MutexCreationError::USED_PRIORITY_UNSUPPORTED_BY_PLATFORM);
        case EPERM:
          spdlog::error("Insufficient permissions to set mutex priorities");
          return tl::make_unexpected(MutexCreationError::PERMISSION_DENIED);
        default:
          spdlog::error(
              "This should never happen. An unknown error occurred while "
              "setting up the mutex priority.");
          return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
      }
    }

    return {};
  }

  tl::expected<void, MutexCreationError> setPrioCeiling(
      const int32_t priorityCeiling) noexcept {
    auto result = SYSTEM_CALL(pthread_mutexattr_setprioceiling)(
                      &*m_attributes, static_cast<int>(priorityCeiling))
                      .returnValueMatchesErrno()
                      .evaluate();
    if (!result.has_value()) {
      switch (result.error().errnum) {
        case EPERM:
          spdlog::error(
              "Insufficient permissions to set the mutex priority ceiling.");
          return tl::make_unexpected(MutexCreationError::PERMISSION_DENIED);
        case ENOSYS:
          spdlog::error(
              "The platform does not support mutex priority ceiling.");
          return tl::make_unexpected(
              MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
        case EINVAL: {
          //   auto minimumPriority =
          //       detail::getSchedulerPriorityMinimum(detail::Scheduler::FIFO);
          //   auto maximumPriority =
          //       detail::getSchedulerPriorityMaximum(detail::Scheduler::FIFO);

          //   spdlog::error("The priority ceiling \""
          //                 << priorityCeiling
          //                 << "\" is not in the valid priority range [ "
          //                 << minimumPriority << ", " << maximumPriority
          //                 << "] of the Scheduler::FIFO.");
          return tl::make_unexpected(
              MutexCreationError::INVALID_PRIORITY_CEILING_VALUE);
        }
      }
    }

    return {};
  }

  tl::expected<void, MutexCreationError> setThreadTerminationBehavior(
      const MutexThreadTerminationBehavior behavior) noexcept {
    auto result = SYSTEM_CALL(pthread_mutexattr_setrobust)(
                      &*m_attributes, static_cast<int>(behavior))
                      .returnValueMatchesErrno()
                      .evaluate();
    if (!result.has_value()) {
      spdlog::error(
          "This should never happen. An unknown error occurred while "
          "setting up the mutex thread "
          "termination behavior.");
      return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
    }

    return {};
  }

  tl::optional<pthread_mutexattr_t> m_attributes;
};

tl::expected<void, MutexCreationError> initializeMutex(
    pthread_mutex_t* const handle,
    const pthread_mutexattr_t* const attributes) noexcept {
  auto initResult = SYSTEM_CALL(pthread_mutex_init)(handle, attributes)
                        .returnValueMatchesErrno()
                        .evaluate();
  if (!initResult.has_value()) {
    switch (initResult.error().errnum) {
      case EAGAIN:
        spdlog::error("Not enough resources to initialize another mutex.");
        return tl::make_unexpected(MutexCreationError::INSUFFICIENT_RESOURCES);
      case ENOMEM:
        spdlog::error("Not enough memory to initialize mutex.");
        return tl::make_unexpected(MutexCreationError::INSUFFICIENT_MEMORY);
      case EPERM:
        spdlog::error("Insufficient permissions to create mutex.");
        return tl::make_unexpected(MutexCreationError::PERMISSION_DENIED);
      default:
        spdlog::error(
            "This should never happen. An unknown error occurred while "
            "initializing the mutex handle. "
            "This is possible when the handle is an already initialized "
            "mutex handle.");
        return tl::make_unexpected(MutexCreationError::UNKNOWN_ERROR);
    }
  }

  return {};
}

tl::expected<void, MutexCreationError> MutexBuilder::create(
    tl::optional<mutex>& uninitializedMutex) noexcept {
  if (uninitializedMutex.has_value()) {
    spdlog::error(
        "Unable to override an already initialized mutex with a new mutex");
    return tl::make_unexpected(MutexCreationError::MUTEX_ALREADY_INITIALIZED);
  }

  MutexAttributes mutexAttributes;

  auto result = mutexAttributes.init();
  if (!result.has_value()) {
    return result;
  }

  result = mutexAttributes.enableIpcSupport(m_isInterProcessCapable);
  if (!result.has_value()) {
    return result;
  }

  result = mutexAttributes.setType(m_mutexType);
  if (!result.has_value()) {
    return result;
  }

  result = mutexAttributes.setProtocol(m_priorityInheritance);
  if (!result.has_value()) {
    return result;
  }

  if (m_priorityInheritance == MutexPriorityInheritance::PROTECT &&
      m_priorityCeiling.has_value()) {
    result = mutexAttributes.setPrioCeiling(*m_priorityCeiling);
    if (!result.has_value()) {
      return result;
    }
  }

  result =
      mutexAttributes.setThreadTerminationBehavior(m_threadTerminationBehavior);
  if (!result.has_value()) {
    return result;
  }

  //   uninitializedMutex.emplace();
  uninitializedMutex->m_isDestructable = false;

  result = initializeMutex(&uninitializedMutex->m_handle,
                           &*mutexAttributes.m_attributes);
  if (!result.has_value()) {
    uninitializedMutex.reset();
    return result;
  }

  uninitializedMutex->m_isDestructable = true;
  return {};
}

mutex::~mutex() noexcept {
  if (m_isDestructable) {
    auto destroyCall = SYSTEM_CALL(pthread_mutex_destroy)(&m_handle)
                           .returnValueMatchesErrno()
                           .evaluate();

    if (!destroyCall.has_value()) {
      switch (destroyCall.error().errnum) {
        case EBUSY:
          spdlog::error(
              "Tried to remove a locked mutex which failed. The mutex "
              "handle is now leaked and "
              "cannot be removed anymore!");
          break;
        default:
          spdlog::error(
              "This should never happen. An unknown error occurred while "
              "cleaning up the mutex.");
          break;
      }
    }
  }
}

void mutex::make_consistent() noexcept {
  if (this->m_hasInconsistentState) {
    auto res = SYSTEM_CALL(pthread_mutex_consistent)(&m_handle)
                   .returnValueMatchesErrno()
                   .evaluate();

    if (res.has_value()) {
      this->m_hasInconsistentState = false;
    } else {
      spdlog::error(
          "This should never happen. Unable to put robust mutex in a "
          "consistent state!");
    }
  }
}

tl::expected<void, MutexLockError> mutex::lock() noexcept {
  auto result = SYSTEM_CALL(pthread_mutex_lock)(&m_handle)
                    .returnValueMatchesErrno()
                    .evaluate();
  if (!result.has_value()) {
    switch (result.error().errnum) {
      case EINVAL:
        spdlog::error(
            "The mutex has the attribute MutexPriorityInheritance::PROTECT "
            "set and the calling threads "
            "priority is greater than the mutex priority.");
        return tl::make_unexpected(MutexLockError::PRIORITY_MISMATCH);
      case EAGAIN:
        spdlog::error("Maximum number of recursive locks exceeded.");
        return tl::make_unexpected(
            MutexLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
      case EDEADLK:
        spdlog::error("Deadlock in mutex detected.");
        return tl::make_unexpected(MutexLockError::DEADLOCK_CONDITION);
      case EOWNERDEAD:
        spdlog::error(
            "The thread/process which owned the mutex died. The mutex is "
            "now in an inconsistent state "
            "and must be put into a consistent state again with "
            "Mutex::make_consistent()");
        this->m_hasInconsistentState = true;
        return tl::make_unexpected(
            MutexLockError::
                LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
      default:
        spdlog::error(
            "This should never happen. An unknown error occurred while "
            "locking the mutex. "
            "This can indicate a either corrupted or non-POSIX compliant "
            "system.");
        return tl::make_unexpected(MutexLockError::UNKNOWN_ERROR);
    }
  }
  return {};
}

tl::expected<void, MutexUnlockError> mutex::unlock() noexcept {
  auto result = SYSTEM_CALL(pthread_mutex_unlock)(&m_handle)
                    .returnValueMatchesErrno()
                    .evaluate();
  if (!result.has_value()) {
    switch (result.error().errnum) {
      case EPERM:
        spdlog::error(
            "The mutex is not owned by the current thread. The mutex must "
            "be unlocked by the same "
            "thread it was locked by.");
        return tl::make_unexpected(MutexUnlockError::NOT_OWNED_BY_THREAD);
      default:
        spdlog::error(
            "This should never happen. An unknown error occurred while "
            "unlocking the mutex. "
            "This can indicate a either corrupted or non-POSIX compliant "
            "system.");
        return tl::make_unexpected(MutexUnlockError::UNKNOWN_ERROR);
    }
  }

  return {};
}

tl::expected<MutexTryLock, MutexTryLockError> mutex::try_lock() noexcept {
  auto result = SYSTEM_CALL(pthread_mutex_trylock)(&m_handle)
                    .returnValueMatchesErrno()
                    .ignoreErrnos(EBUSY)
                    .evaluate();

  if (!result.has_value()) {
    switch (result.error().errnum) {
      case EAGAIN:
        spdlog::error("Maximum number of recursive locks exceeded.");
        return tl::make_unexpected(
            MutexTryLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
      case EINVAL:
        spdlog::error(
            "The mutex has the attribute MutexPriorityInheritance::PROTECT "
            "set and the calling threads "
            "priority is greater than the mutex priority.");
        return tl::make_unexpected(MutexTryLockError::PRIORITY_MISMATCH);
      case EOWNERDEAD:
        spdlog::error(
            "The thread/process which owned the mutex died. The mutex is "
            "now in an inconsistent state and must "
            "be put into a consistent state again with "
            "Mutex::make_consistent()");
        this->m_hasInconsistentState = true;
        return tl::make_unexpected(
            MutexTryLockError::
                LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
      default:
        spdlog::error(
            "This should never happen. An unknown error occurred while "
            "trying to lock the mutex. This can "
            "indicate a either corrupted or non-POSIX compliant system.");
        return tl::make_unexpected(MutexTryLockError::UNKNOWN_ERROR);
    }
  }

  return (result->errnum == EBUSY) ? MutexTryLock::FAILED_TO_ACQUIRE_LOCK
                                   : MutexTryLock::LOCK_SUCCEEDED;
}

}  // namespace shm