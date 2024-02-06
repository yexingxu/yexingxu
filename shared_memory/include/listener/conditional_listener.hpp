

#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "listener/conditional_verialble_data.hpp"

namespace shm {
namespace details {

class ConditionListener {
 public:
  using NotificationVector_t = std::vector<uint8_t>;

  explicit ConditionListener(ConditionVariableData& condVarData) noexcept;
  ~ConditionListener() noexcept = default;
  ConditionListener(const ConditionListener& rhs) = delete;
  ConditionListener(ConditionListener&& rhs) noexcept = delete;
  ConditionListener& operator=(const ConditionListener& rhs) = delete;
  ConditionListener& operator=(ConditionListener&& rhs) noexcept = delete;

  /// @brief Was the ConditionListener notified by a ConditionNotifier?
  /// @return true if it was notified otherwise false
  bool wasNotified() const noexcept;

  /// @brief Used in classes to signal a thread which waits in wait() to return
  ///         and stop working. Destroy will send an empty notification to
  ///         wait() and after this call wait() turns into a non blocking call
  ///         which always returns an empty vector.
  void destroy() volatile noexcept;

  /// @brief returns a sorted vector of indices of active notifications;
  /// blocking if ConditionVariableData was not notified unless destroy() was
  /// called before. The indices of active notifications are never empty unless
  /// destroy() was called, then it's always empty.
  ///
  /// @return a sorted vector of active notifications
  NotificationVector_t wait() noexcept;

  /// @brief returns a sorted vector of indices of active notifications;
  /// blocking for the specified time if ConditionVariableData was not notified
  /// unless destroy() was called before. The indices of active notifications
  /// can be empty (spurious wakeups). When destroy() was called then it is
  /// always empty.
  /// @param[in] timeToWait duration how long at most this method should wait
  ///
  /// @return a sorted vector of active notifications
  NotificationVector_t timedWait(const Duration& timeToWait) noexcept;

 protected:
  const ConditionVariableData* getMembers() volatile const noexcept;
  ConditionVariableData* getMembers() volatile noexcept;

 private:
  void resetUnchecked(const uint64_t index) noexcept;
  void resetSemaphore() noexcept;

  NotificationVector_t waitImpl(const std::function<bool()>& waitCall) noexcept;

 private:
  ConditionVariableData* m_condVarDataPtr{nullptr};
  std::atomic_bool m_toBeDestroyed{false};
};

}  // namespace details
}  // namespace shm