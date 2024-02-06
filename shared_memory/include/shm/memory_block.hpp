

#pragma once

#include "shm/algorithm.hpp"
#include "types/optional.hpp"

namespace shm {
namespace details {

/// @brief The MemoryBlock is a container for general purpose memory. It is used
/// to request some memory from a MemoryProvider, which can be POSIX SHM, the
/// stack or something completely different. To be able to use the container,
/// some functions need to be implemented. For most use cases the
/// GenericMemoryBlock can be used, which is a templated class and implements
/// the most common case.
class MemoryBlock {
  friend class MemoryProvider;

 public:
  MemoryBlock() noexcept = default;
  virtual ~MemoryBlock() noexcept = default;

  /// @note this is intentional not movable/copyable, since a pointer to the
  /// memory block is registered at a MemoryProvider and therefore an instance
  /// of a MemoryBlock must be pinned to memory
  MemoryBlock(const MemoryBlock&) = delete;
  MemoryBlock(MemoryBlock&&) = delete;
  MemoryBlock& operator=(const MemoryBlock&) = delete;
  MemoryBlock& operator=(MemoryBlock&&) = delete;

  /// @brief This function provides the size of the required memory for the
  /// underlying data. It is needed for the MemoryProvider to calculate the
  /// total size of memory.
  /// @return the required memory as multiple of the alignment
  virtual uint64_t size() const noexcept = 0;

  /// @brief This function provides the alignment of the memory for the
  /// underlying data. This information is needed for the MemoryProvider
  /// @return the alignment of the underlying data.
  virtual uint64_t alignment() const noexcept = 0;

  /// @brief This function provides the pointer to the requested memory.
  /// @return an optional pointer to a memory block with the requested size and
  /// alignment if the memory is available, otherwise a nullopt_t
  tl::optional<void*> memory() const noexcept;

 protected:
  /// @brief The MemoryProvider calls this either when MemoryProvider::destroy
  /// is called or in its destructor.
  /// @note This function can be called multiple times. Make sure that the
  /// implementation can handle this.
  virtual void destroy() noexcept = 0;

  /// @brief This function is called once the memory is available and is
  /// therefore the earliest possibility to use the memory.
  /// @param [in] memory pointer to a valid memory block, the same one that the
  /// memory() member function would return
  virtual void onMemoryAvailable(not_null<void*> memory) noexcept;

 private:
  void* m_memory{nullptr};
};

}  // namespace details
}  // namespace shm