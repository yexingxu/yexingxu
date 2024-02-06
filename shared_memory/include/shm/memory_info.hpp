

#pragma once

#include <cstdint>
namespace shm {
namespace details {

struct MemoryInfo {
  static constexpr uint32_t DEFAULT_DEVICE_ID{0U};
  static constexpr uint32_t DEFAULT_MEMORY_TYPE{0U};

  // These are intentionally not defined as enum classes for flexibility and
  // extendibility. Currently only the defaults are used. This will change when
  // we support different devices (CPU, GPUs, ...) and other properties that
  // influence how memory is accessed.

  uint32_t deviceId{DEFAULT_DEVICE_ID};
  uint32_t memoryType{DEFAULT_MEMORY_TYPE};

  MemoryInfo(const MemoryInfo&) noexcept = default;
  MemoryInfo(MemoryInfo&&) noexcept = default;
  MemoryInfo& operator=(const MemoryInfo&) noexcept = default;
  MemoryInfo& operator=(MemoryInfo&&) noexcept = default;

  /// @brief creates a MemoryInfo object
  /// @param[in] deviceId specifies the device where the memory is located
  /// @param[in] memoryType encodes additional information about the memory
  explicit MemoryInfo(uint32_t deviceId = DEFAULT_DEVICE_ID,
                      uint32_t memoryType = DEFAULT_MEMORY_TYPE) noexcept;

  /// @brief comparison operator
  /// @param[in] rhs the right hand side of the comparison
  bool operator==(const MemoryInfo& rhs) const noexcept;
};

}  // namespace details
}  // namespace shm