#include "entity/condition_notifier.hpp"

#include <cstdlib>

namespace shm {
namespace entity {

ConditionNotifier::ConditionNotifier(ConditionVariableData& condVarDataRef,
                                     const uint64_t index) noexcept
    : m_condVarDataPtr(&condVarDataRef), m_notificationIndex(index) {
  if (index >= MAX_NUMBER_OF_NOTIFIERS) {
    // TODO
    std::exit(EXIT_FAILURE);
    // IOX_LOG(FATAL,
    //         "The provided index "
    //             << index
    //             << " is too large. The index has to be in the range of [0, "
    //             << MAX_NUMBER_OF_NOTIFIERS << "[.");
    // errorHandler(PoshError::POPO__CONDITION_NOTIFIER_INDEX_TOO_LARGE,
    //              ErrorLevel::FATAL);
  }
}

void ConditionNotifier::notify() noexcept {
  getMembers()->m_activeNotifications[m_notificationIndex].store(
      true, std::memory_order_release);
  getMembers()->m_wasNotified.store(true, std::memory_order_relaxed);
  getMembers()->m_semaphore->post().or_else([](auto) {
    // TODO
    // errorHandler(
    //     PoshError::POPO__CONDITION_NOTIFIER_SEMAPHORE_CORRUPT_IN_NOTIFY,
    //     ErrorLevel::FATAL);
  });
}

const ConditionVariableData* ConditionNotifier::getMembers() const noexcept {
  return m_condVarDataPtr;
}

ConditionVariableData* ConditionNotifier::getMembers() noexcept {
  return m_condVarDataPtr;
}

}  // namespace entity
}  // namespace shm