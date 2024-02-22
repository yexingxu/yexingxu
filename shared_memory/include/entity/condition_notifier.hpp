#pragma once

#include <limits>

#include "entity/condition_variable_data.hpp"

namespace shm {
namespace entity {

/// @brief ConditionNotifier can notifiy waiting threads and processes using a
/// shared memory condition variable
class ConditionNotifier {
 public:
  static constexpr uint64_t INVALID_NOTIFICATION_INDEX =
      std::numeric_limits<uint64_t>::max();

  ConditionNotifier(ConditionVariableData& condVarDataRef,
                    const uint64_t index) noexcept;

  ConditionNotifier(const ConditionNotifier& rhs) = delete;
  ConditionNotifier(ConditionNotifier&& rhs) noexcept = delete;
  ConditionNotifier& operator=(const ConditionNotifier& rhs) = delete;
  ConditionNotifier& operator=(ConditionNotifier&& rhs) noexcept = delete;
  ~ConditionNotifier() noexcept = default;

  /// @brief If threads are waiting on the condition variable, this call
  /// unblocks one of the waiting threads
  void notify() noexcept;

 protected:
  const ConditionVariableData* getMembers() const noexcept;
  ConditionVariableData* getMembers() noexcept;

 private:
  ConditionVariableData* m_condVarDataPtr{nullptr};
  uint64_t m_notificationIndex = INVALID_NOTIFICATION_INDEX;
};

}  // namespace entity
}  // namespace shm