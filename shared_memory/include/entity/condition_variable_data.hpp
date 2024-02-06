#pragma once

#include <atomic>
#include <string>

#include "sem/unnamed_semaphore.hpp"

namespace shm {
constexpr uint32_t MAX_NUMBER_OF_NOTIFIERS = static_cast<uint32_t>(256);

struct ConditionVariableData {
  ConditionVariableData() noexcept;
  explicit ConditionVariableData(const std::string& runtimeName) noexcept;

  ConditionVariableData(const ConditionVariableData& rhs) = delete;
  ConditionVariableData(ConditionVariableData&& rhs) = delete;
  ConditionVariableData& operator=(const ConditionVariableData& rhs) = delete;
  ConditionVariableData& operator=(ConditionVariableData&& rhs) = delete;
  ~ConditionVariableData() noexcept = default;

  tl::optional<details::UnnamedSemaphore> m_semaphore;
  std::string m_runtimeName;
  std::atomic_bool m_toBeDestroyed{false};
  std::array<std::atomic_bool, MAX_NUMBER_OF_NOTIFIERS> m_activeNotifications;
  std::atomic_bool m_wasNotified{false};
};

}  // namespace shm