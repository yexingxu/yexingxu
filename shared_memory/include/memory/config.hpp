

#pragma once

#include <cstdint>
#include <vector>
namespace shm {
namespace memory {

struct Config {
  static constexpr uint32_t MAX_NUMBER_OF_MEMPOOLS = static_cast<uint32_t>(32);

 public:
  struct Entry {
    /// @brief set the size and count of memory chunks
    Entry(uint32_t f_size, uint32_t f_chunkCount) noexcept
        : m_size(f_size), m_chunkCount(f_chunkCount) {}
    uint32_t m_size{0};
    uint32_t m_chunkCount{0};
  };

  using ConfigContainerType = std::vector<Entry>;
  ConfigContainerType m_mempoolConfig;

  /// @brief Default constructor to set the configuration for memory pools
  Config() noexcept = default;

  /// @brief Get function for receiving memory pool configuration
  /// @return cxx::vector of config information size and count of chunks
  const ConfigContainerType* getMemPoolConfig() const noexcept;

  /// @brief Function for adding new entry
  /// @param[in] Entry structure of mempool configuration
  void addMemPool(Entry f_entry) noexcept;

  /// @brief Function for creating default memory pools
  Config& setDefaults() noexcept;

  /// @brief Function for optimizing the size of memory pool according to new
  /// entry
  Config& optimize() noexcept;
};

}  // namespace memory
}  // namespace shm