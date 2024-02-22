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

template <uint32_t Capacity>
constexpr typename UsedChunkList<Capacity>::DataElement_t
    UsedChunkList<Capacity>::DATA_ELEMENT_LOGICAL_NULLPTR;

template <uint32_t Capacity>
UsedChunkList<Capacity>::UsedChunkList() noexcept {
  static_assert(sizeof(DataElement_t) <= 8U,
                "The size of the data element type must not exceed 64 bit!");
  static_assert(std::is_trivially_copyable<DataElement_t>::value,
                "The data element type must be trivially copyable!");

  init();
}

template <uint32_t Capacity>
bool UsedChunkList<Capacity>::insert(memory::SharedChunk chunk) noexcept {
  auto hasFreeSpace = m_freeListHead != INVALID_INDEX;
  if (hasFreeSpace) {
    // get next free entry after freelistHead
    auto nextFree = m_listIndices[m_freeListHead];

    // freeListHead is getting new usedListHead, next of this entry is updated
    // to next in usedList
    m_listIndices[m_freeListHead] = m_usedListHead;
    m_usedListHead = m_freeListHead;

    m_listData[m_usedListHead] = DataElement_t(chunk);

    // set freeListHead to the next free entry
    m_freeListHead = nextFree;

    /// @todo iox-#623 can we do this cheaper with a global fence in cleanup?
    m_synchronizer.clear(std::memory_order_release);
    return true;
  } else {
    return false;
  }
}

template <uint32_t Capacity>
bool UsedChunkList<Capacity>::remove(const memory::ChunkHeader* chunkHeader,
                                     memory::SharedChunk& chunk) noexcept {
  auto previous = INVALID_INDEX;

  // go through usedList with stored chunks
  for (auto current = m_usedListHead; current != INVALID_INDEX;
       current = m_listIndices[current]) {
    if (!m_listData[current].isLogicalNullptr()) {
      // does the entry match the one we want to remove?
      if (m_listData[current].getChunkHeader() == chunkHeader) {
        chunk = m_listData[current].releaseToSharedChunk();

        // remove index from used list
        if (current == m_usedListHead) {
          m_usedListHead = m_listIndices[current];
        } else {
          m_listIndices[previous] = m_listIndices[current];
        }

        // insert index to free list
        m_listIndices[current] = m_freeListHead;
        m_freeListHead = current;

        /// @todo iox-#623 can we do this cheaper with a global fence in
        /// cleanup?
        m_synchronizer.clear(std::memory_order_release);
        return true;
      }
    }
    previous = current;
  }
  return false;
}

template <uint32_t Capacity>
void UsedChunkList<Capacity>::cleanup() noexcept {
  m_synchronizer.test_and_set(std::memory_order_acquire);

  for (auto& data : m_listData) {
    if (!data.isLogicalNullptr()) {
      // release ownership by creating a SharedChunk
      data.releaseToSharedChunk();
    }
  }

  init();  // just to save us from the future self
}

template <uint32_t Capacity>
void UsedChunkList<Capacity>::init() noexcept {
  // build list
  for (uint32_t i = 0U; i < Capacity; ++i) {
    m_listIndices[i] = i + 1u;
  }

  if (Capacity > 0U) {
    m_listIndices[Capacity - 1U] =
        INVALID_INDEX;  // just to save us from the future self
  } else {
    m_listIndices[0U] = INVALID_INDEX;
  }

  m_usedListHead = INVALID_INDEX;
  m_freeListHead = 0U;

  // clear data
  for (auto& data : m_listData) {
    data.releaseToSharedChunk();
  }

  m_synchronizer.clear(std::memory_order_release);
}

}  // namespace entity
}  // namespace shm