#include "entity/shm_safe_unmanaged_chunk.hpp"

#include "shm/relative_pointer.hpp"

namespace shm {
namespace entity {

// Torn writes are problematic since RouDi needs to cleanup all chunks when an
// application crashes. If the size is larger than 8 bytes on a 64 bit system,
// torn writes happens and the data is only partially written when the
// application crashes at the wrong time. RouDi would then read corrupt data and
// try to access invalid memory.
static_assert(sizeof(ShmSafeUnmanagedChunk) <= 8U,
              "The ShmSafeUnmanagedChunk size must not exceed 64 bit to "
              "prevent torn writes!");
// This ensures that the address of the ShmSafeUnmanagedChunk object is
// appropriately aligned to be accessed within one CPU cycle, i.e. if the size
// is 8 and the alignment is 4 it could be placed at an address with modulo 4
// which would also result in torn writes.
static_assert(sizeof(ShmSafeUnmanagedChunk) == alignof(ShmSafeUnmanagedChunk),
              "A ShmSafeUnmanagedChunk must be placed on an address which does "
              "not cross the native alignment!");
// This is important for the use in the SOFI where under some conditions the
// copy operation could work on partially obsolet data and therefore non-trivial
// copy ctor/assignment operator or dtor would work on corrupted data.
static_assert(std::is_trivially_copyable<ShmSafeUnmanagedChunk>::value,
              "The ShmSafeUnmanagedChunk must be trivially copyable to prevent "
              "Frankenstein objects when the copy ctor "
              "works on half dead objects!");

ShmSafeUnmanagedChunk::ShmSafeUnmanagedChunk(
    memory::SharedChunk chunk) noexcept {
  // this is only necessary if it's not an empty chunk
  if (chunk) {
    details::RelativePointer<memory::ChunkManagement> ptr{chunk.release()};
    auto id = ptr.getId();
    auto offset = ptr.getOffset();
    ENSURES(id <= details::RelativePointerData::ID_RANGE &&
            "RelativePointer id must fit into id type!");
    ENSURES(offset <= details::RelativePointerData::OFFSET_RANGE &&
            "RelativePointer offset must fit into offset type!");
    /// @todo iox-#1196 Unify types to uint64_t
    m_chunkManagement = details::RelativePointerData(
        static_cast<details::RelativePointerData::identifier_t>(id), offset);
  }
}

memory::SharedChunk ShmSafeUnmanagedChunk::releaseToSharedChunk() noexcept {
  if (m_chunkManagement.isLogicalNullptr()) {
    return memory::SharedChunk();
  }
  auto chunkMgmt = details::RelativePointer<memory::ChunkManagement>(
      m_chunkManagement.offset(),
      details::segment_id_t{m_chunkManagement.id()});
  m_chunkManagement.reset();
  return memory::SharedChunk(chunkMgmt.get());
}

memory::SharedChunk ShmSafeUnmanagedChunk::cloneToSharedChunk() noexcept {
  if (m_chunkManagement.isLogicalNullptr()) {
    return memory::SharedChunk();
  }
  auto chunkMgmt = details::RelativePointer<memory::ChunkManagement>(
      m_chunkManagement.offset(),
      details::segment_id_t{m_chunkManagement.id()});
#if (defined(__GNUC__) && __GNUC__ == 13 && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
  chunkMgmt->m_referenceCounter.fetch_add(1U, std::memory_order_relaxed);
#if (defined(__GNUC__) && __GNUC__ == 13 && !defined(__clang__))
#pragma GCC diagnostic pop
#endif
  return memory::SharedChunk(chunkMgmt.get());
}

bool ShmSafeUnmanagedChunk::isLogicalNullptr() const noexcept {
  return m_chunkManagement.isLogicalNullptr();
}

memory::ChunkHeader* ShmSafeUnmanagedChunk::getChunkHeader() noexcept {
  if (m_chunkManagement.isLogicalNullptr()) {
    return nullptr;
  }
  auto chunkMgmt = details::RelativePointer<memory::ChunkManagement>(
      m_chunkManagement.offset(),
      details::segment_id_t{m_chunkManagement.id()});
  return chunkMgmt->m_chunkHeader.get();
}

const memory::ChunkHeader* ShmSafeUnmanagedChunk::getChunkHeader()
    const noexcept {
  return const_cast<ShmSafeUnmanagedChunk*>(this)->getChunkHeader();
}

bool ShmSafeUnmanagedChunk::isNotLogicalNullptrAndHasNoOtherOwners()
    const noexcept {
  if (m_chunkManagement.isLogicalNullptr()) {
    return false;
  }

  auto chunkMgmt = details::RelativePointer<memory::ChunkManagement>(
      m_chunkManagement.offset(),
      details::segment_id_t{m_chunkManagement.id()});
  return chunkMgmt->m_referenceCounter.load(std::memory_order_relaxed) == 1U;
}

}  // namespace entity
}  // namespace shm