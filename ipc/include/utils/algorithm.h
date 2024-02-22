#pragma once

namespace ipc {

template <typename T>
inline constexpr bool DoesContainValue(const T) noexcept {
  return false;
}

template <typename T1, typename T2, typename... ValueList>
inline constexpr bool DoesContainValue(
    const T1 value, const T2 firstValueListEntry,
    const ValueList... remainingValueListEntries) noexcept {
  return (value == firstValueListEntry)
             ? true
             : DoesContainValue(value, remainingValueListEntries...);
}

}  // namespace ipc