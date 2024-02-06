#pragma once

#include "entity/shm_safe_unmanaged_chunk.hpp"
#include "memory/shared_chunk.hpp"

namespace shm {
namespace entity {

/// @brief This class is used to keep track of the chunks currently in use by
/// the application.
///        In case the application terminates while holding chunks, this list is
///        used by RouDi to retain ownership of the chunks and prevent a chunk
///        leak. In order to always be able to access the used chunks, neither a
///        vector or list can be used, because these container could be
///        corrupted when the application dies in the wrong moment. To be able
///        to do the cleanup, RouDi needs to be able to access the list with the
///        used chunk under all circumstances. This is achieved by storing the
///        ChunkManagement pointer in an array which can always be accessed.
///        Additionally, the type stored is this array must be less or equal to
///        64 bit in order to write it within one clock cycle to prevent torn
///        writes, which would corrupt the list and could potentially crash
///        RouDi.
template <uint32_t Capacity>
class UsedChunkList {
  static_assert(Capacity > 0, "UsedChunkList Capacity must be larger than 0!");

 public:
  /// @brief Constructs a default UsedChunkList
  UsedChunkList() noexcept;

  /// @brief Inserts a SharedChunk into the list
  /// @param[in] chunk to store in the list
  /// @return true if successful, otherwise false if e.g. the list is already
  /// full
  /// @note only from runtime context
  bool insert(memory::SharedChunk chunk) noexcept;

  /// @brief Removes a chunk from the list
  /// @param[in] chunkHeader to look for a corresponding SharedChunk
  /// @param[out] chunk which is removed
  /// @return true if successfully removed, otherwise false if e.g. the
  /// chunkHeader was not found in the list
  /// @note only from runtime context
  bool remove(const memory::ChunkHeader* chunkHeader,
              memory::SharedChunk& chunk) noexcept;

  /// @brief Cleans up all the remaining chunks from the list.
  /// @note from RouDi context once the applications walked the plank. It is
  /// unsafe to call this if the application is still running.
  void cleanup() noexcept;

 private:
  void init() noexcept;

 private:
  static constexpr uint32_t INVALID_INDEX{Capacity};

  using DataElement_t = ShmSafeUnmanagedChunk;
  static constexpr DataElement_t DATA_ELEMENT_LOGICAL_NULLPTR{};

 private:
  std::atomic_flag m_synchronizer = ATOMIC_FLAG_INIT;
  uint32_t m_usedListHead{INVALID_INDEX};
  uint32_t m_freeListHead{0u};
  uint32_t m_listIndices[Capacity];
  DataElement_t m_listData[Capacity];
};

}  // namespace entity
}  // namespace shm