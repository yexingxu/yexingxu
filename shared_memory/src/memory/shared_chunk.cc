

#include "memory/shared_chunk.hpp"

#include "memory/chunk_header.hpp"
#include "memory/memory_pool.hpp"

namespace shm {
namespace memory {

SharedChunk::SharedChunk(ChunkManagement* const resource) noexcept
    : m_chunkManagement(resource) {}

SharedChunk::SharedChunk(const SharedChunk& rhs) noexcept { *this = rhs; }

SharedChunk::SharedChunk(SharedChunk&& rhs) noexcept { *this = std::move(rhs); }

SharedChunk::~SharedChunk() noexcept { decrementReferenceCounter(); }

void SharedChunk::incrementReferenceCounter() noexcept {
  if (m_chunkManagement != nullptr) {
    m_chunkManagement->m_referenceCounter.fetch_add(1U,
                                                    std::memory_order_relaxed);
  }
}

void SharedChunk::decrementReferenceCounter() noexcept {
  if ((m_chunkManagement != nullptr) &&
      (m_chunkManagement->m_referenceCounter.fetch_sub(
           1U, std::memory_order_relaxed) == 1U)) {
    freeChunk();
  }
}

void SharedChunk::freeChunk() noexcept {
  m_chunkManagement->m_mempool->freeChunk(
      static_cast<void*>(m_chunkManagement->m_chunkHeader.get()));
  m_chunkManagement->m_chunkManagementPool->freeChunk(m_chunkManagement);
  m_chunkManagement = nullptr;
}

SharedChunk& SharedChunk::operator=(const SharedChunk& rhs) noexcept {
  if (this != &rhs) {
    decrementReferenceCounter();
    m_chunkManagement = rhs.m_chunkManagement;
    incrementReferenceCounter();
  }
  return *this;
}

SharedChunk& SharedChunk::operator=(SharedChunk&& rhs) noexcept {
  if (this != &rhs) {
    decrementReferenceCounter();
    m_chunkManagement = std::move(rhs.m_chunkManagement);
    rhs.m_chunkManagement = nullptr;
  }
  return *this;
}

void* SharedChunk::getUserPayload() const noexcept {
  if (m_chunkManagement == nullptr) {
    return nullptr;
  } else {
    return m_chunkManagement->m_chunkHeader->userPayload();
  }
}

bool SharedChunk::operator==(const SharedChunk& rhs) const noexcept {
  return m_chunkManagement == rhs.m_chunkManagement;
}

bool SharedChunk::operator==(const void* const rhs) const noexcept {
  return getUserPayload() == rhs;
}

bool SharedChunk::operator!=(const SharedChunk& rhs) const noexcept {
  return !(*this == rhs);
}

bool SharedChunk::operator!=(const void* const rhs) const noexcept {
  return !(*this == rhs);
}

SharedChunk::operator bool() const noexcept {
  return m_chunkManagement != nullptr;
}

ChunkHeader* SharedChunk::getChunkHeader() const noexcept {
  if (m_chunkManagement != nullptr) {
    return m_chunkManagement->m_chunkHeader.get();
  } else {
    return nullptr;
  }
}

ChunkManagement* SharedChunk::release() noexcept {
  ChunkManagement* returnValue = m_chunkManagement;
  m_chunkManagement = nullptr;
  return returnValue;
}

}  // namespace memory
}  // namespace shm