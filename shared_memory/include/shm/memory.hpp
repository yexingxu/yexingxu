

#pragma once
#include <cstddef>
#include <cstdint>

namespace shm {
namespace details {

/// @note value + alignment - 1 must not exceed the maximum value for type T
/// @note alignment must be a power of two
template <typename T>
// AXIVION Next Construct AutosarC++19_03-A2.10.5, AutosarC++19_03-M17.0.3 : The
// function is in the 'iox' namespace which prevents easy misuse
T align(const T value, const T alignment) noexcept {
  return (value + (alignment - 1)) & (~alignment + 1);
}

/// @brief allocates aligned memory which can only be free'd by alignedFree
/// @param[in] alignment, alignment of the memory
/// @param[in] size, memory size
/// @return void pointer to the aligned memory
void* alignedAlloc(const uint64_t alignment, const uint64_t size) noexcept;

/// @brief frees aligned memory allocated with alignedAlloc
/// @param[in] memory, pointer to the aligned memory
void alignedFree(void* const memory) noexcept;

/// template recursion stopper for maximum alignment calculation
template <std::size_t S = 0>
// AXIVION Next Construct AutosarC++19_03-A2.10.5 : The function is in the 'iox'
// namespace which prevents easy misuse
constexpr std::size_t maxAlignment() noexcept {
  return S;
}

/// calculate maximum alignment of supplied types
template <typename T, typename... Args>
// AXIVION Next Construct AutosarC++19_03-A2.10.5 : The function is in the 'iox'
// namespace which prevents easy misuse
constexpr std::size_t maxAlignment() noexcept {
  const std::size_t remainingMaxAlignment{maxAlignment<Args...>()};
  const std::size_t currentTypeAligment{alignof(T)};
  return (currentTypeAligment > remainingMaxAlignment) ? currentTypeAligment
                                                       : remainingMaxAlignment;
}

/// template recursion stopper for maximum size calculation
template <std::size_t S = 0>
constexpr std::size_t maxSize() noexcept {
  return S;
}

/// calculate maximum size of supplied types
template <typename T, typename... Args>
constexpr std::size_t maxSize() noexcept {
  return (sizeof(T) > maxSize<Args...>()) ? sizeof(T) : maxSize<Args...>();
}

}  // namespace details
}  // namespace shm